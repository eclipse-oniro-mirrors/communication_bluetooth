/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <future>

#include "semaphore_utils.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
const int TIME = 2;
static BluetoothHost *host_;
static constexpr uint32_t WAIT_TIMEOUT = 10;  // 10ms
class SemaphoreTest : public testing::Test {
public:
    SemaphoreTest() = default;
    ~SemaphoreTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void SemaphoreTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    HILOGI("isBleEnable = %{public}d", isBleEnable);

    if (!isBleEnable) {
        host_->EnableBle();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}

void SemaphoreTest::TearDownTestSuite(void)
{
    host_ = nullptr;
}

void SemaphoreTest::SetUp()
{}

void SemaphoreTest::TearDown()
{}

/**
 * @tc.name: wait_for_ms_001
 * @tc.desc: Verify the WaitForMs function.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, wait_for_ms_001, TestSize.Level0)
{
    auto sem = std::make_shared<utility::Semaphore>(0);
    std::async(std::launch::async, [sem]() {
        sem->Post();
    });
    bool ret = sem->WaitForMs(WAIT_TIMEOUT);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: wait_for_ms_002
 * @tc.desc: Verify the WaitForMs function.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, wait_for_ms_002, TestSize.Level0)
{
    auto sem = std::make_shared<utility::Semaphore>(0);
    bool ret = sem->WaitForMs(WAIT_TIMEOUT);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: wait_for_ms_003
 * @tc.desc: Verify the WaitForMs function.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, wait_for_ms_003, TestSize.Level0)
{
    auto sem = std::make_shared<utility::Semaphore>(1);
    bool ret = sem->WaitForMs(WAIT_TIMEOUT);
    EXPECT_TRUE(ret);
}

}  // namespace bluetooth
}  // namespace OHOS