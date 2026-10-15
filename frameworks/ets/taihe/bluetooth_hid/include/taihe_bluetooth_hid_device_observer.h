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

#ifndef TAIHE_BLUETOOTH_HID_DEVICE_OBSERVER_H_
#define TAIHE_BLUETOOTH_HID_DEVICE_OBSERVER_H_

#include "bluetooth_hid_device.h"
#include "ohos.bluetooth.hid.proj.hpp"
#include "ohos.bluetooth.hid.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "taihe_event_module.h"

namespace OHOS {
namespace Bluetooth {

class TaiheBluetoothHidDeviceObserver : public HidDeviceObserver {
public:
    void OnAppStatusChanged(int state) override;
    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state) override;
    void OnGetReport(int type, int id, uint16_t bufferSize) override;
    void OnSetProtocol(int protocol) override;
    void OnInterruptDataReceived(int id, const std::vector<uint8_t> &data) override;
    void OnSetReport(int type, int id, const std::vector<uint8_t> &data) override;
    void OnVirtualCableUnplug() override;

    TaiheBluetoothHidDeviceObserver();
    ~TaiheBluetoothHidDeviceObserver() override = default;

    EventModule<void(::ohos::bluetooth::baseProfile::StateChangeParam const&)> connectionStateChangeEventModule_;
    EventModule<void(::ohos::bluetooth::hid::ProtocolData const&)> setProtocolEventModule_;
    EventModule<void(::ohos::bluetooth::hid::InterruptData const&)> interruptDataReceivedEventModule_;
    EventModule<void(::ohos::bluetooth::hid::SetReportData const&)> setReportEventModule_;
    EventModule<void(::ohos::bluetooth::hid::GetReportData const&)> getReportEventModule_;
    EventModule<void(::ohos::bluetooth::hid::UndefinedType const&)> virtualCableUnplugEventModule_;
    EventModule<void(bool)> appStatusChangeEventModule_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* TAIHE_BLUETOOTH_HID_DEVICE_OBSERVER_H_ */
