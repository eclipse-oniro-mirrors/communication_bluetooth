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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_gatt_characteristic_parcel.h"
#include "bluetooth_host.h"
#include "uuid.h"
#include "bluetooth_log.h"
#include "bluetooth_gatt_client_callback_stub.h"
#include "bluetooth_gatt_client_proxy.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "vector"
#include "profile_service_manager.h"
#include "bluetooth_profile_manager.h"
#include "bt_def.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 2;
class GattClientCallbackTest : public GattClientCallback {
public:
    GattClientCallbackTest()
    {}
    ~GattClientCallbackTest()
    {}

    void OnConnectionStateChanged(int connectionState, int ret) override
    {
        HILOGI("GattClientCallbackTest::OnConnectionStateChanged called");
    }

    void OnConnectionStateChangedWithReason(int connectionState, int ret, int disconnectReason) override
    {
        HILOGI("GattClientCallbackTest::OnConnectionStateChangedWithReason called");
    }

    void OnServicesChanged() override
    {
        HILOGI("GattClientCallbackTest::OnServicesChanged called");
    }

    void OnCharacteristicChanged(const GattCharacteristic &characteristic) override
    {
        HILOGI("GattClientCallbackTest::OnCharacteristicChanged called");
    }

    void OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret) override
    {
        HILOGI("GattClientCallbackTest::OnCharacteristicReadResult called");
    }

    void OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret) override
    {
        HILOGI("GattClientCallbackTest::OnCharacteristicWriteResult called");
    }

    void OnCharacteristicWriteResultWithContext(const GattCharacteristic &characteristic,
        const BluetoothGattRspContext &rspContext, int ret) override
    {
        HILOGI("GattClientCallbackTest::OnCharacteristicWriteResultWithContext called");
    }

    void OnDescriptorReadResult(const GattDescriptor &descriptor, int ret) override
    {
        HILOGI("GattClientCallbackTest::OnDescriptorReadResult called");
    }

    void OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret) override
    {
        HILOGI("GattClientCallbackTest::OnDescriptorWriteResult called");
    }

    void OnMtuUpdate(int mtu, int ret) override
    {
        HILOGI("GattClientCallbackTest::OnMtuUpdate called");
    }

    void OnServicesDiscovered(int status) override
    {
        HILOGI("GattClientCallbackTest::OnServicesDiscovered called");
    }

    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override
    {
        HILOGI("GattClientCallbackTest::OnConnectionParameterChanged called");
    }

    void OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status) override
    {
        HILOGI("GattClientCallbackTest::OnSetNotifyCharacteristic called");
    }

    void OnBlePhyUpdate(int32_t txPhy, int32_t rxPhy, int32_t status) override
    {
        HILOGI("GattClientCallbackTest::OnBlePhyUpdate called");
    }

    void OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status) override
    {
        HILOGI("GattClientCallbackTest::OnBlePhyRead called");
    }

    void OnConnectionStateChangedWithMessage(int connectionState, int ret, GattDisconnectParam disconnectParam) override
    {
        HILOGI("GattClientCallbackTest::OnConnectionStateChangedWithMessage called");
    }
};
class BluetoothGattClientCallbackStubTest : public BluetoothGattClientCallbackStub {
public:
    void OnServicesChanged() override
    {}

    void OnConnectionStateChanged(
        int32_t state, int32_t newState, int32_t disconnectReason, const std::string &reasonMessage) override
    {}

    void OnCharacteristicChanged(const BluetoothGattCharacteristic &characteristic) override
    {}

    void OnCharacteristicRead(int32_t ret, const BluetoothGattCharacteristic &characteristic) override
    {}

    void OnCharacteristicWrite(int32_t ret, const BluetoothGattCharacteristic &characteristic,
        const BluetoothGattRspContext &rspContext) override
    {}

    void OnDescriptorRead(int32_t ret, const BluetoothGattDescriptor &descriptor) override
    {}

    void OnDescriptorWrite(int32_t ret, const BluetoothGattDescriptor &descriptor) override
    {}

    void OnMtuChanged(int32_t state, int32_t mtu) override
    {}

    void OnServicesDiscovered(int32_t status) override
    {}

    void OnConnectionParameterChanged(int32_t interval, int32_t latency, int32_t timeout, int32_t status) override
    {}

    void OnReadRemoteRssiValue(const bluetooth::RawAddress &addr, int32_t rssi, int32_t status) override
    {}

    void OnBlePhyUpdate(int32_t txPhy, int32_t rxPhy, int32_t status) override
    {}

    void OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status) override
    {}

    explicit BluetoothGattClientCallbackStubTest()
    {}
    ~BluetoothGattClientCallbackStubTest() override
    {}

private:
};
static BluetoothHost *host_;
sptr<IBluetoothGattClient> proxy_;
class GattClientTest : public testing::Test {
public:
    BluetoothRemoteDevice device;
    std::shared_ptr<GattClient> client {nullptr};
    sptr<BluetoothGattClientCallbackStubTest> clientCallback_;
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        BluetoothAccessTokenMock::SetNativeTokenInfo();
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
        proxy_ = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp()
    {
        string randomAddr = "11:22:33:44:55:66";
        device.address_ = randomAddr;
        client = std::make_shared<GattClient>(device);
        client->Init();

        clientCallback_ = new BluetoothGattClientCallbackStubTest();
    }
    void TearDown()
    {
        client = nullptr;
    }
};

/*
 * @tc.number: GattClient_UnitTest_Connect
 * @tc.name: Connect
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_Connect start");
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_NO_ERROR);
    sleep(TIME);
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_NO_ERROR);
    sleep(TIME);
    HILOGI("GattClient_UnitTest_Connect end");
}

/*
 * @tc.number: GattClient_UnitTest_Connect_002
 * @tc.name: Connect
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_Connect_002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_Connect start");
    bool isAutoConnect = true;
    int transport = 2;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_ERR_ADDRESS_OR_TRANSPORT_ERROR);
    transport = 1;
    isAutoConnect = true;
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_NO_ERROR);
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_NO_ERROR);
    sleep(TIME);
    HILOGI("GattClient_UnitTest_Connect end");
}

/*
 * @tc.number: GattClient_UnitTest_Connect_003
 * @tc.name: Connect
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_Connect_003, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_Connect_003 start");

    bool isAutoConnect = true;
    int transport = 2;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_ERR_ADDRESS_OR_TRANSPORT_ERROR);
    int connectionState = 1;
    int ret = 0;
    callback_->OnConnectionStateChanged(connectionState, ret);
    int disconnectReason = 0;
    callback_->OnConnectionStateChangedWithReason(connectionState, ret, disconnectReason);
    GattDisconnectParam disconnectParam(disconnectReason, "");
    callback_->OnConnectionStateChangedWithMessage(connectionState, ret, disconnectParam);
    HILOGI("GattClient_UnitTest_Connect_003 end");
}

/*
 * @tc.number: GattClient_UnitTest_Disconnect
 * @tc.name: Disconnect
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_Disconnect, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_Disconnect start");
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    EXPECT_EQ(client->Disconnect(), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_NO_ERROR);
    sleep(TIME);
    EXPECT_EQ(client->Disconnect(), BT_NO_ERROR);
    HILOGI("GattClient_UnitTest_Disconnect end");
}

/*
 * @tc.number: GattClient_UnitTest_DiscoverServices
 * @tc.name: DiscoverServices
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_DiscoverServices, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_DiscoverServices start");
    EXPECT_EQ(client->DiscoverServices(), BT_ERR_GATT_CONNECT_STATE_ERROR);
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_NO_ERROR);
    sleep(TIME);
    EXPECT_EQ(client->DiscoverServices(), BT_ERR_GATT_CONNECT_STATE_ERROR);
    HILOGI("GattClient_UnitTest_DiscoverServices end");
}

/*
 * @tc.number: GattClient_UnitTest_GetService_1
 * @tc.name: GetService_1
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_GetService_1, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_GetService_1 start");
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    client->Connect(callback_, isAutoConnect, transport);
    UUID id = UUID::RandomUUID();
    EXPECT_EQ(client->GetService(id), std::nullopt);
    HILOGI("GattClient_UnitTest_GetService_1 end");
}

/*
 * @tc.number: GattClient_UnitTest_GetService_2
 * @tc.name: GetService_2
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_GetService_2, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_GetService_2 start");
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    client->Connect(callback_, isAutoConnect, transport);
    EXPECT_EQ((int)client->GetService().size(), 0);
    callback_->OnServicesChanged();
    HILOGI("GattClient_UnitTest_GetService_2 end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadCharacteristic
 * @tc.name: ReadCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadCharacteristic, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadCharacteristic start");
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    int result = client->ReadCharacteristic(characteristic);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    client->Connect(callback_, isAutoConnect, transport);
    sleep(TIME);
    result = client->ReadCharacteristic(characteristic);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    callback_->OnCharacteristicReadResult(characteristic, result);
    HILOGI("GattClient_UnitTest_ReadCharacteristic end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadCharacteristic_002
 * @tc.name: ReadCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadCharacteristic_002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadCharacteristic_002 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    EXPECT_EQ(client->ReadCharacteristic(characteristic), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_ReadCharacteristic_002 end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadCharacteristicByUuid_001
 * @tc.name: ReadCharacteristicByUuid
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadCharacteristicByUuid_001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadCharacteristicByUuid_001 start");
    std::string uuid = UUID::RandomUUID().ToString();
    EXPECT_EQ(client->ReadCharacteristicByUuid(uuid, 0x0001, 0xFFFF), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_ReadCharacteristicByUuid_001 end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadDescriptor
 * @tc.name: ReadDescriptor
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadDescriptor, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadDescriptor start");
    BluetoothRemoteDevice device;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    bool isAutoConnect = true;
    int transport = 1;
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    GattDescriptor descriptor = GattDescriptor(uuid_, permissions);
    int result = client->ReadDescriptor(descriptor);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    client->Connect(callback_, isAutoConnect, transport);
    sleep(TIME);
    result = client->ReadDescriptor(descriptor);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    callback_->OnDescriptorReadResult(descriptor, result);
    HILOGI("GattClient_UnitTest_ReadDescriptor end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadDescriptor_002
 * @tc.name: ReadDescriptor
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadDescriptor_002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadDescriptor_002 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    GattDescriptor descriptor = GattDescriptor(uuid_, permissions);
    EXPECT_EQ(client->ReadDescriptor(descriptor), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_ReadDescriptor_002 end");
}

/*
 * @tc.number: GattClient_UnitTest_RequestBleMtuSize
 * @tc.name: RequestBleMtuSize
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_RequestBleMtuSize, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_RequestBleMtuSize start");
    int mtu = 17;
    int result = client->RequestBleMtuSize(mtu);

    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    client->Connect(callback_, isAutoConnect, transport);
    sleep(TIME);
    result = client->RequestBleMtuSize(mtu);
    EXPECT_EQ(result, BT_ERR_INTERNAL_ERROR);
    callback_->OnMtuUpdate(mtu, result);
    HILOGI("GattClient_UnitTest_RequestBleMtuSize end");
}

/*
 * @tc.number: GattClient_UnitTest_RequestBleMtuSize_002
 * @tc.name: RequestBleMtuSize
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_RequestBleMtuSize_002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_RequestBleMtuSize_002 start");
    int mtu = 17;
    EXPECT_EQ(client->RequestBleMtuSize(mtu), BT_ERR_INTERNAL_ERROR);
    HILOGI("GattClient_UnitTest_RequestBleMtuSize_002 end");
}

/*
 * @tc.number: GattClient_UnitTest_SetNotifyCharacteristic
 * @tc.name: SetNotifyCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_SetNotifyCharacteristic, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_SetNotifyCharacteristic start");
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    bool enable = true;
    int result = client->SetNotifyCharacteristic(characteristic, enable);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    client->Connect(callback_, isAutoConnect, transport);
    sleep(TIME);
    result = client->SetNotifyCharacteristic(characteristic, enable);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    callback_->OnSetNotifyCharacteristic(characteristic, result);
    HILOGI("GattClient_UnitTest_SetNotifyCharacteristic end");
}

/*
 * @tc.number: GattClient_UnitTest_SetNotifyCharacteristic_002
 * @tc.name: SetNotifyCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_SetNotifyCharacteristic_002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_SetNotifyCharacteristic_002 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    bool enable = true;
    EXPECT_EQ(client->SetNotifyCharacteristic(characteristic, enable), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_SetNotifyCharacteristic_002 end");
}

/*
 * @tc.number: GattClient_UnitTest_SetNotifyCharacteristicV2_001
 * @tc.name: SetNotifyCharacteristicV2
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_SetNotifyCharacteristicV2_001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_SetNotifyCharacteristicV2_001 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    bool enable = true;
    client->SetNotifyCharacteristicV2(characteristic, enable);
    HILOGI("GattClient_UnitTest_SetNotifyCharacteristicV2_001 end");
}

/*
 * @tc.number: GattClient_UnitTest_WriteCharacteristic_001
 * @tc.name: WriteCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_WriteCharacteristic_001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_WriteCharacteristic_001 start");
    BluetoothRemoteDevice device;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    bool isAutoConnect = true;
    int transport = 1;
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    int result = client->WriteCharacteristic(characteristic);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    client->Connect(callback_, isAutoConnect, transport);
    result = client->WriteCharacteristic(characteristic);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    sleep(TIME);

    callback_->OnCharacteristicChanged(characteristic);
    callback_->OnCharacteristicWriteResult(characteristic, result);
    HILOGI("GattClient_UnitTest_WriteCharacteristic_001 end");
}

/*
 * @tc.number: GattClient_UnitTest_WriteCharacteristic_002
 * @tc.name: WriteCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_WriteCharacteristic_002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_WriteCharacteristic_002 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    EXPECT_EQ(client->WriteCharacteristic(characteristic), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_WriteCharacteristic_002 end");
}

/*
 * @tc.number: GattClient_UnitTest_WriteDescriptor
 * @tc.name: WriteDescriptor
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_WriteDescriptor, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_WriteDescriptor start");
    BluetoothRemoteDevice device;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    bool isAutoConnect = true;
    int transport = 1;
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    GattDescriptor descriptor = GattDescriptor(uuid_, permissions);
    int result = client->WriteDescriptor(descriptor);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    client->Connect(callback_, isAutoConnect, transport);
    result = client->WriteDescriptor(descriptor);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    sleep(TIME);

    callback_->OnDescriptorWriteResult(descriptor, result);
    HILOGI("GattClient_UnitTest_WriteDescriptor end");
}

/*
 * @tc.number: GattClient_UnitTest_WriteDescriptor_002
 * @tc.name: WriteDescriptor
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_WriteDescriptor_002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_WriteDescriptor_002 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    GattDescriptor descriptor = GattDescriptor(uuid_, permissions);
    EXPECT_EQ(client->WriteDescriptor(descriptor), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_WriteDescriptor_002 end");
}

/*
 * @tc.number: GattClient_UnitTest_RequestConnectionPriority
 * @tc.name: RequestConnectionPriority
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_RequestConnectionPriority, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_RequestConnectionPriority start");
    bool isAutoConnect = true;
    int transport = 1;
    int connPriority = 2;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    EXPECT_EQ(client->RequestConnectionPriority(connPriority), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);

    client->Connect(callback_, isAutoConnect, transport);
    sleep(TIME);
    EXPECT_EQ(client->RequestConnectionPriority(connPriority), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);

    connPriority = 0;
    EXPECT_EQ(client->RequestConnectionPriority(connPriority), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);

    connPriority = 1;
    EXPECT_EQ(client->RequestConnectionPriority(connPriority), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);

    connPriority = 2;
    EXPECT_EQ(client->RequestConnectionPriority(connPriority), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);

    connPriority = 3;
    EXPECT_EQ(client->RequestConnectionPriority(connPriority), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);

    callback_->OnConnectionParameterChanged(100, 100, 100, 0);
    HILOGI("GattClient_UnitTest_RequestConnectionPriority end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadRemoteRssiValue
 * @tc.name: ReadRemoteRssiValue
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadRemoteRssiValue, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadRemoteRssiValue start");
    int result = client->ReadRemoteRssiValue();
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    client->Connect(callback_, isAutoConnect, transport);
    sleep(TIME);
    result = client->ReadRemoteRssiValue();
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    callback_->OnReadRemoteRssiValueResult(100, result);
    HILOGI("GattClient_UnitTest_ReadRemoteRssiValue end");
}

/*
 * @tc.number: GattClient_UnitTest_GetConnectedState001
 * @tc.name: GetConnectedState
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_GetConnectedState001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_GetConnectedState001 start");
    int state = -1;
    int ret = client->GetConnectedState(state);
    EXPECT_EQ(ret, BT_NO_ERROR);
    EXPECT_EQ(state, 3);
    HILOGI("GattClient_UnitTest_GetConnectedState001 end");
}

/*
 * @tc.number: GattClient_UnitTest_Set_ReadPhy_001
 * @tc.name: Set/ReadPhy
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_Set_ReadPhy_001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_Set_ReadPhy_001 start");
    int32_t txPhy = 1;
    int32_t rxPhy = 1;
    int32_t phyOptions = 0;
    EXPECT_EQ(client->SetPhy(txPhy, rxPhy, phyOptions), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    EXPECT_EQ(client->ReadPhy(), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    bool isAutoConnect = true;
    int transport = 1;
    client->Connect(callback_, isAutoConnect, transport);
    int32_t status = 1;
    callback_->OnBlePhyUpdate(txPhy, rxPhy, status);
    callback_->OnBlePhyRead(txPhy, rxPhy, status);
    HILOGI("GattClient_UnitTest_Set_ReadPhy_001 end");
}

/*
 * @tc.number: GattClient_UnitTest_SetIndicateCharacteristic001
 * @tc.name: SetIndicateCharacteristic001
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_SetIndicateCharacteristic001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_SetIndicateCharacteristic001 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    EXPECT_EQ(client->SetIndicateCharacteristic(characteristic, false), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_SetIndicateCharacteristic001 end");
}

/*
 * @tc.number: GattClient_UnitTest_RequestFastestConn
 * @tc.name: RequestFastestConn
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_RequestFastestConn, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_RequestFastestConn start");
    EXPECT_EQ(client->RequestFastestConn(), BT_NO_ERROR);
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    client->Connect(callback_, isAutoConnect, transport);
    sleep(TIME);
    EXPECT_EQ(client->RequestFastestConn(), BT_NO_ERROR);
    HILOGI("GattClient_UnitTest_RequestFastestConn end");
}

/*
 * @tc.number: GattClient_UnitTest_WriteCharacteristic_003
 * @tc.name: WriteCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_WriteCharacteristic_003, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_WriteCharacteristic_003 start");
    BluetoothRemoteDevice device;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    bool isAutoConnect = true;
    int transport = 1;
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    int result = client->WriteCharacteristic(characteristic, true); // WriteCharacteristicWithContext
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    client->Connect(callback_, isAutoConnect, transport);
    result = client->WriteCharacteristic(characteristic, true);
    EXPECT_EQ(result, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    sleep(TIME);

    callback_->OnCharacteristicChanged(characteristic);
    BluetoothGattRspContext rspContext;
    callback_->OnCharacteristicWriteResultWithContext(characteristic, rspContext, result);
    HILOGI("GattClient_UnitTest_WriteCharacteristic_003 end");
}

/*
 * @tc.number: GattClient_UnitTest_WriteCharacteristic002
 * @tc.name: WriteCharacteristic002
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_WriteCharacteristic002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_WriteCharacteristic002 start");
    std::vector<uint8_t> value = {0x1f, 0x77};
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    EXPECT_EQ(client->WriteCharacteristic(characteristic, value), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    client->Connect(callback_, isAutoConnect, transport);
    sleep(TIME);
    EXPECT_EQ(client->WriteCharacteristic(characteristic, value), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_WriteCharacteristic002 end");
}

/*
 * @tc.number: GattClient_UnitTest_Close
 * @tc.name: Close
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_Close, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_Close start");
    EXPECT_EQ(client->Close(), BT_NO_ERROR);
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    client->Connect(callback_, isAutoConnect, transport);
    EXPECT_EQ(client->Close(), BT_NO_ERROR);
    HILOGI("GattClient_UnitTest_Close end");
}

/*
 * @tc.number: GattClient_UnitTest_OnRemoteRequest001
 * @tc.name: OnRemoteRequest
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnRemoteRequest001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnRemoteRequest001 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(bluetoothGattClientCallbackStubTest.OnRemoteRequest(code, data, reply, option), 10);
    HILOGI("GattClient_UnitTest_OnRemoteRequest001 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnRemoteRequest003
 * @tc.name: OnRemoteRequest
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnRemoteRequest003, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnRemoteRequest003 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    uint32_t code = 17;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(bluetoothGattClientCallbackStubTest.OnRemoteRequest(code, data, reply, option), 10);
    HILOGI("GattClient_UnitTest_OnRemoteRequest003 end");
}


/*
 * @tc.number: GattClient_UnitTest_OnCharacteristicChangedInner001
 * @tc.name: OnCharacteristicChangedInner
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnCharacteristicChangedInner001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnCharacteristicChangedInner001 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothGattClientCallbackStub::OnCharacteristicChangedInner(
        &bluetoothGattClientCallbackStubTest, data, reply), 1);
    HILOGI("GattClient_UnitTest_OnCharacteristicChangedInner001 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnCharacteristicReadInner001
 * @tc.name: OnCharacteristicReadInner
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnCharacteristicReadInner001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnCharacteristicReadInner001 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothGattClientCallbackStub::OnCharacteristicReadInner(
        &bluetoothGattClientCallbackStubTest, data, reply), 1);
    HILOGI("GattClient_UnitTest_OnCharacteristicReadInner001 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnCharacteristicWriteInner001
 * @tc.name: OnCharacteristicWriteInner
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnCharacteristicWriteInner001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnCharacteristicWriteInner001 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothGattClientCallbackStub::OnCharacteristicWriteInner(
        &bluetoothGattClientCallbackStubTest, data, reply), 1);
    HILOGI("GattClient_UnitTest_OnCharacteristicWriteInner001 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnDescriptorReadInner001
 * @tc.name: OnDescriptorReadInner
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnDescriptorReadInner001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnDescriptorReadInner001 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothGattClientCallbackStub::OnDescriptorReadInner(
        &bluetoothGattClientCallbackStubTest, data, reply), 1);
    HILOGI("GattClient_UnitTest_OnDescriptorReadInner001 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnDescriptorWriteInner001
 * @tc.name: OnDescriptorWriteInner
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnDescriptorWriteInner001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnDescriptorWriteInner001 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothGattClientCallbackStub::OnDescriptorWriteInner(
        &bluetoothGattClientCallbackStubTest, data, reply), 1);
    HILOGI("GattClient_UnitTest_OnDescriptorWriteInner001 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnServicesChangedInner001
 * @tc.name: OnServicesChangedInner
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnServicesChangedInner001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnServicesChangedInner001 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothGattClientCallbackStub::OnServicesChangedInner(
        &bluetoothGattClientCallbackStubTest, data, reply), BT_NO_ERROR);
    HILOGI("GattClient_UnitTest_OnServicesChangedInner001 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnServicesChangedInner002
 * @tc.name: OnServicesChangedInner
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnServicesChangedInner002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnServicesChangedInner002 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothGattClientCallbackStub::OnServicesChangedInner(
        &bluetoothGattClientCallbackStubTest, data, reply), BT_NO_ERROR);
    HILOGI("GattClient_UnitTest_OnServicesChangedInner002 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnRemoteRequest002
 * @tc.name: OnRemoteRequest
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnRemoteRequest002, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnRemoteRequest002 start");
    BluetoothGattClientCallbackStubTest bluetoothGattClientCallbackStubTest;
    uint32_t code = 1;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothGattClientCallbackStubTest::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(bluetoothGattClientCallbackStubTest.OnRemoteRequest(code, data, reply, option), 1);
    HILOGI("GattClient_UnitTest_OnRemoteRequest002 end");
}

/*
 * @tc.number: GattClient_UnitTest_RegisterApplication001
 * @tc.name: RegisterApplication
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_RegisterApplication001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_RegisterApplication001 start");
    sptr<BluetoothGattClientCallbackStubTest> clientCallback = new BluetoothGattClientCallbackStubTest();
    BluetoothRawAddress addr;
    int32_t transport = 1;
    EXPECT_EQ(proxy_->RegisterApplication(clientCallback, addr, transport), BT_ERR_IPC_TRANS_FAILED);
    HILOGI("GattClient_UnitTest_RegisterApplication001 end");
}

/*
 * @tc.number: GattClient_UnitTest_DeregisterApplication001
 * @tc.name: DeregisterApplication
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_DeregisterApplication001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_DeregisterApplication001 start");
    int32_t appId = 1;
    EXPECT_EQ(proxy_->DeregisterApplication(appId), GattStatus::INVALID_PARAMETER);
    HILOGI("GattClient_UnitTest_DeregisterApplication001 end");
}

/*
 * @tc.number: GattClient_UnitTest_Connect001
 * @tc.name: Connect
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_Connect001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_Connect001 start");
    int32_t appId = 1;
    EXPECT_EQ(proxy_->Connect(appId, true), BT_ERR_REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_Connect001 end");
}

/*
 * @tc.number: GattClient_UnitTest_Disconnect001
 * @tc.name: Disconnect
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_Disconnect001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_Disconnect001 start");
    int32_t appId = 1;
    EXPECT_EQ(proxy_->Disconnect(appId), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_Disconnect001 end");
}

/*
 * @tc.number: GattClient_UnitTest_DiscoveryServices001
 * @tc.name: DiscoveryServices
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_DiscoveryServices001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_DiscoveryServices001 start");
    int32_t appId = 1;
    EXPECT_EQ(proxy_->DiscoveryServices(appId), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_DiscoveryServices001 end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadCharacteristic001
 * @tc.name: ReadCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadCharacteristic001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadCharacteristic001 start");
    int32_t appId = 1;
    BluetoothGattCharacteristic characteristic;
    EXPECT_EQ(proxy_->ReadCharacteristic(appId, characteristic), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_ReadCharacteristic001 end");
}

/*
 * @tc.number: GattClient_UnitTest_WriteCharacteristic001
 * @tc.name: WriteCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_WriteCharacteristic001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_WriteCharacteristic001 start");
    int32_t appId = 1;
    BluetoothGattCharacteristic characteristic;
    EXPECT_EQ(proxy_->WriteCharacteristic(appId, &characteristic, false, false), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_WriteCharacteristic001 end");
}

/*
 * @tc.number: GattClient_UnitTest_SignedWriteCharacteristic001
 * @tc.name: SignedWriteCharacteristic
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_SignedWriteCharacteristic001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_SignedWriteCharacteristic001 start");
    int32_t appId = 1;
    BluetoothGattCharacteristic characteristic;
    EXPECT_EQ(proxy_->SignedWriteCharacteristic(appId, &characteristic), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_SignedWriteCharacteristic001 end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadDescriptor001
 * @tc.name: ReadDescriptor
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadDescriptor001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadDescriptor001 start");
    int32_t appId = 1;
    BluetoothGattDescriptor descriptor;
    EXPECT_EQ(proxy_->ReadDescriptor(appId, descriptor), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_ReadDescriptor001 end");
}

/*
 * @tc.number: GattClient_UnitTest_WriteDescriptor001
 * @tc.name: WriteDescriptor
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_WriteDescriptor001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_WriteDescriptor001 start");
    int32_t appId = 1;
    BluetoothGattDescriptor descriptor;
    EXPECT_EQ(proxy_->WriteDescriptor(appId, &descriptor), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_WriteDescriptor001 end");
}

/*
 * @tc.number: GattClient_UnitTest_RequestExchangeMtu001
 * @tc.name: RequestExchangeMtu
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_RequestExchangeMtu001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_RequestExchangeMtu001 start");
    int32_t appId = 1;
    int32_t mtu = 1;
    EXPECT_EQ(proxy_->RequestExchangeMtu(appId, mtu), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_RequestExchangeMtu001 end");
}

/*
 * @tc.number: GattClient_UnitTest_RequestConnectionPriority001
 * @tc.name: RequestConnectionPriority
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_RequestConnectionPriority001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_RequestConnectionPriority001 start");
    int32_t appId = 1;
    int32_t connPriority = 1;
    EXPECT_EQ(proxy_->RequestConnectionPriority(appId, connPriority), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("GattClient_UnitTest_RequestConnectionPriority001 end");
}

/*
 * @tc.number: GattClient_UnitTest_GetServices001
 * @tc.name: GetServices
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_GetServices001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_GetServices001 start");
    int32_t appId = 1;
    std::vector<BluetoothGattService> service;
    EXPECT_EQ(proxy_->GetServices(appId, service), BT_NO_ERROR);
    HILOGI("GattClient_UnitTest_GetServices001 end");
}

/*
 * @tc.number: GattClient_UnitTest_ReadRemoteRssiValue001
 * @tc.name: ReadRemoteRssiValue
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_ReadRemoteRssiValue001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_ReadRemoteRssiValue001 start");
    int32_t appId = 1;
    EXPECT_EQ(proxy_->ReadRemoteRssiValue(appId), GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattClient_UnitTest_ReadRemoteRssiValue001 end");
}

/*
 * @tc.number: GattClient_UnitTest_RequestNotification001
 * @tc.name: RequestNotification
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_RequestNotification001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_RequestNotification001 start");
    int32_t appId = 1;
    EXPECT_EQ(proxy_->RequestNotification(appId, 1, false), BT_ERR_INTERNAL_ERROR);
    HILOGI("GattClient_UnitTest_RequestNotification001 end");
}

/*
 * @tc.number: GattClient_UnitTest_OnConnectionStateChanged001
 * @tc.name: OnServicesChanged
*/
HWTEST_F(GattClientTest, GattClient_UnitTest_OnConnectionStateChanged001, TestSize.Level1)
{
    HILOGI("GattClient_UnitTest_OnConnectionStateChanged001 start");
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    EXPECT_EQ(client->Connect(callback_, isAutoConnect, transport), BT_NO_ERROR);

    clientCallback_->OnServicesChanged();
    clientCallback_->OnConnectionStateChanged(static_cast<int32_t>(BTConnectState::DISCONNECTING),
        static_cast<int32_t>(BTConnectState::DISCONNECTED), static_cast<int32_t>(GattDisconnectReason::CONN_UNKNOWN),
        "");
    clientCallback_->OnConnectionStateChanged(static_cast<int32_t>(BTConnectState::DISCONNECTING),
        static_cast<int32_t>(BTConnectState::CONNECTED), static_cast<int32_t>(GattDisconnectReason::CONN_UNKNOWN),
        "");
    BluetoothGattCharacteristic characteristic;
    clientCallback_->OnCharacteristicChanged(characteristic);
    int32_t ret = GattStatus::GATT_SUCCESS;
    BluetoothGattRspContext rspContext;
    clientCallback_->OnCharacteristicWrite(ret, characteristic, rspContext);
    clientCallback_->OnCharacteristicRead(ret, characteristic);
    BluetoothGattDescriptor descriptor;
    clientCallback_->OnDescriptorWrite(ret, descriptor);
    clientCallback_->OnDescriptorRead(ret, descriptor);
    int32_t mtu = 1;
    clientCallback_->OnMtuChanged(ret, mtu);
    ret = GattStatus::GATT_FAILURE;
    clientCallback_->OnCharacteristicWrite(ret, characteristic, rspContext);
    clientCallback_->OnCharacteristicRead(ret, characteristic);
    clientCallback_->OnDescriptorWrite(ret, descriptor);
    clientCallback_->OnDescriptorRead(ret, descriptor);
    int32_t status = 1;
    clientCallback_->OnServicesDiscovered(status);
    int32_t interval = 1;
    int32_t latency = 1;
    int32_t timeout = 1;
    clientCallback_->OnConnectionParameterChanged(interval, latency, timeout, status);
    BluetoothRawAddress addr;
    int32_t rssi = 1;
    clientCallback_->OnReadRemoteRssiValue(addr, rssi, status);
    HILOGI("GattClient_UnitTest_OnConnectionStateChanged001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS