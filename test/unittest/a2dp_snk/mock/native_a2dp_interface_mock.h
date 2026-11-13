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

#ifndef NATIVE_A2DP_INTERFACE_MOCK_H
#define NATIVE_A2DP_INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include "bt_av.h"
#include "i_native_a2dp_adapter.h"

namespace OHOS {
namespace bluetooth {

class NativeA2dpInterfaceMock : public INativeA2dpAdapter {
public:
    ~NativeA2dpInterfaceMock() override = default;
    MOCK_METHOD(bt_status_t, Init, (INativeA2dpSinkCallback*, int), (override));
    MOCK_METHOD(bt_status_t, Connect, (const RawAddress&), (override));
    MOCK_METHOD(bt_status_t, Disconnect, (const RawAddress&), (override));
    MOCK_METHOD(void, Cleanup, (), (override));
    MOCK_METHOD(void, SetAudioFocusState, (int), (override));
    MOCK_METHOD(void, SetAudioTrackGain, (float), (override));
    MOCK_METHOD(bt_status_t, SetActiveDevice, (const RawAddress&), (override));
};

} // namespace bluetooth
} // namespace OHOS
#endif
