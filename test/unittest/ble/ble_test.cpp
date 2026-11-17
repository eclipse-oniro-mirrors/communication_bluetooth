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

#include "ble_test.h"
#include "bluetooth_access_token_mock.h"
#include "bluetooth_def.h"
#include "bluetooth_errorcode.h"
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 2;
BleTest *BleTest::bleInstance_ = nullptr;
static BluetoothHost *host_;
std::shared_ptr<BleCentralManagerCallbackTest> bleCentralManagerCallbackTest_ =
    std::make_shared<BleCentralManagerCallbackTest>();
std::shared_ptr<BleCentralManager> bleCentralManager =
    std::make_shared<BleCentralManager>(bleCentralManagerCallbackTest_);

namespace {
const static int defaultInt = 150;
const static int INTERVAL = 32;
const static int MIN_ADV_LENGTH = 31;
const static int MAX_ADV_LENGTH = 1650;
Bluetooth::UUID g_uuid = Bluetooth::UUID::FromString("00000000-0000-1000-8000-00805F9B34FB");
Bluetooth::UUID g_serviceDataUuid = Bluetooth::UUID::FromString("00000000-0000-1000-8000-00805F9B34FB");
Bluetooth::UUID g_serviceUuid = Bluetooth::UUID::FromString("12345678-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
Bluetooth::UUID g_serviceUuidMask = Bluetooth::UUID::FromString("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
Bluetooth::UUID g_serviceSolicitationUuid = Bluetooth::UUID::FromString("87654321-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
Bluetooth::UUID g_serviceSolicitationUuidMask = Bluetooth::UUID::FromString("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
Bluetooth::UUID g_lpDeviceUuid = Bluetooth::UUID::FromString("43D4A49F-6050-45B5-9302-4DDBBFD538FD");
std::string g_serviceData = "123";
int g_manufacturerId = 24;
std::string g_manufacturerData = "1567";

int g_deliveryMode = 10;
int g_duration = 20;
int g_advHandle = 1;
int g_startAdvHandle = 0xFF;
BluetoothRemoteDevice device;
}  // namespace
class BluetoothBleCentralManagerCallbackTest : public BluetoothBleCentralManagerCallBackStub {
    public:
        explicit BluetoothBleCentralManagerCallbackTest()
        {};
        ~BluetoothBleCentralManagerCallbackTest() override = default;
        void OnScanCallback(const BluetoothBleScanResult &result, uint8_t callbackType) override
        {}
        void OnBleBatchScanResultsEvent(std::vector<BluetoothBleScanResult> &results) override
        {}

        void OnStartOrStopScanEvent(int resultCode, bool isStartScan) override
        {}

        void OnNotifyMsgReportFromLpDevice(const bluetooth::Uuid &uuid, int msgType,
            const std::vector<uint8_t> &value) override
        {}

    private:
    };
void BleTest::InitAdvertiseSettings()
{
    BleTest::bleInstance_->bleAdvertiserSettings_.SetConnectable(true);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetLegacyMode(true);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetInterval(INTERVAL);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetTxPower(BLE_ADV_TX_POWER_MEDIUM_VALUE);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetPrimaryPhy(PHY_TYPE::PHY_LE_1M);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetSecondaryPhy(PHY_TYPE::PHY_LE_2M);
}
void BleTest::InitScanSettings()
{
    BleTest::bleInstance_->bleScanSettings_.SetReportDelay(defaultInt);
    BleTest::bleInstance_->bleScanSettings_.SetScanMode(SCAN_MODE::SCAN_MODE_LOW_POWER);
    BleTest::bleInstance_->bleScanSettings_.SetLegacy(true);
    BleTest::bleInstance_->bleScanSettings_.SetPhy(PHY_TYPE::PHY_LE_1M);
    BleTest::bleInstance_->bleScanSettings_.SetReportMode(1);
    BleTest::bleInstance_->bleScanSettings_.SetEnhanceMode(1, 1);
    BleTest::bleInstance_->bleScanSettings_.SetCallbackType(BLE_SCAN_CALLBACK_TYPE_ALL_MATCH);
    BleTest::bleInstance_->bleScanSettings_.SetSensitivityMode(SENSITIVITY_MODE::SENSITIVITY_MODE_HIGH);
    BleTest::bleInstance_->bleScanSettings_.SetMatchTrackAdvType(MATCH_TRACK_ADV_TYPE::MAX_MATCH_TRACK_ADV);
}

void BleTest::InitScanResult()
{
    BleTest::bleInstance_->bleScanResult_.SetPayload(g_serviceData);
    BleTest::bleInstance_->bleScanResult_.SetPeripheralDevice(device);
    BleTest::bleInstance_->bleScanResult_.SetRssi(g_deliveryMode);
    BleTest::bleInstance_->bleScanResult_.SetConnectable(true);
    BleTest::bleInstance_->bleScanResult_.SetAdvertiseFlag(g_advHandle);
    BleTest::bleInstance_->bleScanResult_.SetName(g_manufacturerData);
}

bool BleTest::HaveUuid(Bluetooth::BleAdvertiserData advData)
{
    for (Bluetooth::UUID uuid : advData.GetServiceUuids()) {
        if (uuid.Equals(g_uuid)) {
            isHaveUuid = true;
        }
    }
    return isHaveUuid;
}

bool BleTest::ManufacturerData(Bluetooth::BleAdvertiserData advData)
{
    if (advData.GetManufacturerData().empty()) {
        return false;
    } else {
        std::map<uint16_t, std::string> data = advData.GetManufacturerData();
        std::map<uint16_t, std::string>::iterator iter = data.begin();
        for (; iter != data.end(); ++iter) {
            if (g_manufacturerId == iter->first) {
                EXPECT_STREQ(g_manufacturerData.c_str(), iter->second.c_str());
            }
        }
    }
    return true;
}

bool BleTest::ServiceData(Bluetooth::BleAdvertiserData advData)
{
    if (advData.GetServiceData().empty()) {
        return false;
    } else {
        for (auto &iter : advData.GetServiceData()) {
            if (g_serviceData == iter.second) {
                EXPECT_STREQ(g_serviceData.c_str(), iter.second.c_str());
            }
        }
    }
    return true;
}

bool BleTest::SetLocalName(const std::string &name)
{
    HILOGI("SetLocalName ble start");
    int ret = host_->SetLocalName(name);
    if (ret == BT_NO_ERROR) {
        return true;
    }
    return false;
}

void BleTest::InitAdvData()
{
    advData1.AddServiceUuid(g_uuid); // uuidlen = 2 + 2 = 4
    advData1.AddManufacturerData(g_manufacturerId, "7"); // manulen = 2 + 2 + 1 = 5
    advData1.AddServiceData(g_serviceDataUuid, "1"); //ServiceDatalen = 2 + 2 + 1 = 5
    advData1.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    advData1.SetIncludeDeviceName(true); // namelen = 14

    advData2.AddServiceUuid(g_uuid); // uuidlen = 2 + 2 = 4
    advData2.AddManufacturerData(g_manufacturerId, "1567"); // manulen = 2 + 2 + 4 = 8
    advData2.AddServiceData(g_serviceDataUuid, "123"); //ServiceDatalen = 2 + 2 + 3 = 7
    advData2.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    advData2.SetIncludeDeviceName(true); // namelen = 14
}

void BleTest::InitScanData()
{
    scanData1.AddServiceUuid(g_uuid); // uuidlen = 2 + 2 = 4
    scanData1.AddManufacturerData(g_manufacturerId, "8"); // manulen = 2 + 2 + 1 = 5
    scanData1.AddServiceData(g_serviceDataUuid, "2"); //ServiceDatalen = 2 + 2 + 1 = 5
    scanData1.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    scanData1.SetIncludeDeviceName(true); // namelen = 14

    scanData2.AddServiceUuid(g_uuid); // uuidlen = 2 + 2 = 4
    scanData2.AddManufacturerData(g_manufacturerId, "2345"); // manulen = 2 + 2 + 4 = 8
    scanData2.AddServiceData(g_serviceDataUuid, "789"); //ServiceDatalen = 2 + 2 + 3 = 7
    scanData2.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    scanData2.SetIncludeDeviceName(true); // namelen = 14
}

void BleTest::InitAdvFilterData()
{
    BleTest::bleInstance_->InitAdvertiseSettings();
    BleTest::bleInstance_->InitScanSettings();
    std::vector<BleScanFilter> filters;
    BleScanFilter filter;
    std::string deviceId = "42:0B:AC:A0:A0:ED";
    uint16_t manufacturerId = g_manufacturerId;
    std::vector<uint8_t> serviceData = {0xee, 0xfd, 0x00, 0x05, 0x40};
    std::vector<uint8_t> serviceDataMask = {0xff, 0xff, 0x00, 0xff, 0xff};
    std::vector<uint8_t> manufactureData = {0x01, 0x09, 0x20, 0x02};
    std::vector<uint8_t> manufactureDataMask = {0x01, 0x09, 0x20, 0x02};
    filter.SetDeviceId(deviceId);
    filter.SetName("bluetoothdevice");
    filter.SetServiceUuid(g_serviceUuid);
    filter.SetServiceUuidMask(g_serviceUuidMask);
    filter.SetServiceSolicitationUuid(g_serviceSolicitationUuid);
    filter.SetServiceSolicitationUuidMask(g_serviceSolicitationUuidMask);
    filter.SetManufacturerId(manufacturerId);
    filter.SetManufactureData(manufactureData);
    filter.SetManufactureDataMask(manufactureDataMask);
    filter.SetServiceData(serviceData);
    filter.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter);
    std::vector<uint8_t> value = {0xB5, 0x45, 0x50, 0x60, 0x9F, 0xA4, 0xD4, 0x43, 0xFD, 0x38, 0xD5, 0xBF, 0xDB, 0x4D,
        0x02, 0x93, 0x01, 0x00, 0x00, 0x00};
    std::vector<BleActiveDeviceInfo> deviceinfos;
    BleActiveDeviceInfo deviceinfo;
    deviceinfo.status = 1;
    deviceinfo.timeOut = 1;
    deviceinfo.deviceId = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    deviceinfos.push_back(deviceinfo);
    paramSet.scanSettings = BleTest::bleInstance_->bleScanSettings_;
    paramSet.scanFilters = filters;
    paramSet.advSettings = BleTest::bleInstance_->bleAdvertiserSettings_;
    paramSet.advData = {1, 2, 3};
    paramSet.respData = {3, 2, 1};
    paramSet.advHandle = g_advHandle;
    paramSet.deliveryMode = g_deliveryMode;
    paramSet.duration = g_duration;
    paramSet.uuid = g_lpDeviceUuid;
}

std::shared_ptr<BluetoothBleCentralManagerCallbackTest> bluetoothBleCentralManagerCallbackTest {nullptr};

BleTest::BleTest()
{
    bleInstance_ = this;
}
BleTest::~BleTest()
{}

void BleTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
    host_ = &BluetoothHost::GetDefaultHost();

    bluetoothBleCentralManagerCallbackTest = std::make_shared<BluetoothBleCentralManagerCallbackTest>();

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

void BleTest::TearDownTestCase(void)
{
    host_ = nullptr;
}

void BleTest::SetUp()
{}

void BleTest::TearDown()
{}

void BleAdvertiseCallbackTest::OnStartResultEvent(int result, int advHandle)
{
    HILOGI("OnStartResultEvent result:%{public}d advHandle:%{public}d", result, advHandle);
}

void BleAdvertiseCallbackTest::OnEnableResultEvent(int result, int advHandle)
{
    HILOGI("OnEnableResultEvent result:%{public}d advHandle:%{public}d", result, advHandle);
}

void BleAdvertiseCallbackTest::OnDisableResultEvent(int result, int advHandle)
{
    HILOGI("OnDisableResultEvent result:%{public}d advHandle:%{public}d", result, advHandle);
}

void BleAdvertiseCallbackTest::OnStopResultEvent(int result, int advHandle)
{
    HILOGI("OnStopResultEvent result:%{public}d advHandle:%{public}d", result, advHandle);
    g_startAdvHandle = 0xFF;
}

void BleAdvertiseCallbackTest::OnSetAdvDataEvent(int result)
{}

void BleAdvertiseCallbackTest::OnGetAdvHandleEvent(int result, int advHandle)
{
    HILOGI("OnGetAdvHandleEvent result:%{public}d advHandle:%{public}d", result, advHandle);
    g_startAdvHandle = advHandle;
}

void BleAdvertiseCallbackTest::OnChangeAdvResultEvent(int result, int advHandle)
{
    HILOGI("OnChangeAdvResultEvent result:%{public}d advHandle:%{public}d", result, advHandle);
}

void BleHostObserverTest::OnStateChanged(const int transport, const int status)
{
    HILOGI("OnStateChanged");
}


/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00100, TestSize.Level1)
{
    HILOGI("Ble_ModuleTest_StartAdvertising_00100 start");

    BleTest::bleInstance_->InitAdvertiseSettings();
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    BleAdvertiserData advData;
    BleAdvertiserData scanData;
    uint16_t duration = 0;
    advData.AddServiceUuid(g_uuid);
    advData.AddManufacturerData(g_manufacturerId, g_manufacturerData);
    advData.AddServiceData(g_serviceDataUuid, g_serviceData);
    advData.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData, scanData, duration, bleAdvertiseCallbackTest_);

    HILOGI("advData function test");
    EXPECT_TRUE(BleTest::bleInstance_->HaveUuid(advData));
    EXPECT_TRUE(BleTest::bleInstance_->ManufacturerData(advData));
    EXPECT_TRUE(BleTest::bleInstance_->ServiceData(advData));
    EXPECT_EQ(BLE_ADV_FLAG_GEN_DISC, advData.GetAdvFlag());

    bleAdvertise->StopAdvertising(bleAdvertiseCallbackTest_);
    vector<uint8_t> advData1 = {1, 2, 3};
    vector<uint8_t> scanData1 = {3, 2, 1};
    bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData1, scanData1, duration, bleAdvertiseCallbackTest_);
    bleAdvertise->StopAdvertising(bleAdvertiseCallbackTest_);
    int32_t length = host_->GetBleMaxAdvertisingDataLength();
    EXPECT_LE(MIN_ADV_LENGTH, length);
    EXPECT_GE(MAX_ADV_LENGTH, length);

    bleAdvertise->Close(bleAdvertiseCallbackTest_);
    HILOGI("Ble_ModuleTest_StartAdvertising_00100 end");
}

HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00200, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_StartAdvertising_00200 start");
    BleTest::bleInstance_->InitAdvertiseSettings();
    EXPECT_TRUE(BleTest::bleInstance_->bleAdvertiserSettings_.IsConnectable());
    EXPECT_TRUE(BleTest::bleInstance_->bleAdvertiserSettings_.IsLegacyMode());
    EXPECT_EQ(static_cast<uint16_t>(INTERVAL), BleTest::bleInstance_->bleAdvertiserSettings_.GetInterval());
    EXPECT_EQ(BLE_ADV_TX_POWER_MEDIUM_VALUE, BleTest::bleInstance_->bleAdvertiserSettings_.GetTxPower());
    EXPECT_EQ(PHY_TYPE::PHY_LE_1M, BleTest::bleInstance_->bleAdvertiserSettings_.GetPrimaryPhy());
    EXPECT_EQ(PHY_TYPE::PHY_LE_2M, BleTest::bleInstance_->bleAdvertiserSettings_.GetSecondaryPhy());
    HILOGI("BLE_ModuleTest_StartAdvertising_00200 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00300
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00300, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    HILOGI("BLE_ModuleTest_StartAdvertising_00300 start");
    std::string deviceName = "BluetoothDev";
    EXPECT_TRUE(SetLocalName(deviceName)); // namelen = 12 + 2 = 14
    BleTest::bleInstance_->InitAdvertiseSettings();
    BleTest::bleInstance_->InitAdvData();
    BleTest::bleInstance_->InitScanData();
    uint16_t duration = 0;
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, BleTest::bleInstance_->advData1,
        BleTest::bleInstance_->scanData1, duration, bleAdvertiseCallbackTest_);
    bleAdvertise->StopAdvertising(bleAdvertiseCallbackTest_);
    HILOGI("Ble_ModuleTest_StartAdvertising_00300 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00400
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00400, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    HILOGI("BLE_ModuleTest_StartAdvertising_00400 start");
    std::string deviceName = "BluetoothDev";
    EXPECT_TRUE(SetLocalName(deviceName)); // namelen = 13 + 2 = 15
    BleTest::bleInstance_->InitAdvertiseSettings();
    BleTest::bleInstance_->InitAdvData();
    BleTest::bleInstance_->InitScanData();
    uint16_t duration = 0;
    auto bleAdvertise = BleAdvertiser::CreateInstance();

    int ret = bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, BleTest::bleInstance_->advData2,
        BleTest::bleInstance_->scanData1, duration, bleAdvertiseCallbackTest_);
    EXPECT_NE(ret, BT_NO_ERROR);

    ret = bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, BleTest::bleInstance_->advData1,
        BleTest::bleInstance_->scanData2, duration, bleAdvertiseCallbackTest_);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("Ble_ModuleTest_StartAdvertising_00400 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00500
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00500, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    HILOGI("BLE_ModuleTest_StartAdvertising_00500 start");
    std::string deviceName = "BluetoothDev";
    EXPECT_TRUE(SetLocalName(deviceName)); // namelen = 13 + 2 = 15
    BleTest::bleInstance_->InitAdvertiseSettings();
    BleTest::bleInstance_->bleAdvertiserSettings_.SetLegacyMode(false);
    BleTest::bleInstance_->InitAdvData();
    BleTest::bleInstance_->InitScanData();
    uint16_t duration = 0;
    auto bleAdvertise = BleAdvertiser::CreateInstance();

    bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, BleTest::bleInstance_->advData2,
        BleTest::bleInstance_->scanData2, duration, bleAdvertiseCallbackTest_);
    int handle = bleAdvertise->GetAdvHandle(bleAdvertiseCallbackTest_);
    EXPECT_GE(handle, 0);
    bleAdvertise->StopAdvertising(bleAdvertiseCallbackTest_);
    HILOGI("BLE_ModuleTest_StartAdvertising_00500 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00600
 * @tc.name:
 * @tc.desc: test start adv and callback, enable adv and callback, disable adv and callbacl, stop adv and callback
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00600, TestSize.Level1)
{
    HILOGI("Ble_ModuleTest_StartAdvertising_00600 start %{public}d", g_startAdvHandle);
    BleTest::bleInstance_->InitAdvertiseSettings();
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    BleAdvertiserData advData;
    BleAdvertiserData scanData;
    uint16_t duration = 0;
    advData.AddServiceUuid(g_uuid);
    advData.AddManufacturerData(g_manufacturerId, g_manufacturerData);
    advData.AddServiceData(g_serviceDataUuid, g_serviceData);
    advData.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData, scanData, duration, bleAdvertiseCallbackTest_);

    HILOGI("advData function test");
    EXPECT_TRUE(BleTest::bleInstance_->HaveUuid(advData));
    EXPECT_TRUE(BleTest::bleInstance_->ManufacturerData(advData));
    EXPECT_TRUE(BleTest::bleInstance_->ServiceData(advData));
    EXPECT_EQ(BLE_ADV_FLAG_GEN_DISC, advData.GetAdvFlag());

    std::this_thread::sleep_for(std::chrono::seconds(2));
    bleAdvertise->DisableAdvertising(g_startAdvHandle, bleAdvertiseCallbackTest_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    bleAdvertise->EnableAdvertising(g_startAdvHandle, 0, bleAdvertiseCallbackTest_);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    vector<uint8_t> advData1 = {1, 2, 3};
    vector<uint8_t> scanResponse1 = {3, 2, 1};
    bleAdvertise->SetAdvertisingData(advData1, scanResponse1, bleAdvertiseCallbackTest_);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    bleAdvertise->StopAdvertising(bleAdvertiseCallbackTest_);
    bleAdvertise->Close(bleAdvertiseCallbackTest_);
    HILOGI("Ble_ModuleTest_StartAdvertising_00600 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00700
 * @tc.name:
 * @tc.desc: test wrong advHandle or wrong callback case
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00700, TestSize.Level1)
{
    HILOGI("Ble_ModuleTest_StartAdvertising_00700 start");
    BleTest::bleInstance_->InitAdvertiseSettings();
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    BleAdvertiserData advData;
    BleAdvertiserData scanData;
    uint16_t duration = 0;
    advData.AddServiceUuid(g_uuid);
    advData.AddManufacturerData(g_manufacturerId, g_manufacturerData);
    advData.AddServiceData(g_serviceDataUuid, g_serviceData);
    advData.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData, scanData, duration, bleAdvertiseCallbackTest_);

    HILOGI("advData function test");
    EXPECT_TRUE(BleTest::bleInstance_->HaveUuid(advData));
    EXPECT_TRUE(BleTest::bleInstance_->ManufacturerData(advData));
    EXPECT_TRUE(BleTest::bleInstance_->ServiceData(advData));
    EXPECT_EQ(BLE_ADV_FLAG_GEN_DISC, advData.GetAdvFlag());

    int invalidAdvHandle = 0xFF;
    bleAdvertise->DisableAdvertising(invalidAdvHandle, bleAdvertiseCallbackTest_);
    bleAdvertise->EnableAdvertising(invalidAdvHandle, 0, bleAdvertiseCallbackTest_);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::shared_ptr<BleAdvertiseCallbackTest> invalidCallback =
        std::make_shared<BleAdvertiseCallbackTest>();
    bleAdvertise->DisableAdvertising(g_startAdvHandle, invalidCallback);
    bleAdvertise->EnableAdvertising(g_startAdvHandle, 0, invalidCallback);

    bleAdvertise->StopAdvertising(bleAdvertiseCallbackTest_);
    bleAdvertise->Close(bleAdvertiseCallbackTest_);
    HILOGI("Ble_ModuleTest_StartAdvertising_00700 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00800
 * @tc.name:
 * @tc.desc: test nullptr callback case
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00800, TestSize.Level1)
{
    HILOGI("Ble_ModuleTest_StartAdvertising_00800 start");
    BleTest::bleInstance_->InitAdvertiseSettings();
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    BleAdvertiserData advData;
    BleAdvertiserData scanData;
    uint16_t duration = 0;
    advData.AddServiceUuid(g_uuid);
    advData.AddManufacturerData(g_manufacturerId, g_manufacturerData);
    advData.AddServiceData(g_serviceDataUuid, g_serviceData);
    advData.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    std::shared_ptr<BleAdvertiseCallbackTest> invalidCallback = {nullptr};
    EXPECT_EQ(bleAdvertise->StartAdvertising(BleTest::bleInstance_->bleAdvertiserSettings_,
        advData, scanData, duration, invalidCallback), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(bleAdvertise->DisableAdvertising(g_startAdvHandle, invalidCallback), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(bleAdvertise->EnableAdvertising(g_startAdvHandle, 0, invalidCallback), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(bleAdvertise->StopAdvertising(invalidCallback), BT_ERR_INTERNAL_ERROR);

    vector<uint8_t> advData1 = {1, 2, 3};
    vector<uint8_t> scanData1 = {3, 2, 1};
    EXPECT_EQ(bleAdvertise->StartAdvertising(BleTest::bleInstance_->bleAdvertiserSettings_,
        advData1, scanData1, duration, invalidCallback), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(bleAdvertise->StopAdvertising(invalidCallback), BT_ERR_INTERNAL_ERROR);
    bleAdvertise->Close(invalidCallback);
    HILOGI("Ble_ModuleTest_StartAdvertising_00800 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00900
 * @tc.name:
 * @tc.desc: test nullptr callback case
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00900, TestSize.Level1)
{
    HILOGI("Ble_ModuleTest_StartAdvertising_00900 start");
    BleTest::bleInstance_->InitAdvertiseSettings();
    bleInstance_->bleAdvertiserSettings_.SetTxPower(21); // invalid txpower
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    BleAdvertiserData advData;
    BleAdvertiserData scanData;
    vector<uint8_t> advData1 = {1, 2, 3};
    vector<uint8_t> scanData1 = {3, 2, 1};
    uint16_t duration = 0;
    advData.AddServiceUuid(g_uuid);
    advData.AddManufacturerData(g_manufacturerId, g_manufacturerData);
    advData.AddServiceData(g_serviceDataUuid, g_serviceData);
    advData.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);

    int32_t ret = bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData, scanData, duration, bleAdvertiseCallbackTest_);
    EXPECT_NE(0, ret);
    ret = bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData1, scanData1, duration, bleAdvertiseCallbackTest_);
    EXPECT_NE(0, ret);
    bleInstance_->bleAdvertiserSettings_.SetTxPower(-128); // invalid txpower
    ret = bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData, scanData, duration, bleAdvertiseCallbackTest_);
    EXPECT_NE(0, ret);
    ret = bleAdvertise->StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData1, scanData1, duration, bleAdvertiseCallbackTest_);
    EXPECT_NE(0, ret);

    HILOGI("Ble_ModuleTest_StartAdvertising_00900 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartCentralManager_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartCentralManager_00100, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    HILOGI("BLE_ModuleTest_StartCentralManager_00100 start");

    std::vector<BleScanFilter> filters;
    BleScanFilter filter;
    std::string deviceId = "42:0B:AC:A0:A0:ED";
    uint16_t manufacturerId = 0x4147;
    std::vector<uint8_t> serviceData = {0x1f, 0x77};
    std::vector<uint8_t> serviceDataMask = {0xff, 0xff};
    std::vector<uint8_t> manufactureData = {0x01, 0x09, 0x20, 0x02};
    std::vector<uint8_t> manufactureDataMask = {0x01, 0x09, 0x20, 0x02};
    filter.SetDeviceId(deviceId);
    filter.SetServiceUuid(g_serviceUuid);
    filter.SetServiceUuidMask(g_serviceUuidMask);
    filter.SetServiceSolicitationUuid(g_serviceSolicitationUuid);
    filter.SetServiceSolicitationUuidMask(g_serviceSolicitationUuidMask);
    filter.SetManufacturerId(manufacturerId);
    filter.SetServiceData(serviceData);
    filter.SetServiceDataMask(serviceDataMask);
    filter.SetManufactureData(manufactureData);
    filter.SetManufactureDataMask(manufactureDataMask);
    filters.push_back(filter);
    BleTest::bleInstance_->InitScanSettings();
    EXPECT_EQ(bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, filters), BT_NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(bleCentralManager->StopScan(), BT_NO_ERROR);

    HILOGI("BLE_ModuleTest_StartCentralManager_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartCentralManager_00200
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartCentralManager_00200, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_StartCentralManager_00200 start");
    BleTest::bleInstance_->InitScanSettings();
    EXPECT_EQ(defaultInt, BleTest::bleInstance_->bleScanSettings_.GetReportDelayMillisValue());
    EXPECT_EQ(SCAN_MODE::SCAN_MODE_LOW_POWER, BleTest::bleInstance_->bleScanSettings_.GetScanMode());
    EXPECT_TRUE(BleTest::bleInstance_->bleScanSettings_.GetLegacy());
    EXPECT_EQ(PHY_TYPE::PHY_LE_1M, BleTest::bleInstance_->bleScanSettings_.GetPhy());
    EXPECT_EQ(1, BleTest::bleInstance_->bleScanSettings_.GetReportMode());
    EXPECT_EQ(BLE_SCAN_CALLBACK_TYPE_ALL_MATCH, BleTest::bleInstance_->bleScanSettings_.GetCallbackType());
    EXPECT_EQ(MATCH_TRACK_ADV_TYPE::MAX_MATCH_TRACK_ADV,
        BleTest::bleInstance_->bleScanSettings_.GetMatchTrackAdvType());
    HILOGI("BLE_ModuleTest_StartCentralManager_00200 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartCentralManager_00400
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartCentralManager_00400, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_StartCentralManager_00400 start");
    BleTest::bleInstance_->InitScanSettings();
    std::vector<BleScanFilter> filters;
    BleTest::bleInstance_->bleScanSettings_.SetCallbackType(3); // 3 means invalid type
    int32_t ret = bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, filters);
    EXPECT_NE(0, ret);

    BleTest::bleInstance_->bleScanSettings_.SetCallbackType(1);
    BleTest::bleInstance_->bleScanSettings_.SetMatchTrackAdvType(5); // 5 means invalid type
    ret = bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, filters);
    EXPECT_NE(0, ret);


    BleTest::bleInstance_->bleScanSettings_.SetCallbackType(6); // 6 means found/lost match type
    BleTest::bleInstance_->bleScanSettings_.SetMatchTrackAdvType(1);
    ret = bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, filters);
    EXPECT_NE(0, ret);

    BleScanFilter emptyFilter;
    filters.push_back(emptyFilter);
    ret = bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, filters);
    EXPECT_NE(0, ret);
    HILOGI("BLE_ModuleTest_StartCentralManager_00400 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartCentralManager_00500
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartCentralManager_00500, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    HILOGI("BLE_ModuleTest_StartCentralManager_00500 start");
    BleTest::bleInstance_->InitAdvFilterData();
    BleTest::bleInstance_->InitScanSettings();

    bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, BleTest::bleInstance_->paramSet.scanFilters);
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetLpDeviceAdvParam(10, 10, 100, 200, 1));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetScanReportChannelToLpDevice(1));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->EnableSyncDataToLpDevice());
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->DisableSyncDataToLpDevice());
    bleCentralManager->ChangeScanParams(BleTest::bleInstance_->bleScanSettings_,
        BleTest::bleInstance_->paramSet.scanFilters, 0);

    std::vector<uint8_t> value = {0xB5, 0x45, 0x50, 0x60, 0x9F, 0xA4, 0xD4, 0x43, 0xFD, 0x38, 0xD5, 0xBF, 0xDB, 0x4D,
        0x02, 0x93, 0x01, 0x00, 0x00, 0x00};
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SendParamsToLpDevice(value, 11));
    bleCentralManager->IsLpDeviceAvailable();

    BleTest::bleInstance_->paramSet.fieldValidFlagBit = 0;
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetLpDeviceParam(BleTest::bleInstance_->paramSet));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->RemoveLpDeviceParam(g_lpDeviceUuid));

    HILOGI("BLE_ModuleTest_StartCentralManager_00500 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartCentralManager_00600
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartCentralManager_00600, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    HILOGI("BLE_ModuleTest_StartCentralManager_00600 start");
    BleTest::bleInstance_->InitAdvFilterData();
    BleTest::bleInstance_->InitScanSettings();

    bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, BleTest::bleInstance_->paramSet.scanFilters);
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetLpDeviceAdvParam(10, 10, 100, 200, 1));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetScanReportChannelToLpDevice(1));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->EnableSyncDataToLpDevice());
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->DisableSyncDataToLpDevice());

    std::vector<uint8_t> value = {0xB5, 0x45, 0x50, 0x60, 0x9F, 0xA4, 0xD4, 0x43, 0xFD, 0x38, 0xD5, 0xBF, 0xDB, 0x4D,
        0x02, 0x93, 0x01, 0x00, 0x00, 0x00};
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SendParamsToLpDevice(value, 11));
    bleCentralManager->IsLpDeviceAvailable();

    BleTest::bleInstance_->paramSet.fieldValidFlagBit = 0x1F;
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetLpDeviceParam(BleTest::bleInstance_->paramSet));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->RemoveLpDeviceParam(g_lpDeviceUuid));

    HILOGI("BLE_ModuleTest_StartCentralManager_00600 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartCentralManager_00700
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartCentralManager_00700, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    HILOGI("BLE_ModuleTest_StartCentralManager_00700 start");
    BleTest::bleInstance_->InitAdvFilterData();
    BleTest::bleInstance_->InitScanSettings();

    bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, BleTest::bleInstance_->paramSet.scanFilters);
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetLpDeviceAdvParam(10, 10, 100, 200, 1));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetScanReportChannelToLpDevice(1));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->EnableSyncDataToLpDevice());
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->DisableSyncDataToLpDevice());

    std::vector<uint8_t> value = {0xB5, 0x45, 0x50, 0x60, 0x9F, 0xA4, 0xD4, 0x43, 0xFD, 0x38, 0xD5, 0xBF, 0xDB, 0x4D,
        0x02, 0x93, 0x01, 0x00, 0x00, 0x00};
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SendParamsToLpDevice(value, 11));
    bleCentralManager->IsLpDeviceAvailable();

    BleTest::bleInstance_->paramSet.fieldValidFlagBit = 0x3F;
    BleTest::bleInstance_->paramSet.advHandle = -100;
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->SetLpDeviceParam(BleTest::bleInstance_->paramSet));
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->RemoveLpDeviceParam(g_lpDeviceUuid));

    HILOGI("BLE_ModuleTest_StartCentralManager_00700 end");
}

/**
 * @tc.number: BLE_ModuleTest_StartCentralManager_00800
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartCentralManager_00800, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    HILOGI("BLE_ModuleTest_StartCentralManager_00800 start");
    BleTest::bleInstance_->InitAdvFilterData();
    BleTest::bleInstance_->InitScanSettings();

    BleTest::bleInstance_->bleScanSettings_.SetReportMode(2); // batch scan mode
    bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_, BleTest::bleInstance_->paramSet.scanFilters);
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager->FlushBatchScanResults());
    HILOGI("BLE_ModuleTest_StartCentralManager_00800 end");
}

/**
 * @tc.name: BLE_ModuleTest_SetNewApiFlag_00100
 * @tc.desc: BLE_ModuleTest_SetNewApiFlag_00100
 * @tc.type: FUNC
 */
HWTEST_F(BleTest, BLE_ModuleTest_SetNewApiFlag_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetNewApiFlag_00100 start");
    bleCentralManager->SetNewApiFlag();
    bleCentralManager->CheckValidScannerId();
    EXPECT_TRUE(bleCentralManager->isNewApi_);
    HILOGI("BLE_ModuleTest_SetNewApiFlag_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_GetServiceUuids_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_GetServiceUuids_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_GetServiceUuids_00100 start");
    vector<UUID> serviceUuids_;
    BleTest::bleInstance_->InitScanResult();
    serviceUuids_ = BleTest::bleInstance_->bleScanResult_.GetServiceUuids();
    EXPECT_TRUE(serviceUuids_.empty());
    HILOGI("BLE_BleScanResultTest_GetServiceUuids_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_GetManufacturerData_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_GetManufacturerData_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_GetManufacturerData_00100 start");
    std::map<uint16_t, std::string> manufacturerSpecificData_;
    BleTest::bleInstance_->InitScanResult();
    manufacturerSpecificData_ = BleTest::bleInstance_->bleScanResult_.GetManufacturerData();
    EXPECT_TRUE(manufacturerSpecificData_.empty());
    HILOGI("BLE_BleScanResultTest_GetManufacturerData_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_GetServiceData_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_GetServiceData_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_GetServiceData_00100 start");
    std::map<UUID, std::string> serviceData_;
    BleTest::bleInstance_->InitScanResult();
    serviceData_ = BleTest::bleInstance_->bleScanResult_.GetServiceData();
    EXPECT_TRUE(serviceData_.empty());
    HILOGI("BLE_BleScanResultTest_GetServiceData_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_GetPeripheralDevice_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_GetPeripheralDevice_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_GetPeripheralDevice_00100 start");
    BleTest::bleInstance_->InitScanResult();
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BLE);
    BleTest::bleInstance_->bleScanResult_.SetPeripheralDevice(device);
    BluetoothRemoteDevice peripheralDevice_ = BleTest::bleInstance_->bleScanResult_.GetPeripheralDevice();
    EXPECT_EQ(peripheralDevice_.GetDeviceAddr(), "11:22:33:44:55:66");
    HILOGI("BLE_BleScanResultTest_GetPeripheralDevice_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_GetRssi_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_GetRssi_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_GetRssi_00100 start");
    int8_t rssi;
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.SetRssi(0);
    rssi = BleTest::bleInstance_->bleScanResult_.GetRssi();
    EXPECT_EQ(rssi,  0);
    HILOGI("BLE_BleScanResultTest_GetRssi_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_GetAdvertiseFlag_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_GetAdvertiseFlag_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_GetAdvertiseFlag_00100 start");
    int8_t advertiserFlag;
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.SetAdvertiseFlag(1);
    advertiserFlag = BleTest::bleInstance_->bleScanResult_.GetAdvertiseFlag();
    EXPECT_EQ(advertiserFlag, 1);
    HILOGI("BLE_BleScanResultTest_GetAdvertiseFlag_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_GetPayload_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_GetPayload_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_GetPayload_00100 start");
    std::vector<uint8_t> respData;
    BleTest::bleInstance_->InitScanResult();
    respData = BleTest::bleInstance_->bleScanResult_.GetPayload();
    EXPECT_FALSE(respData.empty());
    HILOGI("BLE_BleScanResultTest_GetPayload_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_AddManufacturerData_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_AddManufacturerData_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_AddManufacturerData_00100 start");
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.AddManufacturerData(0, "bluetoothdevice");
    std::map<uint16_t, std::string> manufacturerSpecificData_ =
        BleTest::bleInstance_->bleScanResult_.GetManufacturerData();
    EXPECT_FALSE(manufacturerSpecificData_.empty());
    HILOGI("BLE_BleScanResultTest_AddManufacturerData_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_AddServiceData_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_AddServiceData_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_AddServiceData_00100 start");
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.AddServiceData(g_serviceDataUuid, "bluetoothdevice");
    EXPECT_FALSE(BleTest::bleInstance_->bleScanResult_.GetServiceData().empty());
    HILOGI("BLE_BleScanResultTest_AddServiceData_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_AddAdvertisingData_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_AddAdvertisingData_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_AddAdvertisingData_00100 start");
    BleTest::bleInstance_->InitScanResult();
    uint8_t advType1 = 0xFF;
    std::string advData1 = "testManufactureData";
    BleTest::bleInstance_->bleScanResult_.AddAdvertisingData(advType1, advData1);
    auto it1 = BleTest::bleInstance_->bleScanResult_.GetAdvertisingData().find(advType1);
    if (it1 != BleTest::bleInstance_->bleScanResult_.GetAdvertisingData().end()) {
        EXPECT_EQ(it1->second, advData1);
    }
    HILOGI("BLE_BleScanResultTest_AddAdvertisingData_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_SetTxPowerLevel_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_SetTxPowerLevel_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_SetTxPowerLevel_00100 start");
    BleTest::bleInstance_->InitScanResult();
    int8_t txPower = -1;
    BleTest::bleInstance_->bleScanResult_.SetTxPowerLevel(txPower);
    EXPECT_EQ(BleTest::bleInstance_->bleScanResult_.GetTxPowerLevel(), txPower);
    HILOGI("BLE_BleScanResultTest_SetTxPowerLevel_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_AddServiceUuid_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_AddServiceUuid_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_AddServiceUuid_00100 start");
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.AddServiceUuid(g_serviceUuid);
    EXPECT_FALSE(BleTest::bleInstance_->bleScanResult_.GetServiceUuids().empty());
    HILOGI("BLE_BleScanResultTest_AddServiceUuid_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_SetPayload_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_SetPayload_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_SetPayload_00100 start");
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.SetPayload("payload");
    EXPECT_FALSE(BleTest::bleInstance_->bleScanResult_.GetPayload().empty());
    HILOGI("BLE_BleScanResultTest_SetPayload_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_SetConnectable_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_SetConnectable_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_SetConnectable_00100 start");
    bool connectable = true;
    const BluetoothRemoteDevice device;
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.SetConnectable(connectable);
    EXPECT_TRUE(BleTest::bleInstance_->bleScanResult_.IsConnectable());
    HILOGI("BLE_BleScanResultTest_SetConnectable_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_GetName_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_GetName_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_GetName_00100 start");
    std::string name;
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.SetName("bluetoothdevice");
    name = BleTest::bleInstance_->bleScanResult_.GetName();
    EXPECT_EQ(name, "bluetoothdevice");
    HILOGI("BLE_BleScanResultTest_GetName_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_SetTimestamp_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_SetTimestamp_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_SetTimestamp_00100 start");
    BleTest::bleInstance_->InitScanResult();
    BleTest::bleInstance_->bleScanResult_.SetTimestamp(123);
    uint16_t timeStamp = BleTest::bleInstance_->bleScanResult_.GetTimestamp();
    EXPECT_EQ(timeStamp, 123);
    HILOGI("BLE_BleScanResultTest_SetTimestamp_00100 end");
}

/**
 * @tc.number: BLE_BleScanResultTest_BleAdvertiserData_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_BleScanResultTest_BleAdvertiserData_00100, TestSize.Level1)
{
    HILOGI("BLE_BleScanResultTest_BleAdvertiserData_00100 start");
    BleAdvertiserData scanData;
    scanData.AddServiceUuid(g_uuid);
    scanData.AddManufacturerData(g_manufacturerId, "");
    scanData.AddServiceData(g_serviceDataUuid, "");
    scanData.SetAdvFlag(BLE_ADV_FLAG_GEN_DISC);
    scanData.SetIncludeDeviceName(true);
    EXPECT_TRUE(scanData.GetIncludeDeviceName());
    HILOGI("BLE_BleScanResultTest_BleAdvertiserData_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_SetAdvertisingData_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_SetAdvertisingData_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetAdvertisingData_00100 start");
    vector<uint8_t> advData = {1, 2, 3};
    vector<uint8_t> scanResponse = {3, 2, 1};
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    std::shared_ptr<BleAdvertiseCallbackTest> bleAdvertiseCallbackTest =
        std::make_shared<BleAdvertiseCallbackTest>();
    bleAdvertise->SetAdvertisingData(advData, scanResponse, bleAdvertiseCallbackTest);
    EXPECT_EQ(bleAdvertise->GetAdvHandle(bleAdvertiseCallbackTest), BLE_INVALID_ADVERTISING_HANDLE);
    HILOGI("BLE_ModuleTest_SetAdvertisingData_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_SetAdvertisingData_00200
 * @tc.name:
 * @tc.desc: set nullptr callback
 */
HWTEST_F(BleTest, BLE_ModuleTest_SetAdvertisingData_00200, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetAdvertisingData_00100 start");
    vector<uint8_t> advData = {1, 2, 3};
    vector<uint8_t> scanResponse = {3, 2, 1};
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    std::shared_ptr<BleAdvertiseCallbackTest> bleAdvertiseCallbackTest = {nullptr};
    bleAdvertise->SetAdvertisingData(advData, scanResponse, bleAdvertiseCallbackTest);
    EXPECT_EQ(bleAdvertise->GetAdvHandle(bleAdvertiseCallbackTest), BLE_INVALID_ADVERTISING_HANDLE);
    HILOGI("BLE_ModuleTest_SetAdvertisingData_00200 end");
}

/**
 * @tc.number: BLE_ModuleTest_SetScanReportChannelToSensorHub_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_SetScanReportChannelToSensorHub_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetScanReportChannelToSensorHub_00100 start");
    BleCentralManager bleCentralManager(bleCentralManagerCallbackTest_);
    EXPECT_EQ(BT_NO_ERROR, bleCentralManager.SetScanReportChannelToLpDevice(1));
    HILOGI("BLE_ModuleTest_SetScanReportChannelToSensorHub_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_OnScanCallbackInner_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_OnScanCallbackInner_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_OnScanCallbackInner_00100 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothBleCentralManagerCallBackStub::OnScanCallbackInner(
        bluetoothBleCentralManagerCallbackTest.get(), data, reply), 1);
    HILOGI("BLE_ModuleTest_OnScanCallbackInner_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_OnBleBatchScanResultsEventInner_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_OnBleBatchScanResultsEventInner_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_OnBleBatchScanResultsEventInner_00100 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothBleCentralManagerCallBackStub::OnBleBatchScanResultsEventInner(
        bluetoothBleCentralManagerCallbackTest.get(), data, reply), 10);
    uint32_t code = 17;
    MessageOption option;
    bluetoothBleCentralManagerCallbackTest->OnRemoteRequest(code, data, reply, option);
    HILOGI("BLE_ModuleTest_OnBleBatchScanResultsEventInner_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_ResetAllProxy_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_ResetAllProxy_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_ResetAllProxy_00100 start");
    sptr<IRemoteObject> object =
        BluetoothProfileManager::GetInstance().GetProfileRemote(BLE_CENTRAL_MANAGER_SERVER);
    BluetoothBleCentralManagerProxy proxy_(object);
    EXPECT_TRUE(proxy_.ResetAllProxy());
    HILOGI("BLE_ModuleTest_ResetAllProxy_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_FreezeByRss_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_FreezeByRss_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_FreezeByRss_00100 start");
    sptr<IRemoteObject> object =
        BluetoothProfileManager::GetInstance().GetProfileRemote(BLE_CENTRAL_MANAGER_SERVER);
    BluetoothBleCentralManagerProxy proxy_(object);
    std::set<int> pidSet {1};
    EXPECT_TRUE(proxy_.FreezeByRss(pidSet, false, 0));
    EXPECT_TRUE(proxy_.FreezeByRss(pidSet, false, 1));
    HILOGI("BLE_ModuleTest_FreezeByRss_00100 end");
}

/**
 * @tc.number: BLE_ModuleTest_FreezeByRss_00200
 * @tc.name:
 * @tc.desc: The app stopped scan during the RSS freeze, the scan should not be resumed when unfreezing.
 */
HWTEST_F(BleTest, BLE_ModuleTest_FreezeByRss_00200, TestSize.Level1)
{
    bleCentralManager->StopScan();
    std::this_thread::sleep_for(std::chrono::seconds(TIME));
    HILOGI("BLE_ModuleTest_FreezeByRss_00200 start");
    sptr<IRemoteObject> object =
        BluetoothProfileManager::GetInstance().GetProfileRemote(BLE_CENTRAL_MANAGER_SERVER);
    BluetoothBleCentralManagerProxy proxy_(object);
    std::set<int> pidSet {static_cast<int>(getpid())};
    BleTest::bleInstance_->InitAdvFilterData();

    /* start scan, freeze the app, stop scan during the freeze, then unfreeze.
       The scan should not be resumed for the app which has stopped scanning. */
    EXPECT_EQ(bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_,
        BleTest::bleInstance_->paramSet.scanFilters), BT_NO_ERROR);
    EXPECT_TRUE(proxy_.FreezeByRss(pidSet, true, 0));
    EXPECT_EQ(bleCentralManager->StopScan(), BT_NO_ERROR);
    EXPECT_TRUE(proxy_.FreezeByRss(pidSet, false, 0));
    HILOGI("BLE_ModuleTest_FreezeByRss_00200 end");
}

/**
 * @tc.number: BLE_ModuleTest_FreezeByRss_00300
 * @tc.name:
 * @tc.desc: The app is still scanning when unfreezing, the scan should be resumed normally.
 */
HWTEST_F(BleTest, BLE_ModuleTest_FreezeByRss_00300, TestSize.Level1)
{
    bleCentralManager->StopScan();
    std::this_thread::sleep_for(std::chrono::seconds(TIME));
    HILOGI("BLE_ModuleTest_FreezeByRss_00300 start");
    sptr<IRemoteObject> object =
        BluetoothProfileManager::GetInstance().GetProfileRemote(BLE_CENTRAL_MANAGER_SERVER);
    BluetoothBleCentralManagerProxy proxy_(object);
    std::set<int> pidSet {static_cast<int>(getpid())};
    BleTest::bleInstance_->InitAdvFilterData();

    /* start scan, freeze and unfreeze the app without stopping scan.
       The scan should be resumed normally and then stopped by the app. */
    EXPECT_EQ(bleCentralManager->StartScan(BleTest::bleInstance_->bleScanSettings_,
        BleTest::bleInstance_->paramSet.scanFilters), BT_NO_ERROR);
    EXPECT_TRUE(proxy_.FreezeByRss(pidSet, true, 0));
    EXPECT_TRUE(proxy_.FreezeByRss(pidSet, false, 0));
    EXPECT_EQ(bleCentralManager->StopScan(), BT_NO_ERROR);
    HILOGI("BLE_ModuleTest_FreezeByRss_00300 end");
}

/**
 * @tc.number: BLE_ModuleTest_SetLpDeviceParam_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_SetLpDeviceParam_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetLpDeviceParam_00100 start");
    sptr<IRemoteObject> object =
        BluetoothProfileManager::GetInstance().GetProfileRemote(BLE_CENTRAL_MANAGER_SERVER);
    BluetoothBleCentralManagerProxy proxy_(object);
    BluetoothLpDeviceParamSet paramSet;
    BluetoothActiveDeviceInfo bluetoothActiveDeviceInfo;
    paramSet.activeDeviceInfos.push_back(bluetoothActiveDeviceInfo);
    EXPECT_EQ(proxy_.SetLpDeviceParam(paramSet), BT_NO_ERROR);
    sptr<IRemoteObject> advertiserobject =
        BluetoothProfileManager::GetInstance().GetProfileRemote(BLE_ADVERTISER_SERVER);
    BluetoothBleAdvertiserProxy proxyAdvertiser_(advertiserobject);
    proxyAdvertiser_.Close(1);
    HILOGI("BLE_ModuleTest_SetLpDeviceParam_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
