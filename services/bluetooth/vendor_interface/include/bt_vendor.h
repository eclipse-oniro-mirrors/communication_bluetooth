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

#ifndef BT_VENDOR_H
#define BT_VENDOR_H

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

namespace OHOS {
namespace bluetooth {
enum class VendorCommandType {
    VENDOR_BATTERY,         // +VENDORBATTERY
    UPDATE_VENDOR_BATTERY,  // +UPDATEVENDORBATTERY
    CLOSE_VENDOR_BATTERY,   // +CLOSEVENDORBATTERY
    VENDOR_BATTERY_QUERY,   // +VENDORBATTERY=?
    VENDOR_BATTERY_OK,      // +VENDORBATTERY=OK
    X_VENDOR_SF,            // +XVENDORSF
    XS_VENDOR_F,            // +XSVENDORF
    VENDOR,                 // VENDOR
};

/*
 * 导出层接口：extern "C" 直接导出符号，vendor_interface 模块经
 * dlopen/dlsym 获取符号后直接调用，两边签名须保持同步。
 */
#ifdef __cplusplus
extern "C" {
#endif
std::vector<std::string> OH_BtGetFastAdvWhitelist();                  // 获取广播间隔白名单
std::unordered_set<std::string> OH_BtGetFastScanWhitelist();          // 获取快速扫描白名单
bool OH_BtIsWalletService(const std::string &callingName);            // 判断调用方是否为钱包服务
std::string OH_BtGetRealAddressWhitelistAppId(const std::string &bundleName);  // 获取白名单 appID
bool OH_BtIsInRealAddressWhitelist(const std::string &bundleName);    // 判断是否在真实地址白名单
std::string OH_BtGetMmiNotDiscoveryBleWhitelistAppId(const std::string &bundleName);  // 获取 MMI 白名单 appID
bool OH_BtIsInMmiNotDiscoveryBleWhitelist(const std::string &bundleName);  // 判断是否在 MMI 白名单
std::string OH_BtGetVendorCommandType(VendorCommandType type);        // 获取类型对应的命令
std::string OH_BtGetAudioManagerPkgName();                            // 获取音频管家包名
std::string OH_BtGetSettingsCallingName();                            // 获取设置包名
std::string OH_BtGetPcFilesBundleName();                              // 获取 PC 文件管理包名
std::string OH_BtGetPhoneFilesBundleName();                           // 获取手机文件管理包名
std::string OH_BtGetPhotoBundleName();                                // 获取图库包名
std::string OH_BtGetPhotoAbilityName();                               // 获取图库主页面
std::string OH_BtGetVoiceRecognitionBundleName();                     // 获取语音识别包名
std::string OH_BtGetWalletServiceName();                              // 获取钱包 service 包名
std::string OH_BtGetWalletPackageName();                              // 获取钱包包名
std::string OH_BtGetWalletBleDumpAction();                            // 获取钱包 BLE dump 上报 action
std::map<std::string, std::string> OH_BtGetRealAddressWhiteMap();     // 获取真实地址白名单映射
std::map<std::string, std::string> OH_BtGetMmiNotDiscoveryBleWhiteMap();  // 获取 MMI 白名单映射
#ifdef __cplusplus
}
#endif
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BT_VENDOR_H