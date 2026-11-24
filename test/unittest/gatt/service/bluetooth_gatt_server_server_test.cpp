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
#include "class_creator.h"
#include "ble_adapter.h"
#include <gmock/gmock.h>

#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "adapter_manager.h"
#include "adapter_config.h"
#include "bluetooth_gatt_server_server.cpp"
#include "bluetooth_gatt_server_callback_stub.h"
#include "bluetooth_gatt_server_callback_proxy.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_device_manager.h"
#include "btcommon/timer_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
const BluetoothRawAddress DUMMY_ADDRESS("11:22:33:44:55:66");
const int TIME = 2;
static BluetoothHost *host_;

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

class BluetoothGattServerCallbackMock : public IBluetoothGattServerCallback {
public:
    MOCK_METHOD(void, OnAddService, (int32_t ret, const BluetoothGattService &service), (override));
    MOCK_METHOD(void, OnCharacteristicReadRequest, (
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic), (override));
    MOCK_METHOD(void, OnCharacteristicWriteRequest, (const BluetoothGattDevice &device,
    const BluetoothGattCharacteristic &characteristic, bool needRespones), (override));
    MOCK_METHOD(void, OnDescriptorReadRequest, (
        const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor), (override));
    MOCK_METHOD(void, OnConnectionStateChanged, (const BluetoothGattDevice &device, int32_t ret, int32_t state,
        int32_t reason, const std::string &reasonMessage), (override));
    MOCK_METHOD(void, OnMtuChanged, (const BluetoothGattDevice &device, int32_t mtu), (override));
    MOCK_METHOD(void, OnDescriptorWriteRequest, (
        const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor), (override));
    MOCK_METHOD(void, OnConnectionParameterChanged, (const BluetoothGattDevice &device,
        int32_t interval, int32_t latency, int32_t timeout, int32_t status), (override));
    MOCK_METHOD(void, OnNotifyConfirm, (
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic, int result), (override));
    MOCK_METHOD(void, OnBlePhyUpdate, (
        const BluetoothGattDevice &device, int32_t txPhy, int32_t rxPhy, int32_t status), (override));
    MOCK_METHOD(void, OnBlePhyRead, (int32_t txPhy, int32_t rxPhy, int32_t status), (override));
    sptr<OHOS::IRemoteObject> AsObject() override
    {
        sptr<BluetoothGattServerCallbackStubTest> bluetoothGattServerCallbackStubTest =
        new BluetoothGattServerCallbackStubTest();
        return bluetoothGattServerCallbackStubTest;
    }
    ~BluetoothGattServerCallbackMock() override = default;
};

// Fake GattServerService to observe the appId/pid passed by the death cleanup path.
class FakeProfileGattServer : public bluetooth::IProfileGattServer {
public:
    int clearServicesCalls_ = 0;
    int clearServicesAppId_ = -1;
    int32_t clearServicesPid_ = -1;
    int deregisterCalls_ = 0;
    int deregisterAppId_ = -1;
    int32_t deregisterPid_ = -1;

    int ClearServices(int appId, int32_t pid) override
    {
        clearServicesCalls_++;
        clearServicesAppId_ = appId;
        clearServicesPid_ = pid;
        return bluetooth::GattStatus::GATT_SUCCESS;
    }

    int DeregisterApplication(int appId, int32_t pid) override
    {
        deregisterCalls_++;
        deregisterAppId_ = appId;
        deregisterPid_ = pid;
        return bluetooth::GattStatus::GATT_SUCCESS;
    }

    int RegisterApplication(std::weak_ptr<bluetooth::IGattServerCallback> callback) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int DeregisterApplication(int appId) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int ClearServices(int appId) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int AddService(int appId, bluetooth::Service &service) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int RemoveService(int appId, const bluetooth::Service &service) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int NotifyClient(const bluetooth::GattDevice &device, bluetooth::Characteristic &characteristic,
        bool needConfirm) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int RespondCharacteristicRead(const bluetooth::GattDevice &device, bluetooth::Characteristic &characteristic,
        int ret) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int RespondCharacteristicReadByUuid(const bluetooth::GattDevice &device, bluetooth::Characteristic &characteristic,
        int ret) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int RespondCharacteristicWrite(const bluetooth::GattDevice &device, const bluetooth::Characteristic &characteristic,
        int ret) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int RespondDescriptorRead(const bluetooth::GattDevice &device, bluetooth::Descriptor &descriptor,
        int ret) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int RespondDescriptorWrite(const bluetooth::GattDevice &device, const bluetooth::Descriptor &descriptor,
        int ret) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int Connect(int appId, const bluetooth::GattDevice &device, bool isDirect) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int CancelConnection(int appId, const bluetooth::GattDevice &device) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int GetConnectedState(const bluetooth::RawAddress &address) override
    {
        return 0;
    }
    int SetPhy(int appId, const bluetooth::RawAddress &address, int txPhy, int rxPhy, int phyOptions) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int ReadPhy(int appId, const bluetooth::RawAddress &address) override
    {
        return bluetooth::GattStatus::GATT_SUCCESS;
    }
    int Connect(const bluetooth::RawAddress &device) override
    {
        return 0;
    }
    int Disconnect(const bluetooth::RawAddress &device) override
    {
        return 0;
    }
    std::list<bluetooth::RawAddress> GetConnectDevices() override
    {
        return {};
    }
    int GetConnectState() override
    {
        return 0;
    }
    int GetMaxConnectNum() override
    {
        return 0;
    }
    utility::Context *GetContext() override
    {
        return nullptr;
    }
};
class BlueToothGattServerServerTest : public testing::Test {
public:
    BlueToothGattServerServerTest()
    {}
    ~BlueToothGattServerServerTest()
    {}

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    int appId_ = -1;
    sptr<BluetoothGattServerCallbackMock> callback_ {nullptr};
    sptr<BluetoothGattServerCallbackStubTest> bluetoothGattServerCallbackStubTest =
        new BluetoothGattServerCallbackStubTest();
    BluetoothGattServerCallbackProxy *proxy_  = new BluetoothGattServerCallbackProxy(
        bluetoothGattServerCallbackStubTest);
    std::unique_ptr<BluetoothGattServerServer> bluetoothGattServerServer {nullptr};
    std::unique_ptr<BluetoothGattServerServer::impl::GattServerCallbackImpl> gattServerCallbackImpl {nullptr};
};

static void *BleAdapterCreate(void)
{
    return new BleAdapter();
}

void BlueToothGattServerServerTest::SetUpTestSuite(void)
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
    // 测试进程内的 AdapterManager 与设备 SA 不同进程，需显式创建 adapter 并置为开启
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);

    HILOGI("SetUpTestCase end");
}

void BlueToothGattServerServerTest::TearDownTestSuite(void)
{
    sleep(TIME);
    host_ = nullptr;
    utility::CloseAllTimer();
}

void BlueToothGattServerServerTest::SetUp()
{
    bluetoothGattServerServer = std::make_unique<BluetoothGattServerServer>();
    callback_ = new BluetoothGattServerCallbackMock();
    BluetoothGattServerServer bluetoothGattServerServer1;
    gattServerCallbackImpl = std::make_unique<BluetoothGattServerServer::impl::
        GattServerCallbackImpl>(callback_, bluetoothGattServerServer1);
    appId_ = bluetoothGattServerServer->RegisterApplication(callback_);
}

void BlueToothGattServerServerTest::TearDown()
{
    sleep(1);
    bluetoothGattServerServer = nullptr;
    callback_ = nullptr;
    gattServerCallbackImpl = nullptr;
}

/*
 * @tc.number: GattServer_ModuleTest_DeregisterApplication001
 * @tc.name: DeregisterApplication
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_DeregisterApplication001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_DeregisterApplication001 start");
    int ret = bluetoothGattServerServer->DeregisterApplication(1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_DeregisterApplication001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnRemoteDied001
 * @tc.name: OnRemoteDied cleans up with the pid recorded at registration
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnRemoteDied001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnRemoteDied001 start");
    constexpr int32_t deadAppId = 1;
    constexpr int32_t deadPid = 4321;
    FakeProfileGattServer fakeService;
    bluetooth::IProfileGattServer *originService = bluetoothGattServerServer->pimpl->serverService_;
    bluetoothGattServerServer->pimpl->serverService_ = &fakeService;

    // Simulate a client process registered deadAppId and then died.
    sptr<BluetoothGattServerCallbackStubTest> deadCallback = new BluetoothGattServerCallbackStubTest();
    auto deadImpl = std::make_shared<BluetoothGattServerServer::impl::GattServerCallbackImpl>(
        deadCallback, *bluetoothGattServerServer);
    deadImpl->SetAppId(deadAppId);
    deadImpl->SetPid(deadPid);
    bluetoothGattServerServer->pimpl->callbacks_.push_back(deadImpl);
    size_t sizeBefore = bluetoothGattServerServer->pimpl->callbacks_.size();

    // Trigger the death notification.
    deadImpl->deathRecipient_->OnRemoteDied(deadCallback->AsObject());

    // The dead client's callback is removed and the cleanup is invoked with its recorded pid.
    EXPECT_EQ(bluetoothGattServerServer->pimpl->callbacks_.size(), sizeBefore - 1);
    EXPECT_EQ(fakeService.clearServicesCalls_, 1);
    EXPECT_EQ(fakeService.clearServicesAppId_, deadAppId);
    EXPECT_EQ(fakeService.clearServicesPid_, deadPid);
    EXPECT_EQ(fakeService.deregisterCalls_, 1);
    EXPECT_EQ(fakeService.deregisterAppId_, deadAppId);
    EXPECT_EQ(fakeService.deregisterPid_, deadPid);

    bluetoothGattServerServer->pimpl->serverService_ = originService;
    HILOGI("GattServer_ModuleTest_OnRemoteDied001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnRemoteDied002
 * @tc.name: OnRemoteDied with an unmatched remote object does not clean up
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnRemoteDied002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnRemoteDied002 start");
    FakeProfileGattServer fakeService;
    bluetooth::IProfileGattServer *originService = bluetoothGattServerServer->pimpl->serverService_;
    bluetoothGattServerServer->pimpl->serverService_ = &fakeService;

    sptr<BluetoothGattServerCallbackStubTest> aliveCallback = new BluetoothGattServerCallbackStubTest();
    auto aliveImpl = std::make_shared<BluetoothGattServerServer::impl::GattServerCallbackImpl>(
        aliveCallback, *bluetoothGattServerServer);
    aliveImpl->SetAppId(2);
    aliveImpl->SetPid(2);
    bluetoothGattServerServer->pimpl->callbacks_.push_back(aliveImpl);
    size_t sizeBefore = bluetoothGattServerServer->pimpl->callbacks_.size();

    // The died remote object does not match any registered callback.
    sptr<BluetoothGattServerCallbackStubTest> otherCallback = new BluetoothGattServerCallbackStubTest();
    aliveImpl->deathRecipient_->OnRemoteDied(otherCallback->AsObject());

    EXPECT_EQ(bluetoothGattServerServer->pimpl->callbacks_.size(), sizeBefore);
    EXPECT_EQ(fakeService.clearServicesCalls_, 0);
    EXPECT_EQ(fakeService.deregisterCalls_, 0);

    bluetoothGattServerServer->pimpl->serverService_ = originService;
    HILOGI("GattServer_ModuleTest_OnRemoteDied002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RespondDescriptorWrite001
 * @tc.name: GattServer_ModuleTest_RespondDescriptorWrite001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RespondDescriptorWrite001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondDescriptorWrite001 start");
    BluetoothGattDevice device;
    BluetoothGattDescriptor descriptor;
    int32_t res = 1;
    int ret = bluetoothGattServerServer->RespondDescriptorWrite(device, descriptor, res);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RespondDescriptorWrite001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RespondDescriptorRead001
 * @tc.name: GattServer_ModuleTest_RespondDescriptorRead001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RespondDescriptorRead001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondDescriptorRead001 start");
    BluetoothGattDevice device;
    BluetoothGattDescriptor descriptor;
    int32_t res = 1;
    int ret = bluetoothGattServerServer->RespondDescriptorRead(device, &descriptor, res);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RespondDescriptorRead001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RespondCharacteristicWrite001
 * @tc.name: GattServer_ModuleTest_RespondCharacteristicWrite001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RespondCharacteristicWrite001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondCharacteristicWrite001 start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    int32_t res = 1;
    int ret = bluetoothGattServerServer->RespondCharacteristicWrite(device, characteristic, res);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RespondCharacteristicWrite001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RespondCharacteristicRead001
 * @tc.name: GattServer_ModuleTest_RespondCharacteristicRead001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RespondCharacteristicRead001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondCharacteristicRead001 start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    int32_t res = 1;
    int ret = bluetoothGattServerServer->RespondCharacteristicRead(device, &characteristic, res);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RespondCharacteristicRead001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RemoveService001
 * @tc.name: GattServer_ModuleTest_RemoveService001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RemoveService001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RemoveService001 start");
    BluetoothGattService services;
    int ret = bluetoothGattServerServer->RemoveService(1, services);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_RemoveService001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_NotifyClient001
 * @tc.name: GattServer_ModuleTest_NotifyClient001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_NotifyClient001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_NotifyClient001 start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    int ret = bluetoothGattServerServer->NotifyClient(device, &characteristic, false);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_NotifyClient001 end");
}

/*
 * @tc.number: GattServer_Connect001
 * @tc.name: GattServer_Connect001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_Connect001, TestSize.Level1)
{
    HILOGI("GattServer_Connect001 start");
    BluetoothGattDevice device;
    int ret = bluetoothGattServerServer->Connect(0, device, true);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("GattServer_Connect001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_CancelConnection001
 * @tc.name: GattServer_ModuleTest_CancelConnection001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_CancelConnection001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_CancelConnection001 start");
    BluetoothGattDevice device;
    bluetoothGattServerServer->CancelConnection(0, device);
    HILOGI("GattServer_ModuleTest_CancelConnection001 end");
}
/*
 * @tc.number: GattServer_ModuleTest_ClearServices001
 * @tc.name: GattServer_ModuleTest_ClearServices001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_ClearServices001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ClearServices001 start");
    bluetoothGattServerServer->ClearServices(1);
    HILOGI("GattServer_ModuleTest_ClearServices001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_AddService001
 * @tc.name: GattServer_ModuleTest_AddService001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_AddService001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_AddService001 start");
    BluetoothGattService services;
    bluetoothGattServerServer->AddService(1, &services);
    HILOGI("GattServer_ModuleTest_AddService001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnRemoteRequest001
 * @tc.name: GattServer_ModuleTest_OnRemoteRequest001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnRemoteRequest001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnRemoteRequest001 start");
    uint32_t code = 1;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothGattServerServer::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    bluetoothGattServerServer->OnRemoteRequest(code, data, reply, option);
    HILOGI("GattServer_ModuleTest_OnRemoteRequest001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnRemoteRequest002
 * @tc.name: GattServer_ModuleTest_OnRemoteRequest002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnRemoteRequest002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnRemoteRequest002 start");
    uint32_t code = 17;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothGattServerServer::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    bluetoothGattServerServer->OnRemoteRequest(code, data, reply, option);
    HILOGI("GattServer_ModuleTest_OnRemoteRequest002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RegisterApplicationInner001
 * @tc.name: GattServer_ModuleTest_RegisterApplicationInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RegisterApplicationInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RegisterApplicationInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteParcelable(&bluetoothGattService);
    bluetoothGattServerServer->RegisterApplicationInner(data, reply);
    bluetoothGattServerServer->RegisterApplicationInner(data1, reply);
    HILOGI("GattServer_ModuleTest_RegisterApplicationInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_DeregisterApplicationInner001
 * @tc.name: GattServer_ModuleTest_DeregisterApplicationInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_DeregisterApplicationInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_DeregisterApplicationInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothGattServerServer->DeregisterApplicationInner(data, reply);
    HILOGI("GattServer_ModuleTest_DeregisterApplicationInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_AddServiceInner001
 * @tc.name: GattServer_ModuleTest_AddServiceInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_AddServiceInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_AddServiceInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService device;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    data1.WriteParcelable(&device);
    bluetoothGattServerServer->AddServiceInner(data, reply);
    bluetoothGattServerServer->AddServiceInner(data1, reply);
    HILOGI("GattServer_ModuleTest_AddServiceInner001 end");
}

/*
 * @tc.number: GattServer_ConnectInner001
 * @tc.name: GattServer_ConnectInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ConnectInner001, TestSize.Level1)
{
    HILOGI("GattServer_ConnectInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    data1.WriteParcelable(&bluetoothGattService);
    data1.WriteBool(true);
    bluetoothGattServerServer->ConnectInner(data, reply);
    int ret = bluetoothGattServerServer->ConnectInner(data1, reply);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("GattServer_ConnectInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_CancelConnectionInner001
 * @tc.name: GattServer_ModuleTest_CancelConnectionInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_CancelConnectionInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_CancelConnectionInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteParcelable(&bluetoothGattService);
    bluetoothGattServerServer->CancelConnectionInner(data, reply);
    bluetoothGattServerServer->CancelConnectionInner(data1, reply);
    HILOGI("GattServer_ModuleTest_CancelConnectionInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_NotifyClientInner001
 * @tc.name: GattServer_ModuleTest_NotifyClientInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_NotifyClientInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_NotifyClientInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteParcelable(&bluetoothGattService);
    MessageParcel data2;
    BluetoothGattCharacteristic bluetoothGattCharacteristic;
    data2.WriteParcelable(&bluetoothGattService);
    data2.WriteParcelable(&bluetoothGattCharacteristic);
    bluetoothGattServerServer->NotifyClientInner(data, reply);
    bluetoothGattServerServer->NotifyClientInner(data1, reply);
    bluetoothGattServerServer->NotifyClientInner(data2, reply);
    HILOGI("GattServer_ModuleTest_NotifyClientInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RemoveServiceInner001
 * @tc.name: GattServer_ModuleTest_RemoveServiceInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RemoveServiceInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RemoveServiceInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    data1.WriteParcelable(&bluetoothGattService);
    bluetoothGattServerServer->RemoveServiceInner(data, reply);
    bluetoothGattServerServer->RemoveServiceInner(data1, reply);
    HILOGI("GattServer_ModuleTest_RemoveServiceInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RespondCharacteristicReadInner001
 * @tc.name: GattServer_ModuleTest_RespondCharacteristicReadInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RespondCharacteristicReadInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondCharacteristicReadInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteParcelable(&bluetoothGattService);
    MessageParcel data2;
    BluetoothGattCharacteristic bluetoothGattCharacteristic;
    data2.WriteParcelable(&bluetoothGattService);
    data2.WriteParcelable(&bluetoothGattCharacteristic);
    bluetoothGattServerServer->RespondCharacteristicReadInner(data, reply);
    bluetoothGattServerServer->RespondCharacteristicReadInner(data1, reply);
    bluetoothGattServerServer->RespondCharacteristicReadInner(data2, reply);
    HILOGI("GattServer_ModuleTest_RespondCharacteristicReadInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RespondCharacteristicWriteInner001
 * @tc.name: GattServer_ModuleTest_RespondCharacteristicWriteInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RespondCharacteristicWriteInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondCharacteristicWriteInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteParcelable(&bluetoothGattService);
    MessageParcel data2;
    BluetoothGattCharacteristic bluetoothGattCharacteristic;
    data2.WriteParcelable(&bluetoothGattService);
    data2.WriteParcelable(&bluetoothGattCharacteristic);
    bluetoothGattServerServer->RespondCharacteristicWriteInner(data, reply);
    bluetoothGattServerServer->RespondCharacteristicWriteInner(data1, reply);
    bluetoothGattServerServer->RespondCharacteristicWriteInner(data2, reply);
    HILOGI("GattServer_ModuleTest_RespondCharacteristicWriteInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RespondDescriptorReadInner001
 * @tc.name: GattServer_ModuleTest_RespondDescriptorReadInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RespondDescriptorReadInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondDescriptorReadInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteParcelable(&bluetoothGattService);
    MessageParcel data2;
    BluetoothGattDescriptor bluetoothGattDescriptor;
    data2.WriteParcelable(&bluetoothGattService);
    data2.WriteParcelable(&bluetoothGattDescriptor);
    bluetoothGattServerServer->RespondDescriptorReadInner(data, reply);
    bluetoothGattServerServer->RespondDescriptorReadInner(data1, reply);
    bluetoothGattServerServer->RespondDescriptorReadInner(data2, reply);
    HILOGI("GattServer_ModuleTest_RespondDescriptorReadInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_RespondDescriptorWriteInner001
 * @tc.name: GattServer_ModuleTest_RespondDescriptorWriteInner001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_RespondDescriptorWriteInner001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_RespondDescriptorWriteInner001 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothGattService bluetoothGattService;
    MessageParcel data1;
    data1.WriteParcelable(&bluetoothGattService);
    MessageParcel data2;
    BluetoothGattDescriptor bluetoothGattDescriptor;
    data2.WriteParcelable(&bluetoothGattService);
    data2.WriteParcelable(&bluetoothGattDescriptor);
    bluetoothGattServerServer->RespondDescriptorWriteInner(data, reply);
    bluetoothGattServerServer->RespondDescriptorWriteInner(data1, reply);
    bluetoothGattServerServer->RespondDescriptorWriteInner(data2, reply);
    HILOGI("GattServer_ModuleTest_RespondDescriptorWriteInner001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnCharacteristicReadRequest001
 * @tc.name: GattServer_ModuleTest_OnCharacteristicReadRequest001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnCharacteristicReadRequest001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequest001 start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    proxy_->OnCharacteristicReadRequest(device, characteristic);
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequest001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnConnectionStateChanged001
 * @tc.name: GattServer_ModuleTest_OnConnectionStateChanged001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnConnectionStateChanged001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnConnectionStateChanged001 start");
    BluetoothGattDevice device;
    proxy_->OnConnectionStateChanged(device, 1, 1, 4, "");
    HILOGI("GattServer_ModuleTest_OnConnectionStateChanged001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnAddService001
 * @tc.name: GattServer_ModuleTest_OnAddService001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnAddService001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnAddService001 start");
    BluetoothGattService service;
    proxy_->OnAddService(1, service);
    HILOGI("GattServer_ModuleTest_OnAddService001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnCharacteristicWriteRequest001
 * @tc.name: GattServer_ModuleTest_OnCharacteristicWriteRequest001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnCharacteristicWriteRequest001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicWriteRequest001 start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    proxy_->OnCharacteristicWriteRequest(device, characteristic, false);
    HILOGI("GattServer_ModuleTest_OnCharacteristicWriteRequest001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnDescriptorReadRequest001
 * @tc.name: GattServer_ModuleTest_OnDescriptorReadRequest001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnDescriptorReadRequest001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnDescriptorReadRequest001 start");
    BluetoothGattDevice device;
    BluetoothGattDescriptor descriptor;
    proxy_->OnDescriptorReadRequest(device, descriptor);
    HILOGI("GattServer_ModuleTest_OnDescriptorReadRequest001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnDescriptorWriteRequest001
 * @tc.name: GattServer_ModuleTest_OnDescriptorWriteRequest001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnDescriptorWriteRequest001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnDescriptorWriteRequest001 start");
    BluetoothGattDevice device;
    BluetoothGattDescriptor descriptor;
    proxy_->OnDescriptorWriteRequest(device, descriptor);
    HILOGI("GattServer_ModuleTest_OnDescriptorWriteRequest001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnMtuChanged001
 * @tc.name: GattServer_ModuleTest_OnMtuChanged001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnMtuChanged001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnMtuChanged001 start");
    BluetoothGattDevice device;
    proxy_->OnMtuChanged(device, 1);
    HILOGI("GattServer_ModuleTest_OnMtuChanged001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnNotifyConfirm001
 * @tc.name: GattServer_ModuleTest_OnNotifyConfirm001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnNotifyConfirm001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnNotifyConfirm001 start");
    BluetoothGattDevice device;
    BluetoothGattCharacteristic characteristic;
    proxy_->OnNotifyConfirm(device, characteristic, 1);
    HILOGI("GattServer_ModuleTest_OnNotifyConfirm001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnConnectionParameterChanged001
 * @tc.name: GattServer_ModuleTest_OnConnectionParameterChanged001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnConnectionParameterChanged001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnConnectionParameterChanged001 start");
    BluetoothGattDevice device;
    proxy_->OnConnectionParameterChanged(device, 1, 1, 1, 1);
    HILOGI("GattServer_ModuleTest_OnConnectionParameterChanged001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_ConvertBluetoothGattDevice001
 * @tc.name: GattServer_ModuleTest_ConvertBluetoothGattDevice001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_ConvertBluetoothGattDevice001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ConvertBluetoothGattDevice001 start");
    BluetoothGattServerServer::impl implTest;
    BluetoothGattDevice bluetoothDevice;
    bluetooth::GattDevice device;
    implTest.ConvertBluetoothGattDevice(bluetoothDevice, device);
    HILOGI("GattServer_ModuleTest_ConvertBluetoothGattDevice001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_CheckGattServerPermission001
 * @tc.name: GattServer_ModuleTest_CheckGattServerPermission001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_CheckGattServerPermission001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_CheckGattServerPermission001 start");
    uint64_t tokenId = 0;
    int sdkVersion = 10;
    EXPECT_FALSE(CheckGattServerPermission(tokenId, sdkVersion));
    sdkVersion = 8;
    EXPECT_FALSE(CheckGattServerPermission(tokenId, sdkVersion));
    HILOGI("GattServer_ModuleTest_CheckGattServerPermission001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_ConvertGattDevice001
 * @tc.name: GattServer_ModuleTest_ConvertGattDevice001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_ConvertGattDevice001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ConvertGattDevice001 start");
    bluetooth::GattDevice device;
    bool addFlag = true;
    BluetoothGattDevice bluetoothDevice;
    gattServerCallbackImpl->isUseRealAddrFlag = true;
    EXPECT_TRUE(gattServerCallbackImpl->ConvertGattDevice(device, addFlag, bluetoothDevice));
    gattServerCallbackImpl->isUseRealAddrFlag = false;
    EXPECT_TRUE(gattServerCallbackImpl->ConvertGattDevice(device, addFlag, bluetoothDevice));
    EXPECT_TRUE(gattServerCallbackImpl->ConvertGattDevice(device, false, bluetoothDevice));

    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("GattServer_ModuleTest_ConvertGattDevice001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnCharacteristicReadRequest002
 * @tc.name: GattServer_ModuleTest_OnCharacteristicReadRequest002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnCharacteristicReadRequest002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequest002 start");
    bluetooth::GattDevice device;
    bluetooth::Characteristic characteristic;
    gattServerCallbackImpl->OnCharacteristicReadRequest(device, characteristic);
    HILOGI("GattServer_ModuleTest_OnCharacteristicReadRequest002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnCharacteristicWriteRequest002
 * @tc.name: GattServer_ModuleTest_OnCharacteristicWriteRequest002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnCharacteristicWriteRequest002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnCharacteristicWriteRequest002 start");
    bluetooth::GattDevice device;
    bluetooth::Characteristic characteristic;
    gattServerCallbackImpl->OnCharacteristicWriteRequest(device, characteristic, false);
    HILOGI("GattServer_ModuleTest_OnCharacteristicWriteRequest002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnDescriptorReadRequest002
 * @tc.name: GattServer_ModuleTest_OnDescriptorReadRequest002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnDescriptorReadRequest002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnDescriptorReadRequest002 start");
    bluetooth::GattDevice device;
    bluetooth::Descriptor descriptor;
    gattServerCallbackImpl->OnDescriptorReadRequest(device, descriptor);
    HILOGI("GattServer_ModuleTest_OnDescriptorReadRequest002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnDescriptorWriteRequest002
 * @tc.name: GattServer_ModuleTest_OnDescriptorWriteRequest002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnDescriptorWriteRequest002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnDescriptorWriteRequest002 start");
    bluetooth::GattDevice device;
    bluetooth::Descriptor descriptor;
    gattServerCallbackImpl->OnDescriptorWriteRequest(device, descriptor);
    HILOGI("GattServer_ModuleTest_OnDescriptorWriteRequest002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnNotifyConfirm002
 * @tc.name: GattServer_ModuleTest_OnNotifyConfirm002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnNotifyConfirm002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnNotifyConfirm002 start");
    bluetooth::GattDevice device;
    bluetooth::Characteristic characteristic;
    gattServerCallbackImpl->OnNotifyConfirm(device, characteristic, 1);
    HILOGI("GattServer_ModuleTest_OnNotifyConfirm002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnConnectionStateChanged002
 * @tc.name: GattServer_ModuleTest_OnConnectionStateChanged002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnConnectionStateChanged002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnConnectionStateChanged002 start");
    bluetooth::GattDevice device;
    gattServerCallbackImpl->OnConnectionStateChanged(device, 1, 1, 4, "");
    gattServerCallbackImpl->OnConnectionStateChanged(device, 1, 0, 4, "");
    gattServerCallbackImpl->OnConnectionStateChanged(device, 1, 3, 4, "");
    gattServerCallbackImpl->callback_ = nullptr;
    gattServerCallbackImpl->OnConnectionStateChanged(device, 1, 1, 4, "");
    gattServerCallbackImpl->callback_ = callback_;
    HILOGI("GattServer_ModuleTest_OnConnectionStateChanged002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnMtuChanged002
 * @tc.name: GattServer_ModuleTest_OnMtuChanged002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnMtuChanged002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnMtuChanged002 start");
    bluetooth::GattDevice device;
    gattServerCallbackImpl->OnMtuChanged(device, 1);
    HILOGI("GattServer_ModuleTest_OnMtuChanged002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnConnectionParameterChanged002
 * @tc.name: GattServer_ModuleTest_OnConnectionParameterChanged002
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnConnectionParameterChanged002, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnConnectionParameterChanged002 start");
    bluetooth::GattDevice device;
    gattServerCallbackImpl->OnConnectionParameterChanged(device, 1, 1, 1, 1);
    HILOGI("GattServer_ModuleTest_OnConnectionParameterChanged002 end");
}

/*
 * @tc.number: GattServer_ModuleTest_WriteToParcel001
 * @tc.name: GattServer_ModuleTest_WriteToParcel001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_WriteToParcel001, TestSize.Level1)
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
HWTEST_F(BlueToothGattServerServerTest, BluetoothGattCharacteristicTest_WriteToParcel001, TestSize.Level1)
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
HWTEST_F(BlueToothGattServerServerTest, BluetoothGattDescriptor_ModuleTest_WriteToParcel001, TestSize.Level1)
{
    HILOGI("BluetoothGattDescriptor_ModuleTest_WriteToParcel001 start");
    BluetoothGattDescriptor descriptor;
    OHOS::Parcel data;
    descriptor.WriteToParcel(data);
    HILOGI("BluetoothGattDescriptor_ModuleTest_WriteToParcel001 end");
}

/*
 * @tc.number: GattServer_ModuleTest_GetConnectedState001
 * @tc.name: GetConnectedState
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_GetConnectedState001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_GetConnectedState start");
    const std::string deviceId = "11:22:33:44:55:66";
    int state = -1;
    int ret = bluetoothGattServerServer->GetConnectedState(deviceId, state);
    EXPECT_EQ(ret, BT_NO_ERROR);
    EXPECT_EQ(state, 3);
    HILOGI("GattServer_ModuleTest_GetConnectedState end");
}

/*
 * @tc.number: GattServer_ModuleTest_OnBlePhyUpdate001
 * @tc.name: GattServer_ModuleTest_OnBlePhyUpdate001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnBlePhyUpdate001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_OnBlePhyUpdate001 start");
    bluetooth::GattDevice device;
    proxy_->OnBlePhyUpdate(device, 1, 1, 0);
    EXPECT_TRUE(proxy_ != nullptr);
    HILOGI("GattServer_ModuleTest_OnBlePhyUpdate001 end");
}
 
/*
 * @tc.number: GattServer_ModuleTest_OnBlePhyRead001
 * @tc.name: GattServer_ModuleTest_OnBlePhyRead001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_OnBlePhyRead001, TestSize.Level1)
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
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_SetPhy001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_SetPhy001 start");
    const std::string deviceId = "11:22:33:44:55:66";
    int ret = bluetoothGattServerServer->SetPhy(1, deviceId, 1, 1, 0);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_SetPhy001 end");
}
 
/*
 * @tc.number: GattServer_ModuleTest_ReadPhy001
 * @tc.name: GattServer_ModuleTest_ReadPhy001
*/
HWTEST_F(BlueToothGattServerServerTest, GattServer_ModuleTest_ReadPhy001, TestSize.Level1)
{
    HILOGI("GattServer_ModuleTest_ReadPhy001 start");
    const std::string deviceId = "11:22:33:44:55:66";
    int ret = bluetoothGattServerServer->ReadPhy(1, deviceId);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("GattServer_ModuleTest_ReadPhy001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
