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

#include "bluetooth_log.h"
#include "pbap_pse_appparam.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;

class PbapPseAppParamTest : public testing::Test {
public:
    PbapPseAppParamTest();
    ~PbapPseAppParamTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
};

PbapPseAppParamTest::PbapPseAppParamTest() {}

PbapPseAppParamTest::~PbapPseAppParamTest() {}

void PbapPseAppParamTest::SetUpTestCase(void) {}

void PbapPseAppParamTest::TearDownTestCase(void) {}

void PbapPseAppParamTest::SetUp(void) {}

void PbapPseAppParamTest::TearDown(void) {}

/*
 * @tc.number: PbapPse_ModuleTest_ParseApplicationParameter_00100
 * @tc.name: ParseApplicationParameter
 * @tc.desc: ParseApplicationParameter
*/
HWTEST_F(PbapPseAppParamTest, PbapPse_ModuleTest_ParseApplicationParameter_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_ParseApplicationParameter_00100 start");
    const std::vector<uint8_t> param = {
        0x01, 0x01, 0x01,
        0x02, 0x0C, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x38, 0x39, 0x00,
        0x03, 0x01, 0x01,
        0x04, 0x02, 0x00, 0x01,
        0x05, 0x02, 0x00, 0x02,
        0x06, 0x08, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x81,
        0x07, 0x01, 0x00,
        0x10, 0x04, 0x00, 0x00, 0x00, 0x00,
        0x0C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82,
        0x0E, 0x01, 0x01};
    PbapPseAppParam pbapPseAppParam;
    EXPECT_TRUE(pbapPseAppParam.ParseApplicationParameter(param));
    EXPECT_EQ(pbapPseAppParam.order_, ORDER_BY_ALPHANUMERIC);
    EXPECT_EQ(pbapPseAppParam.searchValue_, "12345678989");
    EXPECT_EQ(pbapPseAppParam.searchAttribute_, SEARCH_ATTR_NUMBER);
    EXPECT_EQ(pbapPseAppParam.maxListCount_, 1);
    EXPECT_EQ(pbapPseAppParam.listStartOffset_, 2);
    EXPECT_EQ(pbapPseAppParam.propertySelector_, 0x10000081);
    EXPECT_EQ(pbapPseAppParam.vcardFormat_, VCARD_FORMAT_VERSION_21);
    EXPECT_EQ(pbapPseAppParam.supportedFeature_, 0);
    EXPECT_EQ(pbapPseAppParam.vCardSelector_, 0x82);
    EXPECT_EQ(pbapPseAppParam.vCardSelectorOperator_, VCARD_SELECTOR_OPERATOR_AND);
    HILOGI("PbapPse_ModuleTest_ParseApplicationParameter_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_ParseApplicationParameter_00200
 * @tc.name: ParseApplicationParameter
 * @tc.desc: ParseApplicationParameter
*/
HWTEST_F(PbapPseAppParamTest, PbapPse_ModuleTest_ParseApplicationParameter_00200, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_ParseApplicationParameter_00200 start");
    PbapPseAppParam pbapPseAppParam;
    const std::vector<uint8_t> param1 = {0x01, 0x01};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param1));

    const std::vector<uint8_t> param2 = {0x02, 0x00};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param2));

    const std::vector<uint8_t> param3 = {0x03, 0x01};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param3));

    const std::vector<uint8_t> param4 = {0x04, 0x02, 0x00};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param4));

    const std::vector<uint8_t> param5 = {0x05, 0x02, 0x00};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param5));

    const std::vector<uint8_t> param6 = {0x06, 0x08, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param6));

    const std::vector<uint8_t> param7 = {0x07, 0x01};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param7));

    const std::vector<uint8_t> param8 = {0x10, 0x04, 0x00, 0x00, 0x00};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param8));

    const std::vector<uint8_t> param9 = {0x0C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param9));

    const std::vector<uint8_t> param10 = {0x0E, 0x01};
    EXPECT_FALSE(pbapPseAppParam.ParseApplicationParameter(param10));
    HILOGI("PbapPse_ModuleTest_ParseApplicationParameter_00200 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
