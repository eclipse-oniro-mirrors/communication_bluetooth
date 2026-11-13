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

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <chrono>
#include <future>
#include <thread>
#include "bluetooth_switch_module.h"
#include "bluetooth_log.h"
#include "ffrt_inner.h"
#include "bluetooth_errorcode.h"

using namespace testing;
using namespace testing::ext;
using namespace std::chrono_literals;

namespace OHOS {
namespace Bluetooth {
static std::shared_ptr<std::promise<void>> g_promisePtr {nullptr};
#define WAIT_CACHED_EVENT_COMPLETE std::this_thread::sleep_for(std::chrono::milliseconds(10))

class MockBluetoothSwitchAction : public IBluetoothSwitchAction {
public:
    MOCK_METHOD(int, EnableBluetooth, (bool, std::string, bool), (override));
    MOCK_METHOD(int, DisableBluetooth, (std::string, bool), (override));
    MOCK_METHOD(int, EnableBluetoothToRestrictMode, (std::string, bool), (override));
};

class BluetoothSwitchModuleTest : public testing::Test {
public:
    BluetoothSwitchModuleTest() = default;
    ~BluetoothSwitchModuleTest() = default;

    void SetUp() override
    {
        g_promisePtr = std::make_shared<std::promise<void>>();
        auto switchAction = std::make_unique<MockBluetoothSwitchAction>();
        switchAction_ = switchAction.get();
        switchModule_ = std::make_shared<BluetoothSwitchModule>(std::move(switchAction));
        switchModule_->taskTimeout_ = 20000;  // 20000 is 20ms
    }

    void TearDown() override
    {
        g_promisePtr = nullptr;
        switchModule_.reset();
        switchAction_ = nullptr;
    }

    MockBluetoothSwitchAction *switchAction_;
    std::shared_ptr<BluetoothSwitchModule> switchModule_;
};

/**
 * @tc.name: BluetoothSwitchModuleTest_001
 * @tc.desc: 开蓝牙
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_001, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_001 start");
    EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_ON);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_001 end");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_002
 * @tc.desc: 开蓝牙失败
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_002, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_002 start");
    EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), -1);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_002 start");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_003
 * @tc.desc: 关蓝牙
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_003, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_003 start");
    EXPECT_CALL(*switchAction_, DisableBluetooth(_, _)).WillOnce(Return(0));
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_OFF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_003 end");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_004
 * @tc.desc: 关蓝牙失败
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_004, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_004 start");
    EXPECT_CALL(*switchAction_, DisableBluetooth(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), -1);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_004 start");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_005
 * @tc.desc: 半开蓝牙
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_005, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_005 start");
    EXPECT_CALL(*switchAction_, EnableBluetoothToRestrictMode(_, _)).WillOnce(Return(0));
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_HALF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_005 end");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_006
 * @tc.desc: 半开蓝牙失败
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_006, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_006 start");
    EXPECT_CALL(*switchAction_, EnableBluetoothToRestrictMode(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), -1);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_006 end");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_007
 * @tc.desc: 半关蓝牙，从全开进入半关状态
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_007, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_007 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, DisableBluetooth(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*switchAction_, EnableBluetoothToRestrictMode(_, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 1);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_OFF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);
    WAIT_CACHED_EVENT_COMPLETE;

    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_HALF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    HILOGI("BluetoothSwitchModuleTest_007 end");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_008
 * @tc.desc: 反复开蓝牙、关蓝牙（1），
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_008, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_008 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(0));
        EXPECT_CALL(*switchAction_, DisableBluetooth(_, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 3);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_ON);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);
    WAIT_CACHED_EVENT_COMPLETE;

    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_OFF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    HILOGI("BluetoothSwitchModuleTest_008 end");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_009
 * @tc.desc: 反复开蓝牙、关蓝牙（1），
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_009, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_009 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 4);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_ON);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);
    WAIT_CACHED_EVENT_COMPLETE;

    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_009 end");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_010
 * @tc.desc: 反复开蓝牙，半关蓝牙（1）
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_010, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_010 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(0));
        EXPECT_CALL(*switchAction_, DisableBluetooth(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*switchAction_, EnableBluetoothToRestrictMode(_, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 5);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_ON);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 1);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_OFF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_HALF);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);

    HILOGI("BluetoothSwitchModuleTest_010 end");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_011
 * @tc.desc: 反复开蓝牙，半关蓝牙（2）
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_011, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_011 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(0));
        EXPECT_CALL(*switchAction_, DisableBluetooth(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*switchAction_, EnableBluetoothToRestrictMode(_, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 4);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_ON);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 1);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_OFF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_HALF);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);

    HILOGI("BluetoothSwitchModuleTest_011 start");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_012
 * @tc.desc: 关蓝牙动作去重
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_012, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_012 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, DisableBluetooth(_, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 2);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_OFF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_012 start");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_013
 * @tc.desc: 半开蓝牙动作去重（1）
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_013, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_013 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetoothToRestrictMode(_, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 2);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_HALF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_013 start");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_014
 * @tc.desc: 半开蓝牙动作去重（2）
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_014, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_014 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetoothToRestrictMode(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 1);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_HALF);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_ON);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_014 start");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_015
 * @tc.desc: 异常事件时序（1）
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_015, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_015 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(0));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 1);

    switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_ON);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    WAIT_CACHED_EVENT_COMPLETE;
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    HILOGI("BluetoothSwitchModuleTest_015 start");
}

/**
 * @tc.name: BluetoothSwitchModuleTest_016
 * @tc.desc: Test switch module timer dfr
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_016, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_016 start");
    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(0));
    }

    switchModule_->taskTimeout_ = 10000;  // 10ms
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_TRUE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 4);

    // 10 ms timeout, clear all state and cache event.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    HILOGI("BluetoothSwitchModuleTest_016 start");
}

 /**
 * @tc.name: BluetoothSwitchModuleTest_017
 * @tc.desc: Test DeduplicateCacheEvent function
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_017, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_017 start");

    BluetoothSwitchCacheEvent cachedEnableBluetoothEvent = {
        .event = BluetoothSwitchEvent::ENABLE_BLUETOOTH,
        .callingName = "",
    };
    BluetoothSwitchCacheEvent cachedDisableBluetoothEvent = {
        .event = BluetoothSwitchEvent::DISABLE_BLUETOOTH,
        .callingName = "",
    };
    switchModule_->cachedEventVec_.push_back(cachedEnableBluetoothEvent);
    switchModule_->cachedEventVec_.push_back(cachedDisableBluetoothEvent);
    switchModule_->cachedEventVec_.push_back(cachedEnableBluetoothEvent);
    switchModule_->cachedEventVec_.push_back(cachedEnableBluetoothEvent);
    switchModule_->cachedEventVec_.push_back(cachedDisableBluetoothEvent);
    switchModule_->DeduplicateCacheEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 1);
    EXPECT_EQ(switchModule_->cachedEventVec_.back().event, BluetoothSwitchEvent::DISABLE_BLUETOOTH);

    HILOGI("BluetoothSwitchModuleTest_017 start");
}

 /**
 * @tc.name: BluetoothSwitchModuleTest_018
 * @tc.desc: Test dialog for user confirm function
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothSwitchModuleTest, BluetoothSwitchModuleTest_018, TestSize.Level1)
{
    HILOGI("BluetoothSwitchModuleTest_018 start");

    {
        InSequence seq;
        EXPECT_CALL(*switchAction_, EnableBluetooth(_, _, _)).WillOnce(Return(BT_ERR_DIALOG_FOR_USER_CONFIRM));
        EXPECT_CALL(*switchAction_, DisableBluetooth(_, _)).WillOnce(Return(BT_ERR_DIALOG_FOR_USER_CONFIRM));
        EXPECT_CALL(*switchAction_, EnableBluetoothToRestrictMode(_, _)).WillOnce(Return(BT_ERR_DIALOG_FOR_USER_CONFIRM));
    }

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH), 0);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH), 0);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    EXPECT_EQ(switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE), 0);
    EXPECT_FALSE(switchModule_->isBtSwitchProcessing_);
    EXPECT_EQ(switchModule_->cachedEventVec_.size(), 0);

    HILOGI("BluetoothSwitchModuleTest_018 start");
}
} // Bluetooth
} // OHOS
