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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "avrcp_cover_art_storage.h"
#include "cover_art.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int MAX_SIZE = 100;
constexpr int IMAGE_SIZE = 200;
constexpr int IMAGE_HANDLE_LENGTH = 7;
constexpr int TIME = 4;
static BluetoothHost *host_;
static AvrcpCoverArtStorage *storage_;

class AvrcpCoverArtStorageTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        storage_ = AvrcpCoverArtStorage::GetInstance();
        sleep(TIME);

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
        storage_->Clear();
    }
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.number: AvrcpCoverArtStorage_StoreImage_001
 * @tc.name: StoreImage
 * @tc.desc: Test StoreImage with valid image returns non-empty handle
 */
HWTEST_F(AvrcpCoverArtStorageTest, AvrcpCoverArtStorage_StoreImage_001, TestSize.Level1)
{
    HILOGI("AvrcpCoverArtStorage_StoreImage_001 start");
    Media::InitializationOptions options;
    options.size.width = IMAGE_SIZE;
    options.size.height = IMAGE_SIZE;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(options);
    std::shared_ptr<CoverArt> coverArt = std::make_shared<CoverArt>(pixelMap);
    std::string handle = storage_->StoreImage(coverArt);
    EXPECT_FALSE(handle.empty());
    HILOGI("AvrcpCoverArtStorage_StoreImage_001 end");
}

/*
 * @tc.number: AvrcpCoverArtStorage_GetImage_001
 * @tc.name: GetImage
 * @tc.desc: Test GetImage returns non-null for stored image
 */
HWTEST_F(AvrcpCoverArtStorageTest, AvrcpCoverArtStorage_GetImage_001, TestSize.Level1)
{
    HILOGI("AvrcpCoverArtStorage_GetImage_001 start");
    Media::InitializationOptions options;
    options.size.width = IMAGE_SIZE;
    options.size.height = IMAGE_SIZE;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(options);
    std::shared_ptr<CoverArt> coverArt = std::make_shared<CoverArt>(pixelMap);
    std::string handle = storage_->StoreImage(coverArt);
    EXPECT_FALSE(handle.empty());
    std::shared_ptr<CoverArt> retrieved = storage_->GetImage(handle);
    EXPECT_NE(retrieved, nullptr);
    HILOGI("AvrcpCoverArtStorage_GetImage_001 end");
}

/*
 * @tc.number: AvrcpCoverArtStorage_Clear_001
 * @tc.name: Clear
 * @tc.desc: Test Clear resets the storage
 */
HWTEST_F(AvrcpCoverArtStorageTest, AvrcpCoverArtStorage_Clear_001, TestSize.Level1)
{
    HILOGI("AvrcpCoverArtStorage_Clear_001 start");
    Media::InitializationOptions options;
    options.size.width = IMAGE_SIZE;
    options.size.height = IMAGE_SIZE;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(options);
    std::shared_ptr<CoverArt> coverArt1 = std::make_shared<CoverArt>(pixelMap);
    std::string handle1 = storage_->StoreImage(coverArt1);
    EXPECT_FALSE(handle1.empty());
    
    std::shared_ptr<Media::PixelMap> pixelMap2 = Media::PixelMap::Create(options);
    std::shared_ptr<CoverArt> coverArt2 = std::make_shared<CoverArt>(pixelMap2);
    std::string handle2 = storage_->StoreImage(coverArt2);
    EXPECT_FALSE(handle2.empty());
    
    storage_->Clear();
    std::string handle = storage_->GetNextImageHandle();
    EXPECT_EQ(handle, "0000000");
    HILOGI("AvrcpCoverArtStorage_Clear_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS