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
#include "log.h"
#include "system_ability_subscriber.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
class SystemAbilitySubscriberTest : public testing::Test {
public:
    SystemAbilitySubscriberTest() = default;
    ~SystemAbilitySubscriberTest() = default;

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SystemAbilitySubscriberTest::SetUpTestCase()
{}
void SystemAbilitySubscriberTest::TearDownTestCase()
{
}
void SystemAbilitySubscriberTest::SetUp()
{}
void SystemAbilitySubscriberTest::TearDown()
{}

/**
 * @tc.name: system_ability_subscriber_test_001
 * @tc.desc: request active
 * @tc.type: FUNC
 */
HWTEST_F(SystemAbilitySubscriberTest, system_ability_subscriber_test_001, TestSize.Level0)
{
    HILOGI("system_ability_subscriber_test_001 begin");
    std::set<int> saIdSet = {RES_SCHED_SYS_ABILITY_ID, SUSPEND_MANAGER_SYSTEM_ABILITY_ID};
    auto addFunc = [this](int32_t systemAbilityId) {
        return;
    };
    auto removeFunc = [](int32_t systemAbilityId) {
        return;
    };
    auto statusChangeListener = new SystemAbilitySubscriber(saIdSet, addFunc, removeFunc);
    EXPECT_EQ(statusChangeListener->saId_.size(), 2);
    statusChangeListener->OnAddSystemAbility(1, "a");
    statusChangeListener->OnAddSystemAbility(1901, "a");
    statusChangeListener->OnRemoveSystemAbility(1, "a");
    statusChangeListener->OnRemoveSystemAbility(1901, "a");
    HILOGI("system_ability_subscriber_test_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS
