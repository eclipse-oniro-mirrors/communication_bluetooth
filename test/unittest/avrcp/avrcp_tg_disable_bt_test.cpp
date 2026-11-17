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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bluetooth_access_token_mock.h"
#include "bluetooth_avrcp_ct.h"
#include "bluetooth_avrcp_tg.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
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

static AvrcpTarget *profile_;
static BluetoothHost *host_;

class AvrcpTargetDisableBtTest : public testing::Test {
public:
    AvrcpTargetDisableBtTest()
    {}
    ~AvrcpTargetDisableBtTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetRandomAddr(BluetoothRemoteDevice &device);
};


void AvrcpTargetDisableBtTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
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

void AvrcpTargetDisableBtTest::TearDownTestCase(void)
{
    host_ = nullptr;
}

void AvrcpTargetDisableBtTest::SetUp()
{
}

void AvrcpTargetDisableBtTest::TearDown()
{
}

void AvrcpTargetDisableBtTest::SetRandomAddr(BluetoothRemoteDevice &device)
{
    string randomAddr;
    host_->GetRandomAddress(device.GetDeviceAddr(), randomAddr);
    device.address_ = randomAddr;
}

/*
 * @tc.number: AvrcpTarget_DisableBt_001
 * @tc.name: GetConnectedDevices
 * @tc.desc: Gets the connected devices.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_001, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_001 start");
    profile_ = AvrcpTarget::GetProfile();
    vector<BluetoothRemoteDevice> devices = profile_->GetConnectedDevices();
    EXPECT_NE(nullptr, &devices);
    HILOGI("AvrcpTarget_DisableBt_001 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_002
 * @tc.name: GetDeviceState
 * @tc.desc: Gets the connection state of the specified bluetooth device.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_002, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_002 start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);

    int state = profile_->GetDeviceState(device);
    EXPECT_EQ(state, 3);
    HILOGI("AvrcpTarget_DisableBt_002 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_003
 * @tc.name: SetActiveDevice
 * @tc.desc: Sets the active device.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_003, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_003 start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_->SetActiveDevice(device);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_003 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_004
 * @tc.name: GetDevicesByStates
 * @tc.desc: Gets the devices of the specified states.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_004, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_004 start");
    profile_ = AvrcpTarget::GetProfile();
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    vector<BluetoothRemoteDevice> devices = profile_->GetDevicesByStates(states);
    EXPECT_NE(nullptr, &devices);
    HILOGI("AvrcpTarget_DisableBt_004 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_005
 * @tc.name: Connect
 * @tc.desc: Connects to the AVRCP CT service.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_005, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_005 start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    SetRandomAddr(device);
    bool result = profile_->Connect(device);
    EXPECT_FALSE(result);
    HILOGI("AvrcpTarget_DisableBt_005 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_006
 * @tc.name: Disconnect
 * @tc.desc: Disconnects from the AVRCP CT service.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_006, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_006 start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    SetRandomAddr(device);
    bool result = profile_->Disconnect(device);
    EXPECT_FALSE(result);
    HILOGI("AvrcpTarget_DisableBt_006 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_007
 * @tc.name: NotifyPlaybackStatusChanged
 * @tc.desc: Notifies the playback status is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_007, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_007 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyPlaybackStatusChanged(0, 0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_007 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_008
 * @tc.name: NotifyTrackChanged
 * @tc.desc: Notifies the track is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_008, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_008 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyTrackChanged(0, 0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_008 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_009
 * @tc.name: NotifyTrackReachedEnd
 * @tc.desc: Notifies the track reached end is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_009, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_009 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyTrackReachedEnd(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_009 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_010
 * @tc.name: NotifyTrackReachedStart
 * @tc.desc: Notifies the track reached start is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_010, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_010 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyTrackReachedStart(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_010 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_011
 * @tc.name: NotifyPlaybackPosChanged
 * @tc.desc: Notifies the player application setting is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_011, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_011 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyPlaybackPosChanged(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_011 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_012
 * @tc.name: NotifyPlayerAppSettingChanged
 * @tc.desc: Notifies the player application setting is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_012, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_012 start");
    profile_ = AvrcpTarget::GetProfile();
    vector<uint8_t> attributes = {0};
    vector<uint8_t> values = {0};
    profile_->NotifyPlayerAppSettingChanged(attributes, values);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_012 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_013
 * @tc.name: NotifyNowPlayingContentChanged
 * @tc.desc: Notifies the addressed player is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_013, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_013 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyNowPlayingContentChanged();
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_013 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_014
 * @tc.name: NotifyAvailablePlayersChanged
 * @tc.desc: Notifies that has the available player.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_014, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_014 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyAvailablePlayersChanged();
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_014 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_015
 * @tc.name: NotifyAddressedPlayerChanged
 * @tc.desc: Notifies the addressed player is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_015, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_015 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyAddressedPlayerChanged(0, 0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_015 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_016
 * @tc.name: NotifyUidChanged
 * @tc.desc: Notifies the uids is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_016, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_016 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyUidChanged(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_016 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_017
 * @tc.name: NotifyVolumeChanged
 * @tc.desc: Notifies the absolute volume is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_017, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_017 start");
    profile_ = AvrcpTarget::GetProfile();
    profile_->NotifyVolumeChanged(0);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_017 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_019
 * @tc.name: SetDeviceAbsoluteVolume
 * @tc.desc: Notifies the absolute volume is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_019, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_019 start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_->SetDeviceAbsoluteVolume(device, 7);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_019 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_020
 * @tc.name: SetDeviceAbsoluteVolume
 * @tc.desc: Notifies the absolute volume is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_020, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_020 start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int ability = 0;
    profile_->GetDeviceAbsVolumeAbility(device, ability);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_020 end");
}

/*
 * @tc.number: AvrcpTarget_DisableBt_021
 * @tc.name: SetDeviceAbsoluteVolume
 * @tc.desc: Notifies the absolute volume is changed.
*/
HWTEST_F(AvrcpTargetDisableBtTest, AvrcpTarget_DisableBt_021, TestSize.Level1)
{
    HILOGI("AvrcpTarget_DisableBt_021 start");
    profile_ = AvrcpTarget::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int ability = 2;
    profile_->SetDeviceAbsVolumeAbility(device, ability);
    EXPECT_NE(nullptr, profile_);
    HILOGI("AvrcpTarget_DisableBt_021 end");
}
}  // namespace Bluetooth
}  // namespace OHOS