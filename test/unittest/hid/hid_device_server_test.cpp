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

#include <gtest/gtest.h>
#include "bluetooth_hid_device_server.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_errorcode.h"
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
static BluetoothHidDeviceServer* g_hidDeviceServer = nullptr;
class HidDeviceServerTest : public testing::Test {
public:
    HidDeviceServerTest();
    ~HidDeviceServerTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
};

HidDeviceServerTest::HidDeviceServerTest()
{
}

HidDeviceServerTest::~HidDeviceServerTest()
{
}

void HidDeviceServerTest::SetUpTestCase(void)
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
    g_hidDeviceServer = new BluetoothHidDeviceServer();
    HILOGI("SetUpTestCase end");
}

void HidDeviceServerTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    delete g_hidDeviceServer;
    g_hidDeviceServer = nullptr;
    HILOGI("TearDownTestCase end");
}

void HidDeviceServerTest::SetUp()
{}

void HidDeviceServerTest::TearDown()
{}

/*
 * @tc.number: HID_Device_ModuleTest_RegisterHidDevice_00100
 * @tc.name: RegisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_RegisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_RegisterHidDevice_00100 start");
    BluetoothHidDeviceSdp sdpSetting;
    sdpSetting.name_ = "test";
    sdpSetting.description_ = "test";
    sdpSetting.provider_ = "test";
    sdpSetting.subclass_ = static_cast<int>(Subclass::SUBCLASS_MOUSE);
    sdpSetting.sdpSettings_ = {};
    BluetoothHidDeviceQos inQos;
    inQos.serviceType_ = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    inQos.tokenRate_ = DEFAULT_TOKEN_RATE;
    inQos.tokenBucketSize_ = DEFAULT_TOKEN_BUCKET_SIZE;
    inQos.peakBandwidth_ = DEFAULT_PEAK_BANDWIDTH;
    inQos.latency_ = DEFAULT_LATENCY;
    inQos.delayVariation_ = DEFAULT_DELAY_VARIATION;
    BluetoothHidDeviceQos outQos;
    outQos.serviceType_ = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    outQos.tokenRate_ = DEFAULT_TOKEN_RATE;
    outQos.tokenBucketSize_ = DEFAULT_TOKEN_BUCKET_SIZE;
    outQos.peakBandwidth_ = DEFAULT_PEAK_BANDWIDTH;
    outQos.latency_ = DEFAULT_LATENCY;
    outQos.delayVariation_ = DEFAULT_DELAY_VARIATION;
    int ret = g_hidDeviceServer->RegisterHidDevice(sdpSetting, inQos, outQos);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("HID_Device_ModuleTest_RegisterHidDevice_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_UnregisterHidDevice_00100
 * @tc.name: UnregisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_UnregisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_UnregisterHidDevice_00100 start");
    EXPECT_EQ(g_hidDeviceServer->UnregisterHidDevice(), BT_ERR_INTERNAL_ERROR);
    HILOGI("HID_Device_ModuleTest_UnregisterHidDevice_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_GetConnectedDevices_00100
 * @tc.name: GetConnectedDevices
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_GetConnectedDevices_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetDevicesByStates_00100 start");
    std::vector<std::string> bluetoothRemoteDeviceAddr;
    std::vector<BluetoothRawAddress> devices;
    g_hidDeviceServer->GetConnectedDevices(devices);
    EXPECT_TRUE(devices.size() == 0);
    HILOGI("HID_Device_ModuleTest_GetDevicesByStates_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_GetConnectionState_00100
 * @tc.name: GetConnectionState
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_GetConnectionState_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetConnectionState_00100 start");
 
    int success = static_cast<int>(BTConnectState::DISCONNECTED);
    BluetoothRawAddress device("11:22:33:44:55:66");
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    g_hidDeviceServer->GetConnectionState(device, state);
    EXPECT_EQ(success, state);
 
    HILOGI("HID_Device_ModuleTest_GetDeviceState_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_GetConnectionState_00200
 * @tc.name: GetConnectionState
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_GetConnectionState_00200, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetConnectionState_00200 start");
    BluetoothRawAddress device("invalid addr");
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    EXPECT_EQ(g_hidDeviceServer->GetConnectionState(device, state), BT_ERR_INVALID_PARAM);
    HILOGI("HID_Device_ModuleTest_GetConnectionState_00200 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_Connect_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_Connect_00100 start");
    BluetoothRawAddress device("invalid addr");
    EXPECT_EQ(g_hidDeviceServer->Connect(device), BT_ERR_INVALID_PARAM);
    HILOGI("HID_Device_ModuleTest_Connect_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_Connect_00200
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_Connect_00200, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_Connect_00200 start");
    BluetoothRawAddress device("11:22:33:44:55:66");
    EXPECT_EQ(g_hidDeviceServer->Connect(device), BT_ERR_INTERNAL_ERROR);
    HILOGI("HID_Device_ModuleTest_Connect_00200 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_Disconnect_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_Disconnect_00100 start");
    EXPECT_EQ(g_hidDeviceServer->Disconnect(), BT_ERR_INTERNAL_ERROR);
    HILOGI("HID_Device_ModuleTest_Disconnect_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_SendReport_00100
 * @tc.name: SendReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_SendReport_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_SendReport_00100 start");
    int id = 0;
    std::vector<uint8_t> data = {};
    EXPECT_EQ(g_hidDeviceServer->SendReport(id, data), BT_ERR_INTERNAL_ERROR);
    HILOGI("HID_Device_ModuleTest_SendReport_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_ReplyReport_00100
 * @tc.name: ReplyReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_ReplyReport_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_ReplyReport_00100 start");
    ReportType type = REPORT_TYPE_INPUT;
    int id = 0;
    std::vector<uint8_t> data = {};
    EXPECT_EQ(g_hidDeviceServer->ReplyReport(type, id, data), BT_ERR_INTERNAL_ERROR);
    HILOGI("HID_Device_ModuleTest_ReplyReport_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_ReportError_00100
 * @tc.name: ReportError
 * @tc.desc:
 */
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_ReportError_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_ReportError_00100 start");
    ErrorReason type = RSP_SUCCESS;
    EXPECT_EQ(g_hidDeviceServer->ReportError(type), BT_ERR_INTERNAL_ERROR);
    HILOGI("HID_Device_ModuleTest_ReportError_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_GetConnectStrategy_001
 * @tc.name: GetConnectStrategy
 * @tc.desc: Get connect strategy by address
*/
HWTEST_F(HidDeviceServerTest, HID_Device_ModuleTest_GetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetConnectStrategy_001 start");
    BluetoothRawAddress device("11:22:33:44:55:66");
    BluetoothRawAddress invalidDevice("11:22");
    int ret = g_hidDeviceServer->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    int strategy;
    ret = g_hidDeviceServer->GetConnectStrategy(device, strategy);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    ret = g_hidDeviceServer->SetConnectStrategy(invalidDevice, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    ret = g_hidDeviceServer->GetConnectStrategy(invalidDevice, strategy);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("HID_Device_ModuleTest_GetConnectStrategy_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS