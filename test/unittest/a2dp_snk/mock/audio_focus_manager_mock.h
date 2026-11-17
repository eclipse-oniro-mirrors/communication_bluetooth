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

#ifndef AUDIO_FOCUS_MANAGER_MOCK_H
#define AUDIO_FOCUS_MANAGER_MOCK_H

#include <gmock/gmock.h>
#include "audio_focus_manager.h"

namespace OHOS {
namespace bluetooth {

class AudioFocusManagerMock : public AudioFocusManager {
public:
    AudioFocusManagerMock() : AudioFocusManager() {}
    ~AudioFocusManagerMock() override = default;

    MOCK_METHOD(int32_t, InitOnly, (std::function<void(AudioStandard::InterruptHint)>), (override));
    MOCK_METHOD(int32_t, ActivateFocus, (), (override));
    MOCK_METHOD(void, DeactivateFocus, (), (override));
    MOCK_METHOD(void, DeactivateAndDeinit, (), (override));
    MOCK_METHOD(uint32_t, GetSessionId, (), (const, (override)));
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // AUDIO_FOCUS_MANAGER_MOCK_H
