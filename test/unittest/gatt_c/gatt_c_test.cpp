/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <thread>
#include <securec.h>

#include "bluetooth_access_token_mock.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "ohos_bt_def.h"
#include "ohos_bt_gap.h"
#include "ohos_bt_gatt.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
BtGattCallbacks g_btGattCallbacks;
class GattCTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int scannerId;
    BleScanCallbacks callbacks;
};

void GattCTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
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

void GattCTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    host_ = nullptr;
}

void GattCTest::SetUp()
{
    callbacks.scanResultCb = nullptr;
    callbacks.scanStateChangeCb = nullptr;
    callbacks.lpDeviceInfoCb = nullptr;
    BleRegisterScanCallbacks(&callbacks, &scannerId);
}

void GattCTest::TearDown()
{
    BleDeregisterScanCallbacks(scannerId);
}

/*
 * @tc.name: GattCTest_001
 * @tc.desc: Test BleStartScanEx when g_BleCentralManager or BleScanConfigs is nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5OH5C
*/
HWTEST_F(GattCTest, GattCTest_001, TestSize.Level1)
{
    BleScanConfigs configs;
    BleScanNativeFilter filter[1];
    int filterSize = sizeof(filter) / sizeof(BleScanNativeFilter);
    int res = BleStartScanEx(scannerId, &configs, filter, filterSize);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);
    BleGattRegisterCallbacks(&g_btGattCallbacks);
    res = BleStartScanEx(scannerId, nullptr, filter, filterSize);
    EXPECT_EQ(res, OHOS_BT_STATUS_FAIL);
}

/*
 * @tc.name: GattCTest_002
 * @tc.desc: Test BleStartScanEx when BleScanNativeFilter is null or filterSize is zero
 * @tc.type: FUNC
 * @tc.require: issueI5OH5C
*/
HWTEST_F(GattCTest, GattCTest_002, TestSize.Level1)
{
    BleScanConfigs configs;
    configs.reportDelayMillis = 0;
    configs.scanMode = SCAN_MODE_LOW_POWER;
    configs.legacy = true;
    configs.phy = PHY_LE_1M;

    BleScanNativeFilter filter[1];
    int filterSize = sizeof(filter) / sizeof(BleScanNativeFilter);
    int res = BleStartScanEx(scannerId, &configs, nullptr, filterSize);
    EXPECT_EQ(res, BT_ERR_INTERNAL_ERROR);

    res = BleStartScanEx(scannerId, &configs, filter, 0);
    EXPECT_EQ(res, BT_ERR_INTERNAL_ERROR);
}

/*
 * @tc.name: GattCTest_003
 * @tc.desc: Test BleStartScanEx success status
 * @tc.type: FUNC
 * @tc.require: issueI5OH5C
*/
HWTEST_F(GattCTest, GattCTest_003, TestSize.Level1)
{
    BleScanConfigs configs;
    configs.reportDelayMillis = 0;
    configs.scanMode = SCAN_MODE_LOW_POWER;
    configs.legacy = true;
    configs.phy = PHY_LE_1M;

    BleScanNativeFilter filter[1];
    filter[0].address = (char *)"test";
    filter[0].deviceName = (char *)"test";
    filter[0].serviceUuid = (unsigned char*)"FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF";
    filter[0].serviceUuidMask = (unsigned char*)"FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF";
    filter[0].serviceUuidLength = sizeof("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    unsigned char test1[1];
    unsigned char test2[1];
    unsigned char test3[1];
    unsigned char test4[1];
    filter[0].serviceData = test1;
    filter[0].serviceDataMask = test2;
    filter[0].manufactureData = test3;
    filter[0].manufactureDataMask = test4;
    filter[0].serviceData[0] = 1;
    filter[0].serviceDataMask[0] = 1;
    filter[0].manufactureData[0] = 1;
    filter[0].manufactureDataMask[0] = 1;
    filter[0].serviceDataLength = 1;
    filter[0].manufactureDataLength = 1;
    filter[0].manufactureId = 1;
    int filterSize = sizeof(filter) / sizeof(BleScanNativeFilter);
    int res = BleStartScanEx(scannerId, &configs, filter, filterSize);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);

    filter[0].address = nullptr;
    filter[0].deviceName = nullptr;
    filter[0].serviceUuidMask = nullptr;
    filter[0].serviceData = nullptr;
    filter[0].manufactureData = nullptr;
    res = BleStartScanEx(scannerId, &configs, filter, filterSize);
    EXPECT_EQ(res, BT_ERR_BLE_SCAN_ALREADY_STARTED);

    filter[0].serviceData = test1;
    filter[0].manufactureData = test3;
    filter[0].serviceDataMask = nullptr;
    filter[0].manufactureDataMask = nullptr;
    filter[0].serviceUuid = nullptr;
    res = BleStartScanEx(scannerId, &configs, filter, filterSize);
    EXPECT_EQ(res, BT_ERR_BLE_SCAN_ALREADY_STARTED);
}

/*
 * @tc.name: GattCTest_004
 * @tc.desc: Test BleStartScanEx when serviceUuid's length is random
 * @tc.type: FUNC
 * @tc.require: issueI5OH5C
*/
HWTEST_F(GattCTest, GattCTest_004, TestSize.Level1)
{
    BleScanConfigs configs;
    configs.reportDelayMillis = 0;
    configs.scanMode = SCAN_MODE_LOW_POWER;
    configs.legacy = true;
    configs.phy = PHY_LE_1M;

    BleScanNativeFilter filter[1];
    filter[0].address = nullptr;
    filter[0].deviceName = nullptr;
    filter[0].serviceUuid = (unsigned char*)"";
    filter[0].serviceUuidMask = (unsigned char*)"";
    filter[0].serviceUuidLength = 0;
    filter[0].serviceData = nullptr;
    filter[0].manufactureData = nullptr;
    int filterSize = sizeof(filter) / sizeof(BleScanNativeFilter);
    int res = BleStartScanEx(scannerId, &configs, filter, filterSize);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);

    filter[0].serviceUuid = (unsigned char*)"FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF";
    filter[0].serviceUuidMask = (unsigned char*)"FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF";
    filter[0].serviceUuidLength = sizeof("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    res = BleStartScanEx(scannerId, &configs, filter, filterSize);
    EXPECT_EQ(res, BT_ERR_BLE_SCAN_ALREADY_STARTED);
}

/*
 * @tc.name: GattCTest_005
 * @tc.desc: Test BleStartScanEx with adv_ind flage success status
 * @tc.type: FUNC
 * @tc.require: issueI5OH5C
*/
HWTEST_F(GattCTest, GattCTest_005, TestSize.Level1)
{
    BleScanConfigs configs;
    configs.reportDelayMillis = 0;
    configs.scanMode = SCAN_MODE_LOW_POWER;
    configs.legacy = true;
    configs.phy = PHY_LE_1M;

    BleScanNativeFilter filter[1];
    filter[0].address = (char *)"test";
    filter[0].deviceName = (char *)"test";
    filter[0].serviceUuid = (unsigned char*)"FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF";
    filter[0].serviceUuidMask = (unsigned char*)"FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF";
    filter[0].serviceUuidLength = sizeof("FFFFFFF0-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    unsigned char test1[1];
    unsigned char test2[1];
    unsigned char test3[1];
    unsigned char test4[1];
    filter[0].serviceData = test1;
    filter[0].serviceDataMask = test2;
    filter[0].manufactureData = test3;
    filter[0].manufactureDataMask = test4;
    filter[0].serviceData[0] = 1;
    filter[0].serviceDataMask[0] = 1;
    filter[0].manufactureData[0] = 1;
    filter[0].manufactureDataMask[0] = 1;
    filter[0].serviceDataLength = 1;
    filter[0].manufactureDataLength = 1;
    filter[0].manufactureId = 1;
    filter[0].advIndReport = true;
    int filterSize = sizeof(filter) / sizeof(BleScanNativeFilter);
    int res = BleStartScanEx(scannerId, &configs, filter, filterSize);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);

    filter[0].address = nullptr;
    filter[0].deviceName = nullptr;
    filter[0].serviceUuidMask = nullptr;
    filter[0].serviceData = nullptr;
    filter[0].manufactureData = nullptr;
    filter[0].advIndReport = false;
    res = BleStartScanEx(scannerId, &configs, filter, filterSize);
    EXPECT_EQ(res, BT_ERR_BLE_SCAN_ALREADY_STARTED);
}

HWTEST_F(GattCTest, GattCTest_BleStartAdvEx_001, TestSize.Level1)
{
    HILOGI("GattCTest_BleStartAdvEx_001 start");
    uint8_t advData[] = { 0x0B, 0x09, 0x4A, 0x61, 0x63, 0x6B, 0x69, 0x73, 0x74, 0x61, 0x6E, 0x67 };
    uint8_t advDataChanged[] = { 0x0C, 0x09, 0x4A, 0x61, 0x63, 0x6B, 0x69, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x31 };

    int advId = -1;
    StartAdvRawData data = {
        .advData = advData,
        .advDataLen = sizeof(advData) / sizeof(uint8_t),
        .rspData = nullptr,
        .rspDataLen = 0,
    };
    BleAdvParams para = {
        .minInterval = 0x20,
        .advType = OHOS_BLE_ADV_IND,
    };
    int ret = BleStartAdvEx(&advId, data, para);
    EXPECT_EQ(ret, OHOS_BT_STATUS_SUCCESS);
    EXPECT_GE(advId, 0);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    BleDisableAdvEx(advId);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    BleEnableAdvEx(advId);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    data.advData = advDataChanged;
    data.advDataLen = sizeof(advDataChanged) / sizeof(uint8_t);
    ret = BleSetAdvData(advId, data);
    EXPECT_EQ(ret, OHOS_BT_STATUS_SUCCESS);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ret = BleStopAdv(advId);
    EXPECT_EQ(ret, OHOS_BT_STATUS_SUCCESS);

    HILOGI("GattCTest_BleStartAdvEx_001 end");
}

HWTEST_F(GattCTest, GattCTest_BleStartAdvEx_002, TestSize.Level1)
{
    HILOGI("GattCTest_BleStartAdvEx_002 start");
    uint8_t advData[] = { 0x0B, 0x09, 0x4A, 0x61, 0x63, 0x6B, 0x69, 0x73, 0x74, 0x61, 0x6E, 0x67 };
    uint8_t advDataChanged[] = { 0x0C, 0x09, 0x4A, 0x61, 0x63, 0x6B, 0x69, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x31 };

    int advId = -1;
    StartAdvRawData data = {
        .advData = advData,
        .advDataLen = sizeof(advData) / sizeof(uint8_t),
        .rspData = nullptr,
        .rspDataLen = 0,
    };
    BleAdvParams para = {
        .minInterval = 0x20,
        .advType = OHOS_BLE_ADV_IND,
    };
    int ret = BleStartAdvEx(&advId, data, para);
    EXPECT_EQ(ret, OHOS_BT_STATUS_SUCCESS);
    EXPECT_GE(advId, 0);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ret = BleStopAdv(advId);
    EXPECT_EQ(ret, OHOS_BT_STATUS_SUCCESS);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    ret = BleDisableAdvEx(advId);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);
    ret = BleEnableAdvEx(advId);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);

    data.advData = advDataChanged;
    data.advDataLen = sizeof(advDataChanged) / sizeof(uint8_t);
    ret = BleSetAdvData(advId, data);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);

    HILOGI("GattCTest_BleStartAdvEx_002 end");
}

/*
 * @tc.name: GattCTest_006
 * @tc.desc: GattCTest_006
 * @tc.type: FUNC
*/
HWTEST_F(GattCTest, GattCTest_006, TestSize.Level1)
{
    int advId = 0x00;
    int *advHandle = nullptr;
    int res = GetAdvHandle(advId, advHandle);
    EXPECT_NE(res, OHOS_BT_STATUS_SUCCESS);
}

/*
 * @tc.name: GattCTest_007
 * @tc.desc: GattCTest_007
 * @tc.type: FUNC
*/
HWTEST_F(GattCTest, GattCTest_007, TestSize.Level1)
{
    uint32_t dataSize = 0x00;
    int32_t type = 0x00;
    uint8_t *data = nullptr;
    int res = SendParamsToLpDevice(data, dataSize, type);
    EXPECT_NE(res, OHOS_BT_STATUS_SUCCESS);
}

/*
 * @tc.name: IsBtVlinkDeviceConnectOptimScene_001
 * @tc.desc: IsBtVlinkDeviceConnectOptimScene_001
 * @tc.type: FUNC
*/
HWTEST_F(GattCTest, IsBtVlinkDeviceConnectOptimScene_001, TestSize.Level1)
{
    HILOGI("GattCTest_IsBtVlinkDeviceConnectOptimScene_001 start");
    int transport = OHOS_BT_TRANSPORT_INVALID;
    bool res = IsBtVlinkDeviceConnectOptimScene(transport);
    EXPECT_EQ(res, false);
    HILOGI("GattCTest_IsBtVlinkDeviceConnectOptimScene_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
