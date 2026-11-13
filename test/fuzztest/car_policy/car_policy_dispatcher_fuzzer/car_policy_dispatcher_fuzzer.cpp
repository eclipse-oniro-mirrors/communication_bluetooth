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

#include "car_policy_dispatcher_fuzzer.h"

#include "car_policy_manager.h"
#include "dispatcher/car_policy_dispatcher.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
// 静态分发器:任务投递到 car policy 线程异步执行,分发器需进程级存活;
// 不调用 RegisterAll/UnregisterAll,观察者注册机制不在本 fuzzer 范围内
static CarPolicyDispatcher g_dispatcher(CarPolicyManager::GetInstance());

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    // 消费 fuzz 数据驱动立即任务/延时任务/按名取消/队列栅栏不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        switch (provider.ConsumeIntegralInRange<int>(0, 4)) {
            case 0:
                g_dispatcher.Dispatch([]() {});
                break;
            case 1: {
                // 延时任务必须提供非空 name(同名新任务替换旧任务)
                std::string name = provider.ConsumeRandomLengthString(16);
                if (name.empty()) {
                    name = "fuzz_delayed_task";
                }
                uint64_t delayMs = provider.ConsumeIntegralInRange<uint64_t>(0, 100);
                g_dispatcher.DispatchDelayed([]() {}, delayMs, name);
                break;
            }
            case 2:
                g_dispatcher.WaitQueueIdle(provider.ConsumeIntegralInRange<uint32_t>(0, 100));
                break;
            case 3: {
                // 按名取消:布防后立即取消(未到期路径),随机名命中/不命中均安全
                std::string name = provider.ConsumeRandomLengthString(16);
                if (name.empty()) {
                    name = "fuzz_cancel_task";
                }
                g_dispatcher.DispatchDelayed([]() {}, 50, name);
                g_dispatcher.CancelDelayedTask(name);
                break;
            }
            case 4:
                // 取消不存在的任务名(无该名任务时无操作)
                g_dispatcher.CancelDelayedTask(provider.ConsumeRandomLengthString(16));
                break;
            default:
                break;
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
