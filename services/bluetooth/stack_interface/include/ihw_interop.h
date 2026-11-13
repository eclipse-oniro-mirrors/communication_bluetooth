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

#ifndef BLUETOOTH_HW_INTEROP_H
#define BLUETOOTH_HW_INTEROP_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {

/* Vendor interoperability feature bits. Values are opaque placeholders. */
constexpr uint16_t INTEROP_ONLY_PUBLIC_BLE = 111;
constexpr uint16_t INTEROP_CHANGE_PLAY_STATE_PLAY_TO_PAUSE = 125;
constexpr uint16_t INTEROP_DISABLE_DELAY_SEND_CALL_STATE = 124;
constexpr uint16_t INTEROP_DISABLE_INBAND_RING = 90;
constexpr uint16_t INTEROP_DELAY_CELLULAR_CREATE_SCO = 131;
constexpr uint16_t INTEROP_DELAY_CONNECT_VOIP_SCO = 126;
constexpr uint16_t INTEROP_DELAY_CIEV = 127;
constexpr uint16_t INTEROP_DELAY_CALL_STATE = 130;
constexpr uint16_t INTEROP_NOT_DROP_PLAY = 108;
constexpr uint16_t INTEROP_DISABLE_AUTO_CONNECT_DEVICES = 134;

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_HW_INTEROP_H
