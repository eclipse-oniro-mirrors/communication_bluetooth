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
#define LOG_TAG "bt_chr_local_data"
#endif

#include "bt_chr_local_data.h"

#include <atomic>
#include <chrono>
#include <mutex>

#include "bluetooth_log.h"

#ifdef BLUETOOTH_TELEPHONY_EXIST
#include "core_service_client.h"
#include "network_state.h"
#include "string_ex.h"
#include "telephony_errors.h"
#include "telephony_observer.h"
#include "telephony_observer_client.h"
#endif

namespace OHOS {
namespace bluetooth {
namespace {
// 国家码通道：向 CHR 打点提供本端运营商信息，最终格式为 "ISO_MCC_MNC"：
//   ISO : SIM 卡的 ISO 3166-1 两位国家码（GetISOCountryCodeForSim 返回），如 "CN"、"HK"、"US"
//   MCC : Mobile Country Code，PLMN 数字前 3 位，如 460（中国）、454（中国香港）、310（美国）
//   MNC : Mobile Network Code，PLMN 数字去掉 MCC 后的剩余位（2~3 位），如 00（中国移动）、11（中国电信）
// 取数示例：
//   "CN_460_00" 中国移动    "CN_460_11" 中国电信    "HK_454_00" 中国香港
//   "US_310_260" 美国 T-Mobile（MNC 为 3 位）
// 漫游示例：中国 SIM 卡漫游到美国 T-Mobile 网络时，ISO 取 SIM 归属地 "CN"，
//           网络取当前驻网 "310260"，最终为 "CN_310_260"
//
// 取数链路：主卡槽 → SIM ISO 国家码 → 当前驻网 PLMN 数字 → 拼接后缓存
constexpr size_t MCC_LEN = 3; // plmnNumeric 前 3 位为 MCC，如 "46000" 中 MCC 为 "460"
const std::string COUNTRY_INFO_SEPARATOR = "_";
// 取数/订阅失败后 30s 重试冷却，避免频繁 IPC：
// 例：SIM 未插入时首次取数失败，随后 30s 内的上报不再发起 IPC，直接回退缓存（空串）
constexpr int64_t FETCH_FAIL_COOLDOWN_MS = 30000;
// 冷却时间戳哨兵：0 表示未发生（与稳态时钟零点无关），判据对 0 豁免冷却
constexpr int64_t FAIL_TIME_NONE = 0;
// 本地返回码（非 telephony 错误码域）：本次未发起注册（停服/节流/未编译 telephony 依赖）
constexpr int32_t REGISTER_RET_NOT_ATTEMPTED = -1;

// 服务运行状态，注册与取数的同源判据（Start 置 true，Stop 置 false）
// 用 atomic 让高频上报路径不抢锁即可快速判停服：停服后 GetCountryInfo 直接返回空串
// 生命周期示例：蓝牙 SA 启动 → Start() 置 true；SA 停止 → Stop() 置 false
std::atomic<bool> g_serviceRunning = false;

#ifdef BLUETOOTH_TELEPHONY_EXIST
constexpr uint32_t COUNTRY_INFO_OBSERVER_MASK = Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE |
    Telephony::TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
constexpr int32_t SLOT_ID_FOR_ALL = -1; // -1 监听全部卡槽，避免切卡后漏掉状态变化

// SIM/网络状态变化订阅者：注册时 isUpdate=true 会立即回调一次当前状态（预热缓存）；
// 此后拔插 SIM、切主卡、漫游换网等动作都会触发回调，重新取数刷新缓存
class ChrTelephonyStateObserver : public Telephony::TelephonyObserver {
public:
    void OnSimStateUpdated(int32_t slotId, Telephony::CardType type, Telephony::SimState state,
        Telephony::LockReason reason) override
    {
        BtChrLocalData::GetInstance().RefreshCountryInfo();
    }

    void OnNetworkStateUpdated(int32_t slotId, const sptr<Telephony::NetworkState> &networkState) override
    {
        BtChrLocalData::GetInstance().RefreshCountryInfo();
    }
};

// 持有观察者引用，保证回调期间对象存活
sptr<Telephony::TelephonyObserver> g_countryInfoObserver = nullptr;
// 订阅注册状态，供上报路径补注册
std::atomic<bool> g_countryInfoObserverRegistered = false;
// 只保护注册状态与节流判定，跨进程注册/取消在锁外执行，避免持锁等待 IPC
std::mutex g_observerMutex;
// 补注册节流时间戳，由 g_observerMutex 保护
int64_t g_lastRegisterTryTimeMs = FAIL_TIME_NONE;
// 注册在飞标记，由 g_observerMutex 保护；停服复位只清节流不清它，避免在飞注册跨 Stop→Start 并发两条
bool g_registerInFlight = false;
#endif
}  // namespace

BtChrLocalData &BtChrLocalData::GetInstance()
{
    static BtChrLocalData instance;
    return instance;
}

void BtChrLocalData::SetServiceRunning(bool running)
{
    g_serviceRunning = running;
}

void BtChrLocalData::Start()
{
    // 先置运行态再注册，注册与停服的判据同源。
    // 由 BluetoothHostServer::Init() 在蓝牙服务进程启动时调用（不跟随蓝牙开关）：
    //   注册观察者(isUpdate=true) → Telephony 立即回调当前状态 → RefreshCountryInfo() 预热缓存，
    //   如启动后首个断链事件到来时缓存已是 "CN_460_00"，直接命中
    SetServiceRunning(true);
    RegisterCountryInfoObserver();
}

void BtChrLocalData::Stop()
{
    // 先置停服态并取消订阅，每条出口都生效；停服后上报路径不再重建订阅。
    // 由 BluetoothHostServer::Clear() 在服务进程停止时调用：
    //   置 false 后 GetCountryInfo 直接返回空串，在飞取数的结果也会被 gen_ 判为陈旧丢弃
    SetServiceRunning(false);
    UnregisterCountryInfoObserver();
}

// 上报路径统一取数入口，被 CHR 白名单事件（BT_USER_DISCONNECT / BT_BLE_DISCONNECT /
// BT_A2DP_AUDIO_EXCEPTION）和开关事件（BT_SWITCH_EXCEPTION）调用。
// 典型时序示例：
//   1) 首个断链事件：缓存为空 → 现场取数 "CN_460_00" 入缓存后返回；
//   2) 后续事件（默认 useCache=true）：命中缓存直接返回，零 IPC，这是高频事件主路径；
//   3) GetCountryInfo(false)：即使已缓存也强制实时取数，如切到电信卡后立即刷新为 "CN_460_11"；
//   4) 取数失败（如无 SIM 卡）：返回上一次缓存（首次为空串，事件不携带该字段），30s 内不再重试
std::string BtChrLocalData::GetCountryInfo(bool useCache)
{
    if (!g_serviceRunning) {
        return ""; // 停服后不再取数，避免把停服窗口的值写回缓存
    }
#ifdef BLUETOOTH_TELEPHONY_EXIST
    if (!g_countryInfoObserverRegistered) {
        RegisterCountryInfoObserver(); // 补注册失败不影响取数
    }
#endif
    uint64_t gen = 0;
    {
        std::lock_guard<std::mutex> lock(countryInfoMutex_);
        gen = gen_; // 先取代际再判运行态：停服复位后取到新代际，随后必观察到停服态
        if (!g_serviceRunning) {
            return ""; // 停服窗口内不再取数，避免把停服期间的值写回缓存
        }
        if (useCache && isLoaded_) {
            return countryInfo_; // 高频事件主路径：命中缓存零 IPC，如直接返回 "CN_460_00"
        }
        if (useCache && lastFetchFailTime_ != FAIL_TIME_NONE &&
            GetCurrentTimeMs() - lastFetchFailTime_ < FETCH_FAIL_COOLDOWN_MS) {
            // 冷却期内回退缓存，避免频繁触发 IPC
            return countryInfo_;
        }
    }
    // 取数不持锁：跨进程查询会阻塞回调刷新与其余上报入口
    std::string countryInfo;
    if (FetchCountryInfo(countryInfo)) {
        std::lock_guard<std::mutex> lock(countryInfoMutex_);
        if (gen == gen_ && (!useCache || !isLoaded_)) {
            // 代际校验拒绝停服复位的陈旧取数；useCache=false 稳态同样刷新缓存，
            // 如已缓存 "CN_460_00"，强制取到 "CN_460_11" 时用新值覆盖
            countryInfo_ = countryInfo;
            isLoaded_ = true;
            lastFetchFailTime_ = FAIL_TIME_NONE;
        }
        return countryInfo_;
    }
    std::lock_guard<std::mutex> lock(countryInfoMutex_);
    lastFetchFailTime_ = GetCurrentTimeMs();
    // 失败回退缓存
    return countryInfo_;
}

// 状态变化回调路径：SIM/网络变化时由观察者调用，重新取数并刷新缓存。
// 乱序示例：两次回调先后起飞 A(seq=1)、B(seq=2)，B 先返回提交 "CN_460_11"（commitSeq=2）；
// A 后返回 "CN_460_00"，因 seq(1) <= commitSeq(2) 被丢弃，缓存不会退回旧值
void BtChrLocalData::RefreshCountryInfo()
{
    if (!g_serviceRunning) {
        return; // 停服后回调不再取数
    }
    uint64_t gen = 0;
    uint64_t seq = 0;
    {
        std::lock_guard<std::mutex> lock(countryInfoMutex_);
        gen = gen_; // 先取代际再判运行态（同上报路径，堵住停服窗口起飞）
        if (!g_serviceRunning) {
            return; // 停服窗口内不再取数
        }
        if (lastRefreshFailTime_ != FAIL_TIME_NONE &&
            GetCurrentTimeMs() - lastRefreshFailTime_ < FETCH_FAIL_COOLDOWN_MS) {
            return; // 冷却期内抑制重复跨进程查询
        }
        seq = ++refreshSeq_;
    }
    std::string countryInfo;
    if (FetchCountryInfo(countryInfo)) {
        std::lock_guard<std::mutex> lock(countryInfoMutex_);
        if (gen != gen_ || seq <= lastRefreshCommitSeq_) {
            return; // 停服复位或比已提交结果更旧的取数，丢弃
        }
        countryInfo_ = countryInfo;
        isLoaded_ = true;
        lastRefreshFailTime_ = FAIL_TIME_NONE;
        lastRefreshCommitSeq_ = seq;
        return;
    }
    std::lock_guard<std::mutex> lock(countryInfoMutex_);
    if (seq == refreshSeq_) { // 仅最后起飞的取数失败才写冷却，旧失败不污染已提交结果
        lastRefreshFailTime_ = GetCurrentTimeMs();
    }
}

// 订阅 SIM/网络状态观察者，调用场景：Start() 首次注册 + 上报路径发现未注册时补注册。
//   Telephony::TELEPHONY_SUCCESS（0） 订阅成功（或此前已订阅）
//   正值                              透传 telephony 错误码（如 Telephony SA 尚未拉起）
//   REGISTER_RET_NOT_ATTEMPTED（-1）  本次未发起：停服/已有注册在飞/30s 节流期内/未编译 telephony
int32_t BtChrLocalData::RegisterCountryInfoObserver()
{
#ifdef BLUETOOTH_TELEPHONY_EXIST
    sptr<Telephony::TelephonyObserver> observer = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        if (!g_serviceRunning || g_registerInFlight) {
            return REGISTER_RET_NOT_ATTEMPTED; // 停服或在飞，本次不发起
        }
        if (g_countryInfoObserverRegistered) {
            return Telephony::TELEPHONY_SUCCESS;
        }
        if (g_lastRegisterTryTimeMs != FAIL_TIME_NONE &&
            GetCurrentTimeMs() - g_lastRegisterTryTimeMs < FETCH_FAIL_COOLDOWN_MS) {
            return REGISTER_RET_NOT_ATTEMPTED; // 节流期内不重复注册
        }
        g_lastRegisterTryTimeMs = GetCurrentTimeMs(); // 先落占用标记再放锁，避免重复注册
        g_registerInFlight = true;
        observer = new ChrTelephonyStateObserver();
    }
    // isUpdate=true 注册后立即收到当前状态回调，完成缓存预热；跨进程注册在锁外执行
    int32_t ret = Telephony::TelephonyObserverClient::GetInstance().AddStateObserver(
        observer, SLOT_ID_FOR_ALL, COUNTRY_INFO_OBSERVER_MASK, true);
    bool needUndo = false;
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        g_registerInFlight = false; // 注册已返回，在飞标记收尾
        if (ret != Telephony::TELEPHONY_SUCCESS) {
            HILOGE("AddStateObserver failed, ret=%{public}d", ret);
            return ret;
        }
        if (!g_serviceRunning) {
            needUndo = true; // 注册完成前服务已停止，撤销刚建立的订阅
        } else {
            g_countryInfoObserver = observer;
            g_countryInfoObserverRegistered = true;
        }
    }
    if (needUndo) {
        int32_t undoRet = Telephony::TelephonyObserverClient::GetInstance().RemoveStateObserver(
            SLOT_ID_FOR_ALL, COUNTRY_INFO_OBSERVER_MASK);
        if (undoRet != Telephony::TELEPHONY_SUCCESS) {
            HILOGE("undo RemoveStateObserver failed, ret=%{public}d", undoRet);
        }
    }
    return needUndo ? REGISTER_RET_NOT_ATTEMPTED : Telephony::TELEPHONY_SUCCESS;
#else
    return REGISTER_RET_NOT_ATTEMPTED;
#endif
}

// 取消订阅并复位缓存，Stop() 调用。
// 复位示例：Stop 时缓存 "CN_460_00" 被清空；若此刻有一个取数还在路上，
// 其返回结果因代际（gen_）不匹配被丢弃，不会写入下一轮生命周期
void BtChrLocalData::UnregisterCountryInfoObserver()
{
#ifdef BLUETOOTH_TELEPHONY_EXIST
    ResetCountryInfo(); // 复位缓存并递增代际，在飞取数提交时将被判为陈旧
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        g_lastRegisterTryTimeMs = FAIL_TIME_NONE; // 复位节流：立即重启可马上注册
        if (!g_countryInfoObserverRegistered) {
            return;
        }
    }
    int32_t ret = Telephony::TelephonyObserverClient::GetInstance().RemoveStateObserver(
        SLOT_ID_FOR_ALL, COUNTRY_INFO_OBSERVER_MASK);
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        // 失败同样清本地状态：失败码无法区分对端是否仍保留注册（SA 重启后内存记录即丢失），
        // 保留状态会堵死重建通道。重建安全性的前提：对端记录存续期间，其持有的 proxy 远端引用
        // 经内核保持旧 broker 存活，回调仍经旧 broker 到达，刷新通道不中断；记录已失时对端按
        // slot/mask/pid 去重会重新建记录，不会产生重复订阅。
        // 例：Remove 返回失败仍置 registered=false，下次 Start/补注册可直接重建，不会因残留状态被拒
        if (ret != Telephony::TELEPHONY_SUCCESS) {
            HILOGE("RemoveStateObserver failed, ret=%{public}d", ret);
        }
        g_countryInfoObserver = nullptr;
        g_countryInfoObserverRegistered = false;
    }
#endif
}

// 复位国家码缓存：清空缓存、清失败时间戳并递增代际。
// 例：Stop 后缓存由 "CN_460_00" 变为空串；在飞取数提交时 gen_ 已变化，结果被丢弃
void BtChrLocalData::ResetCountryInfo()
{
    std::lock_guard<std::mutex> lock(countryInfoMutex_);
    countryInfo_ = "";
    isLoaded_ = false;
    lastFetchFailTime_ = FAIL_TIME_NONE;
    lastRefreshFailTime_ = FAIL_TIME_NONE;
    ++gen_; // 代际递增：在飞取数的提交被拒绝
}

// 实时取数：主卡槽 → SIM 的 ISO 国家码 → 当前驻网 PLMN 数字 → 拼接 "ISO_MCC_MNC"。
// 取数示例（中国移动卡插在卡槽 1、驻网 PLMN 为 46000）：
//   GetPrimarySlotId                    → slotId = 1
//   GetISOCountryCodeForSim(1)           → "CN"
//   GetNetworkState(1)->GetPlmnNumeric() → "46000"
//   MCC = "460"、MNC = "00"             → 结果 "CN_460_00"
// 失败示例：无 SIM 卡（slotId < 0）、SIM 未就绪（ISO 为空）、未驻网（PLMN 不足 4 位）均返回 false，
// 由上层回退缓存并在 30s 内抑制重试
bool BtChrLocalData::FetchCountryInfo(std::string &countryInfo)
{
#ifdef BLUETOOTH_TELEPHONY_EXIST
    int32_t slotId = -1;
    int32_t ret = Telephony::CoreServiceClient::GetInstance().GetPrimarySlotId(slotId);
    if (ret != Telephony::TELEPHONY_SUCCESS || slotId < 0) {
        // 无 SIM 卡或 Telephony 未就绪时 slotId 保持 -1
        HILOGE("GetPrimarySlotId failed, ret=%{public}d, slotId=%{public}d", ret, slotId);
        return false;
    }

    // 返回 SIM 归属地的 ISO 3166-1 两位国家码，如 "CN"；漫游时仍是归属地国家
    std::u16string countryCode16;
    ret = Telephony::CoreServiceClient::GetInstance().GetISOCountryCodeForSim(slotId, countryCode16);
    if (ret != Telephony::TELEPHONY_SUCCESS || countryCode16.empty()) {
        HILOGE("GetISOCountryCodeForSim failed, ret=%{public}d", ret);
        return false;
    }

    // 当前驻网信息：PLMN 数字表示实际接入的网络，漫游时其国家可能与 SIM 归属地不同
    sptr<Telephony::NetworkState> networkState = nullptr;
    ret = Telephony::CoreServiceClient::GetInstance().GetNetworkState(slotId, networkState);
    if (ret != Telephony::TELEPHONY_SUCCESS || networkState == nullptr) {
        HILOGE("GetNetworkState failed, ret=%{public}d", ret);
        return false;
    }

    std::string plmnNumeric = networkState->GetPlmnNumeric();
    // 例："46000"（MCC 460 / MNC 00）、"310260"（MCC 310 / MNC 260）合法；
    // 不足 4 位（只有 MCC 或为空）视为无效
    if (plmnNumeric.length() <= MCC_LEN) {
        HILOGE("plmnNumeric invalid, len=%{public}zu", plmnNumeric.length());
        return false;
    }

    countryInfo = Str16ToStr8(countryCode16) + COUNTRY_INFO_SEPARATOR + plmnNumeric.substr(0, MCC_LEN) +
        COUNTRY_INFO_SEPARATOR + plmnNumeric.substr(MCC_LEN);
    return true;
#else
    (void)countryInfo;
    return false;
#endif
}

int64_t BtChrLocalData::GetCurrentTimeMs()
{
    // 单调时钟：系统时间回拨/前跳不影响冷却与节流计时
    // 例：用户把系统时间从 10:00 改到 08:00，30s 冷却不会被延长或绕过
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

}  // namespace bluetooth
}  // namespace OHOS
