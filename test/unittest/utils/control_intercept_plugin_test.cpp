/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "control_intercept_plugin.h"
#include "log.h"

using namespace testing;
using namespace testing::ext;

class ControlInterceptPluginTest : public testing::Test {
public:
    ControlInterceptPluginTest(void) = default;
    ~ControlInterceptPluginTest(void) override = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp(void);
    void TearDown(void);
};

void ControlInterceptPluginTest::SetUpTestSuite(void)
{}
void ControlInterceptPluginTest::TearDownTestSuite(void)
{}
void ControlInterceptPluginTest::SetUp(void)
{}
void ControlInterceptPluginTest::TearDown(void)
{
    ControlInterceptDestroy();
}

/**
 * @tc.name: control_intercept_plugin_test_001
 * @tc.desc: 测试插件允许消息
 * @tc.type: FUNC
 */
HWTEST_F(ControlInterceptPluginTest, control_intercept_plugin_test_001, TestSize.Level0)
{
    HILOGI("control_intercept_plugin_test_001 enter");
    auto fastestGattConnTrue = [](const ControlInterceptMessage &msg) { return true; };
    ControlInterceptPlugin plugin {
        .fastestGattConn = fastestGattConnTrue,
    };
    ControlInterceptRegisterPlugin(plugin);

    ControlInterceptMessage msg {
        .addr = "12:34:56:78:90:AB",
        .isSystemHap = true,
        .isNativeCaller = true,
    };
    EXPECT_TRUE(ControlInterceptIsAllowedFastestGattConn(msg));
    HILOGI("control_intercept_plugin_test_001 end");
}

/**
 * @tc.name: control_intercept_plugin_test_002
 * @tc.desc: 测试插件不允许消息
 * @tc.type: FUNC
 */
HWTEST_F(ControlInterceptPluginTest, control_intercept_plugin_test_002, TestSize.Level0)
{
    HILOGI("control_intercept_plugin_test_002 enter");
    auto fastestGattConnFalse = [](const ControlInterceptMessage &msg) { return false; };
    ControlInterceptPlugin plugin {
        .fastestGattConn = fastestGattConnFalse,
    };
    ControlInterceptRegisterPlugin(plugin);

    ControlInterceptMessage msg {
        .addr = "12:34:56:78:90:AB",
        .isSystemHap = true,
        .isNativeCaller = true,
    };
    EXPECT_FALSE(ControlInterceptIsAllowedFastestGattConn(msg));
    HILOGI("control_intercept_plugin_test_002 end");
}

/**
 * @tc.name: control_intercept_plugin_test_003
 * @tc.desc: 测试没有插件
 * @tc.type: FUNC
 */
HWTEST_F(ControlInterceptPluginTest, control_intercept_plugin_test_003, TestSize.Level0)
{
    HILOGI("control_intercept_plugin_test_003 enter");
    ControlInterceptMessage msg {
        .addr = "12:34:56:78:90:AB",
        .isSystemHap = true,
        .isNativeCaller = true,
    };
    EXPECT_TRUE(ControlInterceptIsAllowedFastestGattConn(msg));
    HILOGI("control_intercept_plugin_test_003 end");
}

/**
 * @tc.name: control_intercept_plugin_test_004
 * @tc.desc: 测试多个插件，其中一些允许消息，一些不允许
 * @tc.type: FUNC
 */
HWTEST_F(ControlInterceptPluginTest, control_intercept_plugin_test_004, TestSize.Level0)
{
    HILOGI("control_intercept_plugin_test_004 enter");
    auto fastestGattConnTrue = [](const ControlInterceptMessage &msg) { return true; };
    ControlInterceptPlugin plugin1 {
        .fastestGattConn = fastestGattConnTrue,
    };
    ControlInterceptRegisterPlugin(plugin1);

    auto fastestGattConnFalse = [](const ControlInterceptMessage &msg) { return false; };
    ControlInterceptPlugin plugin2 {
        .fastestGattConn = fastestGattConnFalse,
    };
    ControlInterceptRegisterPlugin(plugin2);

    ControlInterceptMessage msg {
        .addr = "12:34:56:78:90:AB",
        .isSystemHap = true,
        .isNativeCaller = true,
    };
    EXPECT_FALSE(ControlInterceptIsAllowedFastestGattConn(msg));
    HILOGI("control_intercept_plugin_test_004 end");
}

/**
 * @tc.name: control_intercept_plugin_test_005
 * @tc.desc: 测试EDM插件
 * @tc.type: FUNC
 */
HWTEST_F(ControlInterceptPluginTest, control_intercept_plugin_test_005, TestSize.Level0)
{
    HILOGI("control_intercept_plugin_test_005 enter");
    auto edmAllowedConnTrue = [](const ControlInterceptMessage &msg) { return true; };
    ControlInterceptPlugin plugin {
        .gattConn = edmAllowedConnTrue,
        .avrcpConn = edmAllowedConnTrue,
        .pairConn = edmAllowedConnTrue,
        .aclConn = edmAllowedConnTrue,
        .a2dpConn = edmAllowedConnTrue,
        .hfpConn = edmAllowedConnTrue,
        .hidConn = edmAllowedConnTrue,
        .socketConn = edmAllowedConnTrue,
        .oppWrite = edmAllowedConnTrue,
        .oppRead = edmAllowedConnTrue,
        .gattReceive = edmAllowedConnTrue,
    };
    ControlInterceptRegisterPlugin(plugin);

    ControlInterceptMessage msg {
        .addr = "12:34:56:78:90:AB",
    };
    EXPECT_TRUE(ControlInterceptIsAllowedAvrcpConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedPairConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedAclConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedA2dpConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedHfpConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedHidConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedOppRead(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedGattReceive(msg));

    const std::vector<std::string> filePaths = {"/data/test1.txt", "/data/test2.txt"};
    msg.filePaths = filePaths;
    EXPECT_TRUE(ControlInterceptIsAllowedOppWrite(msg));

    msg.filePaths = {};
    msg.isSystemHap = true;
    msg.isNativeCaller = true;
    EXPECT_TRUE(ControlInterceptIsAllowedSocketConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedGattAddService(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedSocketListen(msg));

    HILOGI("control_intercept_plugin_test_005 end");
}