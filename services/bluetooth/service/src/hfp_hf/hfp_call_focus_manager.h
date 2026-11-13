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

#ifndef HFP_CALL_FOCUS_MANAGER_H
#define HFP_CALL_FOCUS_MANAGER_H

#include <functional>
#include <memory>
#include "audio_system_manager.h"
#include "audio_interrupt_info.h"
#include "audio_interrupt_callback.h"

namespace OHOS {
namespace bluetooth {

class HfpCallFocusManager {
public:
    HfpCallFocusManager() = default;
    ~HfpCallFocusManager();

    int32_t Init(std::function<void(AudioStandard::InterruptHint)> callback);

    int32_t ActivateCallFocus();

    void DeactivateCallFocus();

    void Deinit();

    bool IsActivated() const;
    uint32_t GetSessionId() const;

private:
    AudioStandard::AudioInterrupt CreateCallAudioInterrupt();

    uint32_t sessionId_ {0};
    bool activated_ {false};
    bool callbackRegistered_ {false};
    std::shared_ptr<AudioStandard::AudioInterruptCallback> callback_ {nullptr};
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // HFP_CALL_FOCUS_MANAGER_H
