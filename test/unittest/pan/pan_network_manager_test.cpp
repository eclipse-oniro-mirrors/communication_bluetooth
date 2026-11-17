/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <cstring>
#include "pan_network_manager.h"
#include "pan_defines.h"
#include "log.h"
#include "thread_util.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

static int g_dhcpResultCallbackCallCount = 0;
static int g_lastCallbackEvt = 0;
static int g_lastCallbackIpType = 0;
static DhcpResult g_lastCallbackDhcpResult = {};

static void FillValidDhcpResult(DhcpResult &result)
{
    result = {};
    result.iptype = IPV4;
    result.isOptSuc = true;
    strncpy_s(result.strOptClientId, sizeof(result.strOptClientId), "192.168.1.100", strlen("192.168.1.100"));
    strncpy_s(result.strOptRouter1, sizeof(result.strOptRouter1), "192.168.1.1", strlen("192.168.1.1"));
    strncpy_s(result.strOptSubnet, sizeof(result.strOptSubnet), "255.255.255.0", strlen("255.255.255.0"));
    strncpy_s(result.strOptDns1, sizeof(result.strOptDns1), "8.8.8.8", strlen("8.8.8.8"));
    strncpy_s(result.strOptDns2, sizeof(result.strOptDns2), "8.8.4.4", strlen("8.8.4.4"));
    result.uOptLeasetime = 86400;
}

class PanNetworkManagerTest : public testing::Test {
public:
    PanNetworkManagerTest() : panNetworkManager() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        g_dhcpResultCallbackCallCount = 0;
        g_lastCallbackEvt = 0;
        g_lastCallbackIpType = 0;
        g_lastCallbackDhcpResult = {};
        panNetworkManager.SetIfname("bt-pan");
        panNetworkManager.supplierId_ = 0;
        panNetworkManager.dhcpResultCallback_ = nullptr;
        bluetooth::ThreadUtil::GetInstance().ChangeThreadState(
            bluetooth::THREAD_ID_PAN, bluetooth::ThreadUtil::DISABLED);
    }
    void TearDown()
    {
        panNetworkManager.SetIfname("bt-pan");
        panNetworkManager.supplierId_ = 0;
        panNetworkManager.dhcpResultCallback_ = nullptr;
        bluetooth::ThreadUtil::GetInstance().ChangeThreadState(
            bluetooth::THREAD_ID_PAN, bluetooth::ThreadUtil::ENABLED);
    }
private:
    PanNetworkManager panNetworkManager;
};

/*
 * @tc.number: Pan_Network_Manager_SetIfname_GetIfname_00100
 * @tc.name: SetIfname_GetIfname
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, SetIfname_GetIfname, TestSize.Level1)
{
    HILOGI("SetIfname_GetIfname start");
    panNetworkManager.SetIfname("bt-pan0");
    EXPECT_EQ(panNetworkManager.GetIfname(), "bt-pan0");
    panNetworkManager.SetIfname("bt-pan");
    EXPECT_EQ(panNetworkManager.GetIfname(), "bt-pan");
    HILOGI("SetIfname_GetIfname end");
}

/*
 * @tc.number: Pan_Network_Manager_GetSupplierId_00100
 * @tc.name: GetSupplierId
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, GetSupplierId_Default, TestSize.Level1)
{
    HILOGI("GetSupplierId_Default start");
    EXPECT_EQ(panNetworkManager.GetSupplierId(), 0u);
    HILOGI("GetSupplierId_Default end");
}

/*
 * @tc.number: Pan_Network_Manager_SetDhcpResultCallback_OnIpSuccess_00100
 * @tc.name: SetDhcpResultCallback_OnIpSuccess
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, SetDhcpResultCallback_OnIpSuccess, TestSize.Level1)
{
    HILOGI("SetDhcpResultCallback_OnIpSuccess start");
    g_dhcpResultCallbackCallCount = 0;
    auto callback = [](int evt, int ipType, const DhcpResult &result) {
        g_dhcpResultCallbackCallCount++;
        g_lastCallbackEvt = evt;
        g_lastCallbackIpType = ipType;
    };
    panNetworkManager.SetDhcpResultCallback(callback);
    EXPECT_TRUE(panNetworkManager.dhcpResultCallback_ != nullptr);
    HILOGI("SetDhcpResultCallback_OnIpSuccess end");
}

/*
 * @tc.number: Pan_Network_Manager_OnDhcpIpSuccessChanged_NullResult_00100
 * @tc.name: OnDhcpIpSuccessChanged_NullResult
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, OnDhcpIpSuccessChanged_NullResult, TestSize.Level1)
{
    HILOGI("OnDhcpIpSuccessChanged_NullResult start");
    g_dhcpResultCallbackCallCount = 0;
    auto callback = [](int evt, int ipType, const DhcpResult &result) {
        g_dhcpResultCallbackCallCount++;
    };
    panNetworkManager.SetDhcpResultCallback(callback);

    PanNetworkManager::OnDhcpIpSuccessChanged(0, "bt-pan", nullptr);
    EXPECT_EQ(g_dhcpResultCallbackCallCount, 0);
    HILOGI("OnDhcpIpSuccessChanged_NullResult end");
}

/*
 * @tc.number: Pan_Network_Manager_OnDhcpIpSuccessChanged_NoCallback_00100
 * @tc.name: OnDhcpIpSuccessChanged_NoCallback
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, OnDhcpIpSuccessChanged_NoCallback, TestSize.Level1)
{
    HILOGI("OnDhcpIpSuccessChanged_NoCallback start");
    panNetworkManager.dhcpResultCallback_ = nullptr;
    DhcpResult result = {};
    result.isOptSuc = true;
    EXPECT_NO_FATAL_FAILURE(PanNetworkManager::OnDhcpIpSuccessChanged(0, "bt-pan", &result));
    HILOGI("OnDhcpIpSuccessChanged_NoCallback end");
}

/*
 * @tc.number: Pan_Network_Manager_OnDhcpIpFailChanged_00100
 * @tc.name: OnDhcpIpFailChanged
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, OnDhcpIpFailChanged, TestSize.Level1)
{
    HILOGI("OnDhcpIpFailChanged start");
    g_dhcpResultCallbackCallCount = 0;
    g_lastCallbackEvt = 0;
    auto callback = [](int evt, int ipType, const DhcpResult &result) {
        g_dhcpResultCallbackCallCount++;
        g_lastCallbackEvt = evt;
    };
    panNetworkManager.SetDhcpResultCallback(callback);
    EXPECT_NO_FATAL_FAILURE(PanNetworkManager::OnDhcpIpFailChanged(0, "bt-pan", "timeout"));
    HILOGI("OnDhcpIpFailChanged end");
}

/*
 * @tc.number: Pan_Network_Manager_OnDhcpIpFailChanged_NoCallback_00100
 * @tc.name: OnDhcpIpFailChanged_NoCallback
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, OnDhcpIpFailChanged_NoCallback, TestSize.Level1)
{
    HILOGI("OnDhcpIpFailChanged_NoCallback start");
    panNetworkManager.dhcpResultCallback_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(PanNetworkManager::OnDhcpIpFailChanged(0, "bt-pan", "timeout"));
    HILOGI("OnDhcpIpFailChanged_NoCallback end");
}

/*
 * @tc.number: Pan_Network_Manager_UnregisterNetSupplier_ZeroId_00100
 * @tc.name: UnregisterNetSupplier_ZeroId
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, UnregisterNetSupplier_ZeroId, TestSize.Level1)
{
    HILOGI("UnregisterNetSupplier_ZeroId start");
    panNetworkManager.supplierId_ = 0;
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.UnregisterNetSupplier());
    EXPECT_EQ(panNetworkManager.GetSupplierId(), 0u);
    HILOGI("UnregisterNetSupplier_ZeroId end");
}

/*
 * @tc.number: Pan_Network_Manager_UpdateNetSupplierInfo_ZeroId_00100
 * @tc.name: UpdateNetSupplierInfo_ZeroId
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, UpdateNetSupplierInfo_ZeroId, TestSize.Level1)
{
    HILOGI("UpdateNetSupplierInfo_ZeroId start");
    panNetworkManager.supplierId_ = 0;
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.UpdateNetSupplierInfo(true, false));
    HILOGI("UpdateNetSupplierInfo_ZeroId end");
}

/*
 * @tc.number: Pan_Network_Manager_UpdateNetLinkInfo_ZeroId_00100
 * @tc.name: UpdateNetLinkInfo_ZeroId
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, UpdateNetLinkInfo_ZeroId, TestSize.Level1)
{
    HILOGI("UpdateNetLinkInfo_ZeroId start");
    panNetworkManager.supplierId_ = 0;
    PanIpInfo ipInfo;
    ipInfo.ipAddress = "192.168.1.100";
    ipInfo.gateway = "192.168.1.1";
    ipInfo.prefixLength = 24;
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.UpdateNetLinkInfo(ipInfo));
    HILOGI("UpdateNetLinkInfo_ZeroId end");
}

/*
 * @tc.number: Pan_Network_Manager_CreateNetLinkInfo_00100
 * @tc.name: CreateNetLinkInfo
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, CreateNetLinkInfo_FullIpInfo, TestSize.Level1)
{
    HILOGI("CreateNetLinkInfo_FullIpInfo start");
    panNetworkManager.SetIfname("bt-pan0");
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo = sptr<NetManagerStandard::NetLinkInfo>::MakeSptr();
    ASSERT_NE(netLinkInfo, nullptr);

    PanIpInfo ipInfo;
    ipInfo.ipAddress = "192.168.1.100";
    ipInfo.gateway = "192.168.1.1";
    ipInfo.netmask = "255.255.255.0";
    ipInfo.dns1 = "8.8.8.8";
    ipInfo.dns2 = "8.8.4.4";
    ipInfo.domain = "bt-pan";
    ipInfo.prefixLength = 24;

    panNetworkManager.CreateNetLinkInfo(netLinkInfo, ipInfo);

    EXPECT_EQ(netLinkInfo->ifaceName_, "bt-pan0");
    EXPECT_EQ(netLinkInfo->domain_, "bt-pan");
    EXPECT_EQ(static_cast<int>(netLinkInfo->netAddrList_.size()), 1);
    if (!netLinkInfo->netAddrList_.empty()) {
        EXPECT_EQ(netLinkInfo->netAddrList_.front().address_, "192.168.1.100");
        EXPECT_EQ(netLinkInfo->netAddrList_.front().prefixlen_, 24);
    }
    EXPECT_GE(static_cast<int>(netLinkInfo->routeList_.size()), 1);
    EXPECT_GE(static_cast<int>(netLinkInfo->dnsList_.size()), 1);
    if (!netLinkInfo->dnsList_.empty()) {
        EXPECT_EQ(netLinkInfo->dnsList_.front().address_, "8.8.8.8");
    }
    HILOGI("CreateNetLinkInfo_FullIpInfo end");
}

/*
 * @tc.number: Pan_Network_Manager_CreateNetLinkInfo_00200
 * @tc.name: CreateNetLinkInfo
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, CreateNetLinkInfo_EmptyIp, TestSize.Level1)
{
    HILOGI("CreateNetLinkInfo_EmptyIp start");
    panNetworkManager.SetIfname("bt-pan");
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo = sptr<NetManagerStandard::NetLinkInfo>::MakeSptr();
    ASSERT_NE(netLinkInfo, nullptr);

    PanIpInfo ipInfo;
    ipInfo.ipAddress = "";
    ipInfo.dns1 = "";
    ipInfo.domain = "";

    panNetworkManager.CreateNetLinkInfo(netLinkInfo, ipInfo);

    EXPECT_EQ(netLinkInfo->ifaceName_, "bt-pan");
    EXPECT_EQ(netLinkInfo->domain_, "bt-pan");
    EXPECT_EQ(static_cast<int>(netLinkInfo->netAddrList_.size()), 0);
    EXPECT_EQ(static_cast<int>(netLinkInfo->routeList_.size()), 0);
    EXPECT_EQ(static_cast<int>(netLinkInfo->dnsList_.size()), 0);
    HILOGI("CreateNetLinkInfo_EmptyIp end");
}

/*
 * @tc.number: Pan_Network_Manager_CreateNetLinkInfo_00300
 * @tc.name: CreateNetLinkInfo
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, CreateNetLinkInfo_IpWithNoGateway, TestSize.Level1)
{
    HILOGI("CreateNetLinkInfo_IpWithNoGateway start");
    panNetworkManager.SetIfname("bt-pan");
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo = sptr<NetManagerStandard::NetLinkInfo>::MakeSptr();
    ASSERT_NE(netLinkInfo, nullptr);

    PanIpInfo ipInfo;
    ipInfo.ipAddress = "192.168.1.100";
    ipInfo.gateway = "";
    ipInfo.prefixLength = 24;

    panNetworkManager.CreateNetLinkInfo(netLinkInfo, ipInfo);

    EXPECT_EQ(static_cast<int>(netLinkInfo->netAddrList_.size()), 1);
    EXPECT_GE(static_cast<int>(netLinkInfo->routeList_.size()), 1);
    HILOGI("CreateNetLinkInfo_IpWithNoGateway end");
}

/*
 * @tc.number: Pan_Network_Manager_CreateNetLinkInfo_00400
 * @tc.name: CreateNetLinkInfo
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, CreateNetLinkInfo_DefaultDomain, TestSize.Level1)
{
    HILOGI("CreateNetLinkInfo_DefaultDomain start");
    panNetworkManager.SetIfname("bt-pan");
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo = sptr<NetManagerStandard::NetLinkInfo>::MakeSptr();
    ASSERT_NE(netLinkInfo, nullptr);

    PanIpInfo ipInfo;
    ipInfo.ipAddress = "192.168.1.100";
    ipInfo.gateway = "192.168.1.1";
    ipInfo.prefixLength = 24;

    panNetworkManager.CreateNetLinkInfo(netLinkInfo, ipInfo);

    EXPECT_EQ(netLinkInfo->domain_, "bt-pan");
    HILOGI("CreateNetLinkInfo_DefaultDomain end");
}

/*
 * @tc.number: Pan_Network_Manager_CreateNetLinkInfo_00500
 * @tc.name: CreateNetLinkInfo
 * @tc.desc:
 */
HWTEST_F(PanNetworkManagerTest, CreateNetLinkInfo_CustomDomain, TestSize.Level1)
{
    HILOGI("CreateNetLinkInfo_CustomDomain start");
    panNetworkManager.SetIfname("bt-pan");
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo = sptr<NetManagerStandard::NetLinkInfo>::MakeSptr();
    ASSERT_NE(netLinkInfo, nullptr);

    PanIpInfo ipInfo;
    ipInfo.ipAddress = "192.168.1.100";
    ipInfo.gateway = "192.168.1.1";
    ipInfo.prefixLength = 24;
    ipInfo.domain = "custom-domain";

    panNetworkManager.CreateNetLinkInfo(netLinkInfo, ipInfo);

    EXPECT_EQ(netLinkInfo->domain_, "custom-domain");
    HILOGI("CreateNetLinkInfo_CustomDomain end");
}

/*
 * @tc.number: Pan_Network_Manager_OnDhcpIpFailChanged_NullReason_00100
 * @tc.name: OnDhcpIpFailChanged_NullReason
 * @tc.desc: Verify OnDhcpIpFailChanged does not crash when reason is nullptr
 */
HWTEST_F(PanNetworkManagerTest, OnDhcpIpFailChanged_NullReason, TestSize.Level1)
{
    HILOGI("OnDhcpIpFailChanged_NullReason start");
    g_dhcpResultCallbackCallCount = 0;
    auto callback = [](int evt, int ipType, const DhcpResult &result) {
        g_dhcpResultCallbackCallCount++;
    };
    panNetworkManager.SetDhcpResultCallback(callback);

    EXPECT_NO_FATAL_FAILURE(PanNetworkManager::OnDhcpIpFailChanged(0, "bt-pan", nullptr));
    EXPECT_EQ(g_dhcpResultCallbackCallCount, 0);
    HILOGI("OnDhcpIpFailChanged_NullReason end");
}

/*
 * @tc.number: Pan_Network_Manager_IsValidDhcpResult_00100
 * @tc.name: IsValidDhcpResult_ValidResult
 * @tc.desc: Verify IsValidDhcpResult returns true for a valid DhcpResult
 */
HWTEST_F(PanNetworkManagerTest, IsValidDhcpResult_ValidResult, TestSize.Level1)
{
    HILOGI("IsValidDhcpResult_ValidResult start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    EXPECT_TRUE(panNetworkManager.IsValidDhcpResult(result));
    HILOGI("IsValidDhcpResult_ValidResult end");
}

/*
 * @tc.number: Pan_Network_Manager_IsValidDhcpResult_00200
 * @tc.name: IsValidDhcpResult_EmptyClientId
 * @tc.desc: Verify IsValidDhcpResult returns false when strOptClientId is empty
 */
HWTEST_F(PanNetworkManagerTest, IsValidDhcpResult_EmptyClientId, TestSize.Level1)
{
    HILOGI("IsValidDhcpResult_EmptyClientId start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.strOptClientId[0] = '\0';
    EXPECT_FALSE(panNetworkManager.IsValidDhcpResult(result));
    HILOGI("IsValidDhcpResult_EmptyClientId end");
}

/*
 * @tc.number: Pan_Network_Manager_IsValidDhcpResult_00300
 * @tc.name: IsValidDhcpResult_EmptyRouter1
 * @tc.desc: Verify IsValidDhcpResult returns false when strOptRouter1 is empty
 */
HWTEST_F(PanNetworkManagerTest, IsValidDhcpResult_EmptyRouter1, TestSize.Level1)
{
    HILOGI("IsValidDhcpResult_EmptyRouter1 start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.strOptRouter1[0] = '\0';
    EXPECT_FALSE(panNetworkManager.IsValidDhcpResult(result));
    HILOGI("IsValidDhcpResult_EmptyRouter1 end");
}

/*
 * @tc.number: Pan_Network_Manager_IsValidDhcpResult_00400
 * @tc.name: IsValidDhcpResult_EmptySubnet
 * @tc.desc: Verify IsValidDhcpResult returns false when strOptSubnet is empty
 */
HWTEST_F(PanNetworkManagerTest, IsValidDhcpResult_EmptySubnet, TestSize.Level1)
{
    HILOGI("IsValidDhcpResult_EmptySubnet start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.strOptSubnet[0] = '\0';
    EXPECT_FALSE(panNetworkManager.IsValidDhcpResult(result));
    HILOGI("IsValidDhcpResult_EmptySubnet end");
}

/*
 * @tc.number: Pan_Network_Manager_IsValidDhcpResult_00500
 * @tc.name: IsValidDhcpResult_DnsNumberExceedsMax
 * @tc.desc: Verify IsValidDhcpResult returns false when dnsNumber exceeds DHCP_DNS_MAX_NUMBER
 */
HWTEST_F(PanNetworkManagerTest, IsValidDhcpResult_DnsNumberExceedsMax, TestSize.Level1)
{
    HILOGI("IsValidDhcpResult_DnsNumberExceedsMax start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.dnsList.dnsNumber = DHCP_DNS_MAX_NUMBER + 1;
    EXPECT_FALSE(panNetworkManager.IsValidDhcpResult(result));
    HILOGI("IsValidDhcpResult_DnsNumberExceedsMax end");
}


/*
 * @tc.number: Pan_Network_Manager_IsValidDhcpResult_00700
 * @tc.name: IsValidDhcpResult_EmptyDns1
 * @tc.desc: Verify IsValidDhcpResult returns false when strOptDns1 is empty
 */
HWTEST_F(PanNetworkManagerTest, IsValidDhcpResult_EmptyDns1, TestSize.Level1)
{
    HILOGI("IsValidDhcpResult_EmptyDns1 start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.strOptDns1[0] = '\0';
    EXPECT_FALSE(panNetworkManager.IsValidDhcpResult(result));
    HILOGI("IsValidDhcpResult_EmptyDns1 end");
}

/*
 * @tc.number: Pan_Network_Manager_IsValidDhcpResult_00800
 * @tc.name: IsValidDhcpResult_EmptyDns2
 * @tc.desc: Verify IsValidDhcpResult returns false when strOptDns2 is empty
 */
HWTEST_F(PanNetworkManagerTest, IsValidDhcpResult_EmptyDns2, TestSize.Level1)
{
    HILOGI("IsValidDhcpResult_EmptyDns2 start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.strOptDns2[0] = '\0';
    EXPECT_FALSE(panNetworkManager.IsValidDhcpResult(result));
    HILOGI("IsValidDhcpResult_EmptyDns2 end");
}

/*
 * @tc.number: Pan_Network_Manager_IsValidDhcpResult_00900
 * @tc.name: IsValidDhcpResult_EmptyDnsAddr
 * @tc.desc: Verify IsValidDhcpResult returns false when dnsAddr entry is empty
 */
HWTEST_F(PanNetworkManagerTest, IsValidDhcpResult_EmptyDnsAddr, TestSize.Level1)
{
    HILOGI("IsValidDhcpResult_EmptyDnsAddr start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.dnsList.dnsNumber = 1;
    result.dnsList.dnsAddr[0][0] = '\0';
    EXPECT_FALSE(panNetworkManager.IsValidDhcpResult(result));
    HILOGI("IsValidDhcpResult_EmptyDnsAddr end");
}

/*
 * @tc.number: Pan_Network_Manager_SaveIpv4Address_00100
 * @tc.name: SaveIpv4Address_ValidResult
 * @tc.desc: Verify SaveIpv4Address returns true and fills ipInfo correctly for valid DhcpResult
 */
HWTEST_F(PanNetworkManagerTest, SaveIpv4Address_ValidResult, TestSize.Level1)
{
    HILOGI("SaveIpv4Address_ValidResult start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    PanIpInfo ipInfo;
    EXPECT_TRUE(panNetworkManager.SaveIpv4Address(result, ipInfo));
    EXPECT_EQ(ipInfo.ipAddress, "192.168.1.100");
    EXPECT_EQ(ipInfo.gateway, "192.168.1.1");
    EXPECT_EQ(ipInfo.netmask, "255.255.255.0");
    EXPECT_EQ(ipInfo.dns1, "8.8.8.8");
    EXPECT_EQ(ipInfo.dns2, "8.8.4.4");
    EXPECT_EQ(ipInfo.prefixLength, PAN_NETWORK_IPV4_PREFIX_LENGTH);
    EXPECT_EQ(ipInfo.leaseTime, 86400u);
    EXPECT_EQ(ipInfo.domain, "bt-pan");
    HILOGI("SaveIpv4Address_ValidResult end");
}

/*
 * @tc.number: Pan_Network_Manager_SaveIpv4Address_00200
 * @tc.name: SaveIpv4Address_InvalidDhcpResult
 * @tc.desc: Verify SaveIpv4Address returns false when IsValidDhcpResult fails
 */
HWTEST_F(PanNetworkManagerTest, SaveIpv4Address_InvalidDhcpResult, TestSize.Level1)
{
    HILOGI("SaveIpv4Address_InvalidDhcpResult start");
    DhcpResult result = {};
    result.isOptSuc = true;
    PanIpInfo ipInfo;
    EXPECT_FALSE(panNetworkManager.SaveIpv4Address(result, ipInfo));
    HILOGI("SaveIpv4Address_InvalidDhcpResult end");
}

/*
 * @tc.number: Pan_Network_Manager_SaveIpv4Address_00300
 * @tc.name: SaveIpv4Address_NotOptSuc
 * @tc.desc: Verify SaveIpv4Address returns false when isOptSuc is false
 */
HWTEST_F(PanNetworkManagerTest, SaveIpv4Address_NotOptSuc, TestSize.Level1)
{
    HILOGI("SaveIpv4Address_NotOptSuc start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.isOptSuc = false;
    PanIpInfo ipInfo;
    EXPECT_FALSE(panNetworkManager.SaveIpv4Address(result, ipInfo));
    HILOGI("SaveIpv4Address_NotOptSuc end");
}

/*
 * @tc.number: Pan_Network_Manager_SaveIpv4Address_00400
 * @tc.name: SaveIpv4Address_WithDnsListOverride
 * @tc.desc: Verify SaveIpv4Address uses dnsList entries to override dns1/dns2
 */
HWTEST_F(PanNetworkManagerTest, SaveIpv4Address_WithDnsListOverride, TestSize.Level1)
{
    HILOGI("SaveIpv4Address_WithDnsListOverride start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.dnsList.dnsNumber = 2;
    strncpy_s(result.dnsList.dnsAddr[0], sizeof(result.dnsList.dnsAddr[0]),
        "1.1.1.1", strlen("1.1.1.1"));
    strncpy_s(result.dnsList.dnsAddr[1], sizeof(result.dnsList.dnsAddr[1]),
        "1.0.0.1", strlen("1.0.0.1"));
    PanIpInfo ipInfo;
    EXPECT_TRUE(panNetworkManager.SaveIpv4Address(result, ipInfo));
    EXPECT_EQ(ipInfo.dns1, "1.1.1.1");
    EXPECT_EQ(ipInfo.dns2, "1.0.0.1");
    HILOGI("SaveIpv4Address_WithDnsListOverride end");
}

/*
 * @tc.number: Pan_Network_Manager_SaveDhcpResult_Clear_00100
 * @tc.name: SaveDhcpResult_AndClear
 * @tc.desc: Verify SaveDhcpResult saves state and Clear resets it
 */
HWTEST_F(PanNetworkManagerTest, SaveDhcpResult_AndClear, TestSize.Level1)
{
    HILOGI("SaveDhcpResult_AndClear start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    panNetworkManager.SaveDhcpResult(IPV4, result);
    EXPECT_EQ(panNetworkManager.savedIpType_, IPV4);
    EXPECT_EQ(panNetworkManager.savedDhcpResult_.iptype, IPV4);
    EXPECT_EQ(std::string(panNetworkManager.savedDhcpResult_.strOptClientId), "192.168.1.100");

    panNetworkManager.Clear();
    EXPECT_EQ(panNetworkManager.savedIpType_, 0);
    EXPECT_EQ(panNetworkManager.savedDhcpResult_.iptype, 0);
    HILOGI("SaveDhcpResult_AndClear end");
}

/*
 * @tc.number: Pan_Network_Manager_DealDhcpResult_00100
 * @tc.name: DealDhcpResult_IPv4Success
 * @tc.desc: Verify DealDhcpResult returns true for a valid IPv4 DHCP result
 */
HWTEST_F(PanNetworkManagerTest, DealDhcpResult_IPv4Success, TestSize.Level1)
{
    HILOGI("DealDhcpResult_IPv4Success start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    panNetworkManager.SaveDhcpResult(IPV4, result);
    EXPECT_TRUE(panNetworkManager.DealDhcpResult());
    HILOGI("DealDhcpResult_IPv4Success end");
}

/*
 * @tc.number: Pan_Network_Manager_DealDhcpResult_00200
 * @tc.name: DealDhcpResult_IPv6NotSupported
 * @tc.desc: Verify DealDhcpResult returns false for IPv6 type
 */
HWTEST_F(PanNetworkManagerTest, DealDhcpResult_IPv6NotSupported, TestSize.Level1)
{
    HILOGI("DealDhcpResult_IPv6NotSupported start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    panNetworkManager.SaveDhcpResult(IPV6, result);
    EXPECT_FALSE(panNetworkManager.DealDhcpResult());
    HILOGI("DealDhcpResult_IPv6NotSupported end");
}

/*
 * @tc.number: Pan_Network_Manager_DealDhcpResult_00300
 * @tc.name: DealDhcpResult_InvalidResult
 * @tc.desc: Verify DealDhcpResult returns false when saved DhcpResult is invalid
 */
HWTEST_F(PanNetworkManagerTest, DealDhcpResult_InvalidResult, TestSize.Level1)
{
    HILOGI("DealDhcpResult_InvalidResult start");
    DhcpResult result = {};
    result.iptype = IPV4;
    result.isOptSuc = false;
    panNetworkManager.SaveDhcpResult(IPV4, result);
    EXPECT_FALSE(panNetworkManager.DealDhcpResult());
    HILOGI("DealDhcpResult_InvalidResult end");
}

/*
 * @tc.number: Pan_Network_Manager_DealDhcpFailed_00100
 * @tc.name: DealDhcpFailed_ClearsState
 * @tc.desc: Verify DealDhcpFailed calls Clear and resets saved state
 */
HWTEST_F(PanNetworkManagerTest, DealDhcpFailed_ClearsState, TestSize.Level1)
{
    HILOGI("DealDhcpFailed_ClearsState start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    panNetworkManager.SaveDhcpResult(IPV4, result);
    EXPECT_EQ(panNetworkManager.savedIpType_, IPV4);

    panNetworkManager.DealDhcpFailed();
    EXPECT_EQ(panNetworkManager.savedIpType_, 0);
    EXPECT_EQ(panNetworkManager.savedDhcpResult_.iptype, 0);
    HILOGI("DealDhcpFailed_ClearsState end");
}

/*
 * @tc.number: Pan_Network_Manager_CreateNetLinkInfo_00600
 * @tc.name: CreateNetLinkInfo_WithGatewayAndDns
 * @tc.desc: Verify CreateNetLinkInfo sets route gateway and dns list correctly
 */
HWTEST_F(PanNetworkManagerTest, CreateNetLinkInfo_WithGatewayAndDns, TestSize.Level1)
{
    HILOGI("CreateNetLinkInfo_WithGatewayAndDns start");
    panNetworkManager.SetIfname("bt-pan");
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo = sptr<NetManagerStandard::NetLinkInfo>::MakeSptr();
    ASSERT_NE(netLinkInfo, nullptr);

    PanIpInfo ipInfo;
    ipInfo.ipAddress = "10.0.0.2";
    ipInfo.gateway = "10.0.0.1";
    ipInfo.prefixLength = 24;
    ipInfo.dns1 = "8.8.8.8";

    panNetworkManager.CreateNetLinkInfo(netLinkInfo, ipInfo);

    ASSERT_GE(static_cast<int>(netLinkInfo->routeList_.size()), 1);
    EXPECT_EQ(netLinkInfo->routeList_.front().gateway_.address_, "10.0.0.1");
    ASSERT_GE(static_cast<int>(netLinkInfo->dnsList_.size()), 1);
    EXPECT_EQ(netLinkInfo->dnsList_.front().address_, "8.8.8.8");
    HILOGI("CreateNetLinkInfo_WithGatewayAndDns end");
}

/*
 * @tc.number: Pan_Network_Manager_RegisterNetSupplier_00100
 * @tc.name: RegisterNetSupplier_NoFatalFailure
 * @tc.desc: Verify RegisterNetSupplier does not crash
 */
HWTEST_F(PanNetworkManagerTest, RegisterNetSupplier_NoFatalFailure, TestSize.Level1)
{
    HILOGI("RegisterNetSupplier_NoFatalFailure start");
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.RegisterNetSupplier());
    HILOGI("RegisterNetSupplier_NoFatalFailure end");
}

/*
 * @tc.number: Pan_Network_Manager_UnregisterNetSupplier_00200
 * @tc.name: UnregisterNetSupplier_NonZeroId
 * @tc.desc: Verify UnregisterNetSupplier with non-zero supplierId does not crash
 */
HWTEST_F(PanNetworkManagerTest, UnregisterNetSupplier_NonZeroId, TestSize.Level1)
{
    HILOGI("UnregisterNetSupplier_NonZeroId start");
    panNetworkManager.supplierId_ = 100;
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.UnregisterNetSupplier());
    HILOGI("UnregisterNetSupplier_NonZeroId end");
}

/*
 * @tc.number: Pan_Network_Manager_UpdateNetSupplierInfo_00200
 * @tc.name: UpdateNetSupplierInfo_NonZeroId
 * @tc.desc: Verify UpdateNetSupplierInfo with non-zero supplierId does not crash
 */
HWTEST_F(PanNetworkManagerTest, UpdateNetSupplierInfo_NonZeroId, TestSize.Level1)
{
    HILOGI("UpdateNetSupplierInfo_NonZeroId start");
    panNetworkManager.supplierId_ = 100;
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.UpdateNetSupplierInfo(true, false));
    HILOGI("UpdateNetSupplierInfo_NonZeroId end");
}

/*
 * @tc.number: Pan_Network_Manager_UpdateNetLinkInfo_00200
 * @tc.name: UpdateNetLinkInfo_NonZeroId
 * @tc.desc: Verify UpdateNetLinkInfo with non-zero supplierId does not crash
 */
HWTEST_F(PanNetworkManagerTest, UpdateNetLinkInfo_NonZeroId, TestSize.Level1)
{
    HILOGI("UpdateNetLinkInfo_NonZeroId start");
    panNetworkManager.supplierId_ = 100;
    PanIpInfo ipInfo;
    ipInfo.ipAddress = "192.168.1.100";
    ipInfo.gateway = "192.168.1.1";
    ipInfo.prefixLength = 24;
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.UpdateNetLinkInfo(ipInfo));
    HILOGI("UpdateNetLinkInfo_NonZeroId end");
}

/*
 * @tc.number: Pan_Network_Manager_RegisterDhcpClientCallback_00100
 * @tc.name: RegisterDhcpClientCallback_NoFatalFailure
 * @tc.desc: Verify RegisterDhcpClientCallback does not crash
 */
HWTEST_F(PanNetworkManagerTest, RegisterDhcpClientCallback_NoFatalFailure, TestSize.Level1)
{
    HILOGI("RegisterDhcpClientCallback_NoFatalFailure start");
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.RegisterDhcpClientCallback("bt-pan"));
    HILOGI("RegisterDhcpClientCallback_NoFatalFailure end");
}

/*
 * @tc.number: Pan_Network_Manager_StartDhcpClient_00100
 * @tc.name: StartDhcpClient_NoFatalFailure
 * @tc.desc: Verify StartDhcpClient does not crash
 */
HWTEST_F(PanNetworkManagerTest, StartDhcpClient_NoFatalFailure, TestSize.Level1)
{
    HILOGI("StartDhcpClient_NoFatalFailure start");
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.StartDhcpClient("bt-pan"));
    HILOGI("StartDhcpClient_NoFatalFailure end");
}

/*
 * @tc.number: Pan_Network_Manager_StopDhcpClient_00100
 * @tc.name: StopDhcpClient_NoFatalFailure
 * @tc.desc: Verify StopDhcpClient does not crash
 */
HWTEST_F(PanNetworkManagerTest, StopDhcpClient_NoFatalFailure, TestSize.Level1)
{
    HILOGI("StopDhcpClient_NoFatalFailure start");
    EXPECT_NO_FATAL_FAILURE(panNetworkManager.StopDhcpClient("bt-pan", true, false));
    HILOGI("StopDhcpClient_NoFatalFailure end");
}

/*
 * @tc.number: Pan_Network_Manager_DealDhcpResult_00400
 * @tc.name: DealDhcpResult_DefaultState
 * @tc.desc: Verify DealDhcpResult returns false when no DHCP result has been saved
 */
HWTEST_F(PanNetworkManagerTest, DealDhcpResult_DefaultState, TestSize.Level1)
{
    HILOGI("DealDhcpResult_DefaultState start");
    panNetworkManager.Clear();
    EXPECT_FALSE(panNetworkManager.DealDhcpResult());
    HILOGI("DealDhcpResult_DefaultState end");
}

/*
 * @tc.number: Pan_Network_Manager_SaveIpv4Address_00500
 * @tc.name: SaveIpv4Address_DnsListSingleEntry
 * @tc.desc: Verify SaveIpv4Address overrides only dns1 when dnsList has one entry
 */
HWTEST_F(PanNetworkManagerTest, SaveIpv4Address_DnsListSingleEntry, TestSize.Level1)
{
    HILOGI("SaveIpv4Address_DnsListSingleEntry start");
    DhcpResult result = {};
    FillValidDhcpResult(result);
    result.dnsList.dnsNumber = 1;
    strncpy_s(result.dnsList.dnsAddr[0], sizeof(result.dnsList.dnsAddr[0]),
        "9.9.9.9", strlen("9.9.9.9"));
    PanIpInfo ipInfo;
    EXPECT_TRUE(panNetworkManager.SaveIpv4Address(result, ipInfo));
    EXPECT_EQ(ipInfo.dns1, "9.9.9.9");
    EXPECT_EQ(ipInfo.dns2, "8.8.4.4");
    HILOGI("SaveIpv4Address_DnsListSingleEntry end");
}

/*
 * @tc.number: Pan_Network_Manager_CreateNetLinkInfo_00700
 * @tc.name: CreateNetLinkInfo_RouteDetails
 * @tc.desc: Verify CreateNetLinkInfo creates default route, host route, and local route
 */
HWTEST_F(PanNetworkManagerTest, CreateNetLinkInfo_RouteDetails, TestSize.Level1)
{
    HILOGI("CreateNetLinkInfo_RouteDetails start");
    panNetworkManager.SetIfname("bt-pan");
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo = sptr<NetManagerStandard::NetLinkInfo>::MakeSptr();
    ASSERT_NE(netLinkInfo, nullptr);

    PanIpInfo ipInfo;
    ipInfo.ipAddress = "192.168.1.100";
    ipInfo.gateway = "192.168.1.1";
    ipInfo.prefixLength = 24;

    panNetworkManager.CreateNetLinkInfo(netLinkInfo, ipInfo);

    ASSERT_EQ(static_cast<int>(netLinkInfo->routeList_.size()), 3);
    auto it = netLinkInfo->routeList_.begin();
    EXPECT_EQ(it->destination_.address_, "0.0.0.0");
    EXPECT_EQ(it->gateway_.address_, "192.168.1.1");
    ++it;
    EXPECT_EQ(it->destination_.address_, "192.168.1.1");
    EXPECT_EQ(it->destination_.prefixlen_, MAX_PREFIX_LENGTH);
    ++it;
    EXPECT_EQ(it->destination_.address_, "192.168.1.100");
    EXPECT_EQ(it->destination_.prefixlen_, 24);
    EXPECT_EQ(it->gateway_.address_, "0.0.0.0");
    HILOGI("CreateNetLinkInfo_RouteDetails end");
}
}
}
