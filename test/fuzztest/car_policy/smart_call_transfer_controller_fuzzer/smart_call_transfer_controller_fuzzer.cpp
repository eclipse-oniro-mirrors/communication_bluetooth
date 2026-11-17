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

#include "smart_call_transfer_controller_fuzzer.h"

#include "car_policy_config.h"
#include "car_policy_manager.h"
#include "device/car_device_type_resolver.h"
#include "dispatcher/car_policy_dispatcher.h"
#include "guard/car_connection_guard.h"
#include "reconnect/device_priority_manager.h"
#include "reconnect/reconnect_engine.h"
#include "smart_call/smart_call_transfer_controller.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
// 静态组件图:IN 状态触发的回连会经 dispatcher 投递延时任务并捕获组件指针,
// 组件需进程级存活,避免任务执行时访问已销毁对象
static CarPolicyConfig g_config;
static DevicePriorityManager g_dpm;
static CarPolicyDispatcher g_dispatcher(CarPolicyManager::GetInstance());
static CarConnectionGuard g_guard;
static ReconnectEngine g_engine(g_config, g_dpm, g_dispatcher, g_guard);
static CarDeviceTypeResolver g_resolver;
static SmartCallTransferController g_sct(g_resolver, g_dpm, g_engine);
static bool g_dpmInited = false;

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    if (!g_dpmInited) {
        g_dpm.Init();
        g_dpmInited = true;
    }

    // 消费 fuzz 数据驱动状态变化/查询接口不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        std::string addr = provider.ConsumeRandomLengthString(32);
        int op = provider.ConsumeIntegralInRange<int>(0, 2);
        switch (op) {
            case 0:
                // 覆盖 STATUS_UNKNOWN/UNSET、IN(6)、OUT(7) 及未定义状态值
                g_sct.OnStatusChanged(provider.ConsumeIntegralInRange<int>(-3, 10));
                break;
            case 1:
                g_sct.IsHwPhoneAndTransferOut(addr);
                break;
            case 2:
                g_sct.GetStatus();
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
