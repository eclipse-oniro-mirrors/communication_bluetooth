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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "sensorhub_collaboration_service.h"
#include "bluetooth_sensorhub_collaboration.h"
#include "bluetooth_hw_interface.h"
#include "bluetooth_log.h"
#include "common_event_manager.h"

using namespace OHOS;
using namespace OHOS::Bluetooth;
using namespace OHOS::bluetooth;
using namespace testing;
using namespace testing::ext;

static struct SensorhubCollaborationTestSettings {
    uint32_t capability;
} g_shCollTestSettings;

static struct SensorhubCollaborationTestResult {
    bool setBufferSizeAndCount;
    char sendToSensorhubType;
    std::vector<char> sendToSensorhubParam;

    int scanMsgWhat;
    int scanMsgArg1;
    BleScannerStateMachine::ScannerAppInfo sensorScanner;
} g_shCollTestResult;

static void SendParamsToSensorhubMock(const char *params, char type, int length)
{
    HILOGI("save sensorhub msg, type: %{public}d, len: %{public}d", type, length);
    g_shCollTestResult.sendToSensorhubType = type;
    g_shCollTestResult.sendToSensorhubParam = std::vector<char>(params, params + length);
}

static void HwConnAttrSetMock(uint8_t transport, const STACK::RawAddress &addr, const HwConnAttr *attr)
{
    HILOGI("set attr, transport: %{public}hhu, type: %{public}hhu", transport, attr->type);
}

static const bthwif_interface_t g_shCollHwifTestInterface = {
    .sendParamsToSensorhub = SendParamsToSensorhubMock,
    .hwConnAttrSet = HwConnAttrSetMock,
};

namespace {

void TestScannerMachineCallback(const BleScannerMessage &msg)
{
    g_shCollTestResult.scanMsgWhat = msg.what_;
    g_shCollTestResult.scanMsgArg1 = msg.arg1_;
    g_shCollTestResult.sensorScanner = msg.appInfo;
}

std::shared_ptr<SensorhubCollaboration> g_testSc = std::make_shared<SensorhubCollaboration>(TestScannerMachineCallback);

class SensorhubCollaborationTest : public testing::Test {
public:
    SensorhubCollaborationTest() = default;
    ~SensorhubCollaborationTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SensorhubCollaborationTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothHwInterface::GetInstance()->SetBtHwInterface(&g_shCollHwifTestInterface);
    SensorhubCollaborationService::GetInstance()->Init();
    HILOGI("SetUpTestCase end");
}

void SensorhubCollaborationTest::TearDownTestCase(void)
{
}

void SensorhubCollaborationTest::SetUp()
{
    g_shCollTestResult = {};
    g_shCollTestSettings = {};
}

void SensorhubCollaborationTest::TearDown()
{
    g_testSc->SetScanReportChannelToLpDevice(0, false);
    TestSetScreenStatus(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
}

/**
 * @tc.name: QueryCapability0
 * @tc.desc: Sensorhub not support collaboration
 * @tc.type: FUNC
 */
HWTEST_F(SensorhubCollaborationTest, QueryCapability0, TestSize.Level1)
{
    HILOGI("SensorhubCollaborationTest QueryCapability0 start");
    //SensorhubCollaboration sc(TestScannerMachineCallback);
    g_testSc->QueryCapability();

    EXPECT_EQ(g_shCollTestResult.sendToSensorhubType, BLE_LPDEVICE_MSG_CAPABILITY_QUERY);
    EXPECT_EQ(g_shCollTestResult.sendToSensorhubParam.size(), 1lu);
    EXPECT_EQ(IsScanCollborationEnabled(), false);
    HILOGI("SensorhubCollaborationTest QueryCapability0 end");
}

/**
 * @tc.name: QueryCapabilityScanEnabled
 * @tc.desc: Sensorhub scan support collaboration
 * @tc.type: FUNC
 */
HWTEST_F(SensorhubCollaborationTest, QueryCapabilityScanEnabled, TestSize.Level1)
{
    HILOGI("SensorhubCollaborationTest QueryCapabilityScanEnabled start");
    //SensorhubCollaboration sc(TestScannerMachineCallback);
    g_testSc->QueryCapability();

    std::vector<uint8_t> notifyValue = { BLE_LPDEVICE_MSG_CAPABILITY, 3, 0, 0, 0 };
    HwSensorhubCollaborationCb(notifyValue);

    EXPECT_EQ(IsScanCollborationEnabled(), false);
    EXPECT_EQ(SensorhubCollaborationService::GetInstance()->StartConnCollaboration("test"), true);

    g_testSc->SetScanReportChannelToLpDevice(0, true);
    EXPECT_EQ(IsScanCollborationEnabled(), false);

    TestSetScreenStatus(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EXPECT_EQ(IsScanCollborationEnabled(), true);

    notifyValue = { BLE_LPDEVICE_MSG_GATT_CONN_COLLABORATION_NOTIFY_STATUS, 0, 0, 0, 0 };
    HwSensorhubCollaborationCb(notifyValue);
    SensorhubCollaborationService::GetInstance()->AllConnectionComplete("test");

    HILOGI("SensorhubCollaborationTest QueryCapabilityScanEnabled end");
}

/**
 * @tc.name: ScanCollaboration
 * @tc.desc: Sensorhub start scan collaboration
 * @tc.type: FUNC
 */
HWTEST_F(SensorhubCollaborationTest, ScanCollaboration, TestSize.Level1)
{
    HILOGI("SensorhubCollaborationTest ScanCollaboration start");
    //SensorhubCollaboration sc(TestScannerMachineCallback);
    g_testSc->QueryCapability();

    std::vector<uint8_t> notifyValue = { BLE_LPDEVICE_MSG_CAPABILITY, 3, 0, 0, 0 };
    HwSensorhubCollaborationCb(notifyValue);

    g_testSc->SetScanReportChannelToLpDevice(0, true);
    TestSetScreenStatus(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EXPECT_EQ(IsScanCollborationEnabled(), true);

    g_shCollTestResult = {};
    SendScanCollaborationMsg(BLE_LPDEVICE_MSG_SCAN_COLLABORATION_QUERY, {});
    EXPECT_EQ(g_shCollTestResult.sendToSensorhubType, BLE_LPDEVICE_MSG_SCAN_COLLABORATION_QUERY);
    EXPECT_EQ(g_shCollTestResult.sendToSensorhubParam.size(), 1lu);

    uint16_t sensorhubScanInterval = 0;
    uint16_t sensorhubScanWindow = 0;
    notifyValue = { BLE_LPDEVICE_MSG_SCAN_COLLABORATION_NOTIFY_STATUS, 0, 0, 0, 0 };
    HwSensorhubCollaborationCb(notifyValue);
    EXPECT_EQ(g_shCollTestResult.scanMsgWhat, BleScannerStateMachine::SH_COLLABORATION_EVENT);
    EXPECT_EQ(g_shCollTestResult.scanMsgArg1, BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS);
    EXPECT_EQ(g_shCollTestResult.sensorScanner.params.scanInterval, sensorhubScanInterval);
    EXPECT_EQ(g_shCollTestResult.sensorScanner.params.scanInterval, sensorhubScanWindow);

    g_shCollTestResult = {};
    BleScannerStateMachine::ScannerAppInfo a;
    a.scannerId = 1;
    a.params.scanInterval = 1500;
    a.params.scanWindow = 30;
    std::vector<BleScannerStateMachine::ScannerAppInfo> appInfos = {a};
    SendScanCollaborationMsg(BLE_LPDEVICE_MSG_SCAN_COLLABORATION_QUERY, appInfos);
    sensorhubScanInterval = 300;
    sensorhubScanWindow = 30;
    notifyValue = { BLE_LPDEVICE_MSG_SCAN_COLLABORATION_NOTIFY_STATUS,
        static_cast<uint8_t>(sensorhubScanInterval & 0xff),
        static_cast<uint8_t>(sensorhubScanInterval >> 8),
        static_cast<uint8_t>(sensorhubScanWindow & 0xff),
        static_cast<uint8_t>(sensorhubScanWindow >> 8)
    };
    HwSensorhubCollaborationCb(notifyValue);
    EXPECT_EQ(g_shCollTestResult.scanMsgWhat, BleScannerStateMachine::SH_COLLABORATION_EVENT);
    EXPECT_EQ(g_shCollTestResult.scanMsgArg1, BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS);
    EXPECT_EQ(g_shCollTestResult.sensorScanner.params.scanInterval, sensorhubScanInterval);
    EXPECT_EQ(g_shCollTestResult.sensorScanner.params.scanWindow, sensorhubScanWindow);
    HILOGI("SensorhubCollaborationTest ScanCollaboration end");
}

/**
 * @tc.name: GattCollaboration
 * @tc.desc: Sensorhub start gatt collaboration
 * @tc.type: FUNC
 */
HWTEST_F(SensorhubCollaborationTest, GattCollaboration, TestSize.Level1)
{
    HILOGI("SensorhubCollaborationTest GattCollaboration start");
    //SensorhubCollaboration sc(TestScannerMachineCallback);
    g_testSc->QueryCapability();

    std::vector<uint8_t> notifyValue = { BLE_LPDEVICE_MSG_CAPABILITY, 3, 0, 0, 0 };
    HwSensorhubCollaborationCb(notifyValue);

    g_testSc->SetScanReportChannelToLpDevice(0, true);
    TestSetScreenStatus(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EXPECT_EQ(SensorhubCollaborationService::GetInstance()->StartConnCollaboration("test"), true);

    notifyValue = { BLE_LPDEVICE_MSG_GATT_CONN_COLLABORATION_NOTIFY_STATUS, 0, 0, 0, 0 };
    HwSensorhubCollaborationCb(notifyValue);
    SensorhubCollaborationService::GetInstance()->AllConnectionComplete("test");
    HILOGI("SensorhubCollaborationTest GattCollaboration end");
}
}
