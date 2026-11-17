/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <cstring>
#include "common_util.h"
#include "log.h"

// using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class CommonUtilTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

/**
 * @tc.name: Common_util_ConvertStrToDigit_001
 * @tc.desc: ConvertStrToDigit
 * @tc.type: FUNC
 */
HWTEST_F(CommonUtilTest, Common_util_ConvertStrToDigit_001, TestSize.Level1)
{
    HILOGI("Common_util_ConvertStrToDigit_001 enter");
    int32_t result = 0;
    EXPECT_FALSE(ConvertStrToDigit("", result));
    EXPECT_FALSE(ConvertStrToDigit("12345678910", result));
    EXPECT_FALSE(ConvertStrToDigit("123", result, 0));
    EXPECT_FALSE(ConvertStrToDigit("123A", result));
    EXPECT_FALSE(ConvertStrToDigit("123AG", result, HEX_STRING_TO_INT));
    EXPECT_FALSE(ConvertStrToDigit("3147483647", result));

    EXPECT_TRUE(ConvertStrToDigit("2147483647", result));
    EXPECT_TRUE(result == 2147483647);

    EXPECT_TRUE(ConvertStrToDigit("A027", result, HEX_STRING_TO_INT));
    EXPECT_TRUE(result == 40999);
    HILOGI("Common_util_ConvertStrToDigit_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS