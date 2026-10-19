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
#define LOG_TAG "bt_taihe_gatt_server_callback"
#endif

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_gatt_server.h"
#include "taihe_bluetooth_gatt_server_callback.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

TaiheGattServerCallback::TaiheGattServerCallback()
    : eventSubscribe_({STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ,
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE,
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ,
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE,
        STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE,
        STR_BT_GATT_SERVER_CALLBACK_MTU_CHANGE,
        STR_BT_GATT_SERVER_CALLBACK_BLE_PHY_UPDATE},
        "bluetoothBleGattServer_taihe")
{}

void TaiheGattServerCallback::OnCharacteristicReadRequest(
    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    std::string serviceUuid;
    if (characteristic.GetService() != nullptr) {
        serviceUuid = characteristic.GetService()->GetUuid().ToString();
    }
    ::ohos::bluetooth::ble::CharacteristicReadRequest request{
        taihe::string(device.GetDeviceAddr()),
        requestId,
        0,
        taihe::string(characteristic.GetUuid().ToString()),
        taihe::string(serviceUuid)
    };
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::CharacteristicReadRequest const&)
        >(STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ);
    module.PublishEvent(request);
}

void TaiheGattServerCallback::OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    std::string serviceUuid;
    if (characteristic.GetService() != nullptr) {
        serviceUuid = characteristic.GetService()->GetUuid().ToString();
    }
    size_t valueSize = 0;
    const auto &valueData = characteristic.GetValue(&valueSize);
    ::ohos::bluetooth::ble::CharacteristicWriteRequest request{
        taihe::string(device.GetDeviceAddr()),
        requestId,
        0,
        false,
        characteristic.GetWriteType() == GattCharacteristic::WriteType::DEFAULT,
        taihe::array<uint8_t>(taihe::copy_data, valueData.get(), valueSize),
        taihe::string(characteristic.GetUuid().ToString()),
        taihe::string(serviceUuid)
    };
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::CharacteristicWriteRequest const&)
        >(STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE);
    module.PublishEvent(request);
}

void TaiheGattServerCallback::OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
{
    HILOGI("enter, state: %{public}d, remote device address: %{public}s", state, GET_ENCRYPT_ADDR(device));
    std::lock_guard<std::mutex> lock(GattServerImpl::deviceListMutex_);
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("connected");
        bool hasAddr = false;
        for (auto it = GattServerImpl::deviceList_.begin();
                it != GattServerImpl::deviceList_.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                hasAddr = true;
                break;
            }
        }
        if (!hasAddr) {
            HILOGI("add devices");
            GattServerImpl::deviceList_.push_back(device.GetDeviceAddr());
        }
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("disconnected");
        for (auto it = GattServerImpl::deviceList_.begin();
                it != GattServerImpl::deviceList_.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                HILOGI("remove device");
                GattServerImpl::deviceList_.erase(it);
                break;
            }
        }
    }
}

void TaiheGattServerCallback::OnConnectionStateUpdateWithMessage(const BluetoothRemoteDevice &device,
    int state, GattDisconnectParam disconnectParam)
{
    HILOGI("state: %{public}d, reason: %{public}d, remote device address: %{public}s",
        state, disconnectParam.disconnectReason_, GET_ENCRYPT_ADDR(device));
    OnConnectionStateUpdate(device, state);
    ::ohos::bluetooth::ble::BLEConnectionChangeState changeState{
        taihe::string(device.GetDeviceAddr()),
        ::ohos::bluetooth::constant::ProfileConnectionState::from_value(
            TaiheUtils::GetProfileConnectionState(state))
    };
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        changeState.reason.emplace(
            ::ohos::bluetooth::ble::GattDisconnectReason::from_value(disconnectParam.disconnectReason_));
        if (!disconnectParam.reasonMessage_.empty()) {
            changeState.reasonMessage.emplace(taihe::string(disconnectParam.reasonMessage_));
        }
    }
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::BLEConnectionChangeState const&)
        >(STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE);
    module.PublishEvent(changeState);
}

void TaiheGattServerCallback::OnDescriptorWriteRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    std::string characteristicUuid;
    std::string serviceUuid;
    if (descriptor.GetCharacteristic() != nullptr) {
        characteristicUuid = descriptor.GetCharacteristic()->GetUuid().ToString();
        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            serviceUuid = descriptor.GetCharacteristic()->GetService()->GetUuid().ToString();
        }
    }
    size_t valueSize = 0;
    const auto &valueData = descriptor.GetValue(&valueSize);
    ::ohos::bluetooth::ble::DescriptorWriteRequest request{
        taihe::string(device.GetDeviceAddr()),
        requestId,
        0,
        false,
        true,
        taihe::array<uint8_t>(taihe::copy_data, valueData.get(), valueSize),
        taihe::string(descriptor.GetUuid().ToString()),
        taihe::string(characteristicUuid),
        taihe::string(serviceUuid)
    };
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::DescriptorWriteRequest const&)
        >(STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE);
    module.PublishEvent(request);
}

void TaiheGattServerCallback::OnDescriptorReadRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    std::string characteristicUuid;
    std::string serviceUuid;
    if (descriptor.GetCharacteristic() != nullptr) {
        characteristicUuid = descriptor.GetCharacteristic()->GetUuid().ToString();
        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            serviceUuid = descriptor.GetCharacteristic()->GetService()->GetUuid().ToString();
        }
    }
    ::ohos::bluetooth::ble::DescriptorReadRequest request{
        taihe::string(device.GetDeviceAddr()),
        requestId,
        0,
        taihe::string(descriptor.GetUuid().ToString()),
        taihe::string(characteristicUuid),
        taihe::string(serviceUuid)
    };
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::DescriptorReadRequest const&)
        >(STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ);
    module.PublishEvent(request);
}

void TaiheGattServerCallback::OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu)
{
    HILOGI("enter, remote device address: %{public}s, mtu: %{public}d",
        GET_ENCRYPT_ADDR(device), mtu);
    auto &module = eventSubscribe_.GetModule<void(int32_t)>(STR_BT_GATT_SERVER_CALLBACK_MTU_CHANGE);
    module.PublishEvent(mtu);
}

void TaiheGattServerCallback::OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int ret)
{
    HILOGI("ret: %{public}d", ret);
    TaiheAsyncWorkSettleIntResult(asyncWorkMap_, TaiheAsyncType::GATT_SERVER_NOTIFY_CHARACTERISTIC, ret);
}

void TaiheGattServerCallback::OnBlePhyUpdate(const BluetoothRemoteDevice &device,
    int32_t txPhy, int32_t rxPhy, int32_t status)
{
    HILOGI("remote device address: %{public}s, txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d",
        GET_ENCRYPT_ADDR(device), txPhy, rxPhy, status);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_SERVER_SET_PHY, nullptr, status);
    ::ohos::bluetooth::ble::PhyValue phyValue{
        ::ohos::bluetooth::ble::BlePhy::from_value(txPhy),
        ::ohos::bluetooth::ble::BlePhy::from_value(rxPhy)
    };
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::PhyValue const&)
        >(STR_BT_GATT_SERVER_CALLBACK_BLE_PHY_UPDATE);
    module.PublishEvent(phyValue);
}

void TaiheGattServerCallback::OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status)
{
    HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
    auto taihePhyValue = std::make_shared<TaiheNativePhyValue>(txPhy, rxPhy);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_SERVER_READ_PHY, taihePhyValue, status);
}
} // namespace Bluetooth
} // namespace OHOS