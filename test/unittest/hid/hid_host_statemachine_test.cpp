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
#include "hid_host_statemachine.h"
#include "hid_host_service.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

#include "mock/interface_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
using namespace bluetooth::headset;

namespace OHOS {
namespace bluetooth {
std::string address("11:22:33:44:55:66");
HidHostStateMachine stateMachine(address);

constexpr int TIME = 2;

class HidHostStatemachineTest : public testing::Test {
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
        disconnectedState =
            std::make_unique<HidHostDisconnectedState>(HidHostStateMachine::DISCONNECTED, stateMachine);
        connectingState =
            std::make_unique<HidHostConnectingState>(HidHostStateMachine::CONNECTING, stateMachine);
        disconnectingState =
            std::make_unique<HidHostDisconnectingState>(HidHostStateMachine::DISCONNECTING, stateMachine);
        connectedState =
            std::make_unique<HidHostConnectedState>(HidHostStateMachine::CONNECTED, stateMachine);

        // new HidHostService
        auto manager = ProfileServiceManager::GetInstance();
        string name = "HidHostService";
        manager->SetProfileService(ADAPTER_BREDR, name);
    }
    void TearDown()
    {
        disconnectedState.reset();
        connectingState.reset();
        disconnectingState.reset();
        connectedState.reset();

        // delete HidHostService
        auto hidHostService = HidHostService::GetService();
        delete hidHostService;
    }
public:
    std::unique_ptr<HidHostDisconnectedState> disconnectedState;
    std::unique_ptr<HidHostConnectingState> connectingState;
    std::unique_ptr<HidHostDisconnectingState> disconnectingState;
    std::unique_ptr<HidHostConnectedState> connectedState;
};

HWTEST_F(HidHostStatemachineTest, Hid_DisConnectedState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Hid_DisConnectedState_MachineTest_Dispatch_00100 start");

    HidHostMessage msg_profile_disconnection_time_out(HID_HOST_DISCONNECTION_TIMEOUT_EVT);

    EXPECT_TRUE(disconnectedState->Dispatch(msg_profile_disconnection_time_out));

    HILOGI("Hid_DisConnectedState_MachineTest_Dispatch_00100 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_DisConnectedState_MachineTest_Dispatch_00200, TestSize.Level1)
{
    HILOGI("Hid_DisConnectedState_MachineTest_Dispatch_00200 start");

    HidHostMessage msg_profile_connect_changed(HID_HOST_CONNECTION_STATE_CHANGED_EVT);
    msg_profile_connect_changed.sendData_.param = HID_HOST_STATE_CONNECTING;
    EXPECT_TRUE(disconnectedState->Dispatch(msg_profile_connect_changed));

    HidHostMessage msg_profile_connected_changed(HID_HOST_CONNECTION_STATE_CHANGED_EVT);
    msg_profile_connected_changed.sendData_.param = HID_HOST_STATE_CONNECTED;
    EXPECT_TRUE(disconnectedState->Dispatch(msg_profile_connected_changed));

    HILOGI("Hid_DisConnectedState_MachineTest_Dispatch_00200 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_ConnectedState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00100 start");

    HidHostMessage msg_profile_connect(HID_HOST_CONNECT_EVT);
    HidHostMessage msg_profile_state_changed(HID_HOST_CONNECTION_STATE_CHANGED_EVT);

    EXPECT_TRUE(connectedState->Dispatch(msg_profile_connect));

    msg_profile_state_changed.sendData_.param = HID_HOST_STATE_CONNECTED;
    EXPECT_TRUE(connectedState->Dispatch(msg_profile_state_changed));

    msg_profile_state_changed.sendData_.param = HID_HOST_STATE_DISCONNECTED;
    EXPECT_TRUE(connectedState->Dispatch(msg_profile_state_changed));

    HidHostMessage msg_profile_disconnection_time_out(HID_HOST_DISCONNECTION_TIMEOUT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(msg_profile_disconnection_time_out));

    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00100 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_ConnectingState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Hid_ConnectingState_MachineTest_Dispatch_00100 start");

    HidHostMessage msg_profile_connect(HID_HOST_CONNECT_EVT);
    EXPECT_TRUE(connectingState->Dispatch(msg_profile_connect));

    HidHostMessage msg_profile_disconnect(HID_HOST_DISCONNECT_EVT);
    EXPECT_TRUE(connectingState->Dispatch(msg_profile_disconnect));

    HidHostMessage msg_profile_state_changed(HID_HOST_CONNECTION_STATE_CHANGED_EVT);
    msg_profile_state_changed.sendData_.param = HID_HOST_STATE_CONNECTED;
    EXPECT_TRUE(connectingState->Dispatch(msg_profile_state_changed));

    msg_profile_state_changed.sendData_.param = HID_HOST_STATE_DISCONNECTED;
    EXPECT_TRUE(connectingState->Dispatch(msg_profile_state_changed));

    HidHostMessage msg_profile_disconnection_time_out(HID_HOST_DISCONNECTION_TIMEOUT_EVT);
    EXPECT_TRUE(connectingState->Dispatch(msg_profile_disconnection_time_out));

    HILOGI("Hid_ConnectingState_MachineTest_Dispatch_00100 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_DidconnectingState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Hid_DidconnectingState_MachineTest_Dispatch_00100 start");

    HidHostMessage msg_profile_connect(HID_HOST_CONNECT_EVT);
    EXPECT_TRUE(disconnectingState->Dispatch(msg_profile_connect));

    HidHostMessage msg_profile_disconnect(HID_HOST_DISCONNECT_EVT);
    EXPECT_TRUE(disconnectingState->Dispatch(msg_profile_disconnect));

    HidHostMessage msg_profile_state_changed(HID_HOST_CONNECTION_STATE_CHANGED_EVT);
    msg_profile_state_changed.sendData_.param = HID_HOST_STATE_CONNECTED;
    EXPECT_TRUE(disconnectingState->Dispatch(msg_profile_state_changed));

    msg_profile_state_changed.sendData_.param = HID_HOST_STATE_DISCONNECTED;
    EXPECT_TRUE(disconnectingState->Dispatch(msg_profile_state_changed));

    HidHostMessage msg_profile_disconnection_time_out(HID_HOST_DISCONNECTION_TIMEOUT_EVT);
    EXPECT_TRUE(disconnectingState->Dispatch(msg_profile_disconnection_time_out));

    HILOGI("Hid_DidconnectingState_MachineTest_Dispatch_00100 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_GetEventName_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Hid_GetEventName_MachineTest_Dispatch_00100 start");

    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_CONNECTION_TIMEOUT_EVT), "HID_HOST_CONNECTION_TIMEOUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_DISCONNECTION_TIMEOUT_EVT), "HID_HOST_DISCONNECTION_TIMEOUT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_CONNECT_EVT), "HID_HOST_CONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_DISCONNECT_EVT), "HID_HOST_DISCONNECT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_CONNECTION_STATE_CHANGED_EVT),
        "HID_HOST_CONNECTION_STATE_CHANGED_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_VC_UNPLUG_EVT), "HID_HOST_VC_UNPLUG_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_SEND_DATA_EVT), "HID_HOST_SEND_DATA_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_SET_REPORT_EVT), "HID_HOST_SET_REPORT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_GET_REPORT_EVT), "HID_HOST_GET_REPORT_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_INT_HANDSK_EVT), "HID_HOST_INT_HANDSK_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_REMOVE_STATE_MACHINE_EVT), "HID_HOST_REMOVE_STATE_MACHINE_EVT");
    EXPECT_EQ(stateMachine.GetEventName(HID_HOST_DEVICE_TYPE_UNKNOWN), "Unknown");

    HILOGI("Hid_GetEventName_MachineTest_Dispatch_00100 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_DisConnectedState_MachineTest_Dispatch_00300, TestSize.Level1)
{
    HILOGI("Hid_DisConnectedState_MachineTest_Dispatch_00300 start");

    HidHostMessage msg_connect_evt(HID_HOST_CONNECT_EVT);
    EXPECT_TRUE(disconnectedState->Dispatch(msg_connect_evt));

    bthh_interface_t interface;
    interface.connect = connect;
    HidHostService::GetService()->bluetoothHidInterface = &interface;
    EXPECT_TRUE(disconnectedState->Dispatch(msg_connect_evt));

    HILOGI("Hid_DisConnectedState_MachineTest_Dispatch_00300 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_ConnectedState_MachineTest_Dispatch_00200, TestSize.Level1)
{
    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00200 start");

    HidHostMessage msg_profile_disconnect(HID_HOST_DISCONNECT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(msg_profile_disconnect));

    bthh_interface_t interface;
    interface.disconnect = disconnect;
    HidHostService::GetService()->bluetoothHidInterface = &interface;
    EXPECT_TRUE(connectedState->Dispatch(msg_profile_disconnect));

    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00200 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_ConnectedState_MachineTest_Dispatch_00300, TestSize.Level1)
{
    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00300 start");

    HidHostMessage vc_unplug_evt(HID_HOST_VC_UNPLUG_EVT);
    EXPECT_TRUE(connectedState->Dispatch(vc_unplug_evt));

    bthh_interface_t interface;
    interface.virtual_unplug = virtual_unplug;
    HidHostService::GetService()->bluetoothHidInterface = &interface;
    EXPECT_TRUE(connectedState->Dispatch(vc_unplug_evt));

    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00300 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_ConnectedState_MachineTest_Dispatch_00400, TestSize.Level1)
{
    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00400 start");

    HidHostMessage set_report_evt(HID_HOST_SET_REPORT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(set_report_evt));

    bthh_interface_t interface;
    interface.set_report = set_report;
    HidHostService::GetService()->bluetoothHidInterface = &interface;
    EXPECT_TRUE(connectedState->Dispatch(set_report_evt));

    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00400 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_ConnectedState_MachineTest_Dispatch_00500, TestSize.Level1)
{
    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00500 start");

    HidHostMessage get_report_evt(HID_HOST_GET_REPORT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(get_report_evt));

    bthh_interface_t interface;
    interface.get_report = get_report;
    HidHostService::GetService()->bluetoothHidInterface = &interface;
    EXPECT_TRUE(connectedState->Dispatch(get_report_evt));

    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00500 end");
}

HWTEST_F(HidHostStatemachineTest, Hid_ConnectedState_MachineTest_Dispatch_00600, TestSize.Level1)
{
    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00600 start");

    HidHostMessage msg_connect_evt(HID_HOST_CONNECT_EVT);
    EXPECT_TRUE(connectedState->Dispatch(msg_connect_evt));

    bthh_interface_t interface;
    interface.connect = connect;
    HidHostService::GetService()->bluetoothHidInterface = &interface;
    EXPECT_TRUE(connectedState->Dispatch(msg_connect_evt));
    stateMachine.NotifyStateTransitions();
    HILOGI("Hid_ConnectedState_MachineTest_Dispatch_00600 end");
}
}  // namespace bluetooth
}  // namespace OHOS