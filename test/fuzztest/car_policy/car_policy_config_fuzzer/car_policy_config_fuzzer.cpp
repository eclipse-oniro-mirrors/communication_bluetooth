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

#include "car_policy_config_fuzzer.h"

#include "car_policy_config.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    // CarPolicyConfig 是纯常量类，fuzz 用于确保所有 getter 在任意输入下不崩溃
    // （虽然当前实现无输入依赖，保留 fuzzer 以便未来扩展动态配置时覆盖）
    FuzzedDataProvider provider(data, size);
    CarPolicyConfig config;

    // 消费 fuzz 数据（驱动未来可能的参数化配置）
    int unused1 = provider.ConsumeIntegral<int>();
    uint64_t unused2 = provider.ConsumeIntegral<uint64_t>();
    (void)unused1;
    (void)unused2;

    // 遍历所有 getter
    config.GetMaxRetry();
    config.GetRetryHfpTimeoutMs();
    config.GetAclTimeoutRetryDelayMs();
    config.GetBackoffTier1Ms();
    config.GetBackoffTier2Ms();
    config.GetBackoffTier3Ms();
    config.GetBackoffTier2Threshold();
    config.GetBackoffTier3Threshold();
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
