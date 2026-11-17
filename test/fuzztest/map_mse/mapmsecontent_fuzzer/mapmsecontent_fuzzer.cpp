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
#include "mapmsecontent_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "bluetooth_log.h"
#include "map_mse_content.h"
#include "bluetooth_map_mse_stub.h"
#include "bluetooth_map_mse_server.h"
#include "securec.h"
#include "raw_address.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    bluetooth::MapMseContent content;
    std::shared_ptr<bluetooth::MapMseAppParams> appParams = std::make_shared<bluetooth::MapMseAppParams>();
    int64_t highData = provider.ConsumeIntegral<int64_t>();
    int64_t lowData = provider.ConsumeIntegral<int64_t>();
    appParams->SetConversationListingVersionCounter(highData, lowData);
    appParams->SetDatabaseIdentifier(highData, lowData);
    int32_t masInsanceId = provider.ConsumeIntegral<int32_t>();
    appParams->SetMasInsanceId(masInsanceId);

    int listSize = provider.ConsumeIntegral<int>();
    std::string folderName = "testFolderName";
    bool result = provider.ConsumeBool();
    content.GetMessageListing(folderName, appParams, folderName, listSize, result);
    content.GetMessageListingSize(folderName, appParams, result);
    content.GetMessage(folderName, appParams, folderName);
    content.GetConversationListing();
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