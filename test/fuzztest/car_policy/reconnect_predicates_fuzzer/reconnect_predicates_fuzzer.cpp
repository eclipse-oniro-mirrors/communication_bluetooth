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

#include "reconnect_predicates_fuzzer.h"

#include "car_policy_config.h"
#include "car_policy_manager.h"
#include "device/car_device_type_resolver.h"
#include "dispatcher/car_policy_dispatcher.h"
#include "guard/car_connection_guard.h"
#include "reconnect/device_priority_manager.h"
#include "reconnect/reconnect_engine.h"
#include "reconnect/reconnect_predicates.h"
#include "smart_call/smart_call_transfer_controller.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
// 静态组件图:谓词持有的引用(sct/guard/mgr)需与其依赖组件同为进程级存活
static CarPolicyConfig g_config;
static DevicePriorityManager g_dpm;
static CarPolicyDispatcher g_dispatcher(CarPolicyManager::GetInstance());
static CarConnectionGuard g_guard;
static ReconnectEngine g_engine(g_config, g_dpm, g_dispatcher, g_guard);
static CarDeviceTypeResolver g_resolver;
static SmartCallTransferController g_sct(g_resolver, g_dpm, g_engine);
static BtSettingsUiPredicate g_btUiPredicate(CarPolicyManager::GetInstance());
static SmartCallOutPredicate g_smartCallOutPredicate(g_sct);
static CarplayMutexPredicate g_carplayPredicate(g_guard);

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    // 无依赖的谓词按次构造
    IncomingConnPredicate incomingPredicate;

    // 轮询驱动 4 个谓词的 ShouldTerminate/GetName/OnTerminate 不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        std::string addr = provider.ConsumeRandomLengthString(32);
        IReconnectPredicate *predicate = nullptr;
        switch (provider.ConsumeIntegralInRange<int>(0, 3)) {
            case 0:
                predicate = &incomingPredicate;
                break;
            case 1:
                predicate = &g_btUiPredicate;
                break;
            case 2:
                predicate = &g_smartCallOutPredicate;
                break;
            case 3:
                predicate = &g_carplayPredicate;
                break;
        }
        predicate->ShouldTerminate(addr, provider.ConsumeIntegralInRange<int>(0, 10));
        predicate->GetName();
        predicate->OnTerminate();
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
