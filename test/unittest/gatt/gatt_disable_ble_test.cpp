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

#include "bluetooth_log.h"
#include "bluetooth_gatt_server.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_manager.h"
#include "bluetooth_host.h"
#include "bt_uuid.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
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
};
class BluetoothGattServerCallbackTest : public GattServerCallback {
public:
    BluetoothGattServerCallbackTest() = default;
    virtual ~BluetoothGattServerCallbackTest() = default;
    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state) override
    {
        HILOGI("BluetoothGattServerCallbackTest::OnConnectionStateUpdate called");
    }
    void OnServiceAdded(GattService service, int ret) override
    {
        HILOGI("BluetoothGattServerCallbackTest::OnServiceAdded called");
    }
    void OnCharacteristicReadRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override
    {
        HILOGI("BluetoothGattServerCallbackTest::OnCharacteristicReadRequest called");
    }
    void OnCharacteristicWriteRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override
    {
        HILOGI("BluetoothGattServerCallbackTest::OnCharacteristicWriteRequest called");
    }
    void OnDescriptorReadRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override
    {
        HILOGI("BluetoothGattServerCallbackTest::OnDescriptorReadRequest called");
    }
    void OnDescriptorWriteRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override
    {
        HILOGI("BluetoothGattServerCallbackTest::OnDescriptorWriteRequest called");
    }
    void OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu) override
    {
        HILOGI("BluetoothGattServerCallbackTest::OnMtuUpdate called");
    }
    void OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int result) override
    {
        HILOGI("OnNotificationCharacteristicChanged called");
    }
    void OnConnectionParameterChanged(
        const BluetoothRemoteDevice &device, int interval, int latency, int timeout, int status) override
    {
        HILOGI("OnConnectionParameterChanged called");
    }

private:
};

static std::shared_ptr<BluetoothGattServerCallbackTest> g_callback =
    std::make_shared<BluetoothGattServerCallbackTest>();
class GattDisableBleTest : public testing::Test {
public:
    BluetoothRemoteDevice device;
    std::shared_ptr<GattClient> client {nullptr};
    std::shared_ptr<GattServer> server {nullptr};
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (isBleEnable) {
            host_->DisableBle();
            sleep(TIME);
        }

        if (isBrEnable) {
            host_->DisableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        server = GattServer::CreateInstance(std::static_pointer_cast<GattServerCallback>(g_callback));
        client = std::make_shared<GattClient>(device);
        client->Init();
    }

    void TearDown()
    {
        client = nullptr;
        server = nullptr;
    }

public:
};
/**
 * @tc.name: BLE_GattDisableBleTest_00100
 * @tc.desc: Verify Descriptor function.
 * @tc.type: FUNC
 */
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00100, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00100 start");
    std::array<int, GattManager::GATT_CONNECTION_STATE_NUM> states = {1, 2, 3};
    GattManager test;
    test.GetDevicesByStates(states);
    HILOGI("BLE_GattDisableBleTest_00100 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_00200
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00200, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00200 start");
    GattManager test;
    test.GetConnectedDevices();
    HILOGI("BLE_GattDisableBleTest_00200 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_00300
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00300, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00300 start");
    bool isAutoConnect = true;
    int transport = 1;
    std::shared_ptr<GattClientCallbackTest> callbackTest;
    int result = client->Connect(callbackTest, isAutoConnect, transport);
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_00300 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_00400
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00400, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00400 start");
    int result = client->Disconnect();
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_00400 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_00500
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00500, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00500 start");
    int result = client->DiscoverServices();
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_00500 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_00600
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00600, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00600 start");
    UUID id = UUID::RandomUUID();
    client->GetService(id);
    client->GetService();
    HILOGI("BLE_GattDisableBleTest_00600 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_00700
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00700, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00700 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    int result = client->ReadCharacteristic(characteristic);
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_00700 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_00400
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00800, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00800 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    GattDescriptor descriptor = GattDescriptor(uuid_, permissions);
    int result = client->ReadDescriptor(descriptor);
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_00800 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_00900
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_00900, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_00900 start");
    int mtu = 17;
    int result = client->RequestBleMtuSize(mtu);
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_00900 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01000
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01000, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01000 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    bool enable = true;
    int result = client->SetNotifyCharacteristic(characteristic, enable);
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_01000 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01100
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01100, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01100 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    int result = client->WriteCharacteristic(characteristic);
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_01100 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01200
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01200, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01200 start");
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    GattDescriptor descriptor = GattDescriptor(uuid_, permissions);
    int result = client->WriteDescriptor(descriptor);
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_01200 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01300
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01300, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01300 start");
    int connPriority = 2;
    int result = client->RequestConnectionPriority(connPriority);
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_01300 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01400
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01400, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01400 start");
    int result = client->ReadRemoteRssiValue();
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_01400 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01500
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01500, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01500 start");
    int result = client->RequestFastestConn();
    EXPECT_NE(result, 0);
    HILOGI("BLE_GattDisableBleTest_01500 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01600
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01600, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01600 start");
    client->Close();
    EXPECT_NE(client, nullptr);
    HILOGI("BLE_GattDisableBleTest_01600 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01700
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01700, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01700 start");
    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34F7");
    GattService serviceOne(uuidSerPer, GattServiceType::PRIMARY);
    int ret = server->AddService(serviceOne);
    EXPECT_NE(ret, 0);
    HILOGI("BLE_GattDisableBleTest_01700 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01800
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01800, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01800 start");
    BluetoothRemoteDevice deviceBle_;
    server->CancelConnection(deviceBle_);
    HILOGI("BLE_GattDisableBleTest_01800 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_01900
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_01900, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_01900 start");
    std::list<GattService> &list = server->GetServices();
    server->RemoveGattService(list.back());
    HILOGI("BLE_GattDisableBleTest_01900 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_20000
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_20000, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_20000 start");
    server->ClearServices();
    HILOGI("BLE_GattDisableBleTest_20000 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_21000
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_21000, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_21000 start");
    BluetoothRemoteDevice deviceBle_;
    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattCharacteristic* aa = new GattCharacteristic(uuidSerPer, 1, 1);
    int res = server->NotifyCharacteristicChanged(deviceBle_, *aa, false);
    EXPECT_NE(res, 0);
    HILOGI("BLE_GattDisableBleTest_21000 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_22000
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_22000, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_22000 start");
    BluetoothRemoteDevice deviceBle_;
    string valueChrTwo = "2";
    int ret = server->SendResponse(deviceBle_,
        0,
        (int)GattStatus::GATT_SUCCESS,
        1,
        (uint8_t *)valueChrTwo.c_str(),
        valueChrTwo.size());
    EXPECT_NE(ret, 0);
    HILOGI("BLE_GattDisableBleTest_22000 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_24000
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_24000, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_24000 start");
    server->GetServices();
    HILOGI("BLE_GattDisableBleTest_24000 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_25000
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_25000, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_25000 start");
    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34F9");
    server->GetService(uuidSerPer, false);
    HILOGI("BLE_GattDisableBleTest_25000 end");
}

/*
 * @tc.number: BLE_GattDisableBleTest_26000
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(GattDisableBleTest, BLE_GattDisableBleTest_26000, TestSize.Level1)
{
    HILOGI("BLE_GattDisableBleTest_26000 start");
    server->Close();
    HILOGI("BLE_GattDisableBleTest_26000 end");
}
}  // namespace bluetooth
}  // namespace OHOS