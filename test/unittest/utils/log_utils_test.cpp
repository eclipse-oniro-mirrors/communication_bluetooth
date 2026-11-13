/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include <thread>
#include "btcommon/timer_manager.h"

#include "log_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
class LogUtilsTest : public testing::Test {
public:
    LogUtilsTest() = default;
    ~LogUtilsTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void LogUtilsTest::SetUpTestSuite(void)
{}
void LogUtilsTest::TearDownTestSuite(void)
{
    utility::CloseAllTimer();
}
void LogUtilsTest::SetUp()
{}
void LogUtilsTest::TearDown()
{}

/**
 * @tc.name: log_utils_test_001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(LogUtilsTest, log_utils_test_001, TestSize.Level0)
{
    HILOGI("log_utils_test_001 begin");

    LogUtils *logUtil = LogUtils::GetInstance();
    logUtil->timerTimeoutMs_ = 10;  // 10ms
    EXPECT_TRUE(logUtil->IsEnableLog("log", "testfile", 1));
    EXPECT_FALSE(logUtil->IsEnableLog("log", "testfile", 1));
    EXPECT_TRUE(logUtil->timer_->IsStarted());

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(logUtil->timer_->IsStarted());
    EXPECT_TRUE(logUtil->IsEnableLog("log", "testfile", 1));

    HILOGI("log_utils_test_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS
