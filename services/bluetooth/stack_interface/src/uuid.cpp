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

#include "types/bluetooth/uuid.h"
#include "securec.h"
#include <cstring>
#include <algorithm>
#include <random>

namespace bluetooth {

static_assert(sizeof(Uuid) == 16, "Uuid must be 16 bytes !");

using UUID128Bit = Uuid::UUID128Bit;

const Uuid Uuid::kEmpty = Uuid::From128BitBE(UUID128Bit{{0x00}});

bool Uuid::Is16Bit() const {
  return GetShortestRepresentationSize() == kNumBytes16;
}
namespace {
// 蓝牙标准 Base UUID（即 00000000-0000-1000-8000-00805F9B34FB），
// 所有 16/32 位蓝牙 UUID 均由该 Base UUID 扩展而来，用于判断 UUID 的最短表示形式
constexpr Uuid kBase = Uuid::From128BitBE(
    UUID128Bit{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00,
                0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb}});

} // namespace

// 获取 UUID 的最短表示长度：若高12字节与 Base UUID 不同则须用完整128位表示，
// 否则再根据高2字节是否为0进一步判断可用16位或32位表示
size_t Uuid::GetShortestRepresentationSize() const {
  // 比较 UUID 的第4~15字节与 Base UUID 对应字节，任一不同则说明是完整128位 UUID
  if (memcmp(uu.data() + kNumBytes32, kBase.uu.data() + kNumBytes32,
             kNumBytes128 - kNumBytes32) != 0) {
    return kNumBytes128; // 高12字节与 Base UUID 不一致，必须使用128位表示
  }

  if (uu[0] == 0 && uu[1] == 0) {
    return kNumBytes16;
  } 

  return kNumBytes32;
}

uint32_t Uuid::As32Bit() const {
  return (((uint32_t)uu[0]) << 24) + (((uint32_t)uu[1]) << 16) +
         (((uint32_t)uu[2]) << 8) + uu[3];
}

uint16_t Uuid::As16Bit() const { return (((uint16_t)uu[2]) << 8) + uu[3]; }

Uuid Uuid::FromString(const std::string& uu_id, bool* is_valid) {
  if (is_valid) *is_valid = false;
  Uuid ret = kBase;

  if (uu_id.empty()) return ret;

  uint8_t* p = ret.uu.data();
  if (uu_id.size() == kString128BitLen) {
    if (uu_id[8] != '-' || uu_id[13] != '-' || uu_id[18] != '-' ||
        uu_id[23] != '-') {
      return ret;
    }

    int c;
    int rcc =
        sscanf_s(uu_id.c_str(),
               "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx"
               "-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%n",
               &p[0], &p[1], &p[2], &p[3], &p[4], &p[5], &p[6], &p[7], &p[8],
               &p[9], &p[10], &p[11], &p[12], &p[13], &p[14], &p[15], &c);
    if (rcc != 16) return ret;
    if (c != kString128BitLen) return ret;

    if (is_valid) *is_valid = true;
  } else if (uu_id.size() == 8) {
    int c;
    int rcc = sscanf_s(uu_id.c_str(), "%02hhx%02hhx%02hhx%02hhx%n", &p[0], &p[1],
                    &p[2], &p[3], &c);
    if (rcc != 4) return ret;
    if (c != 8) return ret;

    if (is_valid) *is_valid = true;
  } else if (uu_id.size() == 4) {
    int c;
    int rcc = sscanf_s(uu_id.c_str(), "%02hhx%02hhx%n", &p[2], &p[3], &c);
    if (rcc != 2) return ret;
    if (c != 4) return ret;

    if (is_valid) *is_valid = true;
  }

  return ret;
}

Uuid Uuid::From32Bit(uint32_t uuid_32) {
  Uuid u = kBase;

  u.uu[0] = (uint8_t)((0xFF000000 & uuid_32) >> 24);
  u.uu[1] = (uint8_t)((0x00FF0000 & uuid_32) >> 16);
  u.uu[2] = (uint8_t)((0x0000FF00 & uuid_32) >> 8);
  u.uu[3] = (uint8_t)(0x000000FF & uuid_32);
  return u;
}

Uuid Uuid::From16Bit(uint16_t uuid_16) {
  Uuid u = kBase;

  u.uu[2] = (uint8_t)((0xFF00 & uuid_16) >> 8);
  u.uu[3] = (uint8_t)(0x00FF & uuid_16);
  return u;
}

Uuid Uuid::From128BitBE(const uint8_t* uu_id) {
  UUID128Bit tmp;
  errno_t err = memcpy_s(tmp.data(), tmp.size(), uu_id, kNumBytes128);
  if (err != EOK) {
    return Uuid::kEmpty;
  }
  return From128BitBE(tmp);
}

Uuid Uuid::From128BitLE(const UUID128Bit& uu_id) {
  Uuid u;
  std::reverse_copy(uu_id.data(), uu_id.data() + kNumBytes128, u.uu.begin());
  return u;
}

const UUID128Bit Uuid::To128BitLE() const {
  UUID128Bit le;
  std::reverse_copy(uu.data(), uu.data() + kNumBytes128, le.begin());
  return le;
}

Uuid Uuid::From128BitLE(const uint8_t* uu_id) {
  UUID128Bit tmp;
  errno_t err = memcpy_s(tmp.data(), tmp.size(), uu_id, kNumBytes128);
  if (err != EOK) {
    return Uuid::kEmpty;
  }
  return From128BitLE(tmp);
}

const UUID128Bit& Uuid::To128BitBE() const { return uu; }

Uuid Uuid::GetRandom() {
  std::array<uint8_t, kNumBytes128> let = {};
  for (auto& b : let) {
    b = static_cast<uint8_t>(rand() & 0xFF);
  }
  return Uuid::From128BitLE(let);
}

bool Uuid::IsEmpty() const { return *this == kEmpty; }

void Uuid::UpdateUuid(const Uuid& uu_id) {
  uu = uu_id.uu;
}

bool Uuid::operator==(const Uuid& rhs) const { return uu == rhs.uu; }

bool Uuid::operator!=(const Uuid& rhs) const { return uu != rhs.uu; }

bool Uuid::operator<(const Uuid& rhs) const {
  return std::lexicographical_compare(uu.begin(), uu.end(), rhs.uu.begin(),
                                      rhs.uu.end());
}

std::string Uuid::ToString() const {
  // 36 = 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12，其中 1 为连字符
  constexpr size_t UUID_STRING_MAX_LEN = 36;
  char token[UUID_STRING_MAX_LEN + 1] = {0};  // +1 for '\0'

  // 本环境 sprintf_s 为 Annex K 语义（成功返回写入长度），
  // 统一以产出内容为准，不依赖返回值语义
  (void)sprintf_s(token, sizeof(token),
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      uu[0], uu[1], uu[2], uu[3], uu[4], uu[5], uu[6], uu[7],
      uu[8], uu[9], uu[10], uu[11], uu[12], uu[13], uu[14], uu[15]);

  return std::string(token);
}



}  // namespace bluetooth
