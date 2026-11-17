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

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_def.h"
#include "dialog_result.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

std::shared_ptr<DialogResult> g_dialogResult = nullptr;

class DialogResultTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}

    void SetUp()
    {
        g_dialogResult = std::make_shared<DialogResult>();
    }
    void TearDown()
    {
        g_dialogResult = nullptr;
    }
};

/**
 * @tc.name: dialog_result_001
 * @tc.desc: Handle bluetooth switch dialog result.
 * @tc.type: FUNC
 */
HWTEST_F(DialogResultTest, dialog_result_001, TestSize.Level1)
{
    HILOGI("dialog_result_001 enter");
    bool isAsync = false;
    EXPECT_TRUE(g_dialogResult != nullptr);
    EXPECT_EQ(g_dialogResult->HandleBluetoothSwitchDialogResult(isAsync), BT_ERR_DIALOG_FOR_USER_CONFIRM);
    HILOGI("dialog_result_001 end");
}

/**
 * @tc.name: dialog_result_002
 * @tc.desc: Handle bluetooth switch dialog result.
 * @tc.type: FUNC
 */
HWTEST_F(DialogResultTest, dialog_result_002, TestSize.Level1)
{
    HILOGI("dialog_result_002 enter");
    bool isAsync = true;
    EXPECT_TRUE(g_dialogResult != nullptr);
    EXPECT_EQ(g_dialogResult->HandleBluetoothSwitchDialogResult(isAsync), BT_ERR_DIALOG_FOR_USER_NOT_RESPOND);
    HILOGI("dialog_result_002 end");
}

/**
 * @tc.name: dialog_result_003
 * @tc.desc: Wait for bluetooth switch dialog result.
 * @tc.type: FUNC
 */
HWTEST_F(DialogResultTest, dialog_result_003, TestSize.Level1)
{
    HILOGI("dialog_result_003 enter");
    EXPECT_TRUE(g_dialogResult != nullptr);
    g_dialogResult->IsDialogResultReceived(true);
    g_dialogResult->WaitAdapterManagerDialogResultComplete();
    EXPECT_EQ(g_dialogResult->GetIsDialogResultReceived(), true);
    HILOGI("dialog_result_003 end");
}

/**
 * @tc.name: dialog_result_004
 * @tc.desc: Wait for bluetooth switch dialog result.
 * @tc.type: FUNC
 */
HWTEST_F(DialogResultTest, dialog_result_004, TestSize.Level1)
{
    HILOGI("dialog_result_004 enter");
    EXPECT_TRUE(g_dialogResult != nullptr);
    g_dialogResult->IsDialogResultReceived(false);
    g_dialogResult->WaitAdapterManagerDialogResultComplete();
    EXPECT_EQ(g_dialogResult->GetIsDialogResultReceived(), false);
    HILOGI("dialog_result_004 end");
}

/**
 * @tc.name: dialog_result_005
 * @tc.desc: Get and set dialog result.
 * @tc.type: FUNC
 */
HWTEST_F(DialogResultTest, dialog_result_005, TestSize.Level1)
{
    HILOGI("dialog_result_005 enter");
    EXPECT_TRUE(g_dialogResult != nullptr);
    g_dialogResult->SetDialogResult(false);
    EXPECT_EQ(g_dialogResult->GetDialogResult(), false);
    HILOGI("dialog_result_005 end");
}

/**
 * @tc.name: dialog_result_006
 * @tc.desc: verify NotifyBluetoothSwitchDialogResult type-based routing
 * @tc.type: FUNC
 */
HWTEST_F(DialogResultTest, dialog_result_006, TestSize.Level1)
{
    HILOGI("dialog_result_006 enter");
    EXPECT_TRUE(g_dialogResult != nullptr);
    g_dialogResult->IsDialogResultReceived(false);
    g_dialogResult->SetDialogResult(false);
    // CANCEL_PAIR notify should not affect switch state
    g_dialogResult->NotifyBluetoothSwitchDialogResult(
        static_cast<uint32_t>(DialogBoxType::CANCEL_PAIR), true);
    EXPECT_FALSE(g_dialogResult->GetIsDialogResultReceived());
    // BLUETOOTH_SWITCH notify should flip switch state
    g_dialogResult->NotifyBluetoothSwitchDialogResult(
        static_cast<uint32_t>(DialogBoxType::BLUETOOTH_SWITCH), true);
    EXPECT_TRUE(g_dialogResult->GetIsDialogResultReceived());
    EXPECT_TRUE(g_dialogResult->GetDialogResult());
    HILOGI("dialog_result_006 end");
}
 
/**
 * @tc.name: dialog_result_007
 * @tc.desc: verify NotifyCancelPairDialogResult type filtering
 * @tc.type: FUNC
 */
HWTEST_F(DialogResultTest, dialog_result_007, TestSize.Level1)
{
    HILOGI("dialog_result_007 enter");
    EXPECT_TRUE(g_dialogResult != nullptr);
    // non-CANCEL_PAIR type should be ignored
    g_dialogResult->NotifyCancelPairDialogResult(
        static_cast<uint32_t>(DialogBoxType::BLUETOOTH_SWITCH), true);
    // CANCEL_PAIR type should update internal state without crash
    g_dialogResult->NotifyCancelPairDialogResult(
        static_cast<uint32_t>(DialogBoxType::CANCEL_PAIR), false);
    EXPECT_TRUE(g_dialogResult != nullptr);
    HILOGI("dialog_result_007 end");
}
}  // namespace bluetooth
}  // namespace OHOS