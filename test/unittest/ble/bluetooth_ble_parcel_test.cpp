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
#include "bluetooth_ble_scan_result.h"
#include "bluetooth_gatt_characteristic_parcel.h"
#include "bluetooth_gatt_descriptor_parcel.h"
#include "bluetooth_ble_advertiser_data.h"
#include "bluetooth_gatt_device.h"
#include "bluetooth_ble_scan_filter.h"
#include "message_parcel.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_gatt_service_parcel.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"
#include "bt_uuid.h"
#include "bluetooth_ble_advertiser_settings.h"
#include "bluetooth_ble_scan_settings.h"
#include "bluetooth_host.h"


using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
class BlueToothBleParcelTest : public testing::Test {
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
        host_ = nullptr;
    }
    void SetUp()
    {}

    void TearDown()
    {}

public:
};
/**
 * @tc.name: BLE_BlueToothBleParcelTest_00100
 * @tc.desc: BLE_BlueToothBleParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00100, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00100 start");
    MessageParcel data;
    BluetoothBleScanResult bluetoothBleScanResult;
    bluetooth::Uuid uuid;
    std::string serviceData = "testData";
    uint16_t manufacturerId = 0;
    bluetoothBleScanResult.AddServiceData(uuid, serviceData);
    bluetoothBleScanResult.AddServiceUuid(uuid);
    bluetoothBleScanResult.AddManufacturerData(manufacturerId, serviceData);
    EXPECT_TRUE(data.WriteParcelable(&bluetoothBleScanResult));
    std::shared_ptr<BluetoothBleScanResult> result(data.ReadParcelable<BluetoothBleScanResult>());
    HILOGI("BLE_BlueToothBleParcelTest_00100 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_00200
 * @tc.desc: BLE_BlueToothBleParcelTest_00200.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00200, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00200 start");
    MessageParcel data;
    BluetoothGattCharacteristic bluetoothGattCharacteristic;
    EXPECT_TRUE(data.WriteParcelable(&bluetoothGattCharacteristic));
    std::shared_ptr<BluetoothGattCharacteristic> result(data.ReadParcelable<BluetoothGattCharacteristic>());
    HILOGI("BLE_BlueToothBleParcelTest_00200 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_00300
 * @tc.desc: BLE_BlueToothBleParcelTest_00300.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00300, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00300 start");
    MessageParcel data;
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    bluetooth::Uuid uuid;
    std::string serviceData = "testData";
    uint16_t manufacturerId = 0;
    bluetoothBleAdvertiserData.AddServiceData(uuid, serviceData);
    bluetoothBleAdvertiserData.AddServiceUuid(uuid);
    bluetoothBleAdvertiserData.AddManufacturerData(manufacturerId, serviceData);
    EXPECT_TRUE(data.WriteParcelable(&bluetoothBleAdvertiserData));
    std::shared_ptr<BluetoothBleAdvertiserData> result(data.ReadParcelable<BluetoothBleAdvertiserData>());
    HILOGI("BLE_BlueToothBleParcelTest_00300 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_00400
 * @tc.desc: BLE_BlueToothBleParcelTest_00400.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00400, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00400 start");
    MessageParcel data;
    BluetoothBleScanFilter bluetoothBleScanFilter;
    EXPECT_TRUE(data.WriteParcelable(&bluetoothBleScanFilter));
    std::shared_ptr<BluetoothBleScanFilter> result(data.ReadParcelable<BluetoothBleScanFilter>());
    HILOGI("BLE_BlueToothBleParcelTest_00400 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_00500
 * @tc.desc: BLE_BlueToothBleParcelTest_00500.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00500, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00500 start");
    MessageParcel data;
    BluetoothGattService bluetoothGattService;
    BluetoothGattService bluetoothGattService1;
    bluetoothGattService.includeServices_.push_back(bluetoothGattService1);
    EXPECT_TRUE(data.WriteParcelable(&bluetoothGattService));
    std::shared_ptr<BluetoothGattService> result(data.ReadParcelable<BluetoothGattService>());
    HILOGI("BLE_BlueToothBleParcelTest_00500 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_00600
 * @tc.desc: BLE_BlueToothBleParcelTest_00600.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00600, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00600 start");
    MessageParcel data;
    BluetoothBleAdvertiserSettings bluetoothBleAdvertiserSettings;
    EXPECT_TRUE(data.WriteParcelable(&bluetoothBleAdvertiserSettings));
    std::shared_ptr<BluetoothBleAdvertiserSettings> result(data.ReadParcelable<BluetoothBleAdvertiserSettings>());
    HILOGI("BLE_BlueToothBleParcelTest_00600 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_00700
 * @tc.desc: BLE_BlueToothBleParcelTest_00700.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00700, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00700 start");
    MessageParcel data;
    BluetoothBleScanSettings bluetoothBleScanSettings;
    EXPECT_TRUE(data.WriteParcelable(&bluetoothBleScanSettings));
    std::shared_ptr<BluetoothBleScanSettings> result(data.ReadParcelable<BluetoothBleScanSettings>());
    HILOGI("BLE_BlueToothBleParcelTest_00700 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_00800
 * @tc.desc: BLE_BlueToothBleParcelTest_00800.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00800, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00800 start");
    MessageParcel data;
    BluetoothGattDescriptor bluetoothGattDescriptor;
    EXPECT_TRUE(data.WriteParcelable(&bluetoothGattDescriptor));
    std::shared_ptr<BluetoothGattDescriptor> result(data.ReadParcelable<BluetoothGattDescriptor>());
    HILOGI("BLE_BlueToothBleParcelTest_00800 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_00900
 * @tc.desc: BLE_BlueToothBleParcelTest_00900.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_00900, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00900 start");
    MessageParcel data;
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    EXPECT_TRUE(data.WriteParcelable(&bluetoothBleAdvertiserData));
    std::shared_ptr<BluetoothBleAdvertiserData> result(data.ReadParcelable<BluetoothBleAdvertiserData>());
    HILOGI("BLE_BlueToothBleParcelTest_00900 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01000
 * @tc.desc: BLE_BlueToothBleParcelTest_01000.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01000, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_01000 start");
    MessageParcel data;
    BluetoothGattDevice bluetoothGattDevice;
    EXPECT_TRUE(data.WriteParcelable(&bluetoothGattDevice));
    std::shared_ptr<BluetoothGattDevice> result(data.ReadParcelable<BluetoothGattDevice>());
    HILOGI("BLE_BlueToothBleParcelTest_01000 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01100
 * @tc.desc: BLE_BlueToothBleParcelTest_01100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01100, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_00100 start");
    MessageParcel data;
    BluetoothBleScanSettings bluetoothBleScanSettings;
    bluetoothBleScanSettings.SetCallbackType(BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH);
    bluetoothBleScanSettings.SetMatchTrackAdvType(FEW_MATCH_TRACK_ADV);
    EXPECT_TRUE(data.WriteParcelable(&bluetoothBleScanSettings));
    std::shared_ptr<BluetoothBleScanSettings> result(data.ReadParcelable<BluetoothBleScanSettings>());
    EXPECT_EQ(BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH, result->GetCallbackType());
    EXPECT_EQ(FEW_MATCH_TRACK_ADV, result->GetMatchTrackAdvType());
    HILOGI("BLE_BlueToothBleParcelTest_01100 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01200
 * @tc.desc: BLE_BlueToothBleParcelTest_01200.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01200, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_01200 start");
    BluetoothBleScanFilter bluetoothBleScanFilter;
    OHOS::Parcel parcel;
    EXPECT_TRUE(bluetoothBleScanFilter.WriteToParcel(parcel));
    HILOGI("BLE_BlueToothBleParcelTest_01200 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01300
 * @tc.desc: BLE_BlueToothBleParcelTest_01300.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01300, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_01300 start");
    BluetoothBleScanResult bluetoothBleScanResult;
    OHOS::Parcel parcel;
    EXPECT_TRUE(bluetoothBleScanResult.WriteToParcel(parcel));
    HILOGI("BLE_BlueToothBleParcelTest_01300 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01400
 * @tc.desc: BLE_BlueToothBleParcelTest_01400.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01400, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_01400 start");
    BluetoothGattDevice device;
    OHOS::Parcel parcel;
    EXPECT_TRUE(device.WriteToParcel(parcel));
    HILOGI("BLE_BlueToothBleParcelTest_01400 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01500
 * @tc.desc: BLE_BlueToothBleParcelTest_01500.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01500, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_01500 start");
    BluetoothBleScanSettings bluetoothBleScanSettings;
    OHOS::Parcel parcel;
    EXPECT_TRUE(bluetoothBleScanSettings.WriteToParcel(parcel));
    HILOGI("BLE_BlueToothBleParcelTest_01500 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01600
 * @tc.desc: BLE_BlueToothBleParcelTest_01600.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01600, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_01600 start");
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    OHOS::Parcel parcel;
    EXPECT_TRUE(bluetoothBleAdvertiserData.WriteToParcel(parcel));
    HILOGI("BLE_BlueToothBleParcelTest_01600 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01700
 * @tc.desc: BLE_BlueToothBleParcelTest_01700.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01700, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_001700 start");
    MessageParcel data;
    BluetoothBleScanSettings bluetoothBleScanSettings;
    bluetoothBleScanSettings.SetSensitivityMode(
        static_cast<uint8_t>(bluetooth::SENSITIVITY_MODE::SENSITIVITY_MODE_HIGH));
    data.WriteParcelable(&bluetoothBleScanSettings);
    std::shared_ptr<BluetoothBleScanSettings> result(data.ReadParcelable<BluetoothBleScanSettings>());
    EXPECT_EQ(static_cast<uint8_t>(bluetooth::SENSITIVITY_MODE::SENSITIVITY_MODE_HIGH), result->GetSensitivityMode());
    HILOGI("BLE_BlueToothBleParcelTest_001700 end");
}

/**
 * @tc.name: BLE_BlueToothBleParcelTest_01800
 * @tc.desc: BLE_BlueToothBleParcelTest_01800.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BLE_BlueToothBleParcelTest_01800, TestSize.Level1)
{
    HILOGI("BLE_BlueToothBleParcelTest_001800 start");
    MessageParcel parcel;
    BluetoothBleScanFilter filter;
    filter.SetDeviceId("11:22:33:44:55:66");
    filter.SetAddressType(OHOS::bluetooth::AddressType::REAL_ADDRESS);
    filter.SetRawAddressType(OHOS::bluetooth::RawAddressType::PUBLIC_ADDRESS);
    EXPECT_TRUE(filter.WriteAddrInfoToParcel(parcel));
    filter.SetDeviceId("22:22:22:11:11:11");
    filter.SetAddressType(OHOS::bluetooth::AddressType::VIRTUAL_ADDRESS);
    filter.SetRawAddressType(OHOS::bluetooth::RawAddressType::RANDOM_ADDRESS);
    filter.ReadAddrInfoFromParcel(parcel);
    EXPECT_EQ(filter.GetDeviceId(), "11:22:33:44:55:66");
    EXPECT_EQ(filter.GetAddressType(), OHOS::bluetooth::AddressType::REAL_ADDRESS);
    EXPECT_EQ(filter.GetRawAddressType(), OHOS::bluetooth::RawAddressType::PUBLIC_ADDRESS);
    HILOGI("BLE_BlueToothBleParcelTest_001800 end");
}

/**
 * @tc.name: BluetoothBleScanFilterTest_00100
 * @tc.desc: BluetoothBleScanFilterTest_00100
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BluetoothBleScanFilterTest_00100, TestSize.Level1)
{
    HILOGI("BluetoothBleScanFilterTest_00100 start");
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    EXPECT_TRUE(filter1 == filter2);
    HILOGI("BluetoothBleScanFilterTest_00100 end");
}

/**
 * @tc.name: BluetoothBleScanFilterTest_00200
 * @tc.desc: BluetoothBleScanFilterTest_00200
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BluetoothBleScanFilterTest_00200, TestSize.Level1)
{
    HILOGI("BluetoothBleScanFilterTest_00200 start");
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    std::vector<BluetoothBleScanFilter> filters;

    std::string uuidStr = "00001234-0000-1000-8000-00805F9B34FB";
    std::string uuidMaskStr = "0000FFFF-0000-0000-0000-000000000000";
    std::string name = "test_name";
    std::string deviceId = "42:0B:AC:A0:A0:ED";
    bluetooth::Uuid uuid = bluetooth::Uuid::ConvertFromString(uuidStr);
    bluetooth::Uuid uuidMask = bluetooth::Uuid::ConvertFromString(uuidMaskStr);
    std::vector<uint8_t> serviceData = {0x34, 0x12, 0x56, 0x78};
    std::vector<uint8_t> serviceDataMask = {0x00, 0x00, 0xFF, 0xFF};
    uint16_t manufacturerId = 24;
    std::vector<uint8_t> manufactureData = {0x01, 0x09, 0x20, 0x02};
    std::vector<uint8_t> manufactureDataMask = {0x00, 0x00, 0xFF, 0xFF};

    filter1.SetName(name);
    filter1.SetDeviceId(deviceId);
    filter1.SetServiceUuid(uuid);
    filter1.SetServiceUuidMask(uuidMask);
    filter1.SetManufacturerId(manufacturerId);
    filter1.SetManufactureData(manufactureData);
    filter1.SetManufactureDataMask(manufactureDataMask);
    filter1.SetServiceData(serviceData);
    filter1.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter1);

    // filter2 is the same with filter1
    filter2.SetName(name);
    filter2.SetDeviceId(deviceId);
    filter2.SetServiceUuid(uuid);
    filter2.SetServiceUuidMask(uuidMask);
    filter2.SetManufacturerId(manufacturerId);
    filter2.SetManufactureData(manufactureData);
    filter2.SetManufactureDataMask(manufactureDataMask);
    filter2.SetServiceData(serviceData);
    filter2.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter2);

    EXPECT_TRUE(filter1 == filter2);
    HILOGI("BluetoothBleScanFilterTest_00200 end");
}

/**
 * @tc.name: BluetoothBleScanFilterTest_00300
 * @tc.desc: BluetoothBleScanFilterTest_00300
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BluetoothBleScanFilterTest_00300, TestSize.Level1)
{
    HILOGI("BluetoothBleScanFilterTest_00300 start");
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    std::vector<BluetoothBleScanFilter> filters;

    std::string uuidStr1 = "00001234-0000-1000-8000-00805F9B34FB";
    std::string uuidStr2 = "00001222-0000-1000-8000-00805F9B34FB";
    std::string uuidMaskStr = "0000FFFF-0000-0000-0000-000000000000";
    std::string name = "test_name";
    std::string deviceId = "42:0B:AC:A0:A0:ED";
    bluetooth::Uuid uuid1 = bluetooth::Uuid::ConvertFromString(uuidStr1);
    bluetooth::Uuid uuid2 = bluetooth::Uuid::ConvertFromString(uuidStr2);
    bluetooth::Uuid uuidMask = bluetooth::Uuid::ConvertFromString(uuidMaskStr);
    std::vector<uint8_t> serviceData = {0x34, 0x12, 0x56, 0x78};
    std::vector<uint8_t> serviceDataMask = {0x00, 0x00, 0xFF, 0xFF};
    uint16_t manufacturerId = 24;
    std::vector<uint8_t> manufactureData = {0x01, 0x09, 0x20, 0x02};
    std::vector<uint8_t> manufactureDataMask = {0x00, 0x00, 0xFF, 0xFF};

    filter1.SetName(name);
    filter1.SetDeviceId(deviceId);
    filter1.SetServiceUuid(uuid1);
    filter1.SetServiceUuidMask(uuidMask);
    filter1.SetManufacturerId(manufacturerId);
    filter1.SetManufactureData(manufactureData);
    filter1.SetManufactureDataMask(manufactureDataMask);
    filter1.SetServiceData(serviceData);
    filter1.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter1);

    filter2.SetName(name);
    filter2.SetDeviceId(deviceId);
    filter2.SetServiceUuid(uuid2);
    filter2.SetServiceUuidMask(uuidMask);
    filter2.SetManufacturerId(manufacturerId);
    filter2.SetManufactureData(manufactureData);
    filter2.SetManufactureDataMask(manufactureDataMask);
    filter2.SetServiceData(serviceData);
    filter2.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter2);

    EXPECT_FALSE(filter1 == filter2);
    HILOGI("BluetoothBleScanFilterTest_00300 end");
}

/**
 * @tc.name: BluetoothBleScanFilterTest_00400
 * @tc.desc: BluetoothBleScanFilterTest_00400
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BluetoothBleScanFilterTest_00400, TestSize.Level1)
{
    HILOGI("BluetoothBleScanFilterTest_00400 start");
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    std::vector<BluetoothBleScanFilter> filters;

    std::string uuidStr = "00001234-0000-1000-8000-00805F9B34FB";
    std::string uuidMaskStr = "0000FFFF-0000-0000-0000-000000000000";
    std::string name = "test_name";
    std::string deviceId = "42:0B:AC:A0:A0:ED";
    bluetooth::Uuid uuid = bluetooth::Uuid::ConvertFromString(uuidStr);
    bluetooth::Uuid uuidMask = bluetooth::Uuid::ConvertFromString(uuidMaskStr);
    std::vector<uint8_t> serviceData = {0x34, 0x12, 0x56, 0x78};
    std::vector<uint8_t> serviceDataMask = {0x00, 0x00, 0xFF, 0xFF};
    uint16_t manufacturerId1 = 24;
    uint16_t manufacturerId2 = 23;
    std::vector<uint8_t> manufactureData1 = {0x01, 0x09, 0x20, 0x02};
    std::vector<uint8_t> manufactureData2 = {0x01, 0x09, 0x30, 0x03};
    std::vector<uint8_t> manufactureDataMask = {0x00, 0x00, 0xFF, 0xFF};

    filter1.SetName(name);
    filter1.SetDeviceId(deviceId);
    filter1.SetServiceUuid(uuid);
    filter1.SetServiceUuidMask(uuidMask);
    filter1.SetManufacturerId(manufacturerId1);
    filter1.SetManufactureData(manufactureData1);
    filter1.SetManufactureDataMask(manufactureDataMask);
    filter1.SetServiceData(serviceData);
    filter1.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter1);

    filter2.SetName(name);
    filter2.SetDeviceId(deviceId);
    filter2.SetServiceUuid(uuid);
    filter2.SetServiceUuidMask(uuidMask);
    filter2.SetManufacturerId(manufacturerId2);
    filter2.SetManufactureData(manufactureData2);
    filter2.SetManufactureDataMask(manufactureDataMask);
    filter2.SetServiceData(serviceData);
    filter2.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter2);

    EXPECT_FALSE(filter1 == filter2);
    HILOGI("BluetoothBleScanFilterTest_00400 end");
}

/**
 * @tc.name: BluetoothBleScanFilterTest_00500
 * @tc.desc: BluetoothBleScanFilterTest_00500
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothBleParcelTest, BluetoothBleScanFilterTest_00500, TestSize.Level1)
{
    HILOGI("BluetoothBleScanFilterTest_00500 start");
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    std::vector<BluetoothBleScanFilter> filters;

    std::string uuidStr = "00001234-0000-1000-8000-00805F9B34FB";
    std::string uuidMaskStr = "0000FFFF-0000-0000-0000-000000000000";
    std::string name = "test_name";
    std::string deviceId = "42:0B:AC:A0:A0:ED";
    bluetooth::Uuid uuid = bluetooth::Uuid::ConvertFromString(uuidStr);
    bluetooth::Uuid uuidMask = bluetooth::Uuid::ConvertFromString(uuidMaskStr);
    std::vector<uint8_t> serviceData1 = {0x34, 0x12, 0x56, 0x78};
    std::vector<uint8_t> serviceData2 = {0x34, 0x12, 0x9A, 0xBC};
    std::vector<uint8_t> serviceDataMask = {0x00, 0x00, 0xFF, 0xFF};
    uint16_t manufacturerId = 24;
    std::vector<uint8_t> manufactureData = {0x01, 0x09, 0x20, 0x02};
    std::vector<uint8_t> manufactureDataMask = {0x00, 0x00, 0xFF, 0xFF};

    filter1.SetName(name);
    filter1.SetDeviceId(deviceId);
    filter1.SetServiceUuid(uuid);
    filter1.SetServiceUuidMask(uuidMask);
    filter1.SetManufacturerId(manufacturerId);
    filter1.SetManufactureData(manufactureData);
    filter1.SetManufactureDataMask(manufactureDataMask);
    filter1.SetServiceData(serviceData1);
    filter1.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter1);

    filter2.SetName(name);
    filter2.SetDeviceId(deviceId);
    filter2.SetServiceUuid(uuid);
    filter2.SetServiceUuidMask(uuidMask);
    filter2.SetManufacturerId(manufacturerId);
    filter2.SetManufactureData(manufactureData);
    filter2.SetManufactureDataMask(manufactureDataMask);
    filter2.SetServiceData(serviceData2);
    filter2.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter2);

    EXPECT_FALSE(filter1 == filter2);
    HILOGI("BluetoothBleScanFilterTest_00500 end");
}

}  // namespace bluetooth
}  // namespace OHOS