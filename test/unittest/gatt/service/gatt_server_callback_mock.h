/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef GATT_SERVER_CALLBACK_MOCK_H
#define GATT_SERVER_CALLBACK_MOCK_H

#include "gmock/gmock.h"
#include "interface_profile_gatt_server.h"

namespace OHOS {
namespace bluetooth {
class GattServerCallbackMock : public IGattServerCallback {
public:
    MOCK_METHOD(void, OnCharacteristicReadRequest, (const GattDevice &device, const Characteristic &characteristic),
        (override));
    MOCK_METHOD(void, OnCharacteristicReadByUuidRequest, (const GattDevice &device,
        const Characteristic &characteristic), (override));
    MOCK_METHOD(void, OnCharacteristicWriteRequest, (const GattDevice &device, const Characteristic &characteristic,
        bool needRespones), (override));
    MOCK_METHOD(void, OnDescriptorReadRequest, (const GattDevice &device, const Descriptor &descriptor), (override));
    MOCK_METHOD(void, OnDescriptorWriteRequest, (const GattDevice &device, const Descriptor &descriptor), (override));
    MOCK_METHOD(void, OnNotifyConfirm, (const GattDevice &device, const Characteristic &characteristic, int result),
        (override));
    MOCK_METHOD(void, OnConnectionStateChanged, (const GattDevice &device, int ret, int state, int reason,
        const std::string &reasonMessage), (override));
    MOCK_METHOD(void, OnMtuChanged, (const GattDevice &device, int mtu), (override));
    MOCK_METHOD(void, OnAddService, (int ret, const Service &services), (override));
    MOCK_METHOD(void, OnServiceChanged, (const Service &services), (override));
    MOCK_METHOD(void, OnConnectionParameterChanged, (const GattDevice &device, int interval, int latency, int timeout,
        int status), (override));
    ~GattServerCallbackMock() override = default;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif // GATT_SERVER_CALLBACK_MOCK_H
