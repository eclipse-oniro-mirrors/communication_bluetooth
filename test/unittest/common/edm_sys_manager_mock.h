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

#ifndef EDM_UNIT_TEST_EDM_SYS_MANAGER_H
#define EDM_UNIT_TEST_EDM_SYS_MANAGER_H

#include <mutex>
#include <unordered_map>
#include "iremote_object.h"

namespace OHOS {
namespace EDM {
class EdmSysManager {
public:
    static sptr<IRemoteObject> GetRemoteObjectOfSystemAbility(int32_t systemAbilityId);
    void RegisterSystemAbilityOfRemoteObject(int32_t systemAbilityId, sptr<IRemoteObject> object);
    void UnregisterSystemAbilityOfRemoteObject(int32_t systemAbilityId);
private:
    static std::mutex saMutex_;
    static std::unordered_map<int32_t, sptr<IRemoteObject>> remoteServiceMap_;
};
} // namespace EDM
} // namespace OHOS
#endif // EDM_UNIT_TEST_EDM_SYS_MANAGER_H
