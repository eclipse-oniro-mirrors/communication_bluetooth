/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

/**
 * @file interface_profile_a2dp_snk.h
 *
 * @brief This class provides functions called by Framework API for a2dp sink.
 *
 * @since 6
 */

#ifndef INTERFACE_PROFILE_A2DP_SNK_H
#define INTERFACE_PROFILE_A2DP_SNK_H

#include "interface_profile.h"

namespace OHOS {
namespace bluetooth {
struct A2dpSnkCodecInfo {
    // Codec priority
    uint32_t codecPriority;

    // Codec type
    uint8_t codecType;

    // Codec sample
    uint32_t sampleRate;

    // Codec bits per sample
    uint32_t bitsPerSample;

    // Codec channel mode
    uint8_t channelMode;

    // Codec specific value1
    uint64_t codecSpecific1;

    // Codec specific value2
    uint64_t codecSpecific2;

    // Codec specific value3
    uint64_t codecSpecific3;

    // Codec specific value4
    uint64_t codecSpecific4;
};
/**
 * @brief Callback function api of A2DP service, including connection, disconnection.
 *
 */
class IA2dpSnkObserver {
public:
    /**
     * @brief A destructor used to delete the A2DP Service Observer instance.
     *
     * @since 6.0
     */
    virtual ~IA2dpSnkObserver() = default;

    /**
     * @brief The callback function after device's playing state changed.
     *
     * @param device  the remote bluetooth device.
     * @param playingState  the playing state after changing.
     * @param error  the error information.
     * @since 12
     */
    virtual void OnPlayingStatusChaned(const RawAddress &device, int playingState, int error) {};

    /**
     * @brief ConnectionState Changed observer.
     * @param device bluetooth device address.
     * @param state Connection state.
     * @param cause Disconnection cause.
     * @since 12
     */
    virtual void OnConnectionStateChanged(const RawAddress &remoteAddr, int state, int cause) {};

    /**
     * @brief Media stack changed observer.
     * @param device bluetooth device address.
     * @param action Action on the device.
     * @since 10.0
     */
    virtual void OnMediaStackChanged(const RawAddress &remoteAddr, int action) {};
};

/**
 * @brief This class provides functions called by Framework API.
 *
 * @since 6.0
 */
class IProfileA2dpSnk : public IProfile {
public:
    /**
     * @brief Get a2dp source service instance.
     *
     * @return Returns an instance of a2dp source service.
     * @since 6.0
     */
    static IProfileA2dpSnk *GetSrcProfile();

    /**
     * @brief Get a2dp sink service instance.
     *
     * @return Returns an instance of a2dp sink service.
     * @since 6.0
     */
    static IProfileA2dpSnk *GetSnkProfile();

    /**
     * @brief Set target device as active device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> Target device has already been active, or perform normal setting processing.
     *         Returns <b>RET_BAD_STATUS</b> Target device is not on connected, or set fails.
     * @since 6.0
     */
    virtual int SetActiveSrcDevice(const RawAddress &device) = 0;

    virtual int Connect(const RawAddress &device) = 0;
    virtual int Disconnect(const RawAddress &device) = 0;
        /**
     * @brief Register observer function of framework.
     *
     * @param observer The observer function pointer of framework.
     * @since 6.0
     */
    virtual void RegisterObserver(IA2dpSnkObserver *observer) = 0;

    /**
     * @brief Deregister observer function of framework.
     *
     * @since 6.0
     */
    virtual void DeregisterObserver(IA2dpSnkObserver *observer) = 0;
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) = 0;
    virtual int GetDeviceState(const RawAddress &device) = 0;
    virtual int GetPlayingState(const RawAddress &device, int &state) = 0;
    virtual int SetConnectStrategy(const RawAddress &device, int32_t strategy) = 0;
    virtual int GetConnectStrategy(const RawAddress &device) = 0;
    virtual int SendDelay(const RawAddress &device, int32_t delayValue) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // INTERFACE_PROFILE_A2DP_SNK_H