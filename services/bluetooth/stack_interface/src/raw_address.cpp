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

#include "types/raw_address.h"

#include <cstdio>
#include <cstring>

#include "securec.h"

static_assert(sizeof(RawAddress) == RawAddress::kLength, "RawAddress must be layout-compatible");

const RawAddress RawAddress::kAny = RawAddress({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
const RawAddress RawAddress::kEmpty = RawAddress({0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

RawAddress::RawAddress(const uint8_t (&addr)[kLength]) {
  errno_t err = memcpy_s(address, sizeof(address), addr, sizeof(address));
  if (err != EOK) {
    return;
  }
}

RawAddress::RawAddress(const std::array<uint8_t, kLength> array) {
  errno_t err = memcpy_s(address, sizeof(address), array.data(), kLength);
  if (err != EOK) {
    return;
  }
}

size_t RawAddress::FromOctets(const uint8_t* from) {
  errno_t err = memcpy_s(address, sizeof(address), from, kLength);
  if (err != EOK) {
    return 0;
  }
  return kLength;
}

std::string RawAddress::ToColonSepHexString() const {
  char buffer[18];
  (void)snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
      "%02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1],
      address[2], address[3], address[4], address[5]);
  return std::string(buffer);
}

std::string RawAddress::ToString() const { return ToColonSepHexString(); }

std::string RawAddress::ToStringForLogging() const { return ToColonSepHexString(); }

std::string RawAddress::ToLogString() const {
  char buffer[18];
  (void)snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
      "%02x:%02x:**:**:**:%02x", address[0], address[1], address[5]);
  return std::string(buffer);
}

std::string RawAddress::ToThreadName() const {
  char buffer[16];
  (void)snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
      "%02x%02x******%02x", address[0], address[1], address[5]);
  return std::string(buffer);
}

std::string RawAddress::ToRedactedStringForLogging() const {
  char buffer[18];
  (void)snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
      "xx:xx:xx:xx:%02x:%02x", address[4], address[5]);
  return std::string(buffer);
}

bool RawAddress::IsValidAddress(const std::string& address) {
  if (address.length() != kLength * 2 + (kLength - 1)) {
    return false;
  }
  for (size_t i = 0; i < address.length(); ++i) {
    char c = address[i];
    if (i % 3 == 2) {
      if (c != ':') {
        return false;
      }
    } else if (!isxdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  return true;
}

bool RawAddress::FromString(const std::string& from, RawAddress& to) {
  if (!IsValidAddress(from)) {
    return false;
  }
  RawAddress newaddr;
  const char* ptr = from.c_str();
  for (unsigned int i = 0; i < kLength; ++i) {
    newaddr.address[i] = static_cast<uint8_t>(std::strtoul(ptr, nullptr, 16));
    ptr += 3;
  }
  to = newaddr;
  return true;
}

bool RawAddress::ConvertAddr(RawAddress& to) const
{
  RawAddress new_addr;
  for (int i = 0; i < kLength; i++) {
    new_addr.address[i] = address[kLength -1 -i];
  }
  to = new_addr;
  return true;
}
