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
#include "bluetooth_map_mse_observer_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
static BluetoothHost *host_;

class BluetoothMapMseObserverStubMock : public BluetoothMapMseObserverStub {
public:
    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {}
};

class BluetoothMapMseObserverStubTest : public testing::Test {
public:
    BluetoothMapMseObserverStubTest()
    {}
    ~BluetoothMapMseObserverStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};
void BluetoothMapMseObserverStubTest::SetUpTestCase(void)
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
void BluetoothMapMseObserverStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void BluetoothMapMseObserverStubTest::SetUp()
{}
void BluetoothMapMseObserverStubTest::TearDown()
{}

/*
 * @tc.number: BluetoothMapMseObserverStubTest_OnRemoteRequest_001
 * @tc.name: OnRemoteRequest
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothMapMseObserverStubTest, BluetoothMapMseObserverStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    HILOGI("BluetoothMapMseObserverStubTest_OnRemoteRequest_001 start");
    BluetoothMapMseObserverStubMock bluetoothMapMseObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothMapMseObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothMapMseObserverInterfaceCode::MSE_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
    HILOGI("BluetoothMapMseObserverStubTest_OnRemoteRequest_001 end");
}

/*
 * @tc.number: BluetoothMapMseObserverStubTest_OnRemoteRequest_002
 * @tc.name: OnRemoteRequest
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothMapMseObserverStubTest, BluetoothMapMseObserverStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    HILOGI("BluetoothMapMseObserverStubTest_OnRemoteRequest_002 start");
    BluetoothMapMseObserverStubMock bluetoothMapMseObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothMapMseObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothMapMseObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothMapMseObserverInterfaceCode::MSE_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
    HILOGI("BluetoothMapMseObserverStubTest_OnRemoteRequest_002 end");
}

/*
 * @tc.number: BluetoothMapMseObserverStubTest_OnConnectionStateChangedInner_001
 * @tc.name: OnConnectionStateChangedInner
 * @tc.desc: return BT_ERR_INTERNAL_ERROR
*/
HWTEST_F(BluetoothMapMseObserverStubTest, BluetoothMapMseObserverStubTest_OnConnectionStateChangedInner_001,
    TestSize.Level1)
{
    HILOGI("BluetoothMapMseObserverStubTest_OnConnectionStateChangedInner_001 start");
    BluetoothMapMseObserverStubMock bluetoothMapMseObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothMapMseObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteParcelable(nullptr);
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothMapMseObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothMapMseObserverInterfaceCode::MSE_ON_CONNECTION_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    // return BT_NO_ERROR
    data.WriteInterfaceToken(descriptor);
    BluetoothRawAddress addr;
    data.WriteParcelable(&addr);
    data.WriteInt32(0);
    data.WriteInt32(0);
    ret = bluetoothMapMseObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothMapMseObserverInterfaceCode::MSE_ON_CONNECTION_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("BluetoothMapMseObserverStubTest_OnConnectionStateChangedInner_001 end");
}
} // Bluetooth
} // OHOS