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
#include "ble_adapter.h"
#include <gmock/gmock.h>
#include "bt_chr_app_behavior_reporter.h"
#include "bluetooth_errorcode.h"
#include "adapter_manager.h"
#include "adapter_config.h"
#include "bluetooth_gatt_client_server.h"
#include "bluetooth_gatt_client_callback_stub.h"
#include "bluetooth_gatt_client_callback_proxy.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "class_creator.h"
#include "gatt_client_service.h"
#include "profile_service_manager.h"

using namespace OHOS::bluetooth;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
const BluetoothRawAddress DUMMY_ADDRESS("11:22:33:44:55:66");
const int TIME = 2;
static BluetoothHost *host_;
class BluetoothGattClientCallbackStubTest : public BluetoothGattClientCallbackStub {
public:
    void OnServicesChanged() override
    {}

    void OnConnectionStateChanged(
        int32_t state, int32_t newState, int32_t reason, const std::string &reasonMessage) override
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
class BlueToothGattClientServerTest : public testing::Test {
public:
    BlueToothGattClientServerTest()
    {}
    ~BlueToothGattClientServerTest()
    {}

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    int appId_ = -1;
    sptr<BluetoothGattClientCallbackStubTest> bluetoothGattClientCallbackStubTest =
        new BluetoothGattClientCallbackStubTest();
    sptr<BluetoothGattClientCallbackProxy> proxy_ = new BluetoothGattClientCallbackProxy(
        bluetoothGattClientCallbackStubTest);
    std::unique_ptr<BluetoothGattClientServer> bluetoothGattClientServer {nullptr};
};

static void *GattClientServiceCreate(void)
{
    return new GattClientService();
}

static void *GattClientServiceCreateNull(void)
{
    return nullptr;
}

static void *BleAdapterCreate(void)
{
    return new BleAdapter();
}

void BlueToothGattClientServerTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
    // 测试进程内的 AdapterManager 与设备 SA 不同进程，需显式创建 adapter 并置为开启
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);

    HILOGI("SetUpTestCase end");
}

void BlueToothGattClientServerTest::TearDownTestSuite(void)
{
    host_ = nullptr;
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    if (reporter.reportTimer_ != nullptr) {
        reporter.reportTimer_->Stop();
        reporter.reportTimer_ = nullptr;
    }
    sleep(TIME);
}

void BlueToothGattClientServerTest::SetUp()
{
    bluetoothGattClientServer = std::make_unique<BluetoothGattClientServer>();
    appId_ = bluetoothGattClientServer->RegisterApplication(proxy_, DUMMY_ADDRESS, 0);
}

void BlueToothGattClientServerTest::TearDown()
{
    bluetoothGattClientServer = nullptr;
}

/*
 * @tc.number: GattServer_ModuleTest_DeregisterApplication001
 * @tc.name: DeregisterApplication
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_DeregisterApplication001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_DeregisterApplication001 start");
    int ret = bluetoothGattClientServer->DeregisterApplication(1);
    EXPECT_EQ(ret, bluetooth::GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattServer_ModuleTest_DeregisterApplication001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_Connect001
 * @tc.name: GattServer_ModuleTest_Connect001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_Connect001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_Connect001 start");
    int ret = bluetoothGattClientServer->Connect(1, false);
    EXPECT_EQ(ret, BT_ERR_OBJECT_NULL);
    HILOGI("GattServer_ModuleTest_Connect001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_Disconnect001
 * @tc.name: GattServer_ModuleTest_Disconnect001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_Disconnect001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_Disconnect001 start");
    int ret = bluetoothGattClientServer->Disconnect(1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_Disconnect001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_DiscoveryServices001
 * @tc.name: GattServer_ModuleTest_DiscoveryServices001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_DiscoveryServices001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_DiscoveryServices001 start");
    int ret = bluetoothGattClientServer->DiscoveryServices(1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_DiscoveryServices001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_ReadCharacteristic001
 * @tc.name: GattServer_ModuleTest_ReadCharacteristic001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadCharacteristic001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadCharacteristic001 start");
    BluetoothGattCharacteristic characteristic;
    int ret = bluetoothGattClientServer->ReadCharacteristic(1, characteristic);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_ReadCharacteristic001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_WriteCharacteristic001
 * @tc.name: GattServer_ModuleTest_WriteCharacteristic001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_WriteCharacteristic001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_WriteCharacteristic001 start");
    BluetoothGattCharacteristic characteristic;
    int ret = bluetoothGattClientServer->WriteCharacteristic(1, &characteristic, false, false);
    EXPECT_EQ(ret, BT_ERR_GATT_CLIENT_ERROR);
    HILOGI("GattServer_ModuleTest_WriteCharacteristic001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_WriteCharacteristic002
 * @tc.name: GattServer_ModuleTest_WriteCharacteristic002
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_WriteCharacteristic002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_WriteCharacteristic002 start");
    BluetoothGattCharacteristic characteristic;
    int ret = bluetoothGattClientServer->WriteCharacteristic(1, &characteristic, false, true);
    // the testsuit has IsSystemHap() permission
    EXPECT_EQ(ret, BT_ERR_GATT_CLIENT_ERROR);
    HILOGI("GattServer_ModuleTest_WriteCharacteristic002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_SignedWriteCharacteristic001
 * @tc.name: GattServer_ModuleTest_SignedWriteCharacteristic001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_SignedWriteCharacteristic001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_SignedWriteCharacteristic001 start");
    BluetoothGattCharacteristic characteristic;
    int ret = bluetoothGattClientServer->SignedWriteCharacteristic(1, &characteristic);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_SignedWriteCharacteristic001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_ReadDescriptor001
 * @tc.name: GattServer_ModuleTest_ReadDescriptor001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadDescriptor001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadDescriptor001 start");
    BluetoothGattDescriptor descriptor;
    int ret = bluetoothGattClientServer->ReadDescriptor(1, descriptor);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_ReadDescriptor001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_WriteDescriptor001
 * @tc.name: GattServer_ModuleTest_WriteDescriptor001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_WriteDescriptor001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_WriteDescriptor001 start");
    BluetoothGattDescriptor descriptor;
    int ret = bluetoothGattClientServer->WriteDescriptor(1, &descriptor);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_WriteDescriptor001 end");
}
/*
 * @tc.number: GattServer_ModuleTest_RequestExchangeMtu001
 * @tc.name: GattServer_ModuleTest_RequestExchangeMtu001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RequestExchangeMtu001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RequestExchangeMtu001 start");
    int ret = bluetoothGattClientServer->RequestExchangeMtu(1, 1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RequestExchangeMtu001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_GetAllDevice001
 * @tc.name: GattServer_ModuleTest_GetAllDevice001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_GetAllDevice001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetAllDevice001 start");
    std::vector<BluetoothGattDevice> device;
    bluetoothGattClientServer->GetAllDevice(device);
    HILOGI("GattServer_ModuleTest_GetAllDevice001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RequestConnectionPriority001
 * @tc.name: GattServer_ModuleTest_RequestConnectionPriority001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RequestConnectionPriority001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RequestConnectionPriority001 start");
    int ret = bluetoothGattClientServer->RequestConnectionPriority(1, 1);
    EXPECT_EQ(ret, 0);
    HILOGI("GattServer_ModuleTest_RequestConnectionPriority001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_GetServices001
 * @tc.name: GattServer_ModuleTest_GetServices001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_GetServices001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetServices001 start");
    std::vector<BluetoothGattService> services;
    int ret = bluetoothGattClientServer->GetServices(1, services);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_GetServices001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RequestFastestConn001
 * @tc.name: GattServer_ModuleTest_RequestFastestConn001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RequestFastestConn001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RequestFastestConn001 start");
    BluetoothRawAddress addr;
    int ret = bluetoothGattClientServer->RequestFastestConn(addr);
    EXPECT_EQ(ret, bluetooth::GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("GattServer_ModuleTest_RequestFastestConn001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_ReadRemoteRssiValue001
 * @tc.name: GattServer_ModuleTest_ReadRemoteRssiValue001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadRemoteRssiValue001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadRemoteRssiValue001 start");
    int ret = bluetoothGattClientServer->ReadRemoteRssiValue(1);
    EXPECT_EQ(ret, 0);
    HILOGI("GattServer_ModuleTest_ReadRemoteRssiValue001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_RequestNotification001
 * @tc.name: GattServer_ModuleTest_RequestNotification001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RequestNotification001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadDescriptor001 start");
    int ret = bluetoothGattClientServer->RequestNotification(1, 1, false);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_ReadDescriptor001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_enable_001
 * @tc.name: GattServer_ModuleTest_enable_001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_enable_001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_enable_001 start");

    // Create service
    ClassFactory::RegisterClass(PROFILE_NAME_GATT_CLIENT, GattClientServiceCreate);
    auto manager = ProfileServiceManager::GetInstance();
    string name = PROFILE_NAME_GATT_CLIENT;
    manager->SetProfileService(bluetooth::BTTransport::ADAPTER_BLE, name);

    appId_ = bluetoothGattClientServer->RegisterApplication(proxy_, DUMMY_ADDRESS, 0);
    bluetoothGattClientServer->DeregisterApplication(1);
    bluetoothGattClientServer->Connect(1, false);
    bluetoothGattClientServer->Disconnect(1);
    bluetoothGattClientServer->DiscoveryServices(1);
    bluetoothGattClientServer->RefreshServices(1);

    BluetoothGattCharacteristic characteristic;
    bluetoothGattClientServer->ReadCharacteristic(1, characteristic);
    bluetoothGattClientServer->WriteCharacteristic(1, &characteristic, false, false);
    bluetoothGattClientServer->SignedWriteCharacteristic(1, &characteristic);

    BluetoothGattDescriptor descriptor;
    bluetoothGattClientServer->ReadDescriptor(1, descriptor);

    bluetoothGattClientServer->WriteDescriptor(1, &descriptor);
    bluetoothGattClientServer->RequestExchangeMtu(1, 1);

    std::vector<BluetoothGattDevice> device;
    bluetoothGattClientServer->GetAllDevice(device);
    bluetoothGattClientServer->RequestConnectionPriority(1, 1);

    std::vector<BluetoothGattService> services;
    bluetoothGattClientServer->GetServices(1, services);

    BluetoothRawAddress addr;
    bluetoothGattClientServer->RequestFastestConn(addr);
    bluetoothGattClientServer->ReadRemoteRssiValue(1);
    bluetoothGattClientServer->RequestNotification(1, 1, false);

    ClassFactory::RegisterClass(PROFILE_NAME_GATT_CLIENT, GattClientServiceCreateNull);
    manager->SetProfileService(bluetooth::BTTransport::ADAPTER_BLE, name);

    HILOGI("GattServer_ModuleTest_enable_001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnRemoteRequest001
 * @tc.name: GattServer_ModuleTest_OnRemoteRequest001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnRemoteRequest001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnRemoteRequest001 start");
    uint32_t code = 1;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothGattClientServer::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    bluetoothGattClientServer->OnRemoteRequest(code, data, reply, option);
    HILOGI("GattServer_ModuleTest_OnRemoteRequest001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnRemoteRequest002
 * @tc.name: GattServer_ModuleTest_OnRemoteRequest002
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnRemoteRequest002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnRemoteRequest002 start");
    uint32_t code = 17;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothGattClientServer::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    bluetoothGattClientServer->OnRemoteRequest(code, data, reply, option);
    HILOGI("GattServer_ModuleTest_OnRemoteRequest002 start");
}

/*
 * @tc.number: GattServer_ModuleTest_RegisterApplicationInner001
 * @tc.name: GattServer_ModuleTest_RegisterApplicationInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RegisterApplicationInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RegisterApplicationInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress bluetoothRawAddress;
    data1.WriteParcelable(&bluetoothRawAddress);
    bluetoothGattClientServer->RegisterApplicationInner(data, reply);
    bluetoothGattClientServer->RegisterApplicationInner(data1, reply);
    HILOGI("GattServer_ModuleTest_RegisterApplicationInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_DeregisterApplicationInner001
 * @tc.name: GattServer_ModuleTest_DeregisterApplicationInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_DeregisterApplicationInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_DeregisterApplicationInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->DeregisterApplicationInner(data, reply);
    HILOGI("GattServer_ModuleTest_DeregisterApplicationInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_ConnectInner001
 * @tc.name: GattServer_ModuleTest_ConnectInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ConnectInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ConnectInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->ConnectInner(data, reply);
    HILOGI("GattServer_ModuleTest_ConnectInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_DisconnectInner001
 * @tc.name: GattServer_ModuleTest_DisconnectInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_DisconnectInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_DisconnectInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->DisconnectInner(data, reply);
    HILOGI("GattServer_ModuleTest_DisconnectInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_DiscoveryServicesInner001
 * @tc.name: GattServer_ModuleTest_DiscoveryServicesInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_DiscoveryServicesInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_DiscoveryServicesInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->DiscoveryServicesInner(data, reply);
    HILOGI("GattServer_ModuleTest_DiscoveryServicesInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_ReadCharacteristicInner001
 * @tc.name: GattServer_ModuleTest_ReadCharacteristicInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadCharacteristicInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadCharacteristicInner001 start");
    MessageParcel data;
    BluetoothGattCharacteristic bluetoothGattCharacteristic;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    data1.WriteParcelable(&bluetoothGattCharacteristic);
    MessageParcel reply;
    bluetoothGattClientServer->ReadCharacteristicInner(data, reply);
    bluetoothGattClientServer->ReadCharacteristicInner(data1, reply);
    HILOGI("GattServer_ModuleTest_ReadCharacteristicInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_WriteCharacteristicInner001
 * @tc.name: GattServer_ModuleTest_WriteCharacteristicInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_WriteCharacteristicInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_WriteCharacteristicInner001 start");
    MessageParcel data;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    BluetoothGattCharacteristic bluetoothGattCharacteristic;
    data1.WriteParcelable(&bluetoothGattCharacteristic);
    MessageParcel reply;
    bluetoothGattClientServer->WriteCharacteristicInner(data, reply);
    bluetoothGattClientServer->WriteCharacteristicInner(data1, reply);
    HILOGI("GattServer_ModuleTest_WriteCharacteristicInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_SignedWriteCharacteristicInner001
 * @tc.name: GattServer_ModuleTest_SignedWriteCharacteristicInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_SignedWriteCharacteristicInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_SignedWriteCharacteristicInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    BluetoothGattCharacteristic bluetoothGattCharacteristic;
    data1.WriteParcelable(&bluetoothGattCharacteristic);
    bluetoothGattClientServer->SignedWriteCharacteristicInner(data, reply);
    bluetoothGattClientServer->SignedWriteCharacteristicInner(data1, reply);
    HILOGI("GattServer_ModuleTest_SignedWriteCharacteristicInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_ReadDescriptorInner001
 * @tc.name: GattServer_ModuleTest_ReadDescriptorInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadDescriptorInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadDescriptorInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    BluetoothGattDescriptor bluetoothGattDescriptor;
    data1.WriteParcelable(&bluetoothGattDescriptor);
    bluetoothGattClientServer->ReadDescriptorInner(data, reply);
    bluetoothGattClientServer->ReadDescriptorInner(data1, reply);
    HILOGI("GattServer_ModuleTest_ReadDescriptorInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_WriteDescriptorInner001
 * @tc.name: GattServer_ModuleTest_WriteDescriptorInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_WriteDescriptorInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_WriteDescriptorInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    BluetoothGattDescriptor bluetoothGattDescriptor;
    data1.WriteParcelable(&bluetoothGattDescriptor);
    bluetoothGattClientServer->WriteDescriptorInner(data, reply);
    bluetoothGattClientServer->WriteDescriptorInner(data1, reply);
    HILOGI("GattServer_ModuleTest_WriteDescriptorInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_RequestExchangeMtuInner001
 * @tc.name: GattServer_ModuleTest_RequestExchangeMtuInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RequestExchangeMtuInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RequestExchangeMtuInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->RequestExchangeMtuInner(data, reply);
    HILOGI("GattServer_ModuleTest_RequestExchangeMtuInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_GetAllDeviceInner001
 * @tc.name: GattServer_ModuleTest_GetAllDeviceInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_GetAllDeviceInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetAllDeviceInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->GetAllDeviceInner(data, reply);
    HILOGI("GattServer_ModuleTest_GetAllDeviceInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_RequestConnectionPriorityInner001
 * @tc.name: GattServer_ModuleTest_RequestConnectionPriorityInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RequestConnectionPriorityInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RequestConnectionPriorityInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->RequestConnectionPriorityInner(data, reply);
    HILOGI("GattServer_ModuleTest_RequestConnectionPriorityInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_GetServicesInner001
 * @tc.name: GattServer_ModuleTest_GetServicesInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_GetServicesInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetServicesInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->GetServicesInner(data, reply);
    HILOGI("GattServer_ModuleTest_GetServicesInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_RequestFastestConnInner001
 * @tc.name: GattServer_ModuleTest_RequestFastestConnInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RequestFastestConnInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RequestFastestConnInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress addr;
    data1.WriteParcelable(&addr);
    bluetoothGattClientServer->RequestFastestConnInner(data, reply);
    bluetoothGattClientServer->RequestFastestConnInner(data1, reply);
    HILOGI("GattServer_ModuleTest_RequestFastestConnInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_ReadRemoteRssiValueInner001
 * @tc.name: GattServer_ModuleTest_ReadRemoteRssiValueInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadRemoteRssiValueInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadRemoteRssiValueInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->ReadRemoteRssiValueInner(data, reply);
    HILOGI("GattServer_ModuleTest_ReadRemoteRssiValueInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_RequestNotificationInner001
 * @tc.name: GattServer_ModuleTest_RequestNotificationInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RequestNotificationInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RequestNotificationInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattClientServer->RequestNotificationInner(data, reply);
    HILOGI("GattServer_ModuleTest_RequestNotificationInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnConnectionStateChanged001
 * @tc.name: GattServer_ModuleTest_OnConnectionStateChanged001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnConnectionStateChanged001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnConnectionStateChanged001 start");
    proxy_->OnConnectionStateChanged(1, 1, 1, "");
    HILOGI("GattServer_ModuleTest_OnConnectionStateChanged001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnCharacteristicChanged001
 * @tc.name: GattServer_ModuleTest_OnCharacteristicChanged001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnCharacteristicChanged001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicChanged001 start");
    BluetoothGattCharacteristic characteristic;
    proxy_->OnCharacteristicChanged(characteristic);
    HILOGI("GattServer_ModuleTest_OnCharacteristicChanged001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnCharacteristicRead001
 * @tc.name: GattServer_ModuleTest_OnCharacteristicRead001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnCharacteristicRead001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicChanged001 start");
    BluetoothGattCharacteristic characteristic;
    proxy_->OnCharacteristicRead(1, characteristic);
    HILOGI("GattServer_ModuleTest_OnCharacteristicChanged001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnCharacteristicWrite001
 * @tc.name: GattServer_ModuleTest_OnCharacteristicWrite001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnCharacteristicWrite001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicWrite001 start");
    BluetoothGattCharacteristic characteristic;
    BluetoothGattRspContext rspContext;
    proxy_->OnCharacteristicWrite(1, characteristic, rspContext);
    HILOGI("GattServer_ModuleTest_OnCharacteristicWrite001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnDescriptorRead001
 * @tc.name: GattServer_ModuleTest_OnDescriptorRead001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnDescriptorRead001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnDescriptorRead001 start");
    BluetoothGattDescriptor descriptor;
    proxy_->OnDescriptorRead(1, descriptor);
    HILOGI("GattServer_ModuleTest_OnDescriptorRead001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnDescriptorWrite001
 * @tc.name: GattServer_ModuleTest_OnDescriptorWrite001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnDescriptorWrite001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnDescriptorWrite001 start");
    BluetoothGattDescriptor descriptor;
    proxy_->OnDescriptorWrite(1, descriptor);
    HILOGI("GattServer_ModuleTest_OnDescriptorWrite001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnMtuChanged001
 * @tc.name: GattServer_ModuleTest_OnMtuChanged001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnMtuChanged001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnMtuChanged001 start");
    proxy_->OnMtuChanged(1, 1);
    HILOGI("GattServer_ModuleTest_OnMtuChanged001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnReadRemoteRssiValue001
 * @tc.name: GattServer_ModuleTest_OnReadRemoteRssiValue001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnReadRemoteRssiValue001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnReadRemoteRssiValue001 start");
    bluetooth::RawAddress addr;
    int rssi = 1;
    int status = 1;
    proxy_->OnReadRemoteRssiValue(addr, rssi, status);
    HILOGI("GattServer_ModuleTest_OnReadRemoteRssiValue001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnServicesDiscovered001
 * @tc.name: GattServer_ModuleTest_OnServicesDiscovered001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnServicesDiscovered001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnServicesDiscovered001 start");
    proxy_->OnServicesDiscovered(1);
    HILOGI("GattServer_ModuleTest_OnServicesDiscovered001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnServicesChanged001
 * @tc.name: GattServer_ModuleTest_OnServicesChanged001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnServicesChanged001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnServicesChanged001 start");
    proxy_->OnServicesChanged();
    HILOGI("GattServer_ModuleTest_OnServicesChanged001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_WriteToParcel001
 * @tc.name: GattServer_ModuleTest_WriteToParcel001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_WriteToParcel001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_WriteToParcel001 start");
    BluetoothGattService oldService;
    BluetoothGattService newService = oldService;
    OHOS::Parcel data;
    newService.WriteToParcel(data);
    HILOGI("GattServer_ModuleTest_WriteToParcel001 end");
}

/*
 * @tc.number: BluetoothGattCharacteristicTest_WriteToParcel001
 * @tc.name: BluetoothGattCharacteristicTest_WriteToParcel001
*/
HWTEST_F(BlueToothGattClientServerTest, BluetoothGattCharacteristicTest_WriteToParcel001, TestSize.Level1)
{
    HILOGI("BluetoothGattCharacteristicTest_WriteToParcel001 start");
    BluetoothGattCharacteristic characteristic;
    OHOS::Parcel data;
    characteristic.WriteToParcel(data);
    HILOGI("BluetoothGattCharacteristicTest_WriteToParcel001 end");
}

/*
 * @tc.number: BluetoothGattDescriptor_ModuleTest_WriteToParcel001
 * @tc.name: BluetoothGattDescriptor_ModuleTest_WriteToParcel001
*/
HWTEST_F(BlueToothGattClientServerTest, BluetoothGattDescriptor_ModuleTest_WriteToParcel001, TestSize.Level1)
{
    HILOGI("BluetoothGattDescriptor_ModuleTest_WriteToParcel001 start");
    BluetoothGattDescriptor descriptor;
    OHOS::Parcel data;
    descriptor.WriteToParcel(data);
    HILOGI("BluetoothGattDescriptor_ModuleTest_WriteToParcel001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_GetConnectedState001
 * @tc.name: GattServer_ModuleTest_GetConnectedState001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_GetConnectedState001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetConnectedState start");
    const std::string deviceId = "11:22:33:44:55:66";
    int state = -1;
    int ret = bluetoothGattClientServer->GetConnectedState(deviceId, state);
    EXPECT_EQ(ret, BT_NO_ERROR);
    EXPECT_EQ(state, 3);
    HILOGI("GattServer_ModuleTest_GetConnectedState end");
}

/*
 * @tc.number: GattServer_ModuleTest_BluetoothGattRspContext001
 * @tc.name: GattServer_ModuleTest_BluetoothGattRspContext001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_BluetoothGattRspContext001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_BluetoothGattRspContext001 start");
    BluetoothGattRspContext rspContext;
    EXPECT_EQ(rspContext.GetTimeStamp(), 0);
    rspContext.SetTimeStamp(1000);
    EXPECT_EQ(rspContext.GetTimeStamp(), 1000);
    HILOGI("GattServer_ModuleTest_BluetoothGattRspContext001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_BluetoothGattRspContext002
 * @tc.name: GattServer_ModuleTest_BluetoothGattRspContext002
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_BluetoothGattRspContext002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_BluetoothGattRspContext002 start");
    BluetoothGattRspContext rspContext;
    OHOS::Parcel parcel;
    EXPECT_TRUE(rspContext.WriteToParcel(parcel));
    EXPECT_TRUE(rspContext.ReadFromParcel(parcel));
    HILOGI("GattServer_ModuleTest_BluetoothGattRspContext002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_ReadCharacteristicByUuid001
 * @tc.name: GattServer_ModuleTest_ReadCharacteristicByUuid001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadCharacteristicByUuid001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadCharacteristicByUuid001 start");
    std::string uuid = "01010000-1000-1000-8000-00805F9B34FB";
    int ret = bluetoothGattClientServer->ReadCharacteristicByUuid(1, uuid, 0x0001, 0xffff);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_ReadCharacteristicByUuid001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_ReadCharacteristicByUuidInner001
 * @tc.name: GattServer_ModuleTest_ReadCharacteristicByUuidInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadCharacteristicByUuidInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadCharacteristicByUuidInner001 start");
    MessageParcel data;
    data.WriteInt32(1);
    data.WriteString("01010000-1000-1000-8000-00805F9B34FB");
    data.WriteInt32(0x0001);
    data.WriteInt32(0xFFFF);
    MessageParcel reply;
    EXPECT_EQ(bluetoothGattClientServer->ReadCharacteristicByUuidInner(data, reply), BT_NO_ERROR);
    HILOGI("GattServer_ModuleTest_ReadCharacteristicByUuidInner001 start");
}

/*
 * @tc.number: GattServer_ModuleTest_OnBlePhyUpdate001
 * @tc.name: GattServer_ModuleTest_OnBlePhyUpdate001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnBlePhyUpdate001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnBlePhyUpdate001 start");
    proxy_->OnBlePhyUpdate(1, 1, 0);
    EXPECT_TRUE(proxy_ != nullptr);
    HILOGI("GattServer_ModuleTest_OnBlePhyUpdate001 end");
}
 
/*
 * @tc.number: GattServer_ModuleTest_OnBlePhyRead001
 * @tc.name: GattServer_ModuleTest_OnBlePhyRead001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_OnBlePhyRead001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnBlePhyRead001 start");
    proxy_->OnBlePhyRead(1, 1, 0);
    EXPECT_TRUE(proxy_ != nullptr);
    HILOGI("GattServer_ModuleTest_OnBlePhyRead001 end");
}
 
/*
 * @tc.number: GattServer_ModuleTest_SetPhy001
 * @tc.name: GattServer_ModuleTest_SetPhy001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_SetPhy001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_SetPhy001 start");
    int ret = bluetoothGattClientServer->SetPhy(1, 1, 1, 0);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_SetPhy001 end");
}
 
/*
 * @tc.number: GattServer_ModuleTest_ReadPhy001
 * @tc.name: GattServer_ModuleTest_ReadPhy001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_ReadPhy001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadPhy001 start");
    int ret = bluetoothGattClientServer->ReadPhy(1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_ReadPhy001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RefreshServices001
 * @tc.name: GattServer_ModuleTest_RefreshServices001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RefreshServices001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RefreshServices001 start");
    int ret = bluetoothGattClientServer->RefreshServices(1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RefreshServices001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RefreshServices002
 * @tc.name: GattServer_ModuleTest_RefreshServices002
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RefreshServices002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RefreshServices002 start");
    int ret = bluetoothGattClientServer->RefreshServices(appId_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RefreshServices002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RefreshServicesInner001
 * @tc.name: GattServer_ModuleTest_RefreshServicesInner001
*/
HWTEST_F(BlueToothGattClientServerTest, GattServer_ModuleTest_RefreshServicesInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RefreshServicesInner001 start");
    MessageParcel data;
    data.WriteInt32(0x01);
    MessageParcel reply;
    EXPECT_EQ(bluetoothGattClientServer->RefreshServicesInner(data, reply), BT_NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RefreshServicesInner001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
