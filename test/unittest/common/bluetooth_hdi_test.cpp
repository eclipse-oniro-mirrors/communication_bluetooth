/*
 * Copyright (c) 2012 Huawei Device Co., Ltd. All rights reserved.
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
#include "bluetooth_hdi.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

class BluetoothHdiTest : public testing::Test {
public:
    BluetoothHdiTest() {}
    ~BluetoothHdiTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BluetoothHdiTest::SetUpTestCase(void)
{
}
void BluetoothHdiTest::TearDownTestCase(void)
{
}
void BluetoothHdiTest::SetUp()
{
}
void BluetoothHdiTest::TearDown()
{
}

/*
 * @tc.number: ModuleTest_HdiSendHciPacket_001
 * @tc.name: HdiSendHciPacket
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHdiTest, ModuleTest_HdiSendHciPacket_001, TestSize.Level1)
{
    HILOGI("ModuleTest_HdiSendHciPacket_001 start");

    EXPECT_EQ(HdiSendHciPacket(PACKET_TYPE_UNKNOWN, nullptr), TRANSPORT_ERROR);

    BtPacket packet;
    EXPECT_EQ(HdiSendHciPacket(PACKET_TYPE_UNKNOWN, &packet), INITIALIZATION_ERROR);

    HILOGI("ModuleTest_HdiSendHciPacket_001 end");
}

}  // namespace Bluetooth
}  // namespace OHOS