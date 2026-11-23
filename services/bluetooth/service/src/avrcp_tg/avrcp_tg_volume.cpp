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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_tg_volume"
#endif

#include "avrcp_tg_volume.h"

#include <cmath>

#include "bluetooth_common_event_helper.h"
#include "gavdp/a2dp_service.h"
#include "ipc_skeleton.h"
#include "common_util.h"
#include "interface_adapter_classic.h"
#include "interface_adapter_manager.h"
#include "permission_manager.h"
#include "preferences_manager.h"
#include "bt_def.h"
#include "remote_device_properties.h"
#include "refuse_play_helper.h"
#include "hal_util.h"
#include "bt_chr_business_event.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "bluetooth_audio_framework_adapter.h"
#include "hfp_ag_system_interface.h"
#include "thread_util.h"
#ifdef MULTI_A2DP_ENABLED
#include "multi_a2dp_manager.h"
#endif

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
const int HW_BT_PROPERTY_ABS_VOLUM_KEY = 0x211;
STACK::RawAddress GetA2dpActiveDevice()
{
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService == nullptr) {
        return STACK::RawAddress::kEmpty;
    }
    auto device = a2dpService->GetActiveSinkDevice();
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);

    return rawAddr;
}

void AvrcpVolumeInterfaceImpl::Init()
{
    musicMaxVolumeLevel_ = BluetoothAudioFrameworkAdapter::BtGetMaxVolume(BtVolumeType::VOLUME_MUSIC,
        BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP);
}

void AvrcpVolumeInterfaceImpl::Cleanup()
{}

void AvrcpVolumeInterfaceImpl::DeviceConnected(const STACK::RawAddress &bdaddr)
{
    RemoteDeviceProperties::GetInstance()->GetRemoteDeviceProperty(
        bdaddr, static_cast<bt_property_type_t>(HW_BT_PROPERTY_ABS_VOLUM_KEY));
    PreferencesManager::Save(bdaddr.ToString(), false, PreferencesManagerType::ABS_VOLUME_SWITCH);
    SwitchAbsVolumeDevice(bdaddr, false);
    RefusePlayHelper::GetInstance()->SetLastAvrcpConnectTime(ServiceUtil::AddrFromStack(bdaddr).GetAddress());
    BluetoothHelper::BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(bdaddr.ToString(),
        static_cast<int32_t>(BTConnectState::CONNECTED));
    DoInA2dpThread([bdaddr]() {
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(a2dpService, "a2dp service is nullptr");
        a2dpService->ProcessAvrcpDynamicLoad(
            static_cast<int>(BTConnectState::CONNECTED), ServiceUtil::AddrFromStack(bdaddr), "avrcp");
    });
}

void AvrcpVolumeInterfaceImpl::DeviceConnected(const STACK::RawAddress &bdaddr, VolumeChangedCb cb)
{
    HILOGI("support absolute volume %{public}s.", bdaddr.ToLogString().c_str());
    RemoteDeviceProperties::GetInstance()->GetRemoteDeviceProperty(
        bdaddr, static_cast<bt_property_type_t>(HW_BT_PROPERTY_ABS_VOLUM_KEY));
    {
        std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
        volumeCallbackMap_.insert_or_assign(bdaddr, cb);
    }

    PreferencesManager::Save(bdaddr.ToString(), true, PreferencesManagerType::ABS_VOLUME_SWITCH);
#ifdef MULTI_A2DP_ENABLED
    /* Audio's SetActiveSinkDevice resync is unusable under Multi A2DP (it always carries the
     * default zone device), so resync here once the peer is bound to a display. Must run after
     * the callback map insertion above; SyncAbsVolumeOnZoneBound covers the opposite order. */
    bool zoneBound = MultiA2dpManager::GetDisplayIdByAddr(bdaddr.ToString()) >= 0;
    HILOGI("Multi A2DP: %{public}s zone bound %{public}d.", GET_ENCRYPT_STR_ADDR(bdaddr.ToString()), zoneBound);
    SwitchAbsVolumeDevice(bdaddr, zoneBound);
#else
    SwitchAbsVolumeDevice(bdaddr, false);
#endif
    RefusePlayHelper::GetInstance()->SetLastAvrcpConnectTime(ServiceUtil::AddrFromStack(bdaddr).GetAddress());
    BluetoothHelper::BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(bdaddr.ToString(),
        static_cast<int32_t>(BTConnectState::CONNECTED));
    DoInA2dpThread([bdaddr]() {
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(a2dpService, "a2dp service is nullptr");
        a2dpService->ProcessAvrcpDynamicLoad(
            static_cast<int>(BTConnectState::CONNECTED), ServiceUtil::AddrFromStack(bdaddr), "avrcp");
    });
}

#ifdef MULTI_A2DP_ENABLED
bool AvrcpVolumeInterfaceImpl::SyncAbsVolumeOnZoneBound(const STACK::RawAddress &addr)
{
    {
        std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
        if (volumeCallbackMap_.find(addr) == volumeCallbackMap_.end()) {
            HILOGI("Multi A2DP: %{public}s avrcp abs volume not connected, wait for it.",
                GET_ENCRYPT_STR_ADDR(addr.ToString()));
            return false;
        }
    }
    /* Audio may not have registered the A2DP device yet when AVRCP connects, so the abs volume
     * declaration there can fail; the device is always known to audio once bound to a zone. */
    HILOGI("Multi A2DP: %{public}s zone bound, resync abs volume.", GET_ENCRYPT_STR_ADDR(addr.ToString()));
    SwitchAbsVolumeDevice(addr, true);
    return true;
}
#endif

void AvrcpVolumeInterfaceImpl::DeviceEmplaceSetVolume(const STACK::RawAddress &bdaddr, VolumeChangedCb cb)
{
    std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
    volumeCallbackMap_.emplace(bdaddr, cb);
}

void AvrcpVolumeInterfaceImpl::DeviceDisconnected(const STACK::RawAddress &bdaddr)
{
    std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
    volumeCallbackMap_.erase(bdaddr);
    RefusePlayHelper::GetInstance()->ClearRecord(ServiceUtil::AddrFromStack(bdaddr).GetAddress());
    BluetoothHelper::BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(bdaddr.ToString(),
        static_cast<int32_t>(BTConnectState::DISCONNECTED));
    DoInA2dpThread([bdaddr]() {
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(a2dpService, "a2dp service is nullptr");
        a2dpService->ProcessAvrcpDynamicLoad(
            static_cast<int>(BTConnectState::DISCONNECTED), ServiceUtil::AddrFromStack(bdaddr), "avrcp");
    });
}

void AvrcpVolumeInterfaceImpl::SetVolume(int8_t volume)
{
#ifdef MULTI_A2DP_ENABLED
    /* Multi A2DP: no single active device exists. GetA2dpActiveDevice() is always
     * empty under dual, so this legacy entry would persist preferences under an
     * empty key. Per-device uplink must go through setVolumeOfDevice(volume, bdaddr).
     * This guard also covers the base VolumeInterface::setVolumeOfDevice fallback
     * which forwards to SetVolume(). */
    HILOGW("Multi A2DP: legacy SetVolume(int8_t) entry not supported, ignore volume %{public}d.", volume);
    return;
#else
    int32_t deviceVolume = AvrcpToSystemVolume(volume);
    HILOGD("SetVolume avrcpVolume: %{public}d. musicMaxVolume: %{public}d. deviceVolume: %{public}d.",
        volume,
        musicMaxVolumeLevel_,
        deviceVolume);
    auto device = GetA2dpActiveDevice();
    int32_t storeVolume = PreferencesManager::Get(device.ToString(), GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    if (deviceVolume == storeVolume) {
        HILOGW("device %{public}s Skipping SetVolume to same as current %{public}d.", device.ToLogString().c_str(),
            deviceVolume);
        return;
    }
    PreferencesManager::Save(device.ToString(), deviceVolume, PreferencesManagerType::ABS_VOLUME);

    std::string macAddr = ServiceUtil::AddrFromStack(device).GetAddress();
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    bool updateUi = systemInterface.IsCallIdle();
    // If in call, not update ui for music volume
    int32_t result = BluetoothAudioFrameworkAdapter::BtSetA2dpDeviceVolume(macAddr, deviceVolume, updateUi);
    BtChrBusinessEvent::GetInstance().SetVolumeParams(0, static_cast<int>(volume));
    HILOGI("device %{public}s SetVolume status %{public}d. volume: %{public}d.", device.ToLogString().c_str(),
        result, volume);

    if (result != AVRCP_SUCCESS) {
        HILOGE("excute SetVolume error restore device volume.");
        PreferencesManager::Save(device.ToString(), storeVolume, PreferencesManagerType::ABS_VOLUME);
    }
#endif
}

void AvrcpVolumeInterfaceImpl::setVolumeOfDevice(int8_t volume, const STACK::RawAddress &bdaddr)
{
    int32_t deviceVolume = AvrcpToSystemVolume(volume);
    HILOGD("SetVolume avrcpVolume: %{public}d. musicMaxVolume: %{public}d. deviceVolume: %{public}d.",
        volume,
        musicMaxVolumeLevel_,
        deviceVolume);
    int32_t storeVolume = PreferencesManager::Get(bdaddr.ToString(), GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    if (deviceVolume == storeVolume) {
        HILOGW("device %{public}s Skipping SetVolume to same as current %{public}d.", bdaddr.ToLogString().c_str(),
            deviceVolume);
        return;
    }
    PreferencesManager::Save(bdaddr.ToString(), deviceVolume, PreferencesManagerType::ABS_VOLUME);

    std::string macAddr = ServiceUtil::AddrFromStack(bdaddr).GetAddress();
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    bool updateUi = systemInterface.IsCallIdle();
    // If in call, not update ui for music volume
    int32_t result = BluetoothAudioFrameworkAdapter::BtSetA2dpDeviceVolume(macAddr, deviceVolume, updateUi);
    BtChrBusinessEvent::GetInstance().SetVolumeParams(0, static_cast<int>(volume));
    HILOGI("device %{public}s SetA2dpDeviceVolume status %{public}d. volume: %{public}d", bdaddr.ToLogString().c_str(),
        result, volume);

    if (result != AVRCP_SUCCESS) {
        HILOGE("excute SetVolume error restore device volume.");
        PreferencesManager::Save(bdaddr.ToString(), storeVolume, PreferencesManagerType::ABS_VOLUME);
    }
}

void AvrcpVolumeInterfaceImpl::SendVolumeChanged(const STACK::RawAddress &addr, int32_t volume)
{
    if ((volume < 0) || (volume > musicMaxVolumeLevel_)) {
        HILOGE("Invalid volume %{public}d.", volume);
        return;
    }

    if (!PreferencesManager::Get(addr.ToString(), false, PreferencesManagerType::ABS_VOLUME_SWITCH)) {
        HILOGW("device not support abs volume %{public}s.", addr.ToLogString().c_str());
        return;
    }
    int32_t storeVolume = PreferencesManager::Get(addr.ToString(), GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    if (volume == storeVolume) {
        HILOGW("%{public}s Skipping update volume to same as current %{public}d.", addr.ToLogString().c_str(), volume);
        return;
    }
    PreferencesManager::Save(addr.ToString(), volume, PreferencesManagerType::ABS_VOLUME);
    BtChrSetAudioVolume(addr.ToString(), volume);
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteVolChangeUe(BtVolumeType::VOLUME_MUSIC, volume, callingName);

    int32_t avrcpVolume = SystemToAvrcpVolume(volume);
    BtChrBusinessEvent::GetInstance().SetVolumeParams(1, static_cast<int>(avrcpVolume));
    HILOGI("volume: %{public}d. musicMaxVolume: %{public}d. avrcpVolume: %{public}d. "
           "avrcpVolume & 0x7F: %{public}d. addr: %{public}s.",
        volume,
        musicMaxVolumeLevel_,
        avrcpVolume,
        static_cast<int32_t>(static_cast<uint32_t>(avrcpVolume) & 0x7F),
        addr.ToLogString().c_str());
#ifdef MULTI_A2DP_ENABLED
    /* Multi A2DP: push to the exact device only (aligned with multi-framework
     * CdcAvrcpTargetHandler: mDeviceHandlerMap.get(device)), never broadcast. */
    std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
    auto cbIter = volumeCallbackMap_.find(addr);
    if (cbIter == volumeCallbackMap_.end()) {
        HILOGW("device %{public}s has no volume callback.", GET_ENCRYPT_STR_ADDR(addr.ToString()));
        return;
    }
    cbIter->second(addr, static_cast<int32_t>(static_cast<uint32_t>(avrcpVolume) & 0x7F));
#else
    std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
    for (const auto &cb : volumeCallbackMap_) {
        (cb.second)(addr, static_cast<int32_t>(static_cast<uint32_t>(avrcpVolume) & 0x7F));
    }
#endif
}

void AvrcpVolumeInterfaceImpl::HandleUnsupportAbsVolume(const STACK::RawAddress &addr,
    const std::string &macAddr, bool isNeedSetVolume)
{
    int32_t ret = BluetoothAudioFrameworkAdapter::BtSetDeviceAbsVolumeSupported(macAddr, false);
    int32_t volume = PreferencesManager::Get(addr.ToString(), GetDefaultVolume(),
        PreferencesManagerType::NON_ABS_VOLUME);
    if (isNeedSetVolume) {
#ifndef MULTI_A2DP_ENABLED
        BluetoothAudioFrameworkAdapter::SetStreamVolume(STREAM_MUSIC, volume, 1);
#else
        /* Multi A2DP: restore the stored volume to this peer only;
         * the global STREAM_MUSIC is shared by every zone and must not be hijacked. */
        BluetoothAudioFrameworkAdapter::SetStreamVolume(STREAM_MUSIC, volume, 1, macAddr);
#endif
    }
    HILOGI("device %{public}s not support abs volume status %{public}d, volume %{public}d.",
        addr.ToLogString().c_str(), ret, volume);
    return;
}

static void HandleAbsVolumeSwitchOff(const STACK::RawAddress &addr, const RawAddress &rawAddr,
    const std::string &macAddr, int32_t storedVolume, bool isNeedSetVolume)
{
    int32_t ret = BluetoothAudioFrameworkAdapter::BtSetDeviceAbsVolumeSupported(macAddr, false);
    HILOGI("device %{public}s abs volume switch is false, status %{public}d.", addr.ToLogString().c_str(), ret);
    if (isNeedSetVolume) {
#ifndef MULTI_A2DP_ENABLED
        BluetoothAudioFrameworkAdapter::SetStreamVolume(STREAM_MUSIC, storedVolume, 1);
#else
        /* Multi A2DP: abs switch is off, still restore to this peer only;
         * the global STREAM_MUSIC is shared by every zone and must not be hijacked. */
        BluetoothAudioFrameworkAdapter::SetStreamVolume(STREAM_MUSIC, storedVolume, 1, macAddr);
#endif
    }
    return;
}

void AvrcpVolumeInterfaceImpl::NotifyAbsVolumeByAddr(const STACK::RawAddress &addr,
    int32_t avrcpVolume)
{
#ifdef MULTI_A2DP_ENABLED
    /* Multi A2DP: restore stored volume to THIS device only. */
    {
        std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
        auto cbIter = volumeCallbackMap_.find(addr);
        if (cbIter != volumeCallbackMap_.end()) {
           cbIter->second(addr, static_cast<int32_t>(static_cast<uint32_t>(avrcpVolume) & 0x7F));
        }
    }
#else
    {
        std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
        for (const auto &cb : volumeCallbackMap_) {
            (cb.second)(addr, static_cast<int32_t>(static_cast<uint32_t>(avrcpVolume) & 0x7F));
        }
    }
#endif
}

// actice device changed, restore the volume saved last time.
void AvrcpVolumeInterfaceImpl::SwitchAbsVolumeDevice(const STACK::RawAddress &addr, bool isNeedSetVolume)
{
#ifdef MULTI_A2DP_ENABLED
    HILOGI("Multi A2DP: per-device abs volume notify %{public}s.", GET_ENCRYPT_STR_ADDR(addr.ToString()));
#else
    if (addr != GetA2dpActiveDevice()) {
        HILOGW("Not current active device, skip %{public}s.", addr.ToLogString().c_str());
        return;
    }
#endif

    int32_t ret;
    auto rawAddr = ServiceUtil::AddrFromStack(addr);
    std::string macAddr = rawAddr.GetAddress();
    int32_t absVolumeAbility =
        RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(rawAddr);
    if (absVolumeAbility == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
        HandleUnsupportAbsVolume(addr, macAddr, isNeedSetVolume);
        return;
    }

    // get remmber volume and put into effect
    int32_t storedVolume = PreferencesManager::Get(addr.ToString(), GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    HILOGI("device store volume %{public}d.", storedVolume);
    if (!PreferencesManager::Get(addr.ToString(), false, PreferencesManagerType::ABS_VOLUME_SWITCH)) {
        HandleAbsVolumeSwitchOff(addr, rawAddr, macAddr, storedVolume, isNeedSetVolume);
        return;
    }
    
    int32_t avrcpVolume = SystemToAvrcpVolume(storedVolume);
    NotifyAbsVolumeByAddr(addr, avrcpVolume);
    ret = BluetoothAudioFrameworkAdapter::BtSetDeviceAbsVolumeSupported(macAddr, true, storedVolume);
    HILOGI("set device %{public}s support abs volume status %{public}d.", addr.ToLogString().c_str(), ret);
    if (isNeedSetVolume) {
        ret = BluetoothAudioFrameworkAdapter::BtSetA2dpDeviceVolume(macAddr, storedVolume, false);
    }
    HILOGI("set device %{public}s abs volume level status %{public}d.", addr.ToLogString().c_str(), ret);
}

void AvrcpVolumeInterfaceImpl::SetDeviceAbsVolumeAbility(const STACK::RawAddress &rawAddr, int32_t ability)
{
    auto device = ServiceUtil::AddrFromStack(rawAddr);
    int32_t absVolumeAbility = RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(device);
    if (absVolumeAbility == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
        HILOGE("device %{public}s has no abs ability, can not enable or disbale.", rawAddr.ToLogString().c_str());
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE0,
            UE_COMMON_SCENE_CASE2);
        return;
    }

    SetDeviceAbsVolumeProperty(rawAddr, ability);
    HILOGI("device %{public}s, abs status = %{public}d. ", rawAddr.ToLogString().c_str(), ability);
    if (ability == DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN) {
        PreferencesManager::Save(rawAddr.ToString(), true, PreferencesManagerType::ABS_VOLUME_SWITCH);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE0,
            UE_COMMON_SCENE_CASE1);
    } else {
        PreferencesManager::Save(rawAddr.ToString(), false, PreferencesManagerType::ABS_VOLUME_SWITCH);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE0,
            UE_COMMON_SCENE_CASE0);
    }
    SwitchAbsVolumeDevice(rawAddr);
}

int32_t AvrcpVolumeInterfaceImpl::GetDeviceAbsVolumeAbility(const STACK::RawAddress &rawAddr)
{
    auto device = ServiceUtil::AddrFromStack(rawAddr);
    int32_t absVolumeAbility = RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(device);
    HILOGI("device %{public}s, abs ability = %{public}d. ", rawAddr.ToLogString().c_str(), absVolumeAbility);
    return absVolumeAbility;
}

// if absolute volume support, audio set device volume by this interface
void AvrcpVolumeInterfaceImpl::SetDeviceAbsoluteVolume(const STACK::RawAddress &rawAddr, int32_t volumeLevel)
{
    SendVolumeChanged(rawAddr, volumeLevel);
}

void AvrcpVolumeInterfaceImpl::NotifyAudioVolumeEvent(int32_t streamType, int32_t volume)
{
    HILOGD("streamType: %{public}d, volume: %{public}d.", streamType, volume);
#ifdef MULTI_A2DP_ENABLED
    /* Multi A2DP: no single active device. Persist the stream volume for every
     * connected A2DP peer, keeping the original per-device ability check and
     * abs/non-abs preference semantics. */
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "a2dp service nullptr.");
    CHECK_AND_RETURN_LOG(BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothA2dp(), "not output to a2dp.");
    auto activePeers = a2dpService->GetActivePeers();
    CHECK_AND_RETURN_LOG(!activePeers.empty(), "not found active device.");
    for (const auto &device : activePeers) {
        if (device.GetAddress().empty()) {
            continue;
        }
        int32_t absVolumeAbility =
            RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(
            device);
        if (absVolumeAbility == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
            HILOGE("device %{public}s has no abs ability, volume: %{public}d.",
                GET_ENCRYPT_ADDR(device), volume);
            PreferencesManager::Save(device.GetAddress(), volume, PreferencesManagerType::NON_ABS_VOLUME);
            continue;
        }
        // ABS volume switch is on: do not overwrite the remembered value
        if (PreferencesManager::Get(device.GetAddress(), false, PreferencesManagerType::ABS_VOLUME_SWITCH)) {
            HILOGE("device %{public}s AbsVolume is open.ignore.", GET_ENCRYPT_ADDR(device));
            continue;
        }
        PreferencesManager::Save(device.GetAddress(), volume, PreferencesManagerType::ABS_VOLUME);
    }
    return;
#else
    auto device = GetA2dpActiveDevice();
    CHECK_AND_RETURN_LOG(device != STACK::RawAddress::kEmpty, "not found active device.");
    CHECK_AND_RETURN_LOG(BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothA2dp(), "not output to a2dp.");
    int32_t absVolumeAbility =
        RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(ServiceUtil::AddrFromStack(device));
    if (absVolumeAbility == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
        HILOGE("device %{public}s has no abs ability, volume: %{public}d.", device.ToLogString().c_str(), volume);
        PreferencesManager::Save(device.ToString(), volume, PreferencesManagerType::NON_ABS_VOLUME);
        return;
    }
    // 绝对音量关闭时，设置音量值
    if (PreferencesManager::Get(device.ToString(), false, PreferencesManagerType::ABS_VOLUME_SWITCH)) {
        HILOGE("AbsVolume is open.ignore");
        return;
    }

    PreferencesManager::Save(device.ToString(), volume, PreferencesManagerType::ABS_VOLUME);
#endif
}

void AvrcpVolumeInterfaceImpl::SetDeviceAbsVolumeProperty(const STACK::RawAddress &rawAddr, int32_t ability)
{
    bt_property_t prop;
    prop.type = static_cast<bt_property_type_t>(HW_BT_PROPERTY_ABS_VOLUM_KEY);
    prop.len = sizeof(ability);
    prop.val = &ability;
    RemoteDeviceProperties::GetInstance()->SetRemoteDeviceProperty(rawAddr, prop);
}
}  // namespace bluetooth
}  // namespace OHOS
