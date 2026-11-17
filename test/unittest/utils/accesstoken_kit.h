/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNER_KITS_ACCESSTOKEN_KIT_H
#define INTERFACES_INNER_KITS_ACCESSTOKEN_KIT_H

#include <string>
namespace OHOS {
namespace Security {
namespace AccessToken {
typedef enum TypeATokenTypeEnum {
    TOKEN_INVALID = -1,
    TOKEN_HAP = 0,
    TOKEN_NATIVE,
    TOKEN_SHELL,
    TOKEN_TYPE_BUTT,
} ATokenTypeEnum;

typedef enum TypePermissionState {
    PERMISSION_DENIED = -1,
    PERMISSION_GRANTED = 0,
} PermissionState;

struct HapTokenInfo {
    int apiVersion = 0;
    std::string bundleName = "";
    int32_t userID = 0;
};

struct NativeTokenInfo {
    std::string processName;
};

enum AccessTokenKitRet {
    RET_FAILED = -1,
    RET_SUCCESS = 0,
};

class AccessTokenKit {
public:
    static ATokenTypeEnum GetTokenTypeFlag(uint32_t tokenId)
    {
        return tokenType_;
    };

    static int GetHapTokenInfo(uint32_t tokenId, HapTokenInfo& hapTokenInfoRes)
    {
        if (hapTokenInfoRet_ != RET_SUCCESS) {
            return hapTokenInfoRet_;
        }
        hapTokenInfoRes = hapTokenInfo_;
        return RET_SUCCESS;
    };

    static int32_t VerifyAccessToken(uint32_t tokenId, const std::string permissionName)
    {
        return accessTokenKitRet_;
    };

    static int32_t GetNativeTokenInfo(uint32_t tokenId, NativeTokenInfo& nativeTokenInfoRes)
    {
        return 0;
    }

    static ATokenTypeEnum tokenType_;
    static HapTokenInfo hapTokenInfo_;
    static int accessTokenKitRet_;
    // Test seam: override GetHapTokenInfo result. Default RET_SUCCESS keeps the original stub behavior.
    inline static int hapTokenInfoRet_ = RET_SUCCESS;
};

} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif