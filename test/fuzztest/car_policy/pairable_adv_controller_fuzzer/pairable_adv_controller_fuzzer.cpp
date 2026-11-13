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

#include "pairable_adv_controller_fuzzer.h"

#include "adv/pairable_adv_controller.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    // 无延时任务、不捕获 this,按次构造;未分配 handle 时析构早退安全
    PairableAdvController controller;

    // 交替 StartAdv/StopAdv,覆盖首次启动/恢复广播/暂停广播路径不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 10);
    for (int i = 0; i < opCount; ++i) {
        if (provider.ConsumeBool()) {
            controller.StartAdv();
        } else {
            controller.StopAdv();
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
