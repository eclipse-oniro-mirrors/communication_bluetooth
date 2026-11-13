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
#include "sys_state_machine.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
constexpr int TIME = 2;
SysStateMachine sysStateMachine;
AdapterManager* am = AdapterManager::GetInstance();;

class SysStateMachineTest : public testing::Test {
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
        host_ = nullptr;
    }
    void SetUp()
    {
        pSysStoppedState = std::make_unique<SysStoppedState>(sysStateMachine, *am);
        pSysStartedState = std::make_unique<SysStartedState>(sysStateMachine, *am);
        pSysStoppingState = std::make_unique<SysStoppingState>(sysStateMachine, *am);
        pSysResettingState = std::make_unique<SysResettingState>(sysStateMachine, *am);
        pSysFactoryResettingState = std::make_unique<SysFactoryResettingState>(sysStateMachine, *am);
    }
    void TearDown()
    {
        pSysStoppedState.reset();
        pSysStartedState.reset();
        pSysStoppingState.reset();
        pSysResettingState.reset();
        pSysFactoryResettingState.reset();
    }
public:
    std::unique_ptr<SysStoppedState> pSysStoppedState;
    std::unique_ptr<SysStartedState> pSysStartedState;
    std::unique_ptr<SysStoppingState> pSysStoppingState;
    std::unique_ptr<SysResettingState> pSysResettingState;
    std::unique_ptr<SysFactoryResettingState> pSysFactoryResettingState;
};

/*
 * @tc.number: SysState_MachineTest_Dispatch_00100
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(SysStateMachineTest, SysState_MachineTest_Dispatch_00100, TestSize.Level1)
{
    HILOGI("SysState_MachineTest_Dispatch_00100 start");
    utility::Message msg_sys_start_cmp(SysStateMachine::MSG_SYS_START_CMP);
    utility::Message msg_sys_adapter_state_change_req(SysStateMachine::MSG_SYS_ADAPTER_STATE_CHANGE_REQ);

    EXPECT_TRUE(pSysStoppedState->Dispatch(msg_sys_start_cmp));
    EXPECT_FALSE(pSysStoppedState->Dispatch(msg_sys_adapter_state_change_req));

    HILOGI("SysState_MachineTest_Dispatch_00100 end");
}

/*
 * @tc.number: SysState_MachineTest_Dispatch_00200
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(SysStateMachineTest, SysState_MachineTest_Dispatch_00200, TestSize.Level1)
{
    HILOGI("SysState_MachineTest_Dispatch_00200 start");

    utility::Message msg_sys_reset_req(SysStateMachine::MSG_SYS_RESET_REQ);
    utility::Message msg_sys_factory_reset_req(SysStateMachine::MSG_SYS_FACTORY_RESET_REQ);
    utility::Message msg_sys_stop_req(SysStateMachine::MSG_SYS_STOP_REQ);
    utility::Message msg_sys_factory_reset_cmp(SysStateMachine::MSG_SYS_FACTORY_RESET_CMP);

    EXPECT_TRUE(pSysStartedState->Dispatch(msg_sys_reset_req));
    EXPECT_TRUE(pSysStartedState->Dispatch(msg_sys_factory_reset_req));
    EXPECT_TRUE(pSysStartedState->Dispatch(msg_sys_stop_req));
    EXPECT_FALSE(pSysStartedState->Dispatch(msg_sys_factory_reset_cmp));

    HILOGI("SysState_MachineTest_Dispatch_00200 end");
}

/*
 * @tc.number: SysState_MachineTest_Dispatch_00300
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(SysStateMachineTest, SysState_MachineTest_Dispatch_00300, TestSize.Level1)
{
    HILOGI("SysState_MachineTest_Dispatch_00300 start");

    utility::Message msg_sys_stop_cmp(SysStateMachine::MSG_SYS_STOP_CMP);
    utility::Message msg_sys_adapter_state_change_req(SysStateMachine::MSG_SYS_ADAPTER_STATE_CHANGE_REQ);
    utility::Message msg_sys_clear_all_storage_cmp(SysStateMachine::MSG_SYS_CLEAR_ALL_STORAGE_CMP);

    EXPECT_TRUE(pSysStoppingState->Dispatch(msg_sys_stop_cmp));
    EXPECT_TRUE(pSysStoppingState->Dispatch(msg_sys_adapter_state_change_req));
    EXPECT_FALSE(pSysStoppingState->Dispatch(msg_sys_clear_all_storage_cmp));

    HILOGI("SysState_MachineTest_Dispatch_00300 end");
}

/*
 * @tc.number: SysState_MachineTest_Dispatch_00400
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(SysStateMachineTest, SysState_MachineTest_Dispatch_00400, TestSize.Level1)
{
    HILOGI("SysState_MachineTest_Dispatch_00400 start");

    utility::Message msg_sys_adapter_state_change_req(SysStateMachine::MSG_SYS_ADAPTER_STATE_CHANGE_REQ);
    utility::Message msg_sys_stop_cmp(SysStateMachine::MSG_SYS_STOP_CMP);
    utility::Message msg_sys_stop_req(SysStateMachine::MSG_SYS_STOP_REQ);
    utility::Message msg_sys_clear_all_storage_cmp(SysStateMachine::MSG_SYS_CLEAR_ALL_STORAGE_CMP);

    EXPECT_TRUE(pSysResettingState->Dispatch(msg_sys_adapter_state_change_req));
    // libsamgr_proxy.z.so cficheck 导致crash 暂时屏蔽Dispatch(msg_sys_stop_cmp)
    EXPECT_TRUE(pSysResettingState->Dispatch(msg_sys_stop_req));
    EXPECT_FALSE(pSysResettingState->Dispatch(msg_sys_clear_all_storage_cmp));

    HILOGI("SysState_MachineTest_Dispatch_00400 end");
}

/*
 * @tc.number: SysState_MachineTest_Dispatch_00500
 * @tc.name: Dispatch
 * @tc.desc:
 */
HWTEST_F(SysStateMachineTest, SysState_MachineTest_Dispatch_00500, TestSize.Level1)
{
    HILOGI("SysState_MachineTest_Dispatch_00500 start");

    utility::Message msg_sys_adapter_state_change_req(SysStateMachine::MSG_SYS_ADAPTER_STATE_CHANGE_REQ);
    utility::Message msg_sys_stop_cmp(SysStateMachine::MSG_SYS_STOP_CMP);
    utility::Message msg_sys_stop_req(SysStateMachine::MSG_SYS_STOP_REQ);
    utility::Message msg_sys_clear_all_storage_cmp(SysStateMachine::MSG_SYS_CLEAR_ALL_STORAGE_CMP);
    utility::Message msg_sys_factory_reset_cmp(SysStateMachine::MSG_SYS_FACTORY_RESET_CMP);

    EXPECT_TRUE(pSysFactoryResettingState->Dispatch(msg_sys_adapter_state_change_req));
    EXPECT_TRUE(pSysFactoryResettingState->Dispatch(msg_sys_stop_cmp));
    EXPECT_TRUE(pSysFactoryResettingState->Dispatch(msg_sys_stop_req));
    // libsamgr_proxy.z.so cficheck 导致crash 暂时屏蔽Dispatch(msg_sys_clear_all_storage_cmp)
    EXPECT_FALSE(pSysFactoryResettingState->Dispatch(msg_sys_factory_reset_cmp));

    HILOGI("SysState_MachineTest_Dispatch_00500 end");
}

}  // namespace bluetooth
}  // namespace OHOS