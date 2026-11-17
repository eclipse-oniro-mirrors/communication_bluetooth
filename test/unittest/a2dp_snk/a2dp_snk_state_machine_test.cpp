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
#include "a2dp_snk_state_machine.h"
#include "a2dp_snk_def.h"
#include "a2dp_snk_message.h"
#include "mock/a2dp_snk_service_mock.h"
#include "mock/test_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace {
class A2dpSnkStateMachineTest : public Test {
protected:
    void SetUp() override
    {
        serviceMock_ = std::make_unique<A2dpSnkServiceMock>();
        stateMachine_ = std::make_unique<A2dpSnkStateMachine>(TEST_DEVICE_ADDR_1, serviceMock_.get());
        stateMachine_->Init();
    }

    void TearDown() override
    {
        stateMachine_.reset();
        serviceMock_.reset();
    }

    void SendEvent(int what)
    {
        A2dpSnkMessage event(what, 0, nullptr);
        event.addr_ = TEST_DEVICE_ADDR_1;
        stateMachine_->ProcessMessage(event);
    }

    int GetCurrentState()
    {
        return stateMachine_->GetDeviceState();
    }

    std::unique_ptr<A2dpSnkServiceMock> serviceMock_;
    std::unique_ptr<A2dpSnkStateMachine> stateMachine_;
};

HWTEST_F(A2dpSnkStateMachineTest, Init_StateIsDisconnected, TestSize.Level1)
{
    EXPECT_EQ(GetCurrentState(), A2DP_SNK_STATE_DISCONNECTED);
}

HWTEST_F(A2dpSnkStateMachineTest, GetDeviceName_ReturnsAddress, TestSize.Level1)
{
    EXPECT_EQ(stateMachine_->GetDeviceName(), std::string(TEST_DEVICE_ADDR_1));
}

HWTEST_F(A2dpSnkStateMachineTest, IsRemoving_DefaultFalse, TestSize.Level1)
{
    EXPECT_FALSE(stateMachine_->IsRemoving());
}

HWTEST_F(A2dpSnkStateMachineTest, SetRemoving_True, TestSize.Level1)
{
    stateMachine_->SetRemoving(true);
    EXPECT_TRUE(stateMachine_->IsRemoving());
}

HWTEST_F(A2dpSnkStateMachineTest, SetRemoving_False, TestSize.Level1)
{
    stateMachine_->SetRemoving(false);
    EXPECT_FALSE(stateMachine_->IsRemoving());
}

HWTEST_F(A2dpSnkStateMachineTest, GetEventName_ValidEvent, TestSize.Level1)
{
    std::string name = A2dpSnkStateMachine::GetEventName(A2DP_SNK_CONN_CONNECT_EVT);
    EXPECT_FALSE(name.empty());
}

HWTEST_F(A2dpSnkStateMachineTest, StackConnected_TransitionsToConnected, TestSize.Level1)
{
    SendEvent(A2DP_SNK_CONN_STACK_CONNECTED_EVT);
    EXPECT_EQ(GetCurrentState(), A2DP_SNK_STATE_CONNECTED);
}

HWTEST_F(A2dpSnkStateMachineTest, StackDisconnected_TransitionsToDisconnected, TestSize.Level1)
{
    SendEvent(A2DP_SNK_CONN_STACK_CONNECTED_EVT);
    SendEvent(A2DP_SNK_CONN_STACK_DISCONNECTED_EVT);
    EXPECT_EQ(GetCurrentState(), A2DP_SNK_STATE_DISCONNECTED);
}
}
