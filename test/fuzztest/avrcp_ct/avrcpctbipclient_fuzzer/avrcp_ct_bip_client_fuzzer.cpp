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

#include "avrcp_ct_bip_client_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_bip_client.h"
#include "avrcp_ct_bip_request.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int BIP_PSM_MAX = 0xFFFF;

class TestBipClientCallback : public AvrcpCtBipClient::Callback {
public:
    void OnConnectionStateChanged(int oldState, int newState) override
    {
        HILOGI("OnConnectionStateChanged oldState=%{public}d newState=%{public}d", oldState, newState);
    }

    void OnGetImagePropertiesComplete(int status, const std::string &imageHandle,
        const BipImageProperties &properties) override
    {
        HILOGI("OnGetImagePropertiesComplete status=%{public}d handle=%{public}s", status, imageHandle.c_str());
    }

    void OnGetImageComplete(int status, const std::string &imageHandle, const BipImage &image) override
    {
        HILOGI("OnGetImageComplete status=%{public}d handle=%{public}s", status, imageHandle.c_str());
    }
};

static void TestBipClientConnect(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    uint16_t psm = provider.ConsumeIntegralInRange<int>(0, BIP_PSM_MAX);
    auto callback = std::make_shared<TestBipClientCallback>();
    auto client = std::make_shared<AvrcpCtBipClient>(addr, psm, callback);

    client->GetState();
    client->IsConnected();
    client->GetPsm();
    client->Connect();
    client->Disconnect();
    client->GetState();
    client->IsConnected();
}

static void TestBipClientRequest(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    uint16_t psm = provider.ConsumeIntegralInRange<int>(0, BIP_PSM_MAX);
    std::string imageHandle = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    auto callback = std::make_shared<TestBipClientCallback>();
    auto client = std::make_shared<AvrcpCtBipClient>(addr, psm, callback);

    BipImageDescriptor descriptor;
    descriptor.format = provider.ConsumeIntegral<uint8_t>();
    descriptor.width = provider.ConsumeIntegral<uint16_t>();
    descriptor.height = provider.ConsumeIntegral<uint16_t>();

    client->Connect();
    client->GetImageProperties(imageHandle);
    client->GetImage(imageHandle, descriptor);
    client->Disconnect();
}

static void TestBipClientShutdown(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    uint16_t psm = provider.ConsumeIntegralInRange<int>(0, BIP_PSM_MAX);
    auto callback = std::make_shared<TestBipClientCallback>();
    auto client = std::make_shared<AvrcpCtBipClient>(addr, psm, callback);

    client->Connect();
    client->Shutdown();
    client->GetState();
    client->IsConnected();
    client->GetPsm();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtBipClientFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestBipClientConnect(provider);
    TestBipClientRequest(provider);
    TestBipClientShutdown(provider);

    HILOGI("AvrcpCtBipClientFuzzTest success");
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
