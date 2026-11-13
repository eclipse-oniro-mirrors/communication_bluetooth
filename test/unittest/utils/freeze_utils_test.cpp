/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "log.h"
#include "freeze_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
class FreezeUtilsTest : public testing::Test {
public:
    FreezeUtilsTest() = default;
    ~FreezeUtilsTest() = default;

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void FreezeUtilsTest::SetUpTestCase()
{}
void FreezeUtilsTest::TearDownTestCase()
{
    FreezeUtil::GetInstance()->ffrtQueue_.reset();
    FreezeUtil::GetInstance()->ffrtQueue_ = nullptr;
}
void FreezeUtilsTest::SetUp()
{}
void FreezeUtilsTest::TearDown()
{}

/**
 * @tc.name: freeze_utils_test_001
 * @tc.desc: request active
 * @tc.type: FUNC
 */
#ifdef RES_SCHED_SUPPORT
HWTEST_F(FreezeUtilsTest, freeze_utils_test_001, TestSize.Level0)
{
    HILOGI("freeze_utils_test_001 begin");
    FreezeUtil *freezeUtil = FreezeUtil::GetInstance();
    freezeUtil->CheckPeriodical();
    freezeUtil->InitQueueTask();
    freezeUtil->CheckPeriodical();
    EXPECT_NE(freezeUtil->ffrtQueue_, nullptr);
    HILOGI("freeze_utils_test_001 end");
}
#endif

/**
 * @tc.name: freeze_utils_test_002
 * @tc.desc: request active
 * @tc.type: FUNC
 */
#ifdef RES_SCHED_SUPPORT
HWTEST_F(FreezeUtilsTest, freeze_utils_test_002, TestSize.Level0)
{
    HILOGI("freeze_utils_test_002 begin");
    FreezeUtil *freezeUtil = FreezeUtil::GetInstance();
    freezeUtil->ffrtQueue_ == nullptr;
    std::pair<int, int> appInfo;
    appInfo.first = 1;
    appInfo.second = 1;
    bool ret = freezeUtil->isReport(appInfo);
    EXPECT_NE(freezeUtil->ffrtQueue_, nullptr);
    EXPECT_TRUE(ret);
    freezeUtil->RequestActive(1, -1, "test");
    freezeUtil->CheckPeriodical();
    freezeUtil->CheckPeriodical();
    freezeUtil->RequestActive(1, -1, "test");
    freezeUtil->RequestActive(1, -1, "test");
    freezeUtil->CheckPeriodical();
    freezeUtil->RequestActive(1, -1, "test");
    freezeUtil->CheckPeriodical();
    freezeUtil->RequestActive(2, 10001, "test");
    freezeUtil->RequestActive(2, 10001, "test");
    freezeUtil->CheckPeriodical();
    freezeUtil->SetFreezeState();
    freezeUtil->RequestActive(2, 10001, "test");
    EXPECT_NE(freezeUtil->hasDataReport_.Size(), 0);
    HILOGI("freeze_utils_test_002 end");
}
#endif
}  // namespace bluetooth
}  // namespace OHOS
