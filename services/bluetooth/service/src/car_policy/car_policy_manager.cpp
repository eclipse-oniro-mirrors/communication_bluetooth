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

#include "car_policy_manager.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_manager"
#endif
#include "log.h"

#include "dispatcher/car_policy_dispatcher.h"
#include "metrics/car_metrics_reporter.h"
#include "service/car_profile_service.h"

#include "adapter_manager.h"
#include "bluetooth_def.h"
#include "bluetooth_os_account.h"
#include "classic_adapter.h"
#include "common_util.h"
#include "hands_free_unit_calls.h"
#include "raw_address.h"

#include "reconnect/reconnect_predicates.h"
#include <parameters.h>

#include "common_event_manager.h"
#include "want.h"

#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

// 前向声明：bt_chr_util.h 与 common_util.h 存在 GET_ENCRYPT_ADDR 宏重定义冲突，
// 故不引入 bt_chr_util.h，毫秒时间戳符号由 btchr 模块链接提供
std::string GetCurrentTimeMs();

namespace {
// echo 8,9 NOTIFY_OUTPUT_PATH 的 a2dpOperation 取值（payload[2]）
constexpr uint8_t HIECHO_NO_USE_A2DP = 1;  // 不使用 A2DP → FA 卡片不在蓝牙音乐
constexpr uint8_t HIECHO_USE_A2DP = 2;     // 使用 A2DP → FA 卡片在蓝牙音乐

// SWITCH_BLUETOOTH_CARD 公共事件参数
constexpr const char *SWITCH_BLUETOOTH_CARD_ACTION = "usual.event.bluetooth.hiecho.switch_bluetooth_card";
constexpr const char *SWITCH_BLUETOOTH_CARD_EXTRA_KEY = "switch_bluetooth_card";
// extra value: 1=FA 卡片在蓝牙音乐, 0=FA 卡片不在蓝牙音乐
constexpr int SWITCH_BLUETOOTH_CARD_VALUE_ON = 1;
constexpr int SWITCH_BLUETOOTH_CARD_VALUE_OFF = 0;

// BT 设置界面状态：1=活跃（UI 显示），其他=非活跃
constexpr int BT_SETTING_STATUS_ON = 1;

// ── 打点常量（对齐 HwCarPolicy）──
constexpr int RECORD_PROFILE_HFP = 0;            // PROFILE_TYPE_HEADSET_CLIENT
constexpr int RECORD_PROFILE_A2DP_SINK = 1;      // PROFILE_TYPE_A2DP_SINK
constexpr int RECONNECT_OTHER = 0;
constexpr int RECONNECT_CAR_POLICY = 1;
// PbapPcePhoneBookStateType（pbap_pce_app_params.h:27）
constexpr int PBAP_SYNC_STATE_DOWNLOADING = 4;
constexpr int PBAP_SYNC_STATE_DOWNLOADED = 5;
constexpr int PBAP_SYNC_STATE_DOWNLOAD_ERROR = 6;

// Uninit 排空 car policy 线程队列的栅栏等待上限，超时告警并继续（对齐旧行为）
constexpr uint32_t UNINIT_QUEUE_DRAIN_TIMEOUT_MS = 2000;
}  // namespace

// ── CarPolicyManager 实现 ──

CarPolicyManager &CarPolicyManager::GetInstance()
{
    static CarPolicyManager instance;
    return instance;
}

CarPolicyManager::CarPolicyManager() = default;

CarPolicyManager::~CarPolicyManager()
{
    // Uninit() 已由 AdapterManager::UnLoadBluetoothSystemAbility（adapter_manager.cpp:1407）在 BT 关闭时显式调用；
    // 此处为防御性兜底，以防该路径漏调。initialized_ 守卫保证重复 Uninit 幂等。
    if (initialized_) {
        HILOGW("~CarPolicyManager: not Uninit, force Uninit");
        Uninit();
    }
}

void CarPolicyManager::Init()
{
    if (initialized_) {
        HILOGW("Init called twice, ignore");
        return;
    }
    // 开启新会话：使上一会话（若有）遗留的队列延时任务失效
    sessionId_.fetch_add(1);

    // ── 构造 CarMetricsReporter（打点门面，供 Profile/PBAP/扫描等上报使用）──
    metricsReporter_ = std::make_unique<CarMetricsReporter>();

    // ── 构造 CarPolicyDispatcher（事件分发 + Observer 注册）──
    dispatcher_ = std::make_unique<CarPolicyDispatcher>(*this);

    // ── 构造 DevicePriorityManager（先于 ReconnectEngine，后者持有 DPM 引用）──
    dpm_ = std::make_unique<DevicePriorityManager>();
    
    // ── 构造 CarConnectionGuard（先于 ReconnectEngine，后者持有 Guard 引用）──
    connectionGuard_ = std::make_unique<CarConnectionGuard>();

    // ── 构造 ReconnectEngine（持有 Guard 引用，用于 HandleReconnect 中 Guard.Check + SinkLimit 抢占）──
    reconnectEngine_ = std::make_unique<ReconnectEngine>(
        config_, *dpm_, *dispatcher_, *connectionGuard_);

    // ── 构造 PairableAdvController（CarPolicyManager 自身持有，用于可配对广播控制）──
    advController_ = std::make_unique<PairableAdvController>();

    // ── 构造 ProfileCoordinator ──
    profileCoordinator_ = std::make_unique<ProfileCoordinator>(*connectionGuard_, *dpm_, *dispatcher_);

    // ── 构造 CarDeviceTypeResolver ──
    deviceTypeResolver_ = std::make_unique<CarDeviceTypeResolver>();

    // ── 构造 SmartCallTransferController ──
    smartCallTransferController_ = std::make_unique<SmartCallTransferController>(
        *deviceTypeResolver_, *dpm_, *reconnectEngine_);

    // ── 接入 6 个回连谓词 ──
    AssembleReconnectPredicates();

    // ── 切号处理器接入依赖组件（先于 RegisterAll，广播回调可能即时到达）──
    switchHandler_.Init(config_, *reconnectEngine_, *dpm_, *dispatcher_, *connectionGuard_);

    // ── 注册所有 Observer（Profile + FA/BtMusic 广播 + SCT DataShare + 账号子空间订阅
    //     + user_config_update_finished 广播订阅）──
    dispatcher_->RegisterAll();

    initialized_ = true;

    // ── 读取当前前台 subProfileId 建立比较基线 ──
    dispatcher_->Dispatch([this]() { switchHandler_.LoadCurrentSubProfileId(); });

    HILOGI("CarPolicyManager initialized");
}

void CarPolicyManager::Uninit()
{
    if (!initialized_) {
        return;
    }
    // 1) 先失效会话：此后旧会话队列任务经 Dispatcher 会话门控直接丢弃，不再访问任何成员
    initialized_ = false;
    sessionId_.fetch_add(1);
    if (dispatcher_) {
        // 2) 解除观察者/广播订阅（含 user_config_update_finished，阻断 CES 线程回调继续投递，
        //    消除与后续组件销毁的竞态窗口）
        dispatcher_->UnregisterAll();
        // 2.5) 取消在途的切号看门狗延时任务（会话门控亦可兜底，此处提前清理）
        switchHandler_.Uninit();
        // 3) 栚栏排空已入队的立即任务：串行队列保证栅栏完成即此前任务全部执行完毕，
        //    之后销毁组件不会与在途任务并发访问；未执行的延时任务由会话门控兜底
        if (!dispatcher_->WaitQueueIdle(UNINIT_QUEUE_DRAIN_TIMEOUT_MS)) {
            HILOGW("Uninit: car policy queue not drained in time, risk of racing in-flight tasks");
        }
    }
    // 4) 销毁顺序：dispatcher → coordinator → adv → sct → engine → metrics → dpm → resolver → guard
    // unique_ptr::reset() 自身对 nullptr 安全，无需冗余判空
    dispatcher_.reset();
    profileCoordinator_.reset();
    advController_.reset();
    smartCallTransferController_.reset();
    if (reconnectEngine_) {
        reconnectEngine_->Terminate();
    }
    reconnectEngine_.reset();
    metricsReporter_.reset();
    dpm_.reset();
    deviceTypeResolver_.reset();
    connectionGuard_.reset();
    prevHfpState_.clear();
    prevA2dpState_.clear();
    prevPbapState_.clear();
    pbapConnectingTime_.clear();
    pbapConnectedTime_.clear();
    pbapSyncStartTime_.clear();
    pbapSyncEndTime_.clear();
    profileConnectingTime_.clear();
    profileConnectedTime_.clear();
    reconnectFlag_.clear();
    prevBondState_.clear();
    prevBtSettingsUiStatus_ = -1;
    btSettingsUiActive_ = 0;
    callReportMap_.clear();
    scoSwitchList_.clear();
    callIncomingTimeStampMs_ = 0;
    scanReportStartTime_.clear();
    inStandby_ = false;
    HILOGI("CarPolicyManager uninitialized");
}

void CarPolicyManager::AssembleReconnectPredicates()
{
    if (!reconnectEngine_) {
        return;
    }
    // 构造不会失败（unique_ptr 构造 + make_unique），统一不判空，与 IncomingConnPredicate 风格一致
    // IncomingConnPredicate 无外部依赖，直接读单例
    reconnectEngine_->AddPredicate(std::make_unique<IncomingConnPredicate>());

    // BtSettingsUiPredicate：注入 CarPolicyManager 引用查询缓存的 UI 状态
    // OnTerminate 内部直接调 AdapterManager 单例取消扫描
    reconnectEngine_->AddPredicate(std::make_unique<BtSettingsUiPredicate>(*this));

    // SmartCallOutPredicate：注入 SmartCallTransferController 引用
    reconnectEngine_->AddPredicate(std::make_unique<SmartCallOutPredicate>(*smartCallTransferController_));

    // CarplayMutexPredicate：注入 CarConnectionGuard 引用
    reconnectEngine_->AddPredicate(std::make_unique<CarplayMutexPredicate>(*connectionGuard_));

    // IviStandbyPredicate：注入 CarPolicyManager 引用查询 IVI 待机状态
    reconnectEngine_->AddPredicate(std::make_unique<IviStandbyPredicate>(*this));

    HILOGI("Reconnect predicates assembled (Incoming+BtSettingsUi+SmartCallOut+CarplayMutex+IviStandby)");
}

void CarPolicyManager::HandlePolicyChanged()
{
    if (!connectionGuard_) {
        return;
    }
    if (!connectionGuard_->IsAllowConnA2dp()) {
        HILOGI("policy disallowed: disconnect all A2DP Sink");
        CarProfileService::NotifyPlaybackDisallowed();
        DisconnectAllA2dpSink();
        return;
    }
    HILOGI("policy allowed: try auto-connect A2DP by HFP state");
    AutoConnectA2dpByHfpState();
}

void CarPolicyManager::DisconnectAllA2dpSink()
{
    auto a2dpAddrs = connectionGuard_->GetA2dpConnectedAddrs();
    for (const auto &addr : a2dpAddrs) {
        HILOGI("disconnect A2DP Sink for policy change: %{public}s", GetEncryptAddr(addr).c_str());
        CarProfileService::Disconnect(ProfileType::A2DP_SINK, addr);
    }
}

void CarPolicyManager::AutoConnectA2dpByHfpState()
{
    auto hfpAddrs = connectionGuard_->GetHfpConnectedAddrs();
    for (const auto &addr : hfpAddrs) {
        if (CarProfileService::GetHfpDeviceState(addr) == static_cast<int>(BTConnectState::CONNECTED)) {
            HILOGI("auto-connect A2DP by HFP connected: %{public}s", GetEncryptAddr(addr).c_str());
            CarProfileService::Connect(ProfileType::A2DP_SINK, addr);
            return;
        }
    }
    for (const auto &addr : hfpAddrs) {
        if (CarProfileService::GetHfpDeviceState(addr) == static_cast<int>(BTConnectState::CONNECTING)) {
            HILOGI("auto-connect A2DP by HFP connecting: %{public}s", GetEncryptAddr(addr).c_str());
            CarProfileService::Connect(ProfileType::A2DP_SINK, addr);
            return;
        }
    }
    HILOGI("no HFP connected/connecting device, skip auto-connect");
}

// ── 外部入口（切线程到 car policy 线程）──

void CarPolicyManager::StartReconnect(ReconnectReason reason)
{
    if (!initialized_ || !reconnectEngine_ || !dpm_ || !dispatcher_) {
        HILOGW("StartReconnect before Init");
        return;
    }
    // 在 car policy 线程内读取首个设备，避免 DevicePriorityManager 跨线程访问。
    // 捕获单例 this 而非组件裸指针：任务经 Dispatcher 会话门控后成员必存活，
    // 组件销毁/重建后旧任务被门控丢弃，不会悬垂
    dispatcher_->Dispatch([this, reason]() {
        if (!initialized_ || !dpm_ || !reconnectEngine_) {
            return;
        }
        std::string firstDevice = dpm_->GetNextDevice("");
        if (firstDevice.empty()) {
            HILOGW("StartReconnect: no device in order list");
            return;
        }
        reconnectEngine_->StartReconnect(firstDevice, reason);
    });
}

void CarPolicyManager::OnBluetoothOn()
{
    HILOGI("OnBluetoothOn: trigger auto-reconnect (BT_ON)");
    // BT ON 时读取持久化的 FA/BtMusic 状态同步到 Guard。
    // DataShare 持久化存储确保蓝牙重启后仍能恢复上次状态，Guard 据此判断是否允许 A2DP 连接。
    // URI 常量由 CarPolicyDispatcher 统一持有，避免重复定义
    if (dispatcher_ && connectionGuard_) {
        int faStatus = CarConnectionGuard::STATUS_UNKNOWN;
        int btMusicStatus = CarConnectionGuard::STATUS_UNKNOWN;
        dispatcher_->LoadFaBtMusicStatus(faStatus, btMusicStatus);
        connectionGuard_->SetFaStatus(faStatus);
        connectionGuard_->SetBtMusicStatus(btMusicStatus);
    }
    // BT ON 时根据 BT 设置界面状态决定是否开可配对广播。
    HandleBtSettingUiStatusChange(false);
    if (dpm_) {
        dpm_->Init();   // ← 蓝牙已开、bonded 已加载后重建
    }
    StartReconnect(ReconnectReason::BT_ON_AUTO_CONN);
}

// ── IVI 电源状态处理（car policy 线程）──
// 唤醒回连对齐 BT_ON 逻辑：ACTIVE 直接拉起轮询回连（连上即停），无快照/求差/白名单

void CarPolicyManager::OnIviPowerStateChanged(int32_t state)
{
    // 兜底门控：Uninit 后组件已销毁，旧订阅在途事件直接丢弃
    if (!initialized_.load()) {
        HILOGW("OnIviPowerStateChanged: not initialized, drop state=%{public}d", state);
        return;
    }
    HILOGI("OnIviPowerStateChanged: state=%{public}d, inStandby=%{public}d",
        state, inStandby_);
    switch (state) {
        case IVI_POWER_STATE_STANDBY:
            HandleIviStandby();
            break;
        case IVI_POWER_STATE_ACTIVE:
            HandleIviActive();
            break;
        case IVI_POWER_STATE_STR:
        case IVI_POWER_STATE_SHUTDOWN:
        case IVI_POWER_STATE_REBOOT:
            HandleIviPowerExit();
            break;
        default:
            HILOGW("OnIviPowerStateChanged: unknown state=%{public}d, drop", state);
            break;
    }
}

void CarPolicyManager::HandleIviStandby()
{
    inStandby_ = true;
    // STANDBY 期间融合将断开所有用户可感知 profile，进行中的回连会与断连动作冲突
    //（回连成功随即被断开），终止。重复 STANDBY 重复 Terminate
    // 快速翻转兜底：ACTIVE 拉起回连后紧跟 STANDBY，轮询间隙由 IviStandbyPredicate 终止。
    if (reconnectEngine_ != nullptr) {
        reconnectEngine_->Terminate();
    }
    HILOGI("HandleIviStandby: standby entered, reconnect terminated");
}

void CarPolicyManager::HandleIviActive()
{
    inStandby_ = false;
    // 与 BT_ON 一致的回连逻辑：轮询 DPM 优先级列表逐台尝试，连上即停
    //（CheckIfAutoConnect 软跳过）。不重建 DPM：短暂待机蓝牙未重启，orderList 有效。
    // 重复 ACTIVE 会重开会话从头轮询，已有连接时软跳过耗尽自停，幂等无害。
    HILOGI("HandleIviActive: trigger reconnect (CAR_POWER_ACTIVE)");
    if (dpm_ != nullptr && dpm_->GetNextDevice("").empty()) {
        HILOGI("HandleIviActive: order list empty, rebuild from persisted records");
        dpm_->Init();
    }
    StartReconnect(ReconnectReason::CAR_POWER_ACTIVE);
}

void CarPolicyManager::HandleIviPowerExit()
{
    // STR 后蓝牙将被融合关闭（Uninit）；SHUTDOWN/REBOOT 整车下电/重启，
    // 均不回连。下次上车走融合恢复 → BT_ON → 既有 BT_ON_AUTO_CONN 路径
    inStandby_ = false;
    HILOGI("HandleIviPowerExit: state cleared");
}

void CarPolicyManager::HandleBtSettingUiStatusChange(bool isForce)
{
    int newStatus = btSettingsUiActive_;
    HILOGI("HandleBtSettingUiStatusChange: isForce=%{public}d, lastStatus=%{public}d, newStatus=%{public}d",
           isForce, prevBtSettingsUiStatus_, newStatus);
    if (newStatus < 0) {
        return;
    }
    if (prevBtSettingsUiStatus_ == newStatus && !isForce) {
        return;
    }
    prevBtSettingsUiStatus_ = newStatus;

    // 已有车机设备连接/连接中时不操作广播（无需配对新设备）。
    if (IsCarConnectedOrConnecting()) {
        HILOGI("Car has connected or connecting device, skip adv");
        return;
    }
    if (!advController_) {
        return;
    }
    if (newStatus == BT_SETTING_STATUS_ON) {
        advController_->StartAdv();
    } else {
        advController_->StopAdv();
    }
}

bool CarPolicyManager::IsCarConnectedOrConnecting() const
{
    if (!connectionGuard_) {
        return true;
    }
    auto a2dpAddrs = connectionGuard_->GetA2dpConnectedAddrs();
    auto hfpAddrs = connectionGuard_->GetHfpConnectedAddrs();
    return !a2dpAddrs.empty() || !hfpAddrs.empty();
}

void CarPolicyManager::TerminateReconnect()
{
    if (reconnectEngine_) {
        reconnectEngine_->Terminate();
    }
}

void CarPolicyManager::CancelPendingConnectOtherProfiles(const std::string &addr)
{
    HILOGI("CancelPendingConnectOtherProfiles start");
    if (!initialized_ || !profileCoordinator_ || !dispatcher_) {
        HILOGI("CancelPendingConnectOtherProfiles and initialized_ ,reconnectEngine_ and dispatcher_ maybe nullptr");
        return;
    }
    dispatcher_->Dispatch([this, addr]() {
        if (!initialized_ || !profileCoordinator_) {
            return;
        }
        profileCoordinator_->CancelPendingConnectOtherProfiles(addr);
    });
}

bool CarPolicyManager::IsAllowConnect(const std::string &addr, ProfileType profile)
{
    if (!initialized_ || !connectionGuard_) {
        HILOGI("IsAllowConnect not initialized, allow connect: %{public}s", GetEncryptAddr(addr).c_str());
        return true;
    }
    auto result = connectionGuard_->Check(addr, profile);
    if (!result.allowed) {
        if (result.reason == DenyReason::SINK_LIMIT) {
            HILOGI("[Car policy] preemption for %{public}s", GetEncryptAddr(addr).c_str());
            connectionGuard_->DisconnectOthersForSinkLimit(addr);
            TerminateReconnect();
        } else {
            HILOGW("[Car policy] guard deny connect: %{public}s, addr=%{public}s",
                DenyReasonName(result.reason), GetEncryptAddr(addr).c_str());
            return false;
        }
    }
    return true;
}

void CarPolicyManager::OnAclDisconnect(const std::string &addr, int rawReason)
{
    if (!initialized_ || !reconnectEngine_ || !dispatcher_) {
        HILOGI("OnAclDisconnect_emit and initialized_ ,reconnectEngine_ and dispatcher_ maybe nullptr");
        return;
    }
    // 捕获单例 this（同 StartReconnect），会话门控保证组件存活
    dispatcher_->Dispatch([this, addr, rawReason]() {
        if (!initialized_ || !reconnectEngine_) {
            return;
        }
        if (switchHandler_.IsSwitchDisconnecting(addr)) {
            // 切号处理（广播触发、数据就绪后）正在主动断开旧账号设备：其 ACL 断开
            // 不发起回连，避免断开动作本身触发回连把旧设备立刻连回新账号会话
            HILOGI("OnAclDisconnect: sub-profile switch disconnecting, skip reconnect: %{public}s",
                   GetEncryptAddr(addr).c_str());
            return;
        }
        reconnectEngine_->OnAclDisconnect(addr, rawReason);
    });
}

// ── Observer 回调（已在 car policy 线程，由 Dispatcher 调用）──

void CarPolicyManager::OnProfileStateChanged(const std::string &addr, ProfileType profile,
                                              int nextState, int prevState)
{
    if (!initialized_) {
        return;
    }
    // 仅处理 HFP_HF / A2DP_SINK / PBAP_PCE，避免其他 profile 误写状态缓存
    if (profile != ProfileType::HFP_HF && profile != ProfileType::A2DP_SINK && profile != ProfileType::PBAP_PCE) {
        HILOGI("OnProfileStateChanged: ignore unsupported profile=%{public}d", static_cast<int>(profile));
        return;
    }
    // 解析 prevState（调用方传 -1 时从缓存 map 查询）
    int resolvedPrev = prevState;
    std::unordered_map<std::string, int> &stateMap =
        (profile == ProfileType::HFP_HF) ? prevHfpState_ :
        (profile == ProfileType::A2DP_SINK) ? prevA2dpState_ : prevPbapState_;
    if (resolvedPrev == -1) {
        auto it = stateMap.find(addr);
        resolvedPrev = (it != stateMap.end()) ? it->second : static_cast<int>(BTConnectState::DISCONNECTED);
    }
    // 更新缓存：DISCONNECTED 清除，其他状态记录
    if (nextState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        stateMap.erase(addr);
    } else {
        stateMap[addr] = nextState;
    }

    HILOGI("OnProfileStateChanged: addr=%{public}s, profile=%{public}d, next=%{public}d, prev=%{public}d",
           GetEncryptAddr(addr).c_str(), static_cast<int>(profile), nextState, resolvedPrev);

    // HFP/A2DP/PBAP 任何状态变化都停可配对广播（已有设备连接/连接中，无需继续广播）。
    if (advController_) {
        advController_->StopAdv();
    }

    // ── HFP/A2DP 任一 CONNECTED → 终止回连引擎 ──
    // 任何设备占用 SinkLimit 名额后，回连其他设备无意义且会引发"断开→重连→又断开"
    // 循环冲突，故直接终止回连引擎。不区分是否为回连目标。并且若HFP_HF连接成功，立刻连接A2DP_SINK
    if (nextState == static_cast<int>(BTConnectState::CONNECTED)) {
        if (profile == ProfileType::HFP_HF && reconnectEngine_ != nullptr
        && reconnectEngine_->IsActive() && connectionGuard_->IsAllowConnA2dp()) {
            CarProfileService::Connect(ProfileType::A2DP_SINK, addr);
        }
        if (reconnectEngine_) {
            reconnectEngine_->Terminate();
        }
    }

    // ── Profile 链路打点 + PBAP 同步打点 ──
    HandleProfileLinkRecord(addr, profile, nextState);
    HandlePbapSyncRecord(addr, profile, nextState);

    // ── 全部 Profile 断开判定 ──
    if (nextState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        CheckAllProfilesDisconnected(addr);
    }

    // ── 转发给 ProfileCoordinator 处理协同动作 ──
    if (profileCoordinator_) {
        profileCoordinator_->OnProfileStateChanged(addr, profile, nextState, resolvedPrev);
    }
}

void CarPolicyManager::HandleProfileLinkRecord(const std::string &addr, ProfileType profile, int nextState)
{
    int recordProfileType = ToRecordProfileType(profile);
    if (recordProfileType < 0) {
        return;
    }
    std::string recKey = ProfileRecordKey(addr, recordProfileType);
    if (nextState == static_cast<int>(BTConnectState::CONNECTING)) {
        profileConnectingTime_[recKey] = GetCurrentTimeMs();
        // 对齐 triggerReconnection：首个状态变化时锁存回连标记
        reconnectFlag_[recKey] =
            (reconnectEngine_ && reconnectEngine_->IsActive()) ? RECONNECT_CAR_POLICY : RECONNECT_OTHER;
        HILOGI("Profile CONNECTING: record for %{public}s", GetEncryptAddr(addr).c_str());
    } else if (nextState == static_cast<int>(BTConnectState::CONNECTED)) {
        profileConnectedTime_[recKey] = GetCurrentTimeMs();
        HILOGI("Profile CONNECTED: record for %{public}s", GetEncryptAddr(addr).c_str());
    } else if (nextState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        auto connIt = profileConnectingTime_.find(recKey);
        if (connIt != profileConnectingTime_.end()) {
            if (metricsReporter_) {
                metricsReporter_->ReportProfileLinkRecord(recordProfileType,
                    (reconnectFlag_.count(recKey) > 0) ? reconnectFlag_[recKey] : RECONNECT_OTHER,
                    connIt->second,
                    profileConnectedTime_.count(recKey) > 0 ? profileConnectedTime_[recKey] : "",
                    GetCurrentTimeMs());
            }
            HILOGI("Profile DISCONNECTED: report link record for %{public}s", GetEncryptAddr(addr).c_str());
        }
        profileConnectingTime_.erase(recKey);
        profileConnectedTime_.erase(recKey);
        reconnectFlag_.erase(recKey);
    }
}

void CarPolicyManager::HandlePbapSyncRecord(const std::string &addr, ProfileType profile, int nextState)
{
    if (profile != ProfileType::PBAP_PCE) {
        return;
    }
    if (nextState == static_cast<int>(BTConnectState::CONNECTING)) {
        pbapConnectingTime_[addr] = GetCurrentTimeMs();
        HILOGI("PBAP CONNECTING: record for %{public}s", GetEncryptAddr(addr).c_str());
    } else if (nextState == static_cast<int>(BTConnectState::CONNECTED)) {
        pbapConnectedTime_[addr] = GetCurrentTimeMs();
        HILOGI("PBAP CONNECTED: record for %{public}s", GetEncryptAddr(addr).c_str());
    } else if (nextState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        if (pbapConnectingTime_.count(addr) > 0) {
            if (metricsReporter_) {
                metricsReporter_->ReportPbapSync(
                    pbapConnectingTime_[addr],
                    pbapConnectedTime_.count(addr) > 0 ? pbapConnectedTime_[addr] : "",
                    pbapSyncStartTime_.count(addr) > 0 ? pbapSyncStartTime_[addr] : "",
                    pbapSyncEndTime_.count(addr) > 0 ? pbapSyncEndTime_[addr] : "",
                    GetCurrentTimeMs());
            }
            HILOGI("PBAP DISCONNECTED: report contacts syn for %{public}s", GetEncryptAddr(addr).c_str());
        }
        pbapConnectingTime_.erase(addr);
        pbapConnectedTime_.erase(addr);
        pbapSyncStartTime_.erase(addr);
        pbapSyncEndTime_.erase(addr);
    }
}

std::string CarPolicyManager::ProfileRecordKey(const std::string &addr, int profileType)
{
    return addr + "|" + std::to_string(profileType);
}

int CarPolicyManager::ToRecordProfileType(ProfileType profile) const
{
    switch (profile) {
        case ProfileType::HFP_HF:
            return RECORD_PROFILE_HFP;
        case ProfileType::A2DP_SINK:
            return RECORD_PROFILE_A2DP_SINK;
        default:
            return -1;  // PBAP_PCE 不参与链路打点
    }
}

void CarPolicyManager::CheckAllProfilesDisconnected(const std::string &addr)
{
    auto hfpIt = prevHfpState_.find(addr);
    bool hfpDisconnected = (hfpIt == prevHfpState_.end()) ||
                            (hfpIt->second == static_cast<int>(BTConnectState::DISCONNECTED));
    auto a2dpIt = prevA2dpState_.find(addr);
    bool a2dpDisconnected = (a2dpIt == prevA2dpState_.end()) ||
                             (a2dpIt->second == static_cast<int>(BTConnectState::DISCONNECTED));
    auto pbapIt = prevPbapState_.find(addr);
    bool pbapDisconnected = (pbapIt == prevPbapState_.end()) ||
                            (pbapIt->second == static_cast<int>(BTConnectState::DISCONNECTED));
    if (hfpDisconnected && a2dpDisconnected && pbapDisconnected) {
        HILOGI("All profiles disconnected: %{public}s", GetEncryptAddr(addr).c_str());
        if (switchHandler_.IsSwitchDisconnecting(addr)) {
            // 切号处理主动断开的旧账号设备：此刻 DataShare 已指向新账号，
            // 不记入新账号 penultimate（一次性抑制后清除）
            switchHandler_.ConsumeSwitchDisconnect(addr);
            HILOGI("All profiles disconnected: %{public}s is sub-profile switch disconnect, "
                   "skip penultimate record", GetEncryptAddr(addr).c_str());
            return;
        }
        if (reconnectEngine_) {
            reconnectEngine_->OnAllProfilesDisconnected(addr);
        }
    }
}

void CarPolicyManager::OnFaStatusChanged(int newStatus)
{
    if (!initialized_ || !connectionGuard_) {
        return;
    }
    connectionGuard_->SetFaStatus(newStatus);
    HandlePolicyChanged();
}

void CarPolicyManager::OnBtMusicStatusChanged(int newStatus)
{
    if (!initialized_ || !connectionGuard_) {
        return;
    }
    connectionGuard_->SetBtMusicStatus(newStatus);
    HandlePolicyChanged();
}

void CarPolicyManager::OnSmartCallTransferStatusChanged(int newStatus)
{
    if (!initialized_) {
        return;
    }
    if (smartCallTransferController_) {
        smartCallTransferController_->OnStatusChanged(newStatus);
    }
}

void CarPolicyManager::OnHiechoNotifyOutputPath(uint8_t a2dpOp, const std::string &addr)
{
    HILOGI("OnHiechoNotifyOutputPath: a2dpOp=%{public}u, addr=%{public}s", a2dpOp, GetEncryptAddr(addr).c_str());

    // 优先检查初始化状态，避免未初始化时构造广播 payload
    if (!initialized_ || !dispatcher_) {
        HILOGW("OnHiechoNotifyOutputPath: not initialized, drop");
        return;
    }

    // 将 echo 8,9 payload[2] 的 a2dpOperation 映射为广播 extra value：
    //   2(USE_A2DP)     → 1（FA 卡片在蓝牙音乐）
    //   1(NO_USE_A2DP)  → 0（FA 卡片不在蓝牙音乐）
    //   其它值忽略，不发送广播。
    int value = 0;
    if (a2dpOp == HIECHO_USE_A2DP) {
        value = SWITCH_BLUETOOTH_CARD_VALUE_ON;
    } else if (a2dpOp == HIECHO_NO_USE_A2DP) {
        value = SWITCH_BLUETOOTH_CARD_VALUE_OFF;
    } else {
        HILOGW("OnHiechoNotifyOutputPath: invalid a2dpOp=%{public}u, ignore", a2dpOp);
        return;
    }

    // 切到 car policy 线程串行执行，避免与其它策略事件并发竞争。
    dispatcher_->Dispatch([this, value]() { PublishSwitchBluetoothCardBroadcast(value); });
}

void CarPolicyManager::PublishSwitchBluetoothCardBroadcast(int value)
{
    HILOGI("PublishSwitchBluetoothCardBroadcast: value=%{public}d", value);

    // 构造公共事件：action + extra(key=switch_bluetooth_card, int value)
    // 参考 map_mse_content_observer.cpp::PublishRefreshSmsListEvent 的写法。
    AAFwk::Want want;
    want.SetAction(SWITCH_BLUETOOTH_CARD_ACTION);
    want.SetParam(SWITCH_BLUETOOTH_CARD_EXTRA_KEY, value);

    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(value);

    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    publishInfo.SetSticky(false);

    bool result = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    HILOGI("PublishSwitchBluetoothCardBroadcast: action=%{public}s, result=%{public}d",
           SWITCH_BLUETOOTH_CARD_ACTION, result);
}

void CarPolicyManager::OnBondStateChanged(const std::string &addr, int newState)
{
    if (!initialized_) {
        return;
    }
    int prevState = PAIR_NONE;
    auto it = prevBondState_.find(addr);
    if (it != prevBondState_.end()) {
        prevState = it->second;
    }
    HILOGI("OnBondStateChanged: addr=%{public}s, newState=%{public}d, prevState=%{public}d",
           GetEncryptAddr(addr).c_str(), newState, prevState);
    prevBondState_[addr] = newState;

    if (newState == PAIR_NONE && prevState == PAIR_PAIRED) {
        // 解配对：SinkLimit 允许时强制刷新可配对广播。
        // 设备解配对后释放 SinkLimit 名额，若仍有配对容量则重新开启广播允许配对新设备。
        if (connectionGuard_ && connectionGuard_->IsConnectionAllowedBySinkLimit(addr)) {
            HandleBtSettingUiStatusChange(true);
        }
    } else if (newState == PAIR_PAIRED) {
        // 配对成功：仅停止可配对广播，不触发回连。
        // 回连由 BT ON 或 ACL 断开等事件触发，配对成功本身不触发回连。
        if (advController_) {
            advController_->StopAdv();
        }
    }
}

void CarPolicyManager::OnBtSettingsUiChanged(int newStatus)
{
    if (!initialized_) {
        return;
    }
    HILOGI("OnBtSettingsUiChanged: newStatus=%{public}d", newStatus);
    btSettingsUiActive_ = newStatus;
    // BT 设置界面状态变化 → 控制可配对广播。
    HandleBtSettingUiStatusChange(false);
}

// ── 账号子空间切换处理（转发 SubProfileSwitchHandler，订阅与实现见 subprofile/ 与 dispatcher/）──
void CarPolicyManager::OnUserConfigUpdateFinished()
{
    if (!initialized_) {
        return;
    }
    switchHandler_.OnUserConfigUpdateFinished();
}

void CarPolicyManager::OnSubProfileSwitched()
{
    if (!initialized_) {
        return;
    }
    switchHandler_.OnSubProfileSwitched();
}


void CarPolicyManager::OnDiscoveryStateChanged(int discoveryState)
{
    if (!initialized_) {
        return;
    }
    // 对齐 HwCarPolicy MESSAGE_SCAN_STARTED/FINISHED 分支：
    // STARTED 仅在无起点时记录（containsKey(START) 守卫，不覆盖已有起点）；
    // STOPPED 仅在未上报时补报（start, "", finished）——发现设备场景已在 OnScanDeviceFound 上报并清空起点。
    if (discoveryState == DISCOVERY_STARTED) {
        if (scanReportStartTime_.empty()) {
            scanReportStartTime_ = GetCurrentTimeMs();
        }
    } else if (discoveryState == DISCOVERY_STOPED) {
        if (!scanReportStartTime_.empty()) {
            if (metricsReporter_) {
                metricsReporter_->ReportScanDelay(scanReportStartTime_, "", GetCurrentTimeMs());
            }
            scanReportStartTime_.clear();
        }
    }
}

void CarPolicyManager::OnScanDeviceFound()
{
    if (!initialized_) {
        return;
    }
    // 对齐 HwCarPolicy MESSAGE_SCAN_FOUND 分支：首个发现设备即上报（start, found, ""），
    // 上报后清空起点（reportScanRecord + clear 一次性语义），后续发现设备与 STOPPED 均不再上报。
    if (scanReportStartTime_.empty()) {
        return;
    }
    if (metricsReporter_) {
        metricsReporter_->ReportScanDelay(scanReportStartTime_, GetCurrentTimeMs(), "");
    }
    scanReportStartTime_.clear();
}

void CarPolicyManager::OnCallChanged(const std::string &addr, int callState)
{
    if (!initialized_) {
        return;
    }
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string nowStr = GetCurrentTimeMs();
    HILOGI("OnCallChanged: addr=%{public}s, callState=%{public}d", GetEncryptAddr(addr).c_str(), callState);
    if (callState == static_cast<int>(HfpCallState::HFP_CALL_STATE_INCOMING)) {
        callIncomingTimeStampMs_ = nowMs;
        callReportMap_["CALL_INCOMING"] = nowStr;
    } else if (callState == static_cast<int>(HfpCallState::HFP_CALL_STATE_ACTIVE)) {
        if (callReportMap_.find("CALL_INCOMING") == callReportMap_.end()) {
            callIncomingTimeStampMs_ = nowMs;
            callReportMap_["CALL_INCOMING"] = nowStr;
        }
        callReportMap_["CALL_START"] = nowStr;
    } else if (callState == static_cast<int>(HfpCallState::HFP_CALL_STATE_FINISHED)) {
        callReportMap_["CALL_END"] = nowStr;
        if (callReportMap_.find("CALL_START") != callReportMap_.end()) {
            if (metricsReporter_) {
                metricsReporter_->ReportCallRecord(
                    callReportMap_.count("CALL_INCOMING") > 0 ? callReportMap_["CALL_INCOMING"] : "",
                    callReportMap_["CALL_START"],
                    callReportMap_["CALL_END"],
                    BuildScoSwitchString());
            }
            HILOGI("Call record reported, clearing maps");
        }
        callReportMap_.clear();
        scoSwitchList_.clear();
        callIncomingTimeStampMs_ = 0;
    }
}

std::string CarPolicyManager::BuildScoSwitchString() const
{
    // SCO 串对齐 ArrayList.toString() 格式："[1:3, 0:8]"
    std::string scoStr = "[";
    for (size_t i = 0; i < scoSwitchList_.size(); ++i) {
        if (i > 0) {
            scoStr += ", ";
        }
        scoStr += scoSwitchList_[i];
    }
    scoStr += "]";
    return scoStr;
}

void CarPolicyManager::OnScoSwitchChanged(const std::string &addr, int scoState)
{
    if (!initialized_ || callIncomingTimeStampMs_ == 0) {
        return;
    }
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    // 防御时钟回拨：nowMs 小于来电时间戳时跳过，避免负数导致 SCO 切换记录异常
    if (nowMs < callIncomingTimeStampMs_) {
        HILOGW("OnScoSwitchChanged: clock rollback, nowMs=%{public}lld < incoming=%{public}lld, skip",
               static_cast<long long>(nowMs), static_cast<long long>(callIncomingTimeStampMs_));
        return;
    }
    int64_t switchSec = (nowMs - callIncomingTimeStampMs_) / 1000;
    scoSwitchList_.push_back(std::to_string(scoState) + ":" + std::to_string(switchSec));
    HILOGI("OnScoSwitchChanged: addr=%{public}s, state=%{public}d, sec=%{public}lld",
           GetEncryptAddr(addr).c_str(), scoState, static_cast<long long>(switchSec));
}

void CarPolicyManager::OnPbapActionCompleted(const std::string &addr, int syncState)
{
    if (!initialized_) {
        return;
    }
    HILOGI("OnPbapActionCompleted: addr=%{public}s, syncState=%{public}d",
           GetEncryptAddr(addr).c_str(), syncState);
    // 通讯录同步拉取起止时点（CONTACTS_SYN_STATUS_RECORD），对齐 processSyncStateChanged
    if (syncState == PBAP_SYNC_STATE_DOWNLOADING) {
        pbapSyncStartTime_[addr] = GetCurrentTimeMs();
        HILOGI("PBAP sync start: %{public}s", GetEncryptAddr(addr).c_str());
    } else if (syncState == PBAP_SYNC_STATE_DOWNLOADED) {
        pbapSyncEndTime_[addr] = GetCurrentTimeMs();
        HILOGI("PBAP sync end: %{public}s", GetEncryptAddr(addr).c_str());
    } else if (syncState == PBAP_SYNC_STATE_DOWNLOAD_ERROR) {
        // 对齐：同步失败往 SYNCENDTIME 写错误标记
        pbapSyncEndTime_[addr] = "Failed to add account";
        HILOGW("PBAP sync error: %{public}s", GetEncryptAddr(addr).c_str());
    }
}

}  // namespace bluetooth
}  // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif
/**********  Function hook  ***************/
bool IsAllowConnectProfileHook(std::string address, OHOS::bluetooth::ProfileType profile)
{
    return OHOS::bluetooth::CarPolicyManager::GetInstance().IsAllowConnect(address, profile);
}

void OnProfileDisconnectHook(std::string address)
{
    OHOS::bluetooth::CarPolicyManager::GetInstance().CancelPendingConnectOtherProfiles(address);
}
/**********  Function hook  ***************/
#ifdef __cplusplus
}
#endif
