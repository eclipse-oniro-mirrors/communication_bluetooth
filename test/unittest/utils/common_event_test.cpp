/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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
#include "bluetooth_common_event_helper.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "adapter_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
const int TIME = 2;
static BluetoothHost *host_;
static constexpr uint32_t WAIT_TIMEOUT = 10;  // 10ms
static constexpr uint32_t PUBLISH_SUCCESS = 1;  // publish success
static constexpr uint32_t PUBLISH_FAILED = 0;  // publish failed
static const std::string TEST_COMMON_EVENT = "usual.event.bluetooth.test.STATE_CHANGED";
class CommonEventTest : public testing::Test {
public:
    CommonEventTest() = default;
    ~CommonEventTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void CommonEventTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBrEnable = %{public}d", isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}

void CommonEventTest::TearDownTestSuite(void)
{
}

void CommonEventTest::SetUp()
{}

void CommonEventTest::TearDown()
{}

/**
 * @tc.name: Common_Event_Test_PublishEvent_000
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEvent_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEvent_000 start");
    OHOS::AAFwk::Want want;
    want.SetAction(TEST_COMMON_EVENT);

    std::vector<std::string> permissions;
    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEvent(want,
        0, false, permissions, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEvent_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEvent_001
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEvent_001, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEvent_001 start");
    OHOS::AAFwk::Want want;
    want.SetAction(TEST_COMMON_EVENT);

    std::vector<std::string> permissions;
    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEvent(want,
        -1, false, permissions, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEvent_001 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEvent_002
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEvent_002, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEvent_002 start");
    OHOS::AAFwk::Want want;
    want.SetAction(TEST_COMMON_EVENT);

    std::vector<std::string> permissions;
    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEvent(want,
        -1, true, permissions, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEvent_002 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEvent_003
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEvent_003, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEvent_003 start");
    OHOS::AAFwk::Want want;
    want.SetAction(TEST_COMMON_EVENT);

    std::vector<std::string> permissions {"ohos.permission.ACCESS_BLUETOOTH"};
    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEvent(want,
        0, true, permissions, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEvent_003 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithAccessPermission_000
 * @tc.desc: PublishEventWithAccessPermission
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithAccessPermission_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithAccessPermission_000 start");
    OHOS::AAFwk::Want want;
    want.SetAction(TEST_COMMON_EVENT);

    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithAccessPermission(want, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEventWithAccessPermission_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithPeerRealMac_000
 * @tc.desc: PublishEventWithPeerRealMac
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithPeerRealMac_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithPeerRealMac_000 start");
    OHOS::AAFwk::Want want;
    want.SetAction(TEST_COMMON_EVENT);

    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithPeerRealMac(want, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEventWithPeerRealMac_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithoutPermisssion_000
 * @tc.desc: PublishEventWithoutPermisssion
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithoutPermisssion_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithoutPermisssion_000 start");
    OHOS::AAFwk::Want want;
    want.SetAction(TEST_COMMON_EVENT);

    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithoutPermisssion(want);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEventWithoutPermisssion_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithIntParamAndPeerRealMac_000
 * @tc.desc: PublishEventWithIntParamAndPeerRealMac
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithIntParamAndPeerRealMac_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithIntParamAndPeerRealMac_000 start");
    std::string device = "00:00:00:00:00:00";
    std::string key = "state";
    int value = 1;

    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithIntParamAndPeerRealMac(
        TEST_COMMON_EVENT, device, key, value, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEventWithIntParamAndPeerRealMac_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithIntParam_000
 * @tc.desc: PublishEventWithIntParam
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithIntParam_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithIntParam_000 start");
    std::vector<std::string> permissions {"ohos.permission.ACCESS_BLUETOOTH"};
    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithIntParam(TEST_COMMON_EVENT,
        5, permissions, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEventWithIntParam_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithIntParam_001
 * @tc.desc: PublishEventWithIntParam
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithIntParam_001, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithIntParam_001 start");
    std::string device = "00:00:00:00:00:00";
    std::string key = "state";
    int value = 9;
    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithIntParam(TEST_COMMON_EVENT,
        device, key, value, false);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEventWithIntParam_001 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithStringParam_000
 * @tc.desc: PublishEventWithStringParam
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithStringParam_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithStringParam_000 start");
    std::string device = "00:00:00:00:00:00";
    std::string key = "state";
    std::string value = "test value";
    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithStringParam(TEST_COMMON_EVENT,
        device, key, value);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEventWithStringParam_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithVecParam_000
 * @tc.desc: PublishEventWithVecParam
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithVecParam_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithVecParam_000 start");
    std::string device = "00:00:00:00:00:00";
    std::string key = "state";
    std::vector<std::string> value;
    bool publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithVecParam(TEST_COMMON_EVENT,
        device, key, value);

    EXPECT_EQ(publishResult, PUBLISH_SUCCESS);
    HILOGI("Common_Event_Test_PublishEventWithVecParam_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEvent_004
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEvent_004, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEvent_004 start");
    BluetoothHelper::BluetoothCommonEventHelper::PublishBluetoothStateChangeEvent(
        bluetooth::BTStateID::STATE_TURN_ON, bluetooth::BTTransport::ADAPTER_BREDR);
    BluetoothHelper::BluetoothCommonEventHelper::PublishDiscoveryStartedEvent(OHOS::bluetooth::DISCOVERY_STARTED);
    BluetoothHelper::BluetoothCommonEventHelper::PublishDiscoveryFinishedEvent(OHOS::bluetooth::DISCOVERY_STOPED);
    std::string localName = "Sweetheart";
    BluetoothHelper::BluetoothCommonEventHelper::PublishLocalNameChangedEvent(localName);
    std::string device = "00:00:00:00:00:00";
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteNameChangedEvent(device, localName);
    std::vector<std::string> uuids {"0000111E-0000-1000-8000-00805F9B34FB"};
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteUuidsChangedEvent(device, uuids);
    int linktype = LINK_TYPE_BREDR;
    BluetoothHelper::BluetoothCommonEventHelper::PublishAclConnectedEvent(device, linktype);
    BluetoothHelper::BluetoothCommonEventHelper::PublishAclDisconnectedEvent(device, linktype);
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));
    int state= static_cast<int32_t>(BTConnectState::CONNECTED);
    BluetoothHelper::BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(device, state);
    BluetoothHelper::BluetoothCommonEventHelper::PublishHfpConnectStateUpdateEvent(device, state);
    BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpConnectStateUpdateEvent(device, state);
    state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    BluetoothHelper::BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(device, state);
    BluetoothHelper::BluetoothCommonEventHelper::PublishHfpConnectStateUpdateEvent(device, state);
    BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpConnectStateUpdateEvent(device, state);
    HILOGI("Common_Event_Test_PublishEvent_004 end");
}

/**
 * @tc.name: Common_Event_Test_PublishRemoteDevicePairStateEvent_000
 * @tc.desc: PublishRemoteDevicePairStateEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishRemoteDevicePairStateEvent_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishRemoteDevicePairStateEvent_000 start");
    std::string device = "00:00:00:00:00:00";
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteDevicePairStateEvent(device, 0);
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteDevicePairStateEvent(device, 1);
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteDevicePairStateEvent(device, 2);
    HILOGI("Common_Event_Test_PublishRemoteDevicePairStateEvent_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishHidConnectionStateEvent_000
 * @tc.desc: PublishHidConnectionStateEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishHidConnectionStateEvent_000, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishHidConnectionStateEvent_000 start");
    std::string device = "00:00:00:00:00:00";
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));
    int state = static_cast<int32_t>(BTConnectState::CONNECTED);
    BluetoothHelper::BluetoothCommonEventHelper::PublishHidConnectionStateEvent(device, state);
    state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    BluetoothHelper::BluetoothCommonEventHelper::PublishHidConnectionStateEvent(device, state);
    HILOGI("Common_Event_Test_PublishHidConnectionStateEvent_000 end");
}

/**
 * @tc.name: Common_Event_Test_PublishA2dpCodecChangeEvent_100
 * @tc.desc: PublishA2dpCodecChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishA2dpCodecChangeEvent_100, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishA2dpCodecChangeEvent_100 start");
    std::string device = "00:00:00:00:00:00";
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));
    A2dpSrcCodecInfo a2dpSrcCodecInfo;
    bool res = BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpCodecChangeEvent(device, a2dpSrcCodecInfo);
    HILOGI("Common_Event_Test_PublishA2dpCodecChangeEvent_100 end");
}

/**
 * @tc.name: Common_Event_Test_PublishA2dpCodecUpdateEvent_100
 * @tc.desc: PublishA2dpCodecUpdateEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishA2dpCodecUpdateEvent_100, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishA2dpCodecUpdateEvent_100 start");
    std::string device = "00:00:00:00:00:00";
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));
    A2dpSrcCodecInfo a2dpSrcCodecInfo;
    bool res = BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpCodecUpdateEvent(device, a2dpSrcCodecInfo);
    HILOGI("Common_Event_Test_PublishA2dpCodecUpdateEvent_100 end");
}

/**
 * @tc.name: Common_Event_Test_PublishEventWithMultiIntParam_100
 * @tc.desc: PublishEventWithMultiIntParam
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishEventWithMultiIntParam_100, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishEventWithMultiIntParam_100 start");
    std::string eventAction = "";
    std::string device = "";
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));
    std::vector<std::pair<std::string, int>> intParams = {};
    bool isSticky = false;
    EXPECT_FALSE(BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithMultiIntParam(
        eventAction, device, intParams, isSticky));
    HILOGI("Common_Event_Test_PublishEventWithMultiIntParam_100 end");
}

/**
 * @tc.name: Common_Event_Test_PublishScanModeChangeEvent_100
 * @tc.desc: PublishScanModeChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishScanModeChangeEvent_100, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishScanModeChangeEvent_100 start");
    std::string device = "00:00:00:00:00:00";
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));
    int scanMode = 0;
    BluetoothHelper::BluetoothCommonEventHelper::PublishScanModeChangeEvent(scanMode);
    HILOGI("Common_Event_Test_PublishScanModeChangeEvent_100 end");
}

/**
 * @tc.name: Common_Event_Test_PublishScoConnectStateChangeEvent_100
 * @tc.desc: PublishScoConnectStateChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishScoConnectStateChangeEvent_100, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishScoConnectStateChangeEvent_100 start");
    std::string device = "00:00:00:00:00:00";
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));

    int scoState = 1;
    BluetoothHelper::BluetoothCommonEventHelper::PublishScoConnectStateChangeEvent(device, scoState);
    HILOGI("Common_Event_Test_PublishScoConnectStateChangeEvent_100 end");
}

/**
 * @tc.name: Common_Event_Test_PublishA2dpPlayStateChangeEvent_100
 * @tc.desc: PublishA2dpPlayStateChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(CommonEventTest, Common_Event_Test_PublishA2dpPlayStateChangeEvent_100, TestSize.Level1)
{
    HILOGI("Common_Event_Test_PublishA2dpPlayStateChangeEvent_100 start");
    std::string device = "00:00:00:00:00:00";
    RawAddress realAddr(device);
    RawAddress randomAddr;
    EXPECT_TRUE(AdapterManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr));

    int a2dpState = 1;
    BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpPlayStateChangeEvent(device, a2dpState);
    HILOGI("Common_Event_Test_PublishA2dpPlayStateChangeEvent_100 end");
}
}  // namespace bluetooth
}  // namespace OHOS