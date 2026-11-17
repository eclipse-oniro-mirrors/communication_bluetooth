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

#include "avrc_defs.h"
#include "avrcp_tg_avsession_media.h"
#include "avrcp_tg_avsession_media.cpp"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "class_creator.h"
#include "avrcp_tg_service.h"
#include "play_helper.cpp"
#include "btcommon/timer_manager.h"

#define AVRC_ID_VENDOR 0x7E     /* vendor unique */

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
static BluetoothHost *host_;
static AvrcpMediaInterfaceImpl g_mediaInterface;
 
class AvrcpTgAvSessionMediaTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);
 
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);
 
        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
        std::shared_ptr<AvrcpMediaInterfaceImpl::impl> impl1 = std::make_shared<AvrcpMediaInterfaceImpl::impl>();
        std::weak_ptr<AvrcpMediaInterfaceImpl::impl> impl2 = impl1;
        g_mediaInterface.pimpl_->Init(impl2);
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
    }
    void SetUp();
    void TearDown();
};
 
void AvrcpTgAvSessionMediaTest::SetUp()
{
}
 
void AvrcpTgAvSessionMediaTest::TearDown()
{
}
 
void MockSongInfoCallback(SongInfo info)
{
    HILOGI("enter");
    return;
}
 
void MockPlayingStatusCallback(PlayStatus status)
{
    HILOGI("enter");
    return;
}
 
void MockNowPlayingCallback(std::string str, std::vector<SongInfo> info)
{
    HILOGI("enter");
    return;
}
 
void MockMediaListCallback(uint16_t curPlayer, std::vector<MediaPlayerInfo> info)
{
    HILOGI("enter");
    return;
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_UnitTest_SendKeyEvent_001
 * @tc.desc: SendKeyEvent
 * @tc.type: SendKeyEvent
 *           PS: could not mock OHOS::AVSession::AVSessionManager::GetInstance().SendSystemControlCommand(command)
 *               because it could not provide a SetInstance() interface.
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_UnitTest_SendKeyEvent_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_UnitTest_SendKeyEvent_001 enter");
    // state is KeyState::PUSHED
    g_mediaInterface.SendKeyEvent(AVRC_ID_VENDOR, KeyState::PUSHED);
    g_mediaInterface.GetSongInfo(std::bind(MockSongInfoCallback, std::placeholders::_1));
    g_mediaInterface.GetPlayStatus(std::bind(MockPlayingStatusCallback, std::placeholders::_1));
    std::vector<SongInfo> songs;
    g_mediaInterface.GetNowPlayingList(std::bind(MockNowPlayingCallback, std::placeholders::_1, std::placeholders::_2));
    g_mediaInterface.GetMediaPlayerList(std::bind(MockMediaListCallback, std::placeholders::_1, std::placeholders::_2));
    g_mediaInterface.PlayItem(0, false, "001");
    int32_t avState = 2;
    int a2dpPlayState = 0;
    bool isInDenyList = true;
    EXPECT_TRUE(IsNeedChangePlayState(avState, a2dpPlayState, isInDenyList));
    HILOGI("AvrcpTgAvSessionMediaTest_UnitTest_SendKeyEvent_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_ConvertAvSessionStateToStack_001
 * @tc.desc: ConvertAvSessionStateToStack
 * @tc.type: avSessionController_ is null
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_ConvertAvSessionStateToStack_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_ConvertAvSessionStateToStack_001 enter");
    using AVSession::AVPlaybackState;
    auto impl = g_mediaInterface.pimpl_;
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_INITIAL), PlayState::STOPPED);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_PREPARE), PlayState::ERROR);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_MAX), PlayState::ERROR);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_PLAY), PlayState::PLAYING);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_PAUSE), PlayState::PAUSED);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_FAST_FORWARD), PlayState::FWD_SEEK);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_REWIND), PlayState::REV_SEEK);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_STOP), PlayState::STOPPED);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_ERROR), PlayState::ERROR);
    EXPECT_EQ(impl->ConvertAvSessionStateToStack(AVPlaybackState::PLAYBACK_STATE_BUFFERING), PlayState::PLAYING);
    HILOGI("AvrcpTgAvSessionMediaTest_ConvertAvSessionStateToStack_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_ConvertAvSessionLoopModeToStack_001
 * @tc.desc: ConvertAvSessionLoopModeToStack
 * @tc.type:
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_ConvertAvSessionLoopModeToStack_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_ConvertAvSessionLoopModeToStack_001 enter");
    AVSession::AVPlaybackState state;
    uint8_t shuffleMode;
    uint8_t repeatMode;
    state.SetLoopMode(AVSession::AVPlaybackState::LOOP_MODE_SEQUENCE);
    g_mediaInterface.pimpl_->ConvertAvSessionLoopModeToStack(state, shuffleMode, repeatMode);
    EXPECT_EQ(shuffleMode, SHUFFLE_OFF);
    EXPECT_EQ(repeatMode, REPEAT_OFF);
 
    state.SetLoopMode(AVSession::AVPlaybackState::LOOP_MODE_SINGLE);
    g_mediaInterface.pimpl_->ConvertAvSessionLoopModeToStack(state, shuffleMode, repeatMode);
    EXPECT_EQ(shuffleMode, SHUFFLE_OFF);
    EXPECT_EQ(repeatMode, REPEAT_SINGLE_TRACK);
 
    state.SetLoopMode(AVSession::AVPlaybackState::LOOP_MODE_LIST);
    g_mediaInterface.pimpl_->ConvertAvSessionLoopModeToStack(state, shuffleMode, repeatMode);
    EXPECT_EQ(shuffleMode, SHUFFLE_OFF);
    EXPECT_EQ(repeatMode, REPEAT_ALL_TRACKS);
 
    state.SetLoopMode(AVSession::AVPlaybackState::LOOP_MODE_SHUFFLE);
    g_mediaInterface.pimpl_->ConvertAvSessionLoopModeToStack(state, shuffleMode, repeatMode);
    EXPECT_EQ(shuffleMode, SHUFFLE_ALL_TRACKS);
    EXPECT_EQ(repeatMode, REPEAT_ALL_TRACKS);
    HILOGI("AvrcpTgAvSessionMediaTest_ConvertAvSessionLoopModeToStack_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_GetCurrSongInfo_001
 * @tc.desc: GetCurrSongInfo
 * @tc.type:
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_GetCurrSongInfo_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_GetCurrSongInfo_001 enter");
    SongInfo song;
    auto impl = g_mediaInterface.pimpl_;
    EXPECT_EQ(impl->GetCurrSongInfo(song), false);
    AVSession::AVMetaData data;
    data.SetSingleLyricText("123");
    EXPECT_EQ(impl->GetCurrSongInfo(song), false);
    HILOGI("AvrcpTgAvSessionMediaTest_GetCurrSongInfo_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_CheckTrackStateChanged_001
 * @tc.desc: CheckTrackStateChanged
 * @tc.type:
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_CheckTrackStateChanged_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_CheckTrackStateChanged_001 enter");
    AVSession::AVMetaData data;
    data.SetAssetId("123");
    data.SetArtist("123");
    data.SetAlbum("123");
    std::shared_ptr<AVSession::AVSessionPixelMap> pixelMap1 = std::make_shared<AVSession::AVSessionPixelMap>();
    std::vector<uint8_t> imgBuffer1 = {1, 1, 0, 1, 1};
    pixelMap1->SetInnerImgBuffer(imgBuffer1);
    data.SetMediaImage(pixelMap1);
 
    auto impl = g_mediaInterface.pimpl_;
    impl->currMateData_.SetAssetId("123");
    impl->currMateData_.SetArtist("123");
    impl->currMateData_.SetAlbum("123");
    std::shared_ptr<AVSession::AVSessionPixelMap> pixelMap2 = std::make_shared<AVSession::AVSessionPixelMap>();
    std::vector<uint8_t> imgBuffer2 = {1, 1, 0, 1, 1};
    pixelMap2->SetInnerImgBuffer(imgBuffer2);
    impl->currMateData_.SetMediaImage(pixelMap2);
    EXPECT_EQ(impl->CheckTrackStateChanged(data), false);
 
    data.SetAlbum("1234");
    EXPECT_EQ(impl->CheckTrackStateChanged(data), true);
    data.SetArtist("1234");
    EXPECT_EQ(impl->CheckTrackStateChanged(data), true);
    data.SetAssetId("1234");
    EXPECT_EQ(impl->CheckTrackStateChanged(data), true);
    data.SetTitle("1234");
    EXPECT_EQ(impl->CheckTrackStateChanged(data), true);
    data.SetSingleLyricText("1234");
    EXPECT_EQ(impl->CheckTrackStateChanged(data), true);
    AVSession::AVMetaData datas;
    EXPECT_EQ(impl->CheckTrackStateChanged(datas), true);
    datas.SetSingleLyricText("12345");
    EXPECT_EQ(impl->CheckTrackStateChanged(datas), true);
    data.SetAlbum("");
    EXPECT_EQ(impl->CheckTrackStateChanged(data), true);

    std::shared_ptr<AVSession::AVSessionPixelMap> pixelMap3 = std::make_shared<AVSession::AVSessionPixelMap>();
    std::vector<uint8_t> imgBuffer3 = {1, 0, 0, 1, 0};
    pixelMap3->SetInnerImgBuffer(imgBuffer3);
    data.SetMediaImage(pixelMap3);
    EXPECT_EQ(impl->CheckTrackStateChanged(data), true);
    HILOGI("AvrcpTgAvSessionMediaTest_CheckTrackStateChanged_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_CheckPlaybackStateChanged_001
 * @tc.desc: CheckPlaybackStateChanged
 * @tc.type:
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_CheckPlaybackStateChanged_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_CheckPlaybackStateChanged_001 enter");
    AVSession::AVPlaybackState state;
    state.SetState(2);
 
    auto impl = g_mediaInterface.pimpl_;
    impl->currPlaybackstate_.SetState(2);
    EXPECT_EQ(impl->CheckPlaybackStateChanged(state), false);
 
    state.SetState(3);
    EXPECT_EQ(impl->CheckPlaybackStateChanged(state), true);
 
    AVSession::AVPlaybackState states;
    EXPECT_EQ(impl->CheckPlaybackStateChanged(states), false);
    HILOGI("AvrcpTgAvSessionMediaTest_CheckPlaybackStateChanged_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_CheckLoopModeChanged_001
 * @tc.desc: CheckLoopModeChanged
 * @tc.type:
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_CheckLoopModeChanged_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_CheckLoopModeChanged_001 enter");
    AVSession::AVPlaybackState state;
    state.SetLoopMode(2);
 
    auto impl = g_mediaInterface.pimpl_;
    impl->currPlaybackstate_.SetLoopMode(2);
    EXPECT_EQ(impl->CheckLoopModeChanged(state), false);
 
    state.SetLoopMode(3);
    EXPECT_EQ(impl->CheckLoopModeChanged(state), true);
 
    AVSession::AVPlaybackState states;
    EXPECT_EQ(impl->CheckLoopModeChanged(states), false);
    HILOGI("AvrcpTgAvSessionMediaTest_CheckLoopModeChanged_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_SetAppSettingsMode_001
 * @tc.desc: SetAppSettingsMode
 * @tc.type:
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_SetAppSettingsMode_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_SetAppSettingsMode_001 enter");
    auto impl = g_mediaInterface.pimpl_;
    EXPECT_TRUE(impl != nullptr);
    uint8_t shuffleMode = SHUFFLE_QUERY;
    uint8_t repeatMode = REPEAT_ALL_TRACKS;
    impl->SetAppSettingsMode(shuffleMode, repeatMode);
    repeatMode = REPEAT_OFF;
    impl->SetAppSettingsMode(shuffleMode, repeatMode);
    impl->SendKeyEventByWearDetection(AVRC_ID_PLAY, KeyState::PUSHED);
    impl->SendKeyEventByWearDetection(AVRC_ID_VENDOR, KeyState::RELEASED);
    impl->RegisterAvSessionListener();
    impl->AttemptCreateAvSessionController();
    impl->PlaySilenceSound();
    impl->NotifyA2dpPlayStateChange();
    impl->QueryAppSettingsMode();
    AVSession::AVPlaybackState state;
    state.SetLoopMode(AVSession::AVPlaybackState::LOOP_MODE_SINGLE);
    impl->OnPlaybackStateChange(state);
    state.SetLoopMode(AVSession::AVPlaybackState::LOOP_MODE_UNDEFINED);
    impl->OnPlaybackStateChange(state);
    AVSession::AVSessionDescriptor des;
    des.sessionId_ = "";
    des.isTopSession_ = false;
    impl->OnTopSessionChange(des);
    AVSession::AVMetaData data;
    data.SetAssetId("1");
    impl->OnMetaDataChange(data);
    impl->OnMetaDataChange(data);
    impl->OnSessionRelease(des);
    HILOGI("AvrcpTgAvSessionMediaTest_SetAppSettingsMode_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_SendKeyEventByWearDetection_001
 * @tc.desc: SendKeyEventByWearDetection
 * @tc.type:
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_SendKeyEventByWearDetection_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_SendKeyEventByWearDetection_001 enter");
    auto impl = g_mediaInterface.pimpl_;
    EXPECT_TRUE(impl != nullptr);
    impl->SendKeyEventByWearDetection(AVRC_ID_PLAY, KeyState::RELEASED);
    impl->RegisterAvSessionListener();
    impl->AttemptCreateAvSessionController();
    impl->CreateAvSessionController("123");
    AVSession::AVSessionDescriptor des;
    des.sessionId_ = "123";
    des.isTopSession_ = true;
    impl->OnTopSessionChange(des);
    impl->SendKeyEventByWearDetection(AVRC_ID_PAUSE, KeyState::RELEASED);
    impl->SendKeyEventByWearDetection(AVRC_ID_PLAY, KeyState::RELEASED);
    des.isTopSession_ = false;
    impl->SendKeyEventByWearDetection(AVRC_ID_PAUSE, KeyState::RELEASED);
    impl->SendKeyEventByWearDetection(AVRC_ID_PLAY, KeyState::RELEASED);
    EXPECT_FALSE(impl->IsTopSessionInCast());
    AVSession::DeviceInfo deviceInfo;
    deviceInfo.castCategory_ = AVSession::AVCastCategory::CATEGORY_REMOTE;
    des.outputDeviceInfo_.deviceInfos_.push_back(deviceInfo);
    impl->IsTopSessionInCast();
    impl->SubscribeAvsessionSystemAbility();
    EXPECT_NE(nullptr, impl->avsessionStatusChangeListener_);
    HILOGI("AvrcpTgAvSessionMediaTest_SendKeyEventByWearDetection_001 end");
}
 
/**
 * @tc.name: AvrcpTgAvSessionMediaTest_SendKeyEventByWearDetection_002
 * @tc.desc: SendKeyEventByWearDetection
 * @tc.type:
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_SendKeyEventByWearDetection_002, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_SendKeyEventByWearDetection_002 enter");
    auto impl = g_mediaInterface.pimpl_;
    EXPECT_TRUE(impl != nullptr);
    impl->RegisterAvSessionListener();
    impl->AttemptCreateAvSessionController();
    impl->CreateAvSessionController("123");
    AVSession::AVSessionDescriptor des;
    des.sessionId_ = "123";
    des.isTopSession_ = true;
    impl->OnTopSessionChange(des);
    impl->SendKeyEventByWearDetection(AVRC_ID_PAUSE, KeyState::RELEASED);
    impl->CreateAvSessionController("456");
    des.sessionId_ = "456";
    des.isTopSession_ = true;
    impl->OnTopSessionChange(des);
    impl->SendKeyEventByWearDetection(AVRC_ID_PLAY, KeyState::RELEASED);
    EXPECT_NE(nullptr, impl->avsessionStatusChangeListener_);
    HILOGI("AvrcpTgAvSessionMediaTest_SendKeyEventByWearDetection_002 end");
}


/**
 * @tc.name: AvrcpTgAvSessionMediaTest_ConvertMetaDataToSongInfo_001
 * @tc.desc: ConvertMetaDataToSongInfo with default (empty) meta data
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_ConvertMetaDataToSongInfo_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_ConvertMetaDataToSongInfo_001 enter");
    auto impl = g_mediaInterface.pimpl_;
    AVSession::AVMetaData data;
    SongInfo song;
    bool converted = impl->ConvertMetaDataToSongInfo(data, song);
    EXPECT_TRUE(converted);
    EXPECT_EQ(std::string(""), song.media_id);
    EXPECT_FALSE(song.attributes.empty());
    HILOGI("AvrcpTgAvSessionMediaTest_ConvertMetaDataToSongInfo_001 end");
}


/**
 * @tc.name: AvrcpTgAvSessionMediaTest_ConvertMetaDataToSongInfo_002
 * @tc.desc: ConvertMetaDataToSongInfo with lyric/title/artist/album fields
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvSessionMediaTest, AvrcpTgAvSessionMediaTest_ConvertMetaDataToSongInfo_002, TestSize.Level1)
{
    HILOGI("AvrcpTgAvSessionMediaTest_ConvertMetaDataToSongInfo_002 enter");
    auto impl = g_mediaInterface.pimpl_;
    AVSession::AVMetaData data;
    data.SetAssetId("media_id_1");
    data.SetTitle("title_1");
    data.SetArtist("artist_1");
    data.SetAlbum("album_1");
    data.SetDuration(1000);
    data.SetSingleLyricText("lyric_1");
    SongInfo song;
    bool converted = impl->ConvertMetaDataToSongInfo(data, song);
    EXPECT_TRUE(converted);
    EXPECT_EQ(std::string("media_id_1"), song.media_id);
    EXPECT_FALSE(song.attributes.empty());
    HILOGI("AvrcpTgAvSessionMediaTest_ConvertMetaDataToSongInfo_002 end");
}

}  // namespace bluetooth
}  // namespace OHOS
