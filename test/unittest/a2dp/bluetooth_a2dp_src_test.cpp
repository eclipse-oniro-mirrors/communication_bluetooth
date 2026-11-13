/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include <vector>
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "i_bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "i_bluetooth_a2dp_src.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_a2dp_src.h"
#include "bluetooth_access_token_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

constexpr int TIME = 4;

namespace OHOS {
namespace Bluetooth {
static A2dpSource *profile_;
static BluetoothHost *host_;

class BluetoothA2dpSrcTest : public testing::Test {
public:
    BluetoothA2dpSrcTest()
    {}
    ~BluetoothA2dpSrcTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetRandomAddr(BluetoothRemoteDevice &device);
    sptr<IBluetoothA2dpSrc> proxy_ = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
};

void BluetoothA2dpSrcTest::SetUpTestCase(void)
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
    profile_ = A2dpSource::GetProfile();

    HILOGI("SetUpTestCase end");
}

void BluetoothA2dpSrcTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void BluetoothA2dpSrcTest::SetUp()
{
}

void BluetoothA2dpSrcTest::TearDown()
{
}

/*
 * @tc.number: GetDeviceState_Test_001
 * @tc.name: GetDeviceState
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, GetDeviceState_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetDeviceState_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int state = 0;
    int ret = profile_->GetDeviceState(device, state);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetDeviceState_001 end");
}

/*
 * @tc.number: GetPlayingState_Test_001
 * @tc.name: GetPlayingState(const BluetoothRemoteDevice &device) const
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, GetPlayingState_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetPlayingState_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int ret = profile_->GetPlayingState(device);
    EXPECT_EQ(ret, RET_BAD_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetPlayingState_001 end");
}

/*
 * @tc.number: GetPlayingState_Test_002
 * @tc.name: GetPlayingState(const BluetoothRemoteDevice &device, int &state) const
 * @tc.desc: IS_BT_ENABLED: true, IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, GetPlayingState_Test_002, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetPlayingState_002 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int states = 0;
    int ret = profile_->GetPlayingState(device, states);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetPlayingState_002 end");
}

/*
 * @tc.number: Connect_Test_001
 * @tc.name: Connect
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, Connect_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_Connect_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int ret = profile_->Connect(device);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_Connect_004 end");
}

/*
 * @tc.number: Disconnect_Test_001
 * @tc.name: Disconnect
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, Disconnect_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_Disconnect_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int ret = profile_->Disconnect(device);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_Disconnect_001 end");
}

/*
 * @tc.number: SetActiveSinkDevice_Test_001
 * @tc.name: SetActiveSinkDevice
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, SetActiveSinkDevice_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_SetActiveSinkDevice_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int ret = profile_->SetActiveSinkDevice(device);
    EXPECT_EQ(ret, RET_BAD_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_SetActiveSinkDevice_001 end");
}

/*
 * @tc.number: GetConnectStrategy_Test_001
 * @tc.name: GetConnectStrategy
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, GetConnectStrategy_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetConnectStrategy_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int strategy = 0;
    int ret = profile_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetConnectStrategy_001 end");
}

/*
 * @tc.number: GetCodecStatus_Test_001
 * @tc.name: GetCodecStatus
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, GetCodecStatus_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetCodecStatus_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    A2dpCodecInfo codecInfo = profile_->GetCodecStatus(device).codecInfo;
    EXPECT_NE(&codecInfo, nullptr);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetCodecStatus_001 end");
}

/*
 * @tc.number: GetCodecPreference_Test_001
 * @tc.name: GetCodecPreference
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, GetCodecPreference_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetCodecPreference_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    A2dpCodecInfo serviceInfo;
    int ret = profile_->GetCodecPreference(device, serviceInfo);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetCodecPreference_001 end");
}

/*
 * @tc.number: SetCodecPreference_Test_001
 * @tc.name: SetCodecPreference
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, SetCodecPreference_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_SetCodecPreference_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    A2dpCodecInfo serviceInfo;
    int ret = profile_->SetCodecPreference(device, serviceInfo);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_SetCodecPreference_001 end");
}

/*
 * @tc.number: GetOptionalCodecsSupportState_Test_001
 * @tc.name: GetOptionalCodecsSupportState
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, GetOptionalCodecsSupportState_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetOptionalCodecsSupportState_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    bool isEnable = true;
    profile_->SwitchOptionalCodecs(device, isEnable);
    int ret = profile_->GetOptionalCodecsSupportState(device);
    EXPECT_EQ(ret, RET_BAD_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetOptionalCodecsSupportState_001 end");
}

/*
 * @tc.number: StartPlaying_Test_001
 * @tc.name: StartPlaying
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, StartPlaying_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_StartPlaying_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int ret = profile_->StartPlaying(device);
    EXPECT_EQ(ret, RET_BAD_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_StartPlaying_001 end");
}

/*
 * @tc.number: SuspendPlaying_Test_001
 * @tc.name: SuspendPlaying
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, SuspendPlaying_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_SuspendPlaying_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int ret = profile_->SuspendPlaying(device);
    EXPECT_EQ(ret, RET_BAD_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_SuspendPlaying_001 end");
}

/*
 * @tc.number: StopPlaying_Test_001
 * @tc.name: StopPlaying
 * @tc.desc: IsValidBluetoothRemoteDevice(): false
*/
HWTEST_F(BluetoothA2dpSrcTest, StopPlaying_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_StopPlaying_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int ret = profile_->StopPlaying(device);
    EXPECT_EQ(ret, RET_BAD_PARAM);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_StopPlaying_001 end");
}

/*
 * @tc.number: ReadFromParcel_Test_001
 * @tc.name: ReadFromParcel
 * @tc.desc: Assign size to enter the for loop
*/
HWTEST_F(BluetoothA2dpSrcTest, ReadFromParcel_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpA2dpCodecStatusTest_UnitTest_ReadFromParcel_001 start");
    Parcel parcel;
    BluetoothA2dpCodecStatus obj;
    parcel.WriteParcelable(nullptr);
    uint32_t size = 2;
    parcel.WriteUint32(size);
    std::vector<BluetoothRawAddress> devices;
    bool ret = obj.ReadFromParcel(parcel);
    EXPECT_FALSE(ret);
    HILOGI("BluetoothA2dpA2dpCodecStatusTest_UnitTest_ReadFromParcel_001 end");
}

/*
 * @tc.number: GetConnectStrategy_Test_002
 * @tc.name: GetConnectStrategy
 * @tc.desc: strategy: CONNECTION_ALLOWED
*/
HWTEST_F(BluetoothA2dpSrcTest, GetConnectStrategy_Test_002, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetConnectStrategy_002 start");
    BluetoothRemoteDevice device("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int strategy = 0;
    int ret = profile_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetConnectStrategy_002 end");
}

/*
 * @tc.number: GetConnectStrategy_Test_003
 * @tc.name: GetConnectStrategy
 * @tc.desc: strategy: CONNECTION_FORBIDDEN
*/
HWTEST_F(BluetoothA2dpSrcTest, GetConnectStrategy_Test_003, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetConnectStrategy_003 start");
    BluetoothRemoteDevice device("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int strategy = 2;
    int ret = profile_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_GetConnectStrategy_003 end");
}

/*
 * @tc.number: UnitTest_DisableBt_001
 * @tc.name: DisableBt
 * @tc.desc: IS_BT_ENABLED: false
*/
HWTEST_F(BluetoothA2dpSrcTest, UnitTest_DisableBt_001, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcTest_UnitTest_DisableBt_001 start");
    int btState = host_->GetBtState();
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        host_->DisableBle();
        sleep(TIME);
    }
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    A2dpCodecInfo serviceInfo;
    EXPECT_EQ(profile_->GetPlayingState(device), RET_BAD_STATUS);
    EXPECT_EQ(profile_->GetCodecPreference(device, serviceInfo), BT_ERR_INVALID_STATE);
    HILOGI("BluetoothA2dpSrcTest_UnitTest_DisableBt_001 end");
}

}  // namespace Bluetooth
}  // namespace OHOS