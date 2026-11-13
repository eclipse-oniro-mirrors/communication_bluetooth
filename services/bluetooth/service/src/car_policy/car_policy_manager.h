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

#ifndef CAR_POLICY_MANAGER_H
#define CAR_POLICY_MANAGER_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "errors.h"

#include "car_policy_config.h"
#include "coordinator/profile_coordinator.h"
#include "adv/pairable_adv_controller.h"
#include "device/car_device_type_resolver.h"
#include "guard/car_connection_guard.h"
#include "metrics/car_metrics_reporter.h"
#include "reconnect/device_priority_manager.h"
#include "reconnect/reconnect_engine.h"
#include "reconnect/reconnect_predicates.h"
#include "smart_call/smart_call_transfer_controller.h"
#include "subprofile/sub_profile_switch_handler.h"

namespace OHOS {
namespace bluetooth {

class CarPolicyDispatcher;

/// 车机策略编排入口：接收 Dispatcher 投递的事件，协调 ReconnectEngine、ProfileCoordinator、
/// CarConnectionGuard 等组件完成回连、profile 协同、门控等策略。
/// 事件分发委托 CarPolicyDispatcher，自身专注策略逻辑。
class CarPolicyManager {
public:
    static CarPolicyManager &GetInstance();

    void Init();
    void Uninit();

    // 外部入口（由 a2dp_snk_service / hfp_hf_service 调用，切线程由 dispatcher 负责）
    void StartReconnect(ReconnectReason reason);
    void OnAclDisconnect(const std::string &addr, int rawReason);
    void OnBluetoothOn();
    /// 终止当前回连（供 A2DP SinkLimit 抢占后调用，避免被断开设备被立即重连）。
    void TerminateReconnect();
    void CancelPendingConnectOtherProfiles(const std::string &addr);
    /// 连接守卫抢占检查（供 IsAllowConnectProfileHook 强符号实现调用，
    /// hfp_hf Connect、hfp_hf_statemachine AG 接入、a2dp_snk 连接准入检查经钩子传入
    /// ProfileType::HFP_HF / A2DP_SINK 触达，运行在调用方线程）。
    /// SINK_LIMIT 时内部完成抢占（断开其他设备 + 终止回连）后放行连接，
    /// 其余拒绝原因返回 false；未初始化时 fail-open 返回 true。
    bool IsAllowConnect(const std::string &addr, ProfileType profile);

    // Observer 回调（由 CarPolicyDispatcher 在 car policy 线程调用，无需再切线程）
    void OnProfileStateChanged(const std::string &addr, ProfileType profile, int nextState, int prevState);
    void OnFaStatusChanged(int newStatus);
    void OnBtMusicStatusChanged(int newStatus);
    void OnSmartCallTransferStatusChanged(int newStatus);

    /// 收到 echo 8,9 NOTIFY_OUTPUT_PATH 时由协议层入口（bluetooth_hw_interface.cpp）
    void OnHiechoNotifyOutputPath(uint8_t a2dpOp, const std::string &addr);
    void OnBondStateChanged(const std::string &addr, int newState);
    void OnBtSettingsUiChanged(int newStatus);
    void OnCallChanged(const std::string &addr, int callState);
    void OnScoSwitchChanged(const std::string &addr, int scoState);
    /// PBAP PCE Action 完成回调（电话簿拉取完成等），由 PbapPceObserverImpl 转发。
    void OnPbapActionCompleted(const std::string &addr, int syncState);
    /// 经典蓝牙扫描状态变化（STARTED/STOPED），由 ClassicAdapterObserverImpl 转发。
    void OnDiscoveryStateChanged(int discoveryState);
    /// 扫描中发现设备（首个设备触发上报），由 ClassicAdapterObserverImpl 转发。
    void OnScanDeviceFound();
    /// 账号子空间 SWITCHED 事件到达（binder，早于数据同步完成广播）：转发切号处理器
    /// 启动 5s 看门狗延时任务等待广播。由 CarPolicyDispatcher 的
    /// OsAccountSubProfileSubscriberImpl 在 car policy 线程调用。
    void OnSubProfileSwitched();
    /// user_config_update_finished 广播到达（切号数据就绪主感知源）：转发切号处理器
    /// 重查账号 ID 与缓存比较决定是否执行切号处理。由 CarPolicyDispatcher 的
    /// UserConfigUpdateSubscriberImpl 在 car policy 线程调用。
    void OnUserConfigUpdateFinished();
    /// IVI 电源状态变化（STANDBY/ACTIVE/STR/SHUTDOWN/REBOOT），由
    /// IviPowerEventSubscriber 经 CES 订阅转发（Dispatcher 已切 car policy 线程）。
    void OnIviPowerStateChanged(int32_t state);

    /// 是否处于 IVI STANDBY（供 IviStandbyPredicate 查询，回连期间兜底终止）。
    bool IsInIviStandby() const { return inStandby_; }

    CarConnectionGuard &GetConnectionGuard() { return *connectionGuard_; }
    CarDeviceTypeResolver &GetDeviceTypeResolver() { return *deviceTypeResolver_; }
    SmartCallTransferController &GetSmartCallTransferController() { return *smartCallTransferController_; }

    ReconnectEngine &GetReconnectEngine() { return *reconnectEngine_; }
    CarPolicyDispatcher &GetDispatcher() { return *dispatcher_; }
    /// 账号子空间切换处理器（切号感知/看门狗/三场景处理，详见 SubProfileSwitchHandler）
    SubProfileSwitchHandler &GetSwitchHandler() { return switchHandler_; }

    int GetBtSettingsUiActive() const { return btSettingsUiActive_; }

    /// 是否已完成 Init 且未 Uninit（供 car policy 线程队列任务门控判断组件存活）。
    bool IsInitialized() const { return initialized_.load(); }
    /// 策略会话标识：Init/Uninit 各递增一次。队列任务在投递时捕获会话 id，
    /// 执行时 id 不一致说明组件已销毁或重建，直接丢弃，防止访问旧会话对象
    /// （单例自身恒存活，读取此值始终安全）。
    uint64_t GetSessionId() const { return sessionId_.load(); }

private:
    CarPolicyManager();
    ~CarPolicyManager();
    CarPolicyManager(const CarPolicyManager &) = delete;
    CarPolicyManager &operator=(const CarPolicyManager &) = delete;

    void AssembleReconnectPredicates();
    void CheckAllProfilesDisconnected(const std::string &addr);
    void HandlePolicyChanged();
    void DisconnectAllA2dpSink();
    void AutoConnectA2dpByHfpState();
    /// Profile 链路记录键（addr + profileType）。
    static std::string ProfileRecordKey(const std::string &addr, int profileType);
    /// Profile 记录协议类型编码（HFP=0, A2DP_SINK=1，其余 -1 不参与）。
    int ToRecordProfileType(ProfileType profile) const;
    /// BT 设置界面状态变化 → 控制可配对广播。
    /// UI 活跃时开启广播（便于用户配对新设备），UI 退出且无设备连接时关闭广播。
    /// isForce=true 时跳过状态比较强制刷新（解配对时使用）。
    void HandleBtSettingUiStatusChange(bool isForce);
    /// 是否已有车机设备连接或连接中。
    /// 用于决定是否需要开启可配对广播（已有设备连接时无需广播）。
    bool IsCarConnectedOrConnecting() const;

    /// Profile 链路打点处理（HFP/A2DP 三时点 + 回连标记）。
    void HandleProfileLinkRecord(const std::string &addr, ProfileType profile, int nextState);
    /// PBAP 通讯录同步打点处理（CONNECTING/CONNECTED 记时点，DISCONNECTED 上报）。
    void HandlePbapSyncRecord(const std::string &addr, ProfileType profile, int nextState);
    /// 拼接 SCO 切换记录串，格式 "[1:3, 0:8]"。
    std::string BuildScoSwitchString() const;

    // ── IVI 电源状态处理（car policy 线程）──
    // 唤醒回连对齐 BT_ON 逻辑：ACTIVE 直接拉起轮询回连
    void HandleIviStandby();     // 置待机标志 + 终止进行中回连
    void HandleIviActive();      // 清待机标志 + 拉起 CAR_POWER_ACTIVE 回连
    void HandleIviPowerExit();   // STR/SHUTDOWN/REBOOT：清待机标志，不回连

    /// 发送 SWITCH_BLUETOOTH_CARD 公共事件，通知 MC/FA 卡片管家切换卡片。
    /// value: 1=FA 卡片在蓝牙音乐（使用 A2DP），0=FA 卡片不在蓝牙音乐（不使用 A2DP）
    void PublishSwitchBluetoothCardBroadcast(int value);

    CarPolicyConfig config_;
    std::unique_ptr<CarMetricsReporter> metricsReporter_;
    std::unique_ptr<CarPolicyDispatcher> dispatcher_;
    std::unique_ptr<ReconnectEngine> reconnectEngine_;
    std::unique_ptr<DevicePriorityManager> dpm_;
    std::unique_ptr<CarConnectionGuard> connectionGuard_;
    std::unique_ptr<PairableAdvController> advController_;
    std::unique_ptr<ProfileCoordinator> profileCoordinator_;

    // per-device HFP/A2DP 上次状态缓存（用于判定全部 Profile 断开 + HFP 连接失败 prevState）
    std::unordered_map<std::string, int> prevHfpState_;
    std::unordered_map<std::string, int> prevA2dpState_;
    // per-device PBAP 上次状态缓存（用于判定全部 Profile 断开 + PBAP sync 指标起止时间）
    std::unordered_map<std::string, int> prevPbapState_;
    // per-device PBAP 通讯录同步打点时点（value=GetCurrentTimeMs() 格式化串）
    std::unordered_map<std::string, std::string> pbapConnectingTime_;
    std::unordered_map<std::string, std::string> pbapConnectedTime_;
    std::unordered_map<std::string, std::string> pbapSyncStartTime_;
    std::unordered_map<std::string, std::string> pbapSyncEndTime_;

    // Profile 链路打点采集（BT_CALL_MUSIC_LINK_STATUS_RECORD，key=addr|profileType）
    std::unordered_map<std::string, std::string> profileConnectingTime_;
    std::unordered_map<std::string, std::string> profileConnectedTime_;
    std::unordered_map<std::string, int> reconnectFlag_;
    // per-device 上次 bond 状态缓存（observer 接口不提供 prevState，需自行维护）
    std::unordered_map<std::string, int> prevBondState_;

    int btSettingsUiActive_{0};
    int prevBtSettingsUiStatus_{-1};  // 上次 BT 设置界面状态（-1 表示未初始化，用于检测状态变化避免重复处理）

    // 账号子空间切换处理器（声明于组件之后：析构先于组件，悬挂指针不发生实际访问）
    SubProfileSwitchHandler switchHandler_;

    // IVI 电源状态：是否处于 STANDBY（IviStandbyPredicate 查询，回连期间兜底终止）
    bool inStandby_{false};

    // 通话质量上报数据：记录来电、接听、挂断时间戳和 SCO 切换记录，通话结束后一次性上报。
    std::unordered_map<std::string, std::string> callReportMap_;
    std::vector<std::string> scoSwitchList_;
    int64_t callIncomingTimeStampMs_{0};

    // 扫描延迟上报（BT_SCAN_DELAY）：起点为 GetCurrentTimeMs() 串，空=未扫描/已上报（一次性语义，
    // 对齐 scanreportMap 的 START 键存在性 + reportScanRecord 后 clear）
    std::string scanReportStartTime_;

    std::unique_ptr<CarDeviceTypeResolver> deviceTypeResolver_;
    std::unique_ptr<SmartCallTransferController> smartCallTransferController_;

    std::atomic<bool> initialized_{false};
    std::atomic<uint64_t> sessionId_{0};
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_POLICY_MANAGER_H
