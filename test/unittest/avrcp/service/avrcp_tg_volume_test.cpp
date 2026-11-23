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

#include "avrc_defs.h"
#include "avrcp_tg_volume.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "gavdp/a2dp_service.h"
#include "a2dp_src/a2dp_src_service.h"
#include "class_creator.h"
#include "profile_service_manager.h"
#include "profile_config.h"
#include "thread_util.h"
#include "preferences_manager.h"
#include "refuse_play_helper.h"
#include "remote_device_properties.h"
#include "bluetooth_audio_framework_adapter.h"
#include "btcommon/timer_manager.h"
#ifdef MULTI_A2DP_ENABLED
#include "multi_a2dp_manager.h"
#endif

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
// Max music volume differs per product (phone 15, car 38); expectations below assume 15.
constexpr int32_t TEST_MAX_VOLUME = 15;
static BluetoothHost *host_;
static AvrcpVolumeInterfaceImpl g_volumeInterface;
static int32_t g_realMaxVolume = 0;

void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

class AvrcpTgVolumeTest : public testing::Test {
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
        g_volumeInterface.Init();
        ClassFactory::RegisterClass("A2dpSrcService", A2dpSrcServiceCreate);
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        g_volumeInterface.Cleanup();
        BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioFrameworkAdapterListener();
        sleep(TIME);
        utility::CloseAllTimer();
    }
    void SetUp()
    {
        HILOGI("SetUp start");
        auto manager = ProfileServiceManager::GetInstance();
        string name = "A2dpSrcService";
        manager->SetProfileService(BTTransport::ADAPTER_BREDR, name);
        // Disable a2dp thread
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_A2DP, ThreadUtil::DISABLED);
        g_realMaxVolume = g_volumeInterface.musicMaxVolumeLevel_;
        g_volumeInterface.musicMaxVolumeLevel_ = TEST_MAX_VOLUME;
        HILOGI("SetUp end");
    }
    void TearDown()
    {
        g_volumeInterface.musicMaxVolumeLevel_ = g_realMaxVolume;
        auto manager = ProfileServiceManager::GetInstance();
        A2dpSrcService *src = static_cast<A2dpSrcService *>(manager->GetProfileService("A2dpSrcService"));
        delete src;
    }
};

static int g_absVolumeCbCount = 0;
static int8_t g_absVolumeCbValue = -1;

void MockAbsVolumeCb(STACK::RawAddress addr, int8_t volume)
{
    g_absVolumeCbCount++;
    g_absVolumeCbValue = volume;
}

void MockVolumeChangedCb(STACK::RawAddress addr, int8_t volume)
{}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_DeviceConnected_001
 * @tc.desc: DeviceConnected
 * @tc.type: DeviceConnected
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_DeviceConnected_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_DeviceConnected_001 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    g_volumeInterface.DeviceConnected(addr);
    g_volumeInterface.DeviceConnected(addr,
        std::bind(MockVolumeChangedCb, std::placeholders::_1, std::placeholders::_2));
    int64_t ret = RefusePlayHelper::GetInstance()->
        GetLastAvrcpConnectTime(ServiceUtil::AddrFromStack(addr).GetAddress());
    EXPECT_NE(ret, DEFAULT_VALUE);
    HILOGI("AvrcpTgVolumeTest_UnitTest_DeviceConnected_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_SetVolume_001
 * @tc.desc: DeviceConnected
 * @tc.type: DeviceConnected
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_SetVolume_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_SetVolume_001 enter");
    int avrcpVolume = 50;
    g_volumeInterface.SetVolume(avrcpVolume);
    g_volumeInterface.SetVolume(avrcpVolume);
    EXPECT_EQ(g_volumeInterface.AvrcpToSystemVolume(avrcpVolume), 6);
    HILOGI("AvrcpTgVolumeTest_UnitTest_SetVolume_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_SendVolumeChanged_001
 * @tc.desc: DeviceConnected
 * @tc.type: DeviceConnected
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_SendVolumeChanged_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_SendVolumeChanged_001 enter");
    int systemVolume = 7;
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    PreferencesManager::Delete(addr.ToString(), PreferencesManagerType::ABS_VOLUME_SWITCH);
    g_volumeInterface.SendVolumeChanged(addr, systemVolume);

    PreferencesManager::Save(addr.ToString(), true, PreferencesManagerType::ABS_VOLUME_SWITCH);
    PreferencesManager::Save(addr.ToString(), 3, PreferencesManagerType::ABS_VOLUME);
    g_volumeInterface.SendVolumeChanged(addr, systemVolume);
    g_volumeInterface.SendVolumeChanged(addr, -1);
    g_volumeInterface.SendVolumeChanged(addr, systemVolume);
    g_volumeInterface.SendVolumeChanged(addr, 50);
    EXPECT_EQ(g_volumeInterface.GetDefaultVolume(), 7);
    HILOGI("AvrcpTgVolumeTest_UnitTest_SendVolumeChanged_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_001
 * @tc.desc: DeviceConnected
 * @tc.type: DeviceConnected
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_001 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    g_volumeInterface.SwitchAbsVolumeDevice(addr);
    EXPECT_EQ(g_volumeInterface.GetDefaultVolume(), 7);
    HILOGI("AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_002
 * @tc.desc: DeviceConnected
 * @tc.type: DeviceConnected
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_002, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_002 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService) {
        a2dpService->SetActiveSinkDevice(ServiceUtil::AddrFromStack(addr));
    } else {
        HILOGE("AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_002 a2dpService null");
    }
    g_volumeInterface.SwitchAbsVolumeDevice(addr);
    g_volumeInterface.SwitchAbsVolumeDevice(addr, false);
    EXPECT_EQ(g_volumeInterface.GetDefaultVolume(), 7);
    HILOGI("AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_002 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_GetDeviceAbsVolumeAbility_001
 * @tc.desc: DeviceConnected
 * @tc.type: DeviceConnected
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_GetDeviceAbsVolumeAbility_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_GetDeviceAbsVolumeAbility_001 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    g_volumeInterface.SetDeviceAbsVolumeAbility(addr, 0);
    int32_t ret = g_volumeInterface.GetDeviceAbsVolumeAbility(addr);
    EXPECT_EQ(ret, DEVICE_ABSVOL_UNSUPPORT);
    HILOGI("AvrcpTgVolumeTest_UnitTest_GetDeviceAbsVolumeAbility_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_setVolumeOfDevice_001
 * @tc.desc: AvrcpTgVolumeTest_UnitTest_setVolumeOfDevice_001
 * @tc.type: AvrcpTgVolumeTest_UnitTest_setVolumeOfDevice_001
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_setVolumeOfDevice_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_setVolumeOfDevice_001 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    int32_t volume = 50;
    g_volumeInterface.setVolumeOfDevice(50, addr);
    g_volumeInterface.setVolumeOfDevice(50, addr);
    int32_t storeVolume = PreferencesManager::Get(addr.ToString(), g_volumeInterface.GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    EXPECT_EQ(storeVolume, 7);
    HILOGI("AvrcpTgVolumeTest_UnitTest_setVolumeOfDevice_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_NotifyAudioVolumeEvent_001
 * @tc.desc: NotifyAudioVolumeEvent
 * @tc.type: NotifyAudioVolumeEvent
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_NotifyAudioVolumeEvent_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_NotifyAudioVolumeEvent_001 enter");
    g_volumeInterface.NotifyAudioVolumeEvent(1, 3);

    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService) {
        RawAddress device = RawAddress("aa:11:22:33:44:55");
        a2dpService->NotifyMediaStackChanged(0, device);
        a2dpService->activeDevice_ = device;
        g_volumeInterface.NotifyAudioVolumeEvent(1, 3);
        EXPECT_EQ(g_volumeInterface.GetDefaultVolume(), 7);
    } else {
        HILOGE("AvrcpTgVolumeTest_UnitTest_NotifyAudioVolumeEvent_001 a2dpService null");
    }

    HILOGI("AvrcpTgVolumeTest_UnitTest_NotifyAudioVolumeEvent_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_NotifyAbsVolumeByAddr_001
 * @tc.desc: NotifyAbsVolumeByAddr with and without a registered volume callback
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_NotifyAbsVolumeByAddr_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_NotifyAbsVolumeByAddr_001 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    g_volumeInterface.NotifyAbsVolumeByAddr(addr, 50);
    g_volumeInterface.DeviceConnected(addr,
        std::bind(MockAbsVolumeCb, std::placeholders::_1, std::placeholders::_2));
    int32_t countBefore = g_absVolumeCbCount;
    g_volumeInterface.NotifyAbsVolumeByAddr(addr, 50);
    EXPECT_GT(g_absVolumeCbCount, countBefore);
    EXPECT_EQ(50, g_absVolumeCbValue);
    g_volumeInterface.DeviceDisconnected(addr);
    HILOGI("AvrcpTgVolumeTest_UnitTest_NotifyAbsVolumeByAddr_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_003
 * @tc.desc: SwitchAbsVolumeDevice: abs volume switch on (notify + set) and
 *           switch off (restore stored volume to this peer)
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_003, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_003 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:55", addr);
    auto rawAddr = ServiceUtil::AddrFromStack(addr);
    auto remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(rawAddr);
    g_volumeInterface.SetDeviceAbsVolumeAbility(addr, DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN);
    if (remoteDevice != nullptr) {
        remoteDevice->SetDeviceAbsVolumeAbility(DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN);
    }
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->SetActiveSinkDevice(rawAddr);
        a2dpService->activeDevice_ = rawAddr;
    }
    EXPECT_EQ(g_volumeInterface.GetDeviceAbsVolumeAbility(addr), DEVICE_ABSVOL_OPEN);
    PreferencesManager::Save(addr.ToString(), true, PreferencesManagerType::ABS_VOLUME_SWITCH);
    PreferencesManager::Save(addr.ToString(), 5, PreferencesManagerType::ABS_VOLUME);
    g_volumeInterface.SwitchAbsVolumeDevice(addr, true);
    g_volumeInterface.SwitchAbsVolumeDevice(addr, false);
    PreferencesManager::Save(addr.ToString(), false, PreferencesManagerType::ABS_VOLUME_SWITCH);
    g_volumeInterface.SwitchAbsVolumeDevice(addr, true);
    g_volumeInterface.SwitchAbsVolumeDevice(addr, false);
    EXPECT_EQ(false, PreferencesManager::Get(addr.ToString(), true,
        PreferencesManagerType::ABS_VOLUME_SWITCH));
    if (remoteDevice != nullptr) {
        EXPECT_EQ(DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN,
            remoteDevice->GetDeviceAbsVolumeAbility());
    }
    HILOGI("AvrcpTgVolumeTest_UnitTest_SwitchAbsVolumeDevice_003 end");
}

#ifdef MULTI_A2DP_ENABLED
/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_SyncAbsVolumeOnZoneBound_001
 * @tc.desc: Multi A2DP: zone bound before avrcp abs volume connects is deferred to DeviceConnected
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_SyncAbsVolumeOnZoneBound_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_SyncAbsVolumeOnZoneBound_001 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:66", addr);
    g_volumeInterface.DeviceDisconnected(addr);
    PreferencesManager::Save(addr.ToString(), true, PreferencesManagerType::ABS_VOLUME_SWITCH);
    EXPECT_FALSE(g_volumeInterface.SyncAbsVolumeOnZoneBound(addr));
    HILOGI("AvrcpTgVolumeTest_UnitTest_SyncAbsVolumeOnZoneBound_001 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_SyncAbsVolumeOnZoneBound_002
 * @tc.desc: Multi A2DP: zone bound after avrcp connect resyncs like SwitchAbsVolumeDevice; the peer
 *           is notified only when abs volume is usable
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_SyncAbsVolumeOnZoneBound_002, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_SyncAbsVolumeOnZoneBound_002 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:66", addr);
    auto rawAddr = ServiceUtil::AddrFromStack(addr);
    auto remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(rawAddr);
    if (remoteDevice != nullptr) {
        remoteDevice->SetDeviceAbsVolumeAbility(DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN);
    }
    g_volumeInterface.DeviceConnected(addr,
        std::bind(MockAbsVolumeCb, std::placeholders::_1, std::placeholders::_2));
    PreferencesManager::Save(addr.ToString(), 9, PreferencesManagerType::ABS_VOLUME);

    PreferencesManager::Save(addr.ToString(), false, PreferencesManagerType::ABS_VOLUME_SWITCH);
    int32_t countBefore = g_absVolumeCbCount;
    EXPECT_TRUE(g_volumeInterface.SyncAbsVolumeOnZoneBound(addr));
    EXPECT_EQ(g_absVolumeCbCount, countBefore);

    PreferencesManager::Save(addr.ToString(), true, PreferencesManagerType::ABS_VOLUME_SWITCH);
    bool abilityUsable = RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(rawAddr) !=
        DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    EXPECT_TRUE(g_volumeInterface.SyncAbsVolumeOnZoneBound(addr));
    EXPECT_EQ(g_absVolumeCbCount > countBefore, abilityUsable);
    if (abilityUsable) {
        EXPECT_EQ(g_absVolumeCbValue, g_volumeInterface.SystemToAvrcpVolume(9) & 0x7F);
    }
    EXPECT_EQ(PreferencesManager::Get(addr.ToString(), 0, PreferencesManagerType::ABS_VOLUME), 9);

    g_volumeInterface.DeviceDisconnected(addr);
    EXPECT_FALSE(g_volumeInterface.SyncAbsVolumeOnZoneBound(addr));
    HILOGI("AvrcpTgVolumeTest_UnitTest_SyncAbsVolumeOnZoneBound_002 end");
}

/**
 * @tc.name: AvrcpTgVolumeTest_UnitTest_MultiA2dpDeviceConnected_001
 * @tc.desc: Multi A2DP: avrcp connect without zone binding still notifies the peer and keeps stored volume
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgVolumeTest, AvrcpTgVolumeTest_UnitTest_MultiA2dpDeviceConnected_001, TestSize.Level1)
{
    HILOGI("AvrcpTgVolumeTest_UnitTest_MultiA2dpDeviceConnected_001 enter");
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("00:11:22:33:44:77", addr);
    auto rawAddr = ServiceUtil::AddrFromStack(addr);
    EXPECT_LT(MultiA2dpManager::GetDisplayIdByAddr(rawAddr.GetAddress()), 0);
    auto remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(rawAddr);
    if (remoteDevice != nullptr) {
        remoteDevice->SetDeviceAbsVolumeAbility(DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN);
    }
    PreferencesManager::Save(addr.ToString(), 9, PreferencesManagerType::ABS_VOLUME);
    bool abilityUsable = RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(rawAddr) !=
        DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    int32_t countBefore = g_absVolumeCbCount;

    g_volumeInterface.DeviceConnected(addr,
        std::bind(MockAbsVolumeCb, std::placeholders::_1, std::placeholders::_2));
    EXPECT_EQ(g_absVolumeCbCount > countBefore, abilityUsable);
    EXPECT_EQ(PreferencesManager::Get(addr.ToString(), 0, PreferencesManagerType::ABS_VOLUME), 9);

    g_volumeInterface.DeviceDisconnected(addr);
    HILOGI("AvrcpTgVolumeTest_UnitTest_MultiA2dpDeviceConnected_001 end");
}
#endif

}  // namespace bluetooth
}  // namespace OHOS