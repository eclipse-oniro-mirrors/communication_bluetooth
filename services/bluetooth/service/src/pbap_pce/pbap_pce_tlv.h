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

#ifndef PBAP_PCE_TLV_H
#define PBAP_PCE_TLV_H

#include <cstdint>
#include <vector>
#include <memory>
#include <algorithm>
#include <optional>

namespace OHOS {
namespace bluetooth {

constexpr uint8_t TLV_UINT8_LENGTH = 1;
constexpr uint8_t TLV_UINT16_LENGTH = 2;
constexpr uint8_t TLV_UINT32_LENGTH = 4;
constexpr uint8_t TLV_UINT64_LENGTH = 8;

class TlvTriplet {
public:
    virtual ~TlvTriplet() = default;
    explicit TlvTriplet(const uint8_t tagId, const uint8_t len, const uint8_t *val, const uint8_t unitLen = 1);
    explicit TlvTriplet(const uint8_t tagId, const uint8_t val);
    explicit TlvTriplet(const uint8_t tagId, const uint16_t val);
    explicit TlvTriplet(const uint8_t tagId, const uint32_t val);
    explicit TlvTriplet(const uint8_t tagId, const uint64_t val);
    explicit TlvTriplet(const TlvTriplet &tlvTriplet);
    TlvTriplet &operator=(const TlvTriplet &tlvTriplet);
    uint8_t GetTagId() const;
    uint8_t GetLen() const;
    uint8_t GetUnitLen() const;
    const uint8_t *GetVal() const;
    uint16_t GetUint16() const;
    uint32_t GetUint32() const;
    uint64_t GetUint64() const;

private:
    TlvTriplet();
    uint8_t tagId_ = 0;
    uint8_t len_ = 0;
    std::vector<uint8_t> val_ {};
    uint8_t unitLen_ = 1;
};

class ObexTlvParameters {
public:
    ObexTlvParameters() = default;
    virtual ~ObexTlvParameters() = default;
    ObexTlvParameters(const ObexTlvParameters &other);
    ObexTlvParameters &operator=(const ObexTlvParameters &other);
    ObexTlvParameters(ObexTlvParameters &&other) noexcept = default;
    ObexTlvParameters &operator=(ObexTlvParameters &&other) noexcept = default;
    void AppendTlvtriplet(const TlvTriplet &tlvTriplet);
    const std::vector<std::unique_ptr<TlvTriplet>> &GetTlvTriplets() const;
    const TlvTriplet *GetTlvtriplet(const uint8_t tagId) const;
    std::vector<uint8_t> ToBytes() const;
    static std::optional<ObexTlvParameters> ParseFromBytes(const uint8_t *data, size_t length);

protected:
    std::vector<std::unique_ptr<TlvTriplet>> tlvTriplets_ {};
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_TLV_H
