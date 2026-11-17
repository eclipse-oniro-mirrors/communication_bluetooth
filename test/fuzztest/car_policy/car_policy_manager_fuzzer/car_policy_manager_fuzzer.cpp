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

#include "car_policy_manager_fuzzer.h"

#include "car_policy_manager.h"
#include "guard/car_connection_guard.h"
#include "reconnect/reconnect_predicates.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
// 单例不调 Init:仅调用带 initialized_ 守卫的事件入口,未初始化时直接返回。
// OnBluetoothOn/TerminateReconnect 无守卫(会解引用未初始化组件指针),不纳入。
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);

    CarPolicyManager &mgr = CarPolicyManager::GetInstance();

    // 消费 fuzz 数据驱动各事件入口不崩溃
    const int opCount = provider.ConsumeIntegralInRange<int>(1, 20);
    for (int i = 0; i < opCount; ++i) {
        std::string addr = provider.ConsumeRandomLengthString(32);
        switch (provider.ConsumeIntegralInRange<int>(0, 15)) {
            case 0:
                mgr.StartReconnect(static_cast<ReconnectReason>(
                    provider.ConsumeIntegralInRange<int>(0, 2)));
                break;
            case 1:
                mgr.OnAclDisconnect(addr, provider.ConsumeIntegralInRange<int>(0, 20));
                break;
            case 2:
                mgr.OnProfileStateChanged(addr,
                    static_cast<ProfileType>(provider.ConsumeIntegralInRange<int>(0, 2)),
                    provider.ConsumeIntegralInRange<int>(0, 4),
                    provider.ConsumeIntegralInRange<int>(0, 4));
                break;
            case 3:
                mgr.OnFaStatusChanged(provider.ConsumeIntegralInRange<int>(-1, 2));
                break;
            case 4:
                mgr.OnBtMusicStatusChanged(provider.ConsumeIntegralInRange<int>(-1, 2));
                break;
            case 5:
                mgr.OnSmartCallTransferStatusChanged(provider.ConsumeIntegralInRange<int>(-3, 10));
                break;
            case 6:
                mgr.OnHiechoNotifyOutputPath(provider.ConsumeIntegralInRange<int>(0, 9), addr);
                break;
            case 7:
                mgr.OnBondStateChanged(addr, provider.ConsumeIntegralInRange<int>(0, 12));
                break;
            case 8:
                mgr.OnBtSettingsUiChanged(provider.ConsumeIntegralInRange<int>(-1, 2));
                break;
            case 9:
                mgr.OnDiscoveryStateChanged(provider.ConsumeIntegralInRange<int>(0, 4));
                break;
            case 10:
                mgr.OnScanDeviceFound();
                break;
            case 11:
                mgr.OnCallChanged(addr, provider.ConsumeIntegralInRange<int>(0, 6));
                break;
            case 12:
                mgr.OnScoSwitchChanged(addr, provider.ConsumeIntegralInRange<int>(0, 8));
                break;
            case 13:
                mgr.OnPbapActionCompleted(addr, provider.ConsumeIntegralInRange<int>(0, 4));
                break;
            case 14:
                mgr.CancelPendingConnectOtherProfiles(addr);
                break;
            case 15:
                mgr.IsAllowConnect(addr,
                    static_cast<ProfileType>(provider.ConsumeIntegralInRange<int>(0, 2)));
                break;
            default:
                break;
        }
    }

    // 查询接口不崩溃
    mgr.IsInitialized();
    mgr.GetSessionId();
    mgr.GetBtSettingsUiActive();
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
