/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd. All rights reserved.
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

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bt_chr_business_event.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
static BluetoothHost *host_;
class BtChrDftBusinessEventTest : public testing::Test {
public:
    BtChrDftBusinessEventTest() = default;
    ~BtChrDftBusinessEventTest() = default;

    static void SetUpTestCase()
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
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Test_GetSetOffloadFlag_001
 * @tc.desc: Verify the GetSetOffloadFlag function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftBusinessEventTest, Test_GetSetOffloadFlag_001, TestSize.Level1)
{
    HILOGI("Test_GetSetOffloadFlag_001 enter");
    BtChrBusinessEvent::GetInstance().SetOffloadFlag(0);
    int offloadFlag = BtChrBusinessEvent::GetInstance().GetOffloadFlag();
    EXPECT_EQ(0, offloadFlag);
    HILOGI("Test_GetSetOffloadFlag_001 end");
}

/**
 * @tc.name: Test_GetSetVolumeParams_001
 * @tc.desc: Verify the GetSetVolumeParams function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftBusinessEventTest, Test_GetSetVolumeParams_001, TestSize.Level1)
{
    HILOGI("Test_GetSetVolumeParams_001 enter");
    BtChrBusinessEvent::GetInstance().SetVolumeParams(0, 0);
    int volumeType = BtChrBusinessEvent::GetInstance().GetVolumeType();
    int volumeValue = BtChrBusinessEvent::GetInstance().GetVolumeValue();
    EXPECT_EQ(0, volumeType);
    EXPECT_EQ(0, volumeValue);
    HILOGI("Test_GetSetVolumeParams_001 end");
}

/**
 * @tc.name: Test_GetSetAbsolutVolumeParams_001
 * @tc.desc: Verify the GetSetAbsolutVolumeParams function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftBusinessEventTest, Test_GetSetAbsolutVolumeParams_001, TestSize.Level1)
{
    HILOGI("Test_GetSetAbsolutVolumeParams_001 enter");
    BtChrBusinessEvent::GetInstance().SetAbsolutVolumeParams(0, 0);
    int isAbsolutVolume = BtChrBusinessEvent::GetInstance().IsAbsolutVolume();
    int absolutVolumeStatus = BtChrBusinessEvent::GetInstance().GetAbsolutVolumeStatus();
    EXPECT_EQ(0, isAbsolutVolume);
    EXPECT_EQ(0, absolutVolumeStatus);
    HILOGI("Test_GetSetAbsolutVolumeParams_001 end");
}

/**
 * @tc.name: Test_GetSetAudioDeviceParams_001
 * @tc.desc: Verify the GetSetAudioDeviceParams function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftBusinessEventTest, Test_GetSetAudioDeviceParams_001, TestSize.Level1)
{
    HILOGI("Test_GetSetAudioDeviceParams_001 enter");
    std::string pkg = "bluetooth";
    BtChrBusinessEvent::GetInstance().SetAudioDeviceParams(pkg, 0);
    std::string audioDevicePkg = BtChrBusinessEvent::GetInstance().GetAudioDevicePkg();
    int activeDeviceResult = BtChrBusinessEvent::GetInstance().GetActiveDeviceResult();
    EXPECT_EQ(pkg, audioDevicePkg);
    EXPECT_EQ(0, activeDeviceResult);
    HILOGI("Test_GetSetAudioDeviceParams_001 end");
}

/**
 * @tc.name: Test_GetSetAudioStartTime_001
 * @tc.desc: Verify the GetSetAudioStartTime function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftBusinessEventTest, Test_SetOffloadFlag_001 , TestSize.Level1)
{
    HILOGI("Test_GetSetAudioStartTime_001 enter");
    std::string time = "00";
    BtChrBusinessEvent::GetInstance().SetAudioStartTime("00");
    std::string ret = BtChrBusinessEvent::GetInstance().GetAudioStartTime();
    EXPECT_EQ(time, ret);
    HILOGI("Test_GetSetAudioStartTime_001 end");
}

/**
 * @tc.name: Test_BtChrBusinessEventHandler_001
 * @tc.desc: Verify the BtChrBusinessEventHandler function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftBusinessEventTest, Test_BtChrBusinessEventHandler_001 , TestSize.Level1)
{
    HILOGI("Test_BtChrBusinessEventHandler_001 enter");
    BtChrBusinessEvent btChrBusinessEvent;
    std::string peerAddr = "74:0a:12:d4:56:d5";
    bool ret = BtChrBusinessEventHandler(CHR_A2DP_CONNECT_EXCEPTION, peerAddr, "ERRCODE", 1);
    g_btChrBusinessEventMap.Find(peerAddr, btChrBusinessEvent);
    EXPECT_EQ(btChrBusinessEvent.a2dpErrorReason, 1);
    EXPECT_TRUE(ret);

    ret = BtChrBusinessEventHandler(CHR_HID_EXCEPTION, peerAddr, "ERRCODE", 1);
    g_btChrBusinessEventMap.Find(peerAddr, btChrBusinessEvent);
    EXPECT_EQ(btChrBusinessEvent.hidErrorReason, 1);
    EXPECT_TRUE(ret);

    ret = BtChrBusinessEventHandler(CHR_HFP_RFCOMM_EXCEPTION, peerAddr, "ERRCODE", 1);
    g_btChrBusinessEventMap.Find(peerAddr, btChrBusinessEvent);
    EXPECT_EQ(btChrBusinessEvent.hfpErrorReason, 1);
    EXPECT_TRUE(ret);

    ret = BtChrBusinessEventHandler(CHR_BLE_DISCONNECT, peerAddr, "ISPASSIVE", 1);
    g_btChrBusinessEventMap.Find(peerAddr, btChrBusinessEvent);
    EXPECT_EQ(btChrBusinessEvent.isPassive, 1);
    EXPECT_TRUE(ret);

    ret = BtChrBusinessEventHandler(CHR_BT_WATCH_CONNECT, peerAddr, "DISCONNECTREASON", 8);
    g_btChrBusinessEventMap.Find(peerAddr, btChrBusinessEvent);
    EXPECT_EQ(btChrBusinessEvent.disconnectReason, 1);
    EXPECT_TRUE(ret);

    ret = BtChrBusinessEventHandler(CHR_BT_WATCH_CONNECT, peerAddr, "ACLCONNECTIONTIME", "2025-01-18");
    EXPECT_FALSE(ret);

    ret = BtChrBusinessEventHandler(CHR_BT_WATCH_REMOVE_PAIR, peerAddr, "REMOVEPAIRRESULT", 1);
    EXPECT_TRUE(ret);

    ret = BtChrBusinessEventHandler(CHR_BT_WATCH_REMOVE_PAIR, peerAddr, "DELETEKEYRESULT", 1);
    EXPECT_FALSE(ret);

    HILOGI("Test_BtChrBusinessEventHandler_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS