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

#ifndef IA2DP_AUDIO_RENDER_MOCK_H
#define IA2DP_AUDIO_RENDER_MOCK_H

#include <gmock/gmock.h>
#include "btif/a2dp_audio_render_interface.h"

namespace OHOS {
namespace bluetooth {

class IA2dpAudioRenderMock : public IA2dpAudioRender {
public:
    ~IA2dpAudioRenderMock() override = default;

    MOCK_METHOD(bool, CreateRender, (int freq, int bits, int channels), (override));
    MOCK_METHOD(bool, StartRender, (), (override));
    MOCK_METHOD(bool, StopRender, (), (override));
    MOCK_METHOD(bool, PauseRender, (), (override));
    MOCK_METHOD(bool, ReleaseRender, (), (override));
    MOCK_METHOD(bool, WriteStream, (uint8_t* buffer, size_t size), (override));
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // IA2DP_AUDIO_RENDER_MOCK_H
