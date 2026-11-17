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
#include <memory>
#include <string>

#include "avrcp_ct_define.h"
#include "bt_rc.h"
#include "avrcp_ct_message.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class AvrcpCtMessageTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.number: AvrcpCtMessageTest_Constructor_001
 * @tc.name: Constructor with default arguments
 * @tc.desc: Test AvrcpCtMessage constructor with only what
 */
HWTEST_F(AvrcpCtMessageTest, AvrcpCtMessageTest_Constructor_001, TestSize.Level1)
{
    AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU);
    EXPECT_EQ(msg.what_, AVRCP_MSG_PASSTHRU);
    EXPECT_EQ(msg.arg1_, 0);
    EXPECT_EQ(msg.arg2_, nullptr);
}

/*
 * @tc.number: AvrcpCtMessageTest_Constructor_002
 * @tc.name: Constructor with all arguments
 * @tc.desc: Test AvrcpCtMessage constructor with what/arg1/arg2
 */
HWTEST_F(AvrcpCtMessageTest, AvrcpCtMessageTest_Constructor_002, TestSize.Level1)
{
    int value = 42;
    void *arg2 = &value;
    AvrcpCtMessage msg(AVRCP_MSG_SET_REPEAT_SHUFFLE, PASS_THRU_CMD_ID_PLAY, arg2);
    EXPECT_EQ(msg.what_, AVRCP_MSG_SET_REPEAT_SHUFFLE);
    EXPECT_EQ(msg.arg1_, PASS_THRU_CMD_ID_PLAY);
    EXPECT_EQ(msg.arg2_, arg2);
}

/*
 * @tc.number: AvrcpCtMessageTest_DefaultFields_001
 * @tc.name: Default extension fields
 * @tc.desc: Test AvrcpCtMessage extension fields default values
 */
HWTEST_F(AvrcpCtMessageTest, AvrcpCtMessageTest_DefaultFields_001, TestSize.Level1)
{
    AvrcpCtMessage msg(AVRCP_CT_CONNECT_EVT);
    EXPECT_EQ(msg.eventThreeParameter_, 0);
    EXPECT_FALSE(msg.rcConnect);
    EXPECT_TRUE(msg.uuid_.empty());
    EXPECT_TRUE(msg.dev_.empty());
    EXPECT_EQ(msg.track_, nullptr);
    EXPECT_EQ(msg.appSettingCount_, 0);
    EXPECT_FALSE(msg.connectFlag_.remoteCtlConnected);
    EXPECT_FALSE(msg.connectFlag_.browsingConnected);
}

/*
 * @tc.number: AvrcpCtMessageTest_SetFields_001
 * @tc.name: Set extension fields
 * @tc.desc: Test AvrcpCtMessage extension fields assignment
 */
HWTEST_F(AvrcpCtMessageTest, AvrcpCtMessageTest_SetFields_001, TestSize.Level1)
{
    AvrcpCtMessage msg(AVRCP_CT_DISCONNECT_EVT, 8);
    msg.eventThreeParameter_ = 16;
    msg.rcConnect = true;
    msg.uuid_ = "1234567";
    msg.dev_ = "00:11:22:33:44:55";
    auto track = std::make_shared<AvrcpCtMediaItem>();
    msg.track_ = track;
    msg.connectFlag_.remoteCtlConnected = true;
    msg.connectFlag_.browsingConnected = false;
    msg.appSettingCount_ = 2;
    msg.appSettingAttrs_[0] = AVRCP_REPEAT_STATUS;
    msg.appSettingVals_[0] = AVRCP_REPEAT_ALL_TRACK;

    EXPECT_EQ(msg.eventThreeParameter_, 16);
    EXPECT_TRUE(msg.rcConnect);
    EXPECT_EQ(msg.uuid_, "1234567");
    EXPECT_EQ(msg.dev_, "00:11:22:33:44:55");
    EXPECT_EQ(msg.track_, track);
    EXPECT_TRUE(msg.connectFlag_.remoteCtlConnected);
    EXPECT_FALSE(msg.connectFlag_.browsingConnected);
    EXPECT_EQ(msg.appSettingCount_, 2);
    EXPECT_EQ(msg.appSettingAttrs_[0], AVRCP_REPEAT_STATUS);
    EXPECT_EQ(msg.appSettingVals_[0], AVRCP_REPEAT_ALL_TRACK);
}
} // namespace bluetooth
} // namespace OHOS
