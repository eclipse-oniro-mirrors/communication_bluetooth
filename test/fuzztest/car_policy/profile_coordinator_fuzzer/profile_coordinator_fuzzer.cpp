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

#include "profile_coordinator_fuzzer.h"

#include "car_policy_config.h"
#include "car_policy_manager.h"
#include "coordinator/profile_coordinator.h"
#include "dispatcher/car_policy_dispatcher.h"
#include "guard/car_connection_guard.h"
#include "reconnect/device_priority_manager.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
// 静态组件图:HFP 连接成功后会经 dispatcher 投递延时补连任务并捕获 this,
// 协同器需进程级存活,避免延时回调执行时访问已销毁对象
static DevicePriorityManager g_dpm;
static CarPolicyDispatcher g_dispatcher(CarPolicyManager::GetInstance());
static CarConnectionGuard g_guard;
static ProfileCoordinator g_coordinator(g_guard, g_dpm, g_dispatcher);
static bool g_dpmInited = false;

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    if (!g_dpmInited) {
        g_dpm.Init();
        g_dpmInited = true;
    }

    // 消费 fuzz 数据驱动 HFP/A2DP/PBAP 状态变化分支不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        std::string addr = provider.ConsumeRandomLengthString(32);
        ProfileType profile = static_cast<ProfileType>(provider.ConsumeIntegralInRange<int>(0, 2));
        int nextState = provider.ConsumeIntegralInRange<int>(0, 4);
        int prevState = provider.ConsumeIntegralInRange<int>(0, 4);
        g_coordinator.OnProfileStateChanged(addr, profile, nextState, prevState);
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
