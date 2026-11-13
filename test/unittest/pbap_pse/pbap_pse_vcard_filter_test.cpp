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

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "pbap_pse_vcard_filter.h"
#include <sstream>
#include <cctype>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace OHOS::Bluetooth;

constexpr int32_t TIME = 4;
static BluetoothHost *host_;
class PbapPseVcardFilterTest : public testing::Test {
public:
    PbapPseVcardFilterTest();
    ~PbapPseVcardFilterTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
    std::unique_ptr<PbabPseVcardFilter> pbapPseVcardFilterTest = nullptr;
};

PbapPseVcardFilterTest::PbapPseVcardFilterTest()
{
    pbapPseVcardFilterTest = std::make_unique<PbabPseVcardFilter>();
}

PbapPseVcardFilterTest::~PbapPseVcardFilterTest() {}

void PbapPseVcardFilterTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
}

void PbapPseVcardFilterTest::TearDownTestCase(void) {}

void PbapPseVcardFilterTest::SetUp() {}

void PbapPseVcardFilterTest::TearDown() {}

/*
 * @tc.number: PbapPseVcardFilterTest_splitByRegex_00100
 * @tc.name: splitByRegex
 * @tc.desc: splitByRegex
*/
HWTEST_F(PbapPseVcardFilterTest, PbapPseVcardFilterTest_splitByRegex_00100, TestSize.Level0)
{
    HILOGI("PbapPseVcardFilterTest_splitByRegex_00100 start");
    std::string str = "hello;;world";
    std::regex pattern("[;;]");
    std::string currentProp = pbapPseVcardFilterTest->splitByRegex(str, pattern)[0];
    EXPECT_EQ(currentProp, "hello");
    HILOGI("PbapPseVcardFilterTest_splitByRegex_00100 end");
}

/*
 * @tc.number: PbapPseVcardFilterTest_split_00100
 * @tc.name: split
 * @tc.desc: split
*/
HWTEST_F(PbapPseVcardFilterTest, PbapPseVcardFilterTest_split_00100, TestSize.Level0)
{
    HILOGI("PbapPseVcardFilterTest_split_00100 start");
    std::string str = "hello;world";
    std::string delimiter = ";";
    std::string currentProp = pbapPseVcardFilterTest->split(str, delimiter)[0];
    EXPECT_EQ(currentProp, "hello");
    EXPECT_EQ(pbapPseVcardFilterTest->split(str, delimiter)[1], "world");
    HILOGI("PbapPseVcardFilterTest_split_00100 end");
}

/*
 * @tc.number: PbapPseVcardFilterTest_IsFilteredIn_00100
 * @tc.name: IsFilteredIn
 * @tc.desc: IsFilteredIn
*/
HWTEST_F(PbapPseVcardFilterTest, PbapPseVcardFilterTest_IsFilteredIn_00100, TestSize.Level0)
{
    HILOGI("PbapPseVcardFilterTest_IsFilteredIn_00100 start");
    Filter filter(1, "FN", true, true);
    filter.onlyCheckV21 = true;
    int32_t vcardType = VCARD_FORMAT_VERSION_21;
    uint64_t propertySelector = 0x01;
    bool isFiltered = pbapPseVcardFilterTest->IsFilteredIn(filter, vcardType, propertySelector);
    EXPECT_FALSE(isFiltered);
    filter.onlyCheckV21 = false;
    isFiltered = pbapPseVcardFilterTest->IsFilteredIn(filter, vcardType, propertySelector);
    EXPECT_FALSE(isFiltered);
    filter.excludeForV21 = false;
    isFiltered = pbapPseVcardFilterTest->IsFilteredIn(filter, vcardType, propertySelector);
    EXPECT_FALSE(isFiltered);
    filter.excludeForV21 = true;
    isFiltered = pbapPseVcardFilterTest->IsFilteredIn(filter, vcardType, propertySelector);
    EXPECT_FALSE(isFiltered);
    vcardType = VCARD_FORMAT_VERSION_30;
    isFiltered = pbapPseVcardFilterTest->IsFilteredIn(filter, vcardType, propertySelector);
    EXPECT_FALSE(isFiltered);
    filter.excludeForV21 = false;
    isFiltered = pbapPseVcardFilterTest->IsFilteredIn(filter, vcardType, propertySelector);
    EXPECT_FALSE(isFiltered);
    filter.excludeForV21 = true;
    isFiltered = pbapPseVcardFilterTest->IsFilteredIn(filter, vcardType, propertySelector);
    EXPECT_FALSE(isFiltered);
    HILOGI("PbapPseVcardFilterTest_IsFilteredIn_00100 end");
}

/*
 * @tc.number: PbapPseVcardFilterTest_Apply_00100
 * @tc.name: Apply
 * @tc.desc: Apply
*/
HWTEST_F(PbapPseVcardFilterTest, PbapPseVcardFilterTest_Apply_00100, TestSize.Level0)
{
    HILOGI("PbapPseVcardFilterTest_Apply_00100 start");
    std::string vCard = "";
    int32_t vcardType = 0;
    uint64_t propertySelector = 0;
    int32_t shareType = 0;
    std::string res = pbapPseVcardFilterTest->Apply(vCard, vcardType, propertySelector, shareType);
    EXPECT_EQ(res, vCard);
    propertySelector = 0x01;
    vCard = "BEGIN:VCARD\nVERSION:3.0\nN:测试1;;;;\nFN:测试1\nTEL;:156\nX_OHOS_CUSTOM:relation;人大乱;;1\
    \nX_OHOS_CUSTOM:nickname;果冻\nEMAIL:@qq.com;\nADR;TYPE=HOME:;西安;;;;;\
    \nORG;CHARSET=UTF-8:HW\nTITLE;CHARSET=UTF-8:工程师\
    \nNOTE:豆腐\\nX_OHOS_CUSTOM:contact_event;2023-12-13;1;1\nEND:VCARD";
    pbapPseVcardFilterTest->Apply(vCard, vcardType, propertySelector, shareType);
    EXPECT_NE(res, vCard);
    vCard = "\nBEGIN:VCARD\nVERSION:3.0\nN:测试1;;;;\nFN:测试1\nTEL;:156\nX_OHOS_CUSTOM:relation;人大乱;;1\
    \nX_OHOS_CUSTOM:nickname;果冻\nEMAIL:@qq.com;\nADR;TYPE=HOME:;西安;;;;;\
    \nORG;CHARSET=UTF-8:HW\nTITLE;CHARSET=UTF-8:工程师\
    \nNOTE:豆腐\\nX_OHOS_CUSTOM:contact_event;2023-12-13;1;1\nEND:VCARD";
    pbapPseVcardFilterTest->Apply(vCard, vcardType, propertySelector, shareType);
    EXPECT_NE(res, vCard);
    vCard = " \nBEGIN:VCARD\nVERSION:3.0\nN:测试1;;;;\nFN:测试1\nTEL;:156\nX_OHOS_CUSTOM:relation;人大乱;;1\
    \nX_OHOS_CUSTOM:nickname;果冻\nEMAIL:@qq.com;\nADR;TYPE=HOME:;西安;;;;;\
    \nORG;CHARSET=UTF-8:HW\nTITLE;CHARSET=UTF-8:工程师\
    \nNOTE:豆腐\\nX_OHOS_CUSTOM:contact_event;2023-12-13;1;1\nEND:VCARD";
    pbapPseVcardFilterTest->Apply(vCard, vcardType, propertySelector, shareType);
    EXPECT_NE(res, vCard);
    vCard = "=\nBEGIN:VCARD\nVERSION:3.0\nN:测试1;;;;\nFN:测试1\nTEL;:156\nX_OHOS_CUSTOM:relation;人大乱;;1\
    \nX_OHOS_CUSTOM:nickname;果冻\nEMAIL:@qq.com;\nADR;TYPE=HOME:;西安;;;;;\
    \nORG;CHARSET=UTF-8:HW\nTITLE;CHARSET=UTF-8:工程师\
    \nNOTE:豆腐\\nX_OHOS_CUSTOM:contact_event;2023-12-13;1;1\nEND:VCARD";
    pbapPseVcardFilterTest->Apply(vCard, vcardType, propertySelector, shareType);
    EXPECT_NE(res, vCard);
    vCard = ";\nBEGIN:VCARD\nVERSION:3.0\nN:测试1;;;;\nFN:测试1\nTEL;:156\nX_OHOS_CUSTOM:relation;人大乱;;1\
    \nX_OHOS_CUSTOM:nickname;果冻\nEMAIL:@qq.com;\nADR;TYPE=HOME:;西安;;;;;\
    \nORG;CHARSET=UTF-8:HW\nTITLE;CHARSET=UTF-8:工程师\
    \nNOTE:豆腐\\nX_OHOS_CUSTOM:contact_event;2023-12-13;1;1\nEND:VCARD";
    pbapPseVcardFilterTest->Apply(vCard, vcardType, propertySelector, shareType);
    EXPECT_NE(res, vCard);

    shareType = 1;
    vCard = ";\nBEGIN:VCARD\nVERSION:3.0\nN:测试1;;;;\nFN:测试1\nTEL;:156\nX_OHOS_CUSTOM:relation;人大乱;;1\
    \nX_OHOS_CUSTOM:nickname;果冻\nEMAIL:@qq.com;\nADR;TYPE=HOME:;西安;;;;;\
    \nORG;CHARSET=UTF-8:HW\nTITLE;CHARSET=UTF-8:工程师\
    \nNOTE:豆腐\\nX_OHOS_CUSTOM:contact_event;2023-12-13;1;1\n  \nEND:VCARD";
    res = pbapPseVcardFilterTest->Apply(vCard, vcardType, propertySelector, shareType);
    EXPECT_NE(res, vCard);
    HILOGI("PbapPseVcardFilterTest_Apply_00100 end");
}

}  // namespace Bluetooth
}  // namespace OHOS
