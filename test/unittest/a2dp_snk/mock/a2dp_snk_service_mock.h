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

#ifndef A2DP_SNK_SERVICE_MOCK_H
#define A2DP_SNK_SERVICE_MOCK_H

#include <gmock/gmock.h>
#include "a2dp_snk_service.h"
#include "a2dp_snk_message.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {

class A2dpSnkServiceMock : public A2dpSnkService {
public:
    MOCK_METHOD(void, Enable, ());
    MOCK_METHOD(void, Disable, ());
    MOCK_METHOD(int, Connect, (const RawAddress&));
    MOCK_METHOD(int, Disconnect, (const RawAddress&));
    MOCK_METHOD(bool, NativeConnect, (const RawAddress&));
    MOCK_METHOD(bool, NativeDisconnect, (const RawAddress&));
    MOCK_METHOD(void, NotifyConnStateChanged, (const RawAddress&, int, int, int));
    MOCK_METHOD(bool, AcceptIncomingConnection, (const RawAddress&));
    MOCK_METHOD(void, PostEvent, (const A2dpSnkMessage&));
    MOCK_METHOD(std::shared_ptr<IA2dpSnkStreamStateMachine>, GetStreamStateMachine, (), (const));
    MOCK_METHOD(void, NotifyUserPlayIntent, ());
    MOCK_METHOD(void, NotifyUserPauseIntent, ());
    MOCK_METHOD(void, NotifyPlaybackDisallowed, ());
    MOCK_METHOD(void, NotifyA2dpConnected, (bool));
    MOCK_METHOD(void, NotifyDeviceDisconnected, ());
    MOCK_METHOD(bool, CanPlay, (), (const));
};

} // namespace bluetooth
} // namespace OHOS
#endif
