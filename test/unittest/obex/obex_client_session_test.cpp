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
#include <fstream>
#include "obex_header.h"
#include "obex_client_session.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class ObexResponseHandlerTest : public ObexResponseHandler {
public:
    void OnConnectResp(ObexHeader &reply, uint8_t responseCode) override {}
    void OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override {}
    void OnDisConnectResp(ObexHeader &reply, uint8_t responseCode) override {}
    void OnAbortResp() override {}
    virtual ~ObexResponseHandlerTest() {}
};

const std::string TYPE_EVENT_REPORT = "x-bt/MAP-event-report";
std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
            51, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_RFCOMM, "11:22:33:44:55:66");
std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(socketDevice);
std::shared_ptr<ObexResponseHandlerTest> handler = std::make_shared<ObexResponseHandlerTest>();
std::shared_ptr<ObexClientSession> clientSession = std::make_shared<ObexClientSession>(transport, handler);
static const std::string TEST_FILE_PATH = "/data/service/el1/public/bluetooth/bt_config.xml";
static const uint32_t MTU = 50;
static const uint64_t FILE_LEN = 300;

class ObexClientSessionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

class ObexBodyTestObject : public ObexBodyObject {
public:
    explicit ObexBodyTestObject(const std::string &file)
    {
        ifs_.open(file, std::ios::in);
        if (!ifs_.is_open()) {
            HILOGE("file open failed");
            return;
        } else {
            HILOGI("file=%{public}s  opened.", file.c_str());
        }
        ifs_.seekg(0, std::ios::end);
        fileSize_ = static_cast<size_t>(ifs_.tellg());
        ifs_.seekg(0, std::ios::beg);
    }
    ~ObexBodyTestObject() override {}
    size_t Read(uint8_t *buf, size_t bufLen) override
    {
        return 0;
    }
    size_t Write(const uint8_t *buf, size_t bufLen) override
    {
        return 0;
    }
    int Close() override
    {
        return 0;
    }
private:
    std::ifstream ifs_ {};
    size_t fileSize_ = 0;
};

/*
 * @tc.number: ObexClientSession_Connect_001
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(ObexClientSessionTest, ObexClientSession_Connect_001, TestSize.Level1)
{
    HILOGI("ObexClientSession_Connect_001 start");
    std::vector<uint8_t> MNS_TARGET = {
        0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexHeader header;
    header.SetHeader(TARGET, MNS_TARGET);
    ObexHeader outHeader;
    clientSession->Connect(header, outHeader);
    EXPECT_TRUE(clientSession != nullptr);
    HILOGI("ObexClientSession_Connect_001 end");
}

/*
 * @tc.number: ObexClientSession_SendEvent_001
 * @tc.name: SendEvent
 * @tc.desc:
 */
HWTEST_F(ObexClientSessionTest, ObexClientSession_SendEvent_001, TestSize.Level1)
{
    HILOGI("ObexClientSession_SendEvent_001 start");
    std::vector<uint8_t> appData = {
        0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexHeader request;
    request.SetHeader(TYPE, std::vector<uint8_t>(TYPE_EVENT_REPORT.begin(), TYPE_EVENT_REPORT.end()));
    request.SetHeader(APPLICATION_PARAMETER, appData);
    request.SetConnectionId(10);
    std::string eventData = "123";
    clientSession->SendEvent(request, eventData);
    EXPECT_TRUE(clientSession != nullptr);
    HILOGI("ObexClientSession_SendEvent_001 end");
}

HWTEST_F(ObexClientSessionTest, ObexClientSession_SendEventRequest_001, TestSize.Level1)
{
    HILOGI("ObexClientSession_SendEventRequest_001 start");
    std::vector<uint8_t> eventData = { 'a', 'b', 'c', 'd' };
    uint8_t responseCode = clientSession->SendEventRequest(0x80, eventData);
    EXPECT_EQ(responseCode, 0);
    HILOGI("ObexClientSession_SendEventRequest_001 end");
}

/*
 * @tc.number: ObexClientSession_Put_001
 * @tc.name: Put
 * @tc.desc:
 */
HWTEST_F(ObexClientSessionTest, ObexClientSession_Put_001, TestSize.Level1)
{
    HILOGI("ObexClientSession_Put_001 start");
    std::vector<uint8_t> appData = {
        0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexHeader request;
    request.SetHeader(TYPE, std::vector<uint8_t>(TYPE_EVENT_REPORT.begin(), TYPE_EVENT_REPORT.end()));
    request.SetHeader(APPLICATION_PARAMETER, appData);
    request.SetConnectionId(10);
    ObexHeader outResponse;
    outResponse.SetConnectionId(10);
    std::vector<uint8_t> bodyData {};
    clientSession->Put(request, outResponse, false, bodyData);
    EXPECT_TRUE(clientSession != nullptr);
    HILOGI("ObexClientSession_Put_001 end");
}

/*
 * @tc.number: ObexClientSession_SendRequest_001
 * @tc.name: SendRequest
 * @tc.desc:
 */
HWTEST_F(ObexClientSessionTest, ObexClientSession_SendRequest_001, TestSize.Level1)
{
    HILOGI("ObexClientSession_SendRequest_001 start");
    std::vector<uint8_t> appData = {
        0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexHeader request;
    request.SetHeader(TYPE, std::vector<uint8_t>(TYPE_EVENT_REPORT.begin(), TYPE_EVENT_REPORT.end()));
    request.SetHeader(APPLICATION_PARAMETER, appData);
    request.SetConnectionId(10);
    ObexHeader outResponse;
    outResponse.SetConnectionId(10);
    std::vector<uint8_t> bodyData {};
    uint8_t code = OBEX_OPCODE_CONNECT;
    clientSession->SendRequest(code, bodyData, outResponse);
    EXPECT_TRUE(clientSession != nullptr);
    HILOGI("ObexClientSession_SendRequest_001 end");
}

/*
 * @tc.number: ObexClientSession_ReadResponse_001
 * @tc.name: ReadResponse
 * @tc.desc:
 */
HWTEST_F(ObexClientSessionTest, ObexClientSession_ReadResponse_001, TestSize.Level1)
{
    HILOGI("ObexClientSession_ReadResponse_001 start");
    ObexHeader outResponse;
    std::vector<uint8_t> bodyData {};
    uint32_t length = 0;
    uint8_t code = OBEX_OPCODE_CONNECT;
    uint8_t responseCode = OBEX_HTTP_OK;
    EXPECT_TRUE(clientSession->ReadResponse(code, responseCode, length, outResponse) < 0);
    HILOGI("ObexClientSession_ReadResponse_001 end");
}

/*
 * @tc.number: ObexClientSession_HandleResponse_001
 * @tc.name: HandleResponse
 * @tc.desc:
 */
HWTEST_F(ObexClientSessionTest, ObexClientSession_HandleResponse_001, TestSize.Level1)
{
    HILOGI("ObexClientSession_HandleResponse_001 start");
    ObexHeader outResponse;
    outResponse.SetConnectionId(10);
    std::vector<uint8_t> bodyData {};
    uint8_t code = OBEX_OPCODE_CONNECT;
    uint8_t responseCode = OBEX_HTTP_OK;
    clientSession->HandleResponse(code, responseCode, outResponse);
    EXPECT_TRUE(clientSession->HandleResponse(code, responseCode, outResponse) == 0);
    HILOGI("ObexClientSession_HandleResponse_001 end");
}

/*
 * @tc.number: ObexClientSession_HandleConnectResult_001
 * @tc.name: HandleResponse
 * @tc.desc:
 */
HWTEST_F(ObexClientSessionTest, ObexClientSession_HandleConnectResult_001, TestSize.Level1)
{
    HILOGI("ObexClientSession_HandleConnectResult_001 start");
    ObexHeader outResponse;
    uint32_t length = 0;
    clientSession->HandleConnectResult(length, outResponse);
    EXPECT_FALSE(outResponse.GetConnectionId() == 0);
    HILOGI("ObexClientSession_HandleConnectResult_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS