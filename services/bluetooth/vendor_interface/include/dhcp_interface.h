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

#ifndef DHCP_INTERFACE_H
#define DHCP_INTERFACE_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace bluetooth {
constexpr int32_t DHCP_INTERFACE_SUCCESS = 0;
constexpr int32_t DHCP_INTERFACE_FAILED = -1;
constexpr uint32_t DHCP_INTERFACE_STR_MAX_LEN = 128;
constexpr uint32_t DHCP_INTERFACE_MAX_DNS = 10;

/* 跨 so 传递的 DHCP 结果，字段名/类型/数组长度与 dhcp 库的 DhcpResult 保持一致。 */
struct DhcpInterfaceDnsList {
    uint32_t dnsNumber = 0;
    char dnsAddr[DHCP_INTERFACE_MAX_DNS][DHCP_INTERFACE_STR_MAX_LEN] = {{0}};
};

struct DhcpInterfaceResult {
    int32_t iptype = 0;
    bool isOptSuc = false;
    char strOptClientId[DHCP_INTERFACE_STR_MAX_LEN] = {0};
    char strOptServerId[DHCP_INTERFACE_STR_MAX_LEN] = {0};
    char strOptSubnet[DHCP_INTERFACE_STR_MAX_LEN] = {0};
    char strOptDns1[DHCP_INTERFACE_STR_MAX_LEN] = {0};
    char strOptDns2[DHCP_INTERFACE_STR_MAX_LEN] = {0};
    char strOptRouter1[DHCP_INTERFACE_STR_MAX_LEN] = {0};
    char strOptRouter2[DHCP_INTERFACE_STR_MAX_LEN] = {0};
    uint32_t uOptLeasetime = 0;
    DhcpInterfaceDnsList dnsList;
};

/*
 * 映射回调结构：与 dhcp 库原生 ClientCallBack 形状一一对应，
 * 上层在原位置填充回调，由 service 仓映射为原生 ClientCallBack 注册给 dhcp 库。
 */
struct DhcpClientCallBack {
    void (*onIpSuccessChanged)(int32_t status, const char *ifname, const DhcpInterfaceResult *result) = nullptr;
    void (*onIpFailChanged)(int32_t status, const char *ifname, const char *reason) = nullptr;
};

/*
 * 桥接接口：仅透传 dhcp 客户端回调注册/启停与结果字段，
 * vendor_interface 模块经 dlopen/dlsym 获取符号后直接调用，
 * 签名须与 service 仓 bt_vendor/include/dhcp_interface.h 保持同步。
 */
#ifdef __cplusplus
extern "C" {
#endif
int32_t OH_BtDhcpRegisterClientCallBack(const char *ifname, const DhcpClientCallBack *callBack);
int32_t OH_BtDhcpStartClient(const char *ifname, const char *bssid, bool bIpv6);
int32_t OH_BtDhcpStopClient(const char *ifname, bool bIpv4, bool bIpv6);
#ifdef __cplusplus
}
#endif

/*
 * dhcp 客户端接口：上层（PanNetworkManager）持有并操作本类，
 * 内部完成回调映射与 libbt_vendor_service.z.so 的加载及符号解析。
 * 库不可用（未加载/符号缺失）时返回 DHCP_INTERFACE_FAILED。
 */
class DhcpClientInterface {
public:
    static int32_t RegisterClientCallBack(const std::string &ifname, const DhcpClientCallBack &callBack);
    static int32_t StartClient(const std::string &ifname, const std::string &bssid, bool bIpv6);
    static int32_t StopClient(const std::string &ifname, bool bIpv4, bool bIpv6);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // DHCP_INTERFACE_H