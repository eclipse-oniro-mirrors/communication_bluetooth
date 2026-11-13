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
#ifndef BTIF_INTERFACE_MOCK_H
#define BTIF_INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include "a2dp_src_btif_interface_mock.h"
#include "bt_av.h"
#include "bluetooth_log.h"
#include "bt_sdp.h"

using bluetooth::Uuid;

namespace OHOS {
namespace bluetoothMock {

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMock(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMock;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback2(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback2;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback3(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback3;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback4(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback4;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback5(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback5;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback6(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback6;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback7(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback7;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback8(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback8;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback9(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback9;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback10(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback10;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback11(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback11;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback12(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback12;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback13(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback13;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

/** Get Bluetooth profile interface */
const void* GetProfileInterfaceMockForCallback14(const char* profileId)
{
    HILOGI("enter");
    btav_source_interface_t *interface = (btav_source_interface_t *)malloc(sizeof(btav_source_interface_t));
    if (interface != nullptr) {
        interface->init = InitMockForConnectCallback14;
        interface->connect = ConnectMock;
        interface->disconnect = DisconnectMock;
        interface->cleanup = CleanupMock;
        interface->set_silence_device = SetSilenceDeviceMock;
        interface->config_codec = ConfigCodecMock;
        interface->set_active_device = SetActiveDeviceMock;
    }
    return (void*)interface;
}

// sdp mock start

bt_status_t SdpInitMock(btsdp_callbacks_t* callbacks)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

bt_status_t SdpDeinitMock()
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

bt_status_t SdpSearchMock(RawAddress* bd_addr, const Uuid& uuid)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

bt_status_t SdpCreateSdpRecordMock(bluetooth_sdp_record* record, int* record_handle)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

bt_status_t SdpRemoveSdpRecordMock(int record_id)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

const void* GetSdpProfileInterfaceMock(const char* profileId)
{
    HILOGI("enter");
    btsdp_interface_t *interface = (btsdp_interface_t *)malloc(sizeof(btsdp_interface_t));
    if (interface != nullptr) {
        interface->init = SdpInitMock;
        interface->deinit = SdpDeinitMock;
        interface->sdp_search = SdpSearchMock;
        interface->create_sdp_record = SdpCreateSdpRecordMock;
        interface->remove_sdp_record = SdpRemoveSdpRecordMock;
    }
    return (void*)interface;
}
// sdp mock end
}
}
#endif