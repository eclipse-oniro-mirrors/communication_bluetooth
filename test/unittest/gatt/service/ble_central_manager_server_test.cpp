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
#include <gmock/gmock.h>
#include "bluetooth_errorcode.h"
#include "bluetooth_sensorhub_collaboration.cpp"
#include "bluetooth_ble_central_manager_server.cpp"
#include "bluetooth_ble_central_manager_callback_stub.h"
#include "bluetooth_ble_central_manager_callback_proxy.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_ble_advertiser_server.h"

#include "adapter_config.h"
#include "adapter_manager.h"
#include "ble_adapter.h"
#include "class_creator.h"
#include "permission_manager.h"
#include "bluetooth_errorcode.h"
#include "btcommon/timer_manager.h"
#include "vendor_interface.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
const BluetoothRawAddress DUMMY_ADDRESS("11:22:33:44:55:66");
const int TIME = 2;
static BluetoothHost *host_;
class BluetoothBleCentralManagerCallBackStubTest : public BluetoothBleCentralManagerCallBackStub {
public:
    void OnScanCallback(const BluetoothBleScanResult &result, uint8_t callbackType) override
    {}

    void OnBleBatchScanResultsEvent(std::vector<BluetoothBleScanResult> &results) override
    {}

    void OnStartOrStopScanEvent(int resultCode, bool isStartScan) override
    {}

    void OnNotifyMsgReportFromLpDevice(const bluetooth::Uuid &uuid, int msgType,
        const std::vector<uint8_t> &notifyValue) override
    {}

    explicit BluetoothBleCentralManagerCallBackStubTest()
    {}
    ~BluetoothBleCentralManagerCallBackStubTest() override
    {}

private:
};
class BluetoothBleCentralManagerServerTest : public testing::Test {
public:
    BluetoothBleCentralManagerServerTest()
    {}
    ~BluetoothBleCentralManagerServerTest()
    {}

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    sptr<BluetoothBleCentralManagerCallBackStubTest> callback_ {nullptr};
    sptr<BluetoothBleCentralManagerCallBackStubTest> bluetoothBleCentralManagerCallBackStubTest =
        new BluetoothBleCentralManagerCallBackStubTest();
    BluetoothBleCentralManagerCallBackProxy *proxy_ = new BluetoothBleCentralManagerCallBackProxy(
        bluetoothBleCentralManagerCallBackStubTest);
    std::unique_ptr<BluetoothBleCentralManagerServer> bluetoothBleCentralManagerServer {nullptr};

    std::unique_ptr<BluetoothBleCentralManagerServer::impl::BleCentralManagerCallback>
    bleCentralManagerCallback {nullptr};
    BluetoothBleCentralManagerServer::impl implTest ;
};

static void *BleAdapterCreate(void);

void BluetoothBleCentralManagerServerTest::SetUpTestSuite(void)
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
    // 测试进程内的 AdapterManager 与设备 SA 不同进程，需显式创建 adapter 并置为开启
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);

    HILOGI("SetUpTestCase end");
}

class MockPermissionManager : public PermissionManager {
public:
    MOCK_METHOD0(GetCallingName, std::string());
};

void BluetoothBleCentralManagerServerTest::TearDownTestSuite(void)
{
    host_ = nullptr;
    utility::CloseAllTimer();
}

void BluetoothBleCentralManagerServerTest::SetUp()
{
    bluetoothBleCentralManagerServer = std::make_unique<BluetoothBleCentralManagerServer>();
    callback_ = new BluetoothBleCentralManagerCallBackStubTest();
    bleCentralManagerCallback = std::make_unique<BluetoothBleCentralManagerServer::impl
    ::BleCentralManagerCallback>(&implTest);
}

void BluetoothBleCentralManagerServerTest::TearDown()
{
    sleep(1);
    bluetoothBleCentralManagerServer = nullptr;
    callback_ = nullptr;
    bleCentralManagerCallback = nullptr;
}

static void *BleAdapterCreate(void)
{
    return new BleAdapter();
}

static void *BleAdapterCreateNull(void)
{
    return nullptr;
}

/*
 * @tc.number: BleServer_ModuleTest_Init001
 * @tc.name: BleServer_ModuleTest_Init001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_Init001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_Init001 start");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);
    wptr<BluetoothBleCentralManagerServer> wptr {nullptr};
    bluetoothBleCentralManagerServer->Init(wptr);
    wptr = new BluetoothBleCentralManagerServer();
    bluetoothBleCentralManagerServer->Init(wptr);
    EXPECT_TRUE(bluetoothBleCentralManagerServer != nullptr);
    HILOGI("BleServer_ModuleTest_Init001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_UpdateScanSettingsInner001
 * @tc.name: UpdateScanSettingsInner
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_UpdateScanSettingsInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_UpdateScanSettingsInner001 start");
    bluetoothBleCentralManagerServer->pimpl->applications = nullptr;

    int pid = 10;
    int uid = 20;
    int scannerId = 1;
    bluetooth::BleScanSettingsImpl settings;
    int ret = bluetoothBleCentralManagerServer->UpdateScanSettingsInner(pid, uid, scannerId, settings);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("BleServer_ModuleTest_UpdateScanSettingsInner001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_UpdateScanSettingsInner002
 * @tc.name: UpdateScanSettingsInner
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_UpdateScanSettingsInner002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_UpdateScanSettingsInner002 start");
    int pid = 10;
    int uid = 20;
    int scannerId = 1;
    bluetooth::BleScanSettingsImpl settingsImpl;
    bluetoothBleCentralManagerServer->pimpl->applications->SetScanIdState(pid, uid, scannerId, true);
    int ret = bluetoothBleCentralManagerServer->UpdateScanSettingsInner(pid, uid, scannerId, settingsImpl);
    std::string callingName = "com.test.callingName";
    BluetoothBleScanSettings settings;
    bluetoothBleCentralManagerServer->ReportStartScanHiSysEvent(pid, uid, callingName, settings);
    EXPECT_NE(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("BleServer_ModuleTest_UpdateScanSettingsInner002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_FreezeByRss001
 * @tc.name: BleServer_ModuleTest_FreezeByRss001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_FreezeByRss001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_FreezeByRss001 start");
    bluetoothBleCentralManagerServer->pimpl->applications = nullptr;

    std::set<int> pidSet {0};
    EXPECT_FALSE(bluetoothBleCentralManagerServer->FreezeByRss(pidSet, true, 0));
    HILOGI("BleServer_ModuleTest_FreezeByRss001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_FreezeByRss002
 * @tc.name: BleServer_ModuleTest_FreezeByRss002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_FreezeByRss002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_FreezeByRss002 start");
    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 1;
    bluetoothBleCentralManagerServer->pimpl->applications->AddRemoteObject(pid, uid, remote);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScannerId(remote, pid, uid, scannerId);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScanIdState(pid, uid, scannerId, false);

    std::set<int> pidSet {pid};
    EXPECT_TRUE(bluetoothBleCentralManagerServer->FreezeByRss(pidSet, true, 0));
    HILOGI("BleServer_ModuleTest_FreezeByRss002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_FreezeByRss003
 * @tc.name: BleServer_ModuleTest_FreezeByRss003
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_FreezeByRss003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_FreezeByRss003 start");
    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 1;
    bluetoothBleCentralManagerServer->pimpl->applications->AddRemoteObject(pid, uid, remote);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScannerId(remote, pid, uid, scannerId);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScanIdState(pid, uid, scannerId, true);

    std::set<int> pidSet {pid};
    EXPECT_TRUE(bluetoothBleCentralManagerServer->FreezeByRss(pidSet, true, 0));
    HILOGI("BleServer_ModuleTest_FreezeByRss003 end");
}

/*
 * @tc.number: BleServer_ModuleTest_FreezeByRss004
 * @tc.name: BleServer_ModuleTest_FreezeByRss004
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_FreezeByRss004, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_FreezeByRss004 start");

    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);

    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 1;
    bluetoothBleCentralManagerServer->pimpl->applications->AddRemoteObject(pid, uid, remote);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScannerId(remote, pid, uid, scannerId);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScanIdState(pid, uid, scannerId, false);

    std::set<int> pidSet {pid};
    EXPECT_TRUE(bluetoothBleCentralManagerServer->FreezeByRss(pidSet, false, 0));

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);
    HILOGI("BleServer_ModuleTest_FreezeByRss004 end");
}

/*
 * @tc.number: BleServer_ModuleTest_FreezeByRss005
 * @tc.name: BleServer_ModuleTest_FreezeByRss005
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_FreezeByRss005, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_FreezeByRss005 start");

    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);

    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 1;
    bluetoothBleCentralManagerServer->pimpl->applications->AddRemoteObject(pid, uid, remote);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScannerId(remote, pid, uid, scannerId);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScanIdState(pid, uid, scannerId, true);
    bluetoothBleCentralManagerServer->pimpl->applications->SetFreezeState(pid, uid, scannerId, true);

    std::set<int> pidSet {pid};
    EXPECT_TRUE(bluetoothBleCentralManagerServer->FreezeByRss(pidSet, false, 0));

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);
    HILOGI("BleServer_ModuleTest_FreezeByRss005 end");
}

/*
 * @tc.number: BleServer_ModuleTest_DumpRssFreezeInfo001
 * @tc.name: BleServer_ModuleTest_DumpRssFreezeInfo001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_DumpRssFreezeInfo001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_DumpRssFreezeInfo001 start");
    EXPECT_NE(bluetoothBleCentralManagerServer->DumpRssFreezeInfo(), "");
    HILOGI("BleServer_ModuleTest_DumpRssFreezeInfo001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScan001
 * @tc.name: BleServer_ModuleTest_StartScan001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScan001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScan001 start");
    BluetoothBleScanSettings settings;
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter;
    BluetoothBleScanFilter filter1;
    bluetooth::Uuid uuid;
    filter1.SetServiceUuid(uuid);
    filter1.SetServiceUuidMask(uuid);
    filter1.SetServiceSolicitationUuid(uuid);
    filter1.SetServiceSolicitationUuidMask(uuid);
    filters.push_back(filter);
    filters.push_back(filter1);
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(0, settings, filters, false), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StartScan001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScan002
 * @tc.name: BleServer_ModuleTest_StartScan002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScan002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScan002 start");
    BluetoothBleScanSettings settings;
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    BluetoothBleScanFilter filter3;
    BluetoothBleScanFilter filter4;
    filters.push_back(filter1);
    filters.push_back(filter2);
    filters.push_back(filter3);
    filters.push_back(filter4);
    MockPermissionManager mockPermissionManager;
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(0, settings, filters, false), BT_ERR_BLE_SCAN_MAX_FILTER);
    HILOGI("BleServer_ModuleTest_StartScan002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_ConfigScanFilterInner001
 * @tc.name: BleServer_ModuleTest_ConfigScanFilterInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ConfigScanFilterInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ConfigScanFilterInner001 start");
    int scannerId = 1;
    BleScanSettingsImpl setting;
    setting.SetCallbackType(4);
    setting.SetMatchTrackAdvType(1);
    setting.SetMatchMode(2);
    std::vector<BluetoothBleScanFilter> filters;
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);
    EXPECT_EQ(bluetoothBleCentralManagerServer->ConfigScanFilterInner(scannerId, setting, filters), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_ConfigScanFilterInner001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_ConfigScanFilterInner002
 * @tc.name: BleServer_ModuleTest_ConfigScanFilterInner002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ConfigScanFilterInner002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ConfigScanFilterInner002 start");
    int scannerId = 1;
    BleScanSettingsImpl setting;
    setting.SetCallbackType(1);
    setting.SetMatchTrackAdvType(1);
    setting.SetMatchMode(2);
    std::vector<BluetoothBleScanFilter> filters;
    for (int i = 0; i < 10; i++) {
        BluetoothBleScanFilter filter;
        filters.push_back(filter);
    }
    EXPECT_NE(bluetoothBleCentralManagerServer->ConfigScanFilterInner(scannerId, setting, filters), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_ConfigScanFilterInner002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StopScan001
 * @tc.name: BleServer_ModuleTest_StopScan001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StopScan001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StopScan001 start");
    EXPECT_EQ(bluetoothBleCentralManagerServer->StopScan(0), BT_NO_ERROR);
    bluetoothBleCentralManagerServer->RemoveScanFilter(0);
    HILOGI("BleServer_ModuleTest_StopScan001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_RegisterBleCentralManagerCallback001
 * @tc.name: BleServer_ModuleTest_RegisterBleCentralManagerCallback001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, RegisterBleCentralManagerCallback001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_RegisterBleCentralManagerCallback001 start");
    sptr<BluetoothBleCentralManagerCallBackStubTest> callback {nullptr};
    int32_t scannerId = 0;
    bluetoothBleCentralManagerServer->RegisterBleCentralManagerCallback(scannerId, false, callback_);
    bluetoothBleCentralManagerServer->RegisterBleCentralManagerCallback(scannerId, false, callback);

    int32_t newScannerId = 0;
    bluetoothBleCentralManagerServer->RegisterBleCentralManagerCallback(newScannerId, false, callback_);
    EXPECT_TRUE(bluetoothBleCentralManagerServer != nullptr);
    HILOGI("BleServer_ModuleTest_RegisterBleCentralManagerCallback001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_DeregisterBleCentralManagerCallback001
 * @tc.name: BleServer_ModuleTest_DeregisterBleCentralManagerCallback001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, DeregisterBleCentralManagerCallback001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_DeregisterBleCentralManagerCallback001 start");
    sptr<BluetoothBleCentralManagerCallBackStubTest> callback {nullptr};
    bluetoothBleCentralManagerServer->DeregisterBleCentralManagerCallback(0, callback_);
    bluetoothBleCentralManagerServer->DeregisterBleCentralManagerCallback(0, callback);
    EXPECT_TRUE(bluetoothBleCentralManagerServer != nullptr);
    HILOGI("BleServer_ModuleTest_DeregisterBleCentralManagerCallback001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_LpDevice001
 * @tc.name: LpDevice
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_LpDevice001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_LpDevice001 start");
    EXPECT_EQ(bluetoothBleCentralManagerServer->EnableSyncDataToLpDevice(), BT_NO_ERROR);
    BluetoothLpDeviceParamSet paramSet;
    paramSet.fieldValidFlagBit = 0;
    EXPECT_EQ(bluetoothBleCentralManagerServer->SetLpDeviceParam(paramSet), BT_NO_ERROR);
    paramSet.fieldValidFlagBit = 0x1F;
    EXPECT_EQ(bluetoothBleCentralManagerServer->SetLpDeviceParam(paramSet), BT_NO_ERROR);
    paramSet.fieldValidFlagBit = 0x3F;
    EXPECT_EQ(bluetoothBleCentralManagerServer->SetLpDeviceParam(paramSet), BT_NO_ERROR);
    EXPECT_EQ(bluetoothBleCentralManagerServer->SetLpDeviceAdvParam(1, 1, 1, 1, 1), BT_NO_ERROR);
    EXPECT_EQ(bluetoothBleCentralManagerServer->SetScanReportChannelToLpDevice(1, false), BT_NO_ERROR);
    std::vector<uint8_t> dataValue;
    int32_t type = 1;
    EXPECT_EQ(bluetoothBleCentralManagerServer->SendParamsToLpDevice(dataValue, type), BT_NO_ERROR);
    EXPECT_FALSE(bluetoothBleCentralManagerServer->IsLpDeviceAvailable());
    bluetooth::Uuid uuid;
    EXPECT_EQ(bluetoothBleCentralManagerServer->RemoveLpDeviceParam(uuid), BT_NO_ERROR);
    EXPECT_EQ(bluetoothBleCentralManagerServer->DisableSyncDataToLpDevice(), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_LpDevice001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_ConvertToBleScanFilterImplInner_001
 * @tc.name: BleServer_ModuleTest_ConvertToBleScanFilterImplInner_001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ConvertToBleScanFilterImplInner_001,
    TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ConvertToBleScanFilterImplInner_001 start");
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    BluetoothBleScanFilter filter3;
    BluetoothBleScanFilter filter4;
    filters.push_back(filter1);
    filters.push_back(filter2);
    filters.push_back(filter3);
    filters.push_back(filter4);
    MockPermissionManager mockPermissionManager;
    std::vector<BleScanFilterImpl> filterImpl =
        bluetoothBleCentralManagerServer->pimpl->ConvertToBleScanFilterImpl(filters, true);
    EXPECT_EQ(filterImpl.size(), filters.size());
    HILOGI("BleServer_ModuleTest_ConvertToBleScanFilterImplInner_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_UpdateScanFilterInner_001
 * @tc.name: BleServer_ModuleTest_UpdateScanFilterInner_001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_UpdateScanFilterInner_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_UpdateScanFilterInner_001 start");
    BleScanSettingsImpl settings;
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    BluetoothBleScanFilter filter3;
    BluetoothBleScanFilter filter4;
    filters.push_back(filter1);
    filters.push_back(filter2);
    filters.push_back(filter3);
    filters.push_back(filter4);
    MockPermissionManager mockPermissionManager;
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 0), BT_NO_ERROR);
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 1), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 3), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 2), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_UpdateScanFilterInner_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_UpdateScanFilterInner_002
 * @tc.name: BleServer_ModuleTest_UpdateScanFilterInner_002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_UpdateScanFilterInner_002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_UpdateScanFilterInner_002 start");
    BleScanSettingsImpl settings;
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    BluetoothBleScanFilter filter3;
    BluetoothBleScanFilter filter4;
    filters.push_back(filter1);
    filters.push_back(filter2);
    filters.push_back(filter3);
    filters.push_back(filter4);
    MockPermissionManager mockPermissionManager;
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 0), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 1), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 2), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 1), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->UpdateScanFilterInner(0, settings, filters, 3), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_UpdateScanFilterInner_002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_UpdateScanFilterInner_003
 * @tc.name: BleServer_ModuleTest_UpdateScanFilterInner_003
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_UpdateScanFilterInner_003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_UpdateScanFilterInner_003 start");

    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);

    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 1;
    bluetoothBleCentralManagerServer->pimpl->applications->AddRemoteObject(pid, uid, remote);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScannerId(remote, pid, uid, scannerId);
    bluetoothBleCentralManagerServer->pimpl->applications->SetScanIdState(pid, uid, scannerId, true);
    EXPECT_TRUE(bluetoothBleCentralManagerServer->pimpl->applications->GetScanIdState(pid, uid, scannerId));

    std::vector<BleScanFilterImpl> filters;
    BleScanFilterImpl filter1;
    filter1.SetFiltIndex(0);
    BleScanFilterImpl filter2;
    filter1.SetFiltIndex(1);
    filters.push_back(filter1);
    filters.push_back(filter2);
    bluetoothBleCentralManagerServer->pimpl->applications->AppendBleScanFilter(scannerId, filters);
    bluetoothBleCentralManagerServer->pimpl->applications->RemoveBleScanFilter(scannerId, filters);
    HILOGI("BleServer_ModuleTest_UpdateScanFilterInner_003 end");
}

/*
 * @tc.number: BleServer_ModuleTest_ChangeScanParams_001
 * @tc.name: BleServer_ModuleTest_ChangeScanParams_001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ChangeScanParams_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ChangeScanParams_001 start");
    BluetoothBleScanSettings settings;
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter;
    BluetoothBleScanFilter filter1;
    bluetooth::Uuid uuid;
    filter1.SetServiceUuid(uuid);
    filter1.SetServiceUuidMask(uuid);
    filter1.SetServiceSolicitationUuid(uuid);
    filter1.SetServiceSolicitationUuidMask(uuid);
    filters.push_back(filter);
    filters.push_back(filter1);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 0), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 1), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 3), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 2), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_ChangeScanParams_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_ChangeScanParams_002
 * @tc.name: BleServer_ModuleTest_ChangeScanParams_002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ChangeScanParams_002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ChangeScanParams_002 start");
    BluetoothBleScanSettings settings;
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    BluetoothBleScanFilter filter3;
    BluetoothBleScanFilter filter4;
    filters.push_back(filter1);
    filters.push_back(filter2);
    filters.push_back(filter3);
    filters.push_back(filter4);
    MockPermissionManager mockPermissionManager;
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 0), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 1), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 3), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 2), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_ChangeScanParams_002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_ChangeScanParams_003
 * @tc.name: BleServer_ModuleTest_ChangeScanParams_003
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ChangeScanParams_003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ChangeScanParams_003 start");
    BluetoothBleScanSettings settings;
    std::vector<BluetoothBleScanFilter> filters;
    MockPermissionManager mockPermissionManager;
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 0), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 1), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 3), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParams(0, settings, filters, 2), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_ChangeScanParams_003 end");
}

/*
 * @tc.number: BleServer_ModuleTest_IsValidScannerId_001
 * @tc.name: BleServer_ModuleTest_IsValidScannerId_001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_IsValidScannerId_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_IsValidScannerId_001 start");
    BluetoothBleScanSettings settings;
    std::vector<BluetoothBleScanFilter> filters;
    MockPermissionManager mockPermissionManager;
    bool isValid = false;
    bluetoothBleCentralManagerServer->IsValidScannerId(10, isValid);
    EXPECT_EQ(isValid, false);
    HILOGI("BleServer_ModuleTest_IsValidScannerId_001 end");
}

/*
 * @tc.number: BleCentralManagerServer_enable001
 * @tc.name: BleCentralManagerServer_enable001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleCentralManagerServer_enable001, TestSize.Level1)
{
    HILOGI("BleCentralManagerServer_enable001 start");

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);

    std::set<int> pidSet {0};
    bluetoothBleCentralManagerServer->FreezeByRss(pidSet, false, 0);
    bluetoothBleCentralManagerServer->FreezeByRss(pidSet, true, 0);
    bluetoothBleCentralManagerServer->FreezeByRss(pidSet, true, 1);
    bluetoothBleCentralManagerServer->FreezeByRss(pidSet, true, 0);
    bluetoothBleCentralManagerServer->FreezeByRss(pidSet, false, 0);
    bluetoothBleCentralManagerServer->FreezeByRss(pidSet, true, 1);
    bluetoothBleCentralManagerServer->FreezeByRss(pidSet, false, 0);
    bluetoothBleCentralManagerServer->FreezeByRss(pidSet, true, 0);

    BluetoothBleScanSettings settings;
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter;
    BluetoothBleScanFilter filter1;
    bluetooth::Uuid uuid;
    filter1.SetServiceUuid(uuid);
    filter1.SetServiceUuidMask(uuid);
    filter1.SetServiceSolicitationUuid(uuid);
    filter1.SetServiceSolicitationUuidMask(uuid);
    filters.push_back(filter);
    filters.push_back(filter1);

    bluetoothBleCentralManagerServer->StartScan(0, settings, filters, false);
    bluetoothBleCentralManagerServer->StopScan(0);
    bluetoothBleCentralManagerServer->RemoveScanFilter(0);

    sptr<BluetoothBleCentralManagerCallBackStubTest> callback {nullptr};
    int32_t scannerId = 0;
    bluetoothBleCentralManagerServer->RegisterBleCentralManagerCallback(scannerId, false, callback_);
    bluetoothBleCentralManagerServer->RegisterBleCentralManagerCallback(scannerId, false, callback);

    bluetoothBleCentralManagerServer->DeregisterBleCentralManagerCallback(0, callback_);
    bluetoothBleCentralManagerServer->DeregisterBleCentralManagerCallback(0, callback);

    bluetoothBleCentralManagerServer->SetLpDeviceAdvParam(1, 1, 1, 1, 1);

    bluetoothBleCentralManagerServer->SetScanReportChannelToLpDevice(1, false);

    EXPECT_EQ(bluetoothBleCentralManagerServer->EnableSyncDataToLpDevice(), BT_NO_ERROR);

    EXPECT_EQ(bluetoothBleCentralManagerServer->DisableSyncDataToLpDevice(), BT_NO_ERROR);

    std::vector<uint8_t> dataValue;
    int32_t type = 1;
    bluetoothBleCentralManagerServer->SendParamsToLpDevice(dataValue, type);

    bluetoothBleCentralManagerServer->IsLpDeviceAvailable();

    BluetoothLpDeviceParamSet paramSet;
    paramSet.fieldValidFlagBit = 0;
    bluetoothBleCentralManagerServer->SetLpDeviceParam(paramSet);

    paramSet.fieldValidFlagBit = 0x1F;
    bluetoothBleCentralManagerServer->SetLpDeviceParam(paramSet);

    paramSet.fieldValidFlagBit = 0x3F;
    bluetoothBleCentralManagerServer->SetLpDeviceParam(paramSet);

    bluetoothBleCentralManagerServer->RemoveLpDeviceParam(uuid);

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);

    HILOGI("BleCentralManagerServer_enable001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_OnRemoteRequest001
 * @tc.name: BleServer_ModuleTest_OnRemoteRequest001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_OnRemoteRequest001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnRemoteRequest001 start");
    uint32_t code = 17;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothBleCentralManagerServer::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    bluetoothBleCentralManagerServer->OnRemoteRequest(code, data, reply, option);
    code = 0;
    bluetoothBleCentralManagerServer->OnRemoteRequest(code, data, reply, option);
    int ret = bluetoothBleCentralManagerServer->RegisterBleCentralManagerCallbackInner(data, reply);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_OnRemoteRequest001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScanInner001
 * @tc.name: BleServer_ModuleTest_StartScanInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScanInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScanInner001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = bluetoothBleCentralManagerServer->BluetoothBleCentralManagerStub::StartScanInner(data, reply);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StartScanInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScanInner002
 * @tc.name: BleServer_ModuleTest_StartScanInner002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScanInner002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScanInner002 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = bluetoothBleCentralManagerServer->BluetoothBleCentralManagerStub::StartScanInner(data, reply);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StartScanInner002 start");
}

/*
 * @tc.number: BleServer_ModuleTest_StopScanInner001
 * @tc.name: BleServer_ModuleTest_StopScanInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StopScanInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StopScanInner001 start");
    MessageParcel data;
    MessageParcel reply;
    int ret = bluetoothBleCentralManagerServer->BluetoothBleCentralManagerStub::StopScanInner(data, reply);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StopScanInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_FreezeByRssInner001
 * @tc.name: BleServer_ModuleTest_FreezeByRssInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_FreezeByRssInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_FreezeByRssInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleCentralManagerServer->FreezeByRssInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_FreezeByRssInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_ResetAllProxyInner001
 * @tc.name: BleServer_ModuleTest_ResetAllProxyInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ResetAllProxyInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ResetAllProxyInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleCentralManagerServer->ResetAllProxyInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_ResetAllProxyInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceAdvParamInner001
 * @tc.name: BleServer_ModuleTest_SetLpDeviceAdvParamInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceAdvParamInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceAdvParamInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleCentralManagerServer->SetLpDeviceAdvParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceAdvParamInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetScanReportChannelToLpDeviceInner001
 * @tc.name: BleServer_ModuleTest_SetScanReportChannelToLpDeviceInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, SetScanReportChannelToLpDeviceInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetScanReportChannelToLpDeviceInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleCentralManagerServer->SetScanReportChannelToLpDeviceInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetScanReportChannelToLpDeviceInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_EnableSyncDataToLpDeviceInner001
 * @tc.name: BleServer_ModuleTest_EnableSyncDataToLpDeviceInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_EnableSyncDataToLpDeviceInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_EnableSyncDataToLpDeviceInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleCentralManagerServer->EnableSyncDataToLpDeviceInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_EnableSyncDataToLpDeviceInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_DisableSyncDataToLpDeviceInner001
 * @tc.name: BleServer_ModuleTest_DisableSyncDataToLpDeviceInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_DisableSyncDataToLpDeviceInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_DisableSyncDataToLpDeviceInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleCentralManagerServer->DisableSyncDataToLpDeviceInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_DisableSyncDataToLpDeviceInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SendParamsToLpDeviceInner001
 * @tc.name: BleServer_ModuleTest_SendParamsToLpDeviceInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SendParamsToLpDeviceInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SendParamsToLpDeviceInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleCentralManagerServer->SendParamsToLpDeviceInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SendParamsToLpDeviceInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_IsLpDeviceAvailableInner001
 * @tc.name: BleServer_ModuleTest_IsLpDeviceAvailableInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_IsLpDeviceAvailableInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_IsLpDeviceAvailableInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothBleCentralManagerServer->IsLpDeviceAvailableInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_IsLpDeviceAvailableInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner001
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner001 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x00);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner002
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner002 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x1F);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner002 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner003
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner003
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner003 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x1F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner003 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner004
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner004
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner004, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner004 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x1F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x01);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner004 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner005
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner005
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner005, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner005 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x1F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x101);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner005 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner006
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner006
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner006, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner006 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x1F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x01);
    BluetoothBleScanFilter bluetoothBleScanFilter;
    data.WriteParcelable(&bluetoothBleScanFilter);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner006 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner007
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner007
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner007, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner007 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x1F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x01);
    BluetoothBleScanFilter bluetoothBleScanFilter;
    data.WriteParcelable(&bluetoothBleScanFilter);
    BluetoothBleAdvertiserSettings bluetoothBleAdvertiserSettings;
    data.WriteParcelable(&bluetoothBleAdvertiserSettings);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner007 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner008
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner008
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner008, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner008 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x1F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x01);
    BluetoothBleScanFilter bluetoothBleScanFilter;
    data.WriteParcelable(&bluetoothBleScanFilter);
    BluetoothBleAdvertiserSettings bluetoothBleAdvertiserSettings;
    data.WriteParcelable(&bluetoothBleAdvertiserSettings);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    data.WriteParcelable(&bluetoothBleAdvertiserData);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner008 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner009
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner009
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner009, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner009 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x1F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x01);
    BluetoothBleScanFilter bluetoothBleScanFilter;
    data.WriteParcelable(&bluetoothBleScanFilter);
    BluetoothBleAdvertiserSettings bluetoothBleAdvertiserSettings;
    data.WriteParcelable(&bluetoothBleAdvertiserSettings);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    data.WriteParcelable(&bluetoothBleAdvertiserData);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData1;
    data.WriteParcelable(&bluetoothBleAdvertiserData1);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner009 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner010
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner010
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner010, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner010 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x3F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x01);
    BluetoothBleScanFilter bluetoothBleScanFilter;
    data.WriteParcelable(&bluetoothBleScanFilter);
    BluetoothBleAdvertiserSettings bluetoothBleAdvertiserSettings;
    data.WriteParcelable(&bluetoothBleAdvertiserSettings);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    data.WriteParcelable(&bluetoothBleAdvertiserData);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData1;
    data.WriteParcelable(&bluetoothBleAdvertiserData1);
    data.WriteInt32(0x101);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner010 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner011
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner011
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner011, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner010 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x3F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x01);
    BluetoothBleScanFilter bluetoothBleScanFilter;
    data.WriteParcelable(&bluetoothBleScanFilter);
    BluetoothBleAdvertiserSettings bluetoothBleAdvertiserSettings;
    data.WriteParcelable(&bluetoothBleAdvertiserSettings);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    data.WriteParcelable(&bluetoothBleAdvertiserData);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData1;
    data.WriteParcelable(&bluetoothBleAdvertiserData1);
    data.WriteInt32(0x01);
    EXPECT_NE(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner011 start");
}

/*
 * @tc.number: BleServer_ModuleTest_SetLpDeviceParamInner012
 * @tc.name: BleServer_ModuleTest_SetLpDeviceParamInner012
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetLpDeviceParamInner012, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner012 start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0x3F);
    BluetoothBleScanSettings bluetoothBleScanSettings;
    data.WriteParcelable(&bluetoothBleScanSettings);
    data.WriteInt32(0x01);
    BluetoothBleScanFilter bluetoothBleScanFilter;
    data.WriteParcelable(&bluetoothBleScanFilter);
    BluetoothBleAdvertiserSettings bluetoothBleAdvertiserSettings;
    data.WriteParcelable(&bluetoothBleAdvertiserSettings);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData;
    data.WriteParcelable(&bluetoothBleAdvertiserData);
    BluetoothBleAdvertiserData bluetoothBleAdvertiserData1;
    data.WriteParcelable(&bluetoothBleAdvertiserData1);
    data.WriteInt32(0x01);
    BluetoothUuid bluetoothUuid;
    data.WriteParcelable(&bluetoothUuid);
    EXPECT_EQ(bluetoothBleCentralManagerServer->SetLpDeviceParamInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetLpDeviceParamInner012 start");
}

/*
 * @tc.number: BleServer_ModuleTest_RemoveLpDeviceParamInner001
 * @tc.name: BleServer_ModuleTest_RemoveLpDeviceParamInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_RemoveLpDeviceParamInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_RemoveLpDeviceParamInner001 start");
    BluetoothUuid bluetoothUuid;
    MessageParcel data;
    MessageParcel data1;
    MessageParcel reply;
    data1.WriteParcelable(&bluetoothUuid);
    EXPECT_NE(bluetoothBleCentralManagerServer->RemoveLpDeviceParamInner(data, reply), BT_NO_ERROR);
    EXPECT_EQ(bluetoothBleCentralManagerServer->RemoveLpDeviceParamInner(data1, reply), BT_NO_ERROR);
    sptr<BluetoothBleCentralManagerCallBackStubTest> callback = new BluetoothBleCentralManagerCallBackStubTest();
    wptr<BluetoothBleCentralManagerServer> wptr = new BluetoothBleCentralManagerServer();
    bluetoothBleCentralManagerServer->Init(wptr);
    bluetoothBleCentralManagerServer->DeregisterBleCentralManagerCallback(0, callback);
    bluetoothBleCentralManagerServer->pimpl->applications = nullptr;
    bluetoothBleCentralManagerServer->ReportStartedOrStoppedEvent(10, 10, 1, 0, true);
    bluetoothBleCentralManagerServer->ReportStartedOrStoppedEvent(10, 10, 1, 0, false);
    HILOGI("BleServer_ModuleTest_RemoveLpDeviceParamInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_ChangeScanParamsInner001
 * @tc.name: BleServer_ModuleTest_ChangeScanParamsInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ChangeScanParamsInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ChangeScanParamsInner001 start");
    BluetoothUuid bluetoothUuid;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    BluetoothBleScanSettings settings;
    data.WriteParcelable(&settings);
    data.WriteInt32(0x200);
    EXPECT_NE(bluetoothBleCentralManagerServer->ChangeScanParamsInner(data, reply), BT_NO_ERROR);
    MessageParcel data1;
    data1.WriteInt32(1);
    data1.WriteParcelable(&settings);
    data1.WriteInt32(1);
    BluetoothBleScanFilter filter;
    data1.WriteParcelable(&filter);
    data1.WriteInt32(1);
    EXPECT_EQ(bluetoothBleCentralManagerServer->ChangeScanParamsInner(data1, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_ChangeScanParamsInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_RegisterBleCentralManagerCallback_001
 * @tc.name: BleServer_ModuleTest_RegisterBleCentralManagerCallback_001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_RegisterBleCentralManagerCallback001, TestSize.Level1)
{
    HILOGI("BleServer_RegisterBleCentralManagerCallback001 start");
    for (int i = 0; i <= 1001; i++) {
        sptr<BluetoothBleCentralManagerCallBackStubTest> callback;
        wptr<BluetoothBleCentralManagerServer> wptr;
        bluetoothBleCentralManagerServer->Init(wptr);
        bool enableRandomAddrMode = true;
        int32_t deviceId = 0;
        bluetoothBleCentralManagerServer->RegisterBleCentralManagerCallback(deviceId, enableRandomAddrMode, callback);
    }
    EXPECT_TRUE(bluetoothBleCentralManagerServer != nullptr);
    HILOGI("BleServer_RegisterBleCentralManagerCallback001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_OnBleBatchScanResultsEvent001
 * @tc.name: BleServer_ModuleTest_OnBleBatchScanResultsEvent001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_OnBleBatchScanResultsEvent001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnBleBatchScanResultsEvent001 start");
    std::vector<BluetoothBleScanResult> results;
    BluetoothBleScanResult result;
    results.push_back(result);
    proxy_->OnScanCallback(result, 1);
    proxy_->OnBleBatchScanResultsEvent(results);
    proxy_->OnStartOrStopScanEvent(1, false);
    bluetooth::Uuid uuid;
    std::vector<uint8_t> notifyValue;
    proxy_->OnNotifyMsgReportFromLpDevice(uuid, 1, notifyValue);
    EXPECT_TRUE(proxy_ != nullptr);
    HILOGI("BleServer_ModuleTest_OnBleBatchScanResultsEvent001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnScanCallback002
 * @tc.name: BleServer_ModuleTest_OnScanCallback002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_OnScanCallback002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnScanCallback002 start");
    BleScanResultImpl result;
    bleCentralManagerCallback->pimpl_->observers_.Register(callback_);
    bleCentralManagerCallback->pimpl_->applications = nullptr;
    bleCentralManagerCallback->SetObserver(&(bleCentralManagerCallback->pimpl_->observers_));
    bleCentralManagerCallback->OnScanCallback(result);
    EXPECT_EQ(ConvertToScannerStateMachineEvent(0, false), 5);
    HILOGI("BleServer_ModuleTest_OnScanCallback002 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnBleBatchScanResultsEvent002
 * @tc.name: BleServer_ModuleTest_OnBleBatchScanResultsEvent002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_OnBleBatchScanResultsEvent002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnBleBatchScanResultsEvent002 start");
    BluetoothBleScanResult result;
    std::vector<BluetoothBleScanResult> results;
    results.push_back(result);
    bleCentralManagerCallback->pimpl_->observers_.Register(callback_);
    bleCentralManagerCallback->pimpl_->applications = nullptr;
    bleCentralManagerCallback->SetObserver(&(bleCentralManagerCallback->pimpl_->observers_));
    int fakeClientIf = 0;
    bleCentralManagerCallback->OnBleBatchScanResultsEvent(fakeClientIf, results);
    EXPECT_EQ(ConvertToScannerStateMachineEvent(1, false), 6);
    HILOGI("BleServer_ModuleTest_OnBleBatchScanResultsEvent002 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnStartOrStopScanEvent002
 * @tc.name: BleServer_ModuleTest_OnStartOrStopScanEvent002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_OnStartOrStopScanEvent002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnStartOrStopScanEvent002 start");
    bleCentralManagerCallback->pimpl_->observers_.Register(callback_);
    bleCentralManagerCallback->SetObserver(&(bleCentralManagerCallback->pimpl_->observers_));
    bleCentralManagerCallback->OnStartOrStopScanEvent(1, false);
    bleCentralManagerCallback->pimpl_ = nullptr;
    bleCentralManagerCallback->OnStartOrStopScanEvent(1, false);
    EXPECT_EQ(ConvertToScannerStateMachineEvent(0, true), 3);
    HILOGI("BleServer_ModuleTest_OnStartOrStopScanEvent002 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnNotifyMsgReportFromLpDevice002
 * @tc.name: BleServer_ModuleTest_OnNotifyMsgReportFromLpDevice002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_OnNotifyMsgReportFromLpDevice002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnNotifyMsgReportFromLpDevice002 start");
    FilterIdxInfo info;
    uint8_t msgType = 1;
    std::vector<uint8_t> notifyValue;
    bleCentralManagerCallback->pimpl_->observers_.Register(callback_);
    bleCentralManagerCallback->pimpl_->applications = nullptr;
    bleCentralManagerCallback->SetObserver(&(bleCentralManagerCallback->pimpl_->observers_));
    bleCentralManagerCallback->OnNotifyMsgReportFromLpDevice(info, msgType, notifyValue);
    EXPECT_EQ(ConvertToScannerStateMachineEvent(1, true), 4);
    HILOGI("BleServer_ModuleTest_OnNotifyMsgReportFromLpDevice002 start");
}

/*
 * @tc.number: BleServer_ModuleTest_OnFoundOrLostCallback001
 * @tc.name: BleServer_ModuleTest_OnFoundOrLostCallback001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_OnFoundOrLostCallback001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnFoundOrLostCallback001 start");
    int scannerId = 1;
    int advertiserState = 0;
    BleScanResultImpl result;
    bleCentralManagerCallback->OnFoundOrLostCallback(scannerId, advertiserState, result);
    EXPECT_EQ(ConvertToScannerStateMachineEvent(1, true), 4);
    HILOGI("BleServer_ModuleTest_OnFoundOrLostCallback001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_OnStateChange001
 * @tc.name: BleServer_ModuleTest_OnStateChange001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_OnStateChange001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_OnStateChange001 start");
    auto resetBatchScanCb = [this](int32_t pid, int32_t uid, int32_t scannerId) -> bool {
        return true;
    };
    std::shared_ptr<BleScannerStateMachine> sm1 = std::make_shared<BleScannerStateMachine>();
    std::shared_ptr<BluetoothBleScannerApplicationContainer> applications1 =
    std::make_shared<BluetoothBleScannerApplicationContainer>(sm1, resetBatchScanCb);
    std::weak_ptr<BleScannerStateMachine> sm = sm1;
    std::weak_ptr<BluetoothBleScannerApplicationContainer> applications = applications1;
    BluetoothBleCentralManagerServer::impl::AdapterStateObserver adapterStateObserver(applications, sm, nullptr);
    bluetooth::BTTransport transport = bluetooth::BTTransport::ADAPTER_BREDR;
    bluetooth::BTStateID state = bluetooth::BTStateID::STATE_TURNING_ON;
    adapterStateObserver.OnStateChange(transport, state);
    adapterStateObserver.OnStateChange(bluetooth::BTTransport::ADAPTER_BLE, state);
    adapterStateObserver.OnStateChange(bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_OFF);
    adapterStateObserver.OnStateChange(transport, bluetooth::BTStateID::STATE_TURN_OFF);
    adapterStateObserver.applications_.reset();
    adapterStateObserver.OnStateChange(transport, state);
    adapterStateObserver.OnStateChange(transport, state);
    int32_t subErrCode = 0;
    applications1->ChrReportScanFilter(subErrCode);
    HILOGI("BleServer_ModuleTest_OnStateChange001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_ConvertToBleScanFilterImpl001
 * @tc.name: BleServer_ModuleTest_ConvertToBleScanFilterImpl001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ConvertToBleScanFilterImpl001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ConvertToBleScanFilterImpl001 start");
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter bluetoothBleScanFilter;
    BluetoothBleScanFilter bluetoothBleScanFilter1;
    Uuid uuid;
    bluetoothBleScanFilter1.SetServiceUuid(uuid);
    bluetoothBleScanFilter1.SetServiceUuidMask(uuid);
    bluetoothBleScanFilter1.SetServiceSolicitationUuid(uuid);
    bluetoothBleScanFilter1.SetServiceSolicitationUuidMask(uuid);
    bluetoothBleScanFilter1.SetRssiThreshold(-60);
    filters.push_back(bluetoothBleScanFilter);
    filters.push_back(bluetoothBleScanFilter1);
    std::vector<BleScanFilterImpl> filterImpls = implTest.ConvertToBleScanFilterImpl(filters, true);
    EXPECT_FALSE(filterImpls.empty());
    HILOGI("BleServer_ModuleTest_ConvertToBleScanFilterImpl001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_ConvertFilterDeviceId001
 * @tc.name: BleServer_ModuleTest_ConvertFilterDeviceId001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ConvertFilterDeviceId001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ConvertFilterDeviceId001 start");
    BluetoothBleScanFilter filter;
    filter.SetDeviceId("11:22:33:44:55:66");
    filter.SetAddressType(AddressType::REAL_ADDRESS);
    bool isUseRealAddrFlag = false;
    BleScanFilterImpl filterImpl;
    implTest.ConvertFilterDeviceId(filter, filterImpl, isUseRealAddrFlag);
    EXPECT_EQ(filterImpl.GetDeviceId(), "11:22:33:44:55:66");
    filter.SetDeviceId("");
    implTest.ConvertFilterDeviceId(filter, filterImpl, isUseRealAddrFlag);
    EXPECT_EQ(filterImpl.GetDeviceId(), "");
    HILOGI("BleServer_ModuleTest_ConvertFilterDeviceId001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_ConvertFilterDeviceId002
 * @tc.name: BleServer_ModuleTest_ConvertFilterDeviceId002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ConvertFilterDeviceId002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ConvertFilterDeviceId002 start");
    BluetoothBleScanFilter filter;
    filter.SetDeviceId("11:22:33:44:55:66");
    filter.SetAddressType(AddressType::UNSET_ADDRESS);
    bool isUseRealAddrFlag = true;
    BleScanFilterImpl filterImpl;
    implTest.ConvertFilterDeviceId(filter, filterImpl, isUseRealAddrFlag);
    EXPECT_EQ(filterImpl.GetDeviceId(), "11:22:33:44:55:66");
    filter.SetDeviceId("");
    implTest.ConvertFilterDeviceId(filter, filterImpl, isUseRealAddrFlag);
    EXPECT_EQ(filterImpl.GetDeviceId(), "");
    HILOGI("BleServer_ModuleTest_ConvertFilterDeviceId002 start");
}

/*
 * @tc.number: BleServer_ModuleTest_ConvertFilterDeviceId003
 * @tc.name: BleServer_ModuleTest_ConvertFilterDeviceId003
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ConvertFilterDeviceId003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ConvertFilterDeviceId003 start");
    BluetoothBleScanFilter filter;
    filter.SetDeviceId("");
    filter.SetAddressType(AddressType::VIRTUAL_ADDRESS);
    bool isUseRealAddrFlag = false;
    BleScanFilterImpl filterImpl;
    implTest.ConvertFilterDeviceId(filter, filterImpl, isUseRealAddrFlag);
    EXPECT_EQ(filterImpl.GetDeviceId(), "");
    HILOGI("BleServer_ModuleTest_ConvertFilterDeviceId003 start");
}

/*
 * @tc.number: BleServer_ModuleTest_ConvertFilterDeviceId004
 * @tc.name: BleServer_ModuleTest_ConvertFilterDeviceId004
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_ConvertFilterDeviceId004, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_ConvertFilterDeviceId004 start");
    BluetoothBleScanFilter filter;
    filter.SetDeviceId("22:33:44:55:66:77");
    filter.SetAddressType(VIRTUAL_ADDRESS);
    bool isUseRealAddrFlag = false;
    BleScanFilterImpl filterImpl;
    implTest.ConvertFilterDeviceId(filter, filterImpl, isUseRealAddrFlag);
    EXPECT_EQ(filterImpl.GetDeviceId(), "22:33:44:55:66:77");
    RawAddress realAddr;
    realAddr.SetAddress("22:22:22:11:11:11");
    BluetoothDeviceManager::GetInstance()->AddRetentionDeviceInfo(realAddr, RawAddress(filter.GetDeviceId()));
    implTest.ConvertFilterDeviceId(filter, filterImpl, isUseRealAddrFlag);
    EXPECT_EQ(filterImpl.GetDeviceId(), "22:22:22:11:11:11");
    HILOGI("BleServer_ModuleTest_ConvertFilterDeviceId004 end");
}

/*
 * @tc.number: BleServer_ModuleTest_GetScanModeName_001
 * @tc.name: BleServer_ModuleTest_GetScanModeName_001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_GetScanModeName_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_GetScanModeName_001 start");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_LOW_POWER), "SCAN_MODE_LOW_POWER(0)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_BALANCED), "SCAN_MODE_BALANCED(1)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_LOW_LATENCY), "SCAN_MODE_LOW_LATENCY(2)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P2_60_3000), "SCAN_MODE_OP_P2_60_3000(3)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P10_60_600), "SCAN_MODE_OP_P10_60_600(4)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P25_60_240), "SCAN_MODE_OP_P25_60_240(5)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P100_1000_1000), "SCAN_MODE_OP_P100_1000_1000(6)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P50_100_200), "SCAN_MODE_OP_P50_100_200(7)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P10_30_300), "SCAN_MODE_OP_P10_30_300(8)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P2_30_1500), "SCAN_MODE_OP_P2_30_1500(9)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P75_30_40), "SCAN_MODE_OP_P75_30_40(A)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P50_30_60), "SCAN_MODE_OP_P50_30_60(B)");
    EXPECT_EQ(GetScanModeName(SCAN_MODE_OP_P10_25_250), "SCAN_MODE_OP_P10_25_250(C)");
    EXPECT_EQ(GetScanModeName(-1), "Unknown");
    HILOGI("BleServer_ModuleTest_GetScanModeName_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_GetEncryptAddr_001
 * @tc.name: BleServer_ModuleTest_GetEncryptAddr_001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_GetEncryptAddr_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_GetEncryptAddr_001 start");
    std::string addr_1 = "";
    EXPECT_EQ(GetEncryptAddr(addr_1), addr_1);
    std::string addr_2 = "00:00:00:00:00:00:11";
    GetEncryptAddr(addr_2);
    std::string addr_3 = "22:33:44:55:66:77";
    GetEncryptAddr(addr_3);
    HILOGI("BleServer_ModuleTest_GetEncryptAddr_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_RemoveDuplicateFilters001
 * @tc.name: BleServer_ModuleTest_RemoveDuplicateFilters001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_RemoveDuplicateFilters001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_RemoveDuplicateFilters001 start");
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    BluetoothBleScanFilter filter3;

    std::string uuidStr = "00001234-0000-1000-8000-00805F9B34FB";
    std::string uuidMaskStr = "0000FFFF-0000-0000-0000-000000000000";
    std::string name = "test_name";
    std::string deviceId = "42:0B:AC:A0:A0:ED";
    Uuid uuid = Uuid::ConvertFromString(uuidStr);
    Uuid uuidMask = Uuid::ConvertFromString(uuidMaskStr);
    std::vector<uint8_t> serviceData = {0x34, 0x12, 0x56, 0x78};
    std::vector<uint8_t> serviceDataMask = {0x00, 0x00, 0xFF, 0xFF};
    uint16_t manufacturerId = 24;
    std::vector<uint8_t> manufactureData = {0x01, 0x09, 0x20, 0x02};
    std::vector<uint8_t> manufactureDataMask = {0x01, 0x09, 0x20, 0x02};

    filter1.SetName(name);
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
    filter2.SetServiceUuid(uuid);
    filter2.SetServiceUuidMask(uuidMask);
    filter2.SetManufacturerId(manufacturerId);
    filter2.SetManufactureData(manufactureData);
    filter2.SetManufactureDataMask(manufactureDataMask);
    filter2.SetServiceData(serviceData);
    filter2.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter2);

    // filter3 is different with filter1&2
    filter3.SetName(name);
    filter3.SetDeviceId(deviceId);
    filter3.SetServiceUuid(uuid);
    filter3.SetServiceUuidMask(uuidMask);
    filter3.SetManufacturerId(manufacturerId);
    filter3.SetManufactureData(manufactureData);
    filter3.SetManufactureDataMask(manufactureDataMask);
    filter3.SetServiceData(serviceData);
    filter3.SetServiceDataMask(serviceDataMask);
    filters.push_back(filter3);

    EXPECT_EQ(RemoveDuplicateFilters(filters).size(), 2);

    HILOGI("BleServer_ModuleTest_RemoveDuplicateFilters001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_GetBatchScanRandAddr001
 * @tc.name: BleServer_ModuleTest_GetBatchScanRandAddr001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_GetBatchScanRandAddr001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_GetBatchScanRandAddr001 start");
    std::unordered_map<std::string, bluetooth::BluetoothDeviceManager::BluetoothDeviceInfo> btDevicesMap {};
    std::vector<BluetoothBleScanResult> results;
    std::vector<RawAddress> randomAddrs;
    BleScanResultImpl result1;
    std::string deviceId = "42:0B:AC:A0:A0:ED";
    RawAddress address = RawAddress(deviceId);
    BlePeripheralDevice device;
    device.SetAddress(address);
    result1.SetPeripheralDevice(device);
    BluetoothBleScanResult bleScanResult1(result1);
    results.push_back(bleScanResult1);
    BleScanResultImpl result2;
    BluetoothBleScanResult bleScanResult2(result2);
    results.push_back(bleScanResult2);
    implTest.GetRandomAddr(device, address);
    implTest.GetBatchScanRandAddr(results, randomAddrs);
    EXPECT_EQ(results.size(), randomAddrs.size());
    HILOGI("BleServer_ModuleTest_GetBatchScanRandAddr001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_VirtualizeScanResultAddr001
 * @tc.name: BleServer_ModuleTest_VirtualizeScanResultAddr001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_VirtualizeScanResultAddr001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_VirtualizeScanResultAddr001 start");
    int32_t addrTypeInMatchedFilter = AddressType::VIRTUAL_ADDRESS;
    bool isUseRealAddrFlag = true;
    BluetoothBleScanResult bleScanResult;
    RawAddress randomAddr("11:22:33:44:55:66");
    VirtualizeScanResultAddr(bleScanResult, addrTypeInMatchedFilter, isUseRealAddrFlag, randomAddr);
    EXPECT_EQ(bleScanResult.GetPeripheralDevice().GetAddress(), "11:22:33:44:55:66");
    EXPECT_EQ(bleScanResult.GetAddressType(), AddressType::VIRTUAL_ADDRESS);
    HILOGI("BleServer_ModuleTest_VirtualizeScanResultAddr001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_VirtualizeScanResultAddr002
 * @tc.name: BleServer_ModuleTest_VirtualizeScanResultAddr002
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_VirtualizeScanResultAddr002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_VirtualizeScanResultAddr002 start");
    int32_t addrTypeInMatchedFilter = AddressType::UNSET_ADDRESS;
    bool isUseRealAddrFlag = false;
    BluetoothBleScanResult bleScanResult;
    RawAddress randomAddr("11:22:33:44:55:66");
    VirtualizeScanResultAddr(bleScanResult, addrTypeInMatchedFilter, isUseRealAddrFlag, randomAddr);
    EXPECT_EQ(bleScanResult.GetPeripheralDevice().GetAddress(), "11:22:33:44:55:66");
    EXPECT_EQ(bleScanResult.GetAddressType(), AddressType::VIRTUAL_ADDRESS);
    HILOGI("BleServer_ModuleTest_VirtualizeScanResultAddr002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_VirtualizeScanResultAddr003
 * @tc.name: BleServer_ModuleTest_VirtualizeScanResultAddr003
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_VirtualizeScanResultAddr003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_VirtualizeScanResultAddr003 start");
    int32_t addrTypeInMatchedFilter = AddressType::REAL_ADDRESS;
    bool isUseRealAddrFlag = false;
    BluetoothBleScanResult bleScanResult;
    RawAddress realAddr("22:22:22:11:11:11");
    bleScanResult.SetPeripheralDevice(realAddr);
    RawAddress randomAddr("11:22:33:44:55:66");
    VirtualizeScanResultAddr(bleScanResult, addrTypeInMatchedFilter, isUseRealAddrFlag, randomAddr);
    EXPECT_EQ(bleScanResult.GetPeripheralDevice().GetAddress(), "22:22:22:11:11:11");
    EXPECT_EQ(bleScanResult.GetAddressType(), AddressType::REAL_ADDRESS);
    HILOGI("BleServer_ModuleTest_VirtualizeScanResultAddr003 end");
}

/*
 * @tc.number: BleServer_ModuleTest_VirtualizeScanResultAddr004
 * @tc.name: BleServer_ModuleTest_VirtualizeScanResultAddr004
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_VirtualizeScanResultAddr004, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_VirtualizeScanResultAddr004 start");
    int32_t addrTypeInMatchedFilter = AddressType::UNSET_ADDRESS;
    bool isUseRealAddrFlag = true;
    BluetoothBleScanResult bleScanResult;
    RawAddress realAddr("22:22:22:11:11:11");
    bleScanResult.SetPeripheralDevice(realAddr);
    RawAddress randomAddr("11:22:33:44:55:66");
    VirtualizeScanResultAddr(bleScanResult, addrTypeInMatchedFilter, isUseRealAddrFlag, randomAddr);
    EXPECT_EQ(bleScanResult.GetPeripheralDevice().GetAddress(), "22:22:22:11:11:11");
    EXPECT_EQ(bleScanResult.GetAddressType(), AddressType::REAL_ADDRESS);
    HILOGI("BleServer_ModuleTest_VirtualizeScanResultAddr004 end");
}

/*
 * @tc.number: BleServer_ModuleTest_IsResourceScheduleApp001
 * @tc.name: BleServer_ModuleTest_IsResourceScheduleApp001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_IsResourceScheduleApp001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_IsResourceScheduleApp001 start");
    bluetoothBleCentralManagerServer->proxyPids_.clear();
    EXPECT_EQ(bluetoothBleCentralManagerServer->proxyPids_.size(), 0);
    EXPECT_FALSE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0,0));
    bluetoothBleCentralManagerServer->proxyPids_[0] = 1;
    EXPECT_FALSE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0,1));
    EXPECT_TRUE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0,0));
    bluetoothBleCentralManagerServer->proxyPids_[0] = 0;
    EXPECT_TRUE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0,1));
    EXPECT_TRUE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0,0));
    HILOGI("BleServer_ModuleTest_IsResourceScheduleApp001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_FlushBatchScanResultsn001
 * @tc.name: BleServer_ModuleTest_FlushBatchScanResults001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_FlushBatchScanResults001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_FlushBatchScanResults001 start");
    int32_t scannerId  = 1;
    MockPermissionManager mockPermissionManager;
    EXPECT_NE(bluetoothBleCentralManagerServer->FlushBatchScanResults(scannerId), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_FlushBatchScanResults001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_FlushBatchScanResultsInner001
 * @tc.name: BleServer_ModuleTest_FlushBatchScanResultsInner001
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_FlushBatchScanResultsInner001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_FlushBatchScanResultsInner001 start");
    BluetoothUuid bluetoothUuid;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    EXPECT_EQ(bluetoothBleCentralManagerServer->FlushBatchScanResultsInner(data, reply), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_FlushBatchScanResultsInner001 start");
}

/*
 * @tc.number: BleServer_ModuleTest_IsResourceScheduleApp002
 * @tc.name: BleServer_ModuleTest_IsResourceScheduleApp002
 * @tc.desc: Test IsResourceScheduleApp with ACL_STATE_WAKE_TYPE (new wake_type)
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_IsResourceScheduleApp002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_IsResourceScheduleApp002 start");
    bluetoothBleCentralManagerServer->proxyPids_.clear();
    EXPECT_EQ(bluetoothBleCentralManagerServer->proxyPids_.size(), 0);
    EXPECT_FALSE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0, 2));
    bluetoothBleCentralManagerServer->proxyPids_[0] = 1;
    EXPECT_FALSE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0, 2));
    bluetoothBleCentralManagerServer->proxyPids_[0] = 0;
    EXPECT_TRUE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0, 2));
    bluetoothBleCentralManagerServer->proxyPids_[0] = 1;
    EXPECT_FALSE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0, 2));
    bluetoothBleCentralManagerServer->proxyPids_[0] = 2;
    EXPECT_TRUE(bluetoothBleCentralManagerServer->IsResourceScheduleApp(0, 2));
    HILOGI("BleServer_ModuleTest_IsResourceScheduleApp002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_SetBleScanEnhanceMode001
 * @tc.name: BleServer_ModuleTest_SetBleScanEnhanceMode001
 * @tc.desc: Test StartScan with different BleScanEnhanceMode params
*/
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_SetBleScanEnhanceMode001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_SetBleScanEnhanceMode001 start");
    // Invalid enhance mode timeout
    const int scannerId1 = 1;
    BluetoothBleScanSettings settings1;
    settings1.SetEnhanceMode(BLE_SCAN_ENHANCE_MODE_BALANCED, BLE_SCAN_ENHANCE_MODE_MAX_TIMEOUT_MS + 1);
    // Invalid enhance mode
    const int scannerId2 = 2;
    BluetoothBleScanSettings settings2;
    settings1.SetEnhanceMode(BLE_SCAN_ENHANCE_MODE_INVALID + 1, BLE_SCAN_ENHANCE_MODE_MAX_TIMEOUT_MS);
    // Valid enhace mode
    const int scannerId3 = 3;
    BluetoothBleScanSettings settings3;
    settings1.SetEnhanceMode(BLE_SCAN_ENHANCE_MODE_ULTRA_FAST, BLE_SCAN_ENHANCE_MODE_MAX_TIMEOUT_MS - 1);

    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter;
    BluetoothBleScanFilter filter1;
    bluetooth::Uuid uuid;
    filter1.SetServiceUuid(uuid);
    filter1.SetServiceUuidMask(uuid);
    filter1.SetServiceSolicitationUuid(uuid);
    filter1.SetServiceSolicitationUuidMask(uuid);
    filters.push_back(filter);
    filters.push_back(filter1);

    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(scannerId1, settings1, filters, false), BT_NO_ERROR);
    EXPECT_EQ(bluetoothBleCentralManagerServer->StopScan(scannerId1), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(scannerId2, settings2, filters, false), BT_NO_ERROR);
    EXPECT_EQ(bluetoothBleCentralManagerServer->StopScan(scannerId2), BT_NO_ERROR);
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(scannerId3, settings3, filters, false), BT_NO_ERROR);
    EXPECT_EQ(bluetoothBleCentralManagerServer->StopScan(scannerId3), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_SetBleScanEnhanceMode001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_IsWalletService_001
 * @tc.name: BleServer_ModuleTest_IsWalletService_001
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_IsWalletService_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_IsWalletService_001 start");
    std::string callingName = "testCallingName";
    uint32_t tokenId = 537692227;
    EXPECT_FALSE(IsWalletService(callingName, tokenId));
    callingName = VendorInterface::GetWalletServiceName();
    EXPECT_FALSE(IsWalletService(callingName, tokenId));
    HILOGI("BleServer_ModuleTest_IsWalletService_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_001
 * @tc.name: BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_001
 * @tc.desc: Test AdjustScanModeForLowLatencyScan when IsSystemHap is false
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_001 start");
    bluetooth::BleScanSettingsImpl settingsImpl;
    settingsImpl.SetScanMode(SCAN_MODE_LOW_LATENCY);
    int originalScanMode = settingsImpl.GetScanMode();
    bluetoothBleCentralManagerServer->AdjustScanModeForLowLatencyScan(settingsImpl);
    EXPECT_EQ(settingsImpl.GetScanMode(), originalScanMode);
    HILOGI("BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_002
 * @tc.name: BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_002
 * @tc.desc: Test AdjustScanModeForLowLatencyScan when chip type is not mp12
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_002 start");
    bluetooth::BleScanSettingsImpl settingsImpl;
    settingsImpl.SetScanMode(SCAN_MODE_LOW_LATENCY);
    int originalScanMode = settingsImpl.GetScanMode();
    bluetoothBleCentralManagerServer->AdjustScanModeForLowLatencyScan(settingsImpl);
    EXPECT_EQ(settingsImpl.GetScanMode(), originalScanMode);
    HILOGI("BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_003
 * @tc.name: BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_003
 * @tc.desc: Test AdjustScanModeForLowLatencyScan when scan mode is not LOW_LATENCY
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_003 start");
    bluetooth::BleScanSettingsImpl settingsImpl;
    settingsImpl.SetScanMode(SCAN_MODE_BALANCED);
    int originalScanMode = settingsImpl.GetScanMode();
    bluetoothBleCentralManagerServer->AdjustScanModeForLowLatencyScan(settingsImpl);
    EXPECT_EQ(settingsImpl.GetScanMode(), originalScanMode);
    HILOGI("BleServer_ModuleTest_AdjustScanModeForLowLatencyScan_003 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScanWithLowLatencyMode_001
 * @tc.name: BleServer_ModuleTest_StartScanWithLowLatencyMode_001
 * @tc.desc: Test StartScan with SCAN_MODE_LOW_LATENCY
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScanWithLowLatencyMode_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScanWithLowLatencyMode_001 start");
    BluetoothBleScanSettings settings;
    settings.SetScanMode(SCAN_MODE_LOW_LATENCY);
    std::vector<BluetoothBleScanFilter> filters;
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(0, settings, filters, false), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StartScanWithLowLatencyMode_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScanWithLowLatencyMode_002
 * @tc.name: BleServer_ModuleTest_StartScanWithLowLatencyMode_002
 * @tc.desc: Test StartScan with SCAN_MODE_LOW_LATENCY and filters
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScanWithLowLatencyMode_002, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScanWithLowLatencyMode_002 start");
    BluetoothBleScanSettings settings;
    settings.SetScanMode(SCAN_MODE_LOW_LATENCY);
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter;
    filters.push_back(filter);
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(0, settings, filters, false), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StartScanWithLowLatencyMode_002 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScanWithLowLatencyMode_003
 * @tc.name: BleServer_ModuleTest_StartScanWithLowLatencyMode_003
 * @tc.desc: Test StartScan with SCAN_MODE_LOW_LATENCY and multiple filters
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScanWithLowLatencyMode_003, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScanWithLowLatencyMode_003 start");
    BluetoothBleScanSettings settings;
    settings.SetScanMode(SCAN_MODE_LOW_LATENCY);
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter1;
    BluetoothBleScanFilter filter2;
    filters.push_back(filter1);
    filters.push_back(filter2);
    MockPermissionManager mockPermissionManager;
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(0, settings, filters, false), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StartScanWithLowLatencyMode_003 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScanWithBalancedMode_001
 * @tc.name: BleServer_ModuleTest_StartScanWithBalancedMode_001
 * @tc.desc: Test StartScan with SCAN_MODE_BALANCED (non-low-latency mode)
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScanWithBalancedMode_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScanWithBalancedMode_001 start");
    BluetoothBleScanSettings settings;
    settings.SetScanMode(SCAN_MODE_BALANCED);
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter;
    filters.push_back(filter);
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(0, settings, filters, false), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StartScanWithBalancedMode_001 end");
}

/*
 * @tc.number: BleServer_ModuleTest_StartScanWithLowPowerMode_001
 * @tc.name: BleServer_ModuleTest_StartScanWithLowPowerMode_001
 * @tc.desc: Test StartScan with SCAN_MODE_LOW_POWER (non-low-latency mode)
 */
HWTEST_F(BluetoothBleCentralManagerServerTest, BleServer_ModuleTest_StartScanWithLowPowerMode_001, TestSize.Level1)
{
    HILOGI("BleServer_ModuleTest_StartScanWithLowPowerMode_001 start");
    BluetoothBleScanSettings settings;
    settings.SetScanMode(SCAN_MODE_LOW_POWER);
    std::vector<BluetoothBleScanFilter> filters;
    BluetoothBleScanFilter filter;
    filters.push_back(filter);
    EXPECT_NE(bluetoothBleCentralManagerServer->StartScan(0, settings, filters, false), BT_NO_ERROR);
    HILOGI("BleServer_ModuleTest_StartScanWithLowPowerMode_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
