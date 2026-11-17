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
#ifndef BTGATT_SERVER_INTERFACE_MOCK_H
#define BTGATT_SERVER_INTERFACE_MOCK_H

#include "btif_gatt.h"
#include "gmock/gmock.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {
class BtgattServerInterfaceMock {
public:
    MOCK_METHOD(bt_status_t, RegisterServer, (const STACK::bluetooth::Uuid& uuid, bool eattSupport));
    MOCK_METHOD(bt_status_t, UnregisterServer, (int serverIf));
    MOCK_METHOD(bt_status_t, Connect, (int serverIf, const STACK::RawAddress& bdAddr, bool isDirect,
        int transport));
    MOCK_METHOD(bt_status_t, Disconnect, (int serverIf, const STACK::RawAddress& bdAddr, int connId));
    MOCK_METHOD(bt_status_t, AddService, (int serverIf, std::vector<btgatt_db_element_t> service));
    MOCK_METHOD(bt_status_t, StopService, (int serverIf, int serviceHandle));
    MOCK_METHOD(bt_status_t, DeleteService, (int serverIf, int serviceHandle));
    MOCK_METHOD(bt_status_t, SendIndication, (int serverIf, int attributeHandle, int connId, int confirm,
        std::vector<uint8_t> value));
    MOCK_METHOD(bt_status_t, SendResponse, (int connId, int transId, int status, const btgatt_response_t& response));
    MOCK_METHOD(bt_status_t, SetPreferredPhy, (const STACK::RawAddress& bdAddr, uint8_t txPhy, uint8_t rxPhy,
        uint16_t phyOptions));
    ~BtgattServerInterfaceMock() = default;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // BTGATT_SERVER_INTERFACE_MOCK_H