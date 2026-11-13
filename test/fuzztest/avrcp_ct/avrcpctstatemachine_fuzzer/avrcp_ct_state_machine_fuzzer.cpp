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

#include "avrcp_ct_state_machine_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_state_machine.h"
#include "avrcp_ct_service.h"
#include "avrcp_ct_message.h"
#include "avrcp_ct_define.h"
#include "profile_service_manager.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int AVRCP_CT_EVT_MIN = AVRCP_CT_MSG_INTERNAL_ABS_VOL_TIMEOUT;
static constexpr int AVRCP_CT_EVT_MAX = AVRCP_CT_MSG_PROCESS_IMAGE_DOWNLOADED;

static std::shared_ptr<AvrcpCtStateMachine> CreateAvrcpCtStateMachine(const std::string &addr)
{
    auto stateMachine = std::make_shared<AvrcpCtStateMachine>(addr);
    stateMachine->Init();
    return stateMachine;
}

static void TestAvrcpCtSmLifecycle(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    auto stateMachine = CreateAvrcpCtStateMachine(addr);
    if (stateMachine == nullptr) {
        return;
    }
    stateMachine->GetDeviceState();
    stateMachine->GetState();
    stateMachine->GetDeviceName();
    stateMachine->GetEventName(AVRCP_CT_CONNECT_EVT);
    stateMachine->NotifyStateChanged();
    stateMachine->GetAVMetaData();
    stateMachine->GetAVPlaybackState();
    stateMachine->GetCurrentTrack();
    stateMachine->GetAvailablePlayers();
}

static void TestAvrcpCtSmStackEvent(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    auto stateMachine = CreateAvrcpCtStateMachine(addr);
    if (stateMachine == nullptr) {
        return;
    }
    // stack connect/disconnect events drive state transition
    AvrcpCtMessage stackConnect(STACK_CONNECT);
    stackConnect.dev_ = addr;
    stackConnect.rcConnect = provider.ConsumeBool();
    stateMachine->ProcessMessage(stackConnect);

    AvrcpCtMessage stackDisconnect(STACK_DISCONNECT);
    stackDisconnect.dev_ = addr;
    stateMachine->ProcessMessage(stackDisconnect);

    stateMachine->GetDeviceState();
}

static void TestAvrcpCtSmProcessEvent(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    auto stateMachine = CreateAvrcpCtStateMachine(addr);
    if (stateMachine == nullptr) {
        return;
    }
    int32_t eventWhat = provider.ConsumeIntegralInRange<int32_t>(AVRCP_CT_EVT_MIN, AVRCP_CT_EVT_MAX);

    AvrcpCtMessage event(eventWhat);
    event.dev_ = addr;
    event.arg1_ = provider.ConsumeIntegral<int32_t>();
    event.eventThreeParameter_ = provider.ConsumeIntegral<int32_t>();
    stateMachine->ProcessMessage(event);
}

static void TestAvrcpCtSmPlayerInfo(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    auto stateMachine = CreateAvrcpCtStateMachine(addr);
    if (stateMachine == nullptr) {
        return;
    }
    std::string playerName = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    int playerId = provider.ConsumeIntegral<int>();
    int playStatus = provider.ConsumeIntegral<int>();

    stateMachine->UpdatePlayerInfo(playerName, playerId, playStatus);

    auto track = std::make_shared<AvrcpCtMediaItem>();
    track->SetTitle(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16)));
    track->SetArtist(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16)));

    AvrcpCtMessage trackChanged(AVRCP_CT_MSG_PROCESS_TRACK_CHANGED);
    trackChanged.dev_ = addr;
    trackChanged.track_ = track;
    stateMachine->ProcessMessage(trackChanged);
    stateMachine->GetCurrentTrack();
}
static void TestAvrcpCtSmCoverArtPsm(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    auto stateMachine = CreateAvrcpCtStateMachine(addr);
    if (stateMachine == nullptr) {
        return;
    }
    stateMachine->ProcessGetCoverArtPsm();
    stateMachine->CleanupAvSession();
}

static void TestAvrcpCtSmSessionCallback(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    auto stateMachine = CreateAvrcpCtStateMachine(addr);
    if (stateMachine == nullptr) {
        return;
    }
    auto callback = stateMachine->GetAvSessionCallbackImpl();
    if (callback != nullptr) {
        AVSession::AVControlCommand cmd;
        callback->OnPlay(cmd);
        callback->OnPause();
        callback->OnStop();
        callback->OnPlayNext(cmd);
        callback->OnPlayPrevious(cmd);
        callback->OnFastForward(0, cmd);
        callback->OnRewind(0, cmd);
        callback->OnSetLoopMode(provider.ConsumeIntegral<int32_t>());
        callback->OnSeek(0);
        callback->OnSetSpeed(1.0);
        callback->OnToggleFavorite("");
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtStateMachineFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestAvrcpCtSmLifecycle(provider);
    TestAvrcpCtSmStackEvent(provider);
    TestAvrcpCtSmProcessEvent(provider);
    TestAvrcpCtSmPlayerInfo(provider);
    TestAvrcpCtSmCoverArtPsm(provider);
    TestAvrcpCtSmSessionCallback(provider);

    HILOGI("AvrcpCtStateMachineFuzzTest success");
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
