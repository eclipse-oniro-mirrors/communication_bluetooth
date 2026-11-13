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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bt_chr_switch.h"
#include "bluetooth_log.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
class BtChrSwitchMock {
public:
    MOCK_METHOD((void), BtChrWriteHiSysEvent,
        (int errCode, std::string switchTime, std::string callingName, int action, int64_t actionTimeStamp));
};

namespace {
BtChrSwitchMock *g_switchMock = nullptr;
void BtChrSwitchMockInit(BtChrSwitchMock *mock)
{
    g_switchMock = mock;
}
void BtChrSwitchMockDeInit()
{
    g_switchMock = nullptr;
}
}

extern "C" void BtChrWriteHiSysEvent(
    int errCode, const std::string &switchTime, const std::string &callingName, int action, int64_t actionTimeStamp)
{
    if (g_switchMock) {
        g_switchMock->BtChrWriteHiSysEvent(errCode, switchTime, callingName, action, actionTimeStamp);
    }
}

class BtChrSwitchTest : public testing::Test {
public:
    enum SWITCH_ACTION {
        ACTION_INVALID = 0,
        ACTION_ENABLE,
        ACTION_ENABLE_FROM_OFF_TO_HALF,
        ACTION_ENABLE_FROM_HALF_TO_ON,
        ACTION_DISABLE,
    } action;

    BtChrSwitchTest() = default;
    ~BtChrSwitchTest() = default;

    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        const char *btOnState = "1";
        system::SetParameter("persist.bluetooth.switch_enable", btOnState);
    };
    void TearDown()
    {
        BtChrSwitchMockDeInit();
    };
};

/**
 * @tc.name: Test_BtChrSwitchTest_001
 * @tc.desc: Check bluetooth switch success
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSwitchTest, Test_BtChrSwitchTest_001, TestSize.Level0)
{
    HILOGI("Test_BtChrSwitchTest_001 enter");
    BtChrSwitchMock switchMock;
    BtChrSwitchMockInit(&switchMock);

    {
        InSequence seq;

        EXPECT_CALL(switchMock, BtChrWriteHiSysEvent(BT_CHR_SWITCH_SUCCESS, _, "switchTest", ACTION_ENABLE, _));
        EXPECT_CALL(switchMock, BtChrWriteHiSysEvent(BT_CHR_SWITCH_SUCCESS, _, "switchTest", ACTION_DISABLE, _));
        EXPECT_CALL(switchMock,
            BtChrWriteHiSysEvent(BT_CHR_SWITCH_SUCCESS, _, "switchTest", ACTION_ENABLE_FROM_OFF_TO_HALF, _));
        EXPECT_CALL(switchMock,
            BtChrWriteHiSysEvent(BT_CHR_SWITCH_SUCCESS, _, "switchTest", ACTION_ENABLE_FROM_HALF_TO_ON, _));
    }

    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_ENABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_ENABLE_SUCCESS));
    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_DISABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_DISABLE_SUCCESS));

    const char *halfSwitchState = "2";
    system::SetParameter("persist.bluetooth.switch_enable", halfSwitchState);
    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_ENABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_ENABLE_SUCCESS));

    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_ENABLE_FROM_HALF_TO_ON));

    HILOGI("Test_BtChrSwitchTest_001 end");
}

/**
 * @tc.name: Test_BtChrSwitchTest_002
 * @tc.desc: Check bluetooth switch failed
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSwitchTest, Test_BtChrSwitchTest_002, TestSize.Level0)
{
    HILOGI("Test_BtChrSwitchTest_002 enter");
    BtChrSwitchMock switchMock;
    BtChrSwitchMockInit(&switchMock);

    {
        InSequence seq;

        EXPECT_CALL(switchMock,
            BtChrWriteHiSysEvent(BT_CHR_SWITCH_FAILED_OPEN_CHIPSET, _, "switchTest", ACTION_ENABLE, _));
        EXPECT_CALL(switchMock, BtChrWriteHiSysEvent(BT_CHR_SWITCH_FAILED, _, "switchTest", ACTION_ENABLE, _));
        EXPECT_CALL(switchMock, BtChrWriteHiSysEvent(BT_CHR_SWITCH_SUCCESS, _, "switchTest", ACTION_ENABLE, _));

        EXPECT_CALL(switchMock, BtChrWriteHiSysEvent(BT_CHR_SWITCH_FAILED, _, "switchTest", ACTION_DISABLE, _));
        EXPECT_CALL(switchMock, BtChrWriteHiSysEvent(BT_CHR_SWITCH_SUCCESS, _, "switchTest", ACTION_ENABLE, _));
    }

    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_ENABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_ENABLE_FAILED_OPEN_CHIPSET));

    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_ENABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_ENABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_ENABLE_SUCCESS));

    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_DISABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent("switchTest", EVENT_TYPE_BT_ENABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_ENABLE_SUCCESS));
    HILOGI("Test_BtChrSwitchTest_002 end");
}

/**
 * @tc.name: Test_BtChrSwitchTest_003
 * @tc.desc: Check bluetooth switch reboot
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSwitchTest, Test_BtChrSwitchTest_003, TestSize.Level0)
{
    HILOGI("Test_BtChrSwitchTest_003 enter");
    BtChrSwitchMock switchMock;
    BtChrSwitchMockInit(&switchMock);

    {
        InSequence seq;

        EXPECT_CALL(switchMock,
            BtChrWriteHiSysEvent(BT_CHR_SWITCH_FAILED_OPEN_CHIPSET, _, "bluetooth_reboot", ACTION_ENABLE, _)).Times(0);
    }

    BtChrWriteSwitchEvent(ChrSwitchEvent("bluetooth_reboot", EVENT_TYPE_BT_ENABLE));
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_ENABLE_FAILED_OPEN_CHIPSET));
    HILOGI("Test_BtChrSwitchTest_003 end");
}

}  // namespace bluetooth
}  // namespace OHOS
