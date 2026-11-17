/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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
#include <vector>
#include <cstdint>
#include <optional>
#include <string>
#include "map_mse_folder.h"

namespace OHOS {
namespace bluetooth {
using namespace testing;
using namespace testing::ext;
class MapMseFolderTest : public testing::Test {
public:
    MapMseFolderTest() {}
    ~MapMseFolderTest() {}
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(){};
    void TearDown(){};
};

HWTEST_F(MapMseFolderTest, MapMseFolderTest_UnitTest_AddFolder_001, TestSize.Level1)
{
    std::shared_ptr<MapMseFolder> folder = std::make_shared<MapMseFolder>("root", nullptr);
    EXPECT_NE(folder->AddFolder("telecom"), nullptr);
    EXPECT_NE(folder->AddFolder("telecom"), nullptr);
}

HWTEST_F(MapMseFolderTest, MapMseFolderTest_UnitTest_GetSubFolder_001, TestSize.Level1)
{
    std::shared_ptr<MapMseFolder> folder = std::make_shared<MapMseFolder>("root", nullptr);
    EXPECT_EQ(folder->GetSubFolder("telecom"), nullptr);

    folder->AddFolder("telecom");
    EXPECT_NE(folder->GetSubFolder("telecom"), nullptr);
}

HWTEST_F(MapMseFolderTest, MapMseFolderTest_UnitTest_GetRoot_001, TestSize.Level1)
{
    std::shared_ptr<MapMseFolder> folder = std::make_shared<MapMseFolder>("root", nullptr);
    auto telecom = folder->AddFolder("telecom");
    auto msg = telecom->AddFolder("msg");
    EXPECT_NE(telecom->GetRoot(), nullptr);
}

HWTEST_F(MapMseFolderTest, MapMseFolderTest_UnitTest_GetEncodeData_001, TestSize.Level1)
{
    std::shared_ptr<MapMseFolder> folder = std::make_shared<MapMseFolder>("root", nullptr);
    auto telecom = folder->AddFolder("telecom");
    auto msg = telecom->AddFolder("msg");

    EXPECT_EQ(msg->GetEncodeData(5, 2), "");
    EXPECT_NE(telecom->GetEncodeData(0, 1), "");
    EXPECT_NE(telecom->GetEncodeData(0, 2), "");
}
}  // namespace bluetooth
}  // namespace OHOS