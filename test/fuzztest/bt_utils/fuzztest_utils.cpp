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

#include "fuzztest_utils.h"

namespace OHOS {
namespace Bluetooth {

    uint32_t GetU32Data(const uint8_t* data)
    {
        /*
        * Move the 0th digit to the left by 24 bits, the 1st digit to the left by 16 bits,
        * the 2nd digit to the left by 8 bits, and the 3rd digit not to the left
        */
        return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
    }

    std::string BuildAddressString(FuzzedDataProvider &provider)
    {
        std::string addr("00:00:00:00:00:00");
        char temp[18] = {0};
        int ret = sprintf_s(temp, sizeof(temp), "%02X:%02X:%02X:%02X:%02X:%02X",
            provider.ConsumeIntegral<uint8_t>(), provider.ConsumeIntegral<uint8_t>(),
            provider.ConsumeIntegral<uint8_t>(), provider.ConsumeIntegral<uint8_t>(),
            provider.ConsumeIntegral<uint8_t>(), provider.ConsumeIntegral<uint8_t>());
        if (ret != -1) {
            addr = std::string(temp);
        }
        return addr;
    }
}  // namespace Bluetooth
}  // namespace OHOS