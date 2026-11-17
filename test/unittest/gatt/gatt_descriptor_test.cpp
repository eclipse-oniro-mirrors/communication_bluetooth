/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "uuid.h"
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
class GattDescriptorTest : public testing::Test {
public:
    GattDescriptorTest()
    {}
    ~GattDescriptorTest()
    {}

    int tempData_ = 0;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void GattDescriptorTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}
void GattDescriptorTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    host_ = nullptr;
}
void GattDescriptorTest::SetUp()
{
    tempData_ = 0;
}
void GattDescriptorTest::TearDown()
{}

/*
 * @tc.number: GattDescriptor001
 * @tc.name: GetPermissions
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_GetPermissions, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    EXPECT_EQ(permissions, GattDescriptor(uuid_, permissions).GetPermissions());
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::GetPermissions ends");
}

/*
 * @tc.number: GattDescriptor002
 * @tc.name: GetHandle
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_GetHandle, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle = GattDescriptor(uuid_, permissions).GetHandle();
    EXPECT_EQ(handle, 0);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::GetHandle ends");
}

/*
 * @tc.number: GattDescriptor003
 * @tc.name: GetUuid
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_GetUuid, TestSize.Level1)
{
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    UUID uuid_;
    uuid_ = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    const UUID result = GattDescriptor(uuid_, handle_, permissions).GetUuid();
    bool cmp;
    if (uuid_test.ToString() == result.ToString()) {
        cmp = true;
    } else {
        cmp = false;
    }
    EXPECT_EQ(true, cmp);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::GetUuid ends");
}

/*
 * @tc.number: GattDescriptor004
 * @tc.name: GetCharacteristic
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_GetCharacteristic, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    auto res = GattDescriptor(uuid_, handle_, permissions).GetCharacteristic();
    bool result = false;
    if (res) {
        result = true;
    }
    EXPECT_EQ(false, result);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::GetCharacteristic ends");
}

/*
 * @tc.number: GattDescriptor005
 * @tc.name: GetValue
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_GetValue, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t size = 1256;
    uint8_t *test;
    uint16_t handle_test = 0b0000000000010001;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    size_t size_test = 1212;
    GattDescriptor getvalue_test = GattDescriptor(uuid_test, handle_test, permissions);
    test = getvalue_test.GetValue(&size_test).get();
    uint8_t *res;
    GattDescriptor getvalue = GattDescriptor(uuid_, handle_, permissions);
    res = getvalue.GetValue(&size).get();
    bool result = false;
    if (res == test) {
        result = true;
    }
    EXPECT_EQ(true, result);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::GetValue ends");
}

/*
 * @tc.number: GattDescriptor006
 * @tc.name: SetValue
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_SetValue, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    const uint8_t values = 17;
    size_t length = 16;
    GattDescriptor SetValue = GattDescriptor(uuid_, handle_, permissions);
    SetValue.SetValue(&values, length);
    uint8_t *res = SetValue.GetValue(&length).get();
    bool result = false;
    if (res != nullptr) {
        result = true;
    }
    EXPECT_EQ(true, result);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::SetValue ends");
}

/*
 * @tc.number: GattDescriptor0061
 * @tc.name: SetValue
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_SetValue_002, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t length = 0;
    GattDescriptor SetValue = GattDescriptor(uuid_, handle_, permissions);
    SetValue.SetValue(nullptr, length);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::SetValue ends");
}

/*
 * @tc.number: GattDescriptor0062
 * @tc.name: SetValue
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_SetValue_003, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t length = 16;
    GattDescriptor SetValue = GattDescriptor(uuid_, handle_, permissions);
    SetValue.SetValue(nullptr, length);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::SetValue ends");
}

/*
 * @tc.number: GattDescriptor007
 * @tc.name: operator=
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_operator, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t length = 16;
    const uint8_t values = 17;
    GattDescriptor gattDescriptor = GattDescriptor(uuid_, handle_, permissions);
    gattDescriptor.SetValue(&values, length);

    UUID uuid1_ = UUID::RandomUUID();
    int permissions1 = 28;
    uint16_t handle1_ = 0b0000000000010001;
    size_t length1 = 17;
    const uint8_t values1 = 18;
    GattDescriptor gattDescriptor1 = GattDescriptor(uuid1_, handle1_, permissions1);
    gattDescriptor1.SetValue(&values1, length1);

    gattDescriptor = gattDescriptor1;
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::operator= ends");
}

/*
 * @tc.number: GattDescriptor007
 * @tc.name: operator=
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_operator_002, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t length = 0;
    const uint8_t values = 17;
    GattDescriptor gattDescriptor = GattDescriptor(uuid_, handle_, permissions);
    gattDescriptor.SetValue(&values, length);

    UUID uuid1_ = UUID::RandomUUID();
    int permissions1 = 28;
    uint16_t handle1_ = 0b0000000000010001;
    size_t length1 = 17;
    const uint8_t values1 = 18;
    GattDescriptor gattDescriptor1 = GattDescriptor(uuid1_, handle1_, permissions1);
    gattDescriptor1.SetValue(&values1, length1);

    gattDescriptor = gattDescriptor1;
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::operator= ends");
}

/*
 * @tc.number: GattDescriptor0071
 * @tc.name: operator=
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_operator_003, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t length = 16;
    const uint8_t values = 17;
    GattDescriptor gattDescriptor = GattDescriptor(uuid_, handle_, permissions);
    gattDescriptor.SetValue(&values, length);

    UUID uuid1_ = UUID::RandomUUID();
    int permissions1 = 28;
    uint16_t handle1_ = 0b0000000000010001;
    size_t length1 = 17;
    GattDescriptor gattDescriptor1 = GattDescriptor(uuid1_, handle1_, permissions1);
    gattDescriptor1.SetValue(nullptr, length1);

    gattDescriptor = gattDescriptor1;
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::operator= ends");
}

/*
 * @tc.number: GattDescriptor007
 * @tc.name: GattDescriptor
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_GattDescriptor, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t length = 16;
    const uint8_t values = 17;
    GattDescriptor gattDescriptor = GattDescriptor(uuid_, handle_, permissions);
    gattDescriptor.SetValue(&values, length);

    GattDescriptor gattDescriptor1(gattDescriptor);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::GattDescriptor ends");
}

/*
 * @tc.number: GattDescriptor0071
 * @tc.name: GattDescriptor
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_GattDescriptor_002, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t length = 0;
    const uint8_t values = 17;
    GattDescriptor gattDescriptor = GattDescriptor(uuid_, handle_, permissions);
    gattDescriptor.SetValue(&values, length);

    GattDescriptor gattDescriptor1(gattDescriptor);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::GattDescriptor ends");
}

/*
 * @tc.number: GattDescriptor0071
 * @tc.name: GattDescriptor
*/
HWTEST_F(GattDescriptorTest, GattDescriptor_UnitTest_GattDescriptor_003, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 27;
    uint16_t handle_ = 0b0000000000010000;
    size_t length =16;
    GattDescriptor gattDescriptor = GattDescriptor(uuid_, handle_, permissions);
    gattDescriptor.SetValue(nullptr, length);

    GattDescriptor gattDescriptor1(gattDescriptor);
    HILOGI("GattDescriptor::GattDescriptor ends");
    HILOGI("GattDescriptor::GattDescriptor ends");
}

}  // namespace Bluetooth
}  // namespace OHOS
