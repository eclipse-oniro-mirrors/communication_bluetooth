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

#include "pbap_pse_appparam_fuzzer.h"
#include "permission_manager.h"
#include "pbap_pse_appparam.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::shared_ptr<bluetooth::PbapPseAppParam> pbapPseAppParam = std::make_shared<bluetooth::PbapPseAppParam>();
        std::vector<uint8_t> appParam;
        int appParamSize = provider.ConsumeIntegralInRange<int>(1, 40);
        for (int i = 0; i < appParamSize; i++) {
            appParam.push_back(provider.ConsumeIntegral<uint8_t>());
        }
        int32_t paramSize = appParam.size();
        int pos = 0;
        pbapPseAppParam->ParseApplicationParameter(appParam);
        pbapPseAppParam->ParseSelectorTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseSupportedFeatureTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseOderTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseSeachValueTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseSeachAttributeTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseMaxListCountTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseListStartOffsetTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseFormatTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseVcardSelectorTagId(appParam, paramSize, pos);
        pbapPseAppParam->ParseVcardSelectorOperaterTagId(appParam, paramSize, pos);
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
