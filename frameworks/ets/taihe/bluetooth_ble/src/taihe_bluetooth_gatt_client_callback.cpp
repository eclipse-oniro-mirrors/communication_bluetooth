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

#include "taihe_bluetooth_gatt_client_callback.h"

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_gatt_client.h"

namespace OHOS {
namespace Bluetooth {
TaiheGattClientCallback::TaiheGattClientCallback()
    : eventSubscribe_({STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE,
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTION_STATE_CHANGE,
        STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE,
        STR_BT_GATT_CLIENT_CALLBACK_SERVICE_CHANGE,
        STR_BT_GATT_CLIENT_CALLBACK_BLE_PHY_UPDATE},
        "bluetoothBleGattClient_taihe")
{}

void TaiheGattClientCallback::OnCharacteristicChanged(const GattCharacteristic &characteristic)
{
    HILOGI("UUID: %{public}s", characteristic.GetUuid().ToString().c_str());
    std::string serviceUuid;
    if (characteristic.GetService() != nullptr) {
        serviceUuid = characteristic.GetService()->GetUuid().ToString();
    }
    size_t valueSize = 0;
    const auto &valueData = characteristic.GetValue(&valueSize);
    std::vector<::ohos::bluetooth::ble::BLEDescriptor> descriptorVec;
    auto &descriptors = const_cast<GattCharacteristic &>(characteristic).GetDescriptors();
    descriptorVec.reserve(descriptors.size());
    for (const auto &descriptor : descriptors) {
        size_t descriptorValueSize = 0;
        const auto &descriptorValueData = descriptor.GetValue(&descriptorValueSize);
        ::ohos::bluetooth::ble::BLEDescriptor taiheDescriptor{
            taihe::string(serviceUuid),
            taihe::string(characteristic.GetUuid().ToString()),
            taihe::string(descriptor.GetUuid().ToString()),
            taihe::array<uint8_t>(taihe::copy_data, descriptorValueData.get(), descriptorValueSize),
        };
        taiheDescriptor.descriptorHandle.emplace(static_cast<int32_t>(descriptor.GetHandle()));
        descriptorVec.push_back(std::move(taiheDescriptor));
    }
    ::ohos::bluetooth::ble::BLECharacteristic taiheCharacter{
        taihe::string(serviceUuid),
        taihe::string(characteristic.GetUuid().ToString()),
        taihe::array<uint8_t>(taihe::copy_data, valueData.get(), valueSize),
        taihe::array<::ohos::bluetooth::ble::BLEDescriptor>(
            taihe::move_data, descriptorVec.data(), descriptorVec.size())
    };
    taiheCharacter.characteristicValueHandle.emplace(static_cast<int32_t>(characteristic.GetHandle()));
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::BLECharacteristic const&)
        >(STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE);
    module.PublishEvent(taiheCharacter);
}

void TaiheGattClientCallback::OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
    auto taiheCharacter = std::make_shared<TaiheNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_READ_CHARACTER, taiheCharacter, ret);
}

void TaiheGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
    auto taiheDescriptor = std::make_shared<TaiheNativeBleDescriptor>(descriptor);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_READ_DESCRIPTOR, taiheDescriptor, ret);
}

void TaiheGattClientCallback::OnConnectionStateChanged(int connectionState, int ret)
{
    HILOGI("connectionState:%{public}d, ret:%{public}d", connectionState, ret);
}

void TaiheGattClientCallback::OnConnectionStateChangedWithMessage(
    int connectionState, int ret, GattDisconnectParam disconnectParam)
{
    HILOGI("connectionState:%{public}d, ret:%{public}d, reason:%{public}d",
        connectionState, ret, disconnectParam.disconnectReason_);
    ::ohos::bluetooth::ble::BLEConnectionChangeState state{
        taihe::string(deviceAddr_),
        ::ohos::bluetooth::constant::ProfileConnectionState::from_value(
            TaiheUtils::GetProfileConnectionState(connectionState))
    };
    if (connectionState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        state.reason.emplace(
            ::ohos::bluetooth::ble::GattDisconnectReason::from_value(disconnectParam.disconnectReason_));
        if (!disconnectParam.reasonMessage_.empty()) {
            state.reasonMessage.emplace(taihe::string(disconnectParam.reasonMessage_));
        }
    }
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::BLEConnectionChangeState const&)
        >(STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTION_STATE_CHANGE);
    module.PublishEvent(state);
}

void TaiheGattClientCallback::OnServicesDiscovered(int status)
{
    HILOGI("status:%{public}d", status);
}

void TaiheGattClientCallback::OnConnectionParameterChanged(
    int interval, int latency, int timeout, int status)
{
    HILOGI("status: %{public}d", status);
    int ret = (status == static_cast<int>(GattStatus::GATT_SUCCESS))
        ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR;
    AsyncWorkCallFunction(asyncWorkMap_,
        TaiheAsyncType::GATT_CLIENT_UPDATE_CONNECTION_PRIORITY, nullptr, ret);
}

void TaiheGattClientCallback::OnReadRemoteRssiValueResult(int rssi, int ret)
{
    HILOGI("rssi: %{public}d, ret: %{public}d", rssi, ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret));
    auto taiheRssi = std::make_shared<TaiheNativeInt>(rssi);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_READ_REMOTE_RSSI_VALUE, taiheRssi, ret);
}

void TaiheGattClientCallback::OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
    auto taiheCharacter = std::make_shared<TaiheNativeBleCharacteristic>(characteristic);
    TaiheAsyncWorkSettleWithObject(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_WRITE_CHARACTER, taiheCharacter, ret);
}

void TaiheGattClientCallback::OnCharacteristicWriteResultWithContext(const GattCharacteristic &characteristic,
    const BluetoothGattRspContext &rspContext, int ret)
{
    HILOGI("UUID: %{public}s, timeStamp:%{public}lld, ret: %{public}d", characteristic.GetUuid().ToString().c_str(),
        static_cast<long long>(rspContext.GetTimeStamp()), ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
    auto taiheRspContext = std::make_shared<TaiheNativeGattResponseContext>(rspContext);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_WRITE_CHARACTER_WITH_CONTEXT, taiheRspContext,
        ret);
}

void TaiheGattClientCallback::OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret));
    auto taiheDescriptor = std::make_shared<TaiheNativeBleDescriptor>(descriptor);
    TaiheAsyncWorkSettleWithObject(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_WRITE_DESCRIPTOR, taiheDescriptor, ret);
}

void TaiheGattClientCallback::OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status)
{
    HILOGI("UUID: %{public}s, status: %{public}d", characteristic.GetUuid().ToString().c_str(), status);
    status = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(status)); // Adaptation for old sdk
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_ENABLE_CHARACTER_CHANGED, nullptr, status);
}

void TaiheGattClientCallback::OnMtuUpdate(int mtu, int ret)
{
    HILOGI("ret: %{public}d, mtu: %{public}d", ret, mtu);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret));
    auto taiheMtu = std::make_shared<TaiheNativeInt>(mtu);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_MTU_CHANGED, taiheMtu, ret);
    auto &module = eventSubscribe_.GetModule<void(int32_t)>(STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE);
    module.PublishEvent(mtu);
}

void TaiheGattClientCallback::OnBlePhyUpdate(int32_t txPhy, int32_t rxPhy, int32_t status)
{
    HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
    status = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(status));
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_SET_PHY, nullptr, status);
    ::ohos::bluetooth::ble::PhyValue phyValue{
        ::ohos::bluetooth::ble::BlePhy::from_value(txPhy),
        ::ohos::bluetooth::ble::BlePhy::from_value(rxPhy)
    };
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::PhyValue const&)
        >(STR_BT_GATT_CLIENT_CALLBACK_BLE_PHY_UPDATE);
    module.PublishEvent(phyValue);
}

void TaiheGattClientCallback::OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status)
{
    HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
    status = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(status));
    auto taihePhyValue = std::make_shared<TaiheNativePhyValue>(txPhy, rxPhy);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_READ_PHY, taihePhyValue, status);
}

void TaiheGattClientCallback::OnServicesChanged()
{
    HILOGI("enter");
    auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::UndefinedType const&)
        >(STR_BT_GATT_CLIENT_CALLBACK_SERVICE_CHANGE);
    module.PublishEvent(::ohos::bluetooth::ble::UndefinedType::make_undefined());
}

} // namespace Bluetooth
} // namespace OHOS