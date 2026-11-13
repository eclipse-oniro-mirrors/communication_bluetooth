/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "pbap_pse_statemachine.h"
#include "pbap_pse_service_impl.h"
#include "profile_service_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
inline static const std::string WAITFORAUTH = "WaitForAuth";
inline static const std::string DISCONNECTED = "Disconnected";
inline static const std::string CONNECTED = "Connected";

std::string address("11:22:33:44:55:66");
std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
            51, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_RFCOMM, address);
PbapPseStateMachine stateMachine(socketDevice, nullptr);

class PbapPseStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        waitAuthState =
            std::make_unique<PbapPseWaitForAuthState>(WAITFORAUTH, stateMachine);
        disconnectedState =
            std::make_unique<PbapPseDisconnectedState>(DISCONNECTED, stateMachine);
        connectedState =
            std::make_unique<PbapPseConnectedState>(CONNECTED, stateMachine);

        auto manager = ProfileServiceManager::GetInstance();
        manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_PBAP_PSE);
    }
    void TearDown()
    {
        stateMachine.deferMsgs_.clear();
        waitAuthState.reset();
        disconnectedState.reset();
        connectedState.reset();
    }
public:
    std::unique_ptr<PbapPseWaitForAuthState> waitAuthState;
    std::unique_ptr<PbapPseDisconnectedState> disconnectedState;
    std::unique_ptr<PbapPseConnectedState> connectedState;
};

/*
 * @tc.number: PbapPseStateMachine_GetEventName_001
 * @tc.name: GetEventName
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_GetEventName_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_GetEventName_001 start");
    EXPECT_EQ(stateMachine.GetEventName(PBAP_PSE_CONNECT_EVT), "PBAP_PSE_CONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PBAP_PSE_DISCONNECT_EVT), "PBAP_PSE_DISCONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PBAP_PSE_INVALID_EVT), "PBAP_PSE_INVALID_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PBAP_PSE_REQUEST_PERMISSION_EVT), "PBAP_PSE_REQUEST_PERMISSION_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PBAP_PSE_VERIFY_RESULT_EVT), "PBAP_PSE_VERIFY_RESULT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PBAP_PSE_REMOVE_STATE_MACHINE_EVT), "PBAP_PSE_REMOVE_STATE_MACHINE_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT), "PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(10), "Unknown");
    HILOGI("PbapPseStateMachine_GetEventName_001 end");
}

/*
 * @tc.number: PbapPseStateMachine_NotifyStateTransitions_001
 * @tc.name: NotifyStateTransitions
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_NotifyStateTransitions_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_NotifyStateTransitions_001 start");
    PbapPseWaitForAuthState waitAuth(WAITFORAUTH, stateMachine);
    PbapPseConnectedState connected(CONNECTED, stateMachine);
    PbapPseDisconnectedState disconnected(DISCONNECTED, stateMachine);

    stateMachine.current_ = &waitAuth;
    stateMachine.preState_ = PBAP_PSE_STATE_DISCONNECTED;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, PBAP_PSE_STATE_WAITFORAUTH);

    stateMachine.current_ = &waitAuth;
    stateMachine.preState_ = PBAP_PSE_STATE_WAITFORAUTH;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, PBAP_PSE_STATE_WAITFORAUTH);

    stateMachine.current_ = &connected;
    stateMachine.preState_ = PBAP_PSE_STATE_WAITFORAUTH;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, PBAP_PSE_STATE_CONNECTED);

    stateMachine.current_ = &disconnected;
    stateMachine.preState_ = PBAP_PSE_STATE_CONNECTED;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, PBAP_PSE_STATE_DISCONNECTED);

    HILOGI("PbapPseStateMachine_NotifyStateTransitions_001 end");
}

/*
 * @tc.number: PbapPseStateMachine_PbapPseWaitForAuthState_Dispatch_001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_PbapPseWaitForAuthState_Dispatch_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_PbapPseWaitForAuthState_Dispatch_001 start");
    PbapPseMessage verifyEvent(PBAP_PSE_VERIFY_RESULT_EVT);
    verifyEvent.isAccess_ = true;
    EXPECT_TRUE(waitAuthState->Dispatch(verifyEvent));

    verifyEvent.isAccess_ = false;
    EXPECT_TRUE(waitAuthState->Dispatch(verifyEvent));
    sleep(2);

    PbapPseMessage disconnectEvent(PBAP_PSE_DISCONNECT_EVT);
    EXPECT_TRUE(waitAuthState->Dispatch(disconnectEvent));

    PbapPseMessage connectEvent(PBAP_PSE_CONNECT_EVT);
    EXPECT_TRUE(waitAuthState->Dispatch(connectEvent));

    HILOGI("PbapPseStateMachine_PbapPseWaitForAuthState_Dispatch_001 end");
}

/*
 * @tc.number: PbapPseStateMachine_PbapPseDisconnectedState_Entry_001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_PbapPseDisconnectedState_Entry_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_PbapPseDisconnectedState_Entry_001 start");
    PbapPseMessage event(PBAP_PSE_DISCONNECT_EVT);
    disconnectedState->Entry();
    EXPECT_FALSE(stateMachine.isRemoving_);

    disconnectedState->isReentry_ = true;
    disconnectedState->Entry();
    EXPECT_TRUE(stateMachine.isRemoving_);

    HILOGI("PbapPseStateMachine_PbapPseDisconnectedState_Entry_001 end");
}

/*
 * @tc.number: PbapPseStateMachine_PbapPseDisconnectedState_Dispatch_001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_PbapPseDisconnectedState_Dispatch_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_PbapPseDisconnectedState_Dispatch_001 start");
    PbapPseMessage connectEvent(PBAP_PSE_CONNECT_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(connectEvent));

    PbapPseMessage disconnectEvent(PBAP_PSE_DISCONNECT_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(disconnectEvent));

    HILOGI("PbapPseStateMachine_PbapPseDisconnectedState_Dispatch_001 end");
}

/*
 * @tc.number: PbapPseStateMachine_PbapPseConnectedState_Dispatch_001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_PbapPseConnectedState_Dispatch_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_PbapPseConnectedState_Dispatch_001 start");
    PbapPseMessage connectEvent(PBAP_PSE_CONNECT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(connectEvent));

    PbapPseMessage disconnectEvent(PBAP_PSE_DISCONNECT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(disconnectEvent));

    PbapPseMessage invalidEvent(PBAP_PSE_INVALID_EVT);
    EXPECT_TRUE(connectedState->Dispatch(invalidEvent));

    HILOGI("PbapPseStateMachine_PbapPseConnectedState_Dispatch_001 end");
}

/*
 * @tc.number: PbapPseStateMachine_ProcessDisConnectEvent_001
 * @tc.name: ProcessDisConnectEvent
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_ProcessDisConnectEvent_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_ProcessDisConnectEvent_001 start");
    stateMachine.ProcessDisConnectEvent();
    EXPECT_EQ(stateMachine.obexServerSession_, nullptr);

    stateMachine.ProcessConnectEvent();
    EXPECT_NE(stateMachine.obexServerSession_, nullptr);
    stateMachine.ProcessDisConnectEvent();
    EXPECT_EQ(stateMachine.obexServerSession_, nullptr);
    HILOGI("PbapPseStateMachine_ProcessDisConnectEvent_001 end");
}

/*
 * @tc.number: PbapPseStateMachine_GetDeviceStateInt_001
 * @tc.name: GetDeviceStateInt
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_GetDeviceStateInt_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_GetDeviceStateInt_001 start");
    stateMachine.current_ = nullptr;
    EXPECT_EQ(stateMachine.GetDeviceStateInt(), PBAP_PSE_STATE_DISCONNECTED);

    stateMachine.Init();
    EXPECT_EQ(stateMachine.GetDeviceStateInt(), PBAP_PSE_STATE_DISCONNECTED);

    stateMachine.Transition(WAITFORAUTH);
    EXPECT_EQ(stateMachine.GetDeviceStateInt(), PBAP_PSE_STATE_WAITFORAUTH);

    stateMachine.Transition(CONNECTED);
    EXPECT_EQ(stateMachine.GetDeviceStateInt(), PBAP_PSE_STATE_CONNECTED);
    HILOGI("PbapPseStateMachine_GetDeviceStateInt_001 end");
}

/*
 * @tc.number: PbapPseStateMachine_ProcessDeferredMessage_001
 * @tc.name: ProcessDeferredMessage
 * @tc.desc:
 */
HWTEST_F(PbapPseStateMachineTest, PbapPseStateMachine_ProcessDeferredMessage_001, TestSize.Level1)
{
    HILOGI("PbapPseStateMachine_ProcessDeferredMessage_001 start");
    stateMachine.ProcessDeferredMessage();
    EXPECT_TRUE(stateMachine.deferMsgs_.empty());

    stateMachine.AddDeferredMessage(PbapPseMessage(PBAP_PSE_CONNECT_EVT));
    stateMachine.AddDeferredMessage(PbapPseMessage(PBAP_PSE_DISCONNECT_EVT));
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 2);
    stateMachine.ProcessDeferredMessage();
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 2);
    HILOGI("PbapPseStateMachine_ProcessDeferredMessage_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS