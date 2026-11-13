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

#ifndef A2DP_SERVICE_MOCK_H
#define A2DP_SERVICE_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "adapter_config.h"
#include "a2dp_service.h"
#include "a2dp_src_btif_interface_mock.h"
#include "a2dp_snk_btif_interface_mock.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "btif_interface_mock.h"
#include "bluetooth_errorcode.h"
#include "connect_strategy_manager.h"
#include "profile_service_manager.h"
#include "profile_config.h"
#include "remote_device_properties.h"
#include "adapter_manager.h"
#include "a2dp_src/a2dp_src_service.h"
#include "class_creator.h"
#include "thread_util.h"
#include "bluetooth_hw_interface.h"

bool g_isOffloadRunning = true;
int g_offloadPathValue = 1;
int g_currentSceneType = -1;

extern "C" {
bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

int HwA2dpOffloadGetSinkStreamEncodingPathMock(const RawAddress &addr,
    const std::vector<HwBtA2dpSteamSessionInfo> &sessionsInfo, bool needStart)
{
    return g_offloadPathValue;
}

bool HwA2dpOffloadIsRunningMock(const RawAddress &addr)
{
    return g_isOffloadRunning;
}
 
void HwSetSpatialAudioModeEnabledMock(const bool enabled)
{
}
 
void HwSetSpatialAudioHeadTrackingEnabledMock(const bool enabled)
{
}

void NotifyAudioRenderStateMock(bool isRenderActive)
{
}

void HwUpdateBtAudioLatencyMock(const RawAddress& peer_address, int latency)
{
}

bool HwBtifIsShareAudioTwsDdmSupportMock(const RawAddress &activeAddr)
{
    return true;
}

void ChangeCodecMock(int32_t appType, int32_t sampleRate, uint8_t fastFlag)
{
}

void HwSetActiveMode(const RawAddress &addr)
{
}

bool GetA2dpOffloadCodecConfigMock(const RawAddress &addr, A2dpOffloadConfigCallback &config)
{
    return false;
}

void SetHdapActiveMock(const RawAddress &addr)
{
}

void HwSendSceneTypeMock(int type)
{
    g_currentSceneType = type;
}

int HwGetCurrentSceneTypeMock()
{
    return g_currentSceneType;
}

void HwSendLowLatencyStatusMock(bool isEnabled)
{
}

void HwSetAdaptiveSwitchStatus(const bool state)
{
}

bool HwGetDeviceInfoMock(const RawAddress& addr, bt_property_t &prop)
{
    return true;
}

bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .a2dpOffloadGetSinkStreamEncodingPath = HwA2dpOffloadGetSinkStreamEncodingPathMock,
    .a2dpOffloadIsRunning = HwA2dpOffloadIsRunningMock,
    .notifyAudioRenderState = NotifyAudioRenderStateMock,
    .hwSetSpatialAudioModeEnabled = HwSetSpatialAudioModeEnabledMock,
    .hwSetSpatialAudioHeadTrackingEnabled = HwSetSpatialAudioHeadTrackingEnabledMock,
    .hwUpdateBtAudioLatency = HwUpdateBtAudioLatencyMock,
    .hwBtifIsShareAudioTwsDdmSupport = HwBtifIsShareAudioTwsDdmSupportMock,
    .changeCodec = ChangeCodecMock,
    .hwSetActiveMode = HwSetActiveMode,
    .getA2dpOffloadCodecConfig = GetA2dpOffloadCodecConfigMock,
    .setHdapActive = SetHdapActiveMock,
    .hwSendSceneType = HwSendSceneTypeMock,
    .hwGetCurrentSceneType = HwGetCurrentSceneTypeMock,
    .sendLowLatencyStatus = HwSendLowLatencyStatusMock,
    .hwSetAdaptiveSwitchStatus = HwSetAdaptiveSwitchStatus,
    .hwGetDeviceInfo = HwGetDeviceInfoMock,
};
}

void SetOffloadRunning(bool running)
{
    g_isOffloadRunning = running;
}

void SetOffloadEncodePathValue(int pathValue)
{
    g_offloadPathValue = pathValue;
}

void setBtAVSourceInterfaceInit(btav_source_interface_t *interface)
{
    HILOGI("enter");
    interface->connect = OHOS::bluetoothMock::ConnectMock;
    interface->disconnect = OHOS::bluetoothMock::DisconnectMock;
    interface->cleanup = OHOS::bluetoothMock::CleanupMock;
    interface->set_silence_device = OHOS::bluetoothMock::SetSilenceDeviceMock;
    interface->config_codec = OHOS::bluetoothMock::ConfigCodecMock;
    interface->set_active_device = OHOS::bluetoothMock::SetActiveDeviceMock;
}

const void* NewGetProfileInterfaceMock(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        HILOGI("profileId is BT_PROFILE_ADVANCED_AUDIO_ID");
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMock;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        HILOGI("profileId is BT_PROFILE_ADVANCED_AUDIO_ID");
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback4(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback4;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback5(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback5;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback6(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback6;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback13(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback13;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback7(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback7;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback8(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback8;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback9(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback9;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback10(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback10;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback11(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback11;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback12(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback12;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback14(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback14;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}

const void* NewGetProfileInterfaceMockForCallback15(const char* profileId)
{
    HILOGI("enter");
    if (!strcmp(profileId, BT_PROFILE_ADVANCED_AUDIO_ID)) {
        btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
        if (interface != nullptr) {
            interface->init = OHOS::bluetoothMock::InitMockForConnectCallback15;
            setBtAVSourceInterfaceInit(interface);
        }
        return (void*)interface;
    }
    if (!strcmp(profileId, BT_VENDOR_INTERFACE_ID)) {
        HILOGI("profileId is BT_VENDOR_INTERFACE_ID");
        bthwif_interface_t *interface = &g_btHwIfTest;
        return (void*)interface;
    }
    return nullptr;
}
#endif