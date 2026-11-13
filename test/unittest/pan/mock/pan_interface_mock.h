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
#ifndef PAN_INTERFACE_MOCK_H
#define PAN_INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include "bt_hf.h"
#include "raw_address.h"
#include "bt_pan.h"

namespace bluetooth {
namespace headset {

bt_status_t disconnect(const RawAddress* bd_addr)
{
    return BT_STATUS_SUCCESS;
}

class PanInterfaceMock {
public:
    virtual ~PanInterfaceMock() = default;
    virtual bt_status_t Connect(const RawAddress* bd_addr, int localRole, int remoteRole) = 0;
    virtual bt_status_t Disconnect(const RawAddress* bd_addr) = 0;
};

}  // namespace headset
}  // namespace bluetooth

#endif // PAN_INTERFACE_MOCK_H