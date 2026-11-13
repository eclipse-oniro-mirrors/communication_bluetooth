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

#include "bluetooth_ble_filter_matcher_fuzzer.h"
#include "permission_manager.h"
#include "bluetooth_ble_filter_matcher.h"
#include "bluetooth_utils_server.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "uuid.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::string str = "testStr";
        int mode = provider.ConsumeIntegral<int>();
        GetScanModeName(mode);
        BluetoothBleScanResult result;
        std::vector<bluetooth::BleScanFilterImpl> bleScanFilters {};
        bluetooth::BleScanFilterImpl filter;
        std::string deviceId = "0123456";
        filter.SetDeviceId(deviceId);
        std::string name = "testName";
        filter.SetName(name);
        std::vector<uint8_t> manufactureData;
        int manufactureDataSize = provider.ConsumeIntegralInRange<int>(1, 40);
        for (int i = 0; i < manufactureDataSize; i++) {
            manufactureData.push_back(provider.ConsumeIntegral<uint8_t>());
        }
        filter.SetManufactureData(manufactureData);
        bluetooth::Uuid uuid;
        filter.SetServiceUuid(uuid);
        bluetooth::BleScanFilterImpl matchedFilter;
        BluetoothBleFilterMatcher::MatchesScanFilters(bleScanFilters, result, matchedFilter);
        BluetoothBleFilterMatcher::MatchesScanFilter(filter, result);
        BluetoothBleFilterMatcher::MatchesAddress(filter, result);
        BluetoothBleFilterMatcher::MatchesName(filter, result);
        BluetoothBleFilterMatcher::MatchesServiceUuids(filter, result);
        BluetoothBleFilterMatcher::MatchesManufacturerDatas(filter, result);
        BluetoothBleFilterMatcher::MatchesServiceDatas(filter, result);
        BluetoothBleFilterMatcher::MatchesUuidWithMask(uuid, uuid, uuid);
        BluetoothBleFilterMatcher::ParseServiceDataUUidToString(uuid, str);

        std::vector<uint8_t> attrs;
        int attrsSize = provider.ConsumeIntegralInRange<int>(1, 40);
        for (int i = 0; i < attrsSize; i++) {
            attrs.push_back(provider.ConsumeIntegral<uint8_t>());
        }
        BluetoothBleFilterMatcher::MatchesData(attrs, str, attrs);

        uint16_t eventType = provider.ConsumeIntegral<uint16_t>();
        BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, provider.ConsumeBool());
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