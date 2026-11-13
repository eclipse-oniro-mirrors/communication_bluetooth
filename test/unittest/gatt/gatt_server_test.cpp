/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_server.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_gatt_server_callback_stub.h"
#include "bluetooth_gatt_server_proxy.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_profile_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
const int TIME = 2;
class BluetoothGattServerCallbackCommon : public GattServerCallback {
public:
    BluetoothGattServerCallbackCommon() = default;
    virtual ~BluetoothGattServerCallbackCommon() = default;
    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state) override
    {}
    void OnConnectionStateUpdateWithReason(
        const BluetoothRemoteDevice &device, int state, int32_t disconnectReason) override
    {}
    void OnServiceAdded(GattService service, int ret) override
    {}
    void OnCharacteristicReadRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override
    {}
    void OnCharacteristicWriteRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override
    {}
    void OnDescriptorReadRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override
    {}
    void OnDescriptorWriteRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override
    {}
    void OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu) override
    {}
    void OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int result) override
    {}
    void OnConnectionParameterChanged(
        const BluetoothRemoteDevice &device, int interval, int latency, int timeout, int status) override
    {}

    void OnBlePhyUpdate(const BluetoothRemoteDevice &device, int32_t txPhy, int32_t rxPhy, int32_t status) override
    {}

    void OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status) override
    {}

    void OnConnectionStateUpdateWithMessage(
        const BluetoothRemoteDevice &device, int state, GattDisconnectParam disconnectParam) override
    {}
private:
};
class BluetoothGattServerCallbackStubTest : public BluetoothGattServerCallbackStub {
public:
    void OnCharacteristicReadRequest(
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic) override
    {}

    void OnCharacteristicWriteRequest(const BluetoothGattDevice &device,
        const BluetoothGattCharacteristic &characteristic, bool needRespones) override
    {}

    void OnDescriptorReadRequest(const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor) override
    {}

    void OnDescriptorWriteRequest(const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor) override
    {}

    void OnNotifyConfirm(
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic, int result) override
    {}

    void OnConnectionStateChanged(const BluetoothGattDevice &device, int32_t ret, int32_t state, int32_t reason,
        const std::string &reasonMessage) override
    {}

    void OnMtuChanged(const BluetoothGattDevice &device, int32_t mtu) override
    {}

    void OnAddService(int32_t ret, const BluetoothGattService &service) override
    {}

    void OnConnectionParameterChanged(
        const BluetoothGattDevice &device, int32_t interval, int32_t latency, int32_t timeout, int32_t status) override
    {}

    void OnBlePhyUpdate(
        const BluetoothGattDevice &device, int32_t txPhy, int32_t rxPhy, int32_t status) override
    {}

    void OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status) override
    {}

    explicit BluetoothGattServerCallbackStubTest()
    {}
    ~BluetoothGattServerCallbackStubTest() override
    {}

private:
};

static std::shared_ptr<BluetoothGattServerCallbackCommon> callback_ =
    std::make_shared<BluetoothGattServerCallbackCommon>();
static BluetoothHost *host_;
class GattServerTest : public testing::Test {
public:
    sptr<IBluetoothGattServer> proxy_  = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    std::shared_ptr<GattServer> server {nullptr};
    std::shared_ptr<BluetoothGattServerCallbackStubTest> serverCallback_;

    static void SetUpTestSuite()
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

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestSuite()
    {
        host_ = nullptr;
    }
    void SetUp()
    {
        server = GattServer::CreateInstance(std::static_pointer_cast<GattServerCallback>(callback_));
    }
    void TearDown()
    {
        server = nullptr;
    }
};

/*
 * @tc.number: GattServer002
 * @tc.name: CancelConnection
 * @tc.desc: Check the CancelConnection interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_CancelConnection, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_CancelConnection start");
    BluetoothRemoteDevice deviceBle_("11:22:33:44:55:66", BT_TRANSPORT_BLE);
    int ret = server->CancelConnection(deviceBle_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_CancelConnection end");
}

/*
 * @tc.number: GattServer003
 * @tc.name: RemoveService
 * @tc.desc: Check the RemoveService interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_RemoveService, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RemoveService start");

    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattService serviceOne(uuidSerPer, 1, 1, GattServiceType::PRIMARY);
    int result = server->AddService(serviceOne);
    EXPECT_EQ(result, BT_NO_ERROR);
    sleep(5);
    std::list<GattService> &list = server->GetServices();
    int ret = server->RemoveGattService(list.back());
    EXPECT_EQ(ret, 0);
    HILOGI("GattServer_ModuleTest_RemoveService end");
}

/*
 * @tc.number: GattServer005
 * @tc.name: ClearServices
 * @tc.desc: Check the ClearServices interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_ClearServices, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ClearServices start");

    std::list<GattService>& list = server->GetServices();
    server->ClearServices();
    list = server->GetServices();
    EXPECT_EQ((int)list.size(), 0);
    HILOGI("GattServer_ModuleTest_ClearServices end");
}

/*
 * @tc.number: GattServer006
 * @tc.name: NotifyCharacteristicChanged
 * @tc.desc: Check the NotifyCharacteristicChanged interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_NotifyCharacteristicChanged, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_Notify start");

    BluetoothRemoteDevice deviceBle_;
    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattCharacteristic* aa = new GattCharacteristic(uuidSerPer, 1, 1);
    int res = server->NotifyCharacteristicChanged(deviceBle_, *aa, false);
    EXPECT_NE(res, BT_NO_ERROR);

    HILOGI("GattServer_ModuleTest_Notify end");
}

/*
 * @tc.number: GattServer007
 * @tc.name: SendResponse
 * @tc.desc: Check the SendResponse interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_SendResponse, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_SendResponse start");
    auto server = GattServer::CreateInstance(std::static_pointer_cast<GattServerCallback>(callback_));

    BluetoothRemoteDevice deviceBle_;
    string valueChrTwo = "2";
    int ret = server->SendResponse(deviceBle_,
        0,
        (int)GattStatus::GATT_SUCCESS,
        1,
        (uint8_t *)valueChrTwo.c_str(),
        valueChrTwo.size());
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("GattServer_ModuleTest_SendResponse end");
}

/*
 * @tc.number: GattServer008
 * @tc.name: GetServices
 * @tc.desc: Check the GetServices interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_GetServices_001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetServices_001 start");
    auto server = GattServer::CreateInstance(std::static_pointer_cast<GattServerCallback>(callback_));

    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34F5");

    GattService serviceOne(uuidSerPer, GattServiceType::PRIMARY);
    int ret = server->AddService(serviceOne);

    EXPECT_EQ(ret, BT_NO_ERROR);
    sleep(TIME);
    std::list<GattService> list = server->GetServices();
    EXPECT_EQ((int)list.size(), 1);

    HILOGI("GattServer_ModuleTest_GetServices_001 end");
}

/*
 * @tc.number: GattServer009
 * @tc.name: GetService
 * @tc.desc: Check the GetService interface.
*/

HWTEST_F(GattServerTest, GattServer_ModuleTest_GetService_001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetService_001 start");

    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34F9");

    GattService serviceOne(uuidSerPer, GattServiceType::SECONDARY);
    int ret = server->AddService(serviceOne);

    EXPECT_EQ(ret, BT_NO_ERROR);
    sleep(5);

    std::optional<GattService> listSecondary = server->GetService(uuidSerPer, false);
    EXPECT_FALSE(listSecondary->IsPrimary());
    server->ClearServices();
    server->GetService(uuidSerPer, false);
    HILOGI("GattServer_ModuleTest_GetService_001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_Connect
 * @tc.name: Connect
 * @tc.desc: Check the Connect interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_Connect, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_Connect start");
    BluetoothRemoteDevice deviceBle_;
    int ret = server->Connect(deviceBle_, true);
    EXPECT_EQ(ret, BT_NO_ERROR);
    int state = 1;
    callback_->OnConnectionStateUpdate(deviceBle_, state);
    int disconnectReason = 1;
    callback_->OnConnectionStateUpdateWithReason(deviceBle_, state, disconnectReason);
    GattDisconnectParam disconnectParam(disconnectReason, "");
    callback_->OnConnectionStateUpdateWithMessage(deviceBle_, state, disconnectParam);
    HILOGI("GattServer_ModuleTest_Connect end");
}

/*
 * @tc.number: GattServer_UnitTest_Set_ReadPhy_001
 * @tc.name: Set/ReadPhy
*/
HWTEST_F(GattServerTest, GattServer_UnitTest_Set_ReadPhy_001, TestSize.Level1)
{
    HILOGI("GattServer_UnitTest_Set_ReadPhy_001 start");
    std::string deviceId ="11:22:33:44:55:66";
    int32_t txPhy = 1;
    int32_t rxPhy = 1;
    int32_t phyOptions = 0;
    EXPECT_EQ(server->SetPhy(deviceId, txPhy, rxPhy, phyOptions), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    EXPECT_EQ(server->ReadPhy(deviceId), BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    BluetoothRemoteDevice deviceBle(deviceId, BT_TRANSPORT_BLE);
    int32_t status = 1;
    callback_->OnBlePhyUpdate(deviceBle, txPhy, rxPhy, status);
    callback_->OnBlePhyRead(txPhy, rxPhy, status);
    HILOGI("GattServer_UnitTest_Set_ReadPhy_001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_GetServices_002
 * @tc.name: GetServices
 * @tc.desc: Check the GetServices interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_GetServices_002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetServices_002 start");
    auto server = GattServer::CreateInstance(std::static_pointer_cast<GattServerCallback>(callback_));

    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34F5");

    GattService serviceOne(uuidSerPer, GattServiceType::PRIMARY);
    int ret = server->AddService(serviceOne);

    EXPECT_EQ(ret, BT_NO_ERROR);
    sleep(TIME);
    std::list<GattService> list;
    ret = server->GetServices(list);
    EXPECT_EQ((int)list.size(), 1);

    HILOGI("GattServer_ModuleTest_GetServices_002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_GetService_002
 * @tc.name: GetService
 * @tc.desc: Check the GetService interface.
*/

HWTEST_F(GattServerTest, GattServer_ModuleTest_GetService_002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetService_002 start");

    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34F9");

    GattService serviceOne(uuidSerPer, GattServiceType::SECONDARY);
    int ret = server->AddService(serviceOne);

    EXPECT_EQ(ret, BT_NO_ERROR);
    sleep(5);

    UUID fakeUuid = UUID::FromString("00000000-0000-0000-0000-000000000000");
    GattService service(fakeUuid, GattServiceType::PRIMARY);

    ret = server->GetService(uuidSerPer, false, service);
    EXPECT_FALSE(service.IsPrimary());
    server->ClearServices();
    ret = server->GetService(uuidSerPer, false, service);
    EXPECT_EQ(ret, BT_ERR_GATT_SERVICE_NOT_FOUND);
    HILOGI("GattServer_ModuleTest_GetService_002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_GetConnectedState
 * @tc.name: GetConnectedState
 * @tc.desc: Check the GetConnectedState interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_GetConnectedState, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetConnectedState start");
    const std::string deviceId = "11:22:33:44:55:66";
    int state = -1;
    int ret = server->GetConnectedState(deviceId, state);
    EXPECT_EQ(ret, BT_NO_ERROR);
    EXPECT_EQ(state, 3);
    HILOGI("GattServer_ModuleTest_GetConnectedState end");
}

/*
 * @tc.number: GattServer_ModuleTest_Close
 * @tc.name: Close
 * @tc.desc: Check the Close interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_Close, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_Close start");

    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34F9");

    GattService serviceOne(uuidSerPer, GattServiceType::SECONDARY);
    int ret = server->AddService(serviceOne);

    EXPECT_EQ(ret, BT_NO_ERROR);
    sleep(5);

    int result = server->Close();
    EXPECT_EQ(result, 0);
    HILOGI("GattServer_ModuleTest_Close end");
}

/*
 * @tc.number: GattServer011
 * @tc.name: OnCharacteristicReadRequestInner
 * @tc.desc: Check the OnCharacteristicReadRequestInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnCharacteristicReadRequestInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequestInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnCharacteristicReadRequestInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequestInner end");
}

/*
 * @tc.number: GattServer012
 * @tc.name: OnConnectionStateChangedInner
 * @tc.desc: Check the OnConnectionStateChangedInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnConnectionStateChangedInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnConnectionStateChangedInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnConnectionStateChangedInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnConnectionStateChangedInner end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnAddServiceInner
 * @tc.name: OnAddServiceInner
 * @tc.desc: Check the OnAddServiceInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnAddServiceInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnAddServiceInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnAddServiceInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnAddServiceInner end");
}

/*
 * @tc.number: GattServer013
 * @tc.name: OnCharacteristicWriteRequestInner
 * @tc.desc: Check the OnCharacteristicWriteRequestInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnCharacteristicWriteRequestInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicWriteRequestInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnCharacteristicWriteRequestInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnCharacteristicWriteRequestInner end");
}

/*
 * @tc.number: GattServer014
 * @tc.name: OnDescriptorReadRequestInner
 * @tc.desc: Check the OnDescriptorReadRequestInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnDescriptorReadRequestInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnDescriptorReadRequestInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnDescriptorReadRequestInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnDescriptorReadRequestInner end");
}

/*
 * @tc.number: GattServer015
 * @tc.name: OnDescriptorWriteRequestInner
 * @tc.desc: Check the OnDescriptorWriteRequestInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnDescriptorWriteRequestInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnDescriptorWriteRequestInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnDescriptorWriteRequestInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnDescriptorWriteRequestInner end");
}


/*
 * @tc.number: GattServer016
 * @tc.name: OnMtuChangedInner
 * @tc.desc: Check the OnMtuChangedInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnMtuChangedInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnMtuChangedInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnMtuChangedInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnMtuChangedInner end");
}

/*
 * @tc.number: GattServer017
 * @tc.name: OnNotifyConfirmInner
 * @tc.desc: Check the OnNotifyConfirmInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnNotifyConfirmInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnNotifyConfirmInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnNotifyConfirmInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnNotifyConfirmInner end");
}

/*
 * @tc.number: GattServer018
 * @tc.name: OnConnectionParameterChangedInner
 * @tc.desc: Check the OnConnectionParameterChangedInner interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnConnectionParameterChangedInner, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnConnectionParameterChangedInner start");
    BluetoothGattServerCallbackStubTest bluetoothGattServerCallbackStubTest;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = BluetoothGattServerCallbackStub::OnConnectionParameterChangedInner(
        &bluetoothGattServerCallbackStubTest, data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("GattServer_ModuleTest_OnConnectionParameterChangedInner end");
}

/*
 * @tc.number: GattServer019
 * @tc.name: CancelConnection
 * @tc.desc: Check the CancelConnection interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_CancelConnection001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_CancelConnection001 start");
    BluetoothGattDevice device;
    int appId = 1;
    int ret = proxy_->CancelConnection(appId, device);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("GattServer_ModuleTest_CancelConnection001 end");
}

/*
 * @tc.number: GattServer020
 * @tc.name: NotifyClient
 * @tc.desc: Check the NotifyClient interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_NotifyClient, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_NotifyClient start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    int ret = proxy_->NotifyClient(device, &characteristic, false);
    EXPECT_EQ(ret, 0);
    HILOGI("GattServer_ModuleTest_NotifyClient end");
}

/*
 * @tc.number: GattServer021
 * @tc.name: RespondCharacteristicRead
 * @tc.desc: Check the RespondCharacteristicRead interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_RespondCharacteristicRead, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondCharacteristicRead start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    int ret = proxy_->RespondCharacteristicRead(device, &characteristic, 1);
    EXPECT_EQ(ret, 0);
    HILOGI("GattServer_ModuleTest_RespondCharacteristicRead end");
}

/*
 * @tc.number: GattServer022
 * @tc.name: RespondCharacteristicWrite
 * @tc.desc: Check the RespondCharacteristicWrite interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_RespondCharacteristicWrite, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondCharacteristicWrite start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    int ret = proxy_->RespondCharacteristicWrite(device, characteristic, 1);
    EXPECT_EQ(ret, 0);
    HILOGI("GattServer_ModuleTest_RespondCharacteristicWrite end");
}

/*
 * @tc.number: GattServer023
 * @tc.name: RespondDescriptorRead
 * @tc.desc: Check the RespondDescriptorRead interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_RespondDescriptorRead, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondDescriptorRead start");
    BluetoothGattDevice device;
    BluetoothGattDescriptor descriptor;
    int ret = proxy_->RespondDescriptorRead(device, &descriptor, 1);
    EXPECT_EQ(ret, 0);
    HILOGI("GattServer_ModuleTest_RespondDescriptorRead end");
}

/*
 * @tc.number: GattServer024
 * @tc.name: RespondDescriptorWrite
 * @tc.desc: Check the RespondDescriptorWrite interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_RespondDescriptorWrite, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondDescriptorWrite start");
    BluetoothGattDevice device;
    BluetoothGattDescriptor descriptor;
    int ret = proxy_->RespondDescriptorWrite(device, descriptor, 1);
    EXPECT_EQ(ret, 0);
    HILOGI("GattServer_ModuleTest_RespondDescriptorWrite end");
}

/*
 * @tc.number: GattServer027
 * @tc.name: OnCharacteristicReadRequest
 * @tc.desc: Check the OnCharacteristicReadRequest interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnCharacteristicReadRequest001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequest001 start");
    serverCallback_ = std::make_shared<BluetoothGattServerCallbackStubTest>();
    EXPECT_TRUE(serverCallback_ != nullptr);
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    serverCallback_->OnCharacteristicReadRequest(device, characteristic);
    bool needRespones = true;
    serverCallback_->OnCharacteristicWriteRequest(device, characteristic, needRespones);
    BluetoothGattDescriptor descriptor;
    serverCallback_->OnDescriptorWriteRequest(device, descriptor);
    serverCallback_->OnDescriptorReadRequest(device, descriptor);
    int result = 1;
    serverCallback_->OnNotifyConfirm(device, characteristic, result);
    int32_t ret = 0;
    int32_t state = 1; // 1 is BTConnectState::CONNECTED
    int32_t reason = 4;
    std::string reasonMessage = "";
    serverCallback_->OnConnectionStateChanged(device, ret, state, reason, reasonMessage);
    int32_t mtu = 1;
    serverCallback_->OnMtuChanged(device, mtu);
    ret = 1; // GattStatus::GATT_FAILURE
    BluetoothGattService service;
    serverCallback_->OnAddService(ret, service);
    GattService gattService(UUID(), GattServiceType::PRIMARY);
    callback_->OnServiceAdded(gattService, ret);
    ret = 0; // 0 is GattStatus::GATT_SUCCESS
    serverCallback_->OnAddService(ret, service);
    callback_->OnServiceAdded(gattService, ret);
    int32_t interval = 1;
    int32_t latency = 1;
    int32_t timeout = 1;
    int32_t status = 1;
    serverCallback_->OnConnectionParameterChanged(device, interval, latency, timeout, status);
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequest001 end");
}

/*
 * @tc.number: GattServer028
 * @tc.name: OnCharacteristicReadRequest
 * @tc.desc: Check the OnCharacteristicReadRequest interface.
*/
HWTEST_F(GattServerTest, GattServer_ModuleTest_OnCharacteristicReadRequest002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequest002 start");
    server = nullptr;
    serverCallback_ = std::make_shared<BluetoothGattServerCallbackStubTest>();
    EXPECT_TRUE(serverCallback_ != nullptr);
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    serverCallback_->OnCharacteristicReadRequest(device, characteristic);
    bool needRespones = true;
    serverCallback_->OnCharacteristicWriteRequest(device, characteristic, needRespones);
    BluetoothGattDescriptor descriptor;
    serverCallback_->OnDescriptorWriteRequest(device, descriptor);
    serverCallback_->OnDescriptorReadRequest(device, descriptor);
    int result = 1;
    serverCallback_->OnNotifyConfirm(device, characteristic, result);
    int32_t ret = 0;
    int32_t state = 3; // 3 is BTConnectState::DISCONNECTED
    int32_t reason = 4;
    std::string reasonMessage = "";
    serverCallback_->OnConnectionStateChanged(device, ret, state, reason, reasonMessage);
    state = 1; // BTConnectState::CONNECTED
    serverCallback_->OnConnectionStateChanged(device, ret, state, reason, reasonMessage);
    int32_t mtu = 1;
    serverCallback_->OnMtuChanged(device, mtu);
    ret = 1; // 0 is GattStatus::GATT_FAILURE
    BluetoothGattService service;
    serverCallback_->OnAddService(ret, service);
    int32_t interval = 1;
    int32_t latency = 1;
    int32_t timeout = 1;
    int32_t status = 1;
    serverCallback_->OnConnectionParameterChanged(device, interval, latency, timeout, status);
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequest002 end");
}
}  // namespace Bluetooth
}  // namespace OHOS