/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "device_priority_manager.h"

#include <gtest/gtest.h>
#include <memory>

#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

const std::string DEV_A = "00:11:22:33:44:55";
const std::string DEV_B = "AA:BB:CC:DD:EE:FF";
const std::string DEV_C = "11:22:33:44:55:66";

class DevicePriorityManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("DevicePriorityManagerTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("DevicePriorityManagerTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("DevicePriorityManagerTest::SetUp start");
        dpm_ = std::make_unique<DevicePriorityManager>();
    }
    void TearDown() override
    {
        HILOGI("DevicePriorityManagerTest::TearDown start");
        dpm_ = nullptr;
    }

protected:
    std::unique_ptr<DevicePriorityManager> dpm_;
};

/**
 * @tc.name: Device_priority_manager_Init_NoCrash_001
 * @tc.desc: Init 在单测环境（无 DataShare/Adapter）不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_Init_NoCrash_001, TestSize.Level0)
{
    HILOGI("Device_priority_manager_Init_NoCrash_001 enter");
    dpm_->Init();
    SUCCEED();
    HILOGI("Device_priority_manager_Init_NoCrash_001 end");
}

/**
 * @tc.name: Device_priority_manager_GetNextDevice_EmptyList_002
 * @tc.desc: 空列表时 GetNextDevice 返回空字符串
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_GetNextDevice_EmptyList_002, TestSize.Level1)
{
    HILOGI("Device_priority_manager_GetNextDevice_EmptyList_002 enter");
    dpm_->Init();
    EXPECT_TRUE(dpm_->GetNextDevice("").empty());
    HILOGI("Device_priority_manager_GetNextDevice_EmptyList_002 end");
}

/**
 * @tc.name: Device_priority_manager_SetLastDevice_EmptyAddr_003
 * @tc.desc: SetLastDevice 空地址被忽略
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_SetLastDevice_EmptyAddr_003, TestSize.Level1)
{
    HILOGI("Device_priority_manager_SetLastDevice_EmptyAddr_003 enter");
    dpm_->SetLastDevice("");
    EXPECT_TRUE(dpm_->GetLastDevice().empty());
    HILOGI("Device_priority_manager_SetLastDevice_EmptyAddr_003 end");
}

/**
 * @tc.name: Device_priority_manager_SetLastDevice_SameAddr_004
 * @tc.desc: SetLastDevice 相同地址被忽略（不刷新 penultimate）
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_SetLastDevice_SameAddr_004, TestSize.Level1)
{
    HILOGI("Device_priority_manager_SetLastDevice_SameAddr_004 enter");
    dpm_->SetLastDevice(DEV_A);
    dpm_->SetLastDevice(DEV_A);
    EXPECT_EQ(dpm_->GetLastDevice(), DEV_A);
    HILOGI("Device_priority_manager_SetLastDevice_SameAddr_004 end");
}

/**
 * @tc.name: Device_priority_manager_SetLastDevice_UpdatesLastAndPenultimate_005
 * @tc.desc: SetLastDevice 更新 last 并刷新 penultimate
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_SetLastDevice_UpdatesLastAndPenultimate_005, TestSize.Level1)
{
    HILOGI("Device_priority_manager_SetLastDevice_UpdatesLastAndPenultimate_005 enter");
    dpm_->SetLastDevice(DEV_A);
    dpm_->SetLastDevice(DEV_B);
    EXPECT_EQ(dpm_->GetLastDevice(), DEV_B);
    EXPECT_EQ(dpm_->GetPenultimateDevice(), DEV_A);
    HILOGI("Device_priority_manager_SetLastDevice_UpdatesLastAndPenultimate_005 end");
}

/**
 * @tc.name: Device_priority_manager_ClearLastDevice_006
 * @tc.desc: ClearLastDevice 清除 last
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_ClearLastDevice_006, TestSize.Level1)
{
    HILOGI("Device_priority_manager_ClearLastDevice_006 enter");
    dpm_->SetLastDevice(DEV_A);
    dpm_->ClearLastDevice();
    EXPECT_TRUE(dpm_->GetLastDevice().empty());
    HILOGI("Device_priority_manager_ClearLastDevice_006 end");
}

/**
 * @tc.name: Device_priority_manager_ClearPenultimateDevice_007
 * @tc.desc: ClearPenultimateDevice 清除 penultimate
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_ClearPenultimateDevice_007, TestSize.Level1)
{
    HILOGI("Device_priority_manager_ClearPenultimateDevice_007 enter");
    dpm_->SetLastDevice(DEV_A);
    dpm_->SetLastDevice(DEV_B);
    dpm_->ClearPenultimateDevice();
    EXPECT_TRUE(dpm_->GetPenultimateDevice().empty());
    HILOGI("Device_priority_manager_ClearPenultimateDevice_007 end");
}

/**
 * @tc.name: Device_priority_manager_SetPenultimateDevice_SetsDirectly_008
 * @tc.desc: SetPenultimateDevice 直接设置
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_SetPenultimateDevice_SetsDirectly_008, TestSize.Level1)
{
    HILOGI("Device_priority_manager_SetPenultimateDevice_SetsDirectly_008 enter");
    dpm_->SetPenultimateDevice(DEV_C);
    EXPECT_EQ(dpm_->GetPenultimateDevice(), DEV_C);
    HILOGI("Device_priority_manager_SetPenultimateDevice_SetsDirectly_008 end");
}

/**
 * @tc.name: Device_priority_manager_GetPreferredDevice_DefaultEmpty_009
 * @tc.desc: GetPreferredDevice 默认空
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_GetPreferredDevice_DefaultEmpty_009, TestSize.Level1)
{
    HILOGI("Device_priority_manager_GetPreferredDevice_DefaultEmpty_009 enter");
    EXPECT_TRUE(dpm_->GetPreferredDevice().empty());
    HILOGI("Device_priority_manager_GetPreferredDevice_DefaultEmpty_009 end");
}

/**
 * @tc.name: Device_priority_manager_GetOrderList_DefaultEmpty_010
 * @tc.desc: GetOrderList 默认空
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_GetOrderList_DefaultEmpty_010, TestSize.Level1)
{
    HILOGI("Device_priority_manager_GetOrderList_DefaultEmpty_010 enter");
    EXPECT_TRUE(dpm_->GetOrderList().empty());
    HILOGI("Device_priority_manager_GetOrderList_DefaultEmpty_010 end");
}

/**
 * @tc.name: Device_priority_manager_SetPenultimateDevice_EmptyAddr_011
 * @tc.desc: 空地址 SetPenultimateDevice 不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(DevicePriorityManagerTest, Device_priority_manager_SetPenultimateDevice_EmptyAddr_011, TestSize.Level0)
{
    HILOGI("Device_priority_manager_SetPenultimateDevice_EmptyAddr_011 enter");
    dpm_->SetPenultimateDevice("");
    SUCCEED();
    HILOGI("Device_priority_manager_SetPenultimateDevice_EmptyAddr_011 end");
}
}  // namespace bluetooth
}  // namespace OHOS
