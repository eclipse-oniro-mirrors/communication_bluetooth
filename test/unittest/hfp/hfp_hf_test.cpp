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
#include "bluetooth_hfp_hf.h"
#include "bluetooth_hfp_hf_call.h"
#include "bluetooth_hfp_hf_observer_stub.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "i_bluetooth_hfp_hf.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_profile_manager.h"
#include "hands_free_unit_calls.h"
#include "bluetooth_access_token_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace std;

constexpr int TIME = 2;

class HandsFreeUnitObserverCommon : public HandsFreeUnitObserver {
public:
    HandsFreeUnitObserverCommon() = default;
    virtual ~HandsFreeUnitObserverCommon() = default;

    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause) {}
    virtual void OnScoStateChanged(const BluetoothRemoteDevice &device, int state) {}
    virtual void OnCallChanged(const BluetoothRemoteDevice &device, const HandsFreeUnitCall &call) {}
    virtual void OnSignalStrengthChanged(const BluetoothRemoteDevice &device, int signal) {}
    virtual void OnRegistrationStatusChanged(const BluetoothRemoteDevice &device, int status) {}
    virtual void OnRoamingStatusChanged(const BluetoothRemoteDevice &device, int status) {}
    virtual void OnOperatorSelectionChanged(const BluetoothRemoteDevice &device, const std::string &name) {}
    virtual void OnSubscriberNumberChanged(const BluetoothRemoteDevice &device, const std::string &number) {}
    virtual void OnVoiceRecognitionStatusChanged(const BluetoothRemoteDevice &device, int status) {}
    virtual void OnInBandRingToneChanged(const BluetoothRemoteDevice &device, int status) {}

private:
};
static std::shared_ptr<HandsFreeUnitObserverCommon> observer_ =
    std::make_shared<HandsFreeUnitObserverCommon>();
static HandsFreeUnit *profile_;
static BluetoothHost *host_;

class BluetoothHfpHfObserverStubTest : public BluetoothHfpHfObserverStub {
public:
    void OnConnectionStateChanged(
        const BluetoothRawAddress &device, int state, int cause) override
    {}
    void OnScoStateChanged(const BluetoothRawAddress &device,  int state) override
    {}
    void OnCallChanged(const BluetoothRawAddress &device, const BluetoothHfpHfCall &call) override
    {}
    void OnSignalStrengthChanged(const BluetoothRawAddress &device,  int signal) override
    {}
    void OnRegistrationStatusChanged(const BluetoothRawAddress &device,  int status) override
    {}
    void OnRoamingStatusChanged(const BluetoothRawAddress &device,  int status) override
    {}
    void OnOperatorSelectionChanged(const BluetoothRawAddress &device, const std::string &name) override
    {}
    void OnSubscriberNumberChanged(const BluetoothRawAddress &device, const std::string &number) override
    {}
    void OnVoiceRecognitionStatusChanged(const BluetoothRawAddress &device, int status) override
    {}
    void OnInBandRingToneChanged(const BluetoothRawAddress &device, int status) override
    {}

    explicit BluetoothHfpHfObserverStubTest()
    {}

    ~BluetoothHfpHfObserverStubTest() override
    {}
private:
};

class HandsFreeUnitTest : public testing::Test {
public:
    HandsFreeUnitTest()
    {}
    ~HandsFreeUnitTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<IBluetoothHfpHf> proxy_  = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
};


void HandsFreeUnitTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();

    host_ = &BluetoothHost::GetDefaultHost();
    profile_ = HandsFreeUnit::GetProfile();

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

void HandsFreeUnitTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void HandsFreeUnitTest::SetUp()
{
}

void HandsFreeUnitTest::TearDown(void)
{
}

/*
 * @tc.number: HandsFreeUnit001
 * @tc.name: GetProfile
 * @tc.desc: Get the instance of HandsFreeAudioGateway object.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetProfile, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetProfile start");

    profile_ = HandsFreeUnit::GetProfile();
    EXPECT_NE(nullptr, profile_);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetProfile end");
}

/*
 * @tc.number: HandsFreeUnit002
 * @tc.name: ConnectSco
 * @tc.desc:  Initiate the establishment of an audio connection to remote AG device.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_ConnectSco, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_ConnectSco start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->ConnectSco(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_ConnectSco end");
}

/*
 * @tc.number: HandsFreeUnit003
 * @tc.name: DisconnectSco
 * @tc.desc: Release the audio connection from remote HF device.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_DisconnectSco, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_DisconnectSco start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->DisconnectSco(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_DisconnectSco end");
}

/*
 * @tc.number: HandsFreeUnit004
 * @tc.name: GetDevicesByStates
 * @tc.desc: Get remote AG device list which are in the specified states.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetDevicesByStates, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDevicesByStates start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    vector<BluetoothRemoteDevice> devices = profile_->GetDevicesByStates(states);
    EXPECT_NE(nullptr, &devices);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDevicesByStates end");
}

/*
 * @tc.number: HandsFreeUnit005
 * @tc.name: GetDeviceState
 * @tc.desc: Get the connection state of the specified remote AG device.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetDeviceState, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetDeviceState start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->GetDeviceState(device), static_cast<int>(BTConnectState::DISCONNECTED));

    HILOGI("HandsFreeAudioGateway_UnitTest_GetDeviceState end");
}

/*
 * @tc.number: HandsFreeUnit006
 * @tc.name: GetScoState
 * @tc.desc: Get the Audio connection state of the specified remote AG device.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetScoState, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetScoState start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->GetScoState(device), 3);

    HILOGI("HandsFreeAudioGateway_UnitTest_GetScoState end");
}

/*
 * @tc.number: HandsFreeUnit007
 * @tc.name: SendDTMFTone
 * @tc.desc: Send DTMF tone code to remote AG device.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_SendDTMFTone, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_SendDTMFTone start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->SendDTMFTone(device, 0));
    HILOGI("HandsFreeAudioGateway_UnitTest_SendDTMFTone end");
}


/*
 * @tc.number: HandsFreeUnit008
 * @tc.name: Connect
 * @tc.desc: Initiate the establishment of a service level connection to remote AG device.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_Connect start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_TRUE(profile_->Connect(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_Connect end");
}

/*
 * @tc.number: HandsFreeUnit009
 * @tc.name: Disconnect
 * @tc.desc: Release the audio connection from remote AG device.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_Disconnect, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_Disconnect start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_TRUE(profile_->Disconnect(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_Disconnect end");
}


/*
 * @tc.number: HandsFreeUnit010
 * @tc.name: OpenVoiceRecognition
 * @tc.desc: Open voice recognition.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OpenVoiceRecognition, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_OpenVoiceRecognition start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->OpenVoiceRecognition(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_OpenVoiceRecognition end");
}

/*
 * @tc.number: HandsFreeUnit011
 * @tc.name: CloseVoiceRecognition
 * @tc.desc: Close voice recognition.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_CloseVoiceRecognition, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_CloseVoiceRecognition start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->CloseVoiceRecognition(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_CloseVoiceRecognition end");
}

/*
 * @tc.number: HandsFreeUnit012
 * @tc.name: GetExistingCalls
 * @tc.desc: Get a list of all existing calls.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetExistingCalls, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_GetExistingCalls start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    vector<HandsFreeUnitCall> calls = profile_->GetExistingCalls(device);
    EXPECT_NE(nullptr, &calls);
    HILOGI("HandsFreeAudioGateway_UnitTest_GetExistingCalls end");
}

/*
 * @tc.number: HandsFreeUnit013
 * @tc.name: AcceptIncomingCall
 * @tc.desc: Accept an incoming call.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_AcceptIncomingCall, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_AcceptIncomingCall start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->AcceptIncomingCall(device, 0));
    HILOGI("HandsFreeAudioGateway_UnitTest_AcceptIncomingCall end");
}

/*
 * @tc.number: HandsFreeUnit014
 * @tc.name: HoldActiveCall
 * @tc.desc: Hold an active call.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_HoldActiveCall, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_HoldActiveCall start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->HoldActiveCall(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_HoldActiveCall end");
}

/*
 * @tc.number: HandsFreeUnit015
 * @tc.name: RejectIncomingCall
 * @tc.desc: Reject an incoming call.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_RejectIncomingCall, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_RejectIncomingCall start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(profile_->RejectIncomingCall(device));
    HILOGI("HandsFreeAudioGateway_UnitTest_RejectIncomingCall end");
}

/*
 * @tc.number: HandsFreeUnit016
 * @tc.name: FinishActiveCall
 * @tc.desc: Start dial with specified call number.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_FinishActiveCall, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_FinishActiveCall start");

    profile_ = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    HandsFreeUnitCall call;
    EXPECT_FALSE(profile_->FinishActiveCall(device, call));
    HILOGI("HandsFreeAudioGateway_UnitTest_FinishActiveCall end");
}

/*
 * @tc.number: HandsFreeUnit017
 * @tc.name: StartDial
 * @tc.desc: Start dial with specified call number.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_StartDial, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_StartDial start");

    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    std::string number = "000";
    profile_ = HandsFreeUnit::GetProfile();
    std::optional<HandsFreeUnitCall> call = profile_->StartDial(device, number);
    EXPECT_NE(nullptr, &call);
    HILOGI("HandsFreeAudioGateway_UnitTest_StartDial end");
}

/*
 * @tc.number: HandsFreeUnit018
 * @tc.name: RegisterObserver
 * @tc.desc: Register HandsFree Unit observer instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_RegisterObserver, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_RegisterObserver start");

    profile_ = HandsFreeUnit::GetProfile();
    profile_->RegisterObserver(observer_);
    EXPECT_NE(nullptr, profile_);
    HILOGI("HandsFreeAudioGateway_UnitTest_RegisterObserver end");
}

/*
 * @tc.number: HandsFreeUnit019
 * @tc.name: DeregisterObserver
 * @tc.desc: Deregister HandsFree Unit observer instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_DeregisterObserver, TestSize.Level1)
{
    HILOGI("HandsFreeAudioGateway_UnitTest_DeregisterObserver start");

    profile_ = HandsFreeUnit::GetProfile();
    profile_->DeregisterObserver(observer_);
    EXPECT_NE(nullptr, profile_);
    HILOGI("HandsFreeAudioGateway_UnitTest_DeregisterObserver end");
}

/*
 * @tc.number: HandsFreeUnit020
 * @tc.name: OnRemoteRequest
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnRemoteRequestInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnRemoteRequestInner start");
    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(bluetoothHfpHfObserverStubTest.OnRemoteRequest(code, data, reply, option), 10);
    HILOGI("HandsFreeUnit_UnitTest_OnRemoteRequestInner end");
}

/*
 * @tc.number: HandsFreeUnit021
 * @tc.name: OnConnectionStateChangedInner
 * @tc.desc: OnConnectionStateChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnConnectionStateChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnConnectionStateChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnConnectionStateChangedInner(&bluetoothHfpHfObserverStubTest,
         data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnConnectionStateChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnConnectionStateChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit022
 * @tc.name: OnScoStateChangedInner
 * @tc.desc: OnScoStateChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnScoStateChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnScoStateChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnScoStateChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnScoStateChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnScoStateChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit023
 * @tc.name: OnCallChangedInner
 * @tc.desc: OnCallChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnCallChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnCallChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnCallChangedInner(&bluetoothHfpHfObserverStubTest, data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnCallChangedInner(&bluetoothHfpHfObserverStubTest, data, reply), 1);
    HILOGI("HandsFreeUnit_UnitTest_OnCallChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit024
 * @tc.name: OnSignalStrengthChangedInner
 * @tc.desc: OnSignalStrengthChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnSignalStrengthChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnSignalStrengthChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnSignalStrengthChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnSignalStrengthChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnSignalStrengthChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit025
 * @tc.name: OnRegistrationStatusChangedInner
 * @tc.desc: OnRegistrationStatusChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnRegistrationStatusChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnRegistrationStatusChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnRegistrationStatusChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnRegistrationStatusChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnRegistrationStatusChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit026
 * @tc.name: OnRoamingStatusChangedInner
 * @tc.desc: OnRoamingStatusChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnRoamingStatusChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnRoamingStatusChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnRoamingStatusChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnRoamingStatusChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnRoamingStatusChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit027
 * @tc.name: OnOperatorSelectionChangedInner
 * @tc.desc: OnOperatorSelectionChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnOperatorSelectionChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnOperatorSelectionChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnOperatorSelectionChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnOperatorSelectionChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnOperatorSelectionChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit028
 * @tc.name: OnSubscriberNumberChangedInner
 * @tc.desc: OnSubscriberNumberChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnSubscriberNumberChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnSubscriberNumberChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnSubscriberNumberChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnSubscriberNumberChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnSubscriberNumberChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit029
 * @tc.name: OnSubscriberNumberChangedInner
 * @tc.desc: OnSubscriberNumberChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnVoiceRecognitionStatusChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnVoiceRecognitionStatusChangedInner start");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnVoiceRecognitionStatusChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnVoiceRecognitionStatusChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnVoiceRecognitionStatusChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit030
 * @tc.name: OnInBandRingToneChangedInner
 * @tc.desc: OnInBandRingToneChangedInner HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OnInBandRingToneChangedInner, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OnInBandRingToneChangedInner start change");

    BluetoothHfpHfObserverStubTest bluetoothHfpHfObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnInBandRingToneChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), 1);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress("11:22:33:44:55:66");
    data.WriteParcelable(device);
    EXPECT_EQ(BluetoothHfpHfObserverStub::OnInBandRingToneChangedInner(&bluetoothHfpHfObserverStubTest,
        data, reply), NO_ERROR);
    HILOGI("HandsFreeUnit_UnitTest_OnInBandRingToneChangedInner end");
}

/*
 * @tc.number: HandsFreeUnit31
 * @tc.name: ConnectScoProxy
 * @tc.desc: ConnectScoProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_ConnectScoProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_ConnectScoProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_FALSE(proxy_->ConnectSco(addr));
    }
    HILOGI("HandsFreeUnit_UnitTest_ConnectScoProxy end");
}

/*
 * @tc.number: HandsFreeUnit32
 * @tc.name: DisconnectScoProxy
 * @tc.desc: DisconnectScoProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_DisconnectScoProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_DisconnectScoProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_FALSE(proxy_->DisconnectSco(addr));
    }
    HILOGI("HandsFreeUnit_UnitTest_DisconnectScoProxy end");
}

/*
 * @tc.number: HandsFreeUnit33
 * @tc.name: GetDevicesByStatesProxy
 * @tc.desc: GetDevicesByStatesProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetDevicesByStatesProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_GetDevicesByStatesProxy start");

    std::vector<BluetoothRawAddress> rawDevices;
    std::vector<int> states;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_EQ(proxy_->GetDevicesByStates(states, rawDevices), BT_NO_ERROR);
    }
    HILOGI("HandsFreeUnit_UnitTest_GetDevicesByStatesProxy end");
}

/*
 * @tc.number: HandsFreeUnit34
 * @tc.name: GetDeviceStateProxy
 * @tc.desc: GetDeviceStateProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetDeviceStateProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_GetDeviceStateProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_EQ(proxy_->GetDeviceState(addr), BT_ERR_IPC_TRANS_FAILED);
    }
    HILOGI("HandsFreeUnit_UnitTest_GetDeviceStateProxy end");
}

/*
 * @tc.number: HandsFreeUnit35
 * @tc.name: GetScoStateProxy
 * @tc.desc: GetScoStateProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetScoStateProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_GetScoStateProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_EQ(proxy_->GetScoState(addr), BT_ERR_IPC_TRANS_FAILED);
    }
    HILOGI("HandsFreeUnit_UnitTest_GetScoStateProxy end");
}

/*
 * @tc.number: HandsFreeUnit36
 * @tc.name: SendDTMFToneProxy
 * @tc.desc: SendDTMFToneProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_SendDTMFToneProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_SendDTMFToneProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    uint8_t code = 0;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_FALSE(proxy_->SendDTMFTone(addr, code));
    }
    HILOGI("HandsFreeUnit_UnitTest_SendDTMFToneProxy end");
}

/*
 * @tc.number: HandsFreeUnit37
 * @tc.name: ConnectProxy
 * @tc.desc: ConnectProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_ConnectProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_ConnectProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_EQ(proxy_->Connect(addr), BT_ERR_IPC_TRANSACTION_FAILED);
    }
    HILOGI("HandsFreeUnit_UnitTest_ConnectProxy end");
}

/*
 * @tc.number: HandsFreeUnit38
 * @tc.name: DisconnectProxy
 * @tc.desc: DisconnectProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_DisconnectProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_DisconnectProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_EQ(proxy_->Disconnect(addr), BT_ERR_IPC_TRANSACTION_FAILED);
    }
    HILOGI("HandsFreeUnit_UnitTest_DisconnectProxy end");
}

/*
 * @tc.number: HandsFreeUnit39
 * @tc.name: OpenVoiceRecognitionProxy
 * @tc.desc: OpenVoiceRecognitionProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_OpenVoiceRecognitionProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_OpenVoiceRecognitionProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_FALSE(proxy_->OpenVoiceRecognition(addr));
    }
    HILOGI("HandsFreeUnit_UnitTest_OpenVoiceRecognitionProxy end");
}

/*
 * @tc.number: HandsFreeUnit40
 * @tc.name: CloseVoiceRecognitionProxy
 * @tc.desc: CloseVoiceRecognitionProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_CloseVoiceRecognitionProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_CloseVoiceRecognitionProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        EXPECT_FALSE(proxy_->CloseVoiceRecognition(addr));
    }
    HILOGI("HandsFreeUnit_UnitTest_CloseVoiceRecognitionProxy end");
}

/*
 * @tc.number: HandsFreeUnit41
 * @tc.name: GetCurrentCallListProxy
 * @tc.desc: GetCurrentCallListProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_GetCurrentCallListProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_GetCurrentCallListProxy start");
    std::vector<BluetoothHfpHfCall> callsList;
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->GetCurrentCallList(addr, callsList);
    }
    HILOGI("HandsFreeUnit_UnitTest_GetCurrentCallListProxy end");
}

/*
 * @tc.number: HandsFreeUnit42
 * @tc.name: AcceptIncomingCallProxy
 * @tc.desc: AcceptIncomingCallProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_AcceptIncomingCallProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_AcceptIncomingCallProxy start");
    int32_t flag = 0;
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->AcceptIncomingCall(addr, flag);
    }
    HILOGI("HandsFreeUnit_UnitTest_AcceptIncomingCallProxy end");
}

/*
 * @tc.number: HandsFreeUnit43
 * @tc.name: HoldActiveCallProxy
 * @tc.desc: HoldActiveCallProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_HoldActiveCallProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_HoldActiveCallProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->HoldActiveCall(addr);
    }
    HILOGI("HandsFreeUnit_UnitTest_HoldActiveCallProxy end");
}

/*
 * @tc.number: HandsFreeUnit43
 * @tc.name: RejectIncomingCallProxy
 * @tc.desc: RejectIncomingCallProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_RejectIncomingCallProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_RejectIncomingCallProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->RejectIncomingCall(addr);
    }
    HILOGI("HandsFreeUnit_UnitTest_RejectIncomingCallProxy end");
}

/*
 * @tc.number: HandsFreeUnit44
 * @tc.name: SendKeyPressedProxy
 * @tc.desc: SendKeyPressedProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_SendKeyPressedProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_SendKeyPressedProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->SendKeyPressed(addr);
    }
    HILOGI("HandsFreeUnit_UnitTest_SendKeyPressedProxy end");
}

/*
 * @tc.number: HandsFreeUnit45
 * @tc.name: HandleIncomingCallProxy
 * @tc.desc: HandleIncomingCallProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_HandleIncomingCallProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_HandleIncomingCallProxy start");
    int32_t flag = 0;
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->HandleIncomingCall(addr, flag);
    }
    HILOGI("HandsFreeUnit_UnitTest_HandleIncomingCallProxy end");
}

/*
 * @tc.number: HandsFreeUnit46
 * @tc.name: HandleMultiCallProxy
 * @tc.desc: HandleMultiCallProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_HandleMultiCallProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_HandleMultiCallProxy start");
    int32_t flag = 0;
    int32_t index = 0;
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->HandleMultiCall(addr, flag, index);
    }
    HILOGI("HandsFreeUnit_UnitTest_HandleMultiCallProxy end");
}

/*
 * @tc.number: HandsFreeUnit47
 * @tc.name: DialLastNumber
 * @tc.desc: DialLastNumber HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_DialLastNumberProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_DialLastNumberProxy start");
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->DialLastNumber(addr);
    }
    HILOGI("HandsFreeUnit_UnitTest_DialLastNumberProxy end");
}

/*
 * @tc.number: HandsFreeUnit48
 * @tc.name: DialMemoryProxy
 * @tc.desc: DialMemoryProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_DialMemoryProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_DialMemoryProxy start");
    int32_t index = 0;
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->DialMemory(addr, index);
    }
    HILOGI("HandsFreeUnit_UnitTest_DialMemoryProxy end");
}

/*
 * @tc.number: HandsFreeUnit49
 * @tc.name: SendVoiceTagProxy
 * @tc.desc: SendVoiceTagProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_SendVoiceTagProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_SendVoiceTagProxy start");
    int32_t index = 0;
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->SendVoiceTag(addr, index);
    }
    HILOGI("HandsFreeUnit_UnitTest_SendVoiceTagProxy end");
}

/*
 * @tc.number: HandsFreeUnit50
 * @tc.name: FinishActiveCallProxy
 * @tc.desc: FinishActiveCallProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_FinishActiveCallProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_FinishActiveCallProxy start");
    BluetoothHfpHfCall call;
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->FinishActiveCall(addr, call);
    }
    HILOGI("HandsFreeUnit_UnitTest_FinishActiveCallProxy end");
}

/*
 * @tc.number: HandsFreeUnit51
 * @tc.name: StartDialProxy
 * @tc.desc: StartDialProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_StartDialProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_StartDialProxy start");
    std::string number = "";
    BluetoothHfpHfCall call;
    BluetoothRawAddress addr("00:00:00:00:00:00");
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->StartDial(addr, number, call);
    }
    HILOGI("HandsFreeUnit_UnitTest_StartDialProxy end");
}

/*
 * @tc.number: HandsFreeUnit52
 * @tc.name: RegisterObserverProxy
 * @tc.desc: RegisterObserverProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_RegisterObserverProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_RegisterObserverProxy start");
    sptr<BluetoothHfpHfObserverStubTest> observer = new BluetoothHfpHfObserverStubTest();
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->RegisterObserver(observer);
    }
    HILOGI("HandsFreeUnit_UnitTest_RegisterObserverProxy end");
}

/*
 * @tc.number: HandsFreeUnit53
 * @tc.name: DeregisterObserverProxy
 * @tc.desc: DeregisterObserverProxy HandsFree Unit instance.
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnit_UnitTest_DeregisterObserverProxy, TestSize.Level1)
{
    HILOGI("HandsFreeUnit_UnitTest_DeregisterObserverProxy start");
    sptr<BluetoothHfpHfObserverStubTest> observer = new BluetoothHfpHfObserverStubTest();
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        proxy_->DeregisterObserver(observer);
    }
    HILOGI("HandsFreeUnit_UnitTest_DeregisterObserverProxy end");
}

/*
 * @tc.number: HandsFreeUnitCalls01
 * @tc.name: GetRemoteDevice
 * @tc.desc: GetRemoteDevice
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_GetRemoteDevice, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_GetRemoteDevice start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.GetRemoteDevice();
    HILOGI("HandsFreeUnitCalls_UnitTest_GetRemoteDevice end");
}

/*
 * @tc.number: HandsFreeUnitCalls02
 * @tc.name: GetId
 * @tc.desc: GetId
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_GetId, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_GetId start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.GetId();
    HILOGI("HandsFreeUnitCalls_UnitTest_GetId end");
}

/*
 * @tc.number: HandsFreeUnitCalls03
 * @tc.name: GetUuid
 * @tc.desc: GetUuid
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_GetUuid, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_GetUuid start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.GetUuid();
    HILOGI("HandsFreeUnitCalls_UnitTest_GetUuid end");
}

/*
 * @tc.number: HandsFreeUnitCalls04
 * @tc.name: GetState
 * @tc.desc: GetState
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_GetState, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_GetState start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.GetState();
    HILOGI("HandsFreeUnitCalls_UnitTest_GetState end");
}

/*
 * @tc.number: HandsFreeUnitCalls05
 * @tc.name: GetNumber
 * @tc.desc: GetNumber
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_GetNumber, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_GetNumber start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.GetNumber();
    HILOGI("HandsFreeUnitCalls_UnitTest_GetNumber end");
}

/*
 * @tc.number: HandsFreeUnitCalls06
 * @tc.name: GetCreationTime
 * @tc.desc: GetCreationTime
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_GetCreationTime, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_GetCreationTime start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.GetCreationTime();
    HILOGI("HandsFreeUnitCalls_UnitTest_GetCreationTime end");
}

/*
 * @tc.number: HandsFreeUnitCalls07
 * @tc.name: IsMultiParty
 * @tc.desc: IsMultiParty
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_IsMultiParty, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_IsMultiParty start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.IsMultiParty();
    HILOGI("HandsFreeUnitCalls_UnitTest_IsMultiParty end");
}

/*
 * @tc.number: HandsFreeUnitCalls08
 * @tc.name: IsOutgoing
 * @tc.desc: IsOutgoing
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_IsOutgoing, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_IsOutgoing start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.IsOutgoing();
    HILOGI("HandsFreeUnitCalls_UnitTest_IsOutgoing end");
}

/*
 * @tc.number: HandsFreeUnitCalls09
 * @tc.name: IsInBandRing
 * @tc.desc: IsInBandRing
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_IsInBandRing, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_IsInBandRing start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.IsInBandRing();
    HILOGI("HandsFreeUnitCalls_UnitTest_IsInBandRing end");
}

/*
 * @tc.number: HandsFreeUnitCalls10
 * @tc.name: SetState
 * @tc.desc: SetState
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_SetState, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_SetState start");
    int state = 1;
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.SetState(state);
    HILOGI("HandsFreeUnitCalls_UnitTest_SetState end");
}

/*
 * @tc.number: HandsFreeUnitCalls11
 * @tc.name: SetNumber
 * @tc.desc: SetNumber
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_SetNumber, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_SetNumber start");
    std::string number = "123";
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.SetNumber(number);
    HILOGI("HandsFreeUnitCalls_UnitTest_SetNumber end");
}

/*
 * @tc.number: HandsFreeUnitCalls12
 * @tc.name: SetMultiParty
 * @tc.desc: SetMultiParty
*/
HWTEST_F(HandsFreeUnitTest, HandsFreeUnitCalls_UnitTest_SetMultiParty, TestSize.Level1)
{
    HILOGI("HandsFreeUnitCalls_UnitTest_SetMultiParty start");
    OHOS::bluetooth::HandsFreeUnitCalls calls;
    calls.SetMultiParty(true);
    calls.SetMultiParty(false);
    HILOGI("HandsFreeUnitCalls_UnitTest_SetMultiParty end");
}
}  // namespace Bluetooth
}  // namespace OHOS