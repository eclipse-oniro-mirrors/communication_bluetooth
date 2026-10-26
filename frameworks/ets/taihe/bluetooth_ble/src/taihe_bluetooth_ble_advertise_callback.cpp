/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_taihe_ble_advertise_callback"
#endif

#include "taihe_bluetooth_ble_advertise_callback.h"

#include "bluetooth_log.h"
#include "taihe_bluetooth_ble_utils.h"

namespace OHOS {
namespace Bluetooth {

auto g_eventSubscribe = std::make_shared<TaiheEventSubscribeModule>(
    REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE, "bluetoothBleAdvertise_taihe");

static void PublishAdvertisingStateEvent(int advHandle, ::ohos::bluetooth::ble::AdvertisingState::key_t state)
{
    ::ohos::bluetooth::ble::AdvertisingStateChangeInfo info{
        advHandle, ::ohos::bluetooth::ble::AdvertisingState(state) };
    auto &module = g_eventSubscribe->GetModule<
        void(::ohos::bluetooth::ble::AdvertisingStateChangeInfo const&)>(REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE);
    module.PublishEvent(info);
}

TaiheBluetoothBleAdvertiseCallback::TaiheBluetoothBleAdvertiseCallback()
{}

std::shared_ptr<TaiheBluetoothBleAdvertiseCallback> TaiheBluetoothBleAdvertiseCallback::GetInstance(void)
{
    static std::shared_ptr<TaiheBluetoothBleAdvertiseCallback> instance =
        std::make_shared<TaiheBluetoothBleAdvertiseCallback>();
    return instance;
}

const std::shared_ptr<TaiheEventSubscribeModule> &TaiheBluetoothBleAdvertiseCallback::GetEventSubscribe()
{
    return g_eventSubscribe;
}

void TaiheBluetoothBleAdvertiseCallback::OnStartResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        HILOGI("OnStartResultEvent, advHandle is same, advHandle: %{public}d", advHandle_);
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        auto taiheAdvHandle = std::make_shared<TaiheNativeInt>(advHandle);
        AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GET_ADVERTISING_HANDLE, taiheAdvHandle, result);
    }
}

void TaiheBluetoothBleAdvertiseCallback::OnEnableResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        HILOGI("OnEnableResultEvent, advHandle is same, advHandle: %{public}d", advHandle_);
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        TaiheAsyncWorkSettleIntResult(asyncWorkMap_, TaiheAsyncType::BLE_ENABLE_ADVERTISING, result);
    }
    ::ohos::bluetooth::ble::AdvertisingStateChangeInfo info{
        advHandle,
        ::ohos::bluetooth::ble::AdvertisingState(::ohos::bluetooth::ble::AdvertisingState::key_t::ENABLED)
    };
    auto &module = g_eventSubscribe->GetModule<
        void(::ohos::bluetooth::ble::AdvertisingStateChangeInfo const&)>(REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE);
    module.PublishEvent(info);
}

void TaiheBluetoothBleAdvertiseCallback::OnDisableResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        HILOGI("OnDisableResultEvent, advHandle is same, advHandle: %{public}d", advHandle_);
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        TaiheAsyncWorkSettleIntResult(asyncWorkMap_, TaiheAsyncType::BLE_DISABLE_ADVERTISING, result);
    }
    ::ohos::bluetooth::ble::AdvertisingStateChangeInfo info{
        advHandle,
        ::ohos::bluetooth::ble::AdvertisingState(::ohos::bluetooth::ble::AdvertisingState::key_t::DISABLED)
    };
    auto &module = g_eventSubscribe->GetModule<
        void(::ohos::bluetooth::ble::AdvertisingStateChangeInfo const&)>(REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE);
    module.PublishEvent(info);
}

void TaiheBluetoothBleAdvertiseCallback::OnStopResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        TaiheAsyncWorkSettleIntResult(asyncWorkMap_, TaiheAsyncType::BLE_STOP_ADVERTISING, result);
        advHandle_ = -1;
    }
}

void TaiheBluetoothBleAdvertiseAsyncCallback::OnStartResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        HILOGI("OnStartResultEvent, advHandle is same, advHandle: %{public}d", advHandle_);
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        auto taiheAdvHandle = std::make_shared<TaiheNativeInt>(advHandle);
        AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GET_ADVERTISING_HANDLE, taiheAdvHandle, result);
    }
    PublishAdvertisingStateEvent(advHandle, ::ohos::bluetooth::ble::AdvertisingState::key_t::STARTED);
}

void TaiheBluetoothBleAdvertiseAsyncCallback::OnStopResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        HILOGI("OnStopResultEvent, advHandle is same, advHandle: %{public}d", advHandle_);
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        TaiheAsyncWorkSettleIntResult(asyncWorkMap_, TaiheAsyncType::BLE_STOP_ADVERTISING, result);
        advHandle_ = -1;
    }
    PublishAdvertisingStateEvent(advHandle, ::ohos::bluetooth::ble::AdvertisingState::key_t::STOPPED);
}

void TaiheBluetoothBleAdvertiseCallback::OnSetAdvDataEvent(int result)
{}

void TaiheBluetoothBleAdvertiseCallback::OnGetAdvHandleEvent(int result, int advHandle)
{
    advHandle_ = advHandle;
}

void TaiheBluetoothBleAdvertiseCallback::OnChangeAdvResultEvent(int result, int advHandle)
{}
}  // namespace Bluetooth
}  // namespace OHOS