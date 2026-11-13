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
#include "pbap_pse_datashare_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;

class PbabPseDataShareUtilsTest : public testing::Test {
public:
    PbabPseDataShareUtilsTest();
    ~PbabPseDataShareUtilsTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
    std::shared_ptr<PbabPseDataShareUtils> datashareUtilsTest_ = nullptr;
};

PbabPseDataShareUtilsTest::PbabPseDataShareUtilsTest() {}

PbabPseDataShareUtilsTest::~PbabPseDataShareUtilsTest() {}

void PbabPseDataShareUtilsTest::SetUpTestCase(void) {}

void PbabPseDataShareUtilsTest::TearDownTestCase(void) {}

void PbabPseDataShareUtilsTest::SetUp()
{
    datashareUtilsTest_ = std::make_shared<PbabPseDataShareUtils>();
}

void PbabPseDataShareUtilsTest::TearDown()
{
    datashareUtilsTest_ = nullptr;
}

/*
 * @tc.number: PbapPse_GetContactsSize_00100
 * @tc.name: GetContactsSize
 * @tc.desc: GetContactsSize
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_GetContactsSize_00100, TestSize.Level1)
{
    HILOGI("PbapPse_GetContactsSize_00100 start");
    int32_t result = datashareUtilsTest_->GetContactsSize(PbapContentType::PHONEBOOK);
    EXPECT_GE(result, 0);
    
    result = datashareUtilsTest_->GetContactsSize(PbapContentType::FAVORITES);
    EXPECT_GE(result, 0);
    HILOGI("PbapPse_GetContactsSize_00100 end");
}

/*
 * @tc.number: PbapPse_GetCallLogDataShareHelper_00100
 * @tc.name: GetCallLogDataShareHelper
 * @tc.desc: GetCallLogDataShareHelper
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_GetCallLogDataShareHelper_00100, TestSize.Level1)
{
    HILOGI("PbapPse_GetCallLogDataShareHelper_00100 start");
    std::string url = "";
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = datashareUtilsTest_->GetCallLogDataShareHelper(url);
    EXPECT_TRUE(dataShareHelper != nullptr);
    datashareUtilsTest_->GetContactDataShareHelper();
    HILOGI("PbapPse_GetCallLogDataShareHelper_00100 end");
}

/*
 * @tc.number: PbapPse_GenerateCallLogParamByType_00100
 * @tc.name: GenerateCallLogParamByType
 * @tc.desc: GenerateCallLogParamByType
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_GenerateCallLogParamByType_00100, TestSize.Level1)
{
    HILOGI("PbapPse_GenerateCallLogParamByType_00100 start");
    std::string orderByStr = "";
    datashareUtilsTest_->CreatContactPredicates(1, 0, orderByStr, 0);
    orderByStr = "primary_contact";
    DataShare::DataSharePredicates predicates = datashareUtilsTest_->CreatContactPredicates(1, 0, orderByStr, 0);
    EXPECT_TRUE(datashareUtilsTest_ != nullptr);
    datashareUtilsTest_->GenerateCallLogParamByType(INCOMING_CALL_HISTORY, predicates);
    datashareUtilsTest_->GenerateCallLogParamByType(OUTGOING_CALL_HISTORY, predicates);
    datashareUtilsTest_->GenerateCallLogParamByType(MISSED_CALL_HISTORY, predicates);
    datashareUtilsTest_->GenerateCallLogParamByType(COMBINED_CALL_HISTORY, predicates);
    datashareUtilsTest_->GenerateCallLogParamByType(PHONEBOOK, predicates);
    HILOGI("PbapPse_GenerateCallLogParamByType_00100 end");
}

/*
 * @tc.number: PbapPse_GetContactNameList_00100
 * @tc.name: GetContactNameList
 * @tc.desc: GetContactNameList
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_GetContactNameList_00100, TestSize.Level1)
{
    HILOGI("PbapPse_GetContactNameList_00100 start");
    std::vector<std::string> names;
    datashareUtilsTest_->GetContactNameList(names, ORDER_BY_ALPHABETICAL);
    EXPECT_GE(names.size(), 0);

    datashareUtilsTest_->GetContactNameList(names, ORDER_BY_INDEXED);
    EXPECT_GE(names.size(), 0);
    HILOGI("PbapPse_GetContactNameList_00100 end");
}

/*
 * @tc.number: PbapPse_GetContactNamesByNumber_00100
 * @tc.name: GetContactNamesByNumber
 * @tc.desc: GetContactNamesByNumber
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_GetContactNamesByNumber_00100, TestSize.Level1)
{
    HILOGI("PbapPse_GetContactNamesByNumber_00100 start");
    std::vector<std::string> names;
    std::string number = "123456";
    datashareUtilsTest_->GetContactNamesByNumber(number, names);
    EXPECT_GE(names.size(), 0);
    HILOGI("PbapPse_GetContactNamesByNumber_00100 end");
}

/*
 * @tc.number: PbapPse_QueryRawContactIdsByNumber_00100
 * @tc.name: QueryRawContactIdsByNumber
 * @tc.desc: QueryRawContactIdsByNumber
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_QueryRawContactIdsByNumber_00100, TestSize.Level1)
{
    HILOGI("PbapPse_QueryRawContactIdsByNumber_00100 start");
    std::vector<int32_t> rawContactIds;
    std::string number = "123456";
    datashareUtilsTest_->QueryRawContactIdsByNumber(number, rawContactIds);
    EXPECT_GE(rawContactIds.size(), 0);
    number = "1234567890";
    datashareUtilsTest_->QueryRawContactIdsByNumber(number, rawContactIds);
    EXPECT_GE(rawContactIds.size(), 0);
    number = "13100000000";
    datashareUtilsTest_->QueryRawContactIdsByNumber(number, rawContactIds);
    EXPECT_GE(rawContactIds.size(), 0);
    number = "+8613000000000";
    datashareUtilsTest_->QueryRawContactIdsByNumber(number, rawContactIds);
    EXPECT_GE(rawContactIds.size(), 0);
    number = "130-0000-0000";
    datashareUtilsTest_->QueryRawContactIdsByNumber(number, rawContactIds);
    number = "12346";
    datashareUtilsTest_->QueryRawContactIdsByNumber(number, rawContactIds);
    EXPECT_GE(rawContactIds.size(), 0);
    HILOGI("PbapPse_QueryRawContactIdsByNumber_00100 end");
}

/*
 * @tc.number: PbapPse_QueryNameByRawContactId_00100
 * @tc.name: QueryNameByRawContactId
 * @tc.desc: QueryNameByRawContactId
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_QueryNameByRawContactId_00100, TestSize.Level1)
{
    HILOGI("PbapPse_QueryNameByRawContactId_00100 start");
    std::vector<std::string> names;
    int32_t rawContactId = 1;
    datashareUtilsTest_->QueryNameByRawContactId(rawContactId, names);
    EXPECT_GE(names.size(), 0);
    HILOGI("PbapPse_QueryNameByRawContactId_00100 end");
}

/*
 * @tc.number: PbapPse_ComposeOwnerVcard_00100
 * @tc.name: ComposeOwnerVcard
 * @tc.desc: ComposeOwnerVcard
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_ComposeOwnerVcard_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ComposeOwnerVcard_00100 start");
    int32_t vcardType = 0;
    std::string name = "";
    std::string number = "";
    std::string vcard = datashareUtilsTest_->ComposeOwnerVcard(vcardType, name, number);
    EXPECT_EQ(vcard, "");
    HILOGI("PbapPse_ComposeOwnerVcard_00100 end");
}

/*
 * @tc.number: PbapPse_ComposeOwnerVcard_00101
 * @tc.name: ComposeOwnerVcard
 * @tc.desc: ComposeOwnerVcard
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_ComposeOwnerVcard_00101, TestSize.Level1)
{
    HILOGI("PbapPse_ComposeOwnerVcard_00101 start");
    int32_t vcardType = 0;
    std::string name = "test_user_name_1";
    std::string number = "12345678900";
    std::string vcard = datashareUtilsTest_->ComposeOwnerVcard(vcardType, name, number);
    EXPECT_EQ(vcard, "BEGIN:VCARD\r\nVERSION:2.1\r\nFN:test_user_name_1\r\nTEL;HOME:12345678900\r\nEND:VCARD\r\n");
    HILOGI("PbapPse_ComposeOwnerVcard_00101 end");
}

/*
 * @tc.number: PbapPse_ComposeOwnerVcard_00102
 * @tc.name: ComposeOwnerVcard
 * @tc.desc: ComposeOwnerVcard
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_ComposeOwnerVcard_00102, TestSize.Level1)
{
    HILOGI("PbapPse_ComposeOwnerVcard_00102 start");
    int32_t vcardType = 0;
    std::string name = "";
    std::string number = "12345678900";
    std::string vcard = datashareUtilsTest_->ComposeOwnerVcard(vcardType, name, number);
    EXPECT_EQ(vcard, "BEGIN:VCARD\r\nVERSION:2.1\r\nTEL;HOME:12345678900\r\nEND:VCARD\r\n");
    HILOGI("PbapPse_ComposeOwnerVcard_00102 end");
}

/*
 * @tc.number: PbapPse_GetQueryResultCount_00100
 * @tc.name: GetQueryResultCount
 * @tc.desc: GetQueryResultCount
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_GetQueryResultCount_00100, TestSize.Level1)
{
    HILOGI("PbapPse_GetQueryResultCount_00100 start");
    std::vector<std::string> columns = {"id"};
    std::string url = "datashare:///com.ohos.contactsdataability/contacts/contact";
    DataShare::DataSharePredicates predicates;
    EXPECT_GE(datashareUtilsTest_->GetQueryResultCount(QUERY_TABLE_TYPE_CONTACT, url, predicates, columns), 0);
    EXPECT_GE(datashareUtilsTest_->GetQueryResultCount(2, url, predicates, columns), 0);
    HILOGI("PbapPse_GetQueryResultCount_00100 end");
}

/*
 * @tc.number: PbapPse_GetQueryResultCount_00200
 * @tc.name: GetQueryResultCount
 * @tc.desc: GetQueryResultCount
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_GetQueryResultCount_00200, TestSize.Level1)
{
    HILOGI("PbapPse_GetQueryResultCount_00200 start");
    std::vector<std::string> columns = {"id"};
    std::string url = "datashare:///com.ohos.contactsdataability";
    DataShare::DataSharePredicates predicates;
    EXPECT_GE(datashareUtilsTest_->GetQueryResultCount(QUERY_TABLE_TYPE_CONTACT, url, predicates, columns), 0);
    HILOGI("PbapPse_GetQueryResultCount_00200 end");
}

/*
 * @tc.number: PbapPse_ReleaseCallLogDataShareHelper_00100
 * @tc.name: ReleaseCallLogDataShareHelper
 * @tc.desc: ReleaseCallLogDataShareHelper
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_ReleaseCallLogDataShareHelper_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ReleaseCallLogDataShareHelper_00100 start");
    std::string url = "datashare:///com.ohos.contactsdataability";
    EXPECT_TRUE(datashareUtilsTest_->GetCallLogDataShareHelper(url) != nullptr);
    datashareUtilsTest_->ReleaseCallLogDataShareHelper();
    datashareUtilsTest_->ReleaseContactDataShareHelper();
    HILOGI("PbapPse_ReleaseCallLogDataShareHelper_00100 end");
}

/*
 * @tc.number: PbapPse_ReleaseCallLogDataShareHelper_00200
 * @tc.name: ReleaseContactDataShareHelper
 * @tc.desc: ReleaseContactDataShareHelper
*/
HWTEST_F(PbabPseDataShareUtilsTest, PbapPse_ReleaseCallLogDataShareHelper_00200, TestSize.Level1)
{
    HILOGI("PbapPse_ReleaseCallLogDataShareHelper_00200 start");
    EXPECT_FALSE(datashareUtilsTest_->GetContactDataShareHelper() != nullptr);
    datashareUtilsTest_->ReleaseCallLogDataShareHelper();
    datashareUtilsTest_->ReleaseContactDataShareHelper();
    HILOGI("PbapPse_ReleaseCallLogDataShareHelper_00200 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
