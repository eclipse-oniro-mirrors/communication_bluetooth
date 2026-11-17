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

#include "avrcp_ct_cover_art_manager_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_cover_art_manager.h"
#include "avrcp_ct_bip_image_properties.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int BIP_PSM_MAX = 0xFFFF;

static void TestCoverArtManagerLifecycle(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    uint16_t psm = provider.ConsumeIntegralInRange<int>(0, BIP_PSM_MAX);

    // Connect uses shared_from_this, must create via make_shared
    auto manager = std::make_shared<AvrcpCtCoverArtManager>(
        [](const std::string &deviceAddr, const std::string &imageUuid, const std::vector<uint8_t> &imageData) {
            HILOGI("CoverArtCallback device=%{public}s uuid=%{public}s size=%{public}zu",
                deviceAddr.c_str(), imageUuid.c_str(), imageData.size());
        });

    manager->IsConnected(addr);
    manager->Connect(addr, psm);
    manager->IsConnected(addr);
    // repeat Connect on same address
    manager->Connect(addr, psm);
    manager->Disconnect(addr);
    manager->IsConnected(addr);
    manager->Cleanup();
}

static void TestCoverArtManagerDownload(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    uint16_t psm = provider.ConsumeIntegralInRange<int>(0, BIP_PSM_MAX);
    std::string imageUuid = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));

    auto manager = std::make_shared<AvrcpCtCoverArtManager>(
        [](const std::string &deviceAddr, const std::string &imageUuid, const std::vector<uint8_t> &imageData) {
            HILOGI("CoverArtCallback device=%{public}s uuid=%{public}s size=%{public}zu",
                deviceAddr.c_str(), imageUuid.c_str(), imageData.size());
        });

    // download before connect: no client, safe return
    manager->DownloadCoverArt(addr, imageUuid);
    manager->IsCoverArtCached(addr, imageUuid);
    std::vector<uint8_t> imageData;
    manager->GetCoverArt(addr, imageUuid, imageData);
    manager->GetUuidForHandle(addr, imageUuid);

    manager->Connect(addr, psm);
    // connected but uuid has no registered handle, safe return
    manager->DownloadCoverArt(addr, imageUuid);
    manager->RemoveImage(addr, imageUuid);
    manager->ClearDeviceCache(addr);
    manager->DisconnectAll();
}

static void TestCoverArtManagerHandleValidation(FuzzedDataProvider &provider)
{
    std::string handle = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));

    AvrcpCtCoverArtManager::IsValidImageHandle(handle);

    // boundary length handles
    AvrcpCtCoverArtManager::IsValidImageHandle("");
    AvrcpCtCoverArtManager::IsValidImageHandle("12345678");
    AvrcpCtCoverArtManager::IsValidImageHandle("1234567");
}

static void TestCoverArtManagerDescriptor(FuzzedDataProvider &provider)
{
    auto manager = std::make_shared<AvrcpCtCoverArtManager>(
        [](const std::string &deviceAddr, const std::string &imageUuid, const std::vector<uint8_t> &imageData) {
            HILOGI("CoverArtCallback device=%{public}s uuid=%{public}s size=%{public}zu",
                deviceAddr.c_str(), imageUuid.c_str(), imageData.size());
        });

    BipImageProperties properties;
    properties.SetImageHandle(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 7)));
    properties.SetHasThumbnailFormat(provider.ConsumeBool());

    BipImageDescriptor descriptor = manager->DetermineImageDescriptor(properties);
    HILOGI("Descriptor format=%{public}d width=%{public}d height=%{public}d",
        descriptor.format, descriptor.width, descriptor.height);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtCoverArtManagerFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestCoverArtManagerLifecycle(provider);
    TestCoverArtManagerDownload(provider);
    TestCoverArtManagerHandleValidation(provider);
    TestCoverArtManagerDescriptor(provider);

    HILOGI("AvrcpCtCoverArtManagerFuzzTest success");
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
