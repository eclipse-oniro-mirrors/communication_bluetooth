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

#include "ohos_bt_socket.h"

#include <iostream>
#include <cstring>
#include <vector>

#include "ohos_bt_adapter_utils.h"
#include "bluetooth_socket.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_object_map.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {

const int MAX_OBJECT_NUM = 10000;

static BluetoothObjectMap<std::shared_ptr<ServerSocket>, MAX_OBJECT_NUM> g_serverMap;
static BluetoothObjectMap<std::shared_ptr<ClientSocket>, MAX_OBJECT_NUM> g_clientMap;

static bool GetSocketUuidPara(const BluetoothCreateSocketPara *socketPara, UUID &serverUuid)
{
    if (socketPara->socketType == OHOS_SOCKET_SPP_RFCOMM) {
        if (socketPara->uuid.uuid == nullptr || strlen(socketPara->uuid.uuid) != socketPara->uuid.uuidLen) {
            HILOGE("param uuid invalid!");
            return false;
        }
        string tmpUuid(socketPara->uuid.uuid);
        if (!regex_match(tmpUuid, uuidRegex)) {
            HILOGE("match the UUID faild.");
            return false;
        }
        serverUuid = UUID::FromString(tmpUuid);
    } else if (socketPara->socketType == OHOS_SOCKET_L2CAP_LE) {
        serverUuid = UUID::RandomUUID();
    } else {
        HILOGE("param socketType invalid. socketType: %{public}d", socketPara->socketType);
        return false;
    }
    return true;
}

/**
 * @brief Creates an server listening socket based on the service record.
 *
 * @param socketPara The parameters to create a server socket.
 * @param name The service's name.
 * @return Returns a server ID, if create fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketServerCreate(const BluetoothCreateSocketPara *socketPara, const char *name)
{
    HILOGI("SocketServerCreate start!");
    if (!IS_BT_ENABLED()) {
        HILOGE("fail,BR is not TURN_ON");
        return BT_SOCKET_INVALID_ID;
    }

    if (socketPara == nullptr || name == nullptr) {
        HILOGE("socketPara or name is null.");
        return BT_SOCKET_INVALID_ID;
    }

    UUID serverUuid;
    if (!GetSocketUuidPara(socketPara, serverUuid)) {
        return BT_SOCKET_INVALID_ID;
    }

    string serverName(name);
    std::shared_ptr<ServerSocket> server = std::make_shared<ServerSocket>(serverName, serverUuid,
        BtSocketType(socketPara->socketType), socketPara->isEncrypt);
    server->Listen();
    int serverId = g_serverMap.AddObject(server);
    HILOGI("success, serverId: %{public}d, socketType: %{public}d, isEncrypt: %{public}d", serverId,
        socketPara->socketType, socketPara->isEncrypt);
    return serverId;
}

/**
 * @brief Waits for a remote device to connect to this server socket.
 *
 * This method return a client ID indicates a client socket
 * can be used to read data from and write data to remote device.
 * This method will block until a connection is established.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns a client ID, if accept fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketServerAccept(int serverId)
{
    HILOGI("SocketServerAccept start, serverId: %{public}d", serverId);
    std::shared_ptr<ServerSocket> server = g_serverMap.GetObject(serverId);
    if (server == nullptr) {
        HILOGE("server is null!");
        return BT_SOCKET_INVALID_ID;
    }

    std::shared_ptr<ClientSocket> client = server->Accept(0);
    if (client == nullptr) {
        HILOGE("client is null!");
        return BT_SOCKET_INVALID_ID;
    }
    int clientId = g_clientMap.AddObject(client);
    HILOGI("success, clientId: %{public}d", clientId);
    return clientId;
}

/**
 * @brief Disables an socket server socket and releases related resources.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketServerClose(int serverId)
{
    HILOGI("SocketServerClose start, serverId: %{public}d", serverId);
    std::shared_ptr<ServerSocket> server = g_serverMap.GetObject(serverId);
    if (server == nullptr) {
        HILOGE("server is null!");
        return OHOS_BT_STATUS_FAIL;
    }
    server->Close();
    g_serverMap.RemoveObject(serverId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Connects to a remote device over the socket.
 * This method will block until a connection is made or the connection fails.
 *
 * @param socketPara The param to create a client socket and connect to a remote device.
 * @param bdAddr The remote device address to connect.
 * @param psm BluetoothSocketType is {@link OHOS_SOCKET_L2CAP_LE} use dynamic PSM value from remote device.
 * BluetoothSocketType is {@link OHOS_SOCKET_SPP_RFCOMM} use -1.
 * @return Returns a client ID, if connect fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketConnect(const BluetoothCreateSocketPara *socketPara, const BdAddr *bdAddr, int psm)
{
    HILOGI("SocketConnect start.");
    if (socketPara == nullptr || bdAddr == nullptr) {
        HILOGE("socketPara is nullptr, or bdAddr is nullptr");
        return BT_SOCKET_INVALID_ID;
    }

    string strAddress;
    ConvertAddr(bdAddr->addr, strAddress);
    std::shared_ptr<BluetoothRemoteDevice> device = std::make_shared<BluetoothRemoteDevice>(strAddress, 0);

    UUID serverUuid;
    if (!GetSocketUuidPara(socketPara, serverUuid)) {
        return BT_SOCKET_INVALID_ID;
    }

    std::shared_ptr<ClientSocket> client = std::make_shared<ClientSocket>(*device, serverUuid,
        BtSocketType(socketPara->socketType), socketPara->isEncrypt);
    HILOGI("socketType: %{public}d, isEncrypt: %{public}d", socketPara->socketType, socketPara->isEncrypt);
    int result = client->Connect(psm);
    if (result != OHOS_BT_STATUS_SUCCESS) {
        HILOGE("SocketConnect fail, result: %{public}d", result);
        return BT_SOCKET_INVALID_ID;
    }
    int clientId = g_clientMap.AddObject(client);
    HILOGI("SocketConnect success, clientId: %{public}d", clientId);
    return clientId;
}

/**
 * @brief Disables a connection and releases related resources.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketDisconnect(int clientId)
{
    HILOGI("SocketDisconnect start, clientId: %{public}d", clientId);
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null, clientId: %{public}d", clientId);
        return OHOS_BT_STATUS_FAIL;
    }
    client->Close();
    g_clientMap.RemoveObject(clientId);
    HILOGI("SocketDisConnect success, clientId: %{public}d", clientId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Socket get remote device's address.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param remoteAddr Remote device's address, memory allocated by caller.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketGetRemoteAddr(int clientId, BdAddr *remoteAddr)
{
    HILOGI("SocketGetRemoteAddr clientId: %{public}d", clientId);
    if (remoteAddr == nullptr) {
        HILOGE("remoteAddr is null");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null, clientId: %{public}d", clientId);
        return OHOS_BT_STATUS_FAIL;
    }
    string tmpAddr = client->GetRemoteDevice().GetDeviceAddr();
    GetAddrFromString(tmpAddr, remoteAddr->addr);
    HILOGI("device: %{public}s", GetEncryptAddr(tmpAddr).c_str());
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Get the connection status of this socket.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @return Returns true is connected or false is not connected.
 */
bool IsSocketConnected(int clientId)
{
    HILOGI("IsSocketConnected clientId: %{public}d", clientId);
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null, clientId: %{public}d", clientId);
        return false;
    }
    bool isConnected = client->IsConnected();
    HILOGI("clientId: %{public}d, isConnected: %{public}d", clientId, isConnected);
    return isConnected;
}

/**
 * @brief Read data from socket.
 * This method blocks until input data is available.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param buf Indicate the buffer which read in, memory allocated by caller.
 * @param bufLen Indicate the buffer length.
 * @return Returns the length greater than 0 as read the actual length.
 * Returns {@link BT_SOCKET_READ_SOCKET_CLOSED} if the socket is closed.
 * Returns {@link BT_SOCKET_READ_FAILED} if the operation failed.
 */
int SocketRead(int clientId, uint8_t *buf, uint32_t bufLen)
{
    HILOGI("SocketRead start, clientId: %{public}d, bufLen: %{public}d", clientId, bufLen);
    if (buf == nullptr || bufLen == 0) {
        HILOGE("buf is null or bufLen is 0!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null, clientId: %{public}d", clientId);
        return BT_SOCKET_READ_FAILED;
    }

    int readLen = client->GetInputStream().Read(buf, bufLen);
    HILOGI("SocketRead ret, clientId: %{public}d, readLen: %{public}d", clientId, readLen);
    return readLen;
}

/**
 * @brief Client write data to socket.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param data Indicate the data to be written.
 * @return Returns the actual write length.
 * Returns {@link BT_SOCKET_WRITE_FAILED} if the operation failed.
 */
int SocketWrite(int clientId, const uint8_t *data, uint32_t len)
{
    HILOGI("SocketWrite start, clientId: %{public}d, len: %{public}d", clientId, len);
    if (data == nullptr || len == 0) {
        HILOGE("data is null or len is 0!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null!");
        return OHOS_BT_STATUS_FAIL;
    }
    size_t writeLen = client->GetOutputStream().Write(data, len);
    HILOGI("end, writeLen: %{public}zu", writeLen);
    return static_cast<int>(writeLen);
}

/**
 * @brief Get dynamic PSM value.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns the PSM value.
 * Returns {@link BT_SOCKET_INVALID_PSM} if the operation failed.
 */
int SocketGetPsm(int serverId)
{
    HILOGI("SocketGetPsm.");
    std::shared_ptr<ServerSocket> server = g_serverMap.GetObject(serverId);
    if (server == nullptr) {
        HILOGE("server is null!");
        return BT_SOCKET_INVALID_PSM;
    }
    int psm = server->GetPsm();
    return psm;
}

/**
 * @brief Adjust the socket send and recv buffer size, limit range is 4KB to 50KB
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param bufferSize The buffer size want to set, unit is byte.
 * @return  Returns the operation result status {@link BtStatus}.
 */
int SetSocketBufferSize(int clientId, uint32_t bufferSize)
{
    HILOGI("start, clientId: %{public}d, bufferSize: %{public}d", clientId, bufferSize);
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null!");
        return OHOS_BT_STATUS_FAIL;
    }
    int ret = client->SetBufferSize(bufferSize);
    if (ret == RET_BAD_PARAM) {
        return OHOS_BT_STATUS_PARM_INVALID;
    } else if (ret == RET_BAD_STATUS) {
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif