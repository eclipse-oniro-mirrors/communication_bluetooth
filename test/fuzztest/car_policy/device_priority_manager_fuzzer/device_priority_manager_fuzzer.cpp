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

#include "device_priority_manager_fuzzer.h"

#include "device_priority_manager.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    DevicePriorityManager dpm;
    dpm.Init();

    // 消费 fuzz 数据生成地址，测试 SetLastDevice/SetPenultimateDevice/Clear 系列
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        std::string addr = provider.ConsumeRandomLengthString(32);
        int op = provider.ConsumeIntegralInRange<int>(0, 4);
        switch (op) {
            case 0:
                dpm.SetLastDevice(addr);
                break;
            case 1:
                dpm.SetPenultimateDevice(addr);
                break;
            case 2:
                dpm.ClearLastDevice();
                break;
            case 3:
                dpm.ClearPenultimateDevice();
                break;
            case 4:
                dpm.GetNextDevice(addr);
                break;
            default:
                break;
        }
    }

    // 查询接口不崩溃
    dpm.GetPreferredDevice();
    dpm.GetLastDevice();
    dpm.GetPenultimateDevice();
    dpm.GetOrderList();
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
