/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef BTIF_A2DP_AUDIO_RENDER_INTERFACE_H
#define BTIF_A2DP_AUDIO_RENDER_INTERFACE_H

#include <cstdint>
#include <memory>
#include <mutex>

namespace OHOS {
namespace bluetooth {

class IA2dpAudioRender {
public:
    virtual ~IA2dpAudioRender() = default;

    virtual bool CreateRender(int freq, int bits, int channels) = 0;
    virtual bool StartRender() = 0;
    virtual bool StopRender() = 0;
    virtual bool PauseRender() = 0;
    virtual bool ReleaseRender() = 0;
    virtual bool WriteStream(uint8_t *buffer, size_t size) = 0;

    static std::shared_ptr<IA2dpAudioRender> GetInstance();
    static void SetInstance(std::shared_ptr<IA2dpAudioRender> instance);

protected:
    static std::shared_ptr<IA2dpAudioRender> g_instance;
    static std::mutex g_mutex;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BTIF_A2DP_AUDIO_RENDER_INTERFACE_H
