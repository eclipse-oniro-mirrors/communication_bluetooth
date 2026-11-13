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
#include "btcommon/timer_manager.h"
#include "bluetooth_log.h"
#include "pbap_pse_vcard_manager.h"
#include "pbap_pse_service.h"
#include "class_creator.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;

void *PbapPseServiceCreate(void)
{
    return new PbapPseService();
}

class PbabPseVcardManagerTest : public testing::Test {
public:
    PbabPseVcardManagerTest();
    ~PbabPseVcardManagerTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

PbabPseVcardManagerTest::PbabPseVcardManagerTest() {}

PbabPseVcardManagerTest::~PbabPseVcardManagerTest() {}

void PbabPseVcardManagerTest::SetUpTestCase(void)
{
    ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
}

void PbabPseVcardManagerTest::TearDownTestCase(void) {
    utility::CloseAllTimer();
}

void PbabPseVcardManagerTest::SetUp() {}

void PbabPseVcardManagerTest::TearDown() {}

/*
 * @tc.number: PbapPse_ComposePhonebookOneVcard_00100
 * @tc.name: ComposePhonebookOneVcard
 * @tc.desc: ComposePhonebookOneVcard
*/
HWTEST_F(PbabPseVcardManagerTest, PbapPse_ComposePhonebookOneVcard_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ComposePhonebookOneVcard_00100 start");
    PbapPseAppParam appParam;
    std::vector<uint8_t> output;
    EXPECT_EQ(PbabPseVcardManager::GetInstance()->ComposePhonebookOneVcard(appParam, 0, ORDER_BY_INDEXED, 0, output),
        ResponseCodes::OBEX_HTTP_INTERNAL_ERROR);
    EXPECT_EQ(PbabPseVcardManager::GetInstance()->ComposePhonebookOneVcard(appParam, 1, ORDER_BY_INDEXED, 0, output),
        ResponseCodes::OBEX_HTTP_OK);
    EXPECT_EQ(PbabPseVcardManager::GetInstance()->ComposePhonebookOneVcard(appParam, 1, ORDER_BY_ALPHABETICAL,
        0, output), ResponseCodes::OBEX_HTTP_OK);
    HILOGI("PbapPse_ComposePhonebookOneVcard_00100 end");
}

/*
 * @tc.number: PbapPse_GetContactNamesByNumber_00100
 * @tc.name: GetContactNamesByNumber
 * @tc.desc: GetContactNamesByNumber
*/
HWTEST_F(PbabPseVcardManagerTest, PbapPse_GetContactNamesByNumber_00100, TestSize.Level1)
{
    HILOGI("PbapPse_GetContactNamesByNumber_00100 start");
    std::vector<std::string> names = {};
    std::string number = "";
    PbabPseVcardManager::GetInstance()->GetContactNamesByNumber(names, number);
    EXPECT_EQ(names.size(), 0);

    number = "123456";
    PbabPseVcardManager::GetInstance()->GetContactNamesByNumber(names, number);
    EXPECT_GE(names.size(), 0);
    HILOGI("PbapPse_GetContactNamesByNumber_00100 end");
}

/*
 * @tc.number: PbapPse_GetPhonebookSize_00100
 * @tc.name: GetPhonebookSize
 * @tc.desc: GetPhonebookSize
*/
HWTEST_F(PbabPseVcardManagerTest, PbapPse_GetPhonebookSize_00100, TestSize.Level1)
{
    HILOGI("PbapPse_GetPhonebookSize_00100 start");
    EXPECT_GE(PbabPseVcardManager::GetInstance()->GetPhonebookSize(PbapContentType::PHONEBOOK), 0);
    EXPECT_GE(PbabPseVcardManager::GetInstance()->GetPhonebookSize(PbapContentType::FAVORITES), 0);
    EXPECT_GE(PbabPseVcardManager::GetInstance()->GetPhonebookSize(PbapContentType::COMBINED_CALL_HISTORY), 0);
    HILOGI("PbapPse_GetPhonebookSize_00100 end");
}

/*
 * @tc.number: PbapPse_ComposePhonebookVcards_00100
 * @tc.name: ComposePhonebookVcards
 * @tc.desc: ComposePhonebookVcards
*/
HWTEST_F(PbabPseVcardManagerTest, PbapPse_ComposePhonebookVcards_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ComposePhonebookVcards_00100 start");
    ObexHeader reply;
    std::vector<uint8_t> output;

    PbapPseAppParam appParam;
    appParam.contentType_ = PbapContentType::PHONEBOOK;
    appParam.listStartOffset_ = 0;
    CallLogSelectParam selectParam;
    selectParam.pbSize = 10;
    selectParam.startPoint = 0;
    selectParam.endPoint = 10;
    EXPECT_EQ(PbabPseVcardManager::GetInstance()->ComposePhonebookVcards(reply, appParam, selectParam, 0, output),
        ResponseCodes::OBEX_HTTP_OK);
    selectParam.pbSize = 110;
    selectParam.startPoint = 0;
    selectParam.endPoint = 110;
    EXPECT_EQ(PbabPseVcardManager::GetInstance()->ComposePhonebookVcards(reply, appParam, selectParam, 0, output),
        ResponseCodes::OBEX_HTTP_OK);
    HILOGI("PbapPse_ComposePhonebookVcards_00100 end");
}

/*
 * @tc.number: PbapPse_ReleaseDataShareHelper_00100
 * @tc.name: ReleaseDataShareHelper
 * @tc.desc: ReleaseDataShareHelper
*/
HWTEST_F(PbabPseVcardManagerTest, PbapPse_ReleaseDataShareHelper_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ReleaseDataShareHelper_00100 start");
    PbabPseVcardManager::GetInstance()->Init();
    PbabPseVcardManager::GetInstance()->ReleaseDataShareHelper();
    EXPECT_TRUE(PbabPseVcardManager::GetInstance()->GetReleaseDataShareHelperTimer() != nullptr);
    PbabPseVcardManager::GetInstance()->ReleaseDataShareHelper();
    HILOGI("PbapPse_ReleaseDataShareHelper_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS