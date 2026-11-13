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

#include "car_policy_dispatcher.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_dispatcher"
#endif
#include "log.h"

#include "thread_util.h"

#include "a2dp_snk_service.h"
#include "bluetooth_datashare_utils.h"
#include "bluetooth_def.h"
#include "car_policy_manager.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_util.h"
#include "hfp_hf_service.h"
#include "interface_profile_a2dp_snk.h"
#include "interface_profile_hfp_hf.h"
#include "pbap_pce_service.h"
#include "raw_address.h"
#include "uri.h"

#include "adapter_manager.h"
#include "interface_adapter_classic.h"

#include "os_account_subprofile_client.h"
#include "os_account_sub_profile_subscribe_callback.h"

#include <chrono>
#include <future>
#include <set>
#include <string>

namespace OHOS {
namespace bluetooth {

namespace {
const char *const SETTINGSDATA_SCT_KEY = "SMART_CALL_TRANSFER_STATUS";
const char *const SETTINGSDATA_BT_SETTINGS_UI_KEY = "IS_BT_SETTINGS_ACTIVITY";
const char *const SETTINGSDATA_FA_KEY = "FA_MEDIA_TYPE";
const char *const SETTINGSDATA_BT_MUSIC_KEY = "BT_MUSIC_STATUS";
constexpr int ACL_STATE_DISCONNECTED = 0x00;
constexpr int SCT_STATUS_INVALID = -1;

/// 字符串转 int：失败时回退到 fallback。ConvertStrToDigit 失败时不修改 ret，故显式回退。
template <typename T>
T SafeParseInt(const std::string &value, T fallback)
{
    if (value.empty()) {
        return fallback;
    }
    T result = fallback;
    if (!ConvertStrToDigit(value, result)) {
        return fallback;
    }
    return result;
}
}  // namespace

// ── Profile Observer 实现类 ──

class CarPolicyDispatcher::HfpHfObserverImpl : public HfpHfServiceObserver {
public:
    explicit HfpHfObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~HfpHfObserverImpl() override = default;
    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {
        std::string addr = device.GetAddress();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, state]() {
            mgr->OnProfileStateChanged(addr, ProfileType::HFP_HF, state, -1);
        });
    }
    void OnCallChanged(const RawAddress &device, const HandsFreeUnitCalls &call) override
    {
        std::string addr = device.GetAddress();
        int callState = call.GetState();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, callState]() {
            mgr->OnCallChanged(addr, callState);
        });
    }
    void OnScoStateChanged(const RawAddress &device, int state) override
    {
        std::string addr = device.GetAddress();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, state]() {
            mgr->OnScoSwitchChanged(addr, state);
        });
    }

private:
    CarPolicyManager &owner_;
};

class CarPolicyDispatcher::A2dpSnkObserverImpl : public IA2dpSnkObserver {
public:
    explicit A2dpSnkObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~A2dpSnkObserverImpl() override = default;
    void OnConnectionStateChanged(const RawAddress &remoteAddr, int state, int cause) override
    {
        std::string addr = remoteAddr.GetAddress();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, state]() {
            mgr->OnProfileStateChanged(addr, ProfileType::A2DP_SINK, state, -1);
        });
    }

private:
    CarPolicyManager &owner_;
};

// ── PBAP PCE 观察者：监听 PBAP 连接状态变化，转发到 car policy 线程 ──

class CarPolicyDispatcher::PbapPceObserverImpl : public IPbapPceObserver {
public:
    explicit PbapPceObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~PbapPceObserverImpl() override = default;
    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {
        std::string addr = device.GetAddress();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, state]() {
            mgr->OnProfileStateChanged(addr, ProfileType::PBAP_PCE, state, -1);
        });
    }
    void OnSyncStateChange(const RawAddress &device, int syncState) override
    {
        std::string addr = device.GetAddress();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, syncState]() {
            mgr->OnPbapActionCompleted(addr, syncState);
        });
    }

private:
    CarPolicyManager &owner_;
};

// ── Classic 远端设备观察者 ──

class CarPolicyDispatcher::ClassicRemoteDeviceObserverImpl : public IClassicRemoteDeviceObserver {
public:
    explicit ClassicRemoteDeviceObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~ClassicRemoteDeviceObserverImpl() override = default;
    void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) override
    {
        if (state != ACL_STATE_DISCONNECTED) {
            return;
        }
        std::string addr = device.GetAddress();
        CarPolicyManager *mgr = &owner_;
        int rawReason = static_cast<int>(reason);
        DoInCarPolicyThread([mgr, addr, rawReason]() {
            mgr->OnAclDisconnect(addr, rawReason);
        });
    }
    void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, int status, int cause,
                              const std::string &causeMessage) override
    {
        std::string addr = device.GetAddress();
        int newState = status;
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, newState]() {
            mgr->OnBondStateChanged(addr, newState);
        });
    }
    void OnRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) override {}
    void OnRemoteNameChanged(const RawAddress &device, const std::string &deviceName) override {}
    void OnRemoteAliasChanged(const RawAddress &device, const std::string &alias) override {}
    void OnRemoteCodChanged(const RawAddress &device, int cod) override {}
    void OnRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo) override {}
    void OnRemoteDeviceCommonInfoReport(const RawAddress &device, const std::vector<uint8_t> &value) override {}
private:
    CarPolicyManager &owner_;
};

// ── Classic 适配器观察者：监听扫描状态/结果，转发到 car policy 线程（BT_SCAN_DELAY 打点事件源）──

class CarPolicyDispatcher::ClassicAdapterObserverImpl : public IAdapterClassicObserver {
public:
    explicit ClassicAdapterObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~ClassicAdapterObserverImpl() override = default;
    void OnDiscoveryStateChanged(int status) override
    {
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, status]() {
            mgr->OnDiscoveryStateChanged(status);
        });
    }
    void OnDiscoveryResult(const RawAddress &device, int rssi, const std::string deviceName,
        int deviceClass, int transport) override
    {
        // 打点仅需"发现设备"事实（首个设备触发上报），设备详情参数不透传
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr]() {
            mgr->OnScanDeviceFound();
        });
    }
    void OnPairRequested(const BTTransport transport, const RawAddress &device) override {}
    void OnPairConfirmed(const BTTransport transport, const RawAddress &device, int reqType, int number) override {}
    void OnScanModeChanged(int mode) override {}
    void OnDeviceNameChanged(const std::string &deviceName) override {}
    void OnDeviceAddrChanged(const std::string &address) override {}
    void OnGenerateLocalOobData(int32_t ret, const Bluetooth::BluetoothOobData &oobData) override {}

private:
    CarPolicyManager &owner_;
};

// ── AdapterManager 状态观察者：BR TURN_ON 时触发车机自动回连（BT_ON）。
// 经 IAdapterManager 公开 RegisterStateObserver 接入，与 bt_func_hook 公共 hook 调用点
// （adapter_manager.cpp OnAdapterStateChange 中 NotifyAdapterStateChange）同时序，
// 不在公共函数/hook 函数体中添加车机逻辑。──

class CarPolicyDispatcher::AdapterStateObserverImpl : public IAdapterStateObserver {
public:
    explicit AdapterStateObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~AdapterStateObserverImpl() override = default;
    void OnStateChange(const BTTransport transport, const BTStateID state) override
    {
        if (transport != ADAPTER_BREDR || state != STATE_TURN_ON) {
            return;
        }
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr]() {
            mgr->OnBluetoothOn();
        });
    }
    void OnBluetoothStateChanged(int state) override
    {
        (void)state;
    }

private:
    CarPolicyManager &owner_;
};

// ── 账号子空间订阅者：监听 SubProfile SWITCHED，投递到 car policy 线程 ──

class CarPolicyDispatcher::OsAccountSubProfileSubscriberImpl : public AccountSA::OsAccountSubProfileSubscribeCallback {
public:
    explicit OsAccountSubProfileSubscriberImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~OsAccountSubProfileSubscriberImpl() override = default;
    void OnSubProfileChanged(const AccountSA::SubProfileEventData &eventData) override
    {
        // 回调位于账号服务 Binder 线程，仅处理 SWITCHED（账号切换完成，早于用户中心
        // settingsdata 同步完成广播）；切到 car policy 线程由 Manager 启动切号看门狗。
        // 会话门控与 Dispatch 一致，防 Uninit 竞态
        if (eventData.type_ != AccountSA::OsAccountSubProfileEventType::SWITCHED) {
            return;
        }
        CarPolicyManager *mgr = &owner_;
        uint64_t sessionId = owner_.GetSessionId();
        DoInCarPolicyThread([mgr, sessionId]() {
            if (!mgr->IsInitialized() || mgr->GetSessionId() != sessionId) {
                return;
            }
            mgr->OnSubProfileSwitched();
        });
    }

private:
    CarPolicyManager &owner_;
};

// IVI 电源状态公共事件订阅者（CES，融合短距发布）：
// 仅做事件解析层（action 校验 + 取参），切 car policy 线程透传 Manager，
// 业务分发在 CarPolicyManager::OnIviPowerStateChanged。
class CarPolicyDispatcher::IviPowerEventSubscriberImpl : public OHOS::EventFwk::CommonEventSubscriber {
public:
    IviPowerEventSubscriberImpl(const OHOS::EventFwk::CommonEventSubscribeInfo &info,
        CarPolicyManager &owner)
        : OHOS::EventFwk::CommonEventSubscriber(info), owner_(owner) {}
    ~IviPowerEventSubscriberImpl() override = default;

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override
    {
        auto want = data.GetWant();
        std::string action = want.GetAction();
        if (action != IVI_POWER_STATE_CHANGED_ACTION) {
            // MatchingSkills 已限定只收本 action，此处为防御性兜底（防伪造/错投）
            HILOGW("OnReceiveEvent: unexpected action %{public}s, drop", action.c_str());
            return;
        }
        int32_t state = want.GetIntParam(IVI_POWER_STATE_KEY, IVI_POWER_STATE_INVALID);
        if (state == IVI_POWER_STATE_INVALID) {
            HILOGW("OnReceiveEvent: missing or invalid param, drop");
            return;
        }
        HILOGI("OnReceiveEvent: ivi power state=%{public}d", state);
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, state]() { mgr->OnIviPowerStateChanged(state); });
    }

private:
    CarPolicyManager &owner_;
};

// 用户中心 user_config_update_finished 广播订阅者（CES，切号数据就绪主感知源）：
// 仅做事件接收层（action 已由 MatchingSkills 限定），切 car policy 线程透传 Manager
// （会话门控防 Uninit 竞态），业务处理在 SubProfileSwitchHandler。
class CarPolicyDispatcher::UserConfigUpdateSubscriberImpl : public OHOS::EventFwk::CommonEventSubscriber {
public:
    UserConfigUpdateSubscriberImpl(const OHOS::EventFwk::CommonEventSubscribeInfo &info,
        CarPolicyManager &owner)
        : OHOS::EventFwk::CommonEventSubscriber(info), owner_(owner) {}
    ~UserConfigUpdateSubscriberImpl() override = default;

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &) override
    {
        CarPolicyManager *mgr = &owner_;
        uint64_t sessionId = owner_.GetSessionId();
        DoInCarPolicyThread([mgr, sessionId]() {
            if (!mgr->IsInitialized() || mgr->GetSessionId() != sessionId) {
                return;
            }
            mgr->OnUserConfigUpdateFinished();
        });
    }

private:
    CarPolicyManager &owner_;
};

// ── CarPolicyDispatcher 实现 ──

CarPolicyDispatcher::CarPolicyDispatcher(CarPolicyManager &owner) : owner_(owner) {}

CarPolicyDispatcher::~CarPolicyDispatcher()
{
    UnregisterAll();
}

void CarPolicyDispatcher::Dispatch(std::function<void()> task)
{
    // 会话门控：任务执行时若已 Uninit 或已重建会话则直接丢弃。
    // CarPolicyManager 为进程级单例，mgr 指针恒有效；sessionId 相同意味着
    // 组件为本会话创建且未销毁（Uninit 第一步即递增 sessionId）。
    CarPolicyManager *mgr = &owner_;
    uint64_t sessionId = owner_.GetSessionId();
    DoInCarPolicyThread([mgr, sessionId, task = std::move(task)]() {
        if (!mgr->IsInitialized() || mgr->GetSessionId() != sessionId) {
            return;
        }
        task();
    });
}

void CarPolicyDispatcher::DispatchDelayed(std::function<void()> task, uint64_t delayMs, const std::string &name)
{
    CarPolicyManager *mgr = &owner_;
    uint64_t sessionId = owner_.GetSessionId();
    ThreadUtilFunc gatedTask = [mgr, sessionId, task = std::move(task)]() {
        if (!mgr->IsInitialized() || mgr->GetSessionId() != sessionId) {
            return;
        }
        task();
    };
    // 命名投递：ThreadUtil 延时队列按 name 去重（同名新任务取消旧任务），
    // 空 name 延时任务会与队列上所有无名延时任务互相取消，故延时任务必须携带 name
    ThreadUtil::GetInstance().PostTask(THREAD_ID_CAR_POLICY, gatedTask, delayMs, name);
}

void CarPolicyDispatcher::CancelDelayedTask(const std::string &name)
{
    if (name.empty()) {
        return;
    }
    ThreadUtil::GetInstance().RemoveTask(THREAD_ID_CAR_POLICY, name);
}

bool CarPolicyDispatcher::WaitQueueIdle(uint32_t timeoutMs)
{
    // 栅栏任务不经会话门控（Uninit 调用时 initialized_ 已置 false），必须无条件执行
    auto barrier = std::make_shared<std::promise<void>>();
    auto future = barrier->get_future();
    DoInCarPolicyThread([barrier]() { barrier->set_value(); });
    bool drained = future.wait_for(std::chrono::milliseconds(timeoutMs)) == std::future_status::ready;
    if (!drained) {
        HILOGW("WaitQueueIdle: car policy queue not drained within %{public}u ms", timeoutMs);
    }
    return drained;
}

void CarPolicyDispatcher::RegisterAll()
{
    RegisterProfileObservers();
    RegisterClassicRemoteDeviceObserver();
    RegisterClassicAdapterObserver();
    RegisterAdapterManagerStateObserver();
    RegisterFaBtMusicObservers();
    RegisterSmartCallTransferObserver();
    RegisterBtSettingsUiObserver();
    RegisterOsAccountSubProfileSubscriber();
    RegisterIviPowerEventSubscriber();
    RegisterUserConfigUpdateSubscriber();
}

void CarPolicyDispatcher::UnregisterAll()
{
    DeregisterProfileObservers();
    DeregisterClassicRemoteDeviceObserver();
    DeregisterClassicAdapterObserver();
    DeregisterAdapterManagerStateObserver();
    UnregisterFaBtMusicObservers();
    UnregisterSmartCallTransferObserver();
    UnregisterBtSettingsUiObserver();
    UnregisterOsAccountSubProfileSubscriber();
    UnregisterIviPowerEventSubscriber();
    UnregisterUserConfigUpdateSubscriber();
}

void CarPolicyDispatcher::RegisterProfileObservers()
{
    auto *hfpSvc = HfpHfService::GetService();
    if (hfpSvc != nullptr) {
        hfpObserver_ = std::make_unique<HfpHfObserverImpl>(owner_);
        hfpSvc->RegisterObserver(*hfpObserver_);
        HILOGI("HFP-HF observer registered");
    } else {
        HILOGE("HfpHfService null, observer not registered");
    }
    auto *a2dpSvc = A2dpSnkService::GetService();
    if (a2dpSvc != nullptr) {
        a2dpObserver_ = std::make_unique<A2dpSnkObserverImpl>(owner_);
        a2dpSvc->RegisterObserver(a2dpObserver_.get());
        HILOGI("A2DP-Sink observer registered");
    } else {
        HILOGE("A2dpSnkService null, observer not registered");
    }
    auto *pbapSvc = PbapPceService::GetService();
    if (pbapSvc != nullptr) {
        pbapObserver_ = std::make_unique<PbapPceObserverImpl>(owner_);
        pbapSvc->RegisterObserver(*pbapObserver_);
        HILOGI("PBAP-PCE observer registered");
    } else {
        HILOGE("PbapPceService null, observer not registered");
    }
}

void CarPolicyDispatcher::DeregisterProfileObservers()
{
    if (hfpObserver_ != nullptr) {
        auto *hfpSvc = HfpHfService::GetService();
        if (hfpSvc != nullptr) {
            hfpSvc->DeregisterObserver(*hfpObserver_);
        }
        hfpObserver_.reset();
    }
    if (a2dpObserver_ != nullptr) {
        auto *a2dpSvc = A2dpSnkService::GetService();
        if (a2dpSvc != nullptr) {
            a2dpSvc->DeregisterObserver(a2dpObserver_.get());
        }
        a2dpObserver_.reset();
    }
    if (pbapObserver_ != nullptr) {
        auto *pbapSvc = PbapPceService::GetService();
        if (pbapSvc != nullptr) {
            pbapSvc->DeregisterObserver(*pbapObserver_);
        }
        pbapObserver_.reset();
    }
}

bool CarPolicyDispatcher::RegisterSettingsObserver(const std::string &key, const sptr<BluetoothObserver> &observer)
{
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    std::string uriStr = BluetoothBuildSettingsUri(key);
    if (uriStr.empty()) {
        HILOGE("RegisterSettingsObserver: invalid userId, skip key=%{public}s", key.c_str());
        return false;
    }
    Uri uri(uriStr);
    if (!dataShareHelperUtils->RegisterObserver(uri, observer)) {
        HILOGE("RegisterSettingsObserver: RegisterObserver failed, key=%{public}s", key.c_str());
        return false;
    }
    HILOGI("RegisterSettingsObserver: key=%{public}s registered", key.c_str());
    return true;
}

void CarPolicyDispatcher::UnregisterSettingsObserver(const std::string &key, const sptr<BluetoothObserver> &observer)
{
    if (observer == nullptr) {
        return;
    }
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    std::string uriStr = BluetoothBuildSettingsUri(key);
    if (uriStr.empty()) {
        HILOGE("UnregisterSettingsObserver: invalid userId, skip key=%{public}s", key.c_str());
        return;
    }
    Uri uri(uriStr);
    dataShareHelperUtils->UnregisterObserver(uri, observer);
    HILOGI("UnregisterSettingsObserver: key=%{public}s unregistered", key.c_str());
}

void CarPolicyDispatcher::RegisterFaBtMusicObservers()
{
    if (faObserver_ == nullptr) {
        CarPolicyManager *mgr = &owner_;
        // 先注册后接管：注册失败不保存 observer（成员保持 null），下次 RegisterAll 可重试；
        // 注册成功时 sptr 引用计数为 1（仅 faObserver_ 持有），Unregister 时 nullptr 赋值释放
        sptr<BluetoothObserver> observer = new BluetoothObserver([mgr]() {
            std::string value;
            if (!BluetoothDataShareQueryForUser(SETTINGSDATA_FA_KEY, value)) {
                HILOGW("FA observer: DataShare query failed");
                return;
            }
            int newFaStatus = SafeParseInt<int>(value, 0);
            DoInCarPolicyThread([mgr, newFaStatus]() { mgr->OnFaStatusChanged(newFaStatus); });
        });
        if (RegisterSettingsObserver(SETTINGSDATA_FA_KEY, observer)) {
            faObserver_ = observer;
        }
    }
    if (btMusicObserver_ == nullptr) {
        CarPolicyManager *mgr = &owner_;
        sptr<BluetoothObserver> observer = new BluetoothObserver([mgr]() {
            std::string value;
            if (!BluetoothDataShareQueryForUser(SETTINGSDATA_BT_MUSIC_KEY, value)) {
                HILOGW("BtMusic observer: DataShare query failed");
                return;
            }
            int newStatus = SafeParseInt<int>(value, 0);
            DoInCarPolicyThread([mgr, newStatus]() { mgr->OnBtMusicStatusChanged(newStatus); });
        });
        if (RegisterSettingsObserver(SETTINGSDATA_BT_MUSIC_KEY, observer)) {
            btMusicObserver_ = observer;
        }
    }
}

void CarPolicyDispatcher::UnregisterFaBtMusicObservers()
{
    UnregisterSettingsObserver(SETTINGSDATA_FA_KEY, faObserver_);
    faObserver_ = nullptr;
    UnregisterSettingsObserver(SETTINGSDATA_BT_MUSIC_KEY, btMusicObserver_);
    btMusicObserver_ = nullptr;
}

void CarPolicyDispatcher::RegisterSmartCallTransferObserver()
{
    if (sctObserver_ != nullptr) {
        return;
    }
    CarPolicyManager *mgr = &owner_;
    // 先注册后接管：失败不保存，下次 RegisterAll 可重试
    sptr<BluetoothObserver> observer = new BluetoothObserver([mgr]() {
        std::string value;
        if (!BluetoothDataShareQueryForUser(SETTINGSDATA_SCT_KEY, value)) {
            HILOGW("SCT observer: DataShare query failed");
            return;
        }
        // SCT 状态失败时使用 -1（STATUS_UNKNOWN 语义），区别于合法 0
        int newStatus = SafeParseInt<int>(value, SCT_STATUS_INVALID);
        DoInCarPolicyThread([mgr, newStatus]() { mgr->OnSmartCallTransferStatusChanged(newStatus); });
    });
    if (RegisterSettingsObserver(SETTINGSDATA_SCT_KEY, observer)) {
        sctObserver_ = observer;
    }
}

void CarPolicyDispatcher::UnregisterSmartCallTransferObserver()
{
    UnregisterSettingsObserver(SETTINGSDATA_SCT_KEY, sctObserver_);
    sctObserver_ = nullptr;
}

void CarPolicyDispatcher::RegisterClassicRemoteDeviceObserver()
{
    if (classicObserver_ != nullptr) {
        return;
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGE("ClassicAdapter null, remote device observer not registered");
        return;
    }
    classicObserver_ = std::make_unique<ClassicRemoteDeviceObserverImpl>(owner_);
    if (!classicAdapter->RegisterRemoteDeviceObserver(*classicObserver_)) {
        HILOGE("RegisterRemoteDeviceObserver failed");
        classicObserver_.reset();
    } else {
        HILOGI("Classic remote device observer registered");
    }
}

void CarPolicyDispatcher::DeregisterClassicRemoteDeviceObserver()
{
    if (classicObserver_ != nullptr) {
        auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
        if (classicAdapter != nullptr) {
            classicAdapter->DeregisterRemoteDeviceObserver(*classicObserver_);
        }
        classicObserver_.reset();
    }
}

void CarPolicyDispatcher::RegisterClassicAdapterObserver()
{
    if (classicAdapterObserver_ != nullptr) {
        return;
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGE("ClassicAdapter null, adapter observer not registered");
        return;
    }
    classicAdapterObserver_ = std::make_unique<ClassicAdapterObserverImpl>(owner_);
    if (!classicAdapter->RegisterClassicAdapterObserver(*classicAdapterObserver_)) {
        HILOGE("RegisterClassicAdapterObserver failed");
        classicAdapterObserver_.reset();
    } else {
        HILOGI("Classic adapter observer registered");
    }
}

void CarPolicyDispatcher::DeregisterClassicAdapterObserver()
{
    if (classicAdapterObserver_ != nullptr) {
        auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
        if (classicAdapter != nullptr) {
            classicAdapter->DeregisterClassicAdapterObserver(*classicAdapterObserver_);
        }
        classicAdapterObserver_.reset();
    }
}

void CarPolicyDispatcher::RegisterAdapterManagerStateObserver()
{
    if (adapterStateObserver_ != nullptr) {
        return;
    }
    adapterStateObserver_ = std::make_unique<AdapterStateObserverImpl>(owner_);
    if (!AdapterManager::GetInstance()->RegisterStateObserver(*adapterStateObserver_)) {
        HILOGE("RegisterAdapterManagerStateObserver failed");
        adapterStateObserver_.reset();
    } else {
        HILOGI("Adapter manager state observer registered");
    }
}

void CarPolicyDispatcher::DeregisterAdapterManagerStateObserver()
{
    if (adapterStateObserver_ != nullptr) {
        AdapterManager::GetInstance()->DeregisterStateObserver(*adapterStateObserver_);
        adapterStateObserver_.reset();
    }
}

void CarPolicyDispatcher::RegisterBtSettingsUiObserver()
{
    if (btSettingsUiObserver_ != nullptr) {
        return;
    }
    CarPolicyManager *mgr = &owner_;
    // 先注册后接管：失败不保存，下次 RegisterAll 可重试
    sptr<BluetoothObserver> observer = new BluetoothObserver([mgr]() {
        std::string value;
        if (!BluetoothDataShareQueryForUser(SETTINGSDATA_BT_SETTINGS_UI_KEY, value)) {
            HILOGW("BT Settings UI observer: DataShare query failed");
            return;
        }
        int newStatus = SafeParseInt<int>(value, 0);
        DoInCarPolicyThread([mgr, newStatus]() { mgr->OnBtSettingsUiChanged(newStatus); });
    });
    if (RegisterSettingsObserver(SETTINGSDATA_BT_SETTINGS_UI_KEY, observer)) {
        btSettingsUiObserver_ = observer;
    }
}

void CarPolicyDispatcher::RegisterIviPowerEventSubscriber()
{
    if (iviPowerSubscriber_ != nullptr) {
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(IVI_POWER_STATE_CHANGED_ACTION);
    OHOS::EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    auto subscriber = std::make_shared<IviPowerEventSubscriberImpl>(info, owner_);
    bool ret = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    if (!ret) {
        // 先注册后接管：失败不落成员，下次 RegisterAll 自动重试
        HILOGE("SubscribeCommonEvent failed, subscriber not kept");
        return;
    }
    iviPowerSubscriber_ = std::move(subscriber);
    HILOGI("IVI power state event subscribed");
}

void CarPolicyDispatcher::UnregisterIviPowerEventSubscriber()
{
    if (iviPowerSubscriber_ == nullptr) {
        return;
    }
    OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(iviPowerSubscriber_);
    iviPowerSubscriber_.reset();
    HILOGI("IVI power state event unsubscribed");
}

void CarPolicyDispatcher::RegisterUserConfigUpdateSubscriber()
{
    if (userConfigUpdateSubscriber_ != nullptr) {
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(USER_CONFIG_UPDATE_FINISHED_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    auto subscriber = std::make_shared<UserConfigUpdateSubscriberImpl>(info, owner_);
    bool ret = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    if (!ret) {
        // 先注册后接管：失败不落成员，下次 RegisterAll 自动重试
        HILOGE("Subscribe %{public}s failed, subscriber not kept", USER_CONFIG_UPDATE_FINISHED_EVENT);
        return;
    }
    userConfigUpdateSubscriber_ = std::move(subscriber);
    HILOGI("Subscribe %{public}s ok", USER_CONFIG_UPDATE_FINISHED_EVENT);
}

void CarPolicyDispatcher::UnregisterUserConfigUpdateSubscriber()
{
    if (userConfigUpdateSubscriber_ == nullptr) {
        return;
    }
    OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(userConfigUpdateSubscriber_);
    userConfigUpdateSubscriber_.reset();
    HILOGI("Unsubscribe %{public}s", USER_CONFIG_UPDATE_FINISHED_EVENT);
}

void CarPolicyDispatcher::UnregisterBtSettingsUiObserver()
{
    UnregisterSettingsObserver(SETTINGSDATA_BT_SETTINGS_UI_KEY, btSettingsUiObserver_);
    btSettingsUiObserver_ = nullptr;
}

void CarPolicyDispatcher::RegisterOsAccountSubProfileSubscriber()
{
    if (subProfileSubscriber_ == nullptr) {
        subProfileSubscriber_ = std::make_shared<OsAccountSubProfileSubscriberImpl>(owner_);
    }
    std::set<AccountSA::OsAccountSubProfileEventType> types = {AccountSA::OsAccountSubProfileEventType::SWITCHED};
    auto ret = AccountSA::OsAccountSubProfileClient::GetInstance().SubscribeOsAccountSubProfileEvents(
        types, subProfileSubscriber_);
    if (ret != ERR_OK) {
        HILOGE("SubscribeOsAccountSubProfileEvents failed, ret=%{public}d", ret);
        subProfileSubscriber_.reset();
        return;
    }
    HILOGI("OsAccountSubProfile SWITCHED subscriber registered");
}

void CarPolicyDispatcher::UnregisterOsAccountSubProfileSubscriber()
{
    if (subProfileSubscriber_ == nullptr) {
        return;
    }
    auto ret = AccountSA::OsAccountSubProfileClient::GetInstance().UnsubscribeOsAccountSubProfileEvents(
        subProfileSubscriber_);
    if (ret != ERR_OK) {
        HILOGE("UnsubscribeOsAccountSubProfileEvents failed, ret=%{public}d", ret);
    }
    subProfileSubscriber_.reset();
}

void CarPolicyDispatcher::LoadFaBtMusicStatus(int &outFaStatus, int &outBtMusicStatus)
{
    constexpr int STATUS_UNKNOWN = -1;
    outFaStatus = STATUS_UNKNOWN;
    outBtMusicStatus = STATUS_UNKNOWN;
    std::string faValue;
    std::string btMusicValue;
    bool faOk = BluetoothDataShareQueryForUser(SETTINGSDATA_FA_KEY, faValue);
    bool btMusicOk = BluetoothDataShareQueryForUser(SETTINGSDATA_BT_MUSIC_KEY, btMusicValue);
    if (faOk && !faValue.empty()) {
        outFaStatus = SafeParseInt<int>(faValue, STATUS_UNKNOWN);
    } else {
        HILOGW("LoadFaBtMusicStatus: FA query failed or empty, fallback UNKNOWN");
    }
    if (btMusicOk && !btMusicValue.empty()) {
        outBtMusicStatus = SafeParseInt<int>(btMusicValue, STATUS_UNKNOWN);
    } else {
        HILOGW("LoadFaBtMusicStatus: BtMusic query failed or empty, fallback UNKNOWN");
    }
    HILOGI("LoadFaBtMusicStatus: faStatus=%{public}d, btMusicStatus=%{public}d", outFaStatus, outBtMusicStatus);
}

}  // namespace bluetooth
}  // namespace OHOS
