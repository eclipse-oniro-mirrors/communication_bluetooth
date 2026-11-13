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

#include <gtest/gtest.h>
#include <cstring>
#include "bluetooth_log.h"
#include "bluetooth_host.h"
#include "bluetooth_audio_framework_adapter.h"
#include "hfp_ag_defines.h"
#include "profile_service_manager.h"
#include "bluetooth_hw_interface.h"
#include "a2dp_def.h"
#include "a2dp_service.h"
#include "a2dp_src_service.h"
#include "a2dp_service_device.h"
#include "thread_util.h"
#include "class_creator.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

extern "C" {
static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}
 
void HwSetSpatialAudioModeEnabledMock(const bool enabled)
{
    HILOGI("HwSetSpatialAudioModeEnabledMock enter, enabled=%{public}d", enabled);
}
 
void HwSetSpatialAudioHeadTrackingEnabledMock(const bool enabled)
{
    HILOGI("HwSetSpatialAudioHeadTrackingEnabledMock enter, enabled=%{public}d", enabled);
}

static void HwSetLocalVoiceCombineFeatureStateMock(const int featureState)
{
    HILOGI("HwSetLocalVoiceCombineFeatureStateMock enter, featureState=%{public}d", featureState);
}

static void HwSetLocalVoiceCombineNnStateMock(const int nnState)
{
    HILOGI("HwSetLocalVoiceCombineNnStateMock enter, nnState=%{public}d", nnState);
}

#ifdef BLUETOOTH_AUDIO_COLLABORATION_SUPPORT
static void HwSetCollAudioEnableStateMock(const bool enabled)
{
    HILOGI("HwSetCollAudioEnableStateMock enter, enabled=%{public}d", enabled);
}
#endif

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwSetSpatialAudioModeEnabled = HwSetSpatialAudioModeEnabledMock,
    .hwSetSpatialAudioHeadTrackingEnabled = HwSetSpatialAudioHeadTrackingEnabledMock,
    .hwSetLocalVoiceCombineFeatureState = HwSetLocalVoiceCombineFeatureStateMock,
    .hwSetLocalVoiceCombineNnState = HwSetLocalVoiceCombineNnStateMock,
#ifdef BLUETOOTH_AUDIO_COLLABORATION_SUPPORT
    .HwSetCollAudioEnableState = HwSetCollAudioEnableStateMock,
#endif
};
} // extern "C"

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;

void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

class BluetoothAudioFrameworkAdapterTest : public testing::Test {
public:
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
        ClassFactory::RegisterClass("A2dpSrcService", A2dpSrcServiceCreate);
        ProfileServiceManager::GetInstance()->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR,
            "A2dpSrcService");
        BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioVolumeEvent();
        BluetoothAudioFrameworkAdapter::GetInstance().RegisterSpatialAudioListener();
#ifdef BLUETOOTH_AUDIO_COLLABORATION_SUPPORT
        BluetoothAudioFrameworkAdapter::GetInstance().RegisterCollaborativeAudioListener();
#endif
        BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioPreferredOutPutDeviceChangeListener();
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp()
    {
        bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        auto manager = ProfileServiceManager::GetInstance();
        string name = "A2dpSrcService";
        manager->SetProfileService(BTTransport::ADAPTER_BREDR, name);
        // Disable a2dp thread
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_A2DP, ThreadUtil::DISABLED);
    }
    void TearDown()
    {
        BluetoothHwInterface::GetInstance()->SetBtHwInterface(bluetoothHwSrcInterface);
        auto manager = ProfileServiceManager::GetInstance();
        A2dpSrcService *src = static_cast<A2dpSrcService *>(manager->GetProfileService("A2dpSrcService"));
        delete src;
    }

    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> GetAudioDeviceDescriptor(const std::string &macAddr)
    {
        std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev =
            std::make_shared<AudioStandard::AudioDeviceDescriptor>();
        if (audioDev != nullptr) {
            audioDev->macAddress_ = macAddr;
        }
        return audioDev;
    }

    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> GetAudioDeviceDescriptor()
    {
        RawAddress device = RawAddress("00:00:00:00:00:00");
        return GetAudioDeviceDescriptor(device.GetAddress());
    }

    void InitA2dpServiceAndSetActiveDevice(const RawAddress& bdAddr)
    {
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(bdAddr);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(bdAddr.GetAddress().c_str(), deviceInfo);
            a2dpService->UpdateActiveDevice(bdAddr);
        }
    }
private:
    const bthwif_interface_t *bluetoothHwSrcInterface = nullptr;
};

/*
 * @tc.number:audio_event_manager_test_001
 * @tc.name: OnVolumeKeyEvent
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_001 enter");
    if (BluetoothAudioFrameworkAdapter::GetInstance().volumeCallback_ != nullptr) {
        AudioStandard::VolumeEvent volumeEvent;
        volumeEvent.volumeType = AudioStandard::AudioStreamType::STREAM_MUSIC;
        volumeEvent.volume = 1;
        BluetoothAudioFrameworkAdapter::GetInstance().volumeCallback_->OnVolumeKeyEvent(volumeEvent);
    }
    HILOGI("audio_event_manager_test_001 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_002
 * @tc.name: OnVolumeKeyEvent
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_002, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_002 enter");
    ProfileServiceManager::GetInstance()->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, "AvrcpTgService");
    ProfileServiceManager::GetInstance()->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, "HfpAgService");
    if (BluetoothAudioFrameworkAdapter::GetInstance().volumeCallback_ != nullptr) {
        AudioStandard::VolumeEvent volumeEvent;
        volumeEvent.volumeType = AudioStandard::AudioStreamType::STREAM_MUSIC;
        volumeEvent.volume = 1;
        BluetoothAudioFrameworkAdapter::GetInstance().volumeCallback_->OnVolumeKeyEvent(volumeEvent);

        volumeEvent.volumeType = AudioStandard::AudioStreamType::STREAM_VOICE_CALL;
        volumeEvent.volume = 1;
        BluetoothAudioFrameworkAdapter::GetInstance().volumeCallback_->OnVolumeKeyEvent(volumeEvent);

        volumeEvent.volumeType = AudioStandard::AudioStreamType::STREAM_BLUETOOTH_SCO;
        volumeEvent.volume = 1;
        BluetoothAudioFrameworkAdapter::GetInstance().volumeCallback_->OnVolumeKeyEvent(volumeEvent);
    }
    HILOGI("audio_event_manager_test_002 end");
#endif
}

/*
 * @tc.number:set_stream_volume_test_001
 * @tc.name: SetStreamVolume
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, set_stream_volume_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("set_stream_volume_test_001 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().SetStreamVolume(STREAM_MUSIC, 7, 1);
    BluetoothAudioFrameworkAdapter::GetInstance().SetStreamVolume(STREAM_VOICE_CALL, 8, 1);
    BluetoothAudioFrameworkAdapter::GetInstance().SetStreamVolume(STREAM_DEFAULT, 8, 1);
    HILOGI("set_stream_volume_test_001 end");
#endif
}

/*
 * @tc.number:get_max_volume_test_001
 * @tc.name: BtGetMaxVolume
 * @tc.desc:
 */
 HWTEST_F(BluetoothAudioFrameworkAdapterTest, get_max_volume_test_001, TestSize.Level1)
 {
 #ifdef AUDIO_FRAMEWORK
     HILOGI("get_max_volume_test_001 enter");
     EXPECT_EQ(BluetoothAudioFrameworkAdapter::GetInstance().BtGetMaxVolume(BtVolumeType::VOLUME_MUSIC,
         BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP), 15);
     EXPECT_EQ(BluetoothAudioFrameworkAdapter::GetInstance().BtGetMaxVolume(BtVolumeType::VOLUME_VOICE_CALL,
         BtDeviceType::DEVICE_TYPE_BLUETOOTH_SCO), 15);
     EXPECT_NE(BluetoothAudioFrameworkAdapter::GetInstance().BtGetMaxVolume(BtVolumeType::VOLUME_DEFAULT,
        BtDeviceType::DEVICE_TYPE_NONE), 15);
     HILOGI("get_max_volume_test_001 end");
 #endif
 }

/*
 * @tc.number:get_stream_volume_test_001
 * @tc.name: GetStreamVolume
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, get_stream_volume_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("get_stream_volume_test_001 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().SetStreamVolume(STREAM_MUSIC, 5, 1);
    int volume = BluetoothAudioFrameworkAdapter::GetInstance().GetStreamVolume(STREAM_MUSIC);
    HILOGI("volume = %{public}d,", volume);
    BluetoothAudioFrameworkAdapter::GetInstance().SetStreamVolume(STREAM_VOICE_CALL, 6, 1);
    volume = BluetoothAudioFrameworkAdapter::GetInstance().GetStreamVolume(STREAM_VOICE_CALL);
    HILOGI("volume = %{public}d,", volume);
    volume = BluetoothAudioFrameworkAdapter::GetInstance().GetStreamVolume(STREAM_DEFAULT);
    HILOGI("volume = %{public}d,", volume);
    HILOGI("get_stream_volume_test_001 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_003
 * @tc.name: RegisterOutputPipeChangeListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_003, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_003 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterOutputPipeChangeListener();
    HILOGI("audio_event_manager_test_003 end");
#endif
}
 
/*
 * @tc.number:audio_event_manager_test_005
 * @tc.name: OnOutputPipeChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_005, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_005 enter");
    AudioStandard::AudioPipeChangeType changeType = AudioStandard::PIPE_CHANGE_TYPE_PIPE_STREAM;
    std::shared_ptr<AudioStandard::AudioOutputPipeInfo> changedPipeInfo;
    uint32_t renderId = 1;
    AudioStandard::HdiAdapterType adapter = AudioStandard::HDI_ADAPTER_TYPE_A2DP;
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_FAST;
    changedPipeInfo = std::make_shared<AudioStandard::AudioOutputPipeInfo>(renderId, adapter, routeFlag);
    changedPipeInfo->SetStatus(AudioStandard::PIPE_STATUS_OPEN);
    changedPipeInfo->SetDevice(AudioStandard::DEVICE_TYPE_WIRED_HEADPHONES);
    changedPipeInfo->AddStream(1, AudioStandard::STREAM_USAGE_GAME,
        AudioStandard::RendererState::RENDERER_RUNNING, "name"); // streamId_、usage_、state_、bundleName_
    changedPipeInfo->AddStream(2, AudioStandard::STREAM_USAGE_MUSIC, AudioStandard::RendererState::RENDERER_RUNNING, "name");
    changedPipeInfo->AddStream(3, AudioStandard::STREAM_USAGE_NAVIGATION,
        AudioStandard::RendererState::RENDERER_RUNNING, "name");
    changedPipeInfo->AddStream(4, AudioStandard::STREAM_USAGE_MEDIA, AudioStandard::RendererState::RENDERER_PREPARED, "name");
 
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_->OnOutputPipeChange(
            changeType, changedPipeInfo);
    }
    HILOGI("audio_event_manager_test_005 end");
#endif
}

/*
 * @tc.number:preferred_output_device_updated_test_001
 * @tc.name: OnPreferredOutputDeviceUpdated
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, preferred_output_device_updated_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("preferred_output_device_updated_test_001 enter");
    std::vector<std::shared_ptr<AudioStandard::AudioDeviceDescriptor>> audioDeviceDescriptor;
 
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDeviceDescriptor1 =
        std::make_shared<AudioStandard::AudioDeviceDescriptor>();
    audioDeviceDescriptor1->macAddress_ = "A4:3B:0E:7B:D3:36";
    audioDeviceDescriptor1->deviceType_ = AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP;
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDeviceDescriptor2 =
        std::make_shared<AudioStandard::AudioDeviceDescriptor>();
    audioDeviceDescriptor2->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptor2->deviceType_ = AudioStandard::DEVICE_TYPE_SPEAKER;
 
    audioDeviceDescriptor.push_back(std::move(audioDeviceDescriptor1));
    audioDeviceDescriptor.push_back(std::move(audioDeviceDescriptor2));

    if (BluetoothAudioFrameworkAdapter::GetInstance().audioPreferredOutPutDeviceCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance()
            .audioPreferredOutPutDeviceCallback_->OnPreferredOutputDeviceUpdated(audioDeviceDescriptor);
        EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_  != nullptr);
        std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDeviceDescriptor3 =
        std::make_shared<AudioStandard::AudioDeviceDescriptor>();
        audioDeviceDescriptor3->macAddress_ = "00:00:00:00:00:00";
        audioDeviceDescriptor3->deviceType_ = AudioStandard::DEVICE_TYPE_SPEAKER;
        audioDeviceDescriptor.insert(audioDeviceDescriptor.begin(), std::move(audioDeviceDescriptor3));
        BluetoothAudioFrameworkAdapter::GetInstance()
            .audioPreferredOutPutDeviceCallback_->OnPreferredOutputDeviceUpdated(audioDeviceDescriptor);
        std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDeviceDescriptor4 =
        std::make_shared<AudioStandard::AudioDeviceDescriptor>();
        audioDeviceDescriptor4->macAddress_ = "00:00:00:00:00:00";
        audioDeviceDescriptor4->deviceType_ = AudioStandard::DEVICE_TYPE_SPEAKER;
        audioDeviceDescriptor4->networkId_ = "LocalDevice";
        audioDeviceDescriptor.insert(audioDeviceDescriptor.begin(), std::move(audioDeviceDescriptor4));
        BluetoothAudioFrameworkAdapter::GetInstance()
            .audioPreferredOutPutDeviceCallback_->OnPreferredOutputDeviceUpdated(audioDeviceDescriptor);
    }
    HILOGI("preferred_output_device_updated_test_001 end");
#endif
}

/*
 * @tc.number:preferred_output_device_updated_test_002
 * @tc.name: UnregisterAudioPreferredOutPutDeviceChangeListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, preferred_output_device_updated_test_002, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("preferred_output_device_updated_test_002 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioPreferredOutPutDeviceChangeListener();
    HILOGI("preferred_output_device_updated_test_002 end");
#endif
}
 
HWTEST_F(BluetoothAudioFrameworkAdapterTest, set_music_mute_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("set_music_mute_001 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().SetMusicMuteWhenA2dpRelease();
    HILOGI("set_music_mute_001 end");
#endif
}

HWTEST_F(BluetoothAudioFrameworkAdapterTest, set_music_unmute_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("set_music_unmute_001 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().SetMusicUnmute();
    HILOGI("set_music_unmute_001 end");
#endif
}

HWTEST_F(BluetoothAudioFrameworkAdapterTest, set_music_unmute_002, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("set_music_unmute_002 enter");
    int32_t ret = BluetoothAudioFrameworkAdapter::GetInstance().SetMusicUnmute();
    if (!ret) {
        HILOGI("SetMusicUnmute success.");
    }
    HILOGI("set_music_unmute_002 end");
#endif
}

HWTEST_F(BluetoothAudioFrameworkAdapterTest, set_music_mute_002, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("set_music_mute_002 enter");
    int32_t ret = BluetoothAudioFrameworkAdapter::GetInstance().SetMusicMuteWhenA2dpRelease();
    if (ret) {
        HILOGI("current device volume is zero.");
    }
    HILOGI("set_music_mute_002 end");
#endif
}

HWTEST_F(BluetoothAudioFrameworkAdapterTest, set_music_mute_003, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("set_music_mute_003 enter");
    int32_t ret = BluetoothAudioFrameworkAdapter::GetInstance().SetMusicMuteWhenA2dpRelease();
    if (ret) {
        HILOGI("volume is zero or single device, no need mute");
    }
    HILOGI("set_music_mute_003 end");
#endif
}
HWTEST_F(BluetoothAudioFrameworkAdapterTest, set_audio_microphonemute_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("set_audio_microphonemute_001 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().SetAudioMicrophoneMute(true);
    HILOGI("set_audio_microphonemute_001 end");
#endif
}

/*
 * @tc.number:audio_spatial_audio_test_001
 * @tc.name: SpatialAudioModeChange
 * @tc.desc: bluetoothHwSrcInterface null case
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_spatial_audio_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_spatial_audio_test_001 enter");
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioModeCallback_ != nullptr);
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioHeadTrackingCallback_ != nullptr);
    BluetoothHwInterface::GetInstance()->SetBtHwInterface(nullptr);
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev = GetAudioDeviceDescriptor();
    EXPECT_TRUE(audioDev != nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().
        spatialAudioModeCallback_->OnSpatializationEnabledChangeForAnyDevice(audioDev, true);
    BluetoothAudioFrameworkAdapter::GetInstance().
        spatialAudioHeadTrackingCallback_->OnHeadTrackingEnabledChangeForAnyDevice(audioDev, true);
    HILOGI("audio_spatial_audio_test_001 end");
#endif
}

/*
 * @tc.number:audio_spatial_audio_test_002
 * @tc.name: SpatialAudioModeChange
 * @tc.desc: device is not active
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_spatial_audio_test_002, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_spatial_audio_test_002 enter");
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioModeCallback_ != nullptr);
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioHeadTrackingCallback_ != nullptr);

    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    RawAddress activeDevice = RawAddress("11:22:33:44:55:66");
    RawAddress tempDevice = RawAddress("22:33:44:55:66:77");
    InitA2dpServiceAndSetActiveDevice(activeDevice);
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev = GetAudioDeviceDescriptor(tempDevice.GetAddress());
    EXPECT_TRUE(audioDev != nullptr);

    BluetoothAudioFrameworkAdapter::GetInstance().
        spatialAudioModeCallback_->OnSpatializationEnabledChangeForAnyDevice(audioDev, true);
    BluetoothAudioFrameworkAdapter::GetInstance().
        spatialAudioHeadTrackingCallback_->OnHeadTrackingEnabledChangeForAnyDevice(audioDev, true);
    HILOGI("audio_spatial_audio_test_002 end");
#endif
}

/*
 * @tc.number:audio_spatial_audio_test_003
 * @tc.name: SpatialAudioModeChange
 * @tc.desc: device is active
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_spatial_audio_test_003, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_spatial_audio_test_003 enter");
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioModeCallback_ != nullptr);
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioHeadTrackingCallback_ != nullptr);

    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    InitA2dpServiceAndSetActiveDevice(device);
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev = GetAudioDeviceDescriptor(device.GetAddress());
    EXPECT_TRUE(audioDev != nullptr);

    BluetoothAudioFrameworkAdapter::GetInstance().
        spatialAudioModeCallback_->OnSpatializationEnabledChangeForAnyDevice(audioDev, true);
    BluetoothAudioFrameworkAdapter::GetInstance().
        spatialAudioHeadTrackingCallback_->OnHeadTrackingEnabledChangeForAnyDevice(audioDev, true);
    HILOGI("audio_spatial_audio_test_003 end");
#endif
}

/*
 * @tc.number:audio_spatial_audio_test_004
 * @tc.name: CheckIsSpatialAudioModeEnabled
 * @tc.desc: Check device spatial audio state
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_spatial_audio_test_004, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_spatial_audio_test_004 enter");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    EXPECT_FALSE(BluetoothAudioFrameworkAdapter::IsSpatialAudioModeEnabled(device.GetAddress()));
    EXPECT_FALSE(BluetoothAudioFrameworkAdapter::IsSpatialAudioHeadTrackingEnabled(device.GetAddress()));
    HILOGI("audio_spatial_audio_test_004 end");
#endif
}

/*
 * @tc.number:audio_spatial_audio_test_005
 * @tc.name: CheckAudioDeviceDescriptorNull
 * @tc.desc: deviceDescriptor null case
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_spatial_audio_test_005, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_spatial_audio_test_005 enter");
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioModeCallback_ != nullptr);
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioHeadTrackingCallback_ != nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().
        spatialAudioModeCallback_->OnSpatializationEnabledChangeForAnyDevice(nullptr, true);
    BluetoothAudioFrameworkAdapter::GetInstance().
        spatialAudioHeadTrackingCallback_->OnHeadTrackingEnabledChangeForAnyDevice(nullptr, true);
    HILOGI("audio_spatial_audio_test_005 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_006
 * @tc.name: HfpSetExtraAudioParameters
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_006, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_006 enter");

    std::vector<std::pair<std::string, std::string>> audioParametersKVPairs1;
    audioParametersKVPairs1.push_back(std::make_pair(HFP_AG_SERVER_SWB_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));
    audioParametersKVPairs1.push_back(std::make_pair("bt_headset_nrec", HFP_AG_SERVER_AUDIO_FEATURE_ON));
    BluetoothAudioFrameworkAdapter::HfpSetExtraAudioParameters("bt_extra", audioParametersKVPairs1);

    std::vector<std::pair<std::string, std::string>> audioParametersKVPairs2;
    audioParametersKVPairs2.push_back(std::make_pair(HFP_AG_SERVER_WBS_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));
    audioParametersKVPairs2.push_back(std::make_pair(HFP_AG_SERVER_NREC_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));
    BluetoothAudioFrameworkAdapter::HfpSetExtraAudioParameters("bt_extra", audioParametersKVPairs2);

    std::vector<std::pair<std::string, std::string>> audioParametersKVPairs3;
    audioParametersKVPairs3.push_back(std::make_pair(HFP_AG_SERVER_WBS_NAME, HFP_AG_SERVER_AUDIO_FEATURE_OFF));
    audioParametersKVPairs3.push_back(std::make_pair(HFP_AG_SERVER_NREC_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));
    BluetoothAudioFrameworkAdapter::HfpSetExtraAudioParameters("bt_extra", audioParametersKVPairs3);

    HILOGI("audio_event_manager_test_006 end");
#endif
}

/*
 * @tc.number:audio_Collaboration_audio_test_001
 * @tc.name: OnCollaborationEnabledChangeForCurrentDevice
 * @tc.desc: set collaboration enable State
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_Collaboration_audio_test_001, TestSize.Level1)
{
#if defined(AUDIO_FRAMEWORK) && defined(BLUETOOTH_AUDIO_COLLABORATION_SUPPORT)
    HILOGI("audio_Collaboration_audio_test_001 enter");
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().collaborativeAudioModeCallback_ != nullptr);

    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    InitA2dpServiceAndSetActiveDevice(device);

    BluetoothAudioFrameworkAdapter::GetInstance().
        collaborativeAudioModeCallback_->OnCollaborationEnabledChangeForCurrentDevice(true);
    HILOGI("audio_Collaboration_audio_test_001 end");
#endif
}

/*
 * @tc.number:audio_Collaboration_audio_test_002
 * @tc.name: UnRegisterCollaborativeAudioListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_Collaboration_audio_test_002, TestSize.Level1)
{
#if defined(AUDIO_FRAMEWORK) && defined(BLUETOOTH_AUDIO_COLLABORATION_SUPPORT)
    HILOGI("audio_Collaboration_audio_test_002 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().UnRegisterCollaborativeAudioListener();
    HILOGI("audio_Collaboration_audio_test_002 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_007
 * @tc.name: OnOutputPipeChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_007, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_007 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().GetCurrentOutputPipeInfos();
    AudioStandard::AudioPipeChangeType changeType = AudioStandard::PIPE_CHANGE_TYPE_PIPE_STREAM;
    std::shared_ptr<AudioStandard::AudioOutputPipeInfo> changedPipeInfo;
    uint32_t renderId = 1;
    AudioStandard::HdiAdapterType adapter = AudioStandard::HDI_ADAPTER_TYPE_UNKNOWN;
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    changedPipeInfo = std::make_shared<AudioStandard::AudioOutputPipeInfo>(renderId, adapter, routeFlag);
    changedPipeInfo->SetStatus(AudioStandard::PIPE_STATUS_OPEN);
    changedPipeInfo->SetDevice(AudioStandard::DEVICE_TYPE_WIRED_HEADPHONES);
    changedPipeInfo->AddStream(1, AudioStandard::STREAM_USAGE_VOICE_RINGTONE,
        AudioStandard::RendererState::RENDERER_RUNNING, "name"); // streamId_、usage_、state_、bundleName_
    changedPipeInfo->AddStream(2, AudioStandard::STREAM_USAGE_MUSIC, AudioStandard::RendererState::RENDERER_RUNNING, "name");
    changedPipeInfo->AddStream(3, AudioStandard::STREAM_USAGE_RINGTONE, AudioStandard::RendererState::RENDERER_RUNNING, "name");
    changedPipeInfo->AddStream(4, AudioStandard::STREAM_USAGE_VOICE_MESSAGE,
        AudioStandard::RendererState::RENDERER_RUNNING, "name");

    if (BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_->OnOutputPipeChange(
            changeType, changedPipeInfo);
    }
    HILOGI("audio_event_manager_test_007 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_008
 * @tc.name: OnOutputPipeChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_008, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_008 enter");
    AudioStandard::AudioPipeChangeType changeType = AudioStandard::PIPE_CHANGE_TYPE_PIPE_STREAM;
    std::shared_ptr<AudioStandard::AudioOutputPipeInfo> changedPipeInfo;
    uint32_t renderId = 1;
    AudioStandard::HdiAdapterType adapter = AudioStandard::HDI_ADAPTER_TYPE_A2DP;
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    changedPipeInfo = std::make_shared<AudioStandard::AudioOutputPipeInfo>(renderId, adapter, routeFlag);
    changedPipeInfo->SetStatus(AudioStandard::PIPE_STATUS_RUNNING);
    changedPipeInfo->SetDevice(AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP);
    changedPipeInfo->AddStream(1, AudioStandard::STREAM_USAGE_NOTIFICATION,
        AudioStandard::RendererState::RENDERER_PREPARED, "name"); // streamId_、usage_、state_、bundleName_
    changedPipeInfo->AddStream(2, AudioStandard::STREAM_USAGE_MUSIC, AudioStandard::RendererState::RENDERER_PREPARED, "name");
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_->OnOutputPipeChange(
            changeType, changedPipeInfo);
    }
    HILOGI("audio_event_manager_test_008 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_009
 * @tc.name: OnOutputPipeChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_009, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_009 enter");
    AudioStandard::AudioPipeChangeType changeType = AudioStandard::PIPE_CHANGE_TYPE_PIPE_DEVICE;
    std::shared_ptr<AudioStandard::AudioOutputPipeInfo> changedPipeInfo;
    uint32_t renderId = 1;
    AudioStandard::HdiAdapterType adapter = AudioStandard::HDI_ADAPTER_TYPE_PRIMARY;
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    changedPipeInfo = std::make_shared<AudioStandard::AudioOutputPipeInfo>(renderId, adapter, routeFlag);
    changedPipeInfo->SetStatus(AudioStandard::PIPE_STATUS_OPEN);
    changedPipeInfo->SetDevice(AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP);
    changedPipeInfo->AddStream(1, AudioStandard::STREAM_USAGE_NOTIFICATION,
        AudioStandard::RendererState::RENDERER_PREPARED, "name"); // streamId_、usage_、state_、bundleName_
    changedPipeInfo->AddStream(2, AudioStandard::STREAM_USAGE_MUSIC, AudioStandard::RendererState::RENDERER_PREPARED, "name");
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_->OnOutputPipeChange(
            changeType, changedPipeInfo);
        EXPECT_EQ(changedPipeInfo->GetAdapter(), AudioStandard::HDI_ADAPTER_TYPE_PRIMARY);
    }
    HILOGI("audio_event_manager_test_009 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_010
 * @tc.name: OnOutputPipeChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_010, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_010 enter");
    AudioStandard::AudioPipeChangeType changeType = AudioStandard::PIPE_CHANGE_TYPE_PIPE_STATUS;
    std::shared_ptr<AudioStandard::AudioOutputPipeInfo> changedPipeInfo;
    uint32_t renderId = 1;
    AudioStandard::HdiAdapterType adapter = AudioStandard::HDI_ADAPTER_TYPE_A2DP;
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    changedPipeInfo = std::make_shared<AudioStandard::AudioOutputPipeInfo>(renderId, adapter, routeFlag);
    changedPipeInfo->SetStatus(AudioStandard::PIPE_STATUS_CLOSE);
    changedPipeInfo->SetDevice(AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP);
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_->OnOutputPipeChange(
            changeType, changedPipeInfo);
        EXPECT_EQ(changedPipeInfo->GetAdapter(), AudioStandard::HDI_ADAPTER_TYPE_A2DP);
    }
    HILOGI("audio_event_manager_test_010 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_011
 * @tc.name: OnOutputPipeChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_011, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_011 enter");
    AudioStandard::AudioPipeChangeType changeType = AudioStandard::PIPE_CHANGE_TYPE_PIPE_STATUS;
    std::shared_ptr<AudioStandard::AudioOutputPipeInfo> changedPipeInfo;
    uint32_t renderId = 1;
    AudioStandard::HdiAdapterType adapter = AudioStandard::HDI_ADAPTER_TYPE_A2DP;
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_FAST;
    changedPipeInfo = std::make_shared<AudioStandard::AudioOutputPipeInfo>(renderId, adapter, routeFlag);
    changedPipeInfo->SetStatus(AudioStandard::PIPE_STATUS_CLOSE);
    changedPipeInfo->SetDevice(AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP);
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_->OnOutputPipeChange(
            changeType, changedPipeInfo);
        EXPECT_EQ(changedPipeInfo->GetAdapter(), AudioStandard::HDI_ADAPTER_TYPE_A2DP);
    }
    HILOGI("audio_event_manager_test_011 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_012
 * @tc.name: OnOutputPipeChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_012, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_012 enter");
    AudioStandard::AudioPipeChangeType changeType = AudioStandard::PIPE_CHANGE_TYPE_PIPE_STREAM;
    std::shared_ptr<AudioStandard::AudioOutputPipeInfo> changedPipeInfo;
    uint32_t renderId = 1;
    AudioStandard::HdiAdapterType adapter = AudioStandard::HDI_ADAPTER_TYPE_A2DP;
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_FAST;
    changedPipeInfo = std::make_shared<AudioStandard::AudioOutputPipeInfo>(renderId, adapter, routeFlag);
    changedPipeInfo->SetStatus(AudioStandard::PIPE_STATUS_RUNNING);
    changedPipeInfo->SetDevice(AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP);
    changedPipeInfo->AddStream(1, AudioStandard::STREAM_USAGE_GAME,
        AudioStandard::RendererState::RENDERER_RUNNING, "name"); // streamId_、usage_、state_、bundleName_
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_->OnOutputPipeChange(
            changeType, changedPipeInfo);
        EXPECT_EQ(changedPipeInfo->GetAdapter(), AudioStandard::HDI_ADAPTER_TYPE_A2DP);
    }
    HILOGI("audio_event_manager_test_012 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_013
 * @tc.name: OnOutputPipeChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_013, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_013 enter");
    AudioStandard::AudioPipeChangeType changeType = AudioStandard::PIPE_CHANGE_TYPE_PIPE_STREAM;
    std::shared_ptr<AudioStandard::AudioOutputPipeInfo> changedPipeInfo;
    uint32_t renderId = 1;
    AudioStandard::HdiAdapterType adapter = AudioStandard::HDI_ADAPTER_TYPE_A2DP;
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    changedPipeInfo = std::make_shared<AudioStandard::AudioOutputPipeInfo>(renderId, adapter, routeFlag);
    changedPipeInfo->SetStatus(AudioStandard::PIPE_STATUS_RUNNING);
    changedPipeInfo->SetDevice(AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP);
    changedPipeInfo->AddStream(1, AudioStandard::STREAM_USAGE_MUSIC,
        AudioStandard::RendererState::RENDERER_RUNNING, "name"); // streamId_、usage_、state_、bundleName_
    changedPipeInfo->AddStream(2, AudioStandard::STREAM_USAGE_NAVIGATION,
        AudioStandard::RendererState::RENDERER_RUNNING, "name");
    changedPipeInfo->AddStream(3, AudioStandard::STREAM_USAGE_VOICE_RINGTONE,
        AudioStandard::RendererState::RENDERER_RUNNING, "name");
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_->OnOutputPipeChange(
            changeType, changedPipeInfo);
        EXPECT_EQ(changedPipeInfo->GetAdapter(), AudioStandard::HDI_ADAPTER_TYPE_A2DP);
    }
    HILOGI("audio_event_manager_test_013 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_004
 * @tc.name: UnregisterOutputPipeChangeListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_004, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_004 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().UnregisterOutputPipeChangeListener();
    HILOGI("audio_event_manager_test_004 end");
#endif
}

HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_manager_test_055, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_manager_test_055 enter");
    bool ret = BluetoothAudioFrameworkAdapter::GetInstance().IsAudioOutputToBluetoothA2dp();
    EXPECT_EQ(ret, false);
    HILOGI("audio_manager_test_055 end");
#endif
}

HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_manager_test_056, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_manager_test_056 enter");
    bool ret = BluetoothAudioFrameworkAdapter::GetInstance().IsAudioOutputToBluetoothSco();
    EXPECT_EQ(ret, false);
    HILOGI("audio_manager_test_056 end");
#endif
}

/*
 * @tc.number:audio_voice_combine_feature_test_001
 * @tc.name: UpdateLocalVoiceCombineFeatureState
 * @tc.desc: BluetoothHwInterface is null
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_voice_combine_feature_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_voice_combine_feature_test_001 enter");
    BluetoothHwInterface::GetInstance()->SetBtHwInterface(nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateLocalVoiceCombineFeatureState();
    HILOGI("audio_voice_combine_feature_test_001 end");
#endif
}

/*
 * @tc.number:audio_voice_combine_feature_test_002
 * @tc.name: UpdateLocalVoiceCombineFeatureState
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_voice_combine_feature_test_002, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_voice_combine_feature_test_002 enter");
    BluetoothHwInterface::GetInstance()->SetBtHwInterface(&g_btHwIfTest);
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateLocalVoiceCombineFeatureState();
    HILOGI("audio_voice_combine_feature_test_002 end");
#endif
}

/*
 * @tc.number:audio_voice_combine_feature_test_003
 * @tc.name: SetActiveDeviceVoiceCombineAbility
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_voice_combine_feature_test_003, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_voice_combine_feature_test_003 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().SetActiveDeviceVoiceCombineAbility(true);
    BluetoothAudioFrameworkAdapter::GetInstance().SetActiveDeviceVoiceCombineAbility(false);
    HILOGI("audio_voice_combine_feature_test_003 end");
#endif
}

/*
 * @tc.number:audio_voice_combine_feature_test_004
 * @tc.name: HfpGetExtraAudioParameters
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_voice_combine_feature_test_004, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_voice_combine_feature_test_004 enter");
    std::string mainKey = "audio_effect";
    std::vector<std::string> subKeys = {"ai_voice_combine_ability"};
    std::vector<std::pair<std::string, std::string>> params;
    BluetoothAudioFrameworkAdapter::GetInstance().HfpGetExtraAudioParameters(mainKey, subKeys, params);
    HILOGI("audio_voice_combine_feature_test_004 end");
#endif
}

/*
 * @tc.number:audio_voice_combine_feature_test_005
 * @tc.name: RegisterAudioNnStateEventListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_voice_combine_feature_test_005, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_voice_combine_feature_test_005 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioNnStateEventListener();
    HILOGI("audio_voice_combine_feature_test_005 end");
#endif
}

/*
 * @tc.number:audio_voice_combine_feature_test_006
 * @tc.name: RegisterAudioNnStateEventListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_voice_combine_feature_test_006, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_voice_combine_feature_test_006 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioNnStateEventListener();
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().audioNnStateEventCallback_ != nullptr);

    BluetoothHwInterface::GetInstance()->SetBtHwInterface(&g_btHwIfTest);
    /* voce combine feature off */
    BluetoothAudioFrameworkAdapter::GetInstance().audioNnStateEventCallback_->OnNnStateChange(0);
    /* voce combine feature off */
    BluetoothAudioFrameworkAdapter::GetInstance().audioNnStateEventCallback_->OnNnStateChange(1);
    /* voce combine feature off but audio nn switch off */
    BluetoothAudioFrameworkAdapter::GetInstance().audioNnStateEventCallback_->OnNnStateChange(2);
    /* voce combine feature and audio nn switch on */
    BluetoothAudioFrameworkAdapter::GetInstance().BtSetDeviceAbsVolumeSupported("00:11:22:33:44:55", 0, 0);
    BluetoothAudioFrameworkAdapter::GetInstance().BtSetDeviceAbsVolumeSupported("00:11:22:33:44:55", 1, 0);
    HILOGI("audio_voice_combine_feature_test_006 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_014
 * @tc.name: RegisterRendererDataTransferListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_014, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_014 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterRendererDataTransferListener();
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_  != nullptr);
    HILOGI("audio_event_manager_test_014 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_015
 * @tc.name: UnregisterRendererDataTransferListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_015, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_015 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().UnregisterRendererDataTransferListener();
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_  == nullptr);
    HILOGI("audio_event_manager_test_015 end");
#endif
}
/*
 * @tc.number:audio_event_manager_test_016
 * @tc.name: RegisterAudioFrameworkAdapterListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_016, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_016 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioFrameworkAdapterListener();
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_  != nullptr);
    HILOGI("audio_event_manager_test_016 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_017
 * @tc.name: UnregisterAudioFrameworkAdapterListener
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_017, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_017 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioFrameworkAdapterListener();
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_  == nullptr);
    HILOGI("audio_event_manager_test_017 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_018
 * @tc.name: OnDataTransferStateChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_018, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_018 enter");
    AudioStandard::AudioRendererDataTransferStateChangeInfo info;
    info.clientUID = 1000;
    info.sessionId = 1;
    info.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_MEDIA;
    info.stateChangeType = AudioStandard::DataTransferStateChangeType::AUDIO_STREAM_STOP;
    info.isBackground = false;
    info.audioMode = AudioStandard::AUDIO_MODE_RECORD;
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_->OnDataTransferStateChange(info);
        EXPECT_EQ(info.audioMode, AudioStandard::AUDIO_MODE_RECORD);
    }
    HILOGI("audio_event_manager_test_018 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_019
 * @tc.name: OnDataTransferStateChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_019, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_019 enter");
    AudioStandard::AudioRendererDataTransferStateChangeInfo info;
    info.clientUID = 1000;
    info.sessionId = 2;
    info.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_MUSIC;
    info.stateChangeType = AudioStandard::DataTransferStateChangeType::DATA_TRANS_STOP;
    info.isBackground = false;
    info.audioMode = AudioStandard::AUDIO_MODE_PLAYBACK;
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_->OnDataTransferStateChange(info);
        EXPECT_EQ(info.audioMode, AudioStandard::AUDIO_MODE_PLAYBACK);
    }
    HILOGI("audio_event_manager_test_019 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_020
 * @tc.name: OnDataTransferStateChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_020, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_020 enter");
    AudioStandard::AudioRendererDataTransferStateChangeInfo info;
    info.clientUID = 1000;
    info.sessionId = 2;
    info.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_MUSIC;
    info.stateChangeType = AudioStandard::DataTransferStateChangeType::DATA_TRANS_RESUME;
    info.isBackground = false;
    info.audioMode = AudioStandard::AUDIO_MODE_PLAYBACK;
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_->OnDataTransferStateChange(info);
        EXPECT_EQ(info.audioMode, AudioStandard::AUDIO_MODE_PLAYBACK);
    }
    HILOGI("audio_event_manager_test_020 end");
#endif
}

/*
 * @tc.number:audio_event_manager_test_021
 * @tc.name: OnMuteStateChange
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_event_manager_test_021, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_event_manager_test_021 enter");
    if (BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_ != nullptr) {
        BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_->OnMuteStateChange(1, 2, false);
        EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().audioDataTransferCallback_  != nullptr);
    }
    HILOGI("audio_event_manager_test_021 end");
#endif
}

/*
 * @tc.number:update_mute_renderer_state_test_001
 * @tc.name: UpdateMuteRendererState
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, update_mute_renderer_state_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("update_mute_renderer_state_test_001 enter");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    StreamInfo newInfo1 = {
        .routeFlag = AUDIO_OUTPUT_FLAG_NORMAL,
        .streamId = 1,
        .streamUsage = OHOS::AudioStandard::STREAM_USAGE_MUSIC,
        .status = AudioStandard::RendererState::RENDERER_RUNNING,
        .bundleName = "name",
    };
    StreamInfo newInfo2 = {
        .routeFlag = AUDIO_OUTPUT_FLAG_NORMAL,
        .streamId = 2,
        .streamUsage = OHOS::AudioStandard::STREAM_USAGE_MUSIC,
        .status = AudioStandard::RendererState::RENDERER_RUNNING,
        .bundleName = "name",
    };
    BluetoothAudioFrameworkAdapter::GetInstance().AddMemberRouteFlagList(newInfo1);
    BluetoothAudioFrameworkAdapter::GetInstance().AddMemberRouteFlagList(newInfo2);
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateMuteRendererState(device, 1, true);
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateMuteRendererState(device, 2, false);
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateMuteRendererState(device, 3, true);
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().GetRouteFlagList().size() > 0);
    HILOGI("update_mute_renderer_state_test_001 end");
#endif
}

/*
 * @tc.number:on_spatial_audio_source_type_change_test001
 * @tc.name: OnSpatialAudioSourceTypeChange
 * @tc.desc: Test OnSpatialAudioSourceTypeChange
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, on_spatial_audio_source_type_change_test001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("on_spatial_audio_source_type_change_test001 enter");
    auto &listener = BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioSourceTypeCallback_;
    ASSERT_NE(listener, nullptr);
    listener->OnSpatialAudioSourceTypeChange(AudioStandard::SpatialAudioSourceType::SPATIAL_AUDIO_SOURCE_TYPE_STEREO);
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().spatialAudioSourceTypeCallback_ != nullptr);
    HILOGI("on_spatial_audio_source_type_change_test001 end");
#endif
}

/*
 * @tc.number:on_adaptive_spatial_rendering_enabled_change_for_any_device_test_001
 * @tc.name: OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice
 * @tc.desc: Test OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, on_adaptive_spatial_rendering_enabled_change_for_any_device_test_001,
    TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("on_adaptive_spatial_rendering_enabled_change_for_any_device_test_001 enter");
    auto &listener = BluetoothAudioFrameworkAdapter::GetInstance().adaptiveSwitchStatusChangeCallback_;
    ASSERT_NE(listener, nullptr);
    auto deviceDescriptor = GetAudioDeviceDescriptor("00:00:00:00:00:00");
    listener->OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(deviceDescriptor, true);
    bool status = BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::
        GetAdaptiveSwitchStatus();
    EXPECT_FALSE(status);
    HILOGI("on_adaptive_spatial_rendering_enabled_change_for_any_device_test_001 end");
#endif
}

/*
 * @tc.number:is_spatial_audio_adaptive_switch_enabled_test_001
 * @tc.name: IsSpatialAudioAdaptiveSwitchEnabled
 * @tc.desc: Test IsSpatialAudioAdaptiveSwitchEnabled
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, is_spatial_audio_adaptive_switch_enabled_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("is_spatial_audio_adaptive_switch_enabled_test_001 enter");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    bool result = BluetoothAudioFrameworkAdapter::IsSpatialAudioAdaptiveSwitchEnabled(device.GetAddress());
    EXPECT_FALSE(result);
    HILOGI("is_spatial_audio_adaptive_switch_enabled_test_001 end");
#endif
}

/*
 * @tc.number:set_adaptive_switch_status_test_001
 * @tc.name: SetAdaptiveSwitchStatus
 * @tc.desc: Test SetAdaptiveSwitchStatus
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, set_adaptive_switch_status_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("set_adaptive_switch_status_test_001 enter");
    BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::
        SetAdaptiveSwitchStatus(true);
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::
        GetAdaptiveSwitchStatus());
    HILOGI("set_adaptive_switch_status_test_001 end");
#endif
}

/*
 * @tc.number:get_adaptive_switch_status_test_001
 * @tc.name: GetAdaptiveSwitchStatus
 * @tc.desc: Test GetAdaptiveSwitchStatus
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, get_adaptive_switch_status_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("get_adaptive_switch_status_test_001 enter");
    BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::
        SetAdaptiveSwitchStatus(false);
    bool status = BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::
        GetAdaptiveSwitchStatus();
    EXPECT_FALSE(status);
    HILOGI("get_adaptive_switch_status_test_001 end");
#endif
}

/*
 * @tc.number:get_audio_scene_test_001
 * @tc.name: GetAudioScene
 * @tc.desc: Test GetAudioScene
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, get_audio_scene_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("get_audio_scene_test_001 enter");
    BtAudioScene scene = BluetoothAudioFrameworkAdapter::GetAudioScene();
    EXPECT_NE(scene, BtAudioScene::AUDIO_SCENE_MAX);
    HILOGI("get_audio_scene_test_001 end");
#endif
}

/*
 * @tc.number:is_enhanced_routing_supported_test_001
 * @tc.name: IsEnhancedRoutingSupported
 * @tc.desc: Test IsEnhancedRoutingSupported
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, is_enhanced_routing_supported_test_001, TestSize.Level1)
{
    HILOGI("is_enhanced_routing_supported_test_001 enter");
    bool ret = BluetoothAudioFrameworkAdapter::IsEnhancedRoutingSupported();
    HILOGI("is_enhanced_routing_supported_test_001 ret: %{public}d", ret);
    HILOGI("is_enhanced_routing_supported_test_001 end");
}

/*
 * @tc.number:audio_renderer_state_test_001
 * @tc.name: RegisterAudioRenderStateChangeListener
 * @tc.desc: Register renderer state change listener and verify callback
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_001, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_001 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioRenderStateChangeListener();
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_ != nullptr);
    HILOGI("audio_renderer_state_test_001 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_002
 * @tc.name: OnRendererStateChange
 * @tc.desc: Empty rendererChangeInfos list, navigation mode should be false
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_002, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_002 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_FALSE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_002 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_003
 * @tc.name: OnRendererStateChange
 * @tc.desc: RENDERER_RUNNING with STREAM_USAGE_NAVIGATION, navigation mode should be true
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_003, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_003 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto info = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info, nullptr);
    info->rendererState = AudioStandard::RendererState::RENDERER_RUNNING;
    info->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_NAVIGATION;
    audioRendererChangeInfos.push_back(info);
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_TRUE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_003 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_004
 * @tc.name: OnRendererStateChange
 * @tc.desc: RENDERER_PREPARED with STREAM_USAGE_VOICE_ASSISTANT, navigation mode should be false
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_004, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_004 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto info = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info, nullptr);
    info->rendererState = AudioStandard::RendererState::RENDERER_PREPARED;
    info->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_VOICE_ASSISTANT;
    audioRendererChangeInfos.push_back(info);
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_FALSE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_004 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_005
 * @tc.name: OnRendererStateChange
 * @tc.desc: RENDERER_PREPARED with STREAM_USAGE_SYSTEM, navigation mode should be false
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_005, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_005 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto info = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info, nullptr);
    info->rendererState = AudioStandard::RendererState::RENDERER_PREPARED;
    info->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_SYSTEM;
    audioRendererChangeInfos.push_back(info);
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_FALSE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_005 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_006
 * @tc.name: OnRendererStateChange
 * @tc.desc: RENDERER_PREPARED with STREAM_USAGE_ALARM, navigation mode should be false
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_006, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_006 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto info = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info, nullptr);
    info->rendererState = AudioStandard::RendererState::RENDERER_PREPARED;
    info->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_ALARM;
    audioRendererChangeInfos.push_back(info);
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_FALSE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_006 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_007
 * @tc.name: OnRendererStateChange
 * @tc.desc: RENDERER_PREPARED with STREAM_USAGE_NOTIFICATION, navigation mode should be false
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_007, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_007 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto info = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info, nullptr);
    info->rendererState = AudioStandard::RendererState::RENDERER_PREPARED;
    info->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_NOTIFICATION;
    audioRendererChangeInfos.push_back(info);
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_FALSE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_007 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_008
 * @tc.name: OnRendererStateChange
 * @tc.desc: RENDERER_PREPARED with STREAM_USAGE_NAVIGATION, navigation mode should be false
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_008, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_008 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto info = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info, nullptr);
    info->rendererState = AudioStandard::RendererState::RENDERER_PREPARED;
    info->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_NAVIGATION;
    audioRendererChangeInfos.push_back(info);
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_FALSE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_008 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_009
 * @tc.name: OnRendererStateChange
 * @tc.desc: Mixed RENDERER_RUNNING(NAVIGATION) and RENDERER_PREPARED(VOICE_ASSISTANT),
 *           navigation mode should be true
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_009, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_009 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto info1 = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info1, nullptr);
    info1->rendererState = AudioStandard::RendererState::RENDERER_RUNNING;
    info1->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_NAVIGATION;
    audioRendererChangeInfos.push_back(info1);
    auto info2 = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info2, nullptr);
    info2->rendererState = AudioStandard::RendererState::RENDERER_PREPARED;
    info2->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_VOICE_ASSISTANT;
    audioRendererChangeInfos.push_back(info2);
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_TRUE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_009 end");
#endif
}

/*
 * @tc.number:audio_renderer_state_test_010
 * @tc.name: OnRendererStateChange
 * @tc.desc: Multiple RENDERER_RUNNING with one NAVIGATION, navigation mode should be true
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_010, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_010 enter");
    A2dpSrcService *src = static_cast<A2dpSrcService *>(
        ProfileServiceManager::GetInstance()->GetProfileService("A2dpSrcService"));
    ASSERT_NE(src, nullptr);
    EXPECT_FALSE(src->isNavigationRunning_);
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto info1 = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info1, nullptr);
    info1->rendererState = AudioStandard::RendererState::RENDERER_RUNNING;
    info1->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_VOICE_ASSISTANT;
    audioRendererChangeInfos.push_back(info1);
    auto info2 = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info2, nullptr);
    info2->rendererState = AudioStandard::RendererState::RENDERER_RUNNING;
    info2->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_NAVIGATION;
    audioRendererChangeInfos.push_back(info2);
    auto info3 = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    ASSERT_NE(info3, nullptr);
    info3->rendererState = AudioStandard::RendererState::RENDERER_RUNNING;
    info3->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_SYSTEM;
    audioRendererChangeInfos.push_back(info3);
    ASSERT_NE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_, nullptr);
    BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_->OnRendererStateChange(
        audioRendererChangeInfos);
    EXPECT_TRUE(src->isNavigationRunning_);
    HILOGI("audio_renderer_state_test_010 end");
#endif
}
/*
 * @tc.number:audio_renderer_state_test_011
 * @tc.name: UnregisterAudioRenderStateChangeListener
 * @tc.desc: Unregister renderer state change listener and verify callback is null
 */
HWTEST_F(BluetoothAudioFrameworkAdapterTest, audio_renderer_state_test_011, TestSize.Level1)
{
#ifdef AUDIO_FRAMEWORK
    HILOGI("audio_renderer_state_test_011 enter");
    BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioRenderStateChangeListener();
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().rendererStateCallback_ == nullptr);
    HILOGI("audio_renderer_state_test_011 end");
#endif
}
}  // namespace bluetooth
}  // namespace OHOS
