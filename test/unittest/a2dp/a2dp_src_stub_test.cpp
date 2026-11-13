/*
 * Copyright (c) 2012 Huawei Device Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file EXPECT in compliance with the License.
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
#include "bluetooth_a2dp_src_stub.h"
#include "bluetooth_a2dp_source_server.h"
#include "bluetooth_log.h"
#include "bluetooth_host.h"


using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

BluetoothA2dpSourceServer a2dpSrcServer_;
BluetoothA2dpSourceServer *a2dpSrcServer;
static BluetoothHost *host_;
const int TIME = 2;

class A2dpSrcStubTest : public testing::Test {
public:
    A2dpSrcStubTest()
    {}
    ~A2dpSrcStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void A2dpSrcStubTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase");

    host_ = &BluetoothHost::GetDefaultHost();

    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBrEnable = %{public}d", isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    a2dpSrcServer = &a2dpSrcServer_;
    HILOGI("SetUpTestCase end");
}

void A2dpSrcStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
}

void A2dpSrcStubTest::SetUp()
{
}

void A2dpSrcStubTest::TearDown()
{
}

/**
 * @tc.number: OnRemoteRequest_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(A2dpSrcStubTest, A2dpSrcStub_ModuleTest_OnRemoteRequest_00100, TestSize.Level1)
{
    HILOGI("A2dpSrcStub_ModuleTest_OnRemoteRequest_00100 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int ret = a2dpSrcServer->OnRemoteRequest((uint32_t)BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_CONNECT,
        data, reply, option);
    for (BluetoothA2dpSrcInterfaceCode code = BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_CONNECT;
            code <= BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_RENDER_POSITION;
            code = (BluetoothA2dpSrcInterfaceCode)(code + 1)) {
        data.WriteInterfaceToken(BluetoothA2dpSrcStub::GetDescriptor());
        HILOGI("A2dpSrcStub_ModuleTest_OnRemoteRequest_00100 code:%d", code);
        ret = a2dpSrcServer->OnRemoteRequest((uint32_t)code, data, reply, option);
    }
    EXPECT_EQ(ret, RET_NO_ERROR);
    HILOGI("A2dpSrcStub_ModuleTest_OnRemoteRequest_00100 end ret:%d", ret);
}

/**
 * @tc.number: OnRemoteRequest_002
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(A2dpSrcStubTest, A2dpSrcStub_ModuleTest_OnRemoteRequest_00200, TestSize.Level1)
{
    HILOGI("A2dpSrcStub_ModuleTest_OnRemoteRequest_00200 start");
    int ret;
    for (BluetoothA2dpSrcInterfaceCode code = BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_CONNECT;
            code <= BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_RENDER_POSITION;
            code = (BluetoothA2dpSrcInterfaceCode)(code + 1)) {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        data.WriteInterfaceToken(BluetoothA2dpSrcStub::GetDescriptor());
        data.WriteString("11:22:33:44:55:66");
        HILOGI("A2dpSrcStub_ModuleTest_OnRemoteRequest_00200 code:%d", code);
        ret = a2dpSrcServer->OnRemoteRequest((uint32_t)code, data, reply, option);
    }
    EXPECT_EQ(ret, RET_NO_ERROR);
    HILOGI("A2dpSrcStub_ModuleTest_OnRemoteRequest_00200 end ret:%d", ret);
}

}
}