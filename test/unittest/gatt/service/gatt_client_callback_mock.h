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
#ifndef GATT_CLIENT_CALLBACK_MOCK_H
#define GATT_CLIENT_CALLBACK_MOCK_H

#include "gmock/gmock.h"
#include "interface_profile_gatt_client.h"

namespace OHOS {
namespace bluetooth {
class GattClientCallbackMock : public IGattClientCallback {
public:
    MOCK_METHOD(void, OnServicesChanged, (), (override));
    MOCK_METHOD(void, OnCharacteristicRead, (int ret, const Characteristic &characteristic), (override));
    MOCK_METHOD(void, OnCharacteristicWrite, (int ret, const Characteristic &characteristic,
        const Bluetooth::BluetoothGattRspContext &rspContext), (override));
    MOCK_METHOD(void, OnCharacteristicChanged, (const Characteristic &characteristic), (override));
    MOCK_METHOD(void, OnDescriptorRead, (int ret, const Descriptor &descriptor), (override));
    MOCK_METHOD(void, OnDescriptorWrite, (int ret, const Descriptor &descriptor), (override));
    MOCK_METHOD(void, OnConnectionStateChanged, (int state, int newState, const RawAddress &addr, int reason,
        const std::string &reasonMessage), (override));
    MOCK_METHOD(void, OnMtuChanged, (int state, int mtu), (override));
    MOCK_METHOD(void, OnServicesDiscovered, (int status), (override));
    MOCK_METHOD(void, OnConnectionParameterChanged, (int interval, int latency, int timeout, int status), (override));
    MOCK_METHOD(void, OnReadRemoteRssiValue, (const RawAddress &addr, int rssi, int status), (override));
    MOCK_METHOD(void, OnBlePhyUpdate, (int32_t txPhy, int32_t rxPhy, int32_t status), (override));
    MOCK_METHOD(void, OnBlePhyRead, (int32_t txPhy, int32_t rxPhy, int32_t status), (override));
    ~GattClientCallbackMock() override = default;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif // GATT_CLIENT_CALLBACK_MOCK_H
