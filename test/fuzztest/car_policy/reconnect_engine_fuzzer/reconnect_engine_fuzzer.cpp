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

#include "reconnect_engine_fuzzer.h"

#include "car_policy_config.h"
#include "car_policy_manager.h"
#include "dispatcher/car_policy_dispatcher.h"
#include "guard/car_connection_guard.h"
#include "reconnect/device_priority_manager.h"
#include "reconnect/reconnect_engine.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
// 静态组件图:重试回连经 dispatcher 投递延时任务并捕获 this,
// 引擎需进程级存活,避免延时回调执行时访问已销毁对象
static CarPolicyConfig g_config;
static DevicePriorityManager g_dpm;
static CarPolicyDispatcher g_dispatcher(CarPolicyManager::GetInstance());
static CarConnectionGuard g_guard;
static ReconnectEngine g_engine(g_config, g_dpm, g_dispatcher, g_guard);
static bool g_dpmInited = false;

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    if (!g_dpmInited) {
        g_dpm.Init();
        g_dpmInited = true;
    }

    // 消费 fuzz 数据驱动回连触发/终止/复位及查询接口不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        std::string addr = provider.ConsumeRandomLengthString(32);
        switch (provider.ConsumeIntegralInRange<int>(0, 6)) {
            case 0:
                g_engine.StartReconnect(addr,
                    static_cast<ReconnectReason>(provider.ConsumeIntegralInRange<int>(0, 2)));
                break;
            case 1:
                g_engine.OnAclDisconnect(addr, provider.ConsumeIntegralInRange<int>(0, 20));
                break;
            case 2:
                g_engine.OnAllProfilesDisconnected(addr);
                break;
            case 3:
                g_engine.Terminate();
                break;
            case 4:
                g_engine.Reset();
                break;
            case 5:
                g_engine.GetRetryTimes();
                g_engine.IsActive();
                break;
            case 6:
                g_engine.GetCurrentDevice();
                g_engine.GetCurrentReason();
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
