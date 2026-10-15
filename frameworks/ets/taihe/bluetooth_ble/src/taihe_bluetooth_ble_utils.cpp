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
#define LOG_TAG "bt_taihe_ble_utils"
#endif

#include "taihe_bluetooth_utils.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe/array.hpp"
#include "taihe/optional.hpp"
#include "taihe/platform/ani.hpp"

namespace OHOS {
namespace Bluetooth {

int GetCurrentSdkVersion(void)
{
    int version = SDK_VERSION_20;  // default sdk version is api 20

    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOGE("fail to get system ability mgr.");
        return version;
    }
    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOGE("fail to get bundle manager proxy.");
        return version;
    }
    sptr<AppExecFwk::BundleMgrProxy> bundleMgrProxy = iface_cast<AppExecFwk::BundleMgrProxy>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        HILOGE("Failed to get bundle manager proxy.");
        return version;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto flags = AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(flags), bundleInfo);
    if (ret != ERR_OK) {
        HILOGE("GetBundleInfoForSelf: get fail.");
        return version;
    }

    version = static_cast<int>(bundleInfo.targetVersion % 100); // %100 to get the real version
    return version;
}

int GetSDKAdaptedStatusCode(int status)
{
    std::set<int> statusCodeForNewSdk = {
        BT_ERR_MAX_RESOURCES,
        BT_ERR_OPERATION_BUSY,
        BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
        BT_ERR_GATT_CONNECTION_CONGESTED,
        BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
        BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
        BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED,
        BT_ERR_BLE_ADV_DATA_EXCEED_LIMIT,
        BT_ERR_BLE_INVALID_ADV_ID,
    };
    bool isNewStatusCode = false;
    if (statusCodeForNewSdk.find(status) != statusCodeForNewSdk.end()) {
        isNewStatusCode = true;
    }
    // If sdk version is lower than SDK_VERSION_20, the status code is changed to old version.
    if (GetCurrentSdkVersion() < SDK_VERSION_20 && isNewStatusCode) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return status;
}

bool HasProperty(int properties, int propertyMask)
{
    if (properties < 0 || propertyMask < 0) {
        HILOGE("properties or propertyMask is less than 0");
        return false;
    }
    return (static_cast<unsigned int>(properties) & static_cast<unsigned int>(propertyMask)) != 0;
}

ani_object ConvertGattPropertiesToJs(ani_env *env, int properties)
{
    ani_object write = {};
    ani_boolean value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::WRITE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &write, value);

    ani_object writeNoResponse = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::WRITE_WITHOUT_RESPONSE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeNoResponse, value);

    ani_object read = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::READ));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &read, value);

    ani_object notify = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::NOTIFY));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &notify, value);

    ani_object indicate = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::INDICATE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &indicate, value);

    ani_object broadcast = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::BROADCAST));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &broadcast, value);

    ani_object authenticatedSignedWrite = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::AUTHENTICATED_SIGNED_WRITES));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &authenticatedSignedWrite, value);

    ani_object extendedProperties = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::EXTENDED_PROPERTIES));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &extendedProperties, value);

    ani_object object = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.GattPropertiesInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.GattPropertiesInfo", "<ctor>", nullptr),
        &object, write, writeNoResponse, read, notify, indicate, broadcast,
        authenticatedSignedWrite, extendedProperties);
    return object;
}

bool HasPermission(int permissions, int permissionMask)
{
    if (permissions < 0 || permissionMask < 0) {
        HILOGE("permissions or permissionMask is less than 0");
        return false;
    }
    return (static_cast<unsigned int>(permissions) & static_cast<unsigned int>(permissionMask)) != 0;
}

::ohos::bluetooth::ble::GattProperties TaiheConvertGattProperties(int properties)
{
    ::ohos::bluetooth::ble::GattProperties taiheProperties;
    taiheProperties.write.emplace(HasProperty(properties, GattCharacteristic::WRITE));
    taiheProperties.writeNoResponse.emplace(
        HasProperty(properties, GattCharacteristic::WRITE_WITHOUT_RESPONSE));
    taiheProperties.read.emplace(HasProperty(properties, GattCharacteristic::READ));
    taiheProperties.notify.emplace(HasProperty(properties, GattCharacteristic::NOTIFY));
    taiheProperties.indicate.emplace(HasProperty(properties, GattCharacteristic::INDICATE));
    taiheProperties.broadcast.emplace(HasProperty(properties, GattCharacteristic::BROADCAST));
    taiheProperties.authenticatedSignedWrite.emplace(
        HasProperty(properties, GattCharacteristic::AUTHENTICATED_SIGNED_WRITES));
    taiheProperties.extendedProperties.emplace(
        HasProperty(properties, GattCharacteristic::EXTENDED_PROPERTIES));
    return taiheProperties;
}

::ohos::bluetooth::ble::GattPermissions TaiheConvertGattPermissions(int permissions)
{
    ::ohos::bluetooth::ble::GattPermissions taihePermissions;
    taihePermissions.read.emplace(HasPermission(permissions, GattCharacteristic::READABLE));
    taihePermissions.readEncrypted.emplace(
        HasPermission(permissions, GattCharacteristic::READ_ENCRYPTED));
    taihePermissions.readEncryptedMitm.emplace(
        HasPermission(permissions, GattCharacteristic::READ_ENCRYPTED_MITM));
    taihePermissions.write.emplace(HasPermission(permissions, GattCharacteristic::WRITEABLE));
    taihePermissions.writeEncrypted.emplace(
        HasPermission(permissions, GattCharacteristic::WRITE_ENCRYPTED));
    taihePermissions.writeEncryptedMitm.emplace(
        HasPermission(permissions, GattCharacteristic::WRITE_ENCRYPTED_MITM));
    taihePermissions.writeSigned.emplace(HasPermission(permissions, GattCharacteristic::WRITE_SIGNED));
    taihePermissions.writeSignedMitm.emplace(
        HasPermission(permissions, GattCharacteristic::WRITE_SIGNED_MITM));
    return taihePermissions;
}

::ohos::bluetooth::ble::BLEDescriptor TaiheConvertGattDescriptor(const GattDescriptor &descriptor)
{
    std::string characteristicUuid;
    std::string serviceUuid;
    GattCharacteristic *characteristic = descriptor.GetCharacteristic();
    if (characteristic != nullptr) {
        characteristicUuid = characteristic->GetUuid().ToString();
        GattService *service = characteristic->GetService();
        if (service != nullptr) {
            serviceUuid = service->GetUuid().ToString();
        }
    }
    size_t valueSize = 0;
    const auto &valueData = descriptor.GetValue(&valueSize);
    ::ohos::bluetooth::ble::BLEDescriptor taiheDescriptor{
        taihe::string(serviceUuid),
        taihe::string(characteristicUuid),
        taihe::string(descriptor.GetUuid().ToString()),
        taihe::array<uint8_t>(taihe::copy_data, valueData.get(), valueSize)
    };
    taiheDescriptor.descriptorHandle.emplace(static_cast<int32_t>(descriptor.GetHandle()));
    taiheDescriptor.permissions.emplace(TaiheConvertGattPermissions(descriptor.GetPermissions()));
    return taiheDescriptor;
}

::ohos::bluetooth::ble::GattService TaiheConvertGattService(GattService &service)
{
    std::string serviceUuid = service.GetUuid().ToString();
    std::vector<GattCharacteristic> &characteristics = service.GetCharacteristics();
    std::vector<::ohos::bluetooth::ble::BLECharacteristic> charVec;
    charVec.reserve(characteristics.size());
    for (auto &character : characteristics) {
        std::vector<GattDescriptor> &descriptors = character.GetDescriptors();
        std::vector<::ohos::bluetooth::ble::BLEDescriptor> descVec;
        descVec.reserve(descriptors.size());
        for (const auto &descriptor : descriptors) {
            descVec.push_back(TaiheConvertGattDescriptor(descriptor));
        }
        size_t valueSize = 0;
        const auto &valueData = character.GetValue(&valueSize);
        ::ohos::bluetooth::ble::BLECharacteristic taiheChar{
            taihe::string(serviceUuid),
            taihe::string(character.GetUuid().ToString()),
            taihe::array<uint8_t>(taihe::copy_data, valueData.get(), valueSize),
            taihe::array<::ohos::bluetooth::ble::BLEDescriptor>(
                taihe::move_data, descVec.data(), descVec.size())
        };
        taiheChar.properties.emplace(TaiheConvertGattProperties(character.GetProperties()));
        taiheChar.characteristicValueHandle.emplace(static_cast<int32_t>(character.GetHandle()));
        taiheChar.permissions.emplace(TaiheConvertGattPermissions(character.GetPermissions()));
        charVec.push_back(std::move(taiheChar));
    }
    std::vector<::ohos::bluetooth::ble::GattService> includeVec;
    const std::vector<std::reference_wrapper<GattService>> &includedServices = service.GetIncludedServices();
    includeVec.reserve(includedServices.size());
    for (const auto &inner : includedServices) {
        includeVec.push_back(TaiheConvertGattService(inner.get()));
    }
    ::ohos::bluetooth::ble::GattService taiheService{
        taihe::string(serviceUuid),
        service.IsPrimary(),
        taihe::array<::ohos::bluetooth::ble::BLECharacteristic>(
            taihe::move_data, charVec.data(), charVec.size()),
        std::nullopt
    };
    taiheService.includeServices.emplace(taihe::array<::ohos::bluetooth::ble::GattService>(
        taihe::move_data, includeVec.data(), includeVec.size()));
    return taiheService;
}

bool IsValidPhyValue(int32_t txPhy, int32_t rxPhy, int32_t phyMode)
{
    using BlePhy = ::ohos::bluetooth::ble::BlePhy;
    using CodedPhyMode = ::ohos::bluetooth::ble::CodedPhyMode;
    constexpr int32_t BLE_PHY_MIN = BlePhy::table[0];
    constexpr int32_t BLE_PHY_MAX = BlePhy::table[static_cast<int>(BlePhy::key_t::BLE_PHY_CODED)];
    constexpr int32_t CODED_PHY_MODE_MIN = CodedPhyMode::table[0];
    constexpr int32_t CODED_PHY_MODE_MAX =
        CodedPhyMode::table[static_cast<int>(CodedPhyMode::key_t::BLE_PHY_CODED_S8)];
    if (txPhy < BLE_PHY_MIN || txPhy > BLE_PHY_MAX) {
        HILOGE("invalid txPhy: %{public}d", txPhy);
        return false;
    }
    if (rxPhy < BLE_PHY_MIN || rxPhy > BLE_PHY_MAX) {
        HILOGE("invalid rxPhy: %{public}d", rxPhy);
        return false;
    }
    // phyMode == 0 means the coded phy mode is not set, which is allowed.
    if (phyMode != 0 && (phyMode < CODED_PHY_MODE_MIN || phyMode > CODED_PHY_MODE_MAX)) {
        HILOGE("invalid phyMode: %{public}d", phyMode);
        return false;
    }
    return true;
}

ani_object ConvertGattPermissionsToJs(ani_env *env, int permissions)
{
    ani_object read = {};
    ani_boolean value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READABLE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &read, value);

    ani_object readEncrypted = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READ_ENCRYPTED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &readEncrypted, value);

    ani_object readEncryptedMitm = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READ_ENCRYPTED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &readEncryptedMitm, value);

    ani_object write = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITEABLE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &write, value);

    ani_object writeEncrypted = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_ENCRYPTED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeEncrypted, value);

    ani_object writeEncryptedMitm = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_ENCRYPTED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeEncryptedMitm, value);

    ani_object writeSigned = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_SIGNED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeSigned, value);

    ani_object writeSignedMitm = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_SIGNED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeSignedMitm, value);

    ani_object object = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.GattPermissionsInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.GattPermissionsInfo", "<ctor>", nullptr),
        &object, read, readEncrypted, readEncryptedMitm, write, writeEncrypted, writeEncryptedMitm,
        writeSigned, writeSignedMitm);
    return object;
}

ani_object ConvertBLECharacteristicToJS(ani_env *env, GattCharacteristic &characteristic)
{
    ani_string characteristicUuid = {};
    HILOGI("uuid: %{public}s", characteristic.GetUuid().ToString().c_str());
    env->String_NewUTF8(characteristic.GetUuid().ToString().c_str(),
        characteristic.GetUuid().ToString().size(), &characteristicUuid);

    ani_string serviceUuid = {};
    if (characteristic.GetService() != nullptr) {
        env->String_NewUTF8(characteristic.GetService()->GetUuid().ToString().c_str(),
            characteristic.GetService()->GetUuid().ToString().size(), &serviceUuid);
    }

    size_t valueSize = 0;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    ani_arraybuffer characteristicValue = {};
    {
        void* characteristicValueData = {};
        env->CreateArrayBuffer(valueSize,
            &characteristicValueData, &characteristicValue);
        if (memcpy_s(reinterpret_cast<uint8_t*>(characteristicValueData), valueSize, valueData, valueSize) != EOK) {
            HILOGE("memcpy_s error");
            return {};
        }
    }

    ani_ref properties = {};
    ani_object propertiesObject = ConvertGattPropertiesToJs(env, characteristic.GetProperties());
    properties = propertiesObject;

    ani_ref permissions = {};
    ani_object permissionsObject = ConvertGattPermissionsToJs(env, characteristic.GetPermissions());
    permissions = permissionsObject;

    ani_array descriptors {};
    ani_object descriptorsObject = ConvertBLEDescriptorVectorToJS(env, characteristic.GetDescriptors());
    descriptors = reinterpret_cast<ani_array>(descriptorsObject);

    ani_ref characteristicValueHandle = {};
    ani_object characteristicValueHandleObject = {};
    ani_int handle = static_cast<ani_int>(characteristic.GetHandle());
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Int"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "<ctor>", "i:"),
        &characteristicValueHandleObject, handle);
    characteristicValueHandle = characteristicValueHandleObject;

    ani_object object = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.BLECharacteristicInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.BLECharacteristicInfo", "<ctor>", nullptr),
        &object, serviceUuid, characteristicUuid, characteristicValue,
        descriptors, properties, characteristicValueHandle, permissions);
    return object;
}

ani_object ConvertBLEDescriptorToJS(ani_env *env, GattDescriptor& descriptor)
{
    HILOGI("uuid: %{public}s", descriptor.GetUuid().ToString().c_str());
    ani_string descriptorUuid = {};
    env->String_NewUTF8(descriptor.GetUuid().ToString().c_str(),
        descriptor.GetUuid().ToString().size(), &descriptorUuid);

    ani_string characteristicUuid = {};
    ani_string serviceUuid = {};
    if (descriptor.GetCharacteristic() != nullptr) {
        env->String_NewUTF8(descriptor.GetCharacteristic()->GetUuid().ToString().c_str(),
            descriptor.GetCharacteristic()->GetUuid().ToString().size(), &characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            env->String_NewUTF8(descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().size(), &serviceUuid);
        }
    }

    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    void* descriptorValueData = {};
    ani_arraybuffer descriptorValue = {};
    env->CreateArrayBuffer(valueSize * (sizeof(uint8_t) / sizeof(char)), &descriptorValueData, &descriptorValue);
    if (memcpy_s(reinterpret_cast<uint8_t*>(descriptorValueData), valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }

    ani_ref descriptorHandle = {};
    ani_object descriptorHandleObject = {};
    ani_int handle = static_cast<ani_int>(static_cast<uint32_t>(descriptor.GetHandle()));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Int"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "<ctor>", "i:"), &descriptorHandleObject, handle);
    descriptorHandle = descriptorHandleObject;

    ani_ref permissions = {};
    ani_object permissionsObject = ConvertGattPermissionsToJs(env, descriptor.GetPermissions());
    permissions = permissionsObject;

    ani_object object = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.BLEDescriptorInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.BLEDescriptorInfo", "<ctor>", nullptr),
        &object, serviceUuid, characteristicUuid, descriptorUuid, descriptorValue, descriptorHandle, permissions);
    return object;
}

ani_object ConvertBLEDescriptorVectorToJS(ani_env *env, std::vector<GattDescriptor> &descriptors)
{
    HILOGI("size: %{public}zu", descriptors.size());

    if (descriptors.empty()) {
        return nullptr;
    }

    ani_array descriptorsObject = {};
    ani_ref descriptorsNone = {};
    env->GetUndefined(&descriptorsNone);
    env->Array_New(descriptors.size(), descriptorsNone, &descriptorsObject);
    for (size_t iterator = 0; iterator < descriptors.size(); iterator++) {
        ani_object item = ConvertBLEDescriptorToJS(env, descriptors[iterator]);
        env->Array_Set(descriptorsObject, iterator, item);
    }
    return descriptorsObject;
}
}  // namespace Bluetooth
}  // namespace OHOS