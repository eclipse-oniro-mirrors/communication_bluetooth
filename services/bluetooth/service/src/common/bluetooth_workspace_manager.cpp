/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_workspace_manager"
#endif

#include "bluetooth_workspace_manager.h"
#include "log.h"
#include "parameters.h"
#include "thread_util.h"
#include "space_mgr_interface.h"

namespace OHOS {
namespace bluetooth {

std::mutex BluetoothWorkspaceManager::instanceMutexLock_;

BluetoothWorkspaceManager &BluetoothWorkspaceManager::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutexLock_);
    static BluetoothWorkspaceManager ins;
    return ins;
}

void BluetoothWorkspaceManager::Init()
{
    bool isInit = isInit_.exchange(true);
    CHECK_AND_RETURN_LOG(!isInit, "Already init");
    RegisterUserSwitchedCommonEvent();
    HandleUserSwitched(BluetoothOsAccount::GetActiveOsAccountId());
}

bool BluetoothWorkspaceManager::IsEnterpriseWorkspace() const
{
    return isEnterpriseWorkspace_.load();
}

void BluetoothWorkspaceManager::HandleUserSwitched(int32_t userId)
{
    CHECK_AND_RETURN_LOG(userId != INVALID_ID, "invalid userId");
    bool isEnterprise = QueryEnterpriseWorkspace(userId);
    if (isEnterprise == isEnterpriseWorkspace_.load() && userId == currentUserId_.load()) {
        return;
    }
    HILOGI("userId: %{public}d, isEnterpriseWorkspace: %{public}d", userId, isEnterprise);
    currentUserId_ = userId;
    isEnterpriseWorkspace_ = isEnterprise;
}

void BluetoothWorkspaceManager::RegisterUserSwitchedCommonEvent()
{
    if (userSwitchedSubscriber_ != nullptr) {
        return;
    }
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission("ohos.permission.MANAGE_LOCAL_ACCOUNTS");
    auto callback = [](const EventFwk::CommonEventData &data) {
        HILOGI("receive user switched, userId: %{public}d", data.GetCode());
        // Work in CommonEvent thread, need post task first.
        DoInAdapterManagerThread([userId = data.GetCode()]() {
            BluetoothWorkspaceManager::GetInstance().HandleUserSwitched(userId);
        });
    };
    userSwitchedSubscriber_ = std::make_shared<BluetoothCommonEventSubscriberEx>(
        subscribeInfo, EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED, callback);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(userSwitchedSubscriber_)) {
        HILOGE("Subscribe USER_SWITCHED common event failed");
        userSwitchedSubscriber_ = nullptr;
        return;
    }
    HILOGI("Subscribe USER_SWITCHED common event success");
}

bool BluetoothWorkspaceManager::QueryEnterpriseWorkspace(int32_t userId)
{
    HILOGI("enter, userId: %{public}d", userId);
    if (!system::GetBoolParameter("persist.space_mgr_service.enterprise_space_enable", false)) {
        HILOGI("enterprise space is not enabled, skip query");
        return false;
    }
    bool isEnterprise = false;
    int32_t ret = SpaceMgrInterface::IsEnterpriseWorkspace(userId, isEnterprise);
    if (ret != 0) {
        HILOGE("IsEnterpriseWorkspace failed, ret: %{public}d, userId: %{public}d", ret, userId);
        return isEnterpriseWorkspace_.load();
    }
    return isEnterprise;
}

} // namespace bluetooth
} // namespace OHOS
