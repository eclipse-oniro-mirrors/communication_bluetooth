/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <array>
#include <sstream>
#include <string>

#include "common.h"
#include "bluetooth_log.h"

using namespace OHOS::Bluetooth;

std::optional<UUID> BuildUuid(const uint8_t* data, size_t &cur, size_t size)
{
    const size_t len = UUID::UUID128_BYTES_LEN;
    if (size - cur < len) {
        return std::nullopt;
    }

    std::array<uint8_t, len> uuidName {};
    for (size_t i = 0; i < len; i++) {
        uuidName[i] = *(data + cur + i);
    }
    auto uuid = UUID::ConvertFrom128Bits(uuidName);
    HILOGI("%{public}s", uuid.ToString().c_str());

    cur += len;
    return uuid;
}

std::optional<GattDescriptor> BuildGattDescriptor(const uint8_t* data, size_t &cur, size_t size)
{
    HILOGI("enter");
    size_t curInner = cur;
    auto uuidOpt = BuildUuid(data, curInner, size);
    if (!uuidOpt.has_value() || (size - curInner < 1)) {
        return std::nullopt;
    }

    int permissions = *(data + curInner);
    curInner += 1;
    GattDescriptor descriptor(uuidOpt.value(), permissions);

    cur = curInner;
    return descriptor;
}

std::optional<GattCharacteristic> BuildGattCharacteristic(const uint8_t* data, size_t &cur, size_t size)
{
    HILOGI("enter");
    size_t curInner = cur;
    auto uuidOpt = BuildUuid(data, curInner, size);
    if (!uuidOpt.has_value() || (size - curInner < SIZE_TWO)) {
        return std::nullopt;
    }

    int permissions = *(data + curInner);
    int properties = *(data + curInner + 1);
    curInner += SIZE_TWO;
    GattCharacteristic characteristic(uuidOpt.value(), permissions, properties);

    auto descriptorOpt = BuildGattDescriptor(data, curInner, size);
    if (descriptorOpt.has_value()) {
        characteristic.AddDescriptor(descriptorOpt.value());
    }

    cur = curInner;
    return characteristic;
}

std::optional<GattService> BuildGattService(const uint8_t* data, size_t &cur, size_t size)
{
    HILOGI("enter");
    size_t curInner = cur;
    auto uuidOpt = BuildUuid(data, curInner, size);
    if (!uuidOpt.has_value() || (size - curInner < 1)) {
        return std::nullopt;
    }

    int type = *(data + curInner);
    curInner += 1;
    GattService service(uuidOpt.value(), static_cast<GattServiceType>(type));

    auto characteristicOpt = BuildGattCharacteristic(data, curInner, size);
    if (characteristicOpt.has_value()) {
        service.AddCharacteristic(characteristicOpt.value());
    }

    cur = curInner;
    return service;
}

static std::string HexToString(uint8_t hex)
{
    std::stringstream ss;
    ss << hex;
    return ss.str();
}

static const size_t ADDRESS_SIZE = 6;
static std::string BuildAddressString(const uint8_t* data)
{
    std::string addr = "";
    for (size_t i = 0; i < ADDRESS_SIZE; i++) {
        auto str = HexToString(*(data + i));
        addr += str;
        if (i < ADDRESS_SIZE - 1) {
            addr += ":";
        }
    }
    HILOGI("%{public}s", addr.c_str());
    return addr;
}

std::optional<BluetoothRemoteDevice> BuildBluetoothRemoteDevice(const uint8_t* data, size_t &cur, size_t size)
{
    if (size - cur < ADDRESS_SIZE + 1) {
        return std::nullopt;
    }

    auto addr = BuildAddressString(data + cur);
    cur += ADDRESS_SIZE;
    int transport = *(data + cur);
    cur += 1;

    BluetoothRemoteDevice device(addr, transport);
    return device;
}

class GattServerCallbackMock : public GattServerCallback {
    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state) override {}
};

std::shared_ptr<GattServer> GetDefaultGattServer(void)
{
    std::shared_ptr<GattServer> g_gatts = nullptr;
    std::shared_ptr<GattServerCallbackMock> callbackMock = std::make_shared<GattServerCallbackMock>();
    std::shared_ptr<GattServerCallback> tmpSptr = std::static_pointer_cast<GattServerCallback>(callbackMock);
    g_gatts = GattServer::CreateInstance(tmpSptr);
    return g_gatts;
}
