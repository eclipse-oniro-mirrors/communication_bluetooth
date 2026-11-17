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

#include "map_mse_bmessage_fuzzer.h"
#include "permission_manager.h"
#include "map_mse_bmessage.h"
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
        std::shared_ptr<bluetooth::MapMseBMessageSms> message = std::make_shared<bluetooth::MapMseBMessageSms>();
        std::vector<uint8_t> rawData;
        int rawDataSize = provider.ConsumeIntegralInRange<int>(1, 40);
        for (int i = 0; i < rawDataSize; i++) {
            rawData.push_back(provider.ConsumeIntegral<uint8_t>());
        }
        uint8_t charset = provider.ConsumeIntegral<uint8_t>();
        message->ParseMessage(rawData, charset);
        std::string line = "testLine";
        std::vector<std::string> src{"test"};
        uint32_t pos = provider.ConsumeIntegral<uint32_t>();
        message->ParseMessageProperty(line);
        message->ParseOriginator(src, pos);
        message->ParseEnvelope(src, pos);
        message->ParseRecipient(src, pos);
        message->ParseBody(src, pos);
        message->ParseMsgContent(src, pos);
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