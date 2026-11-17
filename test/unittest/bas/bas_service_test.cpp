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

#include "bas_service.h"
#include "bas_defines.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "class_creator.h"
#include "profile_service_manager.h"
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

class BasServiceTestBasObserver : public IBasObserver {
public:
    BasServiceTestBasObserver() = default;
    ~BasServiceTestBasObserver() override = default;

    void OnGetBatteryLevelEvent(const RawAddress &deviceAddress, int32_t batteryLevel) override
    {}
    void OnBatteryLevelChanged(const RawAddress &deviceAddress, int32_t batteryLevel) override
    {}
};

class BasServiceTest : public testing::Test {
public:
    BasServiceTest() = default;
    ~BasServiceTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

protected:
    BasService* basService_ = nullptr;
    std::shared_ptr<IBasObserver> basObserver_ = nullptr;
};

void BasServiceTest::SetUpTestCase(void)
{
    HILOGI("BasServiceTest::SetUpTestCase start");
}

void BasServiceTest::TearDownTestCase(void)
{
    HILOGI("BasServiceTest::TearDownTestCase start");
}

void BasServiceTest::SetUp()
{
    HILOGI("BasServiceTest::SetUp start");
    auto manager = ProfileServiceManager::GetInstance();
        std::string profileName = "BasService";
    manager->SetProfileService(ADAPTER_BREDR, profileName);
    basService_ = BasService::GetService();
    basService_->Enable();
    basObserver_ = std::make_shared<BasServiceTestBasObserver>();
}

void BasServiceTest::TearDown()
{
    HILOGI("BasServiceTest::TearDown start");
    if (basService_ != nullptr) {
        basService_->DeregisterObserver(basObserver_);
        basService_->Disable();
    }
}

/**
 * @tc.number: BasService_Constructor_001
 * @tc.name: testBasServiceConstructor
 * @tc.desc: 测试BasService构造函数,验证服务对象能正常创建
 */
HWTEST_F(BasServiceTest, BasService_Constructor_001, TestSize.Level1)
{
    HILOGI("BasService_Constructor_001 start");
    auto service = std::make_unique<BasService>();
    EXPECT_NE(service, nullptr);
    HILOGI("BasService_Constructor_001 end");
}

/**
 * @tc.number: BasService_GetContext_001
 * @tc.name: testGetContext
 * @tc.desc: 测试获取服务上下文,验证上下文对象不为空
 */
HWTEST_F(BasServiceTest, BasService_GetContext_001, TestSize.Level1)
{
    HILOGI("BasService_GetContext_001 start");
    EXPECT_NE(basService_->GetContext(), nullptr);
    HILOGI("BasService_GetContext_001 end");
}

/**
 * @tc.number: BasService_DeregisterObserver_001
 * @tc.name: testDeregisterObserver
 * @tc.desc: 测试观察者注销功能,验证观察者能成功注销
 */
HWTEST_F(BasServiceTest, BasService_DeregisterObserver_001, TestSize.Level1)
{
    HILOGI("BasService_DeregisterObserver_001 start");
    basService_->RegisterObserver(basObserver_);
    EXPECT_NE(basService_, nullptr);
    basService_->DeregisterObserver(basObserver_);
    HILOGI("BasService_DeregisterObserver_001 end");
}

/**
 * @tc.number: BasService_Connect_001
 * @tc.name: testConnect
 * @tc.desc: 测试设备连接功能,验证连接操作能返回BAS_SUCCESS
 */
HWTEST_F(BasServiceTest, BasService_Connect_001, TestSize.Level1)
{
    HILOGI("BasService_Connect_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int ret = basService_->Connect(device);
    EXPECT_NE(ret, BAS_SUCCESS);
    HILOGI("BasService_Connect_001 end");
}

/**
 * @tc.number: BasService_Connect_002
 * @tc.name: testConnectWithNullDevice
 * @tc.desc: 测试空设备连接,验证连接操作能返回错误码
 */
HWTEST_F(BasServiceTest, BasService_Connect_002, TestSize.Level1)
{
    HILOGI("BasService_Connect_002 start");
    RawAddress device("");
    int ret = basService_->Connect(device);
    EXPECT_NE(ret, BAS_SUCCESS);
    HILOGI("BasService_Connect_002 end");
}

/**
 * @tc.number: BasService_Disconnect_001
 * @tc.name: testDisconnect
 * @tc.desc: 测试设备断开连接功能,验证未连接设备断开返回非零错误码
 */
HWTEST_F(BasServiceTest, BasService_Disconnect_001, TestSize.Level1)
{
    HILOGI("BasService_Disconnect_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int ret = basService_->Disconnect(device);
    EXPECT_NE(ret, 0);
    HILOGI("BasService_Disconnect_001 end");
}

/**
 * @tc.number: BasService_GetBatteryLevel_001
 * @tc.name: testGetBatteryLevel
 * @tc.desc: 测试获取设备电池电量,验证未连接设备返回非零错误码或PROFILE_DISABLED
 */
HWTEST_F(BasServiceTest, BasService_GetBatteryLevel_001, TestSize.Level1)
{
    HILOGI("BasService_GetBatteryLevel_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int ret = basService_->GetBatteryLevel(device);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("BasService_GetBatteryLevel_001 end");
}

/**
 * @tc.number: BasService_GetBatteryLevel_002
 * @tc.name: testGetBatteryLevelWithEmptyAddress
 * @tc.desc: 测试获取空地址设备电池电量,验证返回PROFILE_DISABLED
 */
HWTEST_F(BasServiceTest, BasService_GetBatteryLevel_002, TestSize.Level1)
{
    HILOGI("BasService_GetBatteryLevel_002 start");
    RawAddress device("");
    int ret = basService_->GetBatteryLevel(device);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED);
    HILOGI("BasService_GetBatteryLevel_002 end");
}

/**
 * @tc.number: BasService_GetDeviceState_001
 * @tc.name: testGetDeviceState
 * @tc.desc: 测试获取设备状态,验证未连接设备状态为DISCONNECTED
 */
HWTEST_F(BasServiceTest, BasService_GetDeviceState_001, TestSize.Level1)
{
    HILOGI("BasService_GetDeviceState_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int state = basService_->GetDeviceState(device);
    EXPECT_EQ(state, static_cast<int>(BTConnectState::DISCONNECTED));
    HILOGI("BasService_GetDeviceState_001 end");
}

/**
 * @tc.number: BasService_GetConnectDevices_001
 * @tc.name: testGetConnectDevices
 * @tc.desc: 测试获取已连接设备列表,验证无连接设备时返回空列表
 */
HWTEST_F(BasServiceTest, BasService_GetConnectDevices_001, TestSize.Level1)
{
    HILOGI("BasService_GetConnectDevices_001 start");
    auto devices = basService_->GetConnectDevices();
    EXPECT_TRUE(devices.empty());
    HILOGI("BasService_GetConnectDevices_001 end");
}

/**
 * @tc.number: BasService_GetConnectState_001
 * @tc.name: testGetConnectState
 * @tc.desc: 测试获取服务连接状态,验证默认状态为0
 */
HWTEST_F(BasServiceTest, BasService_GetConnectState_001, TestSize.Level1)
{
    HILOGI("BasService_GetConnectState_001 start");
    int state = basService_->GetConnectState();
    EXPECT_EQ(state, PROFILE_STATE_DISCONNECTED);
    HILOGI("BasService_GetConnectState_001 end");
}

/**
 * @tc.number: BasService_GetMaxConnectNum_001
 * @tc.name: testGetMaxConnectNum
 * @tc.desc: 测试获取最大连接数,验证默认值为0
 */
HWTEST_F(BasServiceTest, BasService_GetMaxConnectNum_001, TestSize.Level1)
{
    HILOGI("BasService_GetMaxConnectNum_001 start");
    int maxNum = basService_->GetMaxConnectNum();
    EXPECT_EQ(maxNum, 0);
    HILOGI("BasService_GetMaxConnectNum_001 end");
}

/**
 * @tc.number: BasService_GetDevicesByStates_001
 * @tc.name: testGetDevicesByStates
 * @tc.desc: 测试按状态获取设备列表,验证无连接设备时返回空列表
 */
HWTEST_F(BasServiceTest, BasService_GetDevicesByStates_001, TestSize.Level1)
{
    HILOGI("BasService_GetDevicesByStates_001 start");
    std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    auto devices = basService_->GetDevicesByStates(states);
    EXPECT_TRUE(devices.empty());
    HILOGI("BasService_GetDevicesByStates_001 end");
}

/**
 * @tc.number: BasService_GetConnectedDeviceBatteryInfos_001
 * @tc.name: testGetConnectedDeviceBatteryInfos
 * @tc.desc: 测试获取已连接设备的电池信息,验证无连接设备时返回空列表
 */
HWTEST_F(BasServiceTest, BasService_GetConnectedDeviceBatteryInfos_001, TestSize.Level1)
{
    HILOGI("BasService_GetConnectedDeviceBatteryInfos_001 start");
    auto batteryInfos = basService_->GetConnectedDeviceBatteryInfos();
    EXPECT_TRUE(batteryInfos.empty());
    HILOGI("BasService_GetConnectedDeviceBatteryInfos_001 end");
}

/**
 * @tc.number: BasService_NotifyReadBatteryLevelEvent_001
 * @tc.name: testNotifyReadBatteryLevelEvent
 * @tc.desc: 测试通知读取电池电量事件,验证通知方法能正常调用
 */
HWTEST_F(BasServiceTest, BasService_NotifyReadBatteryLevelEvent_001, TestSize.Level1)
{
    HILOGI("BasService_NotifyReadBatteryLevelEvent_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    basService_->RegisterObserver(basObserver_);
    basService_->NotifyReadBatteryLevelEvent(device, 80);
    basService_->NotifyBatteryLevelChanged(device, 75);
    EXPECT_NE(basService_, nullptr);
    HILOGI("BasService_NotifyReadBatteryLevelEvent_001 end");
}

/**
 * @tc.number: BasService_SetConnectStrategy_001
 * @tc.name: testSetConnectStrategyAllowed
 * @tc.desc: 测试设置连接策略为允许连接,验证策略设置结果非BT_NO_ERROR
 */
HWTEST_F(BasServiceTest, BasService_SetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("BasService_SetConnectStrategy_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    int ret = basService_->SetConnectStrategy(device, strategy);
    EXPECT_NE(ret, Bluetooth::BT_NO_ERROR);
    HILOGI("BasService_SetConnectStrategy_001 end");
}

/**
 * @tc.number: BasService_SetConnectStrategy_002
 * @tc.name: testSetConnectStrategyForbidden
 * @tc.desc: 测试设置连接策略为禁止连接,验证策略设置结果非BT_NO_ERROR
 */
HWTEST_F(BasServiceTest, BasService_SetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("BasService_SetConnectStrategy_002 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    int ret = basService_->SetConnectStrategy(device, strategy);
    EXPECT_NE(ret, Bluetooth::BT_NO_ERROR);
    HILOGI("BasService_SetConnectStrategy_002 end");
}

/**
 * @tc.number: BasService_SetConnectStrategy_003
 * @tc.name: testSetConnectStrategyInvalid
 * @tc.desc: 测试设置无效连接策略,验证返回BT_ERR_INTERNAL_ERROR
 */
HWTEST_F(BasServiceTest, BasService_SetConnectStrategy_003, TestSize.Level1)
{
    HILOGI("BasService_SetConnectStrategy_003 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int strategy = -1;
    int ret = basService_->SetConnectStrategy(device, strategy);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_INTERNAL_ERROR);
    HILOGI("BasService_SetConnectStrategy_003 end");
}

/**
 * @tc.number: BasService_GetConnectStrategy_001
 * @tc.name: testGetConnectStrategy
 * @tc.desc: 测试获取连接策略,验证返回值非负
 */
HWTEST_F(BasServiceTest, BasService_GetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("BasService_GetConnectStrategy_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int strategy = basService_->GetConnectStrategy(device);
    EXPECT_GE(strategy, 0);
    HILOGI("BasService_GetConnectStrategy_001 end");
}

/**
 * @tc.number: BasService_LoadBasServiceInterfaceLib_001
 * @tc.name: testLoadBasServiceInterfaceLib
 * @tc.desc: 测试加载和卸载BAS服务接口库,验证加载卸载能正常执行
 */
HWTEST_F(BasServiceTest, BasService_LoadBasServiceInterfaceLib_001, TestSize.Level1)
{
    HILOGI("BasService_LoadBasServiceInterfaceLib_001 start");
    basService_->LoadBasServiceInterfaceLib();
    EXPECT_NE(basService_, nullptr);
    basService_->UnloadBasServiceInterfaceLib();
    HILOGI("BasService_LoadBasServiceInterfaceLib_001 end");
}

/**
 * @tc.number: BasService_Connect_Disconnect_SameDevice_001
 * @tc.name: testConnectDisconnectSameDevice
 * @tc.desc: 测试同一设备的连接和断开操作,验证连接成功但断开失败
 */
HWTEST_F(BasServiceTest, BasService_Connect_Disconnect_SameDevice_001, TestSize.Level1)
{
    HILOGI("BasService_Connect_Disconnect_SameDevice_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);

    int connectRet = basService_->Connect(device);
    EXPECT_NE(connectRet, BAS_SUCCESS);

    int disconnectRet = basService_->Disconnect(device);
    EXPECT_GE(disconnectRet, BAS_FAILURE);
    HILOGI("BasService_Connect_Disconnect_SameDevice_001 end");
}

/**
 * @tc.number: BasService_IsBasSupportedByRemoteDevice_001
 * @tc.name: testIsBasSupportedByRemoteDevice
 * @tc.desc: 测试检查远程设备是否支持BAS,验证方法调用不抛出异常
 */
HWTEST_F(BasServiceTest, BasService_IsBasSupportedByRemoteDevice_001, TestSize.Level1)
{
    HILOGI("BasService_IsBasSupportedByRemoteDevice_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    bool supported = basService_->IsBasSupportedByRemoteDevice(device);
    EXPECT_FALSE(supported);
    HILOGI("BasService_IsBasSupportedByRemoteDevice_001 end");
}

/**
 * @tc.number: BasService_GetBasAppId_001
 * @tc.name: testGetBasAppIdNotLoaded
 * @tc.desc: 测试获取BasAppId-服务实现未加载,验证返回-1
 */
HWTEST_F(BasServiceTest, BasService_GetBasAppId_001, TestSize.Level1)
{
    HILOGI("BasService_GetBasAppId_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int appId = basService_->GetBasAppId(device);
    EXPECT_EQ(appId, -1);
    HILOGI("BasService_GetBasAppId_001 end");
}

/**
 * @tc.number: BasService_GetBasAppId_002
 * @tc.name: testGetBasAppIdEmptyAddress
 * @tc.desc: 测试获取BasAppId-空设备地址,验证返回-1
 */
HWTEST_F(BasServiceTest, BasService_GetBasAppId_002, TestSize.Level1)
{
    HILOGI("BasService_GetBasAppId_002 start");
    RawAddress device("");
    int appId = basService_->GetBasAppId(device);
    EXPECT_EQ(appId, -1);
    HILOGI("BasService_GetBasAppId_002 end");
}

/**
 * @tc.number: BasService_GetBasAppId_003
 * @tc.name: testGetBasAppIdAfterLoadUnload
 * @tc.desc: 测试获取BasAppId-加载后卸载服务实现库,验证返回-1
 */
HWTEST_F(BasServiceTest, BasService_GetBasAppId_003, TestSize.Level1)
{
    HILOGI("BasService_GetBasAppId_003 start");
    basService_->LoadBasServiceInterfaceLib();
    basService_->UnloadBasServiceInterfaceLib();
    RawAddress device(TEST_DEVICE_ADDRESS);
    int appId = basService_->GetBasAppId(device);
    EXPECT_EQ(appId, -1);
    HILOGI("BasService_GetBasAppId_003 end");
}

/**
 * @tc.number: BasService_UpdateGattClientIfMap_001
 * @tc.name: testUpdateGattClientIfMapInvalidClientIf
 * @tc.desc: 测试更新GATT客户端映射-无效clientIf(0),验证方法提前返回不修改映射表
 */
HWTEST_F(BasServiceTest, BasService_UpdateGattClientIfMap_001, TestSize.Level1)
{
    HILOGI("BasService_UpdateGattClientIfMap_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int invalidClientIf = 0;
    int state = static_cast<int>(BTConnectState::CONNECTED);
    // clientIf为0时应该直接返回
    basService_->UpdateGattClientIfMap(device, invalidClientIf, state);
    // 验证设备未被添加到映射表
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_UpdateGattClientIfMap_001 end");
}

/**
 * @tc.number: BasService_UpdateGattClientIfMap_002
 * @tc.name: testUpdateGattClientIfMapNegativeClientIf
 * @tc.desc: 测试更新GATT客户端映射-负数clientIf,验证方法提前返回不修改映射表
 */
HWTEST_F(BasServiceTest, BasService_UpdateGattClientIfMap_002, TestSize.Level1)
{
    HILOGI("BasService_UpdateGattClientIfMap_002 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int negativeClientIf = -1;
    int state = static_cast<int>(BTConnectState::CONNECTED);
    // 负数clientIf应该直接返回
    basService_->UpdateGattClientIfMap(device, negativeClientIf, state);
    // 验证设备未被添加到映射表
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_UpdateGattClientIfMap_002 end");
}

/**
 * @tc.number: BasService_UpdateGattClientIfMap_003
 * @tc.name: testUpdateGattClientIfMapConnectState
 * @tc.desc: 测试更新GATT客户端映射-连接状态,将clientIf插入映射表
 */
HWTEST_F(BasServiceTest, BasService_UpdateGattClientIfMap_003, TestSize.Level1)
{
    HILOGI("BasService_UpdateGattClientIfMap_003 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf = 5;
    int state = static_cast<int>(BTConnectState::CONNECTED);
    // 连接状态时插入clientIf
    basService_->UpdateGattClientIfMap(device, clientIf, state);
    // 验证设备已在映射表中
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_TRUE(isConnected);
    HILOGI("BasService_UpdateGattClientIfMap_003 end");
}

/**
 * @tc.number: BasService_UpdateGattClientIfMap_004
 * @tc.name: testUpdateGattClientIfMapDisconnectState
 * @tc.desc: 测试更新GATT客户端映射-断开连接状态,从映射表移除clientIf
 */
HWTEST_F(BasServiceTest, BasService_UpdateGattClientIfMap_004, TestSize.Level1)
{
    HILOGI("BasService_UpdateGattClientIfMap_004 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf = 5;
    // 先建立连接
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::CONNECTED));
    ASSERT_TRUE(basService_->IsAnyBleProfileConnected(device));
    // 再断开
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::DISCONNECTED));
    // 验证设备已从映射表移除
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_UpdateGattClientIfMap_004 end");
}

/**
 * @tc.number: BasService_UpdateGattClientIfMap_005
 * @tc.name: testUpdateGattClientIfMapMultipleClientIfs
 * @tc.desc: 测试更新GATT客户端映射-同一设备多个clientIf连接,验证映射表维护多个连接
 */
HWTEST_F(BasServiceTest, BasService_UpdateGattClientIfMap_005, TestSize.Level1)
{
    HILOGI("BasService_UpdateGattClientIfMap_005 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf1 = 1;
    int clientIf2 = 2;
    int clientIf3 = 3;
    // 添加三个不同的clientIf连接
    basService_->UpdateGattClientIfMap(device, clientIf1, static_cast<int>(BTConnectState::CONNECTED));
    basService_->UpdateGattClientIfMap(device, clientIf2, static_cast<int>(BTConnectState::CONNECTED));
    basService_->UpdateGattClientIfMap(device, clientIf3, static_cast<int>(BTConnectState::CONNECTED));
    // 验证设备仍在映射表中
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_TRUE(isConnected);
    // 断开其中一个
    basService_->UpdateGattClientIfMap(device, clientIf2, static_cast<int>(BTConnectState::DISCONNECTED));
    // 还有clientIf1和clientIf3，设备仍在映射表中
    isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_TRUE(isConnected);
    HILOGI("BasService_UpdateGattClientIfMap_005 end");
}

/**
 * @tc.number: BasService_UpdateGattClientIfMap_006
 * @tc.name: testUpdateGattClientIfMapDisconnectLastClientIf
 * @tc.desc: 测试更新GATT客户端映射-最后一个连接断开时映射表清空,验证设备条目被删除
 */
HWTEST_F(BasServiceTest, BasService_UpdateGattClientIfMap_006, TestSize.Level1)
{
    HILOGI("BasService_UpdateGattClientIfMap_006 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf1 = 1;
    int clientIf2 = 2;
    // 添加两个连接
    basService_->UpdateGattClientIfMap(device, clientIf1, static_cast<int>(BTConnectState::CONNECTED));
    basService_->UpdateGattClientIfMap(device, clientIf2, static_cast<int>(BTConnectState::CONNECTED));
    // 断开第一个
    basService_->UpdateGattClientIfMap(device, clientIf1, static_cast<int>(BTConnectState::DISCONNECTED));
    ASSERT_TRUE(basService_->IsAnyBleProfileConnected(device));
    // 断开第二个 - 此时映射表应为空，设备条目被删除
    basService_->UpdateGattClientIfMap(device, clientIf2, static_cast<int>(BTConnectState::DISCONNECTED));
    // 验证设备不在映射表中
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_UpdateGattClientIfMap_006 end");
}

/**
 * @tc.number: BasService_UpdateGattClientIfMap_007
 * @tc.name: testUpdateGattClientIfMapSameClientIfRepeatedConnect
 * @tc.desc: 测试更新GATT客户端映射-重复连接同一clientIf,验证set中只有一个该clientIf
 */
HWTEST_F(BasServiceTest, BasService_UpdateGattClientIfMap_007, TestSize.Level1)
{
    HILOGI("BasService_UpdateGattClientIfMap_007 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf = 10;
    // 多次插入同一个clientIf
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::CONNECTED));
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::CONNECTED));
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::CONNECTED));
    // set特性保证只有一个元素
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_TRUE(isConnected);
    // 断开一次后应该为空
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::DISCONNECTED));
    isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_UpdateGattClientIfMap_007 end");
}

/**
 * @tc.number: BasService_UpdateGattClientIfMap_008
 * @tc.name: testUpdateGattClientIfMapDifferentDevices
 * @tc.desc: 测试更新GATT客户端映射-不同设备的映射互相独立
 */
HWTEST_F(BasServiceTest, BasService_UpdateGattClientIfMap_008, TestSize.Level1)
{
    HILOGI("BasService_UpdateGattClientIfMap_008 start");
    RawAddress device1(TEST_DEVICE_ADDRESS);
    RawAddress device2(TEST_DEVICE_ADDRESS2);
    int clientIf1 = 1;
    int clientIf2 = 2;
    // device1建立连接
    basService_->UpdateGattClientIfMap(device1, clientIf1, static_cast<int>(BTConnectState::CONNECTED));
    // device2建立连接
    basService_->UpdateGattClientIfMap(device2, clientIf2, static_cast<int>(BTConnectState::CONNECTED));
    // 两个设备都应该在映射表中
    EXPECT_TRUE(basService_->IsAnyBleProfileConnected(device1));
    EXPECT_TRUE(basService_->IsAnyBleProfileConnected(device2));
    // 断开device1
    basService_->UpdateGattClientIfMap(device1, clientIf1, static_cast<int>(BTConnectState::DISCONNECTED));
    // device1不应在映射表中，device2仍在
    EXPECT_FALSE(basService_->IsAnyBleProfileConnected(device1));
    EXPECT_TRUE(basService_->IsAnyBleProfileConnected(device2));
    HILOGI("BasService_UpdateGattClientIfMap_008 end");
}

/**
 * @tc.number: BasService_IsAnyBleProfileConnected_001
 * @tc.name: testIsAnyBleProfileConnectedDeviceNeverConnected
 * @tc.desc: 测试检查BLE Profile连接状态-设备从未连接过,验证返回false
 */
HWTEST_F(BasServiceTest, BasService_IsAnyBleProfileConnected_001, TestSize.Level1)
{
    HILOGI("BasService_IsAnyBleProfileConnected_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    // 设备从未被添加，应该返回false
    bool connected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(connected);
    HILOGI("BasService_IsAnyBleProfileConnected_001 end");
}

/**
 * @tc.number: BasService_IsAnyBleProfileConnected_002
 * @tc.name: testIsAnyBleProfileConnectedDeviceConnected
 * @tc.desc: 测试检查BLE Profile连接状态-设备已连接,验证返回true
 */
HWTEST_F(BasServiceTest, BasService_IsAnyBleProfileConnected_002, TestSize.Level1)
{
    HILOGI("BasService_IsAnyBleProfileConnected_002 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf = 1;
    // 添加设备到映射表
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::CONNECTED));
    // 应该返回true
    bool connected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_TRUE(connected);
    HILOGI("BasService_IsAnyBleProfileConnected_002 end");
}

/**
 * @tc.number: BasService_IsAnyBleProfileConnected_003
 * @tc.name: testIsAnyBleProfileConnectedAfterDisconnect
 * @tc.desc: 测试检查BLE Profile连接状态-设备断开后,验证返回false
 */
HWTEST_F(BasServiceTest, BasService_IsAnyBleProfileConnected_003, TestSize.Level1)
{
    HILOGI("BasService_IsAnyBleProfileConnected_003 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf = 1;
    // 添加设备
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::CONNECTED));
    ASSERT_TRUE(basService_->IsAnyBleProfileConnected(device));
    // 断开设备
    basService_->UpdateGattClientIfMap(device, clientIf, static_cast<int>(BTConnectState::DISCONNECTED));
    // 应该返回false
    bool connected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(connected);
    HILOGI("BasService_IsAnyBleProfileConnected_003 end");
}

/**
 * @tc.number: BasService_IsAnyBleProfileConnected_004
 * @tc.name: testIsAnyBleProfileConnectedPartialDisconnect
 * @tc.desc: 测试检查BLE Profile连接状态-部分断开时仍返回true
 */
HWTEST_F(BasServiceTest, BasService_IsAnyBleProfileConnected_004, TestSize.Level1)
{
    HILOGI("BasService_IsAnyBleProfileConnected_004 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf1 = 1;
    int clientIf2 = 2;
    // 添加两个连接
    basService_->UpdateGattClientIfMap(device, clientIf1, static_cast<int>(BTConnectState::CONNECTED));
    basService_->UpdateGattClientIfMap(device, clientIf2, static_cast<int>(BTConnectState::CONNECTED));
    ASSERT_TRUE(basService_->IsAnyBleProfileConnected(device));
    // 断开其中一个
    basService_->UpdateGattClientIfMap(device, clientIf1, static_cast<int>(BTConnectState::DISCONNECTED));
    // 还有clientIf2存在，仍应返回true
    bool connected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_TRUE(connected);
    HILOGI("BasService_IsAnyBleProfileConnected_004 end");
}

/**
 * @tc.number: BasService_HandleGattStateChanged_001
 * @tc.name: testHandleGattStateChangedInvalidClientIf
 * @tc.desc: 测试处理GATT状态变化-clientIf为0,验证方法提前返回不更新映射表
 */
HWTEST_F(BasServiceTest, BasService_HandleGattStateChanged_001, TestSize.Level1)
{
    HILOGI("BasService_HandleGattStateChanged_001 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int invalidClientIf = 0;
    int state = static_cast<int>(BTConnectState::CONNECTED);
    // clientIf为0时应该直接返回，不更新映射表
    basService_->HandleGattStateChanged(device, invalidClientIf, state);
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_HandleGattStateChanged_001 end");
}

/**
 * @tc.number: BasService_HandleGattStateChanged_002
 * @tc.name: testHandleGattStateChangedNegativeClientIf
 * @tc.desc: 测试处理GATT状态变化-负数clientIf,验证方法提前返回不更新映射表
 */
HWTEST_F(BasServiceTest, BasService_HandleGattStateChanged_002, TestSize.Level1)
{
    HILOGI("BasService_HandleGattStateChanged_002 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int negativeClientIf = -5;
    int state = static_cast<int>(BTConnectState::CONNECTED);
    // 负数clientIf应该直接返回
    basService_->HandleGattStateChanged(device, negativeClientIf, state);
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_HandleGattStateChanged_002 end");
}

/**
 * @tc.number: BasService_HandleGattStateChanged_003
 * @tc.name: testHandleGattStateChangedConnectState
 * @tc.desc: 测试处理GATT状态变化-连接状态时更新映射表
 */
HWTEST_F(BasServiceTest, BasService_HandleGattStateChanged_003, TestSize.Level1)
{
    HILOGI("BasService_HandleGattStateChanged_003 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf = 10;
    int state = static_cast<int>(BTConnectState::CONNECTED);
    // 连接状态应该更新映射表
    basService_->HandleGattStateChanged(device, clientIf, state);
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_TRUE(isConnected);
    HILOGI("BasService_HandleGattStateChanged_003 end");
}

/**
 * @tc.number: BasService_HandleGattStateChanged_004
 * @tc.name: testHandleGattStateChangedDisconnectState
 * @tc.desc: 测试处理GATT状态变化-断开状态时更新映射表
 */
HWTEST_F(BasServiceTest, BasService_HandleGattStateChanged_004, TestSize.Level1)
{
    HILOGI("BasService_HandleGattStateChanged_004 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf = 10;
    // 先建立连接
    basService_->HandleGattStateChanged(device, clientIf, static_cast<int>(BTConnectState::CONNECTED));
    ASSERT_TRUE(basService_->IsAnyBleProfileConnected(device));
    // 再断开
    basService_->HandleGattStateChanged(device, clientIf, static_cast<int>(BTConnectState::DISCONNECTED));
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_HandleGattStateChanged_004 end");
}

/**
 * @tc.number: BasService_HandleGattStateChanged_005
 * @tc.name: testHandleGattStateChangedMultipleConnections
 * @tc.desc: 测试处理GATT状态变化-多连接场景,部分断开仍保持连接状态
 */
HWTEST_F(BasServiceTest, BasService_HandleGattStateChanged_005, TestSize.Level1)
{
    HILOGI("BasService_HandleGattStateChanged_005 start");
    RawAddress device(TEST_DEVICE_ADDRESS);
    int clientIf1 = 1;
    int clientIf2 = 2;
    int clientIf3 = 3;
    // 添加三个连接
    basService_->HandleGattStateChanged(device, clientIf1, static_cast<int>(BTConnectState::CONNECTED));
    basService_->HandleGattStateChanged(device, clientIf2, static_cast<int>(BTConnectState::CONNECTED));
    basService_->HandleGattStateChanged(device, clientIf3, static_cast<int>(BTConnectState::CONNECTED));
    ASSERT_TRUE(basService_->IsAnyBleProfileConnected(device));
    // 断开两个
    basService_->HandleGattStateChanged(device, clientIf1, static_cast<int>(BTConnectState::DISCONNECTED));
    basService_->HandleGattStateChanged(device, clientIf2, static_cast<int>(BTConnectState::DISCONNECTED));
    // 还有clientIf3，设备仍在映射表中
    bool isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_TRUE(isConnected);
    // 断开最后一个
    basService_->HandleGattStateChanged(device, clientIf3, static_cast<int>(BTConnectState::DISCONNECTED));
    isConnected = basService_->IsAnyBleProfileConnected(device);
    EXPECT_FALSE(isConnected);
    HILOGI("BasService_HandleGattStateChanged_005 end");
}
} // Bluetooth
} // OHOS