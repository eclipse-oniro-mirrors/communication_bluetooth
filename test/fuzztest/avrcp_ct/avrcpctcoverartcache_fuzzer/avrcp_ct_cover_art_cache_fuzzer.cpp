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

#include "avrcp_ct_cover_art_cache_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_cover_art_cache.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static void TestCoverArtCacheOperations(FuzzedDataProvider &provider)
{
    AvrcpCtCoverArtCache cache;

    std::string addr = Bluetooth::BuildAddressString(provider);
    std::string imageUuid = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));

    // query/remove before add
    cache.HasImage(addr, imageUuid);
    std::vector<uint8_t> outData;
    cache.GetImage(addr, imageUuid, outData);
    cache.RemoveImage(addr, imageUuid);
    cache.GetDeviceCacheSize(addr);
    cache.GetTotalCacheSize();

    // add then query/remove again
    size_t dataSize = provider.ConsumeIntegralInRange<size_t>(0, 64);
    std::vector<uint8_t> imageData = provider.ConsumeBytes<uint8_t>(dataSize);
    cache.AddImage(addr, imageUuid, imageData);
    cache.HasImage(addr, imageUuid);
    std::vector<uint8_t> gotData;
    cache.GetImage(addr, imageUuid, gotData);
    cache.GetDeviceCacheSize(addr);
    cache.GetTotalCacheSize();
    cache.RemoveImage(addr, imageUuid);
    cache.HasImage(addr, imageUuid);
}

static void TestCoverArtCacheMultiDevice(FuzzedDataProvider &provider)
{
    AvrcpCtCoverArtCache cache;

    std::string addr1 = Bluetooth::BuildAddressString(provider);
    std::string addr2 = Bluetooth::BuildAddressString(provider);
    std::string uuid1 = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    std::string uuid2 = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));

    std::vector<uint8_t> imageData1(provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange<size_t>(0, 32)));
    std::vector<uint8_t> imageData2(provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange<size_t>(0, 32)));

    cache.AddImage(addr1, uuid1, imageData1);
    cache.AddImage(addr2, uuid2, imageData2);
    cache.GetTotalCacheSize();

    // overwrite same uuid
    cache.AddImage(addr1, uuid1, imageData2);
    cache.GetDeviceCacheSize(addr1);

    // clear by device
    cache.ClearDevice(addr1);
    cache.HasImage(addr1, uuid1);
    cache.HasImage(addr2, uuid2);
    cache.GetTotalCacheSize();

    // clear all
    cache.ClearAll();
    cache.HasImage(addr2, uuid2);
    cache.GetTotalCacheSize();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtCoverArtCacheFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestCoverArtCacheOperations(provider);
    TestCoverArtCacheMultiDevice(provider);

    HILOGI("AvrcpCtCoverArtCacheFuzzTest success");
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }

    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
