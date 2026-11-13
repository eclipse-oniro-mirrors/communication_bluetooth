/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <memory>

#include "hearingaid_fuzzer.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "bluetooth_hearingaid.h"
#include "permission_manager.h"
#include "btcommon/timer_manager.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    BluetoothHearingAid* hearingAid = BluetoothHearingAid::GetProfile();
    std::string addr = "11:22:33:44:55:66";
    std::shared_ptr<HearingAidObserver> observer;
    hearingAid->RegisterObserver(observer);
    bool support = provider.ConsumeBool();
    hearingAid->IsLocalDeviceSupportHearingAid(support);
    hearingAid->Connect(addr);
    int64_t volume = provider.ConsumeIntegralInRange<int64_t>(-128, 0);
    hearingAid->SetVolume(addr, volume);
    int32_t state = provider.ConsumeIntegral<int32_t>();
    hearingAid->GetConnectionState(addr, state);
    int32_t side = provider.ConsumeIntegral<int32_t>();
    int32_t mode = provider.ConsumeIntegral<int32_t>();
    int64_t id = provider.ConsumeIntegral<int32_t>();
    hearingAid->GetHearingAidDeviceInfo(addr, side, mode, id);
    std::vector<std::string> connectedDevices;
    hearingAid->GetConnectedDevices(connectedDevices);
    std::string name = "testName";
    hearingAid->GetActiveDevice(addr, name);
    int32_t strategy = provider.ConsumeIntegralInRange<int32_t>(0, 3);
    hearingAid->SetConnectionStrategy(addr, strategy);
    hearingAid->GetConnectionStrategy(addr, strategy);
    hearingAid->Disconnect(addr);
    hearingAid->DeregisterObserver(observer);
    HILOGI("end success");
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        HILOGI("invalid data");
        return 0;
    }
    /* Run your code on data */
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    utility::CloseAllTimer();
    return 0;
}