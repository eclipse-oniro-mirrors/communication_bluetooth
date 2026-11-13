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

#ifndef AVRCp_CT_INTERFACE_MOCK_H
#define AVRCp_CT_INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include <string>

namespace OHOS {
namespace bluetooth {

class IAvrcpCtInterface {
public:
    virtual ~IAvrcpCtInterface() = default;
    virtual void SendAvrcpPlay(const std::string& deviceAddr) = 0;
    virtual void SendAvrcpPause(const std::string& deviceAddr) = 0;
    virtual void SendAvrcpStop(const std::string& deviceAddr) = 0;
};

class AvrcpCtInterfaceMock : public IAvrcpCtInterface {
public:
    MOCK_METHOD(void, SendAvrcpPlay, (const std::string&));
    MOCK_METHOD(void, SendAvrcpPause, (const std::string&));
    MOCK_METHOD(void, SendAvrcpStop, (const std::string&));
};

} // namespace bluetooth
} // namespace OHOS
#endif
