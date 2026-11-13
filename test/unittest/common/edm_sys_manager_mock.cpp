/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "edm_sys_manager_mock.h"
#include "gmock/gmock.h"
#include "edm_log.h"
#include "iservice_registry.h"

namespace OHOS {
namespace EDM {
std::mutex EdmSysManager::saMutex_;
std::unordered_map<int32_t, sptr<IRemoteObject>> EdmSysManager::remoteServiceMap_;
sptr<IRemoteObject> EdmSysManager::GetRemoteObjectOfSystemAbility(int32_t systemAbilityId)
{
    std::lock_guard<std::mutex> lock(saMutex_);
    GTEST_LOG_(INFO) << "mock EdmSysMgrManager GetRemoteObjectOfSystemAbility start remoteServiceMap_ is nullptr :" <<
        (remoteServiceMap_[systemAbilityId] == nullptr);
    if (remoteServiceMap_[systemAbilityId] == nullptr) {
        auto sysAbilityMgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sysAbilityMgr == nullptr) {
            GTEST_LOG_(INFO) << "mock EdmSysMgrManager GetRemoteObjectOfSystemAbility get system ability manager fail.";
            return nullptr;
        }

        auto object = sysAbilityMgr->GetSystemAbility(systemAbilityId);
        if (object == nullptr) {
            GTEST_LOG_(INFO) << "mock EdmSysMgrManager GetRemoteObjectOfSystemAbility get system ability fail.";
            return nullptr;
        }
        remoteServiceMap_[systemAbilityId] = object;
    }
    return remoteServiceMap_[systemAbilityId];
}

void EdmSysManager::RegisterSystemAbilityOfRemoteObject(int32_t systemAbilityId, sptr<IRemoteObject> object)
{
    std::lock_guard<std::mutex> lock(saMutex_);
    remoteServiceMap_[systemAbilityId] = object;
}

void EdmSysManager::UnregisterSystemAbilityOfRemoteObject(int32_t systemAbilityId)
{
    std::lock_guard<std::mutex> lock(saMutex_);
    remoteServiceMap_.erase(systemAbilityId);
}
} // namespace EDM
} // namespace OHOS
