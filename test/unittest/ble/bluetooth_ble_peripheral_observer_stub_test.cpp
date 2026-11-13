/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_raw_address.h"
#include "ipc_types.h"
#include "bluetooth_ble_peripheral_observer_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
static BluetoothHost *host_;
static int32_t transport_ = 1;

class BluetoothBlePeripheralObserverStubMock : public BluetoothBlePeripheralObserverStub {
public:
    void OnReadRemoteRssiEvent(const BluetoothRawAddress &device, int rssi, int status) override
    {}
    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device, int status, int cause,
        const std::string &causeMessage) override
    {}
    void OnAclStateChanged(const BluetoothRawAddress &device, int state, unsigned int reason) override
    {}
};

class BluetoothBlePeripheralObserverStubTest : public testing::Test {
public:
    BluetoothBlePeripheralObserverStubTest()
    {}
    ~BluetoothBlePeripheralObserverStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void BluetoothBlePeripheralObserverStubTest::SetUpTestCase(void)
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

    HILOGI("SetUpTestCase end");
}

void BluetoothBlePeripheralObserverStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void BluetoothBlePeripheralObserverStubTest::SetUp()
{}

void BluetoothBlePeripheralObserverStubTest::TearDown()
{}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_001
 * @tc.name: OnRemoteRequest
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_001,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_001 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_PERIPHERAL_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_001 end");
}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_002
 * @tc.name: OnRemoteRequest
 * @tc.desc: return IPC_STUB_UNKNOW_TRANS_ERR
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_002,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_002 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    std::u16string descriptor = BluetoothBlePeripheralObserverStub::GetDescriptor();
    MessageParcel data;
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_PERIPHERAL_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_002 end %{public}d", ret);
}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_003
 * @tc.name: OnRemoteRequest
 * @tc.desc: return TRANSACTION_ERR
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_003,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_003 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    std::u16string descriptor = BluetoothBlePeripheralObserverStub::GetDescriptor();
    MessageParcel data;
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_ON_READ_REMOTE_RSSI_EVENT),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnRemoteRequest_003 end %{public}d", ret);
}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnReadRemoteRssiEventInner_001
 * @tc.name: OnReadRemoteRssiEventInner
 * @tc.desc: return TRANSACTION_ERR
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnReadRemoteRssiEventInner_001,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnReadRemoteRssiEventInner_001 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    std::u16string descriptor = BluetoothBlePeripheralObserverStub::GetDescriptor();
    MessageParcel data;
    data.WriteInterfaceToken(descriptor);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_ON_READ_REMOTE_RSSI_EVENT),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnReadRemoteRssiEventInner_001 end %{public}d", ret);
}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnReadRemoteRssiEventInner_002
 * @tc.name: OnReadRemoteRssiEventInner
 * @tc.desc: return NO_ERROR
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnReadRemoteRssiEventInner_002,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnReadRemoteRssiEventInner_002 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    std::u16string descriptor = BluetoothBlePeripheralObserverStub::GetDescriptor();
    MessageParcel data;
    data.WriteInterfaceToken(descriptor);
    BluetoothRawAddress item;
    data.WriteParcelable(&item);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_ON_READ_REMOTE_RSSI_EVENT),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnReadRemoteRssiEventInner_002 end %{public}d", ret);
}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnPairStatusChangedInner_001
 * @tc.name: OnPairStatusChangedInner
 * @tc.desc: return TRANSACTION_ERR
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnPairStatusChangedInner_001,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnPairStatusChangedInner_001 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    std::u16string descriptor = BluetoothBlePeripheralObserverStub::GetDescriptor();
    MessageParcel data;
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(transport_);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_PAIR_STATUS_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnPairStatusChangedInner_001 end %{public}d", ret);
}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnPairStatusChangedInner_002
 * @tc.name: OnPairStatusChangedInner
 * @tc.desc: return NO_ERROR
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnPairStatusChangedInner_002,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnPairStatusChangedInner_002 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    std::u16string descriptor = BluetoothBlePeripheralObserverStub::GetDescriptor();
    MessageParcel data;
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(transport_);
    BluetoothRawAddress item;
    data.WriteParcelable(&item);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_PAIR_STATUS_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnPairStatusChangedInner_002 end %{public}d", ret);
}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnAclStateChangedInner_001
 * @tc.name: OnAclStateChangedInner
 * @tc.desc: return TRANSACTION_ERR
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnAclStateChangedInner_001,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnAclStateChangedInner_001 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    std::u16string descriptor = BluetoothBlePeripheralObserverStub::GetDescriptor();
    MessageParcel data;
    data.WriteInterfaceToken(descriptor);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_ACL_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnAclStateChangedInner_001 end %{public}d", ret);
}

/*
 * @tc.number: BluetoothBlePeripheralObserverStubTest_OnAclStateChangedInner_002
 * @tc.name: OnAclStateChangedInner
 * @tc.desc: return NO_ERROR
*/
HWTEST_F(BluetoothBlePeripheralObserverStubTest, BluetoothBlePeripheralObserverStubTest_OnAclStateChangedInner_002,
    TestSize.Level1)
{
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnAclStateChangedInner_002 start");
    BluetoothBlePeripheralObserverStubMock bluetoothBlePeripheralObserverStubTest;
    std::u16string descriptor = BluetoothBlePeripheralObserverStub::GetDescriptor();
    MessageParcel data;
    data.WriteInterfaceToken(descriptor);
    BluetoothRawAddress item;
    data.WriteParcelable(&item);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothBlePeripheralObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothBlePeripheralObserverInterfaceCode::BLE_ACL_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothBlePeripheralObserverStubTest_OnAclStateChangedInner_002 end %{public}d", ret);
}
} // Bluetooth
} // OHOS