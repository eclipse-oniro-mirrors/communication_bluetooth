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
#include <future>
#include <memory>
#include "ble_central_manager_impl.h"
#include "ble_adapter.h"
#include "hw_bt_hwif.h"
#include "bluetooth_gatt_interface.h"
#include "bluetooth_hw_interface.h"
#include "bluetooth_host.h"
#include "class_creator.h"
#include "thread_util.h"
#include "ble_scan_filter.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_channel_rssi_manager.h"
#include "btif_gatt.h"

using namespace testing;
using namespace testing::ext;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

extern "C" {
static const int HANDLE = 2;
static const btgatt_callbacks_t* g_btgattCallbacks = nullptr;

struct UnionHisiJamChrInfoS {
    uint32_t rssiIdx[OHOS::bluetooth::BT_ACL_RSSI_REP_SIZE];
    int64_t actualRssiValue[OHOS::bluetooth::BT_ACL_RSSI_REP_SIZE];
    int64_t timestamp;
};

UnionHisiJamChrInfoS g_unionHisiJamChrInfo = {
    .rssiIdx = { 0 },
    .actualRssiValue = { 0 },
    .timestamp = 0,
};

static bt_status_t BtgattInterfaceInitMock(const btgatt_callbacks_t* callbacks)
{
    g_btgattCallbacks = callbacks;
    return BT_STATUS_SUCCESS;
}
static void BtgattInterfaceCleanupMock(void)
{
}

static btgatt_interface_t g_btgattInterface = {
    .size = sizeof(g_btgattInterface),
    .init = BtgattInterfaceInitMock,
    .cleanup = BtgattInterfaceCleanupMock,
    .client = BtGetGattClientInterface(),
    .server = BtGetGattServerInterface(),
    .scanner = nullptr,    // filled in btif_gatt_get_interface
    .advertiser = nullptr  // filled in btif_gatt_get_interface
};
static const BthwifCallbacks* g_bthwifCallbacks = nullptr;
static bt_status_t BthwifInterfaceInitMock(BthwifCallbacks* callbacks)
{
    g_bthwifCallbacks = callbacks;
    return BT_STATUS_SUCCESS;
}
static void SetBurstParamMock(int duration, int maxExtAdvEvents, int burstWindow, int burstInterval, int advId)
{
}
static void SetScanReportChannelSensorHubMock(uint8_t filterIdx, uint8_t isToAp)
{
}
static void SendParamsToSensorhubMock(const char *params, char type, int length)
{
}
static bool GetSensorhubCapMock()
{
    return true;
}
static bool GetSensorhubCapMock1()
{
    return false;
}
static uint8_t GetSensorhubAdvHandleMock()
{
    return 1;
}
static uint8_t GetSensorhubAdvHandleMock1()
{
    return HANDLE;
}
static void DeleteSensorhubFilterMock(char filterID)
{
}
static int HwSendChannelRssiReadCmd(const STACK::RawAddress &addr, uint8_t transport)
{
    return BT_STATUS_SUCCESS;
}
static int HwGetUnionHisiJamChrInfo(std::vector<uint32_t> &rssiIdx, std::vector<int64_t> &actualRssiValue,
    int64_t &timestamp)
{
    std::copy(g_unionHisiJamChrInfo.rssiIdx, g_unionHisiJamChrInfo.rssiIdx + OHOS::bluetooth::BT_ACL_RSSI_REP_SIZE,
        rssiIdx.begin());
    std::copy(g_unionHisiJamChrInfo.actualRssiValue, g_unionHisiJamChrInfo.actualRssiValue +
        OHOS::bluetooth::BT_ACL_RSSI_REP_SIZE, actualRssiValue.begin());
    timestamp = g_unionHisiJamChrInfo.timestamp;
    return BT_STATUS_SUCCESS;
}
static std::pair<uint32_t, uint32_t> HwGetBrTransTxRxRate()
{
    std::pair<uint32_t, uint32_t> p;
    return p;
}
static const bthwif_interface_t g_bthwifInterface = {
    .Init = BthwifInterfaceInitMock,
    .setBurstParam = SetBurstParamMock,
    .setScanReportChannelSensorHub = SetScanReportChannelSensorHubMock,
    .sendParamsToSensorhub = SendParamsToSensorhubMock,
    .getSensorhubCap = GetSensorhubCapMock,
    .getSensorhubAdvHandle = GetSensorhubAdvHandleMock,
    .deleteSensorhubFilter = DeleteSensorhubFilterMock,
    .hwSendChannelRssiReadCmd = HwSendChannelRssiReadCmd,
    .hwGetUnionHisiJamChrInfo = HwGetUnionHisiJamChrInfo,
};
static const bthwif_interface_t g_bthwifInterface1 = {
    .Init = BthwifInterfaceInitMock,
    .setBurstParam = SetBurstParamMock,
    .setScanReportChannelSensorHub = SetScanReportChannelSensorHubMock,
    .sendParamsToSensorhub = SendParamsToSensorhubMock,
    .getSensorhubCap = GetSensorhubCapMock1,
    .getSensorhubAdvHandle = GetSensorhubAdvHandleMock1,
    .deleteSensorhubFilter = DeleteSensorhubFilterMock,
    .hwSendChannelRssiReadCmd = HwSendChannelRssiReadCmd,
    .hwGetUnionHisiJamChrInfo = HwGetUnionHisiJamChrInfo,
    .hwGetBrTransTxRxRate = HwGetBrTransTxRxRate,
};
} // extern "C"
namespace OHOS {
namespace bluetooth {
class BleCentralManagerCallbackTest : public IBleCentralManagerCallback {
public:
    ~BleCentralManagerCallbackTest() override{};
    void OnScanCallback(const BleScanResultImpl &result) override {}
    void OnFoundOrLostCallback(uint8_t scanId, uint8_t advertiserState, const BleScanResultImpl &result) override {}
    void OnBleBatchScanResultsEvent(int clientIf, std::vector<Bluetooth::BluetoothBleScanResult> &results) override {}
    void OnStartOrStopScanEvent(int resultCode, bool isStartScan) override {}
    void OnNotifyMsgReportFromLpDevice(const FilterIdxInfo &info, uint8_t msgType,
        const std::vector<uint8_t> &notifyValue) override {}
};
const int TIME = 2;
const int TIME_4 = 4;
static Bluetooth::BluetoothHost *host_;
static std::unique_ptr<BleCentralManagerImpl> bleCentralManagerImpl;
static BleCentralManagerCallbackTest g_bleCentralManagerCallbackTest;
class BleCentralManagerImplTest : public testing::Test {
public:
    BleCentralManagerImplTest() = default;
    ~BleCentralManagerImplTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    void AddParams(std::vector<BleScanFilterImpl> scanFliterImpls);

    BleAdapter *bleAdapter;

private:
};

void BleCentralManagerImplTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &Bluetooth::BluetoothHost::GetDefaultHost();
    bleCentralManagerImpl = std::make_unique<BleCentralManagerImpl>(g_bleCentralManagerCallbackTest);

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

void BleCentralManagerImplTest::TearDownTestSuite(void)
{
    sleep(TIME_4);
    host_ = nullptr;
    bleCentralManagerImpl = nullptr;
}

void BleCentralManagerImplTest::SetUp()
{
    bleAdapter = ClassCreator<BleAdapter>::NewInstance(ADAPTER_NAME_BLE);

    // Enable ble thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ThreadState::ENABLED);
}

void BleCentralManagerImplTest::TearDown()
{
    delete bleAdapter;
    bleAdapter = nullptr;
}

/**
 * @tc.name: ble_centralManager_Start_001
 * @tc.desc: ble_centralManager_Start_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_Start_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_Start_001 start");
    bleCentralManagerImpl->Start();
    bleCentralManagerImpl->Start();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_Start_001 end");
}

/**
 * @tc.name: ble_centralManager_SetScanParametersCallback_001
 * @tc.desc: ble_centralManager_SetScanParametersCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetScanParametersCallback_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetScanParametersCallback_001 start");
    uint8_t status = 0;
    bleCentralManagerImpl->SetScanParametersCallback(status);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetScanParametersCallback_001 end");
}

/**
 * @tc.name: ble_centralManager_SetScanParametersCallback_002
 * @tc.desc: ble_centralManager_SetScanParametersCallback_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetScanParametersCallback_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetScanParametersCallback_002 start");
    uint8_t status = 1;
    // Dsiable ble thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ThreadState::DISABLED);
    bleCentralManagerImpl->SetScanParametersCallback(status);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetScanParametersCallback_002 end");
}

/**
 * @tc.name: ble_centralManager_SetScanParameters_001
 * @tc.desc: ble_centralManager_SetScanParameters_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetScanParameters_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetScanParameters_001 start");
    uint16_t intervalMSecs = 0;
    uint16_t windowMSecs = 0;
    bool legacy = false;
    int phy = 0;
    bleCentralManagerImpl->SetScanParameters(intervalMSecs, windowMSecs, legacy, phy);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetScanParameters_001 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_001
 * @tc.desc: ble_centralManager_StartScan_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_001 start");
    BleScanSettingsImpl setting;
    setting.SetCallbackType(4);
    setting.SetMatchTrackAdvType(1);
    setting.SetMatchMode(2);
    bleCentralManagerImpl->StartScan(setting);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_001 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_002
 * @tc.desc: ble_centralManager_StartScan_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_002 start");
    BleScanSettingsImpl setting;
    setting.SetPhy(4);
    bleCentralManagerImpl->StartScan(setting);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_002 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_003
 * @tc.desc: ble_centralManager_StartScan_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_003 start");
    BleScanSettingsImpl setting;
    setting.SetPhy(255);
    bleCentralManagerImpl->StartScan(setting);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_003 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_004
 * @tc.desc: ble_centralManager_StartScan_004
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_004, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_004 start");
    BleScanSettingsImpl setting;
    setting.SetPhy(1);
    bleCentralManagerImpl->StartScan(setting);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_004 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_005
 * @tc.desc: ble_centralManager_StartScan_005
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_005, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_005 start");
    BleScanSettingsImpl setting;
    setting.SetPhy(2);
    bleCentralManagerImpl->StartScan(setting);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_005 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_006
 * @tc.desc: ble_centralManager_StartScan_006
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_006, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_006 start");
    BleScanSettingsImpl setting;
    setting.SetPhy(3);
    bleCentralManagerImpl->StartScan(setting);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_006 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_008
 * @tc.desc: ble_centralManager_StartScan_008
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_008, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_008 start");
    bleCentralManagerImpl->Start();
    sleep(TIME);
    BleScanSettingsImpl setting;
    bleCentralManagerImpl->StartScan(setting);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_008 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_009
 * @tc.desc: ble_centralManager_StartScan_009
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_009, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_009 start");
    bleCentralManagerImpl->Start();
    sleep(TIME);
    BleScanSettingsImpl setting;
    setting.SetScanMode(0);
    bleCentralManagerImpl->StartScan(setting);
    setting.SetScanMode(8);
    bleCentralManagerImpl->StartScan(setting);
    int scanMode = SCAN_MODE_LOW_POWER;
    uint16_t outScanWindow = BLE_SCAN_MODE_LOW_POWER_INTERVAL_MS;
    uint16_t outScanInterval = BLE_SCAN_MODE_LOW_POWER_WINDOW_MS;
    ConvertBleScanMode(scanMode, outScanWindow, outScanInterval);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_009 end");
}

/**
 * @tc.name: ble_centralManager_StartScan_010
 * @tc.desc: ble_centralManager_StartScan_010
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StartScan_010, TestSize.Level1)
{
    HILOGI("ble_centralManager_StartScan_010 start");
    bleCentralManagerImpl->Start();
    sleep(TIME);
    BleScanSettingsImpl setting;
    setting.SetScanMode(10);
    bleCentralManagerImpl->StartScan(setting);
    setting.SetScanMode(11);
    bleCentralManagerImpl->StartScan(setting);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_StartScan_010 end");
}

/**
 * @tc.name: ble_centralManager_StopScan_001
 * @tc.desc: ble_centralManager_StopScan_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StopScan_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_StopScan_001 start");
    bleCentralManagerImpl->StopScan();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_NOT_STARTED);
    HILOGI("ble_centralManager_StopScan_001 end");
}

/**
 * @tc.name: ble_centralManager_StopScan_002
 * @tc.desc: ble_centralManager_StopScan_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StopScan_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_StopScan_002 start");
    bleCentralManagerImpl->Start();
    sleep(TIME);
    bleCentralManagerImpl->StopScan();
    bleCentralManagerImpl->btifBleScanner_ = nullptr;
    bleCentralManagerImpl->StopScan();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_NOT_STARTED);
    HILOGI("ble_centralManager_StopScan_002 end");
}

/**
 * @tc.name: ble_centralManager_StopScanWithDisabling_001
 * @tc.desc: ble_centralManager_StopScanWithDisabling_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StopScanWithDisabling_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_StopScanWithDisabling_001 start");
    bleCentralManagerImpl->StopScanWithDisabling();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_NOT_STARTED);
    HILOGI("ble_centralManager_StopScanWithDisabling_001 end");
}

/**
 * @tc.name: ble_centralManager_StopScanWithDisabling_002
 * @tc.desc: ble_centralManager_StopScanWithDisabling_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_StopScanWithDisabling_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_StopScanWithDisabling_002 start");
    bleCentralManagerImpl->Start();
    sleep(TIME);
    bleCentralManagerImpl->StopScanWithDisabling();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_NOT_STARTED);
    HILOGI("ble_centralManager_StopScanWithDisabling_002 end");
}

/**
 * @tc.name: ble_centralManager_DeregisterCallbackToGap_001
 * @tc.desc: ble_centralManager_DeregisterCallbackToGap_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_DeregisterCallbackToGap_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_DeregisterCallbackToGap_001 start");
    bleCentralManagerImpl->DeregisterCallbackToGap();
    bleCentralManagerImpl->Start();
    sleep(TIME);
    bleCentralManagerImpl->DeregisterCallbackToGap();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_DeregisterCallbackToGap_001 end");
}

/**
 * @tc.name: ble_centralManager_ConfigScanFilter_001
 * @tc.desc: ble_centralManager_ConfigScanFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ConfigScanFilter_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_ConfigScanFilter_001 start");
    int32_t scannerId = 1;
    BleScanSettingsImpl settings;
    std::vector<BleScanFilterImpl> filters;
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_ConfigScanFilter_001 end");
}

/**
 * @tc.name: ble_centralManager_ConfigScanFilter_002
 * @tc.desc: ble_centralManager_ConfigScanFilter_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ConfigScanFilter_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_ConfigScanFilter_002 start");
    int32_t scannerId = 0;
    BleScanSettingsImpl settings;
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);
    settings.SetCallbackType(6);
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_ConfigScanFilter_002 end");
}

/**
 * @tc.name: ble_centralManager_ConfigScanFilter_003
 * @tc.desc: ble_centralManager_ConfigScanFilter_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ConfigScanFilter_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_ConfigScanFilter_003 start");
    int32_t scannerId = 1;
    BleScanSettingsImpl settings;
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_ConfigScanFilter_003 end");
}

/**
 * @tc.name: ble_centralManager_ConfigScanFilter_004
 * @tc.desc: ble_centralManager_ConfigScanFilter_004
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ConfigScanFilter_004, TestSize.Level1)
{
    HILOGI("ble_centralManager_ConfigScanFilter_004 start");
    int32_t scannerId = 1;
    std::string filter1Name = "name1";
    std::string filter2Name = "name2";
    BleScanSettingsImpl settings;
    BleScanFilterImpl filter1;
    filter1.SetName(filter1Name);
    BleScanFilterImpl filter2;
    filter2.SetName(filter2Name);
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter1);
    filters.push_back(filter2);
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_ConfigScanFilter_004 end");
}

/**
 * @tc.name: ble_centralManager_RemoveScanFilter_001
 * @tc.desc: ble_centralManager_RemoveScanFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_RemoveScanFilter_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_RemoveScanFilter_001 start");
    std::vector<BleScanFilterImpl> filters;
    BleScanSettingsImpl settings;
    bleCentralManagerImpl->RemoveScanFilter(1, settings, filters.size());
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_RemoveScanFilter_001 end");
}

/**
 * @tc.name: ble_centralManager_RemoveScanFilter_002
 * @tc.desc: ble_centralManager_RemoveScanFilter_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_RemoveScanFilter_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_RemoveScanFilter_002 start");
    int32_t scannerId = 1;
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    BleScanSettingsImpl settings;
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);
    bleCentralManagerImpl->RemoveScanFilter(scannerId, settings, filters.size());
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_RemoveScanFilter_002 end");
}

/**
 * @tc.name: ble_centralManager_AllocScannerId_001
 * @tc.desc: ble_centralManager_AllocScannerId_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_AllocScannerId_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_AllocScannerId_001 start");
    bleCentralManagerImpl->AllocScannerId();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_AllocScannerId_001 end");
}

/**
 * @tc.name: ble_centralManager_AllocScannerId_002
 * @tc.desc: ble_centralManager_AllocScannerId_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_AllocScannerId_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_AllocScannerId_002 start");
    bleCentralManagerImpl->AllocScannerId();
    bleCentralManagerImpl->AllocScannerId();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_AllocScannerId_002 end");
}

/**
 * @tc.name: ble_centralManager_ClearScannerId_001
 * @tc.desc: ble_centralManager_ClearScannerId_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ClearScannerId_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_ClearScannerId_001 start");
    bleCentralManagerImpl->ClearScannerId();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_ClearScannerId_001 end");
}

/**
 * @tc.name: ble_centralManager_SetLpDeviceAdvParam_001
 * @tc.desc: ble_centralManager_SetLpDeviceAdvParam_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetLpDeviceAdvParam_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetLpDeviceAdvParam_001 start");
    int duration = 1;
    int maxExtAdvEvents = 1;
    int window = 1;
    int interval = 1;
    int advHandle = 1;
    bleCentralManagerImpl->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetLpDeviceAdvParam_001 end");
}

/**
 * @tc.name: ble_centralManager_SetScanReportChannelToLpDevice_001
 * @tc.desc: ble_centralManager_SetScanReportChannelToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetScanReportChannelToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetScanReportChannelToLpDevice_001 start");
    int32_t scannerId = 1;
    bool enable = false;
    bleCentralManagerImpl->SetScanReportChannelToLpDevice(scannerId, enable);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetScanReportChannelToLpDevice_001 end");
}

/**
 * @tc.name: ble_centralManager_SendParamsToLpDevice_001
 * @tc.desc: ble_centralManager_SendParamsToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendParamsToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendParamsToLpDevice_001 start");
    std::vector<uint8_t> dataValue;
    int32_t type = 1;
    bleCentralManagerImpl->SendParamsToLpDevice(dataValue, type);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendParamsToLpDevice_001 end");
}

/**
 * @tc.name: ble_centralManager_IsLpDeviceAvailable_001
 * @tc.desc: ble_centralManager_IsLpDeviceAvailable_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_IsLpDeviceAvailable_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_IsLpDeviceAvailable_001 start");
    EXPECT_FALSE(bleCentralManagerImpl->IsLpDeviceAvailable());
    HILOGI("ble_centralManager_IsLpDeviceAvailable_001 end");
}

/**
 * @tc.name: ble_centralManager_NotifyScanStatusToLpDevice_001
 * @tc.desc: ble_centralManager_NotifyScanStatusToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_NotifyScanStatusToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_NotifyScanStatusToLpDevice_001 start");
    bool isStart = false;
    bleCentralManagerImpl->NotifyScanStatusToLpDevice(isStart);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_NotifyScanStatusToLpDevice_001 end");
}

/**
 * @tc.name: ble_centralManager_NotifyScanStatusToLpDevice_002
 * @tc.desc: ble_centralManager_NotifyScanStatusToLpDevice_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_NotifyScanStatusToLpDevice_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_NotifyScanStatusToLpDevice_002 start");
    bool isStart = false;
    bleCentralManagerImpl->EnableSyncDataToLpDevice();
    bleCentralManagerImpl->NotifyScanStatusToLpDevice(isStart);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_NotifyScanStatusToLpDevice_002 end");
}

/**
 * @tc.name: ble_centralManager_NotifyScanStatusToLpDevice_003
 * @tc.desc: ble_centralManager_NotifyScanStatusToLpDevice_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_NotifyScanStatusToLpDevice_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_NotifyScanStatusToLpDevice_003 start");
    bool isStart = true;
    bleCentralManagerImpl->EnableSyncDataToLpDevice();
    bleCentralManagerImpl->NotifyScanStatusToLpDevice(isStart);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_NotifyScanStatusToLpDevice_003 end");
}

/**
 * @tc.name: ble_centralManager_NotifyScanParamsToLpDevice_001
 * @tc.desc: ble_centralManager_NotifyScanParamsToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_NotifyScanParamsToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_001 start");
    uint32_t scanWindow = 0;
    uint32_t scanInterval = 0;
    bleCentralManagerImpl->NotifyScanParamsToLpDevice(scanWindow, scanInterval);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_001 end");
}

/**
 * @tc.name: ble_centralManager_NotifyScanParamsToLpDevice_002
 * @tc.desc: ble_centralManager_NotifyScanParamsToLpDevice_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_NotifyScanParamsToLpDevice_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_002 start");
    uint32_t scanWindow = 0;
    uint32_t scanInterval = 0;
    bleCentralManagerImpl->EnableSyncDataToLpDevice();
    bleCentralManagerImpl->NotifyScanParamsToLpDevice(scanWindow, scanInterval);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_002 end");
}

/**
 * @tc.name: ble_centralManager_NotifyScanParamsToLpDevice_003
 * @tc.desc: ble_centralManager_NotifyScanParamsToLpDevice_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_NotifyScanParamsToLpDevice_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_003 start");
    uint32_t scanWindow = 1;
    uint32_t scanInterval = 0;
    bleCentralManagerImpl->EnableSyncDataToLpDevice();
    bleCentralManagerImpl->NotifyScanParamsToLpDevice(scanWindow, scanInterval);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_003 end");
}

/**
 * @tc.name: ble_centralManager_NotifyScanParamsToLpDevice_004
 * @tc.desc: ble_centralManager_NotifyScanParamsToLpDevice_004
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_NotifyScanParamsToLpDevice_004, TestSize.Level1)
{
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_004 start");
    uint32_t scanWindow = 1;
    uint32_t scanInterval = 1;
    bleCentralManagerImpl->EnableSyncDataToLpDevice();
    bleCentralManagerImpl->NotifyScanParamsToLpDevice(scanWindow, scanInterval);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_004 end");
}

/**
 * @tc.name: ble_centralManager_NotifyScanParamsToLpDevice_005
 * @tc.desc: ble_centralManager_NotifyScanParamsToLpDevice_005
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_NotifyScanParamsToLpDevice_005, TestSize.Level1)
{
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_005 start");
    uint32_t scanWindow = 0;
    uint32_t scanInterval = 1;
    bleCentralManagerImpl->EnableSyncDataToLpDevice();
    bleCentralManagerImpl->NotifyScanParamsToLpDevice(scanWindow, scanInterval);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_NotifyScanParamsToLpDevice_005 end");
}

/**
 * @tc.name: ble_centralManager_GetLpDeviceAdvHandle_001
 * @tc.desc: ble_centralManager_GetLpDeviceAdvHandle_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_GetLpDeviceAdvHandle_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_GetLpDeviceAdvHandle_001 start");
    bleCentralManagerImpl->GetLpDeviceAdvHandle();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_GetLpDeviceAdvHandle_001 end");
}

/**
 * @tc.name: ble_centralManager_SetLpDeviceParam_001
 * @tc.desc: ble_centralManager_SetLpDeviceParam_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetLpDeviceParam_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetLpDeviceParam_001 start");
    FilterIdxInfo filterIdx;
    BleLpDeviceParamSetImpl paramImpl;
    bleCentralManagerImpl->SetLpDeviceParam(filterIdx, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetLpDeviceParam_001 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvHandleDataToLpDevice_001
 * @tc.desc: ble_centralManager_SendAdvHandleDataToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvHandleDataToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvHandleDataToLpDevice_001 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.advHandle = -100;
    bleCentralManagerImpl->SendAdvHandleDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvHandleDataToLpDevice_001 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvHandleDataToLpDevice_002
 * @tc.desc: ble_centralManager_SendAdvHandleDataToLpDevice_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvHandleDataToLpDevice_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvHandleDataToLpDevice_002 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.advHandle = 0;
    bleCentralManagerImpl->SendAdvHandleDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvHandleDataToLpDevice_002 end");
}

/**
 * @tc.name: ble_centralManager_SendScanDataToLpDevice_001
 * @tc.desc: ble_centralManager_SendScanDataToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendScanDataToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendScanDataToLpDevice_001 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 0;
    bleCentralManagerImpl->SendScanDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendScanDataToLpDevice_001 end");
}

/**
 * @tc.name: ble_centralManager_SendScanDataToLpDevice_002
 * @tc.desc: ble_centralManager_SendScanDataToLpDevice_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendScanDataToLpDevice_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendScanDataToLpDevice_002 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 1;
    bleCentralManagerImpl->SendScanDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendScanDataToLpDevice_002 end");
}

/**
 * @tc.name: ble_centralManager_SendScanDataToLpDevice_003
 * @tc.desc: ble_centralManager_SendScanDataToLpDevice_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendScanDataToLpDevice_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendScanDataToLpDevice_003 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 2;
    bleCentralManagerImpl->SendScanDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendScanDataToLpDevice_003 end");
}

/**
 * @tc.name: ble_centralManager_SendScanDataToLpDevice_004
 * @tc.desc: ble_centralManager_SendScanDataToLpDevice_004
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendScanDataToLpDevice_004, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendScanDataToLpDevice_004 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 3;
    bleCentralManagerImpl->SendScanDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendScanDataToLpDevice_004 end");
}

void BleCentralManagerImplTest::AddParams(std::vector<BleScanFilterImpl> scanFliterImpls)
{
    Uuid uuid;
    std::vector<uint8_t> data;
    data.push_back(1);
    std::vector<uint8_t> dataMask;
    dataMask.push_back(1);
    BleScanFilterImpl bleScanFilterImpl;
    bleScanFilterImpl.SetManufacturerId(0);
    BleScanFilterImpl bleScanFilterImpl1;
    bleScanFilterImpl1.SetDeviceId("00:00:00:00:00:00");
    bleScanFilterImpl1.SetServiceUuid(uuid);
    bleScanFilterImpl1.SetServiceSolicitationUuid(uuid);
    bleScanFilterImpl1.SetName("testData");
    bleScanFilterImpl1.SetManufacturerId(1);
    bleScanFilterImpl1.SetManufactureData(data);
    bleScanFilterImpl1.SetServiceData(data);
    BleScanFilterImpl bleScanFilterImpl2;
    bleScanFilterImpl2.SetDeviceId("00:00:00:00:00:01");
    BleScanFilterImpl bleScanFilterImpl3;
    bleScanFilterImpl3.SetDeviceId("00:00:00:00:00:00");
    bleScanFilterImpl3.SetServiceUuid(uuid);
    bleScanFilterImpl3.SetServiceSolicitationUuid(uuid);
    bleScanFilterImpl3.SetName("testData");
    bleScanFilterImpl3.SetManufacturerId(1);
    bleScanFilterImpl3.SetManufactureData(data);
    bleScanFilterImpl3.SetServiceData(data);
    bleScanFilterImpl3.SetServiceDataMask(dataMask);
    bleScanFilterImpl3.SetManufactureDataMask(dataMask);
    BleScanFilterImpl bleScanFilterImpl4;
    bleScanFilterImpl4.SetDeviceId("00:00:00:00:00:00");
    bleScanFilterImpl4.SetServiceUuid(uuid);
    bleScanFilterImpl4.SetServiceSolicitationUuid(uuid);
    bleScanFilterImpl4.SetName("testData");
    bleScanFilterImpl4.SetManufacturerId(1);
    bleScanFilterImpl4.SetServiceData(data);
    bleScanFilterImpl4.SetServiceDataMask(dataMask);
    bleScanFilterImpl4.SetManufactureDataMask(dataMask);
    scanFliterImpls.push_back(bleScanFilterImpl);
    scanFliterImpls.push_back(bleScanFilterImpl1);
    scanFliterImpls.push_back(bleScanFilterImpl2);
    scanFliterImpls.push_back(bleScanFilterImpl3);
    scanFliterImpls.push_back(bleScanFilterImpl4);
}

/**
 * @tc.name: ble_centralManager_SendScanDataToLpDevice_005
 * @tc.desc: ble_centralManager_SendScanDataToLpDevice_005
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendScanDataToLpDevice_005, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendScanDataToLpDevice_005 start");
    Uuid uuid;
    std::vector<BleScanFilterImpl> scanFliterImpls;
    AddParams(scanFliterImpls);
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.scanFliterImpls = scanFliterImpls;
    paramImpl.fieldValidFlagBit = 3;
    bleCentralManagerImpl->SendScanDataToLpDevice(1, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendScanDataToLpDevice_005 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvertiserDataToLpDevice_001
 * @tc.desc: ble_centralManager_SendAdvertiserDataToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvertiserDataToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_001 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 0;
    bleCentralManagerImpl->SendAdvertiserDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_001 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvertiserDataToLpDevice_002
 * @tc.desc: ble_centralManager_SendAdvertiserDataToLpDevice_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvertiserDataToLpDevice_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_002 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 4;
    bleCentralManagerImpl->SendAdvertiserDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_002 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvertiserDataToLpDevice_003
 * @tc.desc: ble_centralManager_SendAdvertiserDataToLpDevice_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvertiserDataToLpDevice_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_003 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 8;
    bleCentralManagerImpl->SendAdvertiserDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_003 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvertiserDataToLpDevice_004
 * @tc.desc: ble_centralManager_SendAdvertiserDataToLpDevice_004
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvertiserDataToLpDevice_004, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_004 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleAdvertiserSettingsImpl settings;
    settings.SetConnectable(false);
    settings.SetLegacyMode(false);
    settings.SetTxPower(4);
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.advSettingsImpl = settings;
    paramImpl.fieldValidFlagBit = 12;
    bleCentralManagerImpl->SendAdvertiserDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_004 end");
}

/**
 * @tc.name: ble_centralManager_UpdateScanFilter_001
 * @tc.desc: ble_centralManager_UpdateScanFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_UpdateScanFilter_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_UpdateScanFilter_001 start");
    int ret = Bluetooth::BT_NO_ERROR;
    BleScanSettingsImpl settings;
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    ret = bleCentralManagerImpl->UpdateScanFilter(0, settings, filters, 0);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_INTERNAL_ERROR);
    ret = bleCentralManagerImpl->UpdateScanFilter(0, settings, filters, 1);
    ret = bleCentralManagerImpl->UpdateScanFilter(0, settings, filters, 3);
    ret = bleCentralManagerImpl->UpdateScanFilter(0, settings, filters, 2);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_UpdateScanFilter_001 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvertiserDataToLpDevice_005
 * @tc.desc: ble_centralManager_SendAdvertiserDataToLpDevice_005
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvertiserDataToLpDevice_005, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_005 start");
    uint8_t idx = 1;
    Uuid uuid;
    BleAdvertiserSettingsImpl settings;
    settings.SetConnectable(true);
    settings.SetLegacyMode(true);
    settings.SetTxPower(1);
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.advSettingsImpl = settings;
    paramImpl.fieldValidFlagBit = 12;
    bleCentralManagerImpl->SendAdvertiserDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvertiserDataToLpDevice_005 end");
}

/**
 * @tc.name: ble_centralManager_RemoveLpDeviceParam_001
 * @tc.desc: ble_centralManager_RemoveLpDeviceParam_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_RemoveLpDeviceParam_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_RemoveLpDeviceParam_001 start");
    FilterIdxInfo filterIdx;
    bleCentralManagerImpl->RemoveLpDeviceParam(filterIdx);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_RemoveLpDeviceParam_001 end");
}

/**
 * @tc.name: ble_centralManager_FindShFilterIdx_001
 * @tc.desc: ble_centralManager_FindShFilterIdx_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_FindShFilterIdx_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_FindShFilterIdx_001 start");
    FilterIdxInfo info;
    info.pid = 1;
    bleCentralManagerImpl->shFilterIndexMap.insert(std::make_pair(3, info));
    FilterIdxInfo info1;
    bleCentralManagerImpl->FindShFilterIdx(info1);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_FindShFilterIdx_001 end");
}

/**
 * @tc.name: ble_centralManager_GetShFilterIdx_001
 * @tc.desc: ble_centralManager_GetShFilterIdx_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_GetShFilterIdx_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_GetShFilterIdx_001 start");
    uint8_t idx = 1;
    bleCentralManagerImpl->GetShFilterIdx(idx);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_GetShFilterIdx_001 end");
}

/**
 * @tc.name: ble_centralManager_SetLpDeviceAdvParam_002
 * @tc.desc: ble_centralManager_SetLpDeviceAdvParam_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetLpDeviceAdvParam_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetLpDeviceAdvParam_002 start");
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    int duration = 1;
    int maxExtAdvEvents = 1;
    int window = 1;
    int interval = 1;
    int advHandle = 1;
    bleCentralManagerImpl->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetLpDeviceAdvParam_002 end");
}

/**
 * @tc.name: ble_centralManager_SetScanReportChannelToLpDevice_002
 * @tc.desc: ble_centralManager_SetScanReportChannelToLpDevice_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetScanReportChannelToLpDevice_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetScanReportChannelToLpDevice_002 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    int32_t scannerId = 1;
    bool enable = false;
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    BleScanSettingsImpl settings;
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);
    bleCentralManagerImpl->SetScanReportChannelToLpDevice(scannerId, enable);
    bleCentralManagerImpl->SetScanReportChannelToLpDevice(0, enable);
    bleCentralManagerImpl->SetScanReportChannelToLpDevice(0, true);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetScanReportChannelToLpDevice_002 end");
}

/**
 * @tc.name: ble_centralManager_SendParamsToLpDevice_002
 * @tc.desc: ble_centralManager_SendParamsToLpDevice_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendParamsToLpDevice_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendParamsToLpDevice_002 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    std::vector<uint8_t> dataValue;
    int32_t type = 1;
    bleCentralManagerImpl->SendParamsToLpDevice(dataValue, type);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendParamsToLpDevice_002 end");
}

/**
 * @tc.name: ble_centralManager_IsLpDeviceAvailable_002
 * @tc.desc: ble_centralManager_IsLpDeviceAvailable_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_IsLpDeviceAvailable_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_IsLpDeviceAvailable_002 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    EXPECT_TRUE(bleCentralManagerImpl->IsLpDeviceAvailable());
    HILOGI("ble_centralManager_IsLpDeviceAvailable_002 end");
}
/**
 * @tc.name: ble_centralManager_IsLpDeviceAvailable_003
 * @tc.desc: ble_centralManager_IsLpDeviceAvailable_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_IsLpDeviceAvailable_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_IsLpDeviceAvailable_003 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface1);
    EXPECT_FALSE(bleCentralManagerImpl->IsLpDeviceAvailable());
    HILOGI("ble_centralManager_IsLpDeviceAvailable_003 end");
}

/**
 * @tc.name: ble_centralManager_GetLpDeviceAdvHandle_002
 * @tc.desc: ble_centralManager_GetLpDeviceAdvHandle_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_GetLpDeviceAdvHandle_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_GetLpDeviceAdvHandle_002 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    bleCentralManagerImpl->GetLpDeviceAdvHandle();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_GetLpDeviceAdvHandle_002 end");
}

/**
 * @tc.name: ble_centralManager_GetLpDeviceAdvHandle_003
 * @tc.desc: ble_centralManager_GetLpDeviceAdvHandle_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_GetLpDeviceAdvHandle_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_GetLpDeviceAdvHandle_003 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface1);
    bleCentralManagerImpl->GetLpDeviceAdvHandle();
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_GetLpDeviceAdvHandle_003 end");
}

/**
 * @tc.name: ble_centralManager_RemoveLpDeviceParam_002
 * @tc.desc: ble_centralManager_RemoveLpDeviceParam_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_RemoveLpDeviceParam_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_RemoveLpDeviceParam_002 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    Uuid uuid;
    FilterIdxInfo filterIdx;
    bleCentralManagerImpl->RemoveLpDeviceParam(filterIdx);
    bleCentralManagerImpl->AddShFilterIdx(filterIdx);
    bleCentralManagerImpl->RemoveLpDeviceParam(filterIdx);
    FilterIdxInfo filterIdx1(1, 1, uuid);
    bleCentralManagerImpl->AddShFilterIdx(filterIdx1);
    bleCentralManagerImpl->RemoveLpDeviceParam(filterIdx1);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_RemoveLpDeviceParam_002 end");
}

/**
 * @tc.name: ble_centralManager_SetLpDeviceParam_002
 * @tc.desc: ble_centralManager_SetLpDeviceParam_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetLpDeviceParam_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetLpDeviceParam_002 start");
    FilterIdxInfo filterIdx;
    BleLpDeviceParamSetImpl paramImpl;
    bleCentralManagerImpl->AddShFilterIdx(filterIdx);
    bleCentralManagerImpl->SetLpDeviceParam(filterIdx, paramImpl);
    FilterIdxInfo info;
    info.pid = 1;
    bleCentralManagerImpl->shFilterIndexMap.clear();
    bleCentralManagerImpl->shFilterIndexMap.insert(std::make_pair(0, info));
    bleCentralManagerImpl->SetLpDeviceParam(filterIdx, paramImpl);
    bleCentralManagerImpl->shFilterIndexMap.clear();
    BleCentralManagerImpl::ShFilterIdx::GetInstance()->currentIndex_ = 101;
    bleCentralManagerImpl->SetLpDeviceParam(filterIdx, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetLpDeviceParam_002 end");
}

/**
 * @tc.name: ble_centralManager_AddShFilterIdx_001
 * @tc.desc: ble_centralManager_AddShFilterIdx_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_AddShFilterIdx_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_AddShFilterIdx_001 start");
    FilterIdxInfo filterIdx;
    BleCentralManagerImpl::ShFilterIdx* filterIndex = BleCentralManagerImpl::ShFilterIdx::GetInstance();
    filterIndex->Release(1);
    filterIndex->Clear();
    bleCentralManagerImpl->AddShFilterIdx(filterIdx);
    filterIndex->Clear();
    bleCentralManagerImpl->AddShFilterIdx(filterIdx);
    for (int i = 0; i < 40; i++) {
        filterIndex->GetOne();
    }
    for (int i = 0; i < 60; i++) {
        filterIndex->GetOne();
    }
    BleLpDeviceParamSetImpl paramImpl;
    bleCentralManagerImpl->SetLpDeviceParam(filterIdx, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_AddShFilterIdx_001 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvHandleDataToLpDevice_003
 * @tc.desc: ble_centralManager_SendAdvHandleDataToLpDevice_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvHandleDataToLpDevice_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvHandleDataToLpDevice_003 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface1);
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 0;
    paramImpl.advHandle = -100;
    bleCentralManagerImpl->SendAdvHandleDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvHandleDataToLpDevice_003 end");
}

/**
 * @tc.name: ble_centralManager_SendAdvHandleDataToLpDevice_004
 * @tc.desc: ble_centralManager_SendAdvHandleDataToLpDevice_004
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SendAdvHandleDataToLpDevice_004, TestSize.Level1)
{
    HILOGI("ble_centralManager_SendAdvHandleDataToLpDevice_004 start");
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface1);
    uint8_t idx = 1;
    Uuid uuid;
    BleLpDeviceParamSetImpl paramImpl;
    paramImpl.fieldValidFlagBit = 4;
    paramImpl.advHandle = -100;
    bleCentralManagerImpl->SendAdvHandleDataToLpDevice(idx, uuid, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SendAdvHandleDataToLpDevice_004 end");
}

/**
 * @tc.name: ble_centralManager_ScanResultCallback_001
 * @tc.desc: ble_centralManager_ScanResultCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ScanResultCallback_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_ScanResultCallback_001 start");
    bleCentralManagerImpl->Start();
    BluetoothGattInterface::GetInstance()->Initialize(&g_btgattInterface);
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    uint16_t bleEvtType = 1;
    uint8_t addrType = 1;
    STACK::RawAddress bda;
    uint8_t blePrimaryPhy = 1;
    uint8_t bleSecondaryPhy = 1;
    uint8_t bleAdvertisingSid = 1;
    int8_t bleTxPower = 1;
    int8_t rssi = 1;
    uint16_t blePeriodicAdvInt = 1;
    std::vector<uint8_t> advData;
    g_btgattCallbacks->scanner->scan_result_cb(bleEvtType, addrType, &bda, blePrimaryPhy,
    bleSecondaryPhy, bleAdvertisingSid, bleTxPower, rssi, blePeriodicAdvInt, advData);
    g_btgattCallbacks->scanner->scan_result_cb(bleEvtType, addrType, nullptr, blePrimaryPhy,
    bleSecondaryPhy, bleAdvertisingSid, bleTxPower, rssi, blePeriodicAdvInt, advData);
    advData.push_back(1);
    g_btgattCallbacks->scanner->scan_result_cb(bleEvtType, addrType, &bda, blePrimaryPhy,
    bleSecondaryPhy, bleAdvertisingSid, bleTxPower, rssi, blePeriodicAdvInt, advData);

    btgatt_track_adv_info_t trackAdvInfo;
    g_btgattCallbacks->scanner->track_adv_event_cb(&trackAdvInfo);
    std::vector<uint8_t> trackAdvData;
    trackAdvData.push_back(1);
    trackAdvInfo.p_adv_pkt_data = trackAdvData.data();
    trackAdvInfo.adv_pkt_len = trackAdvData.size();
    g_btgattCallbacks->scanner->track_adv_event_cb(&trackAdvInfo);
    std::vector<uint8_t> trackScanRsp;
    trackScanRsp.push_back(1);
    trackAdvInfo.p_scan_rsp_data = trackScanRsp.data();
    trackAdvInfo.scan_rsp_len = trackScanRsp.size();
    g_btgattCallbacks->scanner->track_adv_event_cb(&trackAdvInfo);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_ScanResultCallback_001 end");
}

/**
 * @tc.name: ble_centralManager_SensorhubDevInfoCallback_001
 * @tc.desc: ble_centralManager_SensorhubDevInfoCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SensorhubDevInfoCallback_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SensorhubDevInfoCallback_001 start");
    bleCentralManagerImpl->Start();
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    uint8_t buffer[5] = {1, 2, 3, 4, 5};
    uint8_t buffer1[5] = {2, 2, 3, 4, 5};
    int length = 1;
    g_bthwifCallbacks->hwGetSensorhubDevInfoCb(nullptr, length);
    g_bthwifCallbacks->hwGetSensorhubDevInfoCb(nullptr, 0);
    g_bthwifCallbacks->hwGetSensorhubDevInfoCb(buffer, 0);
    g_bthwifCallbacks->hwGetSensorhubDevInfoCb(buffer, length);
    g_bthwifCallbacks->hwGetSensorhubDevInfoCb(buffer1, length);
    g_bthwifCallbacks->hwGetSensorhubDevInfoCb(buffer1, 5);
    FilterIdxInfo filterIdx;
    bleCentralManagerImpl->AddShFilterIdx(filterIdx);
    g_bthwifCallbacks->hwGetSensorhubDevInfoCb(buffer1, 5);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SensorhubDevInfoCallback_001 end");
}

/**
 * @tc.name: ble_centralManager_SensorhubResetCallback_001
 * @tc.desc: ble_centralManager_SensorhubResetCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SensorhubResetCallback_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_SensorhubResetCallback_001 start");
    bleCentralManagerImpl->Start();
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    g_bthwifCallbacks->hwSensorhubResetCb(1);
    g_bthwifCallbacks->hwSensorhubResetCb(2);
    g_bthwifCallbacks->hwSensorhubResetCb(3);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SensorhubResetCallback_001 end");
}

/**
 * @tc.name: ble_centralManager_SetLpDeviceParam_003
 * @tc.desc: ble_centralManager_SetLpDeviceParam_003
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_SetLpDeviceParam_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_SetLpDeviceParam_003 start");
    bleCentralManagerImpl->Start();
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    g_bthwifCallbacks->hwSensorhubResetCb(1);
    FilterIdxInfo filterIdx;
    BleLpDeviceParamSetImpl paramImpl;
    bleCentralManagerImpl->SetLpDeviceParam(filterIdx, paramImpl);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_SetLpDeviceParam_003 end");
}

/**
 * @tc.name: ble_centralManager_IsLpDeviceAvailable_004
 * @tc.desc: ble_centralManager_IsLpDeviceAvailable_004
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_IsLpDeviceAvailable_004, TestSize.Level1)
{
    HILOGI("ble_centralManager_IsLpDeviceAvailable_004 start");
    bleCentralManagerImpl->Start();
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    g_bthwifCallbacks->hwSensorhubResetCb(1);
    EXPECT_FALSE(bleCentralManagerImpl->IsLpDeviceAvailable());
    HILOGI("ble_centralManager_IsLpDeviceAvailable_004 end");
}

/**
 * @tc.name: ble_centralManager_IsLpDeviceAvailable_005
 * @tc.desc: ble_centralManager_IsLpDeviceAvailable_005
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_IsLpDeviceAvailable_005, TestSize.Level1)
{
    HILOGI("ble_centralManager_IsLpDeviceAvailable_005 start");
    bleCentralManagerImpl->Start();
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface1);
    g_bthwifCallbacks->hwSensorhubResetCb(1);
    EXPECT_FALSE(bleCentralManagerImpl->IsLpDeviceAvailable());
    HILOGI("ble_centralManager_IsLpDeviceAvailable_005 end");
}

/**
 * @tc.name: ble_centralManager_GetDeliveryMode_001
 * @tc.desc: ble_centralManager_GetDeliveryMode_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_GetDeliveryMode_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_GetDeliveryMode_001 start");
    BleScanSettingsImpl settings;
    settings.SetCallbackType(6);
    EXPECT_EQ(BleScanFilter::DELIVERY_MODE_ON_FOUND, bleCentralManagerImpl->GetDeliveryMode(settings));
    settings.SetCallbackType(1);
    EXPECT_EQ(BleScanFilter::DELIVERY_MODE_IMMEDIATE, bleCentralManagerImpl->GetDeliveryMode(settings));
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_GetDeliveryMode_001 end");
}

/**
 * @tc.name: ble_centralManager_GetNumOfTrackAdvs_001
 * @tc.desc: ble_centralManager_GetNumOfTrackAdvs_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_GetNumOfTrackAdvs_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_GetNumOfTrackAdvs_001 start");
    EXPECT_EQ(BleScanFilter::NUM_OF_ONE_MATCH_TRACK_ADV_TYPE, bleCentralManagerImpl->GetNumOfTrackAdvs(1));
    EXPECT_EQ(BleScanFilter::NUM_OF_FEW_MATCH_TRACK_ADV_TYPE, bleCentralManagerImpl->GetNumOfTrackAdvs(2));
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_GetNumOfTrackAdvs_001 end");
}

/**
 * @tc.name: ble_centralManager_RemoveScanFilterInner_001
 * @tc.desc: ble_centralManager_RemoveScanFilterInner_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_RemoveScanFilterInner_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_RemoveScanFilterInner_001 start");
    int ret = Bluetooth::BT_NO_ERROR;
    int32_t scannerId = 1;
    BleScanSettingsImpl settings;
    BleScanFilterImpl filter;
    settings.SetCallbackType(2);
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    bleCentralManagerImpl->RemoveScanFilterInner(5, settings, filters);
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);
    bleCentralManagerImpl->AppendScanFilterInner(scannerId, settings, filters);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_RemoveScanFilterInner_001 end");
}

/**
 * @tc.name: ble_centralManager_AppendScanFilterInner_001
 * @tc.desc: ble_centralManager_AppendScanFilterInner_001
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_AppendScanFilterInner_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_AppendScanFilterInner_001 start");
    int32_t scannerId = 1;
    BleScanSettingsImpl settings;
    BleScanFilterImpl filter;
    settings.SetCallbackType(2);
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    bleCentralManagerImpl->AppendScanFilterInner(5, settings, filters);
    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);

    std::vector<BleScanFilterImpl> filters2;
    BleScanFilterImpl filter2;
    filters2.push_back(filter2);
    bleCentralManagerImpl->AppendScanFilterInner(scannerId, settings, filters2);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_AppendScanFilterInner_001 end");
}

/**
 * @tc.name: ble_centralManager_AppendScanFilterInner_002
 * @tc.desc: ble_centralManager_AppendScanFilterInner_002
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_AppendScanFilterInner_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_AppendScanFilterInner_002 start");
    int32_t scannerId = 1;
    BleScanSettingsImpl settings;
    BleScanFilterImpl filter;
    settings.SetCallbackType(bluetooth::ONE_MATCH_TRACK_ADV);
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    bleCentralManagerImpl->AppendScanFilterInner(scannerId, settings, filters);

    settings.SetCallbackType(bluetooth::FEW_MATCH_TRACK_ADV);
    bleCentralManagerImpl->AppendScanFilterInner(scannerId, settings, filters);

    settings.SetCallbackType(bluetooth::MAX_MATCH_TRACK_ADV);
    bleCentralManagerImpl->AppendScanFilterInner(scannerId, settings, filters);

    bleCentralManagerImpl->bthwInterface_ = nullptr;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
    bleCentralManagerImpl->ConfigScanFilter(scannerId, settings, filters);

    std::vector<BleScanFilterImpl> filters2;
    BleScanFilterImpl filter2;
    filters2.push_back(filter2);
    bleCentralManagerImpl->AppendScanFilterInner(scannerId, settings, filters2);
    EXPECT_EQ(bleCentralManagerImpl->status_, SCAN_FAILED_ALREADY_STARTED);
    HILOGI("ble_centralManager_AppendScanFilterInner_002 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_001
 * @tc.desc: 测试当 numRecords 为 0 时，函数应返回空向量
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_001, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_001 start");
    int numRecords = 0;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x0A, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11 // scanDataResp
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_TRUE(result.empty());
    HILOGI("ble_centralManager_ParseFullResults_001 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_002
 * @tc.desc: 测试当 data 为空时，函数应返回空向量
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_002, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_002 start");
    int numRecords = 5;
    std::vector<uint8_t> data = {};
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_TRUE(result.empty());
    HILOGI("ble_centralManager_ParseFullResults_002 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_003
 * @tc.desc: 测试当 advPacketLen 超过最大长度时，函数应中断循环
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_003, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_003 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, // scanDataResp
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x22, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, // scanDataResp
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11 // scanDataResp
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_EQ(result.size(), 1);
    HILOGI("ble_centralManager_ParseFullResults_003 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_004
 * @tc.desc: 测试当 scanDataRespLen 超过最大长度时，函数应中断循环
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_004, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_004 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, // scanDataResp
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x02, // advPacketLen
        0x13, 0x14, // AdvPacket
        0x2F, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, // scanDataResp
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11 // scanDataResp
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_EQ(result.size(), 1);
    HILOGI("ble_centralManager_ParseFullResults_004 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_005
 * @tc.desc: 测试当数据有效时，函数应返回正确的解析结果
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_005, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_005 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, // scanDataResp
        0x05, 0x02, 0x05, 0x04, 0x03, 0x09, // addr
        0x02, // addrType
        0x4F, // txPwr
        0xC8, // rssi
        0x10, 0x11, // timestamp
        0x09, // advPacketLen
        0x17, 0x18, 0x19, 0x20, 0x22, 0x27, 0x28, 0x29, 0x30, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x05, 0x14, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, // scanDataResp
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, // addr
        0x01, // addrType
        0x1F, // txPwr
        0x09, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30, // AdvPacket
        0x03, // scanDataRespLen
        0x08, 0x09, 0x10 // scanDataResp
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_EQ(result.size(), 3);
    HILOGI("ble_centralManager_ParseFullResults_005 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_006
 * @tc.desc: 测试数据长度不足11位时，函数应返回空向量
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_006, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_006 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F // txPwr
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_TRUE(result.empty());
    HILOGI("ble_centralManager_ParseFullResults_006 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_007
 * @tc.desc: 测试数据长度不足12位时，函数应返回空向量
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_007, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_007 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0x09, // rssi
        0x10, 0x11 // timestamp
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_TRUE(result.empty());
    HILOGI("ble_centralManager_ParseFullResults_007 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_008
 * @tc.desc: 测试当剩余数据长度不足advPacketLen时，函数应返回空向量
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_008, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_008 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0x09, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 // AdvPacket
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_TRUE(result.empty());
    HILOGI("ble_centralManager_ParseFullResults_008 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_009
 * @tc.desc: 测试当advPacketLen=0时，函数应跳过当次循环，进行下次循环
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_009, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_009 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x05, 0x04, 0x03, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0x09, // rssi
        0x10, 0x11, // timestamp
        0x00, // advPacketLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11 // scanDataResp
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_EQ(result.size(), 1);
    HILOGI("ble_centralManager_ParseFullResults_009 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_0010
 * @tc.desc: 测试当缺失scanDataRespLen时，函数应返回空向量
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_010, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_010 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x12, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x30 // AdvPacket
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_TRUE(result.empty());
    HILOGI("ble_centralManager_ParseFullResults_010 end");
}

/**
 * @tc.name: ble_centralManager_ParseFullResults_011
 * @tc.desc: 测试当剩余数据长度不足scanDataRespLen时，函数应返回空向量
 * @tc.type: FUNC
 */
HWTEST_F(BleCentralManagerImplTest, ble_centralManager_ParseFullResults_011, TestSize.Level1)
{
    HILOGI("ble_centralManager_ParseFullResults_011 start");
    int numRecords = 3;
    std::vector<uint8_t> data = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // addr
        0x01, // addrType
        0x7F, // txPwr
        0xC7, // rssi
        0x10, 0x11, // timestamp
        0x0A, // advPacketLen
        0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, // AdvPacket
        0x0B, // scanDataRespLen
        0x01, 0x02 // scanDataResp
    };
    std::vector<Bluetooth::BluetoothBleScanResult> result = ParseFullResults(numRecords, data);
    EXPECT_TRUE(result.empty());
    HILOGI("ble_centralManager_ParseFullResults_011 end");
}

}  // namespace bluetooth
}  // namespace OHOS