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

#include <atomic>
#include <future>
#include <thread>

#include "log.h"
#include "thread_util.h"

#include "event_handler.h"  // libeventhandler
#include "event_runner.h"  // libeventhandler
#include <memory>
using OHOS::AppExecFwk::EventRunner;
using OHOS::AppExecFwk::EventHandler;

#include "ffrt_inner.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
class ThreadUtilTest : public testing::Test {
public:
    ThreadUtilTest() = default;
    ~ThreadUtilTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void ThreadUtilTest::SetUpTestSuite(void)
{}
void ThreadUtilTest::TearDownTestSuite(void)
{}
void ThreadUtilTest::SetUp()
{}
void ThreadUtilTest::TearDown()
{}

/**
 * @tc.name: thread_util_test_001
 * @tc.desc: Verify the thread enabled.
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_001, TestSize.Level0)
{
    HILOGI("thread_util_test_001 enter");

    auto emptyFuc = []() {};
    DoInBleThread(emptyFuc);
    DoInGattThread(emptyFuc);
    DoInAvrcpTgThread(emptyFuc);
    DoInClassicThread(emptyFuc);
    DoInA2dpThread(emptyFuc);
    DoInHfpThread(emptyFuc);
    DoInHidHostThread(emptyFuc);
    DoInHidDeviceThread(emptyFuc);
    DoInPanThread(emptyFuc);
    DoInSocketThread(emptyFuc);
    DoInAdapterManagerThread(emptyFuc);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    HILOGI("thread_util_test_001 end");
}

/**
 * @tc.name: thread_util_test_002
 * @tc.desc: Verify the thread disabled.
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_002, TestSize.Level0)
{
    HILOGI("thread_util_test_002 enter");

    auto emptyFuc = []() {};
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::DISABLED);
    DoInBleThread(emptyFuc);

    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ENABLED);
    HILOGI("thread_util_test_002 end");
}

/**
 * @tc.name: thread_util_test_003
 * @tc.desc: Verify the task execute delay.
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_003, TestSize.Level0)
{
    HILOGI("thread_util_test_003 enter");

    std::atomic_int flag = 0;
    auto emptyFuc = [&flag]() {
        flag = 1;
    };
    DoInBleThread(emptyFuc, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(flag.load(), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(flag.load(), 1);

    HILOGI("thread_util_test_003 end");
}

/**
 * @tc.name: thread_util_test_004
 * @tc.desc: Verify the task execute not switch thread.
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_004, TestSize.Level0)
{
    HILOGI("thread_util_test_004 enter");

    std::atomic_int flag = 0;
    auto emptyFuc = [&flag]() {
        flag++;
    };

    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::NOT_SWITCH_THREAD);
    DoInBleThread(emptyFuc);
    EXPECT_EQ(flag.load(), 1);

    DoInBleThread(emptyFuc, 50);
    EXPECT_EQ(flag.load(), 2);

    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ENABLED);
    HILOGI("thread_util_test_004 end");
}

/**
 * @tc.name: thread_util_test_005
 * @tc.desc: Verify the duplicate delay task, only the last task effect.
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_005, TestSize.Level0)
{
    HILOGI("thread_util_test_005 enter");

    std::atomic_int flag = 0;
    auto testFuc = [&flag]() {
        flag++;
    };

    ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 10, "thread_util_test_005");
    ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 10, "thread_util_test_005");

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_EQ(flag.load(), 1);

    HILOGI("thread_util_test_005 end");
}

/**
 * @tc.name: thread_util_test_006
 * @tc.desc: Verify remove delay task.
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_006, TestSize.Level0)
{
    HILOGI("thread_util_test_006 enter");

    std::atomic_int flag = 0;
    auto testFuc = [&flag]() {
        flag++;
    };

    ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 10, "thread_util_test_006");
    ThreadUtil::GetInstance().RemoveTask(THREAD_ID_BLE, "thread_util_test_006");
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_EQ(flag.load(), 0);


    HILOGI("thread_util_test_006 end");
}

/**
 * @tc.name: thread_util_test_007
 * @tc.desc: Verify the delay task triggered, and remove the triggered task.
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_007, TestSize.Level0)
{
    HILOGI("thread_util_test_007 enter");

    std::atomic_int flag = 0;
    auto testFuc = [&flag]() {
        flag++;
    };

    ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 10, "thread_util_test_007");
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 10, "thread_util_test_007");
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_EQ(flag.load(), 2);

    HILOGI("thread_util_test_007 end");
}

/**
 * @tc.name: thread_util_test_008
 * @tc.desc: Verify the delay task triggered, Multi-thread security
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_008, TestSize.Level0)
{
    HILOGI("thread_util_test_008 enter");

    auto testFuc = []() {};
    ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 100, "test1");
    ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 100, "test2");
    auto postTaskFunc = [testFuc]() {
        ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 10, "thread_util_test_008");
    };
    for (int i = 0; i < 100; i++) {
        std::thread postTaskThread(postTaskFunc);
        postTaskThread.detach();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    HILOGI("thread_util_test_008 end");
}

HWTEST_F(ThreadUtilTest, thread_util_test_009, TestSize.Level0)
{
    HILOGI("thread_util_test_009 enter");

    ffrt::queue queue("test");
    ffrt::recursive_mutex mutex {};
    int flag = 0;
    auto testFuc = [&mutex, &flag, &queue]() {
        std::lock_guard<ffrt::recursive_mutex> lock(mutex);
        flag++;

        auto func = [&mutex, &flag]() {
            std::lock_guard<ffrt::recursive_mutex> lock2(mutex);
            flag++;
        };
        queue.submit(func);
    };

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create("test");
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>(eventRunner);
    auto testFuc1 = [&mutex, &flag, &eventHandler]() {
        std::lock_guard<ffrt::recursive_mutex> lock(mutex);
        flag++;

        std::lock_guard<ffrt::recursive_mutex> lock1(mutex);
        auto func = [&mutex, &flag]() {
            std::lock_guard<ffrt::recursive_mutex> lock2(mutex);
            flag++;
        };
        eventHandler->PostTask(func);
    };

    for (int i = 0; i < 10000; i++) {
        queue.submit(testFuc);
        eventHandler->PostTask(testFuc1);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    EXPECT_EQ(flag, 40000);
    HILOGI("thread_util_test_009 end");
}

/**
 * @tc.name: thread_util_test_010
 * @tc.desc: Test remove task when task executed
 * @tc.type: FUNC
 */
HWTEST_F(ThreadUtilTest, thread_util_test_010, TestSize.Level0)
{
    HILOGI("thread_util_test_010 enter");

    auto testFuc = []() {
        ThreadUtil::GetInstance().RemoveTask(THREAD_ID_BLE, "thread_util_test_010");
    };
    ThreadUtil::GetInstance().PostTask(THREAD_ID_BLE, testFuc, 10, "thread_util_test_010");
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    HILOGI("thread_util_test_010 end");
}

}  // namespace bluetooth
}  // namespace OHOS
