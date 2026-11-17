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

#include "avrcp_ct_bip_image_properties.h"
#include "avrcp_ct_bip_request.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class AvrcpCtBipRequestTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.number: AvrcpCtBipRequestTest_GetImageProperties_001
 * @tc.name: RequestGetImageProperties basic
 * @tc.desc: Test RequestGetImageProperties type/handle/tostring
 */
HWTEST_F(AvrcpCtBipRequestTest, AvrcpCtBipRequestTest_GetImageProperties_001, TestSize.Level1)
{
    RequestGetImageProperties request("1234567");
    EXPECT_EQ(request.GetType(), BipRequest::TYPE_GET_IMAGE_PROPERTIES);
    EXPECT_EQ(request.GetImageHandle(), "1234567");

    std::string str = request.ToString();
    EXPECT_NE(str.find("RequestGetImageProperties"), std::string::npos);
    EXPECT_NE(str.find("1234567"), std::string::npos);
}

/*
 * @tc.number: AvrcpCtBipRequestTest_GetImageProperties_002
 * @tc.name: RequestGetImageProperties response code
 * @tc.desc: Test default response code and connection id accessors
 */
HWTEST_F(AvrcpCtBipRequestTest, AvrcpCtBipRequestTest_GetImageProperties_002, TestSize.Level1)
{
    RequestGetImageProperties request("1234567");
    EXPECT_EQ(request.GetResponseCode(), -1);
    EXPECT_EQ(request.GetConnectionId(), -1);

    request.SetConnectionId(5);
    EXPECT_EQ(request.GetConnectionId(), 5);
}

/*
 * @tc.number: AvrcpCtBipRequestTest_GetImageProperties_003
 * @tc.name: RequestGetImageProperties Execute with null session
 * @tc.desc: Test Execute returns false with null client session
 */
HWTEST_F(AvrcpCtBipRequestTest, AvrcpCtBipRequestTest_GetImageProperties_003, TestSize.Level1)
{
    RequestGetImageProperties request("1234567");
    EXPECT_FALSE(request.Execute(nullptr));
    EXPECT_EQ(request.GetResponseCode(), -1);
}

/*
 * @tc.number: AvrcpCtBipRequestTest_GetImageProperties_004
 * @tc.name: RequestGetImageProperties properties
 * @tc.desc: Test initial parsed properties are invalid
 */
HWTEST_F(AvrcpCtBipRequestTest, AvrcpCtBipRequestTest_GetImageProperties_004, TestSize.Level1)
{
    RequestGetImageProperties request("1234567");
    const BipImageProperties &properties = request.GetProperties();
    EXPECT_FALSE(properties.IsValid());
    EXPECT_TRUE(properties.GetImageHandle().empty());
}

/*
 * @tc.number: AvrcpCtBipRequestTest_GetImage_001
 * @tc.name: RequestGetImage basic
 * @tc.desc: Test RequestGetImage type/handle/descriptor/tostring
 */
HWTEST_F(AvrcpCtBipRequestTest, AvrcpCtBipRequestTest_GetImage_001, TestSize.Level1)
{
    BipImageDescriptor descriptor;
    descriptor.format = BIP_IMAGE_FORMAT_JPEG;
    descriptor.width = 200;
    descriptor.height = 200;

    RequestGetImage request("1234567", descriptor);
    EXPECT_EQ(request.GetType(), BipRequest::TYPE_GET_IMAGE);
    EXPECT_EQ(request.GetImageHandle(), "1234567");

    const BipImageDescriptor &desc = request.GetDescriptor();
    EXPECT_EQ(desc.format, BIP_IMAGE_FORMAT_JPEG);
    EXPECT_EQ(desc.width, 200);
    EXPECT_EQ(desc.height, 200);

    std::string str = request.ToString();
    EXPECT_NE(str.find("RequestGetImage"), std::string::npos);
    EXPECT_NE(str.find("1234567"), std::string::npos);
}

/*
 * @tc.number: AvrcpCtBipRequestTest_GetImage_002
 * @tc.name: RequestGetImage response code
 * @tc.desc: Test default response code and connection id accessors
 */
HWTEST_F(AvrcpCtBipRequestTest, AvrcpCtBipRequestTest_GetImage_002, TestSize.Level1)
{
    BipImageDescriptor descriptor;
    RequestGetImage request("1234567", descriptor);
    EXPECT_EQ(request.GetResponseCode(), -1);
    EXPECT_EQ(request.GetConnectionId(), -1);

    request.SetConnectionId(9);
    EXPECT_EQ(request.GetConnectionId(), 9);
}

/*
 * @tc.number: AvrcpCtBipRequestTest_GetImage_003
 * @tc.name: RequestGetImage Execute with null session
 * @tc.desc: Test Execute returns false with null client session
 */
HWTEST_F(AvrcpCtBipRequestTest, AvrcpCtBipRequestTest_GetImage_003, TestSize.Level1)
{
    BipImageDescriptor descriptor;
    RequestGetImage request("1234567", descriptor);
    EXPECT_FALSE(request.Execute(nullptr));
    EXPECT_EQ(request.GetResponseCode(), -1);
    EXPECT_TRUE(request.GetImage().data.empty());
}
} // namespace bluetooth
} // namespace OHOS
