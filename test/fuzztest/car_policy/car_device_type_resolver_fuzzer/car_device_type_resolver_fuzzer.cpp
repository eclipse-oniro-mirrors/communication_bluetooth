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

#include "car_device_type_resolver_fuzzer.h"

#include "device/car_device_type_resolver.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    // 无状态组件,按次构造即可
    CarDeviceTypeResolver resolver;

    // 消费 fuzz 数据生成地址,测试 IsHwPhone/IsHwCarDevice 查询不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        std::string addr = provider.ConsumeRandomLengthString(32);
        if (provider.ConsumeBool()) {
            resolver.IsHwPhone(addr);
        } else {
            resolver.IsHwCarDevice(addr);
        }
    }
    return true;
}
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
