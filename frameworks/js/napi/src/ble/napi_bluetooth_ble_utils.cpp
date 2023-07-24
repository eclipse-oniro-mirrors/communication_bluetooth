/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_ble_utils.h"
#include <algorithm>
#include <functional>
#include <optional>
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_bluetooth_error.h"
#include "securec.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;
void ConvertGattServiceToJS(napi_env env, napi_value result, GattService& service)
{
    napi_value serviceUuid;
    napi_create_string_utf8(env, service.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &serviceUuid);
    napi_set_named_property(env, result, "serviceUuid", serviceUuid);

    napi_value isPrimary;
    napi_get_boolean(env, service.IsPrimary(), &isPrimary);
    napi_set_named_property(env, result, "isPrimary", isPrimary);
    HILOGI("uuid: %{public}s, isPrimary: %{public}d", service.GetUuid().ToString().c_str(), service.IsPrimary());

    napi_value characteristics;
    napi_create_array(env, &characteristics);
    ConvertBLECharacteristicVectorToJS(env, characteristics, service.GetCharacteristics());
    napi_set_named_property(env, result, "characteristics", characteristics);

    napi_value includedServices;
    napi_create_array(env, &includedServices);
    vector<GattService> services;
    vector<std::reference_wrapper<GattService>> srvs = service.GetIncludedServices();
    for (auto &srv : srvs) {
        services.push_back(srv.get());
    }
    ConvertGattServiceVectorToJS(env, includedServices, services);
    napi_set_named_property(env, result, "includedServices", includedServices);
}

void ConvertGattServiceVectorToJS(napi_env env, napi_value result, vector<GattService>& services)
{
    HILOGI("enter");
    size_t idx = 0;

    if (services.empty()) {
        return;
    }
    HILOGI("size: %{public}zu", services.size());
    for (auto& service : services) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        ConvertGattServiceToJS(env, obj, service);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

void ConvertBLECharacteristicVectorToJS(napi_env env, napi_value result,
    vector<GattCharacteristic>& characteristics)
{
    HILOGI("size: %{public}zu", characteristics.size());
    size_t idx = 0;
    if (characteristics.empty()) {
        return;
    }

    for (auto &characteristic : characteristics) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        ConvertBLECharacteristicToJS(env, obj, characteristic);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

bool HasProperty(int properties, int propertyMask)
{
    return (properties & propertyMask) != 0;
}
napi_value ConvertGattPropertiesToJs(napi_env env, int properties)
{
    napi_value object;
    napi_create_object(env, &object);

    napi_value value;
    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::WRITE), &value);
    napi_set_named_property(env, object, "write", value);

    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::WRITE_WITHOUT_RESPONSE), &value);
    napi_set_named_property(env, object, "writeNoResponse", value);

    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::READ), &value);
    napi_set_named_property(env, object, "read", value);

    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::NOTIFY), &value);
    napi_set_named_property(env, object, "notify", value);

    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::INDICATE), &value);
    napi_set_named_property(env, object, "indicate", value);
    return object;
}

void ConvertBLECharacteristicToJS(napi_env env, napi_value result, GattCharacteristic& characteristic)
{
    napi_value characteristicUuid;
    HILOGI("uuid: %{public}s", characteristic.GetUuid().ToString().c_str());
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
    }

    size_t valueSize = 0;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    {
        napi_value value = nullptr;
        uint8_t* bufferData = nullptr;
        napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
        if (valueSize > 0 && memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
            HILOGE("memcpy_s failed");
            return;
        }
        napi_set_named_property(env, result, "characteristicValue", value);
    }

    napi_value propertiesValue = ConvertGattPropertiesToJs(env, characteristic.GetProperties());
    napi_set_named_property(env, result, "properties", propertiesValue);

    napi_value descriptors;
    napi_create_array(env, &descriptors);
    ConvertBLEDescriptorVectorToJS(env, descriptors, characteristic.GetDescriptors());
    napi_set_named_property(env, result, "descriptors", descriptors);
}

void ConvertBLEDescriptorVectorToJS(napi_env env, napi_value result, vector<GattDescriptor>& descriptors)
{
    HILOGI("size: %{public}zu", descriptors.size());
    size_t idx = 0;

    if (descriptors.empty()) {
        return;
    }

    for (auto& descriptor : descriptors) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        ConvertBLEDescriptorToJS(env, obj, descriptor);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

void ConvertBLEDescriptorToJS(napi_env env, napi_value result, GattDescriptor& descriptor)
{
    HILOGI("uuid: %{public}s", descriptor.GetUuid().ToString().c_str());

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);

    if (descriptor.GetCharacteristic() != nullptr) {
        napi_value characteristicUuid;
        napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &characteristicUuid);
        napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
        }
    }

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    if (memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }
    napi_set_named_property(env, result, "descriptorValue", value);
}

void ConvertCharacteristicReadReqToJS(napi_env env, napi_value result, const std::string &device,
    GattCharacteristic &characteristic, int requestId)
{
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("offset is %{public}d", 0);

    napi_value characteristicUuid;
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);
    HILOGI("characteristicUuid is %{public}s", characteristic.GetUuid().ToString().c_str());

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
    }
}

void ConvertDescriptorReadReqToJS(napi_env env, napi_value result, const std::string &device,
    GattDescriptor& descriptor, int requestId)
{
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("offset is %{public}d", 0);

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);
    HILOGI("descriptorUuid is %{public}s", descriptor.GetUuid().ToString().c_str());

    if (descriptor.GetCharacteristic() != nullptr) {
    napi_value characteristicUuid;
    napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
        &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
        }
    }
}

void ConvertCharacteristicWriteReqToJS(napi_env env, napi_value result, const std::string &device,
    GattCharacteristic& characteristic, int requestId)
{
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("offset is %{public}d", 0);

    napi_value isPrepared;
    napi_get_boolean(env, false, &isPrepared);
    napi_set_named_property(env, result, "isPrepared", isPrepared);

    napi_value needRsp;
    napi_get_boolean(env, true, &needRsp);
    napi_set_named_property(env, result, "needRsp", needRsp);

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    (void)memcpy_s(bufferData, valueSize, valueData, valueSize);
    napi_set_named_property(env, result, "value", value);

    napi_value characteristicUuid;
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);
    HILOGI("characteristicUuid is %{public}s", characteristic.GetUuid().ToString().c_str());

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
    }
}

void ConvertDescriptorWriteReqToJS(napi_env env, napi_value result, const std::string &device,
    GattDescriptor &descriptor, int requestId)
{
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("offset is %{public}d", 0);

    napi_value isPrepared;
    napi_get_boolean(env, false, &isPrepared);
    napi_set_named_property(env, result, "isPrepared", isPrepared);

    napi_value needRsp;
    napi_get_boolean(env, true, &needRsp);
    napi_set_named_property(env, result, "needRsp", needRsp);

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    (void)memcpy_s(bufferData, valueSize, valueData, valueSize);
    napi_set_named_property(env, result, "value", value);

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);
    HILOGI("descriptorUuid is %{public}s", descriptor.GetUuid().ToString().c_str());

    if (descriptor.GetCharacteristic() != nullptr) {
        napi_value characteristicUuid;
        napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &characteristicUuid);
        napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
        }
    }
}

void SetGattClientDeviceId(const std::string &deviceId)
{
    deviceAddr = deviceId;
}

std::string GetGattClientDeviceId()
{
    return deviceAddr;
}

napi_value NapiNativeBleCharacteristic::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_object(env, &object);
    ConvertBLECharacteristicToJS(env, object, const_cast<GattCharacteristic &>(character_));
    return object;
}

napi_value NapiNativeBleDescriptor::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_object(env, &object);
    ConvertBLEDescriptorToJS(env, object, const_cast<GattDescriptor &>(descriptor_));
    return object;
}

napi_value NapiNativeGattServiceArray::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_array(env, &object);
    ConvertGattServiceVectorToJS(env, object, const_cast<vector<GattService> &>(gattServices_));
    return object;
}
}  // namespace Bluetooth
}  // namespace OHOS
