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

#ifndef TAIHE_BLUETOOTH_BAS_OBSERVER_H_
#define TAIHE_BLUETOOTH_BAS_OBSERVER_H_

#include <memory>

#include "bluetooth_bas_host.h"
#include "ohos.bluetooth.bas.BatteryInfo.ani.1.hpp"
#include "ohos.bluetooth.bas.BatteryInfo.proj.1.hpp"
#include "ohos.bluetooth.common.BluetoothRawAddressType.proj.1.hpp"
#include "taihe_async_work.h"
#include "taihe_event_module.h"

namespace OHOS {
namespace Bluetooth {

class TaiheNativeBasBatteryInfo : public TaiheNativeObject {
public:
    explicit TaiheNativeBasBatteryInfo(const ::ohos::bluetooth::bas::BatteryInfo &batteryInfo)
        : batteryInfo_(batteryInfo) {}
    ~TaiheNativeBasBatteryInfo() override = default;

    ani_ref ToTaiheValue(ani_env *env) const override;

private:
    ::ohos::bluetooth::bas::BatteryInfo batteryInfo_;
};

class TaiheBasCallback : public BluetoothRemoteDeviceBatteryObserver {
public:
    TaiheBasCallback();
    ~TaiheBasCallback() override = default;

    static std::shared_ptr<TaiheBasCallback> GetInstance(void);

    void OnGetBatteryLevelEvent(const BluetoothRemoteDevice &device, int32_t batteryLevel) override;
    void OnBatteryLevelChanged(const BluetoothRemoteDevice &device, int32_t batteryLevel) override;

    // Equivalent to NAPI's eventSubscribe_ for onBatteryChange/offBatteryChange
    EventModule<void(::ohos::bluetooth::bas::BatteryInfo const&)> batteryChangeEventModule_;

    // Equivalent to NAPI's asyncWorkMap_ for getRemoteDeviceBatteryInfo
    // OnGetBatteryLevelEvent settles the async work via AsyncWorkCallFunction
    TaiheAsyncWorkMap asyncWorkMap_ {};
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_BLUETOOTH_BAS_OBSERVER_H_
