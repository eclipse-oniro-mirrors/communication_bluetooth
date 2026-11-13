/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>

#include "bluetooth_errorcode.h"
#include "bluetooth_raw_address.h"
#include "hfp_ag_at_phonebook.h"
#include "hfp_ag_service.h"
#include "profile_service_manager.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

void *HfpAgServiceCreate(void)
{
    return new HfpAgService();
}

class HfpAgAtPhonebookTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        auto manager = ProfileServiceManager::GetInstance();
        manager->SetProfileService(ADAPTER_BREDR, "HfpAgService");
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestSuite start");
        utility::CloseAllTimer();
        HILOGI("TearDownTestSuite end");
    }
    void SetUp()
    {
    }
    void TearDown()
    {
        sleep(1);
    }
public:
    std::string address_ = "00:11:22:33:44:55";
    std::unique_ptr<HfpAgAtPhonebook> atPhoneBook_ = std::make_unique<HfpAgAtPhonebook>(address_);
};

/*
 * @tc.number: Hfp_Ag_At_Phonebook_GetAtCommandType_0001
 * @tc.name: GetAtCommandType
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_GetAtCommandType_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_GetAtCommandType_0001 start");
    EXPECT_EQ(atPhoneBook_->GetAtCommandType("+CPBS"), AtCommandType::TYPE_UNKNOWN);
    EXPECT_EQ(atPhoneBook_->GetAtCommandType("+CPBS?"), AtCommandType::TYPE_READ);
    EXPECT_EQ(atPhoneBook_->GetAtCommandType("+CPBS=?"), AtCommandType::TYPE_TEST);
    EXPECT_EQ(atPhoneBook_->GetAtCommandType("+CPBS="), AtCommandType::TYPE_SET);
    EXPECT_EQ(atPhoneBook_->GetAtCommandType("+CPBS#"), AtCommandType::TYPE_UNKNOWN);
    HILOGI("Hfp_Ag_At_Phonebook_GetAtCommandType_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_HandleCscsCommand_0001
 * @tc.name: HandleCscsCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_HandleCscsCommand_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_HandleCscsCommand_0001 start");
    std::string commond = "+CSCS#";
    atPhoneBook_->HandleCscsCommand(commond);

    commond = "+CSCS?";
    atPhoneBook_->HandleCscsCommand(commond);

    commond = "+CSCS=?";
    atPhoneBook_->HandleCscsCommand(commond);

    commond = "+CSCS=\"UTF-8\"";
    atPhoneBook_->HandleCscsCommand(commond);
    EXPECT_EQ(atPhoneBook_->characterSet_, "UTF-8");
    HILOGI("Hfp_Ag_At_Phonebook_HandleCscsCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_HandleCpbsCommand_0001
 * @tc.name: HandleCpbsCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_HandleCpbsCommand_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_HandleCpbsCommand_0001 start");
    std::string commond = "+CPBS#";
    atPhoneBook_->HandleCpbsCommand(commond);

    commond = "+CPBS?";
    atPhoneBook_->HandleCpbsCommand(commond);

    commond = "+CPBS=?";
    atPhoneBook_->HandleCpbsCommand(commond);

    commond = "+CPBS=\"ME\"";
    atPhoneBook_->HandleCpbsCommand(commond);
    EXPECT_EQ(atPhoneBook_->currentPhonebook_, "ME");
    HILOGI("Hfp_Ag_At_Phonebook_HandleCpbsCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_HandleCpbrCommand_0001
 * @tc.name: HandleCpbrCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_HandleCpbrCommand_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_HandleCpbrCommand_0001 start");
    std::string commond = "+CPBR#";
    atPhoneBook_->HandleCpbrCommand(commond);

    commond = "+CPBR=?";
    atPhoneBook_->HandleCpbrCommand(commond);

    commond = "+CPBR?";
    atPhoneBook_->HandleCpbrCommand(commond);

    commond = "+CPBR=";
    atPhoneBook_->HandleCpbrCommand(commond);
    EXPECT_EQ(atPhoneBook_->checkingAccessPermission_, false);
    HILOGI("Hfp_Ag_At_Phonebook_HandleCpbrCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_HandlePbAccessResult_0001
 * @tc.name: HandlePbAccessResult
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_HandlePbAccessResult_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_HandlePbAccessResult_0001 start");
    atPhoneBook_->HandlePbAccessResult(address_, 2, 0);
    atPhoneBook_->HandlePbAccessResult(address_, 2, 1);
    atPhoneBook_->HandlePbAccessResult(address_, 1, 0);
    atPhoneBook_->HandlePbAccessResult(address_, 0, 0);
    EXPECT_EQ(atPhoneBook_->checkingAccessPermission_, false);
    HILOGI("Hfp_Ag_At_Phonebook_HandlePbAccessResult_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_SetCharacterSet_0001
 * @tc.name: SetCharacterSet
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_SetCharacterSet_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_SetCharacterSet_0001 start");
    int32_t errorCode;
    std::string commond = "+CSCS=\"UTF-8\"";
    EXPECT_EQ(atPhoneBook_->SetCharacterSet(commond, errorCode), HFP_AG_RESULT_OK);

    commond = "+CSCS=\"GSM\"";
    EXPECT_EQ(atPhoneBook_->SetCharacterSet(commond, errorCode), HFP_AG_RESULT_OK);

    commond = "+CSCS=\"IRA\"";
    EXPECT_EQ(atPhoneBook_->SetCharacterSet(commond, errorCode), HFP_AG_RESULT_OK);

    commond = "+CSCS=\"UTF8\"";
    EXPECT_EQ(atPhoneBook_->SetCharacterSet(commond, errorCode), HFP_AG_RESULT_OK);

    commond = "+CSCS=";
    EXPECT_EQ(atPhoneBook_->SetCharacterSet(commond, errorCode), HFP_AG_RESULT_ERROR);

    commond = "+CSCS=ABC";
    EXPECT_EQ(atPhoneBook_->SetCharacterSet(commond, errorCode), HFP_AG_RESULT_ERROR);
    HILOGI("Hfp_Ag_At_Phonebook_SetCharacterSet_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_ReadCurrentPhonebookStorage_0001
 * @tc.name: ReadCurrentPhonebookStorage
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_ReadCurrentPhonebookStorage_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_ReadCurrentPhonebookStorage_0001 start");
    std::string atCommandResponse;
    atPhoneBook_->currentPhonebook_ = "SM";
    EXPECT_EQ(atPhoneBook_->ReadCurrentPhonebookStorage(atCommandResponse), HFP_AG_RESULT_OK);

    atPhoneBook_->currentPhonebook_ = "ME";
    EXPECT_EQ(atPhoneBook_->ReadCurrentPhonebookStorage(atCommandResponse), HFP_AG_RESULT_OK);
    HILOGI("Hfp_Ag_At_Phonebook_ReadCurrentPhonebookStorage_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_SetCurrentPhonebookStorage_0001
 * @tc.name: SetCurrentPhonebookStorage
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_SetCurrentPhonebookStorage_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_SetCurrentPhonebookStorage_0001 start");
    int32_t errorCode;
    std::string atCommand = "+CPBS=";
    EXPECT_EQ(atPhoneBook_->SetCurrentPhonebookStorage(atCommand, errorCode), HFP_AG_RESULT_ERROR);

    atCommand = "+CPBS=\"AB\"";
    EXPECT_EQ(atPhoneBook_->SetCurrentPhonebookStorage(atCommand, errorCode), HFP_AG_RESULT_ERROR);

    atCommand = "+CPBS=\"SM\"";
    EXPECT_EQ(atPhoneBook_->SetCurrentPhonebookStorage(atCommand, errorCode), HFP_AG_RESULT_OK);

    atCommand = "+CPBS=\"ME\"";
    EXPECT_EQ(atPhoneBook_->SetCurrentPhonebookStorage(atCommand, errorCode), HFP_AG_RESULT_OK);
    HILOGI("Hfp_Ag_At_Phonebook_SetCurrentPhonebookStorage_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_ReadCurrentPhonebookInfo_0001
 * @tc.name: ReadCurrentPhonebookInfo
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_ReadCurrentPhonebookInfo_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_ReadCurrentPhonebookInfo_0001 start");
    int32_t errorCode;
    std::string atCommandResponse = "";
    atPhoneBook_->currentPhonebook_ = "SM";
    EXPECT_EQ(atPhoneBook_->ReadCurrentPhonebookInfo(atCommandResponse, errorCode), HFP_AG_RESULT_OK);

    atPhoneBook_->currentPhonebook_ = "AB";
    EXPECT_EQ(atPhoneBook_->ReadCurrentPhonebookInfo(atCommandResponse, errorCode), HFP_AG_RESULT_ERROR);

    atPhoneBook_->currentPhonebook_ = "ME";
    EXPECT_EQ(atPhoneBook_->ReadCurrentPhonebookInfo(atCommandResponse, errorCode), HFP_AG_RESULT_OK);
    HILOGI("Hfp_Ag_At_Phonebook_ReadCurrentPhonebookInfo_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_ReadCurrentPhonebookEntries_0001
 * @tc.name: ReadCurrentPhonebookEntries
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_ReadCurrentPhonebookEntries_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_ReadCurrentPhonebookEntries_0001 start");
    int32_t errorCode = 0;
    std::string atCommand = "+CPBR=1,2";
    bool isRequestPermission = false;

    atPhoneBook_->cpbrIndex1_ = 1;
    EXPECT_EQ(atPhoneBook_->ReadCurrentPhonebookEntries(atCommand, errorCode, isRequestPermission),
        HFP_AG_RESULT_ERROR);

    atPhoneBook_->cpbrIndex1_ = -1;
    atPhoneBook_->ReadCurrentPhonebookEntries(atCommand, errorCode, isRequestPermission);

    atCommand = "+CPBR=";
    EXPECT_EQ(atPhoneBook_->ReadCurrentPhonebookEntries(atCommand, errorCode, isRequestPermission),
        HFP_AG_RESULT_ERROR);
    HILOGI("Hfp_Ag_At_Phonebook_ReadCurrentPhonebookEntries_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_CheckPhonebook_0001
 * @tc.name: CheckPhonebook
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_CheckPhonebook_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_CheckPhonebook_0001 start");
    EXPECT_EQ(atPhoneBook_->CheckPhonebook("ME"), true);
    EXPECT_EQ(atPhoneBook_->CheckPhonebook("DC"), true);
    EXPECT_EQ(atPhoneBook_->CheckPhonebook("RC"), true);
    EXPECT_EQ(atPhoneBook_->CheckPhonebook("MC"), true);
    EXPECT_EQ(atPhoneBook_->CheckPhonebook("SM"), false);
    HILOGI("Hfp_Ag_At_Phonebook_CheckPhonebook_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_GetMaxPhoneBookSize_0001
 * @tc.name: GetMaxPhoneBookSize
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_GetMaxPhoneBookSize_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_GetMaxPhoneBookSize_0001 start");
    EXPECT_EQ(atPhoneBook_->GetMaxPhoneBookSize(0), 256);
    HILOGI("Hfp_Ag_At_Phonebook_GetMaxPhoneBookSize_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_GetPhonebookType_0001
 * @tc.name: GetPhonebookType
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_GetPhonebookType_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_GetPhonebookType_0001 start");
    atPhoneBook_->currentPhonebook_ = "SM";
    EXPECT_EQ(atPhoneBook_->GetPhonebookType(), 0);

    atPhoneBook_->currentPhonebook_ = "MC";
    EXPECT_EQ(atPhoneBook_->GetPhonebookType(), PhonebookType::MISSED_CALL_LOG);

    atPhoneBook_->currentPhonebook_ = "DC";
    EXPECT_EQ(atPhoneBook_->GetPhonebookType(), PhonebookType::OUTGOING_CALL_LOG);

    atPhoneBook_->currentPhonebook_ = "RC";
    EXPECT_EQ(atPhoneBook_->GetPhonebookType(), PhonebookType::INCOMING_CALL_LOG);

    atPhoneBook_->currentPhonebook_ = "ME";
    EXPECT_EQ(atPhoneBook_->GetPhonebookType(), PhonebookType::PHONEBOOK);
    HILOGI("Hfp_Ag_At_Phonebook_GetPhonebookType_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_ParseCpbrIndex_0001
 * @tc.name: ParseCpbrIndex
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_ParseCpbrIndex_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_ParseCpbrIndex_0001 start");
    std::string atCommand = "+CPBR=";
    EXPECT_EQ(atPhoneBook_->ParseCpbrIndex(atCommand), false);

    atCommand = "+CPBR=1";
    EXPECT_EQ(atPhoneBook_->ParseCpbrIndex(atCommand), true);
    EXPECT_EQ(atPhoneBook_->cpbrIndex1_, 1);
    EXPECT_EQ(atPhoneBook_->cpbrIndex2_, 1);

    atCommand = "+CPBR=1,4";
    EXPECT_EQ(atPhoneBook_->ParseCpbrIndex(atCommand), true);
    EXPECT_EQ(atPhoneBook_->cpbrIndex1_, 1);
    EXPECT_EQ(atPhoneBook_->cpbrIndex2_, 4);

    atCommand = "+CPBR=a";
    EXPECT_EQ(atPhoneBook_->ParseCpbrIndex(atCommand), false);

    atCommand = "+CPBR=1,a";
    EXPECT_EQ(atPhoneBook_->ParseCpbrIndex(atCommand), false);
    HILOGI("Hfp_Ag_At_Phonebook_ParseCpbrIndex_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_ProcessCpbrCommand_0001
 * @tc.name: ProcessCpbrCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_ProcessCpbrCommand_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_ProcessCpbrCommand_0001 start");
    atPhoneBook_->currentPhonebook_ = "SM";
    EXPECT_EQ(atPhoneBook_->ProcessCpbrCommand(), HFP_AG_RESULT_OK);

    atPhoneBook_->currentPhonebook_ = "AB";
    EXPECT_EQ(atPhoneBook_->ProcessCpbrCommand(), HFP_AG_RESULT_ERROR);

    atPhoneBook_->currentPhonebook_ = "ME";
    EXPECT_EQ(atPhoneBook_->ProcessCpbrCommand(), HFP_AG_RESULT_OK);
    HILOGI("Hfp_Ag_At_Phonebook_ProcessCpbrCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_FormatNameInfo_0001
 * @tc.name: FormatNameInfo
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_FormatNameInfo_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_FormatNameInfo_0001 start");
    std::string name = "zhangsan";
    std::string labelId = "";
    EXPECT_EQ(atPhoneBook_->FormatNameInfo(name, labelId, PhonebookType::INCOMING_CALL_LOG).length(), 8);

    name = "zhangsanzhangsanzhangsanzhangsan";
    EXPECT_EQ(atPhoneBook_->FormatNameInfo(name, labelId, PhonebookType::PHONEBOOK).length(), 30);

    labelId = "1";
    EXPECT_EQ(atPhoneBook_->FormatNameInfo(name, labelId, PhonebookType::PHONEBOOK).length(), 30);
    HILOGI("Hfp_Ag_At_Phonebook_FormatNameInfo_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_FormatNumberInfo_0001
 * @tc.name: FormatNumberInfo
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_FormatNumberInfo_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_FormatNumberInfo_0001 start");
    std::string number = "123456789";
    EXPECT_EQ(atPhoneBook_->FormatNumberInfo(number).length(), 9);

    number = "0123456789012345678901234567890";
    EXPECT_EQ(atPhoneBook_->FormatNumberInfo(number).length(), 30);
    HILOGI("Hfp_Ag_At_Phonebook_FormatNumberInfo_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_ToaFromNumber_0001
 * @tc.name: ToaFromNumber
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_ToaFromNumber_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_ToaFromNumber_0001 start");
    std::string number = "";
    EXPECT_EQ(atPhoneBook_->ToaFromNumber(number), 0x81);

    number = "123";
    EXPECT_EQ(atPhoneBook_->ToaFromNumber(number), 0x81);

    number = "+123";
    EXPECT_EQ(atPhoneBook_->ToaFromNumber(number), 0x91);
    HILOGI("Hfp_Ag_At_Phonebook_ToaFromNumber_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_GetPhoneType_0001
 * @tc.name: GetPhoneType
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_GetPhoneType_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_GetPhoneType_0001 start");
    EXPECT_EQ(atPhoneBook_->GetPhoneType(1), "H");
    EXPECT_EQ(atPhoneBook_->GetPhoneType(2), "M");
    EXPECT_EQ(atPhoneBook_->GetPhoneType(3), "W");
    EXPECT_EQ(atPhoneBook_->GetPhoneType(4), "F");
    EXPECT_EQ(atPhoneBook_->GetPhoneType(5), "F");
    EXPECT_EQ(atPhoneBook_->GetPhoneType(7), "O");
    EXPECT_EQ(atPhoneBook_->GetPhoneType(0), "O");
    EXPECT_EQ(atPhoneBook_->GetPhoneType(8), "O");
    HILOGI("Hfp_Ag_At_Phonebook_GetPhoneType_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_IsNeedSetPermissionToUnknown_0001
 * @tc.name: IsNeedSetPermissionToUnknown
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_IsNeedSetPermissionToUnknown_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_IsNeedSetPermissionToUnknown_0001 start");
    std::string addr = "00:11:22:33:44:55";
    EXPECT_EQ(atPhoneBook_->IsNeedSetPermissionToUnknown(addr, 1), false);
    EXPECT_EQ(atPhoneBook_->IsNeedSetPermissionToUnknown(addr, 2), true);
    HILOGI("Hfp_Ag_At_Phonebook_IsNeedSetPermissionToUnknown_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_GenerateCallLogPredicates_0001
 * @tc.name: GenerateCallLogPredicates
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_GenerateCallLogPredicates_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_GenerateCallLogPredicates_0001 start");
    DataShare::DataSharePredicates predicates;
    atPhoneBook_->GenerateCallLogPredicates(PhonebookType::INCOMING_CALL_LOG, predicates);
    atPhoneBook_->GenerateCallLogPredicates(PhonebookType::OUTGOING_CALL_LOG, predicates);
    atPhoneBook_->GenerateCallLogPredicates(PhonebookType::MISSED_CALL_LOG, predicates);
    atPhoneBook_->GenerateCallLogPredicates(PhonebookType::COMBINED_CALL_LOG, predicates);
    atPhoneBook_->GenerateCallLogPredicates(PhonebookType::PHONEBOOK, predicates);
    HILOGI("Hfp_Ag_At_Phonebook_GenerateCallLogPredicates_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_GetPhonebookCount_0001
 * @tc.name: GetPhonebookCount
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_GetPhonebookCount_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_GetPhonebookCount_0001 start");
    EXPECT_GE(atPhoneBook_->GetPhonebookCount(PhonebookType::PHONEBOOK), 0);
    EXPECT_GE(atPhoneBook_->GetPhonebookCount(PhonebookType::INCOMING_CALL_LOG), 0);
    HILOGI("Hfp_Ag_At_Phonebook_GetPhonebookCount_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_NotifyNewAtCommandRecv_0001
 * @tc.name: NotifyNewAtCommandRecv
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_NotifyNewAtCommandRecv_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_NotifyNewAtCommandRecv_0001 start");
    atPhoneBook_->checkingAccessPermission_ = false;
    atPhoneBook_->NotifyNewAtCommandRecv();

    atPhoneBook_->checkingAccessPermission_ = true;
    atPhoneBook_->NotifyNewAtCommandRecv();
    EXPECT_EQ(atPhoneBook_->checkingAccessPermission_, false);
    HILOGI("Hfp_Ag_At_Phonebook_NotifyNewAtCommandRecv_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_ComposeContactInfo_0001
 * @tc.name: ComposeContactInfo
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_ComposeContactInfo_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_ComposeContactInfo_0001 start");
    int32_t offset = 0;
    int32_t number = 1;
    int rest = atPhoneBook_->ComposeContactInfo(offset, number);
    EXPECT_EQ(rest, HFP_AG_RESULT_ERROR);
    HILOGI("Hfp_Ag_At_Phonebook_ComposeContactInfo_0001 end");
}

/*
 * @tc.number: Hfp_Ag_At_Phonebook_RequestPermissionTimeout_0001
 * @tc.name: RequestPermissionTimeout
 * @tc.desc:
 */
HWTEST_F(HfpAgAtPhonebookTest, Hfp_Ag_At_Phonebook_RequestPermissionTimeout_0001, TestSize.Level0)
{
    HILOGI("Hfp_Ag_At_Phonebook_RequestPermissionTimeout_0001 start");
    atPhoneBook_->RequestPermissionTimeout();
    atPhoneBook_->StopRequestPermissionTimer();
    EXPECT_FALSE(atPhoneBook_->IsCheckingAccessPermission());
    HILOGI("Hfp_Ag_At_Phonebook_RequestPermissionTimeout_0001 end");
}
}  // namespace bluetooth
}  // namespace OHOS