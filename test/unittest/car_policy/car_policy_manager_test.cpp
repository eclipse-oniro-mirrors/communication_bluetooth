/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "car_policy_manager.h"

#include <gtest/gtest.h>

#include "dispatcher/car_policy_dispatcher.h"
#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

// 单例不调 Init：仅验证带 initialized_ 守卫的入口在未初始化时的 fail-open 契约。
class CarPolicyManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("CarPolicyManagerTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("CarPolicyManagerTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("CarPolicyManagerTest::SetUp start");
    }
    void TearDown() override
    {
        HILOGI("CarPolicyManagerTest::TearDown start");
    }
};

/**
 * @tc.name: Car_policy_manager_GetInstance_SameInstance_001
 * @tc.desc: GetInstance 返回同一实例
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_GetInstance_SameInstance_001, TestSize.Level1)
{
    HILOGI("Car_policy_manager_GetInstance_SameInstance_001 enter");
    auto &inst1 = CarPolicyManager::GetInstance();
    auto &inst2 = CarPolicyManager::GetInstance();
    EXPECT_EQ(&inst1, &inst2);
    HILOGI("Car_policy_manager_GetInstance_SameInstance_001 end");
}

/**
 * @tc.name: Car_policy_manager_GetBtSettingsUiActive_DefaultZero_002
 * @tc.desc: 未 Init 时 GetBtSettingsUiActive 返回 0
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_GetBtSettingsUiActive_DefaultZero_002, TestSize.Level1)
{
    HILOGI("Car_policy_manager_GetBtSettingsUiActive_DefaultZero_002 enter");
    EXPECT_EQ(CarPolicyManager::GetInstance().GetBtSettingsUiActive(), 0);
    HILOGI("Car_policy_manager_GetBtSettingsUiActive_DefaultZero_002 end");
}

/**
 * @tc.name: Car_policy_manager_IsAllowConnect_BeforeInit_ReturnTrue_003
 * @tc.desc: 未 Init 时 IsAllowConnect fail-open 返回 true（允许连接）
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_IsAllowConnect_BeforeInit_ReturnTrue_003, TestSize.Level1)
{
    HILOGI("Car_policy_manager_IsAllowConnect_BeforeInit_ReturnTrue_003 enter");
    EXPECT_TRUE(CarPolicyManager::GetInstance().IsAllowConnect("00:11:22:33:44:55",
        ProfileType::HFP_HF));
    HILOGI("Car_policy_manager_IsAllowConnect_BeforeInit_ReturnTrue_003 end");
}

/**
 * @tc.name: Car_policy_manager_EventEntries_BeforeInit_NoCrash_004
 * @tc.desc: 未 Init 时各事件入口带守卫直接返回，批量调用不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_EventEntries_BeforeInit_NoCrash_004, TestSize.Level0)
{
    HILOGI("Car_policy_manager_EventEntries_BeforeInit_NoCrash_004 enter");
    CarPolicyManager &mgr = CarPolicyManager::GetInstance();
    mgr.TerminateReconnect();
    mgr.StartReconnect(ReconnectReason::BT_ON_AUTO_CONN);
    mgr.OnAclDisconnect("00:11:22:33:44:55", 8);
    mgr.OnBluetoothOn();
    mgr.OnProfileStateChanged("00:11:22:33:44:55", ProfileType::HFP_HF, 1, -1);
    mgr.OnFaStatusChanged(1);
    mgr.OnHiechoNotifyOutputPath(2, "00:11:22:33:44:55");
    mgr.OnBondStateChanged("00:11:22:33:44:55", 3);
    mgr.CancelPendingConnectOtherProfiles("00:11:22:33:44:55");
    mgr.Uninit();
    SUCCEED();
    HILOGI("Car_policy_manager_EventEntries_BeforeInit_NoCrash_004 end");
}

/**
 * @tc.name: Car_policy_manager_OnIviPowerStateChanged_BeforeInit_NoCrash_005
 * @tc.desc: 未 Init 时各 IVI 电源状态值经守卫直接丢弃，批量调用不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_OnIviPowerStateChanged_BeforeInit_NoCrash_005, TestSize.Level0)
{
    HILOGI("Car_policy_manager_OnIviPowerStateChanged_BeforeInit_NoCrash_005 enter");
    CarPolicyManager &mgr = CarPolicyManager::GetInstance();
    // 全部契约状态值：STR/STANDBY/ACTIVE/SHUTDOWN/REBOOT + 未知值（防御性）
    const int32_t states[] = {0, 1, 2, 3, 4, 99, -1};
    for (int32_t state : states) {
        mgr.OnIviPowerStateChanged(state);
    }
    // 重复调用（广播重发场景）同样安全
    mgr.OnIviPowerStateChanged(1);
    mgr.OnIviPowerStateChanged(1);
    SUCCEED();
    HILOGI("Car_policy_manager_OnIviPowerStateChanged_BeforeInit_NoCrash_005 end");
}

/**
 * @tc.name: Car_policy_manager_IsInIviStandby_DefaultFalse_006
 * @tc.desc: 未 Init 时 IsInIviStandby 默认 false（IviStandbyPredicate 不误终止回连）
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_IsInIviStandby_DefaultFalse_006, TestSize.Level1)
{
    HILOGI("Car_policy_manager_IsInIviStandby_DefaultFalse_006 enter");
    EXPECT_FALSE(CarPolicyManager::GetInstance().IsInIviStandby());
    HILOGI("Car_policy_manager_IsInIviStandby_DefaultFalse_006 end");
}

// ── 跨仓契约常量断言（契约常量随订阅组件位于 car_policy_dispatcher.h）：
//    与融合仓 fcm_common_event_helper.cpp 严格一致，UT 断言防两仓契约漂移
//   （修改需两仓同步评审）──

/**
 * @tc.name: Car_policy_manager_IviPowerContract_Action_007
 * @tc.desc: 契约 action 字符串与融合仓发布侧一致
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_IviPowerContract_Action_007, TestSize.Level0)
{
    HILOGI("Car_policy_manager_IviPowerContract_Action_007 enter");
    EXPECT_STREQ(IVI_POWER_STATE_CHANGED_ACTION, "usual.event.fusion_connectivity.IVI_POWER_STATE_CHANGED");
    HILOGI("Car_policy_manager_IviPowerContract_Action_007 end");
}

/**
 * @tc.name: Car_policy_manager_IviPowerContract_Key_008
 * @tc.desc: 契约参数 key 与融合仓发布侧一致
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_IviPowerContract_Key_008, TestSize.Level0)
{
    HILOGI("Car_policy_manager_IviPowerContract_Key_008 enter");
    EXPECT_STREQ(IVI_POWER_STATE_KEY, "ivi_power_state");
    HILOGI("Car_policy_manager_IviPowerContract_Key_008 end");
}

/**
 * @tc.name: Car_policy_manager_IviPowerContract_StateValues_009
 * @tc.desc: 契约状态值编码与电源 SA 推送值一致（0=STR/1=STANDBY/2=ACTIVE/3=SHUTDOWN/4=REBOOT）
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_IviPowerContract_StateValues_009, TestSize.Level0)
{
    HILOGI("Car_policy_manager_IviPowerContract_StateValues_009 enter");
    EXPECT_EQ(IVI_POWER_STATE_STR, 0);
    EXPECT_EQ(IVI_POWER_STATE_STANDBY, 1);
    EXPECT_EQ(IVI_POWER_STATE_ACTIVE, 2);
    EXPECT_EQ(IVI_POWER_STATE_SHUTDOWN, 3);
    EXPECT_EQ(IVI_POWER_STATE_REBOOT, 4);
    EXPECT_EQ(IVI_POWER_STATE_INVALID, -1);
    HILOGI("Car_policy_manager_IviPowerContract_StateValues_009 end");
}

/**
 * @tc.name: Car_policy_manager_OnSubProfileSwitched_BeforeInit_NoCrash_010
 * @tc.desc: 未 Init 时 OnSubProfileSwitched（binder SWITCHED 回调入口）直接返回不崩溃：
 *           dispatcher 未创建不布防看门狗，也不触发账号查询与切号处理
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_OnSubProfileSwitched_BeforeInit_NoCrash_010,
    TestSize.Level1)
{
    HILOGI("Car_policy_manager_OnSubProfileSwitched_BeforeInit_NoCrash_010 enter");
    CarPolicyManager::GetInstance().OnSubProfileSwitched();
    SUCCEED();
    HILOGI("Car_policy_manager_OnSubProfileSwitched_BeforeInit_NoCrash_010 end");
}

/**
 * @tc.name: Car_policy_manager_UserConfigUpdateFinished_BeforeInit_NoCrash_011
 * @tc.desc: 未 Init 时 user_config_update_finished 广播转发入口批量调用不崩溃
 *           （handler 内部分支语义由 sub_profile_switch_handler_test 覆盖）
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyManagerTest, Car_policy_manager_UserConfigUpdateFinished_BeforeInit_NoCrash_011,
    TestSize.Level1)
{
    HILOGI("Car_policy_manager_UserConfigUpdateFinished_BeforeInit_NoCrash_011 enter");
    CarPolicyManager &mgr = CarPolicyManager::GetInstance();
    // 广播重发/重复触发场景
    mgr.OnUserConfigUpdateFinished();
    mgr.OnUserConfigUpdateFinished();
    mgr.OnUserConfigUpdateFinished();
    SUCCEED();
    HILOGI("Car_policy_manager_UserConfigUpdateFinished_BeforeInit_NoCrash_011 end");
}
}  // namespace bluetooth
}  // namespace OHOS
