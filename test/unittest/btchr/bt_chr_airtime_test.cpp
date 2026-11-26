/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bluetooth_log.h"
#include "bt_chr_airtime.h"
#include "bt_chr_base.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
const std::string AIRTIME_TEST_ADDR = "11:22:33:44:55:66";
const std::string AIRTIME_TEST_ADDR_ALT = "11:22:33:44:55:67";
const std::string AIRTIME_TEST_PKG = "com.test.airtime";

namespace {
constexpr int32_t AIRTIME_TEST_STATUS_OK = 0;
constexpr int32_t AIRTIME_TEST_STATUS_FAIL = 1;
constexpr int32_t AIRTIME_TEST_HCI_STATUS_OK = 0;
constexpr int32_t AIRTIME_TEST_HCI_STATUS_FAIL = 1;
constexpr int32_t AIRTIME_TEST_FAIL_SCENE_NONE = 0;
constexpr int32_t AIRTIME_TEST_FAIL_SCENE_TEST = 1;
constexpr int32_t AIRTIME_TEST_ZERO = 0;
constexpr int32_t AIRTIME_TEST_INVALID_PARAM = -1;
constexpr int32_t AIRTIME_TEST_CONN_INTERVAL = 24;
constexpr int32_t AIRTIME_TEST_GATT_CONN_INTERVAL = 12;
constexpr int32_t AIRTIME_TEST_CODEC_INDEX = 2;
constexpr int32_t AIRTIME_TEST_BITRATE_BPS = 320000;
constexpr int32_t AIRTIME_TEST_INVALID_CODEC_INDEX = -1;
constexpr int32_t AIRTIME_TEST_CODEC_INDEX_UNKNOWN = 0xFFFF;
constexpr int32_t AIRTIME_TEST_ROLE = 0;
constexpr int32_t AIRTIME_TEST_SCO_CODEC = 1;
constexpr int32_t AIRTIME_TEST_SCO_PKT_TYPE = 0;
constexpr int32_t AIRTIME_TEST_SCO_TSCO = 0;
constexpr int32_t AIRTIME_TEST_BR_HID_REPORT_LEN = 16;
constexpr int32_t AIRTIME_TEST_HOGP_REPORT_LEN = 8;
constexpr int32_t AIRTIME_TEST_ZERO_LEN = 0;
constexpr int32_t AIRTIME_TEST_INVALID_LEN = -1;
constexpr int32_t AIRTIME_TEST_NOTIF_COUNT = 1;
constexpr int32_t AIRTIME_TEST_SOCKET_HANDLE = 10;
constexpr int32_t AIRTIME_TEST_SOCKET_CHANNEL = 3;
constexpr int32_t AIRTIME_TEST_SOCKET_SERVICE_TYPE = 1;
constexpr int32_t AIRTIME_TEST_SOCKET_DATA_LEN = 32;
constexpr int32_t AIRTIME_TEST_MIN_PAYLOAD_BPS = 0;
constexpr int32_t AIRTIME_TEST_GATT_TX_BYTES = 20;
constexpr int32_t AIRTIME_TEST_GATT_TX_BYTES_MAX = std::numeric_limits<int32_t>::max();
constexpr int32_t AIRTIME_TEST_GATT_RX_BYTES = 10;
constexpr int32_t AIRTIME_TEST_SCAN_WINDOW = 100;
constexpr int32_t AIRTIME_TEST_SCAN_INTERVAL = 200;
constexpr int32_t AIRTIME_TEST_SCAN_TYPE_ACTIVE = 1;
constexpr int32_t AIRTIME_TEST_SCAN_WINDOW_FAIL = 50;
constexpr int32_t AIRTIME_TEST_SCAN_INTERVAL_FAIL = 100;
constexpr int32_t AIRTIME_TEST_SCAN_TYPE_PASSIVE = 0;
constexpr int32_t AIRTIME_TEST_SCAN_FAIL_STAT = 1;
constexpr int32_t AIRTIME_TEST_ADV_ID = 1;
constexpr int32_t AIRTIME_TEST_ADV_ID_ACTIVE_ONLY = 2;
constexpr int32_t AIRTIME_TEST_ADV_INTERVAL = 100;
constexpr int32_t AIRTIME_TEST_ADV_TYPE = 2;
constexpr int32_t AIRTIME_TEST_ADV_PAYLOAD_LEN = 20;
constexpr int32_t AIRTIME_TEST_DISC_TYPE_INQUIRY = 1;
constexpr int32_t AIRTIME_TEST_DISC_TYPE_PAGE = 2;
constexpr int32_t AIRTIME_TEST_DISC_TYPE_INQUIRY_SCAN = 3;
constexpr int32_t AIRTIME_TEST_DISC_TYPE_PAGE_SCAN = 4;
constexpr int32_t AIRTIME_TEST_DISC_WINDOW = 10;
constexpr int32_t AIRTIME_TEST_DISC_INTERVAL = 20;
constexpr int32_t AIRTIME_TEST_DISC_FAIL_NONE = 0;
constexpr int32_t AIRTIME_TEST_DISC_WINDOW_A = 1;
constexpr int32_t AIRTIME_TEST_DISC_INTERVAL_A = 2;
constexpr int32_t AIRTIME_TEST_DISC_WINDOW_B = 3;
constexpr int32_t AIRTIME_TEST_DISC_INTERVAL_B = 4;
constexpr size_t AIRTIME_TEST_DISC_FLUSH_EVENT_COUNT = 2;
constexpr int32_t AIRTIME_TEST_SCO_PKT_TYPE_SET = 2;
constexpr int32_t AIRTIME_TEST_SCO_TSCO_SET = 7500;
constexpr int32_t AIRTIME_TEST_SCO_FAIL_SCENE_PEER_DISCONNECT = 7;
constexpr int32_t AIRTIME_TEST_A2DP_BITRATE_PARTIAL = 100000;
constexpr int32_t AIRTIME_TEST_A2DP_BITRATE_TOO_HIGH = 3000000;
constexpr int32_t AIRTIME_TEST_A2DP_BITRATE_NEGATIVE = -100000;
constexpr int32_t AIRTIME_TEST_DISC_TYPE_INVALID = 5;
constexpr int32_t AIRTIME_TEST_DISC_WINDOW_ALT = 11;
constexpr int32_t AIRTIME_TEST_DISC_INTERVAL_ALT = 21;
constexpr size_t AIRTIME_TEST_EVENT_COUNT_ONE = 1;
constexpr size_t AIRTIME_TEST_EVENT_COUNT_TWO = 2;

struct CapturedAirtimeEvent {
    std::string eventName;
    std::vector<BtChrEventParam> params;
};

std::vector<CapturedAirtimeEvent> g_capturedEvents;

void CaptureAirtimeEvent(const std::string &eventName, const std::vector<BtChrEventParam> &params)
{
    CapturedAirtimeEvent event;
    event.eventName = eventName;
    event.params = params;
    g_capturedEvents.push_back(event);
}

const CapturedAirtimeEvent *FindEvent(const std::string &eventName)
{
    for (auto it = g_capturedEvents.rbegin(); it != g_capturedEvents.rend(); ++it) {
        if (it->eventName == eventName) {
            return &(*it);
        }
    }
    return nullptr;
}

const BtChrEventParam *FindParam(const CapturedAirtimeEvent &event, const std::string &name)
{
    for (const auto &param : event.params) {
        if (param.name == name) {
            return &param;
        }
    }
    return nullptr;
}

int32_t GetIntParam(const CapturedAirtimeEvent &event, const std::string &name)
{
    const BtChrEventParam *param = FindParam(event, name);
    return param == nullptr ? AIRTIME_TEST_INVALID_PARAM : param->i32Value;
}

std::string GetStrParam(const CapturedAirtimeEvent &event, const std::string &name)
{
    const BtChrEventParam *param = FindParam(event, name);
    return param == nullptr ? "" : param->strValue;
}
}  // namespace

void BtChrUeEventWrite(const std::string &eventName, std::vector<BtChrEventParam> &params)
{
    CaptureAirtimeEvent(eventName, params);
}

class BtChrAirtimeTest : public testing::Test {
public:
    BtChrAirtimeTest() = default;
    ~BtChrAirtimeTest() = default;
    void SetUp() override
    {
        g_capturedEvents.clear();
    }
    void TearDown() override
    {
        BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);
        BtChr2g4A2dpStop(AIRTIME_TEST_ADDR_ALT);
        BtChr2g4ScoStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_HCI_STATUS_OK, AIRTIME_TEST_FAIL_SCENE_NONE);
        BtChr2g4BrHidStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
        BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
        BtChr2g4BrSocketStop(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_STATUS_OK);
        BtChr2g4BleGattStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
        BtChr2g4BleAdvStop(AIRTIME_TEST_ADV_ID);
        BtChr2g4BleAdvStop(AIRTIME_TEST_ADV_ID_ACTIVE_ONLY);
        BtChr2g4BleScanFlush();
        BtChr2g4DiscFlushAll();
        g_capturedEvents.clear();
    }
};

/**
 * @tc.name: Test_BtChr2g4BleConnInterval_001
 * @tc.desc: Verify connection interval is used by the HOGP airtime report.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4BleConnInterval_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4BleConnInterval_001 enter");
    BtChr2g4BleConnInterval(AIRTIME_TEST_ADDR, AIRTIME_TEST_CONN_INTERVAL);
    BtChr2g4BleHogpStart(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_HOGP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "CONN_INTERVAL"), AIRTIME_TEST_CONN_INTERVAL);
    HILOGI("Test_BtChr2g4BleConnInterval_001 end");
}

/**
 * @tc.name: Test_BtChr2g4A2dpAirtime_001
 * @tc.desc: Verify A2DP session report fields after start/active/stop.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4A2dpAirtime_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4A2dpAirtime_001 enter");
    BtChr2g4A2dpStart(AIRTIME_TEST_ADDR);
    BtChr2g4A2dpSinkCodec(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_BITRATE_BPS);
    BtChr2g4A2dpActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_BITRATE_BPS,
        AIRTIME_TEST_ROLE);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_GT(GetIntParam(*event, "SESSION_ID"), AIRTIME_TEST_ZERO);
    EXPECT_NE(GetStrParam(*event, "START_TS"), "");
    EXPECT_NE(GetStrParam(*event, "ACTIVE_TS"), "");
    EXPECT_NE(GetStrParam(*event, "STOP_TS"), "");
    EXPECT_EQ(GetIntParam(*event, "CODEC_INDEX"), AIRTIME_TEST_CODEC_INDEX);
    EXPECT_EQ(GetIntParam(*event, "BITRATE_BPS"), AIRTIME_TEST_BITRATE_BPS);
    HILOGI("Test_BtChr2g4A2dpAirtime_001 end");
}

/**
 * @tc.name: Test_BtChr2g4A2dpFail_001
 * @tc.desc: Verify A2DP failure event is reported without a prior start.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4A2dpFail_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4A2dpFail_001 enter");
    BtChr2g4A2dpFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    EXPECT_EQ(GetIntParam(*event, "STATUS"), AIRTIME_TEST_STATUS_FAIL);
    EXPECT_EQ(GetIntParam(*event, "FAIL_SCENE"), AIRTIME_TEST_FAIL_SCENE_TEST);
    HILOGI("Test_BtChr2g4A2dpFail_001 end");
}

/**
 * @tc.name: Test_BtChr2g4ScoAirtime_001
 * @tc.desc: Verify SCO session report fields after start/active/stop.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4ScoAirtime_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4ScoAirtime_001 enter");
    BtChr2g4ScoStart(AIRTIME_TEST_ADDR);
    BtChr2g4ScoActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_SCO_CODEC, AIRTIME_TEST_SCO_PKT_TYPE,
        AIRTIME_TEST_SCO_TSCO);
    BtChr2g4ScoStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_HCI_STATUS_OK, AIRTIME_TEST_FAIL_SCENE_NONE);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_SCO_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_GT(GetIntParam(*event, "SESSION_ID"), AIRTIME_TEST_ZERO);
    EXPECT_NE(GetStrParam(*event, "ACTIVE_TS"), "");
    EXPECT_NE(GetStrParam(*event, "STOP_TS"), "");
    EXPECT_EQ(GetIntParam(*event, "SCO_CODEC"), AIRTIME_TEST_SCO_CODEC);
    HILOGI("Test_BtChr2g4ScoAirtime_001 end");
}

/**
 * @tc.name: Test_BtChr2g4ScoFail_001
 * @tc.desc: Verify SCO failure event is reported without a prior start.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4ScoFail_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4ScoFail_001 enter");
    BtChr2g4ScoFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_HCI_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_SCO_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    EXPECT_EQ(GetIntParam(*event, "HCI_STATUS"), AIRTIME_TEST_HCI_STATUS_FAIL);
    EXPECT_EQ(GetIntParam(*event, "FAIL_SCENE"), AIRTIME_TEST_FAIL_SCENE_TEST);
    HILOGI("Test_BtChr2g4ScoFail_001 end");
}

/**
 * @tc.name: Test_BtChr2g4HidAirtime_001
 * @tc.desc: Verify BR HID and BLE HOGP report data and zero/negative length protection.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4HidAirtime_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4HidAirtime_001 enter");
    BtChr2g4BrHidStart(AIRTIME_TEST_ADDR);
    BtChr2g4BrHidActive(AIRTIME_TEST_ADDR);
    BtChr2g4BrHidReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_BR_HID_REPORT_LEN);
    BtChr2g4BrHidReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_ZERO_LEN);
    BtChr2g4BrHidStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BR_HID_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "REPORT_SIZE_B"), AIRTIME_TEST_BR_HID_REPORT_LEN);

    BtChr2g4BleHogpStart(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_HOGP_REPORT_LEN);
    BtChr2g4BleHogpReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_INVALID_LEN);
    BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);

    event = FindEvent(CHR_UE_BT_2G4_BLE_HOGP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "NOTIF_COUNT"), AIRTIME_TEST_NOTIF_COUNT);
    EXPECT_EQ(GetIntParam(*event, "REPORT_SIZE_B"), AIRTIME_TEST_HOGP_REPORT_LEN);
    HILOGI("Test_BtChr2g4HidAirtime_001 end");
}

/**
 * @tc.name: Test_BtChr2g4SocketAirtime_001
 * @tc.desc: Verify RFCOMM socket report fields after data and stop.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4SocketAirtime_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4SocketAirtime_001 enter");
    const int32_t handle = AIRTIME_TEST_SOCKET_HANDLE;
    BtChr2g4BrSocketStart(handle, AIRTIME_TEST_ADDR, AIRTIME_TEST_SOCKET_CHANNEL,
        AIRTIME_TEST_SOCKET_SERVICE_TYPE);
    BtChr2g4BrSocketActive(handle, AIRTIME_TEST_ADDR);
    BtChr2g4BrSocketData(handle, AIRTIME_TEST_SOCKET_DATA_LEN);
    BtChr2g4BrSocketData(handle, AIRTIME_TEST_ZERO_LEN);
    BtChr2g4BrSocketStop(handle, AIRTIME_TEST_STATUS_OK);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BR_SOCKET_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "CHANNEL"), AIRTIME_TEST_SOCKET_CHANNEL);
    EXPECT_EQ(GetIntParam(*event, "SERVICE_TYPE"), AIRTIME_TEST_SOCKET_SERVICE_TYPE);
    EXPECT_GT(GetIntParam(*event, "PAYLOAD_BPS"), AIRTIME_TEST_MIN_PAYLOAD_BPS);
    HILOGI("Test_BtChr2g4SocketAirtime_001 end");
}

/**
 * @tc.name: Test_BtChr2g4SocketFail_001
 * @tc.desc: Verify RFCOMM socket failure event is reported without a prior start.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4SocketFail_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4SocketFail_001 enter");
    BtChr2g4BrSocketFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BR_SOCKET_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    EXPECT_EQ(GetIntParam(*event, "STATUS"), AIRTIME_TEST_STATUS_FAIL);
    EXPECT_EQ(GetIntParam(*event, "FAIL_SCENE"), AIRTIME_TEST_FAIL_SCENE_TEST);
    HILOGI("Test_BtChr2g4SocketFail_001 end");
}

/**
 * @tc.name: Test_BtChr2g4GattAirtime_001
 * @tc.desc: Verify GATT report fields, package name cache and connection interval.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4GattAirtime_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4GattAirtime_001 enter");
    BtChr2g4BleConnInterval(AIRTIME_TEST_ADDR, AIRTIME_TEST_GATT_CONN_INTERVAL);
    BtChr2g4BleGattPkgNameSet(AIRTIME_TEST_ADDR, AIRTIME_TEST_PKG);
    BtChr2g4BleGattStart(AIRTIME_TEST_ADDR, AIRTIME_TEST_PKG);
    BtChr2g4BleGattActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleGattTx(AIRTIME_TEST_ADDR, AIRTIME_TEST_GATT_TX_BYTES);
    BtChr2g4BleGattRx(AIRTIME_TEST_ADDR, AIRTIME_TEST_GATT_RX_BYTES);
    BtChr2g4BleGattStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_GATT_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "TX_BYTES"), AIRTIME_TEST_GATT_TX_BYTES);
    EXPECT_EQ(GetIntParam(*event, "RX_BYTES"), AIRTIME_TEST_GATT_RX_BYTES);
    EXPECT_EQ(GetIntParam(*event, "CONN_INTERVAL"), AIRTIME_TEST_GATT_CONN_INTERVAL);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), AIRTIME_TEST_PKG);
    HILOGI("Test_BtChr2g4GattAirtime_001 end");
}

/**
 * @tc.name: Test_BtChr2g4ScanAirtime_001
 * @tc.desc: Verify scan report fields and failure status.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4ScanAirtime_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4ScanAirtime_001 enter");
    BtChr2g4BleScanPkgNameSet(AIRTIME_TEST_PKG);
    BtChr2g4BleScanParams(AIRTIME_TEST_SCAN_WINDOW, AIRTIME_TEST_SCAN_INTERVAL,
        AIRTIME_TEST_SCAN_TYPE_ACTIVE, AIRTIME_TEST_PKG);
    BtChr2g4BleScanEnable(true, AIRTIME_TEST_PKG);
    BtChr2g4BleScanDisable();

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_SCAN_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "SCAN_WINDOW"), AIRTIME_TEST_SCAN_WINDOW);
    EXPECT_EQ(GetIntParam(*event, "SCAN_INTERVAL"), AIRTIME_TEST_SCAN_INTERVAL);
    EXPECT_EQ(GetIntParam(*event, "SCAN_TYPE"), AIRTIME_TEST_SCAN_TYPE_ACTIVE);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), AIRTIME_TEST_PKG);
    EXPECT_NE(GetStrParam(*event, "START_TS"), "");
    EXPECT_NE(GetStrParam(*event, "ACTIVE_TS"), "");

    BtChr2g4BleScanParams(AIRTIME_TEST_SCAN_WINDOW_FAIL, AIRTIME_TEST_SCAN_INTERVAL_FAIL,
        AIRTIME_TEST_SCAN_TYPE_PASSIVE, "");
    BtChr2g4BleScanEnable(false, "");
    event = FindEvent(CHR_UE_BT_2G4_BLE_SCAN_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "FAIL_STAT"), AIRTIME_TEST_SCAN_FAIL_STAT);
    HILOGI("Test_BtChr2g4ScanAirtime_001 end");
}

/**
 * @tc.name: Test_BtChr2g4AdvAirtime_001
 * @tc.desc: Verify advertising report fields after start/active/stop.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4AdvAirtime_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4AdvAirtime_001 enter");
    const int32_t advertiserId = AIRTIME_TEST_ADV_ID;
    BtChr2g4BleAdvPkgNameSet(advertiserId, AIRTIME_TEST_PKG);
    BtChr2g4BleAdvStart(advertiserId, AIRTIME_TEST_ADV_INTERVAL, AIRTIME_TEST_ADV_TYPE,
        AIRTIME_TEST_ADV_PAYLOAD_LEN, AIRTIME_TEST_PKG);
    BtChr2g4BleAdvActive(advertiserId);
    BtChr2g4BleAdvStop(advertiserId);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_ADV_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "ADV_INTERVAL"), AIRTIME_TEST_ADV_INTERVAL);
    EXPECT_EQ(GetIntParam(*event, "ADV_TYPE"), AIRTIME_TEST_ADV_TYPE);
    EXPECT_EQ(GetIntParam(*event, "PAYLOAD_LEN_B"), AIRTIME_TEST_ADV_PAYLOAD_LEN);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), AIRTIME_TEST_PKG);
    EXPECT_NE(GetStrParam(*event, "ACTIVE_TS"), "");
    HILOGI("Test_BtChr2g4AdvAirtime_001 end");
}

/**
 * @tc.name: Test_BtChr2g4AdvActiveWithoutStart_001
 * @tc.desc: Verify advertising active creates a session when Start was missed.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4AdvActiveWithoutStart_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4AdvActiveWithoutStart_001 enter");
    const int32_t advertiserId = AIRTIME_TEST_ADV_ID_ACTIVE_ONLY;
    BtChr2g4BleAdvPkgNameSet(advertiserId, AIRTIME_TEST_PKG);
    BtChr2g4BleAdvActive(advertiserId);
    BtChr2g4BleAdvStop(advertiserId);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_ADV_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "START_TS"), "");
    EXPECT_NE(GetStrParam(*event, "ACTIVE_TS"), "");
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), AIRTIME_TEST_PKG);
    HILOGI("Test_BtChr2g4AdvActiveWithoutStart_001 end");
}

/**
 * @tc.name: Test_BtChr2g4DiscAirtime_001
 * @tc.desc: Verify classic discovery report fields after start/active/stop.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4DiscAirtime_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4DiscAirtime_001 enter");
    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_WINDOW,
        AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscActive(AIRTIME_TEST_DISC_TYPE_INQUIRY);
    BtChr2g4DiscActive(AIRTIME_TEST_DISC_TYPE_INQUIRY);
    BtChr2g4DiscStop(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_FAIL_NONE);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_CLASSIC_DISCOVERY_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "PROCEDURE_TYPE"), AIRTIME_TEST_DISC_TYPE_INQUIRY);
    EXPECT_EQ(GetIntParam(*event, "SCAN_WINDOW_SLOTS"), AIRTIME_TEST_DISC_WINDOW);
    EXPECT_EQ(GetIntParam(*event, "SCAN_INTERVAL_SLOTS"), AIRTIME_TEST_DISC_INTERVAL);
    EXPECT_NE(GetStrParam(*event, "ACTIVE_TS"), "");
    HILOGI("Test_BtChr2g4DiscAirtime_001 end");
}

/**
 * @tc.name: Test_BtChr2g4DiscActiveWithoutStart_001
 * @tc.desc: Verify classic discovery active creates a session when Start was missed.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4DiscActiveWithoutStart_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4DiscActiveWithoutStart_001 enter");
    BtChr2g4DiscActive(AIRTIME_TEST_DISC_TYPE_INQUIRY_SCAN);
    BtChr2g4DiscStop(AIRTIME_TEST_DISC_TYPE_INQUIRY_SCAN, AIRTIME_TEST_DISC_FAIL_NONE);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_CLASSIC_DISCOVERY_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "PROCEDURE_TYPE"), AIRTIME_TEST_DISC_TYPE_INQUIRY_SCAN);
    EXPECT_NE(GetStrParam(*event, "START_TS"), "");
    EXPECT_NE(GetStrParam(*event, "ACTIVE_TS"), "");
    HILOGI("Test_BtChr2g4DiscActiveWithoutStart_001 end");
}

/**
 * @tc.name: Test_BtChr2g4DiscFlushAll_001
 * @tc.desc: Verify DiscFlushAll reports all pending discovery procedures.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4DiscFlushAll_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4DiscFlushAll_001 enter");
    g_capturedEvents.clear();
    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_WINDOW_A,
        AIRTIME_TEST_DISC_INTERVAL_A);
    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_PAGE, AIRTIME_TEST_DISC_WINDOW_B,
        AIRTIME_TEST_DISC_INTERVAL_B);
    BtChr2g4DiscFlushAll();
    EXPECT_EQ(g_capturedEvents.size(), AIRTIME_TEST_DISC_FLUSH_EVENT_COUNT);
    HILOGI("Test_BtChr2g4DiscFlushAll_001 end");
}

/**
 * @tc.name: Test_BtChr2g4InvalidInputs_001
 * @tc.desc: Verify invalid input branches return without reporting events.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4InvalidInputs_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4InvalidInputs_001 enter");
    g_capturedEvents.clear();
    BtChr2g4BleConnInterval("", AIRTIME_TEST_CONN_INTERVAL);
    BtChr2g4BleConnInterval(AIRTIME_TEST_ADDR, AIRTIME_TEST_ZERO);
    BtChr2g4BleGattPkgNameSet("", AIRTIME_TEST_PKG);
    BtChr2g4BleGattPkgNameSet(AIRTIME_TEST_ADDR, "");
    BtChr2g4BleScanPkgNameSet("");
    BtChr2g4BleAdvPkgNameSet(AIRTIME_TEST_ADV_ID, "");
    EXPECT_TRUE(g_capturedEvents.empty());
    HILOGI("Test_BtChr2g4InvalidInputs_001 end");
}

/**
 * @tc.name: Test_BtChr2g4A2dpCodecCache_001
 * @tc.desc: Verify A2DP codec cache is reused when Start follows SinkCodec.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4A2dpCodecCache_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4A2dpCodecCache_001 enter");
    BtChr2g4A2dpSinkCodec(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_BITRATE_BPS);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);
    g_capturedEvents.clear();

    BtChr2g4A2dpStart(AIRTIME_TEST_ADDR);
    BtChr2g4A2dpActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_BITRATE_BPS,
        AIRTIME_TEST_ROLE);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);

    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "CODEC_INDEX"), AIRTIME_TEST_CODEC_INDEX);
    EXPECT_EQ(GetIntParam(*event, "BITRATE_BPS"), AIRTIME_TEST_BITRATE_BPS);

    g_capturedEvents.clear();
    BtChr2g4A2dpActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_BITRATE_BPS,
        AIRTIME_TEST_ROLE);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);
    event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_GT(GetIntParam(*event, "SESSION_ID"), AIRTIME_TEST_ZERO);
    HILOGI("Test_BtChr2g4A2dpCodecCache_001 end");
}

/**
 * @tc.name: Test_BtChr2g4A2dpNoStartAndFail_001
 * @tc.desc: Verify A2DP Active without Start, repeated Active and Fail with an existing session.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4A2dpNoStartAndFail_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4A2dpNoStartAndFail_001 enter");
    BtChr2g4A2dpActive(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_INVALID_CODEC_INDEX,
        AIRTIME_TEST_ZERO, AIRTIME_TEST_ROLE);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR_ALT);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_GT(GetIntParam(*event, "SESSION_ID"), AIRTIME_TEST_ZERO);

    g_capturedEvents.clear();
    BtChr2g4A2dpStart(AIRTIME_TEST_ADDR);
    BtChr2g4A2dpActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_BITRATE_BPS,
        AIRTIME_TEST_ROLE);
    BtChr2g4A2dpActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_BITRATE_BPS,
        AIRTIME_TEST_ROLE);
    BtChr2g4A2dpFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);
    event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    EXPECT_EQ(GetIntParam(*event, "STATUS"), AIRTIME_TEST_STATUS_FAIL);
    HILOGI("Test_BtChr2g4A2dpNoStartAndFail_001 end");
}

/**
 * @tc.name: Test_BtChr2g4A2dpInvalidBitrate_001
 * @tc.desc: Verify invalid A2DP codec/bitrate combinations are reported as zero bitrate.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4A2dpInvalidBitrate_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4A2dpInvalidBitrate_001 enter");
    BtChr2g4A2dpSinkCodec(AIRTIME_TEST_ADDR, AIRTIME_TEST_INVALID_CODEC_INDEX,
        AIRTIME_TEST_A2DP_BITRATE_PARTIAL);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "BITRATE_BPS"), AIRTIME_TEST_ZERO);

    g_capturedEvents.clear();
    BtChr2g4A2dpSinkCodec(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_A2DP_BITRATE_TOO_HIGH);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);
    event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "BITRATE_BPS"), AIRTIME_TEST_ZERO);

    g_capturedEvents.clear();
    BtChr2g4A2dpSinkCodec(AIRTIME_TEST_ADDR, AIRTIME_TEST_CODEC_INDEX, AIRTIME_TEST_A2DP_BITRATE_NEGATIVE);
    BtChr2g4A2dpStart(AIRTIME_TEST_ADDR);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR);
    event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "BITRATE_BPS"), AIRTIME_TEST_ZERO);

    g_capturedEvents.clear();
    BtChr2g4A2dpSinkCodec(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_CODEC_INDEX_UNKNOWN,
        AIRTIME_TEST_BITRATE_BPS);
    BtChr2g4A2dpStart(AIRTIME_TEST_ADDR_ALT);
    BtChr2g4A2dpActive(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_CODEC_INDEX_UNKNOWN,
        AIRTIME_TEST_BITRATE_BPS, AIRTIME_TEST_ROLE);
    BtChr2g4A2dpStop(AIRTIME_TEST_ADDR_ALT);
    event = FindEvent(CHR_UE_BT_2G4_A2DP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "BITRATE_BPS"), AIRTIME_TEST_ZERO);
    HILOGI("Test_BtChr2g4A2dpInvalidBitrate_001 end");
}

/**
 * @tc.name: Test_BtChr2g4ScoEdgeCases_001
 * @tc.desc: Verify SCO active without start, stop without active and fail with an existing session.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4ScoEdgeCases_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4ScoEdgeCases_001 enter");
    BtChr2g4ScoActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_SCO_CODEC, AIRTIME_TEST_SCO_PKT_TYPE_SET,
        AIRTIME_TEST_SCO_TSCO_SET);
    BtChr2g4ScoStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_HCI_STATUS_OK, AIRTIME_TEST_FAIL_SCENE_NONE);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_SCO_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "SCO_CODEC"), AIRTIME_TEST_SCO_CODEC);

    g_capturedEvents.clear();
    BtChr2g4ScoStart(AIRTIME_TEST_ADDR);
    BtChr2g4ScoStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_HCI_STATUS_OK, AIRTIME_TEST_FAIL_SCENE_NONE);
    event = FindEvent(CHR_UE_BT_2G4_SCO_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "FAIL_SCENE"), AIRTIME_TEST_SCO_FAIL_SCENE_PEER_DISCONNECT);

    g_capturedEvents.clear();
    BtChr2g4ScoStart(AIRTIME_TEST_ADDR);
    BtChr2g4ScoActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_SCO_CODEC, AIRTIME_TEST_ZERO, AIRTIME_TEST_ZERO);
    BtChr2g4ScoActive(AIRTIME_TEST_ADDR, AIRTIME_TEST_SCO_CODEC, AIRTIME_TEST_SCO_PKT_TYPE_SET,
        AIRTIME_TEST_SCO_TSCO_SET);
    BtChr2g4ScoStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_HCI_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);
    event = FindEvent(CHR_UE_BT_2G4_SCO_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "HCI_STATUS"), AIRTIME_TEST_HCI_STATUS_FAIL);
    EXPECT_EQ(GetIntParam(*event, "FAIL_SCENE"), AIRTIME_TEST_FAIL_SCENE_TEST);

    g_capturedEvents.clear();
    BtChr2g4ScoStart(AIRTIME_TEST_ADDR);
    BtChr2g4ScoFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_HCI_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);
    event = FindEvent(CHR_UE_BT_2G4_SCO_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    HILOGI("Test_BtChr2g4ScoEdgeCases_001 end");
}

/**
 * @tc.name: Test_BtChr2g4HidNoStartAndMismatch_001
 * @tc.desc: Verify HID active without start and BR/BLE report mismatch branches.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4HidNoStartAndMismatch_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4HidNoStartAndMismatch_001 enter");
    BtChr2g4BrHidActive(AIRTIME_TEST_ADDR);
    BtChr2g4BrHidReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_BR_HID_REPORT_LEN);
    BtChr2g4BrHidStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BR_HID_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "REPORT_SIZE_B"), AIRTIME_TEST_BR_HID_REPORT_LEN);

    g_capturedEvents.clear();
    BtChr2g4BrHidReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_BR_HID_REPORT_LEN);
    BtChr2g4BrHidStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    EXPECT_TRUE(g_capturedEvents.empty());

    BtChr2g4BleHogpStart(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpActive(AIRTIME_TEST_ADDR);
    BtChr2g4BrHidReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_BR_HID_REPORT_LEN);
    BtChr2g4BrHidStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    BtChr2g4BleHogpReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_HOGP_REPORT_LEN);
    BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    event = FindEvent(CHR_UE_BT_2G4_BLE_HOGP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "NOTIF_COUNT"), AIRTIME_TEST_NOTIF_COUNT);

    g_capturedEvents.clear();
    BtChr2g4BleHogpReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_HOGP_REPORT_LEN);
    BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    EXPECT_TRUE(g_capturedEvents.empty());

    BtChr2g4BrHidStart(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpReportData(AIRTIME_TEST_ADDR, AIRTIME_TEST_HOGP_REPORT_LEN);
    BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    BtChr2g4BrHidStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    event = FindEvent(CHR_UE_BT_2G4_BR_HID_AIRTIME);
    ASSERT_NE(event, nullptr);
    HILOGI("Test_BtChr2g4HidNoStartAndMismatch_001 end");
}

/**
 * @tc.name: Test_BtChr2g4HidStopAndFailEdgeCases_001
 * @tc.desc: Verify HID stop without active and fail with/without an existing session.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4HidStopAndFailEdgeCases_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4HidStopAndFailEdgeCases_001 enter");
    BtChr2g4BrHidStart(AIRTIME_TEST_ADDR);
    BtChr2g4BrHidStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BR_HID_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");

    g_capturedEvents.clear();
    BtChr2g4BleHogpStart(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    event = FindEvent(CHR_UE_BT_2G4_BLE_HOGP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");

    g_capturedEvents.clear();
    BtChr2g4BrHidStart(AIRTIME_TEST_ADDR);
    BtChr2g4BrHidFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL);
    event = FindEvent(CHR_UE_BT_2G4_BR_HID_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");

    g_capturedEvents.clear();
    BtChr2g4BleHogpStart(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL);
    event = FindEvent(CHR_UE_BT_2G4_BLE_HOGP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");

    g_capturedEvents.clear();
    BtChr2g4BrHidStart(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL);
    BtChr2g4BrHidStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    event = FindEvent(CHR_UE_BT_2G4_BR_HID_AIRTIME);
    ASSERT_NE(event, nullptr);

    g_capturedEvents.clear();
    BtChr2g4BrHidFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL);
    BtChr2g4BleHogpFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL);
    EXPECT_TRUE(g_capturedEvents.empty());
    HILOGI("Test_BtChr2g4HidStopAndFailEdgeCases_001 end");
}

/**
 * @tc.name: Test_BtChr2g4SocketEdgeCases_001
 * @tc.desc: Verify socket active without start, data without session and stop without active.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4SocketEdgeCases_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4SocketEdgeCases_001 enter");
    BtChr2g4BrSocketActive(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_ADDR);
    BtChr2g4BrSocketActive(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_ADDR);
    BtChr2g4BrSocketData(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_SOCKET_DATA_LEN);
    BtChr2g4BrSocketStop(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_STATUS_OK);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BR_SOCKET_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_GT(GetIntParam(*event, "PAYLOAD_BPS"), AIRTIME_TEST_MIN_PAYLOAD_BPS);

    g_capturedEvents.clear();
    BtChr2g4BrSocketData(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_SOCKET_DATA_LEN);
    BtChr2g4BrSocketStop(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_STATUS_OK);
    EXPECT_TRUE(g_capturedEvents.empty());

    g_capturedEvents.clear();
    BtChr2g4BrSocketStart(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_ADDR, AIRTIME_TEST_SOCKET_CHANNEL,
        AIRTIME_TEST_SOCKET_SERVICE_TYPE);
    BtChr2g4BrSocketStop(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_STATUS_OK);
    event = FindEvent(CHR_UE_BT_2G4_BR_SOCKET_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    HILOGI("Test_BtChr2g4SocketEdgeCases_001 end");
}

/**
 * @tc.name: Test_BtChr2g4GattNoStartAndEmptyPkg_001
 * @tc.desc: Verify GATT empty package name, active without start and tx/rx without session.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4GattNoStartAndEmptyPkg_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4GattNoStartAndEmptyPkg_001 enter");
    BtChr2g4BleGattStart(AIRTIME_TEST_ADDR, "");
    BtChr2g4BleGattActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleGattActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleGattTx(AIRTIME_TEST_ADDR, AIRTIME_TEST_ZERO);
    BtChr2g4BleGattRx(AIRTIME_TEST_ADDR, AIRTIME_TEST_ZERO);
    BtChr2g4BleGattTx(AIRTIME_TEST_ADDR, AIRTIME_TEST_GATT_TX_BYTES);
    BtChr2g4BleGattRx(AIRTIME_TEST_ADDR, AIRTIME_TEST_GATT_RX_BYTES);
    BtChr2g4BleGattStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_GATT_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), "");
    EXPECT_EQ(GetIntParam(*event, "CONN_INTERVAL"), AIRTIME_TEST_ZERO);

    g_capturedEvents.clear();
    BtChr2g4BleGattActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleGattActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleGattStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    event = FindEvent(CHR_UE_BT_2G4_BLE_GATT_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_GT(GetIntParam(*event, "SESSION_ID"), AIRTIME_TEST_ZERO);

    g_capturedEvents.clear();
    BtChr2g4BleGattTx(AIRTIME_TEST_ADDR, AIRTIME_TEST_GATT_TX_BYTES);
    BtChr2g4BleGattRx(AIRTIME_TEST_ADDR, AIRTIME_TEST_GATT_RX_BYTES);
    BtChr2g4BleGattStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    EXPECT_TRUE(g_capturedEvents.empty());
    HILOGI("Test_BtChr2g4GattNoStartAndEmptyPkg_001 end");
}

/**
 * @tc.name: Test_BtChr2g4GattStopAndFailEdgeCases_001
 * @tc.desc: Verify GATT stop without active and fail with/without an existing session.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4GattStopAndFailEdgeCases_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4GattStopAndFailEdgeCases_001 enter");
    BtChr2g4BleGattStart(AIRTIME_TEST_ADDR, AIRTIME_TEST_PKG);
    BtChr2g4BleGattStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_GATT_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");

    g_capturedEvents.clear();
    BtChr2g4BleGattStart(AIRTIME_TEST_ADDR, AIRTIME_TEST_PKG);
    BtChr2g4BleGattFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);
    event = FindEvent(CHR_UE_BT_2G4_BLE_GATT_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    EXPECT_EQ(GetIntParam(*event, "STATUS"), AIRTIME_TEST_STATUS_FAIL);

    g_capturedEvents.clear();
    BtChr2g4BleGattFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);
    event = FindEvent(CHR_UE_BT_2G4_BLE_GATT_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    HILOGI("Test_BtChr2g4GattStopAndFailEdgeCases_001 end");
}

/**
 * @tc.name: Test_BtChr2g4ScanEdgeCases_001
 * @tc.desc: Verify scan flush before params, repeated params and empty package fallback.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4ScanEdgeCases_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4ScanEdgeCases_001 enter");
    g_capturedEvents.clear();
    BtChr2g4BleScanFlush();
    EXPECT_TRUE(g_capturedEvents.empty());

    BtChr2g4BleScanParams(AIRTIME_TEST_SCAN_WINDOW, AIRTIME_TEST_SCAN_INTERVAL,
        AIRTIME_TEST_SCAN_TYPE_ACTIVE, AIRTIME_TEST_PKG);
    BtChr2g4BleScanParams(AIRTIME_TEST_SCAN_WINDOW, AIRTIME_TEST_SCAN_INTERVAL,
        AIRTIME_TEST_SCAN_TYPE_ACTIVE, AIRTIME_TEST_PKG);
    BtChr2g4BleScanParams(AIRTIME_TEST_SCAN_WINDOW, AIRTIME_TEST_SCAN_INTERVAL,
        AIRTIME_TEST_SCAN_TYPE_ACTIVE, "");
    BtChr2g4BleScanEnable(true, "");
    BtChr2g4BleScanDisable();
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_SCAN_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), AIRTIME_TEST_PKG);

    g_capturedEvents.clear();
    BtChr2g4BleScanPkgNameSet(AIRTIME_TEST_PKG);
    BtChr2g4BleScanParams(AIRTIME_TEST_SCAN_WINDOW, AIRTIME_TEST_SCAN_INTERVAL,
        AIRTIME_TEST_SCAN_TYPE_ACTIVE, "");
    BtChr2g4BleScanEnable(true, "");
    BtChr2g4BleScanEnable(true, "");
    BtChr2g4BleScanDisable();
    event = FindEvent(CHR_UE_BT_2G4_BLE_SCAN_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), AIRTIME_TEST_PKG);

    g_capturedEvents.clear();
    BtChr2g4BleScanParams(AIRTIME_TEST_SCAN_WINDOW, AIRTIME_TEST_SCAN_INTERVAL,
        AIRTIME_TEST_SCAN_TYPE_ACTIVE, "");
    BtChr2g4BleScanEnable(true, "");
    BtChr2g4BleScanDisable();
    event = FindEvent(CHR_UE_BT_2G4_BLE_SCAN_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), "");
    HILOGI("Test_BtChr2g4ScanEdgeCases_001 end");
}

/**
 * @tc.name: Test_BtChr2g4AdvEdgeCases_001
 * @tc.desc: Verify ADV package fallback, repeated active, stop/fail without session and fail with session.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4AdvEdgeCases_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4AdvEdgeCases_001 enter");
    BtChr2g4BleAdvPkgNameSet(AIRTIME_TEST_ADV_ID, AIRTIME_TEST_PKG);
    BtChr2g4BleAdvStart(AIRTIME_TEST_ADV_ID, AIRTIME_TEST_ADV_INTERVAL, AIRTIME_TEST_ADV_TYPE,
        AIRTIME_TEST_ADV_PAYLOAD_LEN, "");
    BtChr2g4BleAdvStop(AIRTIME_TEST_ADV_ID);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_ADV_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), AIRTIME_TEST_PKG);

    g_capturedEvents.clear();
    BtChr2g4BleAdvStart(AIRTIME_TEST_ADV_ID_ACTIVE_ONLY, AIRTIME_TEST_ADV_INTERVAL,
        AIRTIME_TEST_ADV_TYPE, AIRTIME_TEST_ADV_PAYLOAD_LEN, "");
    BtChr2g4BleAdvActive(AIRTIME_TEST_ADV_ID_ACTIVE_ONLY);
    BtChr2g4BleAdvActive(AIRTIME_TEST_ADV_ID_ACTIVE_ONLY);
    BtChr2g4BleAdvStop(AIRTIME_TEST_ADV_ID_ACTIVE_ONLY);
    event = FindEvent(CHR_UE_BT_2G4_BLE_ADV_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetStrParam(*event, "PACKAGE_NAME"), "");

    g_capturedEvents.clear();
    BtChr2g4BleAdvStop(AIRTIME_TEST_ADV_ID);
    BtChr2g4BleAdvFail(AIRTIME_TEST_ADV_ID, AIRTIME_TEST_FAIL_SCENE_TEST);
    EXPECT_TRUE(g_capturedEvents.empty());

    g_capturedEvents.clear();
    BtChr2g4BleAdvStart(AIRTIME_TEST_ADV_ID, AIRTIME_TEST_ADV_INTERVAL, AIRTIME_TEST_ADV_TYPE,
        AIRTIME_TEST_ADV_PAYLOAD_LEN, AIRTIME_TEST_PKG);
    BtChr2g4BleAdvFail(AIRTIME_TEST_ADV_ID, AIRTIME_TEST_FAIL_SCENE_TEST);
    event = FindEvent(CHR_UE_BT_2G4_BLE_ADV_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_NE(GetStrParam(*event, "FAIL_TS"), "");
    HILOGI("Test_BtChr2g4AdvEdgeCases_001 end");
}

/**
 * @tc.name: Test_BtChr2g4DiscEdgeCases_001
 * @tc.desc: Verify discovery invalid types, duplicate starts, stop without session and failure stop.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4DiscEdgeCases_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4DiscEdgeCases_001 enter");
    g_capturedEvents.clear();
    BtChr2g4DiscStart(AIRTIME_TEST_ZERO, AIRTIME_TEST_DISC_WINDOW, AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INVALID, AIRTIME_TEST_DISC_WINDOW,
        AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscActive(AIRTIME_TEST_ZERO);
    BtChr2g4DiscActive(AIRTIME_TEST_DISC_TYPE_INVALID);
    EXPECT_TRUE(g_capturedEvents.empty());

    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INQUIRY_SCAN, AIRTIME_TEST_DISC_WINDOW,
        AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_PAGE_SCAN, AIRTIME_TEST_DISC_WINDOW,
        AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscFlushAll();
    EXPECT_EQ(g_capturedEvents.size(), AIRTIME_TEST_EVENT_COUNT_TWO);
    g_capturedEvents.clear();

    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_WINDOW,
        AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_WINDOW,
        AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscStop(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_FAIL_NONE);
    EXPECT_EQ(g_capturedEvents.size(), AIRTIME_TEST_EVENT_COUNT_ONE);

    g_capturedEvents.clear();
    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_WINDOW,
        AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_WINDOW_ALT,
        AIRTIME_TEST_DISC_INTERVAL_ALT);
    BtChr2g4DiscStop(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_FAIL_NONE);
    EXPECT_EQ(g_capturedEvents.size(), AIRTIME_TEST_EVENT_COUNT_TWO);

    g_capturedEvents.clear();
    BtChr2g4DiscStop(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_FAIL_NONE);
    BtChr2g4DiscFlushAll();
    EXPECT_TRUE(g_capturedEvents.empty());

    BtChr2g4DiscStart(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_DISC_WINDOW,
        AIRTIME_TEST_DISC_INTERVAL);
    BtChr2g4DiscStop(AIRTIME_TEST_DISC_TYPE_INQUIRY, AIRTIME_TEST_FAIL_SCENE_TEST);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_CLASSIC_DISCOVERY_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "FAIL_REASON"), AIRTIME_TEST_FAIL_SCENE_TEST);
    HILOGI("Test_BtChr2g4DiscEdgeCases_001 end");
}

/**
 * @tc.name: Test_BtChr2g4StopAndFailAttribution_001
 * @tc.desc: Verify SCO caller failScene, HID type isolation and single socket failure report.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4StopAndFailAttribution_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4StopAndFailAttribution_001 enter");
    g_capturedEvents.clear();
    BtChr2g4ScoStart(AIRTIME_TEST_ADDR);
    BtChr2g4ScoStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_HCI_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_SCO_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "FAIL_SCENE"), AIRTIME_TEST_FAIL_SCENE_TEST);

    g_capturedEvents.clear();
    BtChr2g4BleHogpStart(AIRTIME_TEST_ADDR_ALT);
    BtChr2g4BrHidFail(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_STATUS_FAIL);
    EXPECT_TRUE(g_capturedEvents.empty());
    BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_STATUS_OK);

    g_capturedEvents.clear();
    BtChr2g4BrSocketStart(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_ADDR, AIRTIME_TEST_SOCKET_CHANNEL,
        AIRTIME_TEST_SOCKET_SERVICE_TYPE);
    BtChr2g4BrSocketFail(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_FAIL, AIRTIME_TEST_FAIL_SCENE_TEST);
    EXPECT_EQ(g_capturedEvents.size(), AIRTIME_TEST_EVENT_COUNT_ONE);
    event = FindEvent(CHR_UE_BT_2G4_BR_SOCKET_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "FAIL_SCENE"), AIRTIME_TEST_FAIL_SCENE_TEST);
    BtChr2g4BrSocketStop(AIRTIME_TEST_SOCKET_HANDLE, AIRTIME_TEST_STATUS_OK);
    EXPECT_EQ(g_capturedEvents.size(), AIRTIME_TEST_EVENT_COUNT_ONE);
    HILOGI("Test_BtChr2g4StopAndFailAttribution_001 end");
}

/**
 * @tc.name: Test_BtChr2g4ReportConsistency_001
 * @tc.desc: Verify latest connection interval and clamped GATT byte counters.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrAirtimeTest, Test_BtChr2g4ReportConsistency_001, TestSize.Level1)
{
    HILOGI("Test_BtChr2g4ReportConsistency_001 enter");
    g_capturedEvents.clear();
    BtChr2g4BleConnInterval(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_CONN_INTERVAL);
    BtChr2g4BleGattStart(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_PKG);
    BtChr2g4BleGattActive(AIRTIME_TEST_ADDR_ALT);
    BtChr2g4BleConnInterval(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_GATT_CONN_INTERVAL);
    BtChr2g4BleGattTx(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_GATT_TX_BYTES_MAX);
    BtChr2g4BleGattTx(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_GATT_TX_BYTES_MAX);
    BtChr2g4BleGattStop(AIRTIME_TEST_ADDR_ALT, AIRTIME_TEST_STATUS_OK);
    const CapturedAirtimeEvent *event = FindEvent(CHR_UE_BT_2G4_BLE_GATT_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "CONN_INTERVAL"), AIRTIME_TEST_GATT_CONN_INTERVAL);
    EXPECT_EQ(GetIntParam(*event, "TX_BYTES"), AIRTIME_TEST_GATT_TX_BYTES_MAX);

    g_capturedEvents.clear();
    BtChr2g4BleConnInterval(AIRTIME_TEST_ADDR, AIRTIME_TEST_CONN_INTERVAL);
    BtChr2g4BleHogpStart(AIRTIME_TEST_ADDR);
    BtChr2g4BleHogpActive(AIRTIME_TEST_ADDR);
    BtChr2g4BleConnInterval(AIRTIME_TEST_ADDR, AIRTIME_TEST_GATT_CONN_INTERVAL);
    BtChr2g4BleHogpStop(AIRTIME_TEST_ADDR, AIRTIME_TEST_STATUS_OK);
    event = FindEvent(CHR_UE_BT_2G4_BLE_HOGP_AIRTIME);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(GetIntParam(*event, "CONN_INTERVAL"), AIRTIME_TEST_GATT_CONN_INTERVAL);
    EXPECT_GE(GetIntParam(*event, "REPORT_RATE_HZ"), AIRTIME_TEST_ZERO);
    HILOGI("Test_BtChr2g4ReportConsistency_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS
