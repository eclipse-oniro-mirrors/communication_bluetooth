/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd. All rights reserved.
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
#include "hfp_hf_service.h"
#ifdef BLUETOOTH_WATCH_ENABLE
#include "watch_service.h"
#endif
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "profile_service_manager.h"

using namespace testing;
using namespace testing::ext;

static bt_status_t g_status = BT_STATUS_FAIL;
static void SetStatus(bt_status_t status) { g_status = status; }

extern "C" {
static bt_status_t InitMock(bthf_client_callbacks_t* callbacks){ return g_status; }

static bt_status_t ConnectMock(RawAddress* bd_addr){ return g_status; }

static bt_status_t DisconnectMock(const RawAddress* bd_addr){ return g_status; }

static bt_status_t ConnectAudioMock(const RawAddress* bd_addr){ return g_status; }

static bt_status_t DisconnectAudioMock(const RawAddress* bd_addr){ return g_status; }

static bt_status_t StartVoiceRecognitionMock(const RawAddress* bd_addr){ return g_status; }

static bt_status_t StopVoiceRecognitionMock(const RawAddress* bd_addr){ return g_status; }

static bt_status_t VolumeControlMock(const RawAddress* bd_addr,
    bthf_client_volume_type_t type, int volume){ return g_status; }

static bt_status_t DialMock(const RawAddress* bd_addr, const char* number){ return g_status; }

static bt_status_t DialMemoryMock(const RawAddress* bd_addr, int location){ return g_status; }

static bt_status_t HandleCallActionMock(const RawAddress* bd_addr,
    bthf_client_call_action_t action, int idx){ return g_status; }

static bt_status_t QueryCurrentCallsMock(const RawAddress* bd_addr){ return g_status; }

static bt_status_t QueryCurrentOperatorNameMock(const RawAddress* bd_addr){ return g_status; }

static bt_status_t RetrieveSubscriberInfoMock(const RawAddress* bd_addr){ return g_status; }

static bt_status_t SendDtmfMock(const RawAddress* bd_addr, char code){ return g_status; }

static bt_status_t RequestLastVoiceTagNumberMock(const RawAddress* bd_addr){ return g_status; }

static void CleanUpMock(void){ }

static bt_status_t SendAtCmdMock(const RawAddress* bd_addr, int cmd, int val1,
    int val2, const char* arg){ return g_status; }

static bthf_client_interface_t g_bthfClientInterface = {
    .init = InitMock,
    .connect = ConnectMock,
    .disconnect = DisconnectMock,
    .connect_audio = ConnectAudioMock,
    .disconnect_audio = DisconnectAudioMock,
    .start_voice_recognition = StartVoiceRecognitionMock,
    .stop_voice_recognition = StopVoiceRecognitionMock,
    .volume_control = VolumeControlMock,
    .dial = DialMock,
    .dial_memory = DialMemoryMock,
    .handle_call_action = HandleCallActionMock,
    .query_current_calls = QueryCurrentCallsMock,
    .query_current_operator_name = QueryCurrentOperatorNameMock,
    .retrieve_subscriber_info = RetrieveSubscriberInfoMock,
    .send_dtmf = SendDtmfMock,
    .request_last_voice_tag_number = RequestLastVoiceTagNumberMock,
    .cleanup = CleanUpMock,
    .send_at_cmd = SendAtCmdMock,
};
}

namespace OHOS {
namespace bluetooth {

std::string address("11:22:33:44:55:66");
HfpHfStateMachine stateMachine(address);
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;

class HfpHfStatemachineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        host_ = &Bluetooth::BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        stateMachine.Init();
        disconnectedState =
            std::make_unique<HfpHfDisconnected>(DISCONNECTED, stateMachine);
        connectingState =
            std::make_unique<HfpHfConnecting>(CONNECTING, stateMachine);
        connectedState =
            std::make_unique<HfpHfConnected>(CONNECTED, stateMachine);
        audioConnected = std::make_unique<HfpHfAudioConnected>(AUDIO_CONNECTED,
            stateMachine, *connectedState);

        auto manager = ProfileServiceManager::GetInstance();
        string name = "HfpHfService";
        manager->SetProfileService(ADAPTER_BREDR, name);
    }
    void TearDown()
    {
        host_ = nullptr;
        stateMachine.deferMsgs_.clear();
        disconnectedState.reset();
        connectingState.reset();
        connectedState.reset();
        audioConnected.reset();
    }
public:
    std::unique_ptr<HfpHfDisconnected> disconnectedState;
    std::unique_ptr<HfpHfConnecting> connectingState;
    std::unique_ptr<HfpHfConnected> connectedState;
    std::unique_ptr<HfpHfAudioConnected> audioConnected;
};

/*
 * @tc.number: Hfp_Hf_Statemachine_GetEventName_0001
 * @tc.name: GetEventName
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_GetEventName_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_GetEventName_0001 start");

    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_SERVICE_STARTUP_EVT), "HFP_HF_SERVICE_STARTUP_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_SERVICE_SHUTDOWN_EVT), "HFP_HF_SERVICE_SHUTDOWN_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_REMOVE_STATE_MACHINE_EVT), "HFP_HF_REMOVE_STATE_MACHINE_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_INTERACTIVE_EVT), "HFP_HF_INTERACTIVE_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_SEND_DTMF_EVT), "HFP_HF_SEND_DTMF_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_ACCEPT_CALL_EVT), "HFP_HF_ACCEPT_CALL_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_HOLD_CALL_EVT), "HFP_HF_HOLD_CALL_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_REJECT_CALL_EVT), "HFP_HF_REJECT_CALL_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_HANDLE_INCOMING_CALL_EVT), "HFP_HF_HANDLE_INCOMING_CALL_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_HANDLE_MULTI_CALL_EVT), "HFP_HF_HANDLE_MULTI_CALL_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_DIAL_LAST_NUMBER), "HFP_HF_DIAL_LAST_NUMBER");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_DIAL_MEMORY), "HFP_HF_DIAL_MEMORY");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_FINISH_CALL_EVT), "HFP_HF_FINISH_CALL_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_DIAL_CALL_EVT), "HFP_HF_DIAL_CALL_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_SEND_AT_COMMAND_EVT), "HFP_HF_SEND_AT_COMMAND_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_OPEN_VOICE_RECOGNITION_EVT), "HFP_HF_OPEN_VOICE_RECOGNITION_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_CLOSE_VOICE_RECOGNITION_EVT), "HFP_HF_CLOSE_VOICE_RECOGNITION_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_SET_VOLUME_EVT), "HFP_HF_SET_VOLUME_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_BATTERY_LEVEL_CHANGED_EVT), "HFP_HF_BATTERY_LEVEL_CHANGED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_ENHANCED_DRIVER_SAFETY_CHANGED_EVT),
        "HFP_HF_ENHANCED_DRIVER_SAFETY_CHANGED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_SDP_DISCOVERY_RESULT_SUCCESS), "HFP_HF_SDP_DISCOVERY_RESULT_SUCCESS");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_SDP_DISCOVERY_RESULT_FAIL), "HFP_HF_SDP_DISCOVERY_RESULT_FAIL");
    
    HILOGI("Hfp_Hf_Statemachine_GetEventName_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_GetVolume_0001
 * @tc.name: GetVolume
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_GetVolume_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_GetVolume_0001 start");
    stateMachine.volume_ = 10;
    EXPECT_EQ(stateMachine.GetVolume(), 10);
    HILOGI("Hfp_Hf_Statemachine_GetVolume_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_GetConnectEventName_0001
 * @tc.name: GetConnectEventName
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_GetConnectEventName_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_GetConnectEventName_0001 start");

    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_CONNECT_EVT), "HFP_HF_CONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_DISCONNECT_EVT), "HFP_HF_DISCONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_CONNECT_AUDIO_EVT), "HFP_HF_CONNECT_AUDIO_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_DISCONNECT_AUDIO_EVT), "HFP_HF_DISCONNECT_AUDIO_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_RETRY_CONNECT_AUDIO_EVT), "HFP_HF_RETRY_CONNECT_AUDIO_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_CONNECTION_TIMEOUT_EVT), "HFP_HF_CONNECTION_TIMEOUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_DISCONNECT_TIMEOUT_EVT), "HFP_HF_DISCONNECT_TIMEOUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_CONNECT_AUDIO_TIMEOUT_EVT), "HFP_HF_CONNECT_AUDIO_TIMEOUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_DISCONNECT_AUDIO_TIMEOUT_EVT), "HFP_HF_DISCONNECT_AUDIO_TIMEOUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_CONNECTED_EVT), "HFP_HF_CONNECTED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_DISCONNECTED_EVT), "HFP_HF_DISCONNECTED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_SLC_ESTABLISHED_EVT), "HFP_HF_SLC_ESTABLISHED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_AUDIO_CONNECTING_EVT), "HFP_HF_AUDIO_CONNECTING_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_AUDIO_CONNECTED_EVT), "HFP_HF_AUDIO_CONNECTED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_AUDIO_CONNECTED_MSBC_EVT), "HFP_HF_AUDIO_CONNECTED_MSBC_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HFP_HF_AUDIO_DISCONNECTED_EVT), "HFP_HF_AUDIO_DISCONNECTED_EVT");


    HILOGI("Hfp_Hf_Statemachine_GetConnectEventName_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_GetInteractiveEventName_0001
 * @tc.name: GetInteractiveEventName
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_GetInteractiveEventName_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_GetInteractiveEventName_0001 start");

    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_NETWORK_STATE), "HFP_HF_TYPE_NETWORK_STATE");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_NETWORK_ROAM), "HFP_HF_TYPE_NETWORK_ROAM");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_NETWORK_SIGNAL), "HFP_HF_TYPE_NETWORK_SIGNAL");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_BATTERY_LEVEL), "HFP_HF_TYPE_BATTERY_LEVEL");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_CURRENT_OPERATOR), "HFP_HF_TYPE_CURRENT_OPERATOR");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_CALL_STATE), "HFP_HF_TYPE_CALL_STATE");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_CALL_SETUP_STATE), "HFP_HF_TYPE_CALL_SETUP_STATE");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_CALL_HELD_STATE), "HFP_HF_TYPE_CALL_HELD_STATE");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_HOLD_RESULT), "HFP_HF_TYPE_HOLD_RESULT");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION),
        "HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_CALL_WAITING), "HFP_HF_TYPE_CALL_WAITING");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_CURRENT_CALLS), "HFP_HF_TYPE_CURRENT_CALLS");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_SET_VOLUME), "HFP_HF_TYPE_SET_VOLUME");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_AT_CMD_RESULT), "HFP_HF_TYPE_AT_CMD_RESULT");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_SUBSCRIBER_NUMBER), "HFP_HF_TYPE_SUBSCRIBER_NUMBER");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_INBAND_RING), "HFP_HF_TYPE_INBAND_RING");
    EXPECT_EQ(stateMachine.GetInteractiveEventName(HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE),
        "HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE");

    HILOGI("Hfp_Hf_Statemachine_GetInteractiveEventName_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_NotifyStateTransitions_0001
 * @tc.name: NotifyStateTransitions
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_NotifyStateTransitions_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_NotifyStateTransitions_0001 start");

    HfpHfDisconnected disconnected(DISCONNECTED, stateMachine);
    HfpHfConnecting connecting(CONNECTING, stateMachine);
    HfpHfConnected connected(CONNECTED, stateMachine);

    stateMachine.current_ = &connected;
    stateMachine.preState_ = HFP_HF_STATE_CONNECTING;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_HF_STATE_CONNECTED);

    stateMachine.current_ = &connected;
    stateMachine.preState_ = HFP_HF_STATE_DISCONNECTED;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_HF_STATE_CONNECTED);

    stateMachine.current_ = &disconnected;
    stateMachine.preState_ = HFP_HF_STATE_DISCONNECTING;
    stateMachine.NotifyStateTransitions();
    EXPECT_EQ(stateMachine.preState_, HFP_HF_STATE_DISCONNECTED);

    HILOGI("Hfp_Hf_Statemachine_NotifyStateTransitions_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfDisconnected_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfDisconnected_Entry_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfDisconnected_Entry_0001 start");

    disconnectedState->Entry();
    EXPECT_FALSE(stateMachine.isRemoving_);

    HfpHfDisconnected disconnected(DISCONNECTED, stateMachine);
    stateMachine.current_ = &disconnected;
    disconnectedState->Exit();
    disconnectedState->Entry();
    EXPECT_TRUE(stateMachine.isRemoving_);

    HILOGI("Hfp_Hf_Statemachine_HfpHfDisconnected_Entry_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfDisconnected_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfDisconnected_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfDisconnected_Dispatch_0001 start");

    SetStatus(BT_STATUS_FAIL);
    HfpHfMessage connectEvt(HFP_HF_CONNECT_EVT);
    disconnectedState->Dispatch(connectEvt);

    HfpHfMessage connectedEvt(HFP_HF_CONNECTED_EVT);
    disconnectedState->Dispatch(connectedEvt);

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;
    disconnectedState->Dispatch(connectEvt);
    disconnectedState->Dispatch(connectedEvt);

    SetStatus(BT_STATUS_SUCCESS);
    disconnectedState->Dispatch(connectEvt);
    disconnectedState->Dispatch(connectedEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfDisconnected_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnecting_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnecting_Entry_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnecting_Entry_0001 start");

    connectingState->Entry();
    connectingState->Exit();

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnecting_Entry_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnecting_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnecting_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnecting_Dispatch_0001 start");

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;

    HfpHfMessage connectEvt(HFP_HF_CONNECT_EVT);
    connectingState->Dispatch(connectEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 1);

    HfpHfMessage disconnectEvt(HFP_HF_DISCONNECT_EVT);
    connectingState->Dispatch(disconnectEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 2);

    HfpHfMessage audioConnectedEvt(HFP_HF_AUDIO_CONNECTED_EVT);
    connectingState->Dispatch(audioConnectedEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 3);

    HfpHfMessage audioDisconnectedEvt(HFP_HF_AUDIO_DISCONNECTED_EVT);
    connectingState->Dispatch(audioDisconnectedEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 4);

    HfpHfMessage batteryLevelChangedEvt(HFP_HF_BATTERY_LEVEL_CHANGED_EVT);
    connectingState->Dispatch(batteryLevelChangedEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 5);

    HfpHfMessage interactiveEvt(HFP_HF_INTERACTIVE_EVT);
    connectingState->Dispatch(interactiveEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 6);

    HfpHfMessage audioConnectedMsbcEvt(HFP_HF_AUDIO_CONNECTED_MSBC_EVT);
    connectingState->Dispatch(audioConnectedMsbcEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 7);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnecting_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnecting_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnecting_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnecting_Dispatch_0002 start");

    HfpHfMessage slcEstablishedEvt(HFP_HF_SLC_ESTABLISHED_EVT);
    slcEstablishedEvt.arg1_ = 0;
    connectingState->Dispatch(slcEstablishedEvt);

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;

    HfpHfMessage disconnectedEvt(HFP_HF_DISCONNECTED_EVT);
    connectingState->Dispatch(disconnectedEvt);

    connectingState->Dispatch(slcEstablishedEvt);

    slcEstablishedEvt.arg1_ = HFP_HF_AG_FEATURES_ENHANCED_CALL_STATUS | HFP_HF_AG_FEATURES_ECNR;
    connectingState->Dispatch(slcEstablishedEvt);

    HfpHfMessage connectionTimeoutEvt(HFP_HF_CONNECTION_TIMEOUT_EVT);
    connectingState->Dispatch(connectionTimeoutEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnecting_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Entry_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Entry_0001 start");

    connectedState->Entry();

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Entry_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0001 start");

    SetStatus(BT_STATUS_FAIL);
    HfpHfMessage connectAudioEvt(HFP_HF_CONNECT_AUDIO_EVT);
    connectedState->Dispatch(connectAudioEvt);

#ifdef BLUETOOTH_WATCH_ENABLE
    WatchService::GetInstance()->SetIsHfOnCall(address, true);
#endif
    connectedState->Dispatch(connectAudioEvt);

    HfpHfMessage retryConnectAudioEvt(HFP_HF_RETRY_CONNECT_AUDIO_EVT);
    connectedState->Dispatch(retryConnectAudioEvt);

    HfpHfMessage disconnectEvt(HFP_HF_DISCONNECT_EVT);
    connectedState->Dispatch(disconnectEvt);

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;

    connectedState->Dispatch(connectAudioEvt);
    connectedState->Dispatch(retryConnectAudioEvt);
    connectedState->Dispatch(disconnectEvt);

    SetStatus(BT_STATUS_SUCCESS);
    connectedState->Dispatch(connectAudioEvt);
    connectedState->Dispatch(retryConnectAudioEvt);
    connectedState->Dispatch(disconnectEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0002 start");

    HfpHfMessage audioConnectedEvt(HFP_HF_AUDIO_CONNECTED_EVT);
    connectedState->Dispatch(audioConnectedEvt);

    HfpHfMessage disconnectedEvt(HFP_HF_DISCONNECTED_EVT);
    connectedState->Dispatch(disconnectedEvt);

#ifdef BLUETOOTH_WATCH_ENABLE
    WatchService::GetInstance()->SetAllowHfScoConnect(address, true);
#endif

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;

    connectedState->Dispatch(audioConnectedEvt);

    HfpHfMessage audioConnectedMsbcEvt(HFP_HF_AUDIO_CONNECTED_MSBC_EVT);
    connectedState->Dispatch(audioConnectedMsbcEvt);

    connectedState->Dispatch(disconnectedEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0003, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0003 start");

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;

    HfpHfMessage interactiveEvt(HFP_HF_INTERACTIVE_EVT);

    stateMachine.networkState_ = HFP_HF_NETWORK_STATE_AVAILABLE;
    interactiveEvt.type_ = HFP_HF_TYPE_NETWORK_STATE;
    interactiveEvt.arg1_ = HFP_HF_NETWORK_STATE_NOT_AVAILABLE;
    connectedState->Dispatch(interactiveEvt);

    stateMachine.networkTRoam_ = 0;
    interactiveEvt.type_ = HFP_HF_TYPE_NETWORK_ROAM;
    connectedState->Dispatch(interactiveEvt);

    stateMachine.networkSignal_ = 0;
    interactiveEvt.type_ = HFP_HF_TYPE_NETWORK_SIGNAL;
    connectedState->Dispatch(interactiveEvt);

    stateMachine.batteryLevel_ = 0;
    interactiveEvt.type_ = HFP_HF_TYPE_BATTERY_LEVEL;
    connectedState->Dispatch(interactiveEvt);

    stateMachine.operatorName_ = "0";
    interactiveEvt.type_ = HFP_HF_TYPE_CURRENT_OPERATOR;
    interactiveEvt.str_ = "1";
    connectedState->Dispatch(interactiveEvt);

    stateMachine.isInBandRing_ = false;
    interactiveEvt.type_ = HFP_HF_TYPE_INBAND_RING;
    interactiveEvt.arg1_ = 1;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_SET_VOLUME;
    interactiveEvt.arg1_ = HFP_HF_VOLUME_TYPE_SPK;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.arg1_ = HFP_HF_VOLUME_TYPE_MIC;
    connectedState->Dispatch(interactiveEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0003 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0004, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0004 start");

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;

    HfpHfMessage interactiveEvt(HFP_HF_INTERACTIVE_EVT);
    interactiveEvt.type_ = HFP_HF_TYPE_CALL_STATE;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_CALL_SETUP_STATE;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_CALL_HELD_STATE;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_HOLD_RESULT;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_CALL_WAITING;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_CURRENT_CALLS;
    interactiveEvt.call_.index = 0;
    interactiveEvt.call_.status = HFP_CALL_STATE_INCOMING;
    interactiveEvt.call_.number = "10086";
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.call_.number = "00000000000";
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.call_.number = "10000000";
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_SUBSCRIBER_NUMBER;
    interactiveEvt.str_ ="10086";
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_CURRENT_CALLS;
    interactiveEvt.call_.number = "10086";
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_CURRENT_CALLS;
    interactiveEvt.call_.number = "10085";
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.type_ = HFP_HF_TYPE_VOICE_RECOGNITION_CHANGED;
    interactiveEvt.arg1_ = 0;
    connectedState->Dispatch(interactiveEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0004 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0005
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0005, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0005 start");

    stateMachine.queuedActions.push(HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE);
    stateMachine.queuedActions.push(HFP_HF_OPEN_VOICE_RECOGNITION_EVT);
    stateMachine.queuedActions.push(HFP_HF_CLOSE_VOICE_RECOGNITION_EVT);
    stateMachine.queuedActions.push(HFP_HF_DIAL_CALL_EVT);
    stateMachine.queuedActions.push(HFP_HF_DIAL_CALL_EVT);
    stateMachine.queuedActions.push(HFP_HF_FINISH_CALL_EVT);
    stateMachine.queuedActions.push(HFP_HF_REJECT_CALL_EVT);

    HfpHfMessage interactiveEvt(HFP_HF_INTERACTIVE_EVT);
    interactiveEvt.type_ = HFP_HF_TYPE_AT_CMD_RESULT;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.arg1_ = HFP_HF_AT_RESULT_OK;
    connectedState->Dispatch(interactiveEvt);

    connectedState->Dispatch(interactiveEvt);

#ifdef BLUETOOTH_WATCH_ENABLE
    WatchService::GetInstance()->SetDialedCallNumber("10086");
#endif
    interactiveEvt.arg1_ = BTHF_CLIENT_CMD_COMPLETE_OK;
    connectedState->Dispatch(interactiveEvt);

    interactiveEvt.arg1_ = BTHF_CLIENT_CMD_COMPLETE_ERROR_CME;
    connectedState->Dispatch(interactiveEvt);

    connectedState->Dispatch(interactiveEvt);

    connectedState->Dispatch(interactiveEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0005 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0006
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0006, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0005 start");

    SetStatus(BT_STATUS_FAIL);
    HfpHfMessage sendDtmfEvt(HFP_HF_SEND_DTMF_EVT);
    sendDtmfEvt.arg1_ = 0;
    connectedState->Dispatch(sendDtmfEvt);

    HfpHfMessage setVolumeEvt(HFP_HF_SET_VOLUME_EVT);
    setVolumeEvt.arg1_ = 2;
    setVolumeEvt.arg3_ = HFP_HF_VOLUME_TYPE_MIC;
    connectedState->Dispatch(setVolumeEvt);

    setVolumeEvt.arg3_ = HFP_HF_VOLUME_TYPE_SPK;
    connectedState->Dispatch(setVolumeEvt);

    HfpHfMessage openVREvt(HFP_HF_OPEN_VOICE_RECOGNITION_EVT);
    stateMachine.voiceRecognitionStatus_ = HFP_HF_VR_STATE_CLOSED;
    connectedState->Dispatch(openVREvt);

    HfpHfMessage closeVREvt(HFP_HF_CLOSE_VOICE_RECOGNITION_EVT);
    stateMachine.voiceRecognitionStatus_ = HFP_HF_VR_STATE_OPENED;
    connectedState->Dispatch(closeVREvt);

    HfpHfMessage batteryLevelChangedEvt(HFP_HF_BATTERY_LEVEL_CHANGED_EVT);
    batteryLevelChangedEvt.arg1_ = 5;
    connectedState->Dispatch(batteryLevelChangedEvt);

    HfpHfMessage sendKeyPressedEvt(HFP_HF_SEND_KEY_PRESSED);
    connectedState->Dispatch(sendKeyPressedEvt);

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;
    connectedState->Dispatch(sendDtmfEvt);
    connectedState->Dispatch(setVolumeEvt);
    stateMachine.voiceRecognitionStatus_ = HFP_HF_VR_STATE_CLOSED;
    connectedState->Dispatch(openVREvt);
    stateMachine.voiceRecognitionStatus_ = HFP_HF_VR_STATE_OPENED;
    connectedState->Dispatch(closeVREvt);
    connectedState->Dispatch(batteryLevelChangedEvt);
    connectedState->Dispatch(sendKeyPressedEvt);

    SetStatus(BT_STATUS_SUCCESS);
    connectedState->Dispatch(sendDtmfEvt);
    connectedState->Dispatch(setVolumeEvt);
    stateMachine.voiceRecognitionStatus_ = HFP_HF_VR_STATE_CLOSED;
    connectedState->Dispatch(openVREvt);
    stateMachine.voiceRecognitionStatus_ = HFP_HF_VR_STATE_OPENED;
    connectedState->Dispatch(closeVREvt);
    connectedState->Dispatch(batteryLevelChangedEvt);
    connectedState->Dispatch(sendKeyPressedEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0006 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0007
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0007, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0007 start");

    SetStatus(BT_STATUS_FAIL);
    HandsFreeUnitCalls callIcomming(address, 0, HFP_CALL_STATE_INCOMING, "10086", false, false, true);
    stateMachine.calls_->UpdateCall(callIcomming);
    stateMachine.calls_->UpdateCallDone();
    HfpHfMessage acceptCallEvt(HFP_HF_ACCEPT_CALL_EVT);
    acceptCallEvt.arg1_ = HFP_HF_ACCEPT_CALL_ACTION_HOLD;
    connectedState->Dispatch(acceptCallEvt);

    acceptCallEvt.arg1_ = HFP_HF_ACCEPT_CALL_ACTION_NONE;
    connectedState->Dispatch(acceptCallEvt);

    HfpHfMessage holdCallEvt(HFP_HF_HOLD_CALL_EVT);
    connectedState->Dispatch(holdCallEvt);

    HfpHfMessage rejectCallEvt(HFP_HF_REJECT_CALL_EVT);
    connectedState->Dispatch(rejectCallEvt);

    HandsFreeUnitCalls callActive(address, 0, HFP_CALL_STATE_ACTIVE, "10086", false, false, true);
    stateMachine.calls_->UpdateCall(callActive);
    stateMachine.calls_->UpdateCallDone();

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;
    connectedState->Dispatch(acceptCallEvt);
    connectedState->Dispatch(holdCallEvt);
    connectedState->Dispatch(rejectCallEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0007 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0008
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0008, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0008 start");

    HandsFreeUnitCalls callWaiting(address, 0, BTHF_CLIENT_CALL_STATE_WAITING, "10086", false, false, true);
    stateMachine.calls_->UpdateCall(callWaiting);
    stateMachine.calls_->UpdateCallDone();
    HfpHfMessage acceptCallEvt(HFP_HF_ACCEPT_CALL_EVT);
    acceptCallEvt.arg1_ = HFP_HF_ACCEPT_CALL_ACTION_HOLD;
    connectedState->Dispatch(acceptCallEvt);

    acceptCallEvt.arg1_ = HFP_HF_ACCEPT_CALL_ACTION_NONE;
    connectedState->Dispatch(acceptCallEvt);

    acceptCallEvt.arg1_ = HFP_HF_ACCEPT_CALL_ACTION_FINISH;
    connectedState->Dispatch(acceptCallEvt);

    HfpHfMessage rejectCallEvt(HFP_HF_REJECT_CALL_EVT);
    connectedState->Dispatch(rejectCallEvt);

    HandsFreeUnitCalls callHeldByRespHold(address, 0,
        BTHF_CLIENT_CALL_STATE_HELD_BY_RESP_HOLD, "10086", false, false, true);
    stateMachine.calls_->UpdateCall(callHeldByRespHold);
    stateMachine.calls_->UpdateCallDone();
    acceptCallEvt.arg1_ = HFP_HF_ACCEPT_CALL_ACTION_NONE;
    connectedState->Dispatch(acceptCallEvt);

    connectedState->Dispatch(rejectCallEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0008 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0009
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0009, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0009 start");

    HandsFreeUnitCalls callHeld(address, 0, BTHF_CLIENT_CALL_STATE_HELD, "10086", false, false, true);
    stateMachine.calls_->UpdateCall(callHeld);
    stateMachine.calls_->UpdateCallDone();
    HfpHfMessage acceptCallEvt(HFP_HF_ACCEPT_CALL_EVT);
    acceptCallEvt.arg1_ = HFP_HF_ACCEPT_CALL_ACTION_FINISH;
    connectedState->Dispatch(acceptCallEvt);

    HfpHfMessage rejectCallEvt(HFP_HF_REJECT_CALL_EVT);
    connectedState->Dispatch(rejectCallEvt);

    HfpHfMessage finishCallEvt(HFP_HF_FINISH_CALL_EVT);
    connectedState->Dispatch(finishCallEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0009 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0010
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0010, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0010 start");

    HandsFreeUnitCalls callActive(address, 0, BTHF_CLIENT_CALL_STATE_ACTIVE, "10086", false, false, true);
    stateMachine.calls_->UpdateCall(callActive);
    stateMachine.calls_->UpdateCallDone();
    HfpHfMessage finishCallEvt(HFP_HF_FINISH_CALL_EVT);
    connectedState->Dispatch(finishCallEvt);

    HfpHfMessage holdCallEvt(HFP_HF_HOLD_CALL_EVT);
    connectedState->Dispatch(holdCallEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0010 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0011
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0011, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0011 start");

    SetStatus(BT_STATUS_FAIL);

    HfpHfMessage dailCallEvt(HFP_HF_DIAL_CALL_EVT);
    connectedState->Dispatch(dailCallEvt);

    HfpHfMessage queryCallsEvt(HFP_HF_QUERY_CURRENT_CALLS);
    connectedState->Dispatch(queryCallsEvt);

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;

    HandsFreeUnitCalls calls(address, 1, HFP_CALL_STATE_DIALING, "10086", false, true, true);
    dailCallEvt.dev_ = address;
    dailCallEvt.calls_ = calls;
    connectedState->Dispatch(dailCallEvt);
    connectedState->Dispatch(queryCallsEvt);

    SetStatus(BT_STATUS_SUCCESS);
    connectedState->Dispatch(dailCallEvt);
    connectedState->Dispatch(queryCallsEvt);

    connectedState->Dispatch(queryCallsEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfConnected_Dispatch_0011 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfAudioConnected_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfAudioConnected_Entry_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfAudioConnected_Entry_0001 start");

    audioConnected->Entry();

    HILOGI("Hfp_Hf_Statemachine_HfpHfAudioConnected_Entry_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfAudioConnected_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfAudioConnected_Dispatch_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfAudioConnected_Dispatch_0001 start");

    SetStatus(BT_STATUS_FAIL);
    HfpHfMessage disconnectAudioEvt(HFP_HF_DISCONNECT_AUDIO_EVT);
    audioConnected->Dispatch(disconnectAudioEvt);

    HfpHfService::GetService()->bluetoothHfInterface = &g_bthfClientInterface;

    HfpHfMessage disconnectedEvt(HFP_HF_DISCONNECTED_EVT);
    audioConnected->Dispatch(disconnectedEvt);

    HfpHfMessage audioDisconnectedEvt(HFP_HF_AUDIO_DISCONNECTED_EVT);
    audioConnected->Dispatch(audioDisconnectedEvt);

    audioConnected->Dispatch(disconnectAudioEvt);

    SetStatus(BT_STATUS_SUCCESS);
    audioConnected->Dispatch(disconnectAudioEvt);

    HfpHfMessage disconnectEvt(HFP_HF_DISCONNECT_EVT);
    audioConnected->Dispatch(disconnectEvt);
    EXPECT_EQ(stateMachine.deferMsgs_.size(), 1);

    HILOGI("Hfp_Hf_Statemachine_HfpHfAudioConnected_Dispatch_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_HfpHfAudioConnected_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HfpHfAudioConnected_Dispatch_0002, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HfpHfAudioConnected_Dispatch_0002 start");

    HandsFreeUnitCalls callIcomming(address, 0, HFP_CALL_STATE_INCOMING, "10086", false, false, true);
    stateMachine.calls_->UpdateCall(callIcomming);
    stateMachine.calls_->UpdateCallDone();
    HfpHfMessage holdCallEvt(HFP_HF_HOLD_CALL_EVT);
    connectedState->Dispatch(holdCallEvt);

    HILOGI("Hfp_Hf_Statemachine_HfpHfAudioConnected_Dispatch_0002 end");
}

/*
 * @tc.number: Hfp_Hf_Statemachine_NotifyChildStateToParentState
 * @tc.name: NotifyChildStateToParentState
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_NotifyChildStateToParentState, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_NotifyChildStateToParentState start");

    stateMachine.NotifyChildStateToParentState(HFP_HF_AUDIO_STATE_CONNECTED, HFP_HF_AUDIO_STATE_DISCONNECTED);
    EXPECT_EQ(stateMachine.preState_, HFP_HF_AUDIO_STATE_DISCONNECTED);

    stateMachine.NotifyChildStateToParentState(HFP_HF_AUDIO_STATE_DISCONNECTED, HFP_HF_AUDIO_STATE_CONNECTED);
    EXPECT_EQ(stateMachine.preState_, HFP_HF_AUDIO_STATE_CONNECTED);

    HILOGI("Hfp_Hf_Statemachine_NotifyChildStateToParentState end");
}

#ifdef BLUETOOTH_WATCH_ENABLE
/*
 * @tc.number: Hfp_Hf_Statemachine_HandleVoipCall
 * @tc.name: HandleVoipCall
 * @tc.desc:
 */
HWTEST_F(HfpHfStatemachineTest, Hfp_Hf_Statemachine_HandleVoipCall, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Statemachine_HandleVoipCall start");

    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT);
    event.dev_ = address;
    event.type_ = HFP_HF_TYPE_CURRENT_CALLS;
    event.call_.number = "00000000000";
    event.call_.status = HFP_CALL_STATE_INCOMING;

    EXPECT_TRUE(stateMachine.HandleVoipCall(event));

    event.call_.number = "10000000";
    EXPECT_TRUE(stateMachine.HandleVoipCall(event));

    stateMachine.subscriberNumber_ = "";
    event.call_.number = "";
    EXPECT_FALSE(stateMachine.HandleVoipCall(event));

    stateMachine.subscriberNumber_ = "123456789";
    event.call_.number = "";
    EXPECT_FALSE(stateMachine.HandleVoipCall(event));

    stateMachine.subscriberNumber_ = "123456789";
    event.call_.number = "123456789";
    EXPECT_TRUE(stateMachine.HandleVoipCall(event));

    stateMachine.subscriberNumber_ = "123456789";
    event.call_.number = "+123456789";
    EXPECT_TRUE(stateMachine.HandleVoipCall(event));

    sleep(TIME);

    HILOGI("Hfp_Hf_Statemachine_HandleVoipCall end");
}
#endif
}  // namespace bluetooth
}  // namespace OHOS