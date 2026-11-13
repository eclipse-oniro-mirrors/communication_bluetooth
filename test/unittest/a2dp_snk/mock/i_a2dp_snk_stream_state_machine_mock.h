/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef I_A2DP_SNK_STREAM_STATE_MACHINE_MOCK_H
#define I_A2DP_SNK_STREAM_STATE_MACHINE_MOCK_H

#include <gmock/gmock.h>
#include "i_a2dp_snk_stream_state_machine.h"

namespace OHOS {
namespace bluetooth {

// Design §29 / task 11.1: mock for IA2dpSnkStreamStateMachine with all 14
// interface methods. Used by tests that need to inject a stream SM mock into
// A2dpSnkService via SetStreamStateMachine().
class IA2dpSnkStreamStateMachineMock : public IA2dpSnkStreamStateMachine {
public:
    MOCK_METHOD(void, NotifyUserPlayIntent, ());
    MOCK_METHOD(void, NotifyUserPauseIntent, ());
    MOCK_METHOD(void, NotifyAudioStreamStarted, ());
    MOCK_METHOD(void, NotifyAudioStreamStopped, ());
    MOCK_METHOD(void, NotifyPlaybackDisallowed, ());
    MOCK_METHOD(void, NotifyA2dpConnected, (bool playbackAllowed));
    MOCK_METHOD(void, NotifyDeviceDisconnected, ());
    MOCK_METHOD(void, NotifyDelayedPause, ());
    MOCK_METHOD(void, PostEvent, (const A2dpSnkMessage&));
    MOCK_METHOD(bool, CanPlay, (), (const));
    MOCK_METHOD(bool, IsPlaying, (), (const));
    MOCK_METHOD(void, Cleanup, ());
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // I_A2DP_SNK_STREAM_STATE_MACHINE_MOCK_H
