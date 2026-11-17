/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#include <memory>
#include <string>
#include <chrono>
#include <thread>

#include "bas_service_impl.h"
#include "bas_service_impl_interface.h"
#include "bas_message.h"
#include "bas_defines.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace {
const std::string TEST_DEVICE_ADDRESS = "11:22:33:44:55:66";
const std::string TEST_DEVICE_ADDRESS2 = "22:33:44:55:66:77";
const std::string TEST_DEVICE_ADDRESS3 = "33:44:55:66:77:88";
}

class BasServiceImplTestBasObserver : public IBasObserver {
public:
    BasServiceImplTestBasObserver() = default;
    ~BasServiceImplTestBasObserver() override = default;

    void OnGetBatteryLevelEvent(const RawAddress &deviceAddress, int32_t batteryLevel) override
    {}
    void OnBatteryLevelChanged(const RawAddress &deviceAddress, int32_t batteryLevel) override
    {}
};

class BasServiceImplTest : public testing::Test {
public:
    BasServiceImplTest() = default;
    ~BasServiceImplTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

protected:
    std::unique_ptr<BasServiceImpl> basServiceImpl_ = nullptr;
    std::shared_ptr<IBasObserver> basObserver_ = nullptr;
};

void BasServiceImplTest::SetUpTestCase(void)
{
    HILOGI("BasServiceImplTest::SetUpTestCase start");
}

void BasServiceImplTest::TearDownTestCase(void)
{
    HILOGI("BasServiceImplTest::TearDownTestCase start");
}

void BasServiceImplTest::SetUp()
{
    HILOGI("BasServiceImplTest::SetUp start");
    basServiceImpl_ = std::make_unique<BasServiceImpl>();
    basObserver_ = std::make_shared<BasServiceImplTestBasObserver>();
    basServiceImpl_->Init();
}

void BasServiceImplTest::TearDown()
{
    HILOGI("BasServiceImplTest::TearDown start");
    if (basServiceImpl_ != nullptr) {
        basServiceImpl_->DeregisterObserver(basObserver_);
        basServiceImpl_->DeInit();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    basServiceImpl_.reset();
}

/**
 * @tc.number: BasServiceImpl_DeregisterObserver_001
 * @tc.name: testDeregisterObserver
 * @tc.desc: 测试观察者注销功能,验证观察者能成功从服务实现注销
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_DeregisterObserver_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_DeregisterObserver_001 start");
    EXPECT_NE(basServiceImpl_, nullptr);
    basServiceImpl_->RegisterObserver(basObserver_);
    basServiceImpl_->DeregisterObserver(basObserver_);
    HILOGI("BasServiceImpl_DeregisterObserver_001 end");
}

/**
 * @tc.number: BasServiceImpl_Connect_001
 * @tc.name: testConnect
 * @tc.desc: 测试设备连接功能,验证连接操作返回BAS_SUCCESS
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_Connect_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_Connect_001 start");
    EXPECT_NE(basServiceImpl_, nullptr);
    RawAddress device(TEST_DEVICE_ADDRESS);
    int ret = basServiceImpl_->Connect(device);
    EXPECT_EQ(ret, BAS_SUCCESS);
    HILOGI("BasServiceImpl_Connect_001 end");
}

/**
 * @tc.number: BasServiceImpl_Disconnect_001
 * @tc.name: testDisconnect
 * @tc.desc: 测试设备断开连接功能,验证断开操作返回BAS_SUCCESS
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_Disconnect_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_Disconnect_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int ret = basServiceImpl_->Disconnect(device);
    EXPECT_NE(ret, BAS_SUCCESS);
    HILOGI("BasServiceImpl_Disconnect_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetBatteryLevel_001
 * @tc.name: testGetBatteryLevel
 * @tc.desc: 测试获取设备电池电量,验证返回值不为BAS_SUCCESS
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetBatteryLevel_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetBatteryLevel_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int ret = basServiceImpl_->GetBatteryLevel(device);
    EXPECT_NE(ret, BAS_SUCCESS);
    HILOGI("BasServiceImpl_GetBatteryLevel_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetDeviceState_001
 * @tc.name: testGetDeviceState
 * @tc.desc: 测试获取设备状态,验证未连接设备状态为BAS_STATE_DISCONNECTED
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetDeviceState_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetDeviceState_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int state = basServiceImpl_->GetDeviceState(device);
    EXPECT_EQ(state, static_cast<int>(BTConnectState::DISCONNECTED));
    HILOGI("BasServiceImpl_GetDeviceState_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetConnectDevices_001
 * @tc.name: testGetConnectDevices
 * @tc.desc: 测试获取已连接设备列表,验证无连接时返回空列表
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetConnectDevices_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetConnectDevices_001 start");
    auto devices = basServiceImpl_->GetConnectDevices();
    EXPECT_TRUE(devices.empty());
    HILOGI("BasServiceImpl_GetConnectDevices_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetConnectState_001
 * @tc.name: testGetConnectState
 * @tc.desc: 测试获取服务连接状态,验证返回值大于等于0
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetConnectState_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetConnectState_001 start");
    int state = basServiceImpl_->GetConnectState();
    EXPECT_GE(state, 0);
    HILOGI("BasServiceImpl_GetConnectState_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetDevicesByStates_001
 * @tc.name: testGetDevicesByStatesDisconnected
 * @tc.desc: 测试按断开状态获取设备列表,验证能返回设备列表
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetDevicesByStates_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetDevicesByStates_001 start");
    std::vector<int> states = {static_cast<int>(BTConnectState::DISCONNECTED)};
    auto devices = basServiceImpl_->GetDevicesByStates(states);
    EXPECT_TRUE(devices.empty());
    HILOGI("BasServiceImpl_GetDevicesByStates_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetDevicesByStates_002
 * @tc.name: testGetDevicesByStatesConnected
 * @tc.desc: 测试按连接状态获取设备列表,验证无连接设备时返回空列表
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetDevicesByStates_002, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetDevicesByStates_002 start");
    std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    auto devices = basServiceImpl_->GetDevicesByStates(states);
    EXPECT_TRUE(devices.empty());
    HILOGI("BasServiceImpl_GetDevicesByStates_002 end");
}

/**
 * @tc.number: BasServiceImpl_GetDevicesByStates_003
 * @tc.name: testGetDevicesByStatesMultiple
 * @tc.desc: 测试按多个状态获取设备列表,验证能返回设备列表
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetDevicesByStates_003, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetDevicesByStates_003 start");
    std::vector<int> states = {
        static_cast<int>(BTConnectState::CONNECTED),
        static_cast<int>(BTConnectState::DISCONNECTED)
    };
    auto devices = basServiceImpl_->GetDevicesByStates(states);
    EXPECT_TRUE(devices.empty());
    HILOGI("BasServiceImpl_GetDevicesByStates_003 end");
}

/**
 * @tc.number: BasServiceImpl_GetConnectedDeviceBatteryInfos_001
 * @tc.name: testGetConnectedDeviceBatteryInfos
 * @tc.desc: 测试获取已连接设备的电池信息,验证无连接设备时返回空列表
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetConnectedDeviceBatteryInfos_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetConnectedDeviceBatteryInfos_001 start");
    auto batteryInfos = basServiceImpl_->GetConnectedDeviceBatteryInfos();
    EXPECT_TRUE(batteryInfos.empty());
    HILOGI("BasServiceImpl_GetConnectedDeviceBatteryInfos_001 end");
}

/**
 * @tc.number: BasServiceImpl_RemoveStateMachine_001
 * @tc.name: testRemoveStateMachine
 * @tc.desc: 测试移除状态机功能,验证状态机移除不抛出异常
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_RemoveStateMachine_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_RemoveStateMachine_001 start");
    EXPECT_NE(basServiceImpl_, nullptr);
    basServiceImpl_->RemoveStateMachine(TEST_DEVICE_ADDRESS);
    HILOGI("BasServiceImpl_RemoveStateMachine_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetEventName_001
 * @tc.name: testGetEventNameStartup
 * @tc.desc: 测试获取事件名称-服务启动事件,验证返回正确的字符串
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetEventName_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetEventName_001 start");
    std::string name = basServiceImpl_->GetEventName(BAS_SERVICE_STARTUP_EVT);
    EXPECT_EQ(name, "BAS_SERVICE_STARTUP_EVT");
    name = basServiceImpl_->GetEventName(BAS_SERVICE_SHUTDOWN_EVT);
    EXPECT_EQ(name, "BAS_SERVICE_SHUTDOWN_EVT");
    name = basServiceImpl_->GetEventName(BAS_CONNECT_EVT);
    EXPECT_EQ(name, "BAS_CONNECT_EVT");
    name = basServiceImpl_->GetEventName(BAS_DISCONNECT_EVT);
    EXPECT_EQ(name, "BAS_DISCONNECT_EVT");
    name = basServiceImpl_->GetEventName(BAS_CONNECTION_STATE_CHANGED_EVT);
    EXPECT_EQ(name, "BAS_CONNECTION_STATE_CHANGED_EVT");
    name = basServiceImpl_->GetEventName(BAS_CONNECTION_TIMEOUT_EVT);
    EXPECT_EQ(name, "BAS_CONNECTION_TIMEOUT_EVT");
    name = basServiceImpl_->GetEventName(BAS_DISCONNECTION_TIMEOUT_EVT);
    EXPECT_EQ(name, "BAS_DISCONNECTION_TIMEOUT_EVT");
    name = basServiceImpl_->GetEventName(BAS_READ_BATTERY_EVT);
    EXPECT_EQ(name, "BAS_READ_BATTERY_EVT");
    name = basServiceImpl_->GetEventName(BAS_SERVICE_DISCOVERED_EVT);
    EXPECT_EQ(name, "BAS_SERVICE_DISCOVERED_EVT");
    name = basServiceImpl_->GetEventName(BAS_BATTERY_LEVEL_READ_EVT);
    EXPECT_EQ(name, "BAS_BATTERY_LEVEL_READ_EVT");
    name = basServiceImpl_->GetEventName(BAS_BATTERY_LEVEL_CHANGED_EVT);
    EXPECT_EQ(name, "BAS_BATTERY_LEVEL_CHANGED_EVT");
    name = basServiceImpl_->GetEventName(99);
    EXPECT_EQ(name, "Unknown");
    HILOGI("BasServiceImpl_GetEventName_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetConnectedDeviceNum_001
 * @tc.name: testGetConnectedDeviceNum
 * @tc.desc: 测试获取已连接设备数量,验证初始状态返回0
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetConnectedDeviceNum_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetConnectedDeviceNum_001 start");
    int num = basServiceImpl_->GetConnectedDeviceNum();
    EXPECT_EQ(num, 0);
    HILOGI("BasServiceImpl_GetConnectedDeviceNum_001 end");
}

/**
 * @tc.number: BasServiceImpl_IsLocalDeviceConnectAllowed_001
 * @tc.name: testIsLocalDeviceConnectAllowedNoExistingConnection
 * @tc.desc: 测试设备连接许可检查-无现有连接,验证返回BT_NO_ERROR
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_IsLocalDeviceConnectAllowed_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_IsLocalDeviceConnectAllowed_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    EXPECT_EQ(basServiceImpl_->Connect(device), BAS_SUCCESS);
    HILOGI("BasServiceImpl_IsLocalDeviceConnectAllowed_001 end");
}

/**
 * @tc.number: BasServiceImpl_ProcessDefaultEvent_001
 * @tc.name: ProcessDefaultEvent
 * @tc.desc:
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_ProcessDefaultEvent_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_ProcessDefaultEvent_001 start");
    EXPECT_NE(basServiceImpl_, nullptr);
    BasMessage event;
    basServiceImpl_->ProcessDefaultEvent(event);
    basServiceImpl_->ProcessBatteryLevelRead(event);
    basServiceImpl_->ProcessBatteryLevelChanged(event);
    HILOGI("BasServiceImpl_ProcessDefaultEvent_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetBasAppId_001
 * @tc.name: testGetBasAppIdNoStateMachine
 * @tc.desc: 测试获取BasAppId-无对应设备状态机,验证返回-1
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetBasAppId_001, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetBasAppId_001 start");
    EXPECT_NE(basServiceImpl_, nullptr);
    RawAddress device(TEST_DEVICE_ADDRESS);
    int appId = basServiceImpl_->GetBasAppId(device);
    EXPECT_EQ(appId, -1);
    HILOGI("BasServiceImpl_GetBasAppId_001 end");
}

/**
 * @tc.number: BasServiceImpl_GetBasAppId_002
 * @tc.name: testGetBasAppIdEmptyAddress
 * @tc.desc: 测试获取BasAppId-空设备地址,验证返回-1
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetBasAppId_002, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetBasAppId_002 start");
    EXPECT_NE(basServiceImpl_, nullptr);
    RawAddress device("");
    int appId = basServiceImpl_->GetBasAppId(device);
    EXPECT_EQ(appId, -1);
    HILOGI("BasServiceImpl_GetBasAppId_002 end");
}

/**
 * @tc.number: BasServiceImpl_GetBasAppId_003
 * @tc.name: testGetBasAppIdMultipleDevices
 * @tc.desc: 测试获取BasAppId-多个不同设备地址,验证均返回-1
 */
HWTEST_F(BasServiceImplTest, BasServiceImpl_GetBasAppId_003, TestSize.Level1)
{
    HILOGI("BasServiceImpl_GetBasAppId_003 start");
    EXPECT_NE(basServiceImpl_, nullptr);
    RawAddress device1(TEST_DEVICE_ADDRESS);
    RawAddress device2(TEST_DEVICE_ADDRESS2);
    RawAddress device3(TEST_DEVICE_ADDRESS3);
    EXPECT_EQ(basServiceImpl_->GetBasAppId(device1), -1);
    EXPECT_EQ(basServiceImpl_->GetBasAppId(device2), -1);
    EXPECT_EQ(basServiceImpl_->GetBasAppId(device3), -1);
    HILOGI("BasServiceImpl_GetBasAppId_003 end");
}
} // Bluetooth
} // OHOS