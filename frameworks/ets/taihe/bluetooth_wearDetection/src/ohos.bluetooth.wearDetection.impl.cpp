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
#define LOG_TAG "bt_wearDetection_impl_ohbluetooth"
#endif

#include "ohos.bluetooth.wearDetection.proj.hpp"
#include "ohos.bluetooth.wearDetection.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_errorcode.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_bluetooth_error.h"
#include "taihe_ha_event_utils.h"

namespace OHOS {
namespace Bluetooth {

constexpr int WEARDETECTION_ENABLED = 1;
constexpr int ENABLE_WEARDETECTION_UNSUPPORT = -1;

void EnableWearDetection(::taihe::string_view deviceId)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "wearDetection.enableWearDetection", validErrCodes);
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

    BluetoothAudioManager &audioManager = BluetoothAudioManager::GetInstance();
    int32_t errorCode = audioManager.EnableWearDetection(remoteAddr);
    HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
}

void DisableWearDetection(::taihe::string_view deviceId)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "wearDetection.disableWearDetection", validErrCodes);
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

    BluetoothAudioManager &audioManager = BluetoothAudioManager::GetInstance();
    int32_t errorCode = audioManager.DisableWearDetection(remoteAddr);
    HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
}

bool IsWearDetectionSupported(::taihe::string_view deviceId)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "wearDetection.isWearDetectionSupported", validErrCodes);
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM, false);

    BluetoothAudioManager &audioManager = BluetoothAudioManager::GetInstance();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    bool isSupported = false;
    int32_t errorCode = audioManager.IsWearDetectionSupported(device, isSupported);
    HILOGI("errorCode:%{public}s, isSupported:%{public}d", GetTaiheErrMsg(errorCode).c_str(), isSupported);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode, false);

    return isSupported;
}

bool IsWearDetectionEnabled(::taihe::string_view deviceId)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "wearDetection.isWearDetectionEnabled", validErrCodes);
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM, false);

    BluetoothAudioManager &audioManager = BluetoothAudioManager::GetInstance();
    int32_t ability = ENABLE_WEARDETECTION_UNSUPPORT;
    int32_t errorCode = audioManager.GetWearDetectionState(remoteAddr, ability);
    HILOGI("errorCode:%{public}s, ability:%{public}d", GetTaiheErrMsg(errorCode).c_str(), ability);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode, false);

    return (ability == WEARDETECTION_ENABLED);
}

}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_EnableWearDetectionAsync(OHOS::Bluetooth::EnableWearDetection);
TH_EXPORT_CPP_API_EnableWearDetectionPromise(OHOS::Bluetooth::EnableWearDetection);
TH_EXPORT_CPP_API_DisableWearDetectionAsync(OHOS::Bluetooth::DisableWearDetection);
TH_EXPORT_CPP_API_DisableWearDetectionPromise(OHOS::Bluetooth::DisableWearDetection);
TH_EXPORT_CPP_API_IsWearDetectionSupportedAsync(OHOS::Bluetooth::IsWearDetectionSupported);
TH_EXPORT_CPP_API_IsWearDetectionSupportedPromise(OHOS::Bluetooth::IsWearDetectionSupported);
TH_EXPORT_CPP_API_IsWearDetectionEnabledAsync(OHOS::Bluetooth::IsWearDetectionEnabled);
TH_EXPORT_CPP_API_IsWearDetectionEnabledPromise(OHOS::Bluetooth::IsWearDetectionEnabled);
// NOLINTEND
