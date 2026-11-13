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

#include "hfp_ag_service.h"
#include "bluetooth_audio_manager/bluetooth_audio_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "remote_device_properties.h"
#include "bluetooth_errorcode.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "thread_util.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_hw_interface.h"
#include "ihw_interop.h"
#ifdef BLUETOOTH_HFP_AG_ENABLE
#include "call_manager_client.h"
#endif
#ifdef BLUETOOTH_WATCH_ENABLE
#include "watch_service.h"
#include "watch_device_manager.h"
#endif

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
using namespace bluetooth::headset;

namespace OHOS {
namespace bluetooth {

const int TIME = 2;
std::string address("11:11:11:11:11:11");
HfpAgStateMachine stateMachine(address);

extern "C" {
static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

static bool InteropMatchMock(const uint16_t feature, const STACK::RawAddress &bdAddr)
{
    HILOGE("enter");
    return true;
}

static void HwEnableHisiPcmMock(bool state)
{
    HILOGE("enter");
}

static void HwSetVoiceCombineScenarioEnabledMock(const STACK::RawAddress &addr, const bool isEnabled)
{
    HILOGE("enter");
}

static int HwGetLocalVoiceCombineFeatureStateMock()
{
    HILOGE("enter");
    return HW_VOICE_COMBINE_STATE_ABILITY_ON_NN_SWITCH_ON;
}

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .interopMatch = InteropMatchMock,
    .hwEnableHisiPcm = HwEnableHisiPcmMock,
    .hwSetVoiceCombineScenarioEnabled = HwSetVoiceCombineScenarioEnabledMock,
    .hwGetLocalVoiceCombineFeatureState = HwGetLocalVoiceCombineFeatureStateMock,
};
}

enum {
    A2DP_STATUS_INVALID = 0,
    A2DP_STATUS_ENABLE = 1,
    A2DP_STATUS_DISABLE_NO_SEIZE = 2,
    A2DP_DISABLE_SEIZE = 3,
};

class HfpAgSystemEventProcesserTest : public HfpAgSystemEventProcesser {
public:
    HfpAgSystemEventProcesserTest() : HfpAgSystemEventProcesser(address) {}
    ~HfpAgSystemEventProcesserTest() = default;
};

class HfpAgServiceObserverTest : public HfpAgServiceObserver {
public:
    HfpAgServiceObserverTest() = default;
    virtual ~HfpAgServiceObserverTest() = default;
    virtual void OnConnectionStateChanged(const RawAddress &device, int state, int cause) {}
    virtual void OnScoStateChanged(const RawAddress &device, int state, int reason) {}
    virtual void OnActiveDeviceChanged(const RawAddress &device) {}
    virtual void OnHfEnhancedDriverSafetyChanged(const RawAddress &device, int indValue) {}
};

static BluetoothHost *host_;
static HfpAgService *hfpAgService_ {nullptr};
static HfpAgSystemEventProcesserTest g_eventProcessor;
static HfpAgServiceObserverTest g_hfpAgServiceObserver;

class HfpAgServiceTest : public testing::Test {
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
        utility::CloseAllTimer();
        HILOGI("TearDownTestSuite end");
    }
    void SetUp()
    {
        // new HfpAgService
        auto manager = ProfileServiceManager::GetInstance();
        std::string profileName = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, profileName);
        hfpAgService_ = static_cast<HfpAgService *>(manager->GetProfileService(profileName));

        // Disable hfp thread
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_HFP, ThreadUtil::DISABLED);
    }
    void TearDown()
    {
        sleep(1);
        // delete HfpAgService
        delete hfpAgService_;
        hfpAgService_ = nullptr;

        HfpAgSystemInterface::GetInstance().SetActiveCallNumber(0);
        HfpAgSystemInterface::GetInstance().SetHeldCallNumber(0);
        HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_DISCONNECTED);
    }

    void ResponseClccTimeOut() {}

    void DialOutCallTimeOut() {}

    void VoiceRecognitionTimeOut() {}
};

/*
 * @tc.number: Hfp_Ag_Service_ShutDownDone_00100
 * @tc.name: ShutDownDone
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ShutDownDone_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ShutDownDone_00100 start");

    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_["1"] = nullptr;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &disconnectedState;

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["3"]->current_ = &connectingState;

    hfpAgService_->ShutDownDone(false);
    EXPECT_FALSE(hfpAgService_->stateMachines_.empty());

    hfpAgService_->stateMachines_.erase("3");
    hfpAgService_->ShutDownDone(false);
    EXPECT_TRUE(hfpAgService_->stateMachines_.empty());

    hfpAgService_->ShutDownDone(true);
    EXPECT_TRUE(hfpAgService_->stateMachines_.empty());

    HILOGI("Hfp_Ag_Service_ShutDownDone_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetConnectedDeviceNum_00100
 * @tc.name: GetConnectedDeviceNum
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetConnectedDeviceNum_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetConnectedDeviceNum_00100 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_["1"] = nullptr;
    EXPECT_EQ(hfpAgService_->GetConnectedDeviceNum(), 0);

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>("2");
    hfpAgService_->stateMachines_["2"]->current_ = &connectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTING));

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>("3");
    hfpAgService_->stateMachines_["3"]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));

    hfpAgService_->stateMachines_["4"] = std::make_unique<HfpAgStateMachine>("4");
    hfpAgService_->stateMachines_["4"]->current_ = &disconnectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTING));

    hfpAgService_->stateMachines_["5"] = std::make_unique<HfpAgStateMachine>("5");
    hfpAgService_->stateMachines_["5"]->current_ = &disconnectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTED));

    EXPECT_EQ(hfpAgService_->GetConnectedDeviceNum(), 0);

    HILOGI("Hfp_Ag_Service_GetConnectedDeviceNum_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetProcessingDeviceNum_00100
 * @tc.name: GetProcessingDeviceNum
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetProcessingDeviceNum_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetProcessingDeviceNum_00100 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_["1"] = nullptr;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>("2");
    hfpAgService_->stateMachines_["2"]->current_ = &connectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTING));

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>("3");
    hfpAgService_->stateMachines_["3"]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));

    hfpAgService_->stateMachines_["4"] = std::make_unique<HfpAgStateMachine>("4");
    hfpAgService_->stateMachines_["4"]->current_ = &disconnectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTING));

    hfpAgService_->stateMachines_["5"] = std::make_unique<HfpAgStateMachine>("5");
    hfpAgService_->stateMachines_["5"]->current_ = &disconnectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTED));

    EXPECT_EQ(hfpAgService_->GetProcessingDeviceNum(), 0);

    HILOGI("Hfp_Ag_Service_GetProcessingDeviceNum_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_Connect_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_Connect_00100 start");

    BluetoothRawAddress device("00");

    hfpAgService_->stateMachines_[device.GetAddress()] = nullptr;
    hfpAgService_->Connect(device);

    hfpAgService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpAgStateMachine>(device.GetAddress());
    hfpAgService_->stateMachines_[device.GetAddress()]->Init();

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &connectedState;
    hfpAgService_->Connect(device);

    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &connectingState;
    hfpAgService_->Connect(device);

    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &disconnectedState;
    hfpAgService_->Connect(device);

    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &disconnectingState;
    hfpAgService_->Connect(device);

    hfpAgService_->maxConnectedNum_ = -1;
    hfpAgService_->Connect(device);

    HILOGI("Hfp_Ag_Service_Connect_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_Disconnect_00100 start");
    BluetoothRawAddress device("00");
    EXPECT_EQ(hfpAgService_->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);

    hfpAgService_->stateMachines_[device.GetAddress()] = nullptr;
    EXPECT_EQ(hfpAgService_->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpAgStateMachine>(device.GetAddress());
    hfpAgService_->stateMachines_[device.GetAddress()]->Init();
    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &connectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        device.GetAddress(), static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTING));
    EXPECT_EQ(hfpAgService_->Disconnect(device), HFP_AG_SUCCESS);

    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        device.GetAddress(), static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(hfpAgService_->Disconnect(device), HFP_AG_SUCCESS);

    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &disconnectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        device.GetAddress(), static_cast<int>(HFP_AG_STATE_DISCONNECTING));
    EXPECT_EQ(hfpAgService_->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);

    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &disconnectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        device.GetAddress(), static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_EQ(hfpAgService_->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);

    HILOGI("Hfp_Ag_Service_Disconnect_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetConnectDevices_00100
 * @tc.name: GetConnectDevices
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetConnectDevices_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetConnectDevices_00100 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_["1"] = nullptr;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &connectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTING));

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["3"]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));

    HILOGI("Hfp_Ag_Service_GetConnectDevices_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetConnectState_00100
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetConnectState_00100 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_["1"] = nullptr;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &connectingState;

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["3"]->current_ = &connectedState;

    hfpAgService_->stateMachines_["4"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["4"]->current_ = &disconnectingState;

    hfpAgService_->stateMachines_["5"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["5"]->current_ = &disconnectedState;

    EXPECT_EQ(hfpAgService_->GetConnectState(), 15);

    HILOGI("Hfp_Ag_Service_GetConnectState_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetDeviceState_00100 start");
    BluetoothRawAddress device(address);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        device.GetAddress(), static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_EQ(hfpAgService_->GetDeviceState(device), hfpAgService_->stateMap_.at(HFP_AG_STATE_DISCONNECTED));

    hfpAgService_->stateMachines_[device.GetAddress()] = nullptr;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        device.GetAddress(), static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_EQ(hfpAgService_->GetDeviceState(device), hfpAgService_->stateMap_.at(HFP_AG_STATE_DISCONNECTED));

    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[device.GetAddress()] = std::make_unique<HfpAgStateMachine>(device.GetAddress());
    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &disconnectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_EQ(hfpAgService_->GetDeviceState(device), hfpAgService_->stateMap_.at(HFP_AG_STATE_DISCONNECTED));

    hfpAgService_->stateMachines_[device.GetAddress()]->current_ = &disconnectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTING));
    EXPECT_EQ(hfpAgService_->GetDeviceState(device), hfpAgService_->stateMap_.at(HFP_AG_STATE_DISCONNECTING));

    HILOGI("Hfp_Ag_Service_GetDeviceState_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsConnected_00100
 * @tc.name: IsConnected
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsConnected_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsConnected_00100 start");
    EXPECT_FALSE(hfpAgService_->IsConnected("00"));

    hfpAgService_->stateMachines_["1"] = nullptr;
    EXPECT_FALSE(hfpAgService_->IsConnected("1"));

    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &disconnectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTING));
    EXPECT_FALSE(hfpAgService_->IsConnected("2"));

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["3"]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_TRUE(hfpAgService_->IsConnected(address));

    HILOGI("Hfp_Ag_Service_IsConnected_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAudioIdle_00100
 * @tc.name: IsAudioIdle
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAudioIdle_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAudioIdle_00100 start");

    EXPECT_TRUE(hfpAgService_->IsAudioIdle());
    hfpAgService_->isVrOpened_ = true;
    EXPECT_FALSE(hfpAgService_->IsAudioIdle());

    HILOGI("Hfp_Ag_Service_IsAudioIdle_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessEvent_00100
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessEvent_00100 start");

    HfpAgMessage startup(HFP_AG_SERVICE_STARTUP_EVT);
    hfpAgService_->ProcessEvent(startup);

    hfpAgService_->isStarted_ = true;
    hfpAgService_->ProcessEvent(startup);

    HILOGI("Hfp_Ag_Service_ProcessEvent_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessEvent_00200
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessEvent_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessEvent_00200 start");

    HfpAgMessage shutdown(HFP_AG_SERVICE_SHUTDOWN_EVT);
    hfpAgService_->isStarted_ = false;
    hfpAgService_->ProcessEvent(shutdown);

    hfpAgService_->isStarted_ = true;
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_["1"] = nullptr;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &connectingState;

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["3"]->current_ = &disconnectedState;

    hfpAgService_->ProcessEvent(shutdown);
    EXPECT_TRUE(hfpAgService_->isShuttingDown_);

    hfpAgService_->stateMachines_.erase("2");
    hfpAgService_->ProcessEvent(shutdown);
    EXPECT_TRUE(hfpAgService_->isShuttingDown_);

    HILOGI("Hfp_Ag_Service_ProcessEvent_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessEvent_00200
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessEvent_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessEvent_00300 start");

    HfpAgMessage connect(HFP_AG_CONNECT_EVT);
    HfpAgMessage connectionStateChanged(HFP_AG_CONNECTION_STATE_CHANGED_EVT);

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectedState;
    hfpAgService_->stateMachines_["1"]->isRemoving_ = true;
    connect.dev_ = "1";
    hfpAgService_->ProcessEvent(connect);
    hfpAgService_->ProcessEvent(connectionStateChanged);
    connectionStateChanged.dev_ = "1";
    hfpAgService_->ProcessEvent(connectionStateChanged);

    hfpAgService_->stateMachines_["1"]->isRemoving_ = false;
    hfpAgService_->ProcessEvent(connect);
    hfpAgService_->ProcessEvent(connectionStateChanged);

    hfpAgService_->maxConnectedNum_ = 0;
    hfpAgService_->ProcessEvent(connect);
    hfpAgService_->ProcessEvent(connectionStateChanged);
    RawAddress rawAddr("00:00:00:00:00:00");
    hfpAgService_->ProcessChrAgConnectionState(rawAddr, 3);

    HILOGI("Hfp_Ag_Service_ProcessEvent_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessEvent_00400
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessEvent_00400, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessEvent_00400 start");

    HfpAgMessage removeStateEvent(HFP_AG_REMOVE_STATE_MACHINE_EVT);
    hfpAgService_->isShuttingDown_ = true;

    HfpAgStateMachine stateMachine1("1");
    HfpAgStateMachine stateMachine2("2");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine1, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine2, g_eventProcessor);

    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>("1");
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>("2");
    hfpAgService_->stateMachines_["2"]->current_ = &connectedState;

    hfpAgService_->stateMachines_["3"] = nullptr;

    hfpAgService_->ProcessEvent(removeStateEvent);

    HILOGI("Hfp_Ag_Service_ProcessEvent_00400 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessEvent_00500
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessEvent_00500, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessEvent_00600 start");

    HfpAgMessage clccTimeOut(HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT);

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;
    hfpAgService_->stateMachines_["2"] = nullptr;

    hfpAgService_->ProcessEvent(clccTimeOut);

    g_eventProcessor.ResponseClccTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::ResponseClccTimeOut, this));
    hfpAgService_->ProcessEvent(clccTimeOut);

    HILOGI("Hfp_Ag_Service_ProcessEvent_00600 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessEvent_00700
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessEvent_00700, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessEvent_00700 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;
    hfpAgService_->stateMachines_["2"] = nullptr;

    HfpAgMessage agDialTimeout(HFP_AG_DIAL_TIME_OUT_EVT);
    hfpAgService_->ProcessEvent(agDialTimeout);

    hfpAgService_->dialingOutTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::DialOutCallTimeOut, this));
    hfpAgService_->ProcessEvent(agDialTimeout);
    EXPECT_EQ(hfpAgService_->dialingOutTimeout_, nullptr);

    HILOGI("Hfp_Ag_Service_ProcessEvent_00700 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessEvent_00800
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessEvent_00800, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessEvent_00800 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;
    hfpAgService_->stateMachines_["2"] = nullptr;

    HfpAgMessage agSetVolume(HFP_AG_SET_VOLUME_EVT);
    hfpAgService_->ProcessEvent(agSetVolume);

    HILOGI("Hfp_Ag_Service_ProcessEvent_00800 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessEvent_00900
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessEvent_00900, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessEvent_00900 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;
    hfpAgService_->stateMachines_["2"] = nullptr;

    HfpAgMessage invalidEvt(HFP_AG_INVALID_EVT);
    hfpAgService_->ProcessEvent(invalidEvt);

    invalidEvt.dev_ = "1";
    hfpAgService_->ProcessEvent(invalidEvt);

    invalidEvt.dev_ = "2";
    hfpAgService_->ProcessEvent(invalidEvt);

    HILOGI("Hfp_Ag_Service_ProcessEvent_00900 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CloseVoiceRecognition_00100
 * @tc.name: CloseVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CloseVoiceRecognition_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CloseVoiceRecognition_00100 start");
    BluetoothRawAddress device(address);

    hfpAgService_->isVrOpened_ = false;
    EXPECT_FALSE(hfpAgService_->CloseVoiceRecognition(device));

    hfpAgService_->isVrOpened_ = true;
    EXPECT_FALSE(hfpAgService_->CloseVoiceRecognition(device));

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_FALSE(hfpAgService_->CloseVoiceRecognition(device));

    hfpAgService_->activeAddr_ = address;
    EXPECT_TRUE(hfpAgService_->CloseVoiceRecognition(device));

    HILOGI("Hfp_Ag_Service_CloseVoiceRecognition_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetDevicesByStates_00100 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &disconnectedState;

    std::vector<int> states =
        {HFP_AG_STATE_DISCONNECTED, HFP_AG_STATE_CONNECTING,
        HFP_AG_STATE_DISCONNECTING, HFP_AG_STATE_CONNECTED};

    EXPECT_EQ(hfpAgService_->GetDevicesByStates(states).size(), 1);

    std::vector<int> emptyStates;
    EXPECT_EQ(hfpAgService_->GetDevicesByStates(emptyStates).size(), 0);

    HILOGI("Hfp_Ag_Service_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsScoAcceptableExceptADevice_00100
 * @tc.name: IsScoAcceptableExceptADevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsScoAcceptableExceptADevice_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsScoAcceptableExceptADevice_00100 start");
    EXPECT_FALSE(hfpAgService_->IsScoAcceptableExceptADevice("00"));
    hfpAgService_->isVrOpened_ = true;
    EXPECT_TRUE(hfpAgService_->IsScoAcceptableExceptADevice("00"));
    HILOGI("Hfp_Ag_Service_IsScoAcceptableExceptADevice_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsScoAcceptableExceptADevice_00200
 * @tc.name: IsScoAcceptableExceptADevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsScoAcceptableExceptADevice_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsScoAcceptableExceptADevice_00200 start");
    EXPECT_FALSE(hfpAgService_->IsScoAcceptableExceptADevice("00"));
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_INCOMING);
    EXPECT_TRUE(hfpAgService_->IsScoAcceptableExceptADevice("00"));
    HILOGI("Hfp_Ag_Service_IsScoAcceptableExceptADevice_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsScoAcceptableExceptADevice_00300
 * @tc.name: IsScoAcceptableExceptADevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsScoAcceptableExceptADevice_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsScoAcceptableExceptADevice_00300 start");
    EXPECT_FALSE(hfpAgService_->IsScoAcceptableExceptADevice("00"));
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_ACTIVE);
    EXPECT_TRUE(hfpAgService_->IsScoAcceptableExceptADevice("00"));
    HILOGI("Hfp_Ag_Service_IsScoAcceptableExceptADevice_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ConnectSco_00100
 * @tc.name: ConnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ConnectSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ConnectSco_00100 start");
    EXPECT_FALSE(hfpAgService_->ConnectSco());

    hfpAgService_->activeAddr_ = "1";
    EXPECT_FALSE(hfpAgService_->ConnectSco());

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgAudioDisconnecting audioDisconnecting(HfpAgStateMachine::AUDIO_DISCONNECTING,
        stateMachine, g_eventProcessor, disconnectingState);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &audioDisconnecting;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_DISCONNECTING));
    EXPECT_FALSE(hfpAgService_->ConnectSco());

    hfpAgService_->stateMachines_["1"]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("1", static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_TRUE(hfpAgService_->ConnectSco());

    HILOGI("Hfp_Ag_Service_ConnectSco_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DisconnectSingleSco_00100
 * @tc.name: DisconnectSingleSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DisconnectSingleSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DisconnectSingleSco_00100 start");

    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_TRUE(hfpAgService_->DisconnectSingleSco(address));

    hfpAgService_->stateMachines_[address] = nullptr;
    EXPECT_TRUE(hfpAgService_->DisconnectSingleSco(address));

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    EXPECT_TRUE(hfpAgService_->DisconnectSingleSco(address));

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address]->current_ = &connectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTING));
    EXPECT_TRUE(hfpAgService_->DisconnectSingleSco(address));

    HILOGI("Hfp_Ag_Service_DisconnectSingleSco_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetScoState_00100
 * @tc.name: GetScoState
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetScoState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetScoState_00100 start");

    BluetoothRawAddress device(address);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(hfpAgService_->GetScoState(device), HFP_AG_AUDIO_STATE_DISCONNECTED);

    hfpAgService_->stateMachines_[address] = nullptr;
    EXPECT_EQ(hfpAgService_->GetScoState(device), HFP_AG_AUDIO_STATE_DISCONNECTED);

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(hfpAgService_->GetScoState(device), HFP_AG_AUDIO_STATE_DISCONNECTED);

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgAudioConnecting audioConnecting(HfpAgStateMachine::AUDIO_CONNECTING,
        stateMachine, g_eventProcessor, connectingState);
    hfpAgService_->stateMachines_[address]->current_ = &audioConnecting;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTING));
    EXPECT_EQ(hfpAgService_->GetScoState(device), HFP_AG_AUDIO_STATE_CONNECTING);

    HILOGI("Hfp_Ag_Service_GetScoState_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_PhoneStateChanged_00100
 * @tc.name: PhoneStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_PhoneStateChanged_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_PhoneStateChanged_00100 start");
    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(0);
    phoneState.SetHeldNum(0);
    phoneState.SetCallState(HFP_AG_CALL_STATE_IDLE);
    phoneState.SetNumber("number");
    phoneState.SetCallType(0);
    phoneState.SetName("name");
    hfpAgService_->PhoneStateChanged(phoneState);

#ifdef BLUETOOTH_WATCH_ENABLE
    AddDeviceManager(address);
    hfpAgService_->PhoneStateChanged(phoneState);
#endif

    hfpAgService_->dialingOutTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::DialOutCallTimeOut, this));
    hfpAgService_->PhoneStateChanged(phoneState);

    hfpAgService_->dialingOutTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::DialOutCallTimeOut, this));
    hfpAgService_->PhoneStateChanged(phoneState);

    hfpAgService_->dialingOutTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::DialOutCallTimeOut, this));
    hfpAgService_->PhoneStateChanged(phoneState);

    hfpAgService_->dialingOutTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::DialOutCallTimeOut, this));
    hfpAgService_->PhoneStateChanged(phoneState);

    phoneState.SetActiveNum(1);
    phoneState.SetCallState(HFP_AG_CALL_STATE_IDLE);
    hfpAgService_->PhoneStateChanged(phoneState);
    phoneState.SetActiveNum(0);
    phoneState.SetHeldNum(1);
    hfpAgService_->PhoneStateChanged(phoneState);
    phoneState.SetCallState(HFP_AG_CALL_STATE_WAITING);
    hfpAgService_->PhoneStateChanged(phoneState);

    hfpAgService_->isVrOpened_ = true;
    hfpAgService_->PhoneStateChanged(phoneState);

    HILOGI("Hfp_Ag_Service_PhoneStateChanged_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_PhoneStateChanged_002
 * @tc.name: PhoneStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_PhoneStateChanged_002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_PhoneStateChanged_002 start");
    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(1);
    phoneState.SetHeldNum(0);
    phoneState.SetCallState(HFP_AG_CALL_STATE_INCOMING);
    phoneState.SetNumber("number");
    phoneState.SetCallType(0);
    phoneState.SetName("name");

    hfpAgService_->isVirtualCallStarted_ = true;
    hfpAgService_->PhoneStateChanged(phoneState);

    HILOGI("Hfp_Ag_Service_PhoneStateChanged_002 end");
}

/*
 * @tc.number: Hfp_Ag_Service_PhoneStateChanged_003
 * @tc.name: PhoneStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_PhoneStateChanged_003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_PhoneStateChanged_003 start");
    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(1);
    phoneState.SetHeldNum(0);
    phoneState.SetCallState(HFP_AG_CALL_STATE_INCOMING);
    phoneState.SetNumber("number");
    phoneState.SetCallType(0);
    phoneState.SetName("name");

    hfpAgService_->isVirtualCallStarted_ = true;
    hfpAgService_->PhoneStateChanged(phoneState);

    std::string address = "B4:EC:02:11:22:33";
    RawAddress device(address);
    hfpAgService_->UpdateActiveDevice(address);
    EXPECT_EQ(HfpAgSystemInterface::GetInstance().callState_, HFP_AG_CALL_STATE_INCOMING);

    HILOGI("Hfp_Ag_Service_PhoneStateChanged_003 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ClccResponse_00100
 * @tc.name: ClccResponse
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ClccResponse_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ClccResponse_00100 start");

    hfpAgService_->ClccResponse(0, 0, 0, 0, true, "number", 0);

    HILOGI("Hfp_Ag_Service_ClccResponse_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsVoiceRecognitionAvailable_00100
 * @tc.name: IsVoiceRecognitionAvailable
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsVoiceRecognitionAvailable_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00100 start");
    hfpAgService_->isVirtualCallStarted_ = true;
    EXPECT_FALSE(hfpAgService_->IsVoiceRecognitionAvailable(address));
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsVoiceRecognitionAvailable_00200
 * @tc.name: IsVoiceRecognitionAvailable
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsVoiceRecognitionAvailable_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00200 start");
    hfpAgService_->isVirtualCallStarted_ = false;

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgAudioConnected audioConnected(HfpAgStateMachine::AUDIO_CONNECTED, stateMachine,
        g_eventProcessor, connectedState);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &audioConnected;

    EXPECT_FALSE(hfpAgService_->IsVoiceRecognitionAvailable(address));
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsVoiceRecognitionAvailable_00300
 * @tc.name: IsVoiceRecognitionAvailable
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsVoiceRecognitionAvailable_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00300 start");
    hfpAgService_->isVirtualCallStarted_ = false;

    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &disconnectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTED));

    EXPECT_FALSE(hfpAgService_->IsVoiceRecognitionAvailable(address));
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsVoiceRecognitionAvailable_00400
 * @tc.name: IsVoiceRecognitionAvailable
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsVoiceRecognitionAvailable_00400, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00400 start");
    hfpAgService_->isVirtualCallStarted_ = false;

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_CONNECTED));

    EXPECT_TRUE(hfpAgService_->IsVoiceRecognitionAvailable(address));
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00400 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsVoiceRecognitionAvailable_00500
 * @tc.name: IsVoiceRecognitionAvailable
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsVoiceRecognitionAvailable_00500, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00500 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));

    EXPECT_TRUE(hfpAgService_->IsVoiceRecognitionAvailable(address));
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionAvailable_00500 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendOpenVoiceEvent_00100
 * @tc.name: SendOpenVoiceEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendOpenVoiceEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendOpenVoiceEvent_00100 start");

    hfpAgService_->SendOpenVoiceEvent(address, true);
    hfpAgService_->SendOpenVoiceEvent(address, false);

    HILOGI("Hfp_Ag_Service_SendOpenVoiceEvent_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognition_001
 * @tc.name: OpenVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognition_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognition_00100 start");
    BluetoothRawAddress device(address);
    hfpAgService_->isVrOpened_ = true;
    EXPECT_FALSE(hfpAgService_->OpenVoiceRecognition(device));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognition_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognition_002
 * @tc.name: OpenVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognition_002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognition_002 start");
    BluetoothRawAddress device(address);
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = true;
    EXPECT_FALSE(hfpAgService_->OpenVoiceRecognition(device));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognition_002 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognition_003
 * @tc.name: OpenVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognition_003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognition_003 start");
    BluetoothRawAddress device(address);
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->voiceRecognitionTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::VoiceRecognitionTimeOut, this));
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_TRUE(hfpAgService_->OpenVoiceRecognition(device));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognition_003 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognition_004
 * @tc.name: OpenVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognition_004, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognition_004 start");
    BluetoothRawAddress device(address);
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->voiceRecognitionTimeout_ = nullptr;
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_TRUE(hfpAgService_->OpenVoiceRecognition(device));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognition_004 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SetActiveDeviceToStack_00100
 * @tc.name: SetActiveDeviceToStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SetActiveDeviceToStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SetActiveDeviceToStack_00100 start");

    BluetoothRawAddress device(address);
    EXPECT_FALSE(hfpAgService_->SetActiveDeviceToStack(device));

    HILOGI("Hfp_Ag_Service_SetActiveDeviceToStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SetActiveDevice_00100
 * @tc.name: SetActiveDevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SetActiveDevice_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SetActiveDevice_00100 start");

    BluetoothRawAddress device(HfpAgService::NULL_ADDRESS);
    EXPECT_FALSE(hfpAgService_->SetActiveDevice(device));

    BluetoothRawAddress device1(address);
    EXPECT_FALSE(hfpAgService_->SetActiveDevice(device1));

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;

    EXPECT_FALSE(hfpAgService_->SetActiveDevice(device1));

    hfpAgService_->activeAddr_ = address;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_TRUE(hfpAgService_->SetActiveDevice(device1));

    HILOGI("Hfp_Ag_Service_SetActiveDevice_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ClearActiveDevice_00100
 * @tc.name: ClearActiveDevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ClearActiveDevice_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ClearActiveDevice_00100 start");

    hfpAgService_->isVrOpened_ = true;
    hfpAgService_->ClearActiveDevice();

    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->ClearActiveDevice();

    HILOGI("Hfp_Ag_Service_ClearActiveDevice_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsActiveDevice_00100
 * @tc.name: IsActiveDevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsActiveDevice_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsActiveDevice_00100 start");

    EXPECT_FALSE(hfpAgService_->IsActiveDevice(address));
    hfpAgService_->activeAddr_ = address;
    EXPECT_TRUE(hfpAgService_->IsActiveDevice(address));

    HILOGI("Hfp_Ag_Service_IsActiveDevice_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifySlcStateChanged_00100
 * @tc.name: NotifySlcStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifySlcStateChanged_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifySlcStateChanged_00100 start");

    BluetoothRawAddress device("00");

    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(HFP_AG_STATE_CONNECTED));
    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(HFP_AG_STATE_DISCONNECTED));

    BluetoothRawAddress activeDevice(HfpAgService::NULL_ADDRESS);
    hfpAgService_->SetActiveDevice(activeDevice);
    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(BTConnectState::CONNECTED));

    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(BTConnectState::CONNECTING));

    HILOGI("Hfp_Ag_Service_NotifySlcStateChanged_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifySlcStateChanged_00200
 * @tc.name: NotifySlcStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifySlcStateChanged_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifySlcStateChanged_00200 start");

    BluetoothRawAddress device("00");
    hfpAgService_->activeAddr_ = "00";
    hfpAgService_->isVirtualCallStarted_ = true;
    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_FALSE(hfpAgService_->isVirtualCallStarted_);

    hfpAgService_->activeAddr_ = "11";
    hfpAgService_->isVirtualCallStarted_ = true;
    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_TRUE(hfpAgService_->isVirtualCallStarted_);

    hfpAgService_->activeAddr_ = "00";
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_FALSE(hfpAgService_->isVirtualCallStarted_);

    hfpAgService_->activeAddr_ = "11";
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_FALSE(hfpAgService_->isVirtualCallStarted_);

    HILOGI("Hfp_Ag_Service_NotifySlcStateChanged_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifySlcStateChanged_00300
 * @tc.name: NotifySlcStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifySlcStateChanged_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifySlcStateChanged_00300 start");

    BluetoothRawAddress device("00");
    hfpAgService_->activeAddr_ = "00";
    hfpAgService_->isVirtualCallStarted_ = true;
    EXPECT_TRUE(hfpAgService_->isVirtualCallStarted_);
    hfpAgService_->NotifySlcStateChanged(device, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    hfpAgService_->activeAddr_ = "11";
    EXPECT_FALSE(hfpAgService_->isVirtualCallStarted_);

    HILOGI("Hfp_Ag_Service_NotifySlcStateChanged_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DialOutCallByHf_00100
 * @tc.name: DialOutCallByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DialOutCallByHf_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DialOutCallByHf_00100 start");

    hfpAgService_->dialingOutTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::DialOutCallTimeOut, this));
    EXPECT_FALSE(hfpAgService_->DialOutCallByHf(address));

    hfpAgService_->dialingOutTimeout_ = nullptr;
    EXPECT_FALSE(hfpAgService_->DialOutCallByHf(address));

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    hfpAgService_->activeAddr_ = address;
    EXPECT_TRUE(hfpAgService_->DialOutCallByHf(address));

    HILOGI("Hfp_Ag_Service_DialOutCallByHf_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognitionByHf_00100
 * @tc.name: OpenVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognitionByHf_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00100 start");
    hfpAgService_->isVrOpened_ = true;
    EXPECT_FALSE(hfpAgService_->OpenVoiceRecognitionByHf(address));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognitionByHf_00200
 * @tc.name: OpenVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognitionByHf_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00200 start");
    hfpAgService_->isVrOpened_ = false;
    string addr = "";
    EXPECT_FALSE(hfpAgService_->OpenVoiceRecognitionByHf(addr));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognitionByHf_00300
 * @tc.name: OpenVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognitionByHf_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00300 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = true;
    EXPECT_FALSE(hfpAgService_->OpenVoiceRecognitionByHf(address));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognitionByHf_00400
 * @tc.name: OpenVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognitionByHf_00400, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00400 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address,
        static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTED));
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    EXPECT_FALSE(hfpAgService_->OpenVoiceRecognitionByHf(address));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00400 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognitionByHf_00500
 * @tc.name: OpenVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognitionByHf_00500, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00500 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address,
        static_cast<int>(HFP_AG_STATE_CONNECTED));
    hfpAgService_->voiceRecognitionTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::VoiceRecognitionTimeOut, this));
    EXPECT_FALSE(hfpAgService_->OpenVoiceRecognitionByHf(address));
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00500 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OpenVoiceRecognitionByHf_00600
 * @tc.name: OpenVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OpenVoiceRecognitionByHf_00600, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00600 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;

    hfpAgService_->voiceRecognitionTimeout_ = nullptr;

    if (hfpAgService_->WakeUpVoiceRecognition(address) == BT_NO_ERROR) {
        EXPECT_TRUE(hfpAgService_->OpenVoiceRecognitionByHf(address));
    } else {
        EXPECT_FALSE(hfpAgService_->OpenVoiceRecognitionByHf(address));
    }
    HILOGI("Hfp_Ag_Service_OpenVoiceRecognitionByHf_00600 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CloseVoiceRecognitionByHf_00100
 * @tc.name: CloseVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CloseVoiceRecognitionByHf_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CloseVoiceRecognitionByHf_00100 start");
    EXPECT_FALSE(hfpAgService_->CloseVoiceRecognitionByHf(address));
    HILOGI("Hfp_Ag_Service_CloseVoiceRecognitionByHf_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CloseVoiceRecognitionByHf_00400
 * @tc.name: CloseVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CloseVoiceRecognitionByHf_00400, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CloseVoiceRecognitionByHf_00200 start");
    hfpAgService_->activeAddr_ = address;
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->voiceRecognitionTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::VoiceRecognitionTimeOut, this));
    EXPECT_TRUE(hfpAgService_->CloseVoiceRecognitionByHf(address));
    HILOGI("Hfp_Ag_Service_CloseVoiceRecognitionByHf_00400 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CloseVoiceRecognitionByHf_00200
 * @tc.name: CloseVoiceRecognitionByHf
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CloseVoiceRecognitionByHf_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CloseVoiceRecognitionByHf_00200 start");
    hfpAgService_->activeAddr_ = address;
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->voiceRecognitionTimeout_ = nullptr;
    EXPECT_FALSE(hfpAgService_->CloseVoiceRecognitionByHf(address));
    HILOGI("Hfp_Ag_Service_CloseVoiceRecognitionByHf_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SlcStateChanged_00100
 * @tc.name: SlcStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SlcStateChanged_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SlcStateChanged_00100 start");

    hfpAgService_->SlcStateChanged(address, HFP_AG_STATE_CONNECTING);
    hfpAgService_->SlcStateChanged(address, HFP_AG_STATE_DISCONNECTED);

    hfpAgService_->activeAddr_ = address;
    hfpAgService_->SlcStateChanged(address, HFP_AG_STATE_DISCONNECTED);

    HILOGI("Hfp_Ag_Service_SlcStateChanged_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ScoStateChanged_00100
 * @tc.name: ScoStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ScoStateChanged_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ScoStateChanged_00100 start");

    hfpAgService_->ScoStateChanged(address, HFP_AG_AUDIO_STATE_CONNECTING);
    hfpAgService_->ScoStateChanged(address, HFP_AG_AUDIO_STATE_DISCONNECTED);

    hfpAgService_->isVrOpened_ = true;
    hfpAgService_->voiceRecognitionTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::VoiceRecognitionTimeOut, this));
    hfpAgService_->ScoStateChanged(address, HFP_AG_AUDIO_STATE_DISCONNECTED);

    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = true;
    hfpAgService_->ScoStateChanged(address, HFP_AG_AUDIO_STATE_DISCONNECTED);

    hfpAgService_->activeAddr_ = address;
    hfpAgService_->ScoStateChanged(address, HFP_AG_AUDIO_STATE_DISCONNECTED);

    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->ScoStateChanged(address, HFP_AG_AUDIO_STATE_DISCONNECTED);
    EXPECT_TRUE(hfpAgService_->isAllowConnectSco_);
    HILOGI("Hfp_Ag_Service_ScoStateChanged_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ScoStateChanged_00200
 * @tc.name: ScoStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ScoStateChanged_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ScoStateChanged_00200 start");

    hfpAgService_->activeAddr_ = address;
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = true;
    hfpAgService_->ScoStateChanged(address, HFP_AG_AUDIO_STATE_DISCONNECTED);
    EXPECT_TRUE(hfpAgService_->isAllowConnectSco_);
    HILOGI("Hfp_Ag_Service_ScoStateChanged_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CovertConnectStateFromStack_00100
 * @tc.name: CovertConnectStateFromStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CovertConnectStateFromStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CovertConnectStateFromStack_00100 start");

    EXPECT_EQ(hfpAgService_->CovertConnectStateFromStack(BTHF_CONNECTION_STATE_DISCONNECTING),
        HFP_AG_STATE_DISCONNECTING);
    EXPECT_EQ(hfpAgService_->CovertConnectStateFromStack(BTHF_CONNECTION_STATE_CONNECTING),
        HFP_AG_STATE_CONNECTING);
    EXPECT_EQ(hfpAgService_->CovertConnectStateFromStack(BTHF_CONNECTION_STATE_CONNECTED),
        HFP_AG_STATE_CONNECTING);
    EXPECT_EQ(hfpAgService_->CovertConnectStateFromStack(BTHF_CONNECTION_STATE_SLC_CONNECTED),
        HFP_AG_STATE_CONNECTED);
    EXPECT_EQ(hfpAgService_->CovertConnectStateFromStack(BTHF_CONNECTION_STATE_DISCONNECTED),
        HFP_AG_STATE_DISCONNECTED);

    HILOGI("Hfp_Ag_Service_CovertConnectStateFromStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CovertAudioStateFromStack_00100
 * @tc.name: CovertAudioStateFromStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CovertAudioStateFromStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CovertAudioStateFromStack_00100 start");

    EXPECT_EQ(hfpAgService_->CovertAudioStateFromStack(BTHF_AUDIO_STATE_CONNECTING),
        HFP_AG_AUDIO_STATE_CONNECTING);
    EXPECT_EQ(hfpAgService_->CovertAudioStateFromStack(BTHF_AUDIO_STATE_DISCONNECTING),
        HFP_AG_AUDIO_STATE_DISCONNECTING);
    EXPECT_EQ(hfpAgService_->CovertAudioStateFromStack(BTHF_AUDIO_STATE_CONNECTED),
        HFP_AG_AUDIO_STATE_CONNECTED);
    EXPECT_EQ(hfpAgService_->CovertAudioStateFromStack(BTHF_AUDIO_STATE_DISCONNECTED),
        HFP_AG_AUDIO_STATE_DISCONNECTED);

    HILOGI("Hfp_Ag_Service_CovertAudioStateFromStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CovertVRStateFromStack_00100
 * @tc.name: CovertVRStateFromStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CovertVRStateFromStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CovertVRStateFromStack_00100 start");

    EXPECT_EQ(hfpAgService_->CovertVRStateFromStack(BTHF_VR_STATE_STOPPED), HFP_AG_HF_VR_ClOSED);
    EXPECT_EQ(hfpAgService_->CovertVRStateFromStack(BTHF_VR_STATE_STARTED), HFP_AG_HF_VR_OPENED);
    EXPECT_EQ(hfpAgService_->CovertVRStateFromStack(static_cast<bthf_vr_state_t>(2)), HFP_AG_HF_VR_ClOSED);

    HILOGI("Hfp_Ag_Service_CovertVRStateFromStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CovertVolumeControlTypeFromStack_00100
 * @tc.name: CovertVolumeControlTypeFromStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CovertVolumeControlTypeFromStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CovertVolumeControlTypeFromStack_00100 start");

    EXPECT_EQ(hfpAgService_->CovertVolumeControlTypeFromStack(BTHF_VOLUME_TYPE_SPK), HFP_AG_VOLUME_TYPE_SPK);
    EXPECT_EQ(hfpAgService_->CovertVolumeControlTypeFromStack(BTHF_VOLUME_TYPE_MIC), HFP_AG_VOLUME_TYPE_MIC);
    EXPECT_EQ(hfpAgService_->CovertVolumeControlTypeFromStack(static_cast<bthf_volume_type_t>(2)),
        HFP_AG_VOLUME_TYPE_SPK);

    HILOGI("Hfp_Ag_Service_CovertVolumeControlTypeFromStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CovertNoiseReductionFromStack_00100
 * @tc.name: CovertNoiseReductionFromStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CovertNoiseReductionFromStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CovertNoiseReductionFromStack_00100 start");

    EXPECT_FALSE(hfpAgService_->CovertNoiseReductionFromStack(BTHF_NREC_STOP));
    EXPECT_TRUE(hfpAgService_->CovertNoiseReductionFromStack(BTHF_NREC_START));
    EXPECT_FALSE(hfpAgService_->CovertNoiseReductionFromStack(static_cast<bthf_nrec_t>(2)));

    HILOGI("Hfp_Ag_Service_CovertNoiseReductionFromStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ConvetWbsConfigFromStack_00100
 * @tc.name: ConvetWbsConfigFromStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ConvetWbsConfigFromStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ConvetWbsConfigFromStack_00100 start");

    EXPECT_EQ(hfpAgService_->ConvetWbsConfigFromStack(BTHF_WBS_NO), HFP_AG_WBS_NO);
    EXPECT_EQ(hfpAgService_->ConvetWbsConfigFromStack(BTHF_WBS_YES), HFP_AG_WBS_YES);
    EXPECT_EQ(hfpAgService_->ConvetWbsConfigFromStack(BTHF_WBS_SUPER), HFP_AG_SWB_YES);
    EXPECT_EQ(hfpAgService_->ConvetWbsConfigFromStack(BTHF_WBS_NONE), HFP_AG_WBS_NONE);

    HILOGI("Hfp_Ag_Service_ConvetWbsConfigFromStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CovertBievValueFromStack_00100
 * @tc.name: CovertBievValueFromStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CovertBievValueFromStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CovertBievValueFromStack_00100 start");

    EXPECT_EQ(hfpAgService_->CovertBievValueFromStack(BTHF_HF_IND_ENHANCED_DRIVER_SAFETY),
        HFP_AG_HF_INDICATOR_ENHANCED_DRIVER_SAFETY_ID);
    EXPECT_EQ(hfpAgService_->CovertBievValueFromStack(BTHF_HF_IND_BATTERY_LEVEL_STATUS),
        HFP_AG_HF_INDICATOR_BATTERY_LEVEL_ID);
    EXPECT_EQ(hfpAgService_->CovertBievValueFromStack(static_cast<bthf_hf_ind_type_t>(3)),
        HFP_AG_HF_INDICATOR_ENHANCED_DRIVER_SAFETY_ID);

    HILOGI("Hfp_Ag_Service_CovertBievValueFromStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_RegisterObserver_00100
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_RegisterObserver_00100 start");

    hfpAgService_->RegisterObserver(g_hfpAgServiceObserver);

    HILOGI("Hfp_Ag_Service_RegisterObserver_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DeregisterObserver_00100
 * @tc.name: DeregisterObserver
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DeregisterObserver_00100 start");

    hfpAgService_->DeregisterObserver(g_hfpAgServiceObserver);

    HILOGI("Hfp_Ag_Service_DeregisterObserver_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SetConnectStrategy_00100
 * @tc.name: SetConnectStrategy
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SetConnectStrategy_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SetConnectStrategy_00100 start");

    BluetoothRawAddress device("00");
    EXPECT_EQ(hfpAgService_->SetConnectStrategy(device, 0), Bluetooth::BT_ERR_INTERNAL_ERROR);

    HILOGI("Hfp_Ag_Service_SetConnectStrategy_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_OnVolumeKeyEvent_00100
 * @tc.name: OnVolumeKeyEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_OnVolumeKeyEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_OnVolumeKeyEvent_00100 start");

    HILOGI("Hfp_Ag_Service_OnVolumeKeyEvent_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptConnection_00100
 * @tc.name: IsAcceptConnection
 * @tc.desc: reject connection with strategy forbidden
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptConnection_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptConnection_00100 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    config->Load();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HFP_CONNECTION_POLICY, strategyValue);

    // check IsAcceptConnection
    EXPECT_FALSE(hfpAgService_->IsAcceptConnection(rawAddr, HFP_AG_STATE_CONNECTING));
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hfp_Ag_Service_IsAcceptConnection_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptConnection_00101
 * @tc.name: IsAcceptConnection
 * @tc.desc: accept connection with strategy allowed
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptConnection_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptConnection_00101 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HFP_CONNECTION_POLICY, strategyValue);

    // check IsAcceptConnection
    EXPECT_TRUE(hfpAgService_->IsAcceptConnection(rawAddr, HFP_AG_STATE_CONNECTING));
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hfp_Ag_Service_IsAcceptConnection_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptConnection_00102
 * @tc.name: IsAcceptConnection
 * @tc.desc: accept connection with HFP_AG_STATE_DISCONNECTING
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptConnection_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptConnection_00102 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HFP_CONNECTION_POLICY, strategyValue);

    // check IsAcceptConnection
    EXPECT_TRUE(hfpAgService_->IsAcceptConnection(rawAddr, HFP_AG_STATE_DISCONNECTING));
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hfp_Ag_Service_IsAcceptConnection_00102 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ConnectionStateCallbackInner_00100
 * @tc.name: ConnectionStateCallbackInner
 * @tc.desc: accept connection
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ConnectionStateCallbackInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ConnectionStateCallbackInner_00100 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    EXPECT_TRUE(config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HFP_CONNECTION_POLICY, strategyValue));

    // check IsAcceptConnection
    hfpAgService_->ConnectionStateCallbackInner(rawAddr, HFP_AG_STATE_DISCONNECTING);
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hfp_Ag_Service_ConnectionStateCallbackInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ConnectionStateCallbackInner_00101
 * @tc.name: ConnectionStateCallbackInner
 * @tc.desc: reject connection
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ConnectionStateCallbackInner_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ConnectionStateCallbackInner_00101 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    // set strategyValue first
    IProfileConfig *config = ProfileConfig::GetInstance();
    config->Load();
    int strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    EXPECT_TRUE(config->SetValue(rawAddr.GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HFP_CONNECTION_POLICY, strategyValue));

    // check IsAcceptConnection
    hfpAgService_->ConnectionStateCallbackInner(rawAddr, HFP_AG_STATE_CONNECTING);
    config->RemoveAddr(rawAddr.GetAddress());

    HILOGI("Hfp_Ag_Service_ConnectionStateCallbackInner_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StartCellularCallSco_00100
 * @tc.name: StartCellularCallSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StartCellularCallSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StartCellularCallSco_00100 start");
    hfpAgService_->StartCellularCallSco();
    HILOGI("Hfp_Ag_Service_StartCellularCallSco_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StopCellularCallSco_00100
 * @tc.name: StopCellularCallSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StopCellularCallSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StopCellularCallSco_00100 start");
    hfpAgService_->StopCellularCallSco();
    HILOGI("Hfp_Ag_Service_StopCellularCallSco_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StartVirtualCallSco_00100
 * @tc.name: StartVirtualCallSco
 * @tc.desc: isVrOpened_ is true
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StartVirtualCallSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00100 start");
    hfpAgService_->isVrOpened_ = true;
    int32_t startScoRes = hfpAgService_->StartVirtualCallSco();
    EXPECT_EQ(startScoRes, Bluetooth::BT_ERR_NO_ACTIVE_HFP_DEVICE);
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StartVirtualCallSco_00101
 * @tc.name: StartVirtualCallSco
 * @tc.desc: isVirtualCallStarted_ is true
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StartVirtualCallSco_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00101 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = true;
    int32_t startScoRes = hfpAgService_->StartVirtualCallSco();
    EXPECT_EQ(startScoRes, Bluetooth::BT_ERR_NO_ACTIVE_HFP_DEVICE);
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StartVirtualCallSco_00102
 * @tc.name: StartVirtualCallSco
 * @tc.desc: StartVirtualCallSco success
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StartVirtualCallSco_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00102 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->StartVirtualCallSco();
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00102 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StartVirtualCallSco_00103
 * @tc.name: StartVirtualCallSco
 * @tc.desc: StartVirtualCallSco success
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StartVirtualCallSco_00103, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00103 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->activeAddr_ = address;
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    hfpAgService_->StartVirtualCallSco();
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00103 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StartVirtualCallSco_00104
 * @tc.name: StartVirtualCallSco
 * @tc.desc: StartVirtualCallSco success
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StartVirtualCallSco_00104, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00104 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->activeAddr_ = address;
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    hfpAgService_->StartVirtualCallSco();
    HILOGI("Hfp_Ag_Service_StartVirtualCallSco_00104 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StopVirtualCallSco_00100
 * @tc.name: StopVirtualCallSco
 * @tc.desc: isVirtualCallStarted_ is true
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StopVirtualCallSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StopVirtualCallSco_00100 start");
    hfpAgService_->isVirtualCallStarted_ = true;
    int32_t startScoRes = hfpAgService_->StopVirtualCallSco();
    EXPECT_EQ(startScoRes, Bluetooth::BT_NO_ERROR);
    HILOGI("Hfp_Ag_Service_StopVirtualCallSco_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StopVirtualCallSco_00101
 * @tc.name: StopVirtualCallSco
 * @tc.desc: isVirtualCallStarted_ is false
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StopVirtualCallSco_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StopVirtualCallSco_00101 start");
    hfpAgService_->isVirtualCallStarted_ = false;
    int32_t startScoRes = hfpAgService_->StopVirtualCallSco();
    EXPECT_EQ(startScoRes, Bluetooth::BT_ERR_VIRTUAL_CALL_NOT_STARTED);
    HILOGI("Hfp_Ag_Service_StopVirtualCallSco_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StopVirtualCallSco_00102
 * @tc.name: StopVirtualCallSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StopVirtualCallSco_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StopVirtualCallSco_00102 start");
    hfpAgService_->isVirtualCallStarted_ = true;
    std::string active_addr = "00:00:00:00:00:00";
    hfpAgService_->activeAddr_ = active_addr;
    std::string deActive_addr = "10:00:00:00:00:00";
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        deActive_addr, static_cast<int>(HFP_AG_STATE_CONNECTED));
    int32_t stopScoRes = hfpAgService_->StopVirtualCallSco();
    EXPECT_EQ(stopScoRes, Bluetooth::BT_NO_ERROR);
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState(deActive_addr);
    HILOGI("Hfp_Ag_Service_StopVirtualCallSco_00102 end");
}

HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StopA2dpOffload_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StopA2dpOffload_001 enter");
    hfpAgService_->StopA2dpOffload();
    HILOGI("Hfp_Ag_Service_StopA2dpOffload_001 end");
}

HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StopA2dpOffload_002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StopA2dpOffload_002 enter");
    BluetoothRawAddress device(address);
    hfpAgService_->isVrOpened_ = false;
    if (hfpAgService_->OpenVoiceRecognition(device)) {
        hfpAgService_->StopA2dpOffload();
    }
    HILOGI("Hfp_Ag_Service_StopA2dpOffload_002 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifyWearDetectionActionAfterConnected_0001
 * @tc.name: NotifyWearDetectionActionAfterConnected
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifyWearDetectionActionAfterConnected_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifyWearDetectionActionAfterConnected_0001 enter");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    hfpAgService_->NotifyWearDetectionActionAfterConnected(device);
    HILOGI("Hfp_Ag_Service_NotifyWearDetectionActionAfterConnected_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Service_BluetoothNotifyHfpStackChanged_0001
 * @tc.name: BluetoothNotifyHfpStackChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_BluetoothNotifyHfpStackChanged_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_BluetoothNotifyHfpStackChanged_0001 enter");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    hfpAgService_->BluetoothNotifyHfpStackChanged(1, device);
    HILOGI("Hfp_Ag_Service_BluetoothNotifyHfpStackChanged_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifyProfileState_0001
 * @tc.name: NotifyProfileState
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifyProfileState_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifyProfileState_0001 enter");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    hfpAgService_->NotifyProfileState(device);
    HILOGI("Hfp_Ag_Service_NotifyProfileState_0001 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ConnectSco_Ex_00100
 * @tc.name: ConnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ConnectSco_Ex_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ConnectSco_Ex_00100 start");
    int32_t res = hfpAgService_->ConnectSco(static_cast<uint8_t>(BTCallType::CALL_TYPE_CELLULAR));
    EXPECT_EQ(res, Bluetooth::BT_ERR_NO_ACTIVE_HFP_DEVICE);
    HILOGI("Hfp_Ag_Service_ConnectSco_Ex_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ConnectSco_Ex_00101
 * @tc.name: ConnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ConnectSco_Ex_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ConnectSco_Ex_00101 start");
    int32_t res = hfpAgService_->ConnectSco(static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL));
    EXPECT_EQ(res, Bluetooth::BT_ERR_NO_ACTIVE_HFP_DEVICE);
    HILOGI("Hfp_Ag_Service_ConnectSco_Ex_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ConnectSco_Ex_00102
 * @tc.name: ConnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ConnectSco_Ex_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ConnectSco_Ex_00102 start");
    int32_t res = hfpAgService_->ConnectSco(static_cast<uint8_t>(BTCallType::CALL_TYPE_RECOGNITION));
    EXPECT_EQ(res, Bluetooth::BT_ERR_INTERNAL_ERROR);
    HILOGI("Hfp_Ag_Service_ConnectSco_Ex_00102 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DisconnectSco_Ex_00101
 * @tc.name: DisconnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DisconnectSco_Ex_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DisconnectSco_Ex_00101 start");
    int32_t res = hfpAgService_->DisconnectSco(static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL));
    EXPECT_EQ(res, Bluetooth::BT_ERR_VIRTUAL_CALL_NOT_STARTED);
    HILOGI("Hfp_Ag_Service_DisconnectSco_Ex_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DisconnectSco_Ex_00102
 * @tc.name: DisconnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DisconnectSco_Ex_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DisconnectSco_Ex_00102 start");
    int32_t res = hfpAgService_->DisconnectSco(static_cast<uint8_t>(BTCallType::CALL_TYPE_RECOGNITION));
    EXPECT_EQ(res, Bluetooth::BT_ERR_INTERNAL_ERROR);
    HILOGI("Hfp_Ag_Service_DisconnectSco_Ex_00102 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptCellularCall_00100
 * @tc.name: IsAcceptCellularCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptCellularCall_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptCellularCall_00100 start");
    hfpAgService_->activeAddr_ = "";
    int32_t res = hfpAgService_->IsAcceptCellularCall();
    EXPECT_EQ(res, Bluetooth::BT_ERR_NO_ACTIVE_HFP_DEVICE);
    HILOGI("Hfp_Ag_Service_IsAcceptCellularCall_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptCellularCall_00101
 * @tc.name: IsAcceptCellularCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptCellularCall_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptCellularCall_00101 start");
    std::string address = "00:00:00:00:00:00";
    hfpAgService_->activeAddr_ = address;
    int32_t res = hfpAgService_->IsAcceptCellularCall();
    EXPECT_EQ(res, Bluetooth::BT_ERR_NULL_HFP_STATE_MACHINE);
    HILOGI("Hfp_Ag_Service_IsAcceptCellularCall_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptCellularCall_00102
 * @tc.name: IsAcceptCellularCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptCellularCall_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptCellularCall_00102 start");
    std::string address = "00:00:00:00:00:00";
    hfpAgService_->activeAddr_ = address;
    hfpAgService_->stateMachines_.insert_or_assign(address, std::make_unique<HfpAgStateMachine>(address));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    int32_t res = hfpAgService_->IsAcceptCellularCall();
    EXPECT_EQ(res, Bluetooth::BT_ERR_HFP_NOT_CONNECT);
    HILOGI("Hfp_Ag_Service_IsAcceptCellularCall_00102 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DisconnectCsCallSco_00100
 * @tc.name: DisconnectCsCallSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DisconnectCsCallSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DisconnectCsCallSco_00100 start");
    std::string address = "00:00:00:00:00:01";
    bool res = hfpAgService_->DisconnectCsCallSco(address);
    EXPECT_FALSE(res);
    HILOGI("Hfp_Ag_Service_DisconnectCsCallSco_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DisconnectCsCallSco_00101
 * @tc.name: DisconnectCsCallSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DisconnectCsCallSco_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DisconnectCsCallSco_00101 start");
    std::string address = "00:00:00:00:00:01";
    hfpAgService_->stateMachines_.insert_or_assign(address, std::make_unique<HfpAgStateMachine>(address));

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address]->current_ = &connectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTING));

    bool res = hfpAgService_->DisconnectCsCallSco(address);
    EXPECT_TRUE(res);
    HILOGI("Hfp_Ag_Service_DisconnectCsCallSco_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptVirtualCall_00100
 * @tc.name: IsAcceptVirtualCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptVirtualCall_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00100 start");
    hfpAgService_->activeAddr_ = "00:00:00:00:00:02";
    hfpAgService_->isVrOpened_ = true;
    int32_t res = hfpAgService_->IsAcceptVirtualCall();
    EXPECT_EQ(res, Bluetooth::BT_ERR_VR_HAS_BEEN_STARTED);
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptVirtualCall_00101
 * @tc.name: IsAcceptVirtualCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptVirtualCall_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00101 start");
    hfpAgService_->activeAddr_ = "00:00:00:00:00:02";
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = true;
    int32_t res = hfpAgService_->IsAcceptVirtualCall();
    EXPECT_EQ(res, Bluetooth::BT_ERR_AUDIO_NOT_IDLE);
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptVirtualCall_00102
 * @tc.name: IsAcceptVirtualCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptVirtualCall_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00102 start");
    std::string activeAddr = "";
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->activeAddr_ = activeAddr;
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[activeAddr] = std::make_unique<HfpAgStateMachine>(activeAddr);
    hfpAgService_->stateMachines_[activeAddr]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        activeAddr, static_cast<int>(HFP_AG_STATE_CONNECTED));
    int32_t res = hfpAgService_->IsAcceptVirtualCall();
    EXPECT_EQ(res, Bluetooth::BT_ERR_NO_ACTIVE_HFP_DEVICE);
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00102 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptVirtualCall_00103
 * @tc.name: IsAcceptVirtualCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptVirtualCall_00103, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00103 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->activeAddr_ = address;
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_[address]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, static_cast<int>(HFP_AG_STATE_CONNECTED));
    int32_t res = hfpAgService_->IsAcceptVirtualCall();
    EXPECT_EQ(res, Bluetooth::BT_NO_ERROR);
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00103 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptVirtualCall_00104
 * @tc.name: IsAcceptVirtualCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptVirtualCall_00104, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00104 start");
    hfpAgService_->isVrOpened_ = false;
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->activeAddr_ = "00:00:00:00:00:02";
    hfpAgService_->stateMachines_.insert_or_assign(address, std::make_unique<HfpAgStateMachine>(address));
    int32_t res = hfpAgService_->IsAcceptVirtualCall();
    EXPECT_EQ(res, Bluetooth::BT_ERR_HFP_NOT_CONNECT);
    HILOGI("Hfp_Ag_Service_IsAcceptVirtualCall_00104 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetVirtualCallActiveNum_00100
 * @tc.name: GetVirtualCallActiveNum
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetVirtualCallActiveNum_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetVirtualCallActiveNum_00100 start");
    int32_t virtualCallActiveNum = hfpAgService_->GetVirtualCallActiveNum();
    EXPECT_EQ(virtualCallActiveNum, 0);
    HILOGI("Hfp_Ag_Service_GetVirtualCallActiveNum_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetVirtualCallActiveNum_00101
 * @tc.name: GetVirtualCallActiveNum
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetVirtualCallActiveNum_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetVirtualCallActiveNum_00101 start");
    hfpAgService_->callStateMachine_ = std::make_unique<CallStateMachine>("00:00:00:00:00:02");
    int32_t virtualCallActiveNum = hfpAgService_->GetVirtualCallActiveNum();
    EXPECT_EQ(virtualCallActiveNum, 0);
    HILOGI("Hfp_Ag_Service_GetVirtualCallActiveNum_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendNoCarrier_00100
 * @tc.name: SendNoCarrier
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendNoCarrier_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendNoCarrier_00100 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    bool res = hfpAgService_->SendNoCarrier(rawAddr);
    EXPECT_FALSE(res);
    HILOGI("Hfp_Ag_Service_SendNoCarrier_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifyAudioVolumeEvent_00100
 * @tc.name: NotifyAudioVolumeEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifyAudioVolumeEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifyAudioVolumeEvent_00100 start");
    hfpAgService_->NotifyAudioVolumeEvent(0, 10);
    HILOGI("Hfp_Ag_Service_NotifyAudioVolumeEvent_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetVirtualCallState_00100
 * @tc.name: GetVirtualCallState
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetVirtualCallState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetVirtualCallState_00100 start");
    int res = hfpAgService_->GetVirtualCallState();
    EXPECT_EQ(res, HFP_AG_CALL_STATE_IDLE);
    HILOGI("Hfp_Ag_Service_GetVirtualCallState_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ClearCallStateMachine_00100
 * @tc.name: ClearCallStateMachine
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ClearCallStateMachine_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ClearCallStateMachine_00100 start");
    hfpAgService_->ClearCallStateMachine();
    HILOGI("Hfp_Ag_Service_ClearCallStateMachine_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifyPhonebookAuthResult_00100
 * @tc.name: NotifyPhonebookAuthResult
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifyPhonebookAuthResult_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifyPhonebookAuthResult_00100 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    hfpAgService_->NotifyPhonebookAuthResult(rawAddr, 1, 0);
    HILOGI("Hfp_Ag_Service_NotifyPhonebookAuthResult_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsVgsSupported_00100
 * @tc.name: IsVgsSupported
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsVgsSupported_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsVgsSupported_00100 start");
    RawAddress rawAddr("00:00:00:00:00:00");
    bool isSupported = false;
    hfpAgService_->IsVgsSupported(rawAddr, isSupported);
    HILOGI("Hfp_Ag_Service_IsVgsSupported_00100 end");
}

/*
 * @tc.number: Bluetooth_Phone_State_Unmarshalling_00100
 * @tc.name: Unmarshalling
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Bluetooth_Phone_State_Unmarshalling_00100, TestSize.Level1)
{
    HILOGI("Bluetooth_Phone_State_Unmarshalling_00100 start");
    BluetoothPhoneState phoneState;
    OHOS::Parcel parcel;
    phoneState.Unmarshalling(parcel);

    HILOGI("Bluetooth_Phone_State_Unmarshalling_00100 end");
}

/*
 * @tc.number: Bluetooth_Phone_State_WriteToParcel_00100
 * @tc.name: WriteToParcel
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Bluetooth_Phone_State_WriteToParcel_00100, TestSize.Level1)
{
    HILOGI("Bluetooth_Phone_State_WriteToParcel_00100 start");
    BluetoothPhoneState phoneState;
    OHOS::Parcel parcel;
    phoneState.WriteToParcel(parcel);
    phoneState.ReadFromParcel(parcel);
    HILOGI("Bluetooth_Phone_State_WriteToParcel_00100 end");
}

/*
 * @tc.number: Bluetooth_Raw_Address_WriteToParcel_00200
 * @tc.name: WriteToParcel
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Bluetooth_Raw_Address_WriteToParcel_00200, TestSize.Level1)
{
    HILOGI("Bluetooth_Raw_Address_WriteToParcel_00200 start");
    BluetoothRawAddress address;
    OHOS::Parcel parcel;
    address.WriteToParcel(parcel);
    HILOGI("Bluetooth_Raw_Address_WriteToParcel_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ShouldCallAudioBeActive_0100
 * @tc.name: ShouldCallAudioBeActive
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ShouldCallAudioBeActive_0100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ShouldCallAudioBeActive_0100 start");
    EXPECT_FALSE(hfpAgService_->ShouldCallAudioBeActive());
    HILOGI("Hfp_Ag_Service_ShouldCallAudioBeActive_0100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleCallStateChange_0100
 * @tc.name: HandleCallStateChange
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleCallStateChange_0100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleCallStateChange_0100 start");
    BluetoothPhoneState phoneState;
    hfpAgService_->HandleCallStateChange(phoneState, true);
    HILOGI("Hfp_Ag_Service_HandleCallStateChange_0100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleCallStateChange_0101
 * @tc.name: HandleCallStateChange
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleCallStateChange_0101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleCallStateChange_0101 start");
    BluetoothPhoneState phoneState;
    hfpAgService_->HandleCallStateChange(phoneState, false);
    HILOGI("Hfp_Ag_Service_HandleCallStateChange_0101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleCallStateChange_0102
 * @tc.name: HandleCallStateChange
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleCallStateChange_0102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleCallStateChange_0102 start");
    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(0);
    phoneState.SetCallState(HFP_AG_CALL_STATE_IDLE);
    hfpAgService_->activeAddr_ = "88:B6:BD:11:22:33";
    hfpAgService_->HandleCallStateChange(phoneState, false);
    EXPECT_EQ(phoneState.GetActiveNum(), 0);
    HILOGI("Hfp_Ag_Service_HandleCallStateChange_0102 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleCallStateChange_0103
 * @tc.name: HandleCallStateChange
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleCallStateChange_0103, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleCallStateChange_0103 start");
    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(1);
    phoneState.SetCallState(HFP_AG_CALL_STATE_INCOMING);
    hfpAgService_->activeAddr_ = "00:E0:4C:00:00:00";
    hfpAgService_->HandleCallStateChange(phoneState, false);
    EXPECT_EQ(phoneState.GetActiveNum(), 1);
    HILOGI("Hfp_Ag_Service_HandleCallStateChange_0103 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleBccBlockTimer_Start_00100
 * @tc.name: HandleBccBlockTimer
 * @tc.desc: Test BCC block timer start when shouldStart is true and timer not active
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleBccBlockTimer_Start_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleBccBlockTimer_Start_00100 start");
    hfpAgService_->isBccBlockTimerActive_ = false;
    hfpAgService_->HandleBccBlockTimer(true);
    EXPECT_TRUE(hfpAgService_->isBccBlockTimerActive_);
    EXPECT_NE(hfpAgService_->bccBlockTimer_, nullptr);
    HILOGI("Hfp_Ag_Service_HandleBccBlockTimer_Start_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleBccBlockTimer_Start_00101
 * @tc.name: HandleBccBlockTimer
 * @tc.desc: Test BCC block timer not start again when already active
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleBccBlockTimer_Start_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleBccBlockTimer_Start_00101 start");
    hfpAgService_->bccBlockTimer_ =
        std::make_unique<utility::Timer>([]() { /* timeout callback */ });
    hfpAgService_->isBccBlockTimerActive_ = true;
    hfpAgService_->HandleBccBlockTimer(true);
    EXPECT_TRUE(hfpAgService_->isBccBlockTimerActive_);
    HILOGI("Hfp_Ag_Service_HandleBccBlockTimer_Start_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleBccBlockTimer_Stop_00100
 * @tc.name: HandleBccBlockTimer
 * @tc.desc: Test BCC block timer stop when shouldStart is false
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleBccBlockTimer_Stop_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleBccBlockTimer_Stop_00100 start");
    hfpAgService_->bccBlockTimer_ =
        std::make_unique<utility::Timer>([]() { /* timeout callback */ });
    hfpAgService_->isBccBlockTimerActive_ = true;
    hfpAgService_->HandleBccBlockTimer(false);
    EXPECT_FALSE(hfpAgService_->isBccBlockTimerActive_);
    EXPECT_EQ(hfpAgService_->bccBlockTimer_, nullptr);
    HILOGI("Hfp_Ag_Service_HandleBccBlockTimer_Stop_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleBccBlockTimer_Stop_00101
 * @tc.name: HandleBccBlockTimer
 * @tc.desc: Test BCC block timer stop when timer not active - should be no-op
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleBccBlockTimer_Stop_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleBccBlockTimer_Stop_00101 start");
    hfpAgService_->isBccBlockTimerActive_ = false;
    hfpAgService_->HandleBccBlockTimer(false);
    EXPECT_FALSE(hfpAgService_->isBccBlockTimerActive_);
    EXPECT_EQ(hfpAgService_->bccBlockTimer_, nullptr);
    HILOGI("Hfp_Ag_Service_HandleBccBlockTimer_Stop_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendCallStateEvent_00100
 * @tc.name: SendCallStateEvent
 * @tc.desc: Test SendCallStateEvent with delay = true - event is posted to delay queue
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendCallStateEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendCallStateEvent_00100 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {0, 0, HFP_AG_CALL_STATE_IDLE, "1234567890", 0, "test"};
    int callStateBefore = HfpAgSystemInterface::GetInstance().GetCallState();
    hfpAgService_->SendCallStateEvent(true, 100, curEvent, "test_delay_task");
    EXPECT_EQ(HfpAgSystemInterface::GetInstance().GetCallState(), callStateBefore);
    HILOGI("Hfp_Ag_Service_SendCallStateEvent_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendCallStateEvent_00101
 * @tc.name: SendCallStateEvent
 * @tc.desc: Test SendCallStateEvent with delay = false - event is posted to immediate queue
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendCallStateEvent_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendCallStateEvent_00101 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {1, 0, HFP_AG_CALL_STATE_ACTIVE, "1234567890", 0, "test"};
    int callStateBefore = HfpAgSystemInterface::GetInstance().GetCallState();
    hfpAgService_->SendCallStateEvent(false, 100, curEvent, "test_immediate_task");
    EXPECT_EQ(HfpAgSystemInterface::GetInstance().GetCallState(), callStateBefore);
    HILOGI("Hfp_Ag_Service_SendCallStateEvent_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleRealCallStateChange_001002
 * @tc.name: HandleRealCallStateChange
 * @tc.desc: Test HandleRealCallStateChange with shouldStopBccTimer (numActive=0, IDLE)
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleRealCallStateChange_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleRealCallStateChange_00100 start");
    hfpAgService_->bccBlockTimer_ =
        std::make_unique<utility::Timer>([]() { /* timeout callback */ });
    hfpAgService_->isBccBlockTimerActive_ = true;
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    int numActive = 0;
    int callState = HFP_AG_CALL_STATE_IDLE;
    bool isDelayCiev = false;
    bool isDelayCallState = false;
    hfpAgService_->HandleRealCallStateChange(numActive, callState, isDelayCiev, isDelayCallState, curEvent);
    EXPECT_FALSE(hfpAgService_->isBccBlockTimerActive_);
    HILOGI("Hfp_Ag_Service_HandleRealCallStateChange_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleRealCallStateChange_00101
 * @tc.name: HandleRealCallStateChange
 * @tc.desc: Test HandleRealCallStateChange with shouldStartBccTimer (numActive=1, IDLE, not active)
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleRealCallStateChange_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleRealCallStateChange_00101 start");
    hfpAgService_->isBccBlockTimerActive_ = false;
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    int numActive = 1;
    int callState = HFP_AG_CALL_STATE_IDLE;
    bool isDelayCiev = false;
    bool isDelayCallState = false;
    hfpAgService_->HandleRealCallStateChange(numActive, callState, isDelayCiev, isDelayCallState, curEvent);
    EXPECT_TRUE(hfpAgService_->isBccBlockTimerActive_);
    HILOGI("Hfp_Ag_Service_HandleRealCallStateChange_00101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleRealCallStateChange_00102
 * @tc.name: HandleRealCallStateChange
 * @tc.desc: Test HandleRealCallStateChange with neither start nor stop condition
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleRealCallStateChange_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleRealCallStateChange_00102 start");
    hfpAgService_->isBccBlockTimerActive_ = false;
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    int numActive = 2;
    int callState = HFP_AG_CALL_STATE_ACTIVE;
    bool isDelayCiev = false;
    bool isDelayCallState = false;
    hfpAgService_->HandleRealCallStateChange(numActive, callState, isDelayCiev, isDelayCallState, curEvent);
    EXPECT_FALSE(hfpAgService_->isBccBlockTimerActive_);
    HILOGI("Hfp_Ag_Service_HandleRealCallStateChange_00102 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleRealCallStateChange_00103
 * @tc.name: HandleRealCallStateChange
 * @tc.desc: Test HandleRealCallStateChange with shouldStopBccTimer and isDelayCiev=true
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleRealCallStateChange_00103, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleRealCallStateChange_00103 start");
    hfpAgService_->bccBlockTimer_ =
        std::make_unique<utility::Timer>([]() { /* timeout callback */ });
    hfpAgService_->isBccBlockTimerActive_ = true;
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    int numActive = 0;
    int callState = HFP_AG_CALL_STATE_IDLE;
    bool isDelayCiev = true;
    bool isDelayCallState = false;
    hfpAgService_->HandleRealCallStateChange(numActive, callState, isDelayCiev, isDelayCallState, curEvent);
    EXPECT_FALSE(hfpAgService_->isBccBlockTimerActive_);
    HILOGI("Hfp_Ag_Service_HandleRealCallStateChange_00103 end");
}


/*
 * @tc.number: Hfp_Ag_Service_HandleA2dpStateAfterCall_0100
 * @tc.name: HandleA2dpStateAfterCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleA2dpStateAfterCall_0100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleA2dpStateAfterCall_0100 start");
    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(1);
    hfpAgService_->HandleA2dpStateAfterCall(HFP_AG_CALL_STATE_INCOMING, false);
    EXPECT_EQ(phoneState.GetActiveNum(), 1);
    HILOGI("Hfp_Ag_Service_HandleA2dpStateAfterCall_0100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_HandleA2dpStateAfterCall_0101
 * @tc.name: HandleA2dpStateAfterCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_HandleA2dpStateAfterCall_0101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_HandleA2dpStateAfterCall_0101 start");
    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(0);
    hfpAgService_->HandleA2dpStateAfterCall(HFP_AG_CALL_STATE_IDLE, false);
    EXPECT_EQ(phoneState.GetActiveNum(), 0);
    HILOGI("Hfp_Ag_Service_HandleA2dpStateAfterCall_0101 end");
}

/*
 * @tc.number: Hfp_Ag_Service_StopDialingTimer_0100
 * @tc.name: StopDialingTimer
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StopDialingTimer_0100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StopDialingTimer_0100 start");
    hfpAgService_->dialingOutTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::DialOutCallTimeOut, this));
    hfpAgService_->StopDialingTimer();
    HILOGI("Hfp_Ag_Service_StopDialingTimer_0100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_EnableBtCallLog_00100
 * @tc.name: EnableBtCallLog
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_EnableBtCallLog_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_EnableBtCallLog_00100 start");
    bool enable = true;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    hfpAgService_->EnableBtCallLog(enable);

    bool disable = false;
    hfpAgService_->EnableBtCallLog(disable);
    HILOGI("EnableBtCallLog end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetConnectingOrConnectionDevices_00100
 * @tc.name: GetConnectingOrConnectionDevices
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetConnectingOrConnectionDevices_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetConnectingOrConnectionDevices_00100 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);

    hfpAgService_->stateMachines_["1"] = nullptr;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &connectingState;

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["3"]->current_ = &connectedState;
    EXPECT_EQ(hfpAgService_->GetConnectingOrConnectionDevices().size(), 4);

    HILOGI("Hfp_Ag_Service_GetConnectingOrConnectionDevices_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendBsirValue_00100
 * @tc.name: SendBsirValue
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendBsirValue_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendBsirValue_00100 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;
    hfpAgService_->SendBsirValue();

    HILOGI("Hfp_Ag_Service_SendBsirValue_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendBsirValue_00200
 * @tc.name: SendBsirValue
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendBsirValue_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendBsirValue_00200 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;
    hfpAgService_->activeAddr_ = "";
    hfpAgService_->SendBsirValue();

    HILOGI("Hfp_Ag_Service_SendBsirValue_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendBsirValue_00300
 * @tc.name: SendBsirValue
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendBsirValue_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendBsirValue_00300 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;
    hfpAgService_->activeAddr_ = "00:00:00:00:00:00";
    hfpAgService_->SendBsirValue();

    HILOGI("Hfp_Ag_Service_SendBsirValue_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendBsirValue_00400
 * @tc.name: SendBsirValue
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendBsirValue_00400, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendBsirValue_00400 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->activeAddr_ = "11:22:33:44:55:66";
    hfpAgService_->stateMachines_["11:22:33:44:55:66"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["11:22:33:44:55:66"]->current_ = &connectingState;
    hfpAgService_->SendBsirValue();

    HILOGI("Hfp_Ag_Service_SendBsirValue_00400 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendBsirToLastRemainingDevice_00100
 * @tc.name: SendBsirToLastRemainingDevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendBsirToLastRemainingDevice_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendBsirToLastRemainingDevice_00100 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &connectingState;
    hfpAgService_->SendBsirToLastRemainingDevice();

    HILOGI("Hfp_Ag_Service_SendBsirToLastRemainingDevice_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendBsirToLastRemainingDevice_00200
 * @tc.name: SendBsirToLastRemainingDevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendBsirToLastRemainingDevice_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendBsirToLastRemainingDevice_00200 start");

    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connectingState;
    hfpAgService_->SendBsirToLastRemainingDevice();

    HILOGI("Hfp_Ag_Service_SendBsirToLastRemainingDevice_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifyAudioStateChanged_00100
 * @tc.name: NotifyAudioStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifyAudioStateChanged_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifyAudioStateChanged_00100 start");

    BluetoothRawAddress device("11:22:33:44:55:66");
    hfpAgService_->NotifyAudioStateChanged(device, HFP_AG_AUDIO_STATE_DISCONNECTED, 1);
    hfpAgService_->NotifyAudioStateChanged(device, HFP_AG_AUDIO_STATE_CONNECTING, 2);
    hfpAgService_->NotifyAudioStateChanged(device, HFP_AG_AUDIO_STATE_DISCONNECTING, 1);
    hfpAgService_->NotifyAudioStateChanged(device, HFP_AG_AUDIO_STATE_CONNECTED, 2);
    hfpAgService_->NotifyAudioStateChanged(device, HFP_AG_STATE_DISCONNECTING, 1);
    hfpAgService_->NotifyAudioStateChanged(device, HFP_AG_STATE_DISCONNECTING, 2);

    HILOGI("Hfp_Ag_Service_NotifyAudioStateChanged_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ReportStackScoStateChangeUeEvent_00100
 * @tc.name: ReportStackScoStateChangeUeEvent
 * @tc.desc: test func ReportStackScoStateChangeUeEvent
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ReportStackScoStateChangeUeEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ReportStackScoStateChangeUeEvent_00100 start");

    BluetoothRawAddress device("11:22:33:44:55:66");
    EXPECT_TRUE(hfpAgService_ != nullptr);
    hfpAgService_->ReportStackScoStateChangeUeEvent(device, HFP_AG_SCO_LOCAL_USER_TERMINATED);
    hfpAgService_->ReportStackScoStateChangeUeEvent(device, HFP_AG_SCO_REMOTE_USER_TERMINATED);
    hfpAgService_->ReportStackScoStateChangeUeEvent(device, HFP_AG_SCO_LOCAL_USER_SET_UP);
    hfpAgService_->ReportStackScoStateChangeUeEvent(device, HFP_AG_SCO_REMOTE_USER_SET_UP);
    hfpAgService_->ReportStackScoStateChangeUeEvent(device, HFP_AG_SCO_INVALID);
    HILOGI("Hfp_Ag_Service_ReportStackScoStateChangeUeEvent_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_NotifyConnStateChangedInner_00100
 * @tc.name: Hfp_Ag_Service_NotifyConnStateChangedInner_00100
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_NotifyConnStateChangedInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_NotifyConnStateChangedInner_00100 start");
    RawAddress addr("00:00:00:00:00:01");
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(addr);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(addr.GetAddress());
    hfpAgService_->stateMachines_["2"]->current_ = &disconnectedState;
    hfpAgService_->stateMachines_["2"]->SetTryReconnectHfp(true);
    hfpAgService_->GetDevicePreState(addr);
    hfpAgService_->stateMachines_["2"]->NotifyChildStateToParentState(0, 3, 0);
    hfpAgService_->GetDevicePreState(addr);
    bluetoothDevice->SetAclConnectState(1);
    hfpAgService_->NotifyConnStateChangedInner(addr, 1, 0);
    hfpAgService_->stateMachines_["2"]->SetTryReconnectHfp(false);
    hfpAgService_->NotifyConnStateChangedInner(addr, 1, 0);
    hfpAgService_->NotifyHfEnhancedDriverSafety(addr, 0);
    HILOGI("Hfp_Ag_Service_NotifyConnStateChangedInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_postHfpConnectEvent_00100
 * @tc.name: postHfpConnectEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_postHfpConnectEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_postHfpConnectEvent_00100 start");
    hfpAgService_->postHfpConnectEvent(false, address);
    hfpAgService_->postHfpConnectEvent(true, address);
    sleep(TIME);
    auto it = hfpAgService_->stateMachines_.find(address);
    EXPECT_TRUE(it->second->GetTryReconnectHfp());
    HILOGI("Hfp_Ag_Service_postHfpConnectEvent_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ClearVirtualCallInfo_00100
 * @tc.name: ClearVirtualCallInfo
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ClearVirtualCallInfo_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ClearVirtualCallInfo_00100 start");

    BluetoothRawAddress device(address);
    hfpAgService_->isVirtualCallStarted_ = false;
    hfpAgService_->ClearVirtualCallInfo(device);
    EXPECT_FALSE(hfpAgService_->isVirtualCallStarted_);

    hfpAgService_->isVirtualCallStarted_ = true;
    hfpAgService_->activeAddr_ = "11:22:33:44:55:66";
    hfpAgService_->ClearVirtualCallInfo(device);
    EXPECT_TRUE(hfpAgService_->isVirtualCallStarted_);

    hfpAgService_->isVirtualCallStarted_ = true;
    hfpAgService_->activeAddr_ = address;
    hfpAgService_->ClearVirtualCallInfo(device);
    EXPECT_FALSE(hfpAgService_->isVirtualCallStarted_);

    HILOGI("Hfp_Ag_Service_ClearVirtualCallInfo_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsNeedConnectSco_00100
 * @tc.name: IsNoNeedConnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsNeedConnectSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsNeedConnectSco_00100 start");

    hfpAgService_->activeAddr_ = "11:22:33:44:55:66";
    EXPECT_FALSE(hfpAgService_->IsNoNeedConnectSco());

    BluetoothAudioManager::GetInstance().SetProfileStatus(hfpAgService_->activeAddr_,
        static_cast<uint8_t>(A2DP_STATUS_ENABLE), static_cast<uint8_t>(HFP_STATUS_DISABLE));
    EXPECT_FALSE(hfpAgService_->IsNoNeedConnectSco());

    BluetoothAudioManager::GetInstance().SetProfileStatus(hfpAgService_->activeAddr_,
        static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE), static_cast<uint8_t>(HFP_STATUS_ENABLE));
    EXPECT_FALSE(hfpAgService_->IsNoNeedConnectSco());

    BluetoothAudioManager::GetInstance().SetProfileStatus(hfpAgService_->activeAddr_,
        static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE), static_cast<uint8_t>(HFP_STATUS_ENABLE));
    hfpAgService_->isVirtualCallStarted_ = false;
    HfpAgSystemInterface::GetInstance().callState_ = HFP_AG_CALL_STATE_INCOMING;
    hfpAgService_->CheckAndResumeSco(1, 0, 8);

    HILOGI("Hfp_Ag_Service_IsNeedConnectSco_00100 end");
}

HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsNeedConnectSco_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsNeedConnectSco_00200 start");
#ifdef BLUETOOTH_HFP_AG_ENABLE
    auto callManagerClient = DelayedSingleton<Telephony::CallManagerClient>::GetInstance();
    if (callManagerClient != nullptr) {
        callManagerClient->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
        int32_t callState = callManagerClient->GetCallState();
        hfpAgService_->isVirtualCallStarted_ = true;
        if (callState != static_cast<int32_t>(Telephony::CallStateToApp::CALL_STATE_RINGING)) {
            EXPECT_FALSE(hfpAgService_->IsNoNeedConnectSco());
        }
    }
#endif
    HILOGI("Hfp_Ag_Service_IsNeedConnectSco_00200 end");
}
/*
 * @tc.number: Hfp_Ag_Service_StopVoiceRecognitionToStack_00100
 * @tc.name: StopVoiceRecognitionToStack
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_StopVoiceRecognitionToStack_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_StopVoiceRecognitionToStack_00100 start");
    hfpAgService_->isVrOpened_ = true;
    hfpAgService_->StopVoiceRecognitionToStack(address);
    EXPECT_TRUE(hfpAgService_->isVrOpened_);
    HILOGI("Hfp_Ag_Service_StopVoiceRecognitionToStack_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsInActualcall_00100
 * @tc.name: IsInActualcall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsInActualcall_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsInActualcall_00100 start");
    EXPECT_FALSE(hfpAgService_->IsInActualcall());
    HILOGI("Hfp_Ag_Service_IsInActualcall_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetMaxConnectionDevicesNum_00100
 * @tc.name: GetMaxConnectionDevicesNum
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetMaxConnectionDevicesNum_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetMaxConnectionDevicesNum_00100 start");
    EXPECT_EQ(hfpAgService_->GetMaxConnectionDevicesNum(), 5);
    HILOGI("Hfp_Ag_Service_GetMaxConnectionDevicesNum_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CallDetailsChanged_00100
 * @tc.name: CallDetailsChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CallDetailsChanged_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CallDetailsChanged_00100 start");
    hfpAgService_->CallDetailsChanged(0, 0);
    HILOGI("Hfp_Ag_Service_CallDetailsChanged_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_CallDetailsChanged_00200
 * @tc.name: CallDetailsChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_CallDetailsChanged_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_CallDetailsChanged_00200 start");
    hfpAgService_->CallDetailsChanged(0, HFP_AG_CALL_STATE_DISCONNECTED);
    HILOGI("Hfp_Ag_Service_CallDetailsChanged_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsRemoteHfpSupported_00100
 * @tc.name: IsRemoteHfpSupported
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsRemoteHfpSupported_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsRemoteHfpSupported_00100 start");
    BluetoothRawAddress device(address);
    hfpAgService_->IsRemoteHfpSupported(device);
    HILOGI("Hfp_Ag_Service_IsRemoteHfpSupported_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendCloseVoiceEvent_00100
 * @tc.name: SendCloseVoiceEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendCloseVoiceEvent_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendCloseVoiceEvent_00100 start");
    bool disconnectSco = true;
    hfpAgService_->SendCloseVoiceEvent(address, disconnectSco);
    HILOGI("Hfp_Ag_Service_SendCloseVoiceEvent_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_ProcessVoiceRecognitionTimeOut_00100
 * @tc.name: ProcessVoiceRecognitionTimeOut
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_ProcessVoiceRecognitionTimeOut_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_ProcessVoiceRecognitionTimeOut_00100 start");
    hfpAgService_->voiceRecognitionTimeout_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpAgServiceTest::VoiceRecognitionTimeOut, this));
    HfpAgMessage clccTimeOut(HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT);
    hfpAgService_->ProcessVoiceRecognitionTimeOut(clccTimeOut);
    hfpAgService_->DialOutCallTimeOut();
    HILOGI("Hfp_Ag_Service_ProcessVoiceRecognitionTimeOut_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_UpdateLocalVoiceCombineStateForScoStateChanged_001
 * @tc.name: UpdateLocalVoiceCombineStateForScoStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_UpdateLocalVoiceCombineStateForScoStateChanged_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_UpdateLocalVoiceCombineStateForScoStateChanged_001 start");
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    std::string address = "00:00:00:00:00:00";
    hfpAgService_->UpdateLocalVoiceCombineStateForScoStateChanged(address, true);
    hfpAgService_->UpdateLocalVoiceCombineStateForScoStateChanged(address, false);
    HILOGI("Hfp_Ag_Service_UpdateLocalVoiceCombineStateForScoStateChanged_001 end");
}

/*
 * @tc.number: Hfp_Ag_Service_SendCloseVoiceEvent_00200
 * @tc.name: SendCloseVoiceEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_SendCloseVoiceEvent_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_SendCloseVoiceEvent_00200 start");
    bool disconnectSco = false;
    hfpAgService_->SendCloseVoiceEvent(address, disconnectSco);
    HILOGI("Hfp_Ag_Service_SendCloseVoiceEvent_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetMaxConnectNum_00100
 * @tc.name: GetMaxConnectNum
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetMaxConnectNum_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetMaxConnectNum_00100 start");
    hfpAgService_->GetMaxConnectNum();
    HILOGI("Hfp_Ag_Service_GetMaxConnectNum_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DisconnectSco_00100
 * @tc.name: DisconnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DisconnectSco_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DisconnectSco_00100 start");
    EXPECT_FALSE(hfpAgService_->DisconnectSco());

    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    HfpAgDisconnected disconnectedState(HfpAgStateMachine::DISCONNECTED, stateMachine, g_eventProcessor);
    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgConnecting connectingState(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgAudioDisconnecting audioDisconnecting(HfpAgStateMachine::AUDIO_DISCONNECTING,
        stateMachine, g_eventProcessor, disconnectingState);
    HfpAgAudioConnecting audioConnecting(HfpAgStateMachine::AUDIO_CONNECTING,
        stateMachine, g_eventProcessor, connectingState);
    HfpAgAudioConnected audioConnected(HfpAgStateMachine::AUDIO_CONNECTED,
        stateMachine, g_eventProcessor, connectedState);

    hfpAgService_->stateMachines_["1"] = nullptr;

    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &disconnectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTING));

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["3"]->current_ = &disconnectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_DISCONNECTED));

    hfpAgService_->stateMachines_["4"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["4"]->current_ = &connectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_CONNECTED));

    hfpAgService_->stateMachines_["5"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["5"]->current_ = &connectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_STATE_CONNECTING));

    hfpAgService_->stateMachines_["6"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["6"]->current_ = &audioDisconnecting;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_DISCONNECTING));

    hfpAgService_->stateMachines_["7"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["7"]->current_ = &audioConnecting;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTING));

    hfpAgService_->stateMachines_["8"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["8"]->current_ = &audioConnected;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTED));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        "8", static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTED));

    EXPECT_TRUE(hfpAgService_->DisconnectSco());

    HILOGI("Hfp_Ag_Service_DisconnectSco_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAudioConnected_00100
 * @tc.name: IsAudioConnected
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAudioConnected_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAudioConnected_00100 start");

    HfpAgConnecting connecting(HfpAgStateMachine::CONNECTING, stateMachine, g_eventProcessor);
    HfpAgAudioConnecting audioConnecting(HfpAgStateMachine::AUDIO_CONNECTING,
        stateMachine, g_eventProcessor, connecting);

    hfpAgService_->stateMachines_["0"] = nullptr;
    hfpAgService_->stateMachines_["1"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["1"]->current_ = &connecting;
    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &audioConnecting;

    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("0", static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTED));
    EXPECT_TRUE(hfpAgService_->IsAudioConnected());

    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    HILOGI("Hfp_Ag_Service_IsAudioConnected_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAudioConnected_00200
 * @tc.name: IsAudioConnected
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAudioConnected_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAudioConnected_00200 start");

    EXPECT_FALSE(hfpAgService_->IsAudioConnected("00"));

    hfpAgService_->stateMachines_["1"] = nullptr;
    EXPECT_FALSE(hfpAgService_->IsAudioConnected("1"));

    HfpAgDisconnecting disconnectingState(HfpAgStateMachine::DISCONNECTING, stateMachine, g_eventProcessor);
    hfpAgService_->stateMachines_["2"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["2"]->current_ = &disconnectingState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_DISCONNECTING));
    EXPECT_FALSE(hfpAgService_->IsAudioConnected("2"));

    HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
    HfpAgAudioConnected audioConnectedState(HfpAgStateMachine::AUDIO_CONNECTED,
        stateMachine, g_eventProcessor, connectedState);
    hfpAgService_->stateMachines_["3"] = std::make_unique<HfpAgStateMachine>(address);
    hfpAgService_->stateMachines_["3"]->current_ = &audioConnectedState;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTED));
    EXPECT_TRUE(hfpAgService_->IsAudioConnected(address));

    HILOGI("Hfp_Ag_Service_IsAudioConnected_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Service_GetVirtualDeviceList_00100
 * @tc.name: GetVirtualDeviceList
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_GetVirtualDeviceList_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_GetVirtualDeviceList_00100 start");
    std::vector<std::string> devices;
    hfpAgService_->GetVirtualDeviceList(devices);
    HILOGI("Hfp_Ag_Service_GetVirtualDeviceList_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DisconnectSco_Ex_00100
 * @tc.name: DisconnectSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DisconnectSco_Ex_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DisconnectSco_Ex_00100 start");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("0");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("1");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("2");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("3");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("4");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("5");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("6");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("7");
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState("8");
    int32_t res = hfpAgService_->DisconnectSco(static_cast<uint8_t>(BTCallType::CALL_TYPE_CELLULAR));
    EXPECT_EQ(res, Bluetooth::BT_NO_ERROR);
    HILOGI("Hfp_Ag_Service_DisconnectSco_Ex_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_UpdateVirtualDevice_00100
 * @tc.name: UpdateVirtualDevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_UpdateVirtualDevice_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_UpdateVirtualDevice_00100 start");
    std::string address = "00:00:00:00:00:00";
    hfpAgService_->UpdateVirtualDevice(0, address);
    hfpAgService_->UpdateVirtualDevice(1, address);
    address = "00:00:00:00:00:0";
    hfpAgService_->UpdateVirtualDevice(0, address);
    hfpAgService_->UpdateVirtualDevice(1, address);
    HILOGI("Hfp_Ag_Service_UpdateVirtualDevice_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsAcceptCellularCall_00300
 * @tc.name: IsAcceptCellularCall
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsAcceptCellularCall_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsAcceptCellularCall_00300 start");
    std::string address = "00:00:00:00:00:01";
    hfpAgService_->activeAddr_ = address;
    hfpAgService_->stateMachines_.insert_or_assign(address, std::make_unique<HfpAgStateMachine>(address));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTED));
    int32_t res = hfpAgService_->IsAcceptCellularCall();
    EXPECT_EQ(res, Bluetooth::BT_ERR_SCO_HAS_BEEN_CONNECTED);
    HILOGI("Hfp_Ag_Service_IsAcceptCellularCall_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DisconnectCsCallSco_00300
 * @tc.name: DisconnectCsCallSco
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DisconnectCsCallSco_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DisconnectCsCallSco_00300 start");
    std::string address = "00:00:00:00:00:01";
    hfpAgService_->stateMachines_.insert_or_assign(address, std::make_unique<HfpAgStateMachine>(address));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_DISCONNECTED));
    bool res = hfpAgService_->DisconnectCsCallSco(address);
    EXPECT_TRUE(res);
    HILOGI("Hfp_Ag_Service_DisconnectCsCallSco_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsVoiceRecognitionSupported_00100
 * @tc.name: IsVoiceRecognitionSupported
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsVoiceRecognitionSupported_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionSupported_00100 start");
    std::string address = "00:00:00:00:00:01";
    BluetoothRawAddress device(address);
    bool isSupported = false;
    bool res = hfpAgService_->IsVoiceRecognitionSupported(device, isSupported);
    EXPECT_FALSE(res);
    HILOGI("Hfp_Ag_Service_IsVoiceRecognitionSupported_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_UpdateActiveDevice_0100
 * @tc.name: UpdateActiveDevice
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_UpdateActiveDevice_0100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_UpdateActiveDevice_0100 start");
    HfpAgSystemInterface::GetInstance().callState_ = HFP_AG_CALL_STATE_INCOMING;
    std::string address1 = "B4:EC:02:11:22:33";
    RawAddress device1(address1);
    hfpAgService_->UpdateActiveDevice(address1);
    EXPECT_EQ(HfpAgSystemInterface::GetInstance().callState_, HFP_AG_CALL_STATE_INCOMING);
    bool res1 = BluetoothHwInterface::GetInstance()->InteropMatch(INTEROP_DISABLE_INBAND_RING, device1);
    EXPECT_TRUE(res1);
    HILOGI("Hfp_Ag_Service_UpdateActiveDevice_0100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_WakeUpVoiceRecognition_00100
 * @tc.name: WakeUpVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_WakeUpVoiceRecognition_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_WakeUpVoiceRecognition_00100 start");
    std::string address = "00:00:00:00:00:01";
    int32_t res = hfpAgService_->WakeUpVoiceRecognition(address);
    EXPECT_NE(res, BT_ERR_INTERNAL_ERROR);
    HILOGI("Hfp_Ag_Service_WakeUpVoiceRecognition_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_UpdateVirtualDeviceByHfpConnectState_00100
 * @tc.name: UpdateVirtualDeviceByHfpConnectState
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_UpdateVirtualDeviceByHfpConnectState_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_UpdateVirtualDeviceByHfpConnectState_00100 start");
    BluetoothRawAddress device(address);
    hfpAgService_->UpdateVirtualDeviceByHfpConnectState(device,
        static_cast<int>(BTConnectState::DISCONNECTED));
    hfpAgService_->UpdateVirtualDeviceByHfpConnectState(device,
        static_cast<int>(BTConnectState::CONNECTED));
    int state = 0; 
    bool res = hfpAgService_->GetRemoteDeviceConnectStatus(device, state);
    EXPECT_EQ(res, true);
    HILOGI("Hfp_Ag_Service_UpdateVirtualDeviceByHfpConnectState_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_IsInbandRingingEnabled_00100
 * @tc.name: IsInbandRingingEnabled
 * @tc.desc:
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_IsInbandRingingEnabled_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_IsInbandRingingEnabled_00100 start");
    hfpAgService_->activeAddr_ = "C0:2A:46:00:00:00";
    bool res = hfpAgService_->IsInbandRingingEnabled();
    EXPECT_FALSE(res);
    hfpAgService_->activeAddr_ = "00:00:00:00:00:00";
    res = hfpAgService_->IsInbandRingingEnabled();
    EXPECT_FALSE(res);
    HILOGI("Hfp_Ag_Service_IsInbandRingingEnabled_00100 end");
}
/*
 * @tc.number: Hfp_Ag_Service_DialCallCallback_00100
 * @tc.name: DialCallCallback
 * @tc.desc: test DialCallCallback with nullptr number
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DialCallCallback_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DialCallCallback_00100 start");

    EXPECT_NE(hfpAgService_->hfpAgServiceCallback_, nullptr);
    char* number = nullptr;
    STACK::RawAddress device;
    hfpAgService_->hfpAgServiceCallback_->DialCallCallback(number, &device);

    HILOGI("Hfp_Ag_Service_DialCallCallback_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Service_DialCallCallback_00200
 * @tc.name: DialCallCallback
 * @tc.desc: test DialCallCallback with valid number
 */
HWTEST_F(HfpAgServiceTest, Hfp_Ag_Service_DialCallCallback_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Service_DialCallCallback_00200 start");

    EXPECT_NE(hfpAgService_->hfpAgServiceCallback_, nullptr);
    STACK::RawAddress device;
    char number[] = "1234567890";
    hfpAgService_->hfpAgServiceCallback_->DialCallCallback(number, &device);

    HILOGI("Hfp_Ag_Service_DialCallCallback_00200 end");
}

}  // namespace bluetooth
}  // namespace OHOS