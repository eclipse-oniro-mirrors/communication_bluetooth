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
#include <gmock/gmock.h>
#include "bluetooth_errorcode.h"
#include "bluetooth_ble_advertiser_server.h"
#include "bluetooth_ble_advertiser_server.cpp"
#include "bluetooth_ble_advertiser_power_compensator.h"
#include "bluetooth_ble_advertise_callback_stub.h"
#include "bluetooth_ble_advertise_callback_proxy.h"
#include "bluetooth_gatt_client_server.h"
#include "bluetooth_ble_central_manager_server.h"
#include "bluetooth_ble_central_manager_callback_stub.h"
#include "bluetooth_gatt_server_callback_stub.h"
#include "bluetooth_gatt_client_callback_stub.h"
#include "bluetooth_gatt_server_server.h"
#include "interface_adapter_manager.h"
#include "profile_service_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "ble_service_data.h"
#include "class_creator.h"
#include "gatt_client_service.h"
#include "parameters.h"
#include "vendor_interface.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

void *GattClientServiceCreate(void)
{
    return new GattClientService();
}

const BluetoothRawAddress DUMMY_ADDRESS("11:22:33:44:55:66");
const int TIME = 2;
static BluetoothHost *host_;
class BluetoothBleAdvertiseCallbackStubTest : public BluetoothBleAdvertiseCallbackStub {
public:
    void OnAutoStopAdvEvent(int32_t advHandle) override {}
    void OnStartResultEvent(int32_t result, int32_t advHandle, int32_t opcode) override {}
    void OnEnableResultEvent(int32_t result, int32_t advHandle) override {}
    void OnDisableResultEvent(int32_t result, int32_t advHandle) override {}
    void OnStopResultEvent(int32_t result, int32_t advHandle) override {}
    void OnSetAdvDataEvent(int32_t result, int32_t advHandle, bluetooth::SetAdvDataType type) override {}
    void OnChangeAdvResultEvent(int32_t result, int32_t advHandle) override {}

    explicit BluetoothBleAdvertiseCallbackStubTest()
    {}
    ~BluetoothBleAdvertiseCallbackStubTest() override
    {}

private:
};

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
class BluetoothBleAdvertiserServerTest : public testing::Test {
public:
    BluetoothBleAdvertiserServerTest()
    {}
    ~BluetoothBleAdvertiserServerTest()
    {}

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    sptr<BluetoothBleAdvertiseCallbackStubTest> callback_ {nullptr};
    sptr<BluetoothBleAdvertiseCallbackStubTest> bluetoothBleAdvertiseCallbackStubTest =
        new BluetoothBleAdvertiseCallbackStubTest();
    sptr<BluetoothBleAdvertiseCallbackProxy> proxy_ = new BluetoothBleAdvertiseCallbackProxy(
        bluetoothBleAdvertiseCallbackStubTest);
    std::unique_ptr<BluetoothBleAdvertiserServer> bluetoothBleAdvertiserServer {nullptr};
    std::unique_ptr<BluetoothGattClientServer> bluetoothGattClientServer {nullptr};
    std::unique_ptr<BluetoothGattServerServer> bluetoothGattServerServer {nullptr};

    std::string savedMlpsAntInfo_;
    std::string savedRangingAntInfo_;
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

class BluetoothGattClientCallbackTest : public IBluetoothGattClientCallback {
public:
    MOCK_METHOD(void, OnServicesChanged, (), (override));
    MOCK_METHOD(void, OnCharacteristicRead, (int32_t ret,
        const BluetoothGattCharacteristic &characteristic), (override));
    MOCK_METHOD(void, OnCharacteristicWrite, (int32_t ret,
        const BluetoothGattCharacteristic &characteristic, const BluetoothGattRspContext &rspContext), (override));
    MOCK_METHOD(void, OnCharacteristicChanged, (const BluetoothGattCharacteristic &characteristic), (override));
    MOCK_METHOD(void, OnDescriptorRead, (int32_t ret, const BluetoothGattDescriptor &descriptor), (override));
    MOCK_METHOD(void, OnDescriptorWrite, (int32_t ret, const BluetoothGattDescriptor &descriptor), (override));
    MOCK_METHOD(void, OnConnectionStateChanged, (int32_t state, int32_t newState, int32_t reason,
        const std::string &reasonMessage), (override));
    MOCK_METHOD(void, OnMtuChanged, (int32_t state, int32_t mtu), (override));
    MOCK_METHOD(void, OnServicesDiscovered, (int32_t status), (override));
    MOCK_METHOD(void, OnConnectionParameterChanged, (int32_t interval, int32_t latency, int32_t timeout,
        int32_t status), (override));
    MOCK_METHOD(void, OnReadRemoteRssiValue, (const bluetooth::RawAddress &addr, int rssi, int status), (override));
    MOCK_METHOD(void, OnBlePhyUpdate, (int32_t txPhy, int32_t rxPhy, int32_t status), (override));
    MOCK_METHOD(void, OnBlePhyRead, (int32_t txPhy, int32_t rxPhy, int32_t status), (override));
    sptr<OHOS::IRemoteObject> AsObject() override
    {
        sptr<BluetoothGattClientCallbackStubTest> bluetoothGattClientCallbackStubTest =
        new BluetoothGattClientCallbackStubTest();
        return bluetoothGattClientCallbackStubTest;
    }
    ~BluetoothGattClientCallbackTest() override = default;
};

void BluetoothBleAdvertiserServerTest::SetUpTestSuite(void)
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

    ClassFactory::RegisterClass("GattClientService", GattClientServiceCreate);

    HILOGI("SetUpTestCase end");
}

void BluetoothBleAdvertiserServerTest::TearDownTestSuite(void)
{
    host_ = nullptr;
}

void BluetoothBleAdvertiserServerTest::SetUp()
{
    savedMlpsAntInfo_ = system::GetParameter("persist.bluetooth.dedicated_antenna_switch_state", "0:0");
    savedRangingAntInfo_ = system::GetParameter("persist.bluetooth.ranging_ant_info", "0:0");

    bluetoothBleAdvertiserServer = std::make_unique<BluetoothBleAdvertiserServer>();
    bluetoothGattClientServer = std::make_unique<BluetoothGattClientServer>();
    bluetoothGattServerServer = std::make_unique<BluetoothGattServerServer>();

    callback_ = new BluetoothBleAdvertiseCallbackStubTest();

    // new GattClientService
    auto manager = ProfileServiceManager::GetInstance();
    string name = "GattClientService";
    manager->SetProfileService(bluetooth::BTTransport::ADAPTER_BLE, name);
}

void BluetoothBleAdvertiserServerTest::TearDown()
{
    system::SetParameter("persist.bluetooth.dedicated_antenna_switch_state", savedMlpsAntInfo_);
    system::SetParameter("persist.bluetooth.ranging_ant_info", savedRangingAntInfo_);

    bluetoothBleAdvertiserServer = nullptr;
    bluetoothGattClientServer = nullptr;
    bluetoothGattServerServer = nullptr;

    // delete GattClientService
    auto manager = ProfileServiceManager::GetInstance();
    if (manager == nullptr) {
        return;
    }
    GattClientService *client = static_cast<GattClientService *>(manager->GetProfileService("GattClientService"));
    if (client == nullptr) {
        delete client;
        return;
    }
    client = nullptr;
    delete client;
}

/*
 * @tc.number: BleServer_ModuleTest_Init001
 * @tc.name: BleServer_ModuleTest_Init001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_Init001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_Init001 start");
    std::unique_ptr<BluetoothBleAdvertiserServer> advertiserServer = std::make_unique<
    BluetoothBleAdvertiserServer>();
    HILOGI("BleServer_ModuleTest_Init001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_StartAdvertising001
 * @tc.name: BleServer_ModuleTest_StartAdvertising001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_StartAdvertising001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartAdvertising001 start");
    BluetoothBleAdvertiserSettings settings;
    settings.SetInterval(32);
    BluetoothBleAdvertiserData advData;
    BluetoothBleAdvertiserData scanResponse;
    uint16_t duration = 0;
    system::SetParameter("ro.bluetooth.advertiser.interval.debug.on", "1");
    bluetoothBleAdvertiserServer->StartAdvertising(settings, advData, scanResponse, 1, duration, false);
    HILOGI("BleServer_ModuleTest_StartAdvertising001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StopAdvertising001
 * @tc.name: BleServer_ModuleTest_StopAdvertising001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_StopAdvertising001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StopAdvertising001 start");
    bluetoothBleAdvertiserServer->StopAdvertising(1);
    HILOGI("BleServer_ModuleTest_StopAdvertising001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_Close001
 * @tc.name: BleServer_ModuleTest_Close001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_Close001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_Close001 start");
    bluetoothBleAdvertiserServer->Close(1);
    HILOGI("BleServer_ModuleTest_Close001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_RegisterBleAdvertiserCallback001
 * @tc.name: BleServer_ModuleTest_RegisterBleAdvertiserCallback001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_RegisterBleAdvertiserCallback001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_RegisterBleAdvertiserCallback001 start");
    bluetoothBleAdvertiserServer->RegisterBleAdvertiserCallback(callback_);
    bluetoothBleAdvertiserServer->RegisterBleAdvertiserCallback(nullptr);
    HILOGI("BleServer_ModuleTest_RegisterBleAdvertiserCallback001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_DeregisterBleAdvertiserCallback001
 * @tc.name: BleServer_ModuleTest_DeregisterBleAdvertiserCallback001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_DeregisterBleAdvertiserCallback001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_DeregisterBleAdvertiserCallback001 start");
    bluetoothBleAdvertiserServer->DeregisterBleAdvertiserCallback(callback_);
    bluetoothBleAdvertiserServer->DeregisterBleAdvertiserCallback(nullptr);
    HILOGI("BleServer_ModuleTest_DeregisterBleAdvertiserCallback001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_GetAdvertiserHandle001
 * @tc.name: BleServer_ModuleTest_GetAdvertiserHandle001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_GetAdvertiserHandle001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_GetAdvertiserHandle001 start");
    int32_t advHandle = 1;
    bluetoothBleAdvertiserServer->GetAdvertiserHandle(advHandle, callback_);
    HILOGI("BleServer_ModuleTest_GetAdvertiserHandle001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_SetAdvertisingData001
 * @tc.name: BleServer_ModuleTest_SetAdvertisingData001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_SetAdvertisingData001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetAdvertisingData001 start");
    BluetoothBleAdvertiserData advData;
    BluetoothBleAdvertiserData scanResponse;
    bluetoothBleAdvertiserServer->SetAdvertisingData(advData, scanResponse, 1);
    HILOGI("BleServer_ModuleTest_SetAdvertisingData001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_OnRemoteRequest001
 * @tc.name: BleServer_ModuleTest_OnRemoteRequest001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_OnRemoteRequest001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnRemoteRequest001 start");
    uint32_t code = 17;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothBleAdvertiserServer::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    bluetoothBleAdvertiserServer->OnRemoteRequest(code, data, reply, option);
    HILOGI("BleServer_ModuleTest_OnRemoteRequest001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnRemoteRequest002
 * @tc.name: BleServer_ModuleTest_OnRemoteRequest002
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_OnRemoteRequest002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnRemoteRequest002 start");
    uint32_t code = 0;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothBleAdvertiserServer::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    bluetoothBleAdvertiserServer->OnRemoteRequest(code, data, reply, option);
    HILOGI("BleServer_ModuleTest_OnRemoteRequest002 start");
}

/*
 * @tc.number: BleServer_ModuleTest_StartAdvertisingInner001
 * @tc.name: BleServer_ModuleTest_StartAdvertisingInner001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_StartAdvertisingInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartAdvertisingInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothBleAdvertiserSettings bluetoothBleAdvertiserSettings;
    data1.WriteParcelable(&bluetoothBleAdvertiserSettings);
    MessageParcel data2;
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    data2.WriteParcelable(&bluetoothBleAdvertiserSettings);
    data2.WriteParcelable(&bluetoothBleAdvertiserData);
    MessageParcel data3;
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData1;
    data3.WriteParcelable(&bluetoothBleAdvertiserSettings);
    data3.WriteParcelable(&bluetoothBleAdvertiserData);
    data3.WriteParcelable(&bluetoothBleAdvertiserData1);
    bluetoothBleAdvertiserServer->StartAdvertisingInner(data, reply);
    bluetoothBleAdvertiserServer->StartAdvertisingInner(data1, reply);
    bluetoothBleAdvertiserServer->StartAdvertisingInner(data2, reply);
    bluetoothBleAdvertiserServer->StartAdvertisingInner(data3, reply);
    HILOGI("BleServer_ModuleTest_StartAdvertisingInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_StopAdvertisingInner001
 * @tc.name: BleServer_ModuleTest_StopAdvertisingInner001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_StopAdvertisingInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StopAdvertisingInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothBleAdvertiserServer->StopAdvertisingInner(data, reply);
    HILOGI("BleServer_ModuleTest_StopAdvertisingInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_GetAdvertiserHandleInner001
 * @tc.name: BleServer_ModuleTest_GetAdvertiserHandleInner001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_GetAdvertiserHandleInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_GetAdvertiserHandleInner001 start");
    MessageParcel data;
    MessageParcel reply;
    bluetoothBleAdvertiserServer->GetAdvertiserHandleInner(data, reply);
    HILOGI("BleServer_ModuleTest_GetAdvertiserHandleInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetAdvertisingDataInner001
 * @tc.name: BleServer_ModuleTest_SetAdvertisingDataInner001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_SetAdvertisingDataInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetAdvertisingDataInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    data1.WriteParcelable(&bluetoothBleAdvertiserData);
    MessageParcel data2;
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData1;
    data2.WriteParcelable(&bluetoothBleAdvertiserData);
    data2.WriteParcelable(&bluetoothBleAdvertiserData1);
    bluetoothBleAdvertiserServer->SetAdvertisingDataInner(data, reply);
    bluetoothBleAdvertiserServer->SetAdvertisingDataInner(data1, reply);
    bluetoothBleAdvertiserServer->SetAdvertisingDataInner(data2, reply);
    HILOGI("BleServer_ModuleTest_SetAdvertisingDataInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_ChangeAdvertisingParamsInner001
 * @tc.name: BleServer_ModuleTest_ChangeAdvertisingParamsInner001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_ChangeAdvertisingParamsInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ChangeAdvertisingParamsInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    data.WriteUint8(1);
    BluetoothBleAdvertiserSettings settings;
    data.WriteParcelable(&settings);
    bluetoothBleAdvertiserServer->ChangeAdvertisingParamsInner(data, reply);
    HILOGI("BleServer_ModuleTest_ChangeAdvertisingParamsInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnStartResultEvent001
 * @tc.name: BleServer_ModuleTest_OnStartResultEvent001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_OnStartResultEvent001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnStartResultEvent001 start");
    proxy_->OnStartResultEvent(0, 1, 1);
    proxy_->OnStartResultEvent(1, 1, 1);
    HILOGI("BleServer_ModuleTest_OnStartResultEvent001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnAutoStopAdvEvent001
 * @tc.name: BleServer_ModuleTest_OnAutoStopAdvEvent001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_OnAutoStopAdvEvent001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnAutoStopAdvEvent001 start");
    proxy_->OnAutoStopAdvEvent(1);
    HILOGI("BleServer_ModuleTest_OnAutoStopAdvEvent001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnSetAdvDataEvent001
 * @tc.name: BleServer_ModuleTest_OnSetAdvDataEvent001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_OnSetAdvDataEvent001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnSetAdvDataEvent001 start");
    proxy_->OnSetAdvDataEvent(1, 1);
    HILOGI("BleServer_ModuleTest_OnSetAdvDataEvent001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnChangeAdvResultEvent001
 * @tc.name: BleServer_ModuleTest_OnChangeAdvResultEvent001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_OnChangeAdvResultEvent001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnChangeAdvResultEvent001 start");
    proxy_->OnChangeAdvResultEvent(1, 1);
    HILOGI("BleServer_ModuleTest_OnChangeAdvResultEvent001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_StopAdvertising002
 * @tc.name: BleServer_ModuleTest_StopAdvertising002
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_StopAdvertising002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StopAdvertising002 start");
    // Masking: External domain (libsamgr_proxy.z.so) cficheck.
    // There is a high probability that this field is related to system status parameters.
    // IAdapterManager::GetInstance()->Start();
    ProfileServiceManager::GetInstance()->Start();
    bluetoothBleAdvertiserServer->StopAdvertising(1);
    HILOGI("BleServer_ModuleTest_StopAdvertising002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_WriteToParcel001
 * @tc.name: BleServer_ModuleTest_WriteToParcel001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_WriteToParcel001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_WriteToParcel001 start");
    BluetoothBleAdvertiserSettings settings;
    OHOS::Parcel parcel;
    settings.WriteToParcel(parcel);
    HILOGI("BleServer_ModuleTest_WriteToParcel001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_EnableAdvertising001
 * @tc.name: BleServer_ModuleTest_EnableAdvertising001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_EnableAdvertising001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_EnableAdvertising001 start");
    uint16_t duration = 0;
    bluetoothBleAdvertiserServer->EnableAdvertising(1, duration);
    HILOGI("BleServer_ModuleTest_EnableAdvertising001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_DisableAdvertising001
 * @tc.name: BleServer_ModuleTest_DisableAdvertising001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_DisableAdvertising001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_DisableAdvertising001 start");
    bluetoothBleAdvertiserServer->DisableAdvertising(1);
    HILOGI("BleServer_ModuleTest_DisableAdvertising001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_Register_OnSetAdvDataEvent001
 * @tc.name: BleServer_ModuleTest_Register_OnSetAdvDataEvent001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_Register_OnSetAdvDataEvent001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_Register_OnSetAdvDataEvent001 start");
    bluetoothBleAdvertiserServer->RegisterBleAdvertiserCallback(callback_);
    int32_t pid = bluetoothBleAdvertiserServer->pimpl->applications->GetRemotePid(callback_->AsObject());

    BluetoothBleCentralManagerServer::proxyPids_.clear();
    EXPECT_TRUE(BluetoothBleCentralManagerServer::proxyPids_.count(pid) == 0);

    bluetoothBleAdvertiserServer->pimpl->observerImp_->OnSetAdvDataEvent(0, 1);

    BluetoothBleCentralManagerServer::proxyPids_[pid] = 0;
    EXPECT_TRUE(BluetoothBleCentralManagerServer::proxyPids_.count(pid) == 1);

    bluetoothBleAdvertiserServer->pimpl->observerImp_->OnSetAdvDataEvent(0, 1);
    HILOGI("BleServer_ModuleTest_Register_OnSetAdvDataEvent001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_RegisterBleAdvertiserCallback002
 * @tc.name: BleServer_ModuleTest_RegisterBleAdvertiserCallback002
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_RegisterBleAdvertiserCallback002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_RegisterBleAdvertiserCallback002 start");
    for(int i = 0; i <= 1001; i++) {
        bluetoothBleAdvertiserServer->RegisterBleAdvertiserCallback(callback_);
    }
    HILOGI("BleServer_ModuleTest_RegisterBleAdvertiserCallback002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_ChangeAdvertisingParams001
 * @tc.name: BleServer_ModuleTest_ChangeAdvertisingParams001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_ChangeAdvertisingParams001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ChangeAdvertisingParams001 start");
    BluetoothBleAdvertiserSettings settings;
    settings.SetInterval(32);
    bluetoothBleAdvertiserServer->ChangeAdvertisingParams(1, settings);
    HILOGI("BleServer_ModuleTest_ChangeAdvertisingParams001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_AdjustTxPowerForWalletService001
 * @tc.name: BleServer_ModuleTest_AdjustTxPowerForWalletService001
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_AdjustTxPowerForWalletService001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_AdjustTxPowerForWalletService001 start");
    std::string callingName = VendorInterface::GetWalletServiceName();
    bluetooth::BleAdvertiserSettingsImpl implSettings;
    // 钱包发送广播时走增加功率的流程
    uint32_t tokenId = 537692227;
    implSettings.SetInterval(150);
    implSettings.SetTxPower(1);
    bluetoothBleAdvertiserServer->AdjustTxPowerForWalletService(callingName, tokenId, implSettings);

    // 同样的广播其他业务发送时不走增加功率的流程
    implSettings.SetTxPower(1);
    callingName = "com.example.hivew";
    bluetoothBleAdvertiserServer->AdjustTxPowerForWalletService(callingName, tokenId, implSettings);
    EXPECT_TRUE(implSettings.GetTxPower() == 1);

    // 其他应用假冒钱包包名时应该拦截
    callingName = "com.example.walletservice";
    tokenId = 000000000;
    implSettings.SetTxPower(1);
    bluetoothBleAdvertiserServer->AdjustTxPowerForWalletService(callingName, tokenId, implSettings);
    EXPECT_TRUE(implSettings.GetTxPower() == 1);

    // 钱包发送广播时设置的TxPower小于最小值
    tokenId = 537692227;
    implSettings.SetInterval(150);
    implSettings.SetTxPower(-128);
    bluetoothBleAdvertiserServer->AdjustTxPowerForWalletService(callingName, tokenId, implSettings);
    EXPECT_FALSE(implSettings.GetTxPower() == -122);

    // 钱包发送广播时设置的TxPower大于最大值
    implSettings.SetTxPower(2);
    bluetoothBleAdvertiserServer->AdjustTxPowerForWalletService(callingName, tokenId, implSettings);
    EXPECT_FALSE(implSettings.GetTxPower() == 8);
    HILOGI("BleServer_ModuleTest_AdjustTxPowerForWalletService001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_SetAdvertisingData002
 * @tc.name: BleServer_ModuleTest_SetAdvertisingData002
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_SetAdvertisingData002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetAdvertisingData002 start");
    BluetoothBleAdvertiserData advData;
    BluetoothBleAdvertiserData scanResponse;
    bool isAdv = true;
    bluetoothBleAdvertiserServer->SetAdvertisingData(advData, advData, 1,
        bluetooth::SetAdvDataType::SET_ADV_DATA_ONLY_ADV);
    bluetoothBleAdvertiserServer->SetAdvertisingData(scanResponse, scanResponse, 1,
        bluetooth::SetAdvDataType::SET_ADV_DATA_ONLY_RSP);
    EXPECT_TRUE(isAdv);
    HILOGI("BleServer_ModuleTest_SetAdvertisingData002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_SetAdvertisingDataInner002
 * @tc.name: BleServer_ModuleTest_SetAdvertisingDataInner002
*/
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_SetAdvertisingDataInner002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetAdvertisingDataInner002 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleAdvertiserServer->SetAdvertisingDataInner(data, reply), TRANSACTION_ERR);
    
    BluetoothBleAdvertiserData bleAdvData;
    data.WriteParcelable(&bleAdvData);
    data.WriteParcelable(&bleAdvData);
    data.WriteInt32(1);
    data.WriteInt32(1);
    EXPECT_EQ(bluetoothBleAdvertiserServer->SetAdvertisingDataInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetAdvertisingDataInner002 start");
}

static int GetProductTxPowerCompensation()
{
    return system::GetIntParameter("const.bluetooth.c1_ble_tx_power_compensation", 0);
}

static int CalcExpectedCompensatedTxPower(int currentTxPower, int compensation)
{
    int newTxPower = currentTxPower + compensation;
    if (newTxPower > 2) {
        newTxPower = 2;
    }
    if (newTxPower < -127) {
        newTxPower = -127;
    }
    return newTxPower;
}

/**
 * @tc.number: BleServer_ModuleTest_BleAdvTxPowerCompensator_001
 * @tc.name: BleServer_ModuleTest_BleAdvTxPowerCompensator_001
 * @tc.desc: Non-dedicated antenna default state, no compensation applied
 */
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_BleAdvTxPowerCompensator_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_001 start");
    system::SetParameter("persist.bluetooth.dedicated_antenna_switch_state", "0:0");
    bluetooth::BleAdvertiserSettingsImpl implSettings;
    implSettings.SetTxPower(-7);
    bluetoothBleAdvertiserServer->txPowerCompensator_.AdjustBleAdvTxPower(implSettings);
    EXPECT_EQ(implSettings.GetTxPower(), -7);
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_001 end");
}

/**
 * @tc.number: BleServer_ModuleTest_BleAdvTxPowerCompensator_002
 * @tc.name: BleServer_ModuleTest_BleAdvTxPowerCompensator_002
 * @tc.desc: Dedicated antenna with product compensation value, tx power adjusted and clamped to max 2
 */
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_BleAdvTxPowerCompensator_002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_002 start");
    system::SetParameter("persist.bluetooth.dedicated_antenna_switch_state", "1:3");
    system::SetParameter("persist.bluetooth.ranging_ant_info", "1:3");
    int compensation = GetProductTxPowerCompensation();
    bluetooth::BleAdvertiserSettingsImpl implSettings;
    implSettings.SetTxPower(1);
    bluetoothBleAdvertiserServer->txPowerCompensator_.AdjustBleAdvTxPower(implSettings);
    EXPECT_EQ(implSettings.GetTxPower(), CalcExpectedCompensatedTxPower(1, compensation));
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_002 end");
}

/**
 * @tc.number: BleServer_ModuleTest_BleAdvTxPowerCompensator_003
 * @tc.name: BleServer_ModuleTest_BleAdvTxPowerCompensator_003
 * @tc.desc: Non-Dedicated antenna (antID=0), no compensation applied
 */
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_BleAdvTxPowerCompensator_003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_003 start");
    system::SetParameter("persist.bluetooth.dedicated_antenna_switch_state", "0:0");
    bluetooth::BleAdvertiserSettingsImpl implSettings;
    implSettings.SetTxPower(-7);
    bluetoothBleAdvertiserServer->txPowerCompensator_.AdjustBleAdvTxPower(implSettings);
    EXPECT_EQ(implSettings.GetTxPower(), -7);
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_003 end");
}

/**
 * @tc.number: BleServer_ModuleTest_BleAdvTxPowerCompensator_004
 * @tc.name: BleServer_ModuleTest_BleAdvTxPowerCompensator_004
 * @tc.desc: Dedicated antenna with negative tx power, product compensation applied within range
 */
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_BleAdvTxPowerCompensator_004, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_004 start");
    system::SetParameter("persist.bluetooth.dedicated_antenna_switch_state", "1:3");
    system::SetParameter("persist.bluetooth.ranging_ant_info", "1:3");
    int compensation = GetProductTxPowerCompensation();
    bluetooth::BleAdvertiserSettingsImpl implSettings;
    implSettings.SetTxPower(-7);
    bluetoothBleAdvertiserServer->txPowerCompensator_.AdjustBleAdvTxPower(implSettings);
    EXPECT_EQ(implSettings.GetTxPower(), CalcExpectedCompensatedTxPower(-7, compensation));
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_004 end");
}

/**
 * @tc.number: BleServer_ModuleTest_BleAdvTxPowerCompensator_005
 * @tc.name: BleServer_ModuleTest_BleAdvTxPowerCompensator_005
 * @tc.desc: Invalid dedicated_antenna_switch_state format, no compensation applied
 */
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_BleAdvTxPowerCompensator_005, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_005 start");
    system::SetParameter("persist.bluetooth.dedicated_antenna_switch_state", "invalid");
    bluetooth::BleAdvertiserSettingsImpl implSettings;
    implSettings.SetTxPower(-7);
    bluetoothBleAdvertiserServer->txPowerCompensator_.AdjustBleAdvTxPower(implSettings);
    EXPECT_EQ(implSettings.GetTxPower(), -7);
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_005 end");
}

/**
 * @tc.number: BleServer_ModuleTest_BleAdvTxPowerCompensator_006
 * @tc.name: BleServer_ModuleTest_BleAdvTxPowerCompensator_006
 * @tc.desc: Non-MAC1 with Dedicated antenna, no compensation applied
 */
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_BleAdvTxPowerCompensator_006, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_006 start");
    system::SetParameter("persist.bluetooth.dedicated_antenna_switch_state", "1:3");
    system::SetParameter("persist.bluetooth.ranging_ant_info", "0:0");
    bluetooth::BleAdvertiserSettingsImpl implSettings;
    implSettings.SetTxPower(-7);
    bluetoothBleAdvertiserServer->txPowerCompensator_.AdjustBleAdvTxPower(implSettings);
    EXPECT_EQ(implSettings.GetTxPower(), -7);
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_006 end");
}

/**
 * @tc.number: BleServer_ModuleTest_BleAdvTxPowerCompensator_007
 * @tc.name: BleServer_ModuleTest_BleAdvTxPowerCompensator_007
 * @tc.desc: MAC1 with Dedicated antenna, product compensation applied and clamped to max 2
 */
HWTEST_F(BluetoothBleAdvertiserServerTest, BleServer_ModuleTest_BleAdvTxPowerCompensator_007, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_007 start");
    system::SetParameter("persist.bluetooth.dedicated_antenna_switch_state", "1:3");
    system::SetParameter("persist.bluetooth.ranging_ant_info", "1:3");
    int compensation = GetProductTxPowerCompensation();
    bluetooth::BleAdvertiserSettingsImpl implSettings;
    implSettings.SetTxPower(1);
    bluetoothBleAdvertiserServer->txPowerCompensator_.AdjustBleAdvTxPower(implSettings);
    EXPECT_EQ(implSettings.GetTxPower(), CalcExpectedCompensatedTxPower(1, compensation));
    HILOGI("BleServer_ModuleTest_BleAdvTxPowerCompensator_007 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
