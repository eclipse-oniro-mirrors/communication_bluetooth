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

#include "bluetooth_device_battery_manager_fuzzer.h"
#include "permission_manager.h"
#include "bluetooth_device_battery_manager.h"
#include "bt_battery_info.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "remote_device_properties.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {
    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        BluetoothDeviceBatteryManager* batteryManager = BluetoothDeviceBatteryManager::GetInstance();
        std::string addr = "11:22:33:44:55:66";
        FuzzedDataProvider provider(data, size);
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(addr));
        uint8_t obtainType = provider.ConsumeIntegralInRange<uint8_t>(1, 3);
        std::string commandValue = "7,1,100,2,100,3,1,4,100,5,1,6,100,7,0";
        batteryManager->ProcessHwBatteryInfo(addr, commandValue, obtainType);
        std::string atCommand = "+VENDORBATTERY=6,2,100,3,1,4,100,5,1,6,100,7,0";
        batteryManager->ProcessHwBatteryCommond(addr, atCommand, obtainType);
        batteryManager->ProcessBatteryCommond(addr, atCommand);
        batteryManager->ProcessHfpConnected(addr);
        std::string profileName = "HfpAgService";
        batteryManager->ProcessActiveDeviceChanged(profileName, addr);
        batteryManager->ProcessHfpDisconnected(addr);
        int32_t connectState = provider.ConsumeIntegralInRange<int32_t>(0, 5);
        batteryManager->ProcessAclStateChanged(addr, connectState);
        batteryManager->ProcessIphoneAccevBatteryEvent(addr, commandValue);
        std::shared_ptr<BatteryInfo> info = std::make_shared<BatteryInfo>();
        batteryManager->ParseBatteryInfo(commandValue, *info);
        uint8_t key = provider.ConsumeIntegralInRange<uint8_t>(1, 8);
        int32_t value = provider.ConsumeIntegral<int32_t>();
        batteryManager->SetBatteryInfo(key, value, *info);
        batteryManager->CheckBatteryInfoValid(key, value);
        batteryManager->LogBatteryInfo(*info);
        batteryManager->GetCurrentBatteryDevice(true);
        batteryManager->GetCurrentBatteryDevice(false);
        batteryManager->ProcessAclConnected(addr);
        batteryManager->ProcessAclDisconnected(addr);
        batteryManager->GetDeviceReportBatteryLevel(addr);
        std::shared_ptr<BatteryDetailInfo> batteryInfo = std::make_shared<BatteryDetailInfo>();
        batteryManager->CalcuBatteryLevel(*batteryInfo);
        batteryManager->UpdateBatteryInfoAndReport(addr, obtainType, *info);
        batteryManager->UpdateDeviceBatteryInfo(*info, obtainType, *batteryInfo);
        std::string newAddress = "00:11:22:33:44:55";
        batteryManager->NotifyForDeviceChanged(newAddress);
        batteryManager->ReportOriginalEchoInfo(newAddress, commandValue);
        uint8_t earphonePropVal = provider.ConsumeIntegral<uint8_t>();
        uint8_t connStateVal = provider.ConsumeIntegral<uint8_t>();
        batteryManager->DeviceConnUpdate(newAddress, earphonePropVal, connStateVal);
        uint8_t leftVal = provider.ConsumeIntegral<uint8_t>();
        uint8_t rightVal = provider.ConsumeIntegral<uint8_t>();
        batteryManager->DeviceControlStatus(newAddress, leftVal, rightVal);
        batteryManager->SetDeviceSupportBattery(newAddress);
        batteryManager->GetLastConnectedDevice(true);
        RemoteDeviceProperties::GetInstance()->FreeMemory();
        HILOGI("OnRemoteRequest success");
        return true;
    }
}  // namespace bluetooth
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        HILOGI("invalid data");
        return 0;
    }
    /* Run your code on data */
    PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}