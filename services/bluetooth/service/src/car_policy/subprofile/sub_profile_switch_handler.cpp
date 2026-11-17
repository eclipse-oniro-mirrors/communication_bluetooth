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
#define LOG_TAG "bt_service_sub_profile_switch"
#endif

#include "sub_profile_switch_handler.h"

#include "car_policy_config.h"
#include "dispatcher/car_policy_dispatcher.h"
#include "guard/car_connection_guard.h"
#include "reconnect/device_priority_manager.h"
#include "reconnect/reconnect_engine.h"
#include "reconnect/reconnect_predicates.h"
#include "service/car_profile_service.h"

#include "bluetooth_datashare_utils.h"
#include "common_util.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {

// SWITCHED 切号看门狗延时任务名：SWITCHED 后 5s 内未收到数据同步完成广播，
// 看门狗兜底执行"查询+回连"逻辑；广播按时到达则被取消
constexpr const char *SUB_PROFILE_SWITCH_WATCHDOG_TASK = "sub_profile_switch_watchdog";

// 切号回连延时任务名：场景 1 断开旧设备后延时回连新目标；Uninit 时按名取消，
// 避免延时任务携带组件裸指针越过组件生命周期
constexpr const char *SUB_PROFILE_SWITCH_RECONNECT_TASK = "sub_profile_switch_reconnect";

void SubProfileSwitchHandler::Init(CarPolicyConfig &config, ReconnectEngine &engine, DevicePriorityManager &dpm,
    CarPolicyDispatcher &dispatcher, CarConnectionGuard &guard)
{
    config_ = &config;
    engine_ = &engine;
    dpm_ = &dpm;
    dispatcher_ = &dispatcher;
    guard_ = &guard;
}

void SubProfileSwitchHandler::Uninit()
{
    // 切号相关的在途延时任务按名取消（会话门控亦可兜底，此处主动清理）
    if (dispatcher_ != nullptr) {
        dispatcher_->CancelDelayedTask(SUB_PROFILE_SWITCH_WATCHDOG_TASK);
        dispatcher_->CancelDelayedTask(SUB_PROFILE_SWITCH_RECONNECT_TASK);
    }
    // 断开组件引用：延时任务 Lambda 执行时对指针做空校验，防止组件销毁后
    // 残留指针被解引用（Uninit 首先递增 Manager 会话使门控丢弃旧任务，
    // 此处置空为第二道防线；Re-Init 时由 Init 重新注入）
    engine_ = nullptr;
    dpm_ = nullptr;
    dispatcher_ = nullptr;
    guard_ = nullptr;
    config_ = nullptr;
    switchDisconnectingAddr_.clear();
}

void SubProfileSwitchHandler::LoadCurrentSubProfileId()
{
    int32_t subProfileId = INVALID_SUB_PROFILE_ID;
    ErrCode ret = QueryMainDrivingSubProfileId(subProfileId);
    if (ret != ERR_OK) {
        // 账号服务未就绪等场景：保持缓存为 INVALID，后续 SWITCHED 事件到达时兜底刷新
        HILOGW("LoadCurrentSubProfileId: query failed, ret=%{public}d, keep cache invalid", ret);
        return;
    }
    cachedSubProfileId_ = subProfileId;
    HILOGI("LoadCurrentSubProfileId: %{public}d", cachedSubProfileId_);
}

void SubProfileSwitchHandler::OnUserConfigUpdateFinished()
{
    // 主感知源：用户中心 settingsdata 同步完成广播。广播到达即认为新账号数据就绪，
    // 重查账号 ID 与本地缓存比较（CheckSubProfileIdAndReconnect）决定是否执行切号处理；
    // SWITCHED 事件侧的 5s 看门狗在 HandleSubProfileChanged 入口被取消。
    int32_t latestSubProfileId = INVALID_SUB_PROFILE_ID;
    ErrCode ret = QueryMainDrivingSubProfileId(latestSubProfileId);
    CheckSubProfileIdAndReconnect("broadcast", latestSubProfileId, ret);
}

void SubProfileSwitchHandler::OnSubProfileSwitched()
{
    // 账号子空间切换完成（binder 事件，早于用户中心数据同步完成广播到达）。
    // 此刻 settingsdata 尚未切到新账号（实测滞后约 2s），不能直接执行回连决策，
    // 仅启动 5s 看门狗等待 user_config_update_finished 广播：
    // 广播按时到达 → HandleSubProfileChanged 入口取消看门狗（主路径）；
    // 5s 超时 → 广播可能丢失 → 看门狗兜底执行"查询+回连"逻辑。
    // 广播先于本回调到达的竞态：ID 已与缓存一致说明切换已被广播路径处理，跳过布防；
    // 查询失败无法判断仍布防（fail-safe）。同名延时任务自动覆盖旧任务：
    // 连续快速切号时看门狗随最新一次 SWITCHED 重新计时
    if (dispatcher_ == nullptr || config_ == nullptr) {
        return;
    }
    int32_t latestSubProfileId = INVALID_SUB_PROFILE_ID;
    ErrCode ret = QueryMainDrivingSubProfileId(latestSubProfileId);
    if (ret == ERR_OK && cachedSubProfileId_ != INVALID_SUB_PROFILE_ID &&
        latestSubProfileId == cachedSubProfileId_) {
        HILOGI("OnSubProfileSwitched: subProfileId %{public}d unchanged (broadcast handled first), "
               "skip arming watchdog", latestSubProfileId);
        return;
    }
    HILOGI("OnSubProfileSwitched: arm %{public}llu ms watchdog for %{public}s",
           static_cast<unsigned long long>(config_->GetSubProfileSwitchWatchdogTimeoutMs()),
           USER_CONFIG_UPDATE_FINISHED_EVENT);
    dispatcher_->DispatchDelayed([this]() { OnSubProfileSwitchWatchdog(); },
        config_->GetSubProfileSwitchWatchdogTimeoutMs(), SUB_PROFILE_SWITCH_WATCHDOG_TASK);
}

void SubProfileSwitchHandler::OnSubProfileSwitchWatchdog()
{
    // SWITCHED 后 5s 内未收到 user_config_update_finished 广播：广播可能丢失。
    // 主动查询账号 ID 走公共比较入口（距切换已超 5s，settingsdata 必然切换完成，
    // 数据就绪假设成立；ID 无变化则本次切换已被处理或事件为虚假切换，忽略）
    int32_t latestSubProfileId = INVALID_SUB_PROFILE_ID;
    ErrCode ret = QueryMainDrivingSubProfileId(latestSubProfileId);
    HILOGW("OnSubProfileSwitchWatchdog: %{public}s not received within %{public}llu ms, "
           "fallback: subProfileId=%{public}d(cached=%{public}d, ret=%{public}d)",
           USER_CONFIG_UPDATE_FINISHED_EVENT,
           static_cast<unsigned long long>(config_ ? config_->GetSubProfileSwitchWatchdogTimeoutMs() : 0),
           latestSubProfileId, cachedSubProfileId_, ret);
    CheckSubProfileIdAndReconnect("watchdog", latestSubProfileId, ret);
}

void SubProfileSwitchHandler::CheckSubProfileIdAndReconnect(
    const std::string &trigger, int32_t latestSubProfileId, ErrCode ret)
{
    if (ret != ERR_OK) {
        // 账号查询失败无法判断切换：忽略本次触发。
        // 广播路径等待下一次广播；看门狗路径无后续兜底（切号场景广播通常仍会到达）
        HILOGW("CheckSubProfileIdAndReconnect(%{public}s): query subProfileId failed, ret=%{public}d, skip",
               trigger.c_str(), ret);
        return;
    }
    if (cachedSubProfileId_ == INVALID_SUB_PROFILE_ID) {
        // 缓存无效（Init 基线未建立）：仅建立基线，不视为切换
        cachedSubProfileId_ = latestSubProfileId;
        HILOGI("CheckSubProfileIdAndReconnect(%{public}s): baseline established, skip reconnect handling",
               trigger.c_str());
        return;
    }
    if (latestSubProfileId == cachedSubProfileId_) {
        // ID 未变化：非切号场景的配置更新、重复广播、或切换已被处理，忽略
        HILOGI("CheckSubProfileIdAndReconnect(%{public}s): subProfileId %{public}d unchanged, skip",
               trigger.c_str(), latestSubProfileId);
        return;
    }
    HILOGI("CheckSubProfileIdAndReconnect(%{public}s): subProfileId %{public}d -> %{public}d, "
           "trigger reconnect handling", trigger.c_str(), cachedSubProfileId_, latestSubProfileId);
    HandleSubProfileChanged();
}

void SubProfileSwitchHandler::HandleSubProfileChanged()
{
    // ── 0. 取消 SWITCHED 看门狗：切号处理已开始执行（广播或看门狗触发），
    //    看门狗使命完成（看门狗自身触发时此取消为无操作）──
    if (dispatcher_ != nullptr) {
        dispatcher_->CancelDelayedTask(SUB_PROFILE_SWITCH_WATCHDOG_TASK);
    }
    // 注：不在此清除 switchDisconnectingAddr_——场景1/2 断开前设置的记录抑制需延续至
    // 主动断开完成被 CheckAllProfilesDisconnected 消费（场景3 清除上一轮遗留）
    // ── 0.5. 账号 ID 缓存刷新：本函数在"数据就绪"时点执行（广播/看门狗触发），
    //    此处更新缓存供后续广播与看门狗比对判断"是否有切换"──
    int32_t freshSubProfileId = INVALID_SUB_PROFILE_ID;
    if (QueryMainDrivingSubProfileId(freshSubProfileId) == ERR_OK) {
        HILOGI("HandleSubProfileChanged: refresh subProfileId cache %{public}d -> %{public}d",
               cachedSubProfileId_, freshSubProfileId);
        cachedSubProfileId_ = freshSubProfileId;
    } else {
        HILOGW("HandleSubProfileChanged: query subProfileId failed, keep cache %{public}d", cachedSubProfileId_);
    }
    // ── 1. 终止当前回连（旧会话延时回调经 epoch 失效，避免串会话连接） ──
    if (engine_ != nullptr) {
        engine_->Terminate();
    }

    // ── 2. 重载新账号三级设备（DataShare 查询自动指向新账号，按新账号已配对过滤） ──
    if (dpm_ != nullptr) {
        dpm_->Init();
    }
    std::string targetDev = dpm_ ? dpm_->GetNextDevice("") : "";

    // ── 3. 场景判定与处理 ──
    std::string currentDev = GetCurrentConnectedDevice();
    if (!targetDev.empty() && targetDev == currentDev) {
        // 场景3：新账号回连目标与当前已连设备一致 → 保持 HFP/A2DP 链路不断；
        // 清除上一轮切号可能遗留的断开记录抑制（防御性，正常已在断开完成时消费）。
        // PBAP 通讯录为账号级数据，切号后旧会话已陈旧：直接重新触发 PCE 连接
        switchDisconnectingAddr_.clear();
        HILOGI("HandleSubProfileChanged: target same as connected device, keep connection, retrigger PBAP PCE");
        CarProfileService::Connect(ProfileType::PBAP_PCE, currentDev);
        return;
    }
    if (!currentDev.empty()) {
        // 场景1/2 共同前置：断开当前已连设备（HFP+A2DP，PBAP 随 ACL 断开自清）。
        // 断开记录抑制的唯一设置点：此刻 DataShare 已随广播切到新账号，该设备属切换前
        // 账号，断开后若不抑制会经 penultimate 轮转记入新账号三级记录；
        // 抑制由 CheckAllProfilesDisconnected 消费（一次性），OnAclDisconnect 据此跳过回连
        switchDisconnectingAddr_ = currentDev;
        HILOGI("HandleSubProfileChanged: disconnect current device, target=%{public}s", targetDev.c_str());
        CarProfileService::Disconnect(ProfileType::HFP_HF, currentDev);
        CarProfileService::Disconnect(ProfileType::A2DP_SINK, currentDev);
    }
    if (targetDev.empty()) {
        // 场景2：新账号无回连设备 → 仅断开，不发起回连
        HILOGI("HandleSubProfileChanged: no reconnect target, done");
        return;
    }
    // 场景1：延时后回连新账号目标设备（断链释放窗口，避免与清理中的资源相撞；
    // 引擎将按 preferred→last→penultimate 轮询，首轮即从 targetDev 开始）。
    // 延时任务捕获 this（Handler 为 Manager 单例成员，进程内恒存活）而非组件裸指针，
    // 执行时经会话门控 + 引用空校验双重防御：Uninit 已取消本任务（按名）且置空组件
    // 引用，即使任务残留执行也不解引用已销毁的 ReconnectEngine
    if (engine_ == nullptr || dispatcher_ == nullptr || config_ == nullptr) {
        HILOGW("HandleSubProfileChanged: engine or dispatcher null, skip reconnect");
        return;
    }
    HILOGI("HandleSubProfileChanged: schedule reconnect to %{public}s after %{public}llu ms",
           targetDev.c_str(), static_cast<unsigned long long>(config_->GetSubProfileSwitchReconnectDelayMs()));
    dispatcher_->DispatchDelayed([this, targetDev]() {
        if (engine_ == nullptr) {
            HILOGW("HandleSubProfileChanged: reconnect task dropped, engine not alive");
            return;
        }
        engine_->StartReconnect(targetDev, ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN);
    }, config_->GetSubProfileSwitchReconnectDelayMs(), SUB_PROFILE_SWITCH_RECONNECT_TASK);
}

std::string SubProfileSwitchHandler::GetCurrentConnectedDevice() const
{
    if (guard_ == nullptr) {
        return "";
    }
    // HFP 优先（车机主业务），其次 A2DP
    auto hfpAddrs = guard_->GetHfpConnectedAddrs();
    if (!hfpAddrs.empty()) {
        return hfpAddrs.front();
    }
    auto a2dpAddrs = guard_->GetA2dpConnectedAddrs();
    if (!a2dpAddrs.empty()) {
        return a2dpAddrs.front();
    }
    return "";
}

bool SubProfileSwitchHandler::IsSwitchDisconnecting(const std::string &addr) const
{
    return !switchDisconnectingAddr_.empty() && addr == switchDisconnectingAddr_;
}

void SubProfileSwitchHandler::ConsumeSwitchDisconnect(const std::string &addr)
{
    if (addr == switchDisconnectingAddr_) {
        switchDisconnectingAddr_.clear();
    }
}

}  // namespace bluetooth
}  // namespace OHOS
