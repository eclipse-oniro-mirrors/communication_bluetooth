/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "bluetooth_host.h"
#include "bluetooth_dialog.h"
#include "dialog_pair.h"
#include "dialog_switch.h"
#include "dialog_cancel_pair.h"
#include "class_creator.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
const int TIME = 2;
bool g_isAllowPairDialog = true;
static BluetoothHost *host_;
void *BluetoothDialogCreate(void)
{
    return new BluetoothDialog();
}
void SetIsAllowPairDialogFlag(bool isAllowed)
{
    g_isAllowPairDialog = isAllowed;
}

bool IsAllowPairDialogHook(void)
{
    return g_isAllowPairDialog;
}
class BluetoothDialogTest : public testing::Test {
public:
    BluetoothDialogTest() = default;
    ~BluetoothDialogTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void) {};
    void SetUp() {};
    void TearDown() {};
};

void BluetoothDialogTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBrEnable = %{public}d", isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
    ClassFactory::RegisterClass("BluetoothDialog", BluetoothDialogCreate);

    HILOGI("SetUpTestCase end");
}

/**
 * @tc.name: dialog_requestpair_001
 * @tc.desc: Verify the Request pair function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDialogTest, dialog_requestpair_001, TestSize.Level1)
{
    HILOGI("dialog_requestpair_001 enter");
    int reqType = 4;
    int number = 0;
    bool result = true;
    RawAddress device("24:24:35:F1:16:A1");
    DialogPair::RequestBluetoothPairDialog(RawAddress("FF:FF:FF:FF:FF:FF"), reqType, number);
    DialogPair::RequestBluetoothPairDialog(RawAddress("00:00:00:00:00:00"), reqType, number);
    DialogPair::RequestBluetoothPairDialog(device, reqType, number);
    HILOGI("dialog_requestpair_001 end");
}

/**
 * @tc.name: dialog_requestswitch_001
 * @tc.desc: Verify the Request switch function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDialogTest, dialog_requestswitch_001, TestSize.Level1)
{
    HILOGI("dialog_requestswitch_001 enter");
    DialogSwitch::RequestBluetoothSwitchDialog(ENABLE_BLUETOOTH);
    DialogSwitch::RequestBluetoothSwitchDialog(DISABLE_BLUETOOTH);
    HILOGI("dialog_requestswitch_001 end");
}

/**
 * @tc.name: dialog_RequestAuthDialog_001
 * @tc.desc: RequestAuthDialog
 * @tc.type: FUNC
 */
static void timeOutTest()
{
    HILOGI("enter");
}

HWTEST_F(BluetoothDialogTest, dialog_RequestAuthDialog_001, TestSize.Level1)
{
    HILOGI("dialog_RequestAuthDialog_001 enter");
    DialogInfo dialog{"00:00:00:00:00:00", AUTO_PLAY_AUTH_DIALOG, 0, nullptr};
    int res = BluetoothDialog::RequestAuthDialog(dialog);
    EXPECT_TRUE(res);

    res = BluetoothDialog::DismissCurAndShowNext();
    EXPECT_FALSE(res);

    DialogInfo dialog1{"00:00:00:00:00:00", PBAP_AUTH_DIALOG, 0, nullptr};
    res = BluetoothDialog::RequestAuthDialog(dialog1);
    EXPECT_TRUE(res);

    std::shared_ptr<utility::Timer> timer = std::make_shared<utility::Timer>(&timeOutTest);
    DialogInfo dialog2{"00:00:00:00:00:00", MAP_AUTH_DIALOG, 0, timer};
    res = BluetoothDialog::RequestAuthDialog(dialog2);
    EXPECT_TRUE(res);
    sleep(TIME);
    res = BluetoothDialog::DismissCurAndShowNext();
    EXPECT_TRUE(res);
    HILOGI("dialog_RequestAuthDialog_001 end");
}

/**
 * @tc.name: dialog_cancel_pair_buildstartcommand_001
 * @tc.desc: verify BuildStartCommand produces expected JSON fields
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDialogTest, dialog_cancel_pair_buildstartcommand_001, TestSize.Level1)
{
    HILOGI("dialog_cancel_pair_buildstartcommand_001 enter");
    RawAddress device("11:22:33:44:55:66");
    std::string cmd = DialogCancelPair::BuildStartCommand(device, "ExampleApp");
    EXPECT_FALSE(cmd.empty());
    EXPECT_NE(cmd.find("\"deviceId\":\"11:22:33:44:55:66\""), std::string::npos);
    EXPECT_NE(cmd.find("\"appName\":\"ExampleApp\""), std::string::npos);
    EXPECT_NE(cmd.find("sysDialog/common"), std::string::npos);
    HILOGI("dialog_cancel_pair_buildstartcommand_001 end");
}
 
/**
 * @tc.name: dialog_cancel_pair_request_001
 * @tc.desc: verify RequestCancelPairDialog
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDialogTest, dialog_cancel_pair_request_001, TestSize.Level1)
{
    HILOGI("dialog_cancel_pair_request_001 enter");
    RawAddress device("11:22:33:44:55:66");
    EXPECT_FALSE(DialogCancelPair::RequestCancelPairDialog(device));
    HILOGI("dialog_cancel_pair_request_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS