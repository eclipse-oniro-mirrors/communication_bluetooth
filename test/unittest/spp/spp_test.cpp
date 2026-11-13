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

#include <gtest/gtest.h>
#include "bluetooth_socket.h"
#include "bluetooth_gatt_characteristic.h"
#include "uuid.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "ohos_bt_def.h"
#include "ohos_bt_gatt_server.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;

static BluetoothHost *host_;
class SocketTest : public testing::Test {
public:
    SocketTest()
    {}
    ~SocketTest()
    {}

    std::shared_ptr<ClientSocket> sppClientSocket_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    BluetoothRemoteDevice *pbluetoothRomote_ = nullptr;
    UUID randomUuid_;
    UUID insecureUuid_;

    std::shared_ptr<ServerSocket> sppServerSocket_ = nullptr;
    UUID uuid_;
};

void SocketTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}
void SocketTest::TearDownTestCase(void)
{
    host_ = nullptr;
}
void SocketTest::SetUp()
{
    std::string address_ = "00:01:02:03:04:05";
    pbluetoothRomote_ = new BluetoothRemoteDevice(address_, 0);
    randomUuid_ = UUID::RandomUUID();
    insecureUuid_ = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    sppClientSocket_ = std::make_shared<ClientSocket>(*pbluetoothRomote_, randomUuid_, TYPE_RFCOMM, false);

    uuid_ = UUID::FromString("11111111-0000-1000-8000-00805F9B34FB");
    if (!sppServerSocket_) {
        HILOGI("SocketFactory::DataListenRfcommByServiceRecord starts");
    }
    sppServerSocket_ = SocketFactory::DataListenRfcommByServiceRecord("server", uuid_);
}

void SocketTest::TearDown()
{
    delete pbluetoothRomote_;
    pbluetoothRomote_ = nullptr;
}

/**
 * @tc.number:Spp_UnitTest001
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("ClientSocket::Connect starts");
    sppClientSocket_->Connect(-1);
    HILOGI("ClientSocket::Connect ends");
}

/**
 * @tc.number:Spp_UnitTest002
 * @tc.name: Close
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_Close, TestSize.Level1)
{
    HILOGI("ClientSocket::Close starts");
    sppClientSocket_->Close();
    sleep(TIME);
    bool connectionState = sppClientSocket_->IsConnected();
    EXPECT_FALSE(connectionState);
    HILOGI("ClientSocket::Close ends");
}

/**
 * @tc.number:Spp_UnitTest003
 * @tc.name: GetInputStream
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetInputStream, TestSize.Level1)
{
    int fd = 37;
    BluetoothRemoteDevice device_;
    std::shared_ptr<ClientSocket> pfd_SppClientSocket =
        std::make_shared<ClientSocket>(fd, device_.GetDeviceAddr(), TYPE_RFCOMM);

    uint8_t receive[512];
    int DATASIZE = 1024;
    ssize_t returnInput = 0;
    HILOGI("ClientSocket::GetInputStream starts");
    std::shared_ptr<InputStream> input = pfd_SppClientSocket->GetInputStream();
    returnInput = input->Read(receive, DATASIZE);
    EXPECT_EQ((int)returnInput, -1);
    HILOGI("ClientSocket::GetInputStream ends");
}

/**
 * @tc.number:Spp_UnitTest004
 * @tc.name: GetOutputStream
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetOutputStream, TestSize.Level1)
{
    HILOGI("ClientSocket::GetOutputStream starts");
    uint8_t multiChar[10] = {'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    int fd = 37;
    BluetoothRemoteDevice device_;
    std::shared_ptr<ClientSocket> pfd_SppClientSocket =
        std::make_shared<ClientSocket>(fd, device_.GetDeviceAddr(), TYPE_RFCOMM);
    std::shared_ptr<OutputStream> output = pfd_SppClientSocket->GetOutputStream();
    size_t returnOutput = 0;
    returnOutput = output->Write(multiChar, 10);
    EXPECT_EQ((int)returnOutput, -1);
    HILOGI("ClientSocket::GetOutputStream ends");
}

/**
 * @tc.number:Spp_UnitTest005
 * @tc.name: GetRemoteDevice
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetRemoteDevice, TestSize.Level1)
{
    HILOGI("ClientSocket::GetRemoteDevice starts");
    int fd = 37;
    BluetoothRemoteDevice device_;
    std::shared_ptr<ClientSocket> pfd_SppClientSocket =
        std::make_shared<ClientSocket>(fd, device_.GetDeviceAddr(), TYPE_RFCOMM);
    BluetoothRemoteDevice tempRemoteDevice = pfd_SppClientSocket->GetRemoteDevice();
    EXPECT_STREQ("00:00:00:00:00:00", tempRemoteDevice.GetDeviceAddr().c_str());
    HILOGI("ClientSocket::GetRemoteDevice ends");
}

/**
 * @tc.number:Spp_UnitTest006
 * @tc.name: IsConnected
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_IsConnected, TestSize.Level1)
{
    HILOGI("ClientSocket::IsConnected starts");
    int fd = 37;
    BluetoothRemoteDevice device_;
    std::shared_ptr<ClientSocket> pfd_SppClientSocket =
        std::make_shared<ClientSocket>(fd, device_.GetDeviceAddr(), TYPE_RFCOMM);
    bool IsConnected = false;
    IsConnected = pfd_SppClientSocket->IsConnected();
    EXPECT_EQ(IsConnected, false);
    HILOGI("ClientSocket::IsConnected ends");
}

/**
 * @tc.number:Spp_UnitTest007
 * @tc.name: GetStringTag
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetStringTag, TestSize.Level1)
{
    HILOGI("ServerSocket::GetStringTag starts");
    std::string returnStr{""};
    returnStr = sppServerSocket_->GetStringTag();
    EXPECT_NE("", returnStr);
    HILOGI("ServerSocket::GetStringTag ends");
}

/**
 * @tc.number:Spp_UnitTest008
 * @tc.name: Accept
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_Accept, TestSize.Level1)
{
    HILOGI("ServerSocket::Accept starts");

    int SERTIMEOUT = 10;
    std::shared_ptr<ClientSocket> preturn_SppClientSocket = sppServerSocket_->Accept(SERTIMEOUT);
    HILOGI("ServerSocket::Accept ends");
    EXPECT_NE(&preturn_SppClientSocket, nullptr);
}

/**
 * @tc.number:Spp_UnitTest009
 * @tc.name: SetSocketBufferSize
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_SetSocketBufferSize, TestSize.Level1)
{
    HILOGI("ClientSocket::SetSocketBufferSize starts");

    int fd = 256;
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    std::shared_ptr<ClientSocket> pfd_SppClientSocket =
        std::make_shared<ClientSocket>(fd, device_.GetDeviceAddr(), TYPE_RFCOMM);
    int bufferSize = 10 * 1024;
    int ret = pfd_SppClientSocket->SetBufferSize(bufferSize);
    EXPECT_EQ(ret, RET_BAD_STATUS);

    bufferSize = 3 * 1024;
    ret = pfd_SppClientSocket->SetBufferSize(bufferSize);
    EXPECT_EQ(ret, RET_BAD_PARAM);

    bufferSize = 51 * 1024;
    ret = pfd_SppClientSocket->SetBufferSize(bufferSize);
    EXPECT_EQ(ret, RET_BAD_PARAM);

    HILOGI("ClientSocket::SetSocketBufferSize ends");
}

/**
 * @tc.number:Spp_UnitTest010
 * @tc.name: GetInputStream Read
 * @tc.desc: socketFd_ == -1
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetInputStream_002, TestSize.Level1)
{
    int fd = -1;
    BluetoothRemoteDevice device_;
    std::shared_ptr<ClientSocket> pfd_SppClientSocket =
        std::make_shared<ClientSocket>(fd, device_.GetDeviceAddr(), TYPE_RFCOMM);
    uint8_t receive[512];
    int DATASIZE = 1024;
    ssize_t returnInput = 0;
    HILOGI("ClientSocket::GetInputStream starts");
    std::shared_ptr<InputStream> input = pfd_SppClientSocket->GetInputStream();
    returnInput = input->Read(receive, DATASIZE);
    EXPECT_EQ((int)returnInput, -1);
    HILOGI("ClientSocket::GetInputStream ends");
}

/**
 * @tc.number:Socket_UnitTest001
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(SocketTest, Scoket_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("ClientSocket::Connect starts");
    std::shared_ptr<ClientSocket> clientSocket =
        std::make_shared<ClientSocket>(*pbluetoothRomote_, randomUuid_, TYPE_L2CAP_LE, true);
    int psm = 0x80;
    int result = clientSocket->Connect(psm);
    EXPECT_NE(result, RET_NO_ERROR);
    HILOGI("ClientSocket::Connect ends");
}


/**
 * @tc.number:Socket_UnitTest002
 * @tc.name: Accept
 * @tc.desc:
 */
HWTEST_F(SocketTest, Socket_UnitTest_Accept, TestSize.Level1)
{
    HILOGI("ServerSocket::Accept starts");

    int SERTIMEOUT = 10;
    std::shared_ptr<ServerSocket> serverSocket = std::make_shared<ServerSocket>("server", uuid_, TYPE_L2CAP_LE, true);
    std::shared_ptr<ClientSocket> clientSocket = serverSocket->Accept(SERTIMEOUT);
    EXPECT_EQ(clientSocket, nullptr);
    HILOGI("ServerSocket::Accept ends");
}

/**
 * @tc.number:Socket_UnitTest003
 * @tc.name: Accept
 * @tc.desc:
 */
HWTEST_F(SocketTest, Socket_UnitTest_GetPsm, TestSize.Level1)
{
    HILOGI("ServerSocket::GetPsm starts");
    std::shared_ptr<ServerSocket> serverSocket = std::make_shared<ServerSocket>("server", uuid_, TYPE_L2CAP_LE, true);
    int psm = -1;
    psm = serverSocket->GetRfcommScn();
    EXPECT_GE(psm, -1);
    HILOGI("ServerSocket::GetPsm ends");
}

/**
 * @tc.number:Spp_UnitTest011
 * @tc.name: OutputStream Write
 * @tc.desc: socketFd_ == -1
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetOutputStream_002, TestSize.Level1)
{
    HILOGI("ClientSocket::GetOutputStream starts");
    uint8_t multiChar[10] = {'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    int fd = -1;
    BluetoothRemoteDevice device_;
    std::shared_ptr<ClientSocket> pfd_SppClientSocket =
        std::make_shared<ClientSocket>(fd, device_.GetDeviceAddr(), TYPE_RFCOMM);
    std::shared_ptr<OutputStream> output = pfd_SppClientSocket->GetOutputStream();
    size_t returnOutput = 0;
    returnOutput = output->Write(multiChar, 10);
    EXPECT_EQ((int)returnOutput, -1);
    HILOGI("ClientSocket::GetOutputStream ends");
}

/**
 * @tc.number: Socket_UnitTest004
 * @tc.name: Accept
 * @tc.desc:
 */

HWTEST_F(SocketTest, Socket_UnitTest_Listen, TestSize.Level1)
{
    HILOGI("Socket_UnitTest_Listen start");
    std::shared_ptr<ServerSocket> serverSocket = std::make_shared<ServerSocket>("server", uuid_, TYPE_L2CAP_LE, true);
    int psm = -1;
    psm = serverSocket->Listen();
    EXPECT_GE(psm, -1);
    HILOGI("Socket_UnitTest_Listen end");
}

/**
 * @tc.number: Socket_UnitTest005
 * @tc.name:
 * @tc.desc:
 */

HWTEST_F(SocketTest, Socket_UnitTest_Close, TestSize.Level1)
{
    HILOGI("Socket_UnitTest_Close start");
    std::shared_ptr<ServerSocket> serverSocket = std::make_shared<ServerSocket>("server", uuid_, TYPE_L2CAP_LE, true);
    serverSocket->Close();
    EXPECT_NE(serverSocket, nullptr);
    HILOGI("Socket_UnitTest_Close end");
}

/**
 * @tc.number: Socket_UnitTest006
 * @tc.name:
 * @tc.desc:
 */

HWTEST_F(SocketTest, Socket_UnitTest_DataListenInsecureRfcommByServiceRecord, TestSize.Level1)
{
    HILOGI("Socket_UnitTest_DataListenInsecureRfcommByServiceRecord start");
    std::shared_ptr<ServerSocket> serverSocket;
    serverSocket = SocketFactory::DataListenInsecureRfcommByServiceRecord("bluetoothdevice", randomUuid_);
    EXPECT_NE(nullptr, serverSocket);
    HILOGI("Socket_UnitTest_DataListenInsecureRfcommByServiceRecord end");
}

/**
 * @tc.number: BLE_BleScanResultTest_BuildInsecureRfcommDataSocketByServiceRecord_00100
 * @tc.name:
 * @tc.desc:
 */

HWTEST_F(SocketTest, BLE_BleScanResultTest_BuildInsecureRfcommDataSocketByServiceRecord_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_BuildInsecureRfcommDataSocketByServiceRecord_00100 start");
    std::shared_ptr<ClientSocket> clientSocket;
    clientSocket = SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord(*pbluetoothRomote_, randomUuid_);
    EXPECT_NE(nullptr, clientSocket);
    HILOGI("BLE_BleScanResultTest_BuildInsecureRfcommDataSocketByServiceRecord_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_BuildRfcommDataSocketByServiceRecord_00100
 * @tc.name:
 * @tc.desc:
 */

HWTEST_F(SocketTest, BLE_BleScanResultTest_BuildRfcommDataSocketByServiceRecord_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_BuildRfcommDataSocketByServiceRecord_00100 start");
    std::shared_ptr<ClientSocket> clientSocket;
    BluetoothRemoteDevice device;
    clientSocket = SocketFactory::BuildRfcommDataSocketByServiceRecord(device, randomUuid_);
    EXPECT_NE(nullptr, clientSocket);
    HILOGI("BLE_BleScanResultTest_BuildRfcommDataSocketByServiceRecord_00100 end");
}

/**
 * @tc.number:Spp_UnitTest001
 * @tc.name: UpdateCocConnectionParams
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_UpdateCocConnectionParams, TestSize.Level1)
{
    HILOGI("ClientSocket::UpdateCocConnectionParams starts");
    CocUpdateSocketParam info;
    info.addr = "00:00:00:00:00:00";
    info.minInterval = 1;
    info.maxInterval = 2;
    info.peripheralLatency = 3;
    info.supervisionTimeout = 4;
    info.minConnEventLen = 5;
    info.maxConnEventLen = 6;
    sppClientSocket_->UpdateCocConnectionParams(info);
    EXPECT_NE(sppClientSocket_, nullptr);
    HILOGI("ClientSocket::UpdateCocConnectionParams ends");
}

/**
 * @tc.number: Socket_UnitTest007
 * @tc.name: GetMaxTransmitPacketSize
 * @tc.desc:
 */

HWTEST_F(SocketTest, Socket_UnitTest_GetMaxTransmitPacketSize, TestSize.Level1)
{
    HILOGI("ServerSocket::GetMaxTransmitPacketSize starts");
    int tx = sppServerSocket_->GetMaxTransmitPacketSize();
    EXPECT_EQ(tx, 0);
    HILOGI("ServerSocket::GetMaxTransmitPacketSize ends");
}

/**
 * @tc.number: Socket_UnitTest008
 * @tc.name: GetMaxReceivePacketSize
 * @tc.desc:
 */

HWTEST_F(SocketTest, Socket_UnitTest_GetMaxReceivePacketSize, TestSize.Level1)
{
    HILOGI("ServerSocket::GetMaxReceivePacketSize starts");
    int rx = sppServerSocket_->GetMaxReceivePacketSize();
    EXPECT_EQ(rx, 0);
    HILOGI("ServerSocket::GetMaxReceivePacketSize ends");
}

/**
 * @tc.number: Socket_UnitTest009
 * @tc.name: GetSocketFd
 * @tc.desc:
 */

HWTEST_F(SocketTest, Socket_UnitTest_GetSocketFd, TestSize.Level1)
{
    HILOGI("ServerSocket::GetSocketFd starts");
    int fd = sppServerSocket_->GetSocketFd();
    EXPECT_EQ(fd, -1);
    HILOGI("ServerSocket::GetSocketFd ends");
}

/**
 * @tc.number:Spp_UnitTest013
 * @tc.name: GetSocketFd
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetSocketFd, TestSize.Level1)
{
    HILOGI("ClientSocket::GetSocketFd starts");
    int fd = sppClientSocket_->GetSocketFd();
    EXPECT_EQ(fd, -1);
    HILOGI("ClientSocket::GetSocketFd ends");
}

/**
 * @tc.number:Spp_UnitTest014
 * @tc.name: GetL2capPsm
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetL2capPsm, TestSize.Level1)
{
    HILOGI("ClientSocket::GetL2capPsm starts");
    int psm = sppClientSocket_->GetL2capPsm();
    EXPECT_EQ(psm, -1);
    HILOGI("ClientSocket::GetL2capPsm ends");
}

/**
 * @tc.number:Spp_UnitTest015
 * @tc.name: GetRfcommScn
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetRfcommScn, TestSize.Level1)
{
    HILOGI("ClientSocket::GetRfcommScn starts");
    int scn = sppClientSocket_->GetRfcommScn();
    EXPECT_EQ(scn, -1);
    HILOGI("ClientSocket::GetRfcommScn ends");
}

/**
 * @tc.number:Spp_UnitTest016
 * @tc.name: GetMaxTransmitPacketSize
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetMaxTransmitPacketSize, TestSize.Level1)
{
    HILOGI("ClientSocket::GetMaxTransmitPacketSize starts");
    int tx = sppClientSocket_->GetMaxTransmitPacketSize();
    EXPECT_EQ(tx, 0);
    HILOGI("ClientSocket::GetMaxTransmitPacketSize ends");
}

/**
 * @tc.number:Spp_UnitTest017
 * @tc.name: GetMaxReceivePacketSize
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_GetMaxReceivePacketSize, TestSize.Level1)
{
    HILOGI("ClientSocket::GetMaxReceivePacketSize starts");
    int rx = sppClientSocket_->GetMaxReceivePacketSize();
    EXPECT_EQ(rx, 0);
    HILOGI("ClientSocket::GetMaxReceivePacketSize ends");
}

/**
 * @tc.number:Spp_UnitTest018
 * @tc.name: IsAllowSocketConnect
 * @tc.desc:
 */
HWTEST_F(SocketTest, Spp_UnitTest_IsAllowSocketConnect, TestSize.Level1)
{
    HILOGI("ClientSocket::IsAllowSocketConnect starts");
    const int OHOS_SOCKET_L2CAP_LE = 0x02;
    bool ret = sppClientSocket_->IsAllowSocketConnect(OHOS_SOCKET_L2CAP_LE);
    EXPECT_TRUE(ret);
    const int OHOS_SOCKET_SPP_RFCOMM = 0x00;
    ret = sppClientSocket_->IsAllowSocketConnect(OHOS_SOCKET_SPP_RFCOMM);
    EXPECT_TRUE(ret);
    HILOGI("ClientSocket::IsAllowSocketConnect ends");
}

/**
 * @tc.number:ClientSocket001
 * @tc.name: BleGattsConnect
 * @tc.desc:
 */
HWTEST_F(SocketTest, ClientSocket_UnitTest_BleGattsConnect, TestSize.Level1)
{
    HILOGI("ClientSocket::ClientSocket_UnitTest_BleGattsConnect starts");
    int serverId = 1;
    BdAddr bdAddr;
    int ret = BleGattsConnect(serverId, bdAddr);
    EXPECT_EQ(ret, 8);
    HILOGI("ClientSocket::ClientSocket_UnitTest_BleGattsConnect ends");
}

/**
 * @tc.number: GetL2capPsm
 * @tc.name: GetL2capPsm
 * @tc.desc:
 */
HWTEST_F(SocketTest, Socket_UnitTest_GetL2capPsm, TestSize.Level1)
{
    HILOGI("ServerSocket::GetSocketFd starts");
    int result = sppServerSocket_->GetL2capPsm();
    EXPECT_EQ(result, -1);
    HILOGI("ServerSocket::GetSocketFd ends");
}

/**
 * @tc.number: Init
 * @tc.name: Init
 * @tc.desc:
 */
HWTEST_F(SocketTest, Socket_UnitTest_Init, TestSize.Level1)
{
    HILOGI("Socket_UnitTest_Init starts");
    bool ret = sppClientSocket_->Init();
    EXPECT_TRUE(ret);
    HILOGI("Socket_UnitTest_Init ends");
}

/*
 * @tc.number: Uuid_UnitTest_FromString
 * @tc.name: FromString
 * @tc.desc:
*/
HWTEST_F(SocketTest, Uuid_UnitTest_FromString, TestSize.Level1)
{
    HILOGI("Uuid_UnitTest_FromString starts");
    std::string str;
    UUID null_uuid = UUID::FromString(str);
    EXPECT_EQ("00000000-0000-0000-0000-000000000000", null_uuid.ToString());
    UUID error_uuid = UUID::FromString("00001810-0000-1000-8000-0080");
    EXPECT_EQ("00000000-0000-0000-0000-000000000000", error_uuid.ToString());
    UUID invalid_uuid = UUID::FromString("00001810000000100008000000805F9B34FB");
    EXPECT_EQ("00000000-0000-0000-0000-000000000000", invalid_uuid.ToString());
    UUID correct_uuid = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    EXPECT_EQ("00001810-0000-1000-8000-00805F9B34FB", correct_uuid.ToString());
    HILOGI("Uuid_UnitTest_FromString ends");
}

/**
 * @tc.number: Socket_UnitTest_listen002
 * @tc.name: 
 * @tc.desc:
 */

HWTEST_F(SocketTest, Socket_UnitTest_Listen002, TestSize.Level1)
{
    HILOGI("Socket_UnitTest_Listen002 start");
    int psm = 241;
    const int SOCK_L2CAP_LE = 0x02;
    std::shared_ptr<ServerSocket> serverSocket = std::make_shared<ServerSocket>("server", uuid_, TYPE_L2CAP_LE, true, psm);
    int ret = serverSocket->Listen();
    EXPECT_GE(ret, -1);
    HILOGI("Socket_UnitTest_Listen002 end");
}

/**
 * @tc.number: Socket_UnitTest_GetType
 * @tc.name: GetType
 * @tc.desc:
 */

HWTEST_F(SocketTest, Socket_UnitTest_GetType001, TestSize.Level1)
{
    HILOGI("Socket_UnitTest_GetType001 start");
    int ret = sppClientSocket_->GetType();
    EXPECT_EQ(ret, 0);
    HILOGI("Socket_UnitTest_GetType001 end");
}

/**
 * @tc.number: Socket_UnitTest_SetMaxTransmitPacketSize
 * @tc.name: SetMaxTransmitPacketSize
 * @tc.desc:
 */
HWTEST_F(SocketTest, Socket_UnitTest_SetMaxTransmitPacketSize001, TestSize.Level1)
{
    HILOGI("Socket_UnitTest_SetMaxTransmitPacketSize001 start");
    sppClientSocket_->SetMaxTransmitPacketSize(65535);
    EXPECT_EQ(sppClientSocket_->GetMaxTransmitPacketSize(), 65535);
    HILOGI("Socket_UnitTest_SetMaxTransmitPacketSize001 end");
}

/**
 * @tc.number: Socket_UnitTest_SetMaxReceivePacketSize
 * @tc.name: SetMaxReceivePacketSize
 * @tc.desc:
 */
HWTEST_F(SocketTest, Socket_UnitTest_SetMaxReceivePacketSize001, TestSize.Level1)
{
    HILOGI("Socket_UnitTest_SetMaxReceivePacketSize001 start");
    sppClientSocket_->SetMaxReceivePacketSize(65535);
    EXPECT_EQ(sppClientSocket_->GetMaxReceivePacketSize(), 65535);
    HILOGI("Socket_UnitTest_SetMaxReceivePacketSize001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS