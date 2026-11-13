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
#include "common/raw_address.h"
#include "adapter_manager.h"
#include "hid_host_statemachine.h"
#include "hid_host_service.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"
#include "profile_config.h"
#include "thread_util.h"
#include "bluetooth_connection_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;

const int TIME = 2;

static int GetRemoteServicesMock(STACK::RawAddress* remote_addr)
{
    HILOGI("GetRemoteServicesMock start");
    return 0;
}

class HidHostServiceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase start");
        host_ = nullptr;
        HILOGI("TearDownTestCase end");
    }
    void SetUp()
    {
        // new HidHostService
        auto manager = ProfileServiceManager::GetInstance();
        string name = "HidHostService";
        manager->SetProfileService(ADAPTER_BREDR, name);

        // Disable hid thread
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_HID_HOST, ThreadUtil::DISABLED);
    }
    void TearDown()
    {
        // delete HidHostService
        auto hidHostService = HidHostService::GetService();
        delete hidHostService;
    }
};

/*
 * @tc.number: Hid_Service_CovertConnectStateFromStack_00100
 * @tc.name: CovertConnectStateFromStack
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_CovertConnectStateFromStack_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_CovertConnectStateFromStack_00100 start");

    auto hidHostService = HidHostService::GetService();
    EXPECT_EQ(hidHostService->CovertConnectStateFromStack(BTHH_CONN_STATE_CONNECTING),
        HID_HOST_STATE_CONNECTING);
    EXPECT_EQ(hidHostService->CovertConnectStateFromStack(BTHH_CONN_STATE_DISCONNECTING),
        HID_HOST_STATE_DISCONNECTING);
    EXPECT_EQ(hidHostService->CovertConnectStateFromStack(BTHH_CONN_STATE_CONNECTED),
        HID_HOST_STATE_CONNECTED);
    EXPECT_EQ(hidHostService->CovertConnectStateFromStack(BTHH_CONN_STATE_UNKNOWN),
        HID_HOST_STATE_DISCONNECTED);

    HILOGI("Hid_Service_CovertConnectStateFromStack_00100 end");
}

/*
 * @tc.number: Hid_Service_ProcessEvent_00100
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_ProcessEvent_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_ProcessEvent_00100 start");
    auto hidHostService = HidHostService::GetService();

    HidHostMessage startUpEvent(HID_HOST_SERVICE_STARTUP_EVT);
    hidHostService->ProcessEvent(startUpEvent);
    hidHostService->isStarted_ = true;
    hidHostService->ProcessEvent(startUpEvent);

    HidHostMessage shutdownEvent(HID_HOST_SERVICE_SHUTDOWN_EVT);
    hidHostService->isStarted_ = false;
    hidHostService->ProcessEvent(shutdownEvent);

    hidHostService->isStarted_ = true;
    std::string address("00");
    HidHostStateMachine stateMachine(address);
    HidHostConnectedState connectedState(HidHostStateMachine::CONNECTED, stateMachine);
    HidHostConnectingState connectingState(HidHostStateMachine::CONNECTING, stateMachine);
    HidHostDisconnectedState disconnectedState(HidHostStateMachine::DISCONNECTED, stateMachine);
    HidHostDisconnectedState disconnectingState(HidHostStateMachine::DISCONNECTING, stateMachine);

    hidHostService->stateMachines_.EnsureInsert("1", nullptr);

    hidHostService->stateMachines_.EnsureInsert("2", std::make_shared<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal("2")->current_ = &connectingState;

    hidHostService->stateMachines_.EnsureInsert("3", std::make_shared<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal("3")->current_ = &disconnectedState;

    hidHostService->ProcessEvent(shutdownEvent);
    EXPECT_EQ(hidHostService->isShuttingDown_, true);

    hidHostService->stateMachines_.Erase("2");
    hidHostService->ProcessEvent(shutdownEvent);
    EXPECT_EQ(hidHostService->isShuttingDown_, false);

    hidHostService->maxConnectionsNum_ = 0;
    HidHostMessage connectEvent(HID_HOST_CONNECT_EVT);
    hidHostService->ProcessEvent(connectEvent);

    HidHostMessage connectChangedEvent(HID_HOST_CONNECTION_STATE_CHANGED_EVT);
    hidHostService->ProcessEvent(connectEvent);

    HILOGI("Hid_Service_ProcessEvent_00100 end");
}

/*
 * @tc.number: Hid_Service_ProcessEvent_00200
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_ProcessEvent_00200, TestSize.Level1)
{
    HILOGI("Hid_Service_ProcessEvent_00200 start");
    auto hidHostService = HidHostService::GetService();

    HidHostMessage removeStateEvent(HID_HOST_REMOVE_STATE_MACHINE_EVT);
    hidHostService->isShuttingDown_ = true;

    HidHostStateMachine stateMachine1("1");
    HidHostStateMachine stateMachine2("2");

    HidHostConnectingState connectingState(HidHostStateMachine::CONNECTING, stateMachine1);
    HidHostConnectedState connectedState(HidHostStateMachine::CONNECTED, stateMachine2);

    hidHostService->stateMachines_.EnsureInsert("1", std::make_shared<HidHostStateMachine>("1"));
    hidHostService->stateMachines_.ReadVal("1")->current_ = &connectingState;

    hidHostService->stateMachines_.EnsureInsert("2", std::make_shared<HidHostStateMachine>("2"));
    hidHostService->stateMachines_.ReadVal("2")->current_ = &connectedState;

    hidHostService->stateMachines_.EnsureInsert("3", nullptr);

    hidHostService->ProcessEvent(removeStateEvent);

    HidHostMessage invalidEvent(HID_HOST_INVALID_EVT);
    hidHostService->ProcessEvent(invalidEvent);
    EXPECT_TRUE(hidHostService->isShuttingDown_);

    HILOGI("Hid_Service_ProcessEvent_00200 end");
}

/*
 * @tc.number: Hid_Service_GetConnectedDeviceNum_00100
 * @tc.name: GetConnectedDeviceNum
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_GetConnectedDeviceNum_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_GetConnectedDeviceNum_00100 start");
    auto hidHostService = HidHostService::GetService();

    HidHostStateMachine stateMachine("00");

    HidHostConnectingState connectingState(HidHostStateMachine::CONNECTING, stateMachine);
    HidHostConnectedState connectedState(HidHostStateMachine::CONNECTED, stateMachine);
    HidHostDisconnectingState disconnectingState(HidHostStateMachine::DISCONNECTING, stateMachine);
    HidHostDisconnectedState disconnectedState(HidHostStateMachine::DISCONNECTED, stateMachine);

    hidHostService->stateMachines_.EnsureInsert("1", nullptr);
    EXPECT_EQ(hidHostService->GetConnectedDeviceNum(), 0);

    hidHostService->stateMachines_.EnsureInsert("2", std::make_shared<HidHostStateMachine>("2"));
    hidHostService->stateMachines_.ReadVal("2")->current_ = &connectingState;

    hidHostService->stateMachines_.EnsureInsert("3", std::make_shared<HidHostStateMachine>("3"));
    hidHostService->stateMachines_.ReadVal("3")->current_ = &connectedState;

    hidHostService->stateMachines_.EnsureInsert("4", std::make_shared<HidHostStateMachine>("4"));
    hidHostService->stateMachines_.ReadVal("4")->current_ = &disconnectingState;

    hidHostService->stateMachines_.EnsureInsert("5", std::make_shared<HidHostStateMachine>("5"));
    hidHostService->stateMachines_.ReadVal("5")->current_ = &disconnectedState;

    EXPECT_EQ(hidHostService->GetConnectedDeviceNum(), 2);

    HILOGI("Hid_Service_GetConnectedDeviceNum_00100 end");
}

/*
 * @tc.number: Hid_Service_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_Connect_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_Connect_00100 start");

    auto hidHostService = HidHostService::GetService();
    BluetoothRawAddress device("00:00:00:00:00:00");

    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    if (remoteDevice != nullptr) {
        std::vector<Uuid> remoteUuids;
        remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HID_HOST));
        remoteDevice->SetDeviceUuids(remoteUuids);
    }
    hidHostService->stateMachines_.EnsureInsert(device.GetAddress(), nullptr);
    EXPECT_EQ(hidHostService->Connect(device), HID_HOST_SUCCESS);

    hidHostService->stateMachines_.EnsureInsert(
        device.GetAddress(), std::make_unique<HidHostStateMachine>(device.GetAddress()));
    hidHostService->stateMachines_.ReadVal(device.GetAddress())->Init();

    std::string address(device.GetAddress());
    HidHostStateMachine stateMachine(address);
    HidHostConnectedState connectedState(HidHostStateMachine::CONNECTED, stateMachine);
    HidHostConnectingState connectingState(HidHostStateMachine::CONNECTING, stateMachine);
    HidHostDisconnectedState disconnectedState(HidHostStateMachine::DISCONNECTED, stateMachine);
    HidHostDisconnectedState disconnectingState(HidHostStateMachine::DISCONNECTING, stateMachine);

    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &connectedState;
    EXPECT_EQ(hidHostService->Connect(device), BT_ERR_INTERNAL_ERROR);

    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &connectingState;
    EXPECT_EQ(hidHostService->Connect(device), BT_ERR_INTERNAL_ERROR);

    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &disconnectedState;
    EXPECT_EQ(hidHostService->Connect(device), HID_HOST_SUCCESS);

    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &disconnectingState;
    EXPECT_EQ(hidHostService->Connect(device), HID_HOST_SUCCESS);

    hidHostService->maxConnectionsNum_ = -1;
    EXPECT_EQ(hidHostService->Connect(device), BT_ERR_MAX_CONNECTION);

    HILOGI("Hid_Service_Connect_00100 end");
}

/*
 * @tc.number: Hid_Service_Connect_00200
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_Connect_00200, TestSize.Level1)
{
    HILOGI("Hid_Service_Connect_00200 start");

    auto hidHostService = HidHostService::GetService();
    BluetoothRawAddress device("00");

    EXPECT_NE(hidHostService->Connect(device), Bluetooth::BT_NO_ERROR);

    HILOGI("Hid_Service_Connect_00200 end");
}

/*
 * @tc.number: Hid_Service_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_Disconnect_00100 start");
    auto hidHostService = HidHostService::GetService();
    BluetoothRawAddress device("00");
    EXPECT_EQ(hidHostService->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);

    hidHostService->stateMachines_.EnsureInsert(device.GetAddress(), nullptr);
    EXPECT_EQ(hidHostService->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);

    std::string address(device.GetAddress());
    HidHostStateMachine stateMachine(address);
    HidHostConnectingState connectingState(HidHostStateMachine::CONNECTING, stateMachine);
    HidHostConnectedState connectedState(HidHostStateMachine::CONNECTED, stateMachine);
    HidHostDisconnectingState disconnectingState(HidHostStateMachine::DISCONNECTING, stateMachine);
    HidHostDisconnectedState disconnectedState(HidHostStateMachine::DISCONNECTED, stateMachine);

    hidHostService->stateMachines_.EnsureInsert(
        device.GetAddress(), std::make_shared<HidHostStateMachine>(device.GetAddress()));
    hidHostService->stateMachines_.ReadVal(device.GetAddress())->Init();
    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &connectingState;
    EXPECT_EQ(hidHostService->Disconnect(device), HID_HOST_SUCCESS);

    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &connectedState;
    EXPECT_EQ(hidHostService->Disconnect(device), HID_HOST_SUCCESS);

    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &disconnectingState;
    EXPECT_EQ(hidHostService->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);

    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &disconnectedState;
    EXPECT_EQ(hidHostService->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);

    HILOGI("Hid_Service_Disconnect_00100 end");
}

/*
 * @tc.number: Hid_Service_SetReport_00100
 * @tc.name: SetReport
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_SetReport_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_SetReport_00100 start");
    auto hidHostService = HidHostService::GetService();

    uint8_t report;
    EXPECT_EQ(hidHostService->HidHostSetReport("abc", 0, 0, &report), HID_HOST_SUCCESS);
    EXPECT_EQ(hidHostService->HidHostSetReport("abc", 0, 1, &report), HID_HOST_SUCCESS);

    uint8_t *nullReport = nullptr;
    EXPECT_EQ(hidHostService->HidHostSetReport("abc", 0, 1, nullReport), HID_HOST_SUCCESS);

    HILOGI("Hid_Service_SetReport_00100 end");
}

/*
 * @tc.number: Hid_Service_SetConnectStrategy_00100
 * @tc.name: SetConnectStrategy
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_SetConnectStrategy_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_SetConnectStrategy_00100 start");
    BluetoothRawAddress device("00");
    auto hidHostService = HidHostService::GetService();

    EXPECT_EQ(hidHostService->SetConnectStrategy(device, 1), BT_ERR_INTERNAL_ERROR);

    HILOGI("Hid_Service_SetConnectStrategy_00100 end");
}

/*
 * @tc.number: Hid_Service_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_GetDevicesByStates_00100 start");
    auto hidHostService = HidHostService::GetService();
    std::string address("00");
    HidHostStateMachine stateMachine(address);

    HidHostConnectingState connectingState(HidHostStateMachine::CONNECTING, stateMachine);
    HidHostConnectedState connectedState(HidHostStateMachine::CONNECTED, stateMachine);
    HidHostDisconnectingState disconnectingState(HidHostStateMachine::DISCONNECTING, stateMachine);
    HidHostDisconnectedState disconnectedState(HidHostStateMachine::DISCONNECTED, stateMachine);

    hidHostService->stateMachines_.EnsureInsert(address, std::make_shared<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal(address)->Init();
    hidHostService->stateMachines_.ReadVal(address)->current_ = &disconnectedState;

    std::vector<int> states =
        {HID_HOST_STATE_DISCONNECTED, HID_HOST_STATE_CONNECTING,
        HID_HOST_STATE_DISCONNECTING, HID_HOST_STATE_CONNECTED};

    EXPECT_EQ(hidHostService->GetDevicesByStates(states).size(), 1);

    std::vector<int> emptyStates;
    EXPECT_EQ(hidHostService->GetDevicesByStates(emptyStates).size(), 0);

    HILOGI("Hid_Service_SetConnectStrategy_00100 end");
}

/*
 * @tc.number: Hid_Service_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_GetDeviceState_00100 start");
    auto hidHostService = HidHostService::GetService();
    BluetoothRawAddress device("00");
    std::string address(device.GetAddress());
    HidHostStateMachine stateMachine(address);

    EXPECT_EQ(hidHostService->GetDeviceState(device), hidHostService->stateMap_.at(HID_HOST_STATE_DISCONNECTED));

    hidHostService->stateMachines_.EnsureInsert(device.GetAddress(), nullptr);
    EXPECT_EQ(hidHostService->GetDeviceState(device), hidHostService->stateMap_.at(HID_HOST_STATE_DISCONNECTED));

    HidHostDisconnectingState disconnectingState(HidHostStateMachine::DISCONNECTING, stateMachine);
    HidHostDisconnectedState disconnectedState(HidHostStateMachine::DISCONNECTED, stateMachine);
    hidHostService->stateMachines_.EnsureInsert(
        device.GetAddress(), std::make_shared<HidHostStateMachine>(device.GetAddress()));
    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &disconnectedState;
    EXPECT_EQ(hidHostService->GetDeviceState(device), hidHostService->stateMap_.at(HID_HOST_STATE_DISCONNECTED));

    hidHostService->stateMachines_.ReadVal(device.GetAddress())->current_ = &disconnectingState;
    EXPECT_EQ(hidHostService->GetDeviceState(device), hidHostService->stateMap_.at(HID_HOST_STATE_DISCONNECTING));

    HILOGI("Hid_Service_GetDeviceState_00100 end");
}

/*
 * @tc.number: Hid_Service_GetConnectDevices_00100
 * @tc.name: GetConnectDevices
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_GetConnectDevices_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_GetConnectDevices_00100 start");
    auto hidHostService = HidHostService::GetService();
    std::string address("00");
    HidHostStateMachine stateMachine(address);

    HidHostConnectingState connectingState(HidHostStateMachine::CONNECTING, stateMachine);
    HidHostConnectedState connectedState(HidHostStateMachine::CONNECTED, stateMachine);

    hidHostService->stateMachines_.EnsureInsert("1", nullptr);
    EXPECT_EQ(hidHostService->GetConnectDevices().size(), 0);

    hidHostService->stateMachines_.EnsureInsert("2", std::make_shared<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal("2")->current_ = &connectingState;
    EXPECT_EQ(hidHostService->GetConnectDevices().size(), 0);

    hidHostService->stateMachines_.EnsureInsert("3", std::make_unique<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal("3")->current_ = &connectedState;
    EXPECT_EQ(hidHostService->GetConnectDevices().size(), 1);

    HILOGI("Hid_Service_GetConnectDevices_00100 end");
}

/*
 * @tc.number: Hid_Service_GetConnectState_00100
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_GetConnectState_00100 start");
    auto hidHostService = HidHostService::GetService();
    std::string address("00");
    HidHostStateMachine stateMachine(address);

    HidHostConnectingState connectingState(HidHostStateMachine::CONNECTING, stateMachine);
    HidHostConnectedState connectedState(HidHostStateMachine::CONNECTED, stateMachine);
    HidHostDisconnectingState disconnectingState(HidHostStateMachine::DISCONNECTING, stateMachine);
    HidHostDisconnectedState disconnectedState(HidHostStateMachine::DISCONNECTED, stateMachine);

    hidHostService->stateMachines_.EnsureInsert("1", nullptr);

    hidHostService->stateMachines_.EnsureInsert("2", std::make_shared<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal("2")->current_ = &connectingState;

    hidHostService->stateMachines_.EnsureInsert("3", std::make_shared<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal("3")->current_ = &connectedState;

    hidHostService->stateMachines_.EnsureInsert("4", std::make_shared<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal("4")->current_ = &disconnectingState;

    hidHostService->stateMachines_.EnsureInsert("5", std::make_shared<HidHostStateMachine>(address));
    hidHostService->stateMachines_.ReadVal("5")->current_ = &disconnectedState;

    EXPECT_EQ(hidHostService->GetConnectState(), 15);

    HILOGI("Hid_Service_GetConnectState_00100 end");
}

/*
 * @tc.number: Hid_Service_GetEventName_00100
 * @tc.name: GetEventName
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_GetEventName_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_GetEventName_00100 start");
    auto hidHostService = HidHostService::GetService();
    EXPECT_EQ(hidHostService->GetEventName(HID_HOST_SERVICE_STARTUP_EVT), "HID_HOST_SERVICE_STARTUP_EVT");
    EXPECT_EQ(hidHostService->GetEventName(HID_HOST_SERVICE_SHUTDOWN_EVT), "HID_HOST_SERVICE_SHUTDOWN_EVT");
    EXPECT_EQ(hidHostService->GetEventName(HID_HOST_CONNECT_EVT), "HID_HOST_CONNECT_EVT");
    EXPECT_EQ(hidHostService->GetEventName(HID_HOST_CONNECTION_STATE_CHANGED_EVT),
        "HID_HOST_CONNECTION_STATE_CHANGED_EVT");
    EXPECT_EQ(hidHostService->GetEventName(HID_HOST_REMOVE_STATE_MACHINE_EVT), "HID_HOST_REMOVE_STATE_MACHINE_EVT");
    EXPECT_EQ(hidHostService->GetEventName(HID_HOST_INVALID_EVT), "Unknown");

    HILOGI("Hid_Service_GetEventName_00100 end");
}

/*
 * @tc.number: Hid_Service_IsAcceptConnection_00100
 * @tc.name: IsAcceptConnection
 * @tc.desc: reject connection with strategy forbidden
 */
HWTEST_F(HidHostServiceTest, Hid_Service_IsAcceptConnection_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_IsAcceptConnection_00100 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    config->Load();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HID_CONNECTION_POLICY, strategyValue);

    // check IsAcceptConnection
    auto hidHostService = HidHostService::GetService();
    EXPECT_FALSE(hidHostService->IsAcceptConnection(rawAddr, HID_HOST_STATE_CONNECTED));
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hid_Service_IsAcceptConnection_00100 end");
}

/*
 * @tc.number: Hid_Service_IsAcceptConnection_00101
 * @tc.name: IsAcceptConnection
 * @tc.desc: accept connection with strategy allowed
 */
HWTEST_F(HidHostServiceTest, Hid_Service_IsAcceptConnection_00101, TestSize.Level1)
{
    HILOGI("Hid_Service_IsAcceptConnection_00101 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HID_CONNECTION_POLICY, strategyValue);

    // check IsAcceptConnection
    auto hidHostService = HidHostService::GetService();
    EXPECT_TRUE(hidHostService->IsAcceptConnection(rawAddr, HID_HOST_STATE_CONNECTED));
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hid_Service_IsAcceptConnection_00101 end");
}

/*
 * @tc.number: Hid_Service_IsAcceptConnection_00102
 * @tc.name: IsAcceptConnection
 * @tc.desc: accept connection with HID_HOST_STATE_DISCONNECTING
 */
HWTEST_F(HidHostServiceTest, Hid_Service_IsAcceptConnection_00102, TestSize.Level1)
{
    HILOGI("Hid_Service_IsAcceptConnection_00101 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HID_CONNECTION_POLICY, strategyValue);

    // check IsAcceptConnection
    auto hidHostService = HidHostService::GetService();
    EXPECT_TRUE(hidHostService->IsAcceptConnection(rawAddr, HID_HOST_STATE_DISCONNECTING));
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hid_Service_IsAcceptConnection_00101 end");
}

/*
 * @tc.number: Hid_Service_ConnectionStateCallbackInner_00100
 * @tc.name: ConnectionStateCallbackInner
 * @tc.desc: accept connection
 */
HWTEST_F(HidHostServiceTest, Hid_Service_ConnectionStateCallbackInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_ConnectionStateCallbackInner_00100 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    EXPECT_TRUE(config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HID_CONNECTION_POLICY, strategyValue));

    // check IsAcceptConnection
    auto hidHostService = HidHostService::GetService();
    hidHostService->ConnectionStateCallbackInner(rawAddr, HID_HOST_STATE_DISCONNECTING);
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hid_Service_ConnectionStateCallbackInner_00100 end");
}

/*
 * @tc.number: Hid_Service_ConnectionStateCallbackInner_00101
 * @tc.name: ConnectionStateCallbackInner
 * @tc.desc: reject connection
 */
HWTEST_F(HidHostServiceTest, Hid_Service_ConnectionStateCallbackInner_00101, TestSize.Level1)
{
    HILOGI("Hid_Service_ConnectionStateCallbackInner_00101 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    config->Load();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    EXPECT_TRUE(config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HID_CONNECTION_POLICY, strategyValue));

    // check IsAcceptConnection
    auto hidHostService = HidHostService::GetService();
    hidHostService->ConnectionStateCallbackInner(rawAddr, HID_HOST_STATE_CONNECTED);
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hid_Service_ConnectionStateCallbackInner_00101 end");
}

/*
 * @tc.number: Hid_Service_ProcessRemoveStateMachine_00100
 * @tc.name: ProcessRemoveStateMachine
 * @tc.desc: ProcessRemoveStateMachine
 */
HWTEST_F(HidHostServiceTest, Hid_Service_ProcessRemoveStateMachine_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_ProcessRemoveStateMachine_00100 start");
    std::string rawAddr("00:00:00:00:00:00");

    auto hidHostService = HidHostService::GetService();
    hidHostService->ProcessRemoveStateMachine(rawAddr);
    std::shared_ptr<HidHostStateMachine> stateMachine = nullptr;
    EXPECT_TRUE(hidHostService->stateMachines_.Find(rawAddr, stateMachine));
    HILOGI("Hid_Service_ProcessRemoveStateMachine_00100 end");
}

/*
 * @tc.number: Hid_Service_RemoveStateMachine_00101
 * @tc.name: RemoveStateMachine
 * @tc.desc: RemoveStateMachine
 */
HWTEST_F(HidHostServiceTest, Hid_Service_RemoveStateMachine_00101, TestSize.Level1)
{
    HILOGI("Hid_Service_RemoveStateMachine_00101 start");
    std::string device("00:00:00:00:00:00");

    auto hidHostService = HidHostService::GetService();
    hidHostService->RemoveStateMachine(device);
    std::shared_ptr<HidHostStateMachine> stateMachine = nullptr;
    EXPECT_FALSE(hidHostService->stateMachines_.Find(device, stateMachine));
    HILOGI("Hid_Service_RemoveStateMachine_00101 end");
}

/*
 * @tc.number: Hid_Service_GetMaxConnectNum_00101
 * @tc.name: GetMaxConnectNum
 * @tc.desc: GetMaxConnectNum
 */
HWTEST_F(HidHostServiceTest, Hid_Service_GetMaxConnectNum_00101, TestSize.Level1)
{
    HILOGI("Hid_Service_GetMaxConnectNum_00101 start");
    auto hidHostService = HidHostService::GetService();
    EXPECT_EQ(hidHostService->GetMaxConnectNum(), 8);
    HILOGI("Hid_Service_GetMaxConnectNum_00101 end");
}

/*
 * @tc.number: Hid_Service_CleanUpAllStateMachine_00100
 * @tc.name: CleanUpAllStateMachine
 * @tc.desc: CleanUpAllStateMachine
 */
HWTEST_F(HidHostServiceTest, Hid_Service_CleanUpAllStateMachine_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_CleanUpAllStateMachine_00100 start");
    auto hidHostService = HidHostService::GetService();
    hidHostService->CleanUpAllStateMachine();
    EXPECT_EQ(hidHostService->stateMachines_.Size(), 0);
    HILOGI("Hid_Service_CleanUpAllStateMachine_00100 end");
}

/*
 * @tc.number: Hid_Service_ConnectionStateCallback_00100
 * @tc.name: ConnectionStateCallback
 * @tc.desc: ConnectionStateCallback
 */
HWTEST_F(HidHostServiceTest, Hid_Service_ConnectionStateCallback_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_ConnectionStateCallback_00100 start");
    STACK::RawAddress addr({0x11, 0x22, 0x33, 0x44, 0x55, 0x66});
    STACK::RawAddress* addrs = &addr;
    HidHostService::ConnectionStateCallback(addrs, BTHH_CONN_STATE_CONNECTING);
    HidHostService::ConnectionStateCallback(addrs, BTHH_CONN_STATE_CONNECTED);
    HidHostService::ConnectionStateCallback(addrs, BTHH_CONN_STATE_DISCONNECTED);
    auto hidHostService = HidHostService::GetService();
    EXPECT_EQ(hidHostService->stateMachines_.Size(), 0);
    HILOGI("Hid_Service_ConnectionStateCallback_00100 end");
}

/*
 * @tc.number: Hid_Service_HidHostGetReport_00100
 * @tc.name: HidHostGetReport
 * @tc.desc: HidHostGetReport
 */
HWTEST_F(HidHostServiceTest, Hid_Service_HidHostGetReport_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_HidHostGetReport_00100 start");
    auto hidHostService = HidHostService::GetService();
    EXPECT_EQ(hidHostService->HidHostGetReport("", 0, 0, 0), 0);
    HILOGI("Hid_Service_HidHostGetReport_00100 end");
}

/*
 * @tc.number: Hid_Service_HidHostSendData_00100
 * @tc.name: HidHostSendData
 * @tc.desc: HidHostSendData
 */
HWTEST_F(HidHostServiceTest, Hid_Service_HidHostSendData_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_HidHostSendData_00100 start");
    auto hidHostService = HidHostService::GetService();
    EXPECT_EQ(hidHostService->HidHostSendData("", 0, 0, 0), 0);
    HILOGI("Hid_Service_HidHostSendData_00100 end");
}

/*
 * @tc.number: Hid_Service_HidHostVCUnplug_00100
 * @tc.name: HidHostVCUnplug
 * @tc.desc: HidHostVCUnplug
 */
HWTEST_F(HidHostServiceTest, Hid_Service_HidHostVCUnplug_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_HidHostVCUnplug_00100 start");
    auto hidHostService = HidHostService::GetService();
    EXPECT_EQ(hidHostService->HidHostVCUnplug("", 0, 0, 0), 0);
    HILOGI("Hid_Service_HidHostVCUnplug_00100 end");
}

/*
 * @tc.number: Hid_Service_StartUp_00100
 * @tc.name: StartUp
 * @tc.desc: StartUp
 */
HWTEST_F(HidHostServiceTest, Hid_Service_StartUp_00100, TestSize.Level1)
{
    HILOGI("Hid_Service_StartUp_00100 start");
    auto hidHostService = HidHostService::GetService();
    hidHostService->StartUp();
    AdapterManager::GetInstance()->SetBluetoothRestrictedFlag(true);
    hidHostService->StartUp();
    EXPECT_TRUE(hidHostService != nullptr);
    HILOGI("Hid_Service_StartUp_00100 end");
}

/*
 * @tc.number: Hid_Service_Connect_00101
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_Connect_00101, TestSize.Level1)
{
    HILOGI("Hid_Service_Connect_00101 start");
    bt_interface_t btInterface = {
        .get_remote_services = GetRemoteServicesMock,
    };
    AdapterManager::GetInstance()->setBluetoothInterface(&btInterface);
    auto hidHostService = HidHostService::GetService();
    RawAddress device = RawAddress("11:11:22:33:44:55");

    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    if (remoteDevice != nullptr) {
        remoteDevice->SetDeviceType(DEVICE_TYPE_LE);
    }

    EXPECT_EQ(hidHostService->Connect(device), Bluetooth::BT_ERR_INTERNAL_ERROR);

    HILOGI("Hid_Service_Connect_00101 end");
}

/*
 * @tc.number: Hid_Service_Connect_00102
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidHostServiceTest, Hid_Service_Connect_00102, TestSize.Level1)
{
    HILOGI("Hid_Service_Connect_00102 start");
    bt_interface_t btInterface = {
        .get_remote_services = GetRemoteServicesMock,
    };
    AdapterManager::GetInstance()->setBluetoothInterface(&btInterface);
    auto hidHostService = HidHostService::GetService();
    RawAddress device = RawAddress("11:11:22:33:44:55");

    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    if (remoteDevice != nullptr) {
        remoteDevice->SetDeviceType(DEVICE_TYPE_DUAL_MONO);
    }
    BluetoothConnectionManager::GetInstance()->AddAclLinks(device.GetAddress(), LINK_TYPE_LE, "");
    EXPECT_EQ(hidHostService->Connect(device), Bluetooth::BT_ERR_INTERNAL_ERROR);

    HILOGI("Hid_Service_Connect_00102 end");
}
}  // namespace bluetooth
}  // namespace OHOS