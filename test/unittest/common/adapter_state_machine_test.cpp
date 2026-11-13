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
#include <cstring>
#include "adapter_state_machine.h"
#include "power_device.h"
#include "btcommon/state_machine.h"
#include "class_creator.h"
#include "adapter_config.h"
#include "ble_adapter.h"
#include "bluetooth_host.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
AdapterStateMachine stateMachine;

static std::unique_ptr<BleAdapter> adapter;

class AdapterStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        adapter = std::make_unique<BleAdapter>();
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
        adapter.reset();
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp()
    {
        pAdapterTurnOffState = std::make_unique<AdapterTurnOffState>(stateMachine, adapter->GetContext());
        pAdapterTurningOnState = std::make_unique<AdapterTurningOnState>(stateMachine, adapter->GetContext());
        pAdapterTurnOnState = std::make_unique<AdapterTurnOnState>(stateMachine, adapter->GetContext());
        pAdapterTurningOffState = std::make_unique<AdapterTurningOffState>(stateMachine, adapter->GetContext());
    }
    void TearDown()
    {
        sleep(TIME);
        pAdapterTurnOffState.reset();
        pAdapterTurningOnState.reset();
        pAdapterTurnOnState.reset();
        pAdapterTurningOffState.reset();
    }
public:
    std::unique_ptr<AdapterTurnOffState> pAdapterTurnOffState;
    std::unique_ptr<AdapterTurningOnState> pAdapterTurningOnState;
    std::unique_ptr<AdapterTurnOnState> pAdapterTurnOnState;
    std::unique_ptr<AdapterTurningOffState> pAdapterTurningOffState;
};

/*
 * @tc.number: PWState_MachineTest_Dispatch_00600
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(AdapterStateMachineTest, PWState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00100 start");

    utility::Message msg_profile_disable_time_out(AdapterStateMachine::MSG_PROFILE_DISABLE_TIME_OUT);

    EXPECT_FALSE(pAdapterTurnOffState->Dispatch(msg_profile_disable_time_out));

    HILOGI("PWState_MachineTest_Dispatch_00100 end");
}

/*
 * @tc.number: PWState_MachineTest_DispatchOtherMsg_00600
 * @tc.name: DispatchOtherMsg
 * @tc.desc:
 */
HWTEST_F(AdapterStateMachineTest, PWState_MachineTest_DispatchOtherMsg_00100, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_DispatchOtherMsg_00100 start");

    utility::Message msg_adapter_enable_time_out(AdapterStateMachine::MSG_ADAPTER_ENABLE_TIME_OUT);
    utility::Message msg_profile_enable_time_out(AdapterStateMachine::MSG_PROFILE_ENABLE_TIME_OUT);
    utility::Message msg_profile_disable_time_out(AdapterStateMachine::MSG_PROFILE_DISABLE_TIME_OUT);
    utility::Message msg_adapter_disable_time_out(AdapterStateMachine::MSG_ADAPTER_DISABLE_TIME_OUT);

    EXPECT_TRUE(pAdapterTurningOnState->Dispatch(msg_adapter_enable_time_out));
    EXPECT_TRUE(pAdapterTurningOnState->Dispatch(msg_profile_enable_time_out));
    EXPECT_FALSE(pAdapterTurningOnState->Dispatch(msg_profile_disable_time_out));
    EXPECT_FALSE(pAdapterTurningOnState->Dispatch(msg_adapter_disable_time_out));

    HILOGI("PWState_MachineTest_DispatchOtherMsg_00100 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00200
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(AdapterStateMachineTest, PWState_MachineTest_Dispatch_00200, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00200 start");

    utility::Message msg_adapter_enable_cmp(AdapterStateMachine::MSG_ADAPTER_ENABLE_CMP);
    utility::Message msg_profile_enable_cmp(AdapterStateMachine::MSG_PROFILE_ENABLE_CMP);
    utility::Message msg_stack_enable_cmp(AdapterStateMachine::MSG_STACK_ENABLE_CMP);
    utility::Message msg_adapter_enable_time_out(AdapterStateMachine::MSG_ADAPTER_ENABLE_TIME_OUT);

    EXPECT_TRUE(pAdapterTurningOnState->Dispatch(msg_adapter_enable_cmp));
    EXPECT_TRUE(pAdapterTurningOnState->Dispatch(msg_profile_enable_cmp));
    EXPECT_TRUE(pAdapterTurningOnState->Dispatch(msg_stack_enable_cmp));
    EXPECT_TRUE(pAdapterTurningOnState->Dispatch(msg_adapter_enable_time_out));

    HILOGI("PWState_MachineTest_Dispatch_00200 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00300
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(AdapterStateMachineTest, PWState_MachineTest_Dispatch_00300, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00300 start");

    utility::Message msg_user_disable_req(AdapterStateMachine::MSG_USER_DISABLE_REQ);
    utility::Message msg_profile_enable_cmp(AdapterStateMachine::MSG_PROFILE_ENABLE_CMP);

    EXPECT_TRUE(pAdapterTurnOnState->Dispatch(msg_user_disable_req));
    EXPECT_FALSE(pAdapterTurnOnState->Dispatch(msg_profile_enable_cmp));

    HILOGI("PWState_MachineTest_Dispatch_00300 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00400
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(AdapterStateMachineTest, PWState_MachineTest_Dispatch_00400, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00400 start");

    utility::Message msg_profile_disable_time_out(AdapterStateMachine::MSG_PROFILE_DISABLE_TIME_OUT);
    utility::Message msg_adapter_disable_time_out(AdapterStateMachine::MSG_ADAPTER_DISABLE_TIME_OUT);
    utility::Message msg_profile_enable_cmp(AdapterStateMachine::MSG_PROFILE_ENABLE_CMP);

    EXPECT_TRUE(pAdapterTurningOffState->Dispatch(msg_profile_disable_time_out));
    EXPECT_FALSE(pAdapterTurningOffState->Dispatch(msg_adapter_disable_time_out));
    EXPECT_FALSE(pAdapterTurningOffState->Dispatch(msg_profile_enable_cmp));

    HILOGI("PWState_MachineTest_Dispatch_00400 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00500
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(AdapterStateMachineTest, PWState_MachineTest_Dispatch_00500, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00500 start");

    utility::Message msg_profile_disable_cmp(AdapterStateMachine::MSG_PROFILE_DISABLE_CMP);
    utility::Message msg_adapter_disable_cmp(AdapterStateMachine::MSG_ADAPTER_DISABLE_CMP);
    utility::Message msg_stack_disable_cmp(AdapterStateMachine::MSG_STACK_DISABLE_CMP);

    EXPECT_TRUE(pAdapterTurningOffState->Dispatch(msg_profile_disable_cmp));
    EXPECT_FALSE(pAdapterTurningOffState->Dispatch(msg_adapter_disable_cmp));
    EXPECT_TRUE(pAdapterTurningOffState->Dispatch(msg_stack_disable_cmp));

    sleep(TIME);
    HILOGI("PWState_MachineTest_Dispatch_00500 end");
}

HWTEST_F(AdapterStateMachineTest, PWState_MachineTest_SetCallingName_001, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_SetCallingName_001 start");

    stateMachine.SetCallingName("test_calling_name");
    EXPECT_EQ(stateMachine.callingName_, "test_calling_name");

    HILOGI("PWState_MachineTest_SetCallingName_001 end");
}

HWTEST_F(AdapterStateMachineTest, PWState_MachineTest_Dispatch_MSG_USER_ENABLE_REQ, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_MSG_USER_ENABLE_REQ start");

    utility::Message msg(AdapterStateMachine::MSG_USER_ENABLE_REQ);
    stateMachine.SetCallingName("test_caller");

    EXPECT_TRUE(pAdapterTurnOffState->Dispatch(msg));
    EXPECT_EQ(stateMachine.callingName_, "test_caller");

    HILOGI("PWState_MachineTest_Dispatch_MSG_USER_ENABLE_REQ end");
}

}  // namespace bluetooth
}  // namespace OHOS