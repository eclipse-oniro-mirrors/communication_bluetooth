
/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "power_state_machine.h"
#include "power_device.h"
#include "btcommon/state_machine.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
constexpr int TIME = 2;
PowerStateMachine psm;
RawAddress rawAddr;
PowerDevice pd(rawAddr);
PowerActiveState pas(psm, pd);

class PowerStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

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
        sleep(TIME);
        host_ = nullptr;
    }
    void SetUp()
    {
        pPowerActiveState = std::make_unique<PowerActiveState>(psm, pd);
        pPowerActiveActivingState = std::make_unique<PowerActiveActivingState>(psm, pd, pas);
        pPowerActiveSniffingState = std::make_unique<PowerActiveSniffingState>(psm, pd, pas);
        pPowerSniffState = std::make_unique<PowerSniffState>(psm, pd);
        pPowerSniffActivingState = std::make_unique<PowerSniffActivingState>(psm, pd, pas);
        pPowerSniffSniffingState = std::make_unique<PowerSniffSniffingState>(psm, pd, pas);
    }
    void TearDown()
    {
        pPowerActiveState.reset();
        pPowerActiveActivingState.reset();
        pPowerActiveSniffingState.reset();
        pPowerSniffState.reset();
        pPowerSniffActivingState.reset();
        pPowerSniffSniffingState.reset();
    }
public:
    std::unique_ptr<PowerActiveState> pPowerActiveState;
    std::unique_ptr<PowerActiveActivingState> pPowerActiveActivingState;
    std::unique_ptr<PowerActiveSniffingState> pPowerActiveSniffingState;
    std::unique_ptr<PowerSniffState> pPowerSniffState;
    std::unique_ptr<PowerSniffActivingState> pPowerSniffActivingState;
    std::unique_ptr<PowerSniffSniffingState> pPowerSniffSniffingState;
};

/*
 * @tc.number: PWState_MachineTest_Dispatch_00100
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PowerStateMachineTest, PWState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00100 start");

    utility::Message msg_pm_mode_change_sniff(PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF);
    utility::Message msg_pm_set_sniff(PowerStateMachine::MSG_PM_SET_SNIFF);
    utility::Message msg_pm_set_subrating_complete(PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE);

    EXPECT_TRUE(pPowerActiveState->Dispatch(msg_pm_mode_change_sniff));
    EXPECT_TRUE(pPowerActiveState->Dispatch(msg_pm_set_sniff));
    EXPECT_FALSE(pPowerActiveState->Dispatch(msg_pm_set_subrating_complete));

    HILOGI("PWState_MachineTest_Dispatch_00100 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00200
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PowerStateMachineTest, PWState_MachineTest_Dispatch_00200, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00200 start");

    utility::Message msg_pm_mode_change_active(PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE);
    utility::Message msg_pm_set_active(PowerStateMachine::MSG_PM_SET_ACTIVE);
    utility::Message msg_pm_set_sniff(PowerStateMachine::MSG_PM_SET_SNIFF);
    utility::Message msg_pm_mode_change_sniff(PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF);
    utility::Message msg_pm_set_subrating_complete(PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE);

    EXPECT_TRUE(pPowerActiveActivingState->Dispatch(msg_pm_mode_change_active));
    EXPECT_TRUE(pPowerActiveActivingState->Dispatch(msg_pm_set_active));
    EXPECT_TRUE(pPowerActiveActivingState->Dispatch(msg_pm_set_sniff));
    EXPECT_FALSE(pPowerActiveActivingState->Dispatch(msg_pm_mode_change_sniff));
    EXPECT_FALSE(pPowerActiveActivingState->Dispatch(msg_pm_set_subrating_complete));

    HILOGI("PWState_MachineTest_Dispatch_00200 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00300
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PowerStateMachineTest, PWState_MachineTest_Dispatch_00300, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00300 start");

    utility::Message msg_pm_mode_change_active(PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE);
    utility::Message msg_pm_set_active(PowerStateMachine::MSG_PM_SET_ACTIVE);
    utility::Message msg_pm_set_sniff(PowerStateMachine::MSG_PM_SET_SNIFF);
    utility::Message msg_pm_mode_change_sniff(PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF);
    utility::Message msg_pm_set_subrating_complete(PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE);

    EXPECT_TRUE(pPowerActiveSniffingState->Dispatch(msg_pm_mode_change_active));
    EXPECT_TRUE(pPowerActiveSniffingState->Dispatch(msg_pm_set_active));
    EXPECT_TRUE(pPowerActiveSniffingState->Dispatch(msg_pm_set_sniff));
    EXPECT_FALSE(pPowerActiveSniffingState->Dispatch(msg_pm_mode_change_sniff));
    EXPECT_TRUE(pPowerActiveSniffingState->Dispatch(msg_pm_set_subrating_complete));

HILOGI("PWState_MachineTest_Dispatch_00300 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00400
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PowerStateMachineTest, PWState_MachineTest_Dispatch_00400, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00400 start");

    utility::Message msg_pm_mode_change_active(PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE);
    utility::Message msg_pm_set_active(PowerStateMachine::MSG_PM_SET_ACTIVE);
    utility::Message msg_pm_set_sniff(PowerStateMachine::MSG_PM_SET_SNIFF);
    utility::Message msg_pm_mode_change_sniff(PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF);
    utility::Message msg_pm_set_subrating_complete(PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE);

    EXPECT_TRUE(pPowerSniffState->Dispatch(msg_pm_mode_change_active));
    EXPECT_TRUE(pPowerSniffState->Dispatch(msg_pm_set_active));
    EXPECT_TRUE(pPowerSniffState->Dispatch(msg_pm_set_sniff));
    EXPECT_FALSE(pPowerSniffState->Dispatch(msg_pm_mode_change_sniff));
    EXPECT_FALSE(pPowerSniffState->Dispatch(msg_pm_set_subrating_complete));

    HILOGI("PWState_MachineTest_Dispatch_00400 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00500
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PowerStateMachineTest, PWState_MachineTest_Dispatch_00500, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00500 start");

    utility::Message msg_pm_mode_change_active(PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE);
    utility::Message msg_pm_set_active(PowerStateMachine::MSG_PM_SET_ACTIVE);
    utility::Message msg_pm_set_sniff(PowerStateMachine::MSG_PM_SET_SNIFF);
    utility::Message msg_pm_mode_change_sniff(PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF);
    utility::Message msg_pm_set_subrating_complete(PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE);

    EXPECT_FALSE(pPowerSniffActivingState->Dispatch(msg_pm_mode_change_active));
    EXPECT_TRUE(pPowerSniffActivingState->Dispatch(msg_pm_set_active));
    EXPECT_TRUE(pPowerSniffActivingState->Dispatch(msg_pm_set_sniff));
    EXPECT_TRUE(pPowerSniffActivingState->Dispatch(msg_pm_mode_change_sniff));
    EXPECT_FALSE(pPowerSniffActivingState->Dispatch(msg_pm_set_subrating_complete));

    HILOGI("PWState_MachineTest_Dispatch_00500 end");
}

/*
 * @tc.number: PWState_MachineTest_Dispatch_00600
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(PowerStateMachineTest, PWState_MachineTest_Dispatch_00600, TestSize.Level1)
{
    HILOGI("PWState_MachineTest_Dispatch_00600 start");

    utility::Message msg_pm_mode_change_active(PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE);
    utility::Message msg_pm_set_active(PowerStateMachine::MSG_PM_SET_ACTIVE);
    utility::Message msg_pm_set_sniff(PowerStateMachine::MSG_PM_SET_SNIFF);
    utility::Message msg_pm_mode_change_sniff(PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF);
    utility::Message msg_pm_set_subrating_complete(PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE);

    EXPECT_FALSE(pPowerSniffSniffingState->Dispatch(msg_pm_mode_change_active));
    EXPECT_TRUE(pPowerSniffSniffingState->Dispatch(msg_pm_set_active));
    EXPECT_TRUE(pPowerSniffSniffingState->Dispatch(msg_pm_set_sniff));
    EXPECT_TRUE(pPowerSniffSniffingState->Dispatch(msg_pm_mode_change_sniff));
    EXPECT_TRUE(pPowerSniffSniffingState->Dispatch(msg_pm_set_subrating_complete));

    HILOGI("PWState_MachineTest_Dispatch_00600 end");
}

}  // namespace bluetooth
}  // namespace OHOS
