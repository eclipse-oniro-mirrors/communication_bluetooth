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
#include "btcommon/timer_manager.h"
#include <cstring>
#include "log.h"
#include "obex_server_actions.h"
#include "pbap_pse_service.h"
#include "pbap_pse_obex_server.h"
#include "class_creator.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
std::string address("11:22:33:44:55:66");
std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
            51, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_L2CAP, address);
std::shared_ptr<ObexServerTransport> obexTransport = std::make_shared<ObexServerTransport>(socketDevice);
std::shared_ptr<PbapPseObexServer> pbapPseObexServer =
    std::make_shared<PbapPseObexServer>(socketDevice->mDeviceAddress, nullptr);
std::unique_ptr<ObexServerActionInterface> actionInterface =
        std::make_unique<ObexServerConnectAction>(obexTransport, pbapPseObexServer);

void *PbapPseServiceCreate(void)
{
    return new PbapPseService();
}

class ObexServerConnectActionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
    }
    static void TearDownTestCase()
    {
        utility::CloseAllTimer();
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: ObexServerActionInterface_ParseReqeustPackageData_001
 * @tc.name: ParseReqeustPackageData
 * @tc.desc:
 */
HWTEST_F(ObexServerConnectActionTest, ObexServerActionInterface_ParseReqeustPackageData_001, TestSize.Level1)
{
    HILOGI("ObexServerActionInterface_ParseReqeustPackageData_001 start");
    std::vector<uint8_t> data;
    actionInterface->SendResponseData(1, data, 2);
    actionInterface->ParseReqeustPackageData(5, OBEX_OPCODE_GET);

    data = {0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c};
    actionInterface->SendResponseData(3, data, 5);
    actionInterface->request_.SetHeader(SINGLE_RESPONSE_MODE, data);
    actionInterface->ParseReqeustPackageData(0, OBEX_OPCODE_GET);

    actionInterface->request_.SetHeader(SINGLE_RESPONSE_MODE_PARAMETER, data);
    actionInterface->ParseReqeustPackageData(0, OBEX_OPCODE_GET_FINAL);

    actionInterface->request_.SetHeader(BODY, data);
    actionInterface->ParseReqeustPackageData(0, OBEX_OPCODE_PUT);

    actionInterface->requestHandler_->SetConnectionId(10);
    actionInterface->ParseReqeustPackageData(0, OBEX_OPCODE_SETPATH);
    EXPECT_TRUE(actionInterface != nullptr);
    HILOGI("ObexServerActionInterface_ParseReqeustPackageData_001 end");
}

/*
 * @tc.number: ObexServerActionInterface_SetReplyHeaderConnectionId_001
 * @tc.name: SetReplyHeaderConnectionId
 * @tc.desc:
 */
HWTEST_F(ObexServerConnectActionTest, ObexServerActionInterface_SetReplyHeaderConnectionId_001, TestSize.Level1)
{
    HILOGI("ObexServerActionInterface_SetReplyHeaderConnectionId_001 start");
    actionInterface->SetReplyHeaderConnectionId();

    actionInterface->requestHandler_->SetConnectionId(5);
    actionInterface->SetReplyHeaderConnectionId();
    EXPECT_TRUE(actionInterface != nullptr);
    HILOGI("ObexServerActionInterface_SetReplyHeaderConnectionId_001 end");
}

/*
 * @tc.number: ObexServerActionInterface_ValidateResponseCode_001
 * @tc.name: ValidateResponseCode
 * @tc.desc:
 */
HWTEST_F(ObexServerConnectActionTest, ObexServerActionInterface_ValidateResponseCode_001, TestSize.Level1)
{
    HILOGI("ObexServerActionInterface_ValidateResponseCode_001 start");
    uint8_t result = actionInterface->ValidateResponseCode(ResponseCodes::OBEX_HTTP_OK);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_OK);

    result = actionInterface->ValidateResponseCode(ResponseCodes::OBEX_HTTP_NO_CONTENT);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_NO_CONTENT);

    result = actionInterface->ValidateResponseCode(ResponseCodes::OBEX_HTTP_MULT_CHOICE);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_MULT_CHOICE);

    result = actionInterface->ValidateResponseCode(ResponseCodes::OBEX_HTTP_BAD_REQUEST);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_BAD_REQUEST);

    result = actionInterface->ValidateResponseCode(ResponseCodes::OBEX_HTTP_VERSION);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_VERSION);

    result = actionInterface->ValidateResponseCode(ResponseCodes::OBEX_DATABASE_FULL);
    EXPECT_EQ(result, ResponseCodes::OBEX_DATABASE_FULL);

    result = actionInterface->ValidateResponseCode(ResponseCodes::OBEX_HTTP_CONTINUE);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_CONTINUE);
    HILOGI("ObexServerActionInterface_ValidateResponseCode_001 end");
}

/*
 * @tc.number: ObexServerConnectAction_HandleRequest_001
 * @tc.name: HandleRequest
 * @tc.desc:
 */
HWTEST_F(ObexServerConnectActionTest, ObexServerConnectAction_HandleRequest_001, TestSize.Level1)
{
    HILOGI("ObexServerConnectAction_HandleRequest_001 start");
    bool result = actionInterface->HandleRequest(OBEX_OPCODE_GET);
    EXPECT_TRUE(result);
    actionInterface->SendReply(OBEX_HTTP_BAD_REQUEST);

    actionInterface->transport_->socketDevice_->mMaxRxPacketSize = 20;
    result = actionInterface->HandleRequest(OBEX_OPCODE_GET);
    EXPECT_TRUE(result);
    actionInterface->maxPacketLength_ = 5;
    actionInterface->SendReply(OBEX_HTTP_BAD_REQUEST);
    HILOGI("ObexServerConnectAction_HandleRequest_001 end");
}

class ObexServerDisconnectActionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
        actionInterface = std::make_unique<ObexServerDisconnectAction>(obexTransport, pbapPseObexServer);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: ObexServerDisconnectAction_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ObexServerDisconnectActionTest, ObexServerDisconnectAction_001, TestSize.Level1)
{
    HILOGI("ObexServerDisconnectAction_001 start");
    bool result = actionInterface->HandleRequest(OBEX_OPCODE_GET);
    EXPECT_TRUE(result);
    actionInterface->SendReply(OBEX_HTTP_USE_PROXY);

    actionInterface->maxPacketLength_ = 2;
    actionInterface->SendReply(OBEX_HTTP_USE_PROXY);
    HILOGI("ObexServerDisconnectAction_001 end");
}

class ObexServerSetPathActionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
        actionInterface = std::make_unique<ObexServerSetPathAction>(obexTransport, pbapPseObexServer);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: ObexServerSetPathAction_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ObexServerSetPathActionTest, ObexServerSetPathAction_001, TestSize.Level1)
{
    HILOGI("ObexServerSetPathAction_001 start");
    bool result = actionInterface->HandleRequest(OBEX_OPCODE_GET);
    EXPECT_TRUE(result);
    actionInterface->SendReply(OBEX_HTTP_TIMEOUT);

    actionInterface->maxPacketLength_ = 2;
    actionInterface->SendReply(OBEX_HTTP_USE_PROXY);
    HILOGI("ObexServerSetPathAction_001 end");
}

class ObexServerAbortActionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
        actionInterface = std::make_unique<ObexServerAbortAction>(obexTransport, pbapPseObexServer);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: ObexServerAbortAction_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ObexServerAbortActionTest, ObexServerAbortAction_001, TestSize.Level1)
{
    HILOGI("ObexServerAbortAction_001 start");
    bool result = actionInterface->HandleRequest(OBEX_OPCODE_GET);
    EXPECT_TRUE(result);
    actionInterface->SendReply(OBEX_HTTP_RESET);
    HILOGI("ObexServerAbortAction_001 end");
}

class ObexServerDefaultActionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
        actionInterface = std::make_unique<ObexServerDefaultAction>(obexTransport, pbapPseObexServer);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: ObexServerDefaultAction_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ObexServerDefaultActionTest, ObexServerDefaultAction_001, TestSize.Level1)
{
    HILOGI("ObexServerDefaultAction_001 start");
    bool result = actionInterface->HandleRequest(OBEX_OPCODE_GET);
    EXPECT_TRUE(result);
    actionInterface->SendReply(OBEX_HTTP_NOT_IMPLEMENTED);
    HILOGI("ObexServerDefaultAction_001 end");
}

class ObexServerGetActionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        std::shared_ptr<PbapPseObexServer> requestHandler_ =
            std::make_shared<PbapPseObexServer>(socketDevice->mDeviceAddress, nullptr);
        getAction = std::make_unique<ObexServerGetAction>(obexTransport, requestHandler_);
    }
    void TearDown()
    {}
public:
    std::unique_ptr<ObexServerGetAction> getAction;
};

/*
 * @tc.number: ObexServerGetAction_HandleRequest_001
 * @tc.name: HandleRequest
 * @tc.desc:
 */
HWTEST_F(ObexServerGetActionTest, ObexServerGetAction_HandleRequest_001, TestSize.Level1)
{
    HILOGI("ObexServerGetAction_HandleRequest_001 start");
    bool result = getAction->HandleRequest(OBEX_OPCODE_GET);
    EXPECT_FALSE(result);
    result = getAction->HandleRequest(OBEX_OPCODE_GET_FINAL);
    EXPECT_TRUE(result);
    getAction->SendReply(OBEX_HTTP_NOT_IMPLEMENTED);

    getAction->maxPacketLength_ = 10;
    getAction->hasSentLength_ = 2;
    getAction->SendReply(OBEX_HTTP_NOT_IMPLEMENTED);
    HILOGI("ObexServerGetAction_HandleRequest_001 end");
}

/*
 * @tc.number: ObexServerGetAction_HandleClientGetContinue_001
 * @tc.name: HandleClientGetContinue
 * @tc.desc:
 */
HWTEST_F(ObexServerGetActionTest, ObexServerGetAction_HandleClientGetContinue_001, TestSize.Level1)
{
    HILOGI("ObexServerGetAction_HandleClientGetContinue_001 start");
    getAction->srmEnabled_ = true;
    getAction->srmActive_ = true;
    bool result = getAction->HandleClientGetContinue();
    EXPECT_FALSE(result);

    getAction->hasSrmResponseSent_ = true;
    result = getAction->HandleClientGetContinue();
    EXPECT_FALSE(result);

    getAction->srmEnabled_ = false;
    getAction->hasSrmResponseSent_ = true;
    result = getAction->HandleClientGetContinue();
    EXPECT_FALSE(result);

    getAction->srmActive_ = false;
    getAction->hasSrmResponseSent_ = true;
    result = getAction->HandleClientGetContinue();
    EXPECT_FALSE(result);

    getAction->maxPacketLength_ = 2;
    result = getAction->HandleClientGetContinue();
    EXPECT_FALSE(result);
    HILOGI("ObexServerGetAction_HandleClientGetContinue_001 end");
}

/*
 * @tc.number: ObexServerGetAction_NeedSubcontractPackage_001
 * @tc.name: NeedSubcontractPackage
 * @tc.desc:
 */
HWTEST_F(ObexServerGetActionTest, ObexServerGetAction_NeedSubcontractPackage_001, TestSize.Level1)
{
    HILOGI("ObexServerGetAction_NeedSubcontractPackage_001 start");
    bool result = getAction->NeedSubcontractPackage();
    EXPECT_FALSE(result);

    getAction->maxPacketLength_ = 2;
    result = getAction->NeedSubcontractPackage();
    EXPECT_TRUE(result);
    HILOGI("ObexServerGetAction_NeedSubcontractPackage_001 end");
}

/*
 * @tc.number: ObexServerGetAction_SendSubcontractPackage_001
 * @tc.name: SendSubcontractPackage
 * @tc.desc:
 */
HWTEST_F(ObexServerGetActionTest, ObexServerGetAction_SendSubcontractPackage_001, TestSize.Level1)
{
    HILOGI("ObexServerGetAction_SendSubcontractPackage_001 start");
    getAction->SendSubcontractPackage();

    getAction->maxPacketLength_ = 15;
    getAction->SendSubcontractPackage();

    getAction->reply_.SetConnectionId(12);
    getAction->SendSubcontractPackage();

    getAction->maxPacketLength_ = 0;
    getAction->SendSubcontractPackage();
    EXPECT_TRUE(getAction != nullptr);
    HILOGI("ObexServerGetAction_SendSubcontractPackage_001 end");
}

/*
 * @tc.number: ObexServerGetAction_SendBigHeader_001
 * @tc.name: SendBigHeader
 * @tc.desc:
 */
HWTEST_F(ObexServerGetActionTest, ObexServerGetAction_SendBigHeader_001, TestSize.Level1)
{
    HILOGI("ObexServerGetAction_SendBigHeader_001 start");
    std::vector<uint8_t> headerValue;
    bool result = getAction->SendBigHeader(headerValue);
    EXPECT_TRUE(result);

    headerValue = {0x78};
    result = getAction->SendBigHeader(headerValue);
    EXPECT_FALSE(result);

    headerValue = {0x01, 0x00, 0x05, 0x00, 0x01};
    result = getAction->SendBigHeader(headerValue);
    EXPECT_TRUE(result);
    HILOGI("ObexServerGetAction_SendBigHeader_001 end");
}

/*
 * @tc.number: ObexServerGetAction_HandleSendReplyContinue_001
 * @tc.name: HandleSendReplyContinue
 * @tc.desc:
 */
HWTEST_F(ObexServerGetActionTest, ObexServerGetAction_HandleSendReplyContinue_001, TestSize.Level1)
{
    HILOGI("ObexServerGetAction_HandleSendReplyContinue_001 start");
    std::vector<uint8_t> sendData{};
    getAction->FillSendData(OBEX_HTTP_NO_CONTENT, 0, 0, sendData);
    bool result = getAction->HandleSendReplyContinue(true);
    EXPECT_TRUE(result);

    getAction->FillSendData(OBEX_HTTP_OK, 0, 0, sendData);
    result = getAction->HandleSendReplyContinue(false);
    EXPECT_FALSE(result);

    getAction->FillSendData(OBEX_OPCODE_ABORT, 0, 0, sendData);
    result = getAction->HandleSendReplyContinue(false);
    EXPECT_FALSE(result);
    HILOGI("ObexServerGetAction_HandleSendReplyContinue_001 end");
}

class ObexServerPutActionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        putAction = std::make_unique<ObexServerPutAction>(obexTransport, pbapPseObexServer);
    }
    void TearDown()
    {}
public:
    std::unique_ptr<ObexServerPutAction> putAction;
};

/*
 * @tc.number: ObexServerPutAction_HandleRequest_001
 * @tc.name: HandleRequest
 * @tc.desc:
 */
HWTEST_F(ObexServerPutActionTest, ObexServerPutAction_HandleRequest_001, TestSize.Level1)
{
    HILOGI("ObexServerPutAction_HandleRequest_001 start");
    EXPECT_FALSE(putAction->HandleRequest(OBEX_OPCODE_PUT));
    HILOGI("ObexServerPutAction_HandleRequest_001 end");
}

/*
 * @tc.number: ObexServerPutAction_HandleRequest_002
 * @tc.name: HandleRequest
 * @tc.desc:
 */
HWTEST_F(ObexServerPutActionTest, ObexServerPutAction_HandleRequest_002, TestSize.Level1)
{
    HILOGI("ObexServerPutAction_HandleRequest_002 start");
    EXPECT_TRUE(putAction->HandleRequest(OBEX_OPCODE_PUT_FINAL));
    HILOGI("ObexServerPutAction_HandleRequest_002 end");
}

/*
 * @tc.number: ObexServerPutAction_HandleRequest_003
 * @tc.name: HandleRequest
 * @tc.desc:
 */
HWTEST_F(ObexServerPutActionTest, ObexServerPutAction_HandleRequest_003, TestSize.Level1)
{
    HILOGI("ObexServerPutAction_HandleRequest_003 start");
    putAction->srmEnabled_ = true;
    EXPECT_TRUE(putAction->HandleRequest(OBEX_OPCODE_PUT_FINAL));
    HILOGI("ObexServerPutAction_HandleRequest_003 end");
}

/*
 * @tc.number: ObexServerPutAction_SendReply_001
 * @tc.name: SendReply
 * @tc.desc:
 */
HWTEST_F(ObexServerPutActionTest, ObexServerPutAction_SendReply_001, TestSize.Level1)
{
    HILOGI("ObexServerPutAction_SendReply_001 start");
    putAction->SendReply(OBEX_HTTP_NOT_IMPLEMENTED);

    putAction->srmEnabled_ = true;
    putAction->SendReply(OBEX_HTTP_NOT_IMPLEMENTED);

    putAction->hasSrmResponseSent_ = true;
    putAction->SendReply(OBEX_HTTP_NOT_IMPLEMENTED);

    putAction->srmEnabled_ = false;
    putAction->hasSrmResponseSent_ = true;
    putAction->SendReply(OBEX_HTTP_NOT_IMPLEMENTED);
    EXPECT_TRUE(putAction != nullptr);
    HILOGI("ObexServerPutAction_SendReply_001 end");
}

/*
 * @tc.number: ObexServerPutAction_HandleClientPutContinue_001
 * @tc.name: HandleClientPutContinue
 * @tc.desc:
 */
HWTEST_F(ObexServerPutActionTest, ObexServerPutAction_HandleClientPutContinue_001, TestSize.Level1)
{
    HILOGI("ObexServerPutAction_HandleClientPutContinue_001 start");
    bool result = putAction->HandleClientPutContinue();
    EXPECT_FALSE(result);

    putAction->maxPacketLength_ = 2;
    result = putAction->HandleClientPutContinue();
    EXPECT_FALSE(result);
    HILOGI("ObexServerPutAction_HandleClientPutContinue_001 end");
}

/*
 * @tc.number: ObexServerPutAction_ParseReplyContinueRequestType_001
 * @tc.name: ParseReplyContinueRequestType
 * @tc.desc:
 */
HWTEST_F(ObexServerPutActionTest, ObexServerPutAction_ParseReplyContinueRequestType_001, TestSize.Level1)
{
    HILOGI("ObexServerPutAction_ParseReplyContinueRequestType_001 start");
    uint8_t requestType = 0;
    uint32_t requestPacketLength = 0;
    bool result = putAction->ParseReplyContinueRequestType(requestType, requestPacketLength);
    EXPECT_FALSE(result);
}

/*
 * @tc.number: ObexServerPutAction_HandleSendReplyContinue_001
 * @tc.name: HandleSendReplyContinue
 * @tc.desc:
 */
HWTEST_F(ObexServerPutActionTest, ObexServerPutAction_HandleSendReplyContinue_001, TestSize.Level1)
{
    HILOGI("ObexServerPutAction_HandleSendReplyContinue_001 start");
    bool result = putAction->HandleSendReplyContinue(true);
    EXPECT_FALSE(result);
    result = putAction->HandleSendReplyContinue(false);
    EXPECT_FALSE(result);
}
}  // namespace bluetooth
}  // namespace OHOS