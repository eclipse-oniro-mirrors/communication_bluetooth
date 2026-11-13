
/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
 
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_os_account.h"
 
using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace Bluetooth {
class BluetoothOsAccountTest : public testing::Test {
public:
    BluetoothOsAccountTest() = default;
    ~BluetoothOsAccountTest() = default;
 
    static void SetUpTestSuite(void) {};
    static void TearDownTestSuite(void) {};
    void SetUp()
    {
        HILOGI("SetUpTestCase start");
        HILOGI("SetUpTestCase end");
    };
    void TearDown() {};
};
 
/**
 * @tc.name: getaccountId_001
 * @tc.desc: getaccountId
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothOsAccountTest, getaccountId_001, TestSize.Level1)
{
    HILOGI("getaccountId_001 enter");
    EXPECT_NE(bluetooth::BluetoothOsAccount::GetActiveOsAccountId(), -1);
    HILOGI("getaccountId_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS