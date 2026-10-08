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
#define LOG_TAG "bt_taihe_gatt_client"
#endif

#include "taihe_bluetooth_gatt_client.h"

#include "bluetooth_host.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_ha_event_utils.h"
#include "taihe_native_object.h"
#include "taihe_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
constexpr int32_t GATT_DEF_BLE_MTU_SIZE = 23;
constexpr int32_t GATT_MAX_MTU_SIZE = 517;
const std::vector<std::pair<int, int>> GattClientDeviceImpl::g_gattStatusSrvToTaihe = {
    { Bluetooth::BT_NO_ERROR,                                 GATT_SUCCESS },
    { Bluetooth::BT_ERR_GATT_WRITE_NOT_PERMITTED,             WRITE_NOT_PERMITTED },
    { Bluetooth::BT_ERR_GATT_READ_NOT_PERMITTED,              READ_NOT_PERMITTED },
    { Bluetooth::BT_ERR_GATT_CONNECTION_CONGESTED,            GATT_CONGESTION },
    { Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,        INSUFFICIENT_ENCRYPTION },
    { Bluetooth::BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,    AUTHENTICATION_FAILED },
    { Bluetooth::BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED,       INSUFFICIENT_AUTHORIZATION },
};
void GattClientDeviceImpl::SetBLEMtuSize(int mtu)
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client_->RequestBleMtuSize(mtu);
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}
void GattClientDeviceImpl::Connect()
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client_->Connect(callback_, false, GATT_TRANSPORT_TYPE_LE);
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

void GattClientDeviceImpl::Disconnect()
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client_->Disconnect();
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

void GattClientDeviceImpl::Close()
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
    int ret = client_->Close();
    HILOGI("ret: %{public}d", ret);

    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

taihe::string GattClientDeviceImpl::GetDeviceNameSync()
{
    HILOGD("start");
    std::string deviceAddr = "";
    if (GetDevice()) {
        deviceAddr = GetDevice()->GetDeviceAddr();
    }

    std::string deviceName = "";
    int32_t err = BluetoothHost::GetDefaultHost().GetRemoteDevice(
        deviceAddr, BT_TRANSPORT_BLE).GetDeviceName(deviceName);
    HILOGI("err: %{public}d, deviceName: %{private}s", err, deviceName.c_str());
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, deviceName);

    return deviceName;
}

static GattCharacteristic *GetCharacteristic(const std::shared_ptr<GattClient> &client,
    const UUID &serviceUuid, const UUID &characterUuid)
{
    GattCharacteristic *character = nullptr;
    if (client) {
        auto service = client->GetService(serviceUuid);
        if (service.has_value()) {
            character = service->get().GetCharacteristic(characterUuid);
        }
    }
    return character;
}

static GattCharacteristic *FindCharacteristic(std::vector<GattService> &service,
    const TaiheBleCharacteristic &taiheCharacter)
{
    GattCharacteristic *character = nullptr;
    for (auto &svc : service) {
        if (svc.GetUuid().Equals(taiheCharacter.serviceUuid)) {
            character = svc.GetCharacteristic(taiheCharacter.characteristicValueHandle);
            if (character && character->GetUuid().Equals(taiheCharacter.characteristicUuid)) {
                return character;
            }
        }
    }
    return nullptr;
}

static GattCharacteristic *GetCharacteristic(const std::shared_ptr<GattClient> &client,
    const TaiheBleCharacteristic &taiheCharacter)
{
    if (client) {
        if (taiheCharacter.characteristicValueHandle > 0) {
            std::vector<GattService> &services = client->GetService();
            return FindCharacteristic(services, taiheCharacter);
        } else {
            GattCharacteristic *character = GetCharacteristic(client, taiheCharacter.serviceUuid,
                taiheCharacter.characteristicUuid);
            return character;
        }
    }
    return nullptr;
}

static GattCharacteristic *GetGattcCharacteristic(const std::shared_ptr<GattClient> &client,
    const TaiheBleCharacteristic &taiheCharacter)
{
    GattCharacteristic *character = GetCharacteristic(client, taiheCharacter);
    if (character) {
        character->SetValue(taiheCharacter.characteristicValue.data(), taiheCharacter.characteristicValue.size());
    }
    return character;
}

static GattDescriptor *GetGattcDescriptor(const std::shared_ptr<GattClient> &client,
    const TaiheBleDescriptor &taiheDescriptor)
{
    GattDescriptor *descriptor = nullptr;
    if (client) {
        auto *character = GetCharacteristic(client, taiheDescriptor.serviceUuid, taiheDescriptor.characteristicUuid);
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return nullptr;
        }
        descriptor = character->GetDescriptor(taiheDescriptor.descriptorUuid);
        if (descriptor) {
            descriptor->SetValue(taiheDescriptor.descriptorValue.data(), taiheDescriptor.descriptorValue.size());
        }
    }
    return descriptor;
}

static TaiheStatus ParseGattClientReadCharacteristicValue(
    const ohos::bluetooth::ble::BLECharacteristic &characteristic,
    GattClientDeviceImpl *gattClient, GattCharacteristic **outCharacter)
{
    TAIHE_BT_RETURN_IF(gattClient == nullptr, "gattClient is nullptr.", TAIHE_INVALID_ARG);

    TaiheBleCharacteristic taiheCharacter;
    TAIHE_BT_CALL_RETURN(TaiheParseGattCharacteristic(characteristic, taiheCharacter));

    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), taiheCharacter);
    TAIHE_BT_RETURN_IF(character == nullptr || outCharacter == nullptr, "Not found character", TAIHE_INVALID_ARG);

    *outCharacter = character;
    return TAIHE_OK;
}

static void TaiheGattClientAsyncError(uintptr_t callback, int32_t errCode)
{
    ani_env *env = taihe::get_env();
    if (env == nullptr) {
        HILOGE("null env");
        return;
    }
    ani_boolean isExist = false;
    if (env->ExistUnhandledError(&isExist) == ANI_OK && isExist) {
        HILOGI("clear pending error before callback delivery");
        ani_status status = env->ResetError();
        if (status != ANI_OK) {
            HILOGE("ResetError failed, status: %{public}d", status);
        }
    }
    auto asyncCallback = TaiheParseAsyncCallback(env, reinterpret_cast<ani_object>(callback));
    if (asyncCallback == nullptr) {
        HILOGE("asyncCallback is nullptr");
        return;
    }
    asyncCallback->CallFunction(errCode, std::make_shared<TaiheNativeUndefined>());
}

static TaihePromiseAndCallback TaiheReadCharacteristicValue(
    const ohos::bluetooth::ble::BLECharacteristic &characteristic, uintptr_t cb,
    GattClientDeviceImpl *client, bool isPromise = true)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_READ_NOT_PERMITTED, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
                                           BT_ERR_GATT_CONNECTION_CONGESTED, BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.ReadCharacteristicValue", validErrCodes);
    GattCharacteristic *character = nullptr;
    auto status = ParseGattClientReadCharacteristicValue(characteristic, client, &character);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(status == TAIHE_OK && client && character, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    auto func = [gattClient = client->GetClient(), character]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadCharacteristic(*character);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(
        env, reinterpret_cast<ani_object>(cb), func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_READ_CHARACTER, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    asyncWork->Run();

    return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(isPromise ? asyncWork->GetRet() : nullptr));
}

uintptr_t GattClientDeviceImpl::ReadCharacteristicValuePromise(
    const ohos::bluetooth::ble::BLECharacteristic &characteristic)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR, BT_ERR_GATT_READ_NOT_PERMITTED,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED, BT_ERR_GATT_CONNECTION_CONGESTED,
                                           BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TaihePromiseAndCallback result = TaiheReadCharacteristicValue(characteristic, reinterpret_cast<uintptr_t>(nullptr),
        this);
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN_VERIFY(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

void GattClientDeviceImpl::ReadCharacteristicValueAsync(const ohos::bluetooth::ble::BLECharacteristic &characteristic,
    uintptr_t callback)
{
    TaihePromiseAndCallback result = TaiheReadCharacteristicValue(characteristic, callback, this, false);
    if (!result.success) {
        TaiheGattClientAsyncError(callback, result.errorCode);
    }
}

static TaiheStatus ParseGattClientReadDescriptorValue(const ohos::bluetooth::ble::BLEDescriptor &bleDescriptor,
    GattClientDeviceImpl *gattClient, GattDescriptor **outDescriptor)
{
    TAIHE_BT_RETURN_IF(gattClient == nullptr, "gattClient is nullptr.", TAIHE_INVALID_ARG);

    TaiheBleDescriptor taiheDescriptor;
    TAIHE_BT_CALL_RETURN(TaiheParseGattDescriptor(bleDescriptor, taiheDescriptor));
    GattDescriptor *descriptor = GetGattcDescriptor(gattClient->GetClient(), taiheDescriptor);
    TAIHE_BT_RETURN_IF(outDescriptor == nullptr || descriptor == nullptr, "Not found Descriptor", TAIHE_INVALID_ARG);

    *outDescriptor = descriptor;
    return TAIHE_OK;
}

static TaihePromiseAndCallback TaiheReadDescriptorValue(const ohos::bluetooth::ble::BLEDescriptor &bleDescriptor,
    uintptr_t cb, GattClientDeviceImpl *client, bool isPromise = true)
{
    HILOGI("enter");
    GattDescriptor *descriptor = nullptr;
    auto status = ParseGattClientReadDescriptorValue(bleDescriptor, client, &descriptor);
    TAIHE_BT_ASSERT_RETURN(status == TAIHE_OK && client && descriptor, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));
    TAIHE_BT_ASSERT_RETURN(client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    auto func = [gattClient = client->GetClient(), descriptor]() {
        if (descriptor == nullptr) {
            HILOGE("descriptor is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadDescriptor(*descriptor);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(taihe::get_env(), reinterpret_cast<ani_object>(cb), func);
    TAIHE_BT_ASSERT_RETURN(asyncWork, BT_ERR_INTERNAL_ERROR, TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_READ_DESCRIPTOR, asyncWork);
    TAIHE_BT_ASSERT_RETURN(success, BT_ERR_INTERNAL_ERROR, TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    asyncWork->Run();

    return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(isPromise ? asyncWork->GetRet() : nullptr));
}

uintptr_t GattClientDeviceImpl::ReadDescriptorValuePromise(const ohos::bluetooth::ble::BLEDescriptor &bleDescriptor)
{
    TaihePromiseAndCallback result =
        TaiheReadDescriptorValue(bleDescriptor, reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

void GattClientDeviceImpl::ReadDescriptorValueAsync(const ohos::bluetooth::ble::BLEDescriptor &bleDescriptor,
    uintptr_t callback)
{
    TaihePromiseAndCallback result = TaiheReadDescriptorValue(bleDescriptor, callback, this, false);
    if (!result.success) {
        TaiheGattClientAsyncError(callback, result.errorCode);
    }
}

static TaiheStatus CheckSetCharacteristicChange(const ohos::bluetooth::ble::BLECharacteristic &characteristic,
    GattCharacteristic **outCharacteristic, GattClientDeviceImpl *gattClient)
{
    TAIHE_BT_RETURN_IF(gattClient == nullptr, "gattClient is nullptr.", TAIHE_INVALID_ARG);

    TaiheBleCharacteristic taiheCharacter;
    TAIHE_BT_CALL_RETURN(TaiheParseGattCharacteristic(characteristic, taiheCharacter));
    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), taiheCharacter);
    TAIHE_BT_RETURN_IF(character == nullptr || outCharacteristic == nullptr, "Not found character", TAIHE_INVALID_ARG);

    *outCharacteristic = character;
    return TAIHE_OK;
}

static TaihePromiseAndCallback TaiheSetCharacteristicChangeIndication(
    const ohos::bluetooth::ble::BLECharacteristic &characteristic, bool enable, uintptr_t cb,
    GattClientDeviceImpl *client, bool isPromise = true)
{
    HILOGI("enter");
    GattCharacteristic *character = nullptr;

    auto status = CheckSetCharacteristicChange(characteristic, &character, client);
    TAIHE_BT_ASSERT_RETURN(status == TAIHE_OK && client && character, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));
    TAIHE_BT_ASSERT_RETURN(client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    auto func = [gattClient = client->GetClient(), character, enable]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->SetIndicateCharacteristic(*character, enable);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(taihe::get_env(), reinterpret_cast<ani_object>(cb), func);
    TAIHE_BT_ASSERT_RETURN(asyncWork, BT_ERR_INTERNAL_ERROR, TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(GATT_CLIENT_ENABLE_CHARACTER_CHANGED, asyncWork);
    TAIHE_BT_ASSERT_RETURN(success, BT_ERR_INTERNAL_ERROR, TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    asyncWork->Run();

    return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(isPromise ? asyncWork->GetRet() : nullptr));
}

uintptr_t GattClientDeviceImpl::SetCharacteristicChangeIndicationPromise(
    const ohos::bluetooth::ble::BLECharacteristic &characteristic, bool enable)
{
    TaihePromiseAndCallback result = TaiheSetCharacteristicChangeIndication(characteristic, enable,
        reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

void GattClientDeviceImpl::SetCharacteristicChangeIndicationAsync(
    const ohos::bluetooth::ble::BLECharacteristic &characteristic, bool enable, uintptr_t callback)
{
    TaihePromiseAndCallback result = TaiheSetCharacteristicChangeIndication(characteristic, enable, callback,
        this, false);
    if (!result.success) {
        TaiheGattClientAsyncError(callback, result.errorCode);
    }
}

int GattClientDeviceImpl::GattStatusFromService(int status)
{
    if (status > 0) {
        return status;
    }
    int ret = BT_ERR_INTERNAL_ERROR;
    auto iter = g_gattStatusSrvToTaihe.begin();
    for (; iter != g_gattStatusSrvToTaihe.end(); iter++) {
        if (iter->second == status) {
            ret = iter->first;
            break;
        }
    }
    if (iter == g_gattStatusSrvToTaihe.end()) {
        HILOGW("Unsupported error code conversion, status: %{public}d", status);
    }
    return ret;
}

uintptr_t GattClientDeviceImpl::ReadPhyPromise()
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.ReadPhy", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));

    auto func = [gattClient = client_]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadPhy();
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_READ_PHY, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();

    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

uintptr_t GattClientDeviceImpl::SetPhyPromise(::ohos::bluetooth::ble::PhyValue const& phyValue)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.SetPhy", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    int32_t phyMode = phyValue.phyMode.has_value() ? static_cast<int32_t>(phyValue.phyMode.value()) : 0;
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(
        IsValidPhyValue(static_cast<int32_t>(phyValue.txPhy), static_cast<int32_t>(phyValue.rxPhy), phyMode),
        BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));

    auto func = [gattClient = client_, phyValue]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->SetPhy(phyValue.txPhy, phyValue.rxPhy,
                phyValue.phyMode.has_value() ? phyValue.phyMode.value() : 0);
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_SET_PHY, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();

    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

// Converts the interface-domain GattWriteType (WRITE = 1 / WRITE_NO_RESPONSE = 2)
// to the protocol-stack domain (DEFAULT = 0 / NO_RESPONSE = 1). Any other value
// is rejected as BT_ERR_INVALID_PARAM.
static bool TaiheConvertGattWriteType(::ohos::bluetooth::ble::GattWriteType writeType, int &convertedWriteType)
{
    switch (static_cast<int>(writeType)) {
        case static_cast<int>(::ohos::bluetooth::ble::GattWriteType::key_t::WRITE):
            convertedWriteType = static_cast<int>(GattCharacteristic::WriteType::DEFAULT);
            return true;
        case static_cast<int>(::ohos::bluetooth::ble::GattWriteType::key_t::WRITE_NO_RESPONSE):
            convertedWriteType = static_cast<int>(GattCharacteristic::WriteType::NO_RESPONSE);
            return true;
        default:
            return false;
    }
}

static GattCharacteristic *ParseGattClientWriteCharacteristic(
    const ::ohos::bluetooth::ble::BLECharacteristic &characteristic,
    ::ohos::bluetooth::ble::GattWriteType writeType, GattClientDeviceImpl *client)
{
    TaiheBleCharacteristic taiheCharacter;
    if (TaiheParseGattCharacteristic(characteristic, taiheCharacter) != TAIHE_OK) {
        HILOGE("parse characteristic failed");
        return nullptr;
    }
    GattCharacteristic *character = GetGattcCharacteristic(client->GetClient(), taiheCharacter);
    if (character == nullptr) {
        HILOGE("not found character");
        return nullptr;
    }
    int convertedWriteType = 0;
    if (!TaiheConvertGattWriteType(writeType, convertedWriteType)) {
        HILOGE("invalid writeType: %{public}d", static_cast<int>(writeType));
        return nullptr;
    }
    character->SetWriteType(convertedWriteType);
    return character;
}

struct GattClientWriteWorkParams {
    GattClientDeviceImpl *client;
    GattCharacteristic *character;
    uintptr_t cb;
    TaiheAsyncType asyncType;
    bool withContext;
    bool isPromise;
};

static TaihePromiseAndCallback TaiheCreateGattClientWriteWork(
    const GattClientWriteWorkParams &params, const ApiContext &apiContext)
{
    ani_env *env = taihe::get_env();
    auto func = [gattClient = params.client->GetClient(), character = params.character,
                 withContext = params.withContext]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_GATT_CHARACTER_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->WriteCharacteristic(*character, withContext);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret));
        }
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(
        env, reinterpret_cast<ani_object>(params.cb), func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    // GattCharacteristic write need callback, write no response is not needed.
    bool isNeedCallback =
        params.character->GetWriteType() == static_cast<int>(GattCharacteristic::WriteType::DEFAULT);
    asyncWork->SetNeedCallback(isNeedCallback);
    if (isNeedCallback) {
        bool success = params.client->GetCallback()->asyncWorkMap_.TryPush(params.asyncType, asyncWork);
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR,
            TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    }

    asyncWork->Run();

    return TaihePromiseAndCallback::Success(
        reinterpret_cast<uintptr_t>(params.isPromise ? asyncWork->GetRet() : nullptr));
}

static TaihePromiseAndCallback TaiheWriteCharacteristicValueWithContext(
    const ::ohos::bluetooth::ble::BLECharacteristic &characteristic,
    ::ohos::bluetooth::ble::GattWriteType writeType, uintptr_t cb,
    GattClientDeviceImpl *client, bool isPromise = true)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_WRITE_NOT_PERMITTED, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
                                           BT_ERR_GATT_CONNECTION_CONGESTED, BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.WriteCharacteristicValueWithContext", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client != nullptr && client->GetCallback() != nullptr,
        BT_ERR_INTERNAL_ERROR, TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    GattCharacteristic *character = ParseGattClientWriteCharacteristic(characteristic, writeType, client);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(character != nullptr, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));

    GattClientWriteWorkParams params = { client, character, cb,
        TaiheAsyncType::GATT_CLIENT_WRITE_CHARACTER_WITH_CONTEXT, true, isPromise };
    return TaiheCreateGattClientWriteWork(params, apiContext);
}

uintptr_t GattClientDeviceImpl::WriteCharacteristicValueWithContextPromise(
    ::ohos::bluetooth::ble::BLECharacteristic const& characteristic,
    ::ohos::bluetooth::ble::GattWriteType writeType)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_WRITE_NOT_PERMITTED, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
                                           BT_ERR_GATT_CONNECTION_CONGESTED, BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TaihePromiseAndCallback result =
        TaiheWriteCharacteristicValueWithContext(characteristic, writeType, reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN_VERIFY(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

static TaihePromiseAndCallback TaiheWriteDescriptorValue(
    const ::ohos::bluetooth::ble::BLEDescriptor &descriptor, uintptr_t cb,
    GattClientDeviceImpl *client, bool isPromise = true)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_WRITE_NOT_PERMITTED, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
                                           BT_ERR_GATT_CONNECTION_CONGESTED, BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.WriteDescriptorValue", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client != nullptr, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    TaiheBleDescriptor taiheDescriptor;
    TaiheStatus parseStatus = TaiheParseGattDescriptor(descriptor, taiheDescriptor);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(parseStatus == TAIHE_OK, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));
    GattDescriptor *desc = GetGattcDescriptor(client->GetClient(), taiheDescriptor);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(desc != nullptr, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));

    auto func = [gattClient = client->GetClient(), desc]() {
        if (desc == nullptr) {
            HILOGE("descriptor is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->WriteDescriptor(*desc);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret));
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(
        env, reinterpret_cast<ani_object>(cb), func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_WRITE_DESCRIPTOR,
        asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    asyncWork->Run();

    return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(isPromise ? asyncWork->GetRet() : nullptr));
}

uintptr_t GattClientDeviceImpl::WriteDescriptorValuePromise(
    ::ohos::bluetooth::ble::BLEDescriptor const& descriptor)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_WRITE_NOT_PERMITTED, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
                                           BT_ERR_GATT_CONNECTION_CONGESTED, BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TaihePromiseAndCallback result =
        TaiheWriteDescriptorValue(descriptor, reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN_VERIFY(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

void GattClientDeviceImpl::WriteDescriptorValueAsync(
    ::ohos::bluetooth::ble::BLEDescriptor const& descriptor, uintptr_t callback)
{
    HILOGI("enter");
    TaihePromiseAndCallback result = TaiheWriteDescriptorValue(descriptor, callback, this, false);
    if (!result.success) {
        TaiheGattClientAsyncError(callback, result.errorCode);
    }
}

::taihe::array<::ohos::bluetooth::ble::GattService> GattClientDeviceImpl::GetServicesSync()
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.GetServices", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR,
        ::taihe::array<::ohos::bluetooth::ble::GattService>{});

    int ret = client_->DiscoverServices();
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(ret == BT_NO_ERROR, ret,
        ::taihe::array<::ohos::bluetooth::ble::GattService>{});

    std::vector<GattService> services = client_->GetService();
    std::vector<::ohos::bluetooth::ble::GattService> serviceVec;
    serviceVec.reserve(services.size());
    for (auto &svc : services) {
        serviceVec.push_back(TaiheConvertGattService(svc));
    }
    return ::taihe::array<::ohos::bluetooth::ble::GattService>(
        taihe::move_data, serviceVec.data(), serviceVec.size());
}

void GattClientDeviceImpl::OnBlePhyUpdate(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::PhyValue const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OnBlePhyUpdate", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::PhyValue const&)>(
        STR_BT_GATT_CLIENT_CALLBACK_BLE_PHY_UPDATE, callback);
}

void GattClientDeviceImpl::OffBlePhyUpdate(
    ::taihe::optional_view<::taihe::callback<void(::ohos::bluetooth::ble::PhyValue const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OffBlePhyUpdate", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::PhyValue const&)>(
        STR_BT_GATT_CLIENT_CALLBACK_BLE_PHY_UPDATE, callback);
}

static TaihePromiseAndCallback TaiheSetCharacteristicChangeNotification(
    const ::ohos::bluetooth::ble::BLECharacteristic &characteristic, bool enable, uintptr_t cb,
    GattClientDeviceImpl *client, bool isPromise = true)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.SetCharacteristicChangeNotification", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client != nullptr, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    TaiheBleCharacteristic taiheCharacter;
    TaiheStatus parseStatus = TaiheParseGattCharacteristic(characteristic, taiheCharacter);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(parseStatus == TAIHE_OK, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));
    GattCharacteristic *character = GetGattcCharacteristic(client->GetClient(), taiheCharacter);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(character != nullptr, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));

    auto func = [gattClient = client->GetClient(), character, enable]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->SetNotifyCharacteristic(*character, enable);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret));
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(
        env, reinterpret_cast<ani_object>(cb), func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(
        TaiheAsyncType::GATT_CLIENT_ENABLE_CHARACTER_CHANGED, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR,
        TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));

    asyncWork->Run();

    return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(isPromise ? asyncWork->GetRet() : nullptr));
}

uintptr_t GattClientDeviceImpl::SetCharacteristicChangeNotificationPromise(
    ::ohos::bluetooth::ble::BLECharacteristic const& characteristic, bool enable)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TaihePromiseAndCallback result =
        TaiheSetCharacteristicChangeNotification(characteristic, enable, reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN_VERIFY(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

void GattClientDeviceImpl::SetCharacteristicChangeNotificationAsync(
    ::ohos::bluetooth::ble::BLECharacteristic const& characteristic, bool enable, uintptr_t callback)
{
    HILOGI("enter");
    TaihePromiseAndCallback result = TaiheSetCharacteristicChangeNotification(characteristic, enable, callback,
        this, false);
    if (!result.success) {
        TaiheGattClientAsyncError(callback, result.errorCode);
    }
}

uintptr_t GattClientDeviceImpl::GetRssiValuePromise()
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.GetRssiValue", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));

    auto func = [gattClient = client_]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadRemoteRssiValue();
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_READ_REMOTE_RSSI_VALUE, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();

    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

void GattClientDeviceImpl::GetRssiValueAsync(uintptr_t callback)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.GetRssiValue", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);

    auto func = [gattClient = client_]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadRemoteRssiValue();
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(
        env, reinterpret_cast<ani_object>(callback), func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR);
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_READ_REMOTE_RSSI_VALUE, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
}

uintptr_t GattClientDeviceImpl::SetBLEMtuPromise(int32_t mtu)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.SetBLEMtu", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(mtu >= GATT_DEF_BLE_MTU_SIZE && mtu <= GATT_MAX_MTU_SIZE,
        BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));

    auto func = [gattClient = client_, mtu]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->RequestBleMtuSize(mtu);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_MTU_CHANGED, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();

    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

void GattClientDeviceImpl::SetBLEMtuAsync(int32_t mtu, uintptr_t callback)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.SetBLEMtu", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(mtu >= GATT_DEF_BLE_MTU_SIZE && mtu <= GATT_MAX_MTU_SIZE,
        BT_ERR_INVALID_PARAM);

    auto func = [gattClient = client_, mtu]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->RequestBleMtuSize(mtu);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(
        env, reinterpret_cast<ani_object>(callback), func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR);
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_MTU_CHANGED, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
}

static TaihePromiseAndCallback TaiheWriteCharacteristicValue(
    const ::ohos::bluetooth::ble::BLECharacteristic &characteristic,
    const ::ohos::bluetooth::ble::GattWriteType writeType, uintptr_t cb,
    GattClientDeviceImpl *client, bool isPromise = true)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_WRITE_NOT_PERMITTED, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
                                           BT_ERR_GATT_CONNECTION_CONGESTED, BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.WriteCharacteristicValue", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client != nullptr && client->GetCallback() != nullptr,
        BT_ERR_INTERNAL_ERROR, TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR));
    GattCharacteristic *character = ParseGattClientWriteCharacteristic(characteristic, writeType, client);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(character != nullptr, BT_ERR_INVALID_PARAM,
        TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM));

    GattClientWriteWorkParams params = { client, character, cb,
        TaiheAsyncType::GATT_CLIENT_WRITE_CHARACTER, false, isPromise };
    return TaiheCreateGattClientWriteWork(params, apiContext);
}

uintptr_t GattClientDeviceImpl::WriteCharacteristicValuePromise(
    ::ohos::bluetooth::ble::BLECharacteristic const& characteristic,
    ::ohos::bluetooth::ble::GattWriteType writeType)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_OPERATION_BUSY, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_WRITE_NOT_PERMITTED, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
                                           BT_ERR_GATT_CONNECTION_CONGESTED, BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
                                           BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    TaihePromiseAndCallback result =
        TaiheWriteCharacteristicValue(characteristic, writeType, reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.object.has_value()) {
        TAIHE_BT_ASSERT_RETURN_VERIFY(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.object.value();
}

void GattClientDeviceImpl::WriteCharacteristicValueAsync(
    ::ohos::bluetooth::ble::BLECharacteristic const& characteristic,
    ::ohos::bluetooth::ble::GattWriteType writeType, uintptr_t callback)
{
    HILOGI("enter");
    TaihePromiseAndCallback result = TaiheWriteCharacteristicValue(characteristic, writeType, callback,
        this, false);
    if (!result.success) {
        TaiheGattClientAsyncError(callback, result.errorCode);
    }
}

void GattClientDeviceImpl::OnBLEMtuChange(::taihe::callback_view<void(int32_t data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OnBLEMtuChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(int32_t)>(
        STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE, callback);
}

void GattClientDeviceImpl::OffBLEMtuChange(
    ::taihe::optional_view<::taihe::callback<void(int32_t data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OffBLEMtuChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(int32_t)>(
        STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE, callback);
}

void GattClientDeviceImpl::OnServiceChange(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::UndefinedType const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OnServiceChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::UndefinedType const&)>(
        STR_BT_GATT_CLIENT_CALLBACK_SERVICE_CHANGE, callback);
}

void GattClientDeviceImpl::OffServiceChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::bluetooth::ble::UndefinedType const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OffServiceChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::UndefinedType const&)>(
        STR_BT_GATT_CLIENT_CALLBACK_SERVICE_CHANGE, callback);
}

void GattClientDeviceImpl::OnBLECharacteristicChange(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::BLECharacteristic const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OnBLECharacteristicChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::BLECharacteristic const&)>(
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE, callback);
}

void GattClientDeviceImpl::OffBLECharacteristicChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::bluetooth::ble::BLECharacteristic const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OffBLECharacteristicChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::BLECharacteristic const&)>(
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE, callback);
}

uintptr_t GattClientDeviceImpl::UpdateConnectionParamPromise(::ohos::bluetooth::ble::ConnectionParam param)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.UpdateConnectionParam", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));

    int connectionPriority = static_cast<int>(GattConnectionPriority::BALANCED);
    switch (param.get_key()) {
        case ::ohos::bluetooth::ble::ConnectionParam::key_t::LOW_POWER:
            connectionPriority = static_cast<int>(GattConnectionPriority::LOW_POWER);
            break;
        case ::ohos::bluetooth::ble::ConnectionParam::key_t::BALANCED:
            connectionPriority = static_cast<int>(GattConnectionPriority::BALANCED);
            break;
        case ::ohos::bluetooth::ble::ConnectionParam::key_t::HIGH:
            connectionPriority = static_cast<int>(GattConnectionPriority::HIGH);
            break;
        default:
            TAIHE_BT_HA_ASSERT_RETURN_VERIFY(false, BT_ERR_INVALID_PARAM,
                reinterpret_cast<uintptr_t>(nullptr));
            break;
    }

    auto func = [gattClient = client_, connectionPriority]() {
        int ret = gattClient->RequestConnectionPriority(connectionPriority);
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_UPDATE_CONNECTION_PRIORITY, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();

    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

void GattClientDeviceImpl::UpdateConnectionParamAsync(::ohos::bluetooth::ble::ConnectionParam param,
    uintptr_t callback)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                           BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED };
    TAIHE_BT_CONTEXT_HA(env, "ble.GattClientDevice.UpdateConnectionParam", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);

    int connectionPriority = static_cast<int>(GattConnectionPriority::BALANCED);
    switch (param.get_key()) {
        case ::ohos::bluetooth::ble::ConnectionParam::key_t::LOW_POWER:
            connectionPriority = static_cast<int>(GattConnectionPriority::LOW_POWER);
            break;
        case ::ohos::bluetooth::ble::ConnectionParam::key_t::BALANCED:
            connectionPriority = static_cast<int>(GattConnectionPriority::BALANCED);
            break;
        case ::ohos::bluetooth::ble::ConnectionParam::key_t::HIGH:
            connectionPriority = static_cast<int>(GattConnectionPriority::HIGH);
            break;
        default:
            TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(false, BT_ERR_INVALID_PARAM);
            break;
    }

    auto func = [gattClient = client_, connectionPriority]() {
        int ret = gattClient->RequestConnectionPriority(connectionPriority);
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(
        env, reinterpret_cast<ani_object>(callback), func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR);
    bool success = callback_->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_UPDATE_CONNECTION_PRIORITY, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
}

void GattClientDeviceImpl::OnBLEConnectionStateChange(
    ::taihe::callback_view<void(::ohos::bluetooth::ble::BLEConnectionChangeState const& data)> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OnBLEConnectionStateChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.RegisterEvent<void(::ohos::bluetooth::ble::BLEConnectionChangeState const&)>(
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTION_STATE_CHANGE, callback);
}

void GattClientDeviceImpl::OffBLEConnectionStateChange(
    ::taihe::optional_view<::taihe::callback<void(
    ::ohos::bluetooth::ble::BLEConnectionChangeState const& data)>> callback)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.OffBLEConnectionStateChange", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);
    callback_->eventSubscribe_.DeregisterEvent<void(::ohos::bluetooth::ble::BLEConnectionChangeState const&)>(
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTION_STATE_CHANGE, callback);
}

::ohos::bluetooth::constant::ProfileConnectionState GattClientDeviceImpl::GetConnectedState()
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "ble.GattClientDevice.GetConnectedState", validErrCodes);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client_ != nullptr, BT_ERR_INTERNAL_ERROR,
        ::ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

    int state = 0;
    int ret = client_->GetConnectedState(state);
    HILOGI("ret: %{public}d, state: %{public}d", ret, state);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(ret == BT_NO_ERROR, ret,
        ::ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

    int profileState = TaiheUtils::GetProfileConnectionState(state);
    return ::ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
}

} // namespace Bluetooth
} // namespace OHOS
