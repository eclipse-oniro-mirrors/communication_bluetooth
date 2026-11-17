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
#include "bluetooth_host_observer_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
static BluetoothHost *host_;

class BluetoothHostObserverStubMock : public BluetoothHostObserverStub {
public:
    void OnStateChanged(int32_t transport, int32_t status) override
    {}
    void OnDiscoveryStateChanged(int32_t status) override
    {}
    void OnDiscoveryResult(const BluetoothRawAddress &device, int rssi,
        const std::string deviceName, int deviceClass, int transport) override
    {}
    void OnPairRequested(const int32_t transport, const BluetoothRawAddress &device) override
    {}
    void OnPairConfirmed(
        const int32_t transport, const BluetoothRawAddress &device, int reqType, int number) override
    {}
    void OnScanModeChanged(int32_t mode) override
    {}
    void OnDeviceNameChanged(const std::string &deviceName) override
    {}
    void OnDeviceAddrChanged(const std::string &address) override
    {}
    void OnBluetoothStateChanged(int32_t state) override
    {}
    void OnRefusePolicyChanged(const int32_t pid, const int64_t prohibitedSecondsTime) override
    {}
    void OnBluetoothSwitchAction(int32_t action, const std::string &callingName) override
    {}
};

class BluetoothHostObserverStubTest : public testing::Test {
public:
    BluetoothHostObserverStubTest()
    {}
    ~BluetoothHostObserverStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};
void BluetoothHostObserverStubTest::SetUpTestCase(void)
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
void BluetoothHostObserverStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void BluetoothHostObserverStubTest::SetUp()
{}
void BluetoothHostObserverStubTest::TearDown()
{}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnRemoteRequest_001
 * @tc.name: OnRemoteRequest
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnRemoteRequest_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
    HILOGI("BluetoothHostObserverStubTest_OnRemoteRequest_001 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnRemoteRequest_002
 * @tc.name: OnRemoteRequest
 * @tc.desc: return IPC_STUB_UNKNOW_TRANS_ERR
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnRemoteRequest_002 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
    HILOGI("BluetoothHostObserverStubTest_OnRemoteRequest_002 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnStateChangedInner_001
 * @tc.name: OnStateChangedInner
 * @tc.desc: return NO_ERROR
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnStateChangedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnStateChangedInner_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(0);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_STATE_CHANGE),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothHostObserverStubTest_OnStateChangedInner_001 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnDiscoveryStateChangedInner_001
 * @tc.name: OnDiscoveryStateChangedInner
 * @tc.desc: return NO_ERROR
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnDiscoveryStateChangedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnDiscoveryStateChangedInner_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DISCOVERY_STATE_CHANGE),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothHostObserverStubTest_OnDiscoveryStateChangedInner_001 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnDiscoveryResultInner_001
 * @tc.name: OnDiscoveryResultInner
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnDiscoveryResultInner_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnDiscoveryResultInner_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DISCOVERY_RESULT),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);

    // return NO_ERROR
    data.WriteInterfaceToken(descriptor);
    BluetoothRawAddress device;
    data.WriteParcelable(&device);
    data.WriteInt32(0);
    std::string deviceName = "test";
    data.WriteString(deviceName);
    data.WriteInt32(0);
    ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DISCOVERY_RESULT),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothHostObserverStubTest_OnDiscoveryResultInner_001 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnPairRequestedInner_001
 * @tc.name: OnPairRequestedInner
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnPairRequestedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnPairRequestedInner_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(0);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_PAIR_REQUESTED),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);

    // return NO_ERROR
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(0);
    BluetoothRawAddress device;
    data.WriteParcelable(&device);
    ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_PAIR_REQUESTED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothHostObserverStubTest_OnPairRequestedInner_001 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnPairConfirmedInner_001
 * @tc.name: OnPairConfirmedInner
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnPairConfirmedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnPairConfirmedInner_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(0);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_PAIR_CONFIRMED),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);

    // return NO_ERROR
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(0);
    BluetoothRawAddress device;
    data.WriteParcelable(&device);
    data.WriteInt32(0);
    data.WriteInt32(0);
    ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_PAIR_CONFIRMED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothHostObserverStubTest_OnPairConfirmedInner_001 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnScanModeChangedInner_001
 * @tc.name: OnScanModeChangedInner
 * @tc.desc: return NO_ERROR
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnScanModeChangedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnScanModeChangedInner_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_SCAN_MODE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothHostObserverStubTest_OnScanModeChangedInner_001 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnDeviceNameChangedInner_001
 * @tc.name: OnDeviceNameChangedInner
 * @tc.desc: return NO_ERROR
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnDeviceNameChangedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnDeviceNameChangedInner_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    std::string deviceName = "test";
    data.WriteString(deviceName);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DEVICE_NAME_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothHostObserverStubTest_OnDeviceNameChangedInner_001 end");
}

/*
 * @tc.number: BluetoothHostObserverStubTest_OnDeviceAddrChangedInner_001
 * @tc.name: OnDeviceAddrChangedInner
 * @tc.desc: return NO_ERROR
*/
HWTEST_F(BluetoothHostObserverStubTest, BluetoothHostObserverStubTest_OnDeviceAddrChangedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothHostObserverStubTest_OnDeviceAddrChangedInner_001 start");
    BluetoothHostObserverStubMock bluetoothHostObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothHostObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    std::string address = "test";
    data.WriteString(address);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothHostObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DEVICE_NAME_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothHostObserverStubTest_OnDeviceAddrChangedInner_001 end");
}
} // Bluetooth
} // OHOS