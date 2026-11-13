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
#include "profile_service_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_errorcode.h"
#include "remote_device_properties.h"
#include "mock/hfp_ag_interface_mock.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_state_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
using namespace bluetooth::headset;

namespace OHOS {
namespace bluetooth {

const int TIME = 2;
const int TIME_4 = 4;
std::string address("00:00:00:00:00:00");
HfpAgStateMachine stateMachine(address);

class HfpAgSystemEventProcesserTest : public HfpAgSystemEventProcesser {
public:
    HfpAgSystemEventProcesserTest() : HfpAgSystemEventProcesser(address) {}
    ~HfpAgSystemEventProcesserTest() = default;
};

static BluetoothHost *host_;
static std::unique_ptr<HfpAgSystemEventProcesser> hfpAgProcesser_ {nullptr};
static HfpAgSystemEventProcesserTest g_eventProcessor;

class HfpAgSystemEventProcessTest : public testing::Test {
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

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestSuite start");
        utility::CloseAllTimer();
        HILOGI("TearDownTestSuite end");
    }
    void SetUp()
    {
        hfpAgProcesser_ = std::make_unique<HfpAgSystemEventProcesser>(address);
        auto manager = ProfileServiceManager::GetInstance();
        string name = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, name);
    }
    void TearDown()
    {
        hfpAgProcesser_ = nullptr;
    }
};

/*
 * @tc.number: Hfp_Ag_system_event_GetCallState_001
 * @tc.name: GetCallState
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_GetCallState_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_GetCallState_001 start");

    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_ACTIVE), "HFP_AG_CALL_STATE_ACTIVE");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_HELD), "HFP_AG_CALL_STATE_HELD");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_DIALING), "HFP_AG_CALL_STATE_DIALING");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_ALERTING), "HFP_AG_CALL_STATE_ALERTING");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_INCOMING), "HFP_AG_CALL_STATE_INCOMING");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_WAITING), "HFP_AG_CALL_STATE_WAITING");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_IDLE), "HFP_AG_CALL_STATE_IDLE");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_DISCONNECTED), "HFP_AG_CALL_STATE_DISCONNECTED");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(HFP_AG_CALL_STATE_DISCONNECTING), "HFP_AG_CALL_STATE_DISCONNECTING");
    EXPECT_EQ(hfpAgProcesser_->GetCallState(99), "Unknown");

    HILOGI("Hfp_Ag_system_event_GetCallState_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_CovertCallStateToBluetdroid_001
 * @tc.name: CovertCallStateToBluetdroid
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_CovertCallStateToBluetdroid_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_CovertCallStateToBluetdroid_001 start");

    EXPECT_EQ(hfpAgProcesser_->CovertCallStateToBluetdroid(HFP_AG_CALL_STATE_DISCONNECTED),
        BTHF_CALL_STATE_DISCONNECTED);
    EXPECT_EQ(hfpAgProcesser_->CovertCallStateToBluetdroid(HFP_AG_CALL_STATE_DISCONNECTING),
        BTHF_CALL_STATE_DISCONNECTED);
        EXPECT_EQ(hfpAgProcesser_->CovertCallStateToBluetdroid(HFP_AG_CALL_STATE_IDLE),
        BTHF_CALL_STATE_IDLE);
    EXPECT_EQ(hfpAgProcesser_->CovertCallStateToBluetdroid(HFP_AG_CALL_STATE_WAITING),
        HFP_AG_CALL_STATE_WAITING);

    HILOGI("Hfp_Ag_system_event_CovertCallStateToBluetdroid_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_001
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_001 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_ANSWER_CALL;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_002
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_002 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_HANGUP_CALL;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_002 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_003
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_003 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    HfpAgService::GetService()->activeAddr_ = "00:00:00:00:00:01";
    message.type_ = HFP_AG_MSG_TYPE_VOLUME_CHANGED;
    message.arg1_ = HFP_AG_VOLUME_TYPE_SPK;
    message.arg3_ = 5;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HfpAgService::GetService()->activeAddr_ = address;
    hfpAgProcesser_->ExecuteEventProcess(message);

    message.arg1_ = HFP_AG_VOLUME_TYPE_MIC;
    hfpAgProcesser_->ExecuteEventProcess(message);

    message.arg1_ = static_cast<HfpAgVolumeType>(2);
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_003 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_004
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_004, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_004 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_CLCC;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->isVrOpened_ = true;
    }
    hfpAgProcesser_->ExecuteEventProcess(message);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, ClccResponse(_, _, _, _, _, _, _, _)).WillRepeatedly(Return(BT_STATUS_SUCCESS));
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_004 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_005
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_005, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_005 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_UNKNOWN;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_005 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_006
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_006, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_006 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_KEY_PRESSED;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_006 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_007
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_007, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_007 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_BIND;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_007 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_008
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_008, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_008 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_BIEV;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_008 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_009
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_009, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_009 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_BIA;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_009 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0010
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_010, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_010 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_QUERY_AG_INDICATOR;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_010 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0011
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0011, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0011 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_TYPE_NONE;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0011 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_ATD_01
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_ATD_01, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_ATD_01 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_DIAL_CALL;
    string number;
    message.str_ = number;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, AtResponse(_, _, _)).Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    hfpAgProcesser_->ExecuteEventProcess(message);
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_ATD_01 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_ATD_02
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_ATD_02, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_ATD_02 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_DIAL_CALL;
    message.str_ = "123456";
    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, AtResponse(_, _, _)).WillRepeatedly(Return(BT_STATUS_SUCCESS));
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_ATD_02 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_ATD_03
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_ATD_03, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_ATD_03 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_DIAL_CALL;
    message.str_ = ">1";
    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, AtResponse(_, _, _)).WillRepeatedly(Return(BT_STATUS_SUCCESS));
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_ATD_03 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_ATD_03
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_ATD_04, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_ATD_04 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_DIAL_CALL;
    message.str_ = ">9999";
    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, AtResponse(_, _, _)).WillRepeatedly(Return(BT_STATUS_SUCCESS));
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_ATD_04 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0014
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0014, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0014 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_SEND_DTMF;
    message.arg1_ = 1;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0014 end");
}

/*
 * @tc.number: ExecuteEventProcess_NOISE_REDUCTION_001
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_NOISE_REDUCTION_001, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_NOISE_REDUCTION_001 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_NOISE_REDUCTION;
    message.arg1_ = 0;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &connectedState;
    }
    hfpAgProcesser_->ExecuteEventProcess(message);

    hfpAgProcesser_->audioParameterMap["bt_headset_nrec"] = "on";
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("ExecuteEventProcess_NOISE_REDUCTION_001 end");
}

/*
 * @tc.number: ExecuteEventProcess_NOISE_REDUCTION_002
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_NOISE_REDUCTION_002, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_NOISE_REDUCTION_002 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_NOISE_REDUCTION;
    message.arg1_ = 0;

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        HfpAgAudioConnected audioConnectedState(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, connectedState);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &audioConnectedState;
    }
    hfpAgProcesser_->audioParameterMap["bt_headset_nrec"] = "on";
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("ExecuteEventProcess_NOISE_REDUCTION_002 end");
}

/*
 * @tc.number: ExecuteEventProcess_AT_WBS_001
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_AT_WBS_001, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_AT_WBS_001 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    message.arg1_ = 0;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &connectedState;
    }
    hfpAgProcesser_->ExecuteEventProcess(message);

    hfpAgProcesser_->audioParameterMap["bt_wbs"] = "on";
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("ExecuteEventProcess_AT_WBS_001 end");
}

/*
 * @tc.number: ExecuteEventProcess_AT_WBS_002
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_AT_WBS_002, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_AT_WBS_002 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    message.arg1_ = 0;

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        HfpAgAudioConnected audioConnectedState(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, connectedState);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &audioConnectedState;
    }

    hfpAgProcesser_->audioParameterMap["bt_wbs"] = "on";
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("ExecuteEventProcess_AT_WBS_002 end");
}

/*
 * @tc.number: ExecuteEventProcess_AT_WBS_003
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_AT_WBS_003, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_AT_WBS_003 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    message.arg1_ = 1;

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        HfpAgAudioConnected audioConnectedState(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, connectedState);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &audioConnectedState;
    }

    hfpAgProcesser_->audioParameterMap["bt_swb"] = "on";
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("ExecuteEventProcess_AT_WBS_003 end");
}

/*
 * @tc.number: ExecuteEventProcess_AT_WBS_004
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_AT_WBS_004, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_AT_WBS_004 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    message.arg1_ = 2;

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        HfpAgAudioConnected audioConnectedState(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, connectedState);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &audioConnectedState;
    }

    hfpAgProcesser_->audioParameterMap["bt_swb"] = "on";
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("ExecuteEventProcess_AT_WBS_004 end");
}

/*
 * @tc.number: ExecuteEventProcess_AT_WBS_005
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_AT_WBS_005, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_AT_WBS_005 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    message.arg1_ = 3;

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        HfpAgAudioConnected audioConnectedState(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, connectedState);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &audioConnectedState;
    }

    hfpAgProcesser_->audioParameterMap["bt_swb"] = "on";
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("ExecuteEventProcess_AT_WBS_005 end");
}

/*
 * @tc.number: ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOnWithConnectedSco_001
 * @tc.name: ExecuteEventProcess
 * @tc.desc: Test ProcessWideBandSpeechEvent with SWB on and SCO connected.
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_ProcessWideBandSpeechEvent_001, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOnWithConnectedSco_001 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    message.arg1_ = HFP_AG_SWB_YES;

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        HfpAgAudioConnected audioConnectedState(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, connectedState);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &audioConnectedState;
    }
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, HFP_AG_AUDIO_STATE_CONNECTED);

    hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_SWB_NAME] = HFP_AG_SERVER_AUDIO_FEATURE_ON;
    hfpAgProcesser_->ExecuteEventProcess(message);

    EXPECT_EQ(hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_SWB_NAME], HFP_AG_SERVER_AUDIO_FEATURE_ON);
    EXPECT_EQ(hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_WBS_NAME], HFP_AG_SERVER_AUDIO_FEATURE_OFF);

    HILOGI("ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOnWithConnectedSco_001 end");
}

/*
 * @tc.number: ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOffWithConnectedSco_001
 * @tc.name: ExecuteEventProcess
 * @tc.desc: Test ProcessWideBandSpeechEvent with SWB off scenario, prevSwb is on and SCO is connected.
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_ProcessWideBandSpeechEvent_002, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOffWithConnectedSco_001 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    message.arg1_ = HFP_AG_WBS_YES;

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        HfpAgAudioConnected audioConnectedState(HfpAgStateMachine::AUDIO_CONNECTED,
            stateMachine, g_eventProcessor, connectedState);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &audioConnectedState;
    }
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, HFP_AG_AUDIO_STATE_CONNECTED);

    hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_SWB_NAME] = HFP_AG_SERVER_AUDIO_FEATURE_ON;
    hfpAgProcesser_->ExecuteEventProcess(message);

    EXPECT_EQ(hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_SWB_NAME], HFP_AG_SERVER_AUDIO_FEATURE_OFF);
    EXPECT_EQ(hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_WBS_NAME], HFP_AG_SERVER_AUDIO_FEATURE_ON);

    HILOGI("ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOffWithConnectedSco_001 end");
}

/*
 * @tc.number: ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOnWithDisconnectedSco_001
 * @tc.name: ExecuteEventProcess
 * @tc.desc: Test ProcessWideBandSpeechEvent with SWB on but SCO disconnected.
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_ProcessWideBandSpeechEvent_003, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOnWithDisconnectedSco_001 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    message.arg1_ = HFP_AG_SWB_YES;

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        HfpAgConnected connectedState(HfpAgStateMachine::CONNECTED, stateMachine, g_eventProcessor);
        service->stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
        service->stateMachines_[address]->current_ = &connectedState;
    }
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address, HFP_AG_AUDIO_STATE_DISCONNECTED);

    hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_SWB_NAME] = HFP_AG_SERVER_AUDIO_FEATURE_ON;
    hfpAgProcesser_->ExecuteEventProcess(message);

    EXPECT_EQ(hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_SWB_NAME], HFP_AG_SERVER_AUDIO_FEATURE_ON);
    EXPECT_EQ(hfpAgProcesser_->audioParameterMap[HFP_AG_SERVER_WBS_NAME], HFP_AG_SERVER_AUDIO_FEATURE_OFF);

    HILOGI("ExecuteEventProcess_ProcessWideBandSpeechEvent_SwbOnWithDisconnectedSco_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0017
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0017, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0017 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_CHLD;
    message.arg1_ = 0;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0017 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0018
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0018, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0018 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_SUBSCRIBER_NUMBER_REQUEST;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0018 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0019
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0019, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0019 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_VR_CHANGED;
    message.arg1_ = static_cast<int>(HFP_AG_HF_VR_OPENED);
    hfpAgProcesser_->ExecuteEventProcess(message);

    message.arg1_ = static_cast<int>(HFP_AG_HF_VR_ClOSED);
    hfpAgProcesser_->ExecuteEventProcess(message);

    message.arg1_ = 3;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0019 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0020
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0020, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0020 start");

    HfpAgMessage message(HFP_AG_CONNECT_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_CIND;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, CindResponse(_, _, _, _, _, _, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    hfpAgProcesser_->ExecuteEventProcess(message);
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0020 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0021
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0021, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0021 start");

    HfpAgMessage message(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_BRSF;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0021 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0022
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0022, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0022 start");

    HfpAgMessage message(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_COPS;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0022 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0023
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0023, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0023 start");

    HfpAgMessage message(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_BCC;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0023 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ExecuteEventProcess_0024
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ExecuteEventProcess_0024, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0024 start");

    HfpAgMessage message(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    message.type_ = HFP_AG_MSG_TYPE_AT_VENDOR_SPECIFIC;
    hfpAgProcesser_->ExecuteEventProcess(message);

    HILOGI("Hfp_Ag_system_event_ExecuteEventProcess_0024 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessPhoneATCommand_0001
 * @tc.name: ExecuteEventProcess
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessPhoneATCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessPhoneATCommand_0001 start");

    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+CGMI"));
    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+CGMM"));
    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+CGMR"));
    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+CGSN"));
    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+CIMI"));
    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+GMI"));
    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+GMI"));
    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+GMM"));
    EXPECT_TRUE(hfpAgProcesser_->ProcessPhoneATCommand("+GMR"));
    EXPECT_FALSE(hfpAgProcesser_->ProcessPhoneATCommand("+TEST"));

    HILOGI("Hfp_Ag_system_event_ProcessPhoneATCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_RecoverScoVolume_0001
 * @tc.name: RecoverScoVolume
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_RecoverScoVolume_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_RecoverScoVolume_0001 start");
    hfpAgProcesser_->SetScoVolume();
    hfpAgProcesser_->ProcessHfVolumeChangedEvent(0, 1);
    hfpAgProcesser_->RemoveTask();
    HILOGI("Hfp_Ag_system_event_RecoverScoVolume_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessScoVolumeChangedEvent_0001
 * @tc.name: ProcessScoVolumeChangedEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessScoVolumeChangedEvent_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessScoVolumeChangedEvent_0001 start");
    hfpAgProcesser_->speakerVolume_ = 5;
    hfpAgProcesser_->ProcessScoVolumeChangedEvent(HFP_AG_VOLUME_TYPE_SPK, 5);
    hfpAgProcesser_->ProcessScoVolumeChangedEvent(HFP_AG_VOLUME_TYPE_SPK, 6);

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, VolumeControl(_, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    hfpAgProcesser_->ProcessScoVolumeChangedEvent(HFP_AG_VOLUME_TYPE_SPK, 7);
    hfpAgProcesser_->ProcessScoVolumeChangedEvent(HFP_AG_VOLUME_TYPE_SPK, 8);
    HILOGI("Hfp_Ag_system_event_ProcessScoVolumeChangedEvent_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHangupCallEvent_0001
 * @tc.name: ProcessHangupCallEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHangupCallEvent_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0001 start");
    HfpAgService::GetService()->isVirtualCallStarted_ = true;
    hfpAgProcesser_->ProcessHangupCallEvent();
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHangupCallEvent_0002
 * @tc.name: ProcessHangupCallEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHangupCallEvent_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0002 start");
    HfpAgService::GetService()->isVirtualCallStarted_ = false;
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_INCOMING);
    hfpAgProcesser_->ProcessHangupCallEvent();
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_ACTIVE);
    hfpAgProcesser_->ProcessHangupCallEvent();

    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0002 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHangupCallEvent_0003
 * @tc.name: ProcessHangupCallEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHangupCallEvent_0003, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0003 start");
    HfpAgService::GetService()->isVirtualCallStarted_ = false;
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_INCOMING);
    std::string address = "00:00:00:00:00:00";
    RawAddress device(address);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        address, static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTING));
    hfpAgProcesser_->ProcessHangupCallEvent();
    EXPECT_EQ(HfpAgService::GetService()->GetScoState(device), HFP_AG_AUDIO_STATE_CONNECTING);
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0003 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHangupCallEvent_0004
 * @tc.name: ProcessHangupCallEvent
 * @tc.desc: When virtual call is started and no voip call exists (proxy not initialized), the
 *           hangup event should fall through to NotifyStopVirtualCall instead of invoking
 *           AnswerCall/HangupCall. Verify virtual call flag is still set after handling.
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHangupCallEvent_0004, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0004 start");
    HfpAgService::GetService()->isVirtualCallStarted_ = true;
    int32_t voipCallState = HfpAgSystemInterface::GetInstance().GetVoipCallState();
    HILOGI("voipCallState under test env: %{public}d", voipCallState);
    // In the unittest env the BluetoothCallClient proxy is not initialized, so the voip state
    // must not match the ringing/active states that drive the AnswerCall/HangupCall branches.
    ASSERT_NE(voipCallState, static_cast<int32_t>(OHOS::Telephony::TelCallState::CALL_STATUS_INCOMING));
    ASSERT_NE(voipCallState, static_cast<int32_t>(OHOS::Telephony::TelCallState::CALL_STATUS_ANSWERED));
    ASSERT_NE(voipCallState, static_cast<int32_t>(OHOS::Telephony::TelCallState::CALL_STATUS_ACTIVE));
    ASSERT_NE(voipCallState, static_cast<int32_t>(OHOS::Telephony::TelCallState::CALL_STATUS_DIALING));
    hfpAgProcesser_->ProcessHangupCallEvent();
    EXPECT_TRUE(HfpAgService::GetService()->IsVirtualCallStarted());
    HfpAgService::GetService()->isVirtualCallStarted_ = false;
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0004 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHangupCallEvent_0005
 * @tc.name: ProcessHangupCallEvent
 * @tc.desc: When virtual call is started and GetVoipCallState returns CALL_STATUS_INCOMING, the
 *           hangup event should answer the voip call and keep virtual call started.
 *           Since the proxy cannot be driven to return INCOMING in the unittest env, this case
 *           verifies the ringing-state query path is exercised and the call still succeeds.
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHangupCallEvent_0005, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0005 start");
    HfpAgService::GetService()->isVirtualCallStarted_ = true;
    int32_t voipCallState = HfpAgSystemInterface::GetInstance().GetVoipCallState();
    // Confirm the precondition: voip is NOT ringing in the test env, so the AnswerCall branch
    // is not taken and we instead verify the ringing branch decision logic.
    EXPECT_NE(voipCallState, static_cast<int32_t>(OHOS::Telephony::TelCallState::CALL_STATUS_INCOMING));
    hfpAgProcesser_->ProcessHangupCallEvent();
    EXPECT_TRUE(HfpAgService::GetService()->IsVirtualCallStarted());
    HfpAgService::GetService()->isVirtualCallStarted_ = false;
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0005 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHangupCallEvent_0006
 * @tc.name: ProcessHangupCallEvent
 * @tc.desc: When virtual call is started and GetVoipCallState reports an active/answered/dialing
 *           voip call, the hangup event should hang up the voip call. In the unittest env the
 *           proxy is uninitialized so this case asserts the active-state branch is not entered
 *           and the function still returns without crashing.
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHangupCallEvent_0006, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0006 start");
    HfpAgService::GetService()->isVirtualCallStarted_ = true;
    int32_t voipCallState = HfpAgSystemInterface::GetInstance().GetVoipCallState();
    bool isActiveVoip = (voipCallState == static_cast<int32_t>(OHOS::Telephony::TelCallState::CALL_STATUS_ANSWERED)) ||
        (voipCallState == static_cast<int32_t>(OHOS::Telephony::TelCallState::CALL_STATUS_ACTIVE)) ||
        (voipCallState == static_cast<int32_t>(OHOS::Telephony::TelCallState::CALL_STATUS_DIALING));
    EXPECT_FALSE(isActiveVoip);
    hfpAgProcesser_->ProcessHangupCallEvent();
    EXPECT_TRUE(HfpAgService::GetService()->IsVirtualCallStarted());
    HfpAgService::GetService()->isVirtualCallStarted_ = false;
    HILOGI("Hfp_Ag_system_event_ProcessHangupCallEvent_0006 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessTalkBandSetFeatureCommand_0001
 * @tc.name: ProcessTalkBandSetFeatureCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessTalkBandSetFeatureCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessTalkBandSetFeatureCommand_0001 start");
    EXPECT_FALSE(hfpAgProcesser_->ProcessTalkBandSetFeatureCommand(""));
    EXPECT_TRUE(hfpAgProcesser_->ProcessTalkBandSetFeatureCommand("?"));
    HILOGI("Hfp_Ag_system_event_ProcessTalkBandSetFeatureCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessTalkBandSetRequestCommand_0001
 * @tc.name: ProcessTalkBandSetRequestCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessTalkBandSetRequestCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessTalkBandSetRequestCommand_0001 start");
    EXPECT_FALSE(hfpAgProcesser_->ProcessTalkBandSetRequestCommand("", ""));
    EXPECT_TRUE(hfpAgProcesser_->ProcessTalkBandSetRequestCommand("1,0", ""));
    EXPECT_TRUE(hfpAgProcesser_->ProcessTalkBandSetRequestCommand("1,1", ""));
    EXPECT_TRUE(hfpAgProcesser_->ProcessTalkBandSetRequestCommand("5,0", ""));
    EXPECT_TRUE(hfpAgProcesser_->ProcessTalkBandSetRequestCommand("6,0", ""));
    HILOGI("Hfp_Ag_system_event_ProcessTalkBandSetRequestCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHwFeatureRequestCommand_0001
 * @tc.name: ProcessHwFeatureRequestCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHwFeatureRequestCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHwFeatureRequestCommand_0001 start");
    EXPECT_FALSE(hfpAgProcesser_->ProcessHwFeatureRequestCommand(""));
    EXPECT_TRUE(hfpAgProcesser_->ProcessHwFeatureRequestCommand("?"));
    HILOGI("Hfp_Ag_system_event_ProcessHwFeatureRequestCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHwSetFeatureCommand_0001
 * @tc.name: ProcessHwSetFeatureCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHwSetFeatureCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHwSetFeatureCommand_0001 start");
    EXPECT_FALSE(hfpAgProcesser_->ProcessHwSetFeatureCommand(""));
    HILOGI("Hfp_Ag_system_event_ProcessHwSetFeatureCommand_0001 end");
}

/* * @tc.number: Hfp_Ag_system_event_IsSupportedAtCommand_0001
 * @tc.name: IsSupportedAtCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_IsSupportedAtCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_IsSupportedAtCommand_0001 start");
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+XEVENT"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+XAPL"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+IPHONEACCEV"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+TBSF"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+HWCMD"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+VENDORBATTERY"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+UPDATEVENDORBATTERY"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+CLOSEVENDORBATTERY"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+TBIF"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+XVENDORSF"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+XSVENDORF"), true);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+AAA"), false);
    EXPECT_EQ(hfpAgProcesser_->IsSupportedAtCommand("+BRSF"), false);
    HILOGI("Hfp_Ag_system_event_IsSupportedAtCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessBatteryAtCommand_0001
 * @tc.name: ProcessBatteryAtCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessBatteryAtCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessBatteryAtCommand_0001 start");
    EXPECT_EQ(hfpAgProcesser_->ProcessBatteryAtCommand("+XEVENT=BATTERY,9,10,1,1"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessBatteryAtCommand("+IPHONEACCEV=1,1,8"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessBatteryAtCommand("+IPHONEACCEV=1,1,11"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessBatteryAtCommand("+XAPL=0000-0000-0100,3"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessBatteryAtCommand("+AAA"), false);
    HILOGI("Hfp_Ag_system_event_ProcessBatteryAtCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessXaplAtCommand_0001
 * @tc.name: ProcessBatteryAtCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessXaplAtCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessXaplAtCommand_0001 start");
    EXPECT_EQ(hfpAgProcesser_->ProcessXaplAtCommand("+XAPL=0000-0000-0100,3"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessXaplAtCommand("+XAPL="), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessXaplAtCommand("+XAPL=0000-0000-0100"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessXaplAtCommand("+XAPL=0000-0000,3"), true);
    HILOGI("Hfp_Ag_system_event_ProcessXaplAtCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHwBatteryAtCommand_0001
 * @tc.name: ProcessHwBatteryAtCommand
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHwBatteryAtCommand_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHwBatteryAtCommand_0001 start");
    EXPECT_EQ(hfpAgProcesser_->ProcessHwBatteryAtCommand("+VENDORBATTERY=?"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessHwBatteryAtCommand("+VENDORBATTERY=1,1,90"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessHwBatteryAtCommand("+UPDATEVENDORBATTERY=1,1,90"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessHwBatteryAtCommand("+CLOSEVENDORBATTERY=2"), true);
    EXPECT_EQ(hfpAgProcesser_->ProcessHwBatteryAtCommand("+AAA=2"), false);
    HILOGI("Hfp_Ag_system_event_ProcessHwBatteryAtCommand_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessAtUnknownEvent_0001
 * @tc.name: ProcessAtUnknownEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessAtUnknownEvent_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessAtUnknownEvent_0001 start");
    hfpAgProcesser_->ProcessAtUnknownEvent("", "00:11:22:33:44:55");
    hfpAgProcesser_->ProcessAtUnknownEvent("+CSCS=UTF-8", "00:11:22:33:44:55");
    hfpAgProcesser_->ProcessAtUnknownEvent("+CGMM", "00:11:22:33:44:55");
    hfpAgProcesser_->ProcessAtUnknownEvent("+IPHONEACCEV=1,1,8", "00:11:22:33:44:55");
    hfpAgProcesser_->ProcessAtUnknownEvent("+VENDORBATTERY=1,1,90", "00:11:22:33:44:55");
    hfpAgProcesser_->ProcessAtUnknownEvent("+AAAA", "00:11:22:33:44:55");
    HILOGI("Hfp_Ag_system_event_ProcessAtUnknownEvent_0001 end");
}

/*
 * @tc.number: ExecuteEventProcess_SetAudioParameters_001
 * @tc.name: SetAudioParameters
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_SetAudioParameters_001, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_SetAudioParameters_001 start");
    hfpAgProcesser_->audioParameterMap["bt_headset_nrec"] = "on";
    hfpAgProcesser_->audioParameterMap["bt_swb"] = "on";
    hfpAgProcesser_->audioParameterMap["bt_wbs"] = "on";
    hfpAgProcesser_->audioParameterMap["bt_swb"] = "off";

    hfpAgProcesser_->SetAudioParameters("");
    hfpAgProcesser_->SetAudioParameters("bt_headset_nrec");
    hfpAgProcesser_->SetAudioParameters("bt_wbs");
    hfpAgProcesser_->SetAudioParameters("bt_swb");

    hfpAgProcesser_->audioParameterMap["bt_swb"] = "on";
    hfpAgProcesser_->SetAudioParameters("");
    hfpAgProcesser_->SetAudioParameters("bt_headset_nrec");
    hfpAgProcesser_->SetAudioParameters("bt_wbs");
    hfpAgProcesser_->SetAudioParameters("bt_swb");

    HILOGI("ExecuteEventProcess_SetAudioParameters_001 end");
}

/*
 * @tc.number: ExecuteEventProcess_SetAudioExtraParametersForSwbOn_001
 * @tc.name: SetAudioExtraParametersForSwbOn
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, ExecuteEventProcess_SetAudioExtraParametersForSwbOn_001, TestSize.Level1)
{
    HILOGI("ExecuteEventProcess_SetAudioExtraParametersForSwbOn_001 start");
    hfpAgProcesser_->audioParameterMap["bt_headset_nrec"] = "on";
    hfpAgProcesser_->audioParameterMap["bt_swb"] = "on";
    hfpAgProcesser_->audioParameterMap["bt_wbs"] = "on";
    hfpAgProcesser_->audioParameterMap["bt_swb"] = "off";

    hfpAgProcesser_->SetAudioExtraParametersForSwbOn();
    hfpAgProcesser_->audioParameterMap["bt_swb"] = "on";
    hfpAgProcesser_->SetAudioExtraParametersForSwbOn();

    HILOGI("ExecuteEventProcess_SetAudioExtraParametersForSwbOn_001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHwDeviceInfo_0001
 * @tc.name: ProcessHwDeviceInfo
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHwDeviceInfo_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHwDeviceInfo_0001 start");
    std::string cmdStr =
        "4,0040068116Sound Joy208000000003040001404027d50400006040000704X0A38060000009080014042ca0200b041903";
    EXPECT_TRUE(hfpAgProcesser_->ProcessHwDeviceInfo(cmdStr));

    cmdStr = "4,00A";
    EXPECT_FALSE(hfpAgProcesser_->ProcessHwDeviceInfo(cmdStr));

    cmdStr = "4,009";
    EXPECT_FALSE(hfpAgProcesser_->ProcessHwDeviceInfo(cmdStr));

    cmdStr = "4,0040068c041903";
    EXPECT_TRUE(hfpAgProcesser_->ProcessHwDeviceInfo(cmdStr));

    cmdStr = "4,004006c120AI Speaker 2e208000000003040001404027d50400006040000704X0A2806000000";
    cmdStr.append("9080014042ca0200b041902");
    hfpAgProcesser_->deviceTimestamp_ = "";
    EXPECT_TRUE(hfpAgProcesser_->ProcessHwDeviceInfo(cmdStr));
    EXPECT_TRUE(hfpAgProcesser_->deviceTimestamp_.empty());
    HILOGI("Hfp_Ag_system_event_ProcessHwDeviceInfo_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessHwDeviceInfo_0002
 * @tc.name: ProcessHwDeviceInfo
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessHwDeviceInfo_0002, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessHwDeviceInfo_0002 start");
    hfpAgProcesser_->ProcessHwDeviceInfo("1");
    HILOGI("Hfp_Ag_system_event_ProcessHwDeviceInfo_0002 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_GetOtherEventType_0001
 * @tc.name: GetOtherEventType
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_GetOtherEventType_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_GetOtherEventType_0001 start");
    EXPECT_EQ(hfpAgProcesser_->GetOtherEventType(HFP_AG_MSG_TYPE_AT_BCC), "HFP_AG_MSG_TYPE_AT_BCC");
    EXPECT_EQ(hfpAgProcesser_->GetOtherEventType(HFP_AG_MSG_TYPE_AT_COPS), "HFP_AG_MSG_TYPE_AT_COPS");
    EXPECT_EQ(hfpAgProcesser_->GetOtherEventType(HFP_AG_MSG_TYPE_AT_VENDOR_SPECIFIC),
        "HFP_AG_MSG_TYPE_AT_VENDOR_SPECIFIC");
    EXPECT_EQ(hfpAgProcesser_->GetOtherEventType(99), "Unknown");
    HILOGI("Hfp_Ag_system_event_GetOtherEventType_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessBccEvent_0001
 * @tc.name: ProcessBccEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessBccEvent_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_0001 start");
    HfpAgService::GetService()->activeAddr_ = address;
    hfpAgProcesser_->ProcessBccEvent();

    HfpAgService::GetService()->activeAddr_ = "00:00:00:00:00:01";
    hfpAgProcesser_->ProcessBccEvent();

    HfpAgService::GetService()->activeAddr_ = "00:00:00:00:00";
    hfpAgProcesser_->ProcessBccEvent();
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessBccEvent_BlockWithin3_5s_00100
 * @tc.name: ProcessBccEvent
 * @tc.desc: Test ProcessBccEvent block when BCC block timer is active and output device is NearLink
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessBccEvent_BlockWithin3_5s_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_BlockWithin3_5s_00100 start");
    auto service = HfpAgService::GetService();
    ASSERT_NE(service, nullptr);
    service->isBccBlockTimerActive_ = true;
    hfpAgProcesser_->ProcessBccEvent();
    EXPECT_TRUE(service->IsBccBlockTimerActive());
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_BlockWithin3_5s_00100 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessBccEvent_BlockWithin3_5s_00101
 * @tc.name: ProcessBccEvent
 * @tc.desc: Test ProcessBccEvent not blocked when BCC block timer not active
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessBccEvent_BlockWithin3_5s_00101, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_BlockWithin3_5s_00101 start");
    auto service = HfpAgService::GetService();
    ASSERT_NE(service, nullptr);
    service->isBccBlockTimerActive_ = false;
    hfpAgProcesser_->ProcessBccEvent();
    EXPECT_FALSE(service->IsBccBlockTimerActive());
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_BlockWithin3_5s_00101 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessBccEvent_BccBlockTimer_00102
 * @tc.name: ProcessBccEvent
 * @tc.desc: Test BCC block timer lifecycle - start and timeout
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessBccEvent_BccBlockTimer_00102, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_BccBlockTimer_00102 start");
    auto service = HfpAgService::GetService();
    ASSERT_NE(service, nullptr);
    service->isBccBlockTimerActive_ = false;
    EXPECT_FALSE(service->IsBccBlockTimerActive());
    service->isBccBlockTimerActive_ = true;
    EXPECT_TRUE(service->IsBccBlockTimerActive());
    service->BccBlockTimerTimeout();
    EXPECT_FALSE(service->IsBccBlockTimerActive());
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_BccBlockTimer_00102 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessBccEvent_BccBlockTimer_00103
 * @tc.name: ProcessBccEvent
 * @tc.desc: Test IsBccBlockTimerActive returns correct value
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessBccEvent_BccBlockTimer_00103, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_BccBlockTimer_00103 start");
    auto service = HfpAgService::GetService();
    ASSERT_NE(service, nullptr);
    service->isBccBlockTimerActive_ = false;
    EXPECT_FALSE(service->IsBccBlockTimerActive());
    service->isBccBlockTimerActive_ = true;
    EXPECT_TRUE(service->IsBccBlockTimerActive());
    HILOGI("Hfp_Ag_system_event_ProcessBccEvent_BccBlockTimer_00103 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_WaitScoState_0001
 * @tc.name: WaitScoState
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_WaitScoState_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_WaitScoState_0001 start");
    hfpAgProcesser_->WaitScoState();
    ASSERT_NE(hfpAgProcesser_->waitScoStateTimeOut_, nullptr);
    HILOGI("Hfp_Ag_system_event_WaitScoState_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_WaitScoStateTimeOut_0001
 * @tc.name: WaitScoStateTimeOut
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_WaitScoStateTimeOut_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_WaitScoStateTimeOut_0001 start");
    HfpAgService::GetService()->activeAddr_ = "00:00:00:00:00:01";
    hfpAgProcesser_->WaitScoStateTimeOut();
    HILOGI("Hfp_Ag_system_event_WaitScoStateTimeOut_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessGetEventType_0001
 * @tc.name: GetEventType
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessGetEventType_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessGetEventType_0001 start");
    EXPECT_EQ("HFP_AG_MSG_TYPE_TYPE_NONE", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_TYPE_NONE));
    EXPECT_EQ("HFP_AG_MSG_TYPE_ANSWER_CALL", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_ANSWER_CALL));
    EXPECT_EQ("HFP_AG_MSG_TYPE_HANGUP_CALL", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_HANGUP_CALL));
    EXPECT_EQ("HFP_AG_MSG_TYPE_VOLUME_CHANGED", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_VOLUME_CHANGED));
    EXPECT_EQ("HFP_AG_MSG_TYPE_DIAL_CALL", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_DIAL_CALL));
    EXPECT_EQ("HFP_AG_MSG_TYPE_SEND_DTMF", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_SEND_DTMF));
    EXPECT_EQ("HFP_AG_MSG_TYPE_NOISE_REDUCTION", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_NOISE_REDUCTION));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_WBS", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_WBS));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_CHLD", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_CHLD));
    EXPECT_EQ("HFP_AG_MSG_TYPE_SUBSCRIBER_NUMBER_REQUEST",
        hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_SUBSCRIBER_NUMBER_REQUEST));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_CIND", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_CIND));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_COPS", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_COPS));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_CLCC", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_CLCC));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_UNKNOWN", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_UNKNOWN));
    EXPECT_EQ("HFP_AG_MSG_TYPE_KEY_PRESSED", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_KEY_PRESSED));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_BIND", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_BIND));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_BIEV", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_BIEV));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_BIA", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_BIA));
    EXPECT_EQ("HFP_AG_MSG_TYPE_QUERY_AG_INDICATOR", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_QUERY_AG_INDICATOR));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_BCC", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_BCC));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_BRSF", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_BRSF));
    EXPECT_EQ("HFP_AG_MSG_TYPE_VR_CHANGED", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_VR_CHANGED));
    EXPECT_EQ("HFP_AG_MSG_TYPE_AT_VENDOR_SPECIFIC", hfpAgProcesser_->GetEventType(HFP_AG_MSG_TYPE_AT_VENDOR_SPECIFIC));
    EXPECT_EQ("Unknown", hfpAgProcesser_->GetEventType(99));
    HILOGI("Hfp_Ag_system_event_ProcessGetEventType_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_DoHwTalkbandHandSetMode_0001
 * @tc.name: DoHwTalkbandHandSetMode
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_DoHwTalkbandHandSetMode_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_DoHwTalkbandHandSetMode_0001 start");
    hfpAgProcesser_->DoHwTalkbandHandSetMode();
    hfpAgProcesser_->DoHwTalkbandBandMode();
    sleep(TIME_4);
}

/*
 * @tc.number: Hfp_Ag_system_event_UpdateHwDeviceInfo_0001
 * @tc.name: UpdateHwDeviceInfo
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_UpdateHwDeviceInfo_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_UpdateHwDeviceInfo_0001 start");
    hfpAgProcesser_->UpdateHwDeviceInfo("", "");
    hfpAgProcesser_->UpdateHwDeviceInfo("123", "00");
    HILOGI("Hfp_Ag_system_event_UpdateHwDeviceInfo_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_UpdateDeviceNameAndTimestamp_0001
 * @tc.name: UpdateDeviceNameAndTimestamp
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_UpdateDeviceNameAndTimestamp_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_UpdateDeviceNameAndTimestamp_0001 start");
    hfpAgProcesser_->deviceTimestamp_ = "";
    hfpAgProcesser_->UpdateDeviceNameAndTimestamp();

    hfpAgProcesser_->deviceTimestamp_ = "123";
    RemoteDeviceProperties::GetInstance()->UpdateRemoteDeviceName(RawAddress(address), "");
    hfpAgProcesser_->UpdateDeviceNameAndTimestamp();

    RemoteDeviceProperties::GetInstance()->UpdateRemoteDeviceName(RawAddress(address), "AI Speaker 2e");
    hfpAgProcesser_->UpdateDeviceNameAndTimestamp();
    HILOGI("Hfp_Ag_system_event_UpdateDeviceNameAndTimestamp_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_ProcessAtResponseStringEvent_0001
 * @tc.name: ProcessAtResponseStringEvent
 * @tc.desc:
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_ProcessAtResponseStringEvent_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_ProcessAtResponseStringEvent_0001 start");

    HfpAgInterfaceMock interface;
    HfpAgService::GetService()->bluetoothHfpInterface = &interface;
    EXPECT_CALL(interface, FormattedAtResponse(_, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    hfpAgProcesser_->ProcessAtResponseStringEvent("+XSVENDORF=6,0");
    hfpAgProcesser_->ProcessAtResponseStringEvent("+XSVENDORF=6,1");

    hfpAgProcesser_->ProcessSendDtmfEvent('A' + 0);
    hfpAgProcesser_->ProcessSendDtmfEvent('D' + 0);
    hfpAgProcesser_->ProcessSendDtmfEvent('Z' + 0);
    hfpAgProcesser_->ProcessSendDtmfEvent('a' + 0);
    hfpAgProcesser_->ProcessSendDtmfEvent(0);
    hfpAgProcesser_->ProcessSendDtmfEvent(9);
    hfpAgProcesser_->ProcessSendDtmfEvent(10);

    HILOGI("Hfp_Ag_system_event_ProcessAtResponseStringEvent_0001 end");
}

/*
 * @tc.number: Hfp_Ag_system_event_SendAtCmdOnApptypeChange_0001
 * @tc.name: SendAtCmdOnApptypeChange
 * @tc.desc: Test SendAtCmdOnApptypeChange function with different app categories.
 */
HWTEST_F(HfpAgSystemEventProcessTest, Hfp_Ag_system_event_SendAtCmdOnApptypeChange_0001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_system_event_SendAtCmdOnApptypeChange_0001 start");
    hfpAgProcesser_->SendAtCmdOnApptypeChange(APP_CATEGORY_MUSIC);
    hfpAgProcesser_->SendAtCmdOnApptypeChange(APP_CATEGORY_VIDEO);
    hfpAgProcesser_->SendAtCmdOnApptypeChange(APP_CATEGORY_GAME);
    HILOGI("Hfp_Ag_system_event_SendAtCmdOnApptypeChange_0001 end");
}
}  // namespace bluetooth
}  // namespace OHOS
