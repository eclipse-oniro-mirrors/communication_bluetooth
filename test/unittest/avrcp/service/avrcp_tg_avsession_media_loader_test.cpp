/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <chrono>
#include <thread>

#include "avrc_defs.h"
#include "avrcp_tg_avsession_media_loader.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"
#include "hfp_ag_system_interface.h"
#include "refuse_play_helper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
static BluetoothHost *host_;

class AvrcpTgAvsessionMediaLoaderTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
        HILOGI("SetUpTestCase end");
        auto manager = ProfileServiceManager::GetInstance();
        string name = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, name);
    }
    static void TearDownTestCase()
    {}
    void SetUp();
    void TearDown();

private:
    std::shared_ptr<AvrcpTgAvsessionMediaLoader> mediaLoader_ = nullptr;
};

void AvrcpTgAvsessionMediaLoaderTest::SetUp()
{
    mediaLoader_ = std::make_shared<AvrcpTgAvsessionMediaLoader>();
}

void AvrcpTgAvsessionMediaLoaderTest::TearDown()
{
    mediaLoader_ = nullptr;
}

void MockSongInfoCallback(SongInfo info)
{}
void MockPlayingStatusCallback(PlayStatus status)
{}
void MockNowPlayingCallback(std::string str, std::vector<SongInfo> info)
{}
void MockMediaListCallback(uint16_t curPlayer, std::vector<MediaPlayerInfo> info)
{}
void MockFolderItemsCallback(std::vector<ListItem> item)
{}
void MockSetBrowsedPlayerCallback(bool success, std::string rootId, uint32_t numItems)
{}
void MockAppSettingsCallback(uint8_t shuffleMode, uint8_t repeatMode)
{}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_001
 * @tc.desc: LoadSuccess, call function
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_001 enter");
    bool isPushed = true;
    mediaLoader_->NeedDropPlayAfterScoEnd(isPushed);
    mediaLoader_->NeedDropPause(isPushed);
    uint8_t shuffleMode = 1;
    uint8_t repeatMode = 1;
    mediaLoader_->SetAppSettingsMode(shuffleMode, repeatMode);
    mediaLoader_->IsLibraryLoaded();
    std::string addr("11:22:33:44:55:66");
    STACK::RawAddress rawAddr;
    STACK::RawAddress::FromString(addr, rawAddr);
    mediaLoader_->SetActiveDevice(rawAddr);
    uint16_t playerId = 1;
    bool nowPlaying = true;
    std::string mediaId = "testMediaId";
    mediaLoader_->PlayItem(playerId, nowPlaying, mediaId);
    EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_001 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_010
 * @tc.desc: PlaySilenceSound
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_010, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_010 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:54:AF:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        deviceInfo->SetPlayingState(A2DP_NOT_PLAYING);
        mediaLoader_->PlaySilenceSound();

        deviceInfo->SetPlayingState(A2DP_IS_PLAYING);
        mediaLoader_->PlaySilenceSound();
    }
        EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_010 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_ProcessPushedToPlayHelper_001
 * @tc.desc: ProcessPushedToPlayHelper
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_ProcessPushedToPlayHelper_001,
    TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_ProcessPushedToPlayHelper_001 enter");
    bool isPushed = true;
    mediaLoader_->ProcessPushedToPlayHelper(isPushed);
    EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_ProcessPushedToPlayHelper_001 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_SavePauseTimeToHelper_001
 * @tc.desc: SavePauseTimeToHelper
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_SavePauseTimeToHelper_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_SavePauseTimeToHelper_001 enter");
    mediaLoader_->SavePauseTimeToHelper();
    EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_SavePauseTimeToHelper_001 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_NeedDropPause_001
 * @tc.desc: NeedDropPause
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_NeedDropPause_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_NeedDropPause_001 enter");
    bool isPushed = true;
    EXPECT_FALSE(mediaLoader_->NeedDropPause(isPushed));
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_NeedDropPause_001 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_01
 * @tc.desc: IsNeededDropKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_01, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_01 enter");
    bool ret = true;
    ret = mediaLoader_->IsNeededDropKeyEvent(AVRC_ID_REWIND, KeyState::PUSHED);
    EXPECT_EQ(ret, false);
    ret = mediaLoader_->IsNeededDropKeyEvent(AVRC_ID_FAST_FOR, KeyState::PUSHED);
    EXPECT_EQ(ret, false);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_01 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_02
 * @tc.desc: IsNeededDropKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_02, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_02 enter");
    bool ret = false;
    HfpAgSystemInterface::GetInstance().SetActiveCallNumber(1);
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_INCOMING);
    ret = mediaLoader_->IsNeededDropKeyEvent(AVRC_ID_PLAY, KeyState::PUSHED);
    EXPECT_EQ(ret, true);
    HfpAgSystemInterface::GetInstance().SetActiveCallNumber(0);
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_IDLE);
    ret = mediaLoader_->IsNeededDropKeyEvent(AVRC_ID_PLAY, KeyState::PUSHED);
    EXPECT_EQ(ret, false);
    HfpAgSystemInterface::GetInstance().SetActiveCallNumber(1);
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_INCOMING);
    ret = mediaLoader_->IsNeededDropKeyEvent(AVRC_ID_PLAY, KeyState::PUSHED);
    EXPECT_EQ(ret, true);
    HfpAgSystemInterface::GetInstance().SetActiveCallNumber(0);
    HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_IDLE);
    ret = mediaLoader_->IsNeededDropKeyEvent(AVRC_ID_PAUSE, KeyState::PUSHED);
    EXPECT_EQ(ret, false);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_02 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_03
 * @tc.desc: IsNeededDropKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_03, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_03 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:54:AF:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        HfpAgSystemInterface::GetInstance().SetActiveCallNumber(1);
        HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_INCOMING);
        EXPECT_TRUE(mediaLoader_->IsNeededDropKeyEvent(AVRC_ID_PLAY, KeyState::PUSHED));
        HfpAgSystemInterface::GetInstance().SetActiveCallNumber(0);
        HfpAgSystemInterface::GetInstance().SetCallState(HFP_AG_CALL_STATE_IDLE);
        EXPECT_TRUE(mediaLoader_->IsNeededDropKeyEvent(AVRC_ID_PAUSE, KeyState::PUSHED));
    }
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_IsNeededDropKeyEvent_03 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_ReportDropPlayPauseChr_001
 * @tc.desc: ReportDropPlayPauseChr
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_ReportDropPlayPauseChr_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_ReportDropPlayPauseChr_001 enter");
    mediaLoader_->ReportDropPlayPauseChr(AVRC_ID_PLAY);
    mediaLoader_->ReportDropPlayPauseChr(AVRC_ID_PAUSE);
    mediaLoader_->ReportDropPlayPauseChr(AVRC_ID_STOP);
    EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    mediaLoader_->loader_.OnDynamicLibraryClosed();
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_ReportDropPlayPauseChr_001 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_SendKeyEvent_01
 * @tc.desc: SendKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_SendKeyEvent_01, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_SendKeyEvent_01 enter");
    mediaLoader_->SendKeyEvent(AVRC_ID_PLAY, KeyState::PUSHED);
    mediaLoader_->SendKeyEvent(AVRC_ID_PLAY, KeyState::RELEASED);
    mediaLoader_->SendKeyEvent(AVRC_ID_PAUSE, KeyState::RELEASED);
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    mediaLoader_->SetActiveDevice(addr);
    mediaLoader_->SendKeyEvent(AVRC_ID_PLAY, KeyState::PUSHED);
    mediaLoader_->SendKeyEventByWearDetection(AVRC_ID_PLAY, KeyState::PUSHED);
    mediaLoader_->SendCtrlCommandToAvSession(AVRC_ID_PLAY);
    EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_SendKeyEvent_01 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_NotifyA2dpPlayStateChange_001
 * @tc.desc: NotifyA2dpPlayStateChange
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_NotifyA2dpPlayStateChange_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_NotifyA2dpPlayStateChange_001 enter");
    mediaLoader_->NotifyA2dpPlayStateChange();
    EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_NotifyA2dpPlayStateChange_001 end");
}

#ifdef MULTI_A2DP_ENABLED
/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_SendKeyEventByAddr_001
 * @tc.desc: Per-address multi A2DP entries: key event, active device, wear
 *           detection, play state notify and control command (lib not loaded)
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_SendKeyEventByAddr_001, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_SendKeyEventByAddr_001 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    mediaLoader_->SendKeyEventByAddr(AVRC_ID_PLAY, KeyState::RELEASED, addr);
    mediaLoader_->SendKeyEventByAddr(AVRC_ID_PLAY, KeyState::PUSHED, addr);
    mediaLoader_->SendKeyEventByAddr(AVRC_ID_PAUSE, KeyState::RELEASED, addr);
    mediaLoader_->AddActiveDevice(addr);
    mediaLoader_->RemoveActiveDevice(addr);
    mediaLoader_->SendKeyEventByWearDetectionByAddr(AVRC_ID_PLAY, KeyState::RELEASED, addr);
    mediaLoader_->NotifyA2dpPlayStateChangeByAddr(addr);
    mediaLoader_->PlaySilenceSoundByAddr(addr);
    EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_SendKeyEventByAddr_001 end");
}

/**
 * @tc.name: AvrcpTgAvsessionMediaLoaderTest_SendKeyEventByAddr_002
 * @tc.desc: key event: refuse auto play after connect branch
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgAvsessionMediaLoaderTest, AvrcpTgAvsessionMediaLoaderTest_SendKeyEventByAddr_002, TestSize.Level1)
{
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_SendKeyEventByAddr_002 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    RefusePlayHelper::GetInstance()->SetLastAvrcpConnectTime(addr.ToString());
    EXPECT_TRUE(RefusePlayHelper::GetInstance()->IsRefusePlayAfterConnect(addr.ToString()));
    mediaLoader_->SendKeyEventByAddr(AVRC_ID_PLAY, KeyState::RELEASED, addr);
    EXPECT_EQ(mediaLoader_->mediaCallbacks_, nullptr);
    RefusePlayHelper::GetInstance()->ClearRecord(addr.ToString());
    HILOGI("AvrcpTgAvsessionMediaLoaderTest_SendKeyEventByAddr_002 end");
}
#endif

}  // namespace bluetooth
}  // namespace OHOS
