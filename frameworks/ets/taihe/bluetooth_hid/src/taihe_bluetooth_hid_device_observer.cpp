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
#define LOG_TAG "bt_taihe_hid_device_observer"
#endif

#include "ohos.bluetooth.hid.proj.hpp"
#include "ohos.bluetooth.hid.impl.hpp"
#include "bluetooth_utils.h"
#include "bluetooth_log.h"
#include "taihe_bluetooth_hid_device_observer.h"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "taihe_event_module.h"

namespace OHOS {
namespace Bluetooth {

TaiheBluetoothHidDeviceObserver::TaiheBluetoothHidDeviceObserver()
{}

void TaiheBluetoothHidDeviceObserver::OnAppStatusChanged(int state)
{
    HILOGI("state: %{public}d", state);
    appStatusChangeEventModule_.PublishEvent(state != 0);
    if (state == HID_DEVICE_UNREGISTERED) {
        appStatusChangeEventModule_.DeregisterEvent(std::nullopt);
    }
}

void TaiheBluetoothHidDeviceObserver::OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state)
{
    HILOGD("enter, remote device address: %{public}s, state: %{public}d",
        GET_ENCRYPT_ADDR(device), state);
    ::taihe::string deviceId_(device.GetDeviceAddr());
    int32_t profileConnectionState = TaiheUtils::GetProfileConnectionState(state);
    auto state_ = ::ohos::bluetooth::constant::ProfileConnectionState::from_value(profileConnectionState);
    auto taiheResult = ::ohos::bluetooth::baseProfile::StateChangeParam {
        .deviceId = deviceId_,
        .state = state_,
        .cause = ::ohos::bluetooth::baseProfile::DisconnectCause::from_value(0),
    };
    connectionStateChangeEventModule_.PublishEvent(taiheResult);
}

void TaiheBluetoothHidDeviceObserver::OnGetReport(int type, int id, uint16_t bufferSize)
{
    HILOGD("type: %{public}d, id: %{public}d, bufferSize: %{public}d", type, id, bufferSize);
    auto taiheResult = ::ohos::bluetooth::hid::GetReportData {
        .type = ::ohos::bluetooth::hid::ReportType::from_value(type),
        .id = id,
        .bufferSize = static_cast<int32_t>(bufferSize),
    };
    getReportEventModule_.PublishEvent(taiheResult);
}

void TaiheBluetoothHidDeviceObserver::OnSetProtocol(int protocol)
{
    HILOGD("protocol: %{public}d", protocol);
    auto taiheResult = ::ohos::bluetooth::hid::ProtocolData {
        .protocol = ::ohos::bluetooth::hid::ProtocolType::from_value(protocol),
    };
    setProtocolEventModule_.PublishEvent(taiheResult);
}

void TaiheBluetoothHidDeviceObserver::OnInterruptDataReceived(int id, const std::vector<uint8_t> &data)
{
    HILOGD("id: %{public}d, data size: %{public}zu", id, data.size());
    auto taiheResult = ::ohos::bluetooth::hid::InterruptData {
        .id = id,
        .data = data.empty() ? taihe::array<uint8_t>{}
            : taihe::array<uint8_t>(taihe::copy_data_t{}, data.data(), data.size()),
    };
    interruptDataReceivedEventModule_.PublishEvent(taiheResult);
}

void TaiheBluetoothHidDeviceObserver::OnSetReport(int type, int id, const std::vector<uint8_t> &data)
{
    HILOGD("type: %{public}d, id: %{public}d, data size: %{public}zu", type, id, data.size());
    auto taiheResult = ::ohos::bluetooth::hid::SetReportData {
        .type = ::ohos::bluetooth::hid::ReportType::from_value(type),
        .id = id,
        .data = data.empty() ? taihe::array<uint8_t>{}
            : taihe::array<uint8_t>(taihe::copy_data_t{}, data.data(), data.size()),
    };
    setReportEventModule_.PublishEvent(taiheResult);
}

void TaiheBluetoothHidDeviceObserver::OnVirtualCableUnplug()
{
    HILOGD("enter");
    virtualCableUnplugEventModule_.PublishEvent(::ohos::bluetooth::hid::UndefinedType::make_undefined());
}

}  // namespace Bluetooth
}  // namespace OHOS
