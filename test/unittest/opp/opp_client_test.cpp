/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "bluetooth_opp.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "btcommon/timer_manager.h"
#include "opp_obex_client.h"
#include "opp_def.h"
#include "obex_def.h"
#include "opp_service_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
using namespace std;

static string filePath = "/data/service/el1/public/bluetooth/bluetooth_audio_manager_config.xml";
static BluetoothHost *host_;
constexpr int TIME = 2;
constexpr int MTU = 300;
constexpr int FILELEN = 5;
static std::unique_ptr<OppObexClient> oppClient_ {nullptr};

class OppClientTest : public testing::Test {
public:
    OppClientTest();
    ~OppClientTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
};

OppClientTest::OppClientTest()
{
}

OppClientTest::~OppClientTest()
{
}

void OppClientTest::SetUpTestCase(void)
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

void OppClientTest::TearDownTestCase(void)
{
}

void OppClientTest::SetUp()
{
    std::shared_ptr<ObexClientConfig> obexConfig = std::make_shared<ObexClientConfig>();
    obexConfig->addr = "11:22:33:44:55:66";
    obexConfig->l2capLocalPsm = OPP_L2CAP_PSM;
    obexConfig->l2capPsm = OPP_L2CAP_PSM;
    obexConfig->rfCommChannel = OPP_RFCOMM_CHANNEL;
    obexConfig->serviceUUID = "00001105-0000-1000-8000-00805F9B34FB";
    oppClient_ = std::make_unique<OppObexClient>(obexConfig, nullptr);
    auto device = std::make_shared<ObexSocketDevice>(-1, 0, 0, SocketType::TYPE_RFCOMM, "11:22:33:44:55:66");
    std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(device);
    oppClient_->clientSession_ = std::make_shared<ObexClientSession>(transport, nullptr);
}

void OppClientTest::TearDown()
{
    oppClient_ = nullptr;
}

/*
 * @tc.number: OPP_ModuleTest_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(OppClientTest, OPP_ModuleTest_Connect_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_Connect_00100 start");
    oppClient_->Connect(nullptr);
    oppClient_->Disconnect(true);
    oppClient_->DisconnectLocal(false);
    EXPECT_TRUE(oppClient_->clientSession_ == nullptr);
    HILOGI("OPP_ModuleTest_Connect_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_RecvSocketDevice_00100
 * @tc.name: RecvSocketDevice
 * @tc.desc:
 */
HWTEST_F(OppClientTest, OPP_ModuleTest_RecvSocketDevice_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_RecvSocketDevice_00100 start");
    EXPECT_EQ(oppClient_->RecvSocketDevice(SocketType::TYPE_L2CAP), nullptr);
    HILOGI("OPP_ModuleTest_RecvSocketDevice_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_SendFileHeader_00100
 * @tc.name: SendFileHeader
 * @tc.desc:
 */
HWTEST_F(OppClientTest, OPP_ModuleTest_SendFileHeader_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_SendFileHeader_00100 start");
    oppClient_->SendFileHeader(nullptr, nullptr, nullptr);
    std::shared_ptr<IOppTransferInformation> fileInfo = std::make_shared<IOppTransferInformation>();
    fileInfo->SetTotalBytes(1);
    fileInfo->SetFileName("test.jpg");
    fileInfo->SetFileType("jpg");
    oppClient_->SendFileHeader(nullptr, fileInfo, nullptr);
    EXPECT_EQ(fileInfo->GetTotalBytes(), 1);
    HILOGI("OPP_ModuleTest_SendFileHeader_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_OnPutDateAvailableResp_00100
 * @tc.name: OnPutDateAvailableResp
 * @tc.desc:
 */
HWTEST_F(OppClientTest, OPP_ModuleTest_OnPutDateAvailableResp_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_OnPutDateAvailableResp_00100 start");
    ObexHeader obexHeader;
    obexHeader.SetConnectionId(1);
    uint8_t responseCode = ResponseCodes::OBEX_HTTP_OK;
    oppClient_->OnPutDateAvailableResp(obexHeader, responseCode);
    EXPECT_FALSE(oppClient_->clientSession_->IsBusy());
    HILOGI("OPP_ModuleTest_OnPutDateAvailableResp_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_SetBusy_00100
 * @tc.name: SetBusy
 * @tc.desc:
 */
HWTEST_F(OppClientTest, OPP_ModuleTest_SetBusy_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_SetBusy_00100 start");
    oppClient_->SetBusy(true);
    EXPECT_TRUE(oppClient_->clientSession_->IsBusy());
    oppClient_->SetBusy(false);
    EXPECT_FALSE(oppClient_->clientSession_->IsBusy());
    HILOGI("OPP_ModuleTest_SetBusy_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_CancelSendFile_00100
 * @tc.name: CancelSendFile
 * @tc.desc:
 */
HWTEST_F(OppClientTest, OPP_ModuleTest_CancelSendFile_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_CancelSendFile_00100 start");
    ObexHeader obexHeader;
    obexHeader.SetConnectionId(1);
    oppClient_->OnActionCompleted(obexHeader, ResponseCodes::OBEX_HTTP_OK);
    oppClient_->OnActionCompleted(obexHeader, ResponseCodes::OBEX_HTTP_FORBIDDEN);
    oppClient_->OnActionCompleted(obexHeader, ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE);
    oppClient_->OnBusy(true);
    oppClient_->OnBusy(false);
    EXPECT_EQ(oppClient_->CancelSendFile(false), RET_NO_ERROR);
    HILOGI("OPP_ModuleTest_CancelSendFile_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_Abort_00100
 * @tc.name: Abort
 * @tc.desc:
 */
HWTEST_F(OppClientTest, OPP_ModuleTest_Abort_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_Abort_00100 start");
    oppClient_->Abort();
    oppClient_->SendAbortRequest();
    EXPECT_EQ(oppClient_->CheckBeforeRequest(1), false);
    HILOGI("OPP_ModuleTest_Abort_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_SendFileOneBody_00100
 * @tc.name: SendFileOneBody
 * @tc.desc:
 */
HWTEST_F(OppClientTest, OPP_ModuleTest_SendFileOneBody_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_SendFileOneBody_00100 start");
    ObexHeader obexHeader;
    obexHeader.SetResponseCode(ResponseCodes::OBEX_HTTP_CONTINUE);
    OppServiceImpl *oppServiceImpl = new OppServiceImpl;
    std::shared_ptr<ObexClientConfig> obexConfig = std::make_shared<ObexClientConfig>();
    obexConfig->addr = "11:22:33:44:55:66";
    obexConfig->l2capLocalPsm = OPP_L2CAP_PSM;
    obexConfig->l2capPsm = OPP_L2CAP_PSM;
    obexConfig->rfCommChannel = OPP_RFCOMM_CHANNEL;
    obexConfig->serviceUUID = "00001105-0000-1000-8000-00805F9B34FB";
    std::unique_ptr<OppObexClient> oppClient = std::make_unique<OppObexClient>(obexConfig, oppServiceImpl);
    oppClient->SendFileOneBody();
    oppClient->Disconnect(false);
    uint8_t responseCode = obexHeader.GetResponseCode();
    EXPECT_EQ(responseCode, static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_CONTINUE));
    HILOGI("OPP_ModuleTest_SendFileOneBody_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
