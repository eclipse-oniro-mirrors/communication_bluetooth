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

#include "btgatt_interface_inner.h"
#include "bluetooth_log.h"

namespace {
OHOS::bluetooth::BtgattServerInterfaceMock *g_btgattServerInterfaceMock = nullptr;
OHOS::bluetooth::BtgattClientInterfaceMock *g_btgattClientInterfaceMock = nullptr;
}

extern "C" {
static bt_status_t RegisterServerCWrapper(const STACK::bluetooth::Uuid& uuid, bool eattSupport)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->RegisterServer(uuid, eattSupport);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t UnregisterServerCWrapper(int serverIf)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->UnregisterServer(serverIf);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ConnectCWrapper(int serverIf, const STACK::RawAddress& bdAddr, bool isDirect, int transport)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->Connect(serverIf, bdAddr, isDirect, transport);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t DisconnectCWrapper(int serverIf, const STACK::RawAddress& bdAddr, int connId)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->Disconnect(serverIf, bdAddr, connId);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t AddServiceCWrapper(int serverIf, std::vector<btgatt_db_element_t> service)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->AddService(serverIf, service);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t StopServiceCWrapper(int serverIf, int serviceHandle)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->StopService(serverIf, serviceHandle);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t DeleteServiceCWrapper(int serverIf, int serviceHandle)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->DeleteService(serverIf, serviceHandle);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t SendIndicationCWrapper(int serverIf, int attributeHandle, int connId, int confirm,
    std::vector<uint8_t> value)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->SendIndication(serverIf, attributeHandle, connId, confirm, value);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t SendResponseCWrapper(int connId, int transId, int status, const btgatt_response_t& response)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->SendResponse(connId, transId, status, response);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t SetPreferredPhyCWrapper(const STACK::RawAddress& bdAddr, uint8_t txPhy, uint8_t rxPhy,
    uint16_t phyOptions)
{
    if (g_btgattServerInterfaceMock) {
        return g_btgattServerInterfaceMock->SetPreferredPhy(bdAddr, txPhy, rxPhy, phyOptions);
    }
    return BT_STATUS_FAIL;
}

static const btgatt_server_interface_t g_btgattServerInterface = {
    .register_server = RegisterServerCWrapper,
    .unregister_server = UnregisterServerCWrapper,
    .connect = ConnectCWrapper,
    .disconnect = DisconnectCWrapper,
    .add_service = AddServiceCWrapper,
    .stop_service = StopServiceCWrapper,
    .delete_service = DeleteServiceCWrapper,
    .send_indication = SendIndicationCWrapper,
    .send_response = SendResponseCWrapper,
    .set_preferred_phy = SetPreferredPhyCWrapper,
};

void RegisterGattServerInterfaceMock(std::function<void(const btgatt_server_interface_t *interface)> func,
    OHOS::bluetooth::BtgattServerInterfaceMock *interfaceMock)
{
    g_btgattServerInterfaceMock = interfaceMock;
    func(&g_btgattServerInterface);
}

void UnRegisterGattServerInterfaceMock(void)
{
    g_btgattServerInterfaceMock = nullptr;
}

static bt_status_t RegisterClientCWrapper(const STACK::bluetooth::Uuid& uuid, bool eattSupport)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->RegisterClient(uuid, eattSupport);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t UnregisterClientCWrapper(int clientIf)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->UnregisterClient(clientIf);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ClientConnectCWrapper(int clientIf, const STACK::RawAddress& bdAddr, bool isDirect,
    int transport, bool opportunistic, int initiatingPhys)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->Connect(clientIf, bdAddr, isDirect, transport, opportunistic,
            initiatingPhys);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ClientDisconnectCWrapper(int clientIf, const STACK::RawAddress& bdAddr, int connId)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->Disconnect(clientIf, bdAddr, connId);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t RefreshCWrapper(int clientIf, const STACK::RawAddress& bdAddr)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->Refresh(clientIf, bdAddr);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t SearchServiceCWrapper(int connId, const STACK::bluetooth::Uuid *filterUuid)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->SearchService(connId, filterUuid);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ReadCharacteristicCWrapper(int connId, uint16_t handle, int authReq)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->ReadCharacteristic(connId, handle, authReq);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ReadUsingCharacteristicUuidCWrapper(int connId, const STACK::bluetooth::Uuid& uuid,
    uint16_t startHandle, uint16_t endHandle, int authReq)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->ReadUsingCharacteristicUuid(connId, uuid, startHandle, endHandle, authReq);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t WriteCharacteristicCWrapper(int connId, uint16_t handle, int writeType, int authReq,
    std::vector<uint8_t> value)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->WriteCharacteristic(connId, handle, writeType, authReq, value);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ReadDescriptorCWrapper(int connId, uint16_t handle, int authReq)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->ReadDescriptor(connId, handle, authReq);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t WriteDescriptorCWrapper(int connId, uint16_t handle, int authReq, std::vector<uint8_t> value)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->WriteDescriptor(connId, handle, authReq, value);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ExecuteWriteCWrapper(int connId, int execute)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->ExecuteWrite(connId, execute);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t RegisterForNotificationCWrapper(int clientIf, const STACK::RawAddress& bdAddr, uint16_t handle)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->RegisterForNotification(clientIf, bdAddr, handle);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t DeregisterForNotificationCWrapper(int clientIf, const STACK::RawAddress& bdAddr, uint16_t handle)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->DeregisterForNotification(clientIf, bdAddr, handle);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ReadRemoteRssiCWrapper(int clientIf, const STACK::RawAddress& bdAddr)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->ReadRemoteRssi(clientIf, bdAddr);
    }
    return BT_STATUS_FAIL;
}
static int GetDeviceTypeCWrapper(const STACK::RawAddress& bdAddr)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->GetDeviceType(bdAddr);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ConfigureMtuCWrapper(int connId, int mtu)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->ConfigureMtu(connId, mtu);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ConnParameterUpdateCWrapper(const STACK::RawAddress& bdAddr, int minInterval, int maxInterval,
    int latency, int timeout, uint16_t minCeLen, uint16_t maxCeLen)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->ConnParameterUpdate(bdAddr, minInterval, maxInterval, latency, timeout,
            minCeLen, maxCeLen);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t ClientSetPreferredPhyCWrapper(const STACK::RawAddress& bdAddr, uint8_t txPhy, uint8_t rxPhy,
    uint16_t phyOptions)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->SetPreferredPhy(bdAddr, txPhy, rxPhy, phyOptions);
    }
    return BT_STATUS_FAIL;
}
static bt_status_t GetGattDbCWrapper(int connId)
{
    if (g_btgattClientInterfaceMock) {
        return g_btgattClientInterfaceMock->GetGattDb(connId);
    }
    return BT_STATUS_FAIL;
}

static const btgatt_client_interface_t g_btgattClientInterface = {
    .register_client = RegisterClientCWrapper,
    .unregister_client = UnregisterClientCWrapper,
    .connect = ClientConnectCWrapper,
    .disconnect = ClientDisconnectCWrapper,
    .refresh = RefreshCWrapper,
    .search_service = SearchServiceCWrapper,
    .read_characteristic = ReadCharacteristicCWrapper,
    .read_using_characteristic_uuid = ReadUsingCharacteristicUuidCWrapper,
    .write_characteristic = WriteCharacteristicCWrapper,
    .read_descriptor = ReadDescriptorCWrapper,
    .write_descriptor = WriteDescriptorCWrapper,
    .execute_write = ExecuteWriteCWrapper,
    .register_for_notification = RegisterForNotificationCWrapper,
    .deregister_for_notification = DeregisterForNotificationCWrapper,
    .read_remote_rssi = ReadRemoteRssiCWrapper,
    .get_device_type = GetDeviceTypeCWrapper,
    .configure_mtu = ConfigureMtuCWrapper,
    .conn_parameter_update = ConnParameterUpdateCWrapper,
    .set_preferred_phy = ClientSetPreferredPhyCWrapper,
    .get_gatt_db = GetGattDbCWrapper,
};

void RegisterGattClientInterfaceMock(std::function<void(const btgatt_client_interface_t *interface)> func,
    OHOS::bluetooth::BtgattClientInterfaceMock *interfaceMock)
{
    g_btgattClientInterfaceMock = interfaceMock;
    func(&g_btgattClientInterface);
}
void UnRegisterGattClientInterfaceMock(void)
{
    g_btgattClientInterfaceMock = nullptr;
}

}  // extern "C"