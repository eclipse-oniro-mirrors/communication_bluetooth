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
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_pan.h"
#include "bluetooth_log.h"
#include "bluetooth_pan_observer_stub.h"
#include "bluetooth_raw_address.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
static BluetoothHost *host_;
static int32_t g_numOne = 1;
static int32_t g_numTwo = 2;

class BluetoothPanObserverStubTestMock : public BluetoothPanObserverStub {
public:
    BluetoothPanObserverStubTestMock()
    {
        HILOGI("enter");
    }
    ~BluetoothPanObserverStubTestMock() override
    {
        HILOGI("enter");
    }
    ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int state, int cause, int role) override
    {
        HILOGI("enter, state: %{public}d, cause: %{public}d", state, cause);
        return NO_ERROR;
    }
};

class BluetoothPanObserverStubTest : public testing::Test {
public:
    BluetoothPanObserverStubTest()
    {}
    ~BluetoothPanObserverStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
    BluetoothPanObserverStubTestMock panObSerTest;
};

void BluetoothPanObserverStubTest::SetUpTestCase(void)
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

void BluetoothPanObserverStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void BluetoothPanObserverStubTest::SetUp()
{}
void BluetoothPanObserverStubTest::TearDown()
{}

/*
 * @tc.number: BluetoothPanObserverStubTest_OnRemoteRequest_00100
 * @tc.name: OnRemoteRequest
 * @tc.desc: 进入case分支，return NO_ERROR
 */
HWTEST_F(BluetoothPanObserverStubTest, BluetoothPanObserverStubTest_OnRemoteRequest_00100, TestSize.Level1)
{
    HILOGI("BluetoothPanObserverStubTest_OnRemoteRequest_00100 start");
    BluetoothRawAddress addr;
    uint32_t code =
        static_cast<uint32_t>(BluetoothPanObserverInterfaceCode::COMMAND_ON_CONNECTION_STATE_CHANGED);
    MessageParcel data;
    std::u16string descriptor = BluetoothPanObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteParcelable(&addr);
    data.WriteInt32(g_numOne);
    data.WriteInt32(g_numTwo);
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(panObSerTest.OnRemoteRequest(code, data, reply, option), NO_ERROR);
    HILOGI("BluetoothPanObserverStubTest_OnRemoteRequest_00100 end");
}

/*
 * @tc.number: BluetoothPanObserverStubTest_OnRemoteRequest_00200
 * @tc.name: OnRemoteRequest
 * @tc.desc: return IPC_INVOKER_TRANSLATE_ERR
 */
HWTEST_F(BluetoothPanObserverStubTest, BluetoothPanObserverStubTest_OnRemoteRequest_00200, TestSize.Level1)
{
    HILOGI("BluetoothPanObserverStubTest_OnRemoteRequest_00200 start");
    BluetoothRawAddress addr;
    uint32_t code =
        static_cast<uint32_t>(BluetoothPanObserverInterfaceCode::COMMAND_ON_CONNECTION_STATE_CHANGED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(panObSerTest.OnRemoteRequest(code, data, reply, option), IPC_INVOKER_TRANSLATE_ERR);
    HILOGI("BluetoothPanObserverStubTest_OnRemoteRequest_00200 end");
}

/*
 * @tc.number: BluetoothPanObserverStubTest_OnRemoteRequest_00300
 * @tc.name: OnRemoteRequest
 * @tc.desc: 进入case分支，return ERR_TRANSACTION_FAILED
 */
HWTEST_F(BluetoothPanObserverStubTest, BluetoothPanObserverStubTest_OnRemoteRequest_00300, TestSize.Level1)
{
    HILOGI("BluetoothPanObserverStubTest_OnRemoteRequest_00300 start");
    BluetoothRawAddress addr;
    uint32_t code =
        static_cast<uint32_t>(BluetoothPanObserverInterfaceCode::COMMAND_ON_CONNECTION_STATE_CHANGED);
    MessageParcel data;
    std::u16string descriptor = BluetoothPanObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteInt32(g_numOne);
    data.WriteInt32(g_numTwo);
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(panObSerTest.OnRemoteRequest(code, data, reply, option), ERR_TRANSACTION_FAILED);
    HILOGI("BluetoothPanObserverStubTest_OnRemoteRequest_00300 end");
}

/*
 * @tc.number: BluetoothPanObserverStubTest_OnRemoteRequest_00400
 * @tc.name: OnRemoteRequest
 * @tc.desc: 进入default分支
 */
HWTEST_F(BluetoothPanObserverStubTest, BluetoothPanObserverStubTest_OnRemoteRequest_00400, TestSize.Level1)
{
    HILOGI("BluetoothPanObserverStubTest_OnRemoteRequest_00400 start");
    BluetoothRawAddress addr;
    uint32_t code = 99;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_NE(panObSerTest.OnRemoteRequest(code, data, reply, option), IPC_INVOKER_TRANSLATE_ERR);
    HILOGI("BluetoothPanObserverStubTest_OnRemoteRequest_00400 end");
}
} // Bluetooth
} // OHOS