/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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
#include "bluetooth_resource_manager.cpp"
#include "interface_profile_resource_manager.h"
#include "bluetooth_host.h"
#include "log.h"
#include "ble_defs.h"
#include "base_def.h"
#include "classic_adapter.h"
#include "adapter_manager.h"
#include "class_creator.h"
#include "ble_service_data.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
constexpr int TIME = 4;
static BluetoothHost *host_;

static const BthwifCallbacks* g_bthwifCallbacks = nullptr;
static bool g_brTransRateFlag = true;

static bt_status_t BthwifInterfaceInitMock(BthwifCallbacks* callbacks)
{
    g_bthwifCallbacks = callbacks;
    return BT_STATUS_SUCCESS;
}

static void HwSetBleScanEnhanceModeStatusMock(bool isEnable)
{
}

static bool HwIsBleScanEnhanceModeEnableMock()
{
    return true;
}

static void HwAdjustTwsBitRateInBleScanEnhanceModeMock(bool isSpeedDown)
{
}

static void HwSetBrTransControlMock(bool isEnable, int workingInterval, int pendingInterval, int sleepingInterval)
{
}

static void HwSetBrLinkSniffModeMock(bool isEnable, const STACK::RawAddress &addr, int8_t index)
{
}

static std::pair<uint32_t, uint32_t> HwGetBrTransTxRxRate()
{
    if (g_brTransRateFlag) {
        return { 1, 1 };
    } else {
        return { 0, 0 };
    }
}

static bool HwGetTransPerformCache(std::vector<int16_t> &bleAdvChannelRssi)
{
    return true;
}

static const bthwif_interface_t g_bthwifInterface = {
    .Init = BthwifInterfaceInitMock,
    .hwSetBleScanEnhanceModeStatus = HwSetBleScanEnhanceModeStatusMock,
    .hwIsBleScanEnhanceModeEnable = HwIsBleScanEnhanceModeEnableMock,
    .hwAdjustTwsBitRateInBleScanEnhanceMode = HwAdjustTwsBitRateInBleScanEnhanceModeMock,
    .hwSetBrTransControl = HwSetBrTransControlMock,
    .hwSetBrLinkSniffMode = HwSetBrLinkSniffModeMock,
    .hwGetBrTransTxRxRate = HwGetBrTransTxRxRate,
    .hwGetTransPerformCache = HwGetTransPerformCache,
};

static void *ClassicAdapterCreate(void)
{
    return new ClassicAdapter();
}

class BluetoothResourceMgrTest : public testing::Test {
public:
    BluetoothResourceMgrTest();
    ~BluetoothResourceMgrTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    BluetoothResourceManager *resourceMgr = nullptr;
    std::shared_ptr<IResourceManagerObserver> observer = nullptr;
};

class ResourceManagerObserverTest : public IResourceManagerObserver {
public:
    void OnSensingStateChanged(uint8_t eventId, const SensingInfo &info) override
    {}

    void OnBluetoothResourceDecision(uint8_t eventId, const SensingInfo &info, uint32_t &result) override
    {}
};

BluetoothResourceMgrTest::BluetoothResourceMgrTest()
{
    HILOGI("BluetoothResourceMgrTest:Construct");
    resourceMgr = BluetoothResourceManager::GetInstance();
    observer = std::make_shared<ResourceManagerObserverTest>();
    if (resourceMgr) {
        resourceMgr->RegisterObserver(observer);
    }
    sleep(1);
}

BluetoothResourceMgrTest::~BluetoothResourceMgrTest()
{
    HILOGI("BluetoothResourceMgrTest:Destruct");
    sleep(1);
    if (resourceMgr) {
        resourceMgr->DeregisterObserver(observer);
        resourceMgr = nullptr;
    }
    observer = nullptr;
}

void BluetoothResourceMgrTest::SetUpTestCase(void)
{
    HILOGI("BluetoothResourceMgrTest SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("BluetoothResourceMgrTest isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    HILOGI("SocketServiceTest SetUpTestCase end");
}

void BluetoothResourceMgrTest::TearDownTestCase(void)
{
    HILOGI("BluetoothResourceMgrTest TearDownTestCase start");
    sleep(TIME);
    host_ = nullptr;
    HILOGI("BluetoothResourceMgrTest TearDownTestCase end");
}

void BluetoothResourceMgrTest::SetUp()
{}

void BluetoothResourceMgrTest::TearDown()
{}

/**
 * @tc.number:bt_resource_mgr_send_sensing_info_test_001
 * @tc.name: send_sensing_info
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_send_sensing_info_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_send_sensing_info_test_001 starts");
    uint8_t eventId = SPP_CLIENT_CONNECT_DONE;
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(eventId, SensingInfo("11:22:33:44:55:66"));
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_send_sensing_info_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_send_sensing_info_test_002
 * @tc.name: send_sensing_info
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_send_sensing_info_test_002, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_send_sensing_info_test_002 starts");
    uint8_t eventId = SPP_PORT_CLOSE;
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(eventId, SensingInfo("11:22:33:44:55:66"));
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_send_sensing_info_test_002 ends");
}
 
/**
 * @tc.number:bt_resource_mgr_send_decision_test_001
 * @tc.name: send_decision
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_send_decision_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_send_decision_test_001 starts");
    uint8_t eventId = BLE_FASTEST_CONN_DECISION;
    uint32_t result = CONNECTION_ACCEPT;
    if (resourceMgr) {
        resourceMgr->SendBluetoothResourceDecision(eventId, SensingInfo(), result);
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_send_decision_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_add_adv_record_test_001
 * @tc.name:add_adv_record
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_add_adv_record_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_add_adv_record_test_001 starts");
    uint8_t advHandle = 0;
    if (resourceMgr) {
        resourceMgr->AddAdvRecord(BleAdvertiserSettingsImpl(), BleAdvertiserDataImpl(), advHandle);
        EXPECT_NE(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_add_adv_record_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_find_adv_record_test_001
 * @tc.name:find_adv_record
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_find_adv_record_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_find_adv_record_test_001 starts");
    uint8_t advHandle = 0;

    if (resourceMgr) {
        std::shared_ptr<SensingInfo> info = resourceMgr->FindAdvRecord(advHandle);
        EXPECT_NE(info, nullptr);
    }
    HILOGI("bt_resource_mgr_find_adv_record_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_send_adv_started_test_001
 * @tc.name:send_adv_started
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_send_adv_started_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_send_adv_started_test_001 starts");
    uint8_t advHandle = 0;
    if (resourceMgr) {
        resourceMgr->SendAdvStartedEvent(ADVERTISE_SUCCESS, advHandle);
        EXPECT_NE(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_send_adv_started_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_remove_adv_record_test_001
 * @tc.name:remove_adv_record
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_remove_adv_record_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_remove_adv_record_test_001 starts");
    uint8_t advHandle = 0;
    if (resourceMgr) {
        resourceMgr->RemoveAdvRecord(advHandle);
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_remove_adv_record_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_update_scan_record_test_001
 * @tc.name:update_scan_record
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_update_scan_record_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_update_scan_record_test_001 starts");
    int scanMode = STARTED;
    if (resourceMgr) {
        resourceMgr->UpdateScanRecord(scanMode);
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_update_scan_record_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_send_scan_start_test_001
 * @tc.name:send_scan_start
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_send_scan_start_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_send_scan_start_test_001 starts");
    if (resourceMgr) {
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_send_scan_start_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_send_scan_start_test_002
 * @tc.name:send_scan_start
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_send_scan_start_test_002, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_send_scan_start_test_002 starts");
    if (resourceMgr) {
        resourceMgr->SendScanStartEvent(SCAN_FAILED_INTERNAL_ERROR);
        resourceMgr->SendScanStopEvent(SCAN_FAILED_INTERNAL_ERROR);
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_send_scan_start_test_002 ends");
}

/**
 * @tc.number:bt_resource_mgr_send_adv_started_test_002
 * @tc.name:send_adv_started
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_send_adv_started_test_002, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_send_adv_started_test_002 starts");
    uint8_t advHandle = 0;
    if (resourceMgr) {
        resourceMgr->SendAdvStartedEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_send_adv_started_test_002 ends");
}

/**
 * @tc.number:bt_resource_mgr_send_adv_stopped_test_001
 * @tc.name:send_adv_stopped
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_send_adv_stopped_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_send_adv_stopped_test_001 starts");
    uint8_t advHandle = 0;
    if (resourceMgr) {
        resourceMgr->SendAdvStoppedEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        EXPECT_EQ(resourceMgr->FindAdvRecord(0), nullptr);
    }
    HILOGI("bt_resource_mgr_send_adv_stopped_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_set_scan_enhance_mode_test_001
 * @tc.name: set_ble_scan_enhance_mode
 * @tc.desc: different enhance mode params test
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_set_scan_enhance_mode_test_001, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_set_scan_enhance_mode_test_001 starts");
    if (resourceMgr && resourceMgr->pimpl) {
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
        resourceMgr->pimpl->IsUnittestModeOn_ = true;
        BleScanEnhanceModeImpl enhanceMode;
        enhanceMode.timeout = 1000;
        for (int mode = BLE_SCAN_ENHANCE_MODE_BALANCED; mode <= BLE_SCAN_ENHANCE_MODE_ULTRA_FAST; mode++) {
            enhanceMode.mode = mode;
            enhanceMode.isEnable = true;
            EXPECT_EQ(resourceMgr->SetBleScanEnhanceMode(enhanceMode), BT_NO_ERROR);
            enhanceMode.isEnable = false;
            EXPECT_EQ(resourceMgr->SetBleScanEnhanceMode(enhanceMode), BT_NO_ERROR);
        }
        enhanceMode.isEnable = true;
        enhanceMode.mode = BLE_SCAN_ENHANCE_MODE_INVALID;
        EXPECT_EQ(resourceMgr->SetBleScanEnhanceMode(enhanceMode), BT_ERR_INTERNAL_ERROR);
    }
    HILOGI("bt_resource_mgr_set_scan_enhance_mode_test_001 ends");
}

/**
 * @tc.number:bt_resource_mgr_set_scan_enhance_mode_test_002
 * @tc.name: set_ble_scan_enhance_mode
 * @tc.desc: ble scan enhance mode timeout test
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_set_scan_enhance_mode_test_002, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_set_scan_enhance_mode_test_002 starts");
    if (resourceMgr && resourceMgr->pimpl) {
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
        resourceMgr->pimpl->IsUnittestModeOn_ = true;

        BleScanEnhanceModeImpl enhanceMode;
        enhanceMode.isEnable = true;
        enhanceMode.mode = BLE_SCAN_ENHANCE_MODE_BALANCED;
        enhanceMode.timeout = 100;
        EXPECT_EQ(resourceMgr->SetBleScanEnhanceMode(enhanceMode), BT_NO_ERROR);
        EXPECT_TRUE(resourceMgr->pimpl->enhanceMode_.isEnable);

        usleep(102 * 1000);
        EXPECT_FALSE(resourceMgr->pimpl->enhanceMode_.isEnable);
    }
    HILOGI("bt_resource_mgr_set_scan_enhance_mode_test_002 ends");
}

/**
 * @tc.number:bt_resource_mgr_set_scan_enhance_mode_test_003
 * @tc.name: set_ble_scan_enhance_mode
 * @tc.desc: ble scan enhance mode repeated start with different mode test
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_set_scan_enhancemode_test_003, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_set_scan_enhancemode_test_003 starts");
    if (resourceMgr && resourceMgr->pimpl) {
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
        resourceMgr->pimpl->IsUnittestModeOn_ = true;

        BleScanEnhanceModeImpl enhanceMode1;
        enhanceMode1.isEnable = true;
        enhanceMode1.mode = BLE_SCAN_ENHANCE_MODE_BALANCED;
        enhanceMode1.timeout = 100;
        BleScanEnhanceModeImpl enhanceMode2;
        enhanceMode2.isEnable = true;
        enhanceMode2.mode = BLE_SCAN_ENHANCE_MODE_ULTRA_FAST;
        enhanceMode2.timeout = 100;

        resourceMgr->SetBleScanEnhanceMode(enhanceMode1);
        resourceMgr->SetBleScanEnhanceMode(enhanceMode2);
        EXPECT_EQ(resourceMgr->pimpl->enhanceMode_.mode, BLE_SCAN_ENHANCE_MODE_ULTRA_FAST);
        resourceMgr->SetBleScanEnhanceMode(BleScanEnhanceModeImpl());
        EXPECT_FALSE(resourceMgr->pimpl->enhanceMode_.isEnable);
    }
    HILOGI("bt_resource_mgr_set_scan_enhancemode_test_003 ends");
}

/**
 * @tc.number:bt_resource_mgr_set_scan_enhance_mode_test_004
 * @tc.name: set_ble_scan_enhance_mode
 * @tc.desc: ble scan enhance mode repeated start with different timeout test
 */
HWTEST_F(BluetoothResourceMgrTest, bt_resource_mgr_set_scan_enhance_mode_test_004, TestSize.Level1)
{
    HILOGI("bt_resource_mgr_set_scan_enhance_mode_test_004 starts");
    if (resourceMgr && resourceMgr->pimpl) {
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_bthwifInterface);
        resourceMgr->pimpl->IsUnittestModeOn_ = true;

        BleScanEnhanceModeImpl enhanceMode1;
        enhanceMode1.isEnable = true;
        enhanceMode1.mode = BLE_SCAN_ENHANCE_MODE_BALANCED;
        enhanceMode1.timeout = 100;
        BleScanEnhanceModeImpl enhanceMode2;
        enhanceMode2.isEnable = true;
        enhanceMode2.mode = BLE_SCAN_ENHANCE_MODE_BALANCED;
        enhanceMode2.timeout = 150;

        resourceMgr->SetBleScanEnhanceMode(enhanceMode1);
        usleep(50 * 1000);
        resourceMgr->SetBleScanEnhanceMode(enhanceMode2);
        EXPECT_EQ(resourceMgr->pimpl->enhanceMode_.timeout, 150);
        resourceMgr->SetBleScanEnhanceMode(BleScanEnhanceModeImpl());
        EXPECT_FALSE(resourceMgr->pimpl->enhanceMode_.isEnable);
    }
    HILOGI("bt_resource_mgr_set_scan_enhance_mode_test_004 ends");
}

/**
 * @tc.number:GetTransRateData_001
 * @tc.name: GetTransRateData
 * @tc.desc: 
 */
HWTEST_F(BluetoothResourceMgrTest, GetTransRateData_001, TestSize.Level1)
{
    HILOGI("ConvertTransRateSensingInfo_001 starts");
    if (resourceMgr) {
        SensingInfo info;
        g_brTransRateFlag = false;
        resourceMgr->GetTransRateData(info);
        EXPECT_TRUE(info.sensingArray_.empty());

        g_brTransRateFlag = true;
        resourceMgr->GetTransRateData(info);
        EXPECT_FALSE(info.sensingArray_.empty());
    }
    HILOGI("ConvertTransRateSensingInfo_001 ends");
}

/**
 * @tc.number:GetBleAdvChannelRssiSensingData_001
 * @tc.name: GetBleAdvChannelRssiSensingData
 * @tc.desc: 
 */
HWTEST_F(BluetoothResourceMgrTest, GetBleAdvChannelRssiSensingData_001, TestSize.Level1)
{
    HILOGI("GetBleAdvChannelRssiSensingData_001 starts");
    if (resourceMgr) {
        SensingInfo info;
        resourceMgr->GetBleAdvChannelRssiSensingData(info);
        EXPECT_TRUE(info.sensingArray_.empty());
    }
    HILOGI("GetBleAdvChannelRssiSensingData_001 ends");
}

}
}