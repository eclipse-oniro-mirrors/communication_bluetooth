/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "bluetooth_a2dp_src_stub.h"
#include "ipc_types.h"
#include "parcel_bt_uuid.h"
#include "raw_address.h"
#include "bluetooth_log.h"
#include "bluetooth_a2dp_source_server.h"
#include "btcommon/timer_manager.h"

#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
BluetoothA2dpSourceServer source;
BluetoothA2dpSourceServer *source_ = &source;

class BluetoothA2dpSrcStubTest : public testing::Test {
public:
    BluetoothA2dpSrcStubTest() {}
    ~BluetoothA2dpSrcStubTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BluetoothA2dpSrcStubTest::SetUpTestCase(void) {}

void BluetoothA2dpSrcStubTest::TearDownTestCase(void) {
    utility::CloseAllTimer();
}

void BluetoothA2dpSrcStubTest::SetUp() {}

void BluetoothA2dpSrcStubTest::TearDown() {}

/*
 * @tc.number: BluetoothA2dpSrcStub001
 * @tc.name: OnRemoteRequest
 * @tc.desc: OnRemoteRequest
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_OnRemoteRequest_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_OnRemoteRequest_001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int ret = source_->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(BT_ERR_IPC_TRANS_FAILED, ret);
    HILOGI("bluetooth_a2dp_src_stub_OnRemoteRequest_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub002
 * @tc.name: ConnectInner
 * @tc.desc: ConnectInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_ConnectInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_ConnectInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = source_->ConnectInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_ConnectInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub003
 * @tc.name: ConnectInner
 * @tc.desc: ConnectInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_ConnectInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_ConnectInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->ConnectInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_ConnectInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub004
 * @tc.name: DisconnectInner
 * @tc.desc: DisconnectInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_DisconnectInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_DisconnectInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->DisconnectInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_DisconnectInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub005
 * @tc.name: DisconnectInner
 * @tc.desc: DisconnectInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_DisconnectInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_DisconnectInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->DisconnectInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_DisconnectInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub006
 * @tc.name: RegisterObserverInner
 * @tc.desc: RegisterObserverInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_RegisterObserverInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_RegisterObserverInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    source_->RegisterObserverInner(data, reply);
    EXPECT_NE(source_, nullptr);
    HILOGI("bluetooth_a2dp_src_stub_RegisterObserverInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub007
 * @tc.name: DeregisterObserverInner
 * @tc.desc: DeregisterObserverInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_DeregisterObserverInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_DeregisterObserverInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    source_->DeregisterObserverInner(data, reply);
    EXPECT_NE(source_, nullptr);
    HILOGI("bluetooth_a2dp_src_stub_DeregisterObserverInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub008
 * @tc.name: GetDevicesByStatesInner
 * @tc.desc: GetDevicesByStatesInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetDevicesByStatesInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetDevicesByStatesInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetDevicesByStatesInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetDevicesByStatesInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub009
 * @tc.name: GetDeviceStateInner
 * @tc.desc: GetDeviceStateInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetDeviceStateInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetDeviceStateInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetDeviceStateInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetDeviceStateInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub010
 * @tc.name: GetDeviceStateInner
 * @tc.desc: GetDeviceStateInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetDeviceStateInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetDeviceStateInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->GetDeviceStateInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetDeviceStateInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub011
 * @tc.name: GetPlayingStateInner
 * @tc.desc: GetPlayingStateInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetPlayingStateInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetPlayingStateInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetPlayingStateInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetPlayingStateInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub012
 * @tc.name: GetPlayingStateInner
 * @tc.desc: GetPlayingStateInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetPlayingStateInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetPlayingStateInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->GetPlayingStateInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetPlayingStateInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub013
 * @tc.name: SetConnectStrategyInner
 * @tc.desc: SetConnectStrategyInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SetConnectStrategyInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SetConnectStrategyInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->SetConnectStrategyInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_SetConnectStrategyInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub014
 * @tc.name: SetConnectStrategyInner
 * @tc.desc: SetConnectStrategyInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SetConnectStrategyInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SetConnectStrategyInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->SetConnectStrategyInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_SetConnectStrategyInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub015
 * @tc.name: GetConnectStrategyInner
 * @tc.desc: GetConnectStrategyInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetConnectStrategyInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetConnectStrategyInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetConnectStrategyInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetConnectStrategyInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub016
 * @tc.name: GetConnectStrategyInner
 * @tc.desc: GetConnectStrategyInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetConnectStrategyInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetConnectStrategyInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->GetConnectStrategyInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetConnectStrategyInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub017
 * @tc.name: SetActiveSinkDeviceInner
 * @tc.desc: SetActiveSinkDeviceInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SetActiveSinkDeviceInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SetActiveSinkDeviceInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->SetActiveSinkDeviceInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_SetActiveSinkDeviceInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub018
 * @tc.name: SetActiveSinkDeviceInner
 * @tc.desc: SetActiveSinkDeviceInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SetActiveSinkDeviceInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SetActiveSinkDeviceInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->SetActiveSinkDeviceInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_SetActiveSinkDeviceInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub019
 * @tc.name: GetActiveSinkDeviceInner
 * @tc.desc: GetActiveSinkDeviceInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetActiveSinkDeviceInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetActiveSinkDeviceInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetActiveSinkDeviceInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetActiveSinkDeviceInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub020
 * @tc.name: GetCodecStatusInner
 * @tc.desc: GetCodecStatusInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetCodecStatusInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetCodecStatusInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetCodecStatusInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetCodecStatusInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub0121
 * @tc.name: GetCodecStatusInner
 * @tc.desc: GetCodecStatusInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetCodecStatusInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetCodecStatusInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->GetCodecStatusInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetCodecStatusInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub022
 * @tc.name: SetCodecPreferenceInner
 * @tc.desc: SetCodecPreferenceInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SetCodecPreferenceInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SetCodecPreferenceInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->SetCodecPreferenceInner(data, reply);
    EXPECT_NE(0, ret);
    HILOGI("bluetooth_a2dp_src_stub_SetCodecPreferenceInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub023
 * @tc.name: SetCodecPreferenceInner
 * @tc.desc: SetCodecPreferenceInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SetCodecPreferenceInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SetCodecPreferenceInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->SetCodecPreferenceInner(data, reply);
    EXPECT_NE(0, ret);
    HILOGI("bluetooth_a2dp_src_stub_SetCodecPreferenceInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub024
 * @tc.name: SwitchOptionalCodecsInner
 * @tc.desc: SwitchOptionalCodecsInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SwitchOptionalCodecsInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SwitchOptionalCodecsInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->SwitchOptionalCodecsInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_SwitchOptionalCodecsInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub025
 * @tc.name: SwitchOptionalCodecsInner
 * @tc.desc: SwitchOptionalCodecsInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SwitchOptionalCodecsInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SwitchOptionalCodecsInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->SwitchOptionalCodecsInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_SwitchOptionalCodecsInner_002 end");
}


/*
 * @tc.number: BluetoothA2dpSrcStub026
 * @tc.name: GetOptionalCodecsSupportStateInner
 * @tc.desc: GetOptionalCodecsSupportStateInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetOptionalCodecsSupportStateInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetOptionalCodecsSupportStateInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetOptionalCodecsSupportStateInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetOptionalCodecsSupportStateInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub027
 * @tc.name: GetOptionalCodecsSupportStateInner
 * @tc.desc: GetOptionalCodecsSupportStateInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetOptionalCodecsSupportStateInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetOptionalCodecsSupportStateInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->GetOptionalCodecsSupportStateInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetOptionalCodecsSupportStateInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub028
 * @tc.name: StartPlayingInner
 * @tc.desc: StartPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_StartPlayingInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_StartPlayingInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->StartPlayingInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_StartPlayingInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub029
 * @tc.name: StartPlayingInner
 * @tc.desc: StartPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_StartPlayingInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_StartPlayingInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->StartPlayingInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_StartPlayingInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub030
 * @tc.name: SuspendPlayingInner
 * @tc.desc: SuspendPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SuspendPlayingInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SuspendPlayingInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->SuspendPlayingInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_SuspendPlayingInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub031
 * @tc.name: SuspendPlayingInner
 * @tc.desc: SuspendPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_SuspendPlayingInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_SuspendPlayingInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->SuspendPlayingInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_SuspendPlayingInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub032
 * @tc.name: StopPlayingInner
 * @tc.desc: StopPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_StopPlayingInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_StopPlayingInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->StopPlayingInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_StopPlayingInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub033
 * @tc.name: StopPlayingInner
 * @tc.desc: StopPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_StopPlayingInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_StopPlayingInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    BluetoothRemoteDevice device;
    data.WriteString(device.GetDeviceAddr());
    int ret = source_->StopPlayingInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_StopPlayingInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub034
 * @tc.name: WriteFrameInner
 * @tc.desc: WriteFrameInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_WriteFrameInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_WriteFrameInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->WriteFrameInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_WriteFrameInner_001 end");
}


/*
 * @tc.number: BluetoothA2dpSrcStub035
 * @tc.name: GetRenderPositionInner
 * @tc.desc: GetRenderPositionInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetRenderPositionInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetRenderPositionInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetRenderPositionInner(data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetRenderPositionInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub036
 * @tc.name: IsValidAddress
 * @tc.desc: string addr is not empty
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_IsValidAddress_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_IsValidAddress_001 start");
    std::string addr("00:00:00:00:00:00");
    bool ret = source_->IsValidAddress(addr);
    ASSERT_TRUE(ret);
    HILOGI("bluetooth_a2dp_src_stub_IsValidAddress_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub037
 * @tc.name: IsValidAddress
 * @tc.desc: string addr is empty
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_IsValidAddress_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_IsValidAddress_002 start");
    std::string addr;
    bool ret = source_->IsValidAddress(addr);
    ASSERT_FALSE(ret);
    HILOGI("bluetooth_a2dp_src_stub_IsValidAddress_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub038
 * @tc.name: GetCodecPreferenceInner
 * @tc.desc: GetCodecPreferenceInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetCodecPreferenceInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetCodecPreferenceInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = source_->GetCodecPreferenceInner(data, reply);
    EXPECT_NE(0, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetCodecPreferenceInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub039
 * @tc.name: OffloadStartPlayingInner
 * @tc.desc: OffloadStartPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_OffloadStartPlayingInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_OffloadStartPlayingInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    std::string addr("00:00:00:00:00:00");
    data.WriteString(addr);
    int ret = source_->OffloadStartPlayingInner(data, reply);
    EXPECT_EQ(BT_ERR_INVALID_PARAM, ret);
    HILOGI("bluetooth_a2dp_src_stub_OffloadStartPlayingInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub040
 * @tc.name: OffloadStartPlayingInner
 * @tc.desc: OffloadStartPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_OffloadStartPlayingInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_OffloadStartPlayingInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    data.WriteInt32(0);
    int ret = source_->OffloadStartPlayingInner(data, reply);
    EXPECT_EQ(BT_ERR_INVALID_PARAM, ret);
    HILOGI("bluetooth_a2dp_src_stub_OffloadStartPlayingInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub041
 * @tc.name: OffloadStartPlayingInner
 * @tc.desc: OffloadStartPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_OffloadStartPlayingInner_003, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_OffloadStartPlayingInner_003 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    data.WriteInt32(33);
    int ret = source_->OffloadStartPlayingInner(data, reply);
    EXPECT_EQ(BT_ERR_INVALID_PARAM, ret);
    HILOGI("bluetooth_a2dp_src_stub_OffloadStartPlayingInner_003 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub042
 * @tc.name: OffloadStartPlayingInner
 * @tc.desc: OffloadStartPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_OffloadStartPlayingInner_004, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_OffloadStartPlayingInner_004 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    data.WriteInt32(1);
    data.WriteInt32(1);
    int ret = source_->OffloadStartPlayingInner(data, reply);
    EXPECT_EQ(BT_NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_OffloadStartPlayingInner_004 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub043
 * @tc.name: OffloadStopPlayingInner
 * @tc.desc: OffloadStopPlayingInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_OffloadStopPlayingInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_OffloadStopPlayingInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    data.WriteInt32(1);
    data.WriteInt32(1);
    int ret = source_->OffloadStopPlayingInner(data, reply);
    EXPECT_EQ(BT_NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_OffloadStopPlayingInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub044
 * @tc.name: A2dpOffloadSessionPathRequestInner
 * @tc.desc: A2dpOffloadSessionPathRequestInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    std::string addr("00:00:00:00:00:00");
    data.WriteString(addr);
    int ret = source_->A2dpOffloadSessionPathRequestInner(data, reply);
    EXPECT_EQ(BT_NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub045
 * @tc.name: A2dpOffloadSessionPathRequestInner
 * @tc.desc: A2dpOffloadSessionPathRequestInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_002 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    data.WriteInt32(35);
    int ret = source_->A2dpOffloadSessionPathRequestInner(data, reply);
    EXPECT_EQ(BT_ERR_INVALID_PARAM, ret);
    HILOGI("bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub046
 * @tc.name: A2dpOffloadSessionPathRequestInner
 * @tc.desc: A2dpOffloadSessionPathRequestInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_003, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_003 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(0);
    data.WriteInt32(1);
    data.WriteInt32(0);
    int ret = source_->A2dpOffloadSessionPathRequestInner(data, reply);
    EXPECT_EQ(BT_NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_A2dpOffloadSessionPathRequestInner_003 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub047
 * @tc.name: GetOffloadCodecStatusInner
 * @tc.desc: GetOffloadCodecStatusInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetOffloadCodecStatusInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetOffloadCodecStatusInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    int ret = source_->GetOffloadCodecStatusInner(data, reply);
    EXPECT_EQ(BT_NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetOffloadCodecStatusInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub048
 * @tc.name: EnableAutoPlayInner
 * @tc.desc: EnableAutoPlayInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_EnableAutoPlayInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_EnableAutoPlayInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    int ret = source_->EnableAutoPlayInner(data, reply);
    EXPECT_EQ(BT_NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_EnableAutoPlayInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub049
 * @tc.name: DisableAutoPlayInner
 * @tc.desc: DisableAutoPlayInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_DisableAutoPlayInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_DisableAutoPlayInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    data.WriteInt32(3000);
    int ret = source_->DisableAutoPlayInner(data, reply);
    EXPECT_EQ(BT_NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_DisableAutoPlayInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub050
 * @tc.name: GetAutoPlayDisabledDurationInner
 * @tc.desc: GetAutoPlayDisabledDurationInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetAutoPlayDisabledDurationInner_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetAutoPlayDisabledDurationInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    int ret = source_->GetAutoPlayDisabledDurationInner(data, reply);
    EXPECT_EQ(BT_NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetAutoPlayDisabledDurationInner_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcStub051
 * @tc.name: GetRenderPositionInner
 * @tc.desc: GetRenderPositionInner
 */
HWTEST_F(BluetoothA2dpSrcStubTest, bluetooth_a2dp_src_stub_GetRenderPositionInner_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_stub_GetRenderPositionInner_002 start");
    MessageParcel data;
    RawAddress device = RawAddress("11:22:33:44:55:66");
    data.WriteString(device.GetAddress());
    data.WriteInt32(1);
    data.WriteInt32(1);
    MessageParcel reply;
    int ret = source_->GetRenderPositionInner(data, reply);
    EXPECT_EQ(NO_ERROR, ret);
    HILOGI("bluetooth_a2dp_src_stub_GetRenderPositionInner_002 end");
}
}
}
