/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "pbap_pce_appparam_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "pbap_pce_app_params.h"
#include "obex_header.h"
#include "pbap_pce_tlv.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {

static void TestPbapPceAppParamsRoundTrip(FuzzedDataProvider &provider)
{
    PbapPceAppParams appParams;
    appParams.SetOrder(provider.ConsumeIntegral<uint8_t>());
    appParams.SetSearchProperty(provider.ConsumeIntegral<uint8_t>());
    appParams.SetMaxListCount(provider.ConsumeIntegral<uint16_t>());
    appParams.SetListStartOffset(provider.ConsumeIntegral<uint16_t>());
    appParams.SetPropertySelector(provider.ConsumeIntegral<uint64_t>());
    appParams.SetFormat(provider.ConsumeIntegral<uint8_t>());
    appParams.SetPhonebookSize(provider.ConsumeIntegral<uint16_t>());
    appParams.SetNewMissedCalls(provider.ConsumeIntegral<uint8_t>());
    appParams.SetVcardSelector(provider.ConsumeIntegral<uint64_t>());
    appParams.SetVcardSelectorOperator(provider.ConsumeIntegral<uint8_t>());
    appParams.SetResetNewMissedCalls(provider.ConsumeIntegral<uint8_t>());
    appParams.SetPbapSupportedFeatures(provider.ConsumeIntegral<uint32_t>());
    int searchLen = provider.ConsumeIntegralInRange<int>(0, 64);
    std::vector<uint8_t> searchVal(searchLen);
    for (int i = 0; i < searchLen; i++) {
        searchVal[i] = provider.ConsumeIntegral<uint8_t>();
    }
    appParams.SetSearchValueUtf8(searchVal);

    ObexHeader hdr;
    appParams.AddToObexHeader(hdr);

    PbapPceAppParams result = PbapPceAppParams::FromObexHeader(hdr);
    result.GetOrder();
    result.GetSearchProperty();
    result.GetMaxListCount();
    result.GetListStartOffset();
    result.GetPropertySelector();
    result.GetFormat();
    result.GetPhonebookSize();
    result.GetNewMissedCalls();
    result.GetVcardSelector();
    result.GetVcardSelectorOperator();
    result.GetResetNewMissedCalls();
    result.GetPbapSupportedFeatures();
    result.GetSearchValueUtf8();
}

static void TestPbapPceAppParamsRawTlv(FuzzedDataProvider &provider)
{
    // 256-byte backing store so any uint8_t len stays in bounds
    std::vector<uint8_t> buf(256);
    for (size_t i = 0; i < buf.size(); i++) {
        buf[i] = provider.ConsumeIntegral<uint8_t>();
    }

    ObexTlvParameters params;
    for (uint8_t tag = PbapPceAppParams::ORDER; tag <= PbapPceAppParams::PBAP_SUPPORTED_FEATURES; tag++) {
        uint8_t len = provider.ConsumeIntegral<uint8_t>();
        params.AppendTlvtriplet(TlvTriplet(tag, len, buf.data()));
    }

    ObexHeader hdr;
    std::vector<uint8_t> appParamsBytes = params.ToBytes();
    if (!appParamsBytes.empty()) {
        hdr.SetHeader(APPLICATION_PARAMETER, appParamsBytes);
    }

    PbapPceAppParams result = PbapPceAppParams::FromObexHeader(hdr);
    result.GetOrder();
    result.GetSearchValueUtf8();
    result.GetSearchProperty();
    result.GetMaxListCount();
    result.GetListStartOffset();
    result.GetPropertySelector();
    result.GetFormat();
    result.GetPhonebookSize();
    result.GetNewMissedCalls();
    result.GetPrimaryFolderVersion();
    result.GetSecondaryFolderVersion();
    result.GetVcardSelector();
    result.GetDatabaseIdentifier();
    result.GetVcardSelectorOperator();
    result.GetResetNewMissedCalls();
    result.GetPbapSupportedFeatures();
}

static void TestPbapPceAppParamsFromEmptyHeader()
{
    ObexHeader hdr;
    PbapPceAppParams result = PbapPceAppParams::FromObexHeader(hdr);
    result.GetOrder();
    result.GetName();
    result.GetSearchValueUtf8();
    result.GetMaxListCount();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("PbapPceAppparamFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestPbapPceAppParamsRoundTrip(provider);
    TestPbapPceAppParamsRawTlv(provider);
    TestPbapPceAppParamsFromEmptyHeader();

    HILOGI("PbapPceAppparamFuzzTest success");
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }

    PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
