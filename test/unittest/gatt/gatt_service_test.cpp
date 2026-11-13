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

#include "bluetooth_gatt_service.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "uuid.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
const int TIME = 2;
static BluetoothHost *host_;
class GattServiceTest : public testing::Test {
public:
    GattServiceTest()
    {}
    ~GattServiceTest()
    {}

    int tempData_ = 0;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GattServiceTest::SetUpTestCase(void)
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
void GattServiceTest::TearDownTestCase(void)
{
    host_ = nullptr;
}
void GattServiceTest::SetUp()
{
    tempData_ = 0;
}

void GattServiceTest::TearDown()
{}

/*
 * @tc.number: GattService001
 * @tc.name: IsPrimary
*/
HWTEST_F(GattServiceTest, GattService_UnitTest_IsPrimary, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    EXPECT_EQ(true, GattService(uuid_, serviceType_).IsPrimary());
    HILOGI("GattService::GattService ends");
    HILOGI("GattService::IsPrimary ends");
}

/*
 * @tc.number: GattService001
 * @tc.name: IsPrimary
*/
HWTEST_F(GattServiceTest, GattService_UnitTest_GetHandle_, TestSize.Level1)
{
    UUID uuid_;
    uuid_ = UUID::FromString("00001820-0000-1000-8000-00805F9B34FB");
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    uint16_t result = GattService(uuid_, handle_, endHandle_, serviceType_).GetHandle();
    EXPECT_EQ(16, result);
    HILOGI("GattService::GattService ends");
    HILOGI("GattService::GetHandle ends");
}

/*
 * @tc.number: GattService001
 * @tc.name: IsPrimary
*/
HWTEST_F(GattServiceTest, GattService_UnitTest_GetUuid, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    const UUID result = GattService(uuid_, handle_, endHandle_, serviceType_).GetUuid();
    bool cmp;
    if (uuid_.ToString() == result.ToString()) {
        cmp = true;
    } else {
        cmp = false;
    }
    EXPECT_EQ(true, cmp);

    int ret = uuid_.CompareTo(result);
    EXPECT_EQ(ret, 0);

    int ret1 = uuid_.GetLeastSignificantBits();
    EXPECT_NE(ret1, 0);

    int ret2 = uuid_.GetMostSignificantBits();
    EXPECT_NE(ret2, 0);

    bool ret3 = uuid_.operator<(result);
    EXPECT_FALSE(ret3);
    HILOGI("GattService::GattService ends");
    HILOGI("GattService::GetUuid ends");
}

/*
 * @tc.number: GattService001
 * @tc.name: IsPrimary
*/
HWTEST_F(GattServiceTest, GattService_UnitTest_AddService, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    std::vector<std::reference_wrapper<GattService>> includedservices;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattService addservice = GattService(uuid_test, serviceType_);
    GattService service = GattService(uuid_, handle_, endHandle_, serviceType_);
    service.AddService(addservice);
    includedservices = service.GetIncludedServices();
    bool result = false;
    if (includedservices.size() == 0) {
        result = true;
    }
    EXPECT_EQ(false, result);
    HILOGI("GattService::GattService ends");
    HILOGI("GattService::AddService ends");
}

/*
 * @tc.number: GattService002
 * @tc.name: GetIncludedServices
*/
HWTEST_F(GattServiceTest, GattService_UnitTest_GetIncludedServices, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    std::vector<std::reference_wrapper<GattService>> includedservices;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattService addservice = GattService(uuid_test, serviceType_);
    GattService service = GattService(uuid_, handle_, endHandle_, serviceType_);
    service.AddService(addservice);
    includedservices = service.GetIncludedServices();
    bool result = false;
    if (includedservices.size() == 1) {
        result = true;
    }
    EXPECT_EQ(true, result);
    HILOGI("GattService::GattService ends");
    HILOGI("GattService::GetIncludedServices ends");
}

/*
 * @tc.number: GattService003
 * @tc.name: AddCharacteristic
*/
HWTEST_F(GattServiceTest, GattService_UnitTest_AddCharacteristic, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    int permissions = 27;
    int properties = 26;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattCharacteristic characteristic = GattCharacteristic(uuid_test, permissions, properties);

    GattService addcharacter = GattService(uuid_, handle_, endHandle_, serviceType_);
    addcharacter.AddCharacteristic(characteristic);
    auto test = addcharacter.GetCharacteristic(uuid_test);
    bool result = false;
    if (test == nullptr) {
        result = true;
    }
    EXPECT_EQ(false, result);
    HILOGI("GattService::GattService ends");
    HILOGI("GattService::AddCharacteristic ends");
}

/*
 * @tc.number: GattService004
 * @tc.name: GetCharacteristic
*/
HWTEST_F(GattServiceTest, GattService_UnitTest_GetCharacteristic, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    UUID uuid_test1;
    uuid_test1 = UUID::FromString("01001810-0000-1000-8000-00805F9B34FB");
    int permissions = 27;
    int properties = 26;
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    GattCharacteristic characteristic = GattCharacteristic(uuid_test, permissions, properties);
    GattService getcharacter = GattService(uuid_, handle_, endHandle_, serviceType_);
    getcharacter.AddCharacteristic(characteristic);
    auto test = getcharacter.GetCharacteristic(uuid_test);
    getcharacter.GetCharacteristic(uuid_test1);
    bool result = false;
    if (test == nullptr) {
        result = true;
    }
    EXPECT_EQ(false, result);

    HILOGI("GattService::GattService ends");
    HILOGI("GattService::GetCharacteristic ends");
}

/*
 * @tc.number: GattService005
 * @tc.name: GetCharacteristics
*/
HWTEST_F(GattServiceTest, GattService_UnitTest_GetCharacteristics, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    int permissions = 27;
    int properties = 26;
    GattCharacteristic characteristic = GattCharacteristic(uuid_test, permissions, properties);

    GattService getcharacters = GattService(uuid_, handle_, endHandle_, serviceType_);
    getcharacters.AddCharacteristic(characteristic);

    std::vector<GattCharacteristic> res;
    res = getcharacters.GetCharacteristics();
    bool result = false;
    if (res.size() == 0) {
        result = true;
    }
    EXPECT_EQ(false, result);
    HILOGI("GattService::GattService ends");
    HILOGI("GattService::GetCharacteristics ends");
}

}  // namespace Bluetooth
}  // namespace OHOS
