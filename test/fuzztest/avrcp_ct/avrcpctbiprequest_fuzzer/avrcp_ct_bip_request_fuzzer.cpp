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

#include "avrcp_ct_bip_request_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_bip_request.h"
#include "avrcp_ct_bip_image_properties.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static void TestRequestGetImageProperties(FuzzedDataProvider &provider)
{
    std::string imageHandle = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    int32_t connectionId = provider.ConsumeIntegral<int32_t>();

    RequestGetImageProperties request(imageHandle);
    request.GetType();
    request.ToString();
    request.GetImageHandle();
    request.SetConnectionId(connectionId);
    request.GetConnectionId();
    request.GetResponseCode();

    // Execute is null-safe for empty session
    request.Execute(nullptr);
    request.GetResponseCode();
    request.GetProperties();
}

static void TestRequestGetImage(FuzzedDataProvider &provider)
{
    std::string imageHandle = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    int32_t connectionId = provider.ConsumeIntegral<int32_t>();
    BipImageDescriptor descriptor;
    descriptor.format = provider.ConsumeIntegral<uint8_t>();
    descriptor.width = provider.ConsumeIntegral<uint16_t>();
    descriptor.height = provider.ConsumeIntegral<uint16_t>();

    RequestGetImage request(imageHandle, descriptor);
    request.GetType();
    request.ToString();
    request.GetImageHandle();
    request.SetConnectionId(connectionId);
    request.GetConnectionId();
    request.GetResponseCode();

    request.Execute(nullptr);
    request.GetResponseCode();
    request.GetImage();
    request.GetDescriptor();
}

static void TestRequestWithEmptyHandle(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    RequestGetImageProperties propertiesRequest("");
    propertiesRequest.GetType();
    propertiesRequest.ToString();
    propertiesRequest.Execute(nullptr);

    RequestGetImage imageRequest("", BipImageDescriptor{});
    imageRequest.GetType();
    imageRequest.ToString();
    imageRequest.Execute(nullptr);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtBipRequestFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestRequestGetImageProperties(provider);
    TestRequestGetImage(provider);
    TestRequestWithEmptyHandle(provider);

    HILOGI("AvrcpCtBipRequestFuzzTest success");
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
