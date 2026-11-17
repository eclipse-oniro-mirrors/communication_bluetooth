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
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

std::string address("00:11:22:33:44:55");
HfpHfStateMachine stateMachine(address);

static bt_status_t g_status = BT_STATUS_FAIL;
static void SetStatus(bt_status_t status) { g_status = status; }

static bt_status_t InitMock(bthf_client_callbacks_t* callbacks){ return g_status; }

const void* GetProfileInterfaceMock(const char* profileId)
{
    HILOGI("enter");
    bthf_client_interface_t *interface = (bthf_client_interface_t *)malloc(sizeof(bthf_client_interface_t));
    if (interface != nullptr) {
        interface->init = InitMock;
    }
    return (void*)interface;
}

class HfpHfServiceObserverTest : public HfpHfServiceObserver {
public:
    HfpHfServiceObserverTest() = default;
    virtual ~HfpHfServiceObserverTest() = default;
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

const int TIME = 2;
static BluetoothHost *host_;
static HfpHfService *hfpHfService_ {nullptr};
static HfpHfServiceObserverTest g_hfpHfServiceObserver;

class HfpHfServiceTest : public testing::Test {
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
        HILOGI("TearDownTestSuite start");
        host_ = nullptr;
        HILOGI("TearDownTestSuite end");
    }
    void SetUp()
    {
        // new HfpHfService
        auto manager = ProfileServiceManager::GetInstance();
        std::string profileName = "HfpHfService";
        manager->SetProfileService(ADAPTER_BREDR, profileName);
        hfpHfService_ = static_cast<HfpHfService *>(manager->GetProfileService(profileName));

        // Disable hfp thread
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_HF, ThreadUtil::DISABLED);

        sleep(1);
    }
    void TearDown()
    {
        sleep(1);
        // delete HfpHfService
        delete hfpHfService_;
        hfpHfService_ = nullptr;
    }
};

/*
 * @tc.number: Hfp_Hf_Service_Enable_00100
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_Enable_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_Enable_00100 start");

    hfpHfService_->Enable();

    HILOGI("Hfp_Hf_Service_Enable_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_Disable_00100
 * @tc.name: Disable
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_Disable_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_Disable_00100 start");

    hfpHfService_->Disable();

    HILOGI("Hfp_Hf_Service_Disable_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_StartUp_00100
 * @tc.name: StartUp
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_StartUp_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_StartUp_00100 start");

    hfpHfService_->StartUp();

    hfpHfService_->isStarted_ = true;
    hfpHfService_->StartUp();

    bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
    btInterface->get_profile_interface = GetProfileInterfaceMock;
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface);
    hfpHfService_->StartUp();

    SetStatus(BT_STATUS_SUCCESS);
    hfpHfService_->StartUp();

    HILOGI("Hfp_Hf_Service_StartUp_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_ShutDown_00100
 * @tc.name: ShutDown
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_ShutDown_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_ShutDown_00100 start");

    hfpHfService_->ShutDown();

    hfpHfService_->isStarted_ = true;
    hfpHfService_->ShutDown();

    HfpHfMessage connected(HFP_HF_CONNECTED_EVT);
    connected.dev_ = address;
    hfpHfService_->ProcessEvent(connected);
    hfpHfService_->ShutDown();
    HILOGI("Hfp_Hf_Service_ShutDown_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_ShutDownDone_00100
 * @tc.name: ShutDownDone
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_ShutDownDone_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_ShutDownDone_00100 start");

    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);
    HfpHfConnecting connectingState(CONNECTING, stateMachine);

    hfpHfService_->stateMachines_["1"] = nullptr;

    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["2"]->current_ = &disconnectedState;

    hfpHfService_->stateMachines_["3"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["3"]->current_ = &connectingState;

    hfpHfService_->ShutDownDone(false);
    EXPECT_FALSE(hfpHfService_->stateMachines_.empty());

    hfpHfService_->stateMachines_.erase("3");
    hfpHfService_->ShutDownDone(false);
    EXPECT_TRUE(hfpHfService_->stateMachines_.empty());

    hfpHfService_->ShutDownDone(true);
    EXPECT_TRUE(hfpHfService_->stateMachines_.empty());

    HILOGI("Hfp_Hf_Service_ShutDownDone_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_GetConnectedDeviceNum_00100
 * @tc.name: GetConnectedDeviceNum
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_GetConnectedDeviceNum_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_GetConnectedDeviceNum_00100 start");

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);

    hfpHfService_->stateMachines_["1"] = nullptr;
    EXPECT_EQ(hfpHfService_->GetConnectedDeviceNum(), 0);

    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>("2");
    hfpHfService_->stateMachines_["2"]->current_ = &connectingState;

    hfpHfService_->stateMachines_["3"] = std::make_unique<HfpHfStateMachine>("3");
    hfpHfService_->stateMachines_["3"]->current_ = &connectedState;

    hfpHfService_->stateMachines_["4"] = std::make_unique<HfpHfStateMachine>("4");
    hfpHfService_->stateMachines_["4"]->current_ = &disconnectedState;

    EXPECT_EQ(hfpHfService_->GetConnectedDeviceNum(), 2);

    HILOGI("Hfp_Hf_Service_GetConnectedDeviceNum_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_Connect_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_Connect_00100 start");

    BluetoothRawAddress device("00");

    hfpHfService_->stateMachines_[device.GetAddress()] = nullptr;
    hfpHfService_->Connect(device);

    hfpHfService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    hfpHfService_->stateMachines_[device.GetAddress()]->Init();

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);

    hfpHfService_->stateMachines_[device.GetAddress()]->current_ = &connectedState;
    hfpHfService_->Connect(device);

    hfpHfService_->stateMachines_[device.GetAddress()]->current_ = &connectingState;
    hfpHfService_->Connect(device);

    hfpHfService_->stateMachines_[device.GetAddress()]->current_ = &disconnectedState;
    hfpHfService_->Connect(device);

    hfpHfService_->maxConnectedNum_ = -1;
    hfpHfService_->Connect(device);

    HILOGI("Hfp_Hf_Service_Connect_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_Disconnect_00100 start");
    BluetoothRawAddress device("00");
    EXPECT_EQ(hfpHfService_->Disconnect(device), BT_ERR_DEVICE_IS_NOT_CONNECTED);

    hfpHfService_->stateMachines_[device.GetAddress()] = nullptr;
    EXPECT_EQ(hfpHfService_->Disconnect(device), BT_ERR_DEVICE_IS_NOT_CONNECTED);

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);

    hfpHfService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    hfpHfService_->stateMachines_[device.GetAddress()]->Init();
    hfpHfService_->stateMachines_[device.GetAddress()]->current_ = &connectingState;
    EXPECT_EQ(hfpHfService_->Disconnect(device), BT_NO_ERROR);

    hfpHfService_->stateMachines_[device.GetAddress()]->current_ = &connectedState;
    EXPECT_EQ(hfpHfService_->Disconnect(device), BT_NO_ERROR);

    hfpHfService_->stateMachines_[device.GetAddress()]->current_ = &disconnectedState;
    EXPECT_EQ(hfpHfService_->Disconnect(device), BT_ERR_DEVICE_IS_NOT_CONNECTED);

    HILOGI("Hfp_Hf_Service_Disconnect_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_GetConnectDevices_00100
 * @tc.name: GetConnectDevices
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_GetConnectDevices_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_GetConnectDevices_00100 start");

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    HfpHfConnected connectedState(CONNECTED, stateMachine);

    hfpHfService_->stateMachines_["1"] = nullptr;
    EXPECT_EQ(hfpHfService_->GetConnectDevices().size(), 0);

    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["2"]->current_ = &connectingState;
    EXPECT_EQ(hfpHfService_->GetConnectDevices().size(), 0);

    hfpHfService_->stateMachines_["3"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["3"]->current_ = &connectedState;
    EXPECT_EQ(hfpHfService_->GetConnectDevices().size(), 1);

    HILOGI("Hfp_Hf_Service_GetConnectDevices_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_GetConnectState_00100
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_GetConnectState_00100 start");

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);

    hfpHfService_->stateMachines_["1"] = nullptr;

    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["2"]->current_ = &connectingState;

    hfpHfService_->stateMachines_["3"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["3"]->current_ = &connectedState;

    hfpHfService_->stateMachines_["4"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["4"]->current_ = &disconnectedState;

    EXPECT_EQ(hfpHfService_->GetConnectState(), 13);

    HILOGI("Hfp_Hf_Service_GetConnectState_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_GetMaxConnectNum_00100
 * @tc.name: GetMaxConnectNum
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_GetMaxConnectNum_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_GetMaxConnectNum_00100 start");

    hfpHfService_->GetMaxConnectNum();

    HILOGI("Hfp_Hf_Service_GetMaxConnectNum_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_GetDeviceState_00100 start");
    BluetoothRawAddress device(address);

    EXPECT_EQ(hfpHfService_->GetDeviceState(device), hfpHfService_->stateMap_.at(HFP_HF_STATE_DISCONNECTED));

    hfpHfService_->stateMachines_[device.GetAddress()] = nullptr;
    EXPECT_EQ(hfpHfService_->GetDeviceState(device), hfpHfService_->stateMap_.at(HFP_HF_STATE_DISCONNECTED));

    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);
    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpHfStateMachine>(device.GetAddress());
    hfpHfService_->stateMachines_[device.GetAddress()]->current_ = &disconnectedState;
    EXPECT_EQ(hfpHfService_->GetDeviceState(device), hfpHfService_->stateMap_.at(HFP_HF_STATE_DISCONNECTED));

    hfpHfService_->stateMachines_[device.GetAddress()]->current_ = &connectedState;
    EXPECT_EQ(hfpHfService_->GetDeviceState(device), hfpHfService_->stateMap_.at(HFP_HF_STATE_CONNECTED));

    HILOGI("Hfp_Hf_Service_GetDeviceState_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_IsConnected_00100
 * @tc.name: IsConnected
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_IsConnected_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_IsConnected_00100 start");
    EXPECT_FALSE(hfpHfService_->IsConnected("00"));

    hfpHfService_->stateMachines_["1"] = nullptr;
    EXPECT_FALSE(hfpHfService_->IsConnected("1"));

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["2"]->current_ = &connectingState;
    EXPECT_FALSE(hfpHfService_->IsConnected("2"));

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_["3"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["3"]->current_ = &connectedState;
    EXPECT_TRUE(hfpHfService_->IsConnected("3"));

    HILOGI("Hfp_Hf_Service_IsConnected_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_ProcessEvent_00100
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_ProcessEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_ProcessEvent_00100 start");

    HfpHfMessage connect(HFP_HF_CONNECT_EVT);
    HfpHfMessage disconnect(HFP_HF_DISCONNECT_EVT);
    HfpHfMessage connected(HFP_HF_CONNECTED_EVT);
    HfpHfMessage disconnected(HFP_HF_DISCONNECTED_EVT);
    HfpHfMessage slcEstablished(HFP_HF_SLC_ESTABLISHED_EVT);

    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);
    hfpHfService_->stateMachines_["1"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["1"]->current_ = &disconnectedState;
    hfpHfService_->stateMachines_["1"]->isRemoving_ = true;
    connect.dev_ = "1";
    hfpHfService_->ProcessEvent(connect);
    slcEstablished.dev_ = "1";
    hfpHfService_->ProcessEvent(slcEstablished);

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["2"]->current_ = &connectedState;
    disconnect.dev_ = "2";
    hfpHfService_->ProcessEvent(disconnect);
    disconnected.dev_ = "2";
    hfpHfService_->ProcessEvent(disconnected);

    hfpHfService_->stateMachines_["1"]->current_ = &disconnectedState;
    connected.dev_ = "1";
    hfpHfService_->ProcessEvent(connected);
    hfpHfService_->ProcessEvent(slcEstablished);
    HILOGI("Hfp_Hf_Service_ProcessEvent_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_ProcessEvent_00200
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_ProcessEvent_00200, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_ProcessEvent_00200 start");

    HfpHfMessage batteryLevelEvent(HFP_HF_BATTERY_LEVEL_CHANGED_EVT, 5);
    HfpHfMessage keyPressedEvent(HFP_HF_SEND_KEY_PRESSED);

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>("2");
    hfpHfService_->stateMachines_["2"]->current_ = &connectedState;

    hfpHfService_->ProcessEvent(batteryLevelEvent);
    hfpHfService_->ProcessEvent(keyPressedEvent);

    HILOGI("Hfp_Hf_Service_ProcessEvent_00200 end");
}

/*
 * @tc.number: Hfp_Hf_Service_SetHfVolume_00100
 * @tc.name: SetHfVolume
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_SetHfVolume_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_SetHfVolume_00100 start");

    hfpHfService_->SetHfVolume(5, HFP_HF_VOLUME_TYPE_MIC);
    hfpHfService_->SetHfVolume(2, HFP_HF_VOLUME_TYPE_SPK);

    HILOGI("Hfp_Hf_Service_SetHfVolume_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_OpenVoiceRecognition_00100
 * @tc.name: OpenVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_OpenVoiceRecognition_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_OpenVoiceRecognition_00100 start");

    BluetoothRawAddress device(address);
    EXPECT_FALSE(hfpHfService_->OpenVoiceRecognition(device));

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;
    EXPECT_TRUE(hfpHfService_->OpenVoiceRecognition(device));

    HILOGI("Hfp_Hf_Service_OpenVoiceRecognition_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_CloseVoiceRecognition_00100
 * @tc.name: CloseVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_CloseVoiceRecognition_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_CloseVoiceRecognition_00100 start");
    BluetoothRawAddress device(address);
    EXPECT_FALSE(hfpHfService_->CloseVoiceRecognition(device));

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;

    EXPECT_TRUE(hfpHfService_->CloseVoiceRecognition(device));

    HILOGI("Hfp_Hf_Service_CloseVoiceRecognition_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_BatteryLevelChanged_00100
 * @tc.name: BatteryLevelChanged
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_BatteryLevelChanged_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_BatteryLevelChanged_00100 start");

    hfpHfService_->BatteryLevelChanged(10);

    HILOGI("Hfp_Hf_Service_BatteryLevelChanged_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_EnhancedDriverSafety_00100
 * @tc.name: EnhancedDriverSafety
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_EnhancedDriverSafety_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_EnhancedDriverSafety_00100 start");

    hfpHfService_->EnhancedDriverSafety(0);

    HILOGI("Hfp_Hf_Service_EnhancedDriverSafety_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_GetDevicesByStates_00100 start");

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);

    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &disconnectedState;

    std::vector<int> states =
        {HFP_HF_STATE_DISCONNECTED, HFP_HF_STATE_CONNECTING, HFP_HF_STATE_CONNECTED};

    EXPECT_EQ(hfpHfService_->GetDevicesByStates(states).size(), 1);

    std::vector<int> emptyStates;
    EXPECT_EQ(hfpHfService_->GetDevicesByStates(emptyStates).size(), 0);

    hfpHfService_->stateMachines_["1"] = std::make_unique<HfpHfStateMachine>("1");
    hfpHfService_->stateMachines_["1"]->current_ = &connectingState;

    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>("2");
    hfpHfService_->stateMachines_["2"]->current_ = &connectedState;
    EXPECT_EQ(hfpHfService_->GetDevicesByStates(states).size(), 3);

    HILOGI("Hfp_Hf_Service_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_ConnectSco_00100
 * @tc.name: ConnectSco
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_ConnectSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_ConnectSco_00100 start");
    RawAddress address("1");
    EXPECT_FALSE(hfpHfService_->ConnectSco(address));

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    HfpHfConnected connectedState(CONNECTED, stateMachine);

    hfpHfService_->stateMachines_["1"] = std::make_unique<HfpHfStateMachine>("1");

    hfpHfService_->stateMachines_["1"]->current_ = &connectingState;
    EXPECT_FALSE(hfpHfService_->ConnectSco(address));

    hfpHfService_->stateMachines_["1"]->current_ = &connectedState;
    EXPECT_TRUE(hfpHfService_->ConnectSco(address));

    HILOGI("Hfp_Hf_Service_ConnectSco_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_DisconnectSco_00100
 * @tc.name: DisconnectSco
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_DisconnectSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_DisconnectSco_00100 start");
    RawAddress address1("1");
    EXPECT_FALSE(hfpHfService_->DisconnectSco(address1));

    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);
    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    HfpHfAudioConnected audioConnected(AUDIO_CONNECTED,
        stateMachine, connectedState);

    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>("2");
    hfpHfService_->stateMachines_["2"]->current_ = &disconnectedState;
    RawAddress address2("2");
    EXPECT_FALSE(hfpHfService_->DisconnectSco(address2));

    hfpHfService_->stateMachines_["3"] = std::make_unique<HfpHfStateMachine>("3");
    hfpHfService_->stateMachines_["3"]->current_ = &connectedState;
    RawAddress address3("3");
    EXPECT_FALSE(hfpHfService_->DisconnectSco(address3));

    hfpHfService_->stateMachines_["4"] = std::make_unique<HfpHfStateMachine>("4");
    hfpHfService_->stateMachines_["4"]->current_ = &audioConnected;
    RawAddress address4("4");
    EXPECT_TRUE(hfpHfService_->DisconnectSco(address4));

    HILOGI("Hfp_Hf_Service_DisconnectSco_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_GetScoState_00100
 * @tc.name: GetScoState
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_GetScoState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_GetScoState_00100 start");

    BluetoothRawAddress device(address);
    EXPECT_EQ(hfpHfService_->GetScoState(device), HFP_HF_AUDIO_STATE_DISCONNECTED);

    hfpHfService_->stateMachines_[address] = nullptr;
    EXPECT_EQ(hfpHfService_->GetScoState(device), HFP_HF_AUDIO_STATE_DISCONNECTED);

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;
    EXPECT_EQ(hfpHfService_->GetScoState(device), HFP_HF_AUDIO_STATE_DISCONNECTED);

    HfpHfAudioConnected audioConnected(AUDIO_CONNECTED,
        stateMachine, connectedState);
    hfpHfService_->stateMachines_[address]->current_ = &audioConnected;
    EXPECT_EQ(hfpHfService_->GetScoState(device), HFP_HF_AUDIO_STATE_CONNECTED);

    HILOGI("Hfp_Hf_Service_GetScoState_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_SendDTMFTone_00100
 * @tc.name: SendDTMFTone
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_SendDTMFTone_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_SendDTMFTone_00100 start");

    BluetoothRawAddress device(address);
    EXPECT_FALSE(hfpHfService_->SendDTMFTone(device, 0));

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;
    EXPECT_TRUE(hfpHfService_->SendDTMFTone(device, 0));

    HILOGI("Hfp_Hf_Service_SendDTMFTone_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_GetCurrentCallList_00100
 * @tc.name: GetCurrentCallList
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_GetCurrentCallList_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_GetCurrentCallList_00100 start");

    BluetoothRawAddress device(address);
    std::vector<HandsFreeUnitCalls> calls = hfpHfService_->GetCurrentCallList(device);
    EXPECT_EQ(calls.size(), 0);

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->Init();
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;
    calls = hfpHfService_->GetCurrentCallList(device);
    EXPECT_EQ(calls.size(), 0);

    HILOGI("Hfp_Hf_Service_GetCurrentCallList_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_HandleCall_00100
 * @tc.name: HandleCall
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_HandleCall_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_HandleCall_00100 start");

    BluetoothRawAddress device(address);
    EXPECT_FALSE(hfpHfService_->AcceptIncomingCall(device, 0));
    EXPECT_FALSE(hfpHfService_->HoldActiveCall(device));
    EXPECT_FALSE(hfpHfService_->RejectIncomingCall(device));
    EXPECT_FALSE(hfpHfService_->HandleIncomingCall(device, 0));
    EXPECT_FALSE(hfpHfService_->HandleMultiCall(device, 0, 0));

    HandsFreeUnitCalls call;
    EXPECT_FALSE(hfpHfService_->FinishActiveCall(device, call));
    hfpHfService_->QueryCall(device, 10);

    hfpHfService_->stateMachines_[address] = nullptr;
    EXPECT_FALSE(hfpHfService_->AcceptIncomingCall(device, 0));
    EXPECT_FALSE(hfpHfService_->HoldActiveCall(device));
    EXPECT_FALSE(hfpHfService_->RejectIncomingCall(device));
    EXPECT_FALSE(hfpHfService_->HandleIncomingCall(device, 0));
    EXPECT_FALSE(hfpHfService_->HandleMultiCall(device, 0, 0));
    EXPECT_FALSE(hfpHfService_->FinishActiveCall(device, call));
    hfpHfService_->QueryCall(device, 10);

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &connectingState;
    EXPECT_FALSE(hfpHfService_->AcceptIncomingCall(device, 0));
    EXPECT_FALSE(hfpHfService_->HoldActiveCall(device));
    EXPECT_FALSE(hfpHfService_->RejectIncomingCall(device));
    EXPECT_FALSE(hfpHfService_->HandleIncomingCall(device, 0));
    EXPECT_FALSE(hfpHfService_->HandleMultiCall(device, 0, 0));
    EXPECT_FALSE(hfpHfService_->FinishActiveCall(device, call));
    hfpHfService_->QueryCall(device, 10);

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;
    EXPECT_TRUE(hfpHfService_->AcceptIncomingCall(device, 0));
    EXPECT_TRUE(hfpHfService_->HoldActiveCall(device));
    EXPECT_TRUE(hfpHfService_->RejectIncomingCall(device));
    EXPECT_TRUE(hfpHfService_->HandleIncomingCall(device, 0));
    EXPECT_TRUE(hfpHfService_->HandleMultiCall(device, 0, 0));
    EXPECT_TRUE(hfpHfService_->FinishActiveCall(device, call));
    hfpHfService_->QueryCall(device, 10);

    HILOGI("Hfp_Hf_Service_HandleCall_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_DialCall_00100
 * @tc.name: DialCall
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_DialCall_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_DialCall_00100 start");

    BluetoothRawAddress device(address);
    std::string number = "000";
    std::optional<HandsFreeUnitCalls> call = hfpHfService_->StartDial(device, number);
    EXPECT_TRUE(call == std::nullopt);
    EXPECT_FALSE(hfpHfService_->DialLastNumber(device));
    EXPECT_FALSE(hfpHfService_->DialMemory(device, 0));

    hfpHfService_->stateMachines_[address] = nullptr;
    call = hfpHfService_->StartDial(device, number);
    EXPECT_TRUE(call == std::nullopt);
    EXPECT_FALSE(hfpHfService_->DialLastNumber(device));
    EXPECT_FALSE(hfpHfService_->DialMemory(device, 0));

    HfpHfConnecting connectingState(CONNECTING, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &connectingState;
    call = hfpHfService_->StartDial(device, number);
    EXPECT_TRUE(call == std::nullopt);
    EXPECT_FALSE(hfpHfService_->DialLastNumber(device));
    EXPECT_FALSE(hfpHfService_->DialMemory(device, 0));

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;
    call = hfpHfService_->StartDial(device, number);
    EXPECT_TRUE(call != std::nullopt);
    EXPECT_TRUE(hfpHfService_->DialLastNumber(device));
    EXPECT_TRUE(hfpHfService_->DialMemory(device, 0));

    HILOGI("Hfp_Hf_Service_DialCall_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_SendKeyPressed_00100
 * @tc.name: SendKeyPressed
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_SendKeyPressed_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_SendKeyPressed_00100 start");

    BluetoothRawAddress device(address);
    EXPECT_FALSE(hfpHfService_->SendKeyPressed(device));

    hfpHfService_->stateMachines_[address] = nullptr;
    EXPECT_FALSE(hfpHfService_->SendKeyPressed(device));

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;
    EXPECT_TRUE(hfpHfService_->SendKeyPressed(device));

    HfpHfAudioConnected audioConnected(AUDIO_CONNECTED,
        stateMachine, connectedState);
    hfpHfService_->stateMachines_[address]->current_ = &audioConnected;
    EXPECT_TRUE(hfpHfService_->SendKeyPressed(device));

    HILOGI("Hfp_Hf_Service_SendKeyPressed_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_SendVoiceTag_00100
 * @tc.name: SendVoiceTag
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_SendVoiceTag_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_SendVoiceTag_00100 start");

    BluetoothRawAddress device(address);
    EXPECT_FALSE(hfpHfService_->SendVoiceTag(device, 0));

    hfpHfService_->stateMachines_[address] = nullptr;
    EXPECT_FALSE(hfpHfService_->SendVoiceTag(device, 0));

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->current_ = &connectedState;
    EXPECT_TRUE(hfpHfService_->SendVoiceTag(device, 0));

    HILOGI("Hfp_Hf_Service_SendVoiceTag_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_SetAudioParameters_00100
 * @tc.name: SetAudioParameters
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_SetAudioParameters_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_SetAudioParameters_00100 start");

    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);
    hfpHfService_->stateMachines_["1"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["1"]->current_ = &disconnectedState;

    HfpHfConnected audioConnectedState(AUDIO_CONNECTED, stateMachine);
    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["2"]->current_ = &audioConnectedState;

    hfpHfService_->SetAudioParameters();

    HILOGI("Hfp_Hf_Service_SetAudioParameters_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_ProcessRemoveStateMachine_00100
 * @tc.name: ProcessRemoveStateMachine
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_ProcessRemoveStateMachine_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_ProcessRemoveStateMachine_00100 start");

    HfpHfDisconnected disconnectedState(DISCONNECTED, stateMachine);
    hfpHfService_->stateMachines_["1"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["1"]->current_ = &disconnectedState;
    hfpHfService_->stateMachines_["1"]->isRemoving_ = true;

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    hfpHfService_->stateMachines_["2"] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_["2"]->current_ = &connectedState;

    hfpHfService_->ProcessRemoveStateMachine(address);

    HILOGI("Hfp_Hf_Service_ProcessRemoveStateMachine_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_RegisterObserver_00100
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_RegisterObserver_00100 start");

    hfpHfService_->RegisterObserver(g_hfpHfServiceObserver);

    HILOGI("Hfp_Hf_Service_RegisterObserver_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_DeregisterObserver_00100
 * @tc.name: DeregisterObserver
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_DeregisterObserver_00100 start");

    hfpHfService_->DeregisterObserver(g_hfpHfServiceObserver);
    
    HILOGI("Hfp_Hf_Service_DeregisterObserver_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_SetConnectStrategy_00100
 * @tc.name: SetConnectStrategy
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_SetConnectStrategy_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_SetConnectStrategy_00100 start");

    BluetoothRawAddress device("00");
    EXPECT_EQ(hfpHfService_->SetConnectStrategy(device, 0), Bluetooth::BT_ERR_INTERNAL_ERROR);

    HILOGI("Hfp_Hf_Service_SetConnectStrategy_00100 end");
}

/*
 * @tc.number: Hfp_Hf_Service_NotifyObserver_00100
 * @tc.name: NotifyObserver
 * @tc.desc:
 */
HWTEST_F(HfpHfServiceTest, Hfp_Hf_Service_NotifyObserver_00100, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Service_NotifyObserver_00100 start");

    hfpHfService_->RegisterObserver(g_hfpHfServiceObserver);

    BluetoothRawAddress device(address);
    hfpHfService_->NotifyStateChanged(device, HFP_HF_STATE_CONNECTED);
    hfpHfService_->NotifyStateChanged(device, HFP_HF_STATE_DISCONNECTED);
    hfpHfService_->NotifyScoStateChanged(device, 0);

    HandsFreeUnitCalls call(
        address, HFP_HF_OUTGOING_CALL_ID, HFP_CALL_STATE_DIALING, "10086", false, true, false);
    hfpHfService_->NotifyCallChanged(device, call);
    call.SetState(HFP_CALL_STATE_FINISHED);
    hfpHfService_->NotifyCallChanged(device, call);
    hfpHfService_->NotifyBatteryLevelChanged(device, 0);
    hfpHfService_->NotifySignalStrengthChanged(device, 0);
    hfpHfService_->NotifyRegistrationStatusChanged(device, 0);
    hfpHfService_->NotifyRoamingStatusChanged(device, 0);
    hfpHfService_->NotifyOperatorSelectionChanged(device, "");
    hfpHfService_->NotifySubscriberNumberChanged(device, "");
    hfpHfService_->NotifyVoiceRecognitionStatusChanged(device, 0);
    hfpHfService_->NotifyInBandRingTone(device, 0);

    sleep(TIME);

    HILOGI("Hfp_Hf_Service_NotifyObserver_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS