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

#include "map_mse_appparams_fuzzer.h"
#include "permission_manager.h"
#include "map_mse_appparams.h"
#include "bluetooth_log.h"
#include "securec.h"
#include <map>
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;

map<uint8_t, uint8_t> appParamsFuncs = {
    {OHOS::bluetooth::PARAM_MAX_LIST_COUNT, OHOS::bluetooth::TAG_LEN_MAX_LIST_COUNT},
    {OHOS::bluetooth::PARAM_LIST_START_OFF_SET, OHOS::bluetooth::TAG_LEN_LIST_START_OFF_SET},
    {OHOS::bluetooth::PARAM_FILTER_MESSAGE_TYPE, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_FILTER_PERIOD_BEGIN, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_FILTER_PERIOD_END, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_FILTER_READ_STATUS, OHOS::bluetooth::TAG_LEN_FILTER_READ_STATUS},
    {OHOS::bluetooth::PARAM_FILTER_RECIPIENT, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_FILTER_ORIGINATOR, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_FILTER_PRIORITY, OHOS::bluetooth::TAG_LEN_FILTER_PRIORITY},
    {OHOS::bluetooth::PARAM_ATTACHMENT, OHOS::bluetooth::TAG_LEN_ATTACHMENT},
    {OHOS::bluetooth::PARAM_TRANSPARENT, OHOS::bluetooth::TAG_LEN_TRANSPARENT},
    {OHOS::bluetooth::PARAM_RETRY, OHOS::bluetooth::TAG_LEN_RETRY},
    {OHOS::bluetooth::PARAM_NEW_MESSAGE, OHOS::bluetooth::TAG_LEN_NEW_MESSAGE},
    {OHOS::bluetooth::PARAM_NOTIFICATION_STATUS, OHOS::bluetooth::TAG_LEN_NOTIFICATION_STATUS},
    {OHOS::bluetooth::PARAM_MAS_INSTANCEID, OHOS::bluetooth::TAG_LEN_MAS_INSTANCEID},
    {OHOS::bluetooth::PARAM_PARAMETER_MASK, OHOS::bluetooth::TAG_LEN_PARAMETER_MASK},
    {OHOS::bluetooth::PARAM_FOLDER_LISTING_SIZE, OHOS::bluetooth::TAG_LEN_FOLDER_LISTING_SIZE},
    {OHOS::bluetooth::PARAM_LISTING_SIZE, OHOS::bluetooth::TAG_LEN_LISTING_SIZE},
    {OHOS::bluetooth::PARAM_SUBJECT_LENGTH, OHOS::bluetooth::TAG_LEN_SUBJECT_LENGTH},
    {OHOS::bluetooth::PARAM_CHARSET, OHOS::bluetooth::TAG_LEN_CHARSET},
    {OHOS::bluetooth::PARAM_FRACTION_REQUEST, OHOS::bluetooth::TAG_LEN_FRACTION_REQUEST},
    {OHOS::bluetooth::PARAM_FRACTION_DELIVER, OHOS::bluetooth::TAG_LEN_FRACTION_DELIVER},
    {OHOS::bluetooth::PARAM_STATUS_INDICATOR, OHOS::bluetooth::TAG_LEN_STATUS_INDICATOR},
    {OHOS::bluetooth::PARAM_STATUS_VALUE, OHOS::bluetooth::TAG_LEN_STATUS_VALUE},
    {OHOS::bluetooth::PARAM_MSE_TIME, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_DATABASE_IDENTIFIER, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_CONVERSATION_LISTING_VERSION_COUNTER,
        OHOS::bluetooth::TAG_LEN_CONVERSATION_LISTING_VERSION_COUNTER},
    {OHOS::bluetooth::PARAM_PRESENCE_AVAILABILITY, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_PRESENCE_TEXT, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_LAST_ACTIVITY, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_FILTER_LAST_ACTIVITY_BEGIN, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_FILTER_LAST_ACTIVITY_END, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_CHAT_STATE, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_CONVERSATION_ID, OHOS::bluetooth::TAG_LEN_CONVERSATION_ID},
    {OHOS::bluetooth::PARAM_FOLDER_VERSION_COUNTER, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_FILTER_MESSAGE_HANDLE, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_HANDLE},
    {OHOS::bluetooth::PARAM_NOTIFICATION_FILTER_MASK, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_CONV_PARAMETER_MASK, OHOS::bluetooth::TAG_LEN_FILTER_MESSAGE_TYPE},
    {OHOS::bluetooth::PARAM_CONVO_LISTING_SIZE, OHOS::bluetooth::TAG_LEN_CONVO_LISTING_SIZE},
};

namespace OHOS {
    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::shared_ptr<bluetooth::MapMseAppParams> mapMseAppParams = std::make_shared<bluetooth::MapMseAppParams>();
        std::vector<uint8_t> appParams;
        for (const auto &[key, value] : appParamsFuncs) {
            appParams.push_back(key);
            appParams.push_back(value);
            for (size_t i = 0; i < value; i++) {
                appParams.push_back(provider.ConsumeIntegral<int>());
            }
        }
        mapMseAppParams->ParseParams(appParams);
        int64_t highData = provider.ConsumeIntegral<int64_t>();
        int64_t lowData = provider.ConsumeIntegral<int64_t>();
        mapMseAppParams->SetConversationListingVersionCounter(highData, lowData);
        mapMseAppParams->SetDatabaseIdentifier(highData, lowData);
        int32_t masInsanceId = provider.ConsumeIntegral<int32_t>();
        mapMseAppParams->SetMasInsanceId(masInsanceId);
        std::vector<uint8_t> output;
        int outputSize = provider.ConsumeIntegralInRange<int>(1, 40);
        for (int i = 0; i < outputSize; i++) {
            output.push_back(provider.ConsumeIntegral<uint8_t>());
        }
        mapMseAppParams->GetEncodeData(output);
        HILOGI("OnRemoteRequest success");
        return true;
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
    return 0;
}