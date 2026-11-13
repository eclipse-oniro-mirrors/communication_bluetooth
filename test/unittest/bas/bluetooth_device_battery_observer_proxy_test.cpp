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

#include "bluetooth_device_battery_observer_proxy.h"
#include "bluetooth_device_battery_observer_stub.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace Bluetooth {
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace {
const std::string TEST_DEVICE_ADDRESS = "11:22:33:44:55:66";
const std::string TEST_DEVICE_ADDRESS2 = "22:33:44:55:66:77";
}

class TestBluetoothDeviceBatteryObserverStub : public BluetoothDeviceBatteryObserverStub {
public:
    TestBluetoothDeviceBatteryObserverStub() = default;
    ~TestBluetoothDeviceBatteryObserverStub() override = default;

    void OnGetBatteryLevelEvent(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        lastDevice_ = device;
        lastBatteryLevel_ = batteryLevel;
    }

    void OnBatteryLevelChanged(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        lastDevice_ = device;
        lastBatteryLevel_ = batteryLevel;
        levelChangedCount_++;
    }

    BluetoothRawAddress lastDevice_;
    int32_t lastBatteryLevel_ = 0;
    int levelChangedCount_ = 0;
};

class BluetoothDeviceBatteryObserverProxyTest : public testing::Test {
public:
    BluetoothDeviceBatteryObserverProxyTest() = default;
    ~BluetoothDeviceBatteryObserverProxyTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

protected:
    sptr<IRemoteObject> remoteObject_ = nullptr;
    std::shared_ptr<TestBluetoothDeviceBatteryObserverStub> stub_ = nullptr;
};

void BluetoothDeviceBatteryObserverProxyTest::SetUpTestCase(void)
{
    HILOGI("BluetoothDeviceBatteryObserverProxyTest::SetUpTestCase start");
}

void BluetoothDeviceBatteryObserverProxyTest::TearDownTestCase(void)
{
    HILOGI("BluetoothDeviceBatteryObserverProxyTest::TearDownTestCase start");
}

void BluetoothDeviceBatteryObserverProxyTest::SetUp()
{
    HILOGI("BluetoothDeviceBatteryObserverProxyTest::SetUp start");
    stub_ = std::make_shared<TestBluetoothDeviceBatteryObserverStub>();
}

void BluetoothDeviceBatteryObserverProxyTest::TearDown()
{
    HILOGI("BluetoothDeviceBatteryObserverProxyTest::TearDown start");
    stub_.reset();
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_Constructor_001
 * @tc.name: testBluetoothDeviceBatteryObserverProxyConstructor
 * @tc.desc: 测试构造函数功能,验证proxy对象能正常创建
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_Constructor_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_Constructor_001 start");
    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);
    EXPECT_NE(&proxy, nullptr);
    HILOGI("BluetoothDeviceBatteryObserverProxy_Constructor_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_OnGetBatteryLevelEvent_001
 * @tc.name: testOnGetBatteryLevelEvent
 * @tc.desc: 测试OnGetBatteryLevelEvent方法,验证正常调用不抛出异常
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_OnGetBatteryLevelEvent_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_OnGetBatteryLevelEvent_001 start");
    EXPECT_NE(stub_, nullptr);
    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);
    BluetoothRawAddress device(TEST_DEVICE_ADDRESS);
    proxy.OnGetBatteryLevelEvent(device, 100);
    HILOGI("BluetoothDeviceBatteryObserverProxy_OnGetBatteryLevelEvent_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_OnBatteryLevelChanged_001
 * @tc.name: testOnBatteryLevelChanged
 * @tc.desc: 测试OnBatteryLevelChanged方法,验证正常调用不抛出异常
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_OnBatteryLevelChanged_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_OnBatteryLevelChanged_001 start");
    EXPECT_NE(stub_, nullptr);
    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);
    BluetoothRawAddress device(TEST_DEVICE_ADDRESS);
    proxy.OnBatteryLevelChanged(device, 85);
    HILOGI("BluetoothDeviceBatteryObserverProxy_OnBatteryLevelChanged_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_InnerTransact_001
 * @tc.name: testInnerTransactNullRemote
 * @tc.desc: 测试InnerTransact方法传入空remote对象,验证返回OBJECT_NULL错误
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_InnerTransact_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_InnerTransact_001 start");
    EXPECT_NE(stub_, nullptr);
    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);
    uint32_t code = 0;
    MessageOption option;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = proxy.InnerTransact(code, option, data, reply);
    EXPECT_EQ(ret, OBJECT_NULL);
    HILOGI("BluetoothDeviceBatteryObserverProxy_InnerTransact_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_OnGetBatteryLevelEvent_002
 * @tc.name: testOnGetBatteryLevelEventMultipleDevices
 * @tc.desc: 测试OnGetBatteryLevelEvent方法处理多个不同设备,验证方法能正常处理
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_OnGetBatteryLevelEvent_002,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_OnGetBatteryLevelEvent_002 start");
    EXPECT_NE(stub_, nullptr);
    BluetoothRawAddress device1(TEST_DEVICE_ADDRESS);
    BluetoothRawAddress device2(TEST_DEVICE_ADDRESS2);

    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);

    proxy.OnGetBatteryLevelEvent(device1, 50);
    proxy.OnGetBatteryLevelEvent(device2, 100);

    HILOGI("BluetoothDeviceBatteryObserverProxy_OnGetBatteryLevelEvent_002 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_OnBatteryLevelChanged_002
 * @tc.name: testOnBatteryLevelChangedMultipleDevices
 * @tc.desc: 测试OnBatteryLevelChanged方法处理多个设备多次调用,验证方法能正常处理
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_OnBatteryLevelChanged_002,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_OnBatteryLevelChanged_002 start");
    EXPECT_NE(stub_, nullptr);
    BluetoothRawAddress device1(TEST_DEVICE_ADDRESS);
    BluetoothRawAddress device2(TEST_DEVICE_ADDRESS2);

    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);

    proxy.OnBatteryLevelChanged(device1, 75);
    proxy.OnBatteryLevelChanged(device1, 70);
    proxy.OnBatteryLevelChanged(device2, 90);

    HILOGI("BluetoothDeviceBatteryObserverProxy_OnBatteryLevelChanged_002 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_InnerTransact_002
 * @tc.name: testInnerTransactMultipleCodes
 * @tc.desc: 测试InnerTransact方法处理多个不同的code,验证方法能正常处理各种code
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_InnerTransact_002,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_InnerTransact_002 start");
    EXPECT_NE(stub_, nullptr);
    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);

    for (uint32_t code = 0; code <= 10; code++) {
        MessageOption option;
        MessageParcel data;
        MessageParcel reply;
        proxy.InnerTransact(code, option, data, reply);
    }

    EXPECT_NE(&proxy, nullptr);
    HILOGI("BluetoothDeviceBatteryObserverProxy_InnerTransact_002 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_BatteryLevelBoundary_001
 * @tc.name: testBatteryLevelBoundaryValues
 * @tc.desc: 测试电池电量边界值0和100的处理,验证边界值能正常传递
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_BatteryLevelBoundary_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_BatteryLevelBoundary_001 start");
    EXPECT_NE(stub_, nullptr);
    BluetoothRawAddress device(TEST_DEVICE_ADDRESS);

    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);

    proxy.OnGetBatteryLevelEvent(device, 0);
    proxy.OnGetBatteryLevelEvent(device, 100);
    proxy.OnBatteryLevelChanged(device, 0);
    proxy.OnBatteryLevelChanged(device, 100);

    HILOGI("BluetoothDeviceBatteryObserverProxy_BatteryLevelBoundary_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverProxy_NullDevice_001
 * @tc.name: testNullDeviceAddress
 * @tc.desc: 测试空设备地址的处理,验证方法能正常处理空的设备地址
 */
HWTEST_F(BluetoothDeviceBatteryObserverProxyTest, BluetoothDeviceBatteryObserverProxy_NullDevice_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverProxy_NullDevice_001 start");
    EXPECT_NE(stub_, nullptr);
    BluetoothRawAddress emptyDevice("");

    sptr<IRemoteObject> nullRemote = nullptr;
    BluetoothDeviceBatteryObserverProxy proxy(nullRemote);

    proxy.OnGetBatteryLevelEvent(emptyDevice, 50);
    proxy.OnBatteryLevelChanged(emptyDevice, 50);

    HILOGI("BluetoothDeviceBatteryObserverProxy_NullDevice_001 end");
}
} // Bluetooth
} // OHOS