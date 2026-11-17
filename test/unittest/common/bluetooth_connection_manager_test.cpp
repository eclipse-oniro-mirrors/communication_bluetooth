/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "service_util.h"
#include "adapter_manager.h"
#include "ble_address_with_type.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_connection_manager.h"
#include "profile_service_manager.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
constexpr int RESET_NO_REFUSE_LINK_TIMEOUT_500MS = 500 * 1000;
ServiceUtil serviceUtil;

class BluetoothConnectionManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
    }
    static void TearDownTestCase()
    {
        HILOGI("start");
        host_ = nullptr;
        utility::CloseAllTimer();
    }

    void InitRecordAclConnect()
    {
        HILOGI("InitRecordAclConnect enter");
        auto connectionManager = BluetoothConnectionManager::GetInstance();
        int linkType = LINK_TYPE_BREDR;
        int connectState = CONNECTION_STATE_CONNECTED;

        std::string address = "11:11:11:11:11:11";
        std::string unPairedAddr;
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "22:22:22:22:22:22";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "33:33:33:33:33:33";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "44:44:44:44:44:44";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "55:55:55:55:55:55";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);
    }

    void ClearRecordAclConnect()
    {
        HILOGI("ClearRecordAclConnect enter");
        auto connectionManager = BluetoothConnectionManager::GetInstance();
        int linkType = LINK_TYPE_BREDR;
        int connectState = CONNECTION_STATE_DISCONNECTED;

        std::string address = "11:11:11:11:11:11";
        std::string unPairedAddr;
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "22:22:22:22:22:22";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "33:33:33:33:33:33";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "44:44:44:44:44:44";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "55:55:55:55:55:55";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "66:66:66:66:66:66";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "77:77:77:77:77:77";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

        address = "xx:xx:xx:xx:xx:xx";
        connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);
    }

    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number:connection_manager_test_001
 * @tc.name: is brLink allowed
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_001, TestSize.Level1)
{
    HILOGI("connection_manager_test_001 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    InitRecordAclConnect();
    int linkType = LINK_TYPE_BREDR;
    int connectState = CONNECTION_STATE_CONNECTED;
    std::string address = "66:66:66:66:66:66";
    bool result = connectionManager->IsBrLinkAllowed(address);
    EXPECT_EQ(result, true);

    // 66 is Exist -> pass
    std::string unPairedAddr;
    connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);
    result = connectionManager->IsBrLinkAllowed(address);
    EXPECT_EQ(result, true);

    std::uint32_t tokenId = 666;
    // acl is exist -> pass
    int64_t currentTime = 0;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    //max 6 -> false
    std::string newAddress = "xx:xx:xx:xx:xx:xx";
    result = connectionManager->IsBrLinkAllowed(newAddress);
    EXPECT_EQ(result, false);

    //66 disconnect ->xx true and record
    connectState = CONNECTION_STATE_DISCONNECTED;
    connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);
    result = connectionManager->IsBrLinkAllowed(newAddress, unPairedAddr);
    EXPECT_EQ(result, true);
    connectState = CONNECTION_STATE_CONNECTED;
    connectionManager->RecordAclConnect(newAddress, linkType, connectState, unPairedAddr);

    // 22 unpaired
    newAddress = "77:77:77:77:77:77";
    unPairedAddr = "22:22:22:22:22:22";
    connectionManager->RecordAclConnect(newAddress, linkType, connectState, unPairedAddr);
    result = connectionManager->IsBrLinkAllowed(newAddress);
    EXPECT_EQ(result, true);
    ClearRecordAclConnect();
    HILOGI("connection_manager_test_001 end");
}

/*
 * @tc.number:connection_manager_test_002
 * @tc.name: sockect connection frequent
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_002, TestSize.Level1)
{
    HILOGI("connection_manager_test_002 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    std::uint32_t tokenId = 666;
    std::string address = "xx:xx:xx:xx:xx:xx";

    // curtime - firstConnectTime < 300 pass
    int64_t currentTime = 0;
    bool result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    // curtime - firstConnectTime < 300 pass
    currentTime = 280;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);
    
    // curtime - lastConnectedTime < 60 allow 1 pass
    // 320 - 280 < 60
    currentTime = 320;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    // curtime - lastConnectedTime < 60 more than 1 fail
    // 322 - 320 < 60
    currentTime = 322;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, true);

    // if address is set no refuse, it should no longer be controlled
    connectionManager->SetConnectionPriority(
        address, Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NO_REFUSE_FREQUENT_CONNECT);
    currentTime = 323;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    // no refuse flag will be reset after 500ms, and will be controlled
    connectionManager->SetConnectionPriority(
        address, Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NO_REFUSE_FREQUENT_CONNECT);
    usleep(RESET_NO_REFUSE_LINK_TIMEOUT_500MS + 1);
    currentTime = 324;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, true);

    // curtime - lastConnectedTime > 60 more than success
    // 385 - 320 > 60
    currentTime = 385;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    // currtime - lastConnectTime > 120 clear
    // 445 - 322 > 120
    currentTime = 445;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, true);

    // curtime - firstConnectTime < 300 pass
    // 450 - 450 allow (less than 300)
    currentTime = 450;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    // curtime - firstConnectTime < 300 pass
    // 1460 - 450 allow (less than 300)
    currentTime = 1460;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);
    HILOGI("connection_manager_test_002 end");
}

/*
 * @tc.number:connection_manager_test_003
 * @tc.name: is bleLink allowed
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_003, TestSize.Level1)
{
    HILOGI("connection_manager_test_003 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    int linkType = LINK_TYPE_LE;
    int connectState = CONNECTION_STATE_CONNECTED;

    std::string address = "66:66:66:66:66:66";

    // ble add list
    std::string unPairedAddr;
    connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

    // ble remove list
    connectState = CONNECTION_STATE_DISCONNECTED;
    connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);

    // ble remove unknown list
    linkType = LINK_TYPE_UNKNOWN;
    connectionManager->RecordAclConnect(address, linkType, connectState, unPairedAddr);
    HILOGI("connection_manager_test_003 end");
}

/*
 * @tc.number:connection_manager_test_004
 * @tc.name: sockect connection frequent change tokenId
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_004, TestSize.Level1)
{
    HILOGI("connection_manager_test_004 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    std::uint32_t tokenId = 555;
    std::string address = "xx:xx:xx:xx:xx:xx";

    int64_t currentTime = 0;
    bool result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    tokenId = 777;
    address = "11:11:11:11:11:11";
    currentTime = 0;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    tokenId = 555;
    address = "xx:xx:xx:xx:xx:xx";
    currentTime = 280;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    currentTime = 320;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);

    currentTime = 1320;
    result = connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, currentTime, address);
    EXPECT_EQ(result, false);
    HILOGI("connection_manager_test_004 end");
}

/*
 * @tc.number:connection_manager_test_005
 * @tc.name: isBleAclAllowed
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_005, TestSize.Level1)
{
    HILOGI("connection_manager_test_005 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    std::string address = "66:66:66:66:66:66";
    RawAddress addr(address);
    EXPECT_FALSE(connectionManager->isBleAclAllowed(addr));
    HILOGI("connection_manager_test_005 end");
}

/*
 * @tc.number:connection_manager_test_006
 * @tc.name: DisconnectForDeviceLimit
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_006, TestSize.Level1)
{
    HILOGI("connection_manager_test_006 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    InitRecordAclConnect();
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(ADAPTER_BREDR, "A2dpSrcService");
    bool result = connectionManager->DisconnectForDeviceLimit();
    EXPECT_EQ(result, false);
    ClearRecordAclConnect();
    HILOGI("connection_manager_test_006 end");
}

/*
 * @tc.number:connection_manager_test_007
 * @tc.name: DisconnectForDeviceLimit
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_007, TestSize.Level1)
{
    HILOGI("connection_manager_test_007 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->AddAclLinks("33:11:55:55:66:22", 0, "");
    connectionManager->RemoveAclLinks("33:11:55:55:66:22", 0);
    ClearRecordAclConnect();
    HILOGI("connection_manager_test_007 end");
}

/*
 * @tc.number:connection_manager_test_008
 * @tc.name: DisconnectForDeviceLimit
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_008, TestSize.Level1)
{
    HILOGI("connection_manager_test_008 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->CheckNeedReportConnectedDevice();
    InitRecordAclConnect();
    connectionManager->CheckNeedReportConnectedDevice();
    ClearRecordAclConnect();
    HILOGI("connection_manager_test_008 end");
}

/*
 * @tc.number:connection_manager_test_009
 * @tc.name: UpdateVirtualAutoConnCap
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_009, TestSize.Level1)
{
    HILOGI("connection_manager_test_009 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->UpdateVirtualAutoConnCap("33:11:55:55:66:22", 1);
    connectionManager->UpdateVirtualAutoConnCap("33:11:55:55:66:22", 4);
    HILOGI("connection_manager_test_009 end");
}

/*
 * @tc.number:connection_manager_test_010
 * @tc.name: OnGetConnReasonEchoRsp
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_010, TestSize.Level1)
{
    HILOGI("connection_manager_test_010 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->OnGetConnReasonEchoRsp("33:11:55:55:66:22", 1, 1);
    connectionManager->OnGetConnReasonEchoRsp("33:11:55:55:66:22", 2, 1);
    connectionManager->OnGetConnReasonEchoRsp("33:11:55:55:66:22", 2, 2);
    HILOGI("connection_manager_test_010 end");
}

/*
 * @tc.number:connection_manager_test_011
 * @tc.name: SetVirtualAutoConnectType
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_011, TestSize.Level1)
{
    HILOGI("connection_manager_test_011 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    const RawAddress device("00:11:22:33:44:55");
    connectionManager->SetVirtualAutoConnectType(device, 1, 0);
    connectionManager->SetVirtualAutoConnectType(device, 2, 3);
    HILOGI("connection_manager_test_011 end");
}

/*
 * @tc.number:connection_manager_test_012
 * @tc.name: RemoveAllAclLinks
 * @tc.desc: This case is placed at the last
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_012, TestSize.Level1)
{
    HILOGI("connection_manager_test_012 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->AddAclLinks("33:11:55:55:66:22", 0, "");
    connectionManager->AddAclLinks("33:11:55:55:66:22", 1, "");
    IAdapterManager::GetInstance()->Start();
    AdapterManager::GetInstance()->CreateAdapters();
    connectionManager->RemoveAllAclLinks();
    EXPECT_TRUE(connectionManager->bleLinks_.Empty());
    EXPECT_TRUE(connectionManager->brLinks_.Empty());
    HILOGI("connection_manager_test_012 end");
}

/*
 * @tc.number:connection_manager_test_013
 * @tc.name: ShouldRefuseSocketConnectEX
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_013, TestSize.Level1)
{
    HILOGI("connection_manager_test_013 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    std::string address = "00:11:22:33:44:55";
    Uuid uuid;
    std::string bundle = "testPackageName";
    bool result = connectionManager->ShouldRefuseSocketConnectEX(address, uuid, bundle);
    EXPECT_FALSE(result);
    bundle = "broker";
    result = connectionManager->ShouldRefuseSocketConnectEX(address, uuid, bundle);
    EXPECT_FALSE(result); 
    HILOGI("connection_manager_test_013 end");
}

/*
 * @tc.number:connection_manager_test_014
 * @tc.name: SetConnectionPriority
 * @tc.desc:
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_014, TestSize.Level1)
{
    HILOGI("connection_manager_test_014 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    InitRecordAclConnect();
    std::vector<std::string> highPriorityLinks = {"11:11:11:11:11:11", "22:22:22:22:22:22", "33:33:33:33:33:33"};
    for (std::string highPriorityLink : highPriorityLinks) {
        connectionManager->SetConnectionPriority(
            highPriorityLink, Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NON_PREEMPTIBLE);
    }
    // repeat set priority for connection 1
    int ret = connectionManager->SetConnectionPriority(
        "11:11:11:11:11:11", Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NON_PREEMPTIBLE);
    EXPECT_EQ(ret, BT_NO_ERROR);

    // connections 1~3 are set priority, 4~5 are unpaired conneciton, 4 is the first unpaired
    std::string unpaired = connectionManager->GetFirstUnPairedBrLink();
    EXPECT_EQ(unpaired, "44:44:44:44:44:44");

    // set priority for 4, connection 1 priority is reset
    ret = connectionManager->SetConnectionPriority(
        "44:44:44:44:44:44", Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NON_PREEMPTIBLE);
    EXPECT_EQ(ret, BT_NO_ERROR);
    EXPECT_FALSE(connectionManager->highPriorityLinks_.IsExist("11:11:11:11:11:11"));
    EXPECT_TRUE(connectionManager->highPriorityLinks_.IsExist("44:44:44:44:44:44"));
    unpaired = connectionManager->GetFirstUnPairedBrLink();
    EXPECT_EQ(unpaired, "11:11:11:11:11:11");

    // reset priority for connection 4
    ret = connectionManager->SetConnectionPriority(
        "44:44:44:44:44:44", Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_DEFAULT);
    EXPECT_EQ(ret, BT_NO_ERROR);
    EXPECT_FALSE(connectionManager->highPriorityLinks_.IsExist("44:44:44:44:44:44"));
    const int invalidPriority = 0XFF;

    // set invalid priority
    ret = connectionManager->SetConnectionPriority("xx:xx:xx:xx:xx:xx", invalidPriority);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    ClearRecordAclConnect();
    HILOGI("connection_manager_test_014 end");
}

/*
 * @tc.number:connection_manager_test_015
 * @tc.name: SendAntennaStatusMsg
 * @tc.desc: This case is placed at the last
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_015, TestSize.Level1)
{
    HILOGI("connection_manager_test_015 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->AddAclLinks("33:11:55:55:66:22", 1, "");
    EXPECT_TRUE(connectionManager->IsBleConnected("33:11:55:55:66:22"));
    connectionManager->SendAntennaStatusMsg(true);
    HILOGI("connection_manager_test_015 end");
}

/*
 * @tc.number:connection_manager_test_016
 * @tc.name: IsBrConnected IsBleConnected
 * @tc.desc: This case is placed at the last
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_016, TestSize.Level1)
{
    HILOGI("connection_manager_test_016 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->AddAclLinks("33:11:55:55:66:22", 0, "");
    connectionManager->AddAclLinks("33:11:55:55:66:22", 1, "");
    EXPECT_TRUE(connectionManager->IsBrConnected("33:11:55:55:66:22"));
    EXPECT_TRUE(connectionManager->IsBleConnected("33:11:55:55:66:22"));
    HILOGI("connection_manager_test_016 end");
}

/*
 * @tc.number:connection_manager_test_017
 * @tc.name: SendBtWearableStatusMsg IsWearableDevice HasWearableBrLinks
 * @tc.desc: This case is placed at the last
 */
HWTEST_F(BluetoothConnectionManagerTest, connection_manager_test_017, TestSize.Level1)
{
    HILOGI("connection_manager_test_017 enter");
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->AddAclLinks("33:11:55:55:66:22", 0, "");
    EXPECT_FALSE(connectionManager->IsWearableDevice("33:11:55:55:66:22"));
    EXPECT_FALSE(connectionManager->HasWearableBrLinks());
    connectionManager->SendBtWearableStatusMsg(true);
    connectionManager->SendBtWearableStatusMsg(false);
    connectionManager->RemoveAclLinks("33:11:55:55:66:22", 0);
    HILOGI("connection_manager_test_017 end");
}
}  // namespace bluetooth
}  // namespace OHOS