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
#include "a2dp_src/a2dp_src_service.h"
#include "bluetooth_a2dp_source_server.h"
#include "bluetooth_a2dp_source_server.cpp"
#include "bluetooth_a2dp_src_observer_stub.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "class_creator.h"
#include "profile_service_manager.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;

class BluetoothA2dpSrcObserverStubTest : public BluetoothA2dpSrcObserverStub {
public:
    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {}
    void OnCaptureConnectionStateChanged(const RawAddress &device, int state, const BluetoothA2dpCodecInfo &info) override
    {}
    void OnPlayingStatusChanged(const RawAddress &device, int playingState, int error) override
    {}
    void OnConfigurationChanged(const RawAddress &device, const BluetoothA2dpCodecInfo &info, int error) override
    {}
    void OnMediaStackChanged(const RawAddress &device, int action) override
    {}
    void OnVirtualDeviceChanged(int32_t action, std::string address) override
    {}
    explicit BluetoothA2dpSrcObserverStubTest()
    {}
    ~BluetoothA2dpSrcObserverStubTest() override
    {}

private:
};

class A2dpSourceServerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
    }
    void SetUp();
    void TearDown();
    void CallbackStatusChange();
public:
    std::unique_ptr<BluetoothA2dpSourceServer> a2dpSrcServer_;
    BluetoothA2dpSourceServer::impl *pimpl_;
    RawAddress addr_;
    RawAddress addrFailed_ = RawAddress("22:33:33:55:55:55");
};

void A2dpSourceServerTest::SetUp()
{
    a2dpSrcServer_ = std::make_unique<BluetoothA2dpSourceServer>();
    addr_ = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(RawAddress("11:22:33:44:55:66"));
}

void A2dpSourceServerTest::TearDown()
{
    a2dpSrcServer_ = nullptr;
    utility::CloseAllTimer();
}

void A2dpSourceServerTest::CallbackStatusChange()
{
    a2dpSrcServer_->pimpl->observerImp_->OnConnectionStateChanged(addr_, 1, 0);
    a2dpSrcServer_->pimpl->observerImp_->OnConnectionStateChanged(addr_, 3, 0);
    a2dpSrcServer_->pimpl->observerImp_->OnPlayingStatusChaned(addr_, 1, 0);
    a2dpSrcServer_->pimpl->observerImp_->OnMediaStackChanged(addr_, 0);
    a2dpSrcServer_->pimpl->observerImp_->OnVirtualDeviceChanged(0, "11:22:33:44:55:66");
    A2dpSrcCodecInfo info;
    a2dpSrcServer_->pimpl->observerImp_->OnConfigurationChanged(addr_, info, 0);
    a2dpSrcServer_->pimpl->observerImp_->OnCaptureConnectionStateChanged(addr_, 0, info);
}

/**
 * @tc.name: a2dp_src_server_register_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_register_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_register_001 enter");
    sptr<BluetoothA2dpSrcObserverStubTest> observer = nullptr;
    a2dpSrcServer_->RegisterObserver(observer);
    a2dpSrcServer_->DeregisterObserver(observer);

    observer = new BluetoothA2dpSrcObserverStubTest();
    a2dpSrcServer_->RegisterObserver(observer);
    a2dpSrcServer_->pimpl->systemStateObserver_->OnSystemStateChange(BTSystemState::ON);
    a2dpSrcServer_->pimpl->systemStateObserver_->OnSystemStateChange(BTSystemState::OFF);
    CallbackStatusChange();
    a2dpSrcServer_->DeregisterObserver(observer);
    EXPECT_TRUE(a2dpSrcServer_ != NULL);
    HILOGI("a2dp_src_server_register_001 end");
}

/**
 * @tc.name: a2dp_src_server_connect_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_connect_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_connect_001 enter");
    int32_t ret = a2dpSrcServer_->Connect(addr_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    CallbackStatusChange();
    ret = a2dpSrcServer_->Disconnect(addr_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    ret = a2dpSrcServer_->Connect(addrFailed_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    CallbackStatusChange();
    ret = a2dpSrcServer_->Disconnect(addrFailed_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_connect_001 end");
}

/**
 * @tc.name: a2dp_src_server_device_state_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_device_state_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_device_state_001 enter");
    int state;
    int ret = a2dpSrcServer_->GetDeviceState(addr_, state);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->GetDeviceState(addrFailed_, state);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    std::vector<int32_t> states;
    std::vector<RawAddress> rawAddrs;
    ret = a2dpSrcServer_->GetDevicesByStates(states, rawAddrs);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_device_state_001 end");
}

/**
 * @tc.name: a2dp_src_server_get_playing_state_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_get_playing_state_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_get_playing_state_001 enter");
    int state;
    int32_t ret = a2dpSrcServer_->GetPlayingState(addr_, state);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->GetPlayingState(addrFailed_, state);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_get_playing_state_001 end");
}

/**
 * @tc.name: a2dp_src_server_connect_strategy_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_connect_strategy_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_connect_strategy_001 enter");
    int ret = a2dpSrcServer_->SetConnectStrategy(addr_, 1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    int strategy;
    ret = a2dpSrcServer_->GetConnectStrategy(addr_, strategy);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    ret = a2dpSrcServer_->SetConnectStrategy(addrFailed_, 1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->GetConnectStrategy(addrFailed_, strategy);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_connect_strategy_001 end");
}

/**
 * @tc.name: a2dp_src_server_active_sink_device_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_active_sink_device_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_active_sink_device_001 enter");
    int ret = a2dpSrcServer_->SetActiveSinkDevice(addr_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    RawAddress address = a2dpSrcServer_->GetActiveSinkDevice();

    ret = a2dpSrcServer_->SetActiveSinkDevice(addrFailed_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_active_sink_device_001 end");
}

/**
 * @tc.name: a2dp_src_server_set_codec_preference_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_set_codec_preference_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_set_codec_preference_001 enter");
    BluetoothA2dpCodecInfo info;
    a2dpSrcServer_->GetCodecStatus(addr_);
    int ret = a2dpSrcServer_->SetCodecPreference(addr_, info);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    a2dpSrcServer_->GetCodecStatus(addrFailed_);
    ret = a2dpSrcServer_->SetCodecPreference(addrFailed_, info);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_set_codec_preference_001 end");
}

/**
 * @tc.name: a2dp_src_server_get_optional_codecs_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_get_optional_codecs_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_get_optional_codecs_001 enter");
    a2dpSrcServer_->SwitchOptionalCodecs(addr_, true);
    int ret = a2dpSrcServer_->GetOptionalCodecsSupportState(addr_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    a2dpSrcServer_->SwitchOptionalCodecs(addrFailed_, true);
    ret = a2dpSrcServer_->GetOptionalCodecsSupportState(addrFailed_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_get_optional_codecs_001 end");
}

/**
 * @tc.name: a2dp_src_server_start_playing_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_start_playing_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_start_playing_001 enter");
    int ret = a2dpSrcServer_->StartPlaying(addr_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->SuspendPlaying(addr_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->StopPlaying(addr_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    ret = a2dpSrcServer_->StartPlaying(addrFailed_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->SuspendPlaying(addrFailed_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->StopPlaying(addrFailed_);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_start_playing_001 end");
}

/**
 * @tc.name: a2dp_src_server_write_frame_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_write_frame_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_write_frame_001 enter");
    uint8_t data;
    int ret = a2dpSrcServer_->WriteFrame(&data, 1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_write_frame_001 end");
}

/**
 * @tc.name: a2dp_src_server_get_render_position_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_get_render_position_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_get_render_position_001 enter");
    uint32_t delayValue;
    uint64_t sendDataSize;
    uint32_t timeStamp;
    int ret = a2dpSrcServer_->GetRenderPosition(addr_, delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_get_render_position_001 end");
}

/**
 * @tc.name: a2dp_src_server_offload_start_playing_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_offload_start_playing_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_offload_start_playing_001 enter");
    std::vector<int32_t> sessionsId;
    sessionsId.push_back(1);
    RawAddress addr = RawAddress("00:00:00:00:00:00");
    int ret = a2dpSrcServer_->OffloadStartPlaying(addr, sessionsId);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->OffloadStartPlaying(addr_, sessionsId);
    EXPECT_NE(ret, 0);
    HILOGI("a2dp_src_server_offload_start_playing_001 end ret:%{public}d", ret);
}

/**
 * @tc.name: a2dp_src_server_offload_stop_playing_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_offload_stop_playing_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_offload_stop_playing_001 enter");
    std::vector<int32_t> sessionsId;
    sessionsId.push_back(1);
    RawAddress addr = RawAddress("00:00:00:00:00:00");
    int ret = a2dpSrcServer_->OffloadStopPlaying(addr, sessionsId);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->OffloadStopPlaying(addr_, sessionsId);
    EXPECT_NE(ret, 0);
    HILOGI("a2dp_src_server_offload_stop_playing_001 end ret:%{public}d", ret);
}

/**
 * @tc.name: a2dp_src_server_offload_session_path_request_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_offload_session_path_request_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_offload_session_path_request_001 enter");
    std::vector<int32_t> sessionsId;
    std::vector<BluetoothA2dpStreamInfo> infos;
    BluetoothA2dpStreamInfo stream;
    stream.sessionId = 1;
    stream.sampleRate = 0;
    stream.streamType = 15;
    stream.isSpatialAudio = false;
    infos.push_back(stream);
    RawAddress addr = RawAddress("00:00:00:00:00:00");
    int ret = a2dpSrcServer_->A2dpOffloadSessionPathRequest(addr, infos);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ret = a2dpSrcServer_->A2dpOffloadSessionPathRequest(addr_, infos);
    EXPECT_NE(ret, 0);
    HILOGI("a2dp_src_server_offload_session_path_request_001 end ret:%{public}d", ret);
}

/**
 * @tc.name: a2dp_src_server_offload_session_path_request_002
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_offload_session_path_request_002, TestSize.Level1)
{
    HILOGI("a2dp_src_server_offload_session_path_request_002 enter");
    std::vector<BluetoothA2dpStreamInfo> infos;
    BluetoothA2dpStreamInfo stream;
    stream.sessionId = 1;
    stream.sampleRate = 0;
    stream.streamType = 1;
    stream.isSpatialAudio = false;
    infos.push_back(stream);
    int ret = a2dpSrcServer_->A2dpOffloadSessionPathRequest(addr_, infos);
    EXPECT_NE(ret, 0);
    HILOGI("a2dp_src_server_offload_session_path_request_002 end");
}

/**
 * @tc.name: a2dp_src_server_offload_session_path_request_003
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_offload_session_path_request_003, TestSize.Level1)
{
    HILOGI("a2dp_src_server_offload_session_path_request_003 enter");
    std::vector<BluetoothA2dpStreamInfo> infos;
    BluetoothA2dpStreamInfo stream;
    stream.sessionId = 1;
    stream.sampleRate = 0;
    stream.streamType = 1;
    stream.isSpatialAudio = false;
    infos.push_back(stream);
    stream.sessionId = 2;
    stream.streamType = 15;
    infos.push_back(stream);
    int ret = a2dpSrcServer_->A2dpOffloadSessionPathRequest(addr_, infos);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_offload_session_path_request_003 end");
}

/**
 * @tc.name: a2dp_src_server_write_frame_002
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_write_frame_002, TestSize.Level1)
{
    HILOGI("a2dp_src_server_write_frame_002 enter");
    uint8_t data;
    int ret = a2dpSrcServer_->WriteFrame(&data, 1);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_write_frame_002 end");
}

/**
 * @tc.name: a2dp_src_server_get_render_position_002
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_get_render_position_002, TestSize.Level1)
{
    HILOGI("a2dp_src_server_get_render_position_002 enter");
    uint32_t delayValue;
    uint64_t sendDataSize;
    uint32_t timeStamp;
    int ret = a2dpSrcServer_->GetRenderPosition(addr_, delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("a2dp_src_server_get_render_position_002 end");
}

/**
 * @tc.name: a2dp_src_server_get_codec_preference_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_get_codec_preference_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_get_codec_preference_001 enter");
    BluetoothA2dpCodecInfo info;
    int ret = a2dpSrcServer_->GetCodecPreference(addr_, info);
    EXPECT_NE(ret, 0);
    HILOGI("a2dp_src_server_get_codec_preference_001 end");
}

/**
 * @tc.name: a2dp_src_server_enable_auto_play_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_enable_auto_play_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_enable_auto_play_001 enter");
    int ret = a2dpSrcServer_->EnableAutoPlay(addr_);
    EXPECT_NE(ret, 0);
    HILOGI("a2dp_src_server_enable_auto_play_001 end");
}

/**
 * @tc.name: a2dp_src_server_disable_auto_play_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_disable_auto_play_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_disable_auto_play_001 enter");
    int duration = 3000;
    int ret = a2dpSrcServer_->DisableAutoPlay(addr_, duration);
    EXPECT_NE(ret, 0);
    HILOGI("a2dp_src_server_disable_auto_play_001 end");
}

/**
 * @tc.name: a2dp_src_server_get_auto_play_disabled_duration_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_get_auto_play_disabled_duration_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_get_auto_play_disabled_duration_001 enter");
    int duration;
    int ret = a2dpSrcServer_->GetAutoPlayDisabledDuration(addr_, duration);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(duration, 0);
    HILOGI("a2dp_src_server_get_auto_play_disabled_duration_001 end");
}

static void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

/**
 * @tc.name: a2dp_src_server_service_enable_001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_service_enable_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_service_enable_001 enter");
    // Create service
    ClassFactory::RegisterClass(PROFILE_NAME_A2DP_SRC, A2dpSrcServiceCreate);
    auto manager = ProfileServiceManager::GetInstance();
    string name = PROFILE_NAME_A2DP_SRC;
    manager->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, name);
    a2dpSrcServer_ = std::make_unique<BluetoothA2dpSourceServer>();

    sptr<BluetoothA2dpSrcObserverStubTest> observer = new BluetoothA2dpSrcObserverStubTest();
    a2dpSrcServer_->RegisterObserver(observer);
    a2dpSrcServer_->DeregisterObserver(observer);
    a2dpSrcServer_->Connect(addr_);
    a2dpSrcServer_->Disconnect(addr_);

    int state;
    a2dpSrcServer_->GetDeviceState(addr_, state);

    std::vector<int32_t> states;
    std::vector<RawAddress> rawAddrs;
    a2dpSrcServer_->GetDevicesByStates(states, rawAddrs);
    a2dpSrcServer_->GetPlayingState(addr_, state);
    a2dpSrcServer_->SetConnectStrategy(addr_, 1);

    int strategy;
    a2dpSrcServer_->GetConnectStrategy(addr_, strategy);
    a2dpSrcServer_->SetActiveSinkDevice(addr_);
    a2dpSrcServer_->GetActiveSinkDevice();
    a2dpSrcServer_->GetCodecStatus(addr_);

    BluetoothA2dpCodecInfo info;
    a2dpSrcServer_->SetCodecPreference(addr_, info);
    a2dpSrcServer_->GetOptionalCodecsSupportState(addr_);
    a2dpSrcServer_->StartPlaying(addr_);
    a2dpSrcServer_->SuspendPlaying(addr_);
    a2dpSrcServer_->StopPlaying(addr_);

    uint8_t data = 0;
    a2dpSrcServer_->WriteFrame(&data, 1);

    uint32_t delayValue;
    uint64_t sendDataSize;
    uint32_t timeStamp;
    int ret = a2dpSrcServer_->GetRenderPosition(addr_, delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, BT_ERR_DEVICE_DISCONNECTED);
    ret = a2dpSrcServer_->GetRenderPosition(addr_, delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, BT_ERR_DEVICE_DISCONNECTED);

    HILOGI("a2dp_src_server_service_enable_001 end");
}

/**
 * @tc.name: a2dp_src_server_GetVirtualDeviceList_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_GetVirtualDeviceList_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_GetVirtualDeviceList_001 enter");
    BluetoothA2dpCodecInfo info;
    OHOS::Parcel parcel;
    info.WriteToParcel(parcel);
    std::vector<std::string> devices;
    a2dpSrcServer_->GetVirtualDeviceList(devices);
    EXPECT_TRUE(devices.empty());
    HILOGI("a2dp_src_server_GetVirtualDeviceList_001 end");
}

/*
 * @tc.number: a2dp_src_server_OnConnectionStateChangedInner_001
 * @tc.name: OnConnectionStateChangedInner
 * @tc.desc: Check OnConnectionStateChangedInner interface
*/
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_OnConnectionStateChangedInner_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_OnConnectionStateChangedInner_001 start");
    BluetoothA2dpSrcObserverStubTest bluetoothA2dpSrcObserverStubTest;
    MessageParcel data;
    MessageParcel reply;

    int32_t ret = BluetoothA2dpSrcObserverStub::OnConnectionStateChangedInner(&bluetoothA2dpSrcObserverStubTest,
        data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("a2dp_src_server_OnConnectionStateChangedInner_001 end");
}

/*
 * @tc.number: a2dp_src_server_OnPlayingStatusChangedInner_001
 * @tc.name: OnPlayingStatusChangedInner
 * @tc.desc: Check OnPlayingStatusChangedInner interface
*/
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_OnPlayingStatusChangedInner_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_OnPlayingStatusChangedInner_001 start");
    BluetoothA2dpSrcObserverStubTest bluetoothA2dpSrcObserverStubTest;
    MessageParcel data;
    MessageParcel reply;

    int32_t ret = BluetoothA2dpSrcObserverStub::OnPlayingStatusChangedInner(&bluetoothA2dpSrcObserverStubTest,
        data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("a2dp_src_server_OnPlayingStatusChangedInner_001 end");
}

/*
 * @tc.number: a2dp_src_server_OnConfigurationChangedInner_001
 * @tc.name: OnConfigurationChangedInner
 * @tc.desc: Check OnConfigurationChangedInner interface
*/
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_OnConfigurationChangedInner_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_OnConfigurationChangedInner_001 start");
    BluetoothA2dpSrcObserverStubTest bluetoothA2dpSrcObserverStubTest;
    MessageParcel data;
    MessageParcel reply;

    int32_t ret = BluetoothA2dpSrcObserverStub::OnConfigurationChangedInner(&bluetoothA2dpSrcObserverStubTest,
        data, reply);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("a2dp_src_server_OnConfigurationChangedInner_001 end");
}

/*
 * @tc.number: a2dp_src_server_OnMediaStackChangedInner_001
 * @tc.name: OnMediaStackChangedInner
 * @tc.desc: Check OnMediaStackChangedInner interface
*/
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_OnMediaStackChangedInner_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_OnMediaStackChangedInner_001 start");
    BluetoothA2dpSrcObserverStubTest bluetoothA2dpSrcObserverStubTest;
    MessageParcel data;
    MessageParcel reply;

    int32_t ret = BluetoothA2dpSrcObserverStub::OnMediaStackChangedInner(&bluetoothA2dpSrcObserverStubTest,
        data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("a2dp_src_server_OnMediaStackChangedInner_001 end");
}

/*
 * @tc.number: a2dp_src_server_OnVirtualDeviceChangedInner_001
 * @tc.name: OnVirtualDeviceChangedInner
 * @tc.desc: Check OnConnectionStateChangedInner interface
*/
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_OnVirtualDeviceChangedInner_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_OnVirtualDeviceChangedInner_001 start");
    BluetoothA2dpSrcObserverStubTest bluetoothA2dpSrcObserverStubTest;
    MessageParcel data;
    MessageParcel reply;

    ErrCode ret = BluetoothA2dpSrcObserverStub::OnVirtualDeviceChangedInner(&bluetoothA2dpSrcObserverStubTest,
        data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("a2dp_src_server_OnVirtualDeviceChangedInner_001 end");
}

/*
 * @tc.number: a2dp_src_server_OnCaptureConnectionStateChangedInner_001
 * @tc.name: OnCaptureConnectionStateChangedInner
 * @tc.desc: Check OnCaptureConnectionStateChangedInner interface
*/
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_OnCaptureConnectionStateChangedInner_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_OnCaptureConnectionStateChangedInner_001 start");
    BluetoothA2dpSrcObserverStubTest bluetoothA2dpSrcObserverStubTest;
    MessageParcel data;
    MessageParcel reply;

    ErrCode ret = BluetoothA2dpSrcObserverStub::OnCaptureConnectionStateChangedInner(&bluetoothA2dpSrcObserverStubTest,
        data, reply);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("a2dp_src_server_OnCaptureConnectionStateChangedInner_001 end");
}

/**
 * @tc.name: a2dp_src_server_BluetoothA2dpCodecInfo_001
 * @tc.desc: A2dpSourceServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSourceServerTest, a2dp_src_server_BluetoothA2dpCodecInfo_001, TestSize.Level1)
{
    HILOGI("a2dp_src_server_BluetoothA2dpCodecInfo_001 enter");
    BluetoothA2dpCodecInfo info;
    OHOS::Parcel parcel;
    EXPECT_TRUE(info.Marshalling(parcel));
    EXPECT_TRUE(info.Unmarshalling(parcel));
    EXPECT_TRUE(info.WriteToParcel(parcel));
    EXPECT_TRUE(info.ReadFromParcel(parcel));
    HILOGI("a2dp_src_server_BluetoothA2dpCodecInfo_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS