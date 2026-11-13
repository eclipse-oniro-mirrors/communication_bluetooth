/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "avrcp_ct_define.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class AvrcpCtDefineTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.number: AvrcpCtDefineTest_ConnectState_001
 * @tc.name: ConnectState
 * @tc.desc: Test AVRCP CT connection state enum values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_ConnectState_001, TestSize.Level1)
{
    EXPECT_EQ(AVRCP_CT_STATE_DISCONNECTED, 0);
    EXPECT_EQ(AVRCP_CT_STATE_CONNECTING, 1);
    EXPECT_EQ(AVRCP_CT_STATE_CONNECTED, 2);
    EXPECT_EQ(AVRCP_CT_STATE_DISCONNECTING, 4);
}

/*
 * @tc.number: AvrcpCtDefineTest_CmdMsg_001
 * @tc.name: CmdMsg
 * @tc.desc: Test AVRCP CT command message enum values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_CmdMsg_001, TestSize.Level1)
{
    EXPECT_EQ(AVRCP_MSG_PASSTHRU, 302);
    EXPECT_EQ(AVRCP_MSG_SET_REPEAT_SHUFFLE, 303);
}

/*
 * @tc.number: AvrcpCtDefineTest_Cmd_001
 * @tc.name: Cmd
 * @tc.desc: Test pass through command id enum values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_Cmd_001, TestSize.Level1)
{
    EXPECT_EQ(PASS_THRU_CMD_ID_VOL_UP, 0x41);
    EXPECT_EQ(PASS_THRU_CMD_ID_VOL_DOWN, 0x42);
    EXPECT_EQ(PASS_THRU_CMD_ID_PLAY, 0x44);
    EXPECT_EQ(PASS_THRU_CMD_ID_STOP, 0x45);
    EXPECT_EQ(PASS_THRU_CMD_ID_PAUSE, 0x46);
    EXPECT_EQ(PASS_THRU_CMD_ID_REWIND, 0x48);
    EXPECT_EQ(PASS_THRU_CMD_ID_FF, 0x49);
    EXPECT_EQ(PASS_THRU_CMD_ID_FORWARD, 0x4B);
    EXPECT_EQ(PASS_THRU_CMD_ID_BACKWARD, 0x4C);
}

/*
 * @tc.number: AvrcpCtDefineTest_InternalMsg_001
 * @tc.name: InternalMsg
 * @tc.desc: Test AVRCP CT internal message enum values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_InternalMsg_001, TestSize.Level1)
{
    EXPECT_EQ(STACK_CONNECT, 0);
    EXPECT_EQ(STACK_DISCONNECT, 1);
    EXPECT_EQ(DEVICE_STATE_ACTIVE, 2);
    EXPECT_EQ(DEVICE_STATE_INACTIVE, 3);
    EXPECT_EQ(AVRCP_CT_GET_COVER_ART_PSM_EVT, 4);
    EXPECT_EQ(CLEANUP, 5);
    EXPECT_EQ(ACTIVE_DEVICE_CHANGE, 3);
}

/*
 * @tc.number: AvrcpCtDefineTest_Event_001
 * @tc.name: Event
 * @tc.desc: Test AVRCP CT event enum values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_Event_001, TestSize.Level1)
{
    EXPECT_EQ(AVRCP_CT_CONNECT_EVT, 10);
    EXPECT_EQ(AVRCP_CT_DISCONNECT_EVT, 11);
    EXPECT_EQ(AVRCP_CT_SET_ABS_VOL_EVT, 20);
    EXPECT_EQ(AVRCP_CT_REGISTER_ABS_VOL_EVT, 21);
}

/*
 * @tc.number: AvrcpCtDefineTest_ProcessMsg_001
 * @tc.name: ProcessMsg
 * @tc.desc: Test AVRCP CT process message enum values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_ProcessMsg_001, TestSize.Level1)
{
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_SET_ABS_VOL_CMD, 203);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_REGISTER_ABS_VOL_NOTIFICATION, 204);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_TRACK_CHANGED, 205);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_PLAY_POS_CHANGED, 206);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_PLAY_STATUS_CHANGED, 207);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_VOLUME_CHANGED_NOTIFICATION, 208);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_ADDRESSED_PLAYER_CHANGED, 215);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_SUPPORTED_APPLICATION_SETTINGS, 217);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_CURRENT_APPLICATION_SETTINGS, 218);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_AVAILABLE_PLAYER_CHANGED, 219);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_RECEIVED_COVER_ART_PSM, 220);
    EXPECT_EQ(AVRCP_CT_MSG_INTERNAL_ABS_VOL_TIMEOUT, 102);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_IMAGE_DOWNLOADED, 400);
    EXPECT_EQ(AVRCP_CT_MSG_PROCESS_PREPARE, 800);
}

/*
 * @tc.number: AvrcpCtDefineTest_VolumeParam_001
 * @tc.name: VolumeParam
 * @tc.desc: Test absolute volume parameter values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_VolumeParam_001, TestSize.Level1)
{
    EXPECT_EQ(ABS_VOL_TIMEOUT_MILLIS, 1000);
    EXPECT_EQ(ABS_VOL_BASE, 127);
    EXPECT_EQ(VOLUME_CHANGE_IGNORED_TIMEDIFF_MS, 300);
}

/*
 * @tc.number: AvrcpCtDefineTest_RspTypeAndKeyState_001
 * @tc.name: RspTypeAndKeyState
 * @tc.desc: Test notification response type and key state enum values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_RspTypeAndKeyState_001, TestSize.Level1)
{
    EXPECT_EQ(NOTIFICATION_RSP_TYPE_INTERIM, 0x00);
    EXPECT_EQ(NOTIFICATION_RSP_TYPE_CHANGED, 0x01);
    EXPECT_EQ(KEY_STATE_PRESSED, 0);
    EXPECT_EQ(KEY_STATE_RELEASED, 1);
}

/*
 * @tc.number: AvrcpCtDefineTest_PlayerAppSetting_001
 * @tc.name: PlayerAppSetting
 * @tc.desc: Test player application setting enum values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_PlayerAppSetting_001, TestSize.Level1)
{
    EXPECT_EQ(AVRCP_EQUALIZER_STATUS, 0x01);
    EXPECT_EQ(AVRCP_REPEAT_STATUS, 0x02);
    EXPECT_EQ(AVRCP_SHUFFLE_STATUS, 0x03);
    EXPECT_EQ(AVRCP_SCAN_STATUS, 0x04);

    EXPECT_EQ(AVRCP_REPEAT_OFF, 0x01);
    EXPECT_EQ(AVRCP_REPEAT_SINGLE_TRACK, 0x02);
    EXPECT_EQ(AVRCP_REPEAT_ALL_TRACK, 0x03);
    EXPECT_EQ(AVRCP_REPEAT_GROUP, 0x04);

    EXPECT_EQ(AVRCP_SHUFFLE_OFF, 0x01);
    EXPECT_EQ(AVRCP_SHUFFLE_ALL_TRACK, 0x02);
    EXPECT_EQ(AVRCP_SHUFFLE_GROUP, 0x03);
}

/*
 * @tc.number: AvrcpCtDefineTest_ConnectFlag_001
 * @tc.name: ConnectFlag
 * @tc.desc: Test AvrcpCtConnectFlag struct default values
 */
HWTEST_F(AvrcpCtDefineTest, AvrcpCtDefineTest_ConnectFlag_001, TestSize.Level1)
{
    AvrcpCtConnectFlag flag{};
    EXPECT_FALSE(flag.remoteCtlConnected);
    EXPECT_FALSE(flag.browsingConnected);

    flag.remoteCtlConnected = true;
    flag.browsingConnected = true;
    EXPECT_TRUE(flag.remoteCtlConnected);
    EXPECT_TRUE(flag.browsingConnected);
}
} // namespace bluetooth
} // namespace OHOS
