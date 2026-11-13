/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>
#include <cstdint>
#include <optional>
#include <securec.h>
#include <string>
#include "map_mse_obex_server.h"
#include "map_mse_masinstance.h"
#include "map_mse_service.h"
#include "obex_header.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "class_creator.h"

namespace OHOS {
namespace bluetooth {
using namespace testing;
using namespace testing::ext;
using namespace Bluetooth;
const std::vector<uint8_t> MAP_TARGET = {
    0xBB, 0x58, 0x2B, 0x40, 0x42, 0x0C, 0x11, 0xDB, 0xB0, 0xDE, 0x08, 0x00, 0x20, 0x0C, 0x9A, 0x66};
constexpr int TIME = 4;
static BluetoothHost *host_;
void *MapMseServiceCreate(void)
{
    return new MapMseService();
}
class MapMseObexServerTest : public testing::Test {
public:
    MapMseObexServerTest() {}
    ~MapMseObexServerTest() {}
    static void SetUpTestCase()
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
        ClassFactory::RegisterClass("MapMseService", MapMseServiceCreate);
    }
    static void TearDownTestCase()
    {
        host_->DisableBle();
        host_->DisableBt();
        sleep(TIME);
    }
    void SetUp(){};
    void TearDown(){};
};

HWTEST_F(MapMseObexServerTest, MapMseObexServerTest_UnitTest_OnConnect_001, TestSize.Level1)
{
    std::shared_ptr<MapMseMasInstance> masInstance = nullptr;
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    std::shared_ptr<MapMseObexServer> obexServer = std::make_shared<MapMseObexServer>(masInstance, mnsClient, nullptr);
    ObexHeader request;
    ObexHeader reply;
    // uuid.empty()
    EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE, obexServer->OnConnect(request, reply));

    // uuid.size() != UUID_LENGTH
    std::vector<uint8_t> invalidTarget(MAP_TARGET.begin(), MAP_TARGET.end() - 2);
    request.SetHeader(TARGET, invalidTarget);
    EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE, obexServer->OnConnect(request, reply));

    // uuid[i] != MAP_TARGET[i])
    std::vector<uint8_t> wrongTarget(MAP_TARGET.begin(), MAP_TARGET.end());
    wrongTarget[7] = 0xFF;
    request.SetHeader(TARGET, invalidTarget);
    EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE, obexServer->OnConnect(request, reply));

    // threadedMailKeyVec.empty()
    request.SetHeader(TARGET, MAP_TARGET);
    obexServer->OnConnect(request, reply);

    // threadedMailKeyVec.size() < sizeof(threadedMailKey)
    std::vector<uint8_t> threadedMailKeyVec{0x01, 0x01, 0x01};
    request.SetHeader(THREADED_MAIL_HEADER_ID, threadedMailKeyVec);
    obexServer->OnConnect(request, reply);

    // threadedMailKeyVec.size() >= sizeof(threadedMailKey)
    threadedMailKeyVec.push_back(0x01);
    threadedMailKeyVec.push_back(0x01);
    request.SetHeader(THREADED_MAIL_HEADER_ID, threadedMailKeyVec);
    obexServer->OnConnect(request, reply);

    // !remote.empty()
    std::vector<uint8_t> who{0x01, 0x02};
    request.SetHeader(WHO, who);
    obexServer->OnConnect(request, reply);

    std::vector<uint8_t> threadedMailKey(4, 0);
    memcpy_s(threadedMailKeyVec.data(), threadedMailKey.size(), &THREAD_MAIL_KEY, sizeof(THREAD_MAIL_KEY));
    obexServer->OnConnect(request, reply);
}

HWTEST_F(MapMseObexServerTest, MapMseObexServerTest_UnitTest_OnPut_001, TestSize.Level1)
{
    std::shared_ptr<MapMseMasInstance> masInstance = std::make_shared<MapMseMasInstance>(0, nullptr);
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    std::shared_ptr<MapMseObexServer> obexServer = std::make_shared<MapMseObexServer>(masInstance, mnsClient, nullptr);
    std::vector<uint8_t> input{};
    bool sendBodyHeader = false;
    ObexHeader request;
    ObexHeader reply;
    // type.empty()
    EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnPut(request, reply, input, sendBodyHeader));
    std::string typeStr = "x-obex/folder-listing";
    {
        // type == TYPE_GET_FOLDER_LISTING
        std::vector<uint8_t> type(typeStr.begin(), typeStr.end());
        request.SetHeader(TYPE, type);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED, obexServer->OnPut(request, reply, input, sendBodyHeader));

        std::vector<uint8_t> appParamsRaw = {0x01, 0x02, 0x00, 0x00};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED, obexServer->OnPut(request, reply, input, sendBodyHeader));
    }

    {
        // type == TYPE_GET_MESSAGE_LISTING
        typeStr = "x-bt/MAP-msg-listing";
        std::vector<uint8_t> type(typeStr.begin(), typeStr.end());
        request.SetHeader(TYPE, type);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED, obexServer->OnPut(request, reply, input, sendBodyHeader));

        std::vector<uint8_t> appParamsRaw = {0x01, 0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED, obexServer->OnPut(request, reply, input, sendBodyHeader));
    }

    {
        // type == TYPE_GET_CONVO_LISTING
        typeStr = "x-bt/MAP-convo-listing";
        std::vector<uint8_t> type(typeStr.begin(), typeStr.end());
        request.SetHeader(TYPE, type);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED, obexServer->OnPut(request, reply, input, sendBodyHeader));

        std::vector<uint8_t> appParamsRaw = {0x01, 0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED, obexServer->OnPut(request, reply, input, sendBodyHeader));
    }
}

HWTEST_F(MapMseObexServerTest, MapMseObexServerTest_UnitTest_OnGet_001, TestSize.Level1)
{
    std::shared_ptr<MapMseMasInstance> masInstance = std::make_shared<MapMseMasInstance>(0, nullptr);
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    std::shared_ptr<MapMseObexServer> obexServer = std::make_shared<MapMseObexServer>(masInstance, mnsClient, nullptr);
    std::vector<uint8_t> output{};
    bool sendBodyHeader = false;
    ObexHeader request;
    ObexHeader reply;
    // type.empty()
    EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnGet(request, reply, output, sendBodyHeader));
    std::string typeStr = "x-obex/folder-listing";
    {
        // type == TYPE_GET_FOLDER_LISTING
        std::vector<uint8_t> type(typeStr.begin(), typeStr.end());
        request.SetHeader(TYPE, type);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnGet(request, reply, output, sendBodyHeader));

        std::vector<uint8_t> appParamsRaw = {0x01, 0x02, 0x00, 0x00};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnGet(request, reply, output, sendBodyHeader));
    }

    {
        // type == TYPE_GET_MESSAGE_LISTING
        typeStr = "x-bt/MAP-msg-listing";
        std::vector<uint8_t> type(typeStr.begin(), typeStr.end());
        request.SetHeader(TYPE, type);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnGet(request, reply, output, sendBodyHeader));

        std::vector<uint8_t> appParamsRaw = {0x01, 0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnGet(request, reply, output, sendBodyHeader));
    }

    {
        // type == TYPE_GET_CONVO_LISTING
        typeStr = "x-bt/MAP-convo-listing";
        std::vector<uint8_t> type(typeStr.begin(), typeStr.end());
        request.SetHeader(TYPE, type);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnGet(request, reply, output, sendBodyHeader));

        std::vector<uint8_t> appParamsRaw = {0x01, 0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnGet(request, reply, output, sendBodyHeader));
    }
}

HWTEST_F(MapMseObexServerTest, MapMseObexServerTest_UnitTest_OnGet_002, TestSize.Level1)
{
    std::shared_ptr<MapMseMasInstance> masInstance = std::make_shared<MapMseMasInstance>(0, nullptr);
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    std::shared_ptr<MapMseObexServer> obexServer = std::make_shared<MapMseObexServer>(masInstance, mnsClient, nullptr);
    obexServer->UpdateMnsClient(mnsClient);
    std::vector<uint8_t> output{};
    bool sendBodyHeader = false;
    ObexHeader request;
    ObexHeader reply;
    // type.empty()
    EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnGet(request, reply, output, sendBodyHeader));
    std::string typeStr = "x-obex/folder-listing";

    {
        // type == TYPE_GET_MAS_INSTANCE_INFORMATION
        typeStr = "x-bt/MASInstanceInformation";
        std::vector<uint8_t> type(typeStr.begin(), typeStr.end());
        request.SetHeader(TYPE, type);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnGet(request, reply, output, sendBodyHeader));

        std::vector<uint8_t> appParamsRaw = {0x0F, 0x01, 0x01};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnGet(request, reply, output, sendBodyHeader));

        appParamsRaw = {0x0F, 0x01, 0x00};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnGet(request, reply, output, sendBodyHeader));
    }

    {
        // type == TYPE_MESSAGE
        typeStr = "x-bt/message";
        std::vector<uint8_t> type(typeStr.begin(), typeStr.end());
        request.SetHeader(TYPE, type);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnGet(request, reply, output, sendBodyHeader));

        std::vector<uint8_t> appParamsRaw = {0x15, 0x01, 0x00};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnGet(request, reply, output, sendBodyHeader));

        appParamsRaw = {0x0F, 0x01, 0x01};
        request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnGet(request, reply, output, sendBodyHeader));

        std::vector<uint8_t> name{'0', '1', '2', '3'};
        request.SetHeader(NAME, name);
        EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnGet(request, reply, output, sendBodyHeader));
    }
}

HWTEST_F(MapMseObexServerTest, MapMseObexServerTest_UnitTest_OnSetPath_001, TestSize.Level1)
{
    std::shared_ptr<MapMseMasInstance> masInstance = std::make_shared<MapMseMasInstance>(0, nullptr);
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    std::shared_ptr<MapMseObexServer> obexServer = std::make_shared<MapMseObexServer>(masInstance, mnsClient, nullptr);
    bool backup = true;
    bool create = false;
    ObexHeader request;
    ObexHeader reply;
    EXPECT_EQ(ResponseCodes::OBEX_HTTP_BAD_REQUEST, obexServer->OnSetPath(request, reply, backup, create));
    backup = false;
    EXPECT_EQ(ResponseCodes::OBEX_HTTP_OK, obexServer->OnSetPath(request, reply, backup, create));
}
}  // namespace bluetooth
}  // namespace OHOS