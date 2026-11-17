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

#include "a2dp_snk_state_machine_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "a2dp_snk_state_machine.h"
#include "a2dp_snk_service.h"
#include "a2dp_snk_def.h"
#include "a2dp_snk_message.h"
#include "profile_service_manager.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int A2DP_SNK_EVT_MIN = A2DP_SNK_CONN_EVT_BEGIN;
static constexpr int A2DP_SNK_EVT_MAX = A2DP_SNK_STREAM_EVT_END;

static A2dpSnkService *GetA2dpSnkServiceInstance()
{
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_A2DP_SINK);
    return static_cast<A2dpSnkService *>(manager->GetProfileService(PROFILE_NAME_A2DP_SINK));
}

static void TestStateMachineLifecycle(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    bool withService = provider.ConsumeBool();

    A2dpSnkService *service = withService ? GetA2dpSnkServiceInstance() : nullptr;
    A2dpSnkStateMachine stateMachine(addr, service);
    stateMachine.Init();

    stateMachine.GetDeviceState();
    stateMachine.GetDeviceName();
    stateMachine.GetService();
    stateMachine.NotifyStateChanged();
    stateMachine.AllowIncomingConnection(addr);
}

static void TestStateMachineDeferredMessage(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    A2dpSnkStateMachine stateMachine(addr, nullptr);
    stateMachine.Init();

    int32_t eventWhat = provider.ConsumeIntegralInRange<int32_t>(A2DP_SNK_EVT_MIN, A2DP_SNK_EVT_MAX);
    A2dpSnkMessage msg(eventWhat);
    msg.addr_ = addr;
    msg.arg1_ = provider.ConsumeIntegral<int32_t>();
    msg.arg3_ = provider.ConsumeIntegral<int32_t>();

    stateMachine.AddDeferredMessage(msg);
    stateMachine.DeferUserCommand(msg);
    stateMachine.HasDeferredMessage(eventWhat);
    stateMachine.RemoveDeferredMessages(eventWhat);
    stateMachine.ProcessDeferredMessage();
    stateMachine.ProcessDeferredMessage();
}

static void TestStateMachineTimer(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    A2dpSnkStateMachine stateMachine(addr, nullptr);
    stateMachine.Init();

    stateMachine.StartConnectionTimer();
    stateMachine.StopConnectionTimer();
    stateMachine.StopConnectionTimer();
}

static void TestStateMachineAudioConfig(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    A2dpSnkStateMachine stateMachine(addr, nullptr);
    stateMachine.Init();

    A2dpSnkCodecInfo codecInfo = {};
    codecInfo.codecPriority = provider.ConsumeIntegral<uint32_t>();
    codecInfo.codecType = provider.ConsumeIntegral<uint8_t>();
    codecInfo.sampleRate = provider.ConsumeIntegral<uint32_t>();
    codecInfo.bitsPerSample = provider.ConsumeIntegral<uint32_t>();
    codecInfo.channelMode = provider.ConsumeIntegral<uint8_t>();

    stateMachine.SetAudioConfig(codecInfo);
    A2dpSnkCodecInfo result = stateMachine.GetAudioConfig();
    stateMachine.GetSampleRate();
    stateMachine.GetChannelCount();
    HILOGI("TestStateMachineAudioConfig sampleRate=%{public}d", result.sampleRate);
}

static void TestStateMachineRemoving(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    A2dpSnkStateMachine stateMachine(addr, nullptr);
    stateMachine.Init();

    bool isRemoving = provider.ConsumeBool();
    stateMachine.SetRemoving(isRemoving);
    stateMachine.IsRemoving();
    stateMachine.NotifyStateChanged();
}

static void TestStateMachineEventName(FuzzedDataProvider &provider)
{
    int32_t what = provider.ConsumeIntegralInRange<int32_t>(A2DP_SNK_EVT_MIN, A2DP_SNK_EVT_MAX);
    std::string eventName = A2dpSnkStateMachine::GetEventName(what);
    HILOGI("TestStateMachineEventName eventName=%{public}s", eventName.c_str());
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("A2dpSnkStateMachineFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestStateMachineLifecycle(provider);
    TestStateMachineDeferredMessage(provider);
    TestStateMachineTimer(provider);
    TestStateMachineAudioConfig(provider);
    TestStateMachineRemoving(provider);
    TestStateMachineEventName(provider);

    HILOGI("A2dpSnkStateMachineFuzzTest success");
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }

    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
