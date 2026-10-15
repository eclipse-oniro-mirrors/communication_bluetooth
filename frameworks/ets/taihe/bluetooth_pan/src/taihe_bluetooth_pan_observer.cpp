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
#define LOG_TAG "bt_taihe_pan_observer"
#endif

#include "ohos.bluetooth.pan.proj.hpp"
#include "ohos.bluetooth.pan.impl.hpp"
#include "bluetooth_utils.h"
#include "bluetooth_log.h"
#include "taihe_bluetooth_pan_observer.h"
#include "taihe/runtime.hpp"
#include "taihe_bluetooth_utils.h"
#include "stdexcept"

namespace OHOS {
namespace Bluetooth {

namespace {
constexpr int NATIVE_PAN_ROLE_NAP = 1;
constexpr int NATIVE_PAN_ROLE_PANU = 2;

::taihe::optional<::ohos::bluetooth::baseProfile::PanRole> ConvertPanRole(int role)
{
    using PanRole = ::ohos::bluetooth::baseProfile::PanRole;
    switch (role) {
        case NATIVE_PAN_ROLE_NAP:
            return ::taihe::optional<PanRole>(
                std::in_place, PanRole::from_value(static_cast<int32_t>(PanRole::key_t::ROLE_PANNAP)));
        case NATIVE_PAN_ROLE_PANU:
            return ::taihe::optional<PanRole>(
                std::in_place, PanRole::from_value(static_cast<int32_t>(PanRole::key_t::ROLE_PANU)));
        default:
            return ::taihe::optional<PanRole>();
    }
}
}

TaiheBluetoothPanObserver::TaiheBluetoothPanObserver()
{}

void TaiheBluetoothPanObserver::OnConnectionStateChanged(
    const BluetoothRemoteDevice &device, int state, int cause, int role)
{
    HILOGD("enter, remote device: %{public}s, state: %{public}d, cause: %{public}d, role: %{public}d",
        GET_ENCRYPT_ADDR(device), state, cause, role);
    ::taihe::string deviceId_(device.GetDeviceAddr());
    int32_t profileConnectionState = TaiheUtils::GetProfileConnectionState(state);
    auto taiheResult = ::ohos::bluetooth::baseProfile::StateChangeParam {
        .deviceId = deviceId_,
        .state = ::ohos::bluetooth::constant::ProfileConnectionState::from_value(profileConnectionState),
        .cause = ::ohos::bluetooth::baseProfile::DisconnectCause::from_value(cause),
        .role = ConvertPanRole(role),
    };
    connectionStateChange_.PublishEvent(taiheResult);
}

}  // namespace Bluetooth
}  // namespace OHOS
