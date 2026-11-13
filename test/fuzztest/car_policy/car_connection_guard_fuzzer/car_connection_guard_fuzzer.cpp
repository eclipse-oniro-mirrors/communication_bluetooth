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

#include "car_connection_guard_fuzzer.h"

#include "car_connection_guard.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    CarConnectionGuard guard;

    // 消费 fuzz 数据驱动 FA/BtMusic 状态变化
    int faStatus = provider.ConsumeIntegralInRange<int>(-1, 1);
    int btMusicStatus = provider.ConsumeIntegralInRange<int>(-1, 1);
    guard.SetFaStatus(faStatus);
    guard.SetBtMusicStatus(btMusicStatus);

    // 消费剩余字节作为地址（可能为任意字节序列，测试空地址/非法地址健壮性）
    std::string addr = provider.ConsumeRemainingBytesAsString();

    // 遍历所有 ProfileType 测试 Check
    guard.Check(addr, ProfileType::HFP_HF);
    guard.Check(addr, ProfileType::A2DP_SINK);
    guard.Check(addr, ProfileType::PBAP_PCE);

    // SinkLimit 与互斥检查
    guard.IsConnectionAllowedBySinkLimit(addr);
    guard.IsCarPlayMutex();
    guard.IsAllowConnA2dp();
    guard.DisconnectOthersForSinkLimit(addr);
    guard.GetA2dpConnectedAddrs();
    guard.GetHfpConnectedAddrs();
    return true;
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
