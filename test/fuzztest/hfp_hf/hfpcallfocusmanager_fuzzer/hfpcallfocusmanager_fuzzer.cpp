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

#include "hfpcallfocusmanager_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#undef private
#define private public
#include "hfp_call_focus_manager.h"
#undef private
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static void FeedCallback(AudioStandard::InterruptHint hint)
{
    HILOGI("HfpCallFocusManagerFuzzTest hint=%{public}d", static_cast<int>(hint));
}

// Init / Activate / Deactivate / Deinit full lifecycle, optionally skipping init.
static void TestCallFocusManagerLifecycle(FuzzedDataProvider &provider)
{
    HfpCallFocusManager focusManager;
    bool initFirst = provider.ConsumeBool();
    if (initFirst) {
        int32_t ret = focusManager.Init(FeedCallback);
        HILOGI("HfpCallFocusManagerFuzzTest Init ret=%{public}d", ret);
    }
    focusManager.IsActivated();
    focusManager.GetSessionId();
    focusManager.ActivateCallFocus();
    focusManager.IsActivated();
    focusManager.GetSessionId();
    // Activate twice: the second call must hit the already-activated branch.
    focusManager.ActivateCallFocus();
    focusManager.DeactivateCallFocus();
    // Deactivate twice: the second call must hit the not-activated branch.
    focusManager.DeactivateCallFocus();
    focusManager.Deinit();
    focusManager.IsActivated();
    focusManager.GetSessionId();
    focusManager.Deinit();
}

// Activate a manager that was never initialized.
static void TestCallFocusManagerWithoutInit(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    HfpCallFocusManager focusManager;
    focusManager.ActivateCallFocus();
    focusManager.IsActivated();
    focusManager.GetSessionId();
    focusManager.DeactivateCallFocus();
    focusManager.Deinit();
}

// Build the AudioInterrupt from the session/pid/uid state via the private helper.
static void TestCallFocusManagerInterrupt(FuzzedDataProvider &provider)
{
    HfpCallFocusManager focusManager;
    bool initFirst = provider.ConsumeBool();
    if (initFirst) {
        focusManager.Init(FeedCallback);
    }
    AudioStandard::AudioInterrupt interrupt = focusManager.CreateCallAudioInterrupt();
    HILOGI("HfpCallFocusManagerFuzzTest interrupt sessionId=%{public}u pid=%{public}d uid=%{public}d",
        interrupt.sessionId, interrupt.pid, interrupt.uid);
}

// Repeated Init/Deinit cycles to exercise session id reuse and callback reset.
static void TestCallFocusManagerReinit(FuzzedDataProvider &provider)
{
    HfpCallFocusManager focusManager;
    int count = provider.ConsumeIntegralInRange<int>(1, 3);
    for (int i = 0; i < count; i++) {
        int32_t ret = focusManager.Init(FeedCallback);
        HILOGI("HfpCallFocusManagerFuzzTest reinit %{public}d ret=%{public}d", i, ret);
        focusManager.ActivateCallFocus();
        focusManager.GetSessionId();
        focusManager.DeactivateCallFocus();
        focusManager.Deinit();
    }
}

// Init with an empty callback: CallFocusCallbackImpl keeps a null std::function,
// OnInterrupt must tolerate it through the registered path.
static void TestCallFocusManagerInitWithEmptyCallback(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    HfpCallFocusManager focusManager;
    int32_t ret = focusManager.Init(nullptr);
    HILOGI("HfpCallFocusManagerFuzzTest Init(nullptr) ret=%{public}d", ret);
    focusManager.ActivateCallFocus();
    focusManager.DeactivateCallFocus();
    focusManager.Deinit();
}

// Init twice without Deinit: the second Init overwrites sessionId_/callback_.
static void TestCallFocusManagerDoubleInit(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    HfpCallFocusManager focusManager;
    int32_t first = focusManager.Init(FeedCallback);
    uint32_t firstSession = focusManager.GetSessionId();
    focusManager.ActivateCallFocus();
    int32_t second = focusManager.Init(FeedCallback);
    uint32_t secondSession = focusManager.GetSessionId();
    HILOGI("HfpCallFocusManagerFuzzTest double init ret=%{public}d/%{public}d session=%{public}u/%{public}u",
        first, second, firstSession, secondSession);
    focusManager.DeactivateCallFocus();
    focusManager.Deinit();
}

// Drive CallFocusCallbackImpl::OnInterrupt with fuzzed InterruptEventInternal
// through the registered callback, including after Deinit (shared_ptr keeps it alive).
static void TestCallFocusManagerInterruptCallback(FuzzedDataProvider &provider)
{
    HfpCallFocusManager focusManager;
    if (focusManager.Init(FeedCallback) != 0) {
        return;
    }
    auto registered = focusManager.callback_;
    if (registered == nullptr) {
        return;
    }
    int count = provider.ConsumeIntegralInRange<int>(1, 3);
    for (int i = 0; i < count; i++) {
        AudioStandard::InterruptEventInternal event;
        event.eventType = static_cast<AudioStandard::InterruptType>(
            provider.ConsumeIntegralInRange<int32_t>(1, 2));
        event.forceType = static_cast<AudioStandard::InterruptForceType>(
            provider.ConsumeIntegralInRange<int32_t>(0, 1));
        event.hintType = static_cast<AudioStandard::InterruptHint>(
            provider.ConsumeIntegralInRange<int32_t>(0, 11));
        event.duckVolume = provider.ConsumeFloatingPoint<float>();
        event.callbackToApp = provider.ConsumeBool();
        event.eventTimestamp = provider.ConsumeIntegral<int64_t>();
        registered->OnInterrupt(event);
    }
    focusManager.Deinit();
    // Callback invocation after Deinit must not crash.
    AudioStandard::InterruptEventInternal event;
    event.hintType = AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE;
    registered->OnInterrupt(event);
}

// Force internal states to fuzzed values to reach branches that the public
// call order cannot produce (activate/Deinit with inconsistent flags).
static void TestCallFocusManagerAbnormalStates(FuzzedDataProvider &provider)
{
    HfpCallFocusManager focusManager;
    uint32_t fuzzSession = provider.ConsumeIntegral<uint32_t>();
    bool fuzzActivated = provider.ConsumeBool();
    bool fuzzRegistered = provider.ConsumeBool();

    // Activate with inconsistent state: registered-but-zero-session / unregistered-nonzero-session.
    focusManager.sessionId_ = fuzzSession;
    focusManager.activated_ = fuzzActivated;
    focusManager.callbackRegistered_ = fuzzRegistered;
    focusManager.ActivateCallFocus();
    focusManager.IsActivated();
    focusManager.GetSessionId();

    // Deactivate/Deinit with activated_ true but callback unregistered.
    focusManager.sessionId_ = (fuzzSession == 0) ? 1 : fuzzSession;
    focusManager.activated_ = true;
    focusManager.callbackRegistered_ = false;
    focusManager.DeactivateCallFocus();
    focusManager.Deinit();

    // Deinit early-return branch: sessionId_ == 0 while activated_ is true.
    focusManager.sessionId_ = 0;
    focusManager.activated_ = true;
    focusManager.callbackRegistered_ = fuzzRegistered;
    focusManager.Deinit();
    focusManager.IsActivated();
    focusManager.GetSessionId();
}

// Two coexisting managers must keep independent session/callback state.
static void TestCallFocusManagerMultipleInstances(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    HfpCallFocusManager first;
    HfpCallFocusManager second;
    first.Init(FeedCallback);
    second.Init(FeedCallback);
    first.ActivateCallFocus();
    second.ActivateCallFocus();
    HILOGI("HfpCallFocusManagerFuzzTest multi instance session=%{public}u/%{public}u activated=%{public}d/%{public}d",
        first.GetSessionId(), second.GetSessionId(),
        static_cast<int>(first.IsActivated()), static_cast<int>(second.IsActivated()));
    first.DeactivateCallFocus();
    second.DeactivateCallFocus();
    second.Deinit();
    first.Deinit();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("HfpCallFocusManagerFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestCallFocusManagerLifecycle(provider);
    TestCallFocusManagerWithoutInit(provider);
    TestCallFocusManagerInterrupt(provider);
    TestCallFocusManagerReinit(provider);
    TestCallFocusManagerInitWithEmptyCallback(provider);
    TestCallFocusManagerDoubleInit(provider);
    TestCallFocusManagerInterruptCallback(provider);
    TestCallFocusManagerAbnormalStates(provider);
    TestCallFocusManagerMultipleInstances(provider);

    HILOGI("HfpCallFocusManagerFuzzTest success");
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }

    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
