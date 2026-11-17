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
#include <cstdint>
#include <string>
#include <vector>

#include "avrcp_ct_media_item.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class AvrcpCtMediaItemTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.number: AvrcpCtMediaItemTest_IsValidImageHandle_001
 * @tc.name: IsValidImageHandle
 * @tc.desc: Test IsValidImageHandle with valid and invalid handles
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_IsValidImageHandle_001, TestSize.Level1)
{
    EXPECT_TRUE(AvrcpCtMediaItem::IsValidImageHandle("1234567"));
    EXPECT_FALSE(AvrcpCtMediaItem::IsValidImageHandle(""));
    EXPECT_FALSE(AvrcpCtMediaItem::IsValidImageHandle("123456"));
    EXPECT_FALSE(AvrcpCtMediaItem::IsValidImageHandle("12345678"));
    EXPECT_FALSE(AvrcpCtMediaItem::IsValidImageHandle("12345a7"));
    EXPECT_FALSE(AvrcpCtMediaItem::IsValidImageHandle("abcdefg"));
}

/*
 * @tc.number: AvrcpCtMediaItemTest_CreateAvrcpAttribute_001
 * @tc.name: CreateAvrcpAttribute
 * @tc.desc: Test CreateAvrcpAttribute with all supported attributes
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_CreateAvrcpAttribute_001, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    std::vector<uint32_t> attrIds = {
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_TITLE,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_ARTIST_NAME,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_ALBUM_NAME,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_TRACK_NUMBER,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_TOTAL_TRACK_NUMBER,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_GENRE,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_PLAYING_TIME,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_COVER_ART_HANDLE,
    };
    std::vector<std::string> attrVals = {
        "test_title", "test_artist", "test_album", "3", "12", "test_genre", "300000", "7654321",
    };
    item.CreateAvrcpAttribute(attrIds, attrVals);

    EXPECT_EQ(item.title_, "test_title");
    EXPECT_EQ(item.artistName_, "test_artist");
    EXPECT_EQ(item.albumName_, "test_album");
    EXPECT_EQ(item.trackNumber_, 3U);
    EXPECT_EQ(item.totalNumberOfTracks_, 12U);
    EXPECT_EQ(item.genre_, "test_genre");
    EXPECT_EQ(item.playingTime_, 300000U);
    EXPECT_EQ(item.coverArtHandle_, "7654321");
}

/*
 * @tc.number: AvrcpCtMediaItemTest_CreateAvrcpAttribute_002
 * @tc.name: CreateAvrcpAttribute with invalid cover art handle
 * @tc.desc: Test invalid cover art handle is not kept
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_CreateAvrcpAttribute_002, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    std::vector<uint32_t> attrIds = {
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_TITLE,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_COVER_ART_HANDLE,
    };
    std::vector<std::string> attrVals = {"test_title", "abc1234"};
    item.CreateAvrcpAttribute(attrIds, attrVals);

    EXPECT_EQ(item.title_, "test_title");
    EXPECT_TRUE(item.coverArtHandle_.empty());
}

/*
 * @tc.number: AvrcpCtMediaItemTest_CreateAvrcpAttribute_003
 * @tc.name: CreateAvrcpAttribute with mismatched vectors
 * @tc.desc: Test only the min size of ids/vals is processed
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_CreateAvrcpAttribute_003, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    std::vector<uint32_t> attrIds = {
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_TITLE,
        AvrcpCtMediaItem::MEDIA_ATTRIBUTE_ARTIST_NAME,
    };
    std::vector<std::string> attrVals = {"only_title"};
    item.CreateAvrcpAttribute(attrIds, attrVals);

    EXPECT_EQ(item.title_, "only_title");
    EXPECT_TRUE(item.artistName_.empty());
}

/*
 * @tc.number: AvrcpCtMediaItemTest_CreateAvrcpAttribute_004
 * @tc.name: CreateAvrcpAttribute with unknown attribute id
 * @tc.desc: Test unknown attribute id is ignored
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_CreateAvrcpAttribute_004, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    std::vector<uint32_t> attrIds = {0xFF};
    std::vector<std::string> attrVals = {"unknown_value"};
    item.CreateAvrcpAttribute(attrIds, attrVals);

    EXPECT_TRUE(item.title_.empty());
    EXPECT_TRUE(item.artistName_.empty());
    EXPECT_TRUE(item.albumName_.empty());
    EXPECT_TRUE(item.genre_.empty());
}

/*
 * @tc.number: AvrcpCtMediaItemTest_HasCoverArt_001
 * @tc.name: HasCoverArt
 * @tc.desc: Test HasCoverArt with no cover, handle and data
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_HasCoverArt_001, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    EXPECT_FALSE(item.HasCoverArt());

    item.SetCoverArtHandle("1234567");
    EXPECT_TRUE(item.HasCoverArt());
    EXPECT_EQ(item.GetCoverArtHandle(), "1234567");

    AvrcpCtMediaItem item2;
    item2.SetCoverArtData({0x01, 0x02, 0x03});
    EXPECT_TRUE(item2.HasCoverArt());
    EXPECT_EQ(item2.GetCoverArtData().size(), 3U);
    EXPECT_EQ(item2.GetCoverArtData()[0], 0x01);
}

/*
 * @tc.number: AvrcpCtMediaItemTest_CoverArtUuid_001
 * @tc.name: CoverArtUuid
 * @tc.desc: Test cover art uuid setter and getter
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_CoverArtUuid_001, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    EXPECT_TRUE(item.GetCoverArtUuid().empty());
    item.SetCoverArtUuid("uuid-1234");
    EXPECT_EQ(item.GetCoverArtUuid(), "uuid-1234");
}

/*
 * @tc.number: AvrcpCtMediaItemTest_ToAVMetaData_001
 * @tc.name: ToAVMetaData
 * @tc.desc: Test ToAVMetaData maps basic media attributes
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_ToAVMetaData_001, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    item.title_ = "meta_title";
    item.artistName_ = "meta_artist";
    item.albumName_ = "meta_album";
    item.playingTime_ = 60000;

    AVSession::AVMetaData meta = item.ToAVMetaData();
    EXPECT_EQ(meta.GetTitle(), "meta_title");
    EXPECT_EQ(meta.GetArtist(), "meta_artist");
    EXPECT_EQ(meta.GetAlbum(), "meta_album");
    EXPECT_EQ(meta.GetDuration(), 60000);
    EXPECT_EQ(meta.GetAssetId(), "meta_title");
}

/*
 * @tc.number: AvrcpCtMediaItemTest_ToAVMetaData_002
 * @tc.name: ToAVMetaData with cover art handle
 * @tc.desc: Test ToAVMetaData sets downloading flag when only handle exists
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_ToAVMetaData_002, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    item.title_ = "meta_title";
    item.SetCoverArtHandle("1234567");

    AVSession::AVMetaData meta = item.ToAVMetaData();
    EXPECT_EQ(meta.GetDescription(), AvrcpCtMediaItem::COVER_ART_DOWNLOADING_FLAG);
}

/*
 * @tc.number: AvrcpCtMediaItemTest_ToAVMetaData_003
 * @tc.name: ToAVMetaData with cover art data
 * @tc.desc: Test ToAVMetaData clears downloading flag when cover data exists
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_ToAVMetaData_003, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    item.title_ = "meta_title";
    item.SetCoverArtHandle("1234567");
    item.SetCoverArtData({0x00, 0x01, 0x02});

    AVSession::AVMetaData meta = item.ToAVMetaData();
    EXPECT_NE(meta.GetMediaImage(), nullptr);
    EXPECT_NE(meta.GetDescription(), AvrcpCtMediaItem::COVER_ART_DOWNLOADING_FLAG);
}

/*
 * @tc.number: AvrcpCtMediaItemTest_PlayableAndType_001
 * @tc.name: PlayableAndType
 * @tc.desc: Test playable flag, item type and media type accessors
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_PlayableAndType_001, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    EXPECT_FALSE(item.IsPlayable());
    item.SetPlayable(true);
    EXPECT_TRUE(item.IsPlayable());

    EXPECT_EQ(item.GetItemType(), 0);
    item.SetItemType(AvrcpCtMediaItem::TYPE_MEDIA);
    EXPECT_EQ(item.GetItemType(), AvrcpCtMediaItem::TYPE_MEDIA);

    EXPECT_EQ(item.GetType(), 0);
    item.SetType(6);
    EXPECT_EQ(item.GetType(), 6);

    EXPECT_EQ(item.GetUid(), 0U);
    item.SetUid(12345U);
    EXPECT_EQ(item.GetUid(), 12345U);
}

/*
 * @tc.number: AvrcpCtMediaItemTest_GetDisplayName_001
 * @tc.name: GetDisplayName
 * @tc.desc: Test GetDisplayName falls back to title when display name is empty
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_GetDisplayName_001, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    item.title_ = "fallback_title";
    EXPECT_EQ(item.GetDisplayName(), "fallback_title");

    item.SetDisplayName("display_name");
    EXPECT_EQ(item.GetDisplayName(), "display_name");
}

/*
 * @tc.number: AvrcpCtMediaItemTest_AttributeAccessors_001
 * @tc.name: AttributeAccessors
 * @tc.desc: Test media attribute setter and getter methods
 */
HWTEST_F(AvrcpCtMediaItemTest, AvrcpCtMediaItemTest_AttributeAccessors_001, TestSize.Level1)
{
    AvrcpCtMediaItem item;
    item.SetTitle("t");
    EXPECT_EQ(item.GetTitle(), "t");
    item.SetArtist("a");
    EXPECT_EQ(item.GetArtist(), "a");
    item.SetAlbum("al");
    EXPECT_EQ(item.GetAlbum(), "al");
    item.SetTrackNumber(5U);
    EXPECT_EQ(item.GetTrackNumber(), 5U);
    item.SetTotalTracks(20U);
    EXPECT_EQ(item.GetTotalTracks(), 20U);
    item.SetDuration(90000U);
    EXPECT_EQ(item.GetDuration(), 90000U);
    item.SetGenre("g");
    EXPECT_EQ(item.GetGenre(), "g");
}
} // namespace bluetooth
} // namespace OHOS
