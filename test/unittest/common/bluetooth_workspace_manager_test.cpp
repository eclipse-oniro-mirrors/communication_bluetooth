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
#include "bluetooth_workspace_manager.h"

#include <gtest/gtest.h>

#include "bluetooth_log.h"
#include "class_creator.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
class WorkspaceManagerTestForceLink {};

static void *WorkspaceManagerTestForceLinkCreator(void)
{
    return static_cast<void *>(new WorkspaceManagerTestForceLink());
}

const ClassRegister g_workspaceManagerTestForceLink(
    "WorkspaceManagerTestForceLink", WorkspaceManagerTestForceLinkCreator);

class BluetoothWorkspaceManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        auto &manager = BluetoothWorkspaceManager::GetInstance();
        manager.isEnterpriseWorkspace_ = false;
        manager.currentUserId_ = INVALID_ID;
        manager.isInit_ = false;
    }
};

/**
 * @tc.number: bluetooth_workspace_manager_test_001
 * @tc.name: Init
 * @tc.desc: Init is idempotent
 */
HWTEST_F(BluetoothWorkspaceManagerTest, bluetooth_workspace_manager_test_001, TestSize.Level1)
{
    HILOGI("bluetooth_workspace_manager_test_001 enter");
    auto &manager = BluetoothWorkspaceManager::GetInstance();
    manager.isInit_ = false;
    manager.Init();
    EXPECT_TRUE(manager.isInit_);
    manager.Init();
    EXPECT_TRUE(manager.isInit_);
    HILOGI("bluetooth_workspace_manager_test_001 end");
}

/**
 * @tc.number: bluetooth_workspace_manager_test_002
 * @tc.name: HandleUserSwitched
 * @tc.desc: HandleUserSwitched with invalid and valid user id
 */
HWTEST_F(BluetoothWorkspaceManagerTest, bluetooth_workspace_manager_test_002, TestSize.Level1)
{
    HILOGI("bluetooth_workspace_manager_test_002 enter");
    auto &manager = BluetoothWorkspaceManager::GetInstance();
    manager.currentUserId_ = INVALID_ID;
    manager.isEnterpriseWorkspace_ = false;
    manager.HandleUserSwitched(INVALID_ID);
    EXPECT_EQ(manager.currentUserId_.load(), INVALID_ID);
    EXPECT_FALSE(manager.IsEnterpriseWorkspace());
    manager.HandleUserSwitched(100);
    EXPECT_EQ(manager.currentUserId_.load(), 100);
    manager.HandleUserSwitched(101);
    EXPECT_EQ(manager.currentUserId_.load(), 101);
    HILOGI("bluetooth_workspace_manager_test_002 end");
}

/**
 * @tc.number: bluetooth_workspace_manager_test_003
 * @tc.name: IsEnterpriseWorkspace
 * @tc.desc: IsEnterpriseWorkspace returns workspace state
 */
HWTEST_F(BluetoothWorkspaceManagerTest, bluetooth_workspace_manager_test_003, TestSize.Level1)
{
    HILOGI("bluetooth_workspace_manager_test_003 enter");
    auto &manager = BluetoothWorkspaceManager::GetInstance();
    manager.isEnterpriseWorkspace_ = false;
    EXPECT_FALSE(manager.IsEnterpriseWorkspace());
    manager.isEnterpriseWorkspace_ = true;
    EXPECT_TRUE(manager.IsEnterpriseWorkspace());
    HILOGI("bluetooth_workspace_manager_test_003 end");
}
}  // namespace bluetooth
}  // namespace OHOS
