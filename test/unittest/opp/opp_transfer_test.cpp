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
#include "bluetooth_errorcode.h"
#include "bluetooth_def.h"
#include <gtest/gtest.h>
#include "opp_def.h"
#include "opp_obex_client.h"
#include "opp_service_impl.h"
#include "opp_transfer.h"

using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
class OppTransferTest : public testing::Test {
public:
    OppTransferTest();
    ~OppTransferTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
    std::shared_ptr<bluetooth::OppTransfer> oppTransfer_ = nullptr;
    std::shared_ptr<ObexClientConfig> obexConfig_ = nullptr;
};

OppTransferTest::OppTransferTest()
{
    std::vector<bluetooth::IOppTransferFileHolder> fileHolders;
    fileHolders.push_back(bluetooth::IOppTransferFileHolder("test.jpg", 1, 1));
    oppTransfer_ = std::make_shared<bluetooth::OppTransfer>("11:22:33:44:55:66", fileHolders, 0, nullptr, nullptr);
    obexConfig_ = std::make_shared<ObexClientConfig>();
    obexConfig_->addr = "11:22:33:44:55:66";
    obexConfig_->l2capLocalPsm = OPP_L2CAP_PSM;
    obexConfig_->l2capPsm = OPP_L2CAP_PSM;
    obexConfig_->rfCommChannel = OPP_RFCOMM_CHANNEL;
    obexConfig_->serviceUUID = "OPP_UUID16";
}

OppTransferTest::~OppTransferTest()
{
}

void OppTransferTest::SetUpTestCase(void)
{
}

void OppTransferTest::TearDownTestCase(void)
{
    utility::CloseAllTimer();
}

void OppTransferTest::SetUp()
{
}

void OppTransferTest::TearDown()
{
}

/*
 * @tc.number: Opp_Transfer_StartTransfer_00100
 * @tc.name: StartTransfer
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_StartTransfer_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_StartTransfer_00100 start");
    int ret = oppTransfer_->StartTransfer();
    EXPECT_EQ(ret, RET_BAD_STATUS);
    HILOGI("Opp_Transfer_StartTransfer_00100 end");
}

/*
 * @tc.number: Opp_Transfer_OnReceiveIncomingConnect_00100
 * @tc.name: OnReceiveIncomingConnect
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_OnReceiveIncomingConnect_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_OnReceiveIncomingConnect_00100 start");
    oppTransfer_->OnReceiveIncomingConnect(1, 1);

    bluetooth::IOppTransferInformation info;
    int ret = oppTransfer_->OnReceiveIncomingFile(info);
    EXPECT_EQ(ret, 0xC0);
    HILOGI("Opp_Transfer_OnReceiveIncomingConnect_00100 end");
}

/*
 * @tc.number: Opp_Transfer_IncomingConnectTimeout_00100
 * @tc.name: IncomingConnectTimeout
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_IncomingConnectTimeout_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_IncomingConnectTimeout_00100 start");
    EXPECT_EQ(oppTransfer_->GetDirection(), 0);
    HILOGI("Opp_Transfer_IncomingConnectTimeout_00100 end");
}

/*
 * @tc.number: Opp_Transfer_ConnectObex_00100
 * @tc.name: ConnectObex
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_ConnectObex_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_ConnectObex_00100 start");
    std::vector<bluetooth::IOppTransferFileHolder> fileHolders;
    fileHolders.push_back(bluetooth::IOppTransferFileHolder("test.jpg", 1, 1));
    OppServiceImpl *oppServiceImpl = new OppServiceImpl;
    std::shared_ptr<bluetooth::OppTransfer> oppTransfer =
        std::make_shared<bluetooth::OppTransfer>("11:22:33:44:55:66", fileHolders, 0, nullptr, oppServiceImpl);
    EXPECT_EQ(oppTransfer->ConnectObex(obexConfig_), 0);
    HILOGI("Opp_Transfer_ConnectObex_00100 end");
}

/*
 * @tc.number: Opp_Transfer_OnObexDisConnected_00100
 * @tc.name: OnObexDisConnected()
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_OnObexDisConnected_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_OnObexDisConnected_00100 start");
    std::vector<bluetooth::IOppTransferFileHolder> fileHolders;
    fileHolders.push_back(bluetooth::IOppTransferFileHolder("test.jpg", 1, 1));
    OppServiceImpl *oppServiceImpl = new OppServiceImpl;
    std::shared_ptr<bluetooth::OppTransfer> oppTransfer =
        std::make_shared<bluetooth::OppTransfer>("11:22:33:44:55:66", fileHolders, 1, nullptr, oppServiceImpl);
    bluetooth::IOppTransferInformation info;
    oppTransfer->OnReceiveIncomingConnect(0, UNKNOWN_OBEX_VALUE);
    oppTransfer->OnObexDisconnected();
    EXPECT_EQ(oppTransfer->AcceptConnect(), true);
    HILOGI("Opp_Transfer_OnObexDisConnected_00100 end");
}

/*
 * @tc.number: Opp_Transfer_CancelTransfer_00100
 * @tc.name: CancelTransfer()
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_CancelTransfer_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_CancelTransfer_00100 start");
    std::vector<bluetooth::IOppTransferFileHolder> fileHolders;
    fileHolders.push_back(bluetooth::IOppTransferFileHolder("test.jpg", 1, 1));
    OppServiceImpl *oppServiceImpl = new OppServiceImpl;
    std::shared_ptr<bluetooth::OppTransfer> oppTransfer =
        std::make_shared<bluetooth::OppTransfer>("11:22:33:44:55:66", fileHolders, 1, nullptr, oppServiceImpl);
    bluetooth::IOppTransferInformation info;
    int ret = oppTransfer->OnReceiveIncomingFile(info);
    EXPECT_EQ(ret, 0);
    oppTransfer->OnTransferStateChange(OPP_TRANSFER_STATUS_PENDING, 1);
    oppTransfer->OnTransferStateChange(OPP_TRANSFER_STATUS_FINISH, 0);
    oppTransfer->OnTransferStateChange(OPP_TRANSFER_STATUS_FINISH, 1);
    oppTransfer->OnTransferStateChange(OPP_TRANSFER_STATUS_RUNNING, 1);
    oppTransfer->IncomingConnectTimeout();
    oppTransfer->IncomingFileTimeout();
    EXPECT_EQ(oppTransfer->CancelTransfer(), RET_BAD_STATUS);
    HILOGI("Opp_Transfer_CancelTransfer_00100 end");
}

/*
 * @tc.number: Opp_Transfer_DisConnectObex_00100
 * @tc.name: DisConnectObex
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_DisConnectObex_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_DisConnectObex_00100 start");
    std::vector<bluetooth::IOppTransferFileHolder> fileHolders;
    fileHolders.push_back(bluetooth::IOppTransferFileHolder("test.jpg", 1, 1));
    OppServiceImpl *oppServiceImpl = new OppServiceImpl;
    std::shared_ptr<bluetooth::OppTransfer> oppTransfer =
        std::make_shared<bluetooth::OppTransfer>("11:22:33:44:55:66", fileHolders, 1, nullptr, oppServiceImpl);
    oppTransfer->DisconnectObex();
    EXPECT_EQ(oppTransfer->StartTransfer(), RET_NO_ERROR);
    HILOGI("Opp_Transfer_DisConnectObex_00100 end");
}

/*
 * @tc.number: Opp_Transfer_GetOppTransferInformation_00100
 * @tc.name: GetOppTransferInformation
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_GetOppTransferInformation_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_GetOppTransferInformation_00100 start");
    std::vector<bluetooth::IOppTransferFileHolder> fileHolders;
    OppServiceImpl *oppServiceImpl = new OppServiceImpl;
    std::shared_ptr<bluetooth::OppTransfer> oppTransfer =
        std::make_shared<bluetooth::OppTransfer>("11:22:33:44:55:66", fileHolders, 1, nullptr, oppServiceImpl);
    bluetooth::IOppTransferInformation info;
    std::shared_ptr infoPtr = oppTransfer->GetOppTransferInformation(info);
    EXPECT_TRUE(infoPtr != nullptr);
    HILOGI("Opp_Transfer_GetOppTransferInformation_00100 end");
}

/*
 * @tc.number: Opp_Transfer_OppTransfer_00100
 * @tc.name: OppTransfer
 * @tc.desc:
 */
HWTEST_F(OppTransferTest, Opp_Transfer_OppTransfer_00100, TestSize.Level1)
{
    HILOGI("Opp_Transfer_OppTransfer_00100 start");
    std::vector<bluetooth::IOppTransferFileHolder> fileHolders;
    fileHolders.push_back(bluetooth::IOppTransferFileHolder("large.jpg", 0xFFFFFFFFFF, 1));
    OppServiceImpl *oppServiceImpl = new OppServiceImpl;
    std::shared_ptr<bluetooth::OppTransfer> oppTransfer =
        std::make_shared<bluetooth::OppTransfer>("11:22:33:44:55:66", fileHolders, 1, nullptr, oppServiceImpl);
    EXPECT_TRUE(oppTransfer != nullptr);
    HILOGI("Opp_Transfer_OppTransfer_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS