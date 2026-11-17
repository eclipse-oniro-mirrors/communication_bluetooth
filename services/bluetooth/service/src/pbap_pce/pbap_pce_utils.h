/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef PBAP_PCE_UTILS_H
#define PBAP_PCE_UTILS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "base_def.h"
#include "../obex/obex_def.h"
#include "../obex/obex_header.h"
#include "pbap_pce_def.h"

namespace OHOS {
namespace bluetooth {

constexpr int OBEX_DEFAULT_MTU = 1024;
// UTF-16 string uses a NUL (0x0000) terminator, which occupies 2 bytes.
constexpr uint8_t UTF16_NULL_TERMINATOR_LENGTH = 2;
constexpr uint8_t UTF16_NUL_LOW_BYTE = 0x00;
constexpr uint8_t UTF16_NUL_HIGH_BYTE = 0x00;
constexpr uint8_t UTF16_LOW_BYTE_MASK = 0xFF;
constexpr uint8_t UTF16_HIGH_BYTE_SHIFT = 8;

/// 将 u16 字符串按 OBEX NAME 头编码（UTF-16BE + NUL 终止符）写入请求头。
/// PCE 专属工具：公共 ObexHeader 不再提供 u16 NAME 公共 API（TLV 解耦整改），
/// PBAP PCE 通过裸字节与公共 obex 交互（对齐 pbap_pse / map_mse 模式）。
inline void SetHeaderNameU16(ObexHeader &header, const std::u16string &name)
{
    std::vector<uint8_t> value;
    for (char16_t c : name) {
        uint16_t v = static_cast<uint16_t>(c);
        value.push_back((v >> UTF16_HIGH_BYTE_SHIFT) & UTF16_LOW_BYTE_MASK);
        value.push_back(v & UTF16_LOW_BYTE_MASK);
    }
    value.push_back(UTF16_NUL_HIGH_BYTE);
    value.push_back(UTF16_NUL_LOW_BYTE);
    header.SetHeader(NAME, value);
}

struct PbapPceConfig {
    bool srmEnable_ = true;
    int rfcommMtu_ = OBEX_DEFAULT_MTU;
    int l2capMtu_ = OBEX_DEFAULT_MTU;
    int pceMaxDevices_ = PCE_MAXIMUM_DEVICES;
};

enum PbapPceServiceStateType {
    PBAP_PCE_STATE_STARTUP,
    PBAP_PCE_STATE_STARTUPING,
    PBAP_PCE_STATE_SHUTDOWN,
    PBAP_PCE_STATE_SHUTDOWNING,
};

enum PbapPceTargetStateType {
    PCE_TARGET_STATE_UNKNOWN,
    PCE_TARGET_STATE_CONNECTED,
    PCE_TARGET_STATE_DISCONNECTED,
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_UTILS_H
