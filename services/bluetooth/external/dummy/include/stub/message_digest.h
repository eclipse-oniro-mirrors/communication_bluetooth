/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef MESSAGE_DIGEST_H
#define MESSAGE_DIGEST_H

#include <vector>
#include <string>
#include <stdint.h>

namespace stub {
// Digest type name of the MD5 message digest algorithm
const std::string DIGEST_TYPE_MD5 = "MD5";

/**
 * @brief Abstract interface of a message digest algorithm.
 *
 * Provides a unified interface for calculating message digests,
 * used by the Bluetooth service for data authentication.
 */
class MessageDigest {
public:
    /**
     * @brief Calculate the message digest of the input data.
     *
     * @param input Pointer to the input data buffer.
     * @param length Length of the input data in bytes.
     * @return Digest result as a byte array.
     */
    virtual std::vector<uint8_t> Digest(const uint8_t *input, const size_t length) = 0;
    /**
     * @brief Destroy the MessageDigest object.
     */
    virtual ~MessageDigest() = default;
};

/**
 * @brief Factory for creating message digest instances by algorithm type.
 */
class MessageDigestFactory {
public:
    /**
     * @brief Get a message digest instance of the specified algorithm type.
     *
     * @param type Algorithm type name, such as DIGEST_TYPE_MD5.
     * @return Pointer to the message digest instance,
     *         or nullptr if the type is not supported.
     */
    static MessageDigest *GetInstance(const std::string &type);
};
} // namespace stub
#endif // MESSAGE_DIGEST_H
