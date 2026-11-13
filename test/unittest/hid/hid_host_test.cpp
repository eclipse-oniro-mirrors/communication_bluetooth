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
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hid_host.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace std;

class HidHostObserverTest : public HidHostObserver {
public:
    HidHostObserverTest() = default;
    virtual ~HidHostObserverTest() = default;
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause) {}

private:
};

constexpr int TIME = 4;
static Bluetooth::HidHost  *profile_;
static std::shared_ptr<HidHostObserverTest> hidHostObserverTest = std::make_shared<HidHostObserverTest>();
static BluetoothHost *host_;

class HidHostTest : public testing::Test {
public:
    HidHostTest();
    ~HidHostTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
                       std::vector<std::string> bluetoothRemoteDeviceAddr);
    void DisableBt();

private:
};

HidHostTest::HidHostTest()
{
}

HidHostTest::~HidHostTest()
{
}

void HidHostTest::SetUpTestCase(void)
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

    profile_ = HidHost::GetProfile();

    HILOGI("SetUpTestCase end");
}
void HidHostTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void HidHostTest::SetUp()
{
}
void HidHostTest::TearDown()
{
}

void HidHostTest::DisableBt()
{
    int btState = host_->GetBtState();
    HILOGI("DisableBt start, btState = %{public}d", btState);
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        sleep(TIME);
    }
}

bool HidHostTest::CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
                                std::vector<std::string> bluetoothRemoteDeviceAddr)
{
    if (bluetoothRemoteDeviceByState.size() != bluetoothRemoteDeviceAddr.size()) {
        return false;
    }
    for (int i = 0; i < bluetoothRemoteDeviceByState.size(); i++) {
        if (bluetoothRemoteDeviceByState[i].GetDeviceAddr().compare(bluetoothRemoteDeviceAddr[i]) != 0) {
            return false;
        }
    }
    return true;
}

/*
 * @tc.number: HID_ModuleTest_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_GetDevicesByStates_00100 start");

    profile_ = HidHost::GetProfile();
    std::vector<int> states;
    std::vector<std::string> bluetoothRemoteDeviceAddr;
    std::vector<BluetoothRemoteDevice> devices;
    profile_->GetDevicesByStates(states, devices);
    EXPECT_TRUE(CompareDevice(devices, bluetoothRemoteDeviceAddr));

    HILOGI("HID_ModuleTest_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_GetDeviceState_00100 start");

    int sucess = static_cast<int>(BTConnectState::DISCONNECTED);
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_ = HidHost::GetProfile();
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    profile_->GetDeviceState(device, state);
    EXPECT_EQ(sucess, state);

    HILOGI("HID_ModuleTest_GetDeviceState_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_GetDeviceState_00200
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_GetDeviceState_00200, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_GetDeviceState_00200 start");

    BluetoothRemoteDevice device("abc", 1);
    profile_ = HidHost::GetProfile();
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    EXPECT_EQ(profile_->GetDeviceState(device, state), BT_ERR_INVALID_PARAM);

    HILOGI("HID_ModuleTest_GetDeviceState_00200 end");
}

/*
 * @tc.number: HID_ModuleTest_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_Connect_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_Connect_00100 start");

    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_ = HidHost::GetProfile();
    profile_->Connect(device);

    HILOGI("HID_ModuleTest_Connect_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_Disconnect_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_Disconnect_00100 start");

    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    profile_ = HidHost::GetProfile();
    profile_->Disconnect(device);

    HILOGI("HID_ModuleTest_Disconnect_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_Disconnect_00200
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_Disconnect_00200, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_Disconnect_00200 start");

    BluetoothRemoteDevice device("abc", 1);
    profile_ = HidHost::GetProfile();
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_INVALID_PARAM);

    HILOGI("HID_ModuleTest_Disconnect_00200 end");
}

/*
 * @tc.number: HID_ModuleTest_RegisterObserver_00100
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_RegisterObserver_00100 start");

    profile_ = HidHost::GetProfile();
    profile_->RegisterObserver(hidHostObserverTest);

    HILOGI("HID_ModuleTest_RegisterObserver_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_DeregisterObserver_00100
 * @tc.name: DeregisterObserver
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_DeregisterObserver_00100 start");

    profile_ = HidHost::GetProfile();
    profile_->DeregisterObserver(hidHostObserverTest);

    HILOGI("HID_ModuleTest_DeregisterObserver_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_OnConnectionStateChanged_00100
 * @tc.name: OnConnectionStateChanged
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_OnConnectionStateChanged_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_OnConnectionStateChanged_00100 start");

    BluetoothRemoteDevice device;
    int ret = 0;
    hidHostObserverTest->OnConnectionStateChanged(device, ret, 0);

    HILOGI("HID_ModuleTest_OnConnectionStateChanged_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_HidHostVCUnplug_00100
 * @tc.name: HidHostVCUnplug
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_HidHostVCUnplug_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_HidHostVCUnplug_00100 start");
    std::string device;
    profile_ = HidHost::GetProfile();
    profile_->HidHostVCUnplug(device, 0, 0, 0);

    HILOGI("HID_ModuleTest_HidHostVCUnplug_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_HidHostSendData_00100
 * @tc.name: HidHostSendData
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_HidHostSendData_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_HidHostSendData_00100 start");
    std::string device;
    profile_ = HidHost::GetProfile();
    profile_->HidHostSendData(device, 0, 0, 0);

    HILOGI("HID_ModuleTest_HidHostSendData_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_HidHostSetReport_00100
 * @tc.name: HidHostSetReport
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_HidHostSetReport_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_HidHostSetReport_00100 start");
    std::string device;
    profile_ = HidHost::GetProfile();
    std::string report;
    profile_->HidHostSetReport(device, 0, report);

    HILOGI("HID_ModuleTest_HidHostSetReport_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_HidHostGetReport_00100
 * @tc.name: HidHostGetReport
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_HidHostGetReport_00100, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_HidHostGetReport_00100 start");
    std::string device;
    profile_ = HidHost::GetProfile();
    profile_->HidHostGetReport(device, 0, 0, 0);

    HILOGI("HID_ModuleTest_HidHostGetReport_00100 end");
}

/*
 * @tc.number: HID_ModuleTest_GetConnectStrategy_001
 * @tc.name: GetConnectStrategy
 * @tc.desc: Get connect strategy by address
*/
HWTEST_F(HidHostTest, HID_ModuleTest_GetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_GetConnectStrategy_001 start");

    profile_ = HidHost::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();

    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));

    HILOGI("HID_ModuleTest_GetConnectStrategy_001 end");
}

/*
 * @tc.number: HID_ModuleTest_GetConnectStrategy_002
 * @tc.name: GetConnectStrategy
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(HidHostTest, HID_ModuleTest_GetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_GetConnectStrategy_002 start");
    profile_ = HidHost::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:01", BT_TRANSPORT_BREDR);

    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN));

    HILOGI("HID_ModuleTest_GetConnectStrategy_002 end");
}

/*
 * @tc.number: HID_ModuleTest_SetConnectStrategy_001
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(HidHostTest, HID_ModuleTest_SetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_SetConnectStrategy_001 start");

    profile_ = HidHost::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->SetConnectStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN)), BT_ERR_INVALID_PARAM);

    HILOGI("HID_ModuleTest_SetConnectStrategy_001 end");
}

/*
 * @tc.number: HID_ModuleTest_SetConnectStrategy_002
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(HidHostTest, HID_ModuleTest_SetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_SetConnectStrategy_002 start");

    sleep(TIME);
    profile_ = HidHost::GetProfile();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();

    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));

    HILOGI("HID_ModuleTest_SetConnectStrategy_002 end");
}

/*
 * @tc.number: HID_ModuleTest_GetDevicesByStates_00200
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_GetDevicesByStates_00200, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_GetDevicesByStates_00200 start");

    profile_ = HidHost::GetProfile();
    std::vector<int> states;
    std::vector<BluetoothRemoteDevice> devices;

    DisableBt();
    EXPECT_EQ(profile_->GetDevicesByStates(states, devices), BT_ERR_INVALID_STATE);
    HILOGI("HID_ModuleTest_GetDevicesByStates_00200 end");
}

/*
 * @tc.number: HID_ModuleTest_GetDeviceState_00300
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_GetDeviceState_00300, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_GetDeviceState_00300 start");

    BluetoothRemoteDevice device;
    profile_ = HidHost::GetProfile();
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);

    DisableBt();
    EXPECT_EQ(profile_->GetDeviceState(device, state), BT_ERR_INVALID_STATE);

    HILOGI("HID_ModuleTest_GetDeviceState_00300 end");
}

/*
 * @tc.number: HID_ModuleTest_Connect_00200
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_Connect_00200, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_Connect_00200 start");

    BluetoothRemoteDevice device;
    profile_ = HidHost::GetProfile();

    DisableBt();
    EXPECT_EQ(profile_->Connect(device), BT_ERR_INVALID_STATE);

    HILOGI("HID_ModuleTest_Connect_00200 end");
}

/*
 * @tc.number: HID_ModuleTest_Disconnect_00300
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_Disconnect_00300, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_Disconnect_00300 start");

    BluetoothRemoteDevice device;
    profile_ = HidHost::GetProfile();

    DisableBt();
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_INVALID_STATE);

    HILOGI("HID_ModuleTest_Disconnect_00300 end");
}

/*
 * @tc.number: HID_ModuleTest_HidHostVCUnplug_00200
 * @tc.name: HidHostVCUnplug
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_HidHostVCUnplug_00200, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_HidHostVCUnplug_00200 start");
    std::string device;
    profile_ = HidHost::GetProfile();

    DisableBt();
    profile_->HidHostVCUnplug(device, 0, 0, 0);

    HILOGI("HID_ModuleTest_HidHostVCUnplug_00200 end");
}

/*
 * @tc.number: HID_ModuleTest_HidHostSendData_00200
 * @tc.name: HidHostSendData
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_HidHostSendData_00200, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_HidHostSendData_00200 start");
    std::string device;
    profile_ = HidHost::GetProfile();

    DisableBt();
    profile_->HidHostSendData(device, 0, 0, 0);

    HILOGI("HID_ModuleTest_HidHostSendData_00200 end");
}

/*
 * @tc.number: HID_ModuleTest_HidHostSetReport_00200
 * @tc.name: HidHostSetReport
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_HidHostSetReport_00200, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_HidHostSetReport_00200 start");
    std::string device;
    profile_ = HidHost::GetProfile();

    DisableBt();
    std::string report;
    profile_->HidHostSetReport(device, 0, report);

    HILOGI("HID_ModuleTest_HidHostSetReport_00200 end");
}

/*
 * @tc.number: HID_ModuleTest_HidHostGetReport_00200
 * @tc.name: HidHostGetReport
 * @tc.desc:
 */
HWTEST_F(HidHostTest, HID_ModuleTest_HidHostGetReport_00200, TestSize.Level1)
{
    HILOGI("HID_ModuleTest_HidHostGetReport_00200 start");
    std::string device;
    profile_ = HidHost::GetProfile();

    DisableBt();
    profile_->HidHostGetReport(device, 0, 0, 0);

    HILOGI("HID_ModuleTest_HidHostGetReport_00200 end");
}

}  // namespace Bluetooth
}  // namespace OHOS
