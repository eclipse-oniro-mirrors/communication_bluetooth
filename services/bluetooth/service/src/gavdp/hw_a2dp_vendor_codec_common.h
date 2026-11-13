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

#ifndef HW_A2DP_VENDOR_CODEC_COMMON_H
#define HW_A2DP_VENDOR_CODEC_COMMON_H

#include <cstdint>
namespace CodecCommon {
/* Vendor vendor A2DP codec "USER" value placeholders. These mirror the
 * closed-source HAL's vendor-specific codec values and are opaque to the
 * migrated service layer (only used as opaque numeric identifiers). */
/* Vendor codec common USER rate values */
constexpr uint32_t L2HCV2_USER_RATE_2MBPS = 32;
constexpr uint32_t L2HCV2_USER_RATE_LOSSLESS = 16;
constexpr uint32_t L2HCV2_USER_AUTO_RATE = 8;
constexpr uint32_t L2HCV2_USER_RATE_HIGH = 4;
constexpr uint32_t L2HCV2_USER_RATE_MID = 2;
constexpr uint32_t L2HCV2_USER_RATE_LOW = 1;
constexpr uint32_t L2HCV2_USER_RATE_DEFAULT = 0;

constexpr uint32_t L2HCST_USER_RATE_480K = 0x01 << 9;
constexpr uint32_t L2HCST_USER_RATE_256K = 0x01 << 8;
constexpr uint32_t L2HCST_USER_RATE_192K = 0x01 << 7;
constexpr uint32_t L2HCST_USER_RATE_128K = 0x01 << 6;
constexpr uint32_t L2HCST_USER_RATE_96K = 0x01 << 5;
constexpr uint32_t L2HCST_USER_RATE_LOSSLESS = 0x01 << 4;
constexpr uint32_t L2HCST_USER_AUTO_RATE = 0x01 << 3;
constexpr uint32_t L2HCST_USER_RATE_960K = 0x01 << 2;
constexpr uint32_t L2HCST_USER_RATE_640K = 0x01 << 1;
constexpr uint32_t L2HCST_USER_RATE_320K = 0x01;
constexpr uint32_t L2HCST_USER_RATE_DEFAULT = 0;
} // namespace CodecCommon
#endif  // HW_A2DP_VENDOR_CODEC_COMMON_H
