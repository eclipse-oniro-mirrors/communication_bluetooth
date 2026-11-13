/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <vector>
#include <raw_address.h>
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "foundation/communication/bluetooth/frameworks/inner/ipc/common/raw_address.h"
#include "bluetooth_channel_rssi_manager.h"
#include "bluetooth_hw_interface.h"
#include "bluetooth_connection_manager.h"
#include "datetime_ex.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
static OHOS::Bluetooth::BluetoothHost *host_;
constexpr int TIME = 2;
constexpr int64_t EMPTY_TIMESTAMP = 0;
constexpr int64_t MIN_RSSI_REFRESH_TIME = 61;
constexpr int64_t MAX_RSSI_VALID_TIME = 61;

struct UnionHisiJamChrInfoS {
    uint32_t rssiIdx[BT_ACL_RSSI_REP_SIZE]; // the index of report rssi uint32_t
    int64_t actualRssiValue[BT_ACL_RSSI_REP_SIZE]; // the actual rssi value of some channel int64_t
    int64_t timestamp; // the timestamp when chip report rssi to bth
};

UnionHisiJamChrInfoS g_unionHisiJamChrInfo = {
    .rssiIdx = { 0 },
    .actualRssiValue = { 0 },
    .timestamp = 0,
};

static const BthwifCallbacks* g_btAdapterhwifCallbacks = nullptr;
static bool g_rssiCmdSendFlag = false;
static bool g_channelScanFlag = false;

static bt_status_t HwIfInitMock(BthwifCallbacks* callbacks)
{
    g_btAdapterhwifCallbacks = callbacks;
    return BT_STATUS_SUCCESS;
}

static int HwSendChannelRssiReadCmd(const STACK::RawAddress &addr, uint8_t transport)
{
    g_rssiCmdSendFlag = true;
    return BT_STATUS_SUCCESS;
}

static int HwGetUnionHisiJamChrInfo(std::vector<uint32_t> &rssiIdx, std::vector<int64_t> &actualRssiValue,
    int64_t &timestamp)
{
    std::copy(g_unionHisiJamChrInfo.rssiIdx, g_unionHisiJamChrInfo.rssiIdx + BT_ACL_RSSI_REP_SIZE, rssiIdx.begin());
    std::copy(g_unionHisiJamChrInfo.actualRssiValue, g_unionHisiJamChrInfo.actualRssiValue + BT_ACL_RSSI_REP_SIZE,
        actualRssiValue.begin());
    timestamp = g_unionHisiJamChrInfo.timestamp;
    return BT_STATUS_SUCCESS;
}

static void HwSendChannelScanEnableCmd(bool enable, uint32_t interval)
{
    g_channelScanFlag = enable;
    return;
}

void SetUnionHisiJamChrInfoS(const std::vector<uint32_t> &rssiIdx, const std::vector<int64_t> &actualRssiValue,
    int64_t timestamp)
{
    if (rssiIdx.size() > static_cast<size_t>(BT_ACL_RSSI_REP_SIZE)) {
        HILOGE("invalid rssiIdx input length: %{public}lu", rssiIdx.size());
        return;
    }
    if (actualRssiValue.size() > static_cast<size_t>(BT_ACL_RSSI_REP_SIZE)) {
        HILOGE("invalid actualRssiValue input length: %{public}lu", actualRssiValue.size());
        return;
    }
    std::copy(rssiIdx.begin(), rssiIdx.end(), g_unionHisiJamChrInfo.rssiIdx);
    std::copy(actualRssiValue.begin(), actualRssiValue.end(), g_unionHisiJamChrInfo.actualRssiValue);
    g_unionHisiJamChrInfo.timestamp = timestamp;
}

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwSendChannelRssiReadCmd = HwSendChannelRssiReadCmd,
    .hwGetUnionHisiJamChrInfo = HwGetUnionHisiJamChrInfo,
    .hwSendChannelScanEnableCmd = HwSendChannelScanEnableCmd,
};

class BluetoothChannelRssiManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {
        HILOGI("SetUpTestCase start");
        host_ = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    }
    static void TearDownTestCase() {
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(nullptr);
    }
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number: bluetooth_channel_manager_test_001
 * @tc.name: ShouldRefuseSndCmdTooFrequently
 * @tc.desc: 
 */
HWTEST_F(BluetoothChannelRssiManagerTest, bluetooth_channel_manager_test_001, TestSize.Level1)
{
    HILOGI("bluetooth_channel_manager_test_001 start");
    auto btChannelRssiMgr = BluetoothChannelRssiManager::GetInstance();
    int timestamp = GetSecondsSince1970ToNow();
    EXPECT_TRUE(btChannelRssiMgr->ShouldRefuseSndCmdTooFrequently(timestamp));
    EXPECT_FALSE(btChannelRssiMgr->ShouldRefuseSndCmdTooFrequently(timestamp - MIN_RSSI_REFRESH_TIME));
    HILOGI("bluetooth_channel_manager_test_001 end");
}

/**
 * @tc.number: bluetooth_channel_manager_test_002
 * @tc.name: ShouldRefuseReadRssiOutOfDate
 * @tc.desc: 
 */
HWTEST_F(BluetoothChannelRssiManagerTest, bluetooth_channel_manager_test_002, TestSize.Level1)
{
    HILOGI("bluetooth_channel_manager_test_002 start");
    auto btChannelRssiMgr = BluetoothChannelRssiManager::GetInstance();
    int timestamp = GetSecondsSince1970ToNow();
    EXPECT_FALSE(btChannelRssiMgr->ShouldRefuseReadRssiOutOfDate(timestamp));
    EXPECT_TRUE(btChannelRssiMgr->ShouldRefuseReadRssiOutOfDate(timestamp - MAX_RSSI_VALID_TIME));
    HILOGI("bluetooth_channel_manager_test_002 end");
}

/**
 * @tc.number: bluetooth_channel_manager_test_003
 * @tc.name: SendChannelRssiReadCommand
 * @tc.desc: 
 */
HWTEST_F(BluetoothChannelRssiManagerTest, bluetooth_channel_manager_test_003, TestSize.Level1)
{
    HILOGI("bluetooth_channel_manager_test_003 start");
    auto btChannelRssiMgr = BluetoothChannelRssiManager::GetInstance();

    // BluetoothHwInterface null case
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(nullptr);
    btChannelRssiMgr->SendChannelRssiReadCommand();
    EXPECT_FALSE(g_rssiCmdSendFlag);
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);

    // send channel rssi read command too frequently
    std::vector<uint32_t> rssiIdx = std::vector<uint32_t>(BT_ACL_RSSI_REP_SIZE, 0);
    std::vector<int64_t> actualRssiValue = std::vector<int64_t>(BT_ACL_RSSI_REP_SIZE, 0);
    int64_t timestamp = GetSecondsSince1970ToNow();
    SetUnionHisiJamChrInfoS(rssiIdx, actualRssiValue, timestamp);
    btChannelRssiMgr->SendChannelRssiReadCommand();
    EXPECT_FALSE(g_rssiCmdSendFlag);

    // send cmd with no br links
    SetUnionHisiJamChrInfoS(rssiIdx, actualRssiValue, EMPTY_TIMESTAMP + 1);
    btChannelRssiMgr->SendChannelRssiReadCommand();
    EXPECT_FALSE(g_rssiCmdSendFlag);

    // send cmd with br links
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->AddAclLinks("11:22:33:44:55:66", 0, "");
    btChannelRssiMgr->SendChannelRssiReadCommand();
    EXPECT_TRUE(g_rssiCmdSendFlag);
    connectionManager->RemoveAclLinks("11:22:33:44:55:66", 0);
    g_rssiCmdSendFlag = false;
    HILOGI("bluetooth_channel_manager_test_003 end");
}

/**
 * @tc.number: bluetooth_channel_manager_test_004
 * @tc.name: GetChannelRssiInfo
 * @tc.desc: 
 */
HWTEST_F(BluetoothChannelRssiManagerTest, bluetooth_channel_manager_test_004, TestSize.Level1)
{
    HILOGI("bluetooth_channel_manager_test_004 start");
    auto btChannelRssiMgr = BluetoothChannelRssiManager::GetInstance();
    std::vector<uint32_t> idxInfo = {};
    std::vector<int64_t> channelRssiInfo = {};
    int64_t timestamp = 0;

    // BluetoothHwInterface null case
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(nullptr);
    int result = btChannelRssiMgr->GetChannelRssiInfo(idxInfo, channelRssiInfo, timestamp);
    EXPECT_EQ(result, BT_STATUS_FAIL);
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);

    // get rssi when timestamp is empty
    std::vector<uint32_t> rssiIdx = std::vector<uint32_t>(BT_ACL_RSSI_REP_SIZE, 0);
    std::vector<int64_t> actualRssiValue = std::vector<int64_t>(BT_ACL_RSSI_REP_SIZE, 0);
    SetUnionHisiJamChrInfoS(rssiIdx, actualRssiValue, EMPTY_TIMESTAMP);
    result = btChannelRssiMgr->GetChannelRssiInfo(idxInfo, channelRssiInfo, timestamp);
    EXPECT_EQ(result, BT_STATUS_FAIL);

    // get rssi out of date
    SetUnionHisiJamChrInfoS(rssiIdx, actualRssiValue, EMPTY_TIMESTAMP + 1);
    result = btChannelRssiMgr->GetChannelRssiInfo(idxInfo, channelRssiInfo, timestamp);
    EXPECT_EQ(result, BT_STATUS_FAIL);

    // get valid rssi
    int64_t currentTime = GetSecondsSince1970ToNow();
    SetUnionHisiJamChrInfoS(rssiIdx, actualRssiValue, currentTime);
    result = btChannelRssiMgr->GetChannelRssiInfo(idxInfo, channelRssiInfo, timestamp);
    EXPECT_EQ(result, BT_STATUS_SUCCESS);
    HILOGI("bluetooth_channel_manager_test_004 end");
}

/**
 * @tc.number: bluetooth_channel_manager_test_005
 * @tc.name: GetChannelRssiInfo
 * @tc.desc: 
 */
HWTEST_F(BluetoothChannelRssiManagerTest, bluetooth_channel_manager_test_005, TestSize.Level1)
{
    HILOGI("bluetooth_channel_manager_test_005 start");
    auto btChannelRssiMgr = BluetoothChannelRssiManager::GetInstance();
 
    btChannelRssiMgr->SetBtChannelScan(true, CHANNEL_SCAN_INTERVAL_100MS);
    EXPECT_TRUE(g_channelScanFlag);
 
    btChannelRssiMgr->SetBtChannelScan(false, CHANNEL_SCAN_INTERVAL_BYPASS);
    EXPECT_FALSE(g_channelScanFlag);
    HILOGI("bluetooth_channel_manager_test_005 end");
}
}
}