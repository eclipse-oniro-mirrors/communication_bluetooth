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

#include "avrcp_ct_bip_image_properties.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class AvrcpCtBipImagePropertiesTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ImageHandle_001
 * @tc.name: ImageHandle
 * @tc.desc: Test image handle setter/getter
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ImageHandle_001, TestSize.Level1)
{
    BipImageProperties properties;
    EXPECT_TRUE(properties.GetImageHandle().empty());
    properties.SetImageHandle("1234567");
    EXPECT_EQ(properties.GetImageHandle(), "1234567");
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ThumbnailFormat_001
 * @tc.name: ThumbnailFormat
 * @tc.desc: Test thumbnail format flag setter/getter
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ThumbnailFormat_001, TestSize.Level1)
{
    BipImageProperties properties;
    EXPECT_FALSE(properties.HasThumbnailFormat());
    properties.SetHasThumbnailFormat(true);
    EXPECT_TRUE(properties.HasThumbnailFormat());
    properties.SetHasThumbnailFormat(false);
    EXPECT_FALSE(properties.HasThumbnailFormat());
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_IsValid_001
 * @tc.name: IsValid
 * @tc.desc: Test IsValid requires handle and thumbnail format
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_IsValid_001, TestSize.Level1)
{
    BipImageProperties properties;
    EXPECT_FALSE(properties.IsValid());

    properties.SetImageHandle("1234567");
    EXPECT_FALSE(properties.IsValid());

    properties.SetHasThumbnailFormat(true);
    EXPECT_TRUE(properties.IsValid());
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ParseFromData_001
 * @tc.name: ParseFromData valid xml
 * @tc.desc: Test ParseFromData parses handle and thumbnail format
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ParseFromData_001, TestSize.Level1)
{
    const std::string xml = "<image-properties handle=\"1234567\" version=\"1.0\">"
                            "<thumbnail encoding=\"JPEG\" pixel=\"200*200\"/>"
                            "</image-properties>";
    std::vector<uint8_t> data(xml.begin(), xml.end());

    BipImageProperties properties;
    EXPECT_TRUE(properties.ParseFromData(data));
    EXPECT_EQ(properties.GetImageHandle(), "1234567");
    EXPECT_TRUE(properties.HasThumbnailFormat());
    EXPECT_TRUE(properties.IsValid());
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ParseFromData_002
 * @tc.name: ParseFromData empty data
 * @tc.desc: Test ParseFromData fails with empty data
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ParseFromData_002, TestSize.Level1)
{
    std::vector<uint8_t> data;
    BipImageProperties properties;
    EXPECT_FALSE(properties.ParseFromData(data));
    EXPECT_FALSE(properties.IsValid());
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ParseFromData_003
 * @tc.name: ParseFromData invalid xml
 * @tc.desc: Test ParseFromData fails with invalid xml content
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ParseFromData_003, TestSize.Level1)
{
    const std::string xml = "not-a-xml-content";
    std::vector<uint8_t> data(xml.begin(), xml.end());

    BipImageProperties properties;
    EXPECT_FALSE(properties.ParseFromData(data));
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ParseFromData_004
 * @tc.name: ParseFromData without handle
 * @tc.desc: Test ParseFromData fails when handle attribute is missing
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ParseFromData_004, TestSize.Level1)
{
    const std::string xml = "<image-properties version=\"1.0\">"
                            "<thumbnail encoding=\"JPEG\" pixel=\"200*200\"/>"
                            "</image-properties>";
    std::vector<uint8_t> data(xml.begin(), xml.end());

    BipImageProperties properties;
    EXPECT_FALSE(properties.ParseFromData(data));
    EXPECT_TRUE(properties.GetImageHandle().empty());
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ParseFromData_005
 * @tc.name: ParseFromData non-thumbnail format
 * @tc.desc: Test ParseFromData with non JPEG 200*200 thumbnail format
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ParseFromData_005, TestSize.Level1)
{
    const std::string xml = "<image-properties handle=\"1234567\" version=\"1.0\">"
                            "<thumbnail encoding=\"PNG\" pixel=\"200*200\"/>"
                            "</image-properties>";
    std::vector<uint8_t> data(xml.begin(), xml.end());

    BipImageProperties properties;
    EXPECT_TRUE(properties.ParseFromData(data));
    EXPECT_EQ(properties.GetImageHandle(), "1234567");
    EXPECT_FALSE(properties.HasThumbnailFormat());
    EXPECT_FALSE(properties.IsValid());
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ParseFromData_006
 * @tc.name: ParseFromData custom version
 * @tc.desc: Test ParseFromData keeps custom version and IsValid fails
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ParseFromData_006, TestSize.Level1)
{
    const std::string xml = "<image-properties handle=\"1234567\" version=\"2.0\">"
                            "<thumbnail encoding=\"JPEG\" pixel=\"200*200\"/>"
                            "</image-properties>";
    std::vector<uint8_t> data(xml.begin(), xml.end());

    BipImageProperties properties;
    EXPECT_TRUE(properties.ParseFromData(data));
    EXPECT_FALSE(properties.IsValid());
}

/*
 * @tc.number: AvrcpCtBipImagePropertiesTest_ToString_001
 * @tc.name: ToString
 * @tc.desc: Test ToString contains handle and thumbnail state
 */
HWTEST_F(AvrcpCtBipImagePropertiesTest, AvrcpCtBipImagePropertiesTest_ToString_001, TestSize.Level1)
{
    BipImageProperties properties;
    properties.SetImageHandle("1234567");
    std::string str = properties.ToString();
    EXPECT_NE(str.find("1234567"), std::string::npos);
    EXPECT_NE(str.find(BipImageProperties::VERSION), std::string::npos);
    EXPECT_NE(str.find("false"), std::string::npos);

    properties.SetHasThumbnailFormat(true);
    str = properties.ToString();
    EXPECT_NE(str.find("true"), std::string::npos);
}
} // namespace bluetooth
} // namespace OHOS
