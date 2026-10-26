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

#ifndef TAIHE_BLUETOOTH_BLE_ADVERTISE_CALLBACK_H
#define TAIHE_BLUETOOTH_BLE_ADVERTISE_CALLBACK_H

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "event_handler.h"
#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "stdexcept"
#include "taihe_async_callback.h"
#include "taihe/runtime.hpp"
#include "taihe_event_subscribe_module.h"

namespace OHOS {
namespace Bluetooth {

const char * const REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE = "advertisingStateChange";

class TaiheBluetoothBleAdvertiseCallback : public BleAdvertiseCallback {
public:
    TaiheBluetoothBleAdvertiseCallback();
    ~TaiheBluetoothBleAdvertiseCallback() override = default;

    static std::shared_ptr<TaiheBluetoothBleAdvertiseCallback> GetInstance(void);

    void OnStartResultEvent(int result, int advHandle) override;
    void OnEnableResultEvent(int result, int advHandle) override;
    void OnDisableResultEvent(int result, int advHandle) override;
    void OnStopResultEvent(int result, int advHandle) override;
    void OnSetAdvDataEvent(int result) override;
    void OnGetAdvHandleEvent(int result, int advHandle) override;
    void OnChangeAdvResultEvent(int result, int advHandle) override;

    TaiheAsyncWorkMap asyncWorkMap_ {};

    const std::shared_ptr<TaiheEventSubscribeModule> &GetEventSubscribe();

    int advHandle_ = -1;
};

class TaiheBluetoothBleAdvertiseAsyncCallback : public TaiheBluetoothBleAdvertiseCallback {
public:
    TaiheBluetoothBleAdvertiseAsyncCallback() = default;
    ~TaiheBluetoothBleAdvertiseAsyncCallback() override = default;

    void OnStartResultEvent(int result, int advHandle) override;
    void OnStopResultEvent(int result, int advHandle) override;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_BLUETOOTH_BLE_ADVERTISE_CALLBACK_H