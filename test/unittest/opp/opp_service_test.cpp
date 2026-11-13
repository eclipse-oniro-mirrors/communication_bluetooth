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
#include <cstring>
#include "opp_service.h"
#include "opp_message.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"
#include "class_creator.h"
#include "classic_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {
using namespace std;
const int TIME = 4;
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
void *OppServiceCreate(void)
{
    return new OppService();
}

class OppServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
    std::string addr_ = "11:22:33:44:55:66";
    std::shared_ptr<OppService> oppService_ = nullptr;
};

void OppServiceTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    ClassFactory::RegisterClass("OppService", OppServiceCreate);
    HILOGI("SetUpTestCase end");
}

void OppServiceTest::TearDownTestCase(void)
{
}

void OppServiceTest::SetUp()
{
    oppService_ = std::make_unique<OppService>();
}

void OppServiceTest::TearDown()
{
}

/*
 * @tc.number: Opp_Service_Connect_00001
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(OppServiceTest, Opp_Service_Connect_00001, TestSize.Level1)
{
    HILOGI("Opp_Service_Connect_00001 start");
    RawAddress addr = RawAddress(addr_);
    int32_t ret = oppService_->Connect(addr);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Opp_Service_Connect_00001 end");
}

/*
 * @tc.number: Opp_Service_Disconnect_00001
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(OppServiceTest, Opp_Service_Disconnect_00001, TestSize.Level1)
{
    HILOGI("Opp_Service_Disconnect_00001 start");
    RawAddress addr = RawAddress(addr_);
    int32_t ret = oppService_->Disconnect(addr);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Opp_Service_Disconnect_00001 end");
}

/*
 * @tc.number: Opp_Service_GetConnectState_00001
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(OppServiceTest, Opp_Service_GetConnectState_00001, TestSize.Level1)
{
    HILOGI("Opp_Service_GetConnectState_00001 start");
    oppService_->GetConnectDevices();
    int32_t ret = oppService_->GetConnectState();
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Opp_Service_GetConnectState_00001 end");
}

/*
 * @tc.number: Opp_Service_Enable_00001
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(OppServiceTest, Opp_Service_Enable_00001, TestSize.Level1)
{
    HILOGI("Opp_Service_Enable_00001 start");
    oppService_->LoadOppServiceManagerInterfaceLib();
    int32_t ret = oppService_->GetMaxConnectNum();
    oppService_->UnloadOppServiceManagerInterfaceLib();
    EXPECT_EQ(ret, RET_NO_SUPPORT);
    HILOGI("Opp_Service_Enable_00001 end");
}

/*
 * @tc.number: Opp_Service_SetLastReceivedFileUri_00001
 * @tc.name: SetLastReceivedFileUri
 * @tc.desc:
 */
HWTEST_F(OppServiceTest, Opp_Service_SetLastReceivedFileUri_00001, TestSize.Level1)
{
    HILOGI("Opp_Service_SetLastReceivedFileUri_00001 start");
    int ret = oppService_->SetLastReceivedFileUri("test.jpg");
    EXPECT_EQ(ret, RET_BAD_STATUS);
    HILOGI("Opp_Service_SetLastReceivedFileUri_00001 end");
}

/*
 * @tc.number: Opp_Service_ConnectSocket_00001
 * @tc.name: ConnectSocket
 * @tc.desc:
 */
HWTEST_F(OppServiceTest, Opp_Service_ConnectSocket_00001, TestSize.Level1)
{
    HILOGI("Opp_Service_ConnectSocket_00001 start");
    auto uuid = Uuid::ConvertFromString(BLUETOOTH_UUID_OPP);
    int securityFlags = 3;  // SOCK_FLAG_AUTHENTICATION | SOCK_FLAG_ENCRYPTION;
    int ret = oppService_->ConnectSocket("00:11:22:33:44:55:66", uuid, securityFlags, 1, 1);
    EXPECT_EQ(ret, RET_NO_SUPPORT);
    HILOGI("Opp_Service_ConnectSocket_00001 end");
}

/*
 * @tc.number: Opp_Service_SendFile_00001
 * @tc.name: SendFile
 * @tc.desc:
 */
HWTEST_F(OppServiceTest, Opp_Service_SendFile_00001, TestSize.Level1)
{
    HILOGI("Opp_Service_SendFile_00001 start");
    RawAddress device = RawAddress(addr_);
    std::vector<IOppTransferFileHolder> fileHolders;
    oppService_->CreateSdpRecord();
    oppService_->SendFile(device, fileHolders);
    IOppTransferInformation info;
    oppService_->NotifyTransferStateChanged(info);
    EXPECT_TRUE(oppService_ != nullptr);
    oppService_->CleanSdpRecord();
    HILOGI("Opp_Service_SendFile_00001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS