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

#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_bas_observer"
#endif

#include "taihe_bluetooth_bas_observer.h"

#include <string>

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_utils.h"
#include "taihe/string.hpp"

namespace OHOS {
namespace Bluetooth {

TaiheBasCallback::TaiheBasCallback()
{}

std::shared_ptr<TaiheBasCallback> TaiheBasCallback::GetInstance(void)
{
    static std::shared_ptr<TaiheBasCallback> instance = std::make_shared<TaiheBasCallback>();
    return instance;
}

ani_ref TaiheNativeBasBatteryInfo::ToTaiheValue(ani_env *env) const
{
    ani_object object = ::taihe::into_ani<::ohos::bluetooth::bas::BatteryInfo>(env, batteryInfo_);
    return reinterpret_cast<ani_ref>(object);
}

// Equivalent to NAPI NapiBasCallback::OnGetBatteryLevelEvent
// Settles the async work from getRemoteDeviceBatteryInfo with the battery info
void TaiheBasCallback::OnGetBatteryLevelEvent(const BluetoothRemoteDevice &device, int32_t batteryLevel)
{
    HILOGI("enter, device: %{public}s, batteryLevel: %{public}d", GET_ENCRYPT_ADDR(device), batteryLevel);
    std::string addrStr = device.GetDeviceAddr();
    ::ohos::bluetooth::common::BluetoothAddress address{
        ::taihe::string(addrStr),
        ::ohos::bluetooth::common::BluetoothAddressType(
            ::ohos::bluetooth::common::BluetoothAddressType::key_t::REAL),
        ::taihe::optional<::ohos::bluetooth::common::BluetoothRawAddressType>{}
    };
    ::ohos::bluetooth::bas::BatteryInfo info{address, batteryLevel};
    auto nativeObject = std::make_shared<TaiheNativeBasBatteryInfo>(info);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::BAS_GET_BATTERY_LEVEL, nativeObject,
        static_cast<int>(BT_NO_ERROR));
}

// Equivalent to NAPI NapiBasCallback::OnBatteryLevelChanged
// Publishes event to registered onBatteryChange callbacks
void TaiheBasCallback::OnBatteryLevelChanged(const BluetoothRemoteDevice &device, int32_t batteryLevel)
{
    HILOGI("enter, device: %{public}s, batteryLevel: %{public}d", GET_ENCRYPT_ADDR(device), batteryLevel);
    std::string addrStr = device.GetDeviceAddr();
    ::ohos::bluetooth::common::BluetoothAddress address{
        ::taihe::string(addrStr),
        ::ohos::bluetooth::common::BluetoothAddressType(
            ::ohos::bluetooth::common::BluetoothAddressType::key_t::REAL),
        ::taihe::optional<::ohos::bluetooth::common::BluetoothRawAddressType>{}
    };
    ::ohos::bluetooth::bas::BatteryInfo info{address, batteryLevel};
    batteryChangeEventModule_.PublishEvent(info);
}

}  // namespace Bluetooth
}  // namespace OHOS
