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
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "call_statemachine.h"
#include "profile_service_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

std::string address("11:22:33:44:55:66");
CallStateMachine callStateMachine(address);
static BluetoothHost *host_;
const int TIME = 4;

class CallStatemachineTest : public testing::Test {
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
        auto manager = ProfileServiceManager::GetInstance();
        string name = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, name);
    }
    static void TearDownTestCase()
    {
        sleep(TIME);
    }
    void SetUp()
    {
        dialingState = std::make_unique<DialingState>(callStateMachine);
        alertingState = std::make_unique<AlertingState>(callStateMachine);
        activeState = std::make_unique<ActiveState>(callStateMachine);
        endState = std::make_unique<EndState>(callStateMachine);

        HfpAgPhoneState state{0, 0, HFP_AG_CALL_STATE_IDLE, "110112", 0, ""};
        callStateMachine.SetPhoneState(state);
    }
    void TearDown()
    {
        dialingState = nullptr;
        alertingState = nullptr;
        activeState = nullptr;
        endState = nullptr;
    }
public:
    std::unique_ptr<DialingState> dialingState;
    std::unique_ptr<AlertingState> alertingState;
    std::unique_ptr<ActiveState> activeState;
    std::unique_ptr<EndState> endState;
};

/*
 * @tc.number: Call_Statemachine_EventToString_0001
 * @tc.name: GetEventName
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_EventToString_0001, TestSize.Level1)
{
    HILOGI("Call_Statemachine_EventToString_0001 start");

    EXPECT_EQ(CallStateMachine::GetEventName(HFP_AG_CALL_STATE_CHANGE), "HFP_AG_CALL_STATE_CHANGE");
    EXPECT_EQ(CallStateMachine::GetEventName(CallStateMachine::CALL_ALERT_EVENT), "CALL_ALERT_EVENT");
    EXPECT_EQ(CallStateMachine::GetEventName(CallStateMachine::CALL_ACTIVE_EVENT), "CALL_ACTIVE_EVENT");
    EXPECT_EQ(CallStateMachine::GetEventName(CallStateMachine::CALL_END_EVENT), "CALL_END_EVENT");
    EXPECT_EQ(CallStateMachine::GetEventName(7), "Unknown");

    HILOGI("Call_Statemachine_EventToString_0001 end");
}

/*
 * @tc.number: Call_Statemachine_DialingState_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_DialingState_Entry_0001, TestSize.Level1)
{
    HILOGI("Call_Statemachine_DialingState_Entry_0001 start");
    dialingState->Exit();
    dialingState->Entry();
    EXPECT_EQ(dialingState->GetStateInt(), HFP_AG_CALL_STATE_DIALING);

    HILOGI("Call_Statemachine_DialingState_Entry_0001 end");
}

/*
 * @tc.number: Call_Statemachine_DialingState_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_DialingState_Dispatch_0001, TestSize.Level1)
{
    callStateMachine.InitState(DIALING_STATE);

    HILOGI("Call_Statemachine_DialingState_Dispatch_0001 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {0, 0, HFP_AG_CALL_STATE_ALERTING, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(dialingState->GetStateInt(), HFP_AG_CALL_STATE_DIALING);
    HILOGI("Call_Statemachine_DialingState_Dispatch_0001 end");
}

/*
 * @tc.number: Call_Statemachine_DialingState_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_DialingState_Dispatch_0002, TestSize.Level1)
{
    callStateMachine.InitState(DIALING_STATE);

    HILOGI("Call_Statemachine_DialingState_Dispatch_0002 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {1, 0, HFP_AG_CALL_STATE_IDLE, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(dialingState->GetStateInt(), HFP_AG_CALL_STATE_DIALING);
    HILOGI("Call_Statemachine_DialingState_Dispatch_0002 end");
}

/*
 * @tc.number: Call_Statemachine_DialingState_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_DialingState_Dispatch_0003, TestSize.Level1)
{
    callStateMachine.InitState(DIALING_STATE);

    HILOGI("Call_Statemachine_DialingState_Dispatch_0003 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {0, 0, HFP_AG_CALL_STATE_IDLE, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(callStateMachine.GetCallState(), HFP_AG_CALL_STATE_IDLE);
    HILOGI("Call_Statemachine_DialingState_Dispatch_0003 end");
}

/*
 * @tc.number: Call_Statemachine_DialingState_Dispatch_0004
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_DialingState_Dispatch_0004, TestSize.Level1)
{
    callStateMachine.InitState(DIALING_STATE);

    HILOGI("Call_Statemachine_DialingState_Dispatch_0004 start");
    callStateMachine.TransitionToTargetState(CallStateMachine::CALL_ALERT_EVENT);

    EXPECT_EQ(alertingState->GetStateInt(), HFP_AG_CALL_STATE_ALERTING);
    HILOGI("Call_Statemachine_DialingState_Dispatch_0004 end");
}

/*
 * @tc.number: Call_Statemachine_AlertingState_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_AlertingState_Entry_0001, TestSize.Level1)
{
    HILOGI("Call_Statemachine_AlertingState_Entry_0001 start");
    alertingState->Exit();
    alertingState->Entry();
    EXPECT_EQ(alertingState->GetStateInt(), HFP_AG_CALL_STATE_ALERTING);

    HILOGI("Call_Statemachine_AlertingState_Entry_0001 end");
}

/*
 * @tc.number: Call_Statemachine_Alertingtate_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_Alertingtate_Dispatch_0001, TestSize.Level1)
{
    callStateMachine.InitState(ALERTING_STATE);

    HILOGI("Call_Statemachine_Alertingtate_Dispatch_0001 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {1, 0, HFP_AG_CALL_STATE_IDLE, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(alertingState->GetStateInt(), HFP_AG_CALL_STATE_ALERTING);
    HILOGI("Call_Statemachine_Alertingtate_Dispatch_0001 end");
}

/*
 * @tc.number: Call_Statemachine_Alertingtate_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_Alertingtate_Dispatch_0002, TestSize.Level1)
{
    callStateMachine.InitState(ALERTING_STATE);

    HILOGI("Call_Statemachine_Alertingtate_Dispatch_0002 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {0, 0, HFP_AG_CALL_STATE_IDLE, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(alertingState->GetStateInt(), HFP_AG_CALL_STATE_ALERTING);
    HILOGI("Call_Statemachine_Alertingtate_Dispatch_0002 end");
}

/*
 * @tc.number: Call_Statemachine_Alertingtate_Dispatch_0003
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_Alertingtate_Dispatch_0003, TestSize.Level1)
{
    callStateMachine.InitState(ALERTING_STATE);

    HILOGI("Call_Statemachine_Alertingtate_Dispatch_0003 start");
    callStateMachine.TransitionToTargetState(CallStateMachine::CALL_ACTIVE_EVENT);

    EXPECT_EQ(alertingState->GetStateInt(), HFP_AG_CALL_STATE_ALERTING);
    HILOGI("Call_Statemachine_Alertingtate_Dispatch_0003 end");
}

/*
 * @tc.number: Call_Statemachine_Alertingtate_Dispatch_0004
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_Alertingtate_Dispatch_0004, TestSize.Level1)
{
    callStateMachine.InitState(ALERTING_STATE);

    HILOGI("Call_Statemachine_Alertingtate_Dispatch_0004 start");
    callStateMachine.TransitionToTargetState(CallStateMachine::CALL_END_EVENT);

    EXPECT_EQ(alertingState->GetStateInt(), HFP_AG_CALL_STATE_ALERTING);
    HILOGI("Call_Statemachine_Alertingtate_Dispatch_0004 end");
}

/*
 * @tc.number: Call_Statemachine_ActiveState_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_ActiveState_Entry_0001, TestSize.Level1)
{
    HILOGI("Call_Statemachine_ActiveState_Entry_0001 start");
    activeState->Exit();
    activeState->Entry();
    EXPECT_EQ(activeState->GetStateInt(), HFP_AG_CALL_STATE_IDLE);

    HILOGI("Call_Statemachine_ActiveState_Entry_0001 end");
}

/*
 * @tc.number: Call_Statemachine_ActiveState_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_ActiveState_Dispatch_0001, TestSize.Level1)
{
    callStateMachine.InitState(CALL_ACTIVE_STATE);

    HILOGI("Call_Statemachine_ActiveState_Dispatch_0001 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {0, 0, HFP_AG_CALL_STATE_IDLE, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(callStateMachine.GetCallState(), HFP_AG_CALL_STATE_IDLE);
    HILOGI("Call_Statemachine_ActiveState_Dispatch_0001 end");
}

/*
 * @tc.number: Call_Statemachine_ActiveState_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_ActiveState_Dispatch_0002, TestSize.Level1)
{
    callStateMachine.InitState(CALL_ACTIVE_STATE);

    HILOGI("Call_Statemachine_ActiveState_Dispatch_0002 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {5, 0, HFP_AG_CALL_STATE_IDLE, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(activeState->GetStateInt(), HFP_AG_CALL_STATE_IDLE);
    HILOGI("Call_Statemachine_ActiveState_Dispatch_0002 end");
}

/*
 * @tc.number: Call_Statemachine_EndState_Entry_0001
 * @tc.name: Entry
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_EndState_Entry_0001, TestSize.Level1)
{
    HILOGI("Call_Statemachine_EndState_Entry_0001 start");
    endState->Exit();
    endState->Entry();
    EXPECT_EQ(endState->GetStateInt(), HFP_AG_CALL_STATE_IDLE);

    HILOGI("Call_Statemachine_EndState_Entry_0001 end");
}

/*
 * @tc.number: Call_Statemachine_EndState_Dispatch_0001
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_EndState_Dispatch_0001, TestSize.Level1)
{
    callStateMachine.InitState(CALL_END_STATE);

    HILOGI("Call_Statemachine_EndState_Dispatch_0001 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {0, 0, HFP_AG_CALL_STATE_DIALING, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(callStateMachine.GetCallState(), HFP_AG_CALL_STATE_DIALING);
    HILOGI("Call_Statemachine_EndState_Dispatch_0001 end");
}

/*
 * @tc.number: Call_Statemachine_EndState_Dispatch_0002
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_EndState_Dispatch_0002, TestSize.Level1)
{
    callStateMachine.InitState(CALL_END_STATE);

    HILOGI("Call_Statemachine_EndState_Dispatch_0002 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {0, 0, HFP_AG_CALL_STATE_INCOMING, "110112", 0, ""};

    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(callStateMachine.GetCallState(), HFP_AG_CALL_STATE_IDLE);
    HILOGI("Call_Statemachine_EndState_Dispatch_0002 end");
}

/*
 * @tc.number: Call_Statemachine_DealCallStateChangeWhileDialing_0001
 * @tc.name: DealCallStateChangeWhileDialing
 * @tc.desc:
 */
HWTEST_F(CallStatemachineTest, Call_Statemachine_DealCallStateChangeWhileDialing_0001, TestSize.Level1)
{
    callStateMachine.InitState(DIALING_STATE);

    HILOGI("Call_Statemachine_DealCallStateChangeWhileDialing_0001 start");
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {0, 0, HFP_AG_CALL_STATE_ALERTING, "110112", 0, ""};
    callStateMachine.DealVoipCallStateChanged(curEvent);

    EXPECT_EQ(callStateMachine.GetCallState(), HFP_AG_CALL_STATE_IDLE);
    HfpAgPhoneState state{0, 0, HFP_AG_CALL_STATE_IDLE, "110112", 0, ""};
    callStateMachine.DealCallStateChangeWhileDialing(state);
    HILOGI("Call_Statemachine_DealCallStateChangeWhileDialing_0001 end");
}
}  // namespace bluetooth
}  // namespace OHOS