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
#include "ble_parcel_data.h"
#include "bluetooth_log.h"
#include "bluetooth_host.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;

class BleParcelDataTest : public testing::Test {
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
    {
    }

    void TearDown()
    {
    }

public:
};
/**
 * @tc.name: ScanResult_00100
 * @tc.desc: Verify ScanResult function.
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BLE_ModuleTest_ScanResult_00100, TestSize.Level1)
{
    HILOGE("BLE_ModuleTest_ScanResult_00100 start");
    BlePeripheralDevice peripheralDevice;
    peripheralDevice.SetRSSI(0x01);
    EXPECT_EQ(peripheralDevice.GetRSSI(), 0x01);
    std::string data = "testdatatestdatatestdata";
    peripheralDevice.SetManufacturerData(data);
    Uuid uuid;
    peripheralDevice.SetServiceUUID(uuid);
    peripheralDevice.SetServiceDataUUID(uuid, data);
    peripheralDevice.SetName(data);
    EXPECT_EQ(peripheralDevice.GetName(), data);

    int8_t txPower = 1;
    peripheralDevice.SetTXPower(txPower);
    EXPECT_EQ(peripheralDevice.GetTXPower(), txPower);
    EXPECT_EQ(peripheralDevice.IsTXPower(), true);

    uint8_t advType1 = 0xFF;
    std::string advData1 = "testManufactureData";
    peripheralDevice.AddAdvertisingData(advType1, advData1);
    EXPECT_EQ(peripheralDevice.IsAdvertisingData(), true);
    std::map<uint8_t, std::string> advertiseDataMap1 = peripheralDevice.GetAdvertisingData();
    auto it1 = advertiseDataMap1.find(advType1);
    if (it1 != peripheralDevice.GetAdvertisingData().end()) {
        EXPECT_EQ(it1->second, advData1);
    }

    uint16_t timestamp = 123;
    peripheralDevice.SetTimestamp(timestamp);
    EXPECT_EQ(peripheralDevice.GetTimestamp(), timestamp);
    EXPECT_EQ(peripheralDevice.IsTimestamp(), true);

    BleScanResultImpl bleScanResultImpl;
    bleScanResultImpl.SetPeripheralDevice(peripheralDevice);
    ScanResult scanResult(bleScanResultImpl);
    bleScanResultImpl.GetServiceUuids();
    bleScanResultImpl.GetServiceData();

    txPower = -1;
    scanResult.SetTxPowerLevel(txPower);
    EXPECT_EQ(scanResult.GetTxPowerLevel(), txPower);

    timestamp = 456;
    scanResult.SetTimestamp(timestamp);
    EXPECT_EQ(scanResult.GetTimestamp(), timestamp);

    uint8_t advType2 = 0x09;
    std::string advData2 = "testLocalName";
    scanResult.AddAdvertisingData(advType2, advData2);
    std::map<uint8_t, std::string> advertiseDataMap2 = scanResult.GetAdvertisingData();
    auto it2 = advertiseDataMap2.find(advType2);
    if (it2 != scanResult.GetAdvertisingData().end()) {
        EXPECT_EQ(it2->second, advData2);
    }
    HILOGE("BLE_ModuleTest_ScanResult_00200 end");
}

/**
 * @tc.name: GetAppearance_00100
 * @tc.desc: GetAppearance
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BLE_ModuleTest_GetAppearance_00100, TestSize.Level1)
{
    HILOGE("BLE_ModuleTest_GetAppearance_00100 start");
    BlePeripheralDevice peripheralDevice;
    uint16_t appearance = 1;
    peripheralDevice.SetAppearance(appearance);
    EXPECT_EQ(peripheralDevice.GetAppearance(), appearance);
    HILOGE("BLE_ModuleTest_GetAppearance_00100 end");
}

/**
 * @tc.name: SetConnectable_00100
 * @tc.desc: SetConnectable
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BLE_ModuleTest_SetConnectable_00100, TestSize.Level1)
{
    HILOGE("BLE_ModuleTest_SetConnectable_00100 start");
    BlePeripheralDevice peripheralDevice;
    peripheralDevice.SetConnectable(true);
    EXPECT_TRUE(peripheralDevice.IsConnectable());
    peripheralDevice.SetConnectable(false);
    EXPECT_FALSE(peripheralDevice.IsConnectable());
    HILOGE("BLE_ModuleTest_SetConnectable_00100 end");
}

/**
 * @tc.name: SetRoles_00100
 * @tc.desc: SetRoles
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BLE_ModuleTest_SetRoles_00100, TestSize.Level1)
{
    HILOGE("BLE_ModuleTest_SetRoles_00100 start");
    uint8_t roles = 0x01;
    BlePeripheralDevice peripheralDevice;
    peripheralDevice.SetRoles(roles);
    EXPECT_EQ(peripheralDevice.roles_, roles);
    HILOGE("BLE_ModuleTest_SetRoles_00100 end");
}

/**
 * @tc.name: SetAliasName_00100
 * @tc.desc: SetAliasName
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BLE_ModuleTest_SetAliasName_00100, TestSize.Level1)
{
    HILOGE("BLE_ModuleTest_SetAliasName_00100 start");
    std::string name = "00:11:22:33:44:55";
    BlePeripheralDevice peripheralDevice;
    peripheralDevice.SetAliasName(name);
    EXPECT_EQ(peripheralDevice.GetAliasName(), name);
    HILOGE("BLE_ModuleTest_SetAliasName_00100 end");
}

/**
 * @tc.name: SetIoCapability_00100
 * @tc.desc: SetIoCapability
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BLE_ModuleTest_SetIoCapability_00100, TestSize.Level1)
{
    HILOGE("BLE_ModuleTest_SetIoCapability_00100 start");
    uint8_t io = 0x01;
    BlePeripheralDevice peripheralDevice;
    peripheralDevice.SetIoCapability(io);
    EXPECT_EQ(peripheralDevice.GetIoCapability(), io);
    HILOGE("BLE_ModuleTest_SetIoCapability_00100 end");
}

/**
 * @tc.name: SetTimestamp_00100
 * @tc.desc: SetTimestamp
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BLE_ModuleTest_SetTimestamp_00100, TestSize.Level1)
{
    HILOGE("BLE_ModuleTest_SetTimestamp_00100 start");
    uint16_t timestamp = 123;
    BlePeripheralDevice peripheralDevice;
    peripheralDevice.SetTimestamp(timestamp);
    EXPECT_EQ(peripheralDevice.GetTimestamp(), timestamp);
    HILOGE("BLE_ModuleTest_SetTimestamp_00100 end");
}
 
/**
 * @tc.name: GetManufacturerData_00100
 * @tc.desc: Verify GetManufacturerData function.
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BleScanResultImpl_GetManufacturerData_00100, TestSize.Level1)
{
    HILOGE("BleScanResultImpl_GetManufacturerData_00100 start");
    BleScanResultImpl bleScanResultImpl;
    uint16_t manufacturerId = 1;
    std::string data = "testData";
    bleScanResultImpl.AddManufacturerData(manufacturerId, data);
    std::map<uint16_t, std::string> map = bleScanResultImpl.GetManufacturerData();
    auto it = map.find(manufacturerId);
    EXPECT_NE(it, map.end());
    EXPECT_EQ(it->second, data);
    HILOGE("BleScanResultImpl_GetManufacturerData_00100 end");
}
 
/**
 * @tc.name: GetRssi_00100
 * @tc.desc: Verify GetRssi function.
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BleScanResultImpl_GetRssi_00100, TestSize.Level1)
{
    HILOGE("BleScanResultImpl_GetRssi_00100 start");
    BleScanResultImpl bleScanResultImpl;
    int8_t rssi = 1;
    bleScanResultImpl.SetRssi(rssi);
    EXPECT_EQ(bleScanResultImpl.GetRssi(), rssi);
    HILOGE("BleScanResultImpl_GetRssi_00100 end");
}

/**
 * @tc.name: SetConnectable_00100
 * @tc.desc: Verify SetConnectable function.
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BleScanResultImpl_SetConnectable_00100, TestSize.Level1)
{
    HILOGE("BleScanResultImpl_SetConnectable_00100 start");
    BleScanResultImpl bleScanResultImpl;
    bool connectable = false;
    bleScanResultImpl.SetConnectable(connectable);
    EXPECT_FALSE(bleScanResultImpl.IsConnectable());
    HILOGE("BleScanResultImpl_SetConnectable_00100 end");
}

/**
 * @tc.name: SetAdvertiseFlag_00100
 * @tc.desc: Verify SetAdvertiseFlag function.
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BleScanResultImpl_SetAdvertiseFlag_00100, TestSize.Level1)
{
    HILOGE("BleScanResultImpl_SetAdvertiseFlag_00100 start");
    BleScanResultImpl bleScanResultImpl;
    uint8_t flag = 1;
    bleScanResultImpl.SetAdvertiseFlag(flag);
    EXPECT_EQ(bleScanResultImpl.GetAdvertiseFlag(), flag);
    HILOGE("BleScanResultImpl_SetAdvertiseFlag_00100 end");
}

/**
 * @tc.name: AddServiceUuid_00100
 * @tc.desc: Verify AddServiceUuid function.
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BleScanResultImpl_AddServiceUuid_00100, TestSize.Level1)
{
    HILOGE("BleScanResultImpl_AddServiceUuid_00100 start");
    BleScanResultImpl bleScanResultImpl;
    Uuid uuid;
    bleScanResultImpl.AddServiceUuid(uuid);
    std::string serviceData = "testServiceData";
    bleScanResultImpl.AddServiceData(uuid, serviceData);
    EXPECT_FALSE(bleScanResultImpl.IsConnectable());
    HILOGE("BleScanResultImpl_AddServiceUuid_00100 end");
}

/**
 * @tc.name: SetTxPowerLevel_00100
 * @tc.desc: Verify SetTxPowerLevel function.
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BleScanResultImpl_SetTxPowerLevel_00100, TestSize.Level1)
{
    HILOGE("BleScanResultImpl_SetTxPowerLevel_00100 start");
    BleScanResultImpl bleScanResultImpl;
    int8_t txPowerLevel = 1;
    bleScanResultImpl.SetTxPowerLevel(txPowerLevel);
    EXPECT_EQ(bleScanResultImpl.GetTxPowerLevel(), txPowerLevel);
    HILOGE("BleScanResultImpl_SetTxPowerLevel_00100 end");
}

/**
 * @tc.name: AddAdvertisingData_00100
 * @tc.desc: Verify AddAdvertisingData function.
 * @tc.type: FUNC
 */
HWTEST_F(BleParcelDataTest, BleScanResultImpl_AddAdvertisingData_00100, TestSize.Level1)
{
    HILOGE("BleScanResultImpl_AddAdvertisingData_00100 start");
    BleScanResultImpl bleScanResultImpl;
    
    uint8_t advType = 0xFF;
    std::string advData = "testManufactureData";
    bleScanResultImpl.AddAdvertisingData(advType, advData);
    std::map<uint8_t, std::string> advertiseDataMap = bleScanResultImpl.GetAdvertisingData();
    auto it = advertiseDataMap.find(advType);
    if (it != bleScanResultImpl.GetAdvertisingData().end()) {
        EXPECT_EQ(it->second, advData);
    }
    HILOGE("BleScanResultImpl_AddAdvertisingData_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS