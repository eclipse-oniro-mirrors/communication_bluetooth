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

#include <thread>

#undef private
#define private public

#include "btcommon/dynamic_library_loader.h"
#include "bluetooth_log.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
static const char *HIECHO_LIB_NAME = "libhiecho.z.so";
static const char *HIECHO_LIB_FUNC_NAME = "HiechoRecvIndMsg";

#define CHECK_DLL_SM_STATE(sm, expect) EXPECT_EQ((sm).GetState()->Name(), std::string(expect))

class DynamicLibraryLoaderTest : public testing::Test {
public:
    DynamicLibraryLoaderTest(void) = default;
    ~DynamicLibraryLoaderTest(void) override = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp(void);
    void TearDown(void);
};

void DynamicLibraryLoaderTest::SetUpTestSuite(void)
{}
void DynamicLibraryLoaderTest::TearDownTestSuite(void)
{
    utility::CloseAllTimer();
}
void DynamicLibraryLoaderTest::SetUp(void)
{}
void DynamicLibraryLoaderTest::TearDown(void)
{}

/**
 * @tc.name: dynamic_library_loader_test_001
 * @tc.desc: load success
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_001, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_001 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);

    loader.OpenLib();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);

    loader.CloseLib();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);

    HILOGI("dynamic_library_loader_test_001 end");
}

/**
 * @tc.name: dynamic_library_loader_test_002
 * @tc.desc: load fail
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_002, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_002 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    loader.OpenLib();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    EXPECT_EQ(loader.GetSymbol("xxxxx"), nullptr);
    loader.CloseLib();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);

    HILOGI("dynamic_library_loader_test_002 end");
}

/**
 * @tc.name: dynamic_library_loader_test_003
 * @tc.desc: Check load times
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_003, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_003 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    EXPECT_EQ(loader.loadTimes_, 0);

    loader.OpenLib();
    loader.OpenLib();
    loader.OpenLib();
    loader.OpenLib();
    EXPECT_EQ(loader.loadTimes_, 4);
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    loader.CloseLib();
    loader.CloseLib();
    loader.CloseLib();
    EXPECT_EQ(loader.loadTimes_, 1);
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    loader.CloseLib();
    EXPECT_EQ(loader.loadTimes_, 0);
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);

    HILOGI("dynamic_library_loader_test_003 end");
}

/**
 * @tc.name: dynamic_library_loader_test_004
 * @tc.desc: Check delay unload
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_004, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_004 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME, 100);

    loader.OpenLib();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    loader.CloseLib();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);

    HILOGI("dynamic_library_loader_test_004 end");
}

/**
 * @tc.name: dynamic_library_loader_test_005
 * @tc.desc: Check get symbol
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_005, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_005 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    EXPECT_EQ(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);

    loader.OpenLib();
    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);
    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);
    EXPECT_EQ(loader.GetSymbol("xxxx"), nullptr);

    HILOGI("dynamic_library_loader_test_005 end");
}

/**
 * @tc.name: dynamic_library_loader_test_006
 * @tc.desc: Check unexpect event
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_006, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_006 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);
    utility::Message msg1(DynamicLibraryLoader::UNLOAD_EVENT);
    loader.ProcessMessage(msg1);

    loader.OpenLib();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);
    utility::Message msg2(DynamicLibraryLoader::UNLOAD_TIMER_EVENT);
    loader.ProcessMessage(msg2);

    loader.CloseLib();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);
    utility::Message msg3(DynamicLibraryLoader::UNLOAD_EVENT);
    loader.ProcessMessage(msg3);

    HILOGI("dynamic_library_loader_test_006 end");
}

/**
 * @tc.name: dynamic_library_loader_test_007
 * @tc.desc: Check load -> wait for unload -> load
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_007, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_007 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);

    loader.OpenLib();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    loader.CloseLib();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);

    loader.OpenLib();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    HILOGI("dynamic_library_loader_test_007 end");
}

/**
 * @tc.name: dynamic_library_loader_test_008
 * @tc.desc: Check CDynamicLibraryLoader
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_008, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_008 enter");

    CDynamicLibraryLoader loader(HIECHO_LIB_NAME);
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);

    loader.OpenLib();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);

    loader.CloseLib();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);

    HILOGI("dynamic_library_loader_test_008 end");
}

/**
 * @tc.name: dynamic_library_loader_test_009
 * @tc.desc: Check CDynamicLibraryLoader
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_009, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_009 enter");

    CDynamicLibraryLoader loader(HIECHO_LIB_NAME);
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);

    loader.OpenLib();
    loader.LoadTask();

    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);

    loader.CloseLib();
    loader.UnloadTask();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);
    HILOGI("dynamic_library_loader_test_009 end");
}

/**
 * @tc.name: dynamic_library_loader_test_010
 * @tc.desc: Check load -> wait for unload -> load
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_010, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_010 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);

    loader.OpenLib();
    loader.LoadTask();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    loader.CloseLib();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);

    loader.OpenLib();
    loader.UnloadTask();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    HILOGI("dynamic_library_loader_test_010 end");
}

/**
 * @tc.name: dynamic_library_loader_test_011
 * @tc.desc: Check unexpect event
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_011, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_011 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);
    utility::Message msg1(DynamicLibraryLoader::UNLOAD_EVENT);
    loader.ProcessMessage(msg1);

    loader.OpenLib();
    loader.LoadTask();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);
    utility::Message msg2(DynamicLibraryLoader::UNLOAD_TIMER_EVENT);
    loader.ProcessMessage(msg2);

    loader.CloseLib();
    loader.UnloadTask();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);
    utility::Message msg3(DynamicLibraryLoader::UNLOAD_EVENT);
    loader.ProcessMessage(msg3);

    HILOGI("dynamic_library_loader_test_011 end");
}

/**
 * @tc.name: dynamic_library_loader_test_012
 * @tc.desc: Check get symbol
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_012, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_012 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    EXPECT_EQ(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);

    loader.OpenLib();
    loader.LoadTask();
    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);
    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);
    EXPECT_EQ(loader.GetSymbol("xxxx"), nullptr);

    HILOGI("dynamic_library_loader_test_012 end");
}

/**
 * @tc.name: dynamic_library_loader_test_013
 * @tc.desc: get load times
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_013, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_013 enter");
    DynamicLibraryLoader loader(HIECHO_LIB_NAME);
    EXPECT_EQ(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);
    loader.OpenLib();
    loader.LoadTask();
    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);
    EXPECT_NE(loader.GetSymbol(HIECHO_LIB_FUNC_NAME), nullptr);
    EXPECT_EQ(loader.GetSymbol("xxxx"), nullptr);
    loader.IsLibraryLoaded();
    HILOGI("dynamic_library_loader_test_013 end");
}

/**
 * @tc.name: dynamic_library_loader_test_014
 * @tc.desc: Check unexpect event
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLibraryLoaderTest, dynamic_library_loader_test_014, TestSize.Level0)
{
    HILOGI("dynamic_library_loader_test_014 enter");

    DynamicLibraryLoader loader(HIECHO_LIB_NAME, 100, [](const ThreadUtilFunc &func) { DoInA2dpThread(func); });
    loader.OpenLib();
    CHECK_DLL_SM_STATE(loader, DLL_LOADED_STATE);

    loader.CloseLib();
    CHECK_DLL_SM_STATE(loader, DLL_WAIT_FOR_UNLOAD_STATE);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CHECK_DLL_SM_STATE(loader, DLL_UNLOAD_STATE);

    HILOGI("dynamic_library_loader_test_014 end");
}
}  // namespace bluetooth
}  // namespace OHOS
