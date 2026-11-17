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

#include "hfphfstatemachine_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#undef private
#define private public
#include "hfp_hf_statemachine.h"
#include "hfp_hf_service.h"
#include "hfp_hf_message.h"
#include "hfp_hf_call_manager.h"
#include "hfp_hf_defines.h"
#include "hands_free_unit_calls.h"
#include "bluetooth_log.h"
#include "raw_address.h"
#include "profile_service_manager.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {

static constexpr int32_t SM_EVT_MIN = HFP_HF_CONNECT_EVT;
static constexpr int32_t SM_EVT_MAX = HFP_HF_SEND_AT_COMMAND_EVT;
static constexpr int32_t INTERACTIVE_TYPE_MIN = HFP_HF_TYPE_NONE;
static constexpr int32_t INTERACTIVE_TYPE_MAX = HFP_HF_TYPE_VOICE_RECOGNITION_CHANGED;

// Fuzz input bounds — named constants to avoid magic numbers.
static constexpr size_t FUZZ_STR_MAX_LEN = 16;
static constexpr int FUZZ_CALL_ID_MIN = HFP_HF_OUTGOING_CALL_ID - 1;
static constexpr int FUZZ_CALL_ID_MAX = 10;
static constexpr int32_t FUZZ_AG_FEATURE_MASK_MAX = 0xFFFF;
static constexpr int FUZZ_DEFER_COUNT_MIN = 1;
static constexpr int FUZZ_DEFER_COUNT_MAX = 5;
static constexpr int32_t FUZZ_DISPATCH_COUNT_MIN = 1;
static constexpr int32_t FUZZ_DISPATCH_COUNT_MAX = 8;

static HfpHfService* GetHfpHfServiceInstance()
{
    auto manager = ProfileServiceManager::GetInstance();
    std::string profileName = "HfpHfService";
    manager->SetProfileService(ADAPTER_BREDR, profileName);
    return static_cast<HfpHfService *>(manager->GetProfileService(profileName));
}

static HfpHfMessage BuildMessage(FuzzedDataProvider &provider, int32_t what)
{
    HfpHfMessage event(what);
    event.dev_ = BuildAddressString(provider);
    event.arg1_ = provider.ConsumeIntegral<int32_t>();
    event.arg3_ = provider.ConsumeIntegral<int32_t>();
    event.type_ = provider.ConsumeIntegralInRange<int32_t>(INTERACTIVE_TYPE_MIN, INTERACTIVE_TYPE_MAX);
    event.str_ = provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN);
    HfpHfCurrentCallData callData {};
    callData.index = provider.ConsumeIntegral<uint16_t>();
    callData.dir = provider.ConsumeIntegral<uint16_t>();
    callData.status = provider.ConsumeIntegral<uint16_t>();
    callData.mode = provider.ConsumeIntegral<uint16_t>();
    callData.mprty = provider.ConsumeIntegral<uint16_t>();
    callData.number = provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN);
    callData.type = provider.ConsumeIntegral<uint16_t>();
    event.call_ = callData;
    return event;
}

// Getters + lifecycle + connect flow + timeout — all randomised.
static void TestStateMachineCore(FuzzedDataProvider &provider, HfpHfStateMachine &sm)
{
    // Simple getters.
    sm.IsInbandRing();
    sm.GetVolume();
    sm.GetDeviceState();
    sm.GetVoiceRecognitionState();
    sm.address_;
    sm.GetCurrentCallList();

    // Removing flag — randomise both values.
    sm.IsRemoving();
    sm.SetRemoving(provider.ConsumeBool());
    sm.SetRemoving(!provider.ConsumeBool());

    // Static event-name helpers — drive with random event codes.
    int evtCode = provider.ConsumeIntegralInRange<int32_t>(SM_EVT_MIN, SM_EVT_MAX);
    HfpHfStateMachine::GetEventName(evtCode);
    HfpHfStateMachine::GetEventName(HFP_HF_INTERACTIVE_EVT);
    HfpHfStateMachine::GetConnectEventName(evtCode);

    // Timer helpers.
    sm.StartConnectionTimer();
    sm.StopConnectionTimer();

    // State transition notification — randomise from/to states.
    int fromState = provider.ConsumeIntegralInRange<int32_t>(
        HFP_HF_STATE_DISCONNECTED, HFP_HF_AUDIO_STATE_CONNECTED);
    int toState = provider.ConsumeIntegralInRange<int32_t>(
        HFP_HF_STATE_DISCONNECTED, HFP_HF_AUDIO_STATE_CONNECTED);
    sm.NotifyStateTransitions();
    sm.NotifyChildStateToParentState(fromState, toState);

    // Audio parameters.
    sm.SetAudioWbs(provider.ConsumeBool());
    sm.SetAudioConnectedParameters();
    sm.ProcessAudioDisconnected();

    // Connect / disconnect flow.
    sm.ProcessConnectEvent();
    sm.ProcessDisconnectEvent();
    sm.ProcessConnectScoEvent();
    sm.ProcessDisconnectScoEvent();
    sm.IsAllowAgConnected(BuildAddressString(provider));

    // Timeout callback.
    sm.ConnectionTimeout();
}

// Call handling + volume + SLC established — all randomised.
static void TestStateMachineCallAndAudio(FuzzedDataProvider &provider, HfpHfStateMachine &sm)
{
    // Accept call — iterate all flags, plus a random one.
    sm.ProcessAcceptCall(HFP_HF_ACCEPT_CALL_ACTION_NONE);
    sm.ProcessAcceptCall(HFP_HF_ACCEPT_CALL_ACTION_HOLD);
    sm.ProcessAcceptCall(HFP_HF_ACCEPT_CALL_ACTION_FINISH);
    sm.ProcessAcceptCall(provider.ConsumeIntegralInRange<int32_t>(
        HFP_HF_ACCEPT_CALL_ACTION_NONE, HFP_HF_ACCEPT_CALL_ACTION_FINISH));

    sm.ProcessHoldCall();
    sm.ProcessRejectCall();
    sm.PrecessSendKeyPressed();
    sm.ProcessOpenVoiceRecognition();
    sm.ProcessCloseVoiceRecognition();

    int batteryLevel = provider.ConsumeIntegral<int32_t>();
    sm.ProcessBatteryLevelUpdate(batteryLevel);

    int dtmfFlag = provider.ConsumeIntegral<int32_t>();
    sm.PrecessSendDtmf(dtmfFlag);

    // ProcessFinishActiveCall — randomised call.
    HfpHfMessage finishEvent(HFP_HF_FINISH_CALL_EVT);
    finishEvent.dev_ = sm.address_;
    HandsFreeUnitCalls finishCall(
        sm.address_,
        provider.ConsumeIntegralInRange<int>(FUZZ_CALL_ID_MIN, FUZZ_CALL_ID_MAX),
        provider.ConsumeIntegralInRange<int>(HFP_CALL_STATE_ACTIVE, HFP_CALL_STATE_FINISHED),
        provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN),
        provider.ConsumeBool(), provider.ConsumeBool(), provider.ConsumeBool());
    finishEvent.calls_ = finishCall;
    sm.ProcessFinishActiveCall(finishEvent);

    // ProcessDialCall — randomised call.
    HfpHfMessage dialEvent(HFP_HF_DIAL_CALL_EVT);
    dialEvent.dev_ = sm.address_;
    HandsFreeUnitCalls dialCall(
        sm.address_,
        provider.ConsumeIntegralInRange<int>(FUZZ_CALL_ID_MIN, FUZZ_CALL_ID_MAX),
        provider.ConsumeIntegralInRange<int>(HFP_CALL_STATE_ACTIVE, HFP_CALL_STATE_FINISHED),
        provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN),
        provider.ConsumeBool(), provider.ConsumeBool(), provider.ConsumeBool());
    dialEvent.calls_ = dialCall;
    sm.ProcessDialCall(dialEvent);

    // Volume — both types, randomised values.
    int vol1 = provider.ConsumeIntegral<int32_t>();
    int vol2 = provider.ConsumeIntegral<int32_t>();
    int volType = provider.ConsumeIntegralInRange<int32_t>(HFP_HF_VOLUME_TYPE_SPK, HFP_HF_VOLUME_TYPE_MIC);
    sm.ProcessSetHfVolume(vol1, HFP_HF_VOLUME_TYPE_SPK);
    sm.ProcessSetHfVolume(vol2, HFP_HF_VOLUME_TYPE_MIC);
    sm.ProcessSetHfVolume(provider.ConsumeIntegral<int32_t>(), volType);

    // SLC Established — with ECS feature.
    HfpHfMessage slcEvent(HFP_HF_SLC_ESTABLISHED_EVT);
    slcEvent.dev_ = sm.address_;
    slcEvent.arg1_ = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_AG_FEATURE_MASK_MAX) |
        HFP_HF_AG_FEATURES_ENHANCED_CALL_STATUS;
    slcEvent.arg3_ = provider.ConsumeIntegral<int32_t>();
    sm.ProcessSlcEstablished(slcEvent);

    // SLC Established — without ECS (triggers disconnect branch).
    HfpHfMessage slcEventNoEcs(HFP_HF_SLC_ESTABLISHED_EVT);
    slcEventNoEcs.dev_ = sm.address_;
    slcEventNoEcs.arg1_ = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_AG_FEATURE_MASK_MAX) &
        ~static_cast<int32_t>(HFP_HF_AG_FEATURES_ENHANCED_CALL_STATUS);
    slcEventNoEcs.arg3_ = provider.ConsumeIntegral<int32_t>();
    sm.ProcessSlcEstablished(slcEventNoEcs);
}

// Interactive events + AT command result + deferred messages.
static void TestStateMachineEvents(FuzzedDataProvider &provider, HfpHfStateMachine &sm)
{
    // ProcessInteractiveEvent — iterate all interactive types with randomised payloads.
    for (int32_t type = INTERACTIVE_TYPE_MIN; type <= INTERACTIVE_TYPE_MAX; type++) {
        HfpHfMessage event(HFP_HF_INTERACTIVE_EVT);
        event.dev_ = sm.address_;
        event.arg1_ = provider.ConsumeIntegral<int32_t>();
        event.arg3_ = provider.ConsumeIntegral<int32_t>();
        event.type_ = type;
        event.str_ = provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN);
        HfpHfCurrentCallData callData {};
        callData.index = provider.ConsumeIntegral<uint16_t>();
        callData.dir = provider.ConsumeIntegral<uint16_t>();
        callData.status = provider.ConsumeIntegral<uint16_t>();
        callData.mode = provider.ConsumeIntegral<uint16_t>();
        callData.mprty = provider.ConsumeIntegral<uint16_t>();
        callData.number = provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN);
        callData.type = provider.ConsumeIntegral<uint16_t>();
        event.call_ = callData;
        // ProcessAtCommandResult calls queuedActions.front() without checking emptiness.
        // Push a valid action when type_ is AT_CMD_RESULT to avoid UB on empty queue.
        if (type == HFP_HF_TYPE_AT_CMD_RESULT) {
            sm.queuedActions.push(HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE);
        }
        sm.ProcessInteractiveEvent(event);
    }

    // ProcessAtCommandResult — push each action into queuedActions so the switch branch hits.
    int32_t actions[] = {
        HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE,
        HFP_HF_OPEN_VOICE_RECOGNITION_EVT,
        HFP_HF_CLOSE_VOICE_RECOGNITION_EVT,
        HFP_HF_DIAL_CALL_EVT,
        HFP_HF_FINISH_CALL_EVT,
        HFP_HF_REJECT_CALL_EVT,
        HFP_HF_SET_VOLUME_EVT,
    };
    for (int32_t action : actions) {
        sm.queuedActions.push(action);
        HfpHfMessage event(HFP_HF_INTERACTIVE_EVT);
        event.dev_ = sm.address_;
        event.type_ = HFP_HF_TYPE_AT_CMD_RESULT;
        event.arg1_ = provider.ConsumeIntegral<int32_t>();
        sm.ProcessAtCommandResult(event);
    }

    // Deferred messages — randomise count and what.
    int deferCount = provider.ConsumeIntegralInRange<int>(FUZZ_DEFER_COUNT_MIN, FUZZ_DEFER_COUNT_MAX);
    for (int i = 0; i < deferCount; i++) {
        int32_t what = provider.ConsumeIntegralInRange<int32_t>(SM_EVT_MIN, SM_EVT_MAX);
        HfpHfMessage msg(what);
        msg.dev_ = sm.address_;
        msg.arg1_ = provider.ConsumeIntegral<int32_t>();
        sm.AddDeferredMessage(msg);
    }
    sm.ProcessDeferredMessage();
}

// Drive the state machine through ProcessMessage with random events.
static void TestStateMachineDispatch(FuzzedDataProvider &provider, HfpHfStateMachine &sm)
{
    int32_t evtCount = provider.ConsumeIntegralInRange<int32_t>(FUZZ_DISPATCH_COUNT_MIN, FUZZ_DISPATCH_COUNT_MAX);
    for (int i = 0; i < evtCount; i++) {
        int32_t what = provider.ConsumeIntegralInRange<int32_t>(SM_EVT_MIN, SM_EVT_MAX);
        HfpHfMessage event = BuildMessage(provider, what);
        // When dispatched via ProcessMessage → ProcessInteractiveEvent → ProcessAtCommandResult,
        // queuedActions.front() is called without emptiness check. Push a dummy action to avoid UB.
        if (what == HFP_HF_INTERACTIVE_EVT && event.type_ == HFP_HF_TYPE_AT_CMD_RESULT) {
            sm.queuedActions.push(HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE);
        }
        sm.ProcessMessage(event);
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("HfpHfStateMachineFuzzTest start");
    FuzzedDataProvider provider(data, size);

    // Ensure the service is registered so HfpHfService::GetService() inside the SM is non-null.
    GetHfpHfServiceInstance();

    std::string addr = BuildAddressString(provider);
    HfpHfStateMachine sm(addr);
    sm.Init();
    sm.calls_ = std::make_unique<HfpHfCallManager>(addr);

    TestStateMachineCore(provider, sm);
    TestStateMachineCallAndAudio(provider, sm);
    TestStateMachineEvents(provider, sm);
    TestStateMachineDispatch(provider, sm);

    HILOGI("HfpHfStateMachineFuzzTest success");
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
