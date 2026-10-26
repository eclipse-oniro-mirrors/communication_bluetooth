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

#ifndef TAIHE_BLUETOOTH_GATT_CLIENT_CALLBACK_H_
#define TAIHE_BLUETOOTH_GATT_CLIENT_CALLBACK_H_

#include <shared_mutex>

#include "bluetooth_gatt_client.h"
#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "stdexcept"
#include "taihe_async_callback.h"
#include "taihe/runtime.hpp"
#include "taihe_event_subscribe_module.h"

namespace OHOS {
namespace Bluetooth {

const char * const STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE = "BLECharacteristicChange";
const char * const STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTION_STATE_CHANGE = "BLEConnectionStateChange";
const char * const STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE = "BLEMtuChange";
const char * const STR_BT_GATT_CLIENT_CALLBACK_SERVICE_CHANGE = "serviceChange";
const char * const STR_BT_GATT_CLIENT_CALLBACK_BLE_PHY_UPDATE = "BlePhyUpdate";
class TaiheGattClientCallback : public GattClientCallback {
public:
    void OnConnectionStateChanged(int connectionState, int ret) override;
    void OnConnectionStateChangedWithMessage(
        int connectionState, int ret, GattDisconnectParam disconnectParam) override;
    void OnCharacteristicChanged(const GattCharacteristic &characteristic) override;
    void OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)  override;
    void OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret) override;
    void OnCharacteristicWriteResultWithContext(const GattCharacteristic &characteristic,
        const BluetoothGattRspContext &rspContext, int ret) override;
    void OnDescriptorReadResult(const GattDescriptor &descriptor, int ret) override;
    void OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret) override;
    void OnMtuUpdate(int mtu, int ret) override;
    void OnServicesDiscovered(int status) override;
    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override;
    void OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status) override;
    void OnReadRemoteRssiValueResult(int rssi, int status)  override;
    void OnBlePhyUpdate(int32_t txPhy, int32_t rxPhy, int32_t status) override;
    void OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status) override;
    void OnServicesChanged() override;

    std::string GetDeviceAddr(void)
    {
        return deviceAddr_;
    }

    void SetDeviceAddr(std::string deviceAddr)
    {
        deviceAddr_ = deviceAddr;
    }

    TaiheGattClientCallback();
    ~TaiheGattClientCallback() override = default;

    TaiheAsyncWorkMap asyncWorkMap_ {};

    TaiheEventSubscribeModule eventSubscribe_;
private:
    std::string deviceAddr_ = INVALID_MAC_ADDRESS;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* TAIHE_BLUETOOTH_GATT_CLIENT_CALLBACK_H_ */