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
#include <cstring>
#include "profile_service_manager.h"
#include "pan_service.h"
#include "pan_statemachine.h"
#include "base_def.h"
#include "bluetooth_host.h"
#include "pan_service_impl.h"

#include "mock/pan_interface_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;
using namespace bluetooth::headset;

namespace OHOS {
namespace Bluetooth {

const int TIME = 2;
std::string address("11:22:33:44:55:66");
PanStateMachine stateMachine(address);

class PanStatemachineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase start");
        HILOGI("TearDownTestCase end");
    }
    void SetUp()
    {
        serviceImpl = new PanServiceImpl();
        serviceImpl->NotifyStateChangedFunc_ = [](const bluetooth::RawAddress &, int, int) {};
        serviceImpl->PostEventFunc_ = [](const PanMessage &) {};
        serviceImpl->IsTetheringOnFunc_ = []() { return true; };
        serviceImpl->GetDeviceStateFunc_ = [](const bluetooth::RawAddress &) { return 0; };
        serviceImpl->SetDevProactiveDisconnectFlagFunc_ = [](const std::string &, uint32_t, bool) {};

        disconnectedState =
            std::make_unique<PanDisconnectedState>(PanStateMachine::DISCONNECTED, stateMachine);
        connectingState =
            std::make_unique<PanConnectingState>(PanStateMachine::CONNECTING, stateMachine);
        disconnectingState =
            std::make_unique<PanDisconnectingState>(PanStateMachine::DISCONNECTING, stateMachine);
        connectedState =
            std::make_unique<PanConnectedState>(PanStateMachine::CONNECTED, stateMachine);
        stateMachine.Init(serviceImpl);
    }
    void TearDown()
    {
        disconnectedState.reset();
        connectingState.reset();
        disconnectingState.reset();
        connectedState.reset();
        delete serviceImpl;
        serviceImpl = nullptr;
    }
public:
    std::unique_ptr<PanDisconnectedState> disconnectedState;
    std::unique_ptr<PanConnectingState> connectingState;
    std::unique_ptr<PanDisconnectingState> disconnectingState;
    std::unique_ptr<PanConnectedState> connectedState;
    PanServiceImpl *serviceImpl = nullptr;
};

/*
 * @tc.number: Pan_DisConnectedState_MachineTest_Dispatch_00100
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_DisConnectedState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Pan_DisConnectedState_MachineTest_Dispatch_00100 start");

    PanMessage msg_pan_connecting_evt(PAN_CONNECTING_IND_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(msg_pan_connecting_evt));

    PanMessage msg_pan_disconnecting_evt(PAN_DISCONNECTING_IND_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(msg_pan_disconnecting_evt));

    HILOGI("Pan_DisConnectedState_MachineTest_Dispatch_00100 end");
}

/*
 * @tc.number: Pan_DisConnectingState_MachineTest_Dispatch_00100
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_DisConnectingState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Pan_DisConnectingState_MachineTest_Dispatch_00100 start");

    PanMessage msg_pan_connected_evt(PAN_CONNECTED_IND_EVT);
    EXPECT_TRUE(disconnectingState->Dispatch(msg_pan_connected_evt));

    PanMessage msg_pan_disconnected_evt(PAN_DISCONNECTED_IND_EVT);
    EXPECT_TRUE(disconnectingState->Dispatch(msg_pan_disconnected_evt));

    HILOGI("Pan_DisConnectingState_MachineTest_Dispatch_00100 end");
}

/*
 * @tc.number: Pan_ConnectingState_MachineTest_Dispatch_00100
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectingState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Pan_ConnectingState_MachineTest_Dispatch_00100 start");

    PanMessage msg_pan_disconnect_evt(PAN_DISCONNECT_EVT);
    connectingState->Dispatch(msg_pan_disconnect_evt);
    EXPECT_EQ(static_cast<int>(stateMachine.deferMsgs_.size()), 1);

    PanMessage msg_pan_connected_evt(PAN_CONNECTED_IND_EVT);
    EXPECT_TRUE(connectingState->Dispatch(msg_pan_connected_evt));

    PanMessage msg_pan_disconnecting_evt(PAN_DISCONNECTING_IND_EVT);
    EXPECT_TRUE(connectingState->Dispatch(msg_pan_disconnecting_evt));

    PanMessage msg_pan_disconnected_evt(PAN_DISCONNECTED_IND_EVT);
    EXPECT_TRUE(connectingState->Dispatch(msg_pan_disconnected_evt));

    HILOGI("Pan_ConnectingState_MachineTest_Dispatch_00100 end");
}

/*
 * @tc.number: Pan_ConnectedState_MachineTest_Dispatch_00100
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectedState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Pan_ConnectedState_MachineTest_Dispatch_00100 start");

    PanMessage msg_pan_disconnect_evt(PAN_DISCONNECT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(msg_pan_disconnect_evt));

    PanMessage msg_pan_disconnected_evt(PAN_DISCONNECTED_IND_EVT);
    EXPECT_TRUE(connectedState->Dispatch(msg_pan_disconnected_evt));

    PanMessage msg_pan_disconnecting_evt(PAN_DISCONNECTING_IND_EVT);
    EXPECT_TRUE(connectedState->Dispatch(msg_pan_disconnecting_evt));

    PanMessage msg_pan_connecting_evt(PAN_CONNECTING_IND_EVT);
    EXPECT_TRUE(connectedState->Dispatch(msg_pan_connecting_evt));

    HILOGI("Pan_ConnectedState_MachineTest_Dispatch_00100 end");
}

/*
 * @tc.number: Pan_GetEventName_00100
 * @tc.name: GetEventName
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_GetEventName_00100, TestSize.Level1)
{
    HILOGI("Pan_GetEventName_00100 start");

    EXPECT_EQ(stateMachine.GetEventName(PAN_DISCONNECTED_IND_EVT), "PAN_DISCONNECTED_IND_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PAN_DISCONNECTING_IND_EVT), "PAN_DISCONNECTING_IND_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PAN_CONNECTING_IND_EVT), "PAN_CONNECTING_IND_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PAN_CONNECTED_IND_EVT), "PAN_CONNECTED_IND_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PAN_DISCONNECT_EVT), "PAN_DISCONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(PAN_NETWORK_IPV4_PREFIX_LENGTH), "Unknown");

    HILOGI("Pan_GetEventName_00100 end");
}

/*
 * @tc.number: Pan_DisConnectedState_MachineTest_Dispatch_00200
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_DisConnectedState_MachineTest_Dispatch_00200, TestSize.Level1)
{
    HILOGI("Pan_DisConnectedState_MachineTest_Dispatch_00200 start");

    PanMessage msg_pan_connected_evt(PAN_CONNECTED_IND_EVT);

    auto manager = ProfileServiceManager::GetInstance();
    string name = "PanService";
    manager->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, name);
    EXPECT_TRUE(disconnectedState->Dispatch(msg_pan_connected_evt));

    PanService *service = PanService::GetService();
    service->isTetheringOn_ = true;
    EXPECT_TRUE(disconnectedState->Dispatch(msg_pan_connected_evt));

    HILOGI("Pan_DisConnectedState_MachineTest_Dispatch_00200 end");
}

/*
 * @tc.number: Pan_ConnectedState_MachineTest_Dispatch_00200
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectedState_MachineTest_Dispatch_00200, TestSize.Level1)
{
    HILOGI("Pan_ConnectedState_MachineTest_Dispatch_00200 start");

    PanMessage msg_pan_disconnect_evt(PAN_DISCONNECT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(msg_pan_disconnect_evt));

    btpan_interface_t interface;
    interface.disconnect = disconnect;
    PanService::GetService()->bluetoothPanInterface_ = &interface;
    EXPECT_TRUE(connectedState->Dispatch(msg_pan_disconnect_evt));

    HILOGI("Pan_ConnectedState_MachineTest_Dispatch_00200 end");
}

/*
 * @tc.number: Pan_ConnectedState_MachineTest_ProcessDeferredMessage_00100
 * @tc.name: ProcessDeferredMessage
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectedState_MachineTest_ProcessDeferredMessage_00100, TestSize.Level1)
{
    HILOGI("Pan_ConnectedState_MachineTest_ProcessDeferredMessage_00100 start");
    EXPECT_NO_FATAL_FAILURE(stateMachine.ProcessDeferredMessage());
    HILOGI("Pan_ConnectedState_MachineTest_ProcessDeferredMessage_00100 end");
}

/*
 * @tc.number: Pan_ConnectedState_MachineTest_NotifyStateTransitions_00100
 * @tc.name: NotifyStateTransitions
 * @tc.desc:
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectedState_MachineTest_NotifyStateTransitions_00100, TestSize.Level1)
{
    HILOGI("Pan_ConnectedState_MachineTest_NotifyStateTransitions_00100 start");

    PanDisconnectedState disconnected(PanStateMachine::DISCONNECTED, stateMachine);
    stateMachine.current_ = &disconnected;
    stateMachine.NotifyStateTransitions();

    PanConnectingState connecting(PanStateMachine::CONNECTING, stateMachine);
    stateMachine.current_ = &connecting;
    EXPECT_NO_FATAL_FAILURE(stateMachine.NotifyStateTransitions());

    HILOGI("Pan_ConnectedState_MachineTest_NotifyStateTransitions_00100 end");
}

/*
 * @tc.number: Pan_StateMachine_Init_00100
 * @tc.name: Init
 * @tc.desc: Test Init method initializes all states correctly
 */
HWTEST_F(PanStatemachineTest, Pan_StateMachine_Init_00100, TestSize.Level1)
{
    HILOGI("Pan_StateMachine_Init_00100 start");

    PanStateMachine testMachine(address);
    testMachine.Init(serviceImpl);
    EXPECT_TRUE(testMachine.GetState() != nullptr);

    HILOGI("Pan_StateMachine_Init_00100 end");
}

/*
 * @tc.number: Pan_GetDeviceAdress_00100
 * @tc.name: GetDeviceAdress
 * @tc.desc: Test GetDeviceAdress returns correct address
 */
HWTEST_F(PanStatemachineTest, Pan_GetDeviceAdress_00100, TestSize.Level1)
{
    HILOGI("Pan_GetDeviceAdress_00100 start");

    EXPECT_EQ(stateMachine.GetDeviceAdress(), address);

    HILOGI("Pan_GetDeviceAdress_00100 end");
}

/*
 * @tc.number: Pan_DisconnectedState_Entry_00100
 * @tc.name: Entry
 * @tc.desc: Test DisconnectedState Entry method processes deferred messages
 */
HWTEST_F(PanStatemachineTest, Pan_DisconnectedState_Entry_00100, TestSize.Level1)
{
    HILOGI("Pan_DisconnectedState_Entry_00100 start");

    disconnectedState->Entry();
    disconnectedState->Exit();
    EXPECT_NO_FATAL_FAILURE(disconnectedState->Entry());
    HILOGI("Pan_DisconnectedState_Entry_00100 end");
}

/*
 * @tc.number: Pan_DisconnectedState_Exit_00100
 * @tc.name: Exit
 * @tc.desc: Test DisconnectedState Exit method sets isReentry flag
 */
HWTEST_F(PanStatemachineTest, Pan_DisconnectedState_Exit_00100, TestSize.Level1)
{
    HILOGI("Pan_DisconnectedState_Exit_00100 start");

    disconnectedState->Exit();
    EXPECT_TRUE(disconnectedState->isReentry_);

    HILOGI("Pan_DisconnectedState_Exit_00100 end");
}

/*
 * @tc.number: Pan_ConnectingState_Entry_00100
 * @tc.name: Entry
 * @tc.desc: Test ConnectingState Entry method notifies state transitions
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectingState_Entry_00100, TestSize.Level1)
{
    HILOGI("Pan_ConnectingState_Entry_00100 start");

    EXPECT_NO_FATAL_FAILURE(connectingState->Entry());

    HILOGI("Pan_ConnectingState_Entry_00100 end");
}

/*
 * @tc.number: Pan_DisconnectingState_Entry_00100
 * @tc.name: Entry
 * @tc.desc: Test DisconnectingState Entry method notifies state transitions
 */
HWTEST_F(PanStatemachineTest, Pan_DisconnectingState_Entry_00100, TestSize.Level1)
{
    HILOGI("Pan_DisconnectingState_Entry_00100 start");

    EXPECT_NO_FATAL_FAILURE(disconnectingState->Entry());

    HILOGI("Pan_DisconnectingState_Entry_00100 end");
}

/*
 * @tc.number: Pan_ConnectedState_Entry_00100
 * @tc.name: Entry
 * @tc.desc: Test ConnectedState Entry method processes deferred messages and notifies
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectedState_Entry_00100, TestSize.Level1)
{
    HILOGI("Pan_ConnectedState_Entry_00100 start");

    EXPECT_NO_FATAL_FAILURE(connectedState->Entry());

    HILOGI("Pan_ConnectedState_Entry_00100 end");
}

/*
 * @tc.number: Pan_ConnectedState_Exit_00100
 * @tc.name: Exit
 * @tc.desc: Test ConnectedState Exit method
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectedState_Exit_00100, TestSize.Level1)
{
    HILOGI("Pan_ConnectedState_Exit_00100 start");

    EXPECT_NO_FATAL_FAILURE(connectedState->Exit());

    HILOGI("Pan_ConnectedState_Exit_00100 end");
}

/*
 * @tc.number: Pan_SetPanRole_00100
 * @tc.name: SetPanRole
 * @tc.desc: Test SetPanRole correctly sets the role value
 */
HWTEST_F(PanStatemachineTest, Pan_SetPanRole_00100, TestSize.Level1)
{
    HILOGI("Pan_SetPanRole_00100 start");

    PanMessage msg(PAN_CONNECT_EVT);
    msg.remoteRole = LOCAL_PANU_ROLE;
    stateMachine.SetPanRole(msg);
    EXPECT_EQ(stateMachine.role_, LOCAL_PANU_ROLE);

    msg.remoteRole = LOCAL_NAP_ROLE;
    stateMachine.SetPanRole(msg);
    EXPECT_EQ(stateMachine.role_, LOCAL_NAP_ROLE);

    HILOGI("Pan_SetPanRole_00100 end");
}

/*
 * @tc.number: Pan_GetPanRole_00100
 * @tc.name: GetPanRole
 * @tc.desc: Test GetPanRole correctly gets the role value
 */
HWTEST_F(PanStatemachineTest, Pan_GetPanRole_00100, TestSize.Level1)
{
    HILOGI("Pan_GetPanRole_00100 start");

    PanMessage msg(PAN_CONNECT_EVT);
    msg.remoteRole = LOCAL_PANU_ROLE;
    stateMachine.SetPanRole(msg);
    EXPECT_EQ(stateMachine.GetPanRole(), LOCAL_PANU_ROLE);
    HILOGI("Pan_GetPanRole_00100 end");
}

/*
 * @tc.number: Pan_DisconnectedState_Dispatch_00300
 * @tc.name: Dispatch
 * @tc.desc: Test DisconnectedState handles PAN_CONNECT_EVT event
 */
HWTEST_F(PanStatemachineTest, Pan_DisconnectedState_Dispatch_00300, TestSize.Level1)
{
    HILOGI("Pan_DisconnectedState_Dispatch_00300 start");

    PanMessage msg_pan_connect_evt(PAN_CONNECT_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(msg_pan_connect_evt));

    HILOGI("Pan_DisconnectedState_Dispatch_00300 end");
}

/*
 * @tc.number: Pan_NotifyStateTransitions_00200
 * @tc.name: NotifyStateTransitions
 * @tc.desc: Test NotifyStateTransitions handles different state transitions
 */
HWTEST_F(PanStatemachineTest, Pan_NotifyStateTransitions_00200, TestSize.Level1)
{
    HILOGI("Pan_NotifyStateTransitions_00200 start");

    PanConnectedState connected(PanStateMachine::CONNECTED, stateMachine);
    stateMachine.current_ = &connected;
    stateMachine.preState_ = PAN_STATE_DISCONNECTED;
    stateMachine.NotifyStateTransitions();

    PanConnectingState connecting(PanStateMachine::CONNECTING, stateMachine);
    stateMachine.current_ = &connecting;
    stateMachine.preState_ = PAN_STATE_CONNECTED;
    EXPECT_NO_FATAL_FAILURE(stateMachine.NotifyStateTransitions());

    HILOGI("Pan_NotifyStateTransitions_00200 end");
}

/*
 * @tc.number: Pan_ProcessConnectReqEvent_00100
 * @tc.name: ProcessConnectReqEvent
 * @tc.desc: Test ProcessConnectReqEvent calls pan interface connect
 */
HWTEST_F(PanStatemachineTest, Pan_ProcessConnectReqEvent_00100, TestSize.Level1)
{
    HILOGI("Pan_ProcessConnectReqEvent_00100 start");

    PanMessage msg(PAN_CONNECT_EVT);
    msg.dev_ = address;
    EXPECT_NO_FATAL_FAILURE(stateMachine.ProcessConnectReqEvent(msg));

    HILOGI("Pan_ProcessConnectReqEvent_00100 end");
}

/*
 * @tc.number: Pan_ProcessCloseReqEvent_00100
 * @tc.name: ProcessCloseReqEvent
 * @tc.desc: Test ProcessCloseReqEvent calls pan interface disconnect
 */
HWTEST_F(PanStatemachineTest, Pan_ProcessCloseReqEvent_00100, TestSize.Level1)
{
    HILOGI("Pan_ProcessCloseReqEvent_00100 start");
    PanMessage msg(PAN_DISCONNECT_EVT);
    msg.dev_ = address;
    EXPECT_NO_FATAL_FAILURE(stateMachine.ProcessCloseReqEvent(msg));
    HILOGI("Pan_ProcessCloseReqEvent_00100 end");
}

/*
 * @tc.number: Pan_ProcessOpenEvent_00100
 * @tc.name: ProcessOpenEvent
 * @tc.desc: Test ProcessOpenEvent with tethering on brings up network
 */
HWTEST_F(PanStatemachineTest, Pan_ProcessOpenEvent_00100, TestSize.Level1)
{
    HILOGI("Pan_ProcessOpenEvent_00100 start");

    PanMessage msg(PAN_CONNECTED_IND_EVT);
    EXPECT_NO_FATAL_FAILURE(stateMachine.ProcessOpenEvent(msg));

    HILOGI("Pan_ProcessOpenEvent_00100 end");
}

/*
 * @tc.number: Pan_Adding_DeferedMessage_00100
 * @tc.name: AddDeferredMessage
 * @tc.desc: Test AddDeferredMessage adds messages to defer list
 */
HWTEST_F(PanStatemachineTest, Pan_Adding_DeferedMessage_00100, TestSize.Level1)
{
    HILOGI("Pan_Adding_DeferedMessage_00100 start");

    int res = static_cast<int>(stateMachine.deferMsgs_.size());
    PanMessage msg(PAN_DISCONNECT_EVT);
    stateMachine.AddDeferredMessage(msg);
    EXPECT_EQ(static_cast<int>(stateMachine.deferMsgs_.size()), res + 1);

    HILOGI("Pan_Adding_DeferedMessage_00100 end");
}

/*
 * @tc.number: Pan_DisconnectingState_Dispatch_00200
 * @tc.name: Dispatch
 * @tc.desc: Test DisconnectingState handles additional events
 */
HWTEST_F(PanStatemachineTest, Pan_DisconnectingState_Dispatch_00200, TestSize.Level1)
{
    HILOGI("Pan_DisconnectingState_Dispatch_00200 start");

    PanMessage msg_pan_connecting_evt(PAN_CONNECTING_IND_EVT);
    EXPECT_TRUE(disconnectingState->Dispatch(msg_pan_connecting_evt));

    PanMessage msg_pan_connected_evt(PAN_CONNECTED_IND_EVT);
    EXPECT_TRUE(disconnectingState->Dispatch(msg_pan_connected_evt));

    HILOGI("Pan_DisconnectingState_Dispatch_00200 end");
}

/*
 * @tc.number: Pan_GetDeviceStateInt_00100
 * @tc.name: GetDeviceStateInt
 * @tc.desc: Test GetDeviceStateInt returns correct state values
 */
HWTEST_F(PanStatemachineTest, Pan_GetDeviceStateInt_00100, TestSize.Level1)
{
    HILOGI("Pan_GetDeviceStateInt_00100 start");
    EXPECT_EQ(stateMachine.GetDeviceStateInt(), PAN_STATE_DISCONNECTED);
    HILOGI("Pan_GetDeviceStateInt_00100 end");
}

/*
 * @tc.number: Pan_GetDeviceStateInt_00200
 * @tc.name: GetDeviceStateInt
 * @tc.desc: Test GetDeviceStateInt returns default state when current_ is nullptr
 */
HWTEST_F(PanStatemachineTest, Pan_GetDeviceStateInt_00200, TestSize.Level1)
{
    HILOGI("Pan_GetDeviceStateInt_00200 start");
    stateMachine.current_ = nullptr;
    EXPECT_EQ(stateMachine.GetDeviceStateInt(), PAN_STATE_DISCONNECTED);
    HILOGI("Pan_GetDeviceStateInt_00200 end");
}

/*
 * @tc.number: Pan_ConnectionTimeout_00100
 * @tc.name: ConnectionTimeout
 * @tc.desc: Test ConnectionTimeout posts timeout event
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectionTimeout_00100, TestSize.Level1)
{
    HILOGI("Pan_ConnectionTimeout_00100 start");
    EXPECT_NO_FATAL_FAILURE(stateMachine.ConnectionTimeout());
    HILOGI("Pan_ConnectionTimeout_00100 end");
}

/*
 * @tc.number: Pan_StartConnectionTimer_00100
 * @tc.name: StartConnectionTimer
 * @tc.desc: Test StartConnectionTimer starts the timer
 */
HWTEST_F(PanStatemachineTest, Pan_StartConnectionTimer_00100, TestSize.Level1)
{
    HILOGI("Pan_StartConnectionTimer_00100 start");
    EXPECT_NO_FATAL_FAILURE(stateMachine.StartConnectionTimer());
    HILOGI("Pan_StartConnectionTimer_00100 end");
}

/*
 * @tc.number: Pan_StopConnectionTimer_00100
 * @tc.name: StopConnectionTimer
 * @tc.desc: Test StopConnectionTimer stops the timer
 */
HWTEST_F(PanStatemachineTest, Pan_StopConnectionTimer_00100, TestSize.Level1)
{
    HILOGI("Pan_StopConnectionTimer_00100 start");
    EXPECT_NO_FATAL_FAILURE(stateMachine.StopConnectionTimer());
    HILOGI("Pan_StopConnectionTimer_00100 end");
}
/*
 * @tc.number: Pan_ConnectedState_SetupNapNetwork_00100
 * @tc.name: SetupNapNetwork_NapRole
 * @tc.desc: Verify SetupNapNetwork is invoked when role is REMOTE_NAP_ROLE in Entry
 */
HWTEST_F(PanStatemachineTest, Pan_ConnectedState_SetupNapNetwork_NapRole, TestSize.Level1)
{
    HILOGI("Pan_ConnectedState_SetupNapNetwork_NapRole start");

    PanMessage msg(PAN_CONNECT_EVT);
    msg.remoteRole = REMOTE_NAP_ROLE;
    stateMachine.SetPanRole(msg);
    EXPECT_EQ(stateMachine.GetPanRole(), REMOTE_NAP_ROLE);

    connectedState->panServiceImpl_ = serviceImpl;
    EXPECT_NO_FATAL_FAILURE(connectedState->SetupNapNetwork());

    HILOGI("Pan_ConnectedState_SetupNapNetwork_NapRole end");
}

}  // namespace bluetooth
}  // namespace OHOS