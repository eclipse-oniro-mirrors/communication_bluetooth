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
#include "ble_scan_filter.h"
#include "bluetooth_host.h"

#include "service_util.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "btcommon/timer_manager.h"

#define BTM_BLE_PF_ADDR_FILTER 0

using namespace testing;
using namespace testing::ext;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace OHOS {
namespace bluetooth {
extern "C" {
static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

static void HwDeletAdvIndReportFlagMock(int filterIndex)
{
    return;
}

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwDeletAdvIndReportFlag = HwDeletAdvIndReportFlagMock,
};
}

using ScanFunc = std::function<void(BleScannerInterface::FilterParamSetupCallback)>;
ScanFunc g_scanFunc = nullptr;

using ScanFilterAddFunc = std::function<void(BleScannerInterface::FilterConfigCallback)>;
ScanFilterAddFunc g_scanFilterAddFunc = nullptr;

static void ScanFilterParamSetupCb(uint8_t availableSpace, uint8_t actionType, uint8_t btmStatus,
    BleScannerInterface::FilterParamSetupCallback cb)
{
    (cb)(availableSpace, actionType, btmStatus);
}

static void ScanFilterAddCb(uint8_t filterType, uint8_t availableSpace, uint8_t actionType, uint8_t btmStatus,
    BleScannerInterface::FilterConfigCallback cb)
{
    (cb)(filterType, availableSpace, actionType, btmStatus);
}

class BleScannerInterfaceImplTest : public BleScannerInterface {
public:
  ~BleScannerInterfaceImplTest() override{};

void RegisterScanner(RegisterCallback) override {
}

void Unregister(int scannerId) override
{
}

void Scan(bool start) override
{
}

virtual void ScanFilterParamSetup(
      uint8_t client_if, uint8_t action, uint8_t filt_index,
      std::unique_ptr<btgatt_filt_param_setup_t> filt_param,
      FilterParamSetupCallback cb) override
{
    if (g_scanFunc != nullptr) {
        g_scanFunc(cb);
    }
}

void ScanFilterAdd(int filterIndex, std::vector<ApcfCommand> filters,
                     FilterConfigCallback cb) override
{
    if (g_scanFilterAddFunc != nullptr) {
        g_scanFilterAddFunc(cb);
    }
}

void ScanFilterClear(int filterIndex, FilterConfigCallback cb) override
{
}

void ScanFilterEnable(bool enable, EnableCallback cb) override
{
}

void SetScanParameters(uint32_t scan_interval, uint32_t scan_window, uint8_t legacy,
                                 uint8_t scan_phy, std::function<void(uint8_t status)> cb) override
{
}

void BatchscanConfigStorage(int clientIf, int batchScanFullMax,
                              int batchScanTruncMax,
                              int batchScanNotifyThreshold, Callback cb) override
{
}

void BatchscanEnable(int scanMode, int scanInterval,
                               int scanWindow, int addrType, int discardRule,
                               Callback cb) override
{
}

void BatchscanDisable(Callback cb) override
{
}

void BatchscanReadReports(int clientIf, int scanMode) override
{
}

void StartSync(uint8_t sid, STACK::RawAddress address, uint16_t skip,
                 uint16_t timeout, StartSyncCb startCb, SyncReportCb reportCb,
                 SyncLostCb lostCb) override
{
}

void StopSync(uint16_t handle) override
{
}

void RegisterCallbacks(ScanningCallbacks* callbacks) override
{
}
};
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;
class BleScanFilterTest : public testing::Test {
public:
    BleScanFilterTest() = default;
    ~BleScanFilterTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    int scannerId_ = 1;
    BleScannerInterfaceImplTest scanner;
    std::unique_ptr<BleScanFilter> bleScanFilter;
private:
};

void BleScanFilterTest::SetUpTestSuite(void)
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

void BleScanFilterTest::TearDownTestSuite(void)
{
    host_ = nullptr;
    utility::CloseAllTimer();
}

void BleScanFilterTest::SetUp()
{
    bleScanFilter = std::make_unique<BleScanFilter>(scannerId_, &scanner);
}

void BleScanFilterTest::TearDown()
{
    bleScanFilter = nullptr;
}

static constexpr int WAIT_TIMEOUT = 1100; // 1.1s

/**
 * @tc.name: ble_scan_filter_SendScanFilterParam_001
 * @tc.desc: ble_scan_filter_SendScanFilterParam_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SendScanFilterParam_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SendScanFilterParam_001 start");
    uint8_t action = 1;
    int filterIndex = 0;
    std::unique_ptr<btgatt_filt_param_setup_t> filtParam = std::make_unique<btgatt_filt_param_setup_t>();
    EXPECT_FALSE(bleScanFilter->SendScanFilterParam(action, filterIndex, std::move(filtParam)));
    bleScanFilter->btifBleScanner_ = nullptr;
    EXPECT_FALSE(bleScanFilter->SendScanFilterParam(action, filterIndex, std::move(filtParam)));
    HILOGI("ble_scan_filter_SendScanFilterParam_001 end");
}

/**
 * @tc.name: ble_scan_filter_SendScanFilterParam_002
 * @tc.desc: ble_scan_filter_SendScanFilterParam_002
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SendScanFilterParam_002, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SendScanFilterParam_002 start");
    uint8_t action = 0;
    int filterIndex = 0;
    g_scanFunc = std::bind(ScanFilterParamSetupCb, 0, 0, 0x17, std::placeholders::_1);
    std::unique_ptr<btgatt_filt_param_setup_t> filtParam = std::make_unique<btgatt_filt_param_setup_t>();
    EXPECT_FALSE(bleScanFilter->SendScanFilterParam(action, filterIndex, std::move(filtParam)));
    g_scanFunc = std::bind(ScanFilterParamSetupCb, 0, 0, 0, std::placeholders::_1);
    EXPECT_TRUE(bleScanFilter->SendScanFilterParam(action, filterIndex, std::move(filtParam)));
    HILOGI("ble_scan_filter_SendScanFilterParam_002 end");
}

/**
 * @tc.name: ble_scan_filter_SendScanFilter_001
 * @tc.desc: ble_scan_filter_SendScanFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SendScanFilter_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SendScanFilter_001 start");
    int filterIndex = 0;
    std::vector<ApcfCommand> cmds;
    EXPECT_FALSE(bleScanFilter->SendScanFilter(filterIndex, cmds));
    bleScanFilter->btifBleScanner_ = nullptr;
    EXPECT_FALSE(bleScanFilter->SendScanFilter(filterIndex, cmds));
    HILOGI("ble_scan_filter_SendScanFilter_001 end");
}

/**
 * @tc.name: ble_scan_filter_SendScanFilter_002
 * @tc.desc: ble_scan_filter_SendScanFilter_002
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SendScanFilter_002, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SendScanFilter_002 start");
    int filterIndex = 0;
    std::vector<ApcfCommand> cmds;

    g_scanFilterAddFunc = std::bind(ScanFilterAddCb, 0, 0, 0, 0x4, std::placeholders::_1);
    EXPECT_FALSE(bleScanFilter->SendScanFilter(filterIndex, cmds));
    g_scanFilterAddFunc = std::bind(ScanFilterAddCb, 0, 0, 0, 0, std::placeholders::_1);
    EXPECT_TRUE(bleScanFilter->SendScanFilter(filterIndex, cmds));
    HILOGI("ble_scan_filter_SendScanFilter_002 end");
}

/**
 * @tc.name: ble_scan_filter_GetFeatureSelection_001
 * @tc.desc: ble_scan_filter_GetFeatureSelection_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetFeatureSelection_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetFeatureSelection_001 start");
    BleScanFilterImpl filter;
    EXPECT_FALSE(bleScanFilter->GetFeatureSelection(filter));
    HILOGI("ble_scan_filter_GetFeatureSelection_001 end");
}

/**
 * @tc.name: ble_scan_filter_GetFeatureSelection_002
 * @tc.desc: ble_scan_filter_GetFeatureSelection_002
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetFeatureSelection_002, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetFeatureSelection_002 start");
    BleScanFilterImpl filter;
    std::string testdata = "testdata";
    filter.SetDeviceId(testdata);
    Uuid uuid;
    filter.SetServiceUuid(uuid);
    filter.SetServiceSolicitationUuid(uuid);
    filter.SetName(testdata);
    std::vector<uint8_t> manufactureData;
    manufactureData.push_back(1);
    filter.SetManufactureData(manufactureData);
    std::vector<uint8_t> serviceData;
    serviceData.push_back(1);
    filter.SetServiceData(serviceData);
    EXPECT_EQ(bleScanFilter->GetFeatureSelection(filter), 125);
    HILOGI("ble_scan_filter_GetFeatureSelection_002 end");
}

/**
 * @tc.name: ble_scan_filter_ApcfCommandLog_001
 * @tc.desc: ble_scan_filter_ApcfCommandLog_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_ApcfCommandLog_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_ApcfCommandLog_001 start");
    std::vector<ApcfCommand> cmds;
    ApcfCommand apcfCommand;
    apcfCommand.type = 0;
    cmds.push_back(apcfCommand);
    ApcfCommand apcfCommand1;
    apcfCommand1.type = 2;
    cmds.push_back(apcfCommand1);
    ApcfCommand apcfCommand2;
    apcfCommand2.type = 3;
    cmds.push_back(apcfCommand2);
    ApcfCommand apcfCommand3;
    apcfCommand3.type = 4;
    cmds.push_back(apcfCommand3);
    ApcfCommand apcfCommand4;
    apcfCommand4.type = 5;
    cmds.push_back(apcfCommand4);
    ApcfCommand apcfCommand5;
    apcfCommand5.type = 6;
    cmds.push_back(apcfCommand5);
    ApcfCommand apcfCommand6;
    apcfCommand6.type = 7;
    cmds.push_back(apcfCommand6);
    bleScanFilter->ApcfCommandLog(cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_ApcfCommandLog_001 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_001
 * @tc.desc: ble_scan_filter_GetCommand_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_001 start");
    uint8_t cmdType = 0;
    BleScanFilterImpl filter;
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    HILOGI("ble_scan_filter_GetCommand_001 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_002
 * @tc.desc: ble_scan_filter_GetCommand_002
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_002, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_002 start");
    uint8_t cmdType = 2;
    BleScanFilterImpl filter;
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    HILOGI("ble_scan_filter_GetCommand_002 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_003
 * @tc.desc: ble_scan_filter_GetCommand_003
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_003, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_003 start");
    uint8_t cmdType = 3;
    BleScanFilterImpl filter;
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    HILOGI("ble_scan_filter_GetCommand_003 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_004
 * @tc.desc: ble_scan_filter_GetCommand_004
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_004, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_004 start");
    uint8_t cmdType = 4;
    BleScanFilterImpl filter;
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    HILOGI("ble_scan_filter_GetCommand_004 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_005
 * @tc.desc: ble_scan_filter_GetCommand_005
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_005, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_005 start");
    uint8_t cmdType = 5;
    BleScanFilterImpl filter;
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    HILOGI("ble_scan_filter_GetCommand_005 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_006
 * @tc.desc: ble_scan_filter_GetCommand_006
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_006, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_006 start");
    uint8_t cmdType = 6;
    BleScanFilterImpl filter;
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    HILOGI("ble_scan_filter_GetCommand_006 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_007
 * @tc.desc: ble_scan_filter_GetCommand_007
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_007, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_007 start");
    uint8_t cmdType = 7;
    BleScanFilterImpl filter;
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    HILOGI("ble_scan_filter_GetCommand_007 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_008
 * @tc.desc: ble_scan_filter_GetCommand_008
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_008, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_008 start");
    uint8_t cmdType = BTM_BLE_PF_ADDR_FILTER;
    BleScanFilterImpl filter;
    filter.SetDeviceId("11:22:33:44:55:66");
    filter.SetRawAddressType(RawAddressType::RANDOM_ADDRESS);
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    EXPECT_EQ(cmd.address, ServiceUtil::AddrToStack(RawAddress("11:22:33:44:55:66")));
    EXPECT_EQ(cmd.addr_type, BLE_ADDR_RANDOM);
    HILOGI("ble_scan_filter_GetCommand_008 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_009
 * @tc.desc: ble_scan_filter_GetCommand_009
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_009, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_009 start");
    uint8_t cmdType = BTM_BLE_PF_ADDR_FILTER;
    BleScanFilterImpl filter;
    filter.SetDeviceId("11:22:33:44:55:66");
    filter.SetRawAddressType(RawAddressType::PUBLIC_ADDRESS);
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    EXPECT_EQ(cmd.address, ServiceUtil::AddrToStack(RawAddress("11:22:33:44:55:66")));
    EXPECT_EQ(cmd.addr_type, BLE_ADDR_PUBLIC);
    HILOGI("ble_scan_filter_GetCommand_009 end");
}

/**
 * @tc.name: ble_scan_filter_GetCommand_010
 * @tc.desc: ble_scan_filter_GetCommand_010
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetCommand_010, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetCommand_010 start");
    uint8_t cmdType = BTM_BLE_PF_ADDR_FILTER;
    BleScanFilterImpl filter;
    filter.SetDeviceId("11:22:33:44:55:66");
    ApcfCommand cmd = bleScanFilter->GetCommand(cmdType, filter);
    EXPECT_EQ(cmd.type, cmdType);
    EXPECT_EQ(cmd.address, ServiceUtil::AddrToStack(RawAddress("11:22:33:44:55:66")));
    EXPECT_EQ(cmd.addr_type, 0x02);
    HILOGI("ble_scan_filter_GetCommand_010 end");
}

/**
 * @tc.name: ble_scan_filter_SetApcfCommand_001
 * @tc.desc: ble_scan_filter_SetApcfCommand_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SetApcfCommand_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SetApcfCommand_001 start");
    BleScanFilterImpl filter;
    std::vector<ApcfCommand> cmds;
    bleScanFilter->SetApcfCommand(filter, cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_SetApcfCommand_001 end");
}

/**
 * @tc.name: ble_scan_filter_SetApcfCommand_002
 * @tc.desc: ble_scan_filter_SetApcfCommand_002
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SetApcfCommand_002, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SetApcfCommand_002 start");
    std::vector<ApcfCommand> cmds;
    BleScanFilterImpl filter;
    std::string testdata = "testdata";
    filter.SetDeviceId(testdata);
    bleScanFilter->SetApcfCommand(filter, cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_SetApcfCommand_002 end");
}

/**
 * @tc.name: ble_scan_filter_SetApcfCommand_003
 * @tc.desc: ble_scan_filter_SetApcfCommand_003
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SetApcfCommand_003, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SetApcfCommand_003 start");
    std::vector<ApcfCommand> cmds;
    BleScanFilterImpl filter;
    Uuid uuid;
    filter.SetServiceUuid(uuid);
    bleScanFilter->SetApcfCommand(filter, cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_SetApcfCommand_003 end");
}

/**
 * @tc.name: ble_scan_filter_SetApcfCommand_004
 * @tc.desc: ble_scan_filter_SetApcfCommand_004
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SetApcfCommand_004, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SetApcfCommand_004 start");
    std::vector<ApcfCommand> cmds;
    BleScanFilterImpl filter;
    Uuid uuid;
    filter.SetServiceSolicitationUuid(uuid);
    bleScanFilter->SetApcfCommand(filter, cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_SetApcfCommand_004 end");
}

/**
 * @tc.name: ble_scan_filter_SetApcfCommand_005
 * @tc.desc: ble_scan_filter_SetApcfCommand_005
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SetApcfCommand_005, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SetApcfCommand_005 start");
    std::vector<ApcfCommand> cmds;
    BleScanFilterImpl filter;
    std::string testdata = "testdata";
    filter.SetName(testdata);
    bleScanFilter->SetApcfCommand(filter, cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_SetApcfCommand_005 end");
}

/**
 * @tc.name: ble_scan_filter_SetApcfCommand_006
 * @tc.desc: ble_scan_filter_SetApcfCommand_006
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SetApcfCommand_006, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SetApcfCommand_006 start");
    std::vector<ApcfCommand> cmds;
    BleScanFilterImpl filter;
    std::vector<uint8_t> manufactureData;
    manufactureData.push_back(1);
    filter.SetManufactureData(manufactureData);
    bleScanFilter->SetApcfCommand(filter, cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_SetApcfCommand_006 end");
}

/**
 * @tc.name: ble_scan_filter_SetApcfCommand_007
 * @tc.desc: ble_scan_filter_SetApcfCommand_007
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SetApcfCommand_007, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SetApcfCommand_007 start");
    std::vector<ApcfCommand> cmds;
    BleScanFilterImpl filter;
    std::vector<uint8_t> serviceData;
    serviceData.push_back(1);
    filter.SetServiceData(serviceData);
    bleScanFilter->SetApcfCommand(filter, cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_SetApcfCommand_007 end");
}

/**
 * @tc.name: ble_scan_filter_SetApcfCommand_008
 * @tc.desc: ble_scan_filter_SetApcfCommand_008
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_SetApcfCommand_008, TestSize.Level1)
{
    HILOGI("ble_scan_filter_SetApcfCommand_008 start");
    std::vector<ApcfCommand> cmds;
    BleScanFilterImpl filter;
    std::vector<uint8_t> irk = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
        0xFF, 0x11};
    filter.SetIrk(irk);
    filter.SetDeviceId("11:22:33:44:55:66");
    filter.SetRawAddressType(RawAddressType::RANDOM_ADDRESS);
    bleScanFilter->SetApcfCommand(filter, cmds);
    filter.SetRawAddressType(RawAddressType::PUBLIC_ADDRESS);
    bleScanFilter->SetApcfCommand(filter, cmds);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_SetApcfCommand_008 end");
}

/**
 * @tc.name: ble_scan_filter_AddScanFilter_001
 * @tc.desc: ble_scan_filter_AddScanFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_AddScanFilter_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_AddScanFilter_001 start");
    BleScanFilterImpl filter;
    int filterIndex = 0;
    EXPECT_FALSE(bleScanFilter->AddScanFilter(filterIndex, filter, 1, 1, 10));
    EXPECT_FALSE(bleScanFilter->AddScanFilter(filterIndex, filter, 1, 2, 10));
    EXPECT_FALSE(bleScanFilter->AddScanFilter(filterIndex, filter, 0, 1, 10));
    HILOGI("ble_scan_filter_AddScanFilter_001 end");
}

/**
 * @tc.name: ble_scan_filter_CheckFilter_001
 * @tc.desc: ble_scan_filter_CheckFilter_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_CheckFilter_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_CheckFilter_001 start");
    BleScanFilterImpl filter;
    std::string testdata = "testdata";
    filter.SetDeviceId(testdata);
    Uuid uuid;
    filter.SetServiceUuid(uuid);
    filter.SetServiceSolicitationUuid(uuid);
    filter.SetName(testdata);
    std::vector<uint8_t> manufactureData;
    manufactureData.push_back(1);
    filter.SetManufactureData(manufactureData);
    std::vector<uint8_t> serviceData;
    serviceData.push_back(1);
    filter.SetServiceData(serviceData);
    bleScanFilter->CheckFilter(filter);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_CheckFilter_001 end");
}

/**
 * @tc.name: ble_scan_filter_CheckFilter_002
 * @tc.desc: ble_scan_filter_CheckFilter_002
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_CheckFilter_002, TestSize.Level1)
{
    HILOGI("ble_scan_filter_CheckFilter_002 start");
    BleScanFilterImpl filter;
    std::string testdata = "testdata";
    filter.SetDeviceId(testdata);
    Uuid uuid;
    filter.SetServiceUuid(uuid);
    filter.SetServiceUuidMask(uuid);
    filter.SetServiceSolicitationUuid(uuid);
    filter.SetServiceSolicitationUuidMask(uuid);
    filter.SetName(testdata);
    std::vector<uint8_t> manufactureData;
    manufactureData.push_back(1);
    filter.SetManufactureData(manufactureData);
    filter.SetManufactureDataMask(manufactureData);
    std::vector<uint8_t> serviceData;
    serviceData.push_back(1);
    filter.SetServiceData(serviceData);
    filter.SetServiceDataMask(serviceData);
    bleScanFilter->CheckFilter(filter);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_CheckFilter_002 end");
}

/**
 * @tc.name: ble_scan_filter_CheckFilter_003
 * @tc.desc: ble_scan_filter_CheckFilter_003
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_CheckFilter_003, TestSize.Level1)
{
    HILOGI("ble_scan_filter_CheckFilter_003 start");
    BleScanFilterImpl filter;
    std::string testdata = "testdata";
    filter.SetDeviceId(testdata);
    Uuid uuid;
    filter.SetServiceUuidMask(uuid);
    filter.SetServiceSolicitationUuidMask(uuid);
    filter.SetName(testdata);
    bleScanFilter->CheckFilter(filter);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_CheckFilter_003 end");
}

/**
 * @tc.name: ble_scan_filter_CheckFilter_004
 * @tc.desc: ble_scan_filter_CheckFilter_004
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_CheckFilter_004, TestSize.Level1)
{
    HILOGI("ble_scan_filter_CheckFilter_004 start");
    BleScanFilterImpl filter;
    std::string testdata = "testdata";
    filter.SetDeviceId(testdata);
    filter.SetName(testdata);
    std::vector<uint8_t> manufactureData;
    manufactureData.push_back(1);
    filter.SetManufactureDataMask(manufactureData);
    std::vector<uint8_t> serviceData;
    serviceData.push_back(1);
    filter.SetServiceDataMask(serviceData);
    bleScanFilter->CheckFilter(filter);
    EXPECT_TRUE(bleScanFilter != nullptr);
    HILOGI("ble_scan_filter_CheckFilter_004 end");
}

/**
 * @tc.name: ble_scan_filter_AddScanFilters_001
 * @tc.desc: ble_scan_filter_AddScanFilters_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_AddScanFilters_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_AddScanFilters_001 start");
    std::vector<BleScanFilterImpl> filters;
    EXPECT_EQ(bleScanFilter->AddScanFilters(filters, 1, 1, 10), -31);
    HILOGI("ble_scan_filter_AddScanFilters_001 end");
}

/**
 * @tc.name: ble_scan_filter_AddScanFilters_002
 * @tc.desc: ble_scan_filter_AddScanFilters_002
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_AddScanFilters_002, TestSize.Level1)
{
    HILOGI("ble_scan_filter_AddScanFilters_002 start");
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    EXPECT_EQ(bleScanFilter->AddScanFilters(filters, 1, 1, 10), -1);
    HILOGI("ble_scan_filter_AddScanFilters_002 end");
}

/**
 * @tc.name: ble_scan_filter_AddScanFilters_003
 * @tc.desc: ble_scan_filter_AddScanFilters_003
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_AddScanFilters_003, TestSize.Level1)
{
    HILOGI("ble_scan_filter_AddScanFilters_003 start");
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    EXPECT_EQ(bleScanFilter->AddScanFilters(filters, 1, 1, 10), -1);
    EXPECT_EQ(bleScanFilter->AddScanFilters(filters, 1, 1, 10), -1);
    HILOGI("ble_scan_filter_AddScanFilters_003 end");
}

/**
 * @tc.name: ble_scan_filter_GetOne_001
 * @tc.desc: ble_scan_filter_GetOne_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetOne_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetOne_001 start");
    BleScanFilter::FilterIndex* filterIndex = BleScanFilter::FilterIndex::GetInstance();
    for (int i = 0; i < 40; i++) {
        filterIndex->GetOne();
    }
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    EXPECT_EQ(bleScanFilter->AddScanFilters(filters, 1, 1, 10), Bluetooth::BT_ERR_BLE_SCAN_NO_RESOURCE);
    HILOGI("ble_scan_filter_GetOne_001 end");
}

/**
 * @tc.name: ble_scan_filter_AddScanFilters_004
 * @tc.desc: ble_scan_filter_AddScanFilters_004
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_AddScanFilters_004, TestSize.Level1)
{
    HILOGI("ble_scan_filter_AddScanFilters_003 start");
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    filters.push_back(filter);
    EXPECT_EQ(bleScanFilter->AddScanFilters(filters, 1, 1, 10), Bluetooth::BT_ERR_BLE_SCAN_NO_RESOURCE);
    HILOGI("ble_scan_filter_AddScanFilters_003 end");
}

/**
 * @tc.name: ble_scan_filter_RemoveScanFilters_001
 * @tc.desc: ble_scan_filter_RemoveScanFilters_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_RemoveScanFilters_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_RemoveScanFilters_001 start");
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    int ret = bleScanFilter->RemoveScanFilters(filters);
    EXPECT_EQ(ret, 0);

    filter.SetFiltIndex(0);
    filters.push_back(filter);
    BleScanFilterImpl filter1;
    filter.SetFiltIndex(1);
    filters.push_back(filter1);
    ret = bleScanFilter->RemoveScanFilters(filters);
    EXPECT_EQ(ret, 0);

    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    ret = bleScanFilter->RemoveScanFilters(filters);
    EXPECT_EQ(ret, 0);

    bleScanFilter->filterImpl_.Insert(0, filter);
    bleScanFilter->filterImpl_.Insert(1, filter1);
    ret = bleScanFilter->RemoveScanFilters(filters);
    EXPECT_EQ(ret, 1);
    HILOGI("ble_scan_filter_RemoveScanFilters_001 end");
}

/**
 * @tc.name: ble_scan_filter_AppendScanFilters_001
 * @tc.desc: ble_scan_filter_AppendScanFilters_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_AppendScanFilters_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_AppendScanFilters_001 start");
    BleScanFilterImpl filter;
    std::vector<BleScanFilterImpl> filters;
    int ret = bleScanFilter->AppendScanFilters(filters, 1, 1, 10);
    EXPECT_EQ(ret, 0);

    filters.push_back(filter);
    bleScanFilter->filterImpl_.Insert(0, filter);
    ret = bleScanFilter->AppendScanFilters(filters, 1, 1, 10);
    EXPECT_EQ(ret, 0);

    bleScanFilter->filterImpl_.Clear();
    HILOGI("ble_scan_filter_AppendScanFilters_001 end");
}

/**
 * @tc.name: ble_scan_filter_GetRssiThres_001
 * @tc.desc: ble_scan_filter_GetRssiThres_001
 * @tc.type: FUNC
 */
HWTEST_F(BleScanFilterTest, ble_scan_filter_GetRssiThres_001, TestSize.Level1)
{
    HILOGI("ble_scan_filter_GetRssiThres_001 start");
    auto [highRssi, lowRssi] = bleScanFilter->GetRssiThres(1, false);
    EXPECT_EQ(highRssi, BleScanFilter::RSSI_HIGH_THRESH);
    EXPECT_EQ(lowRssi, BleScanFilter::RSSI_HIGH_THRESH);
    HILOGI("ble_scan_filter_GetRssiThres_001 end");
}


}  // namespace bluetooth
}  // namespace OHOS