/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <singleton.h>

#include "bas_server_fuzzer.h"
#include "permission_manager.h"
#include "bluetooth_bas_server.h"
#include "bluetooth_errorcode.h"
#include "bt_def.h"
#include "bluetooth_log.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "securec.h"
#include "bluetooth_device_battery_observer_stub.h"

using namespace std;
using namespace OHOS::Bluetooth;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {

class MockBluetoothDeviceBatteryObserver : public BluetoothDeviceBatteryObserverStub {
public:
    MockBluetoothDeviceBatteryObserver() = default;
    ~MockBluetoothDeviceBatteryObserver() override = default;

    void OnGetBatteryLevelEvent(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        (void)device;
        (void)batteryLevel;
    }

    void OnBatteryLevelChanged(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        (void)device;
        (void)batteryLevel;
    }
};

void DoSomethingInterestingWithBluetoothBasServerAPI(FuzzedDataProvider &provider)
{
    std::shared_ptr<BluetoothBasServer> basServer = std::make_shared<BluetoothBasServer>();
    sptr<MockBluetoothDeviceBatteryObserver> observer = nullptr;
    basServer->RegisterDeviceBatteryObserver(observer);
    std::string address = BuildAddressString(provider);
    uint64_t fullTokenId = provider.ConsumeIntegral<uint64_t>();
    int32_t callingUid = provider.ConsumeIntegral<int32_t>();
    basServer->FindCachedObserver(fullTokenId, callingUid);
    std::vector<sptr<IBluetoothDeviceBatteryObserver>> observerList;
    basServer->CheckIfBasRequestExist(observerList, fullTokenId, callingUid);
    basServer->GetBatteryLevel(address);
    std::map<std::string, int32_t> batteryInfos;
    basServer->GetConnectedDeviceBatteryInfos(batteryInfos);
    basServer->DeregisterDeviceBatteryObserver(observer);
}

}  // namespace Bluetooth
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        HILOGI("invalid data");
        return 0;
    }
    PermissionManager::SetAllGranted();
    FuzzedDataProvider provider(data, size);
    OHOS::Bluetooth::DoSomethingInterestingWithBluetoothBasServerAPI(provider);
    return 0;
}
