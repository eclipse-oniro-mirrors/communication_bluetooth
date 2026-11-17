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

#undef private
#define private public

#include "obex_array_body_object_fuzzer.h"
#include "permission_manager.h"
#include "pbap_pce_obex_client.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

#include <memory>
#include <vector>

using namespace std;
namespace OHOS {

/*
 * Note: obex_array_body_object.h / ObexArrayBodyObject do not exist in this
 * codebase. The real in-memory ("array") OBEX body object is:
 *   - bluetooth::MemoryBodyObject (src/pbap_pce/pbap_pce_obex_client.h)
 * which accumulates downloaded PBAP data into a std::vector<uint8_t>.
 * This fuzzer targets that real class.
 */

// Write fuzzed chunks, read back, inspect the accumulated array.
void TestWriteAndRead(FuzzedDataProvider &provider, bluetooth::MemoryBodyObject &bodyObj)
{
    size_t writeLen = provider.ConsumeIntegralInRange<size_t>(0, 1024);
    std::vector<uint8_t> writeData = provider.ConsumeBytes<uint8_t>(writeLen);
    if (!writeData.empty()) {
        bodyObj.Write(writeData.data(), writeData.size());
    }

    size_t bufLen = provider.ConsumeIntegralInRange<size_t>(1, 1024);
    std::vector<uint8_t> readBuf(bufLen, 0);
    bodyObj.Read(readBuf.data(), readBuf.size());
    bodyObj.IsReadFinished();
    const std::vector<uint8_t> &data = bodyObj.GetData();
    (void)data;

    // write again after reading back, mimicking chunked vCard downloads
    std::vector<uint8_t> moreData = provider.ConsumeBytes<uint8_t>(
        provider.ConsumeIntegralInRange<size_t>(0, 256));
    if (!moreData.empty()) {
        bodyObj.Write(moreData.data(), moreData.size());
    }
    bodyObj.IsReadFinished();
    (void)bodyObj.GetData();
}

// Edge cases: null buffers, zero length, Close/OpenFile no-ops of the memory body.
void TestEdgeCases(FuzzedDataProvider &provider, bluetooth::MemoryBodyObject &bodyObj)
{
    size_t bufLen = provider.ConsumeIntegralInRange<size_t>(1, 256);
    bodyObj.Read(nullptr, bufLen);
    bodyObj.Write(nullptr, bufLen);
    bodyObj.Write(nullptr, 0);
    bodyObj.IsReadFinished();
    bodyObj.Close();
    bodyObj.OpenFile(provider.ConsumeIntegral<int>());
    (void)bodyObj.GetData();
}

// Real usage pattern: MemoryBodyObject is passed around as an ObexBodyObject writer.
void TestSharedUsage(FuzzedDataProvider &provider)
{
    std::shared_ptr<bluetooth::ObexBodyObject> writer = std::make_shared<bluetooth::MemoryBodyObject>();
    size_t writeLen = provider.ConsumeIntegralInRange<size_t>(0, 512);
    std::vector<uint8_t> writeData = provider.ConsumeBytes<uint8_t>(writeLen);
    if (!writeData.empty()) {
        writer->Write(writeData.data(), writeData.size());
    }
    writer->IsReadFinished();
    writer->Close();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    auto bodyObj = std::make_shared<bluetooth::MemoryBodyObject>();
    TestWriteAndRead(provider, *bodyObj);
    TestEdgeCases(provider, *bodyObj);
    TestSharedUsage(provider);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
