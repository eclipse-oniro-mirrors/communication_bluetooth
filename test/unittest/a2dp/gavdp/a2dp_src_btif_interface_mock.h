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
#ifndef A2DP_SRC_BTIF_INTERFACE_MOCK_H
#define A2DP_SRC_BTIF_INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include "bt_av.h"
#include "bluetooth_log.h"
#include "service_util.h"

namespace OHOS {
namespace bluetoothMock {

bt_status_t InitMock(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

__attribute__((no_sanitize("cfi")))
bt_status_t InitMockForConnectCallback(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    if (callbacks && callbacks->connection_state_cb) {
        callbacks->connection_state_cb(bluetooth::ServiceUtil::AddrToStack(device),
            BTAV_CONNECTION_STATE_CONNECTING);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback2(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    if (callbacks && callbacks->connection_state_cb) {
        callbacks->connection_state_cb(bluetooth::ServiceUtil::AddrToStack(device),
            BTAV_CONNECTION_STATE_CONNECTED);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback3(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    if (callbacks && callbacks->connection_state_cb) {
        callbacks->connection_state_cb(bluetooth::ServiceUtil::AddrToStack(device),
            BTAV_CONNECTION_STATE_CONNECTED);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback4(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    if (callbacks && callbacks->connection_state_cb) {
        callbacks->connection_state_cb(bluetooth::ServiceUtil::AddrToStack(device),
            BTAV_CONNECTION_STATE_DISCONNECTED);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback5(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    if (callbacks && callbacks->audio_state_cb) {
        callbacks->audio_state_cb(bluetooth::ServiceUtil::AddrToStack(device),
            BTAV_AUDIO_STATE_STARTED);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback6(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    if (callbacks && callbacks->audio_state_cb) {
        callbacks->audio_state_cb(bluetooth::ServiceUtil::AddrToStack(device),
            BTAV_AUDIO_STATE_STOPPED);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback7(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    btav_a2dp_codec_config_t codec_config;
    codec_config.codec_priority = BTAV_A2DP_CODEC_PRIORITY_DISABLED;
    codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_AAC;

    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;

    if (callbacks && callbacks->audio_config_cb) {
        callbacks->audio_config_cb(bluetooth::ServiceUtil::AddrToStack(device),
            codec_config, codecs_local_capabilities, codecs_selectable_capabilities);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback8(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    btav_a2dp_codec_config_t codec_config;
    codec_config.codec_priority = BTAV_A2DP_CODEC_PRIORITY_DEFAULT;
    codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_SBC;
    codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_24000;

    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;

    if (callbacks && callbacks->audio_config_cb) {
        callbacks->audio_config_cb(bluetooth::ServiceUtil::AddrToStack(device),
            codec_config, codecs_local_capabilities, codecs_selectable_capabilities);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback9(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    btav_a2dp_codec_config_t codec_config;
    codec_config.codec_priority = BTAV_A2DP_CODEC_PRIORITY_HIGHEST;
    codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_SBC;
    codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
    codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;

    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;

    if (callbacks && callbacks->audio_config_cb) {
        callbacks->audio_config_cb(bluetooth::ServiceUtil::AddrToStack(device),
            codec_config, codecs_local_capabilities, codecs_selectable_capabilities);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback10(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    btav_a2dp_codec_config_t codec_config;
    codec_config.codec_priority = BTAV_A2DP_CODEC_PRIORITY_DISABLED;
    codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_SBC;
    codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
    codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;
    codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;

    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;

    if (callbacks && callbacks->audio_config_cb) {
        callbacks->audio_config_cb(bluetooth::ServiceUtil::AddrToStack(device),
            codec_config, codecs_local_capabilities, codecs_selectable_capabilities);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback11(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    btav_a2dp_codec_config_t codec_config;
    codec_config.codec_priority = BTAV_A2DP_CODEC_PRIORITY_DISABLED;
    codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_SBC;
    codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_16000;
    codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24;
    codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;

    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;

    if (callbacks && callbacks->audio_config_cb) {
        callbacks->audio_config_cb(bluetooth::ServiceUtil::AddrToStack(device),
            codec_config, codecs_local_capabilities, codecs_selectable_capabilities);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback12(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    btav_a2dp_codec_config_t codec_config;
    codec_config.codec_priority = BTAV_A2DP_CODEC_PRIORITY_DISABLED;
    codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_SBC;
    codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_16000;
    codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32;
    codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;

    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;

    if (callbacks && callbacks->audio_config_cb) {
        callbacks->audio_config_cb(bluetooth::ServiceUtil::AddrToStack(device),
            codec_config, codecs_local_capabilities, codecs_selectable_capabilities);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback13(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    if (callbacks && callbacks->audio_state_cb) {
        callbacks->audio_state_cb(bluetooth::ServiceUtil::AddrToStack(device),
            BTAV_AUDIO_STATE_LATENCY_CHANGED);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback14(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    btav_a2dp_codec_config_t codec_config;
    codec_config.codec_priority = A2DP_CODEC_PRIORITY_L2HCST;
    codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST;
    codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_96000;
    codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24;
    codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
 
    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;
 
    if (callbacks && callbacks->audio_config_cb) {
        callbacks->audio_config_cb(bluetooth::ServiceUtil::AddrToStack(device),
            codec_config, codecs_local_capabilities, codecs_selectable_capabilities);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t InitMockForConnectCallback15(
    btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    const std::vector<btav_a2dp_codec_config_t>& offloading_preference)
{
    HILOGI("start");
    bluetooth::RawAddress device = bluetooth::RawAddress("11:22:33:44:55:66");
    btav_a2dp_codec_config_t codec_config;
    codec_config.codec_priority = A2DP_CODEC_PRIORITY_L2HCST;
    codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC;
    codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_96000;
    codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24;
    codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
 
    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;
    
    btav_a2dp_codec_config_t local_capability = codec_config;
    local_capability.sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(
        BTAV_A2DP_CODEC_SAMPLE_RATE_44100 | BTAV_A2DP_CODEC_SAMPLE_RATE_48000 | BTAV_A2DP_CODEC_SAMPLE_RATE_96000);
    local_capability.bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(
        BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24);
    codecs_local_capabilities.push_back(local_capability);
    codecs_selectable_capabilities.push_back(local_capability);
    if (callbacks && callbacks->audio_config_cb) {
        callbacks->audio_config_cb(bluetooth::ServiceUtil::AddrToStack(device),
            codec_config, codecs_local_capabilities, codecs_selectable_capabilities);
    }
    return BT_STATUS_SUCCESS;
}

/** connect to headset */
bt_status_t ConnectMock(const STACK::RawAddress& bd_addr)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

/** dis-connect from headset */
bt_status_t DisconnectMock(const STACK::RawAddress& bd_addr)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

/** sets the connected device silence state */
bt_status_t SetSilenceDeviceMock(const STACK::RawAddress& bd_addr, bool silence)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

/** sets the connected device as active */
bt_status_t SetActiveDeviceMock(const STACK::RawAddress& bd_addr)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

/** configure the codecs settings preferences */
bt_status_t ConfigCodecMock(
    const STACK::RawAddress& bd_addr,
    std::vector<btav_a2dp_codec_config_t> codec_preferences)
{
    HILOGI("start");
    return BT_STATUS_SUCCESS;
}

/** Closes the interface. */
void CleanupMock(void)
{
    return;
}
}
}
#endif