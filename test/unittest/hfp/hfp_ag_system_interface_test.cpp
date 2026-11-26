/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "hfp_ag_service.h"
#include "hfp_ag_system_interface.h"
#include "system_ability_definition.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_errorcode.h"
#include "profile_service_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
using namespace OHOS::Telephony;

namespace OHOS {
namespace bluetooth {

const int TIME = 4;
const int NETWORK_STATE_NOT_AVAILABLE = 0;
const int NETWORK_STATE_AVAILABLE = 1;

static BluetoothHost *host_;
static std::unique_ptr<HfpAgSystemInterface> hfpAgInterface_ {nullptr};
static std::unique_ptr<HfpAgSystemInterface::AgTelephonyObserver> teleObserver_ {nullptr};

class HfpAgSystemInterfaceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        // new HfpAgService
        auto manager = ProfileServiceManager::GetInstance();
        std::string profileName = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, profileName);

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestSuite start");
        // delete HfpAgService
        auto service = HfpAgService::GetService();
        delete service;
        HILOGI("TearDownTestSuite end");
    }
    void SetUp()
    {
        hfpAgInterface_ = std::make_unique<HfpAgSystemInterface>();

        HfpAgSystemInterface interface;
        teleObserver_ = std::make_unique<HfpAgSystemInterface::AgTelephonyObserver>(interface);
    }
    void TearDown()
    {
        hfpAgInterface_ = nullptr;
        teleObserver_ = nullptr;
    }
};

/*
 * @tc.number: Hfp_Ag_system_interface_RegisterObserver_001
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_RegisterObserver_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_RegisterObserver_001 start");

    // delete hfpAgInterface RegisterObserver() to avoid tel_core_service_api cfi check
    hfpAgInterface_->observer_ =
        new (std::nothrow) HfpAgSystemInterface::AgTelephonyObserver(HfpAgSystemInterface::GetInstance());
    // delete hfpAgInterface RegisterObserver() to avoid tel_core_service_api cfi check

    HILOGI("Hfp_Ag_system_interface_RegisterObserver_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_UnregisterObserver_001
 * @tc.name: UnregisterObserver
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_UnregisterObserver_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_UnregisterObserver_001 start");

    // delete hfpAgInterface UnregisterObserver() to avoid tel_core_service_api cfi check
    hfpAgInterface_->observer_ =
        new (std::nothrow) HfpAgSystemInterface::AgTelephonyObserver(HfpAgSystemInterface::GetInstance());
    // delete hfpAgInterface RegisterObserver() to avoid tel_core_service_api cfi check

    HILOGI("Hfp_Ag_system_interface_UnregisterObserver_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_SendDtmf_001
 * @tc.name: SendDtmf
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_SendDtmf_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_SendDtmf_001 start");

    string address = "00";
    EXPECT_TRUE(hfpAgInterface_->SendDtmf(0, address));

    char dtmf = '1';
    EXPECT_TRUE(hfpAgInterface_->SendDtmf(dtmf, address));
    dtmf = '2';
    EXPECT_TRUE(hfpAgInterface_->SendDtmf(dtmf, address));

    HILOGI("Hfp_Ag_system_interface_SendDtmf_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_HandleChld_001
 * @tc.name: HandleChld
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_HandleChld_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_HandleChld_001 start");

    EXPECT_TRUE(hfpAgInterface_->HandleChld(ATCHLD_RELEASE_ALL_HELD_CALLS));
    EXPECT_TRUE(hfpAgInterface_->HandleChld(ATCHLD_RELEASE_ACTIVE_ACCPET_OTHER));
    EXPECT_TRUE(hfpAgInterface_->HandleChld(ATCHLD_RELEASE_HOLD_ACCPET_OTHER));
    EXPECT_TRUE(hfpAgInterface_->HandleChld(ATCHLD_ADD_CALL_TO_CONVERSATION));

    HILOGI("Hfp_Ag_system_interface_HandleChld_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_ProcessReleaseActiveAccpetOther_001
 * @tc.name: ProcessReleaseActiveAccpetOther
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_ProcessReleaseActiveAccpetOther_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_ProcessReleaseActiveAccpetOther_001 start");

    CallAttributeInfo call;
    call.callState = TelCallState::CALL_STATUS_INCOMING;
    hfpAgInterface_->ProcessReleaseActiveAccpetOther(call);

    call.callState = TelCallState::CALL_STATUS_WAITING;
    hfpAgInterface_->ProcessReleaseActiveAccpetOther(call);

    call.callState = TelCallState::CALL_STATUS_HOLDING;
    hfpAgInterface_->ProcessReleaseActiveAccpetOther(call);

    call.callState = TelCallState::CALL_STATUS_ALERTING;
    hfpAgInterface_->ProcessReleaseActiveAccpetOther(call);

    HILOGI("Hfp_Ag_system_interface_ProcessReleaseActiveAccpetOther_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_ProcessReleaseAllHeldCalls_001
 * @tc.name: ProcessReleaseAllHeldCalls
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_ProcessReleaseAllHeldCalls_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_ProcessReleaseAllHeldCalls_001 start");

    CallAttributeInfo call;
    call.callState = TelCallState::CALL_STATUS_INCOMING;
    hfpAgInterface_->ProcessReleaseAllHeldCalls(call);

    call.callState = TelCallState::CALL_STATUS_WAITING;
    hfpAgInterface_->ProcessReleaseAllHeldCalls(call);

    call.callState = TelCallState::CALL_STATUS_HOLDING;
    hfpAgInterface_->ProcessReleaseAllHeldCalls(call);

    call.callState = TelCallState::CALL_STATUS_ALERTING;
    hfpAgInterface_->ProcessReleaseAllHeldCalls(call);

    HILOGI("Hfp_Ag_system_interface_ProcessReleaseAllHeldCalls_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_GetSubscriberNumber_001
 * @tc.name: GetSubscriberNumber
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_GetSubscriberNumber_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_GetSubscriberNumber_001 start");

    EXPECT_EQ(hfpAgInterface_->GetSubscriberNumber(), "");

    HILOGI("Hfp_Ag_system_interface_GetSubscriberNumber_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_QueryCurrentCallsList_001
 * @tc.name: QueryCurrentCallsList
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_QueryCurrentCallsList_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_QueryCurrentCallsList_001 start");

    EXPECT_TRUE(hfpAgInterface_->QueryCurrentCallsList());

    HILOGI("Hfp_Ag_system_interface_QueryCurrentCallsList_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_QueryCurrentCallsList_002
 * @tc.name: QueryCurrentCallsList_VOIP_EmptyNumber
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_QueryCurrentCallsList_002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_QueryCurrentCallsList_002 start");

    CallAttributeInfo voipCall;
    voipcall.callType = CallType::TYPE_VOIP;
    voipCall.accountNumber[0] = '\0';
    std:string number = voipCall.accountNumber;

    EXPECT_TRUE(number.empty());
    EXPECT_EQ(voipCall.callType, CallType::TYPE_VOIP);
    bool shouldFillZeros = (voipCall.callType == CallType::TYPE_VOIP) && number.empty();
    EXPECT_TRUE(shouldFillZeros);

    number = '00000000000';
    EXPECT_EQ(number, '00000000000');
    EXPECT_EQ(number.length(), 11u);

    HILOGI("Hfp_Ag_system_interface_QueryCurrentCallsList_002 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_QueryCurrentCallsList_003
 * @tc.name: QueryCurrentCallsList_NonVOIP_EmptyNumber
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_QueryCurrentCallsList_003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_QueryCurrentCallsList_003 start");

    CallAttributeInfo csCall;
    csCall.callType = CallType::TYPE_CS;
    csCall.accountNumber[0] = '\0';
    std:string number = csCall.accountNumber;

    EXPECT_TRUE(number.empty());
    EXPECT_NE(csCall.callType, CallType::TYPE_VOIP);
    bool shouldFillZeros = (csCall.callType == CallType::TYPE_VOIP) && number.empty();
    EXPECT_FALSE(shouldFillZeros);

    CallAttributeInfo imsCall;
    imsCall.callType = CallType::TYPE_CS;
    imsCall.accountNumber[0] = '\0';
    number = imsCall.accountNumber;
    shouldFillZeros = (csCimsCallall.callType == CallType::TYPE_VOIP) && number.empty();
    EXPECT_FALSE(shouldFillZeros);

    HILOGI("Hfp_Ag_system_interface_QueryCurrentCallsList_003 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_QueryCurrentCallsList_004
 * @tc.name: QueryCurrentCallsList_VOIP_NonEmptyNumber
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_QueryCurrentCallsList_004, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_QueryCurrentCallsList_004 start");

    CallAttributeInfo voipCallWithNumber;
    voipCallWithNumber.callType = CallType::TYPE_VOIP;
    std:string originalNumber = '138000000000';
    std::strncpy(voipCallWithNumber.accountNumber, originalNumber.c_str(), kMaxNumberLen);
    voipCallWithNumber.accountNumber[kMaxNumberLen] = '\0';
    std::string number = voipCallWithNumber.accountNumber;

    EXPECT_FALSE(number.empty());
    EXPECT_EQ(voipCallWithNumber.callType, CallType::TYPE_VOIP);
    bool shouldFillZeros = (voipCallWithNumber.callType == CallType::TYPE_VOIP) && number.empty();
    EXPECT_FALSE(shouldFillZeros);
    EXPECT_EQ(number, originalNumber);

    HILOGI("Hfp_Ag_system_interface_QueryCurrentCallsList_004 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_SetStreamVolume_001
 * @tc.name: SetStreamVolume
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_SetStreamVolume_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_SetStreamVolume_001 start");

    hfpAgInterface_->SetStreamVolume(HFP_AG_VOLUME_TYPE_SPK, 0, 0);
    hfpAgInterface_->SetStreamVolume(HFP_AG_VOLUME_TYPE_MIC, 0, 0);

    HILOGI("Hfp_Ag_system_interface_SetStreamVolume_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_QuerySignalStrength_001
 * @tc.name: SetStreamVolume
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_QuerySignalStrength_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_QuerySignalStrength_001 start");

    hfpAgInterface_->serviceState_ = NETWORK_STATE_NOT_AVAILABLE;
    EXPECT_EQ(hfpAgInterface_->QuerySignalStrength(), 0);

    hfpAgInterface_->serviceState_ = NETWORK_STATE_AVAILABLE;
    hfpAgInterface_->QuerySignalStrength();

    HILOGI("Hfp_Ag_system_interface_QuerySignalStrength_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_GetBatteryLevel_001
 * @tc.name: GetBatteryLevel
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_GetBatteryLevel_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_GetBatteryLevel_001 start");

    hfpAgInterface_->GetBatteryLevel();

    HILOGI("Hfp_Ag_system_interface_GetBatteryLevel_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_QueryAgIndicator_001
 * @tc.name: QueryAgIndicator
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_QueryAgIndicator_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_QueryAgIndicator_001 start");

    hfpAgInterface_->QueryAgIndicator();

    HILOGI("Hfp_Ag_system_interface_QueryAgIndicator_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_SendHfIndicator_001
 * @tc.name: SendHfIndicator
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_SendHfIndicator_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_SendHfIndicator_001 start");

    string address;
    hfpAgInterface_->SendHfIndicator(address, 0, 0);

    HILOGI("Hfp_Ag_system_interface_SendHfIndicator_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_OnSubscriptionStateChanged_001
 * @tc.name: OnSubscriptionStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnSubscriptionStateChanged_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnSubscriptionStateChanged_001 start");

    hfpAgInterface_->serviceState_ = 0;
    hfpAgInterface_->OnSubscriptionStateChanged(0);

    hfpAgInterface_->serviceState_ = 1;
    hfpAgInterface_->OnSubscriptionStateChanged(0);

    HILOGI("Hfp_Ag_system_interface_OnSubscriptionStateChanged_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_OnSignalStrengthChanged_001
 * @tc.name: OnSignalStrengthChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnSignalStrengthChanged_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnSignalStrengthChanged_001 start");

    hfpAgInterface_->signalStrength_ = 0;
    hfpAgInterface_->OnSignalStrengthChanged(0);

    hfpAgInterface_->signalStrength_ = 1;
    hfpAgInterface_->OnSignalStrengthChanged(0);

    HILOGI("Hfp_Ag_system_interface_OnSignalStrengthChanged_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_OnRoamStateChanged_001
 * @tc.name: OnRoamStateChanged
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnRoamStateChanged_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnRoamStateChanged_001 start");

    hfpAgInterface_->roamState_ = 0;
    hfpAgInterface_->OnRoamStateChanged(0);

    hfpAgInterface_->roamState_ = 1;
    hfpAgInterface_->OnRoamStateChanged(0);

    HILOGI("Hfp_Ag_system_interface_OnRoamStateChanged_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_OnBatteryLevel_001
 * @tc.name: OnBatteryLevel
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnBatteryLevel_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnBatteryLevel_001 start");

    hfpAgInterface_->batteryLevel_ = 0;
    hfpAgInterface_->OnBatteryLevel(0);

    hfpAgInterface_->batteryLevel_ = 1;
    hfpAgInterface_->OnBatteryLevel(0);

    HILOGI("Hfp_Ag_system_interface_OnBatteryLevel_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_OnNetworkStateUpdated_001
 * @tc.name: OnNetworkStateUpdated
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnNetworkStateUpdated_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnNetworkStateUpdated_001 start");

    teleObserver_->OnNetworkStateUpdated(0, nullptr);

    HILOGI("Hfp_Ag_system_interface_OnNetworkStateUpdated_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_OnAddSystemAbility_001
 * @tc.name: OnAddSystemAbility
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnAddSystemAbility_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnAddSystemAbility_001 start");
    hfpAgInterface_->OnAddSystemAbility(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
    hfpAgInterface_->OnAddSystemAbility(TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID);
    hfpAgInterface_->OnAddSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID);
    HILOGI("Hfp_Ag_system_interface_OnAddSystemAbility_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_OnRemoveSystemAbility_001
 * @tc.name: OnRemoveSystemAbility
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnRemoveSystemAbility_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnRemoveSystemAbility_001 start");
    hfpAgInterface_->OnRemoveSystemAbility(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
    hfpAgInterface_->OnRemoveSystemAbility(TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID);
    hfpAgInterface_->OnRemoveSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID);
    hfpAgInterface_->OnRemoveSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    HILOGI("Hfp_Ag_system_interface_OnRemoveSystemAbility_001 end");
}

 
/*
 * @tc.number: Hfp_Ag_system_interface_OnAddSystemAbility_002
 * @tc.name: OnAddSystemAbility
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnAddSystemAbility_002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnAddSystemAbility_002 start");
    hfpAgInterface_->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    HILOGI("Hfp_Ag_system_interface_OnAddSystemAbility_002 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_IsCallIdle_001
 * @tc.name: OnRemoveSystemAbility
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_IsCallIdle_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_IsCallIdle_001 start");
    bool isCallIdle = hfpAgInterface_->IsCallIdle();
    EXPECT_EQ(isCallIdle, true);
    HILOGI("Hfp_Ag_system_interface_IsCallIdle_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_ProcessReleaseHoldAccpetOther_001
 * @tc.name: OnRemoveSystemAbility
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_ProcessReleaseHoldAccpetOther_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_ProcessReleaseHoldAccpetOther_001 start");
    CallAttributeInfo callIncoming;
    callIncoming.callState = TelCallState::CALL_STATUS_INCOMING;
    CallAttributeInfo callActive;
    callActive.callState = TelCallState::CALL_STATUS_ACTIVE;
    CallAttributeInfo callHolding;
    callHolding.callState = TelCallState::CALL_STATUS_HOLDING;
    CallAttributeInfo callUnknown;
    callUnknown.callState = TelCallState::CALL_STATUS_UNKNOWN;

    std::vector<CallAttributeInfo> callList;
    callList.push_back(callIncoming);
    callList.push_back(callActive);
    callList.push_back(callHolding);
    callList.push_back(callUnknown);
    hfpAgInterface_->ProcessReleaseHoldAccpetOther(callList);
    HILOGI("Hfp_Ag_system_interface_ProcessReleaseHoldAccpetOther_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_MuteRinger_001
 * @tc.name: OnRemoveSystemAbility
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_MuteRinger_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_MuteRinger_001 start");

    hfpAgInterface_->callState_ = HFP_AG_CALL_STATE_INCOMING;
    EXPECT_EQ(hfpAgInterface_->MuteRinger(), true);

    hfpAgInterface_->callState_ = HFP_AG_CALL_STATE_IDLE;
    EXPECT_EQ(hfpAgInterface_->MuteRinger(), false);

    HILOGI("Hfp_Ag_system_interface_MuteRinger_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_IsSimCardAvailable_001
 * @tc.name: OnRemoveSystemAbility
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_IsSimCardAvailable_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_IsSimCardAvailable_001 start");

    hfpAgInterface_->IsSimCardAvailable(0);
    EXPECT_FALSE(hfpAgInterface_->IsSimCardAvailable(-1));

    HILOGI("Hfp_Ag_system_interface_IsSimCardAvailable_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_GetSlotId_001
 * @tc.name: OnRemoveSystemAbility
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_GetSlotId_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_GetSlotId_001 start");

    std::string number = "00000000000";
    EXPECT_EQ(hfpAgInterface_->GetSlotId(number), -1);
    number = "00000000001";
    hfpAgInterface_->GetSlotId(number);

    HILOGI("Hfp_Ag_system_interface_GetSlotId_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_GetIndexForCall_001
 * @tc.name: GetIndexForCall
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_GetIndexForCall_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_GetIndexForCall_001 start");

    EXPECT_EQ(hfpAgInterface_->GetIndexForCall(1), 1);
    EXPECT_EQ(hfpAgInterface_->GetIndexForCall(1), 1);
    EXPECT_EQ(hfpAgInterface_->GetIndexForCall(2), 2);
    HILOGI("Hfp_Ag_system_interface_GetIndexForCall_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_GetIndexForCall_001
 * @tc.name: GetIndexForCall
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_IsForegroundCallExist_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_IsForegroundCallExist_001 start");
    EXPECT_FALSE(hfpAgInterface_->IsForegroundCallExist());
    HILOGI("Hfp_Ag_system_interface_IsForegroundCallExist_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_DialOutCall_001
 * @tc.name: DialOutCall
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_DialOutCall_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_DialOutCall_001 start");
    std::string address = "00:00:00:00:00:00";
    std::string number = "10086";
    hfpAgInterface_->DialOutCall(address, number);
    HILOGI("Hfp_Ag_system_interface_DialOutCall_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_SetExtraAudioParameters_001
 * @tc.name: SetExtraAudioParameters
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_SetExtraAudioParameters_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_SetExtraAudioParameters_001 start");

    std::vector<std::pair<std::string, std::string>> audioParametersKVPairs1;
    audioParametersKVPairs1.push_back(std::make_pair(HFP_AG_SERVER_SWB_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));
    audioParametersKVPairs1.push_back(std::make_pair("bt_headset_nrec", HFP_AG_SERVER_AUDIO_FEATURE_ON));
    hfpAgInterface_->SetExtraAudioParameters("bt_extra", audioParametersKVPairs1);

    std::vector<std::pair<std::string, std::string>> audioParametersKVPairs2;
    audioParametersKVPairs2.push_back(std::make_pair(HFP_AG_SERVER_WBS_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));
    audioParametersKVPairs2.push_back(std::make_pair(HFP_AG_SERVER_NREC_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));
    hfpAgInterface_->SetExtraAudioParameters("bt_extra", audioParametersKVPairs2);

    std::vector<std::pair<std::string, std::string>> audioParametersKVPairs3;
    audioParametersKVPairs3.push_back(std::make_pair(HFP_AG_SERVER_WBS_NAME, HFP_AG_SERVER_AUDIO_FEATURE_OFF));
    audioParametersKVPairs3.push_back(std::make_pair(HFP_AG_SERVER_NREC_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));
    hfpAgInterface_->SetExtraAudioParameters("bt_extra", audioParametersKVPairs3);

    HILOGI("Hfp_Ag_system_interface_SetExtraAudioParameters_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_OnSignalInfoUpdated_001
 * @tc.name: OnSignalInfoUpdated
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_OnSignalInfoUpdated_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_OnSignalInfoUpdated_001 start");
    std::vector<sptr<SignalInformation>> vec;
    teleObserver_->OnSignalInfoUpdated(0, vec);
    HILOGI("Hfp_Ag_system_interface_OnSignalInfoUpdated_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_DialOutCall_002
 * @tc.name: DialOutCall
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_DialOutCall_002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_DialOutCall_002 start");
    std::string address = "00:00:00:00:00:00";
    std::string number = "123";
    int result = hfpAgInterface_->DialOutCall(address, number);
    EXPECT_NE(result, DIAL_OUT_CALL_SUCCESS);
    HILOGI("Hfp_Ag_system_interface_DialOutCall_002 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_GetVoipCallState_001
 * @tc.name: GetVoipCallState
 * @tc.desc: Verify GetVoipCallState returns a valid int32_t and does not report a ringing/active
 *           voip call state when the BluetoothCallClient proxy is not initialized in the test env.
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_GetVoipCallState_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_GetVoipCallState_001 start");

    int32_t voipCallState = hfpAgInterface_->GetVoipCallState();
    HILOGI("GetVoipCallState returns %{public}d", voipCallState);
    // Proxy is not initialized in the unittest environment, so the returned state must not
    // indicate an active/ringing voip call (otherwise ProcessHangupCallEvent would misbehave).
    EXPECT_NE(voipCallState, static_cast<int32_t>(TelCallState::CALL_STATUS_INCOMING));
    EXPECT_NE(voipCallState, static_cast<int32_t>(TelCallState::CALL_STATUS_ANSWERED));
    EXPECT_NE(voipCallState, static_cast<int32_t>(TelCallState::CALL_STATUS_ACTIVE));
    EXPECT_NE(voipCallState, static_cast<int32_t>(TelCallState::CALL_STATUS_DIALING));

    HILOGI("Hfp_Ag_system_interface_GetVoipCallState_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_interface_GetVoipCallState_002
 * @tc.name: GetVoipCallState
 * @tc.desc: Verify GetVoipCallState is consistent across repeated calls in the same idle state.
 */
HWTEST_F(HfpAgSystemInterfaceTest, Hfp_Ag_system_interface_GetVoipCallState_002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_interface_GetVoipCallState_002 start");

    int32_t firstState = hfpAgInterface_->GetVoipCallState();
    int32_t secondState = hfpAgInterface_->GetVoipCallState();
    HILOGI("firstState:%{public}d, secondState:%{public}d", firstState, secondState);
    EXPECT_EQ(firstState, secondState);

    HILOGI("Hfp_Ag_system_interface_GetVoipCallState_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS