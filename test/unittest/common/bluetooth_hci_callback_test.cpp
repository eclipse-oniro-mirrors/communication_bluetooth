/*
 * Copyright (c) 2012 Huawei Device Co., Ltd. All rights reserved.
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
#include "bluetooth_hdi.h"
#include "bluetooth_hci_callbacks.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

class BluetoothHciCallbackTest : public testing::Test {
public:
    sptr<BluetoothHciCallbacks> hcicallbacks {nullptr};

    BluetoothHciCallbackTest() {}
    ~BluetoothHciCallbackTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BluetoothHciCallbackTest::SetUpTestCase(void)
{
}
void BluetoothHciCallbackTest::TearDownTestCase(void)
{
}
void BluetoothHciCallbackTest::SetUp()
{
}
void BluetoothHciCallbackTest::TearDown()
{
}

void OnInited(BtInitStatus status)
{
}

void OnReceivedHciPacket(BtPacketType type, const BtPacket *packet)
{
}

/*
 * @tc.number: ModuleTest_OnInited_001
 * @tc.name: OnInited
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHciCallbackTest, ModuleTest_OnInited_001, TestSize.Level1)
{
    HILOGI("ModuleTest_OnInited_001 start");

    BtHciCallbacks callbacks;
    hcicallbacks = new (std::nothrow) BluetoothHciCallbacks(&callbacks);
    EXPECT_EQ(hcicallbacks->OnInited(static_cast<BtStatus>(0)), 0);

    HILOGI("ModuleTest_OnInited_001 end");
}

/*
 * @tc.number: ModuleTest_OnInited_002
 * @tc.name: OnInited
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHciCallbackTest, ModuleTest_OnInited_002, TestSize.Level1)
{
    HILOGI("ModuleTest_OnInited_002 start");

    BtHciCallbacks callbacks;
    hcicallbacks = new (std::nothrow) BluetoothHciCallbacks(&callbacks);
    hcicallbacks->callbacks_ = &callbacks;
    callbacks.OnInited = OnInited;
    EXPECT_EQ(hcicallbacks->OnInited(static_cast<BtStatus>(0)), 0);

    HILOGI("ModuleTest_OnInited_002 end");
}

/*
 * @tc.number: ModuleTest_OnInited_003
 * @tc.name: OnInited
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHciCallbackTest, ModuleTest_OnInited_003, TestSize.Level1)
{
    HILOGI("ModuleTest_OnInited_003 start");

    BtHciCallbacks callbacks;
    hcicallbacks = new (std::nothrow) BluetoothHciCallbacks(&callbacks);
    hcicallbacks->callbacks_ = &callbacks;
    callbacks.OnInited = OnInited;
    EXPECT_EQ(hcicallbacks->OnInited(static_cast<BtStatus>(1)), 0);

    HILOGI("ModuleTest_OnInited_003 end");
}

/*
 * @tc.number: ModuleTest_OnInited_004
 * @tc.name: OnInited
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHciCallbackTest, ModuleTest_OnInited_004, TestSize.Level1)
{
    HILOGI("ModuleTest_OnInited_004 start");

    BtHciCallbacks callbacks;
    hcicallbacks = new (std::nothrow) BluetoothHciCallbacks(&callbacks);
    hcicallbacks->callbacks_ = nullptr;
    EXPECT_EQ(hcicallbacks->OnInited(static_cast<BtStatus>(0)), 0);

    HILOGI("ModuleTest_OnInited_004 end");
}

/*
 * @tc.number: ModuleTest_OnReceivedHciPacket_001
 * @tc.name: OnReceivedHciPacket
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHciCallbackTest, ModuleTest_OnReceivedHciPacket_001, TestSize.Level1)
{
    HILOGI("ModuleTest_OnReceivedHciPacket_001 start");

    BtHciCallbacks callbacks;
    hcicallbacks = new (std::nothrow) BluetoothHciCallbacks(&callbacks);

    hcicallbacks->callbacks_ = nullptr;
    std::vector<uint8_t> data;
    EXPECT_EQ(hcicallbacks->OnReceivedHciPacket(BtType::HCI_EVENT, data), 0);

    HILOGI("ModuleTest_OnReceivedHciPacket_001 end");
}

/*
 * @tc.number: ModuleTest_OnReceivedHciPacket_002
 * @tc.name: OnReceivedHciPacket
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHciCallbackTest, ModuleTest_OnReceivedHciPacket_002, TestSize.Level1)
{
    HILOGI("ModuleTest_OnReceivedHciPacket_002 start");

    BtHciCallbacks callbacks;
    hcicallbacks = new (std::nothrow) BluetoothHciCallbacks(&callbacks);

    std::vector<uint8_t> data;
    EXPECT_EQ(hcicallbacks->OnReceivedHciPacket(BtType::HCI_EVENT, data), 0);

    HILOGI("ModuleTest_OnReceivedHciPacket_002 end");
}

/*
 * @tc.number: ModuleTest_OnReceivedHciPacket_003
 * @tc.name: OnReceivedHciPacket
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHciCallbackTest, ModuleTest_OnReceivedHciPacket_003, TestSize.Level1)
{
    HILOGI("ModuleTest_OnReceivedHciPacket_003 start");

    BtHciCallbacks callbacks;
    callbacks.OnReceivedHciPacket = OnReceivedHciPacket;
    hcicallbacks = new (std::nothrow) BluetoothHciCallbacks(&callbacks);

    std::vector<uint8_t> data;
    EXPECT_EQ(hcicallbacks->OnReceivedHciPacket(BtType::HCI_EVENT, data), 0);
    EXPECT_EQ(hcicallbacks->OnReceivedHciPacket(BtType::ACL_DATA, data), 0);
    EXPECT_EQ(hcicallbacks->OnReceivedHciPacket(BtType::SCO_DATA, data), 0);
    EXPECT_EQ(hcicallbacks->OnReceivedHciPacket(static_cast<BtType>(99), data), 0);
    HILOGI("ModuleTest_OnReceivedHciPacket_003 end");
}

/*
 * @tc.number: ModuleTest_SetRTSchedule_001
 * @tc.name: SetRTSchedule
 * @tc.desc: name :
 */
HWTEST_F(BluetoothHciCallbackTest, ModuleTest_SetRTSchedule_001, TestSize.Level1)
{
    HILOGI("ModuleTest_SetRTSchedule_001 start");

    BtHciCallbacks callbacks;
    callbacks.OnReceivedHciPacket = OnReceivedHciPacket;
    hcicallbacks = new (std::nothrow) BluetoothHciCallbacks(&callbacks);
    hcicallbacks->SetRTSchedule();
    EXPECT_TRUE(hcicallbacks->isThreadPromoted);
    HILOGI("ModuleTest_SetRTSchedule_001 end");
}

}  // namespace Bluetooth
}  // namespace OHOS