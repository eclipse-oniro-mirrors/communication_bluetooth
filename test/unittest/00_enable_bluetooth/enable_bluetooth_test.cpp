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
#include "bluetooth_a2dp_src.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace std;

constexpr int TIME = 4;

static BluetoothHost *host_;

class EnableBluetoothTest : public testing::Test {
public:
    EnableBluetoothTest()
    {}
    ~EnableBluetoothTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetRandomAddr(BluetoothRemoteDevice &device);
};

void EnableBluetoothTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();
    host_->EnableBle();
    host_->EnableBt();
    sleep(TIME);
    HILOGI("SetUpTestCase end");
}

void EnableBluetoothTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void EnableBluetoothTest::SetUp()
{
}

void EnableBluetoothTest::TearDown()
{
}

/*
 * @tc.number: HID_ModuleTest_EnableBluetooth_001
 * @tc.name: EnableBluetooth
 * @tc.desc: Requires an empty test case for turning on bluetooth
 */
HWTEST_F(EnableBluetoothTest, HID_ModuleTest_EnableBluetooth_00100, TestSize.Level1)
{
    EXPECT_TRUE(host_ != nullptr);
}

}  // namespace Bluetooth
}  // namespace OHOS