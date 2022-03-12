/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "permission_helper.h"

namespace bluetooth {
int PermissionHelper::VerifyPermission(const std::string &permissionName, const int &pid, const int &uid)
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (result == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        return PERMISSION_GRANTED;
    } else {
        return PERMISSION_DENIED;
    }
}

int PermissionHelper::VerifyUseBluetoothPermission(const int &pid, const int &uid)
{
    if (VerifyPermission("ohos.permission.USE_BLUETOOTH", pid, uid) == PERMISSION_DENIED) {
        return PERMISSION_DENIED;
    }

    return PERMISSION_GRANTED;
}

int PermissionHelper::VerifyDiscoverBluetoothPermission(const int &pid, const int &uid)
{
    if (VerifyPermission("ohos.permission.DISCOVER_BLUETOOTH", pid, uid) == PERMISSION_DENIED) {
        return PERMISSION_DENIED;
    }

    return PERMISSION_GRANTED;
}

int PermissionHelper::VerifyManageBluetoothPermission(const int &pid, const int &uid)
{
    if (VerifyPermission("ohos.permission.MANAGE_BLUETOOTH", pid, uid) == PERMISSION_DENIED) {
        return PERMISSION_DENIED;
    }

    return PERMISSION_GRANTED;
}
}