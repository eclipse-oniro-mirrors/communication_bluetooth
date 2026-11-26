/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOG_TAG
#define LOG_TAG "MultiA2dpManager"
#endif

#include "multi_a2dp_manager.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "log.h"
#include "bluetooth_def.h"
#include "common_util.h"

#ifdef MULTI_A2DP_ENABLED
#include "zone_audio_interface.h"
#include "display_manager.h"
#ifdef BLUETOOTH_OS_ACCOUNT
#include "os_account_manager.h"
#endif

namespace OHOS {
namespace Bluetooth {

using bluetooth::ZoneAudioDevice;
using bluetooth::ZoneAudioDeviceChangeFunc;
using bluetooth::ZoneAudioInterface;
using bluetooth::ZONE_AUDIO_DEVICE_STATUS_CONNECT;
using bluetooth::ZONE_AUDIO_DEVICE_STATUS_DISCONNECT;
using bluetooth::ZONE_AUDIO_INTERFACE_SUCCESS;

namespace {
    std::mutex g_deviceDisplayMapMutex;
    std::map<std::string, int32_t> g_addrToDisplayId;
    std::map<int32_t, std::string> g_displayIdToAddr;

    // Init 失败重试：AVRCP TG enable 时 audio policy SA / 双屏可能未就绪。
    // 采用"时间退避后台重试 + 连接事件兜底"，重试移出查询路径：
    //   - MultiA2dpManager::Init() 失败即启动 RetryLoop（1s→2s→…→10s 封顶，上限 6 次，
    //     总时长约 35s，仅覆盖开机双屏/SA 未就绪竞态窗口；长期兜底靠 A2DP 连接事件重触发）；
    //   - Disable/Cleanup 经 StopRetry() 置位 + 条件变量唤醒，立即中止并等待退出；
    //   - A2DP 连接回调（ProcessA2dpHdfLoad CONNECTED，见 a2dp_service.cpp）再调 Init() 兜底。
    std::mutex g_initMutex;
    std::mutex g_retryMutex;
    std::condition_variable g_retryCv;
    std::atomic<bool> g_retryRunning{false};
    std::atomic<bool> g_retryStop{false};
    constexpr std::chrono::milliseconds RETRY_INITIAL_DELAY{1000};
    constexpr std::chrono::milliseconds RETRY_MAX_DELAY{10000};
    constexpr int32_t RETRY_MAX_RETRY = 6;

    constexpr int32_t INVALID_USER_ID = -1;
    constexpr int32_t MAIN_DRIVER_USER_ID = 100;

    // Only co-driver and rear-seat displays support BT headset in car Multi A2DP scenario.
    constexpr const char *DISPLAY_NAME_CO_DRIVER = "co-driver_panel";
    constexpr const char *DISPLAY_NAME_CENTRAL_REAR = "central_rear_panel";
    constexpr const char *DISPLAY_NAME_REAR_ARMREST = "rear_armrest_panel";

    bool IsMultiA2dpDisplay(const std::string &name)
    {
        return name == DISPLAY_NAME_CO_DRIVER || name == DISPLAY_NAME_CENTRAL_REAR ||
               name == DISPLAY_NAME_REAR_ARMREST;
    }

    std::mutex g_displayUserMapMutex;
    /* 地址统一转小写作为 map 键：栈侧 ToString 输出大写、audio 侧可能为小写。 */
    std::string NormalizeAddr(const std::string &addr)
    {
        std::string norm = addr;
        for (auto &ch : norm) {
            if (ch >= 'A' && ch <= 'Z') {
                ch = static_cast<char>(ch - 'A' + 'a');
            }
        }
        return norm;
    }

    std::map<int32_t, int32_t> g_displayIdToUserId;

    std::mutex g_displayCallbackMapMutex;
    std::map<int32_t, std::shared_ptr<class BtAudioDeviceChangeCallback>> g_deviceCallbackMap;

    std::mutex g_deviceBoundCallbackMutex;
    MultiA2dpManager::DeviceBoundCallback g_deviceBoundCallback;

    void NotifyDeviceBound(const std::string &addr)
    {
        MultiA2dpManager::DeviceBoundCallback callback;
        {
            std::lock_guard<std::mutex> lock(g_deviceBoundCallbackMutex);
            callback = g_deviceBoundCallback;
        }
        if (callback) {
            callback(addr);
        }
    }

#ifdef BLUETOOTH_OS_ACCOUNT
void CollectForegroundAccounts(std::map<int32_t, int32_t> &newMap)
{
        auto &displayMgr = OHOS::Rosen::DisplayManager::GetInstance();
        std::vector<AccountSA::ForegroundOsAccount> accounts;
        if (AccountSA::OsAccountManager::GetForegroundOsAccounts(accounts) == 0 && !accounts.empty()) {
            for (const auto &acc : accounts) {
                int32_t displayId = static_cast<int32_t>(acc.displayId);
                auto display = displayMgr.GetDisplayById(static_cast<OHOS::Rosen::DisplayId>(displayId));
                if (display == nullptr) {
                    continue;
                }
                std::string name = display->GetName();
                HILOGI("foreground account localId=%{public}d, displayId=%{public}d, name=%{public}s, "
                    "multiA2dp=%{public}d", acc.localId, displayId, name.c_str(),
                    IsMultiA2dpDisplay(name) ? 1 : 0);
                if (!IsMultiA2dpDisplay(name)) {
                    continue;
                }
                newMap[displayId] = acc.localId;
            }
        }
}
#endif

void CollectByDisplayEnumeration(std::map<int32_t, int32_t> &newMap)
{
        auto &displayMgr = OHOS::Rosen::DisplayManager::GetInstance();
        std::vector<int32_t> userIds;
#ifdef BLUETOOTH_OS_ACCOUNT
        if (AccountSA::OsAccountManager::GetOsAccountLocalIds(userIds) != 0) {
            HILOGE("GetOsAccountLocalIds failed, fallback to display enumeration");
        }
#endif
        if (userIds.empty()) {
            userIds.push_back(MAIN_DRIVER_USER_ID);
        }
        for (auto userId : userIds) {
            auto displays = displayMgr.GetAllDisplays(userId);
            for (const auto &display : displays) {
                if (display == nullptr) {
                    continue;
                }
                int32_t displayId = static_cast<int32_t>(display->GetId());
                std::string name = display->GetName();
                if (!IsMultiA2dpDisplay(name)) {
                    continue;
                }
                if (newMap.find(displayId) == newMap.end()) {
                    newMap[displayId] = userId;
                }
            }
        }
}

    // 全量重建 displayId→userId 映射：前台账号与全账号枚举两路合并（互补），
    // 前台账号接口可能漏报（实测存在已上线账号不在返回列表的情况），
    // 枚举兜底补齐其余多A2DP屏。查询异常返回空集时保留上一份映射。
    void BuildDisplayUserMap()
    {
        std::map<int32_t, int32_t> newMap;
#ifdef BLUETOOTH_OS_ACCOUNT
        CollectForegroundAccounts(newMap);
#endif
        CollectByDisplayEnumeration(newMap);
        std::lock_guard<std::mutex> lock(g_displayUserMapMutex);
        if (newMap.empty() && !g_displayIdToUserId.empty()) {
            HILOGW("BuildDisplayUserMap got empty result, keep last map");
            return;
        }
        g_displayIdToUserId = std::move(newMap);
        for (const auto &entry : g_displayIdToUserId) {
            HILOGI("displayUserMap: displayId=%{public}d -> userId=%{public}d", entry.first, entry.second);
        }
        HILOGI("BuildDisplayUserMap done, size=%{public}d", static_cast<int32_t>(g_displayIdToUserId.size()));
    }

class BtAudioDeviceChangeCallback {
public:
    explicit BtAudioDeviceChangeCallback(int32_t displayId) : displayId_(displayId) {}

    void OnDeviceChange(const ZoneAudioDevice *devices, uint32_t deviceCount)
    {
        for (uint32_t i = 0; i < deviceCount; ++i) {
            const ZoneAudioDevice &dev = devices[i];
            std::string addr = dev.deviceAddress;
            if (addr.empty()) continue;
            if (dev.status == ZONE_AUDIO_DEVICE_STATUS_CONNECT) {
                {
                    std::lock_guard<std::mutex> lock(g_deviceDisplayMapMutex);
                    g_addrToDisplayId[NormalizeAddr(addr)] = displayId_;
                    g_displayIdToAddr[displayId_] = NormalizeAddr(addr);
                    HILOGI("Device connected: addr=%{public}s, displayId=%{public}d",
                           bluetooth::GetEncryptAddr(addr).c_str(), displayId_);
                }
                /* The mapping must be visible before notifying, so a concurrent AVRCP connect
                 * that misses this notification still sees the bound display. */
                NotifyDeviceBound(addr);
            } else if (dev.status == ZONE_AUDIO_DEVICE_STATUS_DISCONNECT) {
                std::lock_guard<std::mutex> lock(g_deviceDisplayMapMutex);
                g_addrToDisplayId.erase(NormalizeAddr(addr));
                g_displayIdToAddr.erase(displayId_);
                HILOGI("Device disconnected: addr=%{public}s, displayId=%{public}d",
                       bluetooth::GetEncryptAddr(addr).c_str(), displayId_);
            }
        }
    }

private:
    int32_t displayId_;
};

// vendor 侧设备变化事件的 C 回调入口：userData 即注册时传入的回调对象，
// 由 vendor_interface 桥接层（bt_vendor_service）经 dlsym 调用。
void OnZoneAudioDeviceChange(int32_t displayId, const ZoneAudioDevice *devices,
    uint32_t deviceCount, void *userData)
{
    auto *callback = static_cast<BtAudioDeviceChangeCallback *>(userData);
    if (callback == nullptr || devices == nullptr) {
        HILOGE("invalid zone audio callback, displayId=%{public}d", displayId);
        return;
    }
    callback->OnDeviceChange(devices, deviceCount);
}

// 初始化/刷新真正实现：每次调用（TG enable、A2DP 连接兜底、退避重试）都全量重建映射，
// 再按"只增不删"补注册新增 displayId 的设备回调，幂等可重入：
//   - 副驾/后排账号晚于开机 Init 上线时，开机一次性构建的映射会缺失新 displayId
//     （如 displayId=6），导致 ivi_audio 按 6 推送的设备事件无回调接收、被静默丢弃；
//   - ivi_audio 推送事件所用 displayId 与 Rosen/账号体系同源（调用方经
//     getForegroundOsAccountDisplayId 取得），两侧不会漂移，无需跨侧对齐；
//   - 已注册的不重复注册，失败的不影响已成功的，返回 false 由退避重试/下次连接续办。
bool DoInit()
{
    std::lock_guard<std::mutex> lock(g_initMutex);

    //初始化副驾后排displayid和userId的映射关系。
    BuildDisplayUserMap();

    std::set<int32_t> desired;
    {
        std::lock_guard<std::mutex> mapLock(g_displayUserMapMutex);
        if (g_displayIdToUserId.empty()) {
            // 双屏尚未创建（开机早期/热插未就绪）：交由退避重试/连接兜底。
            HILOGE("dual display map empty, not ready");
            return false;
        }
        for (const auto &entry : g_displayIdToUserId) {
            desired.insert(entry.first);
        }
    }

    // 注：分区音频经 vendor_interface 桥接注册（闭源插件 libbt_vendor_service.z.so
    // 内部对接 AutoZoneManager）；vendor 库不可用（非车产品/符号缺失）时注册失败，
    // 交由退避重试/连接兜底逻辑处理，不在此处区分失败原因。
    bool allRegistered = true;
    std::lock_guard<std::mutex> cbLock(g_displayCallbackMapMutex);
    for (auto displayId : desired) {
        if (g_deviceCallbackMap.find(displayId) != g_deviceCallbackMap.end()) {
            continue;
        }
        auto deviceCb = std::make_shared<BtAudioDeviceChangeCallback>(displayId);
        if (ZoneAudioInterface::RegisterDeviceChangeCallback(displayId,
            OnZoneAudioDeviceChange, deviceCb.get()) != ZONE_AUDIO_INTERFACE_SUCCESS) {
            HILOGE("RegisterDeviceChangeCallback failed, displayId=%{public}d", displayId);
            allRegistered = false;
            continue;
        }
        g_deviceCallbackMap[displayId] = deviceCb;
        HILOGI("registered device callback, displayId=%{public}d", displayId);
    }

    HILOGI("MultiA2dpManager::DoInit done, allRegistered=%{public}d", allRegistered ? 1 : 0);
    return allRegistered;
}

// 时间退避重试线程主体：1s 起倍增、10s 封顶，上限 RETRY_MAX_RETRY 次；成功或
// StopRetry() 置位（Disable/Cleanup）即退出，条件变量保证中止近乎即时。
void RetryLoop()
{
    std::unique_lock<std::mutex> lock(g_retryMutex);
    std::chrono::milliseconds delay = RETRY_INITIAL_DELAY;
    int32_t attempt = 0;
    while (!g_retryStop.load()) {
        if (g_retryCv.wait_for(lock, delay, [&]() { return g_retryStop.load(); })) {
            break;
        }
        attempt++;
        if (attempt > RETRY_MAX_RETRY) {
            HILOGE("MultiA2dpManager retry gave up after %{public}d attempts", RETRY_MAX_RETRY);
            break;
        }
        lock.unlock();
        bool ok = DoInit();
        lock.lock();
        if (ok) {
            HILOGI("MultiA2dpManager retry recovered after %{public}d attempts", attempt);
            break;
        }
        if (delay < RETRY_MAX_DELAY) {
            delay = std::min(delay * 2, RETRY_MAX_DELAY);
        }
    }
    g_retryRunning.store(false);
    g_retryCv.notify_all();
}

void ScheduleRetry()
{
    std::lock_guard<std::mutex> lock(g_retryMutex);
    if (g_retryRunning.load()) {
        return;
    }
    g_retryStop.store(false);
    g_retryRunning.store(true);
    std::thread(RetryLoop).detach();
}

// 中止重试并等待工作线程归还 running 标志（条件变量唤醒，通常立即返回）。
// 等待设 2s 超时：DoInit 内含跨进程调用（账号/显示/AutoZoneManager IPC），
// 卡死时不能让 Cleanup 永久挂起；超时后重试线程仍会因 g_retryStop 在下轮退出。
void StopRetry()
{
    {
        std::lock_guard<std::mutex> lock(g_retryMutex);
        g_retryStop.store(true);
    }
    g_retryCv.notify_all();
    std::unique_lock<std::mutex> lock(g_retryMutex);
    if (!g_retryCv.wait_for(lock, std::chrono::seconds(2), [&]() { return !g_retryRunning.load(); })) {
        HILOGE("MultiA2dpManager StopRetry wait timeout, retry thread may be stuck in DoInit");
    }
}

}  // anonymous namespace

void MultiA2dpManager::Init()
{
    HILOGI("MultiA2dpManager::Init start");
    if (!DoInit()) {
        ScheduleRetry();
    }
}

void MultiA2dpManager::Cleanup()
{
    StopRetry();
    {
        std::lock_guard<std::mutex> cbLock(g_displayCallbackMapMutex);
        for (const auto &entry : g_deviceCallbackMap) {
            ZoneAudioInterface::UnregisterDeviceChangeCallback(entry.first);
        }
        g_deviceCallbackMap.clear();
    }

    {
        std::lock_guard<std::mutex> mapLock(g_deviceDisplayMapMutex);
        g_addrToDisplayId.clear();
        g_displayIdToAddr.clear();
    }
    {
        std::lock_guard<std::mutex> lock(g_displayUserMapMutex);
        g_displayIdToUserId.clear();
    }

    HILOGI("MultiA2dpManager::Cleanup success");
}

void MultiA2dpManager::RegisterDeviceBoundCallback(DeviceBoundCallback callback)
{
    std::lock_guard<std::mutex> lock(g_deviceBoundCallbackMutex);
    g_deviceBoundCallback = std::move(callback);
}

void MultiA2dpManager::UnregisterDeviceBoundCallback()
{
    std::lock_guard<std::mutex> lock(g_deviceBoundCallbackMutex);
    g_deviceBoundCallback = nullptr;
}

int32_t MultiA2dpManager::GetDisplayIdByAddr(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_deviceDisplayMapMutex);
    auto it = g_addrToDisplayId.find(NormalizeAddr(addr));
    if (it != g_addrToDisplayId.end()) {
        return it->second;
    }
    return INVALID_USER_ID;
}

std::string MultiA2dpManager::GetAddrByDisplayId(int32_t displayId)
{
    std::lock_guard<std::mutex> lock(g_deviceDisplayMapMutex);
    auto it = g_displayIdToAddr.find(displayId);
    if (it != g_displayIdToAddr.end()) {
        return it->second;
    }
    return "";
}

int32_t MultiA2dpManager::GetUserIdByDisplayId(int32_t displayId)
{
    std::lock_guard<std::mutex> lock(g_displayUserMapMutex);
    auto it = g_displayIdToUserId.find(displayId);
    if (it != g_displayIdToUserId.end()) {
        return it->second;
    }
    return INVALID_USER_ID;
}

int32_t MultiA2dpManager::GetUserIdByAddr(const std::string &addr)
{
    return GetUserIdByDisplayId(GetDisplayIdByAddr(addr));
}

int32_t MultiA2dpManager::GetDisplayIdByUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(g_displayUserMapMutex);
    for (const auto &entry : g_displayIdToUserId) {
        if (entry.second == userId) {
            return entry.first;
        }
    }
    return INVALID_USER_ID;
}

}  // namespace Bluetooth
}  // namespace OHOS

#else  // MULTI_A2DP_ENABLED

namespace OHOS {
namespace Bluetooth {

void MultiA2dpManager::Init() {}
void MultiA2dpManager::Cleanup() {}
void MultiA2dpManager::RegisterDeviceBoundCallback(DeviceBoundCallback) {}
void MultiA2dpManager::UnregisterDeviceBoundCallback() {}
int32_t MultiA2dpManager::GetDisplayIdByAddr(const std::string &) { return 0; }
std::string MultiA2dpManager::GetAddrByDisplayId(int32_t) { return ""; }
int32_t MultiA2dpManager::GetUserIdByDisplayId(int32_t) { return -1; }
int32_t MultiA2dpManager::GetUserIdByAddr(const std::string &) { return -1; }
int32_t MultiA2dpManager::GetDisplayIdByUserId(int32_t) { return -1; }

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // MULTI_A2DP_ENABLED
