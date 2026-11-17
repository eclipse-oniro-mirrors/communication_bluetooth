/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bt_chr_app_behavior_reporter.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class BtChrAppBehaviorReporterTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase();
    void SetUp() {}
    void TearDown() {}
};

void BtChrAppBehaviorReporterTest::TearDownTestCase()
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    if (reporter.reportTimer_ != nullptr) {
        reporter.reportTimer_->Stop();
        reporter.reportTimer_ = nullptr;
    }
}

HWTEST_F(BtChrAppBehaviorReporterTest, IsSameAppBehavior_Equal_ReturnsTrue, TestSize.Level1)
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    reporter.CheckPeriodical();
    reporter.CheckPeriodical();
    AppBehaviorDataInternal lhs = {"test_app", 1000, 2000, 1, 1};
    AppBehaviorData rhs = {"test_app", 1000, 2000, 1};
    bool result = IsSameAppBehavior(lhs, rhs);
    EXPECT_TRUE(result);
}

HWTEST_F(BtChrAppBehaviorReporterTest, IsSameAppBehavior_DiffAppName_ReturnsFalse, TestSize.Level1)
{
    AppBehaviorDataInternal lhs = {"test_app1", 1000, 2000, 1, 1};
    AppBehaviorData rhs = {"test_app2", 1000, 2000, 1};
    bool result = IsSameAppBehavior(lhs, rhs);
    EXPECT_FALSE(result);
}

HWTEST_F(BtChrAppBehaviorReporterTest, IsSameAppBehavior_DiffAppUid_ReturnsFalse, TestSize.Level1)
{
    AppBehaviorDataInternal lhs = {"test_app", 1000, 2000, 1, 1};
    AppBehaviorData rhs = {"test_app", 1001, 2000, 1};
    bool result = IsSameAppBehavior(lhs, rhs);
    EXPECT_FALSE(result);
}

HWTEST_F(BtChrAppBehaviorReporterTest, IsSameAppBehavior_DiffAppPid_ReturnsFalse, TestSize.Level1)
{
    AppBehaviorDataInternal lhs = {"test_app", 1000, 2000, 1, 1};
    AppBehaviorData rhs = {"test_app", 1000, 2001, 1};
    bool result = IsSameAppBehavior(lhs, rhs);
    EXPECT_FALSE(result);
}

HWTEST_F(BtChrAppBehaviorReporterTest, IsSameAppBehavior_DiffBusinessType_ReturnsFalse, TestSize.Level1)
{
    AppBehaviorDataInternal lhs = {"test_app", 1000, 2000, 1, 1};
    AppBehaviorData rhs = {"test_app", 1000, 2000, 2};
    bool result = IsSameAppBehavior(lhs, rhs);
    EXPECT_FALSE(result);
}

HWTEST_F(BtChrAppBehaviorReporterTest, GetInstance_ReturnsSingleton, TestSize.Level1)
{
    auto& instance1 = BtChrAppBehaviorReporter::GetInstance();
    auto& instance2 = BtChrAppBehaviorReporter::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

HWTEST_F(BtChrAppBehaviorReporterTest, ReportAppBehavior_NewData_AddsToQueue, TestSize.Level1)
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    AppBehaviorData data = {"test_app", 1000, 2000, 1};
    bool result = reporter.ReportAppBehavior(data);
    EXPECT_TRUE(result);
}

HWTEST_F(BtChrAppBehaviorReporterTest, ReportAppBehavior_SameData_IncrementsTriggerCount, TestSize.Level1)
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    AppBehaviorData data = {"test_app_incre", 1000, 2000, 1};
    reporter.ReportAppBehavior(data);
    bool result = reporter.ReportAppBehavior(data);
    EXPECT_TRUE(result);
}

HWTEST_F(BtChrAppBehaviorReporterTest, ReportAppBehavior_DifferentData_AddsNewEntry, TestSize.Level1)
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    AppBehaviorData data1 = {"test_app1", 1000, 2000, 1};
    AppBehaviorData data2 = {"test_app2", 1001, 2001, 2};
    reporter.ReportAppBehavior(data1);
    bool result = reporter.ReportAppBehavior(data2);
    reporter.CheckPeriodical();
    AppBehaviorData data = {"test_app_init", 1000, 2000, 1};
    reporter.ReportAppBehavior(data);
    reporter.InitScreenAndChargingStatus();
    reporter.RegisterCommonEvent();
    reporter.UnRegisterCommonEvent();
    reporter.HandleScreenStateChange(true);
    reporter.HandleScreenStateChange(false);
    reporter.HandleChargingStateChange(true);
    reporter.HandleChargingStateChange(false);
    EXPECT_TRUE(result);
}

HWTEST_F(BtChrAppBehaviorReporterTest, ReportAppBehavior_OnReceiveEvent, TestSize.Level1)
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    std::unordered_map<std::string, int> actionCode = {
        {EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON, -1},
        {EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF, -1},
        {EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING, 0},
        {EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING, 0},
    };
    reporter.CheckPeriodical();
    reporter.RegisterCommonEvent();
    for (const auto &iter : actionCode) {
        AAFwk::Want want;
        want.SetAction(iter.first);
        EventFwk::CommonEventData commonData;
        commonData.SetWant(want);
        if (iter.second != -1) {
            commonData.SetCode(iter.second);
        }
        reporter.subscriber_->OnReceiveEvent(commonData);
    }
    reporter.HandleScreenStateChange(true);
    AppBehaviorData data = {"test_app", 1000, 2000, 1};
    EXPECT_FALSE(reporter.ReportAppBehavior(data));
    reporter.HandleChargingStateChange(true);
    EXPECT_FALSE(reporter.ReportAppBehavior(data));
    reporter.HandleScreenStateChange(false);
    EXPECT_FALSE(reporter.ReportAppBehavior(data));
}

HWTEST_F(BtChrAppBehaviorReporterTest, CheckPeriodical_WritesQueueToUeManager, TestSize.Level1)
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    reporter.HandleScreenStateChange(false);
    reporter.HandleChargingStateChange(false);

    AppBehaviorData data = {"wakeup_app", 3000, 4000, 0};
    reporter.ReportAppBehavior(data);
    reporter.CheckPeriodical();
    SUCCEED();
}

HWTEST_F(BtChrAppBehaviorReporterTest, CheckPeriodical_WritesAggregatedDataToUeManager, TestSize.Level1)
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    reporter.HandleScreenStateChange(false);
    reporter.HandleChargingStateChange(false);

    AppBehaviorData data1 = {"agg_app1", 5000, 6000, 0};
    AppBehaviorData data2 = {"agg_app2", 5001, 6001, 1};
    reporter.ReportAppBehavior(data1);
    reporter.ReportAppBehavior(data1);
    reporter.ReportAppBehavior(data2);
    reporter.CheckPeriodical();
    SUCCEED();
}

HWTEST_F(BtChrAppBehaviorReporterTest, CheckPeriodical_ClearsQueueAfterWrite, TestSize.Level1)
{
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    reporter.HandleScreenStateChange(false);
    reporter.HandleChargingStateChange(false);

    AppBehaviorData data = {"clear_app", 7000, 8000, 0};
    reporter.ReportAppBehavior(data);
    reporter.CheckPeriodical();
    reporter.CheckPeriodical();
    SUCCEED();
}
} // namespace bluetooth
} // namespace OHOS
