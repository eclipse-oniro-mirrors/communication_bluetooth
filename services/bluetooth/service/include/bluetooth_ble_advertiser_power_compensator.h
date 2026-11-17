/*
 * Copyright (C) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_BLE_ADVERTISER_POWER_COMPENSATOR_H
#define BLUETOOTH_BLE_ADVERTISER_POWER_COMPENSATOR_H

#include "ble_service_data.h"

namespace OHOS {
namespace Bluetooth {

class BleAdvTxPowerCompensator {
public:
    BleAdvTxPowerCompensator() = default;
    ~BleAdvTxPowerCompensator() = default;

    void AdjustBleAdvTxPower(bluetooth::BleAdvertiserSettingsImpl &settings);

private:
    bool IsBleAdvOnDedicatedAnt();
    bool IsBleAdvOnMac1();
    bool IsOOBEState();

    const int8_t minTxPower = -127;
    const int8_t maxTxPower = 2;

    bool oobeChecked_ = false;
    bool isOOBE_ = true;
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_BLE_ADVERTISER_POWER_COMPENSATOR_H
