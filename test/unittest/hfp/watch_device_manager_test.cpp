/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
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
#include <gmock/gmock.h>
#include <cstring>
#include "bluetooth_host.h"
#include "watch_service.h"
#include "hfp_hf_service.h"
#include "bt_def.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"
#include "connect_strategy_manager.h"
#include "profile_config.h"
#include "adapter_manager.h"
#include "adapter_config.h"
#include "class_creator.h"
#include "classic_adapter.h"
#include "parameters.h"
#include "bluetooth_connection_manager.h"
#include "mock/watch_service_interface_mock.h"
#include "watch_device_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
using namespace bluetooth::watch;

namespace OHOS {
namespace bluetooth {

std::string address("11:22:33:44:55:66");
std::string address2("00:11:22:33:44:55");
const int TIME = 1;
const uint16_t INVALID_VOIP_STATE = 3;
static std::unique_ptr<BondedDeviceManager> bondedDeviceManager_ {nullptr};


void btDisconnectTimerTest(void)
{
    HILOGI("btDisconnectTimerTest");
}

void connectHfpOnDemandTimerTest(void)
{
    HILOGI("connectHfpOnDemandTimerTest");
}

void connectHfpTimerTest(void)
{
    HILOGI("connectHfpTimerTest");
}

void controlScoConnectTimerTest(void)
{
    HILOGI("controlScoConnectTimerTest");
}

class BondedDeviceManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        HILOGI("SetUp");
        bondedDeviceManager_ = std::make_unique<BondedDeviceManager>(address);
    }
    void TearDown()
    {
        bondedDeviceManager_ = nullptr;
        HILOGI("TearDown");
    }
public:
};

/*
 * @tc.number: bonded_device_manager_GetAddressLocked_0001
 * @tc.name: GetAddressLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_GetAddressLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_GetAddressLocked_0001 start");

    EXPECT_EQ(bondedDeviceManager_->GetAddressLocked(), address);

    HILOGI("bonded_device_manager_GetAddressLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_resetAllStatus_0001
 * @tc.name: resetAllStatus
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_resetAllStatus_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_resetAllStatus_0001 start");

    bondedDeviceManager_->resetAllStatus();
    EXPECT_FALSE(bondedDeviceManager_->isPhoneCreateConnect_);

    HILOGI("bonded_device_manager_resetAllStatus_0001 end");
}

/*
 * @tc.number: bonded_device_manager_AddDevice_0001
 * @tc.name: AddDevice
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_AddDevice_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_AddDevice_0001 start");

    bool ret = GlobalDeviceManager::getInstance().AddDevice(address);
    EXPECT_TRUE(ret);
    ret = GlobalDeviceManager::getInstance().AddDevice(address);
    EXPECT_FALSE(ret);
    RemoveDeviceManager(address);

    HILOGI("bonded_device_manager_AddDevice_0001 end");
}

/*
 * @tc.number: bonded_device_manager_GetDevice_0001
 * @tc.name: GetDevice
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_GetDevice_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_GetDevice_0001 start");

    GlobalDeviceManager::getInstance().AddDevice(address);
    EXPECT_NE(GlobalDeviceManager::getInstance().GetDevice(address), nullptr);

    HILOGI("bonded_device_manager_GetDevice_0001 end");
}

/*
 * @tc.number: bonded_device_manager_GetSaisDevice_0001
 * @tc.name: GetSaisDevice
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_GetSaisDevice_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_GetSaisDevice_0001 start");

    EXPECT_EQ(GlobalDeviceManager::getInstance().GetSaisDevice(), nullptr);
    bondedDeviceManager_->SetIsSaisDeviceLocked(true);
    EXPECT_EQ(GlobalDeviceManager::getInstance().GetSaisDevice(), nullptr);

    HILOGI("bonded_device_manager_GetSaisDevice_0001 end");
}

/*
 * @tc.number: bonded_device_manager_GetAllAddresses_0001
 * @tc.name: GetAllAddresses
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_GetAllAddresses_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_GetAllAddresses_0001 start");

    EXPECT_FALSE(GlobalDeviceManager::getInstance().GetAllAddresses().empty());

    HILOGI("bonded_device_manager_GetAllAddresses_0001 end");
}

/*
 * @tc.number: bonded_device_manager_DeviceExists_0001
 * @tc.name: DeviceExists
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_DeviceExists_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_DeviceExists_0001 start");

    EXPECT_TRUE(GlobalDeviceManager::getInstance().DeviceExists(address));

    HILOGI("bonded_device_manager_DeviceExists_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsAllowAgCallLocked_0001
 * @tc.name: IsAllowAgCallLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsAllowAgCallLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsAllowAgCallLocked_0001 start");

    EXPECT_TRUE(bondedDeviceManager_->IsAllowAgCallLocked());

    HILOGI("bonded_device_manager_IsAllowAgCallLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsAllowConnectScoLocked_0001
 * @tc.name: IsAllowConnectScoLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsAllowConnectScoLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsAllowConnectScoLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsAllowConnectScoLocked());

    HILOGI("bonded_device_manager_IsAllowConnectScoLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_AllHfCallFinishedLocked_0001
 * @tc.name: AllHfCallFinishedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_AllHfCallFinishedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_AllHfCallFinishedLocked_0001 start");

    bondedDeviceManager_->AllHfCallFinishedLocked(true);
    EXPECT_FALSE(bondedDeviceManager_->isRouteScoRunning_);

    HILOGI("bonded_device_manager_AllHfCallFinishedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_ProcessCallStateActiveLocked_0001
 * @tc.name: ProcessCallStateActiveLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_ProcessCallStateActiveLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_ProcessCallStateActiveLocked_0001 start");

    bondedDeviceManager_->ProcessCallStateActiveLocked(10000);
    EXPECT_EQ(bondedDeviceManager_->callActiveTime_, 10000);

    HILOGI("bonded_device_manager_ProcessCallStateActiveLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_ProcessCallStateFinishedLocked_0001
 * @tc.name: ProcessCallStateFinishedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_ProcessCallStateFinishedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_ProcessCallStateFinishedLocked_0001 start");

    bondedDeviceManager_->ProcessCallStateFinishedLocked();
    EXPECT_TRUE(bondedDeviceManager_->isAllowHfScoConnect_);

    HILOGI("bonded_device_manager_ProcessCallStateFinishedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_ControlScoTimeoutLocked_0001
 * @tc.name: ControlScoTimeoutLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_ControlScoTimeoutLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_ControlScoTimeoutLocked_0001 start");

    bondedDeviceManager_->ControlScoTimeoutLocked();
    EXPECT_TRUE(bondedDeviceManager_->isAllowHfScoConnect_);

    HILOGI("bonded_device_manager_ControlScoTimeoutLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetPhoneConnHfpLocked_0001
 * @tc.name: SetPhoneConnHfpLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetPhoneConnHfpLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetPhoneConnHfpLocked_0001 start");

    bondedDeviceManager_->SetPhoneConnHfpLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isPhoneConnHfp_);

    HILOGI("bonded_device_manager_SetPhoneConnHfpLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsPhoneConnHfpLocked_0001
 * @tc.name: IsPhoneConnHfpLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsPhoneConnHfpLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsPhoneConnHfpLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsPhoneConnHfpLocked());

    HILOGI("bonded_device_manager_IsPhoneConnHfpLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetCallIDLocked_0001
 * @tc.name: SetCallIDLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetCallIDLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetCallIDLocked_0001 start");

    bondedDeviceManager_->SetCallIDLocked(1);
    EXPECT_EQ(bondedDeviceManager_->callID_, 1);


    HILOGI("bonded_device_manager_SetCallIDLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetAllowHfScoConnectLocked_0001
 * @tc.name: SetAllowHfScoConnectLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetAllowHfScoConnectLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetAllowHfScoConnectLocked_0001 start");

    bondedDeviceManager_->SetAllowHfScoConnectLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isAllowHfScoConnect_);

    HILOGI("bonded_device_manager_SetAllowHfScoConnectLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetDialingByWatchLocked_0001
 * @tc.name: SetDialingByWatchLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetDialingByWatchLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetDialingByWatchLocked_0001 start");

    bondedDeviceManager_->SetDialingByWatchLocked();
    EXPECT_TRUE(bondedDeviceManager_->isDialingByWatch_);

    HILOGI("bonded_device_manager_SetDialingByWatchLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsDialingByWatchLocked_0001
 * @tc.name: IsDialingByWatchLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsDialingByWatchLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsDialingByWatchLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsDialingByWatchLocked());

    HILOGI("bonded_device_manager_IsDialingByWatchLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetIsHfOnCallLocked_0001
 * @tc.name: SetIsHfOnCallLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetIsHfOnCallLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetIsHfOnCallLocked_0001 start");

    bondedDeviceManager_->SetIsHfOnCallLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isOnHfCall_);

    HILOGI("bonded_device_manager_SetIsHfOnCallLocked_0001 end");
}


/*
 * @tc.number: bonded_device_manager_IsHfOnCallLocked_0001
 * @tc.name: IsHfOnCallLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsHfOnCallLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsHfOnCallLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsHfOnCallLocked());

    HILOGI("bonded_device_manager_IsHfOnCallLocked_0001 end");
}


/*
 * @tc.number: bonded_device_manager_HandleVoipStateLocked_0001
 * @tc.name: HandleVoipStateLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_HandleVoipStateLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_HandleVoipStateLocked_0001 start");

    bondedDeviceManager_->HandleVoipStateLocked(VOIP_CALL_INCOMMING);
    EXPECT_TRUE(bondedDeviceManager_->isOnVoip_);
    bondedDeviceManager_->HandleVoipStateLocked(VOIP_CALL_ACCEPT);
    EXPECT_TRUE(bondedDeviceManager_->isAcceptVoipByWatch);
    bondedDeviceManager_->HandleVoipStateLocked(INVALID_VOIP_STATE);
    EXPECT_FALSE(bondedDeviceManager_->isAcceptVoipByWatch);

    HILOGI("bonded_device_manager_HandleVoipStateLocked_0001 end");
}


/*
 * @tc.number: bonded_device_manager_ProcessCallStateCheckLocked_0001
 * @tc.name: ProcessCallStateCheckLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_ProcessCallStateCheckLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_ProcessCallStateCheckLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->ProcessCallStateCheckLocked(2));

    HILOGI("bonded_device_manager_ProcessCallStateCheckLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetPhoneConnectStateLocked_0001
 * @tc.name: SetPhoneConnectStateLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetPhoneConnectStateLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetPhoneConnectStateLocked_0001 start");

    bondedDeviceManager_->SetPhoneConnectStateLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isPhoneConnected_);

    HILOGI("bonded_device_manager_SetPhoneConnectStateLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsPhoneConnectedLocked_0001
 * @tc.name: IsPhoneConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsPhoneConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsPhoneConnectedLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsPhoneConnectedLocked());

    HILOGI("bonded_device_manager_IsPhoneConnectedLocked_0001 end");
}


/*
 * @tc.number: bonded_device_manager_SetHfConnectedLocked_0001
 * @tc.name: SetHfConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetHfConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetHfConnectedLocked_0001 start");

    bondedDeviceManager_->SetHfConnectedLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isHfConnected_);

    HILOGI("bonded_device_manager_SetHfConnectedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetOnHfCallFalseLocked_0001
 * @tc.name: SetOnHfCallFalseLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetOnHfCallFalseLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetOnHfCallFalseLocked_0001 start");

    bondedDeviceManager_->SetOnHfCallFalseLocked();
    EXPECT_FALSE(bondedDeviceManager_->isOnHfCall_);

    HILOGI("bonded_device_manager_SetOnHfCallFalseLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsWatchHealthConnectedLocked_0001
 * @tc.name: IsWatchHealthConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsWatchHealthConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsWatchHealthConnectedLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsWatchHealthConnectedLocked());

    HILOGI("bonded_device_manager_IsWatchHealthConnectedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsHfConnectedLocked_0001
 * @tc.name: IsHfConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsHfConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsHfConnectedLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsHfConnectedLocked());

    HILOGI("bonded_device_manager_IsHfConnectedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetPhoneCreateConnectLocked_0001
 * @tc.name: SetPhoneCreateConnectLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetPhoneCreateConnectLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetPhoneCreateConnectLocked_0001 start");

    bondedDeviceManager_->SetPhoneCreateConnectLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isPhoneCreateConnect_);

    HILOGI("bonded_device_manager_SetPhoneCreateConnectLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetHfConnectionStateLocked_0001
 * @tc.name: SetHfConnectionStateLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetHfConnectionStateLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetHfConnectionStateLocked_0001 start");

    bondedDeviceManager_->SetHfConnectionStateLocked(1);
    EXPECT_EQ(bondedDeviceManager_->hfConnectionState_, 1);

    HILOGI("bonded_device_manager_SetHfConnectionStateLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_GetHfConnectionStateLocked_0001
 * @tc.name: GetHfConnectionStateLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_GetHfConnectionStateLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_GetHfConnectionStateLocked_0001 start");

    EXPECT_EQ(bondedDeviceManager_->GetHfConnectionStateLocked(), 0);

    HILOGI("bonded_device_manager_GetHfConnectionStateLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetIsConnectedLocked_0001
 * @tc.name: SetIsConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetIsConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetIsConnectedLocked_0001 start");

    bondedDeviceManager_->SetIsConnectedLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isConnected);

    HILOGI("bonded_device_manager_SetIsConnectedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsConnectedLocked_0001
 * @tc.name: IsConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsConnectedLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsConnectedLocked());

    HILOGI("bonded_device_manager_IsConnectedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetIsSaisDeviceLocked_0001
 * @tc.name: SetIsSaisDeviceLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetIsSaisDeviceLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetIsSaisDeviceLocked_0001 start");

    bondedDeviceManager_->SetIsSaisDeviceLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isSaisDevice);

    HILOGI("bonded_device_manager_SetIsSaisDeviceLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsSaisDeviceLocked_0001
 * @tc.name: IsSaisDeviceLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsSaisDeviceLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsSaisDeviceLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsSaisDeviceLocked());

    HILOGI("bonded_device_manager_IsSaisDeviceLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsJustConnectAclLocked_0001
 * @tc.name: IsJustConnectAclLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsJustConnectAclLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsJustConnectAclLocked_0001 start");

    EXPECT_TRUE(bondedDeviceManager_->IsJustConnectAclLocked());

    HILOGI("bonded_device_manager_IsJustConnectAclLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetJustConnectAclLocked_0001
 * @tc.name: SetJustConnectAclLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetJustConnectAclLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetJustConnectAclLocked_0001 start");

    bondedDeviceManager_->SetJustConnectAclLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isJustConnectAcl_);

    HILOGI("bonded_device_manager_SetJustConnectAclLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsRemovePairLocked_0001
 * @tc.name: IsRemovePairLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsRemovePairLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsRemovePairLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsRemovePairLocked());

    HILOGI("bonded_device_manager_IsRemovePairLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetRemovePairLocked_0001
 * @tc.name: SetRemovePairLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetRemovePairLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetRemovePairLocked_0001 start");

    bondedDeviceManager_->SetRemovePairLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isRemovePair_);

    HILOGI("bonded_device_manager_SetRemovePairLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsRouteScoRunningLocked_0001
 * @tc.name: IsRouteScoRunningLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsRouteScoRunningLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsRouteScoRunningLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsRouteScoRunningLocked());

    HILOGI("bonded_device_manager_IsRouteScoRunningLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_StopAllTimers_0001
 * @tc.name: StopAllTimers
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_StopAllTimers_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_StopAllTimers_0001 start");

    auto disconnectTimer = std::make_unique<utility::Timer>(std::bind(btDisconnectTimerTest));
    disconnectTimer->Start(1);
    bondedDeviceManager_->StartBtDisconnectTimerLocked(std::move(disconnectTimer));
    sleep(TIME);
    auto connHfpOnDemandTimer = std::make_unique<utility::Timer>(std::bind(connectHfpOnDemandTimerTest));
    connHfpOnDemandTimer->Start(1);
    bondedDeviceManager_->StartConnectHfpOnDemandTimerLocked(std::move(connHfpOnDemandTimer));
    sleep(TIME);
    auto connHfpTimer = std::make_unique<utility::Timer>(std::bind(connectHfpTimerTest));
    connHfpTimer->Start(1);
    bondedDeviceManager_->StartConnectHfpTimerLocked(std::move(connHfpTimer));
    sleep(TIME);
    auto controlScoConnTimer = std::make_unique<utility::Timer>(std::bind(controlScoConnectTimerTest));
    controlScoConnTimer->Start(1);
    bondedDeviceManager_->StartControlScoConnectTimerLocked(std::move(controlScoConnTimer));
    sleep(TIME);
    EXPECT_NE(bondedDeviceManager_->controlScoConnectTimer_, nullptr);
    bondedDeviceManager_->StopAllTimers();
    EXPECT_EQ(bondedDeviceManager_->controlScoConnectTimer_, nullptr);

    HILOGI("bonded_device_manager_StopAllTimers_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsAcceptVoipByWatchLocked_0001
 * @tc.name: IsAcceptVoipByWatchLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsAcceptVoipByWatchLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsAcceptVoipByWatchLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsAcceptVoipByWatchLocked());

    HILOGI("bonded_device_manager_IsAcceptVoipByWatchLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetNeedDialAfterHFConnectedLocked_0001
 * @tc.name: SetNeedDialAfterHFConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetNeedDialAfterHFConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetNeedDialAfterHFConnectedLocked_0001 start");

    bondedDeviceManager_->SetNeedDialAfterHFConnectedLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isNeedDialAfterHFConnected_);

    HILOGI("bonded_device_manager_SetNeedDialAfterHFConnectedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsNeedDialAfterHFConnectedLocked_0001
 * @tc.name: IsNeedDialAfterHFConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsNeedDialAfterHFConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsNeedDialAfterHFConnectedLocked_0001 start");

    EXPECT_FALSE(bondedDeviceManager_->IsNeedDialAfterHFConnectedLocked());

    HILOGI("bonded_device_manager_IsNeedDialAfterHFConnectedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetWatchHealthConnectedLocked_0001
 * @tc.name: SetWatchHealthConnectedLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetWatchHealthConnectedLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetWatchHealthConnectedLocked_0001 start");

    bondedDeviceManager_->SetWatchHealthConnectedLocked(true);
    EXPECT_TRUE(bondedDeviceManager_->isWatchHealthConnected_);

    HILOGI("bonded_device_manager_SetWatchHealthConnectedLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_SetHfConnectionStateDirectLocked_0001
 * @tc.name: SetHfConnectionStateDirectLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_SetHfConnectionStateDirectLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_SetHfConnectionStateDirectLocked_0001 start");

    bondedDeviceManager_->SetHfConnectionStateDirectLocked(1);
    EXPECT_EQ(bondedDeviceManager_->hfConnectionState_, 1);

    HILOGI("bonded_device_manager_SetHfConnectionStateDirectLocked_0001 end");
}
/*
 * @tc.number: bonded_device_manager_StartBtDisconnectTimerLocked_0001
 * @tc.name: StartBtDisconnectTimerLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_StartBtDisconnectTimerLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_StartBtDisconnectTimerLocked_0001 start");

    auto disconnectTimer = std::make_unique<utility::Timer>(std::bind(btDisconnectTimerTest));
    disconnectTimer->Start(1);
    bondedDeviceManager_->StartBtDisconnectTimerLocked(std::move(disconnectTimer));
    EXPECT_FALSE(bondedDeviceManager_->IsBtDisconnectTimerNullLocked());

    bondedDeviceManager_->StopBtDisconnectTimerLocked();
    EXPECT_TRUE(bondedDeviceManager_->IsBtDisconnectTimerNullLocked());

    HILOGI("bonded_device_manager_StartBtDisconnectTimerLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_StartConnectHfpOnDemandTimerLocked_0001
 * @tc.name: StartConnectHfpOnDemandTimerLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_StartConnectHfpOnDemandTimerLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_StartConnectHfpOnDemandTimerLocked_0001 start");

    auto connHfpOnDemandTimer = std::make_unique<utility::Timer>(std::bind(connectHfpOnDemandTimerTest));
    connHfpOnDemandTimer->Start(1);
    bondedDeviceManager_->StartConnectHfpOnDemandTimerLocked(std::move(connHfpOnDemandTimer));
    EXPECT_FALSE(bondedDeviceManager_->IsConnectHfpOnDemandTimerNullLocked());

    bondedDeviceManager_->StopConnectHfpOnDemandTimerLocked();
    EXPECT_TRUE(bondedDeviceManager_->IsConnectHfpOnDemandTimerNullLocked());

    HILOGI("bonded_device_manager_StartConnectHfpOnDemandTimerLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_StartControlScoConnectTimerLocked_0001
 * @tc.name: StartControlScoConnectTimerLocked
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_StartControlScoConnectTimerLocked_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_StartControlScoConnectTimerLocked_0001 start");

    auto controlScoConnTimer = std::make_unique<utility::Timer>(std::bind(controlScoConnectTimerTest));
    controlScoConnTimer->Start(1);
    bondedDeviceManager_->StartControlScoConnectTimerLocked(std::move(controlScoConnTimer));
    EXPECT_FALSE(bondedDeviceManager_->IsControlScoConnectTimerNullLocked());

    bondedDeviceManager_->StopControlScoConnectTimerLocked();
    EXPECT_TRUE(bondedDeviceManager_->IsControlScoConnectTimerNullLocked());

    HILOGI("bonded_device_manager_StartControlScoConnectTimerLocked_0001 end");
}

/*
 * @tc.number: bonded_device_manager_IsAllowConnectHfpAgSco_0001
 * @tc.name: IsAllowConnectHfpAgSco
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_IsAllowConnectHfpAgSco_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_IsAllowConnectHfpAgSco_0001 start");

    EXPECT_TRUE(GlobalDeviceManager::getInstance().IsAllowConnectHfpAgSco());

    HILOGI("bonded_device_manager_IsAllowConnectHfpAgSco_0001 end");
}

/*
 * @tc.number: bonded_device_manager_ResetDeviceStatus_0001
 * @tc.name: ResetDeviceStatus
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_ResetDeviceStatus_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_ResetDeviceStatus_0001 start");

    EXPECT_TRUE(GlobalDeviceManager::getInstance().ResetDeviceStatus(address));
    EXPECT_FALSE(GlobalDeviceManager::getInstance().ResetDeviceStatus(address2));

    HILOGI("bonded_device_manager_ResetDeviceStatus_0001 end");
}

/*
 * @tc.number: bonded_device_manager_GetDeviceCount_0001
 * @tc.name: GetDeviceCount
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_GetDeviceCount_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_GetDeviceCount_0001 start");

    RemoveDeviceManager(address);
    AddDeviceManager(address);
    EXPECT_EQ(GlobalDeviceManager::getInstance().GetDeviceCount(), 1);

    HILOGI("bonded_device_manager_GetDeviceCount_0001 end");
}

/*
 * @tc.number: bonded_device_manager_RemoveDevice_0001
 * @tc.name: RemoveDevice
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_RemoveDevice_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_RemoveDevice_0001 start");

    EXPECT_FALSE(GlobalDeviceManager::getInstance().RemoveDevice(address2));
    EXPECT_TRUE(GlobalDeviceManager::getInstance().RemoveDevice(address));

    HILOGI("bonded_device_manager_RemoveDevice_0001 end");
}

/*
 * @tc.number: bonded_device_manager_GetDeviceManager_0001
 * @tc.name: GetDeviceManager
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_GetDeviceManager_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_GetDeviceManager_0001 start");

    AddDeviceManager(address);
    EXPECT_NE(GetDeviceManager(address), nullptr);

    HILOGI("bonded_device_manager_GetDeviceManager_0001 end");
}

/*
 * @tc.number: bonded_device_manager_RemoveDeviceManager_0001
 * @tc.name: RemoveDeviceManager
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_RemoveDeviceManager_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_RemoveDeviceManager_0001 start");

    EXPECT_TRUE(RemoveDeviceManager(address));

    HILOGI("bonded_device_manager_RemoveDeviceManager_0001 end");
}

/*
 * @tc.number: bonded_device_manager_AddDeviceManager_0001
 * @tc.name: AddDeviceManager
 * @tc.desc:
 */
HWTEST_F(BondedDeviceManagerTest, bonded_device_manager_AddDeviceManager_0001, TestSize.Level1)
{
    HILOGI("bonded_device_manager_AddDeviceManager_0001 start");

    EXPECT_TRUE(AddDeviceManager(address));
    EXPECT_TRUE(RemoveDeviceManager(address));

    HILOGI("bonded_device_manager_AddDeviceManager_0001 end");
}
}  // namespace bluetooth
}  // namespace OHOS
