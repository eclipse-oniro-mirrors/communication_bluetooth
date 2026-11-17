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

#include "bluetooth_access_token_mock.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_hfp_hf_server.h"
#include "bluetooth_device_manager.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "btcommon/timer_manager.h"
#include "hfp_hf_service.h"
#include "class_creator.h"
#include "profile_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
using namespace std;

static BluetoothHost *host_;
static std::unique_ptr<BluetoothHfpHfServer> hfpHfServer_ {nullptr};
const int TIME = 1;

void *HfpHfServiceCreate(void)
{
    return new HfpHfService();
}

class HfpHfServerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        BluetoothAccessTokenMock::SetNativeTokenInfo();
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
        ProfileConfig::GetInstance()->Load();
        ClassFactory::RegisterClass("HfpHfService", HfpHfServiceCreate);
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp()
    {
        hfpHfServer_ = std::make_unique<BluetoothHfpHfServer>();
    }
    void TearDown()
    {}
};

/*
 * @tc.number: HfpHfServer_UnitTest_001
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_001, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_001 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    EXPECT_FALSE(hfpHfServer_->ConnectSco(addr));
    HILOGI("HfpHfServer_UnitTest_001 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_002
 * @tc.name: DisconnectSco
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_002, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_002 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    EXPECT_FALSE(hfpHfServer_->DisconnectSco(addr));
    HILOGI("HfpHfServer_UnitTest_002 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_003
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_003, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_003 start");
    std::vector<int> states = {0};
    std::vector<BluetoothRawAddress> devices;
    int ret = hfpHfServer_->GetDevicesByStates(states, devices);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("HfpHfServer_UnitTest_003 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_004
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_004, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_004 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int ret = hfpHfServer_->GetDeviceState(addr);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("HfpHfServer_UnitTest_004 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_005
 * @tc.name: GetScoState
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_005, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_005 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int ret = hfpHfServer_->GetScoState(addr);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("HfpHfServer_UnitTest_005 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_006
 * @tc.name: SendDTMFTone
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_006, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_006 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    uint8_t code = 0x01;
    bool ret = hfpHfServer_->SendDTMFTone(addr, code);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_006 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_007
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_007, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_007 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int ret = hfpHfServer_->Connect(addr);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("HfpHfServer_UnitTest_007 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_008
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_008, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_008 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int ret = hfpHfServer_->Disconnect(addr);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("HfpHfServer_UnitTest_008 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_009
 * @tc.name: OpenVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_009, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_009 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    bool ret = hfpHfServer_->OpenVoiceRecognition(addr);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_009 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_010
 * @tc.name: CloseVoiceRecognition
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_010, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_010 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    bool ret = hfpHfServer_->CloseVoiceRecognition(addr);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_010 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_011
 * @tc.name: GetCurrentCallList
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_011, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_011 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    std::vector<BluetoothHfpHfCall> calls;
    int ret = hfpHfServer_->GetCurrentCallList(addr, calls);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("HfpHfServer_UnitTest_011 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_012
 * @tc.name: AcceptIncomingCall
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_012, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_012 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int flag = 0;
    bool ret = hfpHfServer_->AcceptIncomingCall(addr, flag);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_012 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_013
 * @tc.name: HoldActiveCall
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_013, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_013 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    bool ret = hfpHfServer_->HoldActiveCall(addr);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_013 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_014
 * @tc.name: RejectIncomingCall
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_014, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_014 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    bool ret = hfpHfServer_->RejectIncomingCall(addr);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_014 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_015
 * @tc.name: SendKeyPressed
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_015, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_015 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    bool ret = hfpHfServer_->SendKeyPressed(addr);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_015 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_016
 * @tc.name: HandleIncomingCall
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_016, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_016 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int flag = 0;
    bool ret = hfpHfServer_->HandleIncomingCall(addr, flag);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_016 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_017
 * @tc.name: DialLastNumber
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_017, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_017 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    bool ret = hfpHfServer_->DialLastNumber(addr);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_017 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_018
 * @tc.name: DialMemory
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_018, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_018 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int index = 1;
    bool ret = hfpHfServer_->DialMemory(addr, index);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_018 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_019
 * @tc.name: HandleMultiCall
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_019, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_019 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int flag = 0;
    int index = 1;
    bool ret = hfpHfServer_->HandleMultiCall(addr, flag, index);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_019 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_020
 * @tc.name: SendVoiceTag
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_020, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_020 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int index = 1;
    bool ret = hfpHfServer_->SendVoiceTag(addr, index);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_020 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_021
 * @tc.name: FinishActiveCall
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_021, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_021 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    BluetoothHfpHfCall call;
    bool ret = hfpHfServer_->FinishActiveCall(addr, call);
    EXPECT_FALSE(ret);
    HILOGI("HfpHfServer_UnitTest_021 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_022
 * @tc.name: StartDial
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_022, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_022 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    std::string number = "1234567890";
    BluetoothHfpHfCall call;
    int ret = hfpHfServer_->StartDial(addr, number, call);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("HfpHfServer_UnitTest_022 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_023
 * @tc.name: SetConnectStrategy
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_023, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_023 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int strategy = 0;
    int ret = hfpHfServer_->SetConnectStrategy(addr, strategy);
    EXPECT_GE(ret, 0);
    HILOGI("HfpHfServer_UnitTest_023 end");
}

/*
 * @tc.number: HfpHfServer_UnitTest_024
 * @tc.name: GetConnectStrategy
 * @tc.desc:
 */
HWTEST_F(HfpHfServerTest, HfpHfServer_UnitTest_024, TestSize.Level1)
{
    HILOGI("HfpHfServer_UnitTest_024 start");
    BluetoothRawAddress addr("11:22:33:44:55:66");
    int strategy;
    int ret = hfpHfServer_->GetConnectStrategy(addr, strategy);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("HfpHfServer_UnitTest_024 end");
}

}  // namespace Bluetooth
}  // namespace OHOS