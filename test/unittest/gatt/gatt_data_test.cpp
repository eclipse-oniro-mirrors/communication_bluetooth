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
#include <gtest/gtest.h>

#include "bluetooth_host.h"
#include "gatt_data.h"
#include "bluetooth_log.h"
#include "bt_uuid.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
class GattDataTest : public testing::Test {
public:
    static void SetUpTestCase()
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
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase");
        host_ = nullptr;
    }
    void SetUp()
    {
    }

    void TearDown()
    {
    }

public:
};
/**
 * @tc.name: Descriptor_00100
 * @tc.desc: Verify Descriptor function.
 * @tc.type: FUNC
 */
HWTEST_F(GattDataTest, BLE_ModuleTest_Descriptor_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_Descriptor_00100 start");
    std::string testString = "00000000-0000-1000-8000-00805F9B34FB";
    Uuid uuid = Uuid::ConvertFromString(testString);
    uint8_t *testData = new uint8_t(3);
    Descriptor descriptorTest = Descriptor(uuid, 0x00, 0x00, nullptr, 0x00);
    Descriptor descriptorTestt = Descriptor(descriptorTest);
    Descriptor descriptorTest1 = Descriptor(uuid, 0x00, 0x00, nullptr, 0x01);
    Descriptor descriptorTestt1 = Descriptor(descriptorTest1);
    Descriptor descriptorTest2 = Descriptor(uuid, 0x00, 0x00, testData, 0x00);
    Descriptor descriptorTestt2 = Descriptor(descriptorTest2);
    Descriptor descriptorTest3 = Descriptor(uuid, 0x00, 0x00, testData, 0x01);
    Descriptor descriptorTestt3 = Descriptor(descriptorTest3);
    EXPECT_EQ(descriptorTest3.uuid_, uuid);
    delete testData;
    testData = nullptr;
    HILOGI("BLE_ModuleTest_Descriptor_00100 end");
}

/**
 * @tc.name: Characteristic_00100
 * @tc.desc: Verify Characteristic function.
 * @tc.type: FUNC
 */
HWTEST_F(GattDataTest, BLE_ModuleTest_Characteristic_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_Characteristic_00100 start");
    std::string testString = "00000000-0000-1000-8000-00805F9B34FB";
    Uuid uuid = Uuid::ConvertFromString(testString);
    uint8_t *testData = new uint8_t(3);
    Characteristic characteristic = Characteristic(uuid, 0x00, 0x00, 0x00, nullptr, 0x00);
    Characteristic characteristicTest = Characteristic(characteristic);
    Characteristic characteristic1 = Characteristic(uuid, 0x00, 0x00, 0x00, nullptr, 0x01);
    Characteristic characteristicTest1 = Characteristic(characteristic1);
    Characteristic characteristic2 = Characteristic(uuid, 0x00, 0x00, 0x00, testData, 0x00);
    Characteristic characteristicTest2 = Characteristic(characteristic2);
    Characteristic characteristic3 = Characteristic(uuid, 0x00, 0x00, 0x00, testData, 0x01);
    Characteristic characteristicTest3 = Characteristic(characteristic3);
    EXPECT_EQ(characteristic3.uuid_, uuid);
    delete testData;
    testData = nullptr;
    HILOGI("BLE_ModuleTest_Characteristic_00100 end");
}

/**
 * @tc.name: SetValue_00100
 * @tc.desc: Verify SetValue function.
 * @tc.type: FUNC
 */
HWTEST_F(GattDataTest, BLE_ModuleTest_SetValue_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetValue_00100 start");
    std::string testString = "00000000-0000-1000-8000-00805F9B34FB";
    Uuid uuid = Uuid::ConvertFromString(testString);
    uint8_t *testData = new uint8_t(3);
    Characteristic characteristic = Characteristic(uuid, 0x00, 0x00, 0x00, nullptr, 0x00);
    characteristic.SetValue(nullptr, 0);
    characteristic.SetValue(testData, 1);
    EXPECT_EQ(characteristic.uuid_, uuid);
    delete testData;
    testData = nullptr;
    HILOGI("BLE_ModuleTest_SetValue_00100 end");
}

/**
 * @tc.name: SetValue_00100
 * @tc.desc: Verify EqualsOperator function.
 * @tc.type: FUNC
 */
HWTEST_F(GattDataTest, BLE_ModuleTest_EqualsOperator_001, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_EqualsOperator_001 start");
    RawAddress addr1 = RawAddress("00:11:22:33:44:55");
    RawAddress addr2 = RawAddress("11:22:33:44:55:66");
    // 测试相等的情况
    GattDevice device1(addr1, 1, 1);
    GattDevice device2(addr1, 1, 1);
    EXPECT_TRUE(device1 == device2);  // 两个对象完全相同，应返回true

    // 测试`addr_`相同，`transport_`不同
    GattDevice device3(addr1, 1, 2);
    EXPECT_FALSE(device1 == device3);  // `addr_`相同，`transport_`不同，应返回false

    // 测试`addr_`不同，`transport_`相同
    GattDevice device4(addr2, 1, 1);
    EXPECT_FALSE(device1 == device4);  // `addr_`不同，`transport_`相同，应返回false

    // 测试`addr_`和`transport_`都不同
    GattDevice device5(addr2, 2, 1);
    EXPECT_FALSE(device1 == device5);  // `addr_`和`transport_`都不同，应返回false
    HILOGI("BLE_ModuleTest_EqualsOperator_001 end");
}

/**
 * @tc.name: SetValue_00100
 * @tc.desc: Verify LessThanOperator function.
 * @tc.type: FUNC
 */
HWTEST_F(GattDataTest, BLE_ModuleTest_LessThanOperator_001, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_LessThanOperator_001 start");
    RawAddress addr1 = RawAddress("00:11:22:33:44:55");
    RawAddress addr2 = RawAddress("11:22:33:44:55:66");
    // 测试`addr_`小于，`transport_`相同
    GattDevice device1(addr1, 1, 1);
    GattDevice device2(addr2, 1, 1);
    EXPECT_TRUE(device1 < device2);  // addr1 < addr2，且transport相同，应返回true

    // 测试`addr_`小于，`transport_`不同
    GattDevice device3(addr1, 1, 2);
    EXPECT_FALSE(device1 < device3);  // addr1 < addr1，但transport不同，应返回false

    // 测试`addr_`大于，`transport_`相同
    GattDevice device4(addr2, 1, 1);
    EXPECT_FALSE(device2 < device1);  // addr2 > addr1，应返回false

    // 测试`addr_`等于，`transport_`相同
    GattDevice device5(addr1, 1, 1);
    EXPECT_FALSE(device1 < device5);  // addr相等，应返回false

    // 测试`addr_`等于，`transport_`不同
    GattDevice device6(addr1, 2, 1);
    EXPECT_FALSE(device1 < device6);  // addr相等，但transport不同，应返回false
    HILOGI("BLE_ModuleTest_LessThanOperator_001 end");
}

HWTEST_F(GattDataTest, BLE_ModuleTest_SetState, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetState start");
    RawAddress addr = RawAddress("00:11:22:33:44:55");
    GattDevice device(addr, 1, 1);
    device.SetState(1);
    EXPECT_EQ(device.connectState_, 1);
    HILOGI("BLE_ModuleTest_SetState end");
}

}  // namespace bluetooth
}  // namespace OHOS