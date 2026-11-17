/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bt_chr_dft_statictics.h"
#include "btcommon/timer_manager.h"
#include "bt_chr_transaction_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
static BluetoothHost *host_;

class BtChrDftStaticticsTest : public testing::Test {
public:
    BtChrDftStaticticsTest() = default;
    ~BtChrDftStaticticsTest() = default;

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

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Test_WriteCommonStatictics_001
 * @tc.desc: Verify the WriteCommonStatictics function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftStaticticsTest, Test_WriteCommonStatictics_001, TestSize.Level1)
{
    HILOGI("Test_WriteCommonStatictics_001 enter");
    BtChrDftStatictics::GetInstance()->WriteAutoConnectStatictics(0);
    BtChrDftStatictics::GetInstance()->WriteAutoConnectStatictics(1);
    std::vector<BtChrEventParam> params;
    BtChrDftStatictics::GetInstance()->WriteCommonStatictics(EVENT_TYPE_BITRATE_STATISTICS, params);
    params.push_back(BtChrEventParam("SBC_LEVEL0_DURATION", 6));
    BtChrDftStatictics::GetInstance()->WriteCommonStatictics(EVENT_TYPE_BITRATE_STATISTICS, params);
    BtChrDftStatictics::GetInstance()->WriteDurationStatictics(DURATION_STAT_TYPE_CALL, false);
    BtChrDftStatictics::GetInstance()->WriteDurationStatictics(DURATION_STAT_TYPE_CALL, true);
    sleep(TIME);
    BtChrDftStatictics::GetInstance()->WriteDurationStatictics(DURATION_STAT_TYPE_CALL, false);
    BtChrDftStatictics::GetInstance()->WriteDurationStatictics(DURATION_STAT_TYPE_A2DP, true);
    sleep(60);
    BtChrDftStatictics::GetInstance()->WriteDurationStatictics(DURATION_STAT_TYPE_A2DP, false);
    EXPECT_FALSE(BtChrDftStatictics::GetInstance()->GetEventValueJsonStr(params).empty());
    HILOGI("Test_WriteCommonStatictics_001 end");
}

/**
 * @tc.name: Test_WriteTransactionStatictics_001
 * @tc.desc: Verify the WriteTransactionStatictics function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftStaticticsTest, Test_WriteTransactionStatictics_001, TestSize.Level1)
{
    HILOGI("Test_WriteTransactionStatictics_001 enter");
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_NA, TRANSACTION_RESULT_NA,
        1, TRANSACTION_SCENECODE_NA, 1);
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
        TRANSACTION_RESULT_NA, 1, TRANSACTION_SCENECODE_NA, 1);
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
        TRANSACTION_RESULT_TOTAL, 1, TRANSACTION_SCENECODE_NA, 1);
    std::string result = BtChrTransactionManager::GetInstance().GetCounterGroupJsonStr();
    EXPECT_TRUE(result.find("{\"TOTAL_CNT\":1,\"TRANSACTION_TYPE\":\"OPP_SEND\"}") != std::string::npos);
    HILOGI("Test_WriteTransactionStatictics_001 end");
}

/**
 * @tc.name: Test_WriteTransactionStatictics_002
 * @tc.desc: Verify the WriteTransactionStatictics function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftStaticticsTest, Test_WriteTransactionStatictics_002, TestSize.Level1)
{
    HILOGI("Test_WriteTransactionStatictics_002 enter");
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
        TRANSACTION_RESULT_FAIL, 1, TRANSACTION_SCENECODE_1, 1);
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
        TRANSACTION_RESULT_FAIL, 1, TRANSACTION_SCENECODE_3, 2);
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_RECEIVE,
        TRANSACTION_RESULT_FAIL, 1, TRANSACTION_SCENECODE_1, 1);
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_RECEIVE,
        TRANSACTION_RESULT_SUCCESS, 3, TRANSACTION_SCENECODE_NA, 1);
    std::string result = BtChrTransactionManager::GetInstance().GetCounterGroupJsonStr();
    EXPECT_TRUE(result.find("{\"1\":1,\"3\":2,\"FAIL_CNT\":2,\"TOTAL_CNT\":1,\"TRANSACTION_TYPE\":\"OPP_SEND\"}") !=
        std::string::npos);
    EXPECT_TRUE(result.find("{\"1\":1,\"FAIL_CNT\":1,\"SUCCESS_CNT\":3,\"TRANSACTION_TYPE\":\"OPP_RECEIVE\"}") !=
        std::string::npos);
    HILOGI("Test_WriteTransactionStatictics_002 end");
}

/**
 * @tc.name: Test_WriteTransactionStatictics_003
 * @tc.desc: Verify the WriteTransactionStatictics function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftStaticticsTest, Test_WriteTransactionStatictics_003, TestSize.Level1)
{
    HILOGI("Test_WriteTransactionStatictics_003 enter");
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
        TRANSACTION_RESULT_TOTAL, 1, TRANSACTION_SCENECODE_NA, 1);
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
        TRANSACTION_RESULT_TOTAL, 301, TRANSACTION_SCENECODE_NA, 301);
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
        TRANSACTION_RESULT_TOTAL, 1, TRANSACTION_SCENECODE_NA, 301);
    BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
        TRANSACTION_RESULT_TOTAL, 301, TRANSACTION_SCENECODE_NA, 1);
    std::string beginResult = BtChrTransactionManager::GetInstance().GetCounterGroupJsonStr();
    EXPECT_TRUE(beginResult.find("TRANSACTION_TYPE\":\"OPP_SEND") != std::string::npos);
    BtChrTransactionManager::GetInstance().ReportTransaction();
    std::string reportResult = BtChrTransactionManager::GetInstance().GetCounterGroupJsonStr();
    EXPECT_TRUE(reportResult.find("TRANSACTION_TYPE\":\"OPP_SEND") == std::string::npos);
    HILOGI("Test_WriteTransactionStatictics_003 end");
}


/**
 * @tc.name: Test_BleAdvReachingMaxStatisticsEvent_001
 * @tc.desc: Verify the BleAdvReachingMaxStatisticsEvent and SetBleAdvReachingMaxRecord function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftStaticticsTest, Test_BleAdvReachingMaxStatisticsEvent_001, TestSize.Level1)
{
    HILOGI("Test_BleAdvReachingMaxStatisticsEvent_001 enter");
    std::string packageName = "testPackageName";
    int32_t advNum = 6;
    uint32_t maxHoldingTime = 50;
    BtChrDftStatictics::GetInstance()->SetBleAdvReachingMaxRecord(packageName, advNum, maxHoldingTime);
    BtChrDftStatictics::GetInstance()->BleAdvReachingMaxStatisticsEvent();

    packageName = "testPackageName";
    advNum = 7;
    maxHoldingTime = 60;
    BtChrDftStatictics::GetInstance()->SetBleAdvReachingMaxRecord(packageName, advNum, maxHoldingTime);
    BtChrDftStatictics::GetInstance()->BleAdvReachingMaxStatisticsEvent();

    packageName = "testPackageName2";
    advNum = 6;
    maxHoldingTime = 50;
    BtChrDftStatictics::GetInstance()->SetBleAdvReachingMaxRecord(packageName, advNum, maxHoldingTime);
    BtChrDftStatictics::GetInstance()->BleAdvReachingMaxStatisticsEvent();
    
    EXPECT_FALSE(BtChrDftStatictics::GetInstance()->GetBleAdvReachingMaxRecordsJsonStr().empty());
    HILOGI("Test_BleAdvReachingMaxStatisticsEvent_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS