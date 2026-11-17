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
#include "raw_address.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {

class RawAddressTest : public testing::Test {
public:
    RawAddressTest() {}
    ~RawAddressTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    std::unique_ptr<RawAddress> rawAddress;
};

void RawAddressTest::SetUpTestCase(void)
{
}
void RawAddressTest::TearDownTestCase(void)
{
}
void RawAddressTest::SetUp()
{
    rawAddress = std::make_unique<RawAddress>();
}
void RawAddressTest::TearDown()
{
    rawAddress = nullptr;
}

/*
 * @tc.number: ModuleTest_ConvertToString_001
 * @tc.name: ConvertToString
 * @tc.desc: name :
 */
HWTEST_F(RawAddressTest, ModuleTest_ConvertToString_001, TestSize.Level1)
{
    HILOGI("ModuleTest_ConvertToString_001 start");
    uint8_t src = 1;
    RawAddress address1 = rawAddress->ConvertToString(&src, 6);
    RawAddress address2 = rawAddress->ConvertToString(&src, 1);
    EXPECT_FALSE(address1 == address2);
    HILOGI("ModuleTest_ConvertToString_001 end");
}

/*
 * @tc.number: ModuleTest_ConvertToUint8_001
 * @tc.name: ConvertToUint8
 * @tc.desc: name :
 */
HWTEST_F(RawAddressTest, ModuleTest_ConvertToUint8_001, TestSize.Level1)
{
    HILOGI("ModuleTest_ConvertToUint8_001 start");

    rawAddress->ConvertToUint8(nullptr, 6);

    uint8_t dst = 0;
    rawAddress->address_ = "00";
    rawAddress->ConvertToUint8(&dst, 6);

    rawAddress->address_ = "11:22:33:44:55:66";
    rawAddress->ConvertToUint8(&dst, 1);
    EXPECT_NE(rawAddress->GetAddress(), "");
    HILOGI("ModuleTest_ConvertToUint8_001 end");
}

/*
 * @tc.number: ModuleTest_SetAddressType_001
 * @tc.name: SetAddressType
 * @tc.desc: test SetAddressType and GetAddressType;
 */
HWTEST_F(RawAddressTest, ModuleTest_SetAddressType_001, TestSize.Level1)
{
    HILOGI("ModuleTest_SetAddressType_001 start");
    int32_t addrType = rawAddress->GetAddressType();
    EXPECT_EQ(addrType, UNSET_ADDRESS); // default addressType_ should be UNSET

    rawAddress->SetAddressType(VIRTUAL_ADDRESS);
    addrType = rawAddress->GetAddressType();
    EXPECT_EQ(addrType, VIRTUAL_ADDRESS);

    rawAddress->SetAddressType(REAL_ADDRESS);
    addrType = rawAddress->GetAddressType();
    EXPECT_EQ(addrType, REAL_ADDRESS);
    HILOGI("ModuleTest_SetAddressType_001 end");
}

}  // namespace Bluetooth
}  // namespace OHOS