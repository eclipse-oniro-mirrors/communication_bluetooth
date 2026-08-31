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
#define LOG_TAG "bt_taihe_gatt_server"
#endif

#include "taihe_bluetooth_gatt_server.h"

#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_event_module.h"
#include "taihe_ha_event_utils.h"
#include "taihe_parser_utils.h"

namespace OHOS {
namespace Bluetooth {

std::vector<std::string> GattServerImpl::deviceList_;
std::mutex GattServerImpl::deviceListMutex_;

void GattServerImpl::AddService(ohos::bluetooth::ble::GattService service)
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(server_ != nullptr, BT_ERR_INVALID_PARAM);
    std::unique_ptr<GattService> gattService {nullptr};
    GattServiceType type = service.isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY;
    UUID serviceUuid = UUID::FromString(std::string(service.serviceUuid));
    gattService = std::make_unique<GattService>(serviceUuid, type);

    TaiheGattService taiheGattService;
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(
        TaiheParseGattService(service, taiheGattService) == TAIHE_OK, BT_ERR_INVALID_PARAM);

    for (auto &characteristic : taiheGattService.characteristics) {
        int charPermissions = characteristic.permissions;
        int charProperties = characteristic.properties;
        GattCharacteristic character(characteristic.characteristicUuid, charPermissions, charProperties);
        character.SetValue(characteristic.characteristicValue.data(), characteristic.characteristicValue.size());

        for (const auto &descriptor : characteristic.descriptors) {
            GattDescriptor gattDescriptor(descriptor.descriptorUuid, descriptor.permissions);
            gattDescriptor.SetValue(descriptor.descriptorValue.data(), descriptor.descriptorValue.size());
            character.AddDescriptor(gattDescriptor);
        }
        gattService->AddCharacteristic(character);
    }
    int ret = server_->AddService(*gattService);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void GattServerImpl::Close()
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(server_ != nullptr, BT_ERR_INTERNAL_ERROR);
    int ret = server_->Close();
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void GattServerImpl::RemoveService(taihe::string_view serviceUuid)
{
    HILOGI("enter");
    UUID uuid;
    auto status = ParseUuidParams(std::string(serviceUuid), uuid);
    TAIHE_BT_ASSERT_RETURN_VOID((status == TAIHE_OK && server_ != nullptr), BT_ERR_INVALID_PARAM);

    int ret = BT_NO_ERROR;
    auto primaryService = server_->GetService(uuid, true);
    if (primaryService.has_value()) {
        ret = server_->RemoveGattService(primaryService.value());
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    auto secondService = server_->GetService(uuid, false);
    if (secondService.has_value()) {
        ret = server_->RemoveGattService(secondService.value());
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }
    TAIHE_BT_ASSERT_RETURN_VOID((primaryService.has_value() || secondService.has_value()),
        BT_ERR_INVALID_PARAM);
}

void GattServerImpl::SendResponse(ohos::bluetooth::ble::ServerResponse serverResponse)
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(server_ != nullptr, BT_ERR_INVALID_PARAM);
    std::string deviceId = std::string(serverResponse.deviceId);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(IsValidAddress(deviceId), BT_ERR_INVALID_PARAM);
    BluetoothRemoteDevice remoteDevice(deviceId, BTTransport::ADAPTER_BLE);
    HILOGI("Remote device address: %{public}s", GET_ENCRYPT_ADDR(remoteDevice));
    int32_t transId = serverResponse.transId;
    int32_t status = serverResponse.status;
    int32_t offset = serverResponse.offset;
    int ret = server_->SendResponse(remoteDevice, transId, status, offset,
        serverResponse.value.data(), serverResponse.value.size());
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

static GattCharacteristic *GetGattCharacteristic(const std::shared_ptr<GattServer> &server, const UUID &serviceUuid,
    const UUID &characterUuid)
{
    auto service = server->GetService(serviceUuid, true);
    if (!service.has_value()) {
        service = server->GetService(serviceUuid, false);
    }
    if (!service.has_value()) {
        HILOGE("not found service uuid: %{public}s", serviceUuid.ToString().c_str());
        return nullptr;
    }
    GattCharacteristic *character = service.value().get().GetCharacteristic(characterUuid);
    return character;
}

static TaiheStatus CheckNotifyCharacteristicChangedEx(
    taihe::string_view deviceId, const ohos::bluetooth::ble::NotifyCharacteristic &info,
    std::string &outDeviceId, TaiheNotifyCharacteristic &outCharacter)
{
    std::string bdaddr = static_cast<std::string>(deviceId);
    TaiheNotifyCharacteristic character;
    TAIHE_BT_RETURN_IF(!IsValidAddress(bdaddr), "Invalid bdaddr", TAIHE_INVALID_ARG);
    TAIHE_BT_CALL_RETURN(TaiheParseNotifyCharacteristic(info, character));

    outDeviceId = std::move(bdaddr);
    outCharacter = std::move(character);
    return TAIHE_OK;
}

static TaihePromiseAndCallback TaiheNotifyCharacteristicChanged(taihe::string_view deviceId,
    const ohos::bluetooth::ble::NotifyCharacteristic &notifyCharacteristic, uintptr_t cb,
    GattServerImpl *taiheServer, bool isPromise = true)
{
    HILOGI("enter");
    std::string devId {};
    TaiheNotifyCharacteristic notifyCharacter;
    auto status = CheckNotifyCharacteristicChangedEx(deviceId, notifyCharacteristic, devId, notifyCharacter);
    TAIHE_BT_ASSERT_RETURN((status == TAIHE_OK && taiheServer && taiheServer->GetServer()),
        BT_ERR_INVALID_PARAM, TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));

    auto func = [server = taiheServer->GetServer(), notifyCharacter, devId]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        auto character = GetGattCharacteristic(server, notifyCharacter.serviceUuid, notifyCharacter.characterUuid);
        if (character == nullptr) {
            HILOGI("character is null!");
            return TaiheAsyncWorkRet(ret);
        }
        character->SetValue(notifyCharacter.characterValue.data(), notifyCharacter.characterValue.size());
        BluetoothRemoteDevice remoteDevice(devId, BTTransport::ADAPTER_BLE);
        ret = server->NotifyCharacteristicChanged(remoteDevice, *character, notifyCharacter.confirm);
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(taihe::get_env(), reinterpret_cast<ani_object>(cb), func);
    TAIHE_BT_ASSERT_RETURN(asyncWork, BT_ERR_INTERNAL_ERROR, TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    bool success = taiheServer->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_SERVER_NOTIFY_CHARACTERISTIC,
        asyncWork);
    TAIHE_BT_ASSERT_RETURN(success, BT_ERR_INTERNAL_ERROR, TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    asyncWork->Run();

    return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(isPromise ? asyncWork->GetRet() : nullptr));
}

uintptr_t GattServerImpl::NotifyCharacteristicChangedPromise(taihe::string_view deviceId,
    const ohos::bluetooth::ble::NotifyCharacteristic &notifyCharacteristic)
{
    TaihePromiseAndCallback result = TaiheNotifyCharacteristicChanged(deviceId, notifyCharacteristic,
        reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

void GattServerImpl::NotifyCharacteristicChangedAsync(taihe::string_view deviceId,
    const ohos::bluetooth::ble::NotifyCharacteristic &notifyCharacteristic, uintptr_t callback)
{
    TaihePromiseAndCallback result = TaiheNotifyCharacteristicChanged(deviceId, notifyCharacteristic,
        reinterpret_cast<uintptr_t>(callback), this, false);
    if (!result.success) {
        TAIHE_BT_ASSERT_RETURN_VOID(false, result.errorCode);
    }
}

void GattServerImpl::OnBlePhyUpdate(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::PhyValue const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OnBlePhyUpdate", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::PhyValue const&)>(
        STR_BT_GATT_SERVER_CALLBACK_BLE_PHY_UPDATE, callback);
}

void GattServerImpl::OffBlePhyUpdate(
    ::taihe::optional_view<::taihe::callback<void(::ohos::bluetooth::ble::PhyValue const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OffBlePhyUpdate", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::PhyValue const&)>(
        STR_BT_GATT_SERVER_CALLBACK_BLE_PHY_UPDATE, callback);
}

uintptr_t GattServerImpl::SetPhyPromise(
    ::taihe::string_view deviceId, ::ohos::bluetooth::ble::PhyValue const& phyValue)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    ani_env *env = taihe::get_env();
    TAIHE_BT_CONTEXT_HA(env, "ble.GattServer.SetPhy", validErrCodes);
    TAIHE_BT_ASSERT_RETURN_VERIFY(env != nullptr, BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(server_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));

    std::string devId = static_cast<std::string>(deviceId);
    int32_t txPhy = static_cast<int32_t>(phyValue.txPhy);
    int32_t rxPhy = static_cast<int32_t>(phyValue.rxPhy);
    int32_t phyOptions = phyValue.phyMode.has_value() ? phyValue.phyMode.value() : 0;
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(IsValidAddress(devId), BT_ERR_INVALID_PARAM,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(IsValidPhyValue(txPhy, rxPhy, phyOptions), BT_ERR_INVALID_PARAM,
        reinterpret_cast<uintptr_t>(nullptr));

    auto func = [gattServer = server_, devId, txPhy, rxPhy, phyOptions]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattServer) {
            ret = gattServer->SetPhy(devId, txPhy, rxPhy, phyOptions);
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_SERVER_SET_PHY, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();
    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

uintptr_t GattServerImpl::ReadPhyPromise(::taihe::string_view deviceId)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    ani_env *env = taihe::get_env();
    TAIHE_BT_CONTEXT_HA(env, "ble.GattServer.ReadPhy", validErrCodes);
    TAIHE_BT_ASSERT_RETURN_VERIFY(env != nullptr, BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(server_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));

    std::string devId = static_cast<std::string>(deviceId);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(IsValidAddress(devId), BT_ERR_INVALID_PARAM,
        reinterpret_cast<uintptr_t>(nullptr));

    auto func = [gattServer = server_, devId]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattServer) {
            ret = gattServer->ReadPhy(devId);
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_SERVER_READ_PHY, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();
    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

void GattServerImpl::OnBLEMtuChange(::taihe::callback_view<void(int32_t data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OnBLEMtuChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(int32_t)>(STR_BT_GATT_SERVER_CALLBACK_MTU_CHANGE, callback);
}

void GattServerImpl::OffBLEMtuChange(::taihe::optional_view<::taihe::callback<void(int32_t data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OffBLEMtuChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(int32_t)>(STR_BT_GATT_SERVER_CALLBACK_MTU_CHANGE, callback);
}

void GattServerImpl::Disconnect(::taihe::string_view deviceId)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.Disconnect", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(server_ != nullptr, BT_ERR_INTERNAL_ERROR);
    std::string devId = static_cast<std::string>(deviceId);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidAddress(devId), BT_ERR_INVALID_PARAM);
    BluetoothRemoteDevice remoteDevice(devId, BTTransport::ADAPTER_BLE);
    int ret = server_->CancelConnection(remoteDevice);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

void GattServerImpl::Connect(::taihe::string_view deviceId, ::taihe::optional_view<bool> autoConnect)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.Connect", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(server_ != nullptr, BT_ERR_INTERNAL_ERROR);
    std::string devId = static_cast<std::string>(deviceId);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidAddress(devId), BT_ERR_INVALID_PARAM);
    bool isAutoConnect = false;
    if (autoConnect.has_value()) {
        isAutoConnect = autoConnect.value();
    }
    BluetoothRemoteDevice remoteDevice(devId, BTTransport::ADAPTER_BLE);
    int ret = server_->Connect(remoteDevice, !isAutoConnect);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

void GattServerImpl::OnCharacteristicRead(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::CharacteristicReadRequest const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OnCharacteristicRead", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::CharacteristicReadRequest const&)>(
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ, callback);
}

void GattServerImpl::OffCharacteristicRead(
    ::taihe::optional_view<::taihe::callback<
        void(::ohos::bluetooth::ble::CharacteristicReadRequest const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OffCharacteristicRead", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::CharacteristicReadRequest const&)>(
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ, callback);
}

void GattServerImpl::OnCharacteristicWrite(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::CharacteristicWriteRequest const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OnCharacteristicWrite", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::CharacteristicWriteRequest const&)>(
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE, callback);
}

void GattServerImpl::OffCharacteristicWrite(
    ::taihe::optional_view<::taihe::callback<
        void(::ohos::bluetooth::ble::CharacteristicWriteRequest const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OffCharacteristicWrite", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::CharacteristicWriteRequest const&)>(
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE, callback);
}

::ohos::bluetooth::constant::ProfileConnectionState GattServerImpl::GetConnectedState(::taihe::string_view deviceId)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.GetConnectedState", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(server_ != nullptr, BT_ERR_INTERNAL_ERROR,
        ::ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

    std::string devId = static_cast<std::string>(deviceId);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(IsValidAddress(devId), BT_ERR_INVALID_PARAM,
        ::ohos::bluetooth::constant::ProfileConnectionState::from_value(0));
    int state = -1;
    int ret = server_->GetConnectedState(devId, state);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(ret == BT_NO_ERROR, ret,
        ::ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

    int profileState = TaiheUtils::GetProfileConnectionState(state);
    return ::ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
}

::ohos::bluetooth::ble::GattService GattServerImpl::GetService(::taihe::string_view serviceUuid)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_SERVICE_NOT_FOUND };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.GetService", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(server_ != nullptr, BT_ERR_INTERNAL_ERROR,
        (::ohos::bluetooth::ble::GattService{taihe::string(""), false,
            taihe::array<::ohos::bluetooth::ble::BLECharacteristic>{}, std::nullopt}));

    UUID uuid;
    auto status = ParseUuidParams(std::string(serviceUuid), uuid);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(status == TAIHE_OK, BT_ERR_INVALID_PARAM,
        (::ohos::bluetooth::ble::GattService{taihe::string(""), false,
            taihe::array<::ohos::bluetooth::ble::BLECharacteristic>{}, std::nullopt}));

    GattService outService(UUID::FromString("00000000-0000-0000-0000-000000000000"), GattServiceType::PRIMARY);
    int ret = server_->GetService(uuid, true, outService);
    if (ret != BT_NO_ERROR) {
        ret = server_->GetService(uuid, false, outService);
    }
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(ret == BT_NO_ERROR, ret,
        (::ohos::bluetooth::ble::GattService{taihe::string(""), false,
            taihe::array<::ohos::bluetooth::ble::BLECharacteristic>{}, std::nullopt}));

    return TaiheConvertGattService(outService);
}

void GattServerImpl::OnConnectionStateChange(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::BLEConnectionChangeState const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OnConnectionStateChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::BLEConnectionChangeState const&)>(
        STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE, callback);
}

void GattServerImpl::OffConnectionStateChange(
    ::taihe::optional_view<::taihe::callback<
        void(::ohos::bluetooth::ble::BLEConnectionChangeState const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OffConnectionStateChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::BLEConnectionChangeState const&)>(
        STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE, callback);
}

void GattServerImpl::OnDescriptorWrite(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::DescriptorWriteRequest const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OnDescriptorWrite", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::DescriptorWriteRequest const&)>(
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE, callback);
}

void GattServerImpl::OffDescriptorWrite(
    ::taihe::optional_view<::taihe::callback<
        void(::ohos::bluetooth::ble::DescriptorWriteRequest const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OffDescriptorWrite", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::DescriptorWriteRequest const&)>(
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE, callback);
}

void GattServerImpl::OnDescriptorRead(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::DescriptorReadRequest const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OnDescriptorRead", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::DescriptorReadRequest const&)>(
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ, callback);
}

void GattServerImpl::OffDescriptorRead(
    ::taihe::optional_view<::taihe::callback<
        void(::ohos::bluetooth::ble::DescriptorReadRequest const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.OffDescriptorRead", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::DescriptorReadRequest const&)>(
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ, callback);
}

void GattServerImpl::RemoveAllServices()
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.RemoveAllServices", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(server_ != nullptr, BT_ERR_INTERNAL_ERROR);
    int ret = server_->ClearServices();
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

::taihe::array<::ohos::bluetooth::ble::GattService> GattServerImpl::GetServices()
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattServer.GetServices", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(server_ != nullptr, BT_ERR_INTERNAL_ERROR,
        ::taihe::array<::ohos::bluetooth::ble::GattService>{});

    std::list<GattService> serviceList;
    int ret = server_->GetServices(serviceList);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(ret == BT_NO_ERROR, ret,
        ::taihe::array<::ohos::bluetooth::ble::GattService>{});

    std::vector<::ohos::bluetooth::ble::GattService> serviceVec;
    serviceVec.reserve(serviceList.size());
    for (auto &svc : serviceList) {
        serviceVec.push_back(TaiheConvertGattService(svc));
    }
    return ::taihe::array<::ohos::bluetooth::ble::GattService>(
        taihe::move_data, serviceVec.data(), serviceVec.size());
}

} // namespace Bluetooth
} // namespace OHOS
