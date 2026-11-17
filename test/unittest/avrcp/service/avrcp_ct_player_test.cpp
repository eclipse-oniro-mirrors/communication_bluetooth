/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <memory>
#include <string>
#include <vector>

#include "avrcp_ct_media_item.h"
#include "avrcp_ct_player.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

// AVRCP play status values (see btrc_play_status_t)
constexpr int PLAY_STATUS_STOPPED = 0x00;
constexpr int PLAY_STATUS_PLAYING = 0x01;
constexpr int PLAY_STATUS_PAUSED = 0x02;
constexpr int PLAY_STATUS_FWD_SEEK = 0x03;
constexpr int PLAY_STATUS_REV_SEEK = 0x04;

class AvrcpCtPlayerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        player_ = std::make_unique<AvrcpCtPlayer>();
    }
    void TearDown() override
    {
        player_ = nullptr;
    }

protected:
    std::unique_ptr<AvrcpCtPlayer> player_;
};

/*
 * @tc.number: AvrcpCtPlayerTest_PlayerId_001
 * @tc.name: PlayerId
 * @tc.desc: Test player id default value and setter/getter
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_PlayerId_001, TestSize.Level1)
{
    EXPECT_EQ(player_->GetPlayerId(), AvrcpCtPlayer::DEFAULT_ID);
    player_->SetPlayerId(5);
    EXPECT_EQ(player_->GetPlayerId(), 5);
}

/*
 * @tc.number: AvrcpCtPlayerTest_PlayerName_001
 * @tc.name: PlayerName
 * @tc.desc: Test player name setter/getter
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_PlayerName_001, TestSize.Level1)
{
    EXPECT_TRUE(player_->GetPlayerName().empty());
    player_->SetPlayerName("player");
    EXPECT_EQ(player_->GetPlayerName(), "player");
}

/*
 * @tc.number: AvrcpCtPlayerTest_PlayerType_001
 * @tc.name: PlayerType
 * @tc.desc: Test player type default value and setter/getter
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_PlayerType_001, TestSize.Level1)
{
    EXPECT_EQ(player_->GetPlayerType(), AvrcpCtPlayer::TYPE_UNKNOWN);
    player_->SetPlayerType(AvrcpCtPlayer::TYPE_AUDIO);
    EXPECT_EQ(player_->GetPlayerType(), AvrcpCtPlayer::TYPE_AUDIO);
}

/*
 * @tc.number: AvrcpCtPlayerTest_DeviceAddress_001
 * @tc.name: DeviceAddress
 * @tc.desc: Test device address setter/getter
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_DeviceAddress_001, TestSize.Level1)
{
    EXPECT_TRUE(player_->GetDeviceAddress().empty());
    player_->SetDeviceAddress("00:11:22:33:44:55");
    EXPECT_EQ(player_->GetDeviceAddress(), "00:11:22:33:44:55");
}

/*
 * @tc.number: AvrcpCtPlayerTest_PlayStatus_001
 * @tc.name: PlayStatus
 * @tc.desc: Test play status default value and setter/getter
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_PlayStatus_001, TestSize.Level1)
{
    EXPECT_EQ(player_->GetPlayStatus(), 0);
    player_->SetPlayStatus(PLAY_STATUS_PLAYING);
    EXPECT_EQ(player_->GetPlayStatus(), PLAY_STATUS_PLAYING);
}

/*
 * @tc.number: AvrcpCtPlayerTest_SetPlayStatus_Speed_001
 * @tc.name: SetPlayStatus speed mapping
 * @tc.desc: Test play speed is set to 1.0 for playing/stopped/paused/rev-seek
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_SetPlayStatus_Speed_001, TestSize.Level1)
{
    player_->SetPlayStatus(PLAY_STATUS_PLAYING);
    EXPECT_FLOAT_EQ(player_->GetPlaySpeed(), 1.0f);

    player_->SetPlayStatus(PLAY_STATUS_STOPPED);
    EXPECT_FLOAT_EQ(player_->GetPlaySpeed(), 1.0f);

    player_->SetPlayStatus(PLAY_STATUS_PAUSED);
    EXPECT_FLOAT_EQ(player_->GetPlaySpeed(), 1.0f);

    player_->SetPlayStatus(PLAY_STATUS_REV_SEEK);
    EXPECT_FLOAT_EQ(player_->GetPlaySpeed(), 1.0f);

    player_->SetPlayStatus(0xFF);
    EXPECT_FLOAT_EQ(player_->GetPlaySpeed(), 1.0f);
}

/*
 * @tc.number: AvrcpCtPlayerTest_SetPlayStatus_Speed_002
 * @tc.name: SetPlayStatus fwd-seek speed
 * @tc.desc: Test play speed is set to 3.0 for fwd-seek
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_SetPlayStatus_Speed_002, TestSize.Level1)
{
    player_->SetPlayStatus(PLAY_STATUS_FWD_SEEK);
    EXPECT_FLOAT_EQ(player_->GetPlaySpeed(), 3.0f);
}

/*
 * @tc.number: AvrcpCtPlayerTest_PlayTime_001
 * @tc.name: PlayTime and position
 * @tc.desc: Test play time / play position / duration setter/getter
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_PlayTime_001, TestSize.Level1)
{
    EXPECT_EQ(player_->GetPlayTime(), 0);
    player_->SetPlayTime(30000);
    EXPECT_EQ(player_->GetPlayTime(), 30000);

    EXPECT_EQ(player_->GetPlayPosition(), 0);
    player_->SetPlayPosition(15000);
    EXPECT_EQ(player_->GetPlayPosition(), 15000);

    EXPECT_EQ(player_->GetDuration(), 0);
    player_->SetDuration(60000);
    EXPECT_EQ(player_->GetDuration(), 60000);
}

/*
 * @tc.number: AvrcpCtPlayerTest_PlaySpeed_001
 * @tc.name: PlaySpeed
 * @tc.desc: Test play speed setter/getter
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_PlaySpeed_001, TestSize.Level1)
{
    EXPECT_FLOAT_EQ(player_->GetPlaySpeed(), 1.0f);
    player_->SetPlaySpeed(2.5f);
    EXPECT_FLOAT_EQ(player_->GetPlaySpeed(), 2.5f);
}

/*
 * @tc.number: AvrcpCtPlayerTest_CurrentTrack_001
 * @tc.name: CurrentTrack
 * @tc.desc: Test current track update and getter
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_CurrentTrack_001, TestSize.Level1)
{
    EXPECT_EQ(player_->GetCurrentTrack(), nullptr);
    auto track = std::make_shared<AvrcpCtMediaItem>();
    track->SetTitle("track_title");
    player_->UpdateCurrentTrack(track);
    EXPECT_EQ(player_->GetCurrentTrack(), track);
    EXPECT_EQ(player_->GetCurrentTrack()->GetTitle(), "track_title");
}

/*
 * @tc.number: AvrcpCtPlayerTest_HasCoverArt_001
 * @tc.name: HasCoverArt without track
 * @tc.desc: Test HasCoverArt and GetCoverArtHandle without current track
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_HasCoverArt_001, TestSize.Level1)
{
    EXPECT_FALSE(player_->HasCoverArt());
    EXPECT_TRUE(player_->GetCoverArtHandle().empty());
}

/*
 * @tc.number: AvrcpCtPlayerTest_HasCoverArt_002
 * @tc.name: HasCoverArt with track
 * @tc.desc: Test HasCoverArt and GetCoverArtHandle with current track
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_HasCoverArt_002, TestSize.Level1)
{
    auto track = std::make_shared<AvrcpCtMediaItem>();
    player_->UpdateCurrentTrack(track);
    EXPECT_FALSE(player_->HasCoverArt());

    track->SetCoverArtHandle("1234567");
    EXPECT_TRUE(player_->HasCoverArt());
    EXPECT_EQ(player_->GetCoverArtHandle(), "1234567");

    auto track2 = std::make_shared<AvrcpCtMediaItem>();
    track2->SetCoverArtData({0x01});
    player_->UpdateCurrentTrack(track2);
    EXPECT_TRUE(player_->HasCoverArt());
}

/*
 * @tc.number: AvrcpCtPlayerTest_NotifyImageDownload_001
 * @tc.name: NotifyImageDownload
 * @tc.desc: Test NotifyImageDownload with empty uuid / no track / match / mismatch
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_NotifyImageDownload_001, TestSize.Level1)
{
    EXPECT_FALSE(player_->NotifyImageDownload("", "file://uri"));

    auto track = std::make_shared<AvrcpCtMediaItem>();
    EXPECT_FALSE(player_->NotifyImageDownload("uuid-1", "file://uri"));

    track->SetCoverArtUuid("uuid-1");
    player_->UpdateCurrentTrack(track);
    EXPECT_TRUE(player_->NotifyImageDownload("uuid-1", "file://uri"));
    EXPECT_FALSE(player_->NotifyImageDownload("uuid-2", "file://uri"));
}

/*
 * @tc.number: AvrcpCtPlayerTest_SupportsFeature_001
 * @tc.name: SupportsFeature
 * @tc.desc: Test SupportsFeature bit calculation
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_SupportsFeature_001, TestSize.Level1)
{
    EXPECT_FALSE(player_->SupportsFeature(0));
    EXPECT_FALSE(player_->SupportsFeature(AvrcpCtPlayer::FEATURE_PLAY));
    EXPECT_FALSE(player_->SupportsFeature(-1));

    player_->SetSupportedFeatures({0x01, 0x00});
    EXPECT_TRUE(player_->SupportsFeature(0));
    EXPECT_FALSE(player_->SupportsFeature(1));
    EXPECT_TRUE(player_->SupportsFeature(8));
    EXPECT_FALSE(player_->SupportsFeature(9));
    EXPECT_FALSE(player_->SupportsFeature(16));
}

/*
 * @tc.number: AvrcpCtPlayerTest_SetSupportedFeature_001
 * @tc.name: SetSupportedFeature
 * @tc.desc: Test SetSupportedFeature sets and resizes feature vector
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_SetSupportedFeature_001, TestSize.Level1)
{
    player_->SetSupportedFeature(3);
    EXPECT_TRUE(player_->SupportsFeature(3));
    EXPECT_FALSE(player_->SupportsFeature(0));

    player_->SetSupportedFeature(0);
    EXPECT_TRUE(player_->SupportsFeature(0));

    player_->SetSupportedFeature(10);
    EXPECT_TRUE(player_->SupportsFeature(10));
    EXPECT_TRUE(player_->SupportsFeature(3));
    EXPECT_FALSE(player_->SupportsFeature(11));
}

/*
 * @tc.number: AvrcpCtPlayerTest_ToAVPlaybackState_001
 * @tc.name: ToAVPlaybackState stopped
 * @tc.desc: Test ToAVPlaybackState maps stopped status to stop state
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_ToAVPlaybackState_001, TestSize.Level1)
{
    player_->SetPlayStatus(PLAY_STATUS_STOPPED);
    auto state = player_->ToAVPlaybackState();
    EXPECT_EQ(state.GetState(), AVSession::AVPlaybackState::PLAYBACK_STATE_STOP);
    EXPECT_FLOAT_EQ(static_cast<float>(state.GetSpeed()), 1.0f);
}

/*
 * @tc.number: AvrcpCtPlayerTest_ToAVPlaybackState_002
 * @tc.name: ToAVPlaybackState playing
 * @tc.desc: Test ToAVPlaybackState maps playing status to play state
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_ToAVPlaybackState_002, TestSize.Level1)
{
    player_->SetPlayStatus(PLAY_STATUS_PLAYING);
    auto state = player_->ToAVPlaybackState();
    EXPECT_EQ(state.GetState(), AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY);
}

/*
 * @tc.number: AvrcpCtPlayerTest_ToAVPlaybackState_003
 * @tc.name: ToAVPlaybackState paused
 * @tc.desc: Test ToAVPlaybackState maps paused status to pause state
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_ToAVPlaybackState_003, TestSize.Level1)
{
    player_->SetPlayStatus(PLAY_STATUS_PAUSED);
    auto state = player_->ToAVPlaybackState();
    EXPECT_EQ(state.GetState(), AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE);
}

/*
 * @tc.number: AvrcpCtPlayerTest_ToAVPlaybackState_004
 * @tc.name: ToAVPlaybackState seek states
 * @tc.desc: Test ToAVPlaybackState maps fwd-seek to play and rev-seek to pause
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_ToAVPlaybackState_004, TestSize.Level1)
{
    player_->SetPlayStatus(PLAY_STATUS_FWD_SEEK);
    auto state = player_->ToAVPlaybackState();
    EXPECT_EQ(state.GetState(), AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY);
    EXPECT_FLOAT_EQ(static_cast<float>(state.GetSpeed()), 3.0f);

    player_->SetPlayStatus(PLAY_STATUS_REV_SEEK);
    state = player_->ToAVPlaybackState();
    EXPECT_EQ(state.GetState(), AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE);
}

/*
 * @tc.number: AvrcpCtPlayerTest_ToAVPlaybackState_005
 * @tc.name: ToAVPlaybackState unknown status
 * @tc.desc: Test ToAVPlaybackState maps unknown status to idle state
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_ToAVPlaybackState_005, TestSize.Level1)
{
    player_->SetPlayStatus(0xFF);
    auto state = player_->ToAVPlaybackState();
    EXPECT_EQ(state.GetState(), AVSession::AVPlaybackState::PLAYBACK_STATE_IDLE);
}

/*
 * @tc.number: AvrcpCtPlayerTest_ToAVPlaybackState_006
 * @tc.name: ToAVPlaybackState position
 * @tc.desc: Test ToAVPlaybackState sets position only when play time is positive
 */
HWTEST_F(AvrcpCtPlayerTest, AvrcpCtPlayerTest_ToAVPlaybackState_006, TestSize.Level1)
{
    player_->SetPlayStatus(PLAY_STATUS_PLAYING);
    player_->SetPlayTime(30000);
    auto state = player_->ToAVPlaybackState();
    EXPECT_EQ(state.GetPosition().elapsedTime_, 30000);
}
} // namespace bluetooth
} // namespace OHOS
