/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "bluetooth_a2dp_src.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

#include "bluetooth_def.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace std;

constexpr int TIME = 4;

class A2dpSourceObserverCommon : public A2dpSourceObserver {
public:
    A2dpSourceObserverCommon() = default;
    virtual ~A2dpSourceObserverCommon() = default;

    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause){}
    virtual void OnConfigurationChanged(const BluetoothRemoteDevice &device, const A2dpCodecInfo &info, int error){}
    virtual void OnPlayingStatusChanged(const BluetoothRemoteDevice &device, int playingState, int error){}
    virtual void OnMediaStackChanged(const BluetoothRemoteDevice &device, int action){}
    virtual void OnVirtualDeviceChanged(int32_t action, std::string address){}
    virtual void OnCaptureConnectionStateChanged(const BluetoothRemoteDevice &device, int state,
        const A2dpCodecInfo &info){}

private:
};

static std::shared_ptr<A2dpSourceObserverCommon> observer_ = std::make_shared<A2dpSourceObserverCommon>();
static A2dpSource *profile_;
static BluetoothHost *host_;

class A2dpSourceTest : public testing::Test {
public:
    A2dpSourceTest()
    {}
    ~A2dpSourceTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetRandomAddr(BluetoothRemoteDevice &device);
    void CallbackStatusChange();
};


void A2dpSourceTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();

    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBleEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    profile_ = A2dpSource::GetProfile();
    profile_->RegisterObserver(observer_);
    HILOGI("SetUpTestCase end");
}

void A2dpSourceTest::TearDownTestCase(void)
{
    profile_->DeregisterObserver(observer_);
    host_ = nullptr;
}

void A2dpSourceTest::SetUp()
{
}

void A2dpSourceTest::TearDown()
{
}

void A2dpSourceTest::SetRandomAddr(BluetoothRemoteDevice &device)
{
    string randomAddr;
    host_->GetRandomAddress(device.GetDeviceAddr(), randomAddr);
    device.address_ = randomAddr;
}

void A2dpSourceTest::CallbackStatusChange()
{
    std::string address = "11:22:33:44:55:66";
    BluetoothRemoteDevice device(address, BT_TRANSPORT_BREDR);
    A2dpCodecInfo info;
    info.codecPriority = 0;
    info.codecType = 1;
    info.sampleRate = 2;
    info.bitsPerSample = 3;
    info.channelMode = 4;
    info.codecSpecific1 = 5;
    info.codecSpecific2 = 6;
    info.codecSpecific3 = 7;
    info.codecSpecific4 = 8;
    int state = 1;
    A2dpSourceObserver a2dpSrcObserver;
    int32_t action = 0;
    a2dpSrcObserver.OnConnectionStateChanged(device, 0, 0);
    a2dpSrcObserver.OnConfigurationChanged(device, info, 0);
    a2dpSrcObserver.OnPlayingStatusChanged(device, 0, 0);
    a2dpSrcObserver.OnMediaStackChanged(device, action);
    a2dpSrcObserver.OnVirtualDeviceChanged(action, address);
    a2dpSrcObserver.OnCaptureConnectionStateChanged(device, state, info);
}

/*
 * @tc.number: A2dpSource_UnitTest_GetProfile
 * @tc.name: GetProfile
 * @tc.desc: Get a2dp sink instance
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetProfile, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetProfile start");

    profile_ = A2dpSource::GetProfile();
    EXPECT_NE(nullptr, profile_);
    HILOGI("A2dpSource_UnitTest_GetProfile end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetPlayingState
 * @tc.name: GetPlayingState
 * @tc.desc: Get device playing state by address when peer device is on connected
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetPlayingState, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetPlayingState start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int state = profile_->GetPlayingState(device);
    EXPECT_EQ(state, RET_NO_ERROR);
    HILOGI("A2dpSource_UnitTest_GetPlayingState end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetDeviceState
 * @tc.name: GetDeviceState
 * @tc.desc: Get device connection state by address
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetDeviceState, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetDeviceState start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int state = 0;
    int ret = profile_->GetDeviceState(device, state);
    HILOGI("A2dpSource_UnitTest_GetDeviceState ret:%{public}d", ret);
    EXPECT_FALSE(state == 0);

    HILOGI("A2dpSource_UnitTest_GetDeviceState end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetDevicesByStates
 * @tc.name: GetDevicesByStates
 * @tc.desc: Get devices by connection states
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetDevicesByStates, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetDevicesByStates start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    vector<BluetoothRemoteDevice> devices;
    profile_->GetDevicesByStates(states, devices);
    EXPECT_NE(nullptr, &devices);

    HILOGI("A2dpSource_UnitTest_GetDevicesByStates end");
}

/*
 * @tc.number: A2dpSource_UnitTest_Connect
 * @tc.name: Connect
 * @tc.desc: Connect to the peer bluetooth device
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_Connect start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int32_t ret = profile_->Connect(device);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpSource_UnitTest_Connect end");
}

/*
 * @tc.number: A2dpSource_UnitTest_Disconnect
 * @tc.name: Disconnect
 * @tc.desc: Disconnect with the peer bluetooth service
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_Disconnect, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_Disconnect start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int32_t ret = profile_->Disconnect(device);
    EXPECT_NE(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpSource_UnitTest_Disconnect end");
}

/*
 * @tc.number: A2dpSource_UnitTest_SetActiveSinkDevice
 * @tc.name: SetActiveSinkDevice
 * @tc.desc: Set the active sink device for audio connection.
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_SetActiveSinkDevice, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_SetActiveSinkDevice start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_ = A2dpSource::GetProfile();
    profile_->SetActiveSinkDevice(device);
    EXPECT_NE(profile_, nullptr);
    HILOGI("A2dpSource_UnitTest_SetActiveSinkDevice end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetActiveSinkDevice
 * @tc.name: GetActiveSinkDevice
 * @tc.desc: Get the active sink device for audio connection.
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetActiveSinkDevice, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetActiveSinkDevice start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device = profile_->GetActiveSinkDevice();
    EXPECT_NE(nullptr, &device);
    HILOGI("A2dpSource_UnitTest_GetActiveSinkDevice end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetConnectStrategy
 * @tc.name: GetConnectStrategy
 * @tc.desc: Get connect strategy by address
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetConnectStrategy, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();

    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));

    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SetConnectStrategy
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SetConnectStrategy, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SetConnectStrategy start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();

    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    int strategy = 0;
    profile_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));

    HILOGI("A2dpSourceTest_UnitTest_SetConnectStrategy end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetCodecStatus
 * @tc.name: GetCodecStatus
 * @tc.desc: Get codec status by address
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetCodecStatus, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetCodecStatus start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    A2dpCodecStatus ret = profile_->GetCodecStatus(device);
    EXPECT_NE(nullptr, &ret);
    HILOGI("A2dpSourceTest_UnitTest_GetCodecStatus end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SetCodecPreference
 * @tc.name: SetCodecPreference
 * @tc.desc: Set codec preference by address
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SetCodecPreference, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SetCodecPreference start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    A2dpCodecInfo info;
    info.codecPriority = 0;
    info.codecType = 1;
    info.sampleRate = 2;
    info.bitsPerSample = 3;
    info.channelMode = 4;
    info.codecSpecific1 = 5;
    info.codecSpecific2 = 6;
    info.codecSpecific3 = 7;
    info.codecSpecific4 = 8;
    EXPECT_NE(profile_->SetCodecPreference(device, info), 0);

    HILOGI("A2dpSourceTest_UnitTest_SetCodecPreference end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState
 * @tc.name: GetOptionalCodecsSupportState
 * @tc.desc: Get optional codecs support state by address
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    bool isEnable = true;
    profile_->SwitchOptionalCodecs(device, isEnable);
    EXPECT_EQ(profile_->GetOptionalCodecsSupportState(device), 0);

    HILOGI("A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_StartPlaying
 * @tc.name: StartPlaying
 * @tc.desc: Start playing
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_StartPlaying, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_StartPlaying start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->StartPlaying(device), 0);
    CallbackStatusChange();
    HILOGI("A2dpSourceTest_UnitTest_StartPlaying end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SuspendPlaying
 * @tc.name: SuspendPlaying
 * @tc.desc: Suspend playing
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SuspendPlaying, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SuspendPlaying start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    SetRandomAddr(device);
    EXPECT_EQ(profile_->SuspendPlaying(device), 0);
    CallbackStatusChange();
    HILOGI("A2dpSourceTest_UnitTest_SuspendPlaying end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_StopPlaying
 * @tc.name: StopPlaying
 * @tc.desc: Stop playing
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_StopPlaying, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_StopPlaying start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    SetRandomAddr(device);
    EXPECT_EQ(profile_->StopPlaying(device), 0);

    HILOGI("A2dpSourceTest_UnitTest_StopPlaying end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_WriteFrame
 * @tc.name: WriteFrame
 * @tc.desc: Write frame data to a2dp
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_WriteFrame, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_WriteFrame start");

    uint8_t str[4] = {'1', '2', '3', '4'};
    profile_ = A2dpSource::GetProfile();
    int status = profile_->WriteFrame(str, 4);
    EXPECT_NE(status, RET_BAD_STATUS);
    HILOGI("A2dpSourceTest_UnitTest_WriteFrame end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetRenderPosition
 * @tc.name: GetRenderPosition
 * @tc.desc: Get delayValue, sendDataSize and timeStamp from a2dp
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetRenderPosition, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetRenderPosition start");

    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;
    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int32_t ret = profile_->GetRenderPosition(device, delayValue, sendDataSize, timeStamp);
    EXPECT_NE(ret, RET_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_GetRenderPosition end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetDeviceState_002
 * @tc.name: GetDeviceState
 * @tc.desc: IsValidBluetoothRemoteDevice
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetDeviceState_002, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetDeviceState_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int state = 0;
    int ret = profile_->GetDeviceState(device, state);
    HILOGI("A2dpSource_UnitTest_GetDeviceState_002 ret:%{public}d", ret);
    EXPECT_FALSE(state == 0);

    HILOGI("A2dpSource_UnitTest_GetDeviceState_002 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetDeviceState_003
 * @tc.name: GetDeviceState
 * @tc.desc: IS_BT_ENABLED:false
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetDeviceState_003, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetDeviceState_003 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    SetRandomAddr(device);
    int state = 0;
    int ret = profile_->GetDeviceState(device, state);
    HILOGI("A2dpSource_UnitTest_GetDeviceState_003 ret:%{public}d, state:%{public}d", ret, state);
    EXPECT_EQ(ret, RET_NO_ERROR);

    HILOGI("A2dpSource_UnitTest_GetDeviceState_003 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetPlayingState_002
 * @tc.name: GetPlayingState
 * @tc.desc: IsValidBluetoothRemoteDevice
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetPlayingState_002, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetPlayingState_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int state = profile_->GetPlayingState(device);
    EXPECT_EQ(state, BT_NO_ERROR);
    HILOGI("A2dpSource_UnitTest_GetPlayingState_002 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetPlayingState_003
 * @tc.name: GetPlayingState(const BluetoothRemoteDevice &device)
 * @tc.desc: IS_BT_ENABLED:false
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetPlayingState_003, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetPlayingState_003 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int state = profile_->GetPlayingState(device);
    EXPECT_EQ(state, RET_NO_ERROR);
    HILOGI("A2dpSource_UnitTest_GetPlayingState_003 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_Connect_002
 * @tc.name: Connect
 * @tc.desc: IsValidBluetoothRemoteDevice:true
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_Connect_002, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_Connect_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int32_t ret = profile_->Connect(device);
    EXPECT_NE(ret, RET_NO_ERROR);
    HILOGI("A2dpSource_UnitTest_Connect_002 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetPlayingState_004
 * @tc.name: GetPlayingState(const BluetoothRemoteDevice &device, int &state)
 * @tc.desc: IS_BT_ENABLED:false
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetPlayingState_004, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetPlayingState_004 start");
    int btState = 0;
    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int state = profile_->GetPlayingState(device, btState);
    EXPECT_EQ(state, BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpSource_UnitTest_GetPlayingState_004 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetPlayingState_005
 * @tc.name: GetPlayingState(const BluetoothRemoteDevice &device, int &state)
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetPlayingState_005, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetPlayingState_005 start");
    int btState = 0;
    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int state = profile_->GetPlayingState(device, btState);
    EXPECT_EQ(state, BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpSource_UnitTest_GetPlayingState_005 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_Disconnect_002
 * @tc.name: Disconnect
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_Disconnect_002, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_Disconnect_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int32_t ret = profile_->Disconnect(device);
    EXPECT_EQ(ret, RET_BAD_STATUS);
    HILOGI("A2dpSource_UnitTest_Disconnect_002 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_SetActiveSinkDevice_002
 * @tc.name: SetActiveSinkDevice
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_SetActiveSinkDevice_002, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_SetActiveSinkDevice_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    profile_ = A2dpSource::GetProfile();
    int ret = profile_->SetActiveSinkDevice(device);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpSource_UnitTest_SetActiveSinkDevice_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SetConnectStrategy_002
 * @tc.name: SetConnectStrategy
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SetConnectStrategy_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_EQ(profile_->SetConnectStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)), BT_ERR_INTERNAL_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_SetConnectStrategy_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetConnectStrategy_002
 * @tc.name: GetConnectStrategy
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetCodecStatus_002
 * @tc.name: GetCodecStatus
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetCodecStatus_002, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetCodecStatus_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    A2dpCodecStatus ret = profile_->GetCodecStatus(device);
    EXPECT_NE(nullptr, &ret);
    HILOGI("A2dpSourceTest_UnitTest_GetCodecStatus_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SetCodecPreference_002
 * @tc.name: SetCodecPreference
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SetCodecPreference_002, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SetCodecPreference_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    A2dpCodecInfo info;
    info.codecPriority = 0;
    info.codecType = 1;
    info.sampleRate = 2;
    info.bitsPerSample = 3;
    info.channelMode = 4;
    info.codecSpecific1 = 5;
    info.codecSpecific2 = 6;
    info.codecSpecific3 = 7;
    info.codecSpecific4 = 8;
    int ret = profile_->SetCodecPreference(device, info);
    EXPECT_NE(ret, 0);

    HILOGI("A2dpSourceTest_UnitTest_SetCodecPreference_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_002
 * @tc.name: GetOptionalCodecsSupportState
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_002, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int ret = profile_->GetOptionalCodecsSupportState(device);
    EXPECT_EQ(ret, BT_NO_ERROR);
    bool isEnable = true;
    profile_->SwitchOptionalCodecs(device, isEnable);
    HILOGI("A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_StartPlaying_002
 * @tc.name: StartPlaying
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_StartPlaying_002, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_StartPlaying_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int ret = profile_->StartPlaying(device);
    EXPECT_EQ(ret, BT_NO_ERROR);
    
    HILOGI("A2dpSourceTest_UnitTest_StartPlaying_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SuspendPlaying_002
 * @tc.name: SuspendPlaying
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SuspendPlaying_002, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SuspendPlaying_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int ret = profile_->SuspendPlaying(device);
    EXPECT_EQ(ret, BT_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_SuspendPlaying_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_StopPlaying_002
 * @tc.name: StopPlaying
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_StopPlaying_002, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_StopPlaying_002 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", INVALID_TYPE);
    int ret = profile_->StopPlaying(device);
    EXPECT_EQ(ret, BT_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_StopPlaying_002 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetConnectStrategy_003
 * @tc.name: GetConnectStrategy
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetConnectStrategy_003, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy_003 start");

    BluetoothRemoteDevice device("00:00:00:00:00:01", BT_TRANSPORT_BREDR);

    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN));

    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy_003 end");
}

/**
 * @tc.number: A2dpSourceTest_UnitTest_SetConnectStrategy_003
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SetConnectStrategy_003, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SetConnectStrategy_003 start");

    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();

    profile_ = A2dpSource::GetProfile();
    EXPECT_NE(profile_->SetConnectStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN)), BT_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_SetConnectStrategy_003 end");
}

/**
 * @tc.number: A2dpSourceTest_UnitTest_SetConnectStrategy_004
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SetConnectStrategy_004, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SetConnectStrategy_004 start");

    BluetoothRemoteDevice device("00:00:00:00:00:02", BT_TRANSPORT_BREDR);

    profile_ = A2dpSource::GetProfile();
    EXPECT_NE(profile_->SetConnectStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)), BT_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_SetConnectStrategy_004 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_GetDeviceState_004
 * @tc.name: GetDeviceState
 * @tc.desc: IS_BT_ENABLED:true，construct int ret condition
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_GetDeviceState_004, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_GetDeviceState_004 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.transport_ = BT_TRANSPORT_BREDR;
    int state = 0;
    int ret = profile_->GetDeviceState(device, state);
    HILOGI("A2dpSource_UnitTest_GetDeviceState_004 ret:%{public}d", ret);
    EXPECT_EQ(ret, 0);
    HILOGI("A2dpSource_UnitTest_GetDeviceState_004 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetConnectStrategy_004
 * @tc.name: GetConnectStrategy
 * @tc.desc: IS_BT_ENABLED():true, construct if (!device.IsValidBluetoothRemoteDevice()) condition
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetConnectStrategy_004, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy_004 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.transport_ = INVALID_TYPE;
    int strategy = 0;
    int ret = profile_->GetConnectStrategy(device, strategy);
    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy_004 ret:%{public}d", ret);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpSourceTest_UnitTest_GetConnectStrategy_004 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetCodecStatus_003
 * @tc.name: GetCodecStatus
 * @tc.desc: IS_BT_ENABLED:true，construct if (!device.IsValidBluetoothRemoteDevice()){} condition
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetCodecStatus_003, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetCodecStatus_003 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.transport_ = INVALID_TYPE;
    A2dpCodecStatus ret = profile_->GetCodecStatus(device);
    EXPECT_NE(nullptr, &ret);
    HILOGI("A2dpSourceTest_UnitTest_GetCodecStatus_003 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SetCodecPreference_003
 * @tc.name: SetCodecPreference
 * @tc.desc: IS_BT_ENABLED:true, construct if (!device.IsValidBluetoothRemoteDevice()) condition
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SetCodecPreference_003, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SetCodecPreference_003 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    A2dpCodecInfo info;
    info.codecPriority = 0;
    info.codecType = 1;
    info.sampleRate = 2;
    info.bitsPerSample = 3;
    info.channelMode = 4;
    info.codecSpecific1 = 5;
    info.codecSpecific2 = 6;
    info.codecSpecific3 = 7;
    info.codecSpecific4 = 8;
    device.transport_ = INVALID_TYPE;
    int ret = profile_->SetCodecPreference(device, info);
    HILOGI("A2dpSourceTest_UnitTest_SetCodecPreference_003 ret:%{public}d", ret);
    EXPECT_NE(ret, 0);
    HILOGI("A2dpSourceTest_UnitTest_SetCodecPreference_003 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SwitchOptionalCodecs_004
 * @tc.name: SwitchOptionalCodecs
 * @tc.desc: IS_BT_ENABLED:true, construct if (!device.IsValidBluetoothRemoteDevice()) condition
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SwitchOptionalCodecs_004, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SwitchOptionalCodecs_004 start");
    int btState = host_->GetBtState();
    if ((btState == BTStateID::STATE_TURNING_OFF) || (btState == BTStateID::STATE_TURN_OFF)) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
    EXPECT_NE(btState, STATE_TURN_OFF);
    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    bool isEnable = true;
    device.transport_ = INVALID_TYPE;
    profile_->SwitchOptionalCodecs(device, isEnable);
    HILOGI("A2dpSourceTest_UnitTest_SwitchOptionalCodecs_004 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_004
 * @tc.name: GetOptionalCodecsSupportState
 * @tc.desc: IS_BT_ENABLED:TRUE, construct if (!device.IsValidBluetoothRemoteDevice()) condition
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_004, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_004 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.transport_ = INVALID_TYPE;
    int ret = profile_->GetOptionalCodecsSupportState(device);
    HILOGI("A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_004 ret:%{public}d", ret);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpSourceTest_UnitTest_GetOptionalCodecsSupportState_004 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_StartPlaying_004
 * @tc.name: StartPlaying
 * @tc.desc: IS_BT_ENABLED:TRUE, construct if (!device.IsValidBluetoothRemoteDevice()) condition
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_StartPlaying_004, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_StartPlaying_004 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.transport_ = INVALID_TYPE;
    int ret = profile_->StartPlaying(device);
    HILOGI("A2dpSourceTest_UnitTest_StartPlaying_004 ret:%{public}d", ret);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpSourceTest_UnitTest_StartPlaying_004 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_SuspendPlaying_004
 * @tc.name: SuspendPlaying
 * @tc.desc: IS_BT_ENABLED:TRUE, construct if (!device.IsValidBluetoothRemoteDevice()) condition
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_SuspendPlaying_004, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_SuspendPlaying_004 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.transport_ = INVALID_TYPE;
    int ret = profile_->SuspendPlaying(device);
    HILOGI("A2dpSourceTest_UnitTest_SuspendPlaying_004 ret:%{public}d", ret);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpSourceTest_UnitTest_SuspendPlaying_004 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_StopPlaying_004
 * @tc.name: StopPlaying
 * @tc.desc: IS_BT_ENABLED:TRUE, construct if (!device.IsValidBluetoothRemoteDevice()) condition
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_StopPlaying_004, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_StopPlaying_004 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.transport_ = INVALID_TYPE;
    int ret = profile_->StopPlaying(device);
    HILOGI("A2dpSourceTest_UnitTest_StopPlaying_004 ret:%{public}d", ret);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpSourceTest_UnitTest_StopPlaying_004 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetCodecPreference
 * @tc.name: GetCodecPreference
 * @tc.desc: Get codec preference by address
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetCodecPreference, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetCodecPreference start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    A2dpCodecInfo info;
    EXPECT_NE(profile_->GetCodecPreference(device, info), 0);

    HILOGI("A2dpSourceTest_UnitTest_GetCodecPreference end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_EnableAutoPlay_01
 * @tc.name: EnableAutoPlay
 * @tc.desc: Enable Auto Play
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_EnableAutoPlay_01, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_EnableAutoPlay_01 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device;
    EXPECT_EQ(profile_->EnableAutoPlay(device), BT_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_EnableAutoPlay_01 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_EnableAutoPlay_02
 * @tc.name: EnableAutoPlay
 * @tc.desc: Enable Auto Play
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_EnableAutoPlay_02, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_EnableAutoPlay_02 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:", INVALID_TYPE);
    EXPECT_EQ(profile_->EnableAutoPlay(device), BT_ERR_INVALID_PARAM);

    HILOGI("A2dpSourceTest_UnitTest_EnableAutoPlay_02 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_DisableAutoPlay_01
 * @tc.name: DisableAutoPlay
 * @tc.desc: Disable Auto Play
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_DisableAutoPlay_01, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_DisableAutoPlay_01 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device;
    int duration = -1;
    EXPECT_EQ(profile_->DisableAutoPlay(device, duration), BT_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_DisableAutoPlay_01 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_DisableAutoPlay_02
 * @tc.name: DisableAutoPlay
 * @tc.desc: Disable Auto Play
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_DisableAutoPlay_02, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_DisableAutoPlay_02 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device;
    int duration = 3000;
    EXPECT_EQ(profile_->DisableAutoPlay(device, duration), BT_NO_ERROR);

    HILOGI("A2dpSourceTest_UnitTest_DisableAutoPlay_02 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_DisableAutoPlay_03
 * @tc.name: DisableAutoPlay
 * @tc.desc: Disable Auto Play
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_DisableAutoPlay_03, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_DisableAutoPlay_03 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:", INVALID_TYPE);
    int duration = 3000;
    EXPECT_EQ(profile_->DisableAutoPlay(device, duration), BT_ERR_INVALID_PARAM);

    HILOGI("A2dpSourceTest_UnitTest_DisableAutoPlay_03 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetAutoPlayDisabledDuration_01
 * @tc.name: GetAutoPlayDisabledDuration
 * @tc.desc: Get Auto Play Disabled Duration
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetAutoPlayDisabledDuration_01, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetAutoPlayDisabledDuration_01 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device;
    int duration;
    EXPECT_EQ(profile_->GetAutoPlayDisabledDuration(device, duration), BT_NO_ERROR);
    EXPECT_EQ(duration, 6 * 1000);

    HILOGI("A2dpSourceTest_UnitTest_GetAutoPlayDisabledDuration_01 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetAutoPlayDisabledDuration_02
 * @tc.name: GetAutoPlayDisabledDuration
 * @tc.desc: Get Auto Play Disabled Duration
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetAutoPlayDisabledDuration_02, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetAutoPlayDisabledDuration_02 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:", INVALID_TYPE);
    int duration;
    EXPECT_EQ(profile_->GetAutoPlayDisabledDuration(device, duration), BT_ERR_INVALID_PARAM);

    HILOGI("A2dpSourceTest_UnitTest_GetAutoPlayDisabledDuration_02 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_OffloadStartPlaying_01
 * @tc.name: OffloadStartPlaying
 * @tc.desc: Audio start offload streaming for hardware encoding datapath.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_OffloadStartPlaying_01, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_OffloadStartPlaying_01 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    vector<int32_t> sessionsId;
    EXPECT_EQ(profile_->OffloadStartPlaying(device, sessionsId), BT_ERR_INVALID_PARAM);
    HILOGI("A2dpSourceTest_UnitTest_OffloadStartPlaying_01 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_OffloadStartPlaying_02
 * @tc.name: OffloadStartPlaying
 * @tc.desc: Audio start offload streaming for hardware encoding datapath.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_OffloadStartPlaying_02, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_OffloadStartPlaying_02 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    vector<int32_t> sessionsId;
    EXPECT_EQ(profile_->OffloadStartPlaying(device, sessionsId), BT_ERR_INVALID_PARAM);
    HILOGI("A2dpSourceTest_UnitTest_OffloadStartPlaying_02 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_OffloadStartPlaying_03
 * @tc.name: OffloadStartPlaying
 * @tc.desc: Audio start offload streaming for hardware encoding datapath.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_OffloadStartPlaying_03, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_OffloadStartPlaying_03 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    vector<int32_t> sessionsId;
    sessionsId.push_back(1);
    EXPECT_NE(profile_->OffloadStartPlaying(device, sessionsId), 0);
    HILOGI("A2dpSourceTest_UnitTest_OffloadStartPlaying_03 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_OffloadStopPlaying_01
 * @tc.name: OffloadStopPlaying
 * @tc.desc: Audio stop offload streaming for active stream.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_OffloadStopPlaying_01, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_OffloadStopPlaying_01 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    vector<int32_t> sessionsId;
    EXPECT_EQ(profile_->OffloadStopPlaying(device, sessionsId), BT_ERR_INVALID_PARAM);
    HILOGI("A2dpSourceTest_UnitTest_OffloadStopPlaying_01 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_OffloadStopPlaying_02
 * @tc.name: OffloadStopPlaying
 * @tc.desc: Audio stop offload streaming for active stream.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_OffloadStopPlaying_02, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_OffloadStopPlaying_02 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    vector<int32_t> sessionsId;
    EXPECT_EQ(profile_->OffloadStopPlaying(device, sessionsId), BT_ERR_INVALID_PARAM);
    HILOGI("A2dpSourceTest_UnitTest_OffloadStopPlaying_02 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_OffloadStopPlaying_03
 * @tc.name: OffloadStopPlaying
 * @tc.desc: Audio stop offload streaming for active stream.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_OffloadStopPlaying_03, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_OffloadStopPlaying_03 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    vector<int32_t> sessionsId;
    sessionsId.push_back(1);
    int ret = profile_->OffloadStopPlaying(device, sessionsId);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpSourceTest_UnitTest_OffloadStopPlaying_03 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_01
 * @tc.name: A2dpOffloadSessionRequest
 * @tc.desc: Get a2dp encoding data path information of connected sink device.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_01, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_01 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    vector<A2dpStreamInfo> info;
    EXPECT_EQ(profile_->A2dpOffloadSessionRequest(device, info), A2DP_STREAM_ENCODE_UNKNOWN);
    HILOGI("A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_01 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_02
 * @tc.name: A2dpOffloadSessionRequest
 * @tc.desc: Get a2dp encoding data path information of connected sink device.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_02, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_02 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    vector<A2dpStreamInfo> info;
    EXPECT_EQ(profile_->A2dpOffloadSessionRequest(device, info), A2DP_STREAM_ENCODE_SOFTWARE);
    HILOGI("A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_02 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_03
 * @tc.name: A2dpOffloadSessionRequest
 * @tc.desc: Get a2dp encoding data path information of connected sink device.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_03, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_03 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    std::vector<A2dpStreamInfo> streamsInfo;
    A2dpStreamInfo stream;
    stream.sessionId = 1;
    stream.streamType = 1;
    streamsInfo.push_back(stream);
    EXPECT_NE(profile_->A2dpOffloadSessionRequest(device, streamsInfo), A2DP_STREAM_ENCODE_HARDWARE);
    HILOGI("A2dpSourceTest_UnitTest_A2dpOffloadSessionRequest_03 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetOffloadCodecStatus_01
 * @tc.name: GetOffloadCodecStatus
 * @tc.desc: Get A2dp Offload codec status information of connected device.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetOffloadCodecStatus_01, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetOffloadCodecStatus_01 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    A2dpOffloadCodecStatus ret = profile_->GetOffloadCodecStatus(device);
    EXPECT_NE(nullptr, &ret);
    HILOGI("A2dpSourceTest_UnitTest_GetOffloadCodecStatus_01 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetOffloadCodecStatus_02
 * @tc.name: GetOffloadCodecStatus
 * @tc.desc: Get A2dp Offload codec status information of connected device.
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetOffloadCodecStatus_02, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetOffloadCodecStatus_02 start");

    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    A2dpOffloadCodecStatus ret = profile_->GetOffloadCodecStatus(device);
    EXPECT_NE(nullptr, &ret);
    HILOGI("A2dpSourceTest_UnitTest_GetOffloadCodecStatus_02 end");
}

/*
 * @tc.number: A2dpSourceTest_UnitTest_GetVirtualDeviceList_01
 * @tc.name: GetVirtualDeviceList
*/
HWTEST_F(A2dpSourceTest, A2dpSourceTest_UnitTest_GetVirtualDeviceList_01, TestSize.Level1)
{
    HILOGI("A2dpSourceTest_UnitTest_GetVirtualDeviceList_01 start");
    profile_ = A2dpSource::GetProfile();
    std::vector<std::string> devices;
    profile_->GetVirtualDeviceList(devices);
    EXPECT_TRUE(devices.empty());
    HILOGI("A2dpSourceTest_UnitTest_GetVirtualDeviceList_01 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_DisableBt_001
 * @tc.name: DisableBt
 * @tc.desc: IS_BT_ENABLED:false
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_DisableBt_001, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_DisableBt_001 start");
    int btState = host_->GetBtState();
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        host_->DisableBle();
        sleep(TIME);
    }
    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->Connect(device), BT_ERR_INVALID_STATE);
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    vector<BluetoothRemoteDevice> devices;
    EXPECT_EQ(profile_->GetDevicesByStates(states, devices), BT_ERR_INVALID_STATE);
    EXPECT_EQ(profile_->GetPlayingState(device, btState), BT_ERR_INVALID_STATE);
    EXPECT_EQ(profile_->SetActiveSinkDevice(device), RET_BAD_STATUS);
    BluetoothRemoteDevice activeSinkDevice = profile_->GetActiveSinkDevice();
    EXPECT_NE(&activeSinkDevice, nullptr);
    EXPECT_EQ(profile_->SetConnectStrategy(device, 2), BT_ERR_INVALID_STATE);
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_ERR_INVALID_STATE);
    A2dpCodecStatus ret = profile_->GetCodecStatus(device);
    EXPECT_NE(&ret, nullptr);
    HILOGI("A2dpSource_UnitTest_DisableBt_001 end");
}

/*
 * @tc.number: A2dpSource_UnitTest_DisableBt_002
 * @tc.name: DisableBt
 * @tc.desc: IS_BT_ENABLED:false
*/
HWTEST_F(A2dpSourceTest, A2dpSource_UnitTest_DisableBt_002, TestSize.Level1)
{
    HILOGI("A2dpSource_UnitTest_DisableBt_002 start");
    int btState = host_->GetBtState();
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        host_->DisableBle();
        sleep(TIME);
    }
    profile_ = A2dpSource::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    A2dpCodecInfo info;
    EXPECT_EQ(profile_->SetCodecPreference(device, info), BT_ERR_INVALID_STATE);
    EXPECT_EQ(profile_->GetOptionalCodecsSupportState(device), RET_BAD_STATUS);
    profile_->SwitchOptionalCodecs(device, true);
    EXPECT_EQ(profile_->StartPlaying(device), RET_BAD_STATUS);
    observer_->OnPlayingStatusChanged(device, 0, 0);
    EXPECT_EQ(profile_->SuspendPlaying(device), RET_BAD_STATUS);
    EXPECT_EQ(profile_->StopPlaying(device), RET_BAD_STATUS);
    uint8_t str[4] = {'1', '2', '3', '4'};
    EXPECT_EQ(profile_->WriteFrame(str, 4), RET_BAD_STATUS);
    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;
    EXPECT_EQ(profile_->GetRenderPosition(device, delayValue, sendDataSize, timeStamp), RET_BAD_STATUS);
    int state = 0;
    EXPECT_EQ(profile_->GetDeviceState(device, state), BT_ERR_INVALID_STATE);
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_INVALID_STATE);
    HILOGI("A2dpSource_UnitTest_DisableBt_002 end");
    sleep(TIME);
}
}  // namespace Bluetooth
}  // namespace OHOS