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

#include "bip_image_properties.h"
#include "bip_image_format.h"
#include "bip_encoding.h"
#include "bip_transformation.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
static BluetoothHost *host_;
class BipImagePropertiesTest : public testing::Test {
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
 * @tc.number: BipImagePropertiesTest_Encode_001
 * @tc.name: AddNativeFormat
 * @tc.desc: AddNativeFormat
 */
HWTEST_F(BipImagePropertiesTest, BipImagePropertiesTest_Encode_001, TestSize.Level1)
{
    HILOGI("BipImagePropertiesTest_Encode_001 start");
    auto properties = std::make_shared<BipImageProperties>();
    std::shared_ptr<BipImageFormat> format = nullptr;
    properties->AddNativeFormat(format);
    std::string xml = properties->Encode();
    EXPECT_NE(xml.find("version=\"1.0\""), std::string::npos);
    HILOGI("BipImagePropertiesTest_Encode_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS