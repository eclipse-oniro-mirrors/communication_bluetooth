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

#ifndef A2DP_SNK_TEST_UTIL_H
#define A2DP_SNK_TEST_UTIL_H

#include <string>
#include "a2dp_snk_message.h"
#include "a2dp_snk_def.h"

namespace OHOS {
namespace bluetooth {

constexpr char TEST_DEVICE_ADDR_1[] = "00:11:22:33:44:55";
constexpr char TEST_DEVICE_ADDR_2[] = "AA:BB:CC:DD:EE:FF";

constexpr int TEST_SAMPLE_RATE_44K = 44100;
constexpr int TEST_SAMPLE_RATE_48K = 48000;
constexpr int TEST_CHANNEL_STEREO = 2;
constexpr int TEST_CHANNEL_MONO = 1;
constexpr int TEST_BITS_16 = 16;
constexpr int TEST_BITS_24 = 24;

inline A2dpSnkMessage CreateEventMsg(int what, const std::string& addr = TEST_DEVICE_ADDR_1)
{
    A2dpSnkMessage msg(what);
    msg.addr_ = addr;
    return msg;
}

} // namespace bluetooth
} // namespace OHOS
#endif
