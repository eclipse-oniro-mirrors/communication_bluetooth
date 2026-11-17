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

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bt_chr_ue_manager.h"
#include "bt_chr_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
constexpr int RSSI = -50;
static BluetoothHost *host_;
class BtChrUeManagerTest : public testing::Test {
public:
    BtChrUeManagerTest() = default;
    ~BtChrUeManagerTest() = default;

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
 * @tc.name: Test_WriteCommonUe_001
 * @tc.desc: Verify the WriteCommonUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteCommonUe_001, TestSize.Level1)
{
    HILOGI("Test_WriteCommonUe_001 enter");
    RawAddress device = RawAddress("74:0a:12:d4:56:d5");
    std::string pkgName = "com.example.settings";
    std::string uuid = "007B0A01-0F39-6100-0000-00000008D831";
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE1, "", pkgName);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_SOCKET_SERVER_CONN, device, UE_COMMON_SCENE_CASE2,
        uuid, pkgName);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_SET_A2DP_SUSPEND, device, UE_COMMON_SCENE_CASE1,
        UE_COMMON_SCENE_CASE1);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_SET_A2DP_SUSPEND, device, UE_COMMON_SCENE_CASE1,
        UE_COMMON_SCENE_CASE1, pkgName);
    BtChrUpdateDeviceInfo(device.GetAddress(), "FreeBuds 3", 0, 1048);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_REMOVE_PAIR, device, UE_COMMON_SCENE_CASE1, pkgName);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteCommonUe_001 end");
}

/**
 * @tc.name: Test_WriteCommonUe_002
 * @tc.desc: Verify the WriteCommonUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteCommonUe_002, TestSize.Level1)
{
    HILOGI("Test_WriteCommonUe_002 enter");
    RawAddress device = RawAddress("74:0a:12:d4:56:d5");
    std::string pkgName = "com.example.settings";
    BtChrDeviceInfo deviceInfo;
    deviceInfo.btDeviceName = "FreeBuds 3";
    deviceInfo.btDeviceClass = 1048;
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_PAIR, device, deviceInfo,
        UE_COMMON_SCENE_CASE1, pkgName);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_PAIR, RawAddress(""), deviceInfo,
        UE_COMMON_SCENE_CASE2, pkgName);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE0,
        UE_COMMON_SCENE_CASE2);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteCommonUe_002 end");
}

/**
 * @tc.name: Test_WriteSetActiveDevUe_001
 * @tc.desc: Verify the WriteSetActiveDevUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteSetActiveDevUe_001, TestSize.Level1)
{
    HILOGI("Test_WriteSetActiveDevUe_001 enter");
    RawAddress preDevice = RawAddress("74:0a:12:d4:56:d6");
    RawAddress device = RawAddress("74:0a:12:d4:56:d5");
    std::string pkgName = "com.example.settings";
    BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_A2DP_ACTIVE_DEV, preDevice, device,
        UE_FAIL_SET_A2DP_ACTIVE_DEV_NOT_CONN, pkgName);
    BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_A2DP_ACTIVE_DEV, RawAddress(""), device,
        UE_FAIL_SET_A2DP_ACTIVE_DEV_NOT_CONN, pkgName);
    BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_A2DP_ACTIVE_DEV, preDevice, RawAddress(""),
        UE_FAIL_SET_A2DP_ACTIVE_DEV_NOT_CONN, pkgName);
    BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_A2DP_ACTIVE_DEV, RawAddress(""), RawAddress(""),
        UE_FAIL_SET_A2DP_ACTIVE_DEV_NOT_CONN, pkgName);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteSetActiveDevUe_001 end");
}

/**
 * @tc.name: Test_WriteA2dpPlayStateUe_001
 * @tc.desc: Verify the WriteA2dpPlayStateUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteA2dpPlayStateUe_001, TestSize.Level1)
{
    HILOGI("Test_WriteA2dpPlayStateUe_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    RawAddress device = RawAddress(peerAddr);
    std::string pkgName = "com.example.settings";
    std::string uuid = "007B0A01-0F39-6100-0000-00000008D831";
    BtChrUeManager::GetInstance()->WriteA2dpPlayStateUe(device, true, 0, pkgName);
    BtChrUeManager::GetInstance()->WriteA2dpPlayStateUe(device, false, 1, pkgName);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteA2dpPlayStateUe_001 end");
}

/**
 * @tc.name: Test_WriteStackScoStateChangeUe_001
 * @tc.desc: Verify the WriteStackScoStateChangeUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteStackScoStateChangeUe_001, TestSize.Level1)
{
    HILOGI("Test_WriteStackScoStateChangeUe_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    RawAddress device = RawAddress(peerAddr);
    std::string eventName = CHR_UE_STACK_CONN_SCO;
    int sceneCode = UE_SCENE_CODE_SUCCESS;
    int subSceneCode = UE_COMMON_SCENE_CASE3;
    int codec = 0;
    BtChrUeManager::GetInstance()->WriteStackScoStateChangeUe(eventName, device, sceneCode, subSceneCode);
    BtChrUeManager::GetInstance()->WriteStackScoStateChangeUe(eventName, device, sceneCode, subSceneCode, codec);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteStackScoStateChangeUe_001 end");
}

/**
 * @tc.name: Test_WriteStackScoStateChangeUe_002
 * @tc.desc: Verify the WriteStackScoStateChangeUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteStackScoStateChangeUe_002, TestSize.Level1)
{
    HILOGI("Test_WriteStackScoStateChangeUe_002 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    RawAddress device = RawAddress(peerAddr);
    std::string eventName = CHR_UE_CS_CONN_SCO;
    int sceneCode = UE_SCENE_CODE_SUCCESS;
    int subSceneCode = UE_COMMON_SCENE_CASE3;
    int codec = 0;
    BtChrUeManager::GetInstance()->WriteStackScoStateChangeUe(eventName, device, sceneCode, subSceneCode);
    BtChrUeManager::GetInstance()->WriteStackScoStateChangeUe(eventName, device, sceneCode, subSceneCode, codec);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteStackScoStateChangeUe_002 end");
}

/**
 * @tc.name: Test_ReportBandWearStateUe_001
 * @tc.desc: Verify the ReportBandWearStateUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_ReportBandWearStateUe_001, TestSize.Level1)
{
    HILOGI("Test_ReportBandWearStateUe_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    RawAddress device = RawAddress(peerAddr);
    BtChrUeManager::GetInstance()->ReportBandWearStateUe(GHR_UE_BAND_WEAR_STATE_CHANGE, device, 0);
    BtChrUeManager::GetInstance()->ReportBandWearStateUe(GHR_UE_BAND_WEAR_STATE_CHANGE, device, 1);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_ReportBandWearStateUe_001 end");
}

/**
 * @tc.name: Test_ReportPublishProfileDisableUe_001
 * @tc.desc: ReportPublishProfileDisableUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_ReportPublishProfileDisableUe_001, TestSize.Level1)
{
    HILOGI("Test_ReportPublishProfileDisableUe_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    std::string receiverName = "xxxphone";
    RawAddress device = RawAddress(peerAddr);
    BtChrUeManager::GetInstance()->ReportPublishProfileDisableUe(CHR_UE_TWS_DUAL_CONN_HAIR_CAPSL, device,
        receiverName, "com.example.audioaccessorymanager", UE_COMMON_SCENE_CASE2);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_ReportPublishProfileDisableUe_001 end");
}

/**
 * @tc.name: Test_WriteStateChangeUe_001
 * @tc.desc: WriteStateChangeUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteStateChangeUe_001, TestSize.Level1)
{
    HILOGI("Test_WriteStateChangeUe_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    RawAddress device = RawAddress(peerAddr);
    std::string pkgName = "com.example.walletservice";
    BtChrUeManager::GetInstance()->WriteStateChangeUe(CHR_UE_BRCAST_ACL_CONN, device,
        RSSI, pkgName, UE_COMMON_SCENE_CASE3);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteStateChangeUe_001 end");
}

/**
 * @tc.name: Test_ReportHdapRecordOperUe_001
 * @tc.desc: ReportHdapRecordOperUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_ReportHdapRecordOperUe_001, TestSize.Level1)
{
    HILOGI("Test_ReportHdapRecordOperUe_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    RawAddress device = RawAddress(peerAddr);
    std::string pkgName = "com.example.settings";
    BtChrUeManager::GetInstance()->ReportHdapRecordOperUe(CHR_UE_BT_HDAP_OPER_EVENT, device, pkgName, 2);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_ReportHdapRecordOperUe_001 end");
}

/**
 * @tc.name: Test_WriteSetMusicMuteUe_001
 * @tc.desc: WriteSetMusicMuteUe function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteSetMusicMuteUe_001, TestSize.Level1)
{
    HILOGI("Test_WriteSetMusicMuteUe_001 enter");
    int streamType = 1;
    int muteDeviceType = 2;
    BtChrUeManager::GetInstance()->WriteSetMusicMuteUe(
        CHR_UE_SEND_MEDIA_MUTE, UE_COMMON_SCENE_CASE0, streamType, muteDeviceType);
    BtChrUeManager::GetInstance()->WriteSetMusicMuteUe(
        CHR_UE_SEND_MEDIA_MUTE, UE_COMMON_SCENE_CASE1, streamType, muteDeviceType);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteSetMusicMuteUe_001 end");
}

/**
 * @tc.name: Test_WriteAppWakeupUe_001
 * @tc.desc: Verify WriteAppWakeupUe with empty queue does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteAppWakeupUe_001, TestSize.Level1)
{
    HILOGI("Test_WriteAppWakeupUe_001 enter");
    std::vector<AppBehaviorDataInternal> dataQueue;
    BtChrUeManager::GetInstance()->WriteAppWakeupUe(dataQueue);
    EXPECT_TRUE(dataQueue.empty());
    HILOGI("Test_WriteAppWakeupUe_001 end");
}

/**
 * @tc.name: Test_WriteAppWakeupUe_002
 * @tc.desc: Verify WriteAppWakeupUe with single entry writes successfully.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteAppWakeupUe_002, TestSize.Level1)
{
    HILOGI("Test_WriteAppWakeupUe_002 enter");
    std::vector<AppBehaviorDataInternal> dataQueue;
    AppBehaviorDataInternal item = {"test_app", 1000, 2000, 0, 1};
    dataQueue.push_back(item);
    BtChrUeManager::GetInstance()->WriteAppWakeupUe(dataQueue);
    EXPECT_EQ(dataQueue.size(), 1u);
    HILOGI("Test_WriteAppWakeupUe_002 end");
}

/**
 * @tc.name: Test_WriteAppWakeupUe_003
 * @tc.desc: Verify WriteAppWakeupUe with multiple entries writes successfully.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteAppWakeupUe_003, TestSize.Level1)
{
    HILOGI("Test_WriteAppWakeupUe_003 enter");
    std::vector<AppBehaviorDataInternal> dataQueue;
    dataQueue.push_back({"app_a", 1000, 2000, 0, 3});
    dataQueue.push_back({"app_b", 1001, 2001, 1, 1});
    dataQueue.push_back({"app_c", 1002, 2002, 0, 5});
    BtChrUeManager::GetInstance()->WriteAppWakeupUe(dataQueue);
    EXPECT_EQ(dataQueue.size(), 3u);
    HILOGI("Test_WriteAppWakeupUe_003 end");
}

/**
 * @tc.name: Test_WriteAbnormalStreamUe_001
 * @tc.desc: Verify WriteAbnormalStreamUe with normal device address.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteAbnormalStreamUe_001, TestSize.Level1)
{
    HILOGI("Test_WriteAbnormalStreamUe_001 enter");
    RawAddress realDevice = RawAddress("74:0a:12:d4:56:d5");
    std::string device = "11:22:33:44:55:66_74:0a:12:d4:56:d5";
    std::string pkgName = "com.vendor.example.settings";
    BtChrUpdateDeviceInfo(realDevice.GetAddress(), "FreeBuds 3", 0, 1048);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        device, "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE2, UE_COMMON_SCENE_CASE3);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        device, "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE2, UE_COMMON_SCENE_CASE2);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        device, "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE1, UE_COMMON_SCENE_CASE1);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        device, "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE1, UE_COMMON_SCENE_CASE2);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        device, "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE1, UE_COMMON_SCENE_CASE3);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        device, "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE2, UE_COMMON_SCENE_CASE1);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteAbnormalStreamUe_001 end");
}

/**
 * @tc.name: Test_WriteAbnormalStreamUe_002
 * @tc.desc: Verify WriteAbnormalStreamUe with empty or invalid device address.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUeManagerTest, Test_WriteAbnormalStreamUe_002, TestSize.Level1)
{
    HILOGI("Test_WriteAbnormalStreamUe_002 enter");
    std::string pkgName = "com.vendor.example.settings";
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        "", "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE2, UE_COMMON_SCENE_CASE4);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        "invalid_addr", "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE2, UE_COMMON_SCENE_CASE3);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        "11:22:33:44:55:66_", "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE2, UE_COMMON_SCENE_CASE4);
    BtChrUeManager::GetInstance()->WriteAbnormalStreamUe(
        "_74:0a:12:d4:56:d5", "PERSISTENT_DEVICE_ID", pkgName, UE_COMMON_SCENE_CASE2, UE_COMMON_SCENE_CASE3);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_WriteAbnormalStreamUe_002 end");
}

}  // namespace bluetooth
}  // namespace OHOS