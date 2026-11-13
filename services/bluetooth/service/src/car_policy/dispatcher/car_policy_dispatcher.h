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

#ifndef CAR_POLICY_DISPATCHER_H
#define CAR_POLICY_DISPATCHER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "bluetooth_datashare_utils.h"

namespace OHOS {
namespace bluetooth {

class CarPolicyManager;

// ── IVI 电源状态公共事件契约（CES，融合短距发布）──
// 跨仓契约：与融合仓 fcm_common_event_helper.cpp 的发布常量严格一致，
// 修改需两仓同步评审（蓝牙侧断言见 car_policy_manager_test.cpp 契约用例）。
constexpr const char *IVI_POWER_STATE_CHANGED_ACTION =
    "usual.event.fusion_connectivity.IVI_POWER_STATE_CHANGED";
constexpr const char *IVI_POWER_STATE_KEY = "ivi_power_state";
constexpr int32_t IVI_POWER_STATE_STR = 0;
constexpr int32_t IVI_POWER_STATE_STANDBY = 1;
constexpr int32_t IVI_POWER_STATE_ACTIVE = 2;
constexpr int32_t IVI_POWER_STATE_SHUTDOWN = 3;   // 融合侧发送，蓝牙仅用于清理
constexpr int32_t IVI_POWER_STATE_REBOOT = 4;     // 同上
constexpr int32_t IVI_POWER_STATE_INVALID = -1;   // 参数缺失/解析失败

// 用户中心 settingsdata 同步完成广播（切号后新账号数据就绪信号，切号回连主感知源）；
// action 串以用户中心实际发布定义为准
constexpr const char *USER_CONFIG_UPDATE_FINISHED_EVENT = "user_config_update_finished";

/// 事件分发器：将蓝牙协议栈回调、DataShare 状态变化、系统广播等事件投递到车机策略线程
/// （单线程串行执行），避免多线程竞争。
/// 统一汇聚所有异步事件（Profile 状态、FA/BtMusic 广播、Settings 数据变化），
/// 串行化到 car policy 线程处理。内部持有各 Service 的 Observer 实现类。
/// 线程切换复用 DoInCarPolicyThread（thread_util.h），不单独造 FFRT queue。
class CarPolicyDispatcher {
public:
    explicit CarPolicyDispatcher(CarPolicyManager &owner);
    ~CarPolicyDispatcher();

    // 注册/注销所有观察者（Profile Observer + Classic 适配器/远端设备 + FA/BtMusic 广播 + SCT DataShareObserver）
    void RegisterAll();
    void UnregisterAll();

    // 供外部模块切到 car policy 线程执行。
    // 任务经会话门控：执行时若已 Uninit（或 Uninit 后重新 Init），任务直接丢弃，
    // 防止访问已销毁/已重建的组件。
    void Dispatch(std::function<void()> task);
    /// 延时任务。name 用于 ThreadUtil 延时队列去重（同名新任务替换旧任务）。
    /// 注意：延时任务必须提供非空 name —— 无名延时任务会与队列上其他无名延时任务
    /// 互相取消（thread_util PostDelayTask 按 name 匹配清除）。
    void DispatchDelayed(std::function<void()> task, uint64_t delayMs,
                         const std::string &name = std::string());

    /// 按名取消尚未执行的延时任务（无该名任务时无操作）。
    void CancelDelayedTask(const std::string &name);

    /// 等待 car policy 队列中已入队的立即任务全部执行完毕（栅栏同步）。
    /// Uninit 在销毁组件前调用，确保在途任务不会与组件销毁并发访问。
    /// 超时返回 false（队列阻塞等异常场景），调用方告警后继续清理。
    bool WaitQueueIdle(uint32_t timeoutMs);

    /// BT ON 时从 DataShare 读取 FA/BtMusic 持久化状态（URI 常量仅在此定义，避免重复）。
    /// 读取失败时输出 STATUS_UNKNOWN(-1)，由调用方做 fail-open。
    void LoadFaBtMusicStatus(int &outFaStatus, int &outBtMusicStatus);

private:
    // Profile Observer 实现类（注册到各 Profile Service）
    class HfpHfObserverImpl;
    class A2dpSnkObserverImpl;
    class PbapPceObserverImpl;
    // Classic 远端设备观察者（ACL 状态 + 配对状态）
    class ClassicRemoteDeviceObserverImpl;
    // Classic 适配器观察者（扫描状态 + 扫描结果，供 BT_SCAN_DELAY 打点）
    class ClassicAdapterObserverImpl;
    // AdapterManager 状态观察者（BR TURN_ON → OnBluetoothOn，替代 bt_func_hook 公共 hook 侵入）
    class AdapterStateObserverImpl;
    // 账号子空间订阅者（OsAccountSubProfileClient SWITCHED 事件，切号看门狗触发源）
    class OsAccountSubProfileSubscriberImpl;
    // IVI 电源状态公共事件订阅者（CES，融合短距发布；解析后切 car policy 线程透传 Manager）
    class IviPowerEventSubscriberImpl;
    // 用户中心 user_config_update_finished 广播订阅者（CES，切号数据就绪主感知源）
    class UserConfigUpdateSubscriberImpl;

    CarPolicyManager &owner_;
    std::unique_ptr<HfpHfObserverImpl> hfpObserver_;
    std::unique_ptr<A2dpSnkObserverImpl> a2dpObserver_;
    std::unique_ptr<PbapPceObserverImpl> pbapObserver_;
    std::unique_ptr<ClassicRemoteDeviceObserverImpl> classicObserver_;
    std::unique_ptr<ClassicAdapterObserverImpl> classicAdapterObserver_;
    std::unique_ptr<AdapterStateObserverImpl> adapterStateObserver_;
    // Kit 持订阅回调弱引用，业务侧必须保活，故用 shared_ptr 成员
    std::shared_ptr<OsAccountSubProfileSubscriberImpl> subProfileSubscriber_;

    // DataShare 状态变化观察者（复用 BluetoothObserver，回调式，无需 ChangeInfo）
    sptr<BluetoothObserver> faObserver_;
    sptr<BluetoothObserver> btMusicObserver_;
    sptr<BluetoothObserver> sctObserver_;
    sptr<BluetoothObserver> btSettingsUiObserver_;

    // IVI 电源状态公共事件订阅者（CES，融合短距发布）
    std::shared_ptr<IviPowerEventSubscriberImpl> iviPowerSubscriber_;
    // user_config_update_finished 广播订阅器（CES 侧持引用，成员保活）
    std::shared_ptr<UserConfigUpdateSubscriberImpl> userConfigUpdateSubscriber_;

    void RegisterProfileObservers();
    void DeregisterProfileObservers();
    void RegisterFaBtMusicObservers();
    void UnregisterFaBtMusicObservers();
    void RegisterSmartCallTransferObserver();
    void UnregisterSmartCallTransferObserver();
    void RegisterClassicRemoteDeviceObserver();
    void DeregisterClassicRemoteDeviceObserver();
    void RegisterClassicAdapterObserver();
    void DeregisterClassicAdapterObserver();
    void RegisterAdapterManagerStateObserver();
    void DeregisterAdapterManagerStateObserver();
    void RegisterBtSettingsUiObserver();
    void UnregisterBtSettingsUiObserver();
    void RegisterOsAccountSubProfileSubscriber();
    void UnregisterOsAccountSubProfileSubscriber();
    void RegisterIviPowerEventSubscriber();
    void UnregisterIviPowerEventSubscriber();
    void RegisterUserConfigUpdateSubscriber();
    void UnregisterUserConfigUpdateSubscriber();

    /// 注册/注销单个 DataShare key 的观察者（内部构建用户空间 URI）。
    /// 复用 BluetoothDataShareHelperUtils::RegisterObserver，不需调用方持有 helper。
    /// 返回是否注册成功：失败时调用方不应保存 observer，以便下次 RegisterAll 重试。
    bool RegisterSettingsObserver(const std::string &key, const sptr<BluetoothObserver> &observer);
    void UnregisterSettingsObserver(const std::string &key, const sptr<BluetoothObserver> &observer);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_POLICY_DISPATCHER_H
