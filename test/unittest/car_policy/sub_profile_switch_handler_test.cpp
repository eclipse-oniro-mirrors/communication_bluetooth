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

#include "car_policy_config.h"
#include "car_policy_manager.h"
#include "sub_profile_switch_handler.h"

#include <gtest/gtest.h>

#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

// 账号子空间切换处理器（SubProfileSwitchHandler）专项测试：
//    覆盖切号适配修改——ID 缓存比较入口（CheckSubProfileIdAndReconnect）全分支、
//    广播/看门狗两路触发入口、切号处理（HandleSubProfileChanged）空组件 fail-safe、
//    断开抑制标记（IsSwitchDisconnecting/ConsumeSwitchDisconnect）状态机。
//    单测环境无账号/DataShare 服务：查询路径返回失败走 fail-safe 分支，
//    用例经 private=public 读写 cachedSubProfileId_/switchDisconnectingAddr_ 验证分支生效。
//    单例不调 Init：组件引用保持空，同时覆盖"未注入依赖"防御分支。
class SubProfileSwitchHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("SubProfileSwitchHandlerTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("SubProfileSwitchHandlerTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("SubProfileSwitchHandlerTest::SetUp start");
    }
    void TearDown() override
    {
        HILOGI("SubProfileSwitchHandlerTest::TearDown start");
    }
};

/**
 * @tc.name: SubProfileSwitchHandler_GetSwitchHandler_StableRef_001
 * @tc.desc: Manager 暴露的 handler 访问器多次调用返回同一引用（Handler 为 Manager 成员对象）
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_GetSwitchHandler_StableRef_001, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_GetSwitchHandler_StableRef_001 enter");
    CarPolicyManager &mgr = CarPolicyManager::GetInstance();
    EXPECT_EQ(&mgr.GetSwitchHandler(), &mgr.GetSwitchHandler());
    HILOGI("SubProfileSwitchHandler_GetSwitchHandler_StableRef_001 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_InvalidIdSentinel_002
 * @tc.desc: INVALID_SUB_PROFILE_ID 哨兵值为 -1，与任何合法 subProfileId（>=0）可区分：
 *           缓存有效性判定与"查询失败保持缓存无效"语义依赖该约定
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_InvalidIdSentinel_002, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_InvalidIdSentinel_002 enter");
    EXPECT_EQ(SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID, -1);
    HILOGI("SubProfileSwitchHandler_InvalidIdSentinel_002 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_Suppress_DefaultEmpty_003
 * @tc.desc: 初始状态无抑制标记：任意地址（含空串）均不判定为切号断开中
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_Suppress_DefaultEmpty_003, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_Suppress_DefaultEmpty_003 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    EXPECT_FALSE(handler.IsSwitchDisconnecting(""));
    EXPECT_FALSE(handler.IsSwitchDisconnecting("00:11:22:33:44:55"));
    HILOGI("SubProfileSwitchHandler_Suppress_DefaultEmpty_003 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_Suppress_MatchAndMismatch_004
 * @tc.desc: 抑制标记设置后：仅标记地址判定为切号断开中，其他地址与空串不误判
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_Suppress_MatchAndMismatch_004, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_Suppress_MatchAndMismatch_004 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.switchDisconnectingAddr_ = "00:11:22:33:44:55";
    EXPECT_TRUE(handler.IsSwitchDisconnecting("00:11:22:33:44:55"));
    EXPECT_FALSE(handler.IsSwitchDisconnecting("AA:BB:CC:DD:EE:FF"));
    EXPECT_FALSE(handler.IsSwitchDisconnecting(""));
    handler.switchDisconnectingAddr_.clear();
    HILOGI("SubProfileSwitchHandler_Suppress_MatchAndMismatch_004 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_Consume_MatchingClears_005
 * @tc.desc: 地址匹配时消费抑制标记（一次性语义）：消费后不再判定断开中，重复消费不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_Consume_MatchingClears_005, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_Consume_MatchingClears_005 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.switchDisconnectingAddr_ = "00:11:22:33:44:55";
    handler.ConsumeSwitchDisconnect("00:11:22:33:44:55");
    EXPECT_FALSE(handler.IsSwitchDisconnecting("00:11:22:33:44:55"));
    // 抑制为一次性：重复消费（广播重发/重复断开回调）安全
    handler.ConsumeSwitchDisconnect("00:11:22:33:44:55");
    EXPECT_FALSE(handler.IsSwitchDisconnecting("00:11:22:33:44:55"));
    HILOGI("SubProfileSwitchHandler_Consume_MatchingClears_005 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_Consume_MismatchKeeps_006
 * @tc.desc: 地址不匹配时消费不生效：抑制标记保留至目标地址断开完成（防止其他设备断开误清抑制）
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_Consume_MismatchKeeps_006, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_Consume_MismatchKeeps_006 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.switchDisconnectingAddr_ = "00:11:22:33:44:55";
    handler.ConsumeSwitchDisconnect("AA:BB:CC:DD:EE:FF");
    EXPECT_TRUE(handler.IsSwitchDisconnecting("00:11:22:33:44:55"));
    handler.ConsumeSwitchDisconnect("");
    EXPECT_TRUE(handler.IsSwitchDisconnecting("00:11:22:33:44:55"));
    // 目标地址断开完成：正常消费路径
    handler.ConsumeSwitchDisconnect("00:11:22:33:44:55");
    EXPECT_FALSE(handler.IsSwitchDisconnecting("00:11:22:33:44:55"));
    HILOGI("SubProfileSwitchHandler_Consume_MismatchKeeps_006 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_Uninit_ClearsAndSafe_007
 * @tc.desc: Uninit 清除抑制标记并置空组件引用：之后全部入口（含延时任务残留执行等价路径）
 *           空指针防御生效，批量调用不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_Uninit_ClearsAndSafe_007, TestSize.Level0)
{
    HILOGI("SubProfileSwitchHandler_Uninit_ClearsAndSafe_007 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.switchDisconnectingAddr_ = "00:11:22:33:44:55";
    handler.Uninit();
    EXPECT_FALSE(handler.IsSwitchDisconnecting("00:11:22:33:44:55"));
    // Uninit 后各入口空指针防御：不布防看门狗、不发起查询后处理、不崩溃
    handler.LoadCurrentSubProfileId();
    handler.OnSubProfileSwitched();
    handler.OnSubProfileSwitchWatchdog();
    handler.OnUserConfigUpdateFinished();
    handler.HandleSubProfileChanged();
    SUCCEED();
    HILOGI("SubProfileSwitchHandler_Uninit_ClearsAndSafe_007 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_LoadCurrentSubProfileId_NoService_008
 * @tc.desc: 蓝牙启动建立基线：单测环境账号服务不可用查询失败，缓存保持 INVALID
 *           （由后续 SWITCHED 事件兜底刷新，fail-safe）
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_LoadCurrentSubProfileId_NoService_008, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_LoadCurrentSubProfileId_NoService_008 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    handler.LoadCurrentSubProfileId();
    EXPECT_EQ(handler.cachedSubProfileId_, SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID);
    HILOGI("SubProfileSwitchHandler_LoadCurrentSubProfileId_NoService_008 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_CheckSubProfile_QueryFailed_Skip_009
 * @tc.desc: 账号查询失败（ret != ERR_OK）无法判断切换：忽略本次触发，缓存不变
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_CheckSubProfile_QueryFailed_Skip_009, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_CheckSubProfile_QueryFailed_Skip_009 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = 5;
    handler.CheckSubProfileIdAndReconnect("ut", 9, 100);  // ret 非 0：查询失败
    EXPECT_EQ(handler.cachedSubProfileId_, 5);
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    HILOGI("SubProfileSwitchHandler_CheckSubProfile_QueryFailed_Skip_009 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_CheckSubProfile_BaselineEstablish_010
 * @tc.desc: 缓存无效（Init 基线未建立）时首次查询成功仅建立基线，不视为切换
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_CheckSubProfile_BaselineEstablish_010, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_CheckSubProfile_BaselineEstablish_010 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    handler.CheckSubProfileIdAndReconnect("ut", 7, 0);  // ERR_OK
    EXPECT_EQ(handler.cachedSubProfileId_, 7);
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    HILOGI("SubProfileSwitchHandler_CheckSubProfile_BaselineEstablish_010 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_CheckSubProfile_IdUnchanged_Skip_011
 * @tc.desc: 查询成功但 ID 与缓存一致（重复广播/非切号配置更新）：忽略，缓存不变
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_CheckSubProfile_IdUnchanged_Skip_011, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_CheckSubProfile_IdUnchanged_Skip_011 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = 7;
    handler.CheckSubProfileIdAndReconnect("ut", 7, 0);  // ERR_OK，ID 一致
    EXPECT_EQ(handler.cachedSubProfileId_, 7);
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    HILOGI("SubProfileSwitchHandler_CheckSubProfile_IdUnchanged_Skip_011 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_CheckSubProfile_IdChanged_FailSafe_012
 * @tc.desc: ID 变化触发 HandleSubProfileChanged：单测环境账号服务不可用，
 *           内部缓存刷新查询失败保持旧值（fail-safe：刷新只在查询成功时生效），不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_CheckSubProfile_IdChanged_FailSafe_012, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_CheckSubProfile_IdChanged_FailSafe_012 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = 7;
    handler.CheckSubProfileIdAndReconnect("ut", 9, 0);  // ERR_OK，ID 变化 → 切号处理
    // 单测环境无账号服务：HandleSubProfileChanged 内部刷新查询失败，缓存保持旧值
    EXPECT_EQ(handler.cachedSubProfileId_, 7);
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    HILOGI("SubProfileSwitchHandler_CheckSubProfile_IdChanged_FailSafe_012 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_OnSubProfileSwitched_BeforeInit_EarlyReturn_013
 * @tc.desc: SWITCHED 入口在组件未注入（dispatcher/config 为空）时直接返回：
 *           不布防看门狗、不崩溃，缓存不变
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_OnSubProfileSwitched_BeforeInit_EarlyReturn_013,
    TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_OnSubProfileSwitched_BeforeInit_EarlyReturn_013 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = 5;
    handler.OnSubProfileSwitched();
    EXPECT_EQ(handler.cachedSubProfileId_, 5);
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    HILOGI("SubProfileSwitchHandler_OnSubProfileSwitched_BeforeInit_EarlyReturn_013 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_Watchdog_NoService_Skip_014
 * @tc.desc: 看门狗超时回调在单测环境（查询失败）走忽略分支：不触发切号处理，缓存不变
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_Watchdog_NoService_Skip_014, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_Watchdog_NoService_Skip_014 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = 5;
    handler.OnSubProfileSwitchWatchdog();
    EXPECT_EQ(handler.cachedSubProfileId_, 5);
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    HILOGI("SubProfileSwitchHandler_Watchdog_NoService_Skip_014 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_UserConfigUpdateFinished_NoService_015
 * @tc.desc: 广播到达处理入口在单测环境（DataShare/账号服务均不可用）批量调用不崩溃：
 *           探针查询全部失败走忽略分支，缓存不变
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_UserConfigUpdateFinished_NoService_015, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_UserConfigUpdateFinished_NoService_015 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = 5;
    // 广播重发/重复触发场景（单测环境账号查询失败，走忽略分支）
    handler.OnUserConfigUpdateFinished();
    handler.OnUserConfigUpdateFinished();
    handler.OnUserConfigUpdateFinished();
    EXPECT_EQ(handler.cachedSubProfileId_, 5);
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    HILOGI("SubProfileSwitchHandler_UserConfigUpdateFinished_NoService_015 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_HandleSubProfileChanged_NoInit_Scene2_016
 * @tc.desc: 切号处理入口（未 Init 组件全空 + 单测环境查询失败）：
 *           缓存刷新失败保持旧值、无回连目标走场景2仅返回，不崩溃；
 *           guard 为空时当前已连设备查询返回空串
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_HandleSubProfileChanged_NoInit_Scene2_016,
    TestSize.Level0)
{
    HILOGI("SubProfileSwitchHandler_HandleSubProfileChanged_NoInit_Scene2_016 enter");
    SubProfileSwitchHandler &handler = CarPolicyManager::GetInstance().GetSwitchHandler();
    handler.cachedSubProfileId_ = 7;
    EXPECT_EQ(handler.GetCurrentConnectedDevice(), "");
    handler.HandleSubProfileChanged();
    // 查询失败：缓存保持旧值；组件为空不发起断开/回连
    EXPECT_EQ(handler.cachedSubProfileId_, 7);
    handler.cachedSubProfileId_ = SubProfileSwitchHandler::INVALID_SUB_PROFILE_ID;
    HILOGI("SubProfileSwitchHandler_HandleSubProfileChanged_NoInit_Scene2_016 end");
}

/**
 * @tc.name: SubProfileSwitchHandler_ConfigDefaults_017
 * @tc.desc: 切号相关配置默认值：回连延时 1000ms（断链释放窗口）、看门狗超时 5000ms
 *           （SWITCHED 后等待数据同步完成广播的超时），与真机验证时序一致
 * @tc.type: FUNC
 */
HWTEST_F(SubProfileSwitchHandlerTest, SubProfileSwitchHandler_ConfigDefaults_017, TestSize.Level1)
{
    HILOGI("SubProfileSwitchHandler_ConfigDefaults_017 enter");
    CarPolicyConfig config;
    EXPECT_EQ(config.GetSubProfileSwitchReconnectDelayMs(), 1000UL);
    EXPECT_EQ(config.GetSubProfileSwitchWatchdogTimeoutMs(), 5000UL);
    HILOGI("SubProfileSwitchHandler_ConfigDefaults_017 end");
}
}  // namespace bluetooth
}  // namespace OHOS
