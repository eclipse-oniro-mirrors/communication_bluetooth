/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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

#undef private
#define private public

#include "fuzzer/FuzzedDataProvider.h"
#include "pbap_pse_datashare_utils_fuzzer.h"
#include "permission_manager.h"
#include "pbap_pse_datashare_utils.h"
#include "bluetooth_log.h"
#include "securec.h"

using namespace std;
using namespace OHOS::bluetooth;
namespace OHOS {

    void PbapPseDatashareUtilsFuzzerTest(std::shared_ptr<PbabPseDataShareUtils> pbabPseDataShareUtils,
        int32_t vcardType)
    {
        std::string uriStr = "testUri";
        pbabPseDataShareUtils->CreateDataShareHelper(uriStr);
        pbabPseDataShareUtils->GetCallLogDataShareHelper(uriStr);
        pbabPseDataShareUtils->GetContactDataShareHelper();
        DataShare::DataSharePredicates predicates;
        pbabPseDataShareUtils->ComposeContactVcard(vcardType, predicates);
        int32_t queryTableType = QUERY_TABLE_TYPE_CONTACT;
        std::vector<std::string> columns = {"contact_id", "raw_contact_id"};
        pbabPseDataShareUtils->GetQueryResultCount(queryTableType, uriStr, predicates, columns);
        queryTableType = QUERY_TABLE_TYPE_CALLLOG;
        pbabPseDataShareUtils->GetQueryResultCount(queryTableType, uriStr, predicates, columns);
        int32_t contentType = PbapContentType::FAVORITES;
        pbabPseDataShareUtils->GenerateCallLogParamByType(contentType, predicates);
        contentType = INCOMING_CALL_HISTORY;
        pbabPseDataShareUtils->GenerateCallLogParamByType(contentType, predicates);
        contentType = OUTGOING_CALL_HISTORY;
        pbabPseDataShareUtils->GenerateCallLogParamByType(contentType, predicates);
        contentType = MISSED_CALL_HISTORY;
        pbabPseDataShareUtils->GenerateCallLogParamByType(contentType, predicates);
        contentType = COMBINED_CALL_HISTORY;
        pbabPseDataShareUtils->GenerateCallLogParamByType(contentType, predicates);
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::shared_ptr<PbabPseDataShareUtils> pbabPseDataShareUtils = std::make_shared<PbabPseDataShareUtils>();
        int32_t number = provider.ConsumeIntegral<int32_t>();
        int32_t offset = provider.ConsumeIntegral<int32_t>();
        int32_t shareType = provider.ConsumeIntegral<int32_t>();
        std::string orderByStr = "testOrder";
        pbabPseDataShareUtils->CreatContactPredicates(number, offset, orderByStr, shareType);
        std::shared_ptr<DataShare::DataShareResultSet> resultSet = std::make_shared<DataShare::DataShareResultSet>();
        std::string fieldName = "testFieldName";
        std::string out = "1";
        pbabPseDataShareUtils->GetValue(resultSet, fieldName, out);
        int64_t out64 = provider.ConsumeIntegral<int32_t>();
        pbabPseDataShareUtils->GetValue(resultSet, fieldName, out64);
        int32_t out32 = provider.ConsumeIntegral<int32_t>();
        pbabPseDataShareUtils->GetValue(resultSet, fieldName, out32);
        int32_t contentType = provider.ConsumeIntegralInRange<int32_t>(0, 6);
        int32_t vcardType = provider.ConsumeIntegralInRange<int32_t>(0, 6);
        pbabPseDataShareUtils->ComposeCallLogVcard(contentType, vcardType, number, offset);
        pbabPseDataShareUtils->CreateOneCallLog(vcardType, resultSet);
        std::string name = "testName";
        std::string numberStr = std::to_string(number);
        pbabPseDataShareUtils->ComposeOwnerVcard(vcardType, name, numberStr);
        pbabPseDataShareUtils->GetCallLogTimePara(resultSet);
        pbabPseDataShareUtils->ToRfc2455Format(resultSet);
        pbabPseDataShareUtils->GetContactsSize(contentType);
        pbabPseDataShareUtils->GetCallHistorySize(contentType);
        pbabPseDataShareUtils->GetNewMissedCallNum();
        int64_t timeStamp = provider.ConsumeIntegral<int64_t>();
        pbabPseDataShareUtils->GetCallHistorySizeByTime(contentType, timeStamp);
        PbapPseDatashareUtilsFuzzerTest(pbabPseDataShareUtils, vcardType);
        std::vector<std::string> names = {"testName"};
        pbabPseDataShareUtils->GetCallHistoryList(names, contentType);
        pbabPseDataShareUtils->GetNameListBySet(names, resultSet);
        uint8_t orderBy = provider.ConsumeIntegralInRange<uint8_t>(0, 3);
        pbabPseDataShareUtils->GetContactNameList(names, orderBy);
        pbabPseDataShareUtils->GetContactNamesByNumber(numberStr, names);
        std::vector<int32_t> rawContactIds = {1, 2};
        pbabPseDataShareUtils->QueryRawContactIdsByNumber(numberStr, rawContactIds);
        int32_t rawContactId = 1;
        pbabPseDataShareUtils->QueryNameByRawContactId(rawContactId, names);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        HILOGI("invalid data");
        return 0;
    }
    /* Run your code on data */
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

