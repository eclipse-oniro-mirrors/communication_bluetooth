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

#include <gtest/gtest.h>
#include "ohos_bt_gatt_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
class GattUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GattUtilsTest::SetUpTestCase(void) {}

void GattUtilsTest::TearDownTestCase(void) {}

void GattUtilsTest::SetUp() {}

void GattUtilsTest::TearDown() {}

/*
 * @tc.name: GattUtilsTest_001
 * @tc.desc: Test CanStartAdv when addr is new and addr is same in 15 mins.
 * @tc.type: FUNC
 * @tc.require: issueI8EBTR
*/
HWTEST_F(GattUtilsTest, GattUtilsTest_001, TestSize.Level1)
{
    uint8_t addr[6] = {0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    std::string addrStr(reinterpret_cast<const char*>(addr), sizeof(addr));
    // new addr
    bool ret = CanStartAdv(addrStr);
    EXPECT_TRUE(ret);

    // same addr
    ret = CanStartAdv(addrStr);
    EXPECT_TRUE(ret);
    RemoveTimeoutAdvAddr();
}

}  // namespace Bluetooth
}  // namespace OHOS