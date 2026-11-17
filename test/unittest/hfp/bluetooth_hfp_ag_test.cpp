/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "bluetooth_hfp_ag.h"
#include "bluetooth_hfp_ag_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

constexpr int TIME = 4;

namespace OHOS {
namespace Bluetooth {
static BluetoothHost *host_;
static HandsFreeAudioGateway *profile_;

class BluetoothHfpAgTest : public testing::Test {
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

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase start");
        sleep(TIME);
        host_ = nullptr;
        HILOGI("TearDownTestCase end");
    }
    void SetUp()
    {
        profile_ = HandsFreeAudioGateway::GetProfile();
    }
    void TearDown()
    {
        profile_ = nullptr;
    }
};

/*
 * @tc.number: Connect_001
 * @tc.name: Connect
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(BluetoothHfpAgTest, Connect_001, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_Connect_Test_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int result = profile_->Connect(device);
    EXPECT_NE(result, BT_NO_ERROR);
    HILOGI("BluetoothHfpAg_UnitTest_Connect_Test_001 end");
}

/*
 * @tc.number: Connect_Test_002
 * @tc.name: Connect
 * @tc.desc: IsValidBluetoothRemoteDevice():true
*/
HWTEST_F(BluetoothHfpAgTest, Connect_Test_002, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_Connect_Test_002 start");
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int result = profile_->Connect(device);
    EXPECT_NE(result, BT_ERR_INTERNAL_ERROR);
    HILOGI("BluetoothHfpAg_UnitTest_Connect_Test_002 end");
}

/*
 * @tc.number: GetDeviceState_Test_001
 * @tc.name: GetDeviceState
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(BluetoothHfpAgTest, GetDeviceState_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetDeviceState_Test_001 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::CONNECTED);
    int result = profile_->GetDeviceState(device, state);
    EXPECT_EQ(result, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothHfpAg_UnitTest_GetDeviceState_Test_001 end");
}

/*
 * @tc.number: GetScoState_Test_001
 * @tc.name: GetScoState
 * @tc.desc: IS_BT_ENABLED():true, IsValidBluetoothRemoteDevice():true
*/
HWTEST_F(BluetoothHfpAgTest, GetScoState_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetScoState_Test_001 start");
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int result = profile_->GetScoState(device);
    HILOGI("BluetoothHfpAg_UnitTest_GetScoState_Test_001 end");
}

/*
 * @tc.number: GetScoState_Test_002
 * @tc.name: GetScoState
 * @tc.desc: IS_BT_ENABLED():false, IsValidBluetoothRemoteDevice():true
*/
HWTEST_F(BluetoothHfpAgTest, GetScoState_Test_002, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetScoState_Test_002 start");
    int btState = host_->GetBtState();
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        host_->DisableBle();
        sleep(TIME);
    }
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int result = profile_->GetScoState(device);
    EXPECT_EQ(result, 3);
    host_->EnableBt();
    host_->EnableBle();
    sleep(TIME);
    HILOGI("BluetoothHfpAg_UnitTest_GetScoState_Test_002 end");
}

/*
 * @tc.number: GetScoState_Test_003
 * @tc.name: GetScoState
 * @tc.desc: IS_BT_ENABLED():true, IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(BluetoothHfpAgTest, GetScoState_Test_003, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetScoState_Test_003 start");
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int result = profile_->GetScoState(device);
    EXPECT_EQ(result, 3);
    HILOGI("BluetoothHfpAg_UnitTest_GetScoState_Test_003 end");
}

/*
 * @tc.number: IsValidCallType_Test_001
 * @tc.name: IsValidCallType
 * @tc.desc:
*/
HWTEST_F(BluetoothHfpAgTest, IsValidCallType_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_IsValidCallType_Test_001 start");
    uint8_t callType = static_cast<uint8_t>(BTCallType::CALL_TYPE_CELLULAR);
    bool result = profile_->ConnectSco(callType);
    EXPECT_TRUE(result);
    HILOGI("BluetoothHfpAg_UnitTest_IsValidCallType_Test_001 end");
}

/*
 * @tc.number: IsValidCallType_Test_002
 * @tc.name: IsValidCallType
 * @tc.desc:
*/
HWTEST_F(BluetoothHfpAgTest, IsValidCallType_Test_002, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_IsValidCallType_Test_002 start");
    uint8_t callType = static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL);
    bool result = profile_->ConnectSco(callType);
    EXPECT_TRUE(result);
    HILOGI("BluetoothHfpAg_UnitTest_IsValidCallType_Test_002 end");
}

/*
 * @tc.number: IsValidCallType_Test_003
 * @tc.name: IsValidCallType
 * @tc.desc:
*/
HWTEST_F(BluetoothHfpAgTest, IsValidCallType_Test_003, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_IsValidCallType_Test_003 start");
    uint8_t callType = static_cast<uint8_t>(BTCallType::CALL_TYPE_RECOGNITION);
    bool result = profile_->ConnectSco(callType);
    EXPECT_TRUE(result);
    HILOGI("BluetoothHfpAg_UnitTest_IsValidCallType_Test_003 end");
}

/*
 * @tc.number: IsValidCallType_Test_004
 * @tc.name: IsValidCallType
 * @tc.desc:
*/
HWTEST_F(BluetoothHfpAgTest, IsValidCallType_Test_004, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_IsValidCallType_Test_004 start");
    uint8_t callType = 0xFF;
    bool result = profile_->ConnectSco(callType);
    EXPECT_TRUE(result);
    HILOGI("BluetoothHfpAg_UnitTest_IsValidCallType_Test_004 end");
}

/*
 * @tc.number: IsAudioConnected_Test_001
 * @tc.name: IsAudioConnected
 * @tc.desc:
*/
HWTEST_F(BluetoothHfpAgTest, IsAudioConnected_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_IsAudioConnected_Test_001 start");
    bool isAudioOn = false;
    profile_->IsAudioConnected(isAudioOn);
    EXPECT_FALSE(isAudioOn);
    HILOGI("BluetoothHfpAg_UnitTest_IsAudioConnected_Test_001 end");
}

/*
 * @tc.number: GetConnectedDevices_Test_001
 * @tc.name: GetConnectedDevices
 * @tc.desc: IS_BT_ENABLED():true
*/
HWTEST_F(BluetoothHfpAgTest, GetConnectedDevices_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectedDevices_Test_001 start");
    std::vector<BluetoothRemoteDevice> result = profile_->GetConnectedDevices();
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectedDevices_Test_001 end");
}

/*
 * @tc.number: GetConnectedDevices_Test_002
 * @tc.name: GetConnectedDevices
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(BluetoothHfpAgTest, GetConnectedDevices_Test_002, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectedDevices_Test_002 start");
    int btState = host_->GetBtState();
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        host_->DisableBle();
        sleep(TIME);
    }
    std::vector<BluetoothRemoteDevice> result = profile_->GetConnectedDevices();
    host_->EnableBt();
    host_->EnableBle();
    sleep(TIME);
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectedDevices_Test_002 end");
}

/*
 * @tc.number: GetDevicesByStates_Test_001
 * @tc.name: GetDevicesByStates
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(BluetoothHfpAgTest, GetDevicesByStates_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetDevicesByStates_Test_001 start");
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRemoteDevice> result = profile_->GetDevicesByStates(states);
    HILOGI("BluetoothHfpAg_UnitTest_GetDevicesByStates_Test_001 end");
}

/*
 * @tc.number: GetConnectStrategy_Test_001
 * @tc.name: GetConnectStrategy
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(BluetoothHfpAgTest, GetConnectStrategy_Test_001, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectStrategy_Test_001 start");
    int btState = host_->GetBtState();
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        host_->DisableBle();
        sleep(TIME);
    }
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    profile_->SetConnectStrategy(device, strategy);
    int result = profile_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(result, BT_ERR_INVALID_STATE);
    host_->EnableBt();
    host_->EnableBle();
    sleep(TIME);
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectStrategy_Test_001 end");
}

/*
 * @tc.number: GetConnectStrategy_Test_002
 * @tc.name: GetConnectStrategy
 * @tc.desc: IsValidBluetoothRemoteDevice():false
*/
HWTEST_F(BluetoothHfpAgTest, GetConnectStrategy_Test_002, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectStrategy_Test_002 start");
    BluetoothRemoteDevice device("0:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    profile_->SetConnectStrategy(device, strategy);
    int result = profile_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(result, BT_ERR_INVALID_PARAM);
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectStrategy_Test_002 end");
}

/*
 * @tc.number: GetConnectStrategy_Test_003
 * @tc.name: GetConnectStrategy
 * @tc.desc: strategy: CONNECTION_ALLOWED
*/
HWTEST_F(BluetoothHfpAgTest, GetConnectStrategy_Test_003, TestSize.Level1)
{
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectStrategy_Test_003 start");
    BluetoothRemoteDevice device("00:00:00:00:00:11", BT_TRANSPORT_BREDR);
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    profile_->SetConnectStrategy(device, strategy);
    int result = profile_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(result, BT_NO_ERROR);
    HILOGI("BluetoothHfpAg_UnitTest_GetConnectStrategy_Test_003 end");
}

}  // namespace Bluetooth
}  // namespace OHOS