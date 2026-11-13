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

#ifndef TEST_FUZZTEST_GATT_COMMON_H
#define TEST_FUZZTEST_GATT_COMMON_H

#include <optional>

#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_server.h"
#include "bluetooth_remote_device.h"
#include "uuid.h"

constexpr size_t SIZE_TWO = 2;
constexpr size_t SIZE_BYTE = 8;

std::optional<OHOS::Bluetooth::UUID> BuildUuid(const uint8_t* data, size_t &cur, size_t size);
std::optional<OHOS::Bluetooth::GattService> BuildGattService(const uint8_t* data, size_t &cur, size_t size);
std::optional<OHOS::Bluetooth::GattCharacteristic> BuildGattCharacteristic(
    const uint8_t* data, size_t &cur, size_t size);
std::optional<OHOS::Bluetooth::GattDescriptor> BuildGattDescriptor(const uint8_t* data, size_t &cur, size_t size);
std::optional<OHOS::Bluetooth::BluetoothRemoteDevice> BuildBluetoothRemoteDevice(
    const uint8_t* data, size_t &cur, size_t size);
std::shared_ptr<OHOS::Bluetooth::GattServer> GetDefaultGattServer(void);

#endif // TEST_FUZZTEST_GATT_COMMON_H