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
#define LOG_TAG "bt_taihe_map_mse_observer"
#endif

#include "ohos.bluetooth.map.proj.hpp"
#include "ohos.bluetooth.map.impl.hpp"
#include "bluetooth_utils.h"
#include "bluetooth_log.h"
#include "taihe_bluetooth_map_mse_observer.h"
#include "taihe/runtime.hpp"
#include "taihe_bluetooth_utils.h"
#include "stdexcept"

namespace OHOS {
namespace Bluetooth {

TaiheBluetoothMapMseObserver::TaiheBluetoothMapMseObserver()
{}

void TaiheBluetoothMapMseObserver::OnConnectionStateChanged(
    const BluetoothRemoteDevice &device, int32_t state, int32_t cause)
{
    HILOGD("enter, remote device: %{public}s, state: %{public}d, cause: %{public}d",
        GET_ENCRYPT_ADDR(device), state, cause);
    ::taihe::string deviceId_(device.GetDeviceAddr());
    int32_t profileConnectionState = TaiheUtils::GetProfileConnectionState(state);
    auto taiheResult = ::ohos::bluetooth::baseProfile::StateChangeParam {
        .deviceId = deviceId_,
        .state = ::ohos::bluetooth::constant::ProfileConnectionState::from_value(profileConnectionState),
        .cause = ::ohos::bluetooth::baseProfile::DisconnectCause::from_value(cause),
        .role = ::taihe::optional<::ohos::bluetooth::baseProfile::PanRole>(),
    };
    connectionStateChange_.PublishEvent(taiheResult);
}

}  // namespace Bluetooth
}  // namespace OHOS
