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

#include "btcommon/timer_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "cover_art.h"
#include "avrcp_cover_art_storage.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
constexpr int SIZE = 200;
static BluetoothHost *host_;
static std::shared_ptr<CoverArt> coverArt_ = nullptr;
class CoverArtTest : public testing::Test {
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
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.number: CoverArt_GetImageHandle_001
 * @tc.name: GetImageHandle
 * @tc.desc: GetImageHandle
 */
HWTEST_F(CoverArtTest, CoverArt_GetImageHandle_001, TestSize.Level1)
{
    HILOGI("CoverArt_GetImageHandle_001 start");
    std::string testHandle = "1234567";
    Media::InitializationOptions options;
    options.size.width = SIZE;
    options.size.height = SIZE;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(options);
    std::shared_ptr<CoverArt> coverArt = std::make_shared<CoverArt>(pixelMap);
    coverArt->SetImageHandle(testHandle);
    std::string handle = coverArt->GetImageHandle();
    EXPECT_EQ(handle, testHandle);
    HILOGI("CoverArt_GetImageHandle_001 end");
}

/*
 * @tc.number: CoverArt_GetImageProperties_001
 * @tc.name: GetImageProperties
 * @tc.desc: Test GetImageProperties returns non-null properties
 */
HWTEST_F(CoverArtTest, CoverArt_GetImageProperties_001, TestSize.Level1)
{
    HILOGI("CoverArt_GetImageProperties_001 start");
    Media::InitializationOptions options;
    options.size.width = SIZE;
    options.size.height = SIZE;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(options);
    std::shared_ptr<CoverArt> coverArt = std::make_shared<CoverArt>(pixelMap);
    std::shared_ptr<BipImageProperties> properties = coverArt->GetImageProperties();
    EXPECT_NE(properties, nullptr);
    HILOGI("CoverArt_GetImageProperties_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS