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

#undef private
#define private public

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <chrono>
#include <thread>

#include "bas_statemachine.h"
#include "bas_service_impl.h"
#include "bas_message.h"
#include "bas_defines.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace {
const std::string TEST_DEVICE_ADDRESS = "11:22:33:44:55:66";
const std::string GATT_BATTERY_SERVICE_UUID = "0000180F-0000-1000-8000-00805F9B34FB";
const std::string GATT_BATTERY_LEVEL_CHARACTERISTIC_UUID = "00002A19-0000-1000-8000-00805F9B34FB";
const std::string CLIENT_CHARACTERISTIC_CONFIG_DESCRIPTOR_UUID = "00002902-0000-1000-8000-00805F9B34FB";

Service CreateBatteryService()
{
    Service service;
    service.uuid_ = Uuid::ConvertFromString(GATT_BATTERY_SERVICE_UUID);
    Characteristic charac;
    charac.uuid_ = Uuid::ConvertFromString(GATT_BATTERY_LEVEL_CHARACTERISTIC_UUID);
    charac.handle_ = 1;
    Descriptor descriptor;
    descriptor.uuid_ = Uuid::ConvertFromString(CLIENT_CHARACTERISTIC_CONFIG_DESCRIPTOR_UUID);
    descriptor.handle_ = 2;
    charac.descriptors_.push_back(descriptor);
    service.characteristics_.push_back(charac);
    return service;
}
}

class BasStateMachineFriendTest {
public:
    static const Characteristic* TestFindBatteryCharacteristic(BasStateMachine& sm,
        const std::vector<Service>& services)
    {
        return sm.FindBatteryCharacteristic(services);
    }

    static const Descriptor* TestFindCccdDescriptor(BasStateMachine& sm, const Characteristic& charac)
    {
        return sm.FindCccdDescriptor(charac);
    }
};

class BasStateMachineTest : public testing::Test {
public:
    BasStateMachineTest() = default;
    ~BasStateMachineTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

protected:
    std::shared_ptr<BasServiceImpl> basServiceImpl_ = nullptr;
    std::shared_ptr<BasStateMachine> stateMachine_ = nullptr;

    void TransitionToConnectedState(void);
};

void BasStateMachineTest::SetUpTestCase(void)
{
    HILOGI("BasStateMachineTest::SetUpTestCase start");
}

void BasStateMachineTest::TearDownTestCase(void)
{
    HILOGI("BasStateMachineTest::TearDownTestCase start");
}

void BasStateMachineTest::SetUp()
{
    HILOGI("BasStateMachineTest::SetUp start");
    basServiceImpl_ = std::make_shared<BasServiceImpl>();
    basServiceImpl_->Init();
    stateMachine_ = std::make_shared<BasStateMachine>(TEST_DEVICE_ADDRESS, basServiceImpl_.get());
    stateMachine_->Init();
}

void BasStateMachineTest::TearDown()
{
    HILOGI("BasStateMachineTest::TearDown start");
    stateMachine_.reset();
    if (basServiceImpl_ != nullptr) {
        basServiceImpl_->DeInit();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void BasStateMachineTest::TransitionToConnectedState(void)
{
    BasMessage event(BAS_CONNECTION_STATE_CHANGED_EVT, BAS_STATE_CONNECTED);
    event.dev_ = TEST_DEVICE_ADDRESS;
    stateMachine_->ProcessMessage(event);
    ASSERT_EQ(stateMachine_->GetDeviceStateInt(), BAS_STATE_CONNECTED);
}

/**
 * @tc.number: BasStateMachine_GetDeviceAddress_001
 * @tc.name: testGetDeviceAddress
 * @tc.desc: 测试获取设备地址功能,验证返回正确的设备地址
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetDeviceAddress_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetDeviceAddress_001 start");
    std::string address = stateMachine_->GetDeviceAddress();
    EXPECT_EQ(address, TEST_DEVICE_ADDRESS);
    HILOGI("BasStateMachine_GetDeviceAddress_001 end");
}

/**
 * @tc.number: BasStateMachine_SetRemoving_001
 * @tc.name: testSetRemoving
 * @tc.desc: 测试设置移除状态,验证能正确设置和获取移除状态
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_SetRemoving_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_SetRemoving_001 start");
    stateMachine_->SetRemoving(true);
    EXPECT_TRUE(stateMachine_->IsRemoving());
    stateMachine_->SetRemoving(false);
    EXPECT_FALSE(stateMachine_->IsRemoving());
    HILOGI("BasStateMachine_SetRemoving_001 end");
}

/**
 * @tc.number: BasStateMachine_GetDeviceStateInt_001
 * @tc.name: testGetDeviceStateInt
 * @tc.desc: 测试获取设备整型状态值,验证初始状态为BAS_STATE_DISCONNECTED
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetDeviceStateInt_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetDeviceStateInt_001 start");
    int state = stateMachine_->GetDeviceStateInt();
    EXPECT_EQ(state, BAS_STATE_DISCONNECTED);
    HILOGI("BasStateMachine_GetDeviceStateInt_001 end");
}

/**
 * @tc.number: BasStateMachine_GetBasService_001
 * @tc.name: testGetBasService
 * @tc.desc: 测试获取BAS服务,验证返回的服务指针不为空
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetBasService_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetBasService_001 start");
    auto service = stateMachine_->GetBasService();
    EXPECT_NE(service, nullptr);
    HILOGI("BasStateMachine_GetBasService_001 end");
}

/**
 * @tc.number: BasStateMachine_StopConnectionTimer_001
 * @tc.name: testStopConnectionTimer
 * @tc.desc: 测试停止连接计时器,验证计时器停止后状态机仍有效
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_StopConnectionTimer_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_StopConnectionTimer_001 start");
    stateMachine_->StartConnectionTimer();
    stateMachine_->StopConnectionTimer();
    EXPECT_NE(stateMachine_, nullptr);
    HILOGI("BasStateMachine_StopConnectionTimer_001 end");
}

/**
 * @tc.number: BasStateMachine_StopDisconnectionTimer_001
 * @tc.name: testStopDisconnectionTimer
 * @tc.desc: 测试停止断开连接计时器,验证计时器停止后状态机仍有效
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_StopDisconnectionTimer_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_StopDisconnectionTimer_001 start");
    stateMachine_->StartDisconnectionTimer();
    stateMachine_->StopDisconnectionTimer();
    EXPECT_NE(stateMachine_, nullptr);
    HILOGI("BasStateMachine_StopDisconnectionTimer_001 end");
}

/**
 * @tc.number: BasStateMachine_ProcessDeferredMessage_001
 * @tc.name: testProcessDeferredMessage
 * @tc.desc: 测试处理延迟消息,验证消息能正确处理不抛出异常
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_ProcessDeferredMessage_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_ProcessDeferredMessage_001 start");
    BasMessage event(BAS_CONNECT_EVT);
    event.dev_ = TEST_DEVICE_ADDRESS;
    stateMachine_->AddDeferredMessage(event);
    stateMachine_->ProcessDeferredMessage();
    EXPECT_NE(stateMachine_, nullptr);
    HILOGI("BasStateMachine_ProcessDeferredMessage_001 end");
}

/**
 * @tc.number: BasStateMachine_GetCachedBatteryLevel_001
 * @tc.name: testGetCachedBatteryLevel
 * @tc.desc: 测试获取缓存的电池电量,验证无缓存时返回-1
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetCachedBatteryLevel_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetCachedBatteryLevel_001 start");
    int level = stateMachine_->GetCachedBatteryLevel();
    EXPECT_EQ(level, -1);
    HILOGI("BasStateMachine_GetCachedBatteryLevel_001 end");
}

/**
 * @tc.number: BasStateMachine_NotifyStateTransitions_001
 * @tc.name: testNotifyStateTransitions
 * @tc.desc: 测试通知状态转换,验证方法调用不抛出异常
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_NotifyStateTransitions_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_NotifyStateTransitions_001 start");
    stateMachine_->NotifyStateTransitions();
    EXPECT_NE(stateMachine_, nullptr);
    HILOGI("BasStateMachine_NotifyStateTransitions_001 end");
}

/**
 * @tc.number: BasStateMachine_GetEventName_001
 * @tc.name: testGetEventNameConnectionTimeout
 * @tc.desc: 测试获取事件,验证返回正确的字符串
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetEventName_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetEventName_001 start");
    std::string name = BasStateMachine::GetEventName(BAS_CONNECTION_TIMEOUT_EVT);
    EXPECT_EQ(name, "BAS_CONNECTION_TIMEOUT_EVT");
    name = BasStateMachine::GetEventName(BAS_DISCONNECTION_TIMEOUT_EVT);
    EXPECT_EQ(name, "BAS_DISCONNECTION_TIMEOUT_EVT");
    name = BasStateMachine::GetEventName(BAS_CONNECT_EVT);
    EXPECT_EQ(name, "BAS_CONNECT_EVT");
    name = BasStateMachine::GetEventName(BAS_DISCONNECT_EVT);
    EXPECT_EQ(name, "BAS_DISCONNECT_EVT");
    name = BasStateMachine::GetEventName(BAS_CONNECTION_STATE_CHANGED_EVT);
    EXPECT_EQ(name, "BAS_CONNECTION_STATE_CHANGED_EVT");
    name = BasStateMachine::GetEventName(BAS_READ_BATTERY_EVT);
    EXPECT_EQ(name, "BAS_READ_BATTERY_EVT");
    name = BasStateMachine::GetEventName(BAS_SERVICE_DISCOVERED_EVT);
    EXPECT_EQ(name, "BAS_SERVICE_DISCOVERED_EVT");
    name = BasStateMachine::GetEventName(BAS_BATTERY_LEVEL_READ_EVT);
    EXPECT_EQ(name, "BAS_BATTERY_LEVEL_READ_EVT");
    name = BasStateMachine::GetEventName(BAS_BATTERY_LEVEL_CHANGED_EVT);
    EXPECT_EQ(name, "BAS_BATTERY_LEVEL_CHANGED_EVT");
    name = BasStateMachine::GetEventName(BAS_BATTERY_LEVEL_PENDING_READ_EVT);
    EXPECT_EQ(name, "BAS_BATTERY_LEVEL_PENDING_READ_EVT");
    name = BasStateMachine::GetEventName(BAS_SERVICES_CHANGED_EVT);
    EXPECT_EQ(name, "BAS_SERVICES_CHANGED_EVT");
    name = BasStateMachine::GetEventName(99);
    EXPECT_EQ(name, "Unknown");
    HILOGI("BasStateMachine_GetEventName_001 end");
}

/**
 * @tc.number: BasStateMachine_EnableBasNotification_001
 * @tc.name: EnableBasNotification
 * @tc.desc:
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_EnableBasNotification_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_EnableBasNotification_001 start");
    stateMachine_->OnServicesDiscovered();
    EXPECT_FALSE(stateMachine_->EnableBasNotification());
    HILOGI("BasStateMachine_EnableBasNotification_001 end");
}

/**
 * @tc.number: BasStateMachine_UpdateBatteryLevel_001
 * @tc.name: UpdateBatteryLevel
 * @tc.desc:
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_UpdateBatteryLevel_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_UpdateBatteryLevel_001 start");
    int batteryLevel = -1;
    stateMachine_->UpdateBatteryLevel(batteryLevel);
    stateMachine_->OnBatteryLevelRead(batteryLevel);
    batteryLevel = 101;
    stateMachine_->UpdateBatteryLevel(batteryLevel);
    stateMachine_->OnBatteryLevelRead(batteryLevel);
    batteryLevel = 1;
    stateMachine_->UpdateBatteryLevel(batteryLevel);
    stateMachine_->OnBatteryLevelRead(batteryLevel);
    EXPECT_FALSE(stateMachine_->EnableBasNotification());
    HILOGI("BasStateMachine_UpdateBatteryLevel_001 end");
}

/**
 * @tc.number: BasStateMachine_ConnectionTimeout_001
 * @tc.name: ConnectionTimeout
 * @tc.desc:
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_ConnectionTimeout_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_ConnectionTimeout_001 start");
    stateMachine_->ConnectionTimeout();
    EXPECT_FALSE(stateMachine_->EnableBasNotification());
    stateMachine_->DisconnectionTimeout();
    HILOGI("BasStateMachine_ConnectionTimeout_001 end");
}

/**
 * @tc.number: BasStateMachine_FindBatteryCharacteristic_001
 * @tc.name: testFindBatteryCharacteristic_Found
 * @tc.desc: 测试查找电池特征-特征存在时应返回非空指针
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_FindBatteryCharacteristic_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_FindBatteryCharacteristic_001 start");
    std::vector<Service> services;
    services.push_back(CreateBatteryService());
    const Characteristic* batteryCharac = BasStateMachineFriendTest::TestFindBatteryCharacteristic(*stateMachine_,
        services);
    EXPECT_EQ(batteryCharac->uuid_, Uuid::ConvertFromString(GATT_BATTERY_LEVEL_CHARACTERISTIC_UUID));
    HILOGI("BasStateMachine_FindBatteryCharacteristic_001 end");
}

/**
 * @tc.number: BasStateMachine_FindBatteryCharacteristic_002
 * @tc.name: testFindBatteryCharacteristic_NotFound
 * @tc.desc: 测试查找电池特征-服务列表为空时应返回空指针
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_FindBatteryCharacteristic_002, TestSize.Level1)
{
    HILOGI("BasStateMachine_FindBatteryCharacteristic_002 start");
    std::vector<Service> services;
    const Characteristic* batteryCharac = BasStateMachineFriendTest::TestFindBatteryCharacteristic(*stateMachine_,
        services);
    EXPECT_EQ(batteryCharac, nullptr);
    HILOGI("BasStateMachine_FindBatteryCharacteristic_002 end");
}

/**
 * @tc.number: BasStateMachine_FindBatteryCharacteristic_003
 * @tc.name: testFindBatteryCharacteristic_WrongService
 * @tc.desc: 测试查找电池特征-错误的服务UUID时应返回空指针
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_FindBatteryCharacteristic_003, TestSize.Level1)
{
    HILOGI("BasStateMachine_FindBatteryCharacteristic_003 start");
    std::vector<Service> services;
    Service wrongService;
    wrongService.uuid_ = Uuid::Random();
    Characteristic charac;
    charac.uuid_ = Uuid::ConvertFromString(GATT_BATTERY_LEVEL_CHARACTERISTIC_UUID);
    wrongService.characteristics_.push_back(charac);
    services.push_back(wrongService);
    const Characteristic* batteryCharac = BasStateMachineFriendTest::TestFindBatteryCharacteristic(*stateMachine_,
        services);
    EXPECT_EQ(batteryCharac, nullptr);
    HILOGI("BasStateMachine_FindBatteryCharacteristic_003 end");
}

/**
 * @tc.number: BasStateMachine_FindCccdDescriptor_001
 * @tc.name: testFindCccdDescriptor_Found
 * @tc.desc: 测试查找CCCD描述符-描述符存在时应返回非空指针
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_FindCccdDescriptor_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_FindCccdDescriptor_001 start");
    Service service = CreateBatteryService();
    Characteristic charac = service.characteristics_[0];
    const Descriptor* cccdDescriptor = BasStateMachineFriendTest::TestFindCccdDescriptor(*stateMachine_, charac);
    EXPECT_NE(cccdDescriptor, nullptr);
    EXPECT_EQ(cccdDescriptor->uuid_, Uuid::ConvertFromString(CLIENT_CHARACTERISTIC_CONFIG_DESCRIPTOR_UUID));
    EXPECT_EQ(cccdDescriptor->handle_, 2);
    HILOGI("BasStateMachine_FindCccdDescriptor_001 end");
}

/**
 * @tc.number: BasStateMachine_FindCccdDescriptor_002
 * @tc.name: testFindCccdDescriptor_NotFound
 * @tc.desc: 测试查找CCCD描述符-特征无描述符时应返回空指针
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_FindCccdDescriptor_002, TestSize.Level1)
{
    HILOGI("BasStateMachine_FindCccdDescriptor_002 start");
    Characteristic charac;
    charac.uuid_ = Uuid::ConvertFromString(GATT_BATTERY_LEVEL_CHARACTERISTIC_UUID);
    charac.handle_ = 1;
    const Descriptor* cccdDescriptor = BasStateMachineFriendTest::TestFindCccdDescriptor(*stateMachine_, charac);
    EXPECT_EQ(cccdDescriptor, nullptr);
    HILOGI("BasStateMachine_FindCccdDescriptor_002 end");
}

/**
 * @tc.number: BasStateMachine_FindCccdDescriptor_003
 * @tc.name: testFindCccdDescriptor_WrongDescriptor
 * @tc.desc: 测试查找CCCD描述符-描述符UUID不匹配时应返回空指针
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_FindCccdDescriptor_003, TestSize.Level1)
{
    HILOGI("BasStateMachine_FindCccdDescriptor_003 start");
    Characteristic charac;
    charac.uuid_ = Uuid::ConvertFromString(GATT_BATTERY_LEVEL_CHARACTERISTIC_UUID);
    charac.handle_ = 1;
    Descriptor wrongDescriptor;
    wrongDescriptor.uuid_ = Uuid::Random();
    wrongDescriptor.handle_ = 3;
    charac.descriptors_.push_back(wrongDescriptor);
    const Descriptor* cccdDescriptor = BasStateMachineFriendTest::TestFindCccdDescriptor(*stateMachine_, charac);
    EXPECT_EQ(cccdDescriptor, nullptr);
    HILOGI("BasStateMachine_FindCccdDescriptor_003 end");
}

/**
 * @tc.number: BasStateMachine_InitGatt_001
 * @tc.name: testInitGattFirstTime
 * @tc.desc: 测试初始化GATT-首次调用应创建callback,验证callback不为空
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_InitGatt_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_InitGatt_001 start");
    stateMachine_->appId_ = -1;
    stateMachine_->callback_ = nullptr;
    stateMachine_->InitGatt();
    EXPECT_NE(stateMachine_->callback_, nullptr);
    HILOGI("BasStateMachine_InitGatt_001 end");
}

/**
 * @tc.number: BasStateMachine_ConnectGatt_001
 * @tc.name: testConnectGatt
 * @tc.desc: 测试连接GATT-调用ConnectGatt,验证在不崩溃的前提下返回bool值
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_ConnectGatt_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_ConnectGatt_001 start");
    stateMachine_->InitGatt();
    bool ret = stateMachine_->ConnectGatt();
    EXPECT_EQ(ret, false);
    HILOGI("BasStateMachine_ConnectGatt_001 end");
}

/**
 * @tc.number: BasStateMachine_ConnectGatt_002
 * @tc.name: testConnectGattWithoutRegistration
 * @tc.desc: 测试连接GATT-appId未注册(-1)时调用ConnectGatt,验证返回false
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_ConnectGatt_002, TestSize.Level1)
{
    HILOGI("BasStateMachine_ConnectGatt_002 start");
    stateMachine_->appId_ = -1;
    bool ret = stateMachine_->ConnectGatt();
    EXPECT_EQ(ret, false);
    HILOGI("BasStateMachine_ConnectGatt_002 end");
}

/**
 * @tc.number: BasStateMachine_DisconnectGatt_001
 * @tc.name: testDisconnectGatt
 * @tc.desc: 测试断开GATT-调用DisconnectGatt,验证在不崩溃的前提下返回bool值
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_DisconnectGatt_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_DisconnectGatt_001 start");
    stateMachine_->InitGatt();
    bool ret = stateMachine_->DisconnectGatt();
    EXPECT_EQ(ret, false);
    HILOGI("BasStateMachine_DisconnectGatt_001 end");
}

/**
 * @tc.number: BasStateMachine_DisconnectGatt_002
 * @tc.name: testDisconnectGattWithoutRegistration
 * @tc.desc: 测试断开GATT-appId未注册(-1)时调用DisconnectGatt,验证返回false
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_DisconnectGatt_002, TestSize.Level1)
{
    HILOGI("BasStateMachine_DisconnectGatt_002 start");
    stateMachine_->appId_ = -1;
    bool ret = stateMachine_->DisconnectGatt();
    EXPECT_EQ(ret, false);
    HILOGI("BasStateMachine_DisconnectGatt_002 end");
}

/**
 * @tc.number: BasStateMachine_CloseGatt_001
 * @tc.name: testCloseGattNotRegistered
 * @tc.desc: 
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_CloseGatt_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_CloseGatt_001 start");
    stateMachine_->appId_ = -1;
    stateMachine_->CloseGatt();
    EXPECT_EQ(stateMachine_->appId_, -1);
    HILOGI("BasStateMachine_CloseGatt_001 end");
}

/**
 * @tc.number: BasStateMachine_CloseGatt_002
 * @tc.name: testCloseGattAfterInit
 * @tc.desc: 
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_CloseGatt_002, TestSize.Level1)
{
    HILOGI("BasStateMachine_CloseGatt_002 start");
    stateMachine_->InitGatt();
    stateMachine_->CloseGatt();
    EXPECT_EQ(stateMachine_->appId_, -1);
    HILOGI("BasStateMachine_CloseGatt_002 end");
}

/**
 * @tc.number: BasStateMachine_DiscoverServicesGatt_001
 * @tc.name: testDiscoverServicesGatt
 * @tc.desc: 测试发现GATT服务-调用DiscoverServicesGatt,验证在不崩溃的前提下返回bool值
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_DiscoverServicesGatt_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_DiscoverServicesGatt_001 start");
    stateMachine_->InitGatt();
    bool ret = stateMachine_->DiscoverServicesGatt();
    EXPECT_EQ(ret, false);
    HILOGI("BasStateMachine_DiscoverServicesGatt_001 end");
}

/**
 * @tc.number: BasStateMachine_DiscoverServicesGatt_002
 * @tc.name: testDiscoverServicesGattWithoutRegistration
 * @tc.desc: 测试发现GATT服务-appId未注册(-1)时调用DiscoverServicesGatt,验证返回false
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_DiscoverServicesGatt_002, TestSize.Level1)
{
    HILOGI("BasStateMachine_DiscoverServicesGatt_002 start");
    stateMachine_->appId_ = -1;
    bool ret = stateMachine_->DiscoverServicesGatt();
    EXPECT_EQ(ret, false);
    HILOGI("BasStateMachine_DiscoverServicesGatt_002 end");
}

/**
 * @tc.number: BasStateMachine_GetBatteryLevel_001
 * @tc.name: testGetBatteryLevel
 * @tc.desc: 测试获取电池电量-调用GetBatteryLevel,验证在不崩溃的前提下返回bool值
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetBatteryLevel_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetBatteryLevel_001 start");
    stateMachine_->InitGatt();
    bool ret = stateMachine_->GetBatteryLevel();
    EXPECT_EQ(ret, false);
    HILOGI("BasStateMachine_GetBatteryLevel_001 end");
}

/**
 * @tc.number: BasStateMachine_GetBatteryLevel_002
 * @tc.name: testGetBatteryLevelWithoutRegistration
 * @tc.desc: 测试获取电池电量-appId未注册(-1)时调用GetBatteryLevel,验证返回false
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetBatteryLevel_002, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetBatteryLevel_002 start");
    stateMachine_->appId_ = -1;
    bool ret = stateMachine_->GetBatteryLevel();
    EXPECT_EQ(ret, false);
    HILOGI("BasStateMachine_GetBatteryLevel_002 end");
}

/**
 * @tc.number: BasStateMachine_GetBasAppId_001
 * @tc.name: testGetBasAppIdInitial
 * @tc.desc: 测试获取BasAppId-初始状态未注册GATT,验证返回-1
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetBasAppId_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetBasAppId_001 start");
    stateMachine_->appId_ = -1;
    int appId = stateMachine_->GetBasAppId();
    EXPECT_EQ(appId, -1);
    HILOGI("BasStateMachine_GetBasAppId_001 end");
}

/**
 * @tc.number: BasStateMachine_GetBasAppId_002
 * @tc.name: testGetBasAppIdAfterInitGatt
 * @tc.desc: 测试获取BasAppId-调用InitGatt后,验证返回值与appId_一致
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetBasAppId_002, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetBasAppId_002 start");
    stateMachine_->InitGatt();
    int appId = stateMachine_->GetBasAppId();
    EXPECT_EQ(appId, stateMachine_->appId_.load());
    HILOGI("BasStateMachine_GetBasAppId_002 end");
}

/**
 * @tc.number: BasStateMachine_GetBasAppId_003
 * @tc.name: testGetBasAppIdWithValidAppId
 * @tc.desc: 测试获取BasAppId-设置有效appId后,验证返回设置的appId值
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_GetBasAppId_003, TestSize.Level1)
{
    HILOGI("BasStateMachine_GetBasAppId_003 start");
    stateMachine_->appId_ = 5;
    int appId = stateMachine_->GetBasAppId();
    EXPECT_EQ(appId, 5);
    stateMachine_->appId_ = -1;
    HILOGI("BasStateMachine_GetBasAppId_003 end");
}

/**
 * @tc.number: BasStateMachine_SetDiscoverCompleted_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_SetDiscoverCompleted_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_SetDiscoverCompleted_001 start");
    EXPECT_FALSE(stateMachine_->discoverCompleted_);
    stateMachine_->SetDiscoverCompleted(true);
    EXPECT_TRUE(stateMachine_->discoverCompleted_);
    HILOGI("BasStateMachine_SetDiscoverCompleted_001 end");
}

/**
 * @tc.number: BasStateMachine_ProcessBatteryLevelPendingReadEvent_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_ProcessBatteryLevelPendingReadEvent_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_ProcessBatteryLevelPendingReadEvent_001 start");
    stateMachine_->readReqPending_ = true;
    stateMachine_->ProcessBatteryLevelPendingReadEvent();
    EXPECT_FALSE(stateMachine_->readReqPending_);
    HILOGI("BasStateMachine_ProcessBatteryLevelPendingReadEvent_001 end");
}

/**
 * @tc.number: BasStateMachine_OnServicesDiscovered_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BasStateMachineTest, BasStateMachine_OnServicesDiscovered_001, TestSize.Level1)
{
    HILOGI("BasStateMachine_OnServicesDiscovered_001 start");
    EXPECT_FALSE(stateMachine_->discoverCompleted_);
    stateMachine_->OnServicesDiscovered();
    EXPECT_TRUE(stateMachine_->discoverCompleted_);
    HILOGI("BasStateMachine_OnServicesDiscovered_001 end");
}
} // Bluetooth
} // OHOS