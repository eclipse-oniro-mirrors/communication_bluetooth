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

#include "smart_call_transfer_controller.h"

#include <gtest/gtest.h>

#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

// IN/OUT 常量值与 DataShare 侧写入值契约对齐，防误改。
// 状态机与判重逻辑（STATUS_UNSET/STATUS_UNKNOWN 区分）需引入
// CarProfileService/Adapter 桩后补充。
class SmartCallTransferControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("SmartCallTransferControllerTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("SmartCallTransferControllerTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("SmartCallTransferControllerTest::SetUp start");
    }
    void TearDown() override
    {
        HILOGI("SmartCallTransferControllerTest::TearDown start");
    }
};

/**
 * @tc.name: Smart_call_transfer_controller_StatusConstants_001
 * @tc.desc: TRANSFER_STATUS_IN/OUT 常量值（DataShare 契约对齐）
 * @tc.type: FUNC
 */
HWTEST_F(SmartCallTransferControllerTest, Smart_call_transfer_controller_StatusConstants_001, TestSize.Level0)
{
    HILOGI("Smart_call_transfer_controller_StatusConstants_001 enter");
    EXPECT_EQ(SmartCallTransferController::TRANSFER_STATUS_IN, 6);
    EXPECT_EQ(SmartCallTransferController::TRANSFER_STATUS_OUT, 7);
    HILOGI("Smart_call_transfer_controller_StatusConstants_001 end");
}

/**
 * @tc.name: Smart_call_transfer_controller_StatusConstants_Distinct_002
 * @tc.desc: IN/OUT 常量互不相同
 * @tc.type: FUNC
 */
HWTEST_F(SmartCallTransferControllerTest, Smart_call_transfer_controller_StatusConstants_Distinct_002, TestSize.Level0)
{
    HILOGI("Smart_call_transfer_controller_StatusConstants_Distinct_002 enter");
    EXPECT_NE(SmartCallTransferController::TRANSFER_STATUS_IN,
              SmartCallTransferController::TRANSFER_STATUS_OUT);
    HILOGI("Smart_call_transfer_controller_StatusConstants_Distinct_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS
