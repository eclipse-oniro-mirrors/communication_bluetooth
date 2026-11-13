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

#include <gtest/gtest.h>
#include <memory>

#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

class CarPolicyConfigTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("CarPolicyConfigTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("CarPolicyConfigTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("CarPolicyConfigTest::SetUp start");
        config_ = std::make_unique<CarPolicyConfig>();
    }
    void TearDown() override
    {
        HILOGI("CarPolicyConfigTest::TearDown start");
        config_ = nullptr;
    }

protected:
    std::unique_ptr<CarPolicyConfig> config_;
};

/**
 * @tc.name: Car_policy_config_GetMaxRetry_001
 * @tc.desc: 最大回连次数 15。Ref: 设计文档 §6.1
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyConfigTest, Car_policy_config_GetMaxRetry_001, TestSize.Level0)
{
    HILOGI("Car_policy_config_GetMaxRetry_001 enter");
    // 使用 fixture 成员 config_（SetUp 中构造）;
    EXPECT_EQ(config_->GetMaxRetry(), 15);
    HILOGI("Car_policy_config_GetMaxRetry_001 end");
}

/**
 * @tc.name: Car_policy_config_GetRetryHfpTimeoutMs_002
 * @tc.desc: softbus 冲突容错 800ms。Ref: §6.1
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyConfigTest, Car_policy_config_GetRetryHfpTimeoutMs_002, TestSize.Level0)
{
    HILOGI("Car_policy_config_GetRetryHfpTimeoutMs_002 enter");
    // 使用 fixture 成员 config_（SetUp 中构造）;
    EXPECT_EQ(config_->GetRetryHfpTimeoutMs(), 800UL);
    HILOGI("Car_policy_config_GetRetryHfpTimeoutMs_002 end");
}

/**
 * @tc.name: Car_policy_config_GetAclTimeoutRetryDelayMs_003
 * @tc.desc: ACL 超时 1s 快速重试。Ref: §6.1
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyConfigTest, Car_policy_config_GetAclTimeoutRetryDelayMs_003, TestSize.Level0)
{
    HILOGI("Car_policy_config_GetAclTimeoutRetryDelayMs_003 enter");
    // 使用 fixture 成员 config_（SetUp 中构造）;
    EXPECT_EQ(config_->GetAclTimeoutRetryDelayMs(), 1000UL);
    HILOGI("Car_policy_config_GetAclTimeoutRetryDelayMs_003 end");
}

/**
 * @tc.name: Car_policy_config_BackoffTiers_004
 * @tc.desc: 退避策略 10s/20s/30s。Ref: §6.1
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyConfigTest, Car_policy_config_BackoffTiers_004, TestSize.Level0)
{
    HILOGI("Car_policy_config_BackoffTiers_004 enter");
    // 使用 fixture 成员 config_（SetUp 中构造）;
    EXPECT_EQ(config_->GetBackoffTier1Ms(), 10000UL);
    EXPECT_EQ(config_->GetBackoffTier2Ms(), 20000UL);
    EXPECT_EQ(config_->GetBackoffTier3Ms(), 30000UL);
    HILOGI("Car_policy_config_BackoffTiers_004 end");
}

/**
 * @tc.name: Car_policy_config_BackoffThresholds_005
 * @tc.desc: 退避阈值 3/7。Ref: §6.1
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyConfigTest, Car_policy_config_BackoffThresholds_005, TestSize.Level0)
{
    HILOGI("Car_policy_config_BackoffThresholds_005 enter");
    // 使用 fixture 成员 config_（SetUp 中构造）;
    EXPECT_EQ(config_->GetBackoffTier2Threshold(), 3);
    EXPECT_EQ(config_->GetBackoffTier3Threshold(), 7);
    HILOGI("Car_policy_config_BackoffThresholds_005 end");
}

/**
 * @tc.name: Car_policy_config_SubProfileSwitchParams_006
 * @tc.desc: 账号子空间切换参数：断开后回连延时 1s（留断链窗口）、看门狗超时 5s（须大于
 *           settingsdata 子空间切换滞后实测值 ~2s，兜底触发时数据必然就绪）。Ref: §6.1
 * @tc.type: FUNC
 */
HWTEST_F(CarPolicyConfigTest, Car_policy_config_SubProfileSwitchParams_006, TestSize.Level0)
{
    HILOGI("Car_policy_config_SubProfileSwitchParams_006 enter");
    // 使用 fixture 成员 config_（SetUp 中构造）;
    EXPECT_EQ(config_->GetSubProfileSwitchReconnectDelayMs(), 1000UL);
    EXPECT_EQ(config_->GetSubProfileSwitchWatchdogTimeoutMs(), 5000UL);
    // 看门狗超时必须大于回连延时，保证兜底触发后场景 1 的延时回连可完整执行
    EXPECT_GT(config_->GetSubProfileSwitchWatchdogTimeoutMs(),
              config_->GetSubProfileSwitchReconnectDelayMs());
    HILOGI("Car_policy_config_SubProfileSwitchParams_006 end");
}
}  // namespace bluetooth
}  // namespace OHOS
