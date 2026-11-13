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

#include "ble_scanner_state_machine.h"
#include "ble_service_data.h"
#include "bluetooth_ble_advertiser_server.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_ble_central_manager_server.h"
#include "bluetooth_ble_filter_matcher.h"
#include "bluetooth_ble_scan_result.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace OHOS;
using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;

static BluetoothHost *host_;
constexpr int TIME = 2;

class BluetoothBleScanFileterTest : public testing::Test {
public:
    BluetoothBleScanFileterTest() = default;
    ~BluetoothBleScanFileterTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BluetoothBleScanFileterTest::SetUpTestCase(void)
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

void BluetoothBleScanFileterTest::TearDownTestCase(void)
{
    host_ = nullptr;
}

void BluetoothBleScanFileterTest::SetUp()
{
}

void BluetoothBleScanFileterTest::TearDown()
{
}

/**
 * @tc.number: BLE_ModuleTest_MatchesAddr_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_MatchesAddr_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_MatchesAddr_00100 start");

    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    BluetoothBleScanResult result;
    FilterCheckState matchResult;

    result.SetPeripheralDevice(OHOS::bluetooth::RawAddress("B8:8E:82:B1:7D:DE"));
    //all empty
    matchResult = BluetoothBleFilterMatcher::MatchesAddress(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //filter empty
    result.SetPeripheralDevice(OHOS::bluetooth::RawAddress("B8:8E:82:B1:7D:DE"));
    matchResult = BluetoothBleFilterMatcher::MatchesAddress(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result empty
    result.SetPeripheralDevice(OHOS::bluetooth::RawAddress(""));
    filterImpl.SetDeviceId("B8:8E:82:B1:7D:DE");
    matchResult = BluetoothBleFilterMatcher::MatchesAddress(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //same address
    filterImpl.SetDeviceId("B8:8E:82:B1:7D:DE");
    result.SetPeripheralDevice(OHOS::bluetooth::RawAddress("B8:8E:82:B1:7D:DE"));
    matchResult = BluetoothBleFilterMatcher::MatchesAddress(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //different address
    filterImpl.SetDeviceId("B8:8E:82:B1:7D:DD");
    result.SetPeripheralDevice(OHOS::bluetooth::RawAddress("B8:8E:82:B1:7D:DE"));
    matchResult = BluetoothBleFilterMatcher::MatchesAddress(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    HILOGI("BLE_ModuleTest_MatchesAddr_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_MatchesName_00200
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_MatchesName_00200, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_MatchesName_00200 start");

    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    BluetoothBleScanResult result;
    FilterCheckState matchResult;

    //all empty
    matchResult = BluetoothBleFilterMatcher::MatchesName(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //filter empty
    result.SetName("P50Pro");
    matchResult = BluetoothBleFilterMatcher::MatchesName(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result empty
    filterImpl.SetName("P50Pro");
    BluetoothBleScanResult result1;
    matchResult = BluetoothBleFilterMatcher::MatchesName(filterImpl, result1);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //same name
    filterImpl.SetName("P50Pro");
    result.SetName("P50Pro");
    matchResult = BluetoothBleFilterMatcher::MatchesName(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //different name
    filterImpl.SetName("P40Pro");
    result.SetName("P50Pro");
    matchResult = BluetoothBleFilterMatcher::MatchesName(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);
    HILOGI("BLE_ModuleTest_MatchesName_00200 end");
}

/**
 * @tc.number: BLE_ModuleTest_MatchesServiceUUid_00300
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_MatchesServiceUuids_00300, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_MatchesServiceUuids_00300 start");

    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    BluetoothBleScanResult result;
    FilterCheckState matchResult;
    //result 0  mask 0  filter 0  pass
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 1 mask 0  filter 0  pass
    OHOS::bluetooth::Uuid resultUuid;
    resultUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    result.AddServiceUuid(resultUuid);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 0 mask 1  filter 0  pass
    BluetoothBleScanResult result1;
    OHOS::bluetooth::Uuid filterServiceUuidMask;
    filterServiceUuidMask = OHOS::bluetooth::Uuid::ConvertFromString("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    filterImpl.SetServiceUuidMask(filterServiceUuidMask);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl, result1);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 0 mask 0  filter 1  fail
    OHOS::bluetooth::Uuid filterServiceUuid;
    filterServiceUuid = OHOS::bluetooth::Uuid::ConvertFromString("12345678-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    filterImpl.SetServiceUuid(filterServiceUuid);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //result 1   mask 1   filter 0  pass
    resultUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    result.AddServiceUuid(resultUuid);
    OHOS::bluetooth::BleScanFilterImpl filterImpl1;
    filterServiceUuidMask = OHOS::bluetooth::Uuid::ConvertFromString("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    filterImpl1.SetServiceUuidMask(filterServiceUuidMask);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl1, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 有   mask 没有  filter 有    比较值相同
    resultUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    result.AddServiceUuid(resultUuid);
    OHOS::bluetooth::BleScanFilterImpl filterImpl2;
    filterServiceUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    filterImpl2.SetServiceUuid(filterServiceUuid);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl2, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 有   mask 没有  filter 有    比较值不同
    resultUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    result.AddServiceUuid(resultUuid);
    OHOS::bluetooth::BleScanFilterImpl filterImpl3;
    filterServiceUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-A0805F9B34FB");
    filterImpl3.SetServiceUuid(filterServiceUuid);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl3, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //result 没有 mask 有    filter 有    fail
    BluetoothBleScanResult result2;
    filterServiceUuidMask = OHOS::bluetooth::Uuid::ConvertFromString("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    filterImpl.SetServiceUuidMask(filterServiceUuidMask);
    filterServiceUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    filterImpl.SetServiceUuid(filterServiceUuid);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl, result2);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //result 有   mask 有    filter 有    值满足filter & mask = result & mask
    resultUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    result.AddServiceUuid(resultUuid);
    filterServiceUuidMask = OHOS::bluetooth::Uuid::ConvertFromString("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    filterImpl.SetServiceUuidMask(filterServiceUuidMask);
    filterServiceUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000003-0000-1000-8000-00805F9B34FB");
    filterImpl.SetServiceUuid(filterServiceUuid);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 有   mask 有    filter 有    值不满足filter & mask = result & mask
    resultUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    result.AddServiceUuid(resultUuid);
    filterServiceUuidMask = OHOS::bluetooth::Uuid::ConvertFromString("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    filterImpl.SetServiceUuidMask(filterServiceUuidMask);
    filterServiceUuid = OHOS::bluetooth::Uuid::ConvertFromString("00000003-0000-1000-8000-00805F9B34FA");
    filterImpl.SetServiceUuid(filterServiceUuid);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceUuids(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);
    HILOGI("BLE_ModuleTest_MatchesServiceUuids_00300 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_MatchesServiceData_00400, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_MatchesServiceData_00400 start");

    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    BluetoothBleScanResult result;
    FilterCheckState matchResult;

    //result 0  mask 0  filter 0  pass
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 1 mask 0  filter 0  pass
    OHOS::bluetooth::Uuid resultServiceDataUuid = OHOS::bluetooth::Uuid::ConvertFromString("0500fdee");
    //ascii show 0x36
    std::string resultServiceData = "6";
    std::vector<uint8_t> filterServiceData = {0xee, 0xfd, 0x00, 0x05, 0x36};
    std::vector<uint8_t> filterServiceDataMask = {0xff, 0xff, 0x00, 0xff, 0xff};
    //data的uuid + data = filter.ServiceData
    result.AddServiceData(resultServiceDataUuid, resultServiceData);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 0 mask 1  filter 0  pass
    BluetoothBleScanResult result1;
    filterServiceDataMask = {0xff, 0xff, 0x00, 0xff, 0xff};
    filterImpl.SetServiceDataMask(filterServiceDataMask);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl, result1);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 0 mask 0  filter 1  fail
    BluetoothBleScanResult result3;
    filterServiceData = {0xee, 0xfd, 0x00, 0x05, 0x36};
    filterImpl.SetServiceData(filterServiceData);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl, result3);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //result 1   mask 1   filter 0  pass
    resultServiceDataUuid = OHOS::bluetooth::Uuid::ConvertFromString("0500fdee");
    resultServiceData = "6";
    //data的uuid + data = filter.ServiceData
    result.AddServiceData(resultServiceDataUuid, resultServiceData);
    OHOS::bluetooth::BleScanFilterImpl filterImpl1;
    filterServiceDataMask = {0xff, 0xff, 0x00, 0xff, 0xff};
    filterImpl1.SetServiceDataMask(filterServiceDataMask);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl1, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 有   mask 没有  filter 有    比较值相同
    resultServiceDataUuid = OHOS::bluetooth::Uuid::ConvertFromString("0500fdee");
    resultServiceData = "6";
    //data的uuid + data = filter.ServiceData
    result.AddServiceData(resultServiceDataUuid, resultServiceData);
    OHOS::bluetooth::BleScanFilterImpl filterImpl2;
    filterServiceData = {0xee, 0xfd, 0x00, 0x05, 0x36};
    filterImpl2.SetServiceData(filterServiceData);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl2, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 有   mask 没有  filter 有    比较值不同
    resultServiceDataUuid = OHOS::bluetooth::Uuid::ConvertFromString("0500fdee");
    resultServiceData = "6";
    //data的uuid + data = filter.ServiceData
    result.AddServiceData(resultServiceDataUuid, resultServiceData);
    filterServiceData = {0xef, 0xfd, 0x00, 0x05, 0x36};
    filterImpl2.SetServiceData(filterServiceData);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl2, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //result 没有 mask 有    filter 有    fail
    BluetoothBleScanResult result2;
    filterServiceDataMask = {0xff, 0xff, 0x00, 0xff, 0xff};
    filterImpl.SetServiceDataMask(filterServiceDataMask);
    filterServiceData = {0xee, 0xfd, 0x00, 0x05, 0x36};
    filterImpl.SetServiceData(filterServiceData);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl, result2);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //result 有   mask 有    filter 有    值满足filter & mask = result & mask
    resultServiceDataUuid = OHOS::bluetooth::Uuid::ConvertFromString("0500fdee");
    resultServiceData = "6";
    //data的uuid + data = filter.ServiceData
    result.AddServiceData(resultServiceDataUuid, resultServiceData);
    filterServiceDataMask = {0xff, 0xff, 0x00, 0xff, 0xff};
    filterImpl.SetServiceDataMask(filterServiceDataMask);
    filterServiceData = {0xee, 0xfd, 0x02, 0x05, 0x36};
    filterImpl.SetServiceData(filterServiceData);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result 有   mask 有    filter 有    值不满足filter & mask = result & mask
    resultServiceDataUuid = OHOS::bluetooth::Uuid::ConvertFromString("0500fdee");
    resultServiceData = "6";
    //data的uuid + data = filter.ServiceData
    result.AddServiceData(resultServiceDataUuid, resultServiceData);
    filterServiceDataMask = {0xff, 0xff, 0x00, 0xff, 0xff};
    filterImpl.SetServiceDataMask(filterServiceDataMask);
    filterServiceData = {0xef, 0xfd, 0x00, 0x05, 0x36};
    filterImpl.SetServiceData(filterServiceData);
    matchResult = BluetoothBleFilterMatcher::MatchesServiceDatas(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    HILOGI("BLE_ModuleTest_MatchesServiceData_00400 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_MatchesManufacturer_00500, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_MatchesManufacturer_00500 start");

    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    BluetoothBleScanResult result;
    FilterCheckState matchResult;

    int manufacturerId = 24;
    //asci 0109
    std::vector<uint8_t> manufactureData = {0x30, 0x31, 0x30, 0x39};
    std::vector<uint8_t> manufactureDataMask = {0xff, 0xff, 0xff, 0xff};
    std::string resultManufacturerData = "0109";

    //全为空 pass
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //filterId  0  resulId&resultData 1  filterData 0  filterMask  0  pass
    OHOS::bluetooth::BleScanFilterImpl filterImpl1;
    resultManufacturerData = "0109";
    result.AddManufacturerData(manufacturerId, resultManufacturerData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl1, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //filterId  0  resulId&resultData 0  filterData 1  filterMask  0  fail
    manufactureData = {0x30, 0x31, 0x30, 0x39};
    filterImpl1.SetManufactureData(manufactureData);
    BluetoothBleScanResult result1;
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl1, result1);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //filterId  1  resulId&resultData 1  filterData 0  filterMask  0  ID匹配 pass
    OHOS::bluetooth::BleScanFilterImpl filterImpl2;
    manufacturerId = 24;
    filterImpl2.SetManufacturerId(manufacturerId);
    resultManufacturerData = "0109";
    result.AddManufacturerData(manufacturerId, resultManufacturerData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl2, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //filterId  1  resulId&resultData 0  filterData 1  filterMask  0  fail
    OHOS::bluetooth::BleScanFilterImpl filterImpl3;
    manufacturerId = 24;
    filterImpl3.SetManufacturerId(manufacturerId);
    filterImpl3.SetManufactureData(manufactureData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl3, result1);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //filterId  0  resulId&resultData 1  filterData 1  filterMask  0  data匹配ID不匹配 fail
    OHOS::bluetooth::BleScanFilterImpl filterImpl5;
    manufactureData = {0x30, 0x31, 0x30, 0x39};
    filterImpl5.SetManufactureData(manufactureData);
    resultManufacturerData = "0109";
    result.AddManufacturerData(manufacturerId, resultManufacturerData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl5, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //filterId  0  resulId&resultData 0  filterData 1  filterMask  1  fail
    OHOS::bluetooth::BleScanFilterImpl filterImpl6;
    manufactureData = {0x30, 0x31, 0x30, 0x39};
    filterImpl6.SetManufactureData(manufactureData);
    manufactureDataMask = {0xff, 0xff, 0xff, 0x00};
    filterImpl6.SetManufactureDataMask(manufactureDataMask);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl6, result1);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //filterId  1  resulId&resultData 1  filterData 1  filterMask  0  全一样 pass
    OHOS::bluetooth::BleScanFilterImpl filterImpl7;
    manufacturerId = 24;
    filterImpl7.SetManufacturerId(manufacturerId);
    manufactureData = {0x30, 0x31, 0x30, 0x39}; //ASCI 0109
    filterImpl7.SetManufactureData(manufactureData);
    resultManufacturerData = "0109";
    result.AddManufacturerData(manufacturerId, resultManufacturerData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl7, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //filterId  1  resulId&resultData 1  filterData 1  filterMask  0  不一样 fail
    resultManufacturerData = "0209";
    BluetoothBleScanResult result2;
    result2.AddManufacturerData(manufacturerId, resultManufacturerData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl7, result2);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //filterId  0  resulId&resultData 1  filterData 1  filterMask  1  data匹配 fail
    OHOS::bluetooth::BleScanFilterImpl filterImpl8;
    manufacturerId = 24;
    manufactureData = {0x30, 0x31, 0x30, 0x39}; //ASCI 0109
    filterImpl8.SetManufactureData(manufactureData);
    resultManufacturerData = "0109";
    manufactureDataMask = {0xFF, 0xFF, 0xFF, 0xFF};
    filterImpl8.SetManufactureDataMask(manufactureDataMask);
    result.AddManufacturerData(manufacturerId, resultManufacturerData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl8, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //filterId  0  resulId&resultData 1  filterData 1  filterMask  1  data不匹配 fail
    resultManufacturerData = "0108";
    result.AddManufacturerData(manufacturerId, resultManufacturerData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl8, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    //filterId  1  resulId&resultData 1  filterData 1  filterMask  1  掩码匹配 pass
    filterImpl8.SetManufacturerId(manufacturerId);
    manufactureData = {0x30, 0x31, 0x30, 0x39}; //ASCI 0109
    filterImpl8.SetManufactureData(manufactureData);
    manufactureDataMask = {0xff, 0xff, 0xff, 0x00};
    filterImpl8.SetManufactureDataMask(manufactureDataMask);
    BluetoothBleScanResult result3;
    resultManufacturerData = "0108";
    result3.AddManufacturerData(manufacturerId, resultManufacturerData);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl8, result3);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //filterId  1  resulId&resultData 1  filterData 1  filterMask  1  掩码不匹配 fail
    manufactureDataMask = {0xff, 0xff, 0xff, 0xff};
    filterImpl8.SetManufactureDataMask(manufactureDataMask);
    matchResult = BluetoothBleFilterMatcher::MatchesManufacturerDatas(filterImpl8, result3);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    HILOGI("BLE_ModuleTest_MatchesManufacturer_00500 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_MatchesAdvIndReportFlag_00600, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_MatchesAdvIndReportFlag_00600 start");

    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    FilterCheckState matchResult;

    // ADV_IND + flag true --> PASS
    int eventType = 0x0013;
    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, true);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    // ADV_SCAN_IND + flag true --> PASS
    eventType = 0x0012;
    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, true);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    // ADV_IND + flag false --> NO PASS
    eventType = 0x0013;
    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, false);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    // ADV_SCAN_IND + flag false --> NO PASS
    eventType = 0x0012;
    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, false);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);

    // ADV_NONCONN_IND + flag true/false --> PASS
    eventType = 0x0010;
    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, true);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, false);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    // ADV_DIRECT_IND + flag true/false --> PASS
    eventType = 0x0015;
    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, true);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, false);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    // SCAN_RSP + flag true/false --> PASS
    eventType = 0x001B;
    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, true);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, false);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    // SCAN_RSP to ADV_SCAN_IND + flag true/false --> PASS
    eventType = 0x001A;
    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, true);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    matchResult = BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(eventType, false);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);
    HILOGI("BLE_ModuleTest_MatchesAdvIndReportFlag_00600 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_SetAdvIndReportFlag_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetAdvIndReportFlag_00100 start");

    OHOS::bluetooth::BleScanFilterImpl filterImpl;

    //filter false
    filterImpl.SetAdvIndReportFlag(false);
    EXPECT_FALSE(filterImpl.GetAdvIndReportFlag());

    //filter true
    filterImpl.SetAdvIndReportFlag(true);
    EXPECT_TRUE(filterImpl.GetAdvIndReportFlag());
    HILOGI("BLE_ModuleTest_SetAdvIndReportFlag_00100 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_GetServiceSolicitationUuid_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetServiceSolicitationUuid_00100 start");

    BleScanFilter filter;

    UUID uuid = filter.GetServiceSolicitationUuid();
    filter.SetServiceSolicitationUuidMask(uuid);
    EXPECT_TRUE(filter.HasSolicitationUuidMask());
    HILOGI("BLE_ModuleTest_GetServiceSolicitationUuid_00100 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_SetRssiThreshold_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetRssiThreshold_00100 start");
    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    filterImpl.SetRssiThreshold(-10);
    EXPECT_EQ(filterImpl.GetRssiThreshold(), -10);
    HILOGI("BLE_ModuleTest_SetRssiThreshold_00100 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_SetRssiThreshold_00200, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetRssiThreshold_00200 start");
    BleScanFilter filter;
    filter.SetRssiThreshold(-10);
    EXPECT_EQ(filter.GetRssiThreshold(), -10);
    HILOGI("BLE_ModuleTest_SetRssiThreshold_00200 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_MatchesRssiThreshold_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_MatchesRssiThreshold_00100 start");
    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    BluetoothBleScanResult result;
    FilterCheckState matchResult;

    // no rssi filter  pass
    matchResult = BluetoothBleFilterMatcher::MatchesRssiThreshold(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    filterImpl.SetRssiThreshold(-90);
    result.SetRssi(-60);

    //result -60 > filter -90  pass
    matchResult = BluetoothBleFilterMatcher::MatchesRssiThreshold(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_PASS);

    //result -60 < filter -50  fail
    filterImpl.SetRssiThreshold(-50);
    result.SetRssi(-60);
    matchResult = BluetoothBleFilterMatcher::MatchesRssiThreshold(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);
    HILOGI("BLE_ModuleTest_MatchesRssiThreshold_00100 end");
}

HWTEST_F(BluetoothBleScanFileterTest, BLE_ModuleTest_MatchesScanFilter_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_MatchesScanFilter_00100 start");
    OHOS::bluetooth::BleScanFilterImpl filterImpl;
    BluetoothBleScanResult result;
    FilterCheckState matchResult;

    filterImpl.SetRssiThreshold(-60);
    result.SetRssi(-90);

    matchResult = BluetoothBleFilterMatcher::MatchesScanFilter(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);
    result.SetRssi(-60);

    bool enable = true;
    filterImpl.SetLpDeviceFlag(enable);
    matchResult = BluetoothBleFilterMatcher::MatchesScanFilter(filterImpl, result);
    EXPECT_EQ(matchResult, FilterCheckState::FILTER_CHECK_FAIL);
    HILOGI("BLE_ModuleTest_MatchesScanFilter_00100 end");
}

/**
 * @tc.name: BleScanFilterTest_SetAddressType_001
 * @tc.desc: BleScanFilterTest_SetAddressType_001
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothBleScanFileterTest, BleScanFilterTest_SetAddressType_001, TestSize.Level1)
{
    HILOGI("BleScanFilterTest_SetAddressType_001 start");
    BleScanFilter filter;
    EXPECT_EQ(filter.GetAddressType(), AddressType::UNSET_ADDRESS);
    filter.SetAddressType(AddressType::REAL_ADDRESS);
    EXPECT_EQ(filter.GetAddressType(), AddressType::REAL_ADDRESS);
    HILOGI("BleScanFilterTest_SetAddressType_001 end");
}

/**
 * @tc.name: BleScanFilterTest_SetRawAddressType_001
 * @tc.desc: BleScanFilterTest_SetRawAddressType_001
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothBleScanFileterTest, BleScanFilterTest_SetRawAddressType_001, TestSize.Level1)
{
    HILOGI("BleScanFilterTest_SetRawAddressType_001 start");
    BleScanFilter filter;
    EXPECT_EQ(filter.GetRawAddressType(), RawAddressType::UNSET_RAW_ADDRESS);
    filter.SetRawAddressType(RawAddressType::PUBLIC_ADDRESS);
    EXPECT_EQ(filter.GetRawAddressType(), RawAddressType::PUBLIC_ADDRESS);
    HILOGI("BleScanFilterTest_SetRawAddressType_001 end");
}

/**
 * @tc.name: BleScanFilterTest_SetIrk_001
 * @tc.desc: BleScanFilterTest_SetIrk_001
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothBleScanFileterTest, BleScanFilterTest_SetIrk_001, TestSize.Level1)
{
    HILOGI("BleScanFilterTest_SetIrk_001 start");
    BleScanFilter filter;
    EXPECT_TRUE(filter.GetIrk().empty());
    std::vector<uint8_t> irk = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
        0x0F, 0x10};
    filter.SetIrk(irk);
    EXPECT_EQ(filter.GetIrk(), irk);
    HILOGI("BleScanFilterTest_SetIrk_001 end");
}

/**
 * @tc.name: BleScanResultTest_SetAddress_001
 * @tc.desc: BleScanResultTest_SetAddress_001
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothBleScanFileterTest, BleScanResultTest_SetAddress_001, TestSize.Level1)
{
    HILOGI("BleScanResultTest_SetAddress_001 start");
    BleScanResult result;
    BluetoothAddress btAddressDefault = result.GetAddress();
    EXPECT_EQ(btAddressDefault.address, "");
    EXPECT_EQ(btAddressDefault.addressType, AddressType::UNSET_ADDRESS);
    EXPECT_EQ(btAddressDefault.rawAddressType, RawAddressType::UNSET_RAW_ADDRESS);

    BluetoothAddress btAddressTest("11:22:33:44:55:66", AddressType::REAL_ADDRESS, RawAddressType::PUBLIC_ADDRESS);
    result.SetAddress(btAddressTest);

    BluetoothAddress btAddressSet = result.GetAddress();
    EXPECT_EQ(btAddressSet.address, "11:22:33:44:55:66");
    EXPECT_EQ(btAddressSet.addressType, AddressType::REAL_ADDRESS);
    EXPECT_EQ(btAddressSet.rawAddressType, RawAddressType::PUBLIC_ADDRESS);
    HILOGI("BleScanResultTest_SetAddress_001 end");
}

/**
 * @tc.name: BluetoothAddress_001
 * @tc.desc: BluetoothAddress_001
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothBleScanFileterTest, BluetoothAddress_001, TestSize.Level1)
{
    HILOGI("BluetoothAddress_001 start");
    BluetoothAddress btAddressDefault;
    EXPECT_EQ(btAddressDefault.address, "");
    EXPECT_EQ(btAddressDefault.addressType, AddressType::UNSET_ADDRESS);
    EXPECT_EQ(btAddressDefault.rawAddressType, RawAddressType::UNSET_RAW_ADDRESS);

    BluetoothAddress btAddressSet("11:22:33:44:55:66", AddressType::REAL_ADDRESS, RawAddressType::PUBLIC_ADDRESS);
    EXPECT_EQ(btAddressSet.address, "11:22:33:44:55:66");
    EXPECT_EQ(btAddressSet.addressType, AddressType::REAL_ADDRESS);
    EXPECT_EQ(btAddressSet.rawAddressType, RawAddressType::PUBLIC_ADDRESS);
    HILOGI("BluetoothAddress_001 end");
}