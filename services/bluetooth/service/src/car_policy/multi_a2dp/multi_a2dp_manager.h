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

#ifndef MULTI_A2DP_MANAGER_H
#define MULTI_A2DP_MANAGER_H

#include <string>
#include <cstdint>

namespace OHOS {
namespace Bluetooth {

class MultiA2dpManager {
public:
    static void Init();
    static void Cleanup();

    static int32_t GetDisplayIdByAddr(const std::string &addr);
    static std::string GetAddrByDisplayId(int32_t displayId);
    static int32_t GetUserIdByDisplayId(int32_t displayId);
    static int32_t GetUserIdByAddr(const std::string &addr);
    static int32_t GetDisplayIdByUserId(int32_t userId);
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // MULTI_A2DP_MANAGER_H
