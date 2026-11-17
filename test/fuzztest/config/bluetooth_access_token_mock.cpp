/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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
#include "bluetooth_access_token_mock.h"

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

__attribute__((weak)) int GetFfrtQueueId(void)
{
    return -1;
}

namespace OHOS {
namespace Bluetooth {
void BluetoothAccessTokenMock::SetNativeTokenInfo()
{
    constexpr int permissionNum = 3;
    const char *perms[permissionNum] = {
        "ohos.permission.MANAGE_BLUETOOTH",
        "ohos.permission.ACCESS_BLUETOOTH",
        "ohos.permission.GET_BLUETOOTH_LOCAL_MAC"
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permissionNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "bluetooth_unittest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}
} // namespace OHOS
} // namespace bluetooth