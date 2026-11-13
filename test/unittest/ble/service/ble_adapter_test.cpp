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

#include "bluetooth_log.h"
#include "ble_adapter.h"
#include "bluetooth_host.h"
#include "class_creator.h"
#include "hw_bt_hwif.h"
#include "bluetooth_gatt_interface.h"
#include "bluetooth_hw_interface.h"
#include "profile_service_manager.h"
#include "a2dp_def.h"
#include "hfp_ag_defines.h"
#include "ihw_interop.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_channel_rssi_manager.h"

using namespace testing;
using namespace testing::ext;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

extern "C" {
static const BthwifCallbacks* g_btAdapterhwifCallbacks = nullptr;

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

static bt_status_t BtAdapterhwifInterfaceInitMock(BthwifCallbacks* callbacks)
{
    g_btAdapterhwifCallbacks = callbacks;
    return BT_STATUS_SUCCESS;
}
static void ResetSensorhubMock(bool enableSensorhub)
{
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
static uint8_t GetSensorhubAdvHandleMock()
{
    return 1;
}
static void DeleteSensorhubFilterMock(char filterID)
{
}
static void SendHiechoDataMock(uint8_t type, uint8_t *source, uint16_t len, uint8_t *args)
{
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
static const bthwif_interface_t g_btAdapterhwifInterface = {
    .Init = BtAdapterhwifInterfaceInitMock,
    .resetSensorhub = ResetSensorhubMock,
    .setBurstParam = SetBurstParamMock,
    .setScanReportChannelSensorHub = SetScanReportChannelSensorHubMock,
    .sendParamsToSensorhub = SendParamsToSensorhubMock,
    .getSensorhubCap = GetSensorhubCapMock,
    .getSensorhubAdvHandle = GetSensorhubAdvHandleMock,
    .deleteSensorhubFilter = DeleteSensorhubFilterMock,
    .sendHiechoData = SendHiechoDataMock,
    .hwGetUnionHisiJamChrInfo = HwGetUnionHisiJamChrInfo,
    .hwGetBrTransTxRxRate = HwGetBrTransTxRxRate,
};
}
namespace OHOS {
namespace bluetooth {
class BleAdvertiserCallbackTest : public IBleAdvertiserCallback {
public:
    ~BleAdvertiserCallbackTest() override{};
    void OnStartResultEvent(int result, uint8_t advHandle, int opcode = BLE_ADV_DEFAULT_OP_CODE) override {}
    void OnEnableResultEvent(int result, uint8_t advHandle) override {}
    void OnDisableResultEvent(int result, uint8_t advHandle) override {}
    void OnStopResultEvent(int result, uint8_t advHandle) override {}
    void OnAutoStopAdvEvent(uint8_t advHandle) override {}
    void OnSetAdvDataEvent(int result, uint8_t advHandle, bluetooth::SetAdvDataType type) override {}
    void OnChangeAdvResultEvent(int result, uint8_t advHandle) override {}
};
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
class BlePeripheralCallbackTest : public IBlePeripheralCallback {
public:
    ~BlePeripheralCallbackTest() override{};
    void OnReadRemoteRssiEvent(const RawAddress &device, int rssi, int status) override {}
    void OnPairStatusChanged(BTTransport transport, const RawAddress &device, int status, int cause,
        const std::string &causeMessage) override {}
    void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) override {}
};
class AdapterBleObserverTest : public IAdapterBleObserver {
public:
    ~AdapterBleObserverTest() override{};
    void OnDiscoveryStateChanged(const int status) override {}
    void OnDiscoveryResult(const RawAddress &device, int rssi,
        const std::string deviceName, int deviceClass, int transport) override {}
    void OnPairRequested(const BTTransport transport, const RawAddress &device) override {}
    void OnPairConfirmed(
        const BTTransport transport, const RawAddress &device, const int reqType, const int number) override {}
    void OnScanModeChanged(const int mode) override {}
    void OnDeviceNameChanged(const std::string deviceName) override {}
    void OnDeviceAddrChanged(const std::string address) override {}
    void OnAdvertisingStateChanged(const int state) override {}
};
const int TIME = 2;
const int TIME_1 = 1;
static Bluetooth::BluetoothHost *host_;
class BleAdapterTest : public testing::Test {
public:
    BleAdapterTest() = default;
    ~BleAdapterTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    int scannerId_ = 1;
    BleAdapter *bleAdapter;
    BleAdvertiserCallbackTest bleAdvertiserCallbackTest;
    BleCentralManagerCallbackTest bleCentralManagerCallbackTest;
    BlePeripheralCallbackTest blePeripheralCallbackTest;
    AdapterBleObserverTest adapterBleObserverTest;
private:
};

void BleAdapterTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &Bluetooth::BluetoothHost::GetDefaultHost();

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

void BleAdapterTest::TearDownTestSuite(void)
{
    host_ = nullptr;
}

void BleAdapterTest::SetUp()
{
    bleAdapter = ClassCreator<BleAdapter>::NewInstance(ADAPTER_NAME_BLE);
    sleep(TIME_1);
}

void BleAdapterTest::TearDown()
{
    sleep(TIME_1);
    delete bleAdapter;
    bleAdapter = nullptr;
}

static constexpr int WAIT_TIMEOUT = 1100; // 1.1s

/**
 * @tc.name: ble_adapter_EnableTask_001
 * @tc.desc: ble_adapter_EnableTask_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_EnableTask_001, TestSize.Level1)
{
    HILOGI("ble_adapter_EnableTask_001 start");
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface);
    EXPECT_FALSE(bleAdapter->EnableTask());
    bleAdapter->PostEnable();
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->RegisterBlePeripheralCallback(blePeripheralCallbackTest);
    bleAdapter->RegisterBleAdapterObserver(adapterBleObserverTest);
    bleAdapter->DeregisterBleCentralManagerCallback();
    bleAdapter->DeregisterBleCentralManagerCallback();
    bleAdapter->DeregisterBlePeripheralCallback(blePeripheralCallbackTest);
    bleAdapter->DeregisterBleAdapterObserver(adapterBleObserverTest);
    uint8_t advHandle = 1;
    bleAdapter->Close(advHandle);
    bleAdapter->Close(advHandle);
    EXPECT_FALSE(bleAdapter->DisableTask());
    HILOGI("ble_adapter_EnableTask_001 end");
}

/**
 * @tc.name: ble_adapter_GetPeerDeviceAddrType_001
 * @tc.desc: ble_adapter_GetPeerDeviceAddrType_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_GetPeerDeviceAddrType_001, TestSize.Level1)
{
    HILOGI("ble_adapter_GetPeerDeviceAddrType_001 start");
    RawAddress device;
    EXPECT_EQ(bleAdapter->GetPeerDeviceAddrType(device), BLE_ADDR_TYPE_UNKNOWN);
    HILOGI("ble_adapter_GetPeerDeviceAddrType_001 end");
}

/**
 * @tc.name: ble_adapter_GetAdvertiserHandle_001
 * @tc.desc: ble_adapter_GetAdvertiserHandle_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_GetAdvertiserHandle_001, TestSize.Level1)
{
    HILOGI("ble_adapter_GetAdvertiserHandle_001 start");
    int status = -1;
    EXPECT_EQ(bleAdapter->GetAdvertiserHandle(status), BLE_INVALID_ADVERTISING_HANDLE);
    bleAdapter->UnregisterAdvertisingHandle(status);
    HILOGI("ble_adapter_GetAdvertiserHandle_001 end");
}

/**
 * @tc.name: ble_adapter_GetAdvertiserHandle_002
 * @tc.desc: ble_adapter_GetAdvertiserHandle_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_GetAdvertiserHandle_002, TestSize.Level1)
{
    HILOGI("ble_adapter_GetAdvertiserHandle_002 start");
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    int status = -1;
    EXPECT_EQ(bleAdapter->GetAdvertiserHandle(status), BLE_INVALID_ADVERTISING_HANDLE);
    bleAdapter->UnregisterAdvertisingHandle(status);
    HILOGI("ble_adapter_GetAdvertiserHandle_002 end");
}

/**
 * @tc.name: ble_adapter_StartAdvertising_001
 * @tc.desc: ble_adapter_StartAdvertising_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_StartAdvertising_001, TestSize.Level1)
{
    HILOGI("ble_adapter_StartAdvertising_001 start");
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdapter->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    EXPECT_EQ(bleAdapter->GetAdvertisingStatus(), BLE_ADV_STATE_IDLE);
    bleAdapter->OnStartAdvertisingEvt();
    HILOGI("ble_adapter_StartAdvertising_001 end");
}

/**
 * @tc.name: ble_adapter_StartAdvertising_002
 * @tc.desc: ble_adapter_StartAdvertising_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_StartAdvertising_002, TestSize.Level1)
{
    HILOGI("ble_adapter_StartAdvertising_002 start");
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    EXPECT_EQ(bleAdapter->GetAdvertisingStatus(), BLE_ADV_STATE_IDLE);
    HILOGI("ble_adapter_StartAdvertising_002 end");
}

/**
 * @tc.name: ble_adapter_StartAdvertising_003
 * @tc.desc: test adv with addr and addr type.
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_StartAdvertising_003, TestSize.Level1)
{
    HILOGI("ble_adapter_StartAdvertising_003 start");
    BleAdvertiserSettingsImpl settings;
    std::array<uint8_t, 6> addr = {0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    settings.SetOwnAddr(addr); // resolvable private address
    settings.SetOwnAddrType(1); // random address
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    EXPECT_EQ(bleAdapter->GetAdvertisingStatus(), BLE_ADV_STATE_IDLE);
    HILOGI("ble_adapter_StartAdvertising_003 end");
}

/**
 * @tc.name: ble_adapter_SetAdvertisingData_001
 * @tc.desc: ble_adapter_SetAdvertisingData_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_SetAdvertisingData_001, TestSize.Level1)
{
    HILOGI("ble_adapter_SetAdvertisingData_001 start");
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    bleAdapter->SetAdvertisingData(advData, scanResponse, advHandle);
    EXPECT_EQ(bleAdapter->GetAdvertisingStatus(), BLE_ADV_STATE_IDLE);
    HILOGI("ble_adapter_SetAdvertisingData_001 end");
}

/**
 * @tc.name: ble_adapter_SetAdvertisingData_002
 * @tc.desc: ble_adapter_SetAdvertisingData_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_SetAdvertisingData_002, TestSize.Level1)
{
    HILOGI("ble_adapter_SetAdvertisingData_002 start");
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->SetAdvertisingData(advData, scanResponse, advHandle);
    EXPECT_EQ(bleAdapter->GetAdvertisingStatus(), BLE_ADV_STATE_IDLE);
    HILOGI("ble_adapter_SetAdvertisingData_002 end");
}

/**
 * @tc.name: ble_adapter_StopAdvertising_001
 * @tc.desc: ble_adapter_StopAdvertising_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_StopAdvertising_001, TestSize.Level1)
{
    HILOGI("ble_adapter_StopAdvertising_001 start");
    uint8_t advHandle = 1;
    bleAdapter->StopAdvertising(advHandle);
    EXPECT_EQ(bleAdapter->GetAdvertisingStatus(), BLE_ADV_STATE_IDLE);
    bleAdapter->OnStopAdvertisingEvt();
    HILOGI("ble_adapter_StopAdvertising_001 end");
}

/**
 * @tc.name: ble_adapter_StopAdvertising_002
 * @tc.desc: ble_adapter_StopAdvertising_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_StopAdvertising_002, TestSize.Level1)
{
    HILOGI("ble_adapter_StopAdvertising_002 start");
    uint8_t advHandle = 1;
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->StopAdvertising(advHandle);
    EXPECT_EQ(bleAdapter->GetAdvertisingStatus(), BLE_ADV_STATE_IDLE);
    HILOGI("ble_adapter_StopAdvertising_002 end");
}

/**
 * @tc.name: ble_adapter_StartScan_001
 * @tc.desc: ble_adapter_StartScan_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_StartScan_003, TestSize.Level1)
{
    HILOGI("ble_adapter_StartScan_001 start");
    BleScanSettingsImpl setting;
    bleAdapter->StartScan(setting);
    bleAdapter->StopScan();
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->StartScan(setting);
    bleAdapter->StopScan();
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_StartScan_001 end");
}

/**
 * @tc.name: ble_adapter_ConfigScanFilter_001
 * @tc.desc: ble_adapter_ConfigScanFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_ConfigScanFilter_001, TestSize.Level1)
{
    HILOGI("ble_adapter_ConfigScanFilter_001 start");
    int32_t scannerId = 1;
    BleScanSettingsImpl settings;
    std::vector<BleScanFilterImpl> filters;
    EXPECT_EQ(bleAdapter->ConfigScanFilter(scannerId, settings, filters), 0);
    HILOGI("ble_adapter_ConfigScanFilter_001 end");
}

/**
 * @tc.name: ble_adapter_ConfigScanFilter_002
 * @tc.desc: ble_adapter_ConfigScanFilter_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_ConfigScanFilter_002, TestSize.Level1)
{
    HILOGI("ble_adapter_ConfigScanFilter_002 start");
    int32_t scannerId = 1;
    BleScanSettingsImpl settings;
    std::vector<BleScanFilterImpl> filters;
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    EXPECT_NE(bleAdapter->ConfigScanFilter(scannerId, settings, filters), 0);
    HILOGI("ble_adapter_ConfigScanFilter_002 end");
}

/**
 * @tc.name: ble_adapter_RemoveScanFilter_001
 * @tc.desc: ble_adapter_RemoveScanFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_RemoveScanFilter_001, TestSize.Level1)
{
    HILOGI("ble_adapter_RemoveScanFilter_001 start");
    int32_t scannerId = 1;
    BleScanSettingsImpl settings;
    uint16_t filtersSize = 2;
    bleAdapter->RemoveScanFilter(scannerId, settings, filtersSize);
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->RemoveScanFilter(scannerId, settings, 2);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_RemoveScanFilter_001 end");
}

/**
 * @tc.name: ble_adapter_AllocScannerId_001
 * @tc.desc: ble_adapter_AllocScannerId_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_AllocScannerId_001, TestSize.Level1)
{
    HILOGI("ble_adapter_AllocScannerId_001 start");
    EXPECT_EQ(bleAdapter->AllocScannerId(), BLE_SCAN_INVALID_ID);
    HILOGI("ble_adapter_AllocScannerId_001 end");
}

/**
 * @tc.name: ble_adapter_AllocScannerId_002
 * @tc.desc: ble_adapter_AllocScannerId_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_AllocScannerId_002, TestSize.Level1)
{
    HILOGI("ble_adapter_AllocScannerId_002 start");
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    EXPECT_EQ(bleAdapter->AllocScannerId(), BLE_SCAN_INVALID_ID);
    HILOGI("ble_adapter_AllocScannerId_002 end");
}

/**
 * @tc.name: ble_adapter_RemoveScannerId_001
 * @tc.desc: ble_adapter_RemoveScannerId_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_RemoveScannerId_001, TestSize.Level1)
{
    HILOGI("ble_adapter_RemoveScannerId_001 start");
    int32_t scannerId = 1;
    bleAdapter->RemoveScannerId(scannerId);
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->RemoveScannerId(scannerId);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_RemoveScannerId_001 end");
}

/**
 * @tc.name: ble_adapter_GetAdvertisingStatus_001
 * @tc.desc: ble_adapter_GetAdvertisingStatus_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_GetAdvertisingStatus_001, TestSize.Level1)
{
    HILOGI("ble_adapter_GetAdvertisingStatus_001 start");
    bleAdapter->GetAdvertisingStatus();
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->GetAdvertisingStatus();
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_GetAdvertisingStatus_001 end");
}

/**
 * @tc.name: ble_adapter_SetLpDeviceAdvParam_001
 * @tc.desc: ble_adapter_SetLpDeviceAdvParam_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_SetLpDeviceAdvParam_001, TestSize.Level1)
{
    HILOGI("ble_adapter_SetLpDeviceAdvParam_001 start");
    int duration = 1;
    int maxExtAdvEvents = 1;
    int window = 1;
    int interval = 1;
    int advHandle = 1;
    bleAdapter->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_SetLpDeviceAdvParam_001 end");
}

/**
 * @tc.name: ble_adapter_SetScanReportChannelToLpDevice_001
 * @tc.desc: ble_adapter_SetScanReportChannelToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_SetScanReportChannelToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_adapter_SetScanReportChannelToLpDevice_001 start");
    int32_t scannerId = 1;
    bool enable = false;
    bleAdapter->SetScanReportChannelToLpDevice(scannerId, enable);
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->SetScanReportChannelToLpDevice(scannerId, enable);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_SetScanReportChannelToLpDevice_001 end");
}

/**
 * @tc.name: ble_adapter_EnableSyncDataToLpDevice_001
 * @tc.desc: ble_adapter_EnableSyncDataToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_EnableSyncDataToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_adapter_EnableSyncDataToLpDevice_001 start");
    bleAdapter->EnableSyncDataToLpDevice();
    bleAdapter->DisableSyncDataToLpDevice();
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->EnableSyncDataToLpDevice();
    bleAdapter->DisableSyncDataToLpDevice();
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_EnableSyncDataToLpDevice_001 end");
}

/**
 * @tc.name: ble_adapter_SendParamsToLpDevice_001
 * @tc.desc: ble_adapter_SendParamsToLpDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_SendParamsToLpDevice_001, TestSize.Level1)
{
    HILOGI("ble_adapter_SendParamsToLpDevice_001 start");
    std::vector<uint8_t> dataValue;
    int32_t type = 1;
    bleAdapter->SendParamsToLpDevice(dataValue, type);
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->SendParamsToLpDevice(dataValue, type);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_SendParamsToLpDevice_001 end");
}

/**
 * @tc.name: ble_adapter_IsLpDeviceAvailable_001
 * @tc.desc: ble_adapter_IsLpDeviceAvailable_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_IsLpDeviceAvailable_001, TestSize.Level1)
{
    HILOGI("ble_adapter_IsLpDeviceAvailable_001 start");
    EXPECT_FALSE(bleAdapter->IsLpDeviceAvailable());
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    EXPECT_TRUE(bleAdapter->IsLpDeviceAvailable());
    HILOGI("ble_adapter_IsLpDeviceAvailable_001 end");
}

/**
 * @tc.name: ble_adapter_SetLpDeviceParam_001
 * @tc.desc: ble_adapter_SetLpDeviceParam_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_SetLpDeviceParam_001, TestSize.Level1)
{
    HILOGI("ble_adapter_SetLpDeviceParam_001 start");
    FilterIdxInfo filterIdx;
    BleLpDeviceParamSetImpl paramImpl;
    bleAdapter->SetLpDeviceParam(filterIdx, paramImpl);
    bleAdapter->RemoveLpDeviceParam(filterIdx);
    bleAdapter->RegisterBleCentralManagerCallback(bleCentralManagerCallbackTest);
    bleAdapter->SetLpDeviceParam(filterIdx, paramImpl);
    bleAdapter->RemoveLpDeviceParam(filterIdx);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("ble_adapter_SetLpDeviceParam_001 end");
}

HWTEST_F(BleAdapterTest, HwProfileStateCb_001, TestSize.Level1)
{
    HILOGI("HwProfileStateCb_001 start");
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(OHOS::bluetooth::RawAddress("B8:8E:82:B1:7D:DE"));
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface);
    g_btAdapterhwifCallbacks->hwProfileStateCb(
        &rawAddr, A2DP_STATUS_DISABLE_NO_SEIZE, HFP_STATUS_ENABLE, "B8:8E:82:B1:7D:DE", 0);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("HwProfileStateCb_001 end");
}

HWTEST_F(BleAdapterTest, HwProfileStateCb_002, TestSize.Level1)
{
    HILOGI("HwProfileStateCb_002 start");
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(OHOS::bluetooth::RawAddress("B8:8E:82:B1:7D:DE"));
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface);
    g_btAdapterhwifCallbacks->hwProfileStateCb(
        &rawAddr, A2DP_STATUS_ENABLE, HFP_STATUS_ENABLE, "B8:8E:82:B1:7D:DE", 0);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("HwProfileStateCb_002 end");
}

HWTEST_F(BleAdapterTest, HwProfileStateCb_003, TestSize.Level1)
{
    HILOGI("HwProfileStateCb_003 start");
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(OHOS::bluetooth::RawAddress("B8:8E:82:B1:7D:DE"));
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface);
    g_btAdapterhwifCallbacks->hwProfileStateCb(
        &rawAddr, A2DP_STATUS_DISABLE_NO_SEIZE, HFP_STATUS_DISABLE, "B8:8E:82:B1:7D:DE", 0);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("HwProfileStateCb_003 end");
}

HWTEST_F(BleAdapterTest, HwProfileStateCb_004, TestSize.Level1)
{
    HILOGI("HwProfileStateCb_004 start");
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(OHOS::bluetooth::RawAddress("B8:8E:82:B1:7D:DE"));
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface);
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    HILOGI("HwProfileStateCb_004 end");
}

HWTEST_F(BleAdapterTest, InteropMatch_001, TestSize.Level1)
{
    HILOGI("InteropMatch_001 start");
    bool ret = BluetoothHwInterface::GetInstance()->InteropMatch(
        INTEROP_DISABLE_DELAY_SEND_CALL_STATE, RawAddress("88:BD:78:00:00:00"));
    EXPECT_FALSE(ret);
    HILOGI("InteropMatch_001 end");
}

/**
 * @tc.name: ble_adapter_GetConnectedDevices_001
 * @tc.desc: ble_adapter_GetConnectedDevices_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_GetConnectedDevices_001, TestSize.Level1)
{
    HILOGI("ble_adapter_GetConnectedDevices_001 start");
    std::vector<RawAddress> pairedList = bleAdapter->GetConnectedDevices();
    EXPECT_TRUE(pairedList.empty());
    HILOGI("ble_adapter_GetConnectedDevices_001 end");
}

/**
 * @tc.name: ble_adapter_ChangeAdvertisingParams_001
 * @tc.desc: ble_adapter_ChangeAdvertisingParams_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_ChangeAdvertisingParams_001, TestSize.Level1)
{
    HILOGI("ble_adapter_ChangeAdvertisingParams_001 start");
    BleAdvertiserSettingsImpl settings;
    uint8_t advHandle = 1;
    EXPECT_EQ(bleAdapter->ChangeAdvertisingParams(advHandle, settings), Bluetooth::BT_ERR_INTERNAL_ERROR);
    HILOGI("ble_adapter_ChangeAdvertisingParams_001 end");
}

/**
 * @tc.name: ble_adapter_ChangeAdvertisingParams_002
 * @tc.desc: change adv with addr and addr type.
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_ChangeAdvertisingParams_002, TestSize.Level1)
{
    HILOGI("ble_adapter_ChangeAdvertisingParams_002 start");
    BleAdvertiserSettingsImpl settings;
    std::array<uint8_t, 6> addr = {0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    settings.SetOwnAddr(addr); // resolvable private address
    settings.SetOwnAddrType(1); // random address
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    EXPECT_EQ(bleAdapter->ChangeAdvertisingParams(advHandle, settings), Bluetooth::BT_NO_ERROR);
    HILOGI("ble_adapter_ChangeAdvertisingParams_002 end");
}

/**
 * @tc.name: ble_adapter_UpdateScanFilter_001
 * @tc.desc: ble_adapter_UpdateScanFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_UpdateScanFilter_001, TestSize.Level1)
{
    HILOGI("ble_adapter_UpdateScanFilter_001 start");
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    BleScanSettingsImpl settings;

    EXPECT_EQ(bleAdapter->UpdateScanFilter(0, settings, filters, 0), Bluetooth::BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(bleAdapter->UpdateScanFilter(0, settings, filters, 1), Bluetooth::BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(bleAdapter->UpdateScanFilter(0, settings, filters, 2), Bluetooth::BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(bleAdapter->UpdateScanFilter(0, settings, filters, 3), Bluetooth::BT_ERR_INTERNAL_ERROR);
    HILOGI("ble_adapter_UpdateScanFilter_001 end");
}

/**
 * @tc.name: ble_adapter_SetAdvOrRspData_001
 * @tc.desc: ble_adapter_SetAdvOrRspData_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_SetAdvOrRspData_001, TestSize.Level1)
{
    HILOGI("ble_adapter_SetAdvOrRspData_001 start");
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    bool isAdv = true;
    bleAdapter->SetAdvOrRspData(advData, isAdv, advHandle);
    bleAdapter->SetAdvOrRspData(scanResponse, !isAdv, advHandle);

    bleAdapter->RegisterBleAdvertiserCallback(bleAdvertiserCallbackTest);
    bleAdapter->SetAdvOrRspData(advData, isAdv, advHandle);
    bleAdapter->SetAdvOrRspData(scanResponse, !isAdv, advHandle);
    EXPECT_EQ(bleAdapter->GetAdvertisingStatus(), BLE_ADV_STATE_IDLE);
    HILOGI("ble_adapter_SetAdvOrRspData_001 end");
}
 
HWTEST_F(BleAdapterTest, ble_adapter_GetAdvPowerForRangingBusiness_001, TestSize.Level1)
{
    HILOGI("ble_adapter_GetAdvPowerForRangingBusiness_001 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
 
    int advPower = bleAdapter->GetAdvPowerForRangingBusiness(appType);
    HILOGI("GetAdvPowerForRangingBusiness result: %{public}d", advPower);
    EXPECT_TRUE(advPower >= -128 && advPower <= 127);
    HILOGI("ble_adapter_GetAdvPowerForRangingBusiness_001 end");
}
 
HWTEST_F(BleAdapterTest, ble_adapter_GetAdvPowerForRangingBusiness_002, TestSize.Level1)
{
    HILOGI("ble_adapter_GetAdvPowerForRangingBusiness_002 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_APPROACH;
 
    int advPower = bleAdapter->GetAdvPowerForRangingBusiness(appType);
    HILOGI("GetAdvPowerForRangingBusiness result: %{public}d", advPower);
    EXPECT_TRUE(advPower >= -128 && advPower <= 127);
    HILOGI("ble_adapter_GetAdvPowerForRangingBusiness_002 end");
}
 
HWTEST_F(BleAdapterTest, ble_adapter_BleRestoreRangingAntSwitch_001, TestSize.Level1)
{
    HILOGI("ble_adapter_BleRestoreRangingAntSwitch_001 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
 
    int result = bleAdapter->BleRestoreRangingAntSwitch(appType);
    HILOGI("BleRestoreRangingAntSwitch result: %{public}d", result);
    EXPECT_TRUE(result == 0 || result == -1);
    HILOGI("ble_adapter_BleRestoreRangingAntSwitch_001 end");
}

HWTEST_F(BleAdapterTest, ble_adapter_SendBleAdvMsg_001, TestSize.Level1)
{
    HILOGI("ble_adapter_SendBleAdvMsg_001 start");
    bool isStarted = true;
    EXPECT_FALSE(bleAdapter->IsBleEnabled());
    bleAdapter->SendBleAdvMsg(isStarted);
    HILOGI("ble_adapter_SendBleAdvMsg_001 end");
}

/**
 * @tc.name: ble_adapter_GetBrAddressByBleAddress_001
 * @tc.desc: GetBrAddressByBleAddress with valid BLE address
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_GetBrAddressByBleAddress_001, TestSize.Level1)
{
    HILOGI("ble_adapter_GetBrAddressByBleAddress_001 start");
    RawAddress bleAddr("1A:2B:3C:4D:5E:6F");
    RawAddress brAddr;
    bool ret = bleAdapter->GetBrAddressByBleAddress(bleAddr, brAddr);
    EXPECT_FALSE(ret);
    HILOGI("ble_adapter_GetBrAddressByBleAddress_001 end");
}

/**
 * @tc.name: ble_adapter_GetBrAddressByBleAddress_002
 * @tc.desc: GetBrAddressByBleAddress with empty BLE address
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_GetBrAddressByBleAddress_002, TestSize.Level1)
{
    HILOGI("ble_adapter_GetBrAddressByBleAddress_002 start");
    RawAddress bleAddr("");
    RawAddress brAddr;
    bool ret = bleAdapter->GetBrAddressByBleAddress(bleAddr, brAddr);
    EXPECT_FALSE(ret);
    HILOGI("ble_adapter_GetBrAddressByBleAddress_002 end");
}

/**
 * @tc.name: ble_adapter_GetBleMacByBrMac_001
 * @tc.desc: GetBleMacByBrMac with empty BR address
 * @tc.type: FUNC
 */
HWTEST_F(BleAdapterTest, ble_adapter_GetBleMacByBrMac_001, TestSize.Level1)
{
    HILOGI("ble_adapter_GetBleMacByBrMac_001 start");
    RawAddress brMac("");
    RawAddress bleMac;
    bool ret = bleAdapter->GetBleMacByBrMac(brMac, bleMac);
    EXPECT_FALSE(ret);
    HILOGI("ble_adapter_GetBleMacByBrMac_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS