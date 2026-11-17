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
#include <gmock/gmock.h>
#include <cstring>
#include "avrcp_bip_service.h"
#include "avrcp_tg_bip_statemachine.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
inline static const std::string CONNECTED = "Connected";
inline static const std::string DISCONNECTED = "Disconnected";

std::string address("11:22:33:44:55:66");
std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
    51, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_L2CAP, address);
std::shared_ptr<BipService> bipService = {};
std::weak_ptr<BipService> serviceWeak_ = bipService;
BipStateMachine stateMachine(socketDevice, serviceWeak_);

class BipStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        connectedState = std::make_unique<BipConnectedState>(CONNECTED, stateMachine);
        disconnectedState = std::make_unique<BipDisconnectedState>(DISCONNECTED, stateMachine);
    }
    void TearDown()
    {
        connectedState.reset();
        disconnectedState.reset();
    }
public:
    std::unique_ptr<BipConnectedState> connectedState;
    std::unique_ptr<BipDisconnectedState> disconnectedState;
};

/*
 * @tc.number: BipStateMachine_GetEventName_001
 * @tc.name: GetEventName
 * @tc.desc:
 */
HWTEST_F(BipStateMachineTest, BipStateMachine_GetEventName_001, TestSize.Level1)
{
    HILOGI("BipStateMachine_GetEventName_001 start");
    EXPECT_EQ(BipStateMachine::GetEventName(BIP_CONNECT_EVT), "BIP_CONNECT_EVT");
    EXPECT_EQ(BipStateMachine::GetEventName(BIP_DISCONNECT_EVT), "BIP_DISCONNECT_EVT");
    EXPECT_EQ(BipStateMachine::GetEventName(BIP_REMOVE_STATE_MACHINE_EVT), "BIP_REMOVE_STATE_MACHINE_EVT");
    EXPECT_EQ(BipStateMachine::GetEventName(100), "UNKNOWN");
    HILOGI("BipStateMachine_GetEventName_001 end");
}

/*
 * @tc.number: BipStateMachine_BipConnectedState_Dispatch_001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(BipStateMachineTest, BipStateMachine_BipConnectedState_Dispatch_001, TestSize.Level1)
{
    HILOGI("BipStateMachine_BipConnectedState_Dispatch_001 start");
    BipMessage disconnectEvent(BIP_DISCONNECT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(disconnectEvent));

    BipMessage connectEvent(BIP_CONNECT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(connectEvent));
    HILOGI("BipStateMachine_BipConnectedState_Dispatch_001 end");
}

/*
 * @tc.number: BipStateMachine_BipDisconnectedState_Entry_001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(BipStateMachineTest, BipStateMachine_BipDisconnectedState_Entry_001, TestSize.Level1)
{
    HILOGI("BipStateMachine_BipDisconnectedState_Entry_001 start");
    BipMessage event(BIP_DISCONNECT_EVT);
    disconnectedState->Entry();
    EXPECT_FALSE(stateMachine.IsRemoving());

    disconnectedState->isReentry_ = true;
    disconnectedState->Entry();
    EXPECT_TRUE(stateMachine.IsRemoving());

    HILOGI("BipStateMachine_BipDisconnectedState_Entry_001 end");
}

/*
 * @tc.number: BipStateMachine_BipDisconnectedState_Dispatch_001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(BipStateMachineTest, BipStateMachine_BipDisconnectedState_Dispatch_001, TestSize.Level1)
{
    HILOGI("BipStateMachine_BipDisconnectedState_Dispatch_001 start");
    BipMessage connectEvent(BIP_CONNECT_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(connectEvent));

    BipMessage disconnectEvent(BIP_DISCONNECT_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(disconnectEvent));
    HILOGI("BipStateMachine_BipDisconnectedState_Dispatch_001 end");
}

/*
 * @tc.number: BipStateMachine_BipDisconnectedState_Exit_001
 * @tc.name: Exit
 * @tc.desc:
 */
HWTEST_F(BipStateMachineTest, BipStateMachine_BipDisconnectedState_Exit_001, TestSize.Level1)
{
    HILOGI("BipStateMachine_BipDisconnectedState_Exit_001 start");
    disconnectedState->isReentry_ = false;
    disconnectedState->Exit();
    EXPECT_TRUE(disconnectedState->isReentry_);
    HILOGI("BipStateMachine_BipDisconnectedState_Exit_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS