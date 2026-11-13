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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "avrcp_ct_define.h"
#include "avrcp_ct_state_machine.h"

namespace bluetooth {
class AvrcpCtStateMachineTest : public testing::Test {
public:
    AvrcpCtStateMachineTest()
    {
        stateMachine_ = nullptr;
    }
    ~AvrcpCtStateMachineTest()
    {
        if (stateMachine_ != nullptr) {
            delete stateMachine_;
            stateMachine_ = nullptr;
        }
    }

protected:
    void SetUp() override
    {
        std::string deviceAddr = "00:00:00:00:00:00";
        stateMachine_ = new AvrcpCtStateMachine(deviceAddr);
    }

    void TearDown() override
    {
        if (stateMachine_ != nullptr) {
            delete stateMachine_;
            stateMachine_ = nullptr;
        }
    }

    AvrcpCtStateMachine* stateMachine_;
};

HWTEST_F(AvrcpCtStateMachineTest, GetCurrentState001, TestSize.Level1)
{
    AvrcpCtConnectState state = stateMachine_->GetCurrentState();
    EXPECT_EQ(state, AvrcpCtConnectState::DISCONNECTED);
}

HWTEST_F(AvrcpCtStateMachineTest, GetCurrentState002, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    AvrcpCtStateMachine stateMachine(deviceAddr);
    AvrcpCtConnectState state = stateMachine.GetCurrentState();
    EXPECT_EQ(state, AvrcpCtConnectState::DISCONNECTED);
}

HWTEST_F(AvrcpCtStateMachineTest, ProcessMsg001, TestSize.Level1)
{
    AvrcpCtProcessMsg msg;
    msg.msgType = AvrcpCtProcessMsg::MSG_TYPE_EVENT;
    msg.eventId = 0;
    int ret = stateMachine_->ProcessMsg(msg);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtStateMachineTest, ProcessMsg002, TestSize.Level1)
{
    AvrcpCtProcessMsg msg;
    msg.msgType = AvrcpCtProcessMsg::MSG_TYPE_CMD;
    msg.cmdId = AvrcpCtCmdMsg::AVRCP_CT_CMD_CONNECT;
    int ret = stateMachine_->ProcessMsg(msg);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtStateMachineTest, ProcessMsg003, TestSize.Level1)
{
    AvrcpCtProcessMsg msg;
    msg.msgType = AvrcpCtProcessMsg::MSG_TYPE_TIMEOUT;
    int ret = stateMachine_->ProcessMsg(msg);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtStateMachineTest, TransitionState001, TestSize.Level1)
{
    int ret = stateMachine_->TransitionState(AvrcpCtConnectState::CONNECTING);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtStateMachineTest, TransitionState002, TestSize.Level1)
{
    int ret = stateMachine_->TransitionState(AvrcpCtConnectState::CONNECTED);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtStateMachineTest, TransitionState003, TestSize.Level1)
{
    int ret = stateMachine_->TransitionState(AvrcpCtConnectState::DISCONNECTING);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtStateMachineTest, TransitionState004, TestSize.Level1)
{
    int ret = stateMachine_->TransitionState(AvrcpCtConnectState::DISCONNECTED);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtStateMachineTest, GetDeviceAddress001, TestSize.Level1)
{
    std::string deviceAddr = stateMachine_->GetDeviceAddress();
    EXPECT_EQ(deviceAddr, "00:00:00:00:00:00");
}
} // namespace bluetooth