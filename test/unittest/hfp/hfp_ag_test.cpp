/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hfp_ag.h"
#include "bluetooth_hfp_ag_observer_stub.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "i_bluetooth_hfp_ag.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"
#include "raw_address.h"
#include "bluetooth_profile_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;

class HandsFreeAudioGatewayObserverCommon : public HandsFreeAudioGatewayObserver {
public:
    HandsFreeAudioGatewayObserverCommon() = default;
    virtual ~HandsFreeAudioGatewayObserverCommon() = default;

    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause) {}
    void OnScoStateChanged(const BluetoothRemoteDevice &device, int state, int reason) {}
    void OnActiveDeviceChanged(const BluetoothRemoteDevice &device) {}
    void OnHfEnhancedDriverSafetyChanged(const BluetoothRemoteDevice &device, int indValue) {}
    void OnVirtualDeviceChanged(int32_t action, std::string address) {}
private:
};

static std::shared_ptr<HandsFreeAudioGatewayObserverCommon> observer_ =
    std::make_shared<HandsFreeAudioGatewayObserverCommon>();
static HandsFreeAudioGateway *profile_;
static BluetoothHost *host_;

class HandsFreeAudioGatewayTest : public testing::Test {
public:
    HandsFreeAudioGatewayTest()
    {}
    ~HandsFreeAudioGatewayTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetRandomAddr(BluetoothRemoteDevice &device);
    sptr<IBluetoothHfpAg> proxy_  = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
};


void HandsFreeAudioGatewayTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBrEnable = %{public}d", isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        sleep(TIME);
    }

    profile_ = HandsFreeAudioGateway::GetProfile();

    HILOGI("SetUpTestCase end");
}

void HandsFreeAudioGatewayTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void HandsFreeAudioGatewayTest::SetUp()
{
}

void HandsFreeAudioGatewayTest::TearDown()
{
}

class BluetoothHfpAgObserverStubTest : public BluetoothHfpAgObserverStub {
public:
    void OnConnectionStateChanged(
        const BluetoothRawAddress &device, int state, int cause) override
    {}
    void OnScoStateChanged(const BluetoothRawAddress &device, int state, int reason) override
    {}
    void OnActiveDeviceChanged(const BluetoothRawAddress &device) override
    {}
    void OnHfEnhancedDriverSafetyChanged(const BluetoothRawAddress &device, int indValue) override
    {}
    void OnHfpStackChanged(const BluetoothRawAddress &device, int action) override
    {}
    void OnVirtualDeviceChanged(int32_t action, std::string address) override
    {}

    explicit BluetoothHfpAgObserverStubTest()
    {}

    ~BluetoothHfpAgObserverStubTest() override
    {}
private:
};

void HandsFreeAudioGatewayTest::SetRandomAddr(BluetoothRemoteDevice &device)
{
    string randomAddr;
    host_->GetRandomAddress(device.GetDeviceAddr(), randomAddr);
    device.address_ = randomAddr;
}

/*
 * @tc.number: HandsFreeAudioGateway001
 * @tc.name: GetProfile
 * @tc.desc: Get the instance of HandsFreeAudioGateway object
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetProfile, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetProfile start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    EXPECT_NE(nullptr, profile_);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetProfile end");
}

/*
 * @tc.number: HandsFreeAudioGateway002
 * @tc.name: GetConnectedDevices
 * @tc.desc: Get remote HF device list which are in the connected state.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetConnectedDevices, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetConnectedDevices start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    std::vector<BluetoothRemoteDevice> devices;
    profile_->GetConnectedDevices(devices);
    EXPECT_NE(nullptr, &devices);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetConnectedDevices end");
}

/*
 * @tc.number: HandsFreeAudioGateway003
 * @tc.name: GetDevicesByStates
 * @tc.desc: Get remote HF device list which are in the specified states.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetDevicesByStates, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDevicesByStates start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRemoteDevice> devices;
    devices = profile_->GetDevicesByStates(states);
    EXPECT_NE(nullptr, &devices);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDevicesByStates end");
}

/*
 * @tc.number: HandsFreeAudioGateway004
 * @tc.name: GetDeviceState
 * @tc.desc: Get the connection state of the specified remote HF device.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetDeviceState, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDeviceState start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);

    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    EXPECT_EQ(profile_->GetDeviceState(device, state), BT_NO_ERROR);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDeviceState end");
}

/*
 * @tc.number: HandsFreeAudioGateway005
 * @tc.name: Connect
 * @tc.desc: Initiate the establishment of a service level connection to remote HF device.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_Connect start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->Connect(device), BT_ERR_PROFILE_DISABLED);
    HILOGI("HandsFreeAudioGateway_UnitTest_Connect end");
}

/*
 * @tc.number: HandsFreeAudioGateway006
 * @tc.name: Disconnect
 * @tc.desc: Release the audio connection from remote HF device.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_Disconnect, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_ start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_DEVICE_DISCONNECTED);
    HILOGI("HandsFreeAudioGateway_UnitTest_Disconnect end");
}

/*
 * @tc.number: HandsFreeAudioGateway007
 * @tc.name: GetScoState
 * @tc.desc: Get the Audio connection state of the specified remote HF device.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetScoState, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_ start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->GetScoState(device), 3);
    HILOGI("HandsFreeAudioGateway_UnitTest_ end");
}

/*
 * @tc.number: HandsFreeAudioGateway008
 * @tc.name: ConnectSco
 * @tc.desc: Initiate the establishment of an audio connection to remote active HF device.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_ConnectSco, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_ConnectSco start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    EXPECT_TRUE(profile_->ConnectSco());
    HILOGI("HandsFreeAudioGateway_UnitTest_ConnectSco end");
}

/*
 * @tc.number: HandsFreeAudioGateway009
 * @tc.name: DisconnectSco
 * @tc.desc: Release the audio connection from remote active HF device.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_DisconnectSco, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_DisconnectSco start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    EXPECT_TRUE(profile_->DisconnectSco());

    HILOGI("HandsFreeAudioGateway_UnitTest_DisconnectSco end");
}

/*
 * @tc.number: HandsFreeAudioGateway010
 * @tc.name: PhoneStateChanged
 * @tc.desc: This function used to Update changed phone call informations.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_PhoneStateChanged, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_PhoneStateChanged start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    profile_->PhoneStateChanged(0, 0, 0, "123", 0, "");
    EXPECT_NE(nullptr, profile_);
    HILOGI("HandsFreeAudioGateway_UnitTest_PhoneStateChanged end");
}

/*
 * @tc.number: HandsFreeAudioGateway011
 * @tc.name: ClccResponse
 * @tc.desc: Send response for querying standard list current calls by remote Hf device.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_ClccResponse, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_ClccResponse start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    profile_->ClccResponse(0, 0, 0, 0, false, "123", 0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("HandsFreeAudioGateway_UnitTest_ClccResponse end");
}

/*
 * @tc.number: HandsFreeAudioGateway012
 * @tc.name: OpenVoiceRecognition
 * @tc.desc: Open the voice recognition.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_OpenVoiceRecognition, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_OpenVoiceRecognition start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->OpenVoiceRecognition(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_OpenVoiceRecognition end");
}

/*
 * @tc.number: HandsFreeAudioGateway013
 * @tc.name: CloseVoiceRecognition
 * @tc.desc: Close the voice recognition.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_CloseVoiceRecognition, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_CloseVoiceRecognition start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->CloseVoiceRecognition(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_CloseVoiceRecognition end");
}

/*
 * @tc.number: HandsFreeAudioGateway014
 * @tc.name: SetActiveDevice
 * @tc.desc: Set the active device for audio connection.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_SetActiveDevice_002, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_SetActiveDevice_002 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    std::string address = "18:AA:0F:B5:62:98";
    BluetoothRemoteDevice device(address, BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->SetActiveDevice(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_SetActiveDevice_002 end");
}

/*
 * @tc.number: HandsFreeAudioGateway015
 * @tc.name: GetActiveDevice
 * @tc.desc: Set the active device for audio connection.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetActiveDevice, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetActiveDevice start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device = profile_->GetActiveDevice();
    EXPECT_NE(nullptr, &device);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetActiveDevice end");
}

/*
 * @tc.number: HandsFreeAudioGateway016
 * @tc.name: RegisterObserver
 * @tc.desc: Register HandsFree AudioGateway observer instance.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_RegisterObserver, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_RegisterObserver start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    profile_->RegisterObserver(observer_);
    EXPECT_NE(nullptr, profile_);
    profile_->DeregisterObserver(observer_);
    HILOGI("HandsFreeAudioGateway_UnitTest_RegisterObserver end");
}

/*
 * @tc.number: HandsFreeAudioGateway017
 * @tc.name: HandsFreeAudioGateway_UnitTest_IsInbandRingingEnabled_001.
 * @tc.desc: Check whether in-band-ringing is enabled.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_IsInbandRingingEnabled_001, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_IsInbandRingingEnabled_001 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    bool isEnabled = false;
    profile_->IsInbandRingingEnabled(isEnabled);
    EXPECT_TRUE(isEnabled);
    HILOGI("HandsFreeAudioGateway_UnitTest_IsInbandRingingEnabled end");
}

/*
 * @tc.number: HandsFreeAudioGateway018
 * @tc.name: GetDeviceState
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetDeviceState_002, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDeviceState_002 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    EXPECT_EQ(profile_->GetDeviceState(device, state), BT_ERR_IPC_TRANS_FAILED);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDeviceState_002 end");
}

/*
 * @tc.number: HandsFreeAudioGateway019
 * @tc.name: Connect
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_Connect_002, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_Connect_002 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_EQ(profile_->Connect(device), BT_ERR_IPC_TRANS_FAILED);
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_IPC_TRANS_FAILED);
    HILOGI("HandsFreeAudioGateway_UnitTest_Connect_002 end");
}

/*
 * @tc.number: HandsFreeAudioGateway020
 * @tc.name: GetScoState
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetScoState_002, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetScoState_002 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_EQ(profile_->GetScoState(device), BT_ERR_IPC_TRANS_FAILED);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetScoState_002 end");
}

/*
 * @tc.number: HandsFreeAudioGateway021
 * @tc.name: OpenVoiceRecognition
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_OpenVoiceRecognition_002, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_OpenVoiceRecognition_002 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_FALSE(profile_->OpenVoiceRecognition(device));
    EXPECT_FALSE(profile_->CloseVoiceRecognition(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_OpenVoiceRecognition_002 end");
}

/*
 * @tc.number: HandsFreeAudioGateway022
 * @tc.name: SetActiveDevice
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_SetActiveDevice_001, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_SetActiveDevice_001 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice emptyDevice("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_TRUE(profile_->SetActiveDevice(emptyDevice));
    BluetoothRemoteDevice nullDevice("", INVALID_TYPE);
    EXPECT_TRUE(profile_->SetActiveDevice(nullDevice));
    HILOGI("HandsFreeAudioGateway_UnitTest_SetActiveDevice_001 end");
}

/*
 * @tc.number: HandsFreeAudioGateway023
 * @tc.name: GetConnectStrategy
 * @tc.desc: Get connect strategy by address
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetConnectStrategy_001 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();

    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    HILOGI("HandsFreeAudioGateway_UnitTest_GetConnectStrategy_001 end");
}

/*
 * @tc.number: HandsFreeAudioGateway024
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_SetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_SetConnectStrategy_001 start");

    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->SetConnectStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN)), BT_ERR_INVALID_PARAM);
    HILOGI("HandsFreeAudioGateway_UnitTest_SetConnectStrategy_001 end");
}

/*
 * @tc.number: HandsFreeAudioGateway025
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_SetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_SetConnectStrategy_002 start");

    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);

    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    HILOGI("HandsFreeAudioGateway_UnitTest_SetConnectStrategy_002 end");
}

/*
 * @tc.number: HandsFreeAudioGateway026
 * @tc.name: ConnectProxy
 * @tc.desc: ConnectProxy HandsFree AudioGateway observer instance.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_ConnectProxy, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_ConnectProxy start");

    BluetoothRawAddress addr("11:22:33:44:55:66");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_EQ(proxy_->Connect(addr), BT_ERR_INTERNAL_ERROR);
    }
    BluetoothHfpAgObserverStubTest bluetoothHfpAgObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    bluetoothHfpAgObserverStubTest.OnConnectionStateChangedInner(data, reply);
    bluetoothHfpAgObserverStubTest.OnScoStateChangedInner(data, reply);
    bluetoothHfpAgObserverStubTest.OnActiveDeviceChangedInner(data, reply);
    bluetoothHfpAgObserverStubTest.OnHfEnhancedDriverSafetyChangedInner(data, reply);
    HILOGI("HandsFreeAudioGateway_UnitTest_ConnectProxy end");
}

/*
 * @tc.number: HandsFreeAudioGateway027
 * @tc.name: IntoMockProxy
 * @tc.desc: IntoMockProxy HandsFree AudioGateway observer instance.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_IntoMockProxy, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_IntoMockProxy start");

    BluetoothRawAddress addr("11:22:33:44:55:66");
    int32_t state = 0;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_FALSE(proxy_->IntoMock(addr, state));
    }
    HILOGI("HandsFreeAudioGateway_UnitTest_IntoMockProxy end");
}

/*
 * @tc.number: HandsFreeAudioGateway028
 * @tc.name: SendNoCarrierProxy
 * @tc.desc: SendNoCarrierProxy HandsFree AudioGateway observer instance.
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_SendNoCarrierProxy, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_SendNoCarrierProxy start");

    BluetoothRawAddress addr("11:22:33:44:55:66");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_FALSE(proxy_->SendNoCarrier(addr));
    }
    HILOGI("HandsFreeAudioGateway_UnitTest_SendNoCarrierProxy end");
}

/*
 * @tc.number: HandsFreeAudioGateway029
 * @tc.name: HandsFreeAudioGateway_UnitTest_ConnectSco_01
 * @tc.desc: ConnectSco, no active device
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_ConnectSco_01, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_ConnectSco_01 start");

    profile_ = HandsFreeAudioGateway::GetProfile();
    uint8_t callType = static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL);
    EXPECT_TRUE(profile_->ConnectSco(callType));
    EXPECT_TRUE(profile_->DisconnectSco(callType));
    HILOGI("HandsFreeAudioGateway_UnitTest_ConnectSco_01 end");
}

/*
 * @tc.number: HandsFreeAudioGateway030
 * @tc.name: HandsFreeAudioGateway_UnitTest_CallDetailsChanged_01
 * @tc.desc:
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_CallDetailsChanged_01, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_CallDetailsChanged_01 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    int callId = 1;
    int callState = 1;
    profile_->CallDetailsChanged(callId, callState);
    EXPECT_NE(nullptr, profile_);
    HILOGI("HandsFreeAudioGateway_UnitTest_CallDetailsChanged_01 end");
}

/*
 * @tc.number: HandsFreeAudioGateway031
 * @tc.name: HandsFreeAudioGateway_UnitTest_IsVgsSupported_01
 * @tc.desc:
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_IsVgsSupported_01, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_IsVgsSupported_01 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    bool isSupported = false;
    EXPECT_EQ(profile_->IsVgsSupported(device, isSupported), BT_NO_ERROR);
    HILOGI("HandsFreeAudioGateway_UnitTest_IsVgsSupported_01 end");
}

/*
 * @tc.number: HandsFreeAudioGateway032
 * @tc.name: HandsFreeAudioGateway_UnitTest_EnableBtCallLog_01
 * @tc.desc:
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_EnableBtCallLog_01, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_EnableBtCallLog_01 start");
    profile_ = HandsFreeAudioGateway::GetProfile();
    bool disable = false;
    profile_->EnableBtCallLog(disable);

    bool enable = true;
    profile_->EnableBtCallLog(enable);
    EXPECT_NE(nullptr, profile_);
    HILOGI("HandsFreeAudioGateway_UnitTest_EnableBtCallLog_01 end");
}

/*
 * @tc.number: HandsFreeAudioGateway033
 * @tc.name: HandsFreeAudioGateway_UnitTest_GetVirtualDeviceList_01
 * @tc.desc:
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_GetVirtualDeviceList_01, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetVirtualDeviceList_01 start");
    std::vector<std::string> devices;
    profile_->GetVirtualDeviceList(devices);
    EXPECT_NE(nullptr, &devices);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetVirtualDeviceList_01 end");
}

/*
 * @tc.number: HandsFreeAudioGateway034
 * @tc.name: DisableBt
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(HandsFreeAudioGatewayTest, HandsFreeAudioGateway_UnitTest_DisableBt_001, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_DisableBt_001 start");
    int btState = host_->GetBtState();
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        sleep(TIME);
    }
    profile_ = HandsFreeAudioGateway::GetProfile();
    std::vector<BluetoothRemoteDevice> devices;
    EXPECT_EQ(profile_->GetConnectedDevices(devices), BT_ERR_INVALID_STATE);
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    EXPECT_EQ(profile_->GetDeviceState(device, state), BT_ERR_INVALID_STATE);
    EXPECT_EQ(profile_->Connect(device), BT_ERR_INVALID_STATE);
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_INVALID_STATE);
    EXPECT_EQ(profile_->GetScoState(device), static_cast<int>(HfpScoConnectState::SCO_DISCONNECTED));
    EXPECT_TRUE(profile_->ConnectSco());
    EXPECT_TRUE(profile_->DisconnectSco());
    profile_->ClccResponse(0, 0, 0, 0, false, "123", 0);
    EXPECT_FALSE(profile_->OpenVoiceRecognition(device));
    EXPECT_FALSE(profile_->CloseVoiceRecognition(device));
    EXPECT_FALSE(profile_->SetActiveDevice(device));
    profile_->GetActiveDevice();
    bool isEnabled = false;
    EXPECT_EQ(profile_->IsInbandRingingEnabled(isEnabled), BT_ERR_INVALID_STATE);
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_ERR_INVALID_STATE);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN));
    host_->EnableBle();
    sleep(TIME);
    HILOGI("HandsFreeAudioGateway_UnitTest_DisableBt_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS