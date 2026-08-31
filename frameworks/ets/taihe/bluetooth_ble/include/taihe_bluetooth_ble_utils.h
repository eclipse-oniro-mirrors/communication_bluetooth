/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_BLUETOOTH_BLE_UTILS_H_
#define TAIHE_BLUETOOTH_BLE_UTILS_H_

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_server.h"
#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"
#include "taihe_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
constexpr int SDK_VERSION_20 = 20;

enum class ScanReportType {
    ON_FOUND = 1, // the found of advertisement packet
    ON_LOST = 2,  // the lost of advertisement packet
    ON_BATCH = 3  // the batch scan report
};

enum class SensitivityMode {
    SENSITIVITY_MODE_HIGH = 1,  //  high sensitivity mode
    SENSITIVITY_MODE_LOW = 2    //  low sensitivity mode
};

int GetCurrentSdkVersion(void);
int GetSDKAdaptedStatusCode(int status);
TaiheStatus CheckBleScanParams(const ohos::bluetooth::ble::ScanFilterNullValue &filters,
                               taihe::optional_view<ohos::bluetooth::ble::ScanOptions> options,
                               std::vector<BleScanFilter> &outScanfilters,
                               BleScanSettings &outSettings);
ani_object ConvertBLECharacteristicToJS(ani_env *env, GattCharacteristic &characteristic);
ani_object ConvertBLEDescriptorToJS(ani_env *env, GattDescriptor& descriptor);
ani_object ConvertBLEDescriptorVectorToJS(ani_env *env, std::vector<GattDescriptor> &descriptors);
::ohos::bluetooth::ble::GattProperties TaiheConvertGattProperties(int properties);
::ohos::bluetooth::ble::GattPermissions TaiheConvertGattPermissions(int permissions);
::ohos::bluetooth::ble::BLEDescriptor TaiheConvertGattDescriptor(const GattDescriptor &descriptor);
::ohos::bluetooth::ble::GattService TaiheConvertGattService(GattService &service);
bool IsValidPhyValue(int32_t txPhy, int32_t rxPhy, int32_t phyMode);
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_BLE_UTILS_H_