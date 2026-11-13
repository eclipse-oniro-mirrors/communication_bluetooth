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
#ifndef BLUETOOTH_WORKSPACE_MANAGER_H
#define BLUETOOTH_WORKSPACE_MANAGER_H

#include <atomic>
#include <memory>
#include <mutex>
#include "bluetooth_common_event_subscriber.h"
#include "bluetooth_os_account.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Represents bluetooth workspace manager, which detects the switch
 *        between personal space and enterprise workspace by user switched event.
 */
class BluetoothWorkspaceManager {
public:
    /**
     * @brief Get bluetooth workspace manager singleton instance.
     *
     * @return Returns the singleton instance.
     */
    static BluetoothWorkspaceManager &GetInstance();

    /**
     * @brief Subscribe user switched common event and init workspace state.
     */
    void Init();

    /**
     * @brief Check whether current foreground user is in enterprise workspace.
     *
     * @return Returns <b>true</b> if in enterprise workspace;
     *         returns <b>false</b> otherwise.
     */
    bool IsEnterpriseWorkspace() const;

    /**
     * @brief Handle user switched, query workspace state by user id.
     *
     * @param userId The user id switched to.
     */
    void HandleUserSwitched(int32_t userId);

private:
    BluetoothWorkspaceManager() = default;
    ~BluetoothWorkspaceManager() = default;
    static std::mutex instanceMutexLock_;

    void RegisterUserSwitchedCommonEvent();
    bool QueryEnterpriseWorkspace(int32_t userId);

    std::shared_ptr<BluetoothCommonEventSubscriberEx> userSwitchedSubscriber_ = nullptr;
    std::atomic_bool isEnterpriseWorkspace_ {false};
    std::atomic<int32_t> currentUserId_ {INVALID_ID};
    std::atomic_bool isInit_ {false};
};
} // namespace bluetooth
} // namespace OHOS

#endif // BLUETOOTH_WORKSPACE_MANAGER_H
