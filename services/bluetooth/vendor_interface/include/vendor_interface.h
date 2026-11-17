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

#ifndef VENDOR_INTERFACE_H
#define VENDOR_INTERFACE_H

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "bt_vendor.h"

namespace OHOS {
namespace bluetooth {
/*
 * 便利封装层：内部自动完成 libbt_vendor.z.so 的加载与符号解析（懒加载缓存），
 * 同级及其他模块直接 include 本头文件调用即可，无需自行 dlopen/dlsym。
 * 库不可用（未加载/符号缺失）时返回默认值：false / 空串 / 空容器。
 */
class VendorInterface {
public:
    static std::vector<std::string> GetFastAdvWhitelist();
    static std::unordered_set<std::string> GetFastScanWhitelist();
    static bool IsWalletService(const std::string &callingName);
    static std::string GetRealAddressWhitelistAppId(const std::string &bundleName);
    static bool IsInRealAddressWhitelist(const std::string &bundleName);
    static std::string GetMmiNotDiscoveryBleWhitelistAppId(const std::string &bundleName);
    static bool IsInMmiNotDiscoveryBleWhitelist(const std::string &bundleName);
    static std::string GetVendorCommandType(VendorCommandType type);
    static std::string GetAudioManagerPkgName();
    static std::string GetSettingsCallingName();
    static std::string GetPcFilesBundleName();
    static std::string GetPhoneFilesBundleName();
    static std::string GetPhotoBundleName();
    static std::string GetPhotoAbilityName();
    static std::string GetVoiceRecognitionBundleName();
    static std::string GetWalletServiceName();
    static std::string GetWalletPackageName();
    static std::string GetWalletBleDumpAction();
    static std::map<std::string, std::string> GetRealAddressWhiteMap();
    static std::map<std::string, std::string> GetMmiNotDiscoveryBleWhiteMap();
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // VENDOR_INTERFACE_H