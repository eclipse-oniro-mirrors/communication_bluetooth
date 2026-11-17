/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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

#include "hfphfservice_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "hfp_hf_service.h"
#include "hfp_hf_statemachine.h"
#include "hfp_hf_message.h"
#include "hfp_hf_defines.h"
#include "bluetooth_log.h"
#include "raw_address.h"
#include "profile_service_manager.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {

static constexpr int HFP_HF_CONN_EVT_MIN = HFP_HF_CONNECT_EVT;
static constexpr int HFP_HF_CONN_EVT_MAX = HFP_HF_DISCONNECT_AUDIO_EVT;
static constexpr int HFP_HF_STACK_EVT_MIN = HFP_HF_CONNECTED_EVT;
static constexpr int HFP_HF_STACK_EVT_MAX = HFP_HF_AUDIO_DISCONNECTED_EVT;

// Fuzz input bounds — named constants to avoid magic numbers.
static constexpr size_t FUZZ_STR_MAX_LEN = 16;
static constexpr int FUZZ_CALL_ID_MIN = HFP_HF_OUTGOING_CALL_ID;
static constexpr int FUZZ_CALL_ID_MAX = 5;
// Strategy bounds: maps to BTStrategyType (CONNECTION_UNKNOWN=0, CONNECTION_ALLOWED=1, CONNECTION_FORBIDDEN=2).
static constexpr int32_t FUZZ_STRATEGY_MIN = 0;
static constexpr int32_t FUZZ_STRATEGY_MAX = 2;

static HfpHfService* GetHfpHfServiceInstance()
{
    auto manager = ProfileServiceManager::GetInstance();
    std::string profileName = "HfpHfService";
    manager->SetProfileService(ADAPTER_BREDR, profileName);
    return static_cast<HfpHfService *>(manager->GetProfileService(profileName));
}

// Connect / Disconnect / Sco / State queries — all randomised.
static void TestHfpHfServiceConnect(FuzzedDataProvider &provider)
{
    HfpHfService* service = GetHfpHfServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);

    service->Connect(rawAddr);
    service->Disconnect(rawAddr);

    // Sco connect/disconnect with a separate random address.
    std::string scoAddr = BuildAddressString(provider);
    RawAddress scoRawAddr(scoAddr);
    service->ConnectSco(scoRawAddr);
    service->DisconnectSco(scoRawAddr);

    service->GetDeviceState(rawAddr);
    service->GetScoState(rawAddr);
    service->GetMaxConnectNum();
    service->GetConnectState();
    service->GetConnectDevices();
    service->IsConnected(addr);
}

// ProcessEvent with randomised event codes and payloads.
static void TestHfpHfServiceProcessEvent(FuzzedDataProvider &provider)
{
    HfpHfService* service = GetHfpHfServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);

    // Connect-event branch: iterate all events that trigger ProcessConnectEvent.
    int32_t connectEvents[] = {
        HFP_HF_CONNECT_EVT,
        HFP_HF_DISCONNECT_EVT,
        HFP_HF_CONNECTED_EVT,
        HFP_HF_DISCONNECTED_EVT,
        HFP_HF_CONNECTING_EVT,
        HFP_HF_SLC_ESTABLISHED_EVT,
    };
    for (int32_t evt : connectEvents) {
        HfpHfMessage event(evt);
        event.dev_ = addr;
        event.arg1_ = provider.ConsumeIntegral<int32_t>();
        event.arg3_ = provider.ConsumeIntegral<int32_t>();
        service->ProcessEvent(event);
    }

    // Volume event: triggers SendEventToEachStateMachine.
    HfpHfMessage volEvent(HFP_HF_SET_VOLUME_EVT);
    volEvent.dev_ = addr;
    volEvent.arg1_ = provider.ConsumeIntegral<int32_t>();
    volEvent.arg3_ = provider.ConsumeIntegral<int32_t>();
    service->ProcessEvent(volEvent);

    // Default-branch events.
    int32_t defaultEvents[] = {
        HFP_HF_CONNECT_AUDIO_EVT,
        HFP_HF_DISCONNECT_AUDIO_EVT,
        HFP_HF_INTERACTIVE_EVT,
        HFP_HF_ACCEPT_CALL_EVT,
        HFP_HF_HOLD_CALL_EVT,
        HFP_HF_REJECT_CALL_EVT,
        HFP_HF_DIAL_CALL_EVT,
        HFP_HF_OPEN_VOICE_RECOGNITION_EVT,
        HFP_HF_CLOSE_VOICE_RECOGNITION_EVT,
        HFP_HF_BATTERY_LEVEL_CHANGED_EVT,
    };
    for (int32_t evt : defaultEvents) {
        HfpHfMessage event(evt);
        event.dev_ = addr;
        event.arg1_ = provider.ConsumeIntegral<int32_t>();
        event.arg3_ = provider.ConsumeIntegral<int32_t>();
        event.type_ = provider.ConsumeIntegral<int32_t>();
        event.str_ = provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN);
        service->ProcessEvent(event);
    }

    // PostDelayEvent: delayed dispatch path.
    std::string delayAddr = BuildAddressString(provider);
    HfpHfMessage delayEvent(HFP_HF_QUERY_CURRENT_CALLS);
    delayEvent.dev_ = delayAddr;
    delayEvent.arg1_ = provider.ConsumeIntegral<int32_t>();
    service->PostDelayEvent(delayEvent, provider.ConsumeIntegral<int64_t>());
}

// State machine management + strategy — randomised.
static void TestHfpHfServiceStateMachineMgmt(FuzzedDataProvider &provider)
{
    HfpHfService* service = GetHfpHfServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);

    service->ProcessRemoveStateMachine(addr);

    // Build a randomised states list from the four HFP states.
    std::vector<int> states;
    bool s0 = provider.ConsumeBool();
    bool s1 = provider.ConsumeBool();
    bool s2 = provider.ConsumeBool();
    bool s3 = provider.ConsumeBool();
    if (s0) states.push_back(HFP_HF_STATE_DISCONNECTED);
    if (s1) states.push_back(HFP_HF_STATE_CONNECTING);
    if (s2) states.push_back(HFP_HF_STATE_DISCONNECTING);
    if (s3) states.push_back(HFP_HF_STATE_CONNECTED);
    if (states.empty()) {
        states.push_back(provider.ConsumeIntegralInRange<int>(
            HFP_HF_STATE_DISCONNECTED, HFP_HF_STATE_CONNECTED));
    }
    service->GetDevicesByStates(states);

    int connectedNum = service->GetConnectedDeviceNum();
    HILOGI("TestHfpHfServiceStateMachineMgmt connectedNum=%{public}d", connectedNum);

    // Strategy.
    int32_t strategy = provider.ConsumeIntegralInRange<int32_t>(FUZZ_STRATEGY_MIN, FUZZ_STRATEGY_MAX);
    service->SetConnectStrategy(rawAddr, strategy);
    service->GetConnectStrategy(rawAddr);
}

// Observer notifications — all randomised.
static void TestHfpHfServiceNotify(FuzzedDataProvider &provider)
{
    HfpHfService* service = GetHfpHfServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);

    int32_t state = provider.ConsumeIntegralInRange<int32_t>(
        HFP_HF_STATE_DISCONNECTED, HFP_HF_STATE_CONNECTED);
    service->NotifyStateChanged(rawAddr, state);
    service->NotifyScoStateChanged(rawAddr, provider.ConsumeIntegral<int32_t>());
    service->NotifyBatteryLevelChanged(rawAddr, provider.ConsumeIntegral<int32_t>());
    service->NotifySignalStrengthChanged(rawAddr, provider.ConsumeIntegral<int32_t>());
    service->NotifyRegistrationStatusChanged(rawAddr, provider.ConsumeIntegral<int32_t>());
    service->NotifyRoamingStatusChanged(rawAddr, provider.ConsumeIntegral<int32_t>());
    service->NotifyOperatorSelectionChanged(rawAddr, provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN));
    service->NotifySubscriberNumberChanged(rawAddr, provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN));
    service->NotifyVoiceRecognitionStatusChanged(rawAddr, provider.ConsumeIntegral<int32_t>());
    service->NotifyInBandRingTone(rawAddr, provider.ConsumeIntegral<int32_t>());
}

// Call-related APIs + voice recognition + volume + battery + lifecycle — all randomised.
static void TestHfpHfServiceCallAndLifecycle(FuzzedDataProvider &provider)
{
    HfpHfService* service = GetHfpHfServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);

    // Call APIs.
    service->SendDTMFTone(rawAddr, provider.ConsumeIntegral<uint8_t>());
    service->GetCurrentCallList(rawAddr);

    int flag = provider.ConsumeIntegralInRange<int32_t>(HFP_HF_ACCEPT_CALL_ACTION_NONE,
        HFP_HF_ACCEPT_CALL_ACTION_FINISH);
    service->AcceptIncomingCall(rawAddr, flag);
    service->HoldActiveCall(rawAddr);
    service->RejectIncomingCall(rawAddr);
    service->SendKeyPressed(rawAddr);
    service->HandleIncomingCall(rawAddr, flag);
    service->HandleMultiCall(rawAddr, flag, provider.ConsumeIntegral<int32_t>());
    service->DialLastNumber(rawAddr);
    service->DialMemory(rawAddr, provider.ConsumeIntegral<int32_t>());
    service->SendVoiceTag(rawAddr, provider.ConsumeIntegral<int32_t>());

    HandsFreeUnitCalls call(
        addr,
        provider.ConsumeIntegralInRange<int>(FUZZ_CALL_ID_MIN, FUZZ_CALL_ID_MAX),
        provider.ConsumeIntegralInRange<int>(HFP_CALL_STATE_ACTIVE, HFP_CALL_STATE_FINISHED),
        provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN),
        provider.ConsumeBool(), provider.ConsumeBool(), provider.ConsumeBool());
    service->FinishActiveCall(rawAddr, call);
    service->StartDial(rawAddr, provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN));

    // Voice recognition.
    service->OpenVoiceRecognition(rawAddr);
    service->CloseVoiceRecognition(rawAddr);

    // Volume + battery + enhanced driver safety.
    service->SetHfVolume(provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegralInRange<int32_t>(HFP_HF_VOLUME_TYPE_SPK, HFP_HF_VOLUME_TYPE_MIC));
    service->BatteryLevelChanged(provider.ConsumeIntegral<int32_t>());
    service->EnhancedDriverSafety(provider.ConsumeIntegral<int32_t>());

    // Query call + audio parameters.
    service->QueryCall(rawAddr, provider.ConsumeIntegral<int64_t>());
    service->SetAudioParameters();

    // Shutdown.
    service->ShutDownDone(provider.ConsumeBool());

    // Static helper.
    HfpHfService::ConvertStateToCause(provider.ConsumeIntegralInRange<int>(
        HFP_HF_STATE_DISCONNECTED, HFP_HF_STATE_CONNECTED));

    // Interface getters.
    service->GetBluetoothHfInterface();
    service->GetContext();

    // Lifecycle — randomise enable vs disable.
    if (provider.ConsumeBool()) {
        service->Enable();
    } else {
        service->Disable();
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("HfpHfServiceFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestHfpHfServiceConnect(provider);
    TestHfpHfServiceProcessEvent(provider);
    TestHfpHfServiceStateMachineMgmt(provider);
    TestHfpHfServiceNotify(provider);
    TestHfpHfServiceCallAndLifecycle(provider);

    HILOGI("HfpHfServiceFuzzTest success");
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }

    PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
