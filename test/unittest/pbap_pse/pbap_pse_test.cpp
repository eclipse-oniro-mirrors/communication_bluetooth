/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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
#include "bluetooth_pbap_pse.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_log.h"
#include "napi_bluetooth_utils.h"
#include "bluetooth_pbap_pse.cpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace std;

class PbapPseObserverTest : public PbapPseObserver {
public:
    PbapPseObserverTest() = default;
    virtual ~PbapPseObserverTest() = default;
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int32_t cause) {}

private:
};

constexpr int TIME = 4;
static Bluetooth::PbapPse  *profile_;
static std::shared_ptr<PbapPseObserverTest> pbapPseObserverTest = std::make_shared<PbapPseObserverTest>();
static BluetoothHost *host_;

class PbapPseTest : public testing::Test {
public:
    PbapPseTest();
    ~PbapPseTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
                       std::vector<std::string> bluetoothRemoteDeviceAddr);
    void DisableBt();

private:
};

PbapPseTest::PbapPseTest()
{
}

PbapPseTest::~PbapPseTest()
{
}

void PbapPseTest::SetUpTestCase(void)
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

    profile_ = PbapPse::GetProfile();

    HILOGI("SetUpTestCase end");
}
void PbapPseTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void PbapPseTest::SetUp()
{
}
void PbapPseTest::TearDown()
{
}

void PbapPseTest::DisableBt()
{
    int btState = host_->GetBtState();
    HILOGI("DisableBt start, btState = %{public}d", btState);
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        sleep(TIME);
    }
}

bool PbapPseTest::CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
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
 * @tc.number: PbapPse_ModuleTest_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetDevicesByStates_00100 start");

    profile_ = PbapPse::GetProfile();
    std::vector<int> states;
    std::vector<std::string> bluetoothRemoteDeviceAddr;
    std::vector<BluetoothRemoteDevice> devices;
    profile_->GetDevicesByStates(states, devices);
    EXPECT_TRUE(CompareDevice(devices, bluetoothRemoteDeviceAddr));

    HILOGI("PbapPse_ModuleTest_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetDevicesByStates_00200
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetDevicesByStates_00200, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetDevicesByStates_00200 start");

    profile_ = PbapPse::GetProfile();
    std::vector<int> states;
    std::vector<BluetoothRemoteDevice> devices;

    DisableBt();
    EXPECT_EQ(profile_->GetDevicesByStates(states, devices), BT_ERR_INVALID_STATE);
    HILOGI("PbapPse_ModuleTest_GetDevicesByStates_00200 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetDeviceState_00100 start");

    int sucess = static_cast<int>(BTConnectState::DISCONNECTED);
    BluetoothRemoteDevice device;
    profile_ = PbapPse::GetProfile();
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    profile_->GetDeviceState(device, state);
    EXPECT_EQ(sucess, state);

    HILOGI("PbapPse_ModuleTest_GetDeviceState_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetDeviceState_00200
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetDeviceState_00200, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetDeviceState_00200 start");

    BluetoothRemoteDevice device("abc", 1);
    profile_ = PbapPse::GetProfile();
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    EXPECT_EQ(profile_->GetDeviceState(device, state), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_GetDeviceState_00200 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetDeviceState_00300
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetDeviceState_00300, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetDeviceState_00300 start");

    BluetoothRemoteDevice device;
    profile_ = PbapPse::GetProfile();
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);

    DisableBt();
    EXPECT_EQ(profile_->GetDeviceState(device, state), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_GetDeviceState_00300 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_Disconnect_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_Disconnect_00100 start");

    BluetoothRemoteDevice device;
    profile_ = PbapPse::GetProfile();
    bool isOK = (profile_->Disconnect(device) == BT_NO_ERROR ? true : false);
    EXPECT_FALSE(isOK);
    int ret = 0;
    pbapPseObserverTest->OnConnectionStateChanged(device, ret, 0);

    HILOGI("PbapPse_ModuleTest_Disconnect_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_Disconnect_00200
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_Disconnect_00200, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_Disconnect_00200 start");

    BluetoothRemoteDevice device("abc", 1);
    profile_ = PbapPse::GetProfile();
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_Disconnect_00200 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_Disconnect_00300
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_Disconnect_00300, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_Disconnect_00300 start");

    BluetoothRemoteDevice device;
    profile_ = PbapPse::GetProfile();

    DisableBt();
    EXPECT_EQ(profile_->Disconnect(device), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_Disconnect_00300 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_DeregisterObserver_00100
 * @tc.name: DeregisterObserver
 * @tc.desc:
 */
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_DeregisterObserver_00100 start");

    profile_ = PbapPse::GetProfile();
    profile_->RegisterObserver(pbapPseObserverTest);
    profile_->DeregisterObserver(pbapPseObserverTest);
    EXPECT_TRUE(profile_ != nullptr);
    HILOGI("PbapPse_ModuleTest_DeregisterObserver_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_SetConnectionStrategy_00100
 * @tc.name: SetConnectionStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_SetConnectionStrategy_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_SetConnectionStrategy_00100 start");

    profile_ = PbapPse::GetProfile();
    BluetoothRemoteDevice device;
    device.StartPair();

    EXPECT_EQ(profile_->SetConnectionStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_SetConnectionStrategy_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_SetConnectionStrategy_00200
 * @tc.name: SetConnectionStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_SetConnectionStrategy_00200, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_SetConnectionStrategy_00200 start");

    profile_ = PbapPse::GetProfile();
    BluetoothRemoteDevice device;
    device.StartPair();

    EXPECT_EQ(profile_->SetConnectionStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_SetConnectionStrategy_00200 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetConnectionStrategy_00100
 * @tc.name: GetConnectionStrategy
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetConnectionStrategy_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetConnectionStrategy_00100 start");
    profile_ = PbapPse::GetProfile();
    BluetoothRemoteDevice device("00:00:00:00:00:01", BT_TRANSPORT_BREDR);

    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectionStrategy(device, strategy), BT_ERR_INVALID_STATE);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN));

    HILOGI("PbapPse_ModuleTest_GetConnectionStrategy_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetConnectionStrategy_00200
 * @tc.name: GetConnectionStrategy
 * @tc.desc: get connect strategy by address
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetConnectionStrategy_00200, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetConnectionStrategy_00200 start");

    profile_ = PbapPse::GetProfile();
    BluetoothRemoteDevice device;
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectionStrategy(device, strategy), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_GetConnectionStrategy_00200 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_CheckShareTypeInvalid_00100
 * @tc.name: CheckShareTypeInvalid
 * @tc.desc: CheckShareTypeInvalid
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_CheckShareTypeInvalid_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_CheckShareTypeInvalid_00100 start");

    profile_ = PbapPse::GetProfile();
    int32_t shareType = 0; // BTShareType::SHARE_NAME_AND_PHONE_NUMBER
    EXPECT_TRUE(CheckShareTypeInvalid(shareType));
    shareType = 1;
    EXPECT_TRUE(CheckShareTypeInvalid(shareType));
    shareType = 2;
    EXPECT_TRUE(CheckShareTypeInvalid(shareType));
    shareType = -1;
    EXPECT_FALSE(CheckShareTypeInvalid(shareType));
    shareType = 3;
    EXPECT_FALSE(CheckShareTypeInvalid(shareType));

    HILOGI("PbapPse_ModuleTest_CheckShareTypeInvalid_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_SetShareType_00100
 * @tc.name: SetShareType
 * @tc.desc: Set share type by address
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_SetShareType_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_SetShareType_00100 start");

    profile_ = PbapPse::GetProfile();
    BluetoothRemoteDevice device;
    device.StartPair();

    EXPECT_EQ(profile_->SetShareType(device, static_cast<int>(BTShareType::SHARE_NOTHING)), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_SetShareType_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetShareType_00100
 * @tc.name: GetShareType
 * @tc.desc: get share type by address
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetShareType_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetShareType_00100 start");

    profile_ = PbapPse::GetProfile();
    BluetoothRemoteDevice device;
    device.StartPair();

    int32_t shareType;
    EXPECT_EQ(profile_->GetShareType(device, shareType), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_GetShareType_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_SetPhoneBookAccessAuthorization_00100
 * @tc.name: SetPhoneBookAccessAuthorization
 * @tc.desc: Set PhoneBook AccessAuthorization by address
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_SetPhoneBookAccessAuthorization_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_SetPhoneBookAccessAuthorization_00100 start");

    sleep(TIME);
    profile_ = PbapPse::GetProfile();
    BluetoothRemoteDevice device;
    device.StartPair();

    EXPECT_EQ(profile_->SetPhoneBookAccessAuthorization(device,
        static_cast<int>(AccessAuthorization::UNKNOWN)), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_SetPhoneBookAccessAuthorization_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetPhoneBookAccessAuthorization_00100
 * @tc.name: GetPhoneBookAccessAuthorization
 * @tc.desc: Get PhoneBook AccessAuthorization by address
*/
HWTEST_F(PbapPseTest, PbapPse_ModuleTest_GetPhoneBookAccessAuthorization_00100, TestSize.Level1)
{
    HILOGI("PbapPse_ModuleTest_GetPhoneBookAccessAuthorization_00100 start");

    profile_ = PbapPse::GetProfile();
    BluetoothRemoteDevice device;
    device.StartPair();

    int32_t accessAuthorization;
    EXPECT_EQ(profile_->GetPhoneBookAccessAuthorization(device, accessAuthorization), BT_ERR_INVALID_STATE);

    HILOGI("PbapPse_ModuleTest_GetPhoneBookAccessAuthorization_00100 end");
}

}  // namespace Bluetooth
}  // namespace OHOS
