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

#include "obex_header_fuzzer.h"
#include "permission_manager.h"
#include "obex_header.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;

namespace OHOS {

    void TestForObexHeader(FuzzedDataProvider &provider)
    {
        std::shared_ptr<bluetooth::ObexHeader> obexHeader = std::make_shared<bluetooth::ObexHeader>();
        std::vector<uint8_t> bodyData;
        int bodyDataSize = provider.ConsumeIntegralInRange<int>(1, 40);
        for (int i = 0; i < bodyDataSize; i++) {
            bodyData.push_back(provider.ConsumeIntegral<uint8_t>());
        }
        uint16_t length = provider.ConsumeIntegral<uint16_t>();
        obexHeader->SetBody(bodyData, length);
        obexHeader->SetEndOfBody(bodyData, length);

        uint32_t connectionId = provider.ConsumeIntegral<uint32_t>();
        obexHeader->SetConnectionId(connectionId);
        uint32_t lengthSet = provider.ConsumeIntegral<uint32_t>();
        obexHeader->SetLength(lengthSet);
        uint32_t count = provider.ConsumeIntegral<uint32_t>();
        obexHeader->SetCount(count);
        obexHeader->SetSrm(true);
        uint8_t singleResponseMode = provider.ConsumeIntegral<uint8_t>();
        uint16_t bodyLen = provider.ConsumeIntegral<uint16_t>();
        std::string name = "testName";
        obexHeader->SetName(name);
        uint8_t value8 = provider.ConsumeIntegral<uint8_t>();
        obexHeader->ConvertToVector(value8, bodyData);
        uint32_t value32 = provider.ConsumeIntegral<uint32_t>();
        obexHeader->ConvertToVector(value32, bodyData);
        std::string strs = "testStrs";
        obexHeader->ConvertUnicodeToVector(strs, bodyData);

        obexHeader->ConvertToInt32(bodyData);
        obexHeader->ConvertToString(bodyData);

        obexHeader->GetEndOfBody(bodyLen);
        obexHeader->SetSingleResponseMode(singleResponseMode);
        uint32_t lengthTmp = provider.ConsumeIntegral<uint32_t>();
        obexHeader->GetLength(lengthTmp);
        obexHeader->GetName();
        obexHeader->GetType();
        obexHeader->GetBody(bodyLen);
    }

    void TestCopyAndAssignment(FuzzedDataProvider &provider, bluetooth::ObexHeader &header)
    {
        bluetooth::ObexHeader copied(header);
        bluetooth::ObexHeader assigned;
        assigned = header;
        assigned = copied;
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::shared_ptr<bluetooth::ObexHeader> obexHeader = std::make_shared<bluetooth::ObexHeader>();
        std::vector<uint8_t> headerValue;
        int headerValueSize = provider.ConsumeIntegralInRange<int>(1, 40);
        for (int i = 0; i < headerValueSize; i++) {
            headerValue.push_back(provider.ConsumeIntegral<uint8_t>());
        }
        uint8_t headerId = provider.ConsumeIntegral<uint8_t>();
        obexHeader->GetHeader(headerId);
        obexHeader->ParseHeaderValue(headerValue);
        obexHeader->SetHeader(headerId, headerValue);
        uint32_t start = provider.ConsumeIntegral<uint32_t>();
        uint32_t maxSize = provider.ConsumeIntegral<uint32_t>();
        obexHeader->FindHeaderEnd(headerValue, start, maxSize);

        obexHeader->GenerateHeaderValue(headerValue);
        obexHeader->AppendConnectionIdValue(headerValue);
        obexHeader->AppendByteValue(headerId, headerValue);
        obexHeader->AppendByteArrayValue(headerId, true, headerValue);
        TestForObexHeader(provider);
        obexHeader->GetConnectionId();

        TestCopyAndAssignment(provider, *obexHeader);
        obexHeader->Clear();
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

