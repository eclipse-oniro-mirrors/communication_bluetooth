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

#ifndef SUB_PROFILE_SWITCH_HANDLER_H
#define SUB_PROFILE_SWITCH_HANDLER_H

#include <cstdint>
#include <string>

#include "errors.h"

namespace OHOS {
namespace bluetooth {

class CarPolicyConfig;
class CarPolicyDispatcher;
class CarConnectionGuard;
class DevicePriorityManager;
class ReconnectEngine;

/// 账号子空间（SubProfile）切换处理器：独立封装切号感知（广播主感知 + SWITCHED 看门狗
/// 兜底）、subProfileId 基线缓存与切号三场景处理（断开旧设备/回连新目标/保持链路）。
/// 事件由 CarPolicyDispatcher 订阅并投递到 car policy 线程后经 CarPolicyManager 转发进入；
/// 组件引用由 CarPolicyManager::Init 注入（不持有所有权，生命周期由 Manager 保证）。
class SubProfileSwitchHandler {
public:
    /// subProfileId 缓存无效哨兵值（>=0 均为合法 ID，与任何合法值可区分）
    static constexpr int32_t INVALID_SUB_PROFILE_ID = -1;

    SubProfileSwitchHandler() = default;
    ~SubProfileSwitchHandler() = default;

    /// 注入依赖组件（CarPolicyManager::Init 中、组件构造完成后调用）
    void Init(CarPolicyConfig &config, ReconnectEngine &engine, DevicePriorityManager &dpm,
        CarPolicyDispatcher &dispatcher, CarConnectionGuard &guard);

    /// 取消在途切号看门狗（CarPolicyManager::Uninit 中、组件销毁前调用）
    void Uninit();

    /// 蓝牙启动时读取当前前台 subProfileId 建立比较基线（car policy 线程执行）。
    /// 读取失败保持 INVALID，由后续 SWITCHED 事件兜底刷新。
    void LoadCurrentSubProfileId();

    /// user_config_update_finished 广播到达处理（car policy 线程）：重查账号 ID 与缓存
    /// 比较，有变化即触发切号处理（回连决策），无变化忽略。
    void OnUserConfigUpdateFinished();

    /// 账号子空间 SWITCHED 事件到达（binder，早于数据同步完成广播）：启动 5s 看门狗
    /// 延时任务等待广播；广播按时到达则看门狗被取消，超时视为广播丢失由看门狗兜底。
    /// 广播先到（切换已被处理）时 ID 与缓存一致，跳过布防。
    void OnSubProfileSwitched();

    /// SWITCHED 看门狗超时回调（car policy 线程）：广播未按时到达，主动查询账号 ID
    /// 并执行切号处理（距切换已超时，settingsdata 必然切换完成）。
    void OnSubProfileSwitchWatchdog();

    /// 切号处理中主动断开的旧账号设备判定（供 Manager ACL 断开路径跳过回连）
    bool IsSwitchDisconnecting(const std::string &addr) const;

    /// 消费断开抑制标记（仅当地址匹配时清除，供 Manager 全 profile 断开回调使用）
    void ConsumeSwitchDisconnect(const std::string &addr);

private:
    /// 查询结果与缓存的公共比较入口（广播与看门狗两路共用）：
    /// ID 变化 → HandleSubProfileChanged；无变化/查询失败/基线未建立 → 忽略。
    void CheckSubProfileIdAndReconnect(const std::string &trigger, int32_t latestSubProfileId, ErrCode ret);

    /// subProfileId 发生变化后的账号切换处理（由广播或看门狗触发，此时新账号数据已就绪）：
    /// 刷新 ID 缓存 → 终止当前回连 → 重载新账号三级设备 → 与当前已连设备比较 →
    /// 目标一致保持链路并重连 PBAP；目标为空仅断开；目标不同断开旧设备后延时回连新目标。
    void HandleSubProfileChanged();

    /// 当前已连设备地址（CONNECTING|CONNECTED，HFP 优先其次 A2DP；无连接返回空串）。
    std::string GetCurrentConnectedDevice() const;

    CarPolicyConfig *config_ = nullptr;
    ReconnectEngine *engine_ = nullptr;
    DevicePriorityManager *dpm_ = nullptr;
    CarPolicyDispatcher *dispatcher_ = nullptr;
    CarConnectionGuard *guard_ = nullptr;

    // 当前前台账号子空间 ID 缓存（仅 car policy 线程读写）：
    // 启动时建立基线，广播与看门狗到达后拉取最新值与之比较，变化才触发切号处理
    int32_t cachedSubProfileId_{INVALID_SUB_PROFILE_ID};
    // 切号处理中主动断开的旧账号设备地址（断开前设置）：其断开不记入新账号
    // penultimate、不触发回连（一次性抑制，断开完成时消费）
    std::string switchDisconnectingAddr_;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // SUB_PROFILE_SWITCH_HANDLER_H
