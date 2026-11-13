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
#include "bluetooth_hearingaid_server.h"
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
static BluetoothHearingAidServer* g_hearingaid = nullptr;
class HearingaidServerTest : public testing::Test {
public:
    HearingaidServerTest();
    ~HearingaidServerTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
};

class TestObserver : public IBluetoothHearingAidObserver {
    ErrCode OnConnectionStateChanged(
        const std::string& address,
        ConnectionState state,
        int32_t cause){return BT_OV_NO_ERROR;};

    ErrCode OnActiveDeviceChanged(
        const std::string& address,
        const std::string& name){return BT_OV_NO_ERROR;};
};

HearingaidServerTest::HearingaidServerTest()
{
}

HearingaidServerTest::~HearingaidServerTest()
{
}

void HearingaidServerTest::SetUpTestCase(void)
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
    g_hearingaid = new BluetoothHearingAidServer();
    HILOGI("SetUpTestCase end");
}

void HearingaidServerTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void HearingaidServerTest::SetUp()
{}

void HearingaidServerTest::TearDown()
{}
/*
 * @tc.number: Hearingaid_ModuleTest_Connect_00200
 * @tc.name: Connect
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_Connect_00200, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Connect_00200 start");
    std::string validAddress = "00:11:22:33:44:55";
    int ret = g_hearingaid->Connect(validAddress);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_Connect_00200 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_Connect_00300
 * @tc.name: Connect
 * @tc.desc: Multiple invalid address formats
 */
HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_Connect_00300, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Connect_00300 start");
    std::string invalidFormat[] = {
        "00-11-22-33-44-55", "00:11:22:33:44", "00:11:22:33:44:GG", "00:11:22:33:44:"
    };
    for (const auto& address : invalidFormat) {
        int ret = g_hearingaid->Connect(address);
        EXPECT_EQ(ret, BT_OV_ERR_INVALID_PARAM);
    }
    HILOGI("Hearingaid_ModuleTest_Connect_00300 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_Disconnect_00200, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Disconnect_00200 start");
    std::string validAddress = "00:11:22:33:44:55";
    int ret = g_hearingaid->Disconnect(validAddress);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_Disconnect_00200 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_Disconnect_00300, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Disconnect_00300 start");
    std::string invalidAddress = "XY:00:00:00:00:00";
    int ret = g_hearingaid->Disconnect(invalidAddress);
    EXPECT_EQ(ret, BT_OV_ERR_INVALID_PARAM);
    HILOGI("Hearingaid_ModuleTest_Disconnect_00300 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_SupportHearingAid_00200, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SupportHearingAid_00200 start");
    bool support = false;
    int ret = g_hearingaid->IsLocalDeviceSupportHearingAid(support);
    support = !support; // Test when support status is toggled
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_SupportHearingAid_00200 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_SetVolume_00200, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetVolume_00200 start");
    std::string address = "00:11:22:33:44:55";
    int ret = g_hearingaid->SetVolume(address, -120);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_SetVolume_00200 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_SetVolume_00300, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetVolume_00300 start");
    std::string address1 = "XY:00:00:00:00:00";
    std::string address2 = "00:11:22:33:44:55";
    int ret1 = g_hearingaid->SetVolume(address1, -120);
    int ret2 = g_hearingaid->SetVolume(address2, 120);
    EXPECT_EQ(ret1, BT_OV_ERR_INVALID_PARAM);
    EXPECT_EQ(ret2, BT_OV_ERR_INVALID_PARAM);
    HILOGI("Hearingaid_ModuleTest_SetVolume_00300 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_GetDeviceInfo_00200, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetDeviceInfo_00200 start");
    std::string validAddress = "00:11:22:33:44:55";
    DeviceSide side;
    DeviceMode mode;
    int64_t id = 0;
    int ret = g_hearingaid->GetHearingAidDeviceInfo(validAddress, side, mode, id);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetDeviceInfo_00200 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_GetConnectionState_00200, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetConnectionState_00200 start");
    std::string validAddress = "00:11:22:33:44:55";
    ConnectionState state;
    int ret = g_hearingaid->GetConnectionState(validAddress, state);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetConnectionState_00200 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_SetStrategy_00200, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetStrategy_00200 start");
    std::string validAddress = "00:11:22:33:44:55";
    int32_t strategy = 1;
    int ret = g_hearingaid->SetConnectionStrategy(validAddress, static_cast<ConnectionStrategy>(strategy));
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_SetStrategy_00200 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_GetStrategy_00200, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetStrategy_00200 start");
    std::string validAddress = "00:11:22:33:44:55";
    ConnectionStrategy strategy;
    int ret = g_hearingaid->GetConnectionStrategy(validAddress, strategy);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetStrategy_00200 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_GetConnectedDevices_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetConnectedDevices_00100 start");
    std::vector<std::string> connectedDevices;
    int ret = g_hearingaid->GetConnectedDevices(connectedDevices);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetConnectedDevices_00100 end");
}

HWTEST_F(HearingaidServerTest, Hearingaid_ModuleTest_GetActiveDevice_00100, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetActiveDevice_00100 start");
    std::string address, name;
    int ret = g_hearingaid->GetActiveDevice(address, name);
    EXPECT_EQ(ret, BT_OV_ERR_INTERNAL_ERROR);
    HILOGI("Hearingaid_ModuleTest_GetActiveDevice_00100 end");
}

}  // namespace Bluetooth
}  // namespace OHOS