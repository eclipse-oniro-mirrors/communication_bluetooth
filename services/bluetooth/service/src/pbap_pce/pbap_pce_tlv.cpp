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

#ifndef LOG_TAG
#define LOG_TAG "bt_pbap_pce_tlv"
#endif

#include "pbap_pce_tlv.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {

TlvTriplet::TlvTriplet() : tagId_(0x00), len_(0x00), unitLen_(1) {}

TlvTriplet::TlvTriplet(const uint8_t tagId, const uint8_t len, const uint8_t *val, const uint8_t unitLen)
{
    tagId_ = tagId;
    len_ = len;
    unitLen_ = unitLen;
    val_.insert(val_.end(), val, val + len);
}

TlvTriplet::TlvTriplet(const TlvTriplet &tlvTriplet)
    : TlvTriplet(tlvTriplet.tagId_, tlvTriplet.len_, tlvTriplet.val_.data(), tlvTriplet.unitLen_)
{}

TlvTriplet &TlvTriplet::operator=(const TlvTriplet &tlvTriplet)
{
    if (this != &tlvTriplet) {
        tagId_ = tlvTriplet.tagId_;
        len_ = tlvTriplet.len_;
        unitLen_ = tlvTriplet.unitLen_;
        val_ = tlvTriplet.val_;
    }
    return *this;
}

TlvTriplet::TlvTriplet(const uint8_t tagId, const uint8_t val)
    : TlvTriplet(tagId, static_cast<uint8_t>(1), reinterpret_cast<const uint8_t *>(&val), static_cast<uint8_t>(1))
{}

TlvTriplet::TlvTriplet(const uint8_t tagId, const uint16_t val)
    : TlvTriplet(tagId, static_cast<uint8_t>(2), reinterpret_cast<const uint8_t *>(&val), static_cast<uint8_t>(2))
{}

TlvTriplet::TlvTriplet(const uint8_t tagId, const uint32_t val)
    : TlvTriplet(tagId, static_cast<uint8_t>(4), reinterpret_cast<const uint8_t *>(&val), static_cast<uint8_t>(4))
{}

TlvTriplet::TlvTriplet(const uint8_t tagId, const uint64_t val)
    : TlvTriplet(tagId, static_cast<uint8_t>(8), reinterpret_cast<const uint8_t *>(&val), static_cast<uint8_t>(8))
{}

uint8_t TlvTriplet::GetTagId() const
{
    return tagId_;
}

uint8_t TlvTriplet::GetLen() const
{
    return len_;
}

uint8_t TlvTriplet::GetUnitLen() const
{
    return unitLen_;
}

const uint8_t *TlvTriplet::GetVal() const
{
    return val_.data();
}

uint16_t TlvTriplet::GetUint16() const
{
    if (len_ == TLV_UINT16_LENGTH) {
        return (static_cast<uint16_t>(val_[0]) << 8) | val_[1];
    }
    return 0;
}

uint32_t TlvTriplet::GetUint32() const
{
    if (len_ == TLV_UINT32_LENGTH) {
        return (static_cast<uint32_t>(val_[0]) << 24) | (static_cast<uint32_t>(val_[1]) << 16) |
               (static_cast<uint32_t>(val_[2]) << 8) | val_[3];
    }
    return 0;
}

uint64_t TlvTriplet::GetUint64() const
{
    if (len_ == TLV_UINT64_LENGTH) {
        uint64_t result = 0;
        for (int i = 0; i < TLV_UINT64_LENGTH; i++) {
            result = (result << 8) | val_[i];
        }
        return result;
    }
    return 0;
}

void ObexTlvParameters::AppendTlvtriplet(const TlvTriplet &tlvTriplet)
{
    tlvTriplets_.push_back(std::make_unique<TlvTriplet>(tlvTriplet));
}

ObexTlvParameters::ObexTlvParameters(const ObexTlvParameters &other)
{
    for (const auto &tlv : other.tlvTriplets_) {
        tlvTriplets_.push_back(std::make_unique<TlvTriplet>(*tlv));
    }
}

ObexTlvParameters &ObexTlvParameters::operator=(const ObexTlvParameters &other)
{
    if (this != &other) {
        tlvTriplets_.clear();
        for (const auto &tlv : other.tlvTriplets_) {
            tlvTriplets_.push_back(std::make_unique<TlvTriplet>(*tlv));
        }
    }
    return *this;
}

const std::vector<std::unique_ptr<TlvTriplet>> &ObexTlvParameters::GetTlvTriplets() const
{
    return tlvTriplets_;
}

const TlvTriplet *ObexTlvParameters::GetTlvtriplet(const uint8_t tagId) const
{
    auto itr = std::find_if(tlvTriplets_.begin(), tlvTriplets_.end(),
        [tagId](const std::unique_ptr<TlvTriplet> &rhs) -> bool {
            return (tagId == rhs->GetTagId());
        });
    if (itr != tlvTriplets_.end()) {
        return itr->get();
    }
    return nullptr;
}

std::vector<uint8_t> ObexTlvParameters::ToBytes() const
{
    std::vector<uint8_t> result;
    for (const auto &tlv : tlvTriplets_) {
        result.push_back(tlv->GetTagId());
        result.push_back(tlv->GetLen());
        const uint8_t *val = tlv->GetVal();
        for (uint8_t i = 0; i < tlv->GetLen(); i++) {
            result.push_back(val[i]);
        }
    }
    return result;
}

std::optional<ObexTlvParameters> ObexTlvParameters::ParseFromBytes(const uint8_t *data, size_t length)
{
    if (data == nullptr || length == 0) {
        return std::nullopt;
    }
    ObexTlvParameters params;
    size_t pos = 0;
    while (pos + 1 < length) {
        uint8_t tagId = data[pos++];
        uint8_t valLen = data[pos++];
        if (pos + valLen > length) {
            return std::nullopt;
        }
        params.tlvTriplets_.push_back(
            std::make_unique<TlvTriplet>(tagId, valLen, data + pos, valLen == 0 ? 1 : valLen));
        pos += valLen;
    }
    return params;
}

}  // namespace bluetooth
}  // namespace OHOS
