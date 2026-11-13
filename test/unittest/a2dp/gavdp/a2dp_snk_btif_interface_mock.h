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
#ifndef A2DP_SNK_BTIF_INTERFACE_MOCK_H
#define A2DP_SNK_BTIF_INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include "bt_av.h"

namespace OHOS {
namespace bluetoothMock {
bt_status_t InitSnkMock(btav_sink_callbacks_t* callbacks, int max_connected_audio_devices)
{
    return BT_STATUS_SUCCESS;
}

/** connect to headset */
bt_status_t ConnectSnkMock(const RawAddress& bd_addr)
{
    return BT_STATUS_SUCCESS;
}

/** dis-connect from headset */
bt_status_t DisconnectSnkMock(const RawAddress& bd_addr)
{
    return BT_STATUS_SUCCESS;
}

/** Closes the interface. */
void CleanupSnkMock(void)
{
    return;
}

/** Sends Audio Focus State */
bt_status_t SetAdioFocusStateSnkMock(int focus_state)
{
    return BT_STATUS_SUCCESS;
}

/** Sets the audio track gain. */
void SetAudioTrackGainSnkMock(float gain)
{
    return;
}

/** sets the connected device as active */
bt_status_t SetActiveDeviceSnkMock(const RawAddress& bd_addr)
{
    return BT_STATUS_SUCCESS;
}
}
}
#endif