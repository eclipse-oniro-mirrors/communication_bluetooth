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
#include <cstring>
#include "adapter_properties.h"
#include "adapter_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
AdapterProperties adapterProperties;

enum {
  BT_DEVICE_TYPE_BREDR = (1 << 0), /* 仅支持经典蓝牙(BR/EDR) */
  BT_DEVICE_TYPE_BLE = (1 << 1), /* 仅支持低功耗蓝牙(BLE) */
  BT_DEVICE_TYPE_DUMO = BT_DEVICE_TYPE_BREDR | BT_DEVICE_TYPE_BLE, /* 双模设备(同时支持BR/EDR和BLE) */
};
class AdapterPropertiesTest : public testing::Test {
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
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number:adapter_properties_test_001
 * @tc.name: LoadConfigInfo
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_001, TestSize.Level1)
{
    HILOGI("adapter_properties_test_001 enter");
    // no data insert into dataBase leads cfi crash at datashare so need skip this case
    HILOGI("adapter_properties_test_001 end");
}

/*
 * @tc.number:adapter_properties_test_002
 * @tc.name: LoadHostInfo
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_002, TestSize.Level1)
{
    HILOGI("adapter_properties_test_002 enter");
    adapterProperties.LoadHostInfo();
    EXPECT_EQ(adapterProperties.ioCapability_, GAP_IO_DISPLAYONLY);
    HILOGI("adapter_properties_test_002 end");
}

/*
 * @tc.number:adapter_properties_test_003
 * @tc.name: SetDeviceName
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_003, TestSize.Level1)
{
    HILOGI("adapter_properties_test_003 enter");
    std::string deviceName = "";
    EXPECT_FALSE(adapterProperties.SetDeviceName(deviceName));
    std::string deviceName1 = "adapter_properties_test_003";
    EXPECT_FALSE(adapterProperties.SetDeviceName(deviceName1));
    deviceName = "11111111111111111111111111111111111";
    EXPECT_FALSE(adapterProperties.SetDeviceName(deviceName));
    HILOGI("adapter_properties_test_003 end");
}

/*
 * @tc.number:adapter_properties_test_004
 * @tc.name: SetDeviceClass
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_004, TestSize.Level1)
{
    HILOGI("adapter_properties_test_004 enter");
    int deviceClass = 1;
    EXPECT_FALSE(adapterProperties.SetDeviceClass(deviceClass));
    HILOGI("adapter_properties_test_004 end");
}

/*
 * @tc.number:adapter_properties_test_005
 * @tc.name: RemovePairedDeviceList
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_005, TestSize.Level1)
{
    HILOGI("adapter_properties_test_005 enter");
    std::string addr = "11:22:33:44:55:66";
    RawAddress device = RawAddress(addr);
    adapterProperties.AddPairedAddrList(device);
    EXPECT_EQ(adapterProperties.GetPairedAddrList()[0], addr);
    adapterProperties.RemovePairedDeviceList(addr);
    HILOGI("adapter_properties_test_005 end");
}

/*
 * @tc.number:adapter_properties_test_006
 * @tc.name: SetIoCapability
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_006, TestSize.Level1)
{
    HILOGI("adapter_properties_test_006 enter");
    int ioCapability = 1;
    EXPECT_FALSE(adapterProperties.SetIoCapability(ioCapability));
    HILOGI("adapter_properties_test_006 end");
}

/*
 * @tc.number:adapter_properties_test_007
 * @tc.name: ParseAdapterProps
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_007, TestSize.Level1)
{
    HILOGI("adapter_properties_test_007 enter");
    bt_status_t status = BT_STATUS_FAIL;
    int numProperties = 1;
    bt_property_t* properties = nullptr;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    status = BT_STATUS_SUCCESS;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    (void)memset_s(properties, sizeof(bt_property_t), 0, sizeof(bt_property_t));
    properties->type = bt_property_type_t::BT_PROPERTY_BDNAME;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    properties->type = bt_property_type_t::BT_PROPERTY_BDADDR;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    properties->type = bt_property_type_t::BT_PROPERTY_UUIDS;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    properties->type = STACK::BT_PROPERTY_CLASS_OF_DEVICE;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    properties->type = bt_property_type_t::BT_PROPERTY_LOCAL_IO_CAPS;
    EXPECT_EQ(adapterProperties.ioCapability_, INVALID_VALUE);
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    properties->type = bt_property_type_t::BT_PROPERTY_ADAPTER_BONDED_DEVICES;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    properties->type = bt_property_type_t::BT_PROPERTY_LOCAL_LE_FEATURES;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    properties->type = bt_property_type_t::BT_PROPERTY_ADAPTER_SCAN_MODE;
    adapterProperties.ParseAdapterProps(status, numProperties, properties);
    HILOGI("adapter_properties_test_007 end");
}

/*
 * @tc.number:adapter_properties_test_008
 * @tc.name: ParseDeviceName
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_008, TestSize.Level1)
{
    HILOGI("adapter_properties_test_008 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    adapterProperties.ParseDeviceName(properties);
    properties->len = 2;
    properties->val = nullptr;
    adapterProperties.ParseDeviceName(properties);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    properties->len = 249;
    adapterProperties.ParseDeviceName(properties);
    properties->len = 200;
    EXPECT_NE(adapterProperties.ParseDeviceName(properties), INVALID_NAME);
    HILOGI("adapter_properties_test_008 end");
}

/*
 * @tc.number:adapter_properties_test_009
 * @tc.name: ParseDeviceAddr
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_009, TestSize.Level1)
{
    HILOGI("adapter_properties_test_009 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    HILOGI("renhao %lu", sizeof(STACK::RawAddress));
    properties->len = sizeof(STACK::RawAddress) + 1;
    adapterProperties.ParseDeviceAddr(properties);
    properties->len = sizeof(STACK::RawAddress);
    properties->val = nullptr;
    adapterProperties.ParseDeviceAddr(properties);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    adapterProperties.ParseDeviceAddr(properties);
    EXPECT_FALSE(adapterProperties.macAddr_.empty());
    HILOGI("adapter_properties_test_009 end");
}

/*
 * @tc.number:adapter_properties_test_010
 * @tc.name: ParseDeviceUuid
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_010, TestSize.Level1)
{
    HILOGI("adapter_properties_test_010 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->val = nullptr;
    adapterProperties.ParseDeviceUuid(properties);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    properties->len = -1;
    adapterProperties.ParseDeviceUuid(properties);
    HILOGI("renhao %lu", sizeof(STACK::bluetooth::Uuid));
    properties->len = sizeof(STACK::bluetooth::Uuid) + 1;
    adapterProperties.ParseDeviceUuid(properties);
    properties->len = sizeof(STACK::bluetooth::Uuid);
    std::vector<Uuid> uuids = adapterProperties.ParseDeviceUuid(properties);
    EXPECT_FALSE(uuids.empty());
    HILOGI("adapter_properties_test_010 end");
}

/*
 * @tc.number:adapter_properties_test_011
 * @tc.name: ConvertDeviceTypeFromBluetdroid
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_011, TestSize.Level1)
{
    HILOGI("adapter_properties_test_011 enter");
    uint32_t deviceType= BT_DEVICE_TYPE_BREDR;
    EXPECT_EQ(adapterProperties.ConvertDeviceTypeFromBluetdroid(deviceType), DEVICE_TYPE_BREDR);
    deviceType = BT_DEVICE_TYPE_BLE;
    EXPECT_EQ(adapterProperties.ConvertDeviceTypeFromBluetdroid(deviceType), DEVICE_TYPE_LE);
    deviceType = BT_DEVICE_TYPE_DUMO;
    EXPECT_EQ(adapterProperties.ConvertDeviceTypeFromBluetdroid(deviceType), DEVICE_TYPE_DUAL_MONO);
    deviceType = DEVICE_TYPE_UNKNOWN;
    EXPECT_EQ(adapterProperties.ConvertDeviceTypeFromBluetdroid(deviceType), DEVICE_TYPE_UNKNOWN);
    HILOGI("adapter_properties_test_011 end");
}

/*
 * @tc.number:adapter_properties_test_012
 * @tc.name: ParseDeviceType
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_012, TestSize.Level1)
{
    HILOGI("adapter_properties_test_012 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = sizeof(uint32_t) - 1;
    EXPECT_EQ(adapterProperties.ParseDeviceType(properties), INVALID_VALUE);
    properties->len = sizeof(uint32_t);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    EXPECT_NE(adapterProperties.ParseDeviceType(properties), INVALID_VALUE);
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceType(properties), INVALID_VALUE);
    free(properties);
    HILOGI("adapter_properties_test_012 end");
}

/*
 * @tc.number:adapter_properties_test_013
 * @tc.name: ParseDeviceCod
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_013, TestSize.Level1)
{
    HILOGI("adapter_properties_test_013 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = sizeof(uint32_t) - 1;
    EXPECT_EQ(adapterProperties.ParseDeviceCod(properties), INVALID_VALUE);
    properties->len = sizeof(uint32_t);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    EXPECT_NE(adapterProperties.ParseDeviceCod(properties), INVALID_VALUE);
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceCod(properties), INVALID_VALUE);
    free(properties);
    HILOGI("adapter_properties_test_013 end");
}

/*
 * @tc.number:adapter_properties_test_014
 * @tc.name: ParseDeviceRssi
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_014, TestSize.Level1)
{
    HILOGI("adapter_properties_test_014 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = sizeof(uint8_t) - 1;
    EXPECT_EQ(adapterProperties.ParseDeviceRssi(properties), INVALID_VALUE);
    properties->len = sizeof(uint8_t);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    EXPECT_NE(adapterProperties.ParseDeviceRssi(properties), INVALID_VALUE);
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceRssi(properties), INVALID_VALUE);
    free(properties);
    HILOGI("adapter_properties_test_014 end");
}

/*
 * @tc.number:adapter_properties_test_015
 * @tc.name: ParseDeviceIoCapability
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_015, TestSize.Level1)
{
    HILOGI("adapter_properties_test_015 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = sizeof(int) - 1;
    EXPECT_EQ(adapterProperties.ParseDeviceIoCapability(properties), INVALID_VALUE);
    properties->len = sizeof(int);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    EXPECT_NE(adapterProperties.ParseDeviceIoCapability(properties), INVALID_VALUE);
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceIoCapability(properties), INVALID_VALUE);
    free(properties);
    HILOGI("adapter_properties_test_015 end");
}

/*
 * @tc.number:adapter_properties_test_016
 * @tc.name: ParseBondedDevices
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_016, TestSize.Level1)
{
    HILOGI("adapter_properties_test_016 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    adapterProperties.ParseBondedDevices(properties);
    properties->len = 12;
    properties->val = nullptr;
    adapterProperties.ParseBondedDevices(properties);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    std::vector<std::string> pairedAddrList = adapterProperties.ParseBondedDevices(properties);
    EXPECT_FALSE(pairedAddrList.empty());
    free(properties);
    HILOGI("adapter_properties_test_016 end");
}

/*
 * @tc.number:adapter_properties_test_017
 * @tc.name: ParseLocalLeFeatures
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_017, TestSize.Level1)
{
    HILOGI("adapter_properties_test_017 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    bt_local_le_features_t features;
    features.total_trackable_advertisers = 20;
    features.le_maximum_advertising_data_length = 1650;
    features.le_extended_advertising_supported = true;
    properties->len = sizeof(features);
    properties->val = nullptr;
    adapterProperties.ParseLocalLeFeatures(properties);
    properties->type = bt_property_type_t::BT_PROPERTY_LOCAL_LE_FEATURES;
    properties->val = static_cast<void*>(&features);
    adapterProperties.ParseLocalLeFeatures(properties);
    EXPECT_EQ(20, adapterProperties.GetTotalNumOfTrackableAdvertisements());
    EXPECT_EQ(1650, adapterProperties.GetBleMaxAdvertisingDataLength());
    EXPECT_TRUE(adapterProperties.GetLeExtendedAdvertisingSupported());
    free(properties);
    HILOGI("adapter_properties_test_017 end");
}

/*
 * @tc.number:adapter_properties_test_018
 * @tc.name: SetBroadcastName
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_018, TestSize.Level1)
{
    HILOGI("adapter_properties_test_018 enter");
    std::string deviceName = "adapter_properties_test_003";
    EXPECT_FALSE(adapterProperties.SetBroadcastName(deviceName));
    deviceName = "11111111111111111111111111111111111";
    EXPECT_FALSE(adapterProperties.SetBroadcastName(deviceName));
    EXPECT_FALSE(adapterProperties.GetSettingDeviceName(deviceName));
    EXPECT_FALSE(adapterProperties.SetBroadcastName(deviceName));
    HILOGI("adapter_properties_test_018 end");
}

/*
 * @tc.number:adapter_properties_test_019
 * @tc.name: ParseAlias
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_019, TestSize.Level1)
{
    HILOGI("adapter_properties_test_019 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    EXPECT_EQ(adapterProperties.ParseAlias(properties), INVALID_NAME);
    properties->len = 2;
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseAlias(properties), INVALID_NAME);
    int x = 10;
    properties->val = static_cast<void*>(&x);
    properties->len = 65;
    EXPECT_NE(adapterProperties.ParseAlias(properties), INVALID_NAME);
    properties->len = 50;
    EXPECT_NE(adapterProperties.ParseAlias(properties), INVALID_NAME);
    HILOGI("adapter_properties_test_019 end");
}

/*
 * @tc.number:adapter_properties_test_020
 * @tc.name: ParseAlias
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_020, TestSize.Level1)
{
    HILOGI("adapter_properties_test_020 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    EXPECT_EQ(adapterProperties.ParseAbsVolumeAbility(properties), DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT);

    properties->len = 4;
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseAbsVolumeAbility(properties), DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT);

    int32_t absVolumeAbility = 0;
    properties->val = static_cast<void*>(&absVolumeAbility);
    properties->len = sizeof(absVolumeAbility);

    absVolumeAbility = 0;
    EXPECT_NE(adapterProperties.ParseAbsVolumeAbility(properties), DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT);
    absVolumeAbility = 1;
    EXPECT_NE(adapterProperties.ParseAbsVolumeAbility(properties), DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT);
    absVolumeAbility = 2;
    EXPECT_EQ(adapterProperties.ParseAbsVolumeAbility(properties), DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT);

    free(properties);
    HILOGI("adapter_properties_test_020 end");
}

/*
 * @tc.number:adapter_properties_test_021
 * @tc.name: ParseAlias
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_021, TestSize.Level1)
{
    HILOGI("adapter_properties_test_021 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    EXPECT_EQ(adapterProperties.ParseDeviceCustomType(properties), 0);

    properties->len = 4;
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceCustomType(properties), 0);

    int32_t customType = 0;
    properties->val = static_cast<void*>(&customType);
    properties->len = sizeof(customType);

    EXPECT_EQ(adapterProperties.ParseDeviceCustomType(properties), 0);

    free(properties);
    HILOGI("adapter_properties_test_021 end");
}

/*
 * @tc.number:adapter_properties_test_022
 * @tc.name: ParseRemoteDeviceIoCapability
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_022, TestSize.Level1)
{
    HILOGI("adapter_properties_test_022 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    EXPECT_EQ(adapterProperties.ParseRemoteDeviceIoCapability(properties), INVALID_VALUE);

    properties->len = 4;
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseRemoteDeviceIoCapability(properties), INVALID_VALUE);

    int32_t IoCapability = 0;
    properties->val = static_cast<void*>(&IoCapability);
    properties->len = sizeof(IoCapability);

    IoCapability = 0;
    EXPECT_EQ(adapterProperties.ParseRemoteDeviceIoCapability(properties), INVALID_VALUE);
    IoCapability = 1;
    EXPECT_NE(adapterProperties.ParseRemoteDeviceIoCapability(properties), INVALID_VALUE);
    IoCapability = 2;
    EXPECT_NE(adapterProperties.ParseRemoteDeviceIoCapability(properties), INVALID_VALUE);

    free(properties);
    HILOGI("adapter_properties_test_022 end");
}

/*
 * @tc.number:adapter_properties_test_023
 * @tc.name: ParseAlias
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_023, TestSize.Level1)
{
    HILOGI("adapter_properties_test_023 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    EXPECT_EQ(adapterProperties.ParseDeviceVendorId(properties), INVALID_VALUE);

    properties->len = 4;
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceVendorId(properties), INVALID_VALUE);

    int32_t vendorId = 0;
    properties->val = static_cast<void*>(&vendorId);
    properties->len = sizeof(vendorId);
    EXPECT_EQ(adapterProperties.ParseDeviceVendorId(properties), INVALID_VALUE);

    free(properties);
    HILOGI("adapter_properties_test_023 end");
}

/*
 * @tc.number:adapter_properties_test_024
 * @tc.name: ParseAlias
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_024, TestSize.Level1)
{
    HILOGI("adapter_properties_test_024 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    EXPECT_EQ(adapterProperties.ParseDeviceProductId(properties), INVALID_VALUE);

    properties->len = 4;
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceProductId(properties), INVALID_VALUE);

    int32_t productId = 0;
    properties->val = static_cast<void*>(&productId);
    properties->len = sizeof(productId);
    EXPECT_EQ(adapterProperties.ParseDeviceProductId(properties), INVALID_VALUE);

    free(properties);
    HILOGI("adapter_properties_test_024 end");
}

/*
 * @tc.number:adapter_properties_test_025
 * @tc.name: ParseAlias
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_025, TestSize.Level1)
{
    HILOGI("adapter_properties_test_025 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    EXPECT_EQ(adapterProperties.ParseDeviceAutoConnSwitch(properties), INVALID_VALUE);

    properties->len = 4;
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceAutoConnSwitch(properties), INVALID_VALUE);

    int32_t autoConnSwitch = 0;
    properties->val = static_cast<void*>(&autoConnSwitch);
    properties->len = sizeof(autoConnSwitch);
    EXPECT_EQ(adapterProperties.ParseDeviceAutoConnSwitch(properties), INVALID_VALUE);

    free(properties);
    HILOGI("adapter_properties_test_025 end");
}

/*
 * @tc.number:adapter_properties_test_026
 * @tc.name: LoadSettingsData
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_026, TestSize.Level1)
{
    HILOGI("adapter_properties_test_026 enter");
    std::string deviceName = "";
    EXPECT_EQ(adapterProperties.GetTruncationName(deviceName), deviceName);
    deviceName = "11111111111111111111111111111111111";
    EXPECT_EQ(adapterProperties.GetTruncationName(deviceName).length(), 30);
    HILOGI("adapter_properties_test_026 end");
}

/*
 * @tc.number:adapter_properties_test_027
 * @tc.name: ParseAlias
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_027, TestSize.Level1)
{
    HILOGI("adapter_properties_test_027 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    properties->len = -1;
    EXPECT_EQ(adapterProperties.ParseDeviceConnectionTime(properties), INVALID_VALUE);

    properties->len = 4;
    properties->val = nullptr;
    EXPECT_EQ(adapterProperties.ParseDeviceConnectionTime(properties), INVALID_VALUE);

    int64_t connectionTime = 0;
    properties->val = static_cast<void*>(&connectionTime);
    properties->len = sizeof(connectionTime);

    EXPECT_EQ(adapterProperties.ParseDeviceConnectionTime(properties), INVALID_VALUE);

    free(properties);
    HILOGI("adapter_properties_test_027 end");
}

/*
 * @tc.number:adapter_properties_test_028
 * @tc.name: GetTruncationName
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_028, TestSize.Level1)
{
    HILOGI("adapter_properties_test_028 enter");
    std::string name = "蓝牙Mate 60 pro 1234😀😀😀";
    std::string truncationName = adapterProperties.GetTruncationName(name);
    EXPECT_EQ(truncationName.length(), 30);

    std::string name1 = "蓝牙Mate 60 pro 123😀😀😀";
    std::string truncationName1 = adapterProperties.GetTruncationName(name1);
    EXPECT_EQ(truncationName1.length(), 30);
    HILOGI("adapter_properties_test_028 end");
}

/*
 * @tc.number:adapter_properties_test_029
 * @tc.name: GetUTF8StringLength
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_029, TestSize.Level1)
{
    HILOGI("adapter_properties_test_029 enter");
    const char firstByte = '\xc2';
    OHOS::bluetooth::AdapterProperties adapterProperties;
    int length = adapterProperties.GetUTF8StringLength(firstByte);
    EXPECT_EQ(length, 2);

    HILOGI("adapter_properties_test_029 end");
}

/*
 * @tc.number:adapter_properties_test_30
 * @tc.name: RegisterNameChangeObserver
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_30, TestSize.Level1)
{
    HILOGI("adapter_properties_test_30 enter");
    EXPECT_TRUE(adapterProperties.RegisterNameChangeObserver());
    adapterProperties.UnregisterNameChangeObserver();
    HILOGI("adapter_properties_test_30 end");
}

/*
 * @tc.number:adapter_properties_test_031
 * @tc.name: ParseScanMode
 * @tc.desc:
 */
HWTEST_F(AdapterPropertiesTest, adapter_properties_test_031, TestSize.Level1)
{
    HILOGI("adapter_properties_test_031 enter");
    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    int curScanMode = static_cast<int>(BT_SCAN_MODE_NONE);
    properties->len = -1;

    adapterProperties.HandlePropertyScanMode(properties);
    bool res = true;
    res = adapterProperties.ParseScanMode(properties, curScanMode);
    EXPECT_EQ(res, false);

    properties->len = 4;
    properties->val = static_cast<void*>(&curScanMode);
    adapterProperties.HandlePropertyScanMode(properties);
    int newScanMode = -1;
    res = adapterProperties.ParseScanMode(properties, newScanMode);
    EXPECT_EQ(res, true);
    EXPECT_EQ(newScanMode, SCAN_MODE_NONE);

    curScanMode = static_cast<int>(BT_SCAN_MODE_CONNECTABLE);
    properties->len = 4;
    properties->val = static_cast<void*>(&curScanMode);
    adapterProperties.HandlePropertyScanMode(properties);
    res = adapterProperties.ParseScanMode(properties, newScanMode);
    EXPECT_EQ(res, true);
    EXPECT_EQ(newScanMode, SCAN_MODE_CONNECTABLE);

    curScanMode = static_cast<int>(BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
    properties->len = 4;
    properties->val = static_cast<void*>(&curScanMode);
    res = adapterProperties.ParseScanMode(properties, newScanMode);
    EXPECT_EQ(res, true);
    EXPECT_EQ(newScanMode, SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE);
    HILOGI("adapter_properties_test_031 end");
}

}  // namespace bluetooth
}  // namespace OHOS