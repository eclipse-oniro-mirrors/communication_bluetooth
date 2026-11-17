
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

#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD000102
#endif
#ifndef LOG_TAG
#define LOG_TAG "bt_vendor_interface"
#endif

#include "vendor_interface.h"
#include "vendor_loader.h"
#include "vendor_log.h"

namespace {
/*
 * 懒加载 vendor 库并解析单个导出符号（extern "C"，如 OH_BtGetFastAdvWhitelist）。
 * 库不可用或符号缺失时返回 nullptr，由调用方回退默认值。
 * 各接口使用 decltype(&OH_BtXxx) 取头文件声明的函数指针类型，
 * 保证与 vendor 侧定义签名严格一致。
 */
void *GetVendorSymbol(const char *symbolName)
{
    void *handle = nullptr;
    if (vendor_interface_load_vendor_library(&handle) != 0) {
        HILOGE("vendor library is unavailable");
        return nullptr;
    }
    return vendor_interface_get_vendor_symbol(symbolName);
}
}  // namespace

namespace OHOS {
namespace bluetooth {
std::vector<std::string> VendorInterface::GetFastAdvWhitelist()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetFastAdvWhitelist)>(
        GetVendorSymbol("OH_BtGetFastAdvWhitelist"));
    if (vendorFunc == nullptr) {
        return {};
    }
    return vendorFunc();
}

std::unordered_set<std::string> VendorInterface::GetFastScanWhitelist()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetFastScanWhitelist)>(
        GetVendorSymbol("OH_BtGetFastScanWhitelist"));
    if (vendorFunc == nullptr) {
        return {};
    }
    return vendorFunc();
}

bool VendorInterface::IsWalletService(const std::string &callingName)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtIsWalletService)>(
        GetVendorSymbol("OH_BtIsWalletService"));
    if (vendorFunc == nullptr) {
        return false;
    }
    return vendorFunc(callingName);
}

std::string VendorInterface::GetRealAddressWhitelistAppId(const std::string &bundleName)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetRealAddressWhitelistAppId)>(
        GetVendorSymbol("OH_BtGetRealAddressWhitelistAppId"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc(bundleName);
}

bool VendorInterface::IsInRealAddressWhitelist(const std::string &bundleName)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtIsInRealAddressWhitelist)>(
        GetVendorSymbol("OH_BtIsInRealAddressWhitelist"));
    if (vendorFunc == nullptr) {
        return false;
    }
    return vendorFunc(bundleName);
}

std::string VendorInterface::GetMmiNotDiscoveryBleWhitelistAppId(const std::string &bundleName)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetMmiNotDiscoveryBleWhitelistAppId)>(
        GetVendorSymbol("OH_BtGetMmiNotDiscoveryBleWhitelistAppId"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc(bundleName);
}

bool VendorInterface::IsInMmiNotDiscoveryBleWhitelist(const std::string &bundleName)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtIsInMmiNotDiscoveryBleWhitelist)>(
        GetVendorSymbol("OH_BtIsInMmiNotDiscoveryBleWhitelist"));
    if (vendorFunc == nullptr) {
        return false;
    }
    return vendorFunc(bundleName);
}

std::string VendorInterface::GetVendorCommandType(VendorCommandType type)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetVendorCommandType)>(
        GetVendorSymbol("OH_BtGetVendorCommandType"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc(type);
}

std::string VendorInterface::GetAudioManagerPkgName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetAudioManagerPkgName)>(
        GetVendorSymbol("OH_BtGetAudioManagerPkgName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetSettingsCallingName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetSettingsCallingName)>(
        GetVendorSymbol("OH_BtGetSettingsCallingName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetPcFilesBundleName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetPcFilesBundleName)>(
        GetVendorSymbol("OH_BtGetPcFilesBundleName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetPhoneFilesBundleName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetPhoneFilesBundleName)>(
        GetVendorSymbol("OH_BtGetPhoneFilesBundleName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetPhotoBundleName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetPhotoBundleName)>(
        GetVendorSymbol("OH_BtGetPhotoBundleName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetPhotoAbilityName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetPhotoAbilityName)>(
        GetVendorSymbol("OH_BtGetPhotoAbilityName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetVoiceRecognitionBundleName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetVoiceRecognitionBundleName)>(
        GetVendorSymbol("OH_BtGetVoiceRecognitionBundleName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetWalletServiceName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetWalletServiceName)>(
        GetVendorSymbol("OH_BtGetWalletServiceName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetWalletPackageName()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetWalletPackageName)>(
        GetVendorSymbol("OH_BtGetWalletPackageName"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::string VendorInterface::GetWalletBleDumpAction()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetWalletBleDumpAction)>(
        GetVendorSymbol("OH_BtGetWalletBleDumpAction"));
    if (vendorFunc == nullptr) {
        return "";
    }
    return vendorFunc();
}

std::map<std::string, std::string> VendorInterface::GetRealAddressWhiteMap()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetRealAddressWhiteMap)>(
        GetVendorSymbol("OH_BtGetRealAddressWhiteMap"));
    if (vendorFunc == nullptr) {
        return {};
    }
    return vendorFunc();
}

std::map<std::string, std::string> VendorInterface::GetMmiNotDiscoveryBleWhiteMap()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetMmiNotDiscoveryBleWhiteMap)>(
        GetVendorSymbol("OH_BtGetMmiNotDiscoveryBleWhiteMap"));
    if (vendorFunc == nullptr) {
        return {};
    }
    return vendorFunc();
}
}  // namespace bluetooth
}  // namespace OHOS