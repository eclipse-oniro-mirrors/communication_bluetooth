/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "hfpagservice_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "hfp_ag_system_event_processer.h"
#include "hfp_ag_at_phonebook.h"
#include "bluetooth_log.h"
#include "class_creator.h"
#include "profile_service_manager.h"
#include "btcommon/timer_manager.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
using namespace OHOS::bluetooth;

namespace OHOS {

    void *HfpAgSystemEventProcesserCreate()
    {
        return new bluetooth::HfpAgSystemEventProcesser("");
    }

    void TestHfpAgPhoneBook(FuzzedDataProvider& provider)
    {
        std::string address = BuildAddressString(provider);
        HfpAgAtPhonebook phoneBook(address);
        phoneBook.StopRequestPermissionTimer();
        phoneBook.RequestPermissionTimeout();
        std::string atStr = "+XSVENDORF=6,0";
        phoneBook.ProcessAtResponseStringEvent(atStr);
        phoneBook.ProcessAtResponseCodeEvent(0, 0);
        phoneBook.GetPhonebookCount(PhonebookType::PHONEBOOK);
        phoneBook.GetPhonebookCount(PhonebookType::INCOMING_CALL_LOG);
        phoneBook.IsNeedSetPermissionToUnknown(address, 0);
        phoneBook.CheckAccessPermission(address);
        int32_t type = provider.ConsumeIntegralInRange<int32_t>(0, 7);
        phoneBook.GetPhoneType(type);
        phoneBook.FormatNumberInfo(atStr);
        std::string labelId = "";
        phoneBook.FormatNameInfo(atStr, labelId, PhonebookType::PHONEBOOK);
        phoneBook.ComposeCallLogInfo(0, 0, 0);
        phoneBook.ProcessCpbrCommand();
        bool isRequestPermission = provider.ConsumeBool();
        phoneBook.ProcessPbAccessPermission(isRequestPermission);
        phoneBook.SetCheckingAccessPermission(provider.ConsumeBool());
        phoneBook.IsCheckingAccessPermission();
        phoneBook.ResetCpbrIndex();
        phoneBook.FormatStr(atStr);
        phoneBook.ParseCpbrIndex(atStr);
        phoneBook.GetPhonebookOrCalllogCount();
        std::string cmdStr = atStr;
        phoneBook.CheckPhonebook(cmdStr);
        std::string retCmd = "";
        int32_t errorCode = provider.ConsumeIntegralInRange<int32_t>(0, 32);
        phoneBook.ReadCurrentPhonebookEntries(retCmd, errorCode, isRequestPermission);
        phoneBook.ReadCurrentPhonebookInfo(retCmd, errorCode);
        phoneBook.SetCurrentPhonebookStorage(retCmd, errorCode);
        phoneBook.ReadCurrentPhonebookStorage(retCmd);
        phoneBook.HandleCpbrCommand(atStr);
        phoneBook.HandleCpbsCommand(atStr);
        phoneBook.HandleCscsCommand(atStr);
        phoneBook.GetAtCommandType(atStr);
    }

    void TestHfpAgPhoneBook2(FuzzedDataProvider& provider)
    {
        std::string address = BuildAddressString(provider);
        HfpAgAtPhonebook phoneBook(address);
        int32_t access = provider.ConsumeIntegralInRange<int32_t>(0, 2);
        int32_t pbapLoadFlag = provider.ConsumeIntegralInRange<int32_t>(0, 1);
        phoneBook.HandlePbAccessResult(address, access, pbapLoadFlag);
        access = static_cast<int>(BTPermissionType::ACCESS_ALLOWED);
        phoneBook.HandlePbAccessResult(address, access, pbapLoadFlag);
        phoneBook.NotifyNewAtCommandRecv();
        phoneBook.currentPhonebook_ = "DC";
        phoneBook.GetPhonebookType();
        phoneBook.currentPhonebook_ = "MC";
        phoneBook.GetPhonebookType();
        int32_t offset = provider.ConsumeIntegral<int32_t>();
        int32_t number = provider.ConsumeIntegral<int32_t>();
        phoneBook.ComposeContactInfo(offset, number);
        std::shared_ptr<DataShare::DataShareHelper> datashare;
        int32_t contactId = provider.ConsumeIntegral<int32_t>();
        int32_t index = provider.ConsumeIntegral<int32_t>();
        phoneBook.ComposeAndSendOneContactInfo(datashare, contactId, index);
        int32_t rawContactId =  provider.ConsumeIntegral<int32_t>();
        int32_t typeId = provider.ConsumeIntegralInRange<int32_t>(5, 6);
        std::string outInfo = "testOutInfo";
        std::string labelId = "1";
        phoneBook.GetContactInfoByTypeId(datashare, rawContactId, typeId, outInfo, labelId);
        std::shared_ptr<DataShare::DataShareResultSet> resultSet =
            std::make_shared<DataShare::DataShareResultSet>();
        int8_t pbType = provider.ConsumeIntegral<int8_t>();
        phoneBook.ComposeAndSendOneCallLogInfo(resultSet, pbType, index);
        std::string numberStr = "1";
        phoneBook.ToaFromNumber(numberStr);
        std::string fieldName = "testFieldName";
        std::string outStr = "1";
        std::string defaultValueStr = "1";
        phoneBook.GetValue(resultSet, fieldName, outStr, defaultValueStr);
        int32_t out = provider.ConsumeIntegral<int32_t>();
        int32_t defaultValue = provider.ConsumeIntegral<int32_t>();
        phoneBook.GetValue(resultSet, fieldName, out, defaultValue);
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        auto manager = ProfileServiceManager::GetInstance();
        std::string profileName = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, profileName);

        FuzzedDataProvider provider(data, size);
        std::string addr = BuildAddressString(provider);
        bluetooth::ClassFactory::RegisterClass("HfpAgSystemEventProcesser", HfpAgSystemEventProcesserCreate);
        bluetooth::HfpAgSystemEventProcesser hfpAgProcesser(addr);
        std::string atArgument = "4,0040068c041903";
        bool result = hfpAgProcesser.ProcessHwDeviceInfo(atArgument);
        HILOGI("ProcessHwDeviceInfo result: %{public}d", result);
        TestHfpAgPhoneBook(provider);
        TestHfpAgPhoneBook2(provider);
        return result;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    /* Run your code on data */
    PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    utility::CloseAllTimer();
    return 0;
}