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
#include "parselatencyconfig_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "av_sync_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_hw_interface.h"
#include "cJSON.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
cJSON* CreateLatencyJson(const uint8_t* data, size_t size)
{
    int32_t latencyMs = GetU32Data(data);
    std::string deviceName = "testName";
    std::string modelId = "1234";
    std::string newModelId = "5678";

    cJSON* baseListJson = cJSON_CreateArray();
    cJSON* deviceNameJson = cJSON_CreateObject();
    cJSON_AddStringToObject(deviceNameJson, "device_name", deviceName.c_str());
    cJSON_AddNumberToObject(deviceNameJson, "latency_ms", latencyMs);
    cJSON_AddItemToArray(baseListJson, deviceNameJson);

    cJSON* modelIdJson = cJSON_CreateObject();
    cJSON_AddStringToObject(modelIdJson, "model_id", modelId.c_str());
    cJSON_AddNumberToObject(modelIdJson, "latency_ms", latencyMs);
    cJSON_AddItemToArray(baseListJson, modelIdJson);

    cJSON* newModelIdJson = cJSON_CreateObject();
    cJSON_AddStringToObject(newModelIdJson, "new_model_id", newModelId.c_str());
    cJSON_AddNumberToObject(newModelIdJson, "latency_ms", latencyMs);
    cJSON_AddItemToArray(baseListJson, newModelIdJson);

    cJSON* root = cJSON_CreateArray();
    cJSON* baseJson = cJSON_CreateObject();
    cJSON_AddItemToObject(baseJson, "latency_base", baseListJson);
    cJSON_AddItemToArray(root, baseJson);
    return root;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    if ((data == nullptr) || (size < FUZZ_DATA_LEN)) {
        return false;
    }
    FuzzedDataProvider provider(data, size);

    // Test ParseLatencyConfig
    cJSON* jsonVal = CreateLatencyJson(data, size);
    std::shared_ptr<bluetooth::AvSyncManager> avSyncManager = std::make_shared<bluetooth::AvSyncManager>();
    avSyncManager->ParseLatencyConfig(jsonVal);
    cJSON_Delete(jsonVal);

    // Test SaveHiechoAvLatency
    std::string addr = "11:22:33:44:55:66";
    bluetooth::RawAddress rawAddr(addr);
    int32_t avLatency = provider.ConsumeIntegral<int32_t>();
    avSyncManager->SaveHiechoAvLatency(rawAddr, avLatency);

    // Test GetAvSyncLatency
    (void)avSyncManager->GetAvSyncLatency(rawAddr);

    // Test UpdateBtAudioLatency
    avSyncManager->UpdateBtAudioLatency(rawAddr);

    // Test ClearLatencyInfoByAddr
    avSyncManager->ClearLatencyInfoByAddr(rawAddr);

    // Test ClearLatencyInfo
    avSyncManager->ClearLatencyInfo();

    // Test with different addresses
    std::string addr2 = "AA:BB:CC:DD:EE:FF";
    bluetooth::RawAddress rawAddr2(addr2);
    avSyncManager->SaveHiechoAvLatency(rawAddr2, avLatency);
    avSyncManager->GetAvSyncLatency(rawAddr2);
    avSyncManager->UpdateBtAudioLatency(rawAddr2);
    avSyncManager->ClearLatencyInfoByAddr(rawAddr2);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
