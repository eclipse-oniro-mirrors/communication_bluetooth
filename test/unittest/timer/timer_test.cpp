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

#ifndef LOG_TAG
#define LOG_TAG "bt_timer_test"
#endif

#include <gtest/gtest.h>
#include "bluetooth_timer.h"
#include "btcommon/timer_manager.h"
#include "btcommon/timer_manager_ffrt.h"
#include "bluetooth_log.h"

using namespace testing::ext;
using utility::TimerManager;

namespace OHOS {
namespace Bluetooth {
class TimerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TimerTest::SetUpTestCase(void) {}

void TimerTest::TearDownTestCase(void)
{}

void TimerTest::SetUp() {}

void TimerTest::TearDown() {}

/*
 * @tc.name: TimerTest_001
 * @tc.desc: Test Register when all is ok.
 * @tc.type: FUNC
 * @tc.require: issueI8EBTR
*/
HWTEST_F(TimerTest, TimerTest_001, TestSize.Level0)
{
    auto timerCallback = []() {};
    uint32_t timerId = 0;
    BluetoothTimer::GetInstance()->Register(timerCallback, timerId);
    uint32_t failed = 0;
    EXPECT_NE(timerId, failed);
    BluetoothTimer::GetInstance()->UnRegister(timerId);
}

/*
 * @tc.name: BT_Service_Timer_ffrt_Test_001
 * @tc.desc: Test once timer.
 * @tc.type: FUNC
*/
HWTEST_F(TimerTest, BT_Service_Timer_ffrt_Test_001, TestSize.Level0)
{
    HILOGI("BT_Service_Timer_ffrt_Test_001 begin");
    int timerCount = 0;
    auto timerCallback = [&timerCount]() {
        timerCount++;
    };

    uint64_t timerId = TimerManager::GetInstance()->StartTimer(timerCallback, 10, false);
    EXPECT_TRUE(TimerManager::GetInstance()->IsTimerStarted(timerId));

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    TimerManager::GetInstance()->StopTimer(timerId);
    EXPECT_FALSE(TimerManager::GetInstance()->IsTimerStarted(timerId));

    EXPECT_EQ(timerCount, 1);
    HILOGI("BT_Service_Timer_ffrt_Test_001 end");
}

/*
 * @tc.name: BT_Service_Timer_ffrt_Test_002
 * @tc.desc: Test periodic timer.
 * @tc.type: FUNC
*/
HWTEST_F(TimerTest, BT_Service_Timer_ffrt_Test_002, TestSize.Level0)
{
    HILOGI("BT_Service_Timer_ffrt_Test_002 begin");
    int timerCount = 0;
    auto timerCallback = [&timerCount]() {
        timerCount++;
    };

    uint64_t timerId = TimerManager::GetInstance()->StartTimer(timerCallback, 10, true);
    EXPECT_TRUE(TimerManager::GetInstance()->IsTimerStarted(timerId));

    std::this_thread::sleep_for(std::chrono::milliseconds(35));
    TimerManager::GetInstance()->StopTimer(timerId);
    EXPECT_FALSE(TimerManager::GetInstance()->IsTimerStarted(timerId));

    EXPECT_EQ(timerCount, 3);
    HILOGI("BT_Service_Timer_ffrt_Test_002 end");
}

/*
 * @tc.name: BT_Service_TimerTest_001
 * @tc.desc: Test Register when all is ok.
 * @tc.type: FUNC
*/
HWTEST_F(TimerTest, BT_Service_TimerTest_001, TestSize.Level0)
{
    HILOGI("BT_Service_TimerTest_001 begin");
    int timerCount = 0;
    auto timerCallback = [&timerCount]() {
        timerCount++;
    };
    utility::Timer timer(timerCallback);

    EXPECT_FALSE(timer.IsStarted());

    timer.Start(10, false);  // no periodic
    EXPECT_TRUE(timer.IsStarted());

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    timer.Stop();
    EXPECT_FALSE(timer.IsStarted());

    EXPECT_EQ(timerCount, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    HILOGI("BT_Service_TimerTest_001 end");
}

/*
 * @tc.name: BT_Service_TimerTest_002
 * @tc.desc: Test Register when all is ok.
 * @tc.type: FUNC
*/
HWTEST_F(TimerTest, BT_Service_TimerTest_002, TestSize.Level0)
{
    HILOGI("BT_Service_TimerTest_002 begin");
    int timerCount = 0;
    auto timerCallback = [&timerCount]() {
        timerCount++;
    };
    {
        utility::Timer timer(timerCallback);
        timer.Start(100, true);  // periodic
        std::this_thread::sleep_for(std::chrono::milliseconds(320));
        EXPECT_EQ(timerCount, 3);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(timerCount, 3);
    HILOGI("BT_Service_TimerTest_002 end");
}

/*
 * @tc.name: BT_Service_TimerTest_003
 * @tc.desc: Test Register when all is ok.
 * @tc.type: FUNC
*/
HWTEST_F(TimerTest, BT_Service_TimerTest_003, TestSize.Level0)
{
    HILOGI("BT_Service_TimerTest_003 begin");
    auto timerCallback = []() {};
    {
        utility::Timer timer(timerCallback);
        EXPECT_TRUE(timer.Start(100));
        EXPECT_FALSE(timer.Start(100));
    }

    {
        utility::Timer timer(timerCallback);
        EXPECT_TRUE(timer.Start(100));
        timer.Stop();
        timer.Stop();
    }
    HILOGI("BT_Service_TimerTest_003 end");
}

/*
 * @tc.name: BT_Service_TimerTest_004
 * @tc.desc: Test Clear timerId_ when once timer is triggered.
 * @tc.type: FUNC
*/
HWTEST_F(TimerTest, BT_Service_TimerTest_004, TestSize.Level0)
{
    HILOGI("BT_Service_TimerTest_004 begin");
    auto timerCallback = []() {};

    /******************** timerOnce ********************/
    utility::Timer timerOnce(timerCallback);
    EXPECT_FALSE(timerOnce.IsStarted());

    timerOnce.Start(10, false);  // no periodic
    EXPECT_TRUE(timerOnce.IsStarted());

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(timerOnce.IsStarted());

    /******************** timerPeriodic ********************/
    utility::Timer timerPeriodic(timerCallback);
    EXPECT_FALSE(timerPeriodic.IsStarted());

    timerPeriodic.Start(10, true);  // periodic
    EXPECT_TRUE(timerPeriodic.IsStarted());

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_TRUE(timerPeriodic.IsStarted());

    timerPeriodic.Stop();
    EXPECT_FALSE(timerPeriodic.IsStarted());

    HILOGI("BT_Service_TimerTest_004 end");
}

/*
 * @tc.name: BT_Service_TimerTest_005
 * @tc.desc: Test timer not crash.
 * @tc.type: FUNC
*/
HWTEST_F(TimerTest, BT_Service_TimerTest_005, TestSize.Level0)
{
    HILOGI("BT_Service_TimerTest_005 begin");
    auto timerCallback = []() {
        HILOGI("timer trigger");
    };

    {
        // 多线程同时操作同一个定时器
        utility::Timer timer(timerCallback);
        std::thread startThread1([&timer]() {
            for (int i = 0; i < 1000; i++) {
                timer.Start(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        std::thread startThread2([&timer]() {
            for (int i = 0; i < 1000; i++) {
                timer.Start(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        std::thread stopThread1([&timer]() {
            for (int i = 0; i < 1000; i++) {
                timer.Stop();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        std::thread stopThread2([&timer]() {
            for (int i = 0; i < 1000; i++) {
                timer.Stop();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        startThread1.join();
        startThread2.join();
        stopThread1.join();
        stopThread2.join();

        timer.Stop();
        EXPECT_FALSE(timer.IsStarted());
    }

    HILOGI("BT_Service_TimerTest_005 end");
}

}  // namespace Bluetooth
}  // namespace OHOS