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
#define LOG_TAG "bt_taihe_bas_impl"
#endif

#include "ohos.bluetooth.bas.impl.hpp"
#include "ohos.bluetooth.bas.proj.hpp"
#include "taihe/runtime.hpp"

#include <map>
#include <mutex>
#include <string>

#include "bluetooth_bas_host.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_utils.h"
#include "taihe_async_work.h"
#include "taihe_bluetooth_bas_observer.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_ha_event_utils.h"

using namespace OHOS::Bluetooth;

namespace {
std::shared_ptr<OHOS::Bluetooth::TaiheBasCallback> g_basCallback = OHOS::Bluetooth::TaiheBasCallback::GetInstance();

void RegisterBasObserver()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, []() {
        int32_t ret = BluetoothBasHost::GetProfile()->RegisterBatteryObserver(g_basCallback);
        HILOGI("RegisterBatteryObserver ret: %{public}d", ret);
    });
}

bool IsBasSupported()
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_SYSTEM_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                           BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "bas.IsBasSupported", validErrCodes);
    bool isSupported = false;
    int32_t ret = BluetoothBasHost::GetProfile()->IsBasSupported(isSupported);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(ret == BT_NO_ERROR, ret, false);
    return isSupported;
}

static TaihePromiseAndCallback TaiheGetRemoteDeviceBatteryInfo(
    const ::ohos::bluetooth::common::BluetoothAddress &deviceId, uintptr_t cb, bool isPromise = true)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "bas.GetRemoteDeviceBatteryInfo", validErrCodes);

    std::string addrStr(deviceId.address.c_str());
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(IsValidAddress(addrStr), BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));

    RegisterBasObserver();

    auto func = [addrStr]() {
        int ret = BluetoothBasHost::GetProfile()->GetBatteryLevel(addrStr);
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, reinterpret_cast<ani_object>(cb), func,
        apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    bool success = g_basCallback->asyncWorkMap_.TryPush(TaiheAsyncType::BAS_GET_BATTERY_LEVEL, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    asyncWork->Run();

    return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(isPromise ? asyncWork->GetRet() : nullptr));
}

uintptr_t GetRemoteDeviceBatteryInfoPromise(
    ::ohos::bluetooth::common::BluetoothAddress const& deviceId)
{
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TaihePromiseAndCallback result = TaiheGetRemoteDeviceBatteryInfo(deviceId,
        reinterpret_cast<uintptr_t>(nullptr));
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN_VERIFY(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

void GetRemoteDeviceBatteryInfoAsync(
    ::ohos::bluetooth::common::BluetoothAddress const& deviceId, uintptr_t callback)
{
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TaihePromiseAndCallback result = TaiheGetRemoteDeviceBatteryInfo(deviceId, callback, false);
    if (!result.success) {
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(false, result.errorCode);
    }
}

void OnBatteryChange(::taihe::callback_view<void(::ohos::bluetooth::bas::BatteryInfo const& data)> callback)
{
    HILOGI("enter");
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "bas.OnBatteryChange", {});
    g_basCallback->batteryChangeEventModule_.RegisterEvent(callback);

    std::map<std::string, int32_t> batteryInfos;
    int32_t ret = BluetoothBasHost::GetProfile()->GetConnectedDeviceBatteryInfos(batteryInfos);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
    const int32_t maxBatteryLevel = 100;
    for (const auto& [deviceAddr, batteryLevel] : batteryInfos) {
        if (batteryLevel < 0 || batteryLevel > maxBatteryLevel || !IsValidAddress(deviceAddr)) {
            continue;
        }
        BluetoothRemoteDevice remoteDevice(deviceAddr, BTTransport::ADAPTER_BLE);
        g_basCallback->OnBatteryLevelChanged(remoteDevice, batteryLevel);
    }
}

void OffBatteryChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::bluetooth::bas::BatteryInfo const& data)>> callback)
{
    HILOGI("enter");
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "bas.OffBatteryChange", {});
    g_basCallback->batteryChangeEventModule_.DeregisterEvent(callback);
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_IsBasSupported(IsBasSupported);
TH_EXPORT_CPP_API_GetRemoteDeviceBatteryInfoPromise(GetRemoteDeviceBatteryInfoPromise);
TH_EXPORT_CPP_API_GetRemoteDeviceBatteryInfoAsync(GetRemoteDeviceBatteryInfoAsync);
TH_EXPORT_CPP_API_OnBatteryChange(OnBatteryChange);
TH_EXPORT_CPP_API_OffBatteryChange(OffBatteryChange);
// NOLINTEND
