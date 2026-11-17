/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <cstring>
#include "bluetooth_log.h"
#include "obex_server_transport.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
std::string address("11:22:33:44:55:66");
std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
            51, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_RFCOMM, address);
ObexServerTransport obexTransport(socketDevice);

class ObexServerTransportTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase(
    )
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: ObexServerTransport_Read_001
 * @tc.name: Read
 * @tc.desc:
 */
HWTEST_F(ObexServerTransportTest, ObexServerTransport_Read_001, TestSize.Level1)
{
    HILOGI("ObexServerTransport_Read_001 start");
    uint8_t buf[] = { 0x10, 0x11 };
    ssize_t result = obexTransport.Read(buf, 2);
    HILOGI("ObexServerTransport_Read_001 result: %{public}zd", result);
    EXPECT_EQ(result, -1);

    result = obexTransport.Read(buf, 0);
    EXPECT_EQ(result, -1);

    result = obexTransport.Read(nullptr, 0);
    EXPECT_EQ(result, -1);

    obexTransport.socketDevice_->mSocketType = SocketType::TYPE_L2CAP;
    result = obexTransport.Read(buf, 2);
    HILOGI("ObexServerTransport_Read_001 end");
}

/*
 * @tc.number: ObexServerTransport_Read_002
 * @tc.name: Read
 * @tc.desc:
 */
HWTEST_F(ObexServerTransportTest, ObexServerTransport_Read_002, TestSize.Level1)
{
    HILOGI("ObexServerTransport_Read_002 start");
    uint8_t buf[] = { 0x10, 0x11 };
    std::shared_ptr<ObexSocketDevice> device = std::make_shared<ObexSocketDevice>(
            51, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_L2CAP_LE, address);
    ObexServerTransport transport(device);
    transport.l2capBufferSize_ = 2;
    ssize_t result = transport.Read(buf, 2);
    EXPECT_EQ(result, 2);

    transport.l2capBufferSize_ = 1;
    transport.l2capBufferReadIndex_ = 0;
    result = transport.Read(buf, 2);
    EXPECT_EQ(result, 1);

    transport.l2capBufferSize_ = 3;
    result = transport.Read(buf, 2);
    EXPECT_EQ(result, 2);

    transport.l2capBufferSize_ = 3;
    result = transport.Read(buf, 2, 1);
    EXPECT_EQ(result, -1);

    transport.socketDevice_->mSocketFd = -1;
    result = transport.Read(buf, 2);
    EXPECT_EQ(result, -1);
    HILOGI("ObexServerTransport_Read_002 end");
}

/*
 * @tc.number: ObexServerTransport_Write_001
 * @tc.name: Write
 * @tc.desc:
 */
HWTEST_F(ObexServerTransportTest, ObexServerTransport_Write_001, TestSize.Level1)
{
    HILOGI("ObexServerTransport_Write_001 start");
    uint8_t buf[2] = {};
    ssize_t result = obexTransport.Write(buf, 2);
    HILOGI("ObexServerTransport_Write_001 result: %{public}zd", result);
    EXPECT_EQ(result, -1);

    if (obexTransport.socketDevice_ == nullptr) {
        HILOGI("ObexServerTransport_Write_001 socketDevice_ is null");
    }

    obexTransport.socketDevice_->mSocketFd = -1;
    result = obexTransport.Write(buf, 2);
    EXPECT_EQ(result, -1);
    HILOGI("ObexServerTransport_Write_001 end");
}

/*
 * @tc.number: ObexServerTransport_ReadL2capData_001
 * @tc.name: ReadL2capData
 * @tc.desc:
 */
HWTEST_F(ObexServerTransportTest, ObexServerTransport_ReadL2capData_001, TestSize.Level1)
{
    HILOGI("ObexServerTransport_ReadL2capData_001 start");
    uint8_t buf[] = { 0x10, 0x11 };
    size_t length = 2;

    obexTransport.l2capBufferSize_ = 2;
    obexTransport.l2capBufferReadIndex_ = 0;
    obexTransport.socketDevice_->mSocketFd = 256;
    ssize_t res = obexTransport.ReadL2capData(buf, length);
    EXPECT_EQ(res, 2);
    HILOGI("ObexServerTransport_ReadL2capData_001 end");
}

/*
 * @tc.number: ObexServerTransport_ReadL2capData_002
 * @tc.name: ReadL2capData
 * @tc.desc:
 */
HWTEST_F(ObexServerTransportTest, ObexServerTransport_ReadL2capData_002, TestSize.Level1)
{
    HILOGI("ObexServerTransport_ReadL2capData_002 start");
    uint8_t buf[] = { 0x10, 0x11 };
    size_t length = 2;

    obexTransport.l2capBufferSize_ = 1;
    obexTransport.l2capBufferReadIndex_ = 2;
    ssize_t res = obexTransport.ReadL2capData(buf, length);
    EXPECT_EQ(res, -1);
    HILOGI("ObexServerTransport_ReadL2capData_002 end");
}

/*
 * @tc.number: ObexServerTransport_ReadL2capData_003
 * @tc.name: ReadL2capData
 * @tc.desc:
 */
HWTEST_F(ObexServerTransportTest, ObexServerTransport_ReadL2capData_003, TestSize.Level1)
{
    HILOGI("ObexServerTransport_ReadL2capData_003 start");
    uint8_t buf[] = {};
    size_t length = 0;

    obexTransport.l2capBufferSize_ = 2;
    obexTransport.l2capBufferReadIndex_ = 1;
    ssize_t res = obexTransport.ReadL2capData(buf, length);
    EXPECT_EQ(res, 0);
    HILOGI("ObexServerTransport_ReadL2capData_003 end");
}

/*
 * @tc.number: ObexServerTransport_Close_001
 * @tc.name: Write
 * @tc.desc:
 */
HWTEST_F(ObexServerTransportTest, ObexServerTransport_Close_001, TestSize.Level1)
{
    HILOGI("ObexServerTransport_Close_001 start");
    obexTransport.socketDevice_->mSocketFd = 256;
    obexTransport.Close();
    EXPECT_EQ(obexTransport.socketDevice_->mSocketFd, -1);

    obexTransport.socketDevice_->mSocketFd = -1;
    obexTransport.Close();
    EXPECT_EQ(obexTransport.socketDevice_->mSocketFd, -1);
    HILOGI("ObexServerTransport_Close_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS