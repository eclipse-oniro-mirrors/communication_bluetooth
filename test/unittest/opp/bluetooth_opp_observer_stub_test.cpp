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
#include "bluetooth_errorcode.h"
#include "bluetooth_opp_observer_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
static BluetoothHost *host_;

class BluetoothOppObserverStubMock : public BluetoothOppObserverStub {
public:
    void OnReceiveIncomingFileChanged(const BluetoothIOppTransferInformation &transferInformation) override
    {}
    void OnTransferStateChanged(const BluetoothIOppTransferInformation &transferInformation) override
    {}
};

class BluetoothOppObserverStubTest : public testing::Test {
public:
    BluetoothOppObserverStubTest()
    {}
    ~BluetoothOppObserverStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};
void BluetoothOppObserverStubTest::SetUpTestCase(void)
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
void BluetoothOppObserverStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void BluetoothOppObserverStubTest::SetUp()
{}
void BluetoothOppObserverStubTest::TearDown()
{}

/*
 * @tc.number: BluetoothOppObserverStubTest_OnRemoteRequest_001
 * @tc.name: OnRemoteRequest
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothOppObserverStubTest, BluetoothOppObserverStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    HILOGI("BluetoothOppObserverStubTest_OnRemoteRequest_001 start");
    BluetoothOppObserverStubMock bluetoothOppObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothOppObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothOppObserverInterfaceCode::OPP_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
    HILOGI("BluetoothOppObserverStubTest_OnRemoteRequest_001 end");
}

/*
 * @tc.number: BluetoothOppObserverStubTest_OnRemoteRequest_002
 * @tc.name: OnRemoteRequest
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothOppObserverStubTest, BluetoothOppObserverStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    HILOGI("BluetoothOppObserverStubTest_OnRemoteRequest_002 start");
    BluetoothOppObserverStubMock bluetoothOppObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothOppObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothOppObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothOppObserverInterfaceCode::OPP_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
    HILOGI("BluetoothOppObserverStubTest_OnRemoteRequest_002 end");
}

/*
 * @tc.number: BluetoothOppObserverStubTest_OnReceiveIncomingFileChangedInner_001
 * @tc.name: OnReceiveIncomingFileChangedInner
 * @tc.desc: return BT_ERR_INTERNAL_ERROR
*/
HWTEST_F(BluetoothOppObserverStubTest, BluetoothOppObserverStubTest_OnReceiveIncomingFileChangedInner_001,
    TestSize.Level1)
{
    HILOGI("BluetoothOppObserverStubTest_OnReceiveIncomingFileChangedInner_001 start");
    BluetoothOppObserverStubMock bluetoothOppObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothOppObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothOppObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothOppObserverInterfaceCode::OPP_ON_RECEIVE_INCOMING_FILE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    data.WriteInterfaceToken(descriptor);
    BluetoothIOppTransferInformation bluetoothIOppTransferInformation;
    data.WriteParcelable(&bluetoothIOppTransferInformation);
    ret = bluetoothOppObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothOppObserverInterfaceCode::OPP_ON_RECEIVE_INCOMING_FILE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("BluetoothOppObserverStubTest_OnReceiveIncomingFileChangedInner_001 end");
}

/*
 * @tc.number: BluetoothOppObserverStubTest_OnTransferStateChangedInner_001
 * @tc.name: OnTransferStateChangedInner
 * @tc.desc: return BT_ERR_INTERNAL_ERROR
*/
HWTEST_F(BluetoothOppObserverStubTest, BluetoothOppObserverStubTest_OnTransferStateChangedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothOppObserverStubTest_OnTransferStateChangedInner_001 start");
    BluetoothOppObserverStubMock bluetoothOppObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothOppObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothOppObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothOppObserverInterfaceCode::OPP_ON_TRANSFER_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    data.WriteInterfaceToken(descriptor);
    BluetoothIOppTransferInformation bluetoothIOppTransferInformation;
    data.WriteParcelable(&bluetoothIOppTransferInformation);
    ret = bluetoothOppObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothOppObserverInterfaceCode::OPP_ON_TRANSFER_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("BluetoothOppObserverStubTest_OnTransferStateChangedInner_001 end");
}
} // Bluetooth
} // OHOS