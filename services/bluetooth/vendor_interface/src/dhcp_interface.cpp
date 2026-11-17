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
#define LOG_TAG "bt_dhcp_interface"
#endif

#include "dhcp_interface.h"
#include "vendor_loader.h"
#include "vendor_log.h"

namespace {
/*
 * 懒加载 vendor 库并解析单个导出符号（extern "C"，如 OH_BtDhcpRegisterClientCallBack）。
 * 库不可用或符号缺失时返回 nullptr，由调用方回退默认值。
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
int32_t DhcpClientInterface::RegisterClientCallBack(const std::string &ifname, const DhcpClientCallBack &callBack)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtDhcpRegisterClientCallBack)>(
        GetVendorSymbol("OH_BtDhcpRegisterClientCallBack"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_BtDhcpRegisterClientCallBack GetVendorSymbol failed");
        return DHCP_INTERFACE_FAILED;
    }
    int32_t ret = vendorFunc(ifname.c_str(), &callBack);
    HILOGI("OH_BtDhcpRegisterClientCallBack ifname=%{public}s, ret=%{public}d", ifname.c_str(), ret);
    return ret;
}

int32_t DhcpClientInterface::StartClient(const std::string &ifname, const std::string &bssid, bool bIpv6)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtDhcpStartClient)>(
        GetVendorSymbol("OH_BtDhcpStartClient"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_BtDhcpStartClient GetVendorSymbol failed");
        return DHCP_INTERFACE_FAILED;
    }
    int32_t ret = vendorFunc(ifname.c_str(), bssid.c_str(), bIpv6);
    HILOGI("OH_BtDhcpStartClient ifname=%{public}s, ret=%{public}d", ifname.c_str(), ret);
    return ret;
}

int32_t DhcpClientInterface::StopClient(const std::string &ifname, bool bIpv4, bool bIpv6)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtDhcpStopClient)>(
        GetVendorSymbol("OH_BtDhcpStopClient"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_BtDhcpStopClient GetVendorSymbol failed");
        return DHCP_INTERFACE_FAILED;
    }
    int32_t ret = vendorFunc(ifname.c_str(), bIpv4, bIpv6);
    HILOGI("OH_BtDhcpStopClient ifname=%{public}s, ret=%{public}d", ifname.c_str(), ret);
    return ret;
}
}  // namespace bluetooth
}  // namespace OHOS