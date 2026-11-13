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

#include "cdc_stream_state_machine_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "cdc_stream_state_machine.h"
#include "a2dp_snk_def.h"
#include "a2dp_snk_message.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int A2DP_SNK_STREAM_EVT_MIN = A2DP_SNK_STREAM_EVT_BEGIN;
static constexpr int A2DP_SNK_STREAM_EVT_MAX = A2DP_SNK_STREAM_EVT_END;
static constexpr int AUDIO_FOCUS_HINT_MAX = 4;

static std::shared_ptr<CdcStreamStateMachine> CreateStreamStateMachine()
{
    auto streamSm = std::make_shared<CdcStreamStateMachine>();
    streamSm->Init();
    return streamSm;
}

static void TestStreamSmNotify(FuzzedDataProvider &provider)
{
    auto streamSm = CreateStreamStateMachine();
    if (streamSm == nullptr) {
        return;
    }
    bool playbackAllowed = provider.ConsumeBool();

    streamSm->NotifyUserPlayIntent();
    streamSm->NotifyUserPauseIntent();
    streamSm->NotifyAudioStreamStarted();
    streamSm->NotifyAudioStreamStopped();
    streamSm->NotifyPlaybackDisallowed();
    streamSm->NotifyA2dpConnected(playbackAllowed);
    streamSm->NotifyDeviceDisconnected();
    streamSm->NotifyDelayedPause();
    streamSm->CanPlay();
    streamSm->IsPlaying();
    streamSm->CurrentStateName();
}

static void TestStreamSmPostEvent(FuzzedDataProvider &provider)
{
    auto streamSm = CreateStreamStateMachine();
    if (streamSm == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    int32_t eventWhat = provider.ConsumeIntegralInRange<int32_t>(A2DP_SNK_STREAM_EVT_MIN, A2DP_SNK_STREAM_EVT_MAX);

    A2dpSnkMessage event(eventWhat);
    event.addr_ = addr;
    event.arg1_ = provider.ConsumeIntegral<int32_t>();
    event.arg3_ = provider.ConsumeIntegral<int32_t>();
    streamSm->PostEvent(event);
}

static void TestStreamSmFocusChange(FuzzedDataProvider &provider)
{
    auto streamSm = CreateStreamStateMachine();
    if (streamSm == nullptr) {
        return;
    }
    int32_t hint = provider.ConsumeIntegralInRange<int32_t>(0, AUDIO_FOCUS_HINT_MAX);

    streamSm->PostFocusChangeEvent(hint);
    streamSm->CanPlay();
    streamSm->CurrentStateName();
}

static void TestStreamSmLifecycle(FuzzedDataProvider &provider)
{
    auto streamSm = CreateStreamStateMachine();
    if (streamSm == nullptr) {
        return;
    }
    bool cleanupFirst = provider.ConsumeBool();
    if (cleanupFirst) {
        streamSm->Cleanup();
    } else {
        streamSm->CurrentStateName();
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("CdcStreamStateMachineFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestStreamSmNotify(provider);
    TestStreamSmPostEvent(provider);
    TestStreamSmFocusChange(provider);
    TestStreamSmLifecycle(provider);

    HILOGI("CdcStreamStateMachineFuzzTest success");
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
