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
#include "hfp_ag_service.h"
#include "profile_service_manager.h"
#include "remote_device_properties.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "mock/hfp_ag_interface_mock.h"
#ifdef BLUETOOTH_WATCH_ENABLE
#include "watch_service.h"
#endif

using namespace testing;
using namespace testing::ext;
using namespace bluetooth::headset;

namespace OHOS {
namespace bluetooth {

std::string address("11:22:33:44:55:66");
HfpAgStateMachine stateMachine(address);

class HfpAgSystemEventProcesserTest : public HfpAgSystemEventProcesser {
public:
    HfpAgSystemEventProcesserTest() : HfpAgSystemEventProcesser(address) {}
    ~HfpAgSystemEventProcesserTest() = default;
};

static HfpAgSystemEventProcesserTest g_eventProcessor;

class HfpAgStatemachineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        disconnectedState =
            std::make_unique<HfpAgDisconnected>(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
        connectingState =
            std::make_unique<HfpAgConnecting>(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
        disconnectingState =
            std::make_unique<HfpAgDisconnecting>(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
        connectedState =
            std::make_unique<HfpAgConnected>(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        audioConnecting = std::make_unique<HfpAgAudioConnecting>(HfpAgStateMachine::AUDIO_CONNECTING,
            stateMachine, g_eventProcessor, *connectedState);
        audioConnected = std::make_unique<HfpAgAudioConnected>(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, *connectedState);
        audioDisconnecting = std::make_unique<HfpAgAudioDisconnecting>(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, *connectedState);

        auto manager = ProfileServiceManager::GetInstance();
        string name = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, name);
    }
    void TearDown()
    {
        stateMachine.deferMsgs_.clear();
        disconnectedState.reset();
        connectingState.reset();
        disconnectingState.reset();
        connectedState.reset();
        audioConnecting.reset();
        audioConnected.reset();
        audioDisconnecting.reset();
        HfpAgService::GetService()->bluetoothHfpInterface = nullptr;
    }
public:
    std::unique_ptr<HfpAgDisconnected> disconnectedState;
    std::unique_ptr<HfpAgConnecting> connectingState;
    std::unique_ptr<HfpAgDisconnecting> disconnectingState;
    std::unique_ptr<HfpAgConnected> connectedState;
    std::unique_ptr<HfpAgAudioConnecting> audioConnecting;
    std::unique_ptr<HfpAgAudioConnected> audioConnected;
    std::unique_ptr<HfpAgAudioDisconnecting> audioDisconnecting;
};

/*
 * @tc.number: Hfp_Ag_Statemachine_GetEventName_0001
 * @tc.name: GetEventName
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_GetEventName_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_GetEventName_0001 start");

    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_INVALID_EVT), "HFP_AG_INVALID_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_SERVICE_STARTUP_EVT), "HFP_AG_SERVICE_STARTUP_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_SERVICE_SHUTDOWN_EVT), "HFP_AG_SERVICE_SHUTDOWN_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_PROCESS_CKPD_EVT), "HFP_AG_PROCESS_CKPD_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_OPEN_VOICE_RECOGNITION_EVT), "HFP_AG_OPEN_VOICE_RECOGNITION_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_CLOSE_VOICE_RECOGNITION_EVT), "HFP_AG_CLOSE_VOICE_RECOGNITION_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_VOICE_RECOGNITION_RESULT_EVT), "HFP_AG_VOICE_RECOGNITION_RESULT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_SET_VOLUME_EVT), "HFP_AG_SET_VOLUME_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_SET_INBAND_RING_TONE_EVT), "HFP_AG_SET_INBAND_RING_TONE_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_SEND_SUBSCRIBER_NUMBER_EVT), "HFP_AG_SEND_SUBSCRIBER_NUMBER_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_SEND_NETWORK_OPERATOR_EVT), "HFP_AG_SEND_NETWORK_OPERATOR_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_CONTROL_OTHER_MODULES_EVT), "HFP_AG_CONTROL_OTHER_MODULES_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_DIALING_OUT_RESULT), "HFP_AG_DIALING_OUT_RESULT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_CALL_STATE_CHANGE), "HFP_AG_CALL_STATE_CHANGE");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_SEND_CCLC_RESPONSE), "HFP_AG_SEND_CCLC_RESPONSE");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT), "HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_DIAL_TIME_OUT_EVT), "HFP_AG_DIAL_TIME_OUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_NOTIFY_DEVICE_STATE_CHANGED), "HFP_AG_NOTIFY_DEVICE_STATE_CHANGED");

    HILOGI("Hfp_Ag_Statemachine_GetEventName_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_GetEventName_0002
 * @tc.name: GetEventName
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_GetEventName_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_GetEventName_0002 start");

    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_CONNECT_EVT), "HFP_AG_CONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_DISCONNECT_EVT), "HFP_AG_DISCONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_CONNECTION_STATE_CHANGED_EVT), "HFP_AG_CONNECTION_STATE_CHANGED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_CONNECT_AUDIO_EVT), "HFP_AG_CONNECT_AUDIO_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_DISCONNECT_AUDIO_EVT), "HFP_AG_DISCONNECT_AUDIO_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_CONNECTED_EVT), "HFP_AG_CONNECTED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_DISCONNECTED_EVT), "HFP_AG_DISCONNECTED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_CONNECT_FAILED_EVT), "HFP_AG_CONNECT_FAILED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_DISCONNECT_FAILED_EVT), "HFP_AG_DISCONNECT_FAILED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_AUDIO_CONNECTING_EVT), "HFP_AG_AUDIO_CONNECTING_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_AUDIO_DISCONNECTING_EVT), "HFP_AG_AUDIO_DISCONNECTING_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_AUDIO_CONNECTED_EVT), "HFP_AG_AUDIO_CONNECTED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_AUDIO_DISCONNECTED_EVT), "HFP_AG_AUDIO_DISCONNECTED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_AUDIO_CONNECT_FAILED_EVT), "HFP_AG_AUDIO_CONNECT_FAILED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_AUDIO_DISCONNECT_FAILED_EVT), "HFP_AG_AUDIO_DISCONNECT_FAILED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_AUDIO_STATE_CHANGED_EVT), "HFP_AG_AUDIO_STATE_CHANGED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_AG_RING_TIMEOUT_EVT), "Unknown");

    HILOGI("Hfp_Ag_Statemachine_GetEventName_0002 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_NotifyStateTransitions_0001
 * @tc.name: NotifyStateTransitions
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_NotifyStateTransitions_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_NotifyStateTransitions_0001 start");

    HfpAgDisconnected disconnected(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnecting disconnecting(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnecting connecting(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnected connected(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);

    stateMachine.current_ = &connected;
    stateMachine.preState_ = HFP_AG_STATE_CONNECTING;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_AG_STATE_CONNECTED);

    stateMachine.current_ = &connected;
    stateMachine.preState_ = HFP_AG_STATE_DISCONNECTED;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_AG_STATE_CONNECTED);

    stateMachine.current_ = &disconnecting;
    stateMachine.preState_ = HFP_AG_STATE_CONNECTING;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_AG_STATE_DISCONNECTING);

    stateMachine.current_ = &disconnected;
    stateMachine.preState_ = HFP_AG_STATE_DISCONNECTING;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_AG_STATE_DISCONNECTED);

    HILOGI("Hfp_Ag_Statemachine_NotifyStateTransitions_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_NotifyStateTransitions_0002
 * @tc.name: NotifyStateTransitions
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_NotifyStateTransitions_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_NotifyStateTransitions_0002 start");

    HfpAgConnected connected(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgAudioConnected audioConnected(HfpAgStateMachine::AUDIO_CONNECTED, stateMachine, g_eventProcessor, connected);
    HfpAgAudioConnecting audioConnecting(HfpAgStateMachine::AUDIO_CONNECTING, stateMachine,
        g_eventProcessor, connected);
    HfpAgAudioDisconnecting audioDisconnecing(HfpAgStateMachine::AUDIO_DISCONNECTING, stateMachine,
        g_eventProcessor, connected);

    stateMachine.current_ = &audioConnected;
    stateMachine.preState_ = HFP_AG_AUDIO_STATE_CONNECTING;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_AG_AUDIO_STATE_CONNECTED);

    stateMachine.current_ = &audioConnected;
    stateMachine.preState_ = HFP_AG_STATE_CONNECTED;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_AG_AUDIO_STATE_CONNECTED);

    stateMachine.current_ = &audioConnecting;
    stateMachine.preState_ = HFP_AG_STATE_CONNECTED;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_AG_AUDIO_STATE_CONNECTING);

    HILOGI("Hfp_Ag_Statemachine_NotifyStateTransitions_0002 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_NotifyChildStateToParentState_0001
 * @tc.name: NotifyChildStateToParentState
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_NotifyChildStateToParentState_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_NotifyChildStateToParentState_0001 start");

    stateMachine.NotifyChildStateToParentState(HFP_AG_STATE_DISCONNECTING, HFP_AG_STATE_DISCONNECTED);
    EXPECT_EQ(stateMachine.preState_, HFP_AG_STATE_DISCONNECTED);

    HILOGI("Hfp_Ag_Statemachine_NotifyChildStateToParentState_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgDisconnected_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgDisconnected_Entry_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnected_Entry_0001 start");

    disconnectedState->Entry();
    EXPECT_FALSE(stateMachine.isRemoving_);

    HfpAgDisconnected disconnected(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    stateMachine.current_ = &disconnected;
    disconnectedState->Exit();
    disconnectedState->Entry();
    EXPECT_TRUE(stateMachine.isRemoving_);

    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnected_Entry_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgDisconnected_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgDisconnected_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnected_Dispatch_0001 start");

    HfpAgMessage connectStateChanged(HFP_AG_CONNECT_EVT);
    disconnectedState->Dispatch(connectStateChanged);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, Connect(_)).Times(2).WillOnce(Return(BT_STATUS_SUCCESS)).WillOnce(Return(BT_STATUS_FAIL));

    disconnectedState->Dispatch(connectStateChanged);

    disconnectedState->Dispatch(connectStateChanged);

    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnected_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgDisconnected_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgDisconnected_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnected_Dispatch_0002 start");

    HfpAgMessage connectionStateChanged(HFP_AG_CONNECTION_STATE_CHANGED_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(connectionStateChanged));

    connectionStateChanged.arg1_ = HFP_AG_STATE_CONNECTING;
    EXPECT_TRUE(disconnectedState->Dispatch(connectionStateChanged));

    HfpAgMessage invalidEvt(HFP_AG_INVALID_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(invalidEvt));

    HfpAgMessage event(HFP_AG_RETRY_CONNECT_EVT);
    RawAddress addr("00:00:00:00:00:01");
    event.dev_ = addr.GetAddress();
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(addr);
    stateMachine.NotifyChildStateToParentState(0, 3, 0);
    HfpAgService::GetService()->GetDevicePreState(addr);
    stateMachine.NotifyChildStateToParentState(0, 3, 0);
    HfpAgService::GetService()->GetDevicePreState(addr);
    bluetoothDevice->SetAclConnectState(1);
    bluetoothDevice->SetPairedStatus(3);
    EXPECT_TRUE(disconnectedState->Dispatch(event));
    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnected_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0001 start");

    HfpAgMessage connectEvt(HFP_AG_CONNECT_EVT);
    connectingState->Dispatch(connectEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 1);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0002 start");

    HfpAgMessage connectionStateChanged(HFP_AG_CONNECTION_STATE_CHANGED_EVT);
    connectionStateChanged.arg1_ = HFP_AG_STATE_DISCONNECTED;
    connectingState->Dispatch(connectionStateChanged);

    connectionStateChanged.arg1_ = HFP_AG_STATE_CONNECTED;
    connectingState->Dispatch(connectionStateChanged);

    connectionStateChanged.arg1_ = HFP_AG_STATE_CONNECTING;
    connectingState->Dispatch(connectionStateChanged);

    connectionStateChanged.arg1_ = HFP_AG_AUDIO_STATE_CONNECTING;
    connectingState->Dispatch(connectionStateChanged);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0003 start");
    HfpAgMessage agControl(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    agControl.type_ = HFP_AG_MSG_TYPE_ANSWER_CALL;
    connectingState->Dispatch(agControl);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0003 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0004
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0004, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0004 start");
    HfpAgMessage connectEvt(HFP_AG_DISCONNECT_EVT);
    connectingState->Dispatch(connectEvt);

    HfpAgMessage invalidEvt(HFP_AG_INVALID_EVT);
    connectingState->Dispatch(connectEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnecting_Dispatch_0004 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0001 start");

    HfpAgMessage connectEvt(HFP_AG_CONNECT_EVT);
    disconnectingState->Dispatch(connectEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 1);

    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0002 start");

    HfpAgMessage stateChanged(HFP_AG_CONNECTION_STATE_CHANGED_EVT);
    stateChanged.arg1_ = HFP_AG_STATE_DISCONNECTED;
    disconnectingState->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_STATE_CONNECTED;
    disconnectingState->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_STATE_CONNECTING;
    disconnectingState->Dispatch(stateChanged);

    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0003 start");

    HfpAgMessage timeout(HFP_AG_CONNECTION_TIMEOUT_EVT);
    disconnectingState->Dispatch(timeout);

    HfpAgMessage invalidEvt(HFP_AG_INVALID_EVT);
    disconnectingState->Dispatch(invalidEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgDisconnecting_Dispatch_0003 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0001 start");

    HfpAgMessage connect(HFP_AG_CONNECT_EVT);
    connectedState->Dispatch(connect);

    HfpAgMessage disconnectAudio(HFP_AG_DISCONNECT_AUDIO_EVT);
    connectedState->Dispatch(disconnectAudio);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0002 start");

    HfpAgMessage disconnect(HFP_AG_DISCONNECT_EVT);
    connectedState->Dispatch(disconnect);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, Disconnect(_)).Times(2).WillOnce(Return(BT_STATUS_SUCCESS)).WillOnce(Return(BT_STATUS_FAIL));

    connectedState->Dispatch(disconnect);

    connectedState->Dispatch(disconnect);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0003 start");

    HfpAgMessage connectAudio(HFP_AG_CONNECT_AUDIO_EVT);
    connectedState->Dispatch(connectAudio);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, ConnectAudio(_)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));

    connectedState->Dispatch(connectAudio);

    connectedState->Dispatch(connectAudio);

#ifdef BLUETOOTH_WATCH_ENABLE
    WatchService::GetInstance()->SetIsHfOnCall(address, true);
    connectedState->Dispatch(connectAudio);
#endif

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0003 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0004, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0004 start");

    HfpAgMessage stateChanged(HFP_AG_CONNECTION_STATE_CHANGED_EVT);
    stateChanged.arg1_ = HFP_AG_STATE_DISCONNECTING;
    connectedState->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_STATE_DISCONNECTED;
    connectedState->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_STATE_CONNECTING;
    connectedState->Dispatch(stateChanged);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0004 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0005
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0005, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0005 start");

    HfpAgMessage audioStateChanged(HFP_AG_AUDIO_STATE_CHANGED_EVT);
    audioStateChanged.arg1_ = HFP_AG_AUDIO_STATE_CONNECTED;
    connectedState->Dispatch(audioStateChanged);

    audioStateChanged.arg1_ = HFP_AG_AUDIO_STATE_CONNECTING;
    connectedState->Dispatch(audioStateChanged);

    audioStateChanged.arg1_ = HFP_AG_AUDIO_STATE_DISCONNECTED;
    connectedState->Dispatch(audioStateChanged);

    audioStateChanged.arg1_ = HFP_AG_STATE_DISCONNECTING;
    connectedState->Dispatch(audioStateChanged);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0005 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0006
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0006, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0005 start");

    HfpAgMessage agControl(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    agControl.type_ = HFP_AG_MSG_TYPE_ANSWER_CALL;
    connectedState->Dispatch(agControl);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0006 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0007
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0007, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0007 start");

    HfpAgMessage openVoiceEvt(HFP_AG_OPEN_VOICE_RECOGNITION_EVT);
    connectedState->Dispatch(openVoiceEvt);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, StartVoiceRecognition(_)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));

    connectedState->Dispatch(openVoiceEvt);

    connectedState->Dispatch(openVoiceEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0007 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0008
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0008, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0008 start");

    HfpAgMessage openVoiceResult(HFP_AG_VOICE_RECOGNITION_RESULT_EVT);
    openVoiceResult.arg1_ = 1;
    connectedState->Dispatch(openVoiceResult);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, AtResponse(_, _, _)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    connectedState->Dispatch(openVoiceResult);
    connectedState->Dispatch(openVoiceResult);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0008 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0009
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0009, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0009 start");

    HfpAgMessage openVoiceResult(HFP_AG_VOICE_RECOGNITION_RESULT_EVT);
    openVoiceResult.arg1_ = 0;
    connectedState->Dispatch(openVoiceResult);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0009 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0010
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0010, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0010 start");

    HfpAgMessage closeVoiceEvt(HFP_AG_CLOSE_VOICE_RECOGNITION_EVT);
    connectedState->Dispatch(closeVoiceEvt);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, StopVoiceRecognition(_)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));

    connectedState->Dispatch(closeVoiceEvt);

    connectedState->Dispatch(closeVoiceEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0010 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_00011
 * @tc.name: Dispatch
 * @tc.desc: test event HFP_AG_OPEN_VOICE_RECOGNITION_EVT
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_00011, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_00011 start");
    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, StartVoiceRecognition(_)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_SUCCESS));

    HfpAgMessage openVoiceEvtHf(HFP_AG_OPEN_VOICE_RECOGNITION_EVT, HFP_START_VOICE_REG_BY_HF);
    connectedState->Dispatch(openVoiceEvtHf);
    EXPECT_TRUE(openVoiceEvtHf.arg1_ == HFP_START_VOICE_REG_BY_HF);
    HfpAgMessage openVoiceEvtAg(HFP_AG_OPEN_VOICE_RECOGNITION_EVT, HFP_START_VOICE_REG_BY_AG);
    connectedState->Dispatch(openVoiceEvtAg);
    EXPECT_TRUE(openVoiceEvtAg.arg1_ == HFP_START_VOICE_REG_BY_AG);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_00011 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0012
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0012, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0012 start");
    HfpAgConnected connected(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgConnecting connecting(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);

    HfpAgMessage ckpdEvt(HFP_AG_PROCESS_CKPD_EVT);
    HfpAgSystemInterface::GetInstance().callState_ = HFP_AG_CALL_STATE_INCOMING;
    connectedState->Dispatch(ckpdEvt);

    stateMachine.current_ = &connecting;
    HfpAgSystemInterface::GetInstance().callState_ = HFP_AG_CALL_STATE_WAITING;
    connectedState->Dispatch(ckpdEvt);

    stateMachine.current_ = &connected;
    HfpAgSystemInterface::GetInstance().callState_ = HFP_AG_CALL_STATE_DISCONNECTED;
    connectedState->Dispatch(ckpdEvt);

    connectedState->callState_ = HFP_AG_CALL_STATE_DIALING;
    connectedState->Dispatch(ckpdEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0012 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0013
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0013, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0013 start");

    HfpAgMessage evt(HFP_AG_SEND_CCLC_RESPONSE);
    connectedState->Dispatch(evt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0013 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0014
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0014, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0014 start");

    HfpAgMessage evt(HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT);
    connectedState->Dispatch(evt);
    connectingState->Dispatch(evt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0014 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0015
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0015, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0015 start");

    HfpAgMessage evt(HFP_AG_CALL_STATE_CHANGE);
    connectedState->Dispatch(evt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0015 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0017
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0017, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0017 start");

    HfpAgMessage evt(HFP_AG_DIALING_OUT_RESULT);
    connectedState->Dispatch(evt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0017 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0018
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0018, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0018 start");

    HfpAgMessage evt(HFP_AG_SET_INBAND_RING_TONE_EVT);
    connectedState->Dispatch(evt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0018 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0019
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0019, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0019 start");

    HfpAgMessage invalidEvt(HFP_AG_INVALID_EVT);
    connectedState->Dispatch(invalidEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0019 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0020
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0020, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0020 start");
    HfpAgMessage msg(HFP_AG_AUDIO_STATE_CHANGED_EVT, HFP_AG_AUDIO_STATE_CONNECTED);
    connectedState->Dispatch(msg);
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0020 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0021
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0021, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0021 start");
    HfpAgService::GetService()->SetInbandRing(true);
    HfpAgMessage msg(HFP_AG_AUDIO_STATE_CHANGED_EVT, HFP_AG_AUDIO_STATE_CONNECTED);
    connectedState->Dispatch(msg);

    HfpAgService::GetService()->SetInbandRing(false);
    HfpAgSystemInterface::GetInstance().callState_ = HFP_AG_CALL_STATE_INCOMING;
    connectedState->Dispatch(msg);
    HILOGI("Hfp_Ag_Statemachine_HfpAgConnected_Dispatch_0021 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgAudioConnecting_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgAudioConnecting_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnecting_Dispatch_0001 start");

    HfpAgMessage connectEvt(HFP_AG_CONNECT_EVT);
    audioConnecting->Dispatch(connectEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 1);

    HfpAgMessage otherModules(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    audioConnecting->Dispatch(otherModules);

    stateMachine.deferMsgs_.clear();
    HfpAgMessage timeoutEvt(HFP_AG_CONNECTION_TIMEOUT_EVT);
    audioConnecting->Dispatch(timeoutEvt);

    HfpAgMessage connectingStateChanged(HFP_AG_CONNECTION_STATE_CHANGED_EVT);
    connectingStateChanged.arg1_ = HFP_AG_STATE_DISCONNECTING;
    audioConnecting->Dispatch(connectingStateChanged);

    connectingStateChanged.arg1_ = HFP_AG_STATE_DISCONNECTED;
    audioConnecting->Dispatch(connectingStateChanged);

    connectingStateChanged.arg1_ = HFP_AG_STATE_CONNECTED;
    audioConnecting->Dispatch(connectingStateChanged);

    HfpAgMessage StateChanged(HFP_AG_AUDIO_STATE_CHANGED_EVT);
    StateChanged.arg1_ = HFP_AG_AUDIO_STATE_CONNECTED;
    audioConnecting->Dispatch(StateChanged);

    StateChanged.arg1_ = HFP_AG_AUDIO_STATE_DISCONNECTED;
    audioConnecting->Dispatch(StateChanged);

    StateChanged.arg1_ = HFP_AG_AUDIO_STATE_CONNECTING;
    audioConnecting->Dispatch(StateChanged);

    StateChanged.arg1_ = HFP_AG_STATE_DISCONNECTED;
    audioConnecting->Dispatch(StateChanged);

    HfpAgMessage invalidEvt(HFP_AG_INVALID_EVT);
    audioConnecting->Dispatch(invalidEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnecting_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgAudioConnecting_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgAudioConnecting_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnecting_Dispatch_0002 start");

    HfpAgMessage evt(HFP_AG_CALL_STATE_CHANGE);
    audioConnecting->Dispatch(evt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnecting_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgAudioDisconnecting_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgAudioDisconnecting_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioDisconnecting_Dispatch_0001 start");

    HfpAgMessage disconnectEvt(HFP_AG_DISCONNECT_EVT);
    audioDisconnecting->Dispatch(disconnectEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 1);

    HfpAgMessage disconnectAudioEvt(HFP_AG_DISCONNECT_AUDIO_EVT);
    audioDisconnecting->Dispatch(disconnectAudioEvt);

    stateMachine.deferMsgs_.clear();
    HfpAgMessage timeoutEvt(HFP_AG_CONNECTION_TIMEOUT_EVT);
    audioDisconnecting->Dispatch(timeoutEvt);

    HfpAgMessage connectionStateChanged(HFP_AG_CONNECTION_STATE_CHANGED_EVT);
    connectionStateChanged.arg1_ = HFP_AG_STATE_DISCONNECTED;
    audioDisconnecting->Dispatch(connectionStateChanged);

    connectionStateChanged.arg1_ = HFP_AG_STATE_DISCONNECTING;
    audioDisconnecting->Dispatch(connectionStateChanged);

    connectionStateChanged.arg1_ = HFP_AG_STATE_CONNECTING;
    audioDisconnecting->Dispatch(connectionStateChanged);

    HfpAgMessage stateChanged(HFP_AG_AUDIO_STATE_CHANGED_EVT);
    stateChanged.arg1_ = HFP_AG_AUDIO_STATE_DISCONNECTED;
    audioDisconnecting->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_AUDIO_STATE_CONNECTING;
    audioDisconnecting->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_AUDIO_STATE_CONNECTED;
    audioDisconnecting->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_STATE_CONNECTING;
    audioDisconnecting->Dispatch(stateChanged);

    HfpAgMessage invalidEvt(HFP_AG_INVALID_EVT);
    audioDisconnecting->Dispatch(invalidEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioDisconnecting_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0001 start");

    HfpAgMessage connectEvt(HFP_AG_CONNECT_EVT);
    audioConnected->Dispatch(connectEvt);

    HfpAgMessage connectionStateChanged(HFP_AG_CONNECTION_STATE_CHANGED_EVT);
    connectionStateChanged.arg1_ = HFP_AG_STATE_DISCONNECTED;
    audioConnected->Dispatch(connectionStateChanged);

    connectionStateChanged.arg1_ = HFP_AG_STATE_CONNECTING;
    audioConnected->Dispatch(connectionStateChanged);

    stateMachine.deferMsgs_.clear();
    HfpAgMessage stateChanged(HFP_AG_AUDIO_STATE_CHANGED_EVT);
    stateChanged.arg1_ = HFP_AG_AUDIO_STATE_DISCONNECTED;
    audioConnected->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_AUDIO_STATE_DISCONNECTING;
    audioConnected->Dispatch(stateChanged);

    stateChanged.arg1_ = HFP_AG_AUDIO_STATE_CONNECTED;
    audioConnected->Dispatch(stateChanged);

    HfpAgMessage invalidEvt(HFP_AG_INVALID_EVT);
    audioConnected->Dispatch(invalidEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0002 start");

    HfpAgMessage disconnectEvt(HFP_AG_DISCONNECT_EVT);
    audioConnected->Dispatch(disconnectEvt);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, Disconnect(_)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));

    audioConnected->Dispatch(disconnectEvt);

    audioConnected->Dispatch(disconnectEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0003 start");

    HfpAgMessage disconnectEvt(HFP_AG_DISCONNECT_AUDIO_EVT);
    audioConnected->Dispatch(disconnectEvt);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, DisconnectAudio(_)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));

    audioConnected->Dispatch(disconnectEvt);

    audioConnected->Dispatch(disconnectEvt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0003 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0004
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0004, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0004 start");

    HfpAgMessage evt(HFP_AG_SET_VOLUME_EVT);
    audioConnected->Dispatch(evt);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, VolumeControl(_, _, _)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));

    evt.arg3_ = 1;
    audioConnected->Dispatch(evt);

    evt.arg3_ = 2;
    audioConnected->Dispatch(evt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0004 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0005
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0005, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0005 start");

    HfpAgMessage evt(HFP_AG_CALL_STATE_CHANGE);
    audioConnected->Dispatch(evt);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, PhoneStateChange(_, _, _, _, _, _, _)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));

    audioConnected->Dispatch(evt);

    audioConnected->Dispatch(evt);

    HILOGI("Hfp_Ag_Statemachine_HfpAgAudioConnected_Dispatch_0005 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_ProcessTransition_0001
 * @tc.name: ProcessTransition
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_ProcessTransition_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_ProcessTransition_0001 start");
    connectedState->ProcessTransition(HfpAgStateMachine::CONNECTED);
    disconnectedState->ProcessTransition(HfpAgStateMachine::DISCONNECTED);
    audioConnected->ProcessTransition(HfpAgStateMachine::AUDIO_CONNECTED);

    HILOGI("Hfp_Ag_Statemachine_ProcessTransition_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_GetStateName_0001
 * @tc.name: GetStateName
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_GetStateName_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_GetStateName_0001 start");
    EXPECT_EQ(stateMachine.GetStateName(HFP_AG_STATE_DISCONNECTED), "Disconnected");
    EXPECT_EQ(stateMachine.GetStateName(HFP_AG_STATE_CONNECTING), "Connecting");
    EXPECT_EQ(stateMachine.GetStateName(HFP_AG_STATE_DISCONNECTING), "Disconnecting");
    EXPECT_EQ(stateMachine.GetStateName(HFP_AG_STATE_CONNECTED), "Connected(AudioDisconnected)");
    EXPECT_EQ(stateMachine.GetStateName(HFP_AG_AUDIO_STATE_CONNECTING), "AudioConnecting");
    EXPECT_EQ(stateMachine.GetStateName(HFP_AG_AUDIO_STATE_DISCONNECTING), "AudioDisconnecting");
    EXPECT_EQ(stateMachine.GetStateName(HFP_AG_AUDIO_STATE_CONNECTED), "AudioConnected");
    EXPECT_EQ(stateMachine.GetStateName(-1), "Unknown");
 
    HILOGI("Hfp_Ag_Statemachine_GetStateName_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Statemachine_SetTryReconnectHfp_0001
 * @tc.name: SetTryReconnectHfp
 * @tc.desc:
 */
HWTEST_F(HfpAgStatemachineTest, Hfp_Ag_Statemachine_SetTryReconnectHfp_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Statemachine_SetTryReconnectHfp_0001 start");

    stateMachine.SetTryReconnectHfp(false);
    EXPECT_FALSE(stateMachine.GetTryReconnectHfp());

    HILOGI("Hfp_Ag_Statemachine_SetTryReconnectHfp_0001 end");
}

}  // namespace bluetooth
}  // namespace OHOS