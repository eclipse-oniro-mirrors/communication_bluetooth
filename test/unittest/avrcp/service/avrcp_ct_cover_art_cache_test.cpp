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

#include "avrcp_ct_cover_art_cache.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

static const std::string DEVICE_ADDR = "00:11:22:33:44:55";
static const std::string DEVICE_ADDR_2 = "AA:BB:CC:DD:EE:FF";

class AvrcpCtCoverArtCacheTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        cache_ = std::make_unique<AvrcpCtCoverArtCache>();
    }
    void TearDown() override
    {
        cache_ = nullptr;
    }

protected:
    std::unique_ptr<AvrcpCtCoverArtCache> cache_;
};

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_AddAndHasImage_001
 * @tc.name: AddImage and HasImage
 * @tc.desc: Test AddImage stores image and HasImage queries it
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_AddAndHasImage_001, TestSize.Level1)
{
    EXPECT_FALSE(cache_->HasImage(DEVICE_ADDR, "uuid-1"));

    cache_->AddImage(DEVICE_ADDR, "uuid-1", {0x01, 0x02, 0x03});
    EXPECT_TRUE(cache_->HasImage(DEVICE_ADDR, "uuid-1"));
    EXPECT_FALSE(cache_->HasImage(DEVICE_ADDR, "uuid-2"));
    EXPECT_FALSE(cache_->HasImage(DEVICE_ADDR_2, "uuid-1"));
}

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_AddImage_001
 * @tc.name: AddImage with invalid parameters
 * @tc.desc: Test AddImage ignores invalid parameters
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_AddImage_001, TestSize.Level1)
{
    cache_->AddImage("", "uuid-1", {0x01});
    cache_->AddImage(DEVICE_ADDR, "", {0x01});
    cache_->AddImage(DEVICE_ADDR, "uuid-1", {});
    EXPECT_FALSE(cache_->HasImage(DEVICE_ADDR, "uuid-1"));
    EXPECT_EQ(cache_->GetTotalCacheSize(), 0U);
}

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_GetImage_001
 * @tc.name: GetImage
 * @tc.desc: Test GetImage returns stored image data
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_GetImage_001, TestSize.Level1)
{
    std::vector<uint8_t> imageData = {0x0A, 0x0B, 0x0C, 0x0D};
    cache_->AddImage(DEVICE_ADDR, "uuid-1", imageData);

    std::vector<uint8_t> outData;
    EXPECT_TRUE(cache_->GetImage(DEVICE_ADDR, "uuid-1", outData));
    EXPECT_EQ(outData, imageData);
}

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_GetImage_002
 * @tc.name: GetImage with invalid parameters
 * @tc.desc: Test GetImage returns false for invalid parameters or missing entries
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_GetImage_002, TestSize.Level1)
{
    cache_->AddImage(DEVICE_ADDR, "uuid-1", {0x01});

    std::vector<uint8_t> outData;
    EXPECT_FALSE(cache_->GetImage("", "uuid-1", outData));
    EXPECT_FALSE(cache_->GetImage(DEVICE_ADDR, "", outData));
    EXPECT_FALSE(cache_->GetImage(DEVICE_ADDR_2, "uuid-1", outData));
    EXPECT_FALSE(cache_->GetImage(DEVICE_ADDR, "uuid-2", outData));
    EXPECT_TRUE(outData.empty());
}

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_RemoveImage_001
 * @tc.name: RemoveImage
 * @tc.desc: Test RemoveImage removes image and cleans up empty device cache
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_RemoveImage_001, TestSize.Level1)
{
    cache_->AddImage(DEVICE_ADDR, "uuid-1", {0x01});
    cache_->AddImage(DEVICE_ADDR, "uuid-2", {0x02});

    cache_->RemoveImage(DEVICE_ADDR, "uuid-1");
    EXPECT_FALSE(cache_->HasImage(DEVICE_ADDR, "uuid-1"));
    EXPECT_TRUE(cache_->HasImage(DEVICE_ADDR, "uuid-2"));
    EXPECT_EQ(cache_->GetDeviceCacheSize(DEVICE_ADDR), 1U);

    cache_->RemoveImage(DEVICE_ADDR, "uuid-2");
    EXPECT_EQ(cache_->GetDeviceCacheSize(DEVICE_ADDR), 0U);
    EXPECT_EQ(cache_->GetTotalCacheSize(), 0U);
}

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_RemoveImage_002
 * @tc.name: RemoveImage with invalid parameters
 * @tc.desc: Test RemoveImage ignores invalid parameters and missing entries
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_RemoveImage_002, TestSize.Level1)
{
    cache_->AddImage(DEVICE_ADDR, "uuid-1", {0x01});
    cache_->RemoveImage("", "uuid-1");
    cache_->RemoveImage(DEVICE_ADDR, "");
    cache_->RemoveImage(DEVICE_ADDR_2, "uuid-1");
    cache_->RemoveImage(DEVICE_ADDR, "not-exist");
    EXPECT_TRUE(cache_->HasImage(DEVICE_ADDR, "uuid-1"));
}

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_ClearDevice_001
 * @tc.name: ClearDevice
 * @tc.desc: Test ClearDevice removes all images of one device only
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_ClearDevice_001, TestSize.Level1)
{
    cache_->AddImage(DEVICE_ADDR, "uuid-1", {0x01});
    cache_->AddImage(DEVICE_ADDR, "uuid-2", {0x02});
    cache_->AddImage(DEVICE_ADDR_2, "uuid-3", {0x03});

    cache_->ClearDevice(DEVICE_ADDR);
    EXPECT_EQ(cache_->GetDeviceCacheSize(DEVICE_ADDR), 0U);
    EXPECT_FALSE(cache_->HasImage(DEVICE_ADDR, "uuid-1"));
    EXPECT_TRUE(cache_->HasImage(DEVICE_ADDR_2, "uuid-3"));
    EXPECT_EQ(cache_->GetTotalCacheSize(), 1U);

    cache_->ClearDevice("");
    EXPECT_EQ(cache_->GetTotalCacheSize(), 1U);
}

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_ClearAll_001
 * @tc.name: ClearAll
 * @tc.desc: Test ClearAll removes all cached images
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_ClearAll_001, TestSize.Level1)
{
    cache_->AddImage(DEVICE_ADDR, "uuid-1", {0x01});
    cache_->AddImage(DEVICE_ADDR_2, "uuid-2", {0x02});

    cache_->ClearAll();
    EXPECT_FALSE(cache_->HasImage(DEVICE_ADDR, "uuid-1"));
    EXPECT_FALSE(cache_->HasImage(DEVICE_ADDR_2, "uuid-2"));
    EXPECT_EQ(cache_->GetDeviceCacheSize(DEVICE_ADDR), 0U);
    EXPECT_EQ(cache_->GetTotalCacheSize(), 0U);
}

/*
 * @tc.number: AvrcpCtCoverArtCacheTest_CacheSize_001
 * @tc.name: GetDeviceCacheSize and GetTotalCacheSize
 * @tc.desc: Test cache size statistics
 */
HWTEST_F(AvrcpCtCoverArtCacheTest, AvrcpCtCoverArtCacheTest_CacheSize_001, TestSize.Level1)
{
    EXPECT_EQ(cache_->GetDeviceCacheSize(DEVICE_ADDR), 0U);
    EXPECT_EQ(cache_->GetTotalCacheSize(), 0U);

    cache_->AddImage(DEVICE_ADDR, "uuid-1", {0x01});
    cache_->AddImage(DEVICE_ADDR, "uuid-2", {0x02});
    cache_->AddImage(DEVICE_ADDR_2, "uuid-3", {0x03});

    EXPECT_EQ(cache_->GetDeviceCacheSize(DEVICE_ADDR), 2U);
    EXPECT_EQ(cache_->GetDeviceCacheSize(DEVICE_ADDR_2), 1U);
    EXPECT_EQ(cache_->GetTotalCacheSize(), 3U);
    EXPECT_EQ(cache_->GetDeviceCacheSize(""), 0U);

    // same uuid overwrite
    cache_->AddImage(DEVICE_ADDR, "uuid-1", {0x04, 0x05});
    EXPECT_EQ(cache_->GetDeviceCacheSize(DEVICE_ADDR), 2U);
    std::vector<uint8_t> outData;
    EXPECT_TRUE(cache_->GetImage(DEVICE_ADDR, "uuid-1", outData));
    EXPECT_EQ(outData.size(), 2U);
    EXPECT_EQ(outData[0], 0x04);
}
} // namespace bluetooth
} // namespace OHOS
