/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "bluetooth_hearingaid.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_override_errorcode.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_access_token_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace std;

constexpr int TIME = 4;

static BluetoothHost *host_ = nullptr;
static BluetoothHearingAid* g_hearingaid = nullptr;

class HearingaidTest : public testing::Test {
public:
    HearingaidTest();
    ~HearingaidTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
};

HearingaidTest::HearingaidTest()
{
}

HearingaidTest::~HearingaidTest()
{
}

void HearingaidTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();
    BluetoothAccessTokenMock::SetNativeTokenInfo();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
    g_hearingaid = BluetoothHearingAid::GetProfile();
    HILOGI("SetUpTestCase end");
}

void HearingaidTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void HearingaidTest::SetUp()
{}

void HearingaidTest::TearDown()
{}

/*
 * @tc.number: Hearingaid_ModuleTest_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_Connect_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Connect_00100 start");
    std::string address = "11:22:33:44:55:66";
    int ret = g_hearingaid->Connect(address);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_Connect_00100 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Disconnect_00100 start");
    std::string address = "11:22:33:44:55:66";
    int ret = g_hearingaid->Disconnect(address);
    EXPECT_EQ(ret, BT_OV_NO_ERROR);
    HILOGI("Hearingaid_ModuleTest_Disconnect_00100 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_SupportHearingAid_00100
 * @tc.name: IsLocalDeviceSupportHearingAid
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_SupportHearingAid_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SupportHearingAid_00100 start");
    bool support = false;
    int ret = g_hearingaid->IsLocalDeviceSupportHearingAid(support);
    EXPECT_EQ(ret, BT_OV_NO_ERROR);
    HILOGI("Hearingaid_ModuleTest_SupportHearingAid_00100 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_SetVolume_00100
 * @tc.name: SetVolume
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_SetVolume_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetVolume_00100 start");
    std::string address = "11:22:33:44:55:66";
    int64_t volume = -129;
    int ret = g_hearingaid->SetVolume(address, volume);
    EXPECT_EQ(ret, BT_OV_ERR_INVALID_PARAM);
    volume = 1;
    ret = g_hearingaid->SetVolume(address, volume);
    EXPECT_EQ(ret, BT_OV_ERR_INVALID_PARAM);
    volume = -64;
    ret = g_hearingaid->SetVolume(address, volume);
    EXPECT_EQ(ret, BT_OV_NO_ERROR);
    HILOGI("Hearingaid_ModuleTest_SetVolume_00100 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetHearingAidDeviceInfo_00100
 * @tc.name: GetHearingAidDeviceInfo
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_GetHearingAidDeviceInfo_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetHearingAidDeviceInfo_00100 start");
    std::string address = "11:22:33:44:55:66";
    int32_t side;
    int32_t mode;
    int64_t id;
    int ret = g_hearingaid->GetHearingAidDeviceInfo(address, side, mode, id);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetHearingAidDeviceInfo_00100 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetConnectionState_00100
 * @tc.name: GetConnectionState
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_GetConnectionState_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetConnectionState_00100 start");
    std::string address = "11:22:33:44:55:66";
    int32_t state;
    int ret = g_hearingaid->GetConnectionState(address, state);
    EXPECT_EQ(ret, BT_OV_NO_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetConnectionState_00100 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_SetConnectionStrategy_00100
 * @tc.name: SetConnectionStrategy
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_SetConnectionStrategy_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetConnectionStrategy_00100 start");
    std::string address = "11:22:33:44:55:66";
    int strategy = -1;
    int ret = g_hearingaid->SetConnectionStrategy(address, strategy);
    EXPECT_EQ(ret, BT_OV_ERR_INVALID_PARAM);
    strategy = 3;
    ret = g_hearingaid->SetConnectionStrategy(address, strategy);
    EXPECT_EQ(ret, BT_OV_ERR_INVALID_PARAM);
    strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    ret = g_hearingaid->SetConnectionStrategy(address, strategy);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_SetConnectionStrategy_00100 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetConnectionStrategy_00100
 * @tc.name: GetConnectionStrategy
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_GetConnectionStrategy_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetConnectionStrategy_00100 start");
    std::shared_ptr<HearingAidObserver> observer = std::make_shared<HearingAidObserver>();
    g_hearingaid->RegisterObserver(observer);
    std::vector<std::string> connectedDevices;
    g_hearingaid->GetConnectedDevices(connectedDevices);
    g_hearingaid->DeregisterObserver(observer);
    std::string address = "11:22:33:44:55:66";
    int32_t strategy = 0;
    int ret = g_hearingaid->GetConnectionStrategy(address, strategy);
    EXPECT_EQ(ret, BT_OV_NO_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetConnectionStrategy_00100 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetActiveDevice_00100
 * @tc.name: GetActiveDevice
 * @tc.desc:
 */
HWTEST_F(HearingaidTest, Hearingaid_ModuleTest_GetActiveDevice_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetActiveDevice_00100 start");
    std::string address = "11:22:33:44:55:66";
    std::string name = "testName";
    int ret = g_hearingaid->GetActiveDevice(address, name);
    EXPECT_EQ(ret, BT_OV_NO_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetActiveDevice_00100 end");
}

}  // namespace Bluetooth
}  // namespace OHOS