/*
 * Copyright (c) 2012 Huawei Device Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file EXPECT in compliance with the License.
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
#include <thread>

#include "bluetooth_host.h"
#include "bluetooth_host_stub.h"
#include "bluetooth_log.h"
#include "bluetooth_host_server.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_errorcode.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

static sptr<BluetoothHostServer> hostServer;
static BluetoothHost *host_;
const int TIME = 2;

class HostStubTest : public testing::Test {
public:
    HostStubTest()
    {}
    ~HostStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HostStubTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase");

    host_ = &BluetoothHost::GetDefaultHost();

    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBrEnable = %{public}d", isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
    hostServer = BluetoothHostServer::GetInstance();
    HILOGI("SetUpTestCase end");
}

void HostStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
}

void HostStubTest::SetUp()
{
}

void HostStubTest::TearDown()
{
}

/**
 * @tc.number: OnRemoteRequest-00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_OnRemoteRequest_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_OnRemoteRequest_00100 start");

    int ret;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ret = hostServer->OnRemoteRequest((uint32_t)0, data, reply, option);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);

    data.WriteInterfaceToken(BluetoothHostStub::GetDescriptor());
    ret = hostServer->OnRemoteRequest((uint32_t)1000, data, reply, option);
    EXPECT_EQ(ret, 305);

    for (BluetoothHostInterfaceCode code = BluetoothHostInterfaceCode::BT_REGISTER_OBSERVER;
            code <= BluetoothHostInterfaceCode::BT_HOST_BUTT;
            code = (BluetoothHostInterfaceCode)(code+1)) {
        data.WriteInterfaceToken(BluetoothHostStub::GetDescriptor());
        HILOGI("HostStub_ModuleTest_OnRemoteRequest_00100 code:%d", code);
        hostServer->OnRemoteRequest((uint32_t)code, data, reply, option);
    }
    hostServer->GetRandomAddressInner(data, reply);

    HILOGI("HostStub_ModuleTest_OnRemoteRequest_00100 end ret:%d", ret);
}

/**
 * @tc.number: GenerateLocalOobDataInner_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_GenerateLocalOobDataInner_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_GenerateLocalOobDataInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    int32_t transport = 1;
    data.WriteInt32(transport);
    data.WriteRemoteObject(nullptr);

    int32_t ret = hostServer->GenerateLocalOobDataInner(data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("HostStub_ModuleTest_GenerateLocalOobDataInner_00100 end");
}

/**
 * @tc.number: SetRemoteDeviceBatteryInfoInner_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_SetRemoteDeviceBatteryInfoInner_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_SetRemoteDeviceBatteryInfoInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    std::string address = "11:22:33:44:55:66";
    data.WriteString(address);

    BluetoothBatteryInfo batteryInfo;
    data.WriteParcelable(&batteryInfo);

    int32_t ret = hostServer->SetRemoteDeviceBatteryInfoInner(data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("HostStub_ModuleTest_SetRemoteDeviceBatteryInfoInner_00100 end");
}

/**
 * @tc.number: CancelPairingInner_00100
 * @tc.name:
 * @tc.desc: The CANCEL_PAIRING stub writes a bool result into the reply parcel
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_CancelPairingInner_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_CancelPairingInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    int32_t transport = 1;
    std::string address = "11:22:33:44:55:66";
    data.WriteInt32(transport);
    data.WriteString(address);

    int32_t ret = hostServer->CancelPairingInner(data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    bool result = true;
    EXPECT_TRUE(reply.ReadBool(result));
    EXPECT_FALSE(result);
    HILOGI("HostStub_ModuleTest_CancelPairingInner_00100 end, result:%d", result);
}

/**
 * @tc.number: CancelPairingByInitiatorInner_00100
 * @tc.name:
 * @tc.desc: The CANCEL_PAIRING_BY_INITIATOR stub writes an int32 error code into the reply parcel
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_CancelPairingByInitiatorInner_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_CancelPairingByInitiatorInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    int32_t transport = 1;
    std::string address = "11:22:33:44:55:66";
    data.WriteInt32(transport);
    data.WriteString(address);

    int32_t ret = hostServer->CancelPairingByInitiatorInner(data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    int32_t errCode = BT_NO_ERROR;
    EXPECT_TRUE(reply.ReadInt32(errCode));
    EXPECT_TRUE(errCode == BT_ERR_DEVICE_NOT_PAIRING || errCode == BT_ERR_INVALID_STATE);
    HILOGI("HostStub_ModuleTest_CancelPairingByInitiatorInner_00100 end, errCode:%d", errCode);
}

/**
 * @tc.number: CancelPairingPermItem_00100
 * @tc.name:
 * @tc.desc: The CANCEL_PAIRING permission item keeps the original system-hap gate (202 for
 *           third-party applications), api10+ permission set is exactly ACCESS_BLUETOOTH (201 model),
 *           and the api9 permission set keeps empty as before
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_CancelPairingPermItem_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_CancelPairingPermItem_00100 start");

    auto it = BluetoothHostStub::memberFuncMap_.find(
        static_cast<uint32_t>(BluetoothHostInterfaceCode::CANCEL_PAIRING));
    ASSERT_TRUE(it != BluetoothHostStub::memberFuncMap_.end());
    std::shared_ptr<PermissionItem> permItem = it->second.second;
    ASSERT_TRUE(permItem != nullptr);

    EXPECT_TRUE(permItem->SystemHapNeeded());

    std::set<std::string> permsApi10 = permItem->GetPermissions(API_VERSION_10);
    EXPECT_EQ(permsApi10.size(), static_cast<size_t>(1));
    EXPECT_TRUE(permsApi10.find("ohos.permission.ACCESS_BLUETOOTH") != permsApi10.end());

    std::set<std::string> permsApi9 = permItem->GetPermissions(API_VERSION_9);
    EXPECT_TRUE(permsApi9.empty());

    HILOGI("HostStub_ModuleTest_CancelPairingPermItem_00100 end");
}

/**
 * @tc.number: CancelPairingByInitiatorPermItem_00100
 * @tc.name:
 * @tc.desc: The CANCEL_PAIRING_BY_INITIATOR permission item is open to third-party applications:
 *           no system-hap gate (no 202), api10+ permission set is exactly ACCESS_BLUETOOTH (201 model),
 *           and the api9 permission set keeps empty as before
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_CancelPairingByInitiatorPermItem_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_CancelPairingByInitiatorPermItem_00100 start");

    auto it = BluetoothHostStub::memberFuncMap_.find(
        static_cast<uint32_t>(BluetoothHostInterfaceCode::CANCEL_PAIRING_BY_INITIATOR));
    ASSERT_TRUE(it != BluetoothHostStub::memberFuncMap_.end());
    std::shared_ptr<PermissionItem> permItem = it->second.second;
    ASSERT_TRUE(permItem != nullptr);

    EXPECT_FALSE(permItem->SystemHapNeeded());

    std::set<std::string> permsApi10 = permItem->GetPermissions(API_VERSION_10);
    EXPECT_EQ(permsApi10.size(), static_cast<size_t>(1));
    EXPECT_TRUE(permsApi10.find("ohos.permission.ACCESS_BLUETOOTH") != permsApi10.end());

    std::set<std::string> permsApi9 = permItem->GetPermissions(API_VERSION_9);
    EXPECT_TRUE(permsApi9.empty());

    HILOGI("HostStub_ModuleTest_CancelPairingByInitiatorPermItem_00100 end");
}

/**
 * @tc.number: GetVirtualAddressByHashInner_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_GetVirtualAddressByHashInner_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_GetVirtualAddressByHashInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    int hashType = 0;
    data.WriteInt32(hashType);
    std::string hashValue = "11111111111111";
    data.WriteString(hashValue);

    int32_t ret = hostServer->GetVirtualAddressByHashInner(data, reply);
    EXPECT_EQ(ret, BT_ERR_PARAM_NOT_COMPLIANT);
    HILOGI("HostStub_ModuleTest_GetVirtualAddressByHashInner_00100 end");
}


/**
 * @tc.number: IsValidHashValue_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_IsValidHashValue_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_IsValidHashValue_00100 start");
    std::string hashValue = "111";
    EXPECT_FALSE(hostServer->IsValidHashValue(hashValue));
    HILOGI("HostStub_ModuleTest_IsValidHashValue_00100 end");
}

/**
 * @tc.number: IsValidHashAlgorithmType_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_IsValidHashAlgorithmType_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_IsValidHashAlgorithmType_00100 start");
    int hashType = 0;
    EXPECT_TRUE(hostServer->IsValidHashAlgorithmType(hashType));
    hashType = 1;
    EXPECT_FALSE(hostServer->IsValidHashAlgorithmType(hashType));
    HILOGI("HostStub_ModuleTest_IsValidHashAlgorithmType_00100 end");
}

/**
 * @tc.number: VerifyMultiPermissions_00100
 * @tc.name:
 * @tc.desc: Test VerifyMultiPermissions with empty permissions set
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_VerifyMultiPermissions_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_VerifyMultiPermissions_00100 start");

    bool systemHapNeeded = false;
    std::set<std::string> permissions;

    int32_t ret = hostServer->VerifyMultiPermissions(systemHapNeeded, permissions);
    HILOGI("VerifyMultiPermissions ret: %{public}d", ret);
    EXPECT_NE(ret, BT_NO_ERROR);

    HILOGI("HostStub_ModuleTest_VerifyMultiPermissions_00100 end");
}

/**
 * @tc.number: VerifyMultiPermissions_00200
 * @tc.name:
 * @tc.desc: Test VerifyMultiPermissions with valid permission and systemHapNeeded false
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_VerifyMultiPermissions_00200, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_VerifyMultiPermissions_00200 start");

    bool systemHapNeeded = false;
    std::set<std::string> permissions = {"ohos.permission.ACCESS_BLUETOOTH"};

    int32_t ret = hostServer->VerifyMultiPermissions(systemHapNeeded, permissions);
    HILOGI("VerifyMultiPermissions ret: %{public}d", ret);
    EXPECT_NE(ret, BT_NO_ERROR);

    HILOGI("HostStub_ModuleTest_VerifyMultiPermissions_00200 end");
}

/**
 * @tc.number: VerifyMultiPermissions_00300
 * @tc.name:
 * @tc.desc: Test VerifyMultiPermissions with multiple permissions and systemHapNeeded false
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_VerifyMultiPermissions_00300, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_VerifyMultiPermissions_00300 start");

    bool systemHapNeeded = false;
    std::set<std::string> permissions = {"ohos.permission.ACCESS_BLUETOOTH", "ohos.permission.USE_BLUETOOTH"};

    int32_t ret = hostServer->VerifyMultiPermissions(systemHapNeeded, permissions);
    HILOGI("VerifyMultiPermissions ret: %{public}d", ret);
    EXPECT_NE(ret, BT_NO_ERROR);

    HILOGI("HostStub_ModuleTest_VerifyMultiPermissions_00300 end");
}

/**
 * @tc.number: VerifyMultiPermissions_00400
 * @tc.name:
 * @tc.desc: Test VerifyMultiPermissions with invalid permission name
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_VerifyMultiPermissions_00400, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_VerifyMultiPermissions_00400 start");

    bool systemHapNeeded = false;
    std::set<std::string> permissions = {"invalid.permission.name"};

    int32_t ret = hostServer->VerifyMultiPermissions(systemHapNeeded, permissions);
    HILOGI("VerifyMultiPermissions ret: %{public}d", ret);
    EXPECT_TRUE(ret == BT_ERR_PERMISSION_FAILED);

    HILOGI("HostStub_ModuleTest_VerifyMultiPermissions_00400 end");
}

/**
 * @tc.number: GetBrAddressByBleAddressInner_00100
 * @tc.name: GetBrAddressByBleAddressInner
 * @tc.desc: Test GetBrAddressByBleAddressInner with valid BLE address
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_GetBrAddressByBleAddressInner_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_GetBrAddressByBleAddressInner_00100 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(BluetoothHostStub::GetDescriptor());
    data.WriteString("1A:2B:3C:4D:5E:6F");
    int32_t ret = hostServer->GetBrAddressByBleAddressInner(data, reply);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("HostStub_ModuleTest_GetBrAddressByBleAddressInner_00100 end");
}

/**
 * @tc.number: GetBrAddressByBleAddressInner_00200
 * @tc.name: GetBrAddressByBleAddressInner
 * @tc.desc: Test GetBrAddressByBleAddressInner with invalid address
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_GetBrAddressByBleAddressInner_00200, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_GetBrAddressByBleAddressInner_00200 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(BluetoothHostStub::GetDescriptor());
    data.WriteString("INVALID_ADDR");
    int32_t ret = hostServer->GetBrAddressByBleAddressInner(data, reply);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("HostStub_ModuleTest_GetBrAddressByBleAddressInner_00200 end");
}

/**
 * @tc.number: GetBleMacByBrMacInner_00100
 * @tc.name: GetBleMacByBrMacInner
 * @tc.desc: Test GetBleMacByBrMacInner with valid BR address
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_GetBleMacByBrMacInner_00100, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_GetBleMacByBrMacInner_00100 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(BluetoothHostStub::GetDescriptor());
    data.WriteString("1A:2B:3C:4D:5E:6F");
    int32_t ret = hostServer->GetBleMacByBrMacInner(data, reply);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("HostStub_ModuleTest_GetBleMacByBrMacInner_00100 end");
}
 
/**
 * @tc.number: GetBleMacByBrMacInner_00200
 * @tc.name: GetBleMacByBrMacInner
 * @tc.desc: Test GetBleMacByBrMacInner with invalid address
 */
HWTEST_F(HostStubTest, HostStub_ModuleTest_GetBleMacByBrMacInner_00200, TestSize.Level1)
{
    HILOGI("HostStub_ModuleTest_GetBleMacByBrMacInner_00200 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(BluetoothHostStub::GetDescriptor());
    data.WriteString("INVALID_ADDR");
    int32_t ret = hostServer->GetBleMacByBrMacInner(data, reply);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("HostStub_ModuleTest_GetBleMacByBrMacInner_00200 end");
}
}
}