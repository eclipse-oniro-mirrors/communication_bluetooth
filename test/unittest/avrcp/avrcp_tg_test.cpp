/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "bluetooth_avrcp_ct.h"
#include "bluetooth_avrcp_tg.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_avrcp_tg_proxy.h"
#include "bluetooth_avrcp_tg_observer_stub.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"
#include "raw_address.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;

class AvrcpTargetObserverCommon : public AvrcpTarget::IObserver {
public:
    AvrcpTargetObserverCommon() = default;
    virtual ~AvrcpTargetObserverCommon() = default;

    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause){}
private:
};

class BluetoothAvrcpTgObserverStubTest : public BluetoothAvrcpTgObserverStub {
public:
    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {}

    explicit BluetoothAvrcpTgObserverStubTest()
    {}
    ~BluetoothAvrcpTgObserverStubTest() override
    {}

private:
};


static std::shared_ptr<AvrcpTargetObserverCommon> observer_ = std::make_shared<AvrcpTargetObserverCommon>();
static AvrcpTarget *profile_;
static BluetoothHost *host_;

class AvrcpTargetTest : public testing::Test {
public:
    AvrcpTargetTest()
    {}
    ~AvrcpTargetTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetRandomAddr(BluetoothRemoteDevice &device);
};


void AvrcpTargetTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
    host_ = &BluetoothHost::GetDefaultHost();
    profile_ = AvrcpTarget::GetProfile();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBleEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}

void AvrcpTargetTest::TearDownTestCase(void)
{
    host_ = nullptr;
}

void AvrcpTargetTest::SetUp()
{
}

void AvrcpTargetTest::TearDown()
{
}

void AvrcpTargetTest::SetRandomAddr(BluetoothRemoteDevice &device)
{
    string randomAddr;
    host_->GetRandomAddress(device.GetDeviceAddr(), randomAddr);
    device.address_ = randomAddr;
}

/*
 * @tc.number: AvrcpTarget001
 * @tc.name: GetProfile
 * @tc.desc: Gets the static instance of the <b>AvrcpTarget</b> class.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_GetProfile, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_GetProfile start");
    profile_ = AvrcpTarget::GetProfile();
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_GetProfile end");
}

/*
 * @tc.number: AvrcpTarget002
 * @tc.name: GetConnectedDevices
 * @tc.desc: Gets the connected devices.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_GetConnectedDevices, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_GetConnectedDevices start");
    profile_ = AvrcpTarget::GetProfile();
    vector<BluetoothRemoteDevice> devices = profile_->GetConnectedDevices();
    EXPECT_NE(nullptr, &devices);
    HILOGI("Avrcp_UnitTest_GetConnectedDevices end");
}

/*
 * @tc.number: AvrcpTarget003
 * @tc.name: GetDeviceState
 * @tc.desc: Gets the connection state of the specified bluetooth device.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_GetDeviceState, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_GetDeviceState start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);

    int state = profile_->GetDeviceState(device);
    EXPECT_EQ(state, 3);
    HILOGI("Avrcp_UnitTest_GetDeviceState end");
}

/*
 * @tc.number: AvrcpTarget004
 * @tc.name: RegisterObserver
 * @tc.desc: Registers the observer.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_RegisterObserver, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_RegisterObserver start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->RegisterObserver(observer_);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_RegisterObserver end");
}

/*
 * @tc.number: AvrcpTarget005
 * @tc.name: UnregisterObserver
 * @tc.desc: Unregisters the observer.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_UnregisterObserver, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_DeregisterObserver start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->UnregisterObserver(observer_);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_DeregisterObserver end");
}

/*
 * @tc.number: AvrcpTarget006
 * @tc.name: SetActiveDevice
 * @tc.desc: Sets the active device.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_SetActiveDevice, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_SetActiveDevice start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_->SetActiveDevice(device);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_SetActiveDevice end");
}

/*
 * @tc.number: AvrcpTarget007
 * @tc.name: GetDevicesByStates
 * @tc.desc: Gets the devices of the specified states.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_GetDevicesByStates, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_GetDevicesByStates start");
    profile_ = AvrcpTarget::GetProfile();
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    vector<BluetoothRemoteDevice> devices = profile_->GetDevicesByStates(states);
    EXPECT_NE(nullptr, &devices);
    HILOGI("Avrcp_UnitTest_GetDevicesByStates end");
}

/*
 * @tc.number: AvrcpTarget008
 * @tc.name: Connect
 * @tc.desc: Connects to the AVRCP CT service.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_Connect start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    bool result = profile_->Connect(device);
    EXPECT_FALSE(result);
    HILOGI("Avrcp_UnitTest_Connect end");
}

/*
 * @tc.number: AvrcpTarget009
 * @tc.name: Disconnect
 * @tc.desc: Disconnects from the AVRCP CT service.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_Disconnect, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_Disconnect start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    bool result = profile_->Disconnect(device);
    EXPECT_FALSE(result);
    HILOGI("Avrcp_UnitTest_Disconnect end");
}

/*
 * @tc.number: AvrcpTarget010
 * @tc.name: NotifyPlaybackStatusChanged
 * @tc.desc: Notifies the playback status is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyPlaybackStatusChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyPlaybackStatusChanged start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyPlaybackStatusChanged(0,0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyPlaybackStatusChanged end");
}

/*
 * @tc.number: AvrcpTarget011
 * @tc.name: NotifyTrackChanged
 * @tc.desc: Notifies the track is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyTrackChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyTrackChanged start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyTrackChanged(0,0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyTrackChanged end");
}

/*
 * @tc.number: AvrcpTarget012
 * @tc.name: NotifyTrackReachedEnd
 * @tc.desc: Notifies the track reached end is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyTrackReachedEnd, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyTrackReachedEnd start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyTrackReachedEnd(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyTrackReachedEnd end");
}

/*
 * @tc.number: AvrcpTarget013
 * @tc.name: NotifyTrackReachedStart
 * @tc.desc: Notifies the track reached start is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyTrackReachedStart, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyTrackReachedStart start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyTrackReachedStart(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyTrackReachedStart end");
}

/*
 * @tc.number: AvrcpTarget014
 * @tc.name: NotifyPlaybackPosChanged
 * @tc.desc: Notifies the player application setting is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyPlaybackPosChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyPlaybackPosChanged start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyPlaybackPosChanged(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyPlaybackPosChanged end");
}

/*
 * @tc.number: AvrcpTarget015
 * @tc.name: NotifyPlayerAppSettingChanged
 * @tc.desc: Notifies the player application setting is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyPlayerAppSettingChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyPlayerAppSettingChanged start");
    profile_ = AvrcpTarget::GetProfile();
    vector<uint8_t> attributes = {0};
    vector<uint8_t> values = {0};
    profile_->NotifyPlayerAppSettingChanged(attributes, values);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyPlayerAppSettingChanged end");
}

/*
 * @tc.number: AvrcpTarget016
 * @tc.name: NotifyNowPlayingContentChanged
 * @tc.desc: Notifies the addressed player is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyNowPlayingContentChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyNowPlayingContentChanged start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyNowPlayingContentChanged();
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyNowPlayingContentChanged end");
}

/*
 * @tc.number: AvrcpTarget017
 * @tc.name: NotifyAvailablePlayersChanged
 * @tc.desc: Notifies that has the available player.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyAvailablePlayersChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyAvailablePlayersChanged start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyAvailablePlayersChanged();
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyAvailablePlayersChanged end");
}

/*
 * @tc.number: AvrcpTarget018
 * @tc.name: NotifyAddressedPlayerChanged
 * @tc.desc: Notifies the addressed player is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyAddressedPlayerChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyAddressedPlayerChanged start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyAddressedPlayerChanged(0 ,0);
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_->SetActiveDevice(device);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyAddressedPlayerChanged end");
}

/*
 * @tc.number: AvrcpTarget019
 * @tc.name: NotifyUidChanged
 * @tc.desc: Notifies the uids is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyUidChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyUidChanged start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyUidChanged(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyUidChanged end");
}

/*
 * @tc.number: AvrcpTarget020
 * @tc.name: NotifyVolumeChanged
 * @tc.desc: Notifies the absolute volume is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_NotifyVolumeChanged, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyVolumeChanged start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyVolumeChanged(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyVolumeChanged end");
}

/*
 * @tc.number: AvrcpTarget021
 * @tc.name: OnConnectionStateChangedInner
 * @tc.desc: Check OnConnectionStateChangedInner interface
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_OnConnectionStateChangedInner, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_OnConnectionStateChangedInner start");
    BluetoothAvrcpTgObserverStubTest bluetoothAvrcpTgObserverStubTest;
    MessageParcel data;
    MessageParcel reply;

    ErrCode ret = BluetoothAvrcpTgObserverStub::OnConnectionStateChangedInner(&bluetoothAvrcpTgObserverStubTest,
        data, reply);
    EXPECT_EQ(ret, 1);
    HILOGI("Avrcp_UnitTest_OnConnectionStateChangedInner end");
}

/*
 * @tc.number: AvrcpTarget022
 * @tc.name: SetDeviceAbsoluteVolume
 * @tc.desc: Notifies the absolute volume is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_SetDeviceAbsoluteVolume, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyVolumeChanged start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_->SetDeviceAbsoluteVolume(device, 7);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyVolumeChanged end");
}

/*
 * @tc.number: AvrcpTarget023
 * @tc.name: SetDeviceAbsoluteVolume
 * @tc.desc: Notifies the absolute volume is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_GetDeviceAbsVolumeAbility, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyVolumeChanged start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int ability = 0;
    profile_->GetDeviceAbsVolumeAbility(device, ability);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyVolumeChanged end");
}

/*
 * @tc.number: AvrcpTarget024
 * @tc.name: SetDeviceAbsoluteVolume
 * @tc.desc: Notifies the absolute volume is changed.
*/
HWTEST_F(AvrcpTargetTest, Avrcp_UnitTest_SetDeviceAbsVolumeAbility, TestSize.Level1)
{
    HILOGI("Avrcp_UnitTest_NotifyVolumeChanged start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int ability = 2;
    profile_->SetDeviceAbsVolumeAbility(device, ability);
    EXPECT_NE(nullptr, profile_);
    HILOGI("Avrcp_UnitTest_NotifyVolumeChanged end");
}
}  // namespace Bluetooth
}  // namespace OHOS