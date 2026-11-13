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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_pan.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
using namespace std;

class PanObserverTest : public PanObserver {
public:
    PanObserverTest() = default;
    virtual ~PanObserverTest() = default;
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause, int role) {
        flag = true;
    }
private:
    bool flag {false};
};


static Bluetooth::Pan *profile_;
static std::shared_ptr<PanObserverTest> panObserverTest = std::make_shared<PanObserverTest>();
static BluetoothHost *host_;
class PanTest : public testing::Test {
public:
    PanTest();
    ~PanTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
                       std::vector<std::string> bluetoothRemoteDeviceAddr);
    void SetRandomAddr(BluetoothRemoteDevice &device);

private:
};

PanTest::PanTest()
{
}

PanTest::~PanTest()
{
}

void PanTest::SetUpTestCase(void)
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

    profile_ = Pan::GetProfile();

    HILOGI("SetUpTestCase end");
}

void PanTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void PanTest::SetUp()
{
}

void PanTest::TearDown()
{
}

void PanTest::SetRandomAddr(BluetoothRemoteDevice &device)
{
    string randomAddr;
    host_->GetRandomAddress(device.GetDeviceAddr(), randomAddr);
    device.address_ = randomAddr;
}

bool PanTest::CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
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
 * @tc.number: PAN_ModuleTest_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_GetDevicesByStates_00100 start");

    std::vector<int> states;
    std::vector<std::string> bluetoothRemoteDeviceAddr;
    std::vector<BluetoothRemoteDevice> devices;
    int res = profile_->GetDevicesByStates(states, devices);
    EXPECT_EQ(res, BT_NO_ERROR);
    EXPECT_TRUE(CompareDevice(devices, bluetoothRemoteDeviceAddr));

    HILOGI("PAN_ModuleTest_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: PAN_ModuleTest_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_GetDeviceState_00100 start");

    int sucess = static_cast<int>(BTConnectState::DISCONNECTED);
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    profile_->GetDeviceState(device, state);
    EXPECT_EQ(sucess, state);

    HILOGI("PAN_ModuleTest_GetDeviceState_00100 end");
}

/*
 * @tc.number: PAN_ModuleTest_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_Disconnect_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_Disconnect_00100 start");

    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_PROFILE_DISABLED);

    HILOGI("PAN_ModuleTest_Disconnect_00100 end");
}

/*
 * @tc.number: PAN_ModuleTest_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_Connect_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_Connect_00100 start");

    BluetoothRemoteDevice device("11:22:33:AD:FF:1A", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->Connect(device), BT_ERR_PROFILE_DISABLED);

    HILOGI("PAN_ModuleTest_Connect_00100 end");
}

/*
 * @tc.number: PAN_ModuleTest_IsPanSupported_00100
 * @tc.name: IsPanSupported
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_IsPanSupported_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_IsPanSupported_00100 start");
    bool res = false;
    EXPECT_TRUE(profile_->IsPanSupported(res) == BT_NO_ERROR);
    EXPECT_TRUE(res);
    HILOGI("PAN_ModuleTest_IsPanSupported_00100 end");
}

/*
 * @tc.number: PAN_ModuleTest_GetConnectStrategy_001
 * @tc.name: GetConnectStrategy
 * @tc.desc: Get connect strategy by address
*/
HWTEST_F(PanTest, PAN_ModuleTest_GetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_GetConnectStrategy_001 start");
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();
    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    HILOGI("PAN_ModuleTest_GetConnectStrategy_001 end");
}
 
/*
 * @tc.number: PAN_ModuleTest_GetConnectStrategy_002
 * @tc.name: GetConnectStrategy
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(PanTest, PAN_ModuleTest_GetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_GetConnectStrategy_002 start");
    BluetoothRemoteDevice device("00:00:00:00:00:01", BT_TRANSPORT_BREDR);
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN));
    HILOGI("PAN_ModuleTest_GetConnectStrategy_002 end");
}
 
/*
 * @tc.number: PAN_ModuleTest_SetConnectStrategy_001
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(PanTest, PAN_ModuleTest_SetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_SetConnectStrategy_001 start");
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->SetConnectStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN)), BT_ERR_INVALID_PARAM);
    HILOGI("PAN_ModuleTest_SetConnectStrategy_001 end");
}
 
/*
 * @tc.number: PAN_ModuleTest_SetConnectStrategy_002
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(PanTest, PAN_ModuleTest_SetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_SetConnectStrategy_002 start");
    sleep(TIME);
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();
    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    HILOGI("PAN_ModuleTest_SetConnectStrategy_002 end");
}

/*
 * @tc.number: PAN_ModuleTest_RegisterObserver_00100
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_RegisterObserver_00100 start");

    profile_->RegisterObserver(panObserverTest);
    EXPECT_TRUE(profile_ != nullptr);
    HILOGI("PAN_ModuleTest_RegisterObserver_00100 end");
}

/*
 * @tc.number: PAN_ModuleTest_DeregisterObserver_00100
 * @tc.name: DeregisterObserver
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_DeregisterObserver_00100 start");

    profile_->DeregisterObserver(panObserverTest);
    EXPECT_TRUE(profile_ != nullptr);
    HILOGI("PAN_ModuleTest_DeregisterObserver_00100 end");
}

/*
 * @tc.number: PAN_ModuleTest_OnConnectionStateChanged_00100
 * @tc.name: OnConnectionStateChanged
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_OnConnectionStateChanged_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_OnConnectionStateChanged_00100 start");

    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int state = static_cast<int>(BTConnectState::DISCONNECTED);;
    panObserverTest->OnConnectionStateChanged(device, state, 0, 0);
    EXPECT_TRUE(panObserverTest->flag);

    HILOGI("PAN_ModuleTest_OnConnectionStateChanged_00100 end");
}

/*
 * @tc.number: PAN_ModuleTest_SetTethering_00100
 * @tc.name: SetTethering
 * @tc.desc:
 */
HWTEST_F(PanTest, PAN_ModuleTest_SetTethering_00100, TestSize.Level1)
{
    HILOGI("PAN_ModuleTest_SetTethering_00100 start");

    bool value = false;
    EXPECT_EQ(profile_->SetTethering(value), BT_NO_ERROR);
    bool res = true;
    EXPECT_EQ(profile_->IsTetheringOn(res), BT_NO_ERROR);
    EXPECT_FALSE(res);
    value = true;
    EXPECT_EQ(profile_->SetTethering(value), BT_NO_ERROR);
    EXPECT_EQ(profile_->IsTetheringOn(res), BT_NO_ERROR);
    EXPECT_TRUE(res);
    HILOGI("PAN_ModuleTest_SetTethering_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS