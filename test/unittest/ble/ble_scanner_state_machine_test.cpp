/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <set>
#include <thread>

#include "ble_scanner_state_machine.h"
#include "bluetooth_host.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hisysevent.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_log.h"

using namespace OHOS;
using namespace OHOS::Bluetooth;
using namespace OHOS::bluetooth;
using namespace testing;
using namespace testing::ext;

namespace {
static const BleScannerMessage T_START_EVENT_1 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(10, 10, 1,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P2_60_3000, true, 0)),
};
static const BleScannerMessage T_STOP_EVENT_1 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(10, 10, 1),
};

static const BleScannerMessage T_START_EVENT_2 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(20, 20, 2,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P10_60_600, true, 0)),
};
static const BleScannerMessage T_STOP_EVENT_2 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(20, 20, 2),
};

static const BleScannerMessage T_START_EVENT_3 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(30, 30, 3,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P25_60_240, true, 0)),
};

static const BleScannerMessage T_STOP_EVENT_3 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(30, 30, 3),
};

static const BleScannerMessage T_START_EVENT_4 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(40, 40, 4,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P100_1000_1000, true, 0)),
};

static const BleScannerMessage T_STOP_EVENT_4 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(40, 40, 4),
};

static const BleScannerMessage T_START_EVENT_5 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(50, 50, 5,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P2_60_3000, false, 0)),
};

static const BleScannerMessage T_STOP_EVENT_5 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(50, 50, 5),
};

static const BleScannerMessage T_START_EVENT_6 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(60, 60, 6,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P2_60_3000, false, 0)),
};

static const BleScannerMessage T_STOP_EVENT_6 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(60, 60, 6),
};

static const BleScannerMessage T_START_EVENT_7 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(70, 70, 7,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P10_60_600, true, 0)),
};

static const BleScannerMessage T_STOP_EVENT_7 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(70, 70, 7),
};

static const BleScannerMessage T_START_EVENT_8 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(80, 80, 8,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P2_60_3000, true, 0)),
};

static const BleScannerMessage T_START_EVENT_9 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(90, 90, 9,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P2_60_3000, false, 0)),
};

static const BleScannerMessage T_START_EVENT_100 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(10, 10, 100,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P10_60_600, true, 0)),
};

static const BleScannerMessage T_STOP_EVENT_100 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(10, 10, 100),
};

static const BleScannerMessage T_START_EVENT_101 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(10, 10, 101,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P25_60_240, true, 0)),
};

static const BleScannerMessage T_STOP_EVENT_101 {
    BleScannerStateMachine::STOP_EVENT,
    BleScannerStateMachine::ScannerAppInfo(10, 10, 101),
};

static const BleScannerMessage T_START_EVENT_102 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(10, 10, 102,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_LOW_LATENCY, true, 0)),
};

static const BleScannerMessage T_STOP_EVENT_102 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(10, 10, 102),
};

static const BleScannerMessage T_START_EVENT_103 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(20, 20, 103,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P10_30_300, true, 0)),
};

static const BleScannerMessage T_STOP_EVENT_103 {
    BleScannerStateMachine::START_EVENT,
    BleScannerStateMachine::ScannerAppInfo(20, 20, 103),
};

static const BleScannerMessage T_START_SUCCESS_EVENT {
    BleScannerStateMachine::START_SUCCESS_EVENT,
};
static const BleScannerMessage T_START_FAILED_EVENT {
    BleScannerStateMachine::START_FAILED_EVENT,
};
static const BleScannerMessage T_STOP_SUCCESS_EVENT {
    BleScannerStateMachine::STOP_SUCCESS_EVENT,
};
static const BleScannerMessage T_STOP_FAILED_EVENT {
    BleScannerStateMachine::STOP_FAILED_EVENT,
};

#ifdef CONTEXTHUB_BLE_V3
static const BleScannerMessage T_SH_COLLABORATION_EVENT_60_3000 {
    BleScannerStateMachine::SH_COLLABORATION_EVENT,
    BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS,
    BleScannerStateMachine::ScannerAppInfo(0, 0, 0,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P2_60_3000, true, 0)),
};
static const BleScannerMessage T_SH_COLLABORATION_EVENT_60_600 {
    BleScannerStateMachine::SH_COLLABORATION_EVENT,
    BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS,
    BleScannerStateMachine::ScannerAppInfo(0, 0, 0,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P10_60_600, true, 0)),
};
static const BleScannerMessage T_SH_COLLABORATION_EVENT_30_300 {
    BleScannerStateMachine::SH_COLLABORATION_EVENT,
    BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS,
    BleScannerStateMachine::ScannerAppInfo(0, 0, 0,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P10_30_300, true, 0)),
};
static const BleScannerMessage T_SH_COLLABORATION_EVENT_100_200 {
    BleScannerStateMachine::SH_COLLABORATION_EVENT,
    BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS,
    BleScannerStateMachine::ScannerAppInfo(0, 0, 0,
        BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P50_100_200, true, 0)),
};
static const BleScannerMessage T_SH_COLLABORATION_EVENT_NOT_STARTED {
    BleScannerStateMachine::SH_COLLABORATION_EVENT,
    BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS,
};
static const BleScannerMessage T_SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN_EVENT {
    BleScannerStateMachine::SH_COLLABORATION_EVENT,
    BleScannerStateMachine::SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN,
};
static const BleScannerMessage T_SH_COLLABORATION_INVALID_EVENT {
    BleScannerStateMachine::SH_COLLABORATION_EVENT,
    100,
};
#endif

OHOS::bluetooth::BleScanSettingsImpl GetTestScanSetting(const BleScannerMessage &msg)
{
    OHOS::bluetooth::BleScanSettingsImpl setting;
    setting.SetScanMode(msg.appInfo.params.scanMode);
    setting.SetReportDelay(0);
    setting.SetLegacy(msg.scanParams.legacy);
    setting.SetPhy(0);
    setting.SetScanInterval(msg.scanParams.scanInterval);
    setting.SetScanWindow(msg.scanParams.scanWindow);
    return setting;
}
static BluetoothHost *host_;
constexpr int TIME = 2;
class BleScannerInterfaceMock {
public:
    MOCK_METHOD(void, StartScan, (const BleScanSettingsImpl &setting));
    MOCK_METHOD(void, StopScan, ());
    MOCK_METHOD(void, ReportStartedEvent, (int pid, int uid, int scannerId, int resultCode));
    MOCK_METHOD(void, ReportStoppedEvent, (int pid, int uid, int scannerId, int resultCode));
    ~BleScannerInterfaceMock() = default;
};

#define CHECK_STATE_STOPPED()  EXPECT_EQ(GetCurrentState(), BLE_SCAN_STOPPED)
#define CHECK_STATE_STARTING() EXPECT_EQ(GetCurrentState(), BLE_SCAN_STARTING)
#define CHECK_STATE_STARTED()  EXPECT_EQ(GetCurrentState(), BLE_SCAN_STARTED)
#define CHECK_STATE_STOPPING() EXPECT_EQ(GetCurrentState(), BLE_SCAN_STOPPING)
#define CHECK_STATE_INNER_STOPPING() EXPECT_EQ(GetCurrentState(), BLE_SCAN_INNER_STOPPING)
#define CHECK_STATE_SH_COLLABORATION() EXPECT_EQ(GetCurrentState(), BLE_SCAN_SH_COLLABORATION)

class BleScannerStateMachineTest : public testing::Test {
public:
    BleScannerStateMachineTest() = default;
    ~BleScannerStateMachineTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void InitStateMachine(BleScannerInterfaceMock &interface, bool useCheckFunc = false)
    {
        auto postTask = [](std::function<void(void)> func) { func(); };
        auto startScan = [&interface](const OHOS::bluetooth::BleScanSettingsImpl &setting) {
            interface.StartScan(setting);
        };
        auto stopScan = [&interface]() { interface.StopScan(); };
        auto reportStartedEvent = [&interface](int pid, int uid, int scannerId, int resultCode) {
            interface.ReportStartedEvent(pid, uid, scannerId, resultCode);
        };
        auto reportStoppedEvent = [&interface](int pid, int uid, int scannerId, int resultCode) {
            interface.ReportStoppedEvent(pid, uid, scannerId, resultCode);
        };
        BleScannerStateMachine::CheckAppActiveScanFunc checkAppActiveScanFunc = nullptr;
        if (useCheckFunc) {
            checkAppActiveScanFunc = [this](const BleScannerStateMachine::ScannerAppInfo &appInfo) {
                if (stoppedScannerIds_.count(appInfo.scannerId) > 0) {
                    return false;
                }
                return appActiveScan_;
            };
        }
#ifdef CONTEXTHUB_BLE_V3
        auto sendFunc = [](int type, const std::vector<BleScannerStateMachine::ScannerAppInfo> &appInfos) {};
        auto collaborationEnabledFunc = [this]() { return collaborationEnabled_; };
        stateMachine_->Init(postTask, startScan, stopScan, reportStartedEvent, reportStoppedEvent,
            BleScannerStateMachine::CollabrationFunc(sendFunc, collaborationEnabledFunc), checkAppActiveScanFunc);
#else
        stateMachine_->Init(postTask, startScan, stopScan, reportStartedEvent, reportStoppedEvent,
            checkAppActiveScanFunc);
#endif
    }

    std::string GetCurrentState(void)
    {
        auto state = stateMachine_->GetState();
        if (state == nullptr) {
            return BLE_SCAN_STOPPED;
        }
        return stateMachine_->GetState()->Name();
    }

    // Initial state is BLE_SCAN_STOPPED.
    void StartCycleInit(const BleScannerMessage &msg);
    // Initial state is BLE_SCAN_STARTED.
    void StartCycleLarger(const BleScannerMessage &msg);
    void StartCycleLower(const BleScannerMessage &msg);

#ifdef CONTEXTHUB_BLE_V3
    void EnableShCollaboration();
    void DisableShCollaboration();
    void ShCollaborationEventCheck();
    void StartingAndStoppingCollaborationStateCheck(
        const BleScannerMessage &startEvent, const BleScannerMessage &shEventInStartEvent,
        const BleScannerMessage &stopEvent, const BleScannerMessage &shEventInStopEvent);
    bool collaborationEnabled_ = false;
#endif

    std::shared_ptr<BleScannerStateMachine> stateMachine_ {nullptr};
    // Simulates the real-time isAppActiveScan state of the application container.
    bool appActiveScan_ = true;
    // Simulates the apps (by scannerId) which have stopped scanning, overriding appActiveScan_.
    std::set<int> stoppedScannerIds_ {};
};

#ifdef CONTEXTHUB_BLE_V3
void BleScannerStateMachineTest::EnableShCollaboration()
{
    collaborationEnabled_ = true;
}

void BleScannerStateMachineTest::DisableShCollaboration()
{
    collaborationEnabled_ = false;
}
#endif

void BleScannerStateMachineTest::SetUpTestCase(void)
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

    HILOGI("SetUpTestCase end");
}
void BleScannerStateMachineTest::TearDownTestCase(void)
{
    host_ = nullptr;
    utility::CloseAllTimer();
}
void BleScannerStateMachineTest::SetUp()
{
    stateMachine_ = std::make_shared<BleScannerStateMachine>();
}
void BleScannerStateMachineTest::TearDown()
{
    stateMachine_->Reset();
#ifdef CONTEXTHUB_BLE_V3
    DisableShCollaboration();
#endif
    stateMachine_ = nullptr;
}

void BleScannerStateMachineTest::StartCycleInit(const BleScannerMessage &msg)
{
    stateMachine_->SendMessage(msg);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
}
void BleScannerStateMachineTest::StartCycleLarger(const BleScannerMessage &msg)
{
    stateMachine_->SendMessage(msg);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
}
void BleScannerStateMachineTest::StartCycleLower(const BleScannerMessage &msg)
{
    stateMachine_->SendMessage(msg);
    CHECK_STATE_STARTED();
}

/**
 * @tc.name: ble_scanner_state_machine_test_001
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Stop -> StopSuccess.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_001, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_001 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_001 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_002
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(>) -> StopSuccess -> Inner Start -> StartSuccess ->
                Stop -> StopSuccess -> Inner Start -> StartSuccess -> Stop -> StopSuccess.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_002, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_002 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    StartCycleLarger(T_START_EVENT_2);

    stateMachine_->SendMessage(T_STOP_EVENT_2);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_002 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_003
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> Start(>) -> StartSuccess -> StopSuccess -> Inner Start.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_003, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_003 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_003 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_004
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> Stop -> StartSuccess -> StopSuccess.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_004, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_004 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_004 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_005
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(<) -> Start(=) -> Stop(<).
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_005, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_005 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    CHECK_STATE_STOPPED();
    BleScannerMessage msg = T_START_EVENT_2;
    msg.appInfo.pid = 0;
    msg.appInfo.uid = 0;

    StartCycleInit(T_START_EVENT_2);
    StartCycleLower(T_START_EVENT_1);
    StartCycleLower(msg);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);
    HILOGI("ble_scanner_state_machine_test_005 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_006
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Stop -> Start -> StopSuccess -> Inner Start -> StartSuccess.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_006, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_006 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_006 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_007
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> Timeout.
                Start -> StartSuccess -> Stop -> Timeout.
                Start -> StartSuccess -> Start(>) -> TImeout.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_007, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_007 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner(AppExecFwk::EventRunner::Create("ble_scanner_sm_test_007"));
    auto eventHandler = std::make_shared<AppExecFwk::EventHandler>(eventRunner);
    auto postTask = [eventHandler](std::function<void(void)> func) {
        eventHandler->PostTask(func);
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    };
    auto startScan = [&interface](const OHOS::bluetooth::BleScanSettingsImpl &setting) {
        interface.StartScan(setting);
    };
    auto stopScan = [&interface]() { interface.StopScan(); };
    auto reportStartedEvent = [&interface](int pid, int uid, int scannerId, int resultCode) { };
    auto reportStoppedEvent = [&interface](int pid, int uid, int scannerId, int resultCode) { };

#ifdef CONTEXTHUB_BLE_V3
    auto sendFunc = [](int type, const std::vector<BleScannerStateMachine::ScannerAppInfo> &appInfos) {};
    auto collaborationEnabledFunc = []() { return false; };
    stateMachine_->Init(postTask, startScan, stopScan, reportStartedEvent, reportStoppedEvent,
        BleScannerStateMachine::CollabrationFunc(sendFunc, collaborationEnabledFunc));
#else
    stateMachine_->Init(postTask, startScan, stopScan, reportStartedEvent, reportStoppedEvent);
#endif
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
    }

    const int timeoutDelay = 30;
    stateMachine_->SetTimeoutMs(timeoutDelay);
    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTING();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeoutDelay));
    CHECK_STATE_STOPPED();

    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPING();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeoutDelay));
    CHECK_STATE_STOPPED();

    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_INNER_STOPPING();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeoutDelay));
    CHECK_STATE_STOPPED();
    HILOGI("ble_scanner_state_machine_test_007 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_008
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(>) -> StopSuccess -> Inner Start -> StartSuccess ->
                Stop -> Stop -> StopSuccess -> Inner Start -> StartSuccess -> Inner Stop -> StopSuccess.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_008, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_008 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    StartCycleLarger(T_START_EVENT_2);

    stateMachine_->SendMessage(T_STOP_EVENT_2);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_008 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_009
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartFailed.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_009, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_009 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_FAILED_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_009 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_010
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(>) -> StopSuccess -> Inner Start -> StartFailed ->
                Inner Start -> StartFailed
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_010, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_010 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_FAILED_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_FAILED_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_010 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_011
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(>) -> StopSuccess -> Inner Start -> StartSuccess ->
                Stop -> StopSuccess -> Inner Start -> StartFailed
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_011, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_011 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    StartCycleLarger(T_START_EVENT_2);

    stateMachine_->SendMessage(T_STOP_EVENT_2);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_FAILED_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_011 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_012
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> (Start(>) -> StopSuccess -> Inner Start -> StartSuccess) * 3
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_012, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_012 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    StartCycleLarger(T_START_EVENT_2);
    StartCycleLarger(T_START_EVENT_3);
    StartCycleLarger(T_START_EVENT_4);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 4);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_012 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_013
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> same app Start(>)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_013, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_013 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    CHECK_STATE_STOPPED();
    const BleScannerMessage msg {
        BleScannerStateMachine::START_EVENT,
        BleScannerStateMachine::ScannerAppInfo(20, 20, 2,
            BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P25_60_240, true, 0)),
    };
    const BleScannerMessage msg1 {
        BleScannerStateMachine::START_EVENT,
        BleScannerStateMachine::ScannerAppInfo(20, 20, 2,
            BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P2_60_3000, true, 0)),
    };

    StartCycleInit(T_START_EVENT_2);
    stateMachine_->SendMessage(msg);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(msg1);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_013 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_014
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(>) -> StopSuccess -> InnerStart -> StartSuccess ->
                Mock DisableBle StopSuccess
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_014, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_014 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    StartCycleLarger(T_START_EVENT_2);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    HILOGI("ble_scanner_state_machine_test_014 end");
}
/**
 * @tc.name: ble_scanner_state_machine_test_015
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Stop -> Stop -> StopSuccess
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_015, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_015 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    // state machine should reset.
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_015 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_016
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Stop -> Stop -> Start -> StopSuccess
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_016, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_016 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    stateMachine_->SendMessage(T_START_EVENT_2);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_016 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_017
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> Start(<) -> Start(<) -> StartSuccess
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_017, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_017 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_3);
    stateMachine_->SendMessage(T_START_EVENT_2);
    stateMachine_->SendMessage(T_START_EVENT_1);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 3);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_017 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_018
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(<) -> Start(<) -> Start(>) -> StopSuccess -> Inner Start ->
                Stop(<) -> StopSuccess -> Start -> StartSuccess -> Stop(<) -> StopSuccess -> Start -> StartSuccess
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_018, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_018 enter");

    BleScannerMessage T_START_EVENT_4_SCAN_240_240(T_START_EVENT_4);
    T_START_EVENT_4_SCAN_240_240.scanParams.scanInterval = 240;
    T_START_EVENT_4_SCAN_240_240.scanParams.scanWindow = 240;
    BleScannerMessage T_START_EVENT_4_SCAN_600_600(T_START_EVENT_4);
    T_START_EVENT_4_SCAN_600_600.scanParams.scanInterval = 600;
    T_START_EVENT_4_SCAN_600_600.scanParams.scanWindow = 600;

    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4_SCAN_240_240)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_3);
    StartCycleLower(T_START_EVENT_2);
    StartCycleLower(T_START_EVENT_1);
    stateMachine_->SendMessage(T_START_EVENT_4);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_STOP_EVENT_3);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 3);
    stateMachine_->SendMessage(T_STOP_EVENT_2);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_STOP_EVENT_4);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_018 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_019
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(=) -> Stop(=)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_019, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_019 enter");
    const BleScannerMessage startMsg {
        BleScannerStateMachine::START_EVENT,
        BleScannerStateMachine::ScannerAppInfo(50, 40, 5,
            BleScannerStateMachine::ScannerParameters(0, OHOS::bluetooth::SCAN_MODE_OP_P100_1000_1000, true, 0)),
    };
    const BleScannerMessage stopMsg {
        BleScannerStateMachine::STOP_EVENT,
        BleScannerStateMachine::ScannerAppInfo(50, 40, 5),
    };

    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_4);
    stateMachine_->SendMessage(startMsg);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(stopMsg);
    CHECK_STATE_STARTED();

    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_019 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_020
 * @tc.desc: Verify the BleScannerStateMachine:
                Reset -> Reset -> Start -> StartSuccess -> Start(>) -> StopSuccess -> Inner Start -> StartSuccess
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_020, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_020 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    stateMachine_->Reset();
    stateMachine_->Reset();
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    StartCycleLarger(T_START_EVENT_2);

    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_020 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_021
 * @tc.desc: Verify the BleScannerStateMachine: same app
                Start -> StartSuccess -> (Start(>) -> StopSuccess -> Inner Start -> StartSuccess) * 2
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_021, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_021 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_100)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_101)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    StartCycleLarger(T_START_EVENT_100);
    StartCycleLarger(T_START_EVENT_101);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 3);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);

    OHOS::bluetooth::BleScanSettingsImpl settings;
    BleScannerStateMachine::ScannerParameters params(settings);
    EXPECT_EQ(params.callbackType, settings.GetCallbackType());
    EXPECT_EQ(params.matchTrackAdvType, settings.GetMatchTrackAdvType());
    EXPECT_EQ(params.matchMode, settings.GetMatchMode());
    HILOGI("ble_scanner_state_machine_test_021 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_022
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(>) -> StopSuccess -> Inner Start ->
                StartSuccess -> Stop -> StopSuccess -> Start -> StartSuccess.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_022, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_022 enter");

    BleScannerMessage T_START_EVENT_4_SCAN_600_600(T_START_EVENT_4);
    T_START_EVENT_4_SCAN_600_600.scanParams.scanInterval = 600;
    T_START_EVENT_4_SCAN_600_600.scanParams.scanWindow = 600;

    NiceMock<BleScannerInterfaceMock> interface;

    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4_SCAN_600_600)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_2);
    StartCycleLarger(T_START_EVENT_4);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    stateMachine_->SendMessage(T_STOP_EVENT_4);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_022 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_023
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Start(<) -> Stop -> StopSuccess.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_023, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_023 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_2);
    StartCycleLower(T_START_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_STOP_EVENT_2);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    sleep(TIME);
    HILOGI("ble_scanner_state_machine_test_023 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_024
 * @tc.desc: Verify the BleScannerStateMachine:
                ReportStartedEvent or ReportStoppedEvent.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_024, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_024 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, ReportStartedEvent(10, 10, 1, 0)).Times(1);
        EXPECT_CALL(interface, ReportStoppedEvent(10, 10, 1, 0)).Times(1);
        EXPECT_CALL(interface, StopScan());
    }
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);

    HILOGI("ble_scanner_state_machine_test_024 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_025
 * @tc.desc: Verify the BleScannerStateMachine:
                ReportStartedEvent or ReportStoppedEvent.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_025, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_025 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        /* Start-1 60/600 */
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, ReportStartedEvent(20, 20, 2, 0)).Times(1);
        /* Start-2 参数不跟新60/600 */
        EXPECT_CALL(interface, ReportStartedEvent(10, 10, 100, 0)).Times(1);
        /* Start-3 参数更新60/240 */
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
        EXPECT_CALL(interface, ReportStartedEvent(30, 30, 3, 0)).Times(1);
        /* Stop-3 参数更新60/600 */
        EXPECT_CALL(interface, ReportStoppedEvent(30, 30, 3, 0)).Times(1);
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        /* Start-4 参数更新600/600 */
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4)));
        EXPECT_CALL(interface, ReportStartedEvent(40, 40, 4, 0)).Times(1);
        /* Stop-4 参数更新60/600 */
        EXPECT_CALL(interface, ReportStoppedEvent(40, 40, 4, 0)).Times(1);
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_100)));
        /* Stop-2 参数不更新*/
        EXPECT_CALL(interface, ReportStoppedEvent(10, 10, 100, 0)).Times(1);
        /* Stop-1 appQueue empty 参数不更新 0,0*/
        EXPECT_CALL(interface, ReportStoppedEvent(20, 20, 2, 0)).Times(1);
        EXPECT_CALL(interface, StopScan());
    }

    /* Start-1 60/600 */
    stateMachine_->SendMessage(T_START_EVENT_2);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    /* Start-2 参数不跟新60/600 */
    stateMachine_->SendMessage(T_START_EVENT_100);
    /* Start-3 参数更新60/240 */
    stateMachine_->SendMessage(T_START_EVENT_3);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    /* Stop-3 参数更新60/600 */
    stateMachine_->SendMessage(T_STOP_EVENT_3);
    EXPECT_EQ(stateMachine_->isStartByStateMachine_, true);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    /* Start-4 参数更新600/600 */
    stateMachine_->SendMessage(T_START_EVENT_4);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    /* Stop-4 参数更新60/600 */
    stateMachine_->SendMessage(T_STOP_EVENT_4);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    /* Stop-2 参数不更新60/600 */
    stateMachine_->SendMessage(T_STOP_EVENT_100);
    /* Stop-1 appQueue empty 参数不更新 0,0*/
    stateMachine_->SendMessage(T_STOP_EVENT_2);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
}

/**
 * @tc.name: ble_scanner_state_machine_test_026
 * @tc.desc: Verify the BleScannerStateMachine:
                ReportStartedEvent FAILED.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_026, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_026 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, ReportStartedEvent(10, 10, 1, 3)).Times(1);
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_FAILED_EVENT);
    EXPECT_EQ(stateMachine_->isStartByStateMachine_, false);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_026 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_027
 * @tc.desc: Verify the BleScannerStateMachine:
                1、Stopped after STOP_EVENT，ReportStoppedEvent SCAN_NOT_STARTED.
                2、Started after STOP_EVENT，ReportStartedEvent SCAN_NOT_STARTED.
                3、Started after START_EVENT，ReportStartedEvent SCAN_FAILED_ALREADY_STARTED
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_027, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_027 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, ReportStoppedEvent(10, 10, 1, -1)).Times(1);

        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, ReportStartedEvent(10, 10, 1, 0)).Times(1);

        EXPECT_CALL(interface, ReportStoppedEvent(20, 20, 2, -1)).Times(1);

        EXPECT_CALL(interface, ReportStartedEvent(10, 10, 1, 1)).Times(1);
    }

    /* Stopped after STOP_EVENT，ReportStoppedEvent SCAN_NOT_STARTED */
    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_STOP_EVENT_1);

    stateMachine_->SendMessage(T_START_EVENT_1);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);

    /* Started after STOP_EVENT，ReportStoppedEvent SCAN_NOT_STARTED */
    stateMachine_->SendMessage(T_STOP_EVENT_2);

    /* Started after START_EVENT，ReportStartedEvent SCAN_FAILED_ALREADY_STARTED */
    stateMachine_->SendMessage(T_START_EVENT_1);

    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    HILOGI("ble_scanner_state_machine_test_027 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_028
 * @tc.desc: Verify the BleScannerStateMachine:
                Start event4 -> (event4)Starting -> Stop event1(not exist) -> Stop event4 -> StartSuccess ->
                (event4)Started -> (event4)Stoping -> StopSuccess -> (event4)Stoped
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_028, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_028 enter");
    // stop event pending
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4)));
        EXPECT_CALL(interface, StopScan());
    }
    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_4);
    CHECK_STATE_STARTING();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_2);
    stateMachine_->SendMessage(T_STOP_EVENT_4);
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 3);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_028 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_029
 * @tc.desc: Verify the BleScannerStateMachine:
                Start event2 -> (event2)Starting -> Start event2(already exist) -> Start event4 -> StartSuccess ->
                (event2)Started -> inner Stoping -> StopSuccess -> StartSuccess -> (event4)Started
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_029, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_029 enter");
    // start event pending
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4)));
    }
    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_STARTING();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    stateMachine_->SendMessage(T_START_EVENT_2);
    stateMachine_->SendMessage(T_START_EVENT_2);
    stateMachine_->SendMessage(T_START_EVENT_4);
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 3);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_029 end");
}

#ifdef CONTEXTHUB_BLE_V3
void BleScannerStateMachineTest::ShCollaborationEventCheck()
{
    DisableShCollaboration();

    // send SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN_EVENT
    stateMachine_->SendMessage(T_SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN_EVENT);
    EXPECT_EQ(stateMachine_->IsNeedCollboration(), false);

    // send T_SH_COLLABORATION_INVALID_EVENT
    stateMachine_->SendMessage(T_SH_COLLABORATION_INVALID_EVENT);
    EXPECT_EQ(stateMachine_->IsNeedCollboration(), false);

    // send T_SH_COLLABORATION_EVENT_30_300 and nothing happened
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_30_300);
}

/**
 * @tc.name: ble_scanner_state_machine_test_030
 * @tc.desc: Handle collaboration event in stopped state of BleScannerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_030, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_030 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    CHECK_STATE_STOPPED();
    DisableShCollaboration();

    ShCollaborationEventCheck();

    // state not changed in collaboration result
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_600);
    CHECK_STATE_STOPPED();
    HILOGI("ble_scanner_state_machine_test_030 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_031
 * @tc.desc: Handle collaboration event in starting state of BleScannerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_031, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_031 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
    }
    CHECK_STATE_STOPPED();
    DisableShCollaboration();

    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTING();

    ShCollaborationEventCheck();

    // state not changed in collaboration result
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_600);
    CHECK_STATE_STARTING();

    HILOGI("ble_scanner_state_machine_test_031 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_032
 * @tc.desc: Handle collaboration event in started state of BleScannerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_032, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_032 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
    }
    DisableShCollaboration();
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    CHECK_STATE_STARTED();

    ShCollaborationEventCheck();

    // state not changed in collaboration result
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_600);
    CHECK_STATE_STARTED();

    HILOGI("ble_scanner_state_machine_test_032 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_033
 * @tc.desc: Handle collaboration event in stopping state of BleScannerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_033, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_033 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
    }
    DisableShCollaboration();
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPING();

    ShCollaborationEventCheck();

    // state not changed in collaboration result
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_600);
    CHECK_STATE_STOPPING();

    HILOGI("ble_scanner_state_machine_test_033 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_034
 * @tc.desc: Handle collaboration event in inner stopping state of BleScannerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_034, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_034 enter");
        BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        //EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_INNER_STOPPING();

    ShCollaborationEventCheck();

    // state not changed in collaboration result
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_600);
    CHECK_STATE_INNER_STOPPING();

    HILOGI("ble_scanner_state_machine_test_034 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_036
 * @tc.desc: Verify the BleScannerStateMachine:
                Stop -> ShCollaboration(sh scan not started) -> Starting -> Started
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_036, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_036 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
    }
    CHECK_STATE_STOPPED();
    EnableShCollaboration();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_NOT_STARTED);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);

    HILOGI("ble_scanner_state_machine_test_036 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_037
 * @tc.desc: Verify the BleScannerStateMachine:
                Stop -> ShCollaboration(sh scan not started and remove event) -> Stop
                Stop -> ShCollaboration(sh scan not started, remove event and currentApp_ not null) -> Started
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_037, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_037 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    CHECK_STATE_STOPPED();
    EnableShCollaboration();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_SH_COLLABORATION();

    // remove event
    stateMachine_->priorityEventQueue_.pop_front();

    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_NOT_STARTED);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);

    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_SH_COLLABORATION();

    // remove event
    stateMachine_->priorityEventQueue_.pop_front();
    stateMachine_->currentApp_ = std::make_shared<BleScannerStateMachine::ScannerAppInfo>(
        0, 0, 0, BleScannerStateMachine::ScannerParameters(0, 7, 0, 0));
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_NOT_STARTED);
    CHECK_STATE_STARTED();

    HILOGI("ble_scanner_state_machine_test_037 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_038
 * @tc.desc: Verify the BleScannerStateMachine:
                Stop -> ShCollaboration(sh scan not started and stop event) -> Stop
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_038, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_0032 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    CHECK_STATE_STOPPED();
    EnableShCollaboration();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_SH_COLLABORATION();

    // remove event
    stateMachine_->priorityEventQueue_.pop_front();
    stateMachine_->SendMessage(T_STOP_EVENT_1);

    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_NOT_STARTED);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);

    HILOGI("ble_scanner_state_machine_test_038 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_039
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess(AP 60/3000) -> Stop -> ShCollaboration(stop AP 60/3000, sh 60/3000) ->
                Stopping -> Stopped -> Starting -> Started(sh 60/3000)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_039, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_039 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_SH_COLLABORATION_EVENT_60_3000)));
    }
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);

    EnableShCollaboration();
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_3000);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->currentApp_->scannerId, BleScannerStateMachine::LP_DEVICE_SCANNER_ID);

    HILOGI("ble_scanner_state_machine_test_039 end");
}

void BleScannerStateMachineTest::StartingAndStoppingCollaborationStateCheck(
    const BleScannerMessage &startEvent, const BleScannerMessage &shEventInStartEvent,
    const BleScannerMessage &stopEvent, const BleScannerMessage &shEventInStopEvent)
{
    stateMachine_->SendMessage(startEvent);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(shEventInStartEvent);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->currentApp_->scannerId, startEvent.appInfo.scannerId);

    // stop the larger AP duty cycle
    stateMachine_->SendMessage(stopEvent);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(shEventInStopEvent);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
}

/**
 * @tc.name: ble_scanner_state_machine_test_040
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(AP 60/3000) -> StartSuccess -> Start(AP 60/3000) ->
                ShCollaboration(start AP 60/600, sh 60/600) -> Inner Stopping -> Start(AP 60/600) ->
                ShCollaboration(stop AP 60/600) -> state not changed -> Start(sh 60/600) ->
                ShCollaboration(start AP 60/240, sh 100/200) -> Inner Stopping -> Stopped -> Start(AP 60/240) ->
                Inner Stopping -> Stopped -> Start(sh 100/200) ->
                Inner Stopping(sh scanner is removed) -> Stopped -> Start(AP 60/240)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_040, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_040 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_SH_COLLABORATION_EVENT_100_200)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
    }
    StartCycleInit(T_START_EVENT_1);

    EnableShCollaboration();

    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_600);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->currentApp_->scannerId, T_START_EVENT_2.appInfo.scannerId);

    // stop the larger AP duty cycle and no stop
    stateMachine_->SendMessage(T_STOP_EVENT_2);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, T_START_EVENT_2.appInfo.params.scanWindow);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, T_START_EVENT_2.appInfo.params.scanInterval);

    // start the AP 60/240 and sh duty cycle changed from 60/600 to 100/200
    stateMachine_->SendMessage(T_START_EVENT_3);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_100_200);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_INNER_STOPPING(); // started state will start 100/200 scan duty
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->currentApp_->scannerId, BleScannerStateMachine::LP_DEVICE_SCANNER_ID);

    // remove lp device scan
    stateMachine_->SendMessage(T_SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN_EVENT);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_NOT_STARTED);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->currentApp_->scannerId, T_START_EVENT_3.appInfo.scannerId);
    HILOGI("ble_scanner_state_machine_test_040 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_041
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(AP 60/3000) -> StartSuccess -> Start -> ShCollaboration(AP 60/240, sh 30/300) ->
                Inner Stopping -> Start(AP 60/240) ->
                ShCollaboration(stop AP 60/240, sh update to 60/600) ->
                Inner Stopping -> Stopped -> Start(sh duty cycle 30/300) ->
                Inner Stopping -> Stopped -> Start(AP 60/3000) ->
                Inner Stopping -> Stopped -> Start(sh 60/600)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_041, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_041 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_SH_COLLABORATION_EVENT_30_300)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_SH_COLLABORATION_EVENT_60_600)));
    }
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    uint32_t appCount = stateMachine_->appQueue_.size();

    EnableShCollaboration();

    // now starting sh 30/300
    StartingAndStoppingCollaborationStateCheck(T_START_EVENT_3, T_SH_COLLABORATION_EVENT_30_300,
        T_STOP_EVENT_3, T_SH_COLLABORATION_EVENT_60_600);
    appCount++;
    EXPECT_EQ(stateMachine_->appQueue_.size(), appCount);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 2);

    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STOPPING(); // stopping sh 30/300
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING(); // starting AP T_START_EVENT_1
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_INNER_STOPPING(); // stopping AP T_START_EVENT_1
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING(); // starting sh 60/600
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->currentApp_->scannerId, BleScannerStateMachine::LP_DEVICE_SCANNER_ID);

    HILOGI("ble_scanner_state_machine_test_041 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_042
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(AP 60/600) -> StartSuccess -> Start -> ShCollaboration(AP 60/240, sh 30/300) ->
                Inner Stopping -> Start(AP 60/240) ->
                ShCollaboration(stop AP 60/240, sh update to 60/3000) ->
                Inner Stopping -> Stopped -> Start(sh duty cycle 30/300) ->
                Inner Stopping -> Stopped -> Start(AP 60/600)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_042, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_042 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_SH_COLLABORATION_EVENT_30_300)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_2);

    EnableShCollaboration();
    stateMachine_->SendMessage(T_START_EVENT_3);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_30_300);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 3);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->currentApp_->scannerId, T_START_EVENT_3.appInfo.scannerId);

    // stop the larger AP duty cycle and sh cycle changed to 60/3000
    stateMachine_->SendMessage(T_STOP_EVENT_3);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_3000);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING(); // starting sh 30/300
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 2);

    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STOPPING(); // stopping sh 30/300
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING(); // starting AP T_START_EVENT_2
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->currentApp_->scannerId, T_START_EVENT_2.appInfo.scannerId);

    HILOGI("ble_scanner_state_machine_test_042 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_043
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(AP 60/3000) -> StartSuccess -> Start -> ShCollaboration(sh duty cycle equal to AP 60/600) ->
                Inner Stopping -> Start -> no change(stop AP 60/600 and sh duty cycle equal to EVENT2) ->
                Start(sh duty cycle) -> no change (stop AP 60/3000)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_043, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_043 enter");
    BleScannerInterfaceMock interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);

    EnableShCollaboration();
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_SH_COLLABORATION();
    stateMachine_->SendMessage(T_SH_COLLABORATION_EVENT_60_600);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 3);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, T_START_EVENT_2.appInfo.params.scanWindow);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, T_START_EVENT_2.appInfo.params.scanInterval);

    // stop the larger AP duty cycle
    stateMachine_->SendMessage(T_STOP_EVENT_2);
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, T_START_EVENT_2.appInfo.params.scanWindow);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, T_START_EVENT_2.appInfo.params.scanInterval);

    // stop the AP scan and notify lp device the result
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, T_START_EVENT_2.appInfo.params.scanWindow);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, T_START_EVENT_2.appInfo.params.scanInterval);
    HILOGI("ble_scanner_state_machine_test_043 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_044
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> Stop -> ShCollaboration(timeout) ->
                Stopping -> Stopped
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_044, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_044 enter");
    BleScannerInterfaceMock interface;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner(AppExecFwk::EventRunner::Create("ble_scanner_sm_test_007"));
    auto eventHandler = std::make_shared<AppExecFwk::EventHandler>(eventRunner);
    auto postTask = [eventHandler](std::function<void(void)> func) {
        eventHandler->PostTask(func);
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    };
    auto startScan = [&interface](const OHOS::bluetooth::BleScanSettingsImpl &setting) {
        interface.StartScan(setting);
    };
    auto stopScan = [&interface]() { interface.StopScan(); };
    auto sendFunc = [](int type, const std::vector<BleScannerStateMachine::ScannerAppInfo> &appInfos) {};
    auto collaborationEnabledFunc = []() { return true; };
    auto reportStartedEvent = [&interface](int pid, int uid, int scannerId, int resultCode) {};
    auto reportStoppedEvent = [&interface](int pid, int uid, int scannerId, int resultCode) {};
    stateMachine_->Init(postTask, startScan, stopScan, reportStartedEvent, reportStoppedEvent,
        BleScannerStateMachine::CollabrationFunc(sendFunc, collaborationEnabledFunc));
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
    }
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);

    const int timeoutDelay = 3000;
    stateMachine_->SetTimeoutMs(timeoutDelay);

    EnableShCollaboration();
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_SH_COLLABORATION();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();

    HILOGI("ble_scanner_state_machine_test_044 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_045
 * @tc.desc: Verify the BleScannerStateMachine:
                Start -> StartSuccess -> ShCollaboration(timeout) ->
                Inner Stopping -> Started
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_045, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_045 enter");
    BleScannerInterfaceMock interface;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner(AppExecFwk::EventRunner::Create("ble_scanner_sm_test_007"));
    auto eventHandler = std::make_shared<AppExecFwk::EventHandler>(eventRunner);
    auto postTask = [eventHandler](std::function<void(void)> func) {
        eventHandler->PostTask(func);
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    };
    auto startScan = [&interface](const OHOS::bluetooth::BleScanSettingsImpl &setting) {
        interface.StartScan(setting);
    };
    auto stopScan = [&interface]() { interface.StopScan(); };
    auto sendFunc = [](int type, const std::vector<BleScannerStateMachine::ScannerAppInfo> &appInfos) {};
    auto collaborationEnabledFunc = []() { return true; };
    auto reportStartedEvent = [&interface](int pid, int uid, int scannerId, int resultCode) {};
    auto reportStoppedEvent = [&interface](int pid, int uid, int scannerId, int resultCode) {};
    stateMachine_->Init(postTask, startScan, stopScan, reportStartedEvent, reportStoppedEvent,
        BleScannerStateMachine::CollabrationFunc(sendFunc, collaborationEnabledFunc));
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);

    const int timeoutDelay = 3000;
    stateMachine_->SetTimeoutMs(timeoutDelay);

    EnableShCollaboration();
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_SH_COLLABORATION();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();

    HILOGI("ble_scanner_state_machine_test_045 end");
}
#endif // CONTEXTHUB_BLE_V3

}  // namespace {}

/**
 * @tc.name: ble_scanner_state_machine_test_046
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(4000/4000) -> StartSucess -> Start(30/300)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_046, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_046 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_102)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_102);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_START_EVENT_103);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, BLE_SCAN_MODE_LOW_LATENCY_WINDOW_MS);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, BLE_SCAN_MODE_LOW_LATENCY_INTERVAL_MS);

    HILOGI("ble_scanner_state_machine_test_046 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_047
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(30/300) -> StartSucess -> Start(4000/4000) -> StopSuccess -> Inner Start ->
                StartSuccess
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_047, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_047 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_103)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_102)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_103);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_START_EVENT_102);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();

    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, BLE_SCAN_MODE_LOW_LATENCY_WINDOW_MS);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, BLE_SCAN_MODE_LOW_LATENCY_INTERVAL_MS);

    HILOGI("ble_scanner_state_machine_test_047 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_048
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(1000/1000) -> StartSucess -> Start(4000/4000)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_048, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_048 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_4);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_START_EVENT_102);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, BLE_SCAN_MODE_OP_P100_1000_1000_WINDOW_MS);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, BLE_SCAN_MODE_OP_P100_1000_1000_INTERVAL_MS);

    HILOGI("ble_scanner_state_machine_test_048 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_049
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(4000/4000) -> StartSucess -> Start(1000/1000)
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_049, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_049 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_102)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_102);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_START_EVENT_4);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, BLE_SCAN_MODE_LOW_LATENCY_WINDOW_MS);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, BLE_SCAN_MODE_LOW_LATENCY_INTERVAL_MS);

    HILOGI("ble_scanner_state_machine_test_049 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_050
 * @tc.desc: Verify the BleScannerStateMachine:
                Start(30/300) -> StartSucess -> Start(60/240) -> Start(60/240) -> Start(60/240) -> StartSucess
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_050, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_050 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_103)));
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_103);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_START_EVENT_3);
    CHECK_STATE_INNER_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_STOP_EVENT_3);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_EVENT_3);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_STOP_EVENT_3);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_EVENT_3);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->optimalParam_.scanWindow, BLE_SCAN_MODE_OP_P25_60_240_WINDOW_MS);
    EXPECT_EQ(stateMachine_->optimalParam_.scanInterval, BLE_SCAN_MODE_OP_P25_60_240_INTERVAL_MS);

    HILOGI("ble_scanner_state_machine_test_050 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_051
 * @tc.desc: Verify the BleScannerStateMachine:
 *                The app stopped scan before a resume START_EVENT arrives in StateStopped
 *                (e.g. the StopScan request races with the RSS unfreeze resume scan),
 *                the START_EVENT is discarded and the scan is not restarted.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_051, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_051 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    appActiveScan_ = false;
    InitStateMachine(interface, true);
    {
        InSequence s;
        EXPECT_CALL(interface, ReportStoppedEvent(10, 10, 1, -1)).Times(1);
    }
    EXPECT_CALL(interface, StartScan(_)).Times(0);
    EXPECT_CALL(interface, StopScan()).Times(0);

    /* StopScan request is dropped in StateStopped as before */
    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPED();

    /* resume START_EVENT for the app which has stopped scan is discarded */
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_051 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_052
 * @tc.desc: Verify the BleScannerStateMachine:
 *                The resume START_EVENT for the app which is still scanning works normally.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_052, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_052 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    appActiveScan_ = true;
    InitStateMachine(interface, true);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
    }

    CHECK_STATE_STOPPED();
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_052 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_053
 * @tc.desc: Verify the BleScannerStateMachine:
 *                Start -> StartSuccess -> Stop -> (app stops scan) -> resume Start(pending) -> StopSuccess ->
 *                the pending START_EVENT is discarded in the Entry of StateStopped.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_053, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_053 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    appActiveScan_ = true;
    InitStateMachine(interface, true);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, ReportStartedEvent(10, 10, 1, 0)).Times(1);
        EXPECT_CALL(interface, ReportStoppedEvent(10, 10, 1, 0)).Times(1);
        EXPECT_CALL(interface, StopScan());
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    stateMachine_->SendMessage(T_STOP_EVENT_1);
    CHECK_STATE_STOPPING();

    /* the app stopped scan while the state machine is stopping */
    appActiveScan_ = false;
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STOPPING();

    /* the pending resume START_EVENT is discarded in the Entry of StateStopped */
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_053 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_054
 * @tc.desc: Verify the BleScannerStateMachine:
 *                Start -> StartSuccess -> (app2 stopped scan) -> resume Start(app2) ->
 *                the START_EVENT is ignored in StateStarted and the running scan is not affected.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_054, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_054 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    appActiveScan_ = true;
    InitStateMachine(interface, true);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1)));
        EXPECT_CALL(interface, ReportStartedEvent(10, 10, 1, 0)).Times(1);
    }
    EXPECT_CALL(interface, StopScan()).Times(0);

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    CHECK_STATE_STARTED();

    /* the resume START_EVENT for the app which has stopped scan is ignored in StateStarted */
    appActiveScan_ = false;
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);
    HILOGI("ble_scanner_state_machine_test_054 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_055
 * @tc.desc: Verify the BleScannerStateMachine:
 *                Multiple apps are scanning. The app with the max duty cycle stops scanning and
 *                triggers a parameter re-negotiation stop, and the app selected for the resume
 *                also stops scanning while the state machine is stopping. The scan of the app
 *                which is still scanning is resumed by re-selecting from the appQueue in the
 *                Entry of StateStopped.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_055, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_055 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    appActiveScan_ = true;
    InitStateMachine(interface, true);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_4)));
        EXPECT_CALL(interface, ReportStartedEvent(40, 40, 4, 0)).Times(1);
        EXPECT_CALL(interface, ReportStartedEvent(30, 30, 3, 0)).Times(1);
        EXPECT_CALL(interface, ReportStartedEvent(20, 20, 2, 0)).Times(1);
        EXPECT_CALL(interface, ReportStoppedEvent(40, 40, 4, 0)).Times(1);
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
    }

    /* app4(max duty cycle), app3 and app2 are scanning */
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_4);
    stateMachine_->SendMessage(T_START_EVENT_3);
    CHECK_STATE_STARTED();
    stateMachine_->SendMessage(T_START_EVENT_2);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 3);

    /* app4 stops scanning, the optimal parameter is changed and the scan is stopping for
       re-negotiation with the resume START_EVENT(app3) queued */
    stoppedScannerIds_.insert(4);
    stateMachine_->SendMessage(T_STOP_EVENT_4);
    CHECK_STATE_STOPPING();

    /* app3 also stops scanning while the state machine is stopping */
    stoppedScannerIds_.insert(3);
    stateMachine_->SendMessage(T_STOP_EVENT_3);
    CHECK_STATE_STOPPING();

    /* the pending resume START_EVENT(app3) is discarded, and the scan of app2 which is still
       scanning is resumed by re-selecting from the appQueue */
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);

    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    HILOGI("ble_scanner_state_machine_test_055 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_056
 * @tc.desc: Verify the BleScannerStateMachine:
 *                All the apps stop scanning while the state machine is stopping for parameter
 *                re-negotiation, no scan is resumed. The isStartByStateMachine_ flag is cleared
 *                so that the start event report of a subsequent external START_EVENT is not
 *                suppressed.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_056, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_056 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    appActiveScan_ = true;
    InitStateMachine(interface, true);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_2)));
        EXPECT_CALL(interface, ReportStartedEvent(20, 20, 2, 0)).Times(1);
        EXPECT_CALL(interface, ReportStartedEvent(10, 10, 1, 0)).Times(1);
        EXPECT_CALL(interface, ReportStoppedEvent(20, 20, 2, 0)).Times(1);
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_3)));
        EXPECT_CALL(interface, ReportStartedEvent(30, 30, 3, 0)).Times(1);
    }

    /* app2(max duty cycle) and app1 are scanning */
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_2);
    stateMachine_->SendMessage(T_START_EVENT_1);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);

    /* app2 stops scanning, the optimal parameter is changed and the scan is stopping for
       re-negotiation with the resume START_EVENT(app1) queued */
    stoppedScannerIds_.insert(2);
    stateMachine_->SendMessage(T_STOP_EVENT_2);
    CHECK_STATE_STOPPING();

    /* app1 also stops scanning while the state machine is stopping, no scan is resumed
       after the scan stops */
    stoppedScannerIds_.insert(1);
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STOPPED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 0);
    EXPECT_EQ(stateMachine_->priorityEventQueue_.size(), 0);

    /* a new app starts scanning, its start event report is not suppressed */
    stateMachine_->SendMessage(T_START_EVENT_3);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 1);
    HILOGI("ble_scanner_state_machine_test_056 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_057
 * @tc.desc: Verify the BleScannerStateMachine:
 *                app1 starts scanning with legacy mode, then app5 starts scanning with extended
 *                scan(legacy == false) using the same scan parameters. The scan must be
 *                restarted with legacy == false. After app5 stops scanning, the scan is
 *                restored to legacy == true.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_057, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_057 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    InitStateMachine(interface);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1))); // legacy == true
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_5))); // legacy == false
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_1))); // legacy == true
    }

    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_1);
    /* The extended scan app joins, the scan is restarted with legacy == false. */
    StartCycleLarger(T_START_EVENT_5);
    EXPECT_FALSE(stateMachine_->optimalParam_.legacy);
    /* The extended scan app stops, the scan is restarted with legacy == true. */
    stateMachine_->SendMessage(T_STOP_EVENT_5);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_TRUE(stateMachine_->optimalParam_.legacy);
    HILOGI("ble_scanner_state_machine_test_057 end");
}

/**
 * @tc.name: ble_scanner_state_machine_test_058
 * @tc.desc: Verify the BleScannerStateMachine:
 *                During the re-negotiation window, one app stops and another app starts. The
 *                legacy mode sent to the stack must still be merged from all the scanning
 *                apps, not the mode of the app which triggers the start event.
 * @tc.type: FUNC
 */
HWTEST_F(BleScannerStateMachineTest, ble_scanner_state_machine_test_058, TestSize.Level0)
{
    HILOGI("ble_scanner_state_machine_test_058 enter");
    NiceMock<BleScannerInterfaceMock> interface;
    appActiveScan_ = true;
    InitStateMachine(interface, true);
    auto app7Setting = GetTestScanSetting(T_START_EVENT_7);
    app7Setting.SetLegacy(false);
    auto app8Setting = GetTestScanSetting(T_START_EVENT_8);
    app8Setting.SetLegacy(false);
    {
        InSequence s;
        EXPECT_CALL(interface, StartScan(GetTestScanSetting(T_START_EVENT_6))); // legacy == false
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(app7Setting)); // merged legacy == false
        EXPECT_CALL(interface, StopScan());
        EXPECT_CALL(interface, StartScan(app8Setting)); // merged legacy == false
    }

    /* app6(extended) and app9(extended) are scanning */
    CHECK_STATE_STOPPED();
    StartCycleInit(T_START_EVENT_6);
    stateMachine_->SendMessage(T_START_EVENT_9);
    CHECK_STATE_STARTED();
    EXPECT_EQ(stateMachine_->appQueue_.size(), 2);

    /* app7(legacy, higher duty cycle) joins and the scan is restarted with merged legacy == false */
    StartCycleLarger(T_START_EVENT_7);
    EXPECT_FALSE(stateMachine_->optimalParam_.legacy);

    /* app7 stops, the scan is stopped for re-negotiation */
    stoppedScannerIds_.insert(7);
    stateMachine_->SendMessage(T_STOP_EVENT_7);
    CHECK_STATE_STOPPING();

    /* In the re-negotiation window, app6 stops and app8(legacy) starts. The queued resume
       START_EVENT(app6) is discarded, and app8 starts with merged legacy == false because
       app9 is still scanning with extended scan. */
    stoppedScannerIds_.insert(6);
    stateMachine_->SendMessage(T_STOP_EVENT_6);
    stateMachine_->SendMessage(T_START_EVENT_8);
    CHECK_STATE_STOPPING();
    stateMachine_->SendMessage(T_STOP_SUCCESS_EVENT);
    CHECK_STATE_STARTING();
    stateMachine_->SendMessage(T_START_SUCCESS_EVENT);
    CHECK_STATE_STARTED();
    EXPECT_FALSE(stateMachine_->optimalParam_.legacy);
    HILOGI("ble_scanner_state_machine_test_058 end");
}
