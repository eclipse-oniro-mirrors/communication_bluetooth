/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <memory>
#include <string>
#include <vector>

#include "btcommon/timer_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "avrcp_tg_bip_obex_server.h"
#include "avrcp_tg_service_manager.h"
#include "cover_art.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
static BluetoothHost *host_;
class AvrcpBipObexServerTest : public testing::Test {
public:
    AvrcpBipObexServerTest();
    ~AvrcpBipObexServerTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {}
    void TearDown() {}

private:
    std::string deviceAddr_ = "11:22:33:44:55:66";
    std::shared_ptr<BipService> bipService = {};
    std::weak_ptr<BipService> serviceWeak_ = bipService;
    std::shared_ptr<AvrcpBipObexServer> server_ = nullptr;
};

AvrcpBipObexServerTest::AvrcpBipObexServerTest()
{
    server_ = std::make_shared<AvrcpBipObexServer>(deviceAddr_, serviceWeak_);
}

AvrcpBipObexServerTest::~AvrcpBipObexServerTest() {}

void AvrcpBipObexServerTest::SetUpTestCase(void)
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

void AvrcpBipObexServerTest::TearDownTestCase(void) {
    host_ = nullptr;
}

/*
 * @tc.number: AvrcpBipObexServerTest_OnConnect_001
 * @tc.name: OnConnect
 * @tc.desc: Test OnConnect with valid UUID
 */
HWTEST_F(AvrcpBipObexServerTest, AvrcpBipObexServerTest_OnConnect_001, TestSize.Level1)
{
    HILOGI("AvrcpBipObexServerTest_OnConnect_001 start");
    ObexHeader request;
    ObexHeader reply;

    std::vector<uint8_t> uuid1 = {
        0x71, 0x63, 0xDD, 0x54, 0x4A, 0x7E, 0x11, 0xE2,
        0xB4, 0x7C, 0x00, 0x50, 0xC2, 0x49, 0x00,
    };
    request.SetHeader(TARGET, uuid1);
    EXPECT_EQ(server_->OnConnect(request, reply), ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE);

    std::vector<uint8_t> uuid2 = {
        0x71, 0x63, 0xDD, 0x54, 0x4A, 0x7E, 0x11, 0xE2,
        0xB4, 0x7C, 0x00, 0x50, 0xC2, 0x49, 0x00, 0x00
    };
    request.SetHeader(TARGET, uuid2);
    EXPECT_EQ(server_->OnConnect(request, reply), ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE);


    std::vector<uint8_t> uuid3 = {
        0x71, 0x63, 0xDD, 0x54, 0x4A, 0x7E, 0x11, 0xE2,
        0xB4, 0x7C, 0x00, 0x50, 0xC2, 0x49, 0x00, 0x48
    };
    request.SetHeader(TARGET, uuid3);
    EXPECT_EQ(server_->OnConnect(request, reply), ResponseCodes::OBEX_HTTP_OK);
    HILOGI("AvrcpBipObexServerTest_OnConnect_001 end");
}

/*
 * @tc.number: AvrcpBipObexServerTest_OnDisconnect_001
 * @tc.name: OnDisconnect
 * @tc.desc: Test OnDisconnect
 */
HWTEST_F(AvrcpBipObexServerTest, AvrcpBipObexServerTest_OnDisconnect_001, TestSize.Level1)
{
    HILOGI("AvrcpBipObexServerTest_OnDisconnect_001 start");
    ObexHeader request;
    ObexHeader reply;
    int result = server_->OnDisconnect(request, reply);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("AvrcpBipObexServerTest_OnDisconnect_001 end");
}

/*
 * @tc.number: AvrcpBipObexServerTest_OnPut_001
 * @tc.name: OnPut
 * @tc.desc: Test OnPut returns NOT_IMPLEMENTED
 */
HWTEST_F(AvrcpBipObexServerTest, AvrcpBipObexServerTest_OnPut_001, TestSize.Level1)
{
    HILOGI("AvrcpBipObexServerTest_OnPut_001 start");
    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> input;
    bool requestFinished = false;

    int result = server_->OnPut(request, reply, input, requestFinished);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED);
    HILOGI("AvrcpBipObexServerTest_OnPut_001 end");
}

/*
 * @tc.number: AvrcpBipObexServerTest_OnAbort_001
 * @tc.name: OnAbort
 * @tc.desc: Test OnAbort returns NOT_IMPLEMENTED
 */
HWTEST_F(AvrcpBipObexServerTest, AvrcpBipObexServerTest_OnAbort_001, TestSize.Level1)
{
    HILOGI("AvrcpBipObexServerTest_OnAbort_001 start");
    ObexHeader request;
    ObexHeader reply;
    int result = server_->OnAbort(request, reply);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED);
    HILOGI("AvrcpBipObexServerTest_OnAbort_001 end");
}

/*
 * @tc.number: AvrcpBipObexServerTest_OnSetPath_001
 * @tc.name: OnSetPath
 * @tc.desc: Test OnSetPath returns NOT_IMPLEMENTED
 */
HWTEST_F(AvrcpBipObexServerTest, AvrcpBipObexServerTest_OnSetPath_001, TestSize.Level1)
{
    HILOGI("AvrcpBipObexServerTest_OnSetPath_001 start");
    ObexHeader request;
    ObexHeader reply;

    int result = server_->OnSetPath(request, reply, true, false);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED);

    result = server_->OnSetPath(request, reply, false, true);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED);
    HILOGI("AvrcpBipObexServerTest_OnSetPath_001 end");
}

/*
 * @tc.number: AvrcpBipObexServerTest_OnDelete_001
 * @tc.name: OnDelete
 * @tc.desc: OnDelete
 */
HWTEST_F(AvrcpBipObexServerTest, AvrcpBipObexServerTest_OnDelete_001, TestSize.Level1)
{
    HILOGI("AvrcpBipObexServerTest_OnDelete_001 start");
    ObexHeader request;
    ObexHeader reply;

    int result = server_->OnDelete(request, reply);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("AvrcpBipObexServerTest_OnDelete_001 end");
}

/*
 * @tc.number: AvrcpBipObexServerTest_SetConnectionId_001
 * @tc.name: SetConnectionId
 * @tc.desc: SetConnectionId
 */
HWTEST_F(AvrcpBipObexServerTest, AvrcpBipObexServerTest_SetConnectionId_001, TestSize.Level1)
{
    HILOGI("AvrcpBipObexServerTest_SetConnectionId_001 start");
    server_->SetConnectionId(123);
    EXPECT_EQ(server_->GetConnectionId(), 123);
    HILOGI("AvrcpBipObexServerTest_SetConnectionId_001 end");
}

/*
 * @tc.number: AvrcpBipObexServerTest_OnGet_001
 * @tc.name: OnGet
 * @tc.desc: Test OnGet with thumbnail type (x-bt/img-thm)
 */
HWTEST_F(AvrcpBipObexServerTest, AvrcpBipObexServerTest_OnGet_001, TestSize.Level1)
{
    HILOGI("AvrcpBipObexServerTest_OnGet_001 start");
    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> output;
    bool sendBodyHeader = false;

    std::string type1 = "x-bt/img-thm";
    std::vector<uint8_t> typeValue1(type1.begin(), type1.end());
    request.SetHeader(TYPE, typeValue1);
    int result = server_->OnGet(request, reply, output, sendBodyHeader);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_BAD_REQUEST);

    std::string type2 = "x-bt/img-properties";
    std::vector<uint8_t> typeValue2(type2.begin(), type2.end());
    request.SetHeader(TYPE, typeValue2);
    result = server_->OnGet(request, reply, output, sendBodyHeader);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_BAD_REQUEST);

    std::string type3 = "x-bt/img-img";
    std::vector<uint8_t> typeValue3(type3.begin(), type3.end());
    request.SetHeader(TYPE, typeValue3);
    result = server_->OnGet(request, reply, output, sendBodyHeader);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_BAD_REQUEST);

    std::string type4 = "unknown/type";
    std::vector<uint8_t> typeValue4(type4.begin(), type4.end());
    request.SetHeader(TYPE, typeValue4);
    result = server_->OnGet(request, reply, output, sendBodyHeader);
    EXPECT_EQ(result, ResponseCodes::OBEX_HTTP_BAD_REQUEST);
    HILOGI("AvrcpBipObexServerTest_OnGet_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS