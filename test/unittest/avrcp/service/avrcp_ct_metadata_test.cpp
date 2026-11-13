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

#include "avrcp_ct_metadata.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class AvrcpCtMetaDataTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.number: AvrcpCtMetaDataTest_GetMetaData_001
 * @tc.name: GetMetaData
 * @tc.desc: Test GetMetaData returns nullptr before CreateMetaData
 */
HWTEST_F(AvrcpCtMetaDataTest, AvrcpCtMetaDataTest_GetMetaData_001, TestSize.Level1)
{
    AvrcpCtMetaData metaData;
    EXPECT_EQ(metaData.GetMetaData(), nullptr);
}

/*
 * @tc.number: AvrcpCtMetaDataTest_CreateMetaData_001
 * @tc.name: CreateMetaData
 * @tc.desc: Test CreateMetaData creates a valid AVMetaData instance
 */
HWTEST_F(AvrcpCtMetaDataTest, AvrcpCtMetaDataTest_CreateMetaData_001, TestSize.Level1)
{
    AvrcpCtMetaData metaData;
    metaData.CreateMetaData();
    std::shared_ptr<AVSession::AVMetaData> avMetaData = metaData.GetMetaData();
    EXPECT_NE(avMetaData, nullptr);
}

/*
 * @tc.number: AvrcpCtMetaDataTest_CreateMetaData_002
 * @tc.name: CreateMetaData twice
 * @tc.desc: Test CreateMetaData keeps the same instance when called twice
 */
HWTEST_F(AvrcpCtMetaDataTest, AvrcpCtMetaDataTest_CreateMetaData_002, TestSize.Level1)
{
    AvrcpCtMetaData metaData;
    metaData.CreateMetaData();
    auto first = metaData.GetMetaData();
    metaData.CreateMetaData();
    auto second = metaData.GetMetaData();
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

/*
 * @tc.number: AvrcpCtMetaDataTest_MetaDataContent_001
 * @tc.name: MetaData content
 * @tc.desc: Test created AVMetaData instance can be used
 */
HWTEST_F(AvrcpCtMetaDataTest, AvrcpCtMetaDataTest_MetaDataContent_001, TestSize.Level1)
{
    AvrcpCtMetaData metaData;
    metaData.CreateMetaData();
    auto avMetaData = metaData.GetMetaData();
    ASSERT_NE(avMetaData, nullptr);
    avMetaData->SetTitle("title");
    avMetaData->SetArtist("artist");
    EXPECT_EQ(avMetaData->GetTitle(), "title");
    EXPECT_EQ(avMetaData->GetArtist(), "artist");
}
} // namespace bluetooth
} // namespace OHOS
