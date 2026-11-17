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

#include "avrcp_ct_bip_image_properties_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_bip_image_properties.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static const std::string VALID_PROPERTIES_XML =
    "<image-properties version=\"1.0\" handle=\"1234567\">"
    "<thumbnail encoding=\"JPEG\" pixel=\"200*200\"/>"
    "</image-properties>";

static void TestImagePropertiesParse(FuzzedDataProvider &provider)
{
    BipImageProperties properties;
    size_t dataSize = provider.remaining_bytes();
    std::vector<uint8_t> data(provider.ConsumeBytes<uint8_t>(dataSize));

    properties.ParseFromData(data);
    properties.IsValid();
    properties.ToString();
}

static void TestImagePropertiesParseWithSeed(FuzzedDataProvider &provider)
{
    BipImageProperties properties;
    std::vector<uint8_t> seedData(VALID_PROPERTIES_XML.begin(), VALID_PROPERTIES_XML.end());
    size_t prefixSize = provider.ConsumeIntegralInRange<size_t>(0, 4);
    std::vector<uint8_t> prefix(provider.ConsumeBytes<uint8_t>(prefixSize));
    std::vector<uint8_t> suffix(provider.ConsumeRemainingBytes<uint8_t>());

    std::vector<uint8_t> data = prefix;
    data.insert(data.end(), seedData.begin(), seedData.end());
    data.insert(data.end(), suffix.begin(), suffix.end());

    properties.ParseFromData(data);
    properties.IsValid();
    properties.ToString();
}

static void TestImagePropertiesAccessors(FuzzedDataProvider &provider)
{
    BipImageProperties properties;
    std::string handle = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    bool hasThumbnail = provider.ConsumeBool();

    properties.SetImageHandle(handle);
    properties.GetImageHandle();
    properties.SetHasThumbnailFormat(hasThumbnail);
    properties.HasThumbnailFormat();
    properties.IsValid();
    properties.ToString();
}

static void TestImagePropertiesEmptyData(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    BipImageProperties properties;
    std::vector<uint8_t> emptyData;

    properties.ParseFromData(emptyData);
    properties.IsValid();
    properties.ToString();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtBipImagePropertiesFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestImagePropertiesParse(provider);
    TestImagePropertiesParseWithSeed(provider);
    TestImagePropertiesAccessors(provider);
    TestImagePropertiesEmptyData(provider);

    HILOGI("AvrcpCtBipImagePropertiesFuzzTest success");
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
