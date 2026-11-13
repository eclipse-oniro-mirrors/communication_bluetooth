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

#include "reconnect_engine.h"

#include <gtest/gtest.h>

#include "car_policy_config.h"
#include "car_policy_manager.h"
#include "dispatcher/car_policy_dispatcher.h"
#include "guard/car_connection_guard.h"
#include "log.h"
#include "reconnect/device_priority_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

// 静态组件图：重试回连经 dispatcher 投递延时任务并捕获 this，
// 引擎需进程级存活，避免延时回调执行时访问已销毁对象
static CarPolicyConfig g_config;
static DevicePriorityManager g_dpm;
static CarPolicyDispatcher g_dispatcher(CarPolicyManager::GetInstance());
static CarConnectionGuard g_guard;
static ReconnectEngine g_engine(g_config, g_dpm, g_dispatcher, g_guard);
static bool g_dpmInited = false;

const std::string DEV_A = "00:11:22:33:44:55";

class ReconnectEngineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("ReconnectEngineTest::SetUpTestCase start");
        if (!g_dpmInited) {
            g_dpm.Init();
            g_dpmInited = true;
        }
    }
    static void TearDownTestCase()
    {
        HILOGI("ReconnectEngineTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("ReconnectEngineTest::SetUp start");
        g_engine.Reset();
    }
    void TearDown() override
    {
        HILOGI("ReconnectEngineTest::TearDown start");
        g_engine.Reset();
    }
};

/**
 * @tc.name: Reconnect_engine_ReconnectReason_Distinct_001
 * @tc.desc: ReconnectReason 枚举值互不相同
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_ReconnectReason_Distinct_001, TestSize.Level0)
{
    HILOGI("Reconnect_engine_ReconnectReason_Distinct_001 enter");
    EXPECT_NE(static_cast<int>(ReconnectReason::BT_ON_AUTO_CONN),
              static_cast<int>(ReconnectReason::ACL_CONNECTION_TIMEOUT));
    EXPECT_NE(static_cast<int>(ReconnectReason::ACL_CONNECTION_TIMEOUT),
              static_cast<int>(ReconnectReason::SMART_CALL_TRANSFER_IN));
    EXPECT_NE(static_cast<int>(ReconnectReason::BT_ON_AUTO_CONN),
              static_cast<int>(ReconnectReason::SMART_CALL_TRANSFER_IN));
    EXPECT_NE(static_cast<int>(ReconnectReason::CAR_POWER_ACTIVE),
              static_cast<int>(ReconnectReason::BT_ON_AUTO_CONN));
    EXPECT_NE(static_cast<int>(ReconnectReason::CAR_POWER_ACTIVE),
              static_cast<int>(ReconnectReason::ACL_CONNECTION_TIMEOUT));
    HILOGI("Reconnect_engine_ReconnectReason_Distinct_001 end");
}

/**
 * @tc.name: Reconnect_engine_GetBackoffDelay_Tier1_002
 * @tc.desc: 退避档位一：重试次数 < Tier2 阈值（含 0 与负数边界）返回 Tier1 时延
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_GetBackoffDelay_Tier1_002, TestSize.Level1)
{
    HILOGI("Reconnect_engine_GetBackoffDelay_Tier1_002 enter");
    EXPECT_EQ(g_engine.GetBackoffDelay(0), g_config.GetBackoffTier1Ms());
    EXPECT_EQ(g_engine.GetBackoffDelay(1), g_config.GetBackoffTier1Ms());
    EXPECT_EQ(g_engine.GetBackoffDelay(g_config.GetBackoffTier2Threshold() - 1),
              g_config.GetBackoffTier1Ms());
    EXPECT_EQ(g_engine.GetBackoffDelay(-1), g_config.GetBackoffTier1Ms());
    HILOGI("Reconnect_engine_GetBackoffDelay_Tier1_002 end");
}

/**
 * @tc.name: Reconnect_engine_GetBackoffDelay_Tier2_003
 * @tc.desc: 退避档位二：重试次数达到 Tier2 阈值且未达 Tier3 阈值返回 Tier2 时延
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_GetBackoffDelay_Tier2_003, TestSize.Level1)
{
    HILOGI("Reconnect_engine_GetBackoffDelay_Tier2_003 enter");
    EXPECT_EQ(g_engine.GetBackoffDelay(g_config.GetBackoffTier2Threshold()),
              g_config.GetBackoffTier2Ms());
    EXPECT_EQ(g_engine.GetBackoffDelay(g_config.GetBackoffTier3Threshold() - 1),
              g_config.GetBackoffTier2Ms());
    HILOGI("Reconnect_engine_GetBackoffDelay_Tier2_003 end");
}

/**
 * @tc.name: Reconnect_engine_GetBackoffDelay_Tier3_004
 * @tc.desc: 退避档位三：重试次数达到 Tier3 阈值返回 Tier3 时延
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_GetBackoffDelay_Tier3_004, TestSize.Level1)
{
    HILOGI("Reconnect_engine_GetBackoffDelay_Tier3_004 enter");
    EXPECT_EQ(g_engine.GetBackoffDelay(g_config.GetBackoffTier3Threshold()),
              g_config.GetBackoffTier3Ms());
    EXPECT_EQ(g_engine.GetBackoffDelay(g_config.GetMaxRetry()), g_config.GetBackoffTier3Ms());
    HILOGI("Reconnect_engine_GetBackoffDelay_Tier3_004 end");
}

/**
 * @tc.name: Reconnect_engine_CheckIfAutoConnect_NoAdapter_005
 * @tc.desc: 单测环境无 ClassicAdapter，软跳过检查返回 false（本轮不连）
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_CheckIfAutoConnect_NoAdapter_005, TestSize.Level1)
{
    HILOGI("Reconnect_engine_CheckIfAutoConnect_NoAdapter_005 enter");
    EXPECT_FALSE(g_engine.CheckIfAutoConnect());
    HILOGI("Reconnect_engine_CheckIfAutoConnect_NoAdapter_005 end");
}

/**
 * @tc.name: Reconnect_engine_StartReconnect_SoftSkip_006
 * @tc.desc: 首轮软跳过：不发起连接但重试计数 +1，引擎保持活跃，设备与原因记录正确
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_StartReconnect_SoftSkip_006, TestSize.Level1)
{
    HILOGI("Reconnect_engine_StartReconnect_SoftSkip_006 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::BT_ON_AUTO_CONN);
    EXPECT_TRUE(g_engine.IsActive());
    EXPECT_EQ(g_engine.GetRetryTimes(), 1);
    EXPECT_EQ(g_engine.GetCurrentDevice(), DEV_A);
    EXPECT_TRUE(g_engine.GetCurrentReason() == ReconnectReason::BT_ON_AUTO_CONN);
    HILOGI("Reconnect_engine_StartReconnect_SoftSkip_006 end");
}

/**
 * @tc.name: Reconnect_engine_SoftSkip_MaxRetry_Terminates_007
 * @tc.desc: 连续软跳过达到最大重试次数后引擎终止（软跳过照常计数）
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_SoftSkip_MaxRetry_Terminates_007, TestSize.Level1)
{
    HILOGI("Reconnect_engine_SoftSkip_MaxRetry_Terminates_007 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::BT_ON_AUTO_CONN);
    EXPECT_TRUE(g_engine.IsActive());
    while (g_engine.GetRetryTimes() < g_config.GetMaxRetry()) {
        g_engine.HandleReconnect();
    }
    EXPECT_EQ(g_engine.GetRetryTimes(), g_config.GetMaxRetry());
    EXPECT_FALSE(g_engine.IsActive());
    HILOGI("Reconnect_engine_SoftSkip_MaxRetry_Terminates_007 end");
}

/**
 * @tc.name: Reconnect_engine_Terminate_Deactivates_008
 * @tc.desc: Terminate 后引擎不再活跃
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_Terminate_Deactivates_008, TestSize.Level1)
{
    HILOGI("Reconnect_engine_Terminate_Deactivates_008 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::BT_ON_AUTO_CONN);
    EXPECT_TRUE(g_engine.IsActive());
    g_engine.Terminate();
    EXPECT_FALSE(g_engine.IsActive());
    HILOGI("Reconnect_engine_Terminate_Deactivates_008 end");
}

/**
 * @tc.name: Reconnect_engine_Reset_ClearsState_009
 * @tc.desc: Reset 清零重试计数并终止引擎
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_Reset_ClearsState_009, TestSize.Level1)
{
    HILOGI("Reconnect_engine_Reset_ClearsState_009 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::BT_ON_AUTO_CONN);
    g_engine.Reset();
    EXPECT_FALSE(g_engine.IsActive());
    EXPECT_EQ(g_engine.GetRetryTimes(), 0);
    HILOGI("Reconnect_engine_Reset_ClearsState_009 end");
}

/**
 * @tc.name: Reconnect_engine_OnAclDisconnect_NoCrash_010
 * @tc.desc: ACL 断开入口在单测环境（无 Adapter/DataShare）不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_OnAclDisconnect_NoCrash_010, TestSize.Level0)
{
    HILOGI("Reconnect_engine_OnAclDisconnect_NoCrash_010 enter");
    g_engine.OnAclDisconnect(DEV_A, 8);
    g_engine.OnAllProfilesDisconnected(DEV_A);
    SUCCEED();
    HILOGI("Reconnect_engine_OnAclDisconnect_NoCrash_010 end");
}

/**
 * @tc.name: Reconnect_engine_CarPowerActive_SoftSkip_011
 * @tc.desc: CAR_POWER_ACTIVE 会话行为对齐 BT_ON：首轮软跳过计数 +1、引擎活跃、reason 记录正确
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_CarPowerActive_SoftSkip_011, TestSize.Level1)
{
    HILOGI("Reconnect_engine_CarPowerActive_SoftSkip_011 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::CAR_POWER_ACTIVE);
    EXPECT_TRUE(g_engine.IsActive());
    EXPECT_EQ(g_engine.GetRetryTimes(), 1);
    EXPECT_EQ(g_engine.GetCurrentDevice(), DEV_A);
    EXPECT_TRUE(g_engine.GetCurrentReason() == ReconnectReason::CAR_POWER_ACTIVE);
    HILOGI("Reconnect_engine_CarPowerActive_SoftSkip_011 end");
}

/**
 * @tc.name: Reconnect_engine_CarPowerActive_Terminate_012
 * @tc.desc: CAR_POWER_ACTIVE 会话可被 Terminate 终止（STANDBY 到达时的停回连路径）
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_CarPowerActive_Terminate_012, TestSize.Level1)
{
    HILOGI("Reconnect_engine_CarPowerActive_Terminate_012 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::CAR_POWER_ACTIVE);
    EXPECT_TRUE(g_engine.IsActive());
    g_engine.Terminate();
    EXPECT_FALSE(g_engine.IsActive());
    HILOGI("Reconnect_engine_CarPowerActive_Terminate_012 end");
}

/**
 * @tc.name: Reconnect_engine_SubProfileSwitchReason_Distinct_013
 * @tc.desc: SUB_PROFILE_SWITCH_AUTO_CONN 枚举值与其他 reason 互不相同（切号回连会话标识唯一）
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_SubProfileSwitchReason_Distinct_013, TestSize.Level0)
{
    HILOGI("Reconnect_engine_SubProfileSwitchReason_Distinct_013 enter");
    EXPECT_NE(static_cast<int>(ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN),
              static_cast<int>(ReconnectReason::BT_ON_AUTO_CONN));
    EXPECT_NE(static_cast<int>(ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN),
              static_cast<int>(ReconnectReason::ACL_CONNECTION_TIMEOUT));
    EXPECT_NE(static_cast<int>(ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN),
              static_cast<int>(ReconnectReason::SMART_CALL_TRANSFER_IN));
    EXPECT_NE(static_cast<int>(ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN),
              static_cast<int>(ReconnectReason::CAR_POWER_ACTIVE));
    HILOGI("Reconnect_engine_SubProfileSwitchReason_Distinct_013 end");
}

/**
 * @tc.name: Reconnect_engine_SubProfileSwitch_SoftSkip_014
 * @tc.desc: 子空间切换自动回连行为对齐 BT_ON（IsAutoConnReason）：首轮软跳过计数 +1、
 *           引擎保持活跃、设备与原因记录正确
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_SubProfileSwitch_SoftSkip_014, TestSize.Level1)
{
    HILOGI("Reconnect_engine_SubProfileSwitch_SoftSkip_014 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN);
    EXPECT_TRUE(g_engine.IsActive());
    EXPECT_EQ(g_engine.GetRetryTimes(), 1);
    EXPECT_EQ(g_engine.GetCurrentDevice(), DEV_A);
    EXPECT_TRUE(g_engine.GetCurrentReason() == ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN);
    HILOGI("Reconnect_engine_SubProfileSwitch_SoftSkip_014 end");
}

/**
 * @tc.name: Reconnect_engine_SubProfileSwitch_Terminate_015
 * @tc.desc: 子空间切换回连会话可被 Terminate 终止（下一次切号处理开始时停旧会话路径）
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_SubProfileSwitch_Terminate_015, TestSize.Level1)
{
    HILOGI("Reconnect_engine_SubProfileSwitch_Terminate_015 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN);
    EXPECT_TRUE(g_engine.IsActive());
    g_engine.Terminate();
    EXPECT_FALSE(g_engine.IsActive());
    HILOGI("Reconnect_engine_SubProfileSwitch_Terminate_015 end");
}

/**
 * @tc.name: Reconnect_engine_SubProfileSwitch_MaxRetry_Terminates_016
 * @tc.desc: 子空间切换场景连续软跳过达到最大重试次数后引擎终止（计数路径与 BT_ON 一致）
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectEngineTest, Reconnect_engine_SubProfileSwitch_MaxRetry_Terminates_016, TestSize.Level1)
{
    HILOGI("Reconnect_engine_SubProfileSwitch_MaxRetry_Terminates_016 enter");
    g_engine.StartReconnect(DEV_A, ReconnectReason::SUB_PROFILE_SWITCH_AUTO_CONN);
    EXPECT_TRUE(g_engine.IsActive());
    while (g_engine.GetRetryTimes() < g_config.GetMaxRetry()) {
        g_engine.HandleReconnect();
    }
    EXPECT_EQ(g_engine.GetRetryTimes(), g_config.GetMaxRetry());
    EXPECT_FALSE(g_engine.IsActive());
    HILOGI("Reconnect_engine_SubProfileSwitch_MaxRetry_Terminates_016 end");
}
}  // namespace bluetooth
}  // namespace OHOS
