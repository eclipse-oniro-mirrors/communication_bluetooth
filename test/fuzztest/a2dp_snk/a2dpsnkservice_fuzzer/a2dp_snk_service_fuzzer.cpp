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

#include "a2dp_snk_service_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "a2dp_snk_service.h"
#include "a2dp_snk_def.h"
#include "a2dp_snk_message.h"
#include "profile_service_manager.h"
#include "bluetooth_log.h"
#include "raw_address.h"
#include "bt_def.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int A2DP_SNK_EVT_MIN = A2DP_SNK_CONN_EVT_BEGIN;
static constexpr int A2DP_SNK_EVT_MAX = A2DP_SNK_STREAM_EVT_END;
static constexpr int BT_STACK_STATE_MAX = 3;

static A2dpSnkService *GetA2dpSnkServiceInstance()
{
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_A2DP_SINK);
    return static_cast<A2dpSnkService *>(manager->GetProfileService(PROFILE_NAME_A2DP_SINK));
}

static void TestA2dpSnkServiceLifecycle(FuzzedDataProvider &provider)
{
    A2dpSnkService *service = GetA2dpSnkServiceInstance();
    if (service == nullptr) {
        return;
    }
    bool enableFirst = provider.ConsumeBool();
    if (enableFirst) {
        service->Enable();
    } else {
        service->Disable();
    }
}

static void TestA2dpSnkServiceConnect(FuzzedDataProvider &provider)
{
    A2dpSnkService *service = GetA2dpSnkServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);

    service->Connect(rawAddr);
    service->Disconnect(rawAddr);
    service->NativeConnect(rawAddr);
    service->NativeDisconnect(rawAddr);
    service->AcceptIncomingConnection(rawAddr);
    service->SetActiveSrcDevice(rawAddr);
    service->ProcessRemoveStateMachine(addr);
}

static void TestA2dpSnkServiceQuery(FuzzedDataProvider &provider)
{
    A2dpSnkService *service = GetA2dpSnkServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    int32_t strategy = provider.ConsumeIntegralInRange<int32_t>(0, 2);
    int32_t delayValue = provider.ConsumeIntegral<int32_t>();
    int playingState = 0;
    A2dpSnkCodecInfo codecInfo = {};

    service->GetDeviceState(rawAddr);
    std::vector<int32_t> states = {
        A2DP_SNK_STATE_DISCONNECTED,
        A2DP_SNK_STATE_CONNECTING,
        A2DP_SNK_STATE_CONNECTED,
        A2DP_SNK_STATE_DISCONNECTING,
    };
    service->GetDevicesByStates(states);
    service->GetPlayingState(rawAddr, playingState);
    service->SetConnectStrategy(rawAddr, strategy);
    service->GetConnectStrategy(rawAddr);
    service->SendDelay(rawAddr, delayValue);
    service->GetAudioConfig(rawAddr, codecInfo);
    service->GetConnectDevices();
    service->GetConnectState();
    service->GetMaxConnectNum();
    service->GetActiveDeviceAddr();
    service->CanPlay();
    service->GetNativeAdapter();
    HILOGI("TestA2dpSnkServiceQuery playingState=%{public}d", playingState);
}

static void TestA2dpSnkServiceProcessEvent(FuzzedDataProvider &provider)
{
    A2dpSnkService *service = GetA2dpSnkServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    int32_t eventWhat = provider.ConsumeIntegralInRange<int32_t>(A2DP_SNK_EVT_MIN, A2DP_SNK_EVT_MAX);

    A2dpSnkMessage event(eventWhat);
    event.addr_ = addr;
    event.arg1_ = provider.ConsumeIntegral<int32_t>();
    event.arg3_ = provider.ConsumeIntegral<int32_t>();
    event.retryCount_ = provider.ConsumeIntegral<int32_t>();
    service->PostEvent(event);
}

static void TestA2dpSnkServiceStackCallback(FuzzedDataProvider &provider)
{
    A2dpSnkService *service = GetA2dpSnkServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    int connState = provider.ConsumeIntegralInRange<int>(0, BT_STACK_STATE_MAX);
    int audioState = provider.ConsumeIntegralInRange<int>(0, BT_STACK_STATE_MAX);
    uint32_t sampleRate = provider.ConsumeIntegral<uint32_t>();
    uint8_t channelCount = provider.ConsumeIntegral<uint8_t>();

    service->OnNativeConnectionStateChanged(rawAddr, static_cast<btav_connection_state_t>(connState));
    service->OnNativeAudioStateChanged(rawAddr, static_cast<btav_audio_state_t>(audioState));
    service->OnNativeAudioConfigChanged(rawAddr, sampleRate, channelCount);
}

static void TestA2dpSnkServiceNotify(FuzzedDataProvider &provider)
{
    A2dpSnkService *service = GetA2dpSnkServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    int prevState = provider.ConsumeIntegralInRange<int>(0, A2DP_SNK_STATE_DISCONNECTING);
    int state = provider.ConsumeIntegralInRange<int>(0, A2DP_SNK_STATE_DISCONNECTING);
    int cause = provider.ConsumeIntegral<int32_t>();
    int focusState = provider.ConsumeIntegralInRange<int>(STATE_FOCUS_LOST, STATE_FOCUS_GRANTED);

    service->NotifyConnStateChanged(rawAddr, prevState, state, cause);
    service->NotifyUserPlayIntent();
    service->NotifyUserPauseIntent();
    service->NotifyPlaybackDisallowed();
    service->NotifyA2dpConnected(provider.ConsumeBool());
    service->NotifyDeviceDisconnected();
    service->NotifyDelayedPause();
    service->SetAudioFocusState(focusState);
    service->GetStreamStateMachine();
    service->SetStreamStateMachine(nullptr);
    service->SetNativeAdapter(nullptr);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("A2dpSnkServiceFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestA2dpSnkServiceLifecycle(provider);
    TestA2dpSnkServiceConnect(provider);
    TestA2dpSnkServiceQuery(provider);
    TestA2dpSnkServiceProcessEvent(provider);
    TestA2dpSnkServiceStackCallback(provider);
    TestA2dpSnkServiceNotify(provider);

    HILOGI("A2dpSnkServiceFuzzTest success");
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
