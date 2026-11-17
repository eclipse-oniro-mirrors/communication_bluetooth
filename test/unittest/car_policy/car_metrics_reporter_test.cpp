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

#include "car_metrics_reporter.h"

#include <gtest/gtest.h>
#include <memory>

#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

// GetCurrentTimeMs() 格式："YYYY-MM-DD HH:MM:SS.mmm"
static const char *TS_1 = "2026-09-07 10:00:00.000";
static const char *TS_2 = "2026-09-07 10:00:01.500";
static const char *TS_3 = "2026-09-07 10:00:03.000";
static const char *TS_4 = "2026-09-07 10:00:05.200";
static const char *TS_5 = "2026-09-07 10:00:10.123";

// profileType: 0=HFP, 1=A2DP_SINK；reconnect: 1=车机策略回连, 0=其他

// 仅验证各上报接口在正常/缺失参数下不抛异常不崩溃。
class CarMetricsReporterTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("CarMetricsReporterTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("CarMetricsReporterTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("CarMetricsReporterTest::SetUp start");
        reporter_ = std::make_unique<CarMetricsReporter>();
    }
    void TearDown() override
    {
        HILOGI("CarMetricsReporterTest::TearDown start");
        reporter_ = nullptr;
    }

protected:
    std::unique_ptr<CarMetricsReporter> reporter_;
};

/**
 * @tc.name: Car_metrics_reporter_ReportCallRecord_Normal_001
 * @tc.desc: 通话记录上报，正常参数不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportCallRecord_Normal_001, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportCallRecord_Normal_001 enter");
    reporter_->ReportCallRecord(TS_1, TS_2, TS_3, "[1:3, 0:8, 1:15]");
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportCallRecord_Normal_001 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportCallRecord_MissingIncoming_002
 * @tc.desc: 通话记录上报，incoming 缺失不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportCallRecord_MissingIncoming_002, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportCallRecord_MissingIncoming_002 enter");
    reporter_->ReportCallRecord("", TS_1, TS_2, "[0:5]");
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportCallRecord_MissingIncoming_002 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportCallRecord_EmptySco_003
 * @tc.desc: 通话记录上报，SCO 列表为空不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportCallRecord_EmptySco_003, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportCallRecord_EmptySco_003 enter");
    reporter_->ReportCallRecord(TS_1, TS_2, TS_3, "[]");
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportCallRecord_EmptySco_003 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportPbapSync_Normal_004
 * @tc.desc: PBAP 同步上报，正常参数不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportPbapSync_Normal_004, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportPbapSync_Normal_004 enter");
    reporter_->ReportPbapSync(TS_1, TS_2, TS_3, TS_4, TS_5);
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportPbapSync_Normal_004 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportPbapSync_PartialMissing_005
 * @tc.desc: PBAP 同步上报，connected/syncStart/syncEnd 缺失（设备连上后断开但未开始同步）不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportPbapSync_PartialMissing_005, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportPbapSync_PartialMissing_005 enter");
    reporter_->ReportPbapSync(TS_1, "", "", "", TS_2);
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportPbapSync_PartialMissing_005 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportPbapSync_AllEmpty_006
 * @tc.desc: PBAP 同步上报，全空参数不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportPbapSync_AllEmpty_006, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportPbapSync_AllEmpty_006 enter");
    reporter_->ReportPbapSync("", "", "", "", "");
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportPbapSync_AllEmpty_006 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportProfileLinkRecord_HfpReconnect_007
 * @tc.desc: profile 链路记录上报，HFP 车机策略回连不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportProfileLinkRecord_HfpReconnect_007, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportProfileLinkRecord_HfpReconnect_007 enter");
    reporter_->ReportProfileLinkRecord(0, 1, TS_1, TS_2, TS_3);
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportProfileLinkRecord_HfpReconnect_007 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportProfileLinkRecord_A2dpNoReconnect_008
 * @tc.desc: profile 链路记录上报，A2DP 非策略回连不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportProfileLinkRecord_A2dpNoReconnect_008, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportProfileLinkRecord_A2dpNoReconnect_008 enter");
    reporter_->ReportProfileLinkRecord(1, 0, TS_1, TS_2, "");
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportProfileLinkRecord_A2dpNoReconnect_008 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportProfileLinkRecord_NoConnected_009
 * @tc.desc: profile 链路记录上报，连接未成功就断开（connected 为空）不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportProfileLinkRecord_NoConnected_009, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportProfileLinkRecord_NoConnected_009 enter");
    reporter_->ReportProfileLinkRecord(0, 1, TS_1, "", TS_2);
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportProfileLinkRecord_NoConnected_009 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportScanDelay_Normal_010
 * @tc.desc: 扫描时延上报，正常参数不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportScanDelay_Normal_010, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportScanDelay_Normal_010 enter");
    reporter_->ReportScanDelay(TS_1, TS_2, TS_3);
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportScanDelay_Normal_010 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportScanDelay_FoundNotFinished_011
 * @tc.desc: 扫描时延上报，扫描发现设备但未结束不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportScanDelay_FoundNotFinished_011, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportScanDelay_FoundNotFinished_011 enter");
    reporter_->ReportScanDelay(TS_1, TS_2, "");
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportScanDelay_FoundNotFinished_011 end");
}

/**
 * @tc.name: Car_metrics_reporter_ReportScanDelay_AllEmpty_012
 * @tc.desc: 扫描时延上报，全空参数不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarMetricsReporterTest, Car_metrics_reporter_ReportScanDelay_AllEmpty_012, TestSize.Level0)
{
    HILOGI("Car_metrics_reporter_ReportScanDelay_AllEmpty_012 enter");
    reporter_->ReportScanDelay("", "", "");
    SUCCEED();
    HILOGI("Car_metrics_reporter_ReportScanDelay_AllEmpty_012 end");
}
}  // namespace bluetooth
}  // namespace OHOS
