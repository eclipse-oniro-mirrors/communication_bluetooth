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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines a bluetooth system that provides basic blurtooth connection and profile functions,
 *        including A2DP, AVRCP, BLE, GATT, HFP, MAP, PBAP, and SPP, etc.
 *
 * @since 6
 */

/**
 * @file bluetooth_socket.h
 *
 * @brief Declares spp socket framework functions, including basic functions.
 *
 * @since 6
 */

#ifndef BLUETOOTH_SOCKET_H
#define BLUETOOTH_SOCKET_H

#include <string>
#include <vector>
#include <memory>

#include "bluetooth_remote_device.h"
#include "bluetooth_socket_inputstream.h"
#include "bluetooth_socket_outputstream.h"

namespace OHOS {
namespace Bluetooth {
enum BtSocketType {
    TYPE_RFCOMM = 0x0,
    TYPE_L2CAP = 0x01,
    TYPE_L2CAP_LE = 0x02,
};

enum SocketState {
    SOCKET_INIT,
    SOCKET_CONNECTED,
    SOCKET_LISTENING,
    SOCKET_CLOSED,
};

const int FLAG_ENCRYPT = 1;
const int FLAG_AUTH = 1 << 1;

const int SPP_SOCKET_PSM_VALUE = -1;

/**
 * @brief Class for client socket functions.
 *
 * @since 6
 */
class BLUETOOTH_API ClientSocket {
public:
    /**
     * @brief A constructor used to create an ClientSocket instance.
     *
     * @param bda Remote device object.
     * @param uuid Uuid.
     * @param type Socket type.
     * @param auth Connection state.
     * @since 6
     */
    ClientSocket(const BluetoothRemoteDevice &bda, UUID uuid, BtSocketType type, bool auth);

    /**
     * @brief A constructor used to create an ClientSocket instance. This constructor to construct the
     * ClientSocket object when the Accept function is called.
     *
     * @param fd Socket fd.
     * @param address Remote bluetooth address.
     * @param type Socket type.
     * @since 6
     */
    ClientSocket(int fd, std::string address, BtSocketType type);

    /**
     * @brief Destroy the ClientSocket object.
     *
     * @since 6
     */
    virtual ~ClientSocket();

    /**
     * @brief The function is used to connect to a remote device.
     *
     * @param psm dynamic PSM value from remote device.
     * @return Returns <b>0</b> if the operation is successful.
     *         Returns <b>-1</b> if the operation fails.
     * @since 6
     */
    int Connect(int psm);

    /**
     * @brief Client disconnected.
     *
     * @since 6
     */
    void Close();

    /**
     * @brief Get the input stream with this socket.
     *
     * @return Returns the object of the InputStream class.
     * @since 6
     */
    InputStream &GetInputStream();

    /**
     * @brief Get the output stream with this socket.
     *
     * @return Returns the object of the OutputStream class.
     * @since 6
     */
    OutputStream &GetOutputStream();

    /**
     * @brief Get the remote device with this socket.
     *
     * @return Remote device.
     * @since 6
     */
    BluetoothRemoteDevice &GetRemoteDevice();

    /**
     * @brief Get the connection status of this socket.
     *
     * @return Returns <b>true</b> is connected.
     *         Returns <b>false</b> is not connected.
     * @since 6
     */
    bool IsConnected() const;

    /**
     * @brief Set socket send & recv buffer size, The size limit ranges from 4KB to 50KB.
     *
     * @return the operation status
     * @since 6
     */
    int SetBufferSize(int bufferSize);

    /**
     * @brief Get client socket fd
     *
     * @return int fd
     * @since 6
     */
    int GetSocketFd();

private:
    ClientSocket() = delete;
    BLUETOOTH_DECLARE_IMPL();
};

/**
 * @brief Class for server socket functions.
 *
 * @since 6
 */
class BLUETOOTH_API ServerSocket {
public:
    /**
     * @brief A constructor used to create an ServerSocket instance.
     *
     * @param name Server name.
     * @param uuid Uuid.
     * @param type Socket type.
     * @param encrypt Remote device auth and encrypt connection.
     * @since 6
     */
    ServerSocket(const std::string &name, UUID uuid, BtSocketType type, bool encrypt);

    /**
     * @brief Destroy the ServerSocket object.
     *
     * @since 6
     */
    ~ServerSocket();

    /**
     * @brief Listen the client connect event.
     *
     * @return listen error code.
     * @since 6
     */
    int Listen();

    /**
     * @brief Accept a client connection and return an acceptClientSocket to interact with the client.
     *
     * @param timeout Timeout for the accept.
     * @return A ClientSocket.
     * @since 6
     */
    std::shared_ptr<ClientSocket> Accept(int timeout);

    /**
     * @brief Server disconnected.
     *
     * @since 6
     */
    void Close();

    /**
     * @brief Get the server socket type and server name.
     *
     * @return A string.
     * @since 6
     */
    const std::string &GetStringTag();

    /**
     * @brief Get dynamic PSM value.
     *
     * @return int psm.
     * @since 6
     */
    int GetPsm();

    /**
     * @brief Get server socket fd
     *
     * @return int fd.
     * @since 6
     */
    int GetSocketFd();

private:
    BLUETOOTH_DECLARE_IMPL();
};

class BLUETOOTH_API SocketFactory {
public:
    /**
     * @brief Create a server record to listen to the insecure rfcomm.
     *
     * @param name Server name.
     * @param uuid Uuid.
     * @return A ServerSocket.
     * @since 6
     */
    static std::shared_ptr<ServerSocket> DataListenInsecureRfcommByServiceRecord(
        const std::string &name, const UUID &uuid);

    /**
     * @brief Create a server record to listen to the rfcomm.
     *
     * @param name Server name.
     * @param uuid Uuid.
     * @return A ServerSocket.
     * @since 6
     */
    static std::shared_ptr<ServerSocket> DataListenRfcommByServiceRecord(const std::string &name, const UUID &uuid);

    /**
     * @brief Build insecure rfcomm data socket by service record.
     *
     * @param device Remote device object.
     * @param uuid Uuid.
     * @return A ClientSocket.
     * @since 6
     */
    static std::shared_ptr<ClientSocket> BuildInsecureRfcommDataSocketByServiceRecord(
        const BluetoothRemoteDevice &device, const UUID &uuid);

    /**
     * @brief Build rfcomm data socket by service record.
     *
     * @param device Remote device object.
     * @param uuid Uuid.
     * @return A ClientSocket.
     * @since 6
     */
    static std::shared_ptr<ClientSocket> BuildRfcommDataSocketByServiceRecord(
        const BluetoothRemoteDevice &device, const UUID &uuid);
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // BLUETOOTH_SOCKET_H