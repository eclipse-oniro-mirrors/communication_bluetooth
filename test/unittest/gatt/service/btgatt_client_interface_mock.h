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
#ifndef BTGATT_CLIENT_INTERFACE_MOCK_H
#define BTGATT_CLIENT_INTERFACE_MOCK_H

#include "btif_gatt.h"
#include "gmock/gmock.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {
class BtgattClientInterfaceMock {
public:
    MOCK_METHOD(bt_status_t, RegisterClient, (const STACK::bluetooth::Uuid& uuid, bool eattSupport));
    MOCK_METHOD(bt_status_t, UnregisterClient, (int clientIf));
    MOCK_METHOD(bt_status_t, Connect, (int clientIf, const STACK::RawAddress& bdAddr, bool isDirect,
        int transport, bool opportunistic, int initiatingPhys));
    MOCK_METHOD(bt_status_t, Disconnect, (int clientIf, const STACK::RawAddress& bdAddr, int connId));
    MOCK_METHOD(bt_status_t, Refresh, (int clientIf, const STACK::RawAddress& bdAddr));
    MOCK_METHOD(bt_status_t, SearchService, (int connId, const STACK::bluetooth::Uuid *filterUuid));
    MOCK_METHOD(bt_status_t, ReadCharacteristic, (int connId, uint16_t handle, int authReq));
    MOCK_METHOD(bt_status_t, ReadUsingCharacteristicUuid, (int connId, const STACK::bluetooth::Uuid& uuid,
        uint16_t startHandle, uint16_t endHandle, int authReq));
    MOCK_METHOD(bt_status_t, WriteCharacteristic, (int connId, uint16_t handle, int writeType, int authReq,
        std::vector<uint8_t> value));
    MOCK_METHOD(bt_status_t, ReadDescriptor, (int connId, uint16_t handle, int authReq));
    MOCK_METHOD(bt_status_t, WriteDescriptor, (int connId, uint16_t handle, int authReq, std::vector<uint8_t> value));
    MOCK_METHOD(bt_status_t, ExecuteWrite, (int connId, int execute));
    MOCK_METHOD(bt_status_t, RegisterForNotification, (int clientIf, const STACK::RawAddress& bdAddr,
        uint16_t handle));
    MOCK_METHOD(bt_status_t, DeregisterForNotification, (int clientIf, const STACK::RawAddress& bdAddr,
        uint16_t handle));
    MOCK_METHOD(bt_status_t, ReadRemoteRssi, (int clientIf, const STACK::RawAddress& bdAddr));
    MOCK_METHOD(int, GetDeviceType, (const STACK::RawAddress& bdAddr));
    MOCK_METHOD(bt_status_t, ConfigureMtu, (int connId, int mtu));
    MOCK_METHOD(bt_status_t, ConnParameterUpdate, (const STACK::RawAddress& bdAddr, int minInterval,
        int maxInterval, int latency, int timeout, uint16_t minCeLen, uint16_t maxCeLen));
    MOCK_METHOD(bt_status_t, SetPreferredPhy, (const STACK::RawAddress& bdAddr, uint8_t txPhy, uint8_t rxPhy,
        uint16_t phyOptions));
    MOCK_METHOD(bt_status_t, GetGattDb, (int connId));
    ~BtgattClientInterfaceMock() = default;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // BTGATT_CLIENT_INTERFACE_MOCK_H