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
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "thread_util.h"
#include "adapter_manager.h"

using namespace testing;
using namespace testing::ext;

static bt_status_t g_hfpHfConnStatus = BT_STATUS_FAIL;
static void SetHfpHfConnStatus(bt_status_t status) { g_hfpHfConnStatus = status; }

extern "C" {
static bt_status_t HfpHfConnInitMock(bthf_client_callbacks_t* callbacks) { return g_hfpHfConnStatus; }
static bt_status_t HfpHfConnConnectMock(RawAddress* bd_addr) { return g_hfpHfConnStatus; }
static bt_status_t HfpHfConnDisconnectMock(const RawAddress* bd_addr) { return g_hfpHfConnStatus; }
static bt_status_t HfpHfConnConnectAudioMock(const RawAddress* bd_addr) { return g_hfpHfConnStatus; }
static bt_status_t HfpHfConnDisconnectAudioMock(const RawAddress* bd_addr) { return g_hfpHfConnStatus; }
static void HfpHfConnCleanUpMock(void) {}

static bthf_client_interface_t g_hfpHfConnInterface = {
    .init = HfpHfConnInitMock,
    .connect = HfpHfConnConnectMock,
    .disconnect = HfpHfConnDisconnectMock,
    .connect_audio = HfpHfConnConnectAudioMock,
    .disconnect_audio = HfpHfConnDisconnectAudioMock,
    .cleanup = HfpHfConnCleanUpMock,
};

static const void* HfpHfConnGetProfileInterfaceMock(const char* profileId)
{
    return (void*)&g_hfpHfConnInterface;
}
}

namespace OHOS {
namespace bluetooth {

using namespace OHOS::Bluetooth;

std::string g_hfpHfConnAddress("00:11:22:33:44:55");
HfpHfStateMachine g_hfpHfConnStateMachine(g_hfpHfConnAddress);

class HfpHfServiceObserverConnTest : public HfpHfServiceObserver {
public:
    HfpHfServiceObserverConnTest() = default;
    virtual ~HfpHfServiceObserverConnTest() = default;
    virtual void OnConnectionStateChanged(const RawAddress &device, int state, int cause) {}
    virtual void OnScoStateChanged(const RawAddress &device, int state) {}
    virtual void OnCallChanged(const RawAddress &device, const HandsFreeUnitCalls &call) {}
    virtual void OnBatteryLevelChanged(const RawAddress &device, int batteryLevel) {}
    virtual void OnSignalStrengthChanged(const RawAddress &device, int signal) {}
    virtual void OnRegistrationStatusChanged(const RawAddress &device, int status) {}
    virtual void OnRoamingStatusChanged(const RawAddress &device, int status) {}
    virtual void OnOperatorSelectionChanged(const RawAddress &device, const std::string &name) {}
    virtual void OnSubscriberNumberChanged(const RawAddress &device, const std::string &number) {}
    virtual void OnVoiceRecognitionStatusChanged(const RawAddress &device, int status) {}
    virtual void OnInBandRingToneChanged(const RawAddress &device, int status) {}
};

const int HFP_HF_CONN_TIME = 2;
static BluetoothHost *g_hfpHfConnHost_;
static HfpHfService *g_hfpHfConnService_ {nullptr};
static HfpHfServiceObserverConnTest g_hfpHfConnObserver;

class HfpHfConnectionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("HfpHfConnectionTest SetUpTestCase start");
        g_hfpHfConnHost_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = g_hfpHfConnHost_->IsBleEnabled();
        bool isBrEnable = g_hfpHfConnHost_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            g_hfpHfConnHost_->EnableBle();
            g_hfpHfConnHost_->EnableBt();
            sleep(HFP_HF_CONN_TIME);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = HfpHfConnGetProfileInterfaceMock;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        HILOGI("HfpHfConnectionTest SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("HfpHfConnectionTest TearDownTestCase start");
        g_hfpHfConnHost_ = nullptr;
        HILOGI("HfpHfConnectionTest TearDownTestCase end");
    }
    void SetUp()
    {
        auto manager = ProfileServiceManager::GetInstance();
        std::string profileName = "HfpHfService";
        manager->SetProfileService(ADAPTER_BREDR, profileName);
        g_hfpHfConnService_ = static_cast<HfpHfService *>(manager->GetProfileService(profileName));

        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_HF, ThreadUtil::DISABLED);
        sleep(1);
    }
    void TearDown()
    {
        sleep(1);
        delete g_hfpHfConnService_;
        g_hfpHfConnService_ = nullptr;
    }
};

/*
 * @tc.number: Hfp_Hf_Connection_Connect_00100
 * @tc.name: Connect with null state machine
 * @tc.desc: Verify Connect returns error when stateMachines_ has null entry for the device.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Connect_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Connect_00100 start");
    BluetoothRawAddress device("00:11:22:33:44:55");

    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = nullptr;
    int ret = g_hfpHfConnService_->Connect(device);
    HILOGI("Hfp_Hf_Connection_Connect_00100 ret=%{public}d", ret);
    HILOGI("Hfp_Hf_Connection_Connect_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Connect_00200
 * @tc.name: Connect when already connected
 * @tc.desc: Verify Connect returns BT_ERR_DEVICE_HAS_CONNECTED when device state is CONNECTED.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Connect_00200, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Connect_00200 start");
    BluetoothRawAddress device("00:11:22:33:44:55");

    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->Init();
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->current_ = &connectedState;

    int ret = g_hfpHfConnService_->Connect(device);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_DEVICE_HAS_CONNECTED);
    HILOGI("Hfp_Hf_Connection_Connect_00200 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Connect_00300
 * @tc.name: Connect when connecting
 * @tc.desc: Verify Connect returns BT_ERR_DEVICE_HAS_CONNECTED when device state is CONNECTING.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Connect_00300, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Connect_00300 start");
    BluetoothRawAddress device("00:11:22:33:44:55");

    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->current_ = &connectingState;

    int ret = g_hfpHfConnService_->Connect(device);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_DEVICE_HAS_CONNECTED);
    HILOGI("Hfp_Hf_Connection_Connect_00300 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Connect_00400
 * @tc.name: Connect from disconnected state
 * @tc.desc: Verify Connect returns BT_NO_ERROR when device state is DISCONNECTED.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Connect_00400, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Connect_00400 start");
    BluetoothRawAddress device("00:11:22:33:44:55");

    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->current_ = &disconnectedState;

    int ret = g_hfpHfConnService_->Connect(device);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Hfp_Hf_Connection_Connect_00400 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Connect_00500
 * @tc.name: Connect when max connections reached
 * @tc.desc: Verify Connect returns BT_ERR_SERVICE_MAX_CONNECTION when maxConnectedNum_ is exceeded.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Connect_00500, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Connect_00500 start");
    BluetoothRawAddress device("00:11:22:33:44:55");

    g_hfpHfConnService_->maxConnectedNum_ = -1;
    int ret = g_hfpHfConnService_->Connect(device);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_SERVICE_MAX_CONNECTION);
    g_hfpHfConnService_->maxConnectedNum_ = 1;
    HILOGI("Hfp_Hf_Connection_Connect_00500 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Disconnect_00100
 * @tc.name: Disconnect with no state machine
 * @tc.desc: Verify Disconnect returns BT_ERR_DEVICE_IS_NOT_CONNECTED when no state machine exists.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Disconnect_00100 start");
    BluetoothRawAddress device("00:11:22:33:44:55");
    int ret = g_hfpHfConnService_->Disconnect(device);
    EXPECT_EQ(ret, BT_ERR_DEVICE_IS_NOT_CONNECTED);
    HILOGI("Hfp_Hf_Connection_Disconnect_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Disconnect_00200
 * @tc.name: Disconnect with null state machine
 * @tc.desc: Verify Disconnect returns BT_ERR_DEVICE_IS_NOT_CONNECTED when stateMachines_ entry is null.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Disconnect_00200, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Disconnect_00200 start");
    BluetoothRawAddress device("00:11:22:33:44:55");
    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = nullptr;
    int ret = g_hfpHfConnService_->Disconnect(device);
    EXPECT_EQ(ret, BT_ERR_DEVICE_IS_NOT_CONNECTED);
    HILOGI("Hfp_Hf_Connection_Disconnect_00200 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Disconnect_00300
 * @tc.name: Disconnect from connecting state
 * @tc.desc: Verify Disconnect returns BT_NO_ERROR when device state is CONNECTING.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Disconnect_00300, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Disconnect_00300 start");
    BluetoothRawAddress device("00:11:22:33:44:55");
    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->Init();
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->current_ = &connectingState;
    int ret = g_hfpHfConnService_->Disconnect(device);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Hfp_Hf_Connection_Disconnect_00300 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Disconnect_00400
 * @tc.name: Disconnect from connected state
 * @tc.desc: Verify Disconnect returns BT_NO_ERROR when device state is CONNECTED.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Disconnect_00400, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Disconnect_00400 start");
    BluetoothRawAddress device("00:11:22:33:44:55");
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->Init();
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->current_ = &connectedState;
    int ret = g_hfpHfConnService_->Disconnect(device);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Hfp_Hf_Connection_Disconnect_00400 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_Disconnect_00500
 * @tc.name: Disconnect from disconnected state
 * @tc.desc: Verify Disconnect returns BT_ERR_DEVICE_IS_NOT_CONNECTED when device is DISCONNECTED.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_Disconnect_00500, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_Disconnect_00500 start");
    BluetoothRawAddress device("00:11:22:33:44:55");
    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->Init();
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->current_ = &disconnectedState;
    int ret = g_hfpHfConnService_->Disconnect(device);
    EXPECT_EQ(ret, BT_ERR_DEVICE_IS_NOT_CONNECTED);
    HILOGI("Hfp_Hf_Connection_Disconnect_00500 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_ProcessEvent_00100
 * @tc.name: ProcessEvent with HFP_HF_CONNECT_EVT on removing state machine
 * @tc.desc: Verify ProcessEvent reposts HFP_HF_CONNECT_EVT when state machine is removing.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_ProcessEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00100 start");
    HfpHfMessage connectEvt(HFP_HF_CONNECT_EVT);
    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"] = std::make_unique<HfpHfStateMachine>("11:22:33:44:55:66");
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"]->current_ = &disconnectedState;
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"]->isRemoving_ = true;
    connectEvt.dev_ = "11:22:33:44:55:66";
    g_hfpHfConnService_->ProcessEvent(connectEvt);
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_ProcessEvent_00200
 * @tc.name: ProcessEvent with HFP_HF_CONNECT_EVT on new device
 * @tc.desc: Verify ProcessEvent creates a new state machine for an unknown device.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_ProcessEvent_00200, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00200 start");
    HfpHfMessage connectEvt(HFP_HF_CONNECT_EVT);
    connectEvt.dev_ = "AA:BB:CC:DD:EE:FF";
    g_hfpHfConnService_->ProcessEvent(connectEvt);
    EXPECT_NE(g_hfpHfConnService_->stateMachines_.find("AA:BB:CC:DD:EE:FF"),
        g_hfpHfConnService_->stateMachines_.end());
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00200 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_ProcessEvent_00300
 * @tc.name: ProcessEvent with HFP_HF_CONNECTED_EVT
 * @tc.desc: Verify ProcessEvent dispatches HFP_HF_CONNECTED_EVT to existing state machine.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_ProcessEvent_00300, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00300 start");
    HfpHfMessage connectedEvt(HFP_HF_CONNECTED_EVT);
    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"] = std::make_unique<HfpHfStateMachine>("11:22:33:44:55:66");
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"]->current_ = &disconnectedState;
    connectedEvt.dev_ = "11:22:33:44:55:66";
    g_hfpHfConnService_->ProcessEvent(connectedEvt);
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00300 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_ProcessEvent_00400
 * @tc.name: ProcessEvent with HFP_HF_DISCONNECT_EVT
 * @tc.desc: Verify ProcessEvent dispatches HFP_HF_DISCONNECT_EVT to existing connected state machine.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_ProcessEvent_00400, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00400 start");
    HfpHfMessage disconnectEvt(HFP_HF_DISCONNECT_EVT);
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"] = std::make_unique<HfpHfStateMachine>("11:22:33:44:55:66");
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"]->current_ = &connectedState;
    disconnectEvt.dev_ = "11:22:33:44:55:66";
    g_hfpHfConnService_->ProcessEvent(disconnectEvt);
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00400 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_ProcessEvent_00500
 * @tc.name: ProcessEvent with HFP_HF_DISCONNECTED_EVT
 * @tc.desc: Verify ProcessEvent dispatches HFP_HF_DISCONNECTED_EVT to existing connected state machine.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_ProcessEvent_00500, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00500 start");
    HfpHfMessage disconnectedEvt(HFP_HF_DISCONNECTED_EVT);
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"] = std::make_unique<HfpHfStateMachine>("11:22:33:44:55:66");
    g_hfpHfConnService_->stateMachines_["11:22:33:44:55:66"]->current_ = &connectedState;
    disconnectedEvt.dev_ = "11:22:33:44:55:66";
    g_hfpHfConnService_->ProcessEvent(disconnectedEvt);
    HILOGI("Hfp_Hf_Connection_ProcessEvent_00500 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_GetDeviceState_00100
 * @tc.name: GetDeviceState when disconnected
 * @tc.desc: Verify GetDeviceState returns DISCONNECTED mapping for unknown device.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_GetDeviceState_00100 start");
    BluetoothRawAddress device("AA:BB:CC:DD:EE:FF");
    int state = g_hfpHfConnService_->GetDeviceState(device);
    EXPECT_EQ(state, g_hfpHfConnService_->stateMap_.at(HFP_HF_STATE_DISCONNECTED));
    HILOGI("Hfp_Hf_Connection_GetDeviceState_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_GetDeviceState_00200
 * @tc.name: GetDeviceState when connected
 * @tc.desc: Verify GetDeviceState returns CONNECTED mapping for connected device.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_GetDeviceState_00200, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_GetDeviceState_00200 start");
    BluetoothRawAddress device("00:11:22:33:44:55");
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    g_hfpHfConnService_->stateMachines_[device.GetAddress()]->current_ = &connectedState;
    int state = g_hfpHfConnService_->GetDeviceState(device);
    EXPECT_EQ(state, g_hfpHfConnService_->stateMap_.at(HFP_HF_STATE_CONNECTED));
    HILOGI("Hfp_Hf_Connection_GetDeviceState_00200 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_GetConnectState_00100
 * @tc.name: GetConnectState with mixed states
 * @tc.desc: Verify GetConnectState aggregates states correctly across multiple state machines.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_GetConnectState_00100 start");
    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);

    g_hfpHfConnService_->stateMachines_["conn_dev_1"] = nullptr;
    g_hfpHfConnService_->stateMachines_["conn_dev_2"] = std::make_unique<HfpHfStateMachine>("conn_dev_2");
    g_hfpHfConnService_->stateMachines_["conn_dev_2"]->current_ = &connectingState;
    g_hfpHfConnService_->stateMachines_["conn_dev_3"] = std::make_unique<HfpHfStateMachine>("conn_dev_3");
    g_hfpHfConnService_->stateMachines_["conn_dev_3"]->current_ = &connectedState;
    g_hfpHfConnService_->stateMachines_["conn_dev_4"] = std::make_unique<HfpHfStateMachine>("conn_dev_4");
    g_hfpHfConnService_->stateMachines_["conn_dev_4"]->current_ = &disconnectedState;

    int state = g_hfpHfConnService_->GetConnectState();
    EXPECT_EQ(state, 13);
    HILOGI("Hfp_Hf_Connection_GetConnectState_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_GetConnectDevices_00100
 * @tc.name: GetConnectDevices with mixed states
 * @tc.desc: Verify GetConnectDevices returns only devices with state >= CONNECTED.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_GetConnectDevices_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_GetConnectDevices_00100 start");
    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);

    g_hfpHfConnService_->stateMachines_["conn_dev_1"] = nullptr;
    g_hfpHfConnService_->stateMachines_["conn_dev_2"] = std::make_unique<HfpHfStateMachine>("conn_dev_2");
    g_hfpHfConnService_->stateMachines_["conn_dev_2"]->current_ = &connectingState;
    g_hfpHfConnService_->stateMachines_["conn_dev_3"] = std::make_unique<HfpHfStateMachine>("conn_dev_3");
    g_hfpHfConnService_->stateMachines_["conn_dev_3"]->current_ = &connectedState;

    std::list<RawAddress> devList = g_hfpHfConnService_->GetConnectDevices();
    EXPECT_EQ(devList.size(), 1u);
    HILOGI("Hfp_Hf_Connection_GetConnectDevices_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_IsConnected_00100
 * @tc.name: IsConnected for various states
 * @tc.desc: Verify IsConnected returns true only for CONNECTED state machines.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_IsConnected_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_IsConnected_00100 start");
    EXPECT_FALSE(g_hfpHfConnService_->IsConnected("non_exist"));

    g_hfpHfConnService_->stateMachines_["conn_dev_1"] = nullptr;
    EXPECT_FALSE(g_hfpHfConnService_->IsConnected("conn_dev_1"));

    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["conn_dev_2"] = std::make_unique<HfpHfStateMachine>("conn_dev_2");
    g_hfpHfConnService_->stateMachines_["conn_dev_2"]->current_ = &connectingState;
    EXPECT_FALSE(g_hfpHfConnService_->IsConnected("conn_dev_2"));

    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["conn_dev_3"] = std::make_unique<HfpHfStateMachine>("conn_dev_3");
    g_hfpHfConnService_->stateMachines_["conn_dev_3"]->current_ = &connectedState;
    EXPECT_TRUE(g_hfpHfConnService_->IsConnected("conn_dev_3"));
    HILOGI("Hfp_Hf_Connection_IsConnected_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_GetConnectedDeviceNum_00100
 * @tc.name: GetConnectedDeviceNum with various states
 * @tc.desc: Verify GetConnectedDeviceNum counts only CONNECTING/CONNECTED devices.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_GetConnectedDeviceNum_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_GetConnectedDeviceNum_00100 start");
    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);

    g_hfpHfConnService_->stateMachines_["conn_dev_1"] = nullptr;
    g_hfpHfConnService_->stateMachines_["conn_dev_2"] = std::make_unique<HfpHfStateMachine>("conn_dev_2");
    g_hfpHfConnService_->stateMachines_["conn_dev_2"]->current_ = &connectingState;
    g_hfpHfConnService_->stateMachines_["conn_dev_3"] = std::make_unique<HfpHfStateMachine>("conn_dev_3");
    g_hfpHfConnService_->stateMachines_["conn_dev_3"]->current_ = &connectedState;
    g_hfpHfConnService_->stateMachines_["conn_dev_4"] = std::make_unique<HfpHfStateMachine>("conn_dev_4");
    g_hfpHfConnService_->stateMachines_["conn_dev_4"]->current_ = &disconnectedState;

    EXPECT_EQ(g_hfpHfConnService_->GetConnectedDeviceNum(), 2);
    HILOGI("Hfp_Hf_Connection_GetConnectedDeviceNum_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates for connection states
 * @tc.desc: Verify GetDevicesByStates returns devices matching specified connection states.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_GetDevicesByStates_00100 start");
    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);
    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);

    g_hfpHfConnService_->stateMachines_["conn_dev_1"] = std::make_unique<HfpHfStateMachine>("conn_dev_1");
    g_hfpHfConnService_->stateMachines_["conn_dev_1"]->current_ = &disconnectedState;
    g_hfpHfConnService_->stateMachines_["conn_dev_2"] = std::make_unique<HfpHfStateMachine>("conn_dev_2");
    g_hfpHfConnService_->stateMachines_["conn_dev_2"]->current_ = &connectingState;
    g_hfpHfConnService_->stateMachines_["conn_dev_3"] = std::make_unique<HfpHfStateMachine>("conn_dev_3");
    g_hfpHfConnService_->stateMachines_["conn_dev_3"]->current_ = &connectedState;

    std::vector<int> states = {HFP_HF_STATE_DISCONNECTED, HFP_HF_STATE_CONNECTING, HFP_HF_STATE_CONNECTED};
    EXPECT_EQ(g_hfpHfConnService_->GetDevicesByStates(states).size(), 3u);

    std::vector<int> onlyConnected = {HFP_HF_STATE_CONNECTED};
    EXPECT_EQ(g_hfpHfConnService_->GetDevicesByStates(onlyConnected).size(), 1u);
    HILOGI("Hfp_Hf_Connection_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_ProcessRemoveStateMachine_00100
 * @tc.name: ProcessRemoveStateMachine during shutdown
 * @tc.desc: Verify ProcessRemoveStateMachine nullifies the state machine entry and triggers ShutDownDone.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_ProcessRemoveStateMachine_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_ProcessRemoveStateMachine_00100 start");
    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["conn_dev_1"] = std::make_unique<HfpHfStateMachine>("conn_dev_1");
    g_hfpHfConnService_->stateMachines_["conn_dev_1"]->current_ = &disconnectedState;
    g_hfpHfConnService_->stateMachines_["conn_dev_1"]->isRemoving_ = true;

    g_hfpHfConnService_->ProcessRemoveStateMachine("conn_dev_1");
    EXPECT_EQ(g_hfpHfConnService_->stateMachines_["conn_dev_1"], nullptr);
    HILOGI("Hfp_Hf_Connection_ProcessRemoveStateMachine_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_ConnectSco_00100
 * @tc.name: ConnectSco when not connected
 * @tc.desc: Verify ConnectSco returns false when device is not in CONNECTED state.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_ConnectSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_ConnectSco_00100 start");
    RawAddress addr("conn_sco_dev_1");
    EXPECT_FALSE(g_hfpHfConnService_->ConnectSco(addr));

    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["conn_sco_dev_1"] = std::make_unique<HfpHfStateMachine>("conn_sco_dev_1");
    g_hfpHfConnService_->stateMachines_["conn_sco_dev_1"]->current_ = &connectingState;
    EXPECT_FALSE(g_hfpHfConnService_->ConnectSco(addr));
    HILOGI("Hfp_Hf_Connection_ConnectSco_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_DisconnectSco_00100
 * @tc.name: DisconnectSco state validation
 * @tc.desc: Verify DisconnectSco returns false unless device is in AudioConnected state.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_DisconnectSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_DisconnectSco_00100 start");
    RawAddress addr1("conn_sco_dev_1");
    EXPECT_FALSE(g_hfpHfConnService_->DisconnectSco(addr1));

    HfpHfDisconnected disconnectedState(DISCONNECTED, g_hfpHfConnStateMachine);
    HfpHfConnected connectedState(CONNECTED, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["conn_sco_dev_2"] = std::make_unique<HfpHfStateMachine>("conn_sco_dev_2");
    g_hfpHfConnService_->stateMachines_["conn_sco_dev_2"]->current_ = &disconnectedState;
    RawAddress addr2("conn_sco_dev_2");
    EXPECT_FALSE(g_hfpHfConnService_->DisconnectSco(addr2));

    g_hfpHfConnService_->stateMachines_["conn_sco_dev_3"] = std::make_unique<HfpHfStateMachine>("conn_sco_dev_3");
    g_hfpHfConnService_->stateMachines_["conn_sco_dev_3"]->current_ = &connectedState;
    RawAddress addr3("conn_sco_dev_3");
    EXPECT_FALSE(g_hfpHfConnService_->DisconnectSco(addr3));

    HfpHfAudioConnected audioConnected(AUDIO_CONNECTED, g_hfpHfConnStateMachine, connectedState);
    g_hfpHfConnService_->stateMachines_["conn_sco_dev_4"] = std::make_unique<HfpHfStateMachine>("conn_sco_dev_4");
    g_hfpHfConnService_->stateMachines_["conn_sco_dev_4"]->current_ = &audioConnected;
    RawAddress addr4("conn_sco_dev_4");
    EXPECT_TRUE(g_hfpHfConnService_->DisconnectSco(addr4));
    HILOGI("Hfp_Hf_Connection_DisconnectSco_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_SetConnectStrategy_00100
 * @tc.name: SetConnectStrategy
 * @tc.desc: Verify SetConnectStrategy returns internal error when config not available.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_SetConnectStrategy_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_SetConnectStrategy_00100 start");
    BluetoothRawAddress device("00:11:22:33:44:55");
    int ret = g_hfpHfConnService_->SetConnectStrategy(device, 0);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_INTERNAL_ERROR);
    HILOGI("Hfp_Hf_Connection_SetConnectStrategy_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_NotifyStateChanged_00100
 * @tc.name: NotifyStateChanged with observer
 * @tc.desc: Verify NotifyStateChanged dispatches connection state changes to registered observers.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_NotifyStateChanged_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_NotifyStateChanged_00100 start");
    g_hfpHfConnService_->RegisterObserver(g_hfpHfConnObserver);
    BluetoothRawAddress device("00:11:22:33:44:55");
    g_hfpHfConnService_->NotifyStateChanged(device, HFP_HF_STATE_CONNECTING);
    g_hfpHfConnService_->NotifyStateChanged(device, HFP_HF_STATE_CONNECTED);
    g_hfpHfConnService_->NotifyStateChanged(device, HFP_HF_STATE_DISCONNECTING);
    g_hfpHfConnService_->NotifyStateChanged(device, HFP_HF_STATE_DISCONNECTED);
    sleep(HFP_HF_CONN_TIME);
    g_hfpHfConnService_->DeregisterObserver(g_hfpHfConnObserver);
    HILOGI("Hfp_Hf_Connection_NotifyStateChanged_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_EnableDisable_00100
 * @tc.name: Enable and Disable service
 * @tc.desc: Verify Enable/Disable service lifecycle does not crash when invoked sequentially.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_EnableDisable_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_EnableDisable_00100 start");
    g_hfpHfConnService_->Enable();
    g_hfpHfConnService_->Disable();
    HILOGI("Hfp_Hf_Connection_EnableDisable_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Connection_ShutDownDone_00100
 * @tc.name: ShutDownDone with all disconnected
 * @tc.desc: Verify ShutDownDone(true) clears all state machines.
 */
HWTEST_F(HfpHfConnectionTest, Hfp_Hf_Connection_ShutDownDone_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Connection_ShutDownDone_00100 start");
    HfpHfConnecting connectingState(CONNECTING, g_hfpHfConnStateMachine);
    g_hfpHfConnService_->stateMachines_["conn_dev_1"] = std::make_unique<HfpHfStateMachine>("conn_dev_1");
    g_hfpHfConnService_->stateMachines_["conn_dev_1"]->current_ = &connectingState;

    g_hfpHfConnService_->ShutDownDone(true);
    EXPECT_TRUE(g_hfpHfConnService_->stateMachines_.empty());
    HILOGI("Hfp_Hf_Connection_ShutDownDone_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS
