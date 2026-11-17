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

#include "bluetooth_device_battery_observer_stub.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

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
        getBatteryLevelCount_++;
    }

    void OnBatteryLevelChanged(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        lastDevice_ = device;
        lastBatteryLevel_ = batteryLevel;
        levelChangedCount_++;
    }

    BluetoothRawAddress GetLastDevice() const
    {
        return lastDevice_;
    }

    int32_t GetLastBatteryLevel() const
    {
        return lastBatteryLevel_;
    }

    int GetBatteryLevelEventCount() const
    {
        return getBatteryLevelCount_;
    }

    int GetLevelChangedCount() const
    {
        return levelChangedCount_;
    }

private:
    BluetoothRawAddress lastDevice_;
    int32_t lastBatteryLevel_ = 0;
    int getBatteryLevelCount_ = 0;
    int levelChangedCount_ = 0;
};

class BluetoothDeviceBatteryObserverStubTest : public testing::Test {
public:
    BluetoothDeviceBatteryObserverStubTest() = default;
    ~BluetoothDeviceBatteryObserverStubTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

protected:
    std::shared_ptr<TestBluetoothDeviceBatteryObserverStub> stub_ = nullptr;
};

void BluetoothDeviceBatteryObserverStubTest::SetUpTestCase(void)
{
    HILOGI("BluetoothDeviceBatteryObserverStubTest::SetUpTestCase start");
}

void BluetoothDeviceBatteryObserverStubTest::TearDownTestCase(void)
{
    HILOGI("BluetoothDeviceBatteryObserverStubTest::TearDownTestCase start");
}

void BluetoothDeviceBatteryObserverStubTest::SetUp()
{
    HILOGI("BluetoothDeviceBatteryObserverStubTest::SetUp start");
    stub_ = std::make_shared<TestBluetoothDeviceBatteryObserverStub>();
}

void BluetoothDeviceBatteryObserverStubTest::TearDown()
{
    HILOGI("BluetoothDeviceBatteryObserverStubTest::TearDown start");
    stub_.reset();
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_Constructor_001
 * @tc.name: testBluetoothDeviceBatteryObserverStubConstructor
 * @tc.desc: 测试构造函数功能,验证对象能正常创建
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest, BluetoothDeviceBatteryObserverStub_Constructor_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_Constructor_001 start");
    auto stub = std::make_shared<TestBluetoothDeviceBatteryObserverStub>();
    EXPECT_NE(stub, nullptr);
    HILOGI("BluetoothDeviceBatteryObserverStub_Constructor_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_Destructor_001
 * @tc.name: testBluetoothDeviceBatteryObserverStubDestructor
 * @tc.desc: 测试析构函数功能,验证对象能正常析构
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest, BluetoothDeviceBatteryObserverStub_Destructor_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_Destructor_001 start");
    auto stub = std::make_shared<TestBluetoothDeviceBatteryObserverStub>();
    EXPECT_NE(stub, nullptr);
    HILOGI("BluetoothDeviceBatteryObserverStub_Destructor_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnRemoteRequest_001
 * @tc.name: testOnRemoteRequestGetBatteryLevelEvent
 * @tc.desc: 测试OnRemoteRequest方法处理获取电池电量事件,验证返回NO_ERROR
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest, BluetoothDeviceBatteryObserverStub_OnRemoteRequest_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnRemoteRequest_001 start");
    EXPECT_NE(stub_, nullptr);

    uint32_t code =
        static_cast<uint32_t>(BluetoothDeviceBatteryObserverInterfaceCode::BT_DEVICE_GET_BATTERYLEVEL_EVENT);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(stub_->GetDescriptor());
    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS);
    data.WriteParcelable(&rawAddr);
    data.WriteInt32(100);

    int32_t ret = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnRemoteRequest_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnRemoteRequest_002
 * @tc.name: testOnRemoteRequestBatteryLevelChanged
 * @tc.desc: 测试OnRemoteRequest方法处理电池电量变化事件,验证返回NO_ERROR
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest, BluetoothDeviceBatteryObserverStub_OnRemoteRequest_002,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnRemoteRequest_002 start");
    EXPECT_NE(stub_, nullptr);

    uint32_t code = static_cast<uint32_t>(BluetoothDeviceBatteryObserverInterfaceCode::BT_DEVICE_BATTERYLEVEL_CHANGED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(stub_->GetDescriptor());
    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS);
    data.WriteParcelable(&rawAddr);
    data.WriteInt32(85);

    int32_t ret = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnRemoteRequest_002 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnRemoteRequest_003
 * @tc.name: testOnRemoteRequestUnknownCode
 * @tc.desc: 测试OnRemoteRequest方法处理未知code,验证返回非零错误码
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest, BluetoothDeviceBatteryObserverStub_OnRemoteRequest_003,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnRemoteRequest_003 start");
    EXPECT_NE(stub_, nullptr);

    uint32_t code = 999;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(stub_->GetDescriptor());

    int32_t ret = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(ret, 0);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnRemoteRequest_003 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnRemoteRequest_004
 * @tc.name: testOnRemoteRequestInvalidToken
 * @tc.desc: 测试OnRemoteRequest方法处理无效的interface token,验证返回ERR_INVALID_STATE
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest, BluetoothDeviceBatteryObserverStub_OnRemoteRequest_004,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnRemoteRequest_004 start");
    EXPECT_NE(stub_, nullptr);

    uint32_t code =
        static_cast<uint32_t>(BluetoothDeviceBatteryObserverInterfaceCode::BT_DEVICE_GET_BATTERYLEVEL_EVENT);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"invalid_token");

    int32_t ret = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnRemoteRequest_004 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_001
 * @tc.name: testOnGetBatteryLevelEventInnerNormal
 * @tc.desc: 测试OnGetBatteryLevelEventInner方法正常处理获取电池电量事件,验证返回NO_ERROR
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest,
    BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_001, TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_001 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS);
    data.WriteParcelable(&rawAddr);
    data.WriteInt32(100);

    ErrCode ret = stub_->OnGetBatteryLevelEventInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_002
 * @tc.name: testOnGetBatteryLevelEventInnerBoundaryValue
 * @tc.desc: 测试OnGetBatteryLevelEventInner方法处理边界值0,验证能正确处理
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest,
    BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_002, TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_002 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS);
    data.WriteParcelable(&rawAddr);
    data.WriteInt32(0);

    ErrCode ret = stub_->OnGetBatteryLevelEventInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);
    EXPECT_EQ(stub_->GetLastBatteryLevel(), 0);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_002 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_003
 * @tc.name: testOnGetBatteryLevelEventInnerDifferentDevice
 * @tc.desc: 测试OnGetBatteryLevelEventInner方法处理不同设备地址,验证设备地址和电量值正确记录
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest,
    BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_003, TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_003 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS2);
    data.WriteParcelable(&rawAddr);
    data.WriteInt32(50);

    ErrCode ret = stub_->OnGetBatteryLevelEventInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);
    EXPECT_STREQ(stub_->GetLastDevice().GetAddress().c_str(), TEST_DEVICE_ADDRESS2.c_str());
    EXPECT_EQ(stub_->GetLastBatteryLevel(), 50);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_003 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_NullDevice_001
 * @tc.name: testOnGetBatteryLevelEventInnerNullDevice
 * @tc.desc: 测试OnGetBatteryLevelEventInner方法处理空的device parcel,验证返回TRANSACTION_ERR
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest,
    BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_NullDevice_001, TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_NullDevice_001 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    ErrCode ret = stub_->OnGetBatteryLevelEventInner(data, reply);
    EXPECT_EQ(ret, TRANSACTION_ERR);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEventInner_NullDevice_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_001
 * @tc.name: testOnBatteryLevelChangedInnerNormal
 * @tc.desc: 测试OnBatteryLevelChangedInner方法正常处理电池变化事件,验证返回NO_ERROR
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest,
    BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_001, TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_001 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS);
    data.WriteParcelable(&rawAddr);
    data.WriteInt32(85);

    ErrCode ret = stub_->OnBatteryLevelChangedInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_002
 * @tc.name: testOnBatteryLevelChangedInnerVerifyData
 * @tc.desc: 测试OnBatteryLevelChangedInner方法处理并验证回调数据正确性,验证设备地址和电量值
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest,
    BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_002, TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_002 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS);
    data.WriteParcelable(&rawAddr);
    data.WriteInt32(100);

    ErrCode ret = stub_->OnBatteryLevelChangedInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);
    EXPECT_EQ(stub_->GetLastBatteryLevel(), 100);
    EXPECT_STREQ(stub_->GetLastDevice().GetAddress().c_str(), TEST_DEVICE_ADDRESS.c_str());
    HILOGI("BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_002 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_003
 * @tc.name: testOnBatteryLevelChangedInnerDifferentDevice
 * @tc.desc: 测试OnBatteryLevelChangedInner方法处理不同设备地址,验证回调数据正确记录
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest,
    BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_003, TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_003 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS2);
    data.WriteParcelable(&rawAddr);
    data.WriteInt32(75);

    ErrCode ret = stub_->OnBatteryLevelChangedInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);
    EXPECT_STREQ(stub_->GetLastDevice().GetAddress().c_str(), TEST_DEVICE_ADDRESS2.c_str());
    EXPECT_EQ(stub_->GetLastBatteryLevel(), 75);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_003 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_004
 * @tc.name: testOnBatteryLevelChangedInnerNullDevice
 * @tc.desc: 测试OnBatteryLevelChangedInner方法处理空的device parcel,验证返回TRANSACTION_ERR
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest,
    BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_004, TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_004 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    ErrCode ret = stub_->OnBatteryLevelChangedInner(data, reply);
    EXPECT_EQ(ret, TRANSACTION_ERR);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnBatteryLevelChangedInner_004 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEvent_001
 * @tc.name: testMultipleEventsHandling
 * @tc.desc: 测试多次调用处理获取电池电量事件和电池变化事件,验证事件计数正确累加
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest, BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEvent_001,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEvent_001 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data1;
    MessageParcel reply1;
    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS);
    data1.WriteParcelable(&rawAddr);
    data1.WriteInt32(100);
    stub_->OnGetBatteryLevelEventInner(data1, reply1);

    MessageParcel data2;
    MessageParcel reply2;
    data2.WriteParcelable(&rawAddr);
    data2.WriteInt32(85);
    stub_->OnBatteryLevelChangedInner(data2, reply2);

    MessageParcel data3;
    MessageParcel reply3;
    data3.WriteParcelable(&rawAddr);
    data3.WriteInt32(95);
    stub_->OnGetBatteryLevelEventInner(data3, reply3);

    EXPECT_EQ(stub_->GetBatteryLevelEventCount(), 2);
    EXPECT_EQ(stub_->GetLevelChangedCount(), 1);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEvent_001 end");
}

/**
 * @tc.number: BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEvent_002
 * @tc.name: testBatteryLevelBoundaryValues
 * @tc.desc: 测试OnGetBatteryLevelEventInner和OnBatteryLevelChangedInner方法处理边界值0和100
 */
HWTEST_F(BluetoothDeviceBatteryObserverStubTest, BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEvent_002,
    TestSize.Level1)
{
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEvent_002 start");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data1;
    MessageParcel reply1;
    BluetoothRawAddress rawAddr(TEST_DEVICE_ADDRESS);
    data1.WriteParcelable(&rawAddr);
    data1.WriteInt32(0);
    ErrCode ret1 = stub_->OnGetBatteryLevelEventInner(data1, reply1);
    EXPECT_EQ(ret1, NO_ERROR);

    MessageParcel data2;
    MessageParcel reply2;
    data2.WriteParcelable(&rawAddr);
    data2.WriteInt32(100);
    ErrCode ret2 = stub_->OnBatteryLevelChangedInner(data2, reply2);
    EXPECT_EQ(ret2, NO_ERROR);

    EXPECT_EQ(stub_->GetLastBatteryLevel(), 100);
    HILOGI("BluetoothDeviceBatteryObserverStub_OnGetBatteryLevelEvent_002 end");
}
} // Bluetooth
} // OHOS