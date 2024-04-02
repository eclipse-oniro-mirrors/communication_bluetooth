/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_PARCEL_BATTERY_INFO_H
#define BLUETOOTH_PARCEL_BATTERY_INFO_H

#include "../common/bt_battery_info.h"
#include "parcel.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBatteryInfo : public Parcelable, public bluetooth::BatteryInfo {
public:
    BluetoothBatteryInfo() = default;
    explicit BluetoothBatteryInfo(const bluetooth::BatteryInfo& other)
        : bluetooth::BatteryInfo(other) {}
    explicit BluetoothBatteryInfo(const BluetoothBatteryInfo& other)
        : bluetooth::BatteryInfo(other) {}
    BluetoothBatteryInfo& operator = (const BluetoothBatteryInfo &other) = default;

    ~BluetoothBatteryInfo() override = default;

    bool Marshalling(Parcel &parcel) const override;

    static BluetoothBatteryInfo *Unmarshalling(Parcel &parcel);

    bool WriteToParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_BATTERY_INFO_H