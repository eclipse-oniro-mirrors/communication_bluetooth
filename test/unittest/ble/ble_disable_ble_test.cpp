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

#include "bluetooth_log.h"
#include "bluetooth_access_token_mock.h"
#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "bt_uuid.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace Bluetooth {

static BluetoothHost *host_;
constexpr int TIME = 2;

class BleAdvertiseCallbackTest : public Bluetooth::BleAdvertiseCallback {
public:
    BleAdvertiseCallbackTest(){};
    ~BleAdvertiseCallbackTest(){};

private:
    void OnStartResultEvent(int result, int advHandle) override {}
    void OnEnableResultEvent(int result, int advHandle) override {}
    void OnDisableResultEvent(int result, int advHandle) override {}
    void OnStopResultEvent(int result, int advHandle) override {}
    void OnSetAdvDataEvent(int result) override {}
    void OnGetAdvHandleEvent(int result, int advHandle) override {}
};

class BleCentralManagerCallbackTest : public Bluetooth::BleCentralManagerCallback {
public:
    BleCentralManagerCallbackTest() {};
    ~BleCentralManagerCallbackTest() override {};

private:
    void OnScanCallback(const Bluetooth::BleScanResult &result) override
    {}
    void OnFoundOrLostCallback(const Bluetooth::BleScanResult &result, uint8_t callbackType) override
    {}
    void OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult> &results) override
    {}
    void OnStartOrStopScanEvent(int resultCode, bool isStartScan) override
    {}
};
Bluetooth::UUID g_lpDeviceUuid = Bluetooth::UUID::FromString("43D4A49F-6050-45B5-9302-4DDBBFD538FD");
class BleDisableBleTest : public testing::Test {
public:
    Bluetooth::BleAdvertiserSettings bleAdvertiserSettings_ {};
    std::shared_ptr<BleCentralManagerCallbackTest> bleCentralManagerCallbackTest_ {};
    std::shared_ptr<BleAdvertiseCallbackTest> bleAdvertiseCallbackTest_ {};
    Bluetooth::BleScanSettings bleScanSettings_ {};
    Bluetooth::BleScanResult bleScanResult_ {};

    BleAdvertiserData advData1;
    BleAdvertiserData advData2;
    BleAdvertiserData scanData1;
    BleAdvertiserData scanData2;

    BleLpDeviceParamSet paramSet;
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        BluetoothAccessTokenMock::SetNativeTokenInfo();
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (isBleEnable) {
            host_->DisableBle();
            sleep(TIME);
        }

        if (isBrEnable) {
            host_->DisableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {
    }

    void TearDown()
    {
    }

public:
};
/**
 * @tc.name: BLE_BleDisableBleTest_00100
 * @tc.desc: Verify Descriptor function.
 * @tc.type: FUNC
 */
HWTEST_F(BleDisableBleTest, BLE_BleDisableBleTest_00100, TestSize.Level1)
{
    HILOGI("BLE_BleDisableBleTest_00100 start");
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    vector<uint8_t> advData1 = {1, 2, 3};
    vector<uint8_t> scanData1 = {3, 2, 1};
    uint16_t duration = 0;
    EXPECT_NE(bleAdvertise->StartAdvertising(bleAdvertiserSettings_,
        advData1, scanData1, duration, bleAdvertiseCallbackTest_), BT_NO_ERROR);
    HILOGI("BLE_BleDisableBleTest_00100 end");
}

/*
 * @tc.number: BLE_BleDisableBleTest_00200
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(BleDisableBleTest, BLE_BleDisableBleTest_00200, TestSize.Level1)
{
    HILOGI("BLE_BleDisableBleTest_00200 start");
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    uint16_t duration = 0;
    EXPECT_NE(bleAdvertise->StartAdvertising(bleAdvertiserSettings_,
        advData1, scanData1, duration, bleAdvertiseCallbackTest_), BT_NO_ERROR);
    HILOGI("BLE_BleDisableBleTest_00200 end");
}

/*
 * @tc.number: BLE_BleDisableBleTest_00300
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(BleDisableBleTest, BLE_BleDisableBleTest_00300, TestSize.Level1)
{
    HILOGI("BLE_BleDisableBleTest_00300 start");
    vector<uint8_t> advData = {1, 2, 3};
    vector<uint8_t> scanResponse = {3, 2, 1};
    auto bleAdvertise = BleAdvertiser::CreateInstance();
    std::shared_ptr<BleAdvertiseCallbackTest> bleAdvertiseCallbackTest = nullptr;
    bleAdvertise->SetAdvertisingData(advData, scanResponse, bleAdvertiseCallbackTest);
    EXPECT_NE(bleAdvertise->StopAdvertising(bleAdvertiseCallbackTest_), BT_NO_ERROR);
    bleAdvertise->Close(bleAdvertiseCallbackTest_);
    HILOGI("BLE_BleDisableBleTest_00300 end");
}


/*
 * @tc.number: BLE_BleDisableBleTest_00600
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(BleDisableBleTest, BLE_BleDisableBleTest_00600, TestSize.Level1)
{
    HILOGI("BLE_BleDisableBleTest_00600 start");
    BleCentralManager bleCentralManager(bleCentralManagerCallbackTest_);
    std::vector<BleScanFilter> filters;
    int32_t ret = bleCentralManager.StartScan(bleScanSettings_, filters);
    EXPECT_NE(0, ret);
    bleCentralManager.StopScan();
    HILOGI("BLE_BleDisableBleTest_00600 end");
}

/*
 * @tc.number: BLE_BleDisableBleTest_00700
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(BleDisableBleTest, BLE_BleDisableBleTest_00700, TestSize.Level1)
{
    HILOGI("BLE_BleDisableBleTest_00700 start");
    BleCentralManager bleCentralManager(bleCentralManagerCallbackTest_);
    bleCentralManager.SetLpDeviceAdvParam(10, 10, 100, 200, 1);
    bleCentralManager.SetScanReportChannelToLpDevice(1);
    bleCentralManager.EnableSyncDataToLpDevice();
    bleCentralManager.DisableSyncDataToLpDevice();
    std::vector<uint8_t> value = {0xB5, 0x45, 0x50, 0x60, 0x9F, 0xA4, 0xD4, 0x43, 0xFD, 0x38, 0xD5, 0xBF, 0xDB, 0x4D,
        0x02, 0x93, 0x01, 0x00, 0x00, 0x00};
    bleCentralManager.SendParamsToLpDevice(value, 11);
    bleCentralManager.IsLpDeviceAvailable();
    bleCentralManager.SetLpDeviceParam(paramSet);
    bleCentralManager.RemoveLpDeviceParam(g_lpDeviceUuid);
    bleCentralManager.SetNewApiFlag();
    EXPECT_TRUE(bleCentralManager.isNewApi_);
    HILOGI("BLE_BleDisableBleTest_00700 end");
}

/*
 * @tc.number: BLE_BleDisableBleTest_00800
 * @tc.name: Verify Descriptor function.
*/
HWTEST_F(BleDisableBleTest, BLE_BleDisableBleTest_00800, TestSize.Level1)
{
    HILOGI("BLE_BleDisableBleTest_00700 start");
    bleScanResult_.SetEventType(0);
    EXPECT_EQ(bleScanResult_.GetEventType(), 0);
    HILOGI("BLE_BleDisableBleTest_00800 end");
}
}  // namespace bluetooth
}  // namespace OHOS