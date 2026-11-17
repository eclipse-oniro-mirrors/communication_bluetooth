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

#include "sub_profile_switch_handler_fuzzer.h"

#include "car_policy_manager.h"
#include "sub_profile_switch_handler.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
// 账号子空间切换处理器 fuzz：单例不调 Init，组件引用保持空，
// 覆盖切号事件入口的空指针防御、查询失败 fail-safe 与断开抑制标记的随机地址输入。
// 广播/看门狗入口在 fuzz 环境查询账号服务失败走忽略分支，验证不崩溃契约。
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    CarPolicyManager &mgr = CarPolicyManager::GetInstance();
    SubProfileSwitchHandler &handler = mgr.GetSwitchHandler();

    // 消费 fuzz 数据驱动各入口不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        std::string addr = provider.ConsumeRandomLengthString(32);
        switch (provider.ConsumeIntegralInRange<int>(0, 7)) {
            case 0:
                // SWITCHED 广播转发入口：未 Init 不布防看门狗
                mgr.OnSubProfileSwitched();
                break;
            case 1:
                // user_config_update_finished 广播转发入口：查询失败走忽略分支
                mgr.OnUserConfigUpdateFinished();
                break;
            case 2:
                // 看门狗超时入口：config_ 空校验 + 查询失败 fail-safe
                handler.OnSubProfileSwitchWatchdog();
                break;
            case 3:
                // 启动基线建立入口：查询失败保持 INVALID
                handler.LoadCurrentSubProfileId();
                break;
            case 4:
                // 断开抑制判定：随机地址（含空串/超长串）
                handler.IsSwitchDisconnecting(addr);
                break;
            case 5:
                // 断开抑制消费：匹配/不匹配/重复消费路径
                handler.ConsumeSwitchDisconnect(addr);
                break;
            case 6:
                // 生命周期入口：dispatcher 空校验，重复 Uninit 安全
                handler.Uninit();
                break;
            case 7:
                // 组合场景：先消费再判定（一次性语义随机路径）
                handler.ConsumeSwitchDisconnect(addr);
                handler.IsSwitchDisconnecting(addr);
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
