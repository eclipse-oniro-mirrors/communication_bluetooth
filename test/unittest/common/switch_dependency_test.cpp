/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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

#include "bluetooth_datashare_utils.h"
#include "bluetooth_common_event_subscriber.h"
#include "bluetooth_log.h"
#include "thread_util.h"
#include "switch_dependency.h"

using namespace testing;
using namespace testing::ext;
// using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static bool g_isDataShareReady = false;
bool BluetoothDataShareHelperUtils::CheckDataShareIsReady(void)
{
    return g_isDataShareReady;
}

class SwitchDependencyTest : public testing::Test {
public:
    SwitchDependencyTest() {}
    ~SwitchDependencyTest() {}

    static void SetUpTestCase(void)
    {
        ThreadUtil::GetInstance().ChangeThreadState(
            THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::NOT_SWITCH_THREAD);
    }
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        g_isDataShareReady = false;
    }
    void TearDown() {}
};

void BluetoothCommonEventSubscriberEx::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{}

class DependencyCallbackMock {
public:
    MOCK_METHOD(void, DependencyCallback, ());
};

/*
 * @tc.number: UnitTest_SwitchDependencyTest_001
 * @tc.name: data share is not ready
 * @tc.desc: name :
 */
HWTEST_F(SwitchDependencyTest, UnitTest_SwitchDependencyTest_001, TestSize.Level0)
{
    HILOGI("UnitTest_SwitchDependencyTest_001 start");
    DependencyCallbackMock callbackMock;
    EXPECT_CALL(callbackMock, DependencyCallback()).Times(0);

    g_isDataShareReady = false;
    auto func = [&callbackMock]() { callbackMock.DependencyCallback(); };
    auto dependency = std::make_shared<SwitchDependency>(func);
    int ret = dependency->Init();
    EXPECT_EQ(ret, 0);

    // Wait system ability callback
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("UnitTest_SwitchDependencyTest_001 end");
}

/*
 * @tc.number: UnitTest_SwitchDependencyTest_002
 * @tc.name: data share is already ready
 * @tc.desc: name :
 */
HWTEST_F(SwitchDependencyTest, UnitTest_SwitchDependencyTest_002, TestSize.Level0)
{
    HILOGI("UnitTest_SwitchDependencyTest_002 start");
    DependencyCallbackMock callbackMock;
    EXPECT_CALL(callbackMock, DependencyCallback()).Times(1);

    g_isDataShareReady = true;
    auto func = [&callbackMock]() { callbackMock.DependencyCallback(); };
    auto dependency = std::make_shared<SwitchDependency>(func);
    int ret = dependency->Init();
    EXPECT_EQ(ret, 0);

    // Wait system ability callback
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("UnitTest_SwitchDependencyTest_002 end");
}

/*
 * @tc.number: UnitTest_SwitchDependencyTest_003
 * @tc.name: data share is already ready
 * @tc.desc: name :
 */
HWTEST_F(SwitchDependencyTest, UnitTest_SwitchDependencyTest_003, TestSize.Level0)
{
    HILOGI("UnitTest_SwitchDependencyTest_003 start");
    DependencyCallbackMock callbackMock;
    EXPECT_CALL(callbackMock, DependencyCallback()).Times(1);

    g_isDataShareReady = false;
    auto func = [&callbackMock]() { callbackMock.DependencyCallback(); };
    auto dependency = std::make_shared<SwitchDependency>(func);
    int ret = dependency->Init();
    EXPECT_EQ(ret, 0);
    // Wait system ability callback
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::NOT_SWITCH_THREAD);
    dependency->OnDataShareReadyEvent();
    HILOGI("UnitTest_SwitchDependencyTest_003 end");
}

/*
 * @tc.number: UnitTest_SwitchDependencyTest_004
 * @tc.name: Check dependency callback can't double trigger
 * @tc.desc: name :
 */
HWTEST_F(SwitchDependencyTest, UnitTest_SwitchDependencyTest_004, TestSize.Level0)
{
    HILOGI("UnitTest_SwitchDependencyTest_004 start");
    DependencyCallbackMock callbackMock;
    EXPECT_CALL(callbackMock, DependencyCallback()).Times(1);

    g_isDataShareReady = true;
    auto func = [&callbackMock]() { callbackMock.DependencyCallback(); };
    auto dependency = std::make_shared<SwitchDependency>(func);
    int ret = dependency->Init();
    EXPECT_EQ(ret, 0);
    // Wait system ability callback
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    dependency->CheckAllDependencySatisfied();
    dependency->CheckAllDependencySatisfied();
    HILOGI("UnitTest_SwitchDependencyTest_004 end");
}

/*
 * @tc.number: UnitTest_SwitchDependencyTest_005
 * @tc.name: Dependency callback is nullptr
 * @tc.desc: name :
 */
HWTEST_F(SwitchDependencyTest, UnitTest_SwitchDependencyTest_005, TestSize.Level0)
{
    HILOGI("UnitTest_SwitchDependencyTest_005 start");
    g_isDataShareReady = true;
    auto dependency = std::make_shared<SwitchDependency>(nullptr);
    int ret = dependency->Init();
    EXPECT_EQ(ret, 0);
    // Wait system ability callback
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("UnitTest_SwitchDependencyTest_005 end");
}

/*
 * @tc.number: UnitTest_SwitchDependencyTest_006
 * @tc.name: Check invalid_sys_param success
 * @tc.desc: name :
 */
HWTEST_F(SwitchDependencyTest, UnitTest_SwitchDependencyTest_006, TestSize.Level0)
{
    HILOGI("UnitTest_SwitchDependencyTest_006 start");
    bool trigger = false;
    auto func = [&trigger]() {
        trigger = true;
    };

    g_isDataShareReady = true;
    auto dependency = std::make_shared<SwitchDependency>(func);
    std::vector<SwitchDependency::DependedSystemParam> vec {
        {"invalid_sys_param", "success", "false", false},
    };
    dependency->dependedSystemParamsVec_ = vec;

    int ret = dependency->Init();
    EXPECT_EQ(ret, 0);
    // Wait system ability callback
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(trigger);

    dependency->OnSystemParameterChanged("invalid_sys_param", "success");
    EXPECT_TRUE(trigger);
    HILOGI("UnitTest_SwitchDependencyTest_006 end");
}

/*
 * @tc.number: UnitTest_SwitchDependencyTest_007
 * @tc.name: Check invalid_sys_param false
 * @tc.desc: name :
 */
HWTEST_F(SwitchDependencyTest, UnitTest_SwitchDependencyTest_007, TestSize.Level0)
{
    HILOGI("UnitTest_SwitchDependencyTest_007 start");
    bool trigger = false;
    auto func = [&trigger]() {
        trigger = true;
    };

    g_isDataShareReady = true;
    auto dependency = std::make_shared<SwitchDependency>(func);
    std::vector<SwitchDependency::DependedSystemParam> vec {
        {"invalid_sys_param", "success", "false", false},
    };
    dependency->dependedSystemParamsVec_ = vec;

    int ret = dependency->Init();
    EXPECT_EQ(ret, 0);
    dependency->RegisterBootCompleteCommonEvent();
    // Wait system ability callback
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(trigger);

    dependency->OnSystemParameterChanged("invalid_sys_param", "false");
    EXPECT_FALSE(trigger);
    HILOGI("UnitTest_SwitchDependencyTest_007 end");
}
}  // namespace bluetooth
}  // namespace OHOS
