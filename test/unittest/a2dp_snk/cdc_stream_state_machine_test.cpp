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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cdc_stream_state_machine.h"
#include "a2dp_snk_def.h"
#include "a2dp_snk_message.h"
#include "mock/a2dp_snk_service_mock.h"
#include "mock/audio_focus_manager_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;
namespace AudioStandard = OHOS::AudioStandard;

// Helper to build an A2dpSnkMessage with arg3 set (for focus hint / reason).
static A2dpSnkMessage MakeEvent(int what, int arg3 = 0)
{
    A2dpSnkMessage msg(what);
    msg.arg3_ = arg3;
    return msg;
}

// ===========================================================================
// CdcStreamStateMachineTest
//
// Tests follow design §7.3 test groups:
//   7.3.1 Focus management
//   7.3.2 Playback control
//   7.3.3 DELAYED_PAUSE (D7 / M20)
//   7.3.4 Policy layer notifications
//   7.3.5 Device disconnect
//   7.3.6 Broadcast
//   7.3.7 D9 dual-path
//
// Approach: the state machine posts events via service_->PostEvent. With a
// NiceMock service, PostEvent is mocked and does nothing. To drive state
// transitions we call ProcessMessage() directly with A2dpSnkMessage events,
// which is what the real A2dpSnkService::ProcessEvent would do after dequeue.
//
// AudioFocusManager is mocked via AudioFocusManagerMock injected through
// SetFocusManager before Init. The SM constructor does not call Init();
// the test calls Init() explicitly after injection so that SetFocusManager
// can replace the default-created manager before InitOnly binds the callback.
// ===========================================================================
class CdcStreamStateMachineTest : public Test {
protected:
    void SetUp() override
    {
        focusMock_ = new NiceMock<AudioFocusManagerMock>();
        // Default: InitOnly succeeds, ActivateFocus succeeds, others no-op.
        ON_CALL(*focusMock_, InitOnly(_)).WillByDefault(Return(0));
        ON_CALL(*focusMock_, ActivateFocus()).WillByDefault(Return(0));
        // Create SM, inject mock AFM, then Init so InitOnly binds to the mock.
        streamSM_ = std::make_shared<CdcStreamStateMachine>();
        streamSM_->SetFocusManager(std::unique_ptr<AudioFocusManager>(focusMock_));
        streamSM_->Init();
    }

    void TearDown() override
    {
        if (streamSM_) {
            streamSM_->Cleanup();
            streamSM_.reset();
        }
        // focusMock_ is deleted by streamSM_->Cleanup() via unique_ptr reset.
    }

    // Drive an event into the state machine (bypasses PostEvent routing).
    void Dispatch(int what, int arg3 = 0)
    {
        A2dpSnkMessage msg = MakeEvent(what, arg3);
        streamSM_->ProcessMessage(msg);
    }

    // Transition: IDLE -> READY by requesting focus (A2dpConnected(true)).
    void ReachReadyState()
    {
        EXPECT_CALL(*focusMock_, ActivateFocus()).WillOnce(Return(0));
        Dispatch(A2DP_SNK_STREAM_A2DP_CONNECTED_EVT, 1);  // arg3=1 → allowed
        ASSERT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::READY);
    }

    // Transition: IDLE -> READY -> PLAYING by requesting focus + stream start.
    void ReachPlayingState()
    {
        ReachReadyState();
        Dispatch(A2DP_SNK_STREAM_AUDIO_STARTED_EVT);
        ASSERT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::PLAYING);
    }

    // Transition: PLAYING -> TRANSIENT_PAUSED via PAUSE hint.
    void ReachTransientPausedState()
    {
        ReachPlayingState();
        Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
            static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
        ASSERT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
    }

    // Transition: PLAYING -> NO_FOCUS via STOP hint.
    void ReachNoFocusState()
    {
        ReachPlayingState();
        Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
            static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_STOP));
        ASSERT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::NO_FOCUS);
    }

    std::unique_ptr<NiceMock<AudioFocusManagerMock>> focusMock_ = nullptr;
    std::shared_ptr<CdcStreamStateMachine> streamSM_;
};

// ===========================================================================
// §7.3.1 — Focus management
// ===========================================================================

// RequestFocus_Success_EntersReady: IDLE + A2dpConnected(true) → READY.
TEST_F(CdcStreamStateMachineTest, RequestFocus_Success_EntersReady)
{
    EXPECT_CALL(*focusMock_, ActivateFocus()).WillOnce(Return(0));
    Dispatch(A2DP_SNK_STREAM_A2DP_CONNECTED_EVT, 1);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::READY);
}

// RequestFocus_Fail_RemainsIdle: IDLE + A2dpConnected(true) + Activate fails → IDLE.
TEST_F(CdcStreamStateMachineTest, RequestFocus_Fail_RemainsIdle)
{
    EXPECT_CALL(*focusMock_, ActivateFocus()).WillOnce(Return(-1));
    Dispatch(A2DP_SNK_STREAM_A2DP_CONNECTED_EVT, 1);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::IDLE);
}

// FocusLoss_Pause_ToTransientPaused: PLAYING + PAUSE hint → TRANSIENT_PAUSED (C12).
TEST_F(CdcStreamStateMachineTest, FocusLoss_Pause_ToTransientPaused)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
}

// FocusLoss_Stop_ToNoFocus: PLAYING + STOP hint → NO_FOCUS (C10).
TEST_F(CdcStreamStateMachineTest, FocusLoss_Stop_ToNoFocus)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_STOP));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::NO_FOCUS);
}

// FocusLoss_CallsDeactivateFocus: PLAYING + STOP → DeactivateFocus invoked.
TEST_F(CdcStreamStateMachineTest, FocusLoss_CallsDeactivateFocus)
{
    ReachPlayingState();
    EXPECT_CALL(*focusMock_, DeactivateFocus()).Times(AtLeast(1));
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_STOP));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::NO_FOCUS);
}

// ReadyState_Pause_ToTransientPaused: READY + PAUSE hint → TRANSIENT_PAUSED.
TEST_F(CdcStreamStateMachineTest, ReadyState_Pause_ToTransientPaused)
{
    ReachReadyState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
}

// ReadyState_Stop_ToNoFocus: READY + STOP hint → NO_FOCUS.
TEST_F(CdcStreamStateMachineTest, ReadyState_Stop_ToNoFocus)
{
    ReachReadyState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_STOP));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::NO_FOCUS);
}

// NoFocus_ResumeIgnored: NO_FOCUS + RESUME hint → stays NO_FOCUS (STOP is permanent).
TEST_F(CdcStreamStateMachineTest, NoFocus_ResumeIgnored)
{
    ReachNoFocusState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_RESUME));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::NO_FOCUS);
}

// NoFocus_UserPlay_RequestsFocus: NO_FOCUS + SINK_PLAY → re-request focus → READY/PLAYING (M03-2).
TEST_F(CdcStreamStateMachineTest, NoFocus_UserPlay_RequestsFocus)
{
    ReachNoFocusState();
    // streamAvailable_ is still true from ReachPlayingState; re-focus → PLAYING.
    EXPECT_CALL(*focusMock_, ActivateFocus()).WillOnce(Return(0));
    Dispatch(A2DP_SNK_STREAM_USER_PLAY_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::PLAYING);
}

// NoFocus_UserPlay_Fail_StaysNoFocus: NO_FOCUS + SINK_PLAY + Activate fails → NO_FOCUS.
TEST_F(CdcStreamStateMachineTest, NoFocus_UserPlay_Fail_StaysNoFocus)
{
    ReachNoFocusState();
    EXPECT_CALL(*focusMock_, ActivateFocus()).WillOnce(Return(-1));
    Dispatch(A2DP_SNK_STREAM_USER_PLAY_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::NO_FOCUS);
}

// ===========================================================================
// §7.3.2 — Playback control
// ===========================================================================

// StreamStart_ReadyToPlaying: READY + AudioStreamStarted → PLAYING.
TEST_F(CdcStreamStateMachineTest, StreamStart_ReadyToPlaying)
{
    ReachReadyState();
    Dispatch(A2DP_SNK_STREAM_AUDIO_STARTED_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::PLAYING);
    EXPECT_TRUE(streamSM_->IsPlaying());
}

// StreamStop_PlayingToReady: PLAYING + AudioStreamStopped → READY (merged PAUSED).
TEST_F(CdcStreamStateMachineTest, StreamStop_PlayingToReady)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_AUDIO_STOPPED_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::READY);
    EXPECT_FALSE(streamSM_->IsPlaying());
}

// UserPlayIntent_IdleRequestsFocus: IDLE + UserPlayIntent → READY.
TEST_F(CdcStreamStateMachineTest, UserPlayIntent_IdleRequestsFocus)
{
    EXPECT_CALL(*focusMock_, ActivateFocus()).WillOnce(Return(0));
    Dispatch(A2DP_SNK_STREAM_USER_PLAY_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::READY);
}

// UserPlayIntent_IdleFail_StaysIdle: IDLE + UserPlayIntent + Activate fails → IDLE.
TEST_F(CdcStreamStateMachineTest, UserPlayIntent_IdleFail_StaysIdle)
{
    EXPECT_CALL(*focusMock_, ActivateFocus()).WillOnce(Return(-1));
    Dispatch(A2DP_SNK_STREAM_USER_PLAY_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::IDLE);
}

// UserPauseIntent_PlayingSetsAutoResumeFalse: PLAYING + UserPauseIntent → autoResume=false.
TEST_F(CdcStreamStateMachineTest, UserPauseIntent_PlayingSetsAutoResumeFalse)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_USER_PAUSE_EVT);
    EXPECT_FALSE(streamSM_->autoResume_);
    // Stays PLAYING (phone will stop stream).
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::PLAYING);
}

// ===========================================================================
// §7.3.3 — DELAYED_PAUSE (D7 / M20)
// ===========================================================================

// DelayedPause_TriggersRemotePause: PLAYING + PAUSE + DELAYED_PAUSE timeout → RequestRemotePause.
TEST_F(CdcStreamStateMachineTest, DelayedPause_TriggersRemotePause)
{
    ReachPlayingState();
    // LOSS_TRANSIENT → TRANSIENT_PAUSED + DELAYED_PAUSE timer started.
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
    // DELAYED_PAUSE timeout: stream available, not in call → RequestRemotePause.
    Dispatch(A2DP_SNK_STREAM_DELAYED_PAUSE_EVT);
    // autoResume_ stays true (set by PLAYING.Entry, not touched by DELAYED_PAUSE).
    EXPECT_TRUE(streamSM_->autoResume_);
}

// DelayedPause_SkippedWhenStreamStopped: PLAYING + PAUSE + STREAM_STOP + DELAYED_PAUSE → no pause.
TEST_F(CdcStreamStateMachineTest, DelayedPause_SkippedWhenStreamStopped)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    // Stream stops before timer fires.
    Dispatch(A2DP_SNK_STREAM_AUDIO_STOPPED_EVT);
    Dispatch(A2DP_SNK_STREAM_DELAYED_PAUSE_EVT);
}

// GainResume_AutoResumeTrue_SendsPlay: TRANSIENT_PAUSED(autoResume=true) + RESUME → PLAYING + RequestRemotePlay (C13).
TEST_F(CdcStreamStateMachineTest, GainResume_AutoResumeTrue_SendsPlay)
{
    ReachPlayingState();
    // autoResume_ is true from PLAYING.Entry.
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    ASSERT_TRUE(streamSM_->autoResume_);
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_RESUME));
    // Stream still available → PLAYING.
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::PLAYING);
}

// GainResume_AutoResumeFalse_NoPlay: TRANSIENT_PAUSED(autoResume=false) + RESUME → READY, no play.
TEST_F(CdcStreamStateMachineTest, GainResume_AutoResumeFalse_NoPlay)
{
    ReachPlayingState();
    // User paused → autoResume=false.
    Dispatch(A2DP_SNK_STREAM_USER_PAUSE_EVT);
    ASSERT_FALSE(streamSM_->autoResume_);
    // LOSS_TRANSIENT → TRANSIENT_PAUSED.
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    // Stop stream so RESUME → READY (no stream).
    Dispatch(A2DP_SNK_STREAM_AUDIO_STOPPED_EVT);
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_RESUME));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::READY);
}

// M20_UserPauseInDelayedWindow: TRANSIENT_PAUSED + UserPauseIntent → autoResume=false, stay TRANSIENT_PAUSED.
TEST_F(CdcStreamStateMachineTest, M20_UserPauseInDelayedWindow)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    // User pauses during delayed window → autoResume=false, stay TRANSIENT_PAUSED.
    Dispatch(A2DP_SNK_STREAM_USER_PAUSE_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
    EXPECT_FALSE(streamSM_->autoResume_);
    // RESUME → no auto play (autoResume=false). Stream still available → PLAYING.
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_RESUME));
}

// M20_UserPlayInDelayedWindow: TRANSIENT_PAUSED + UserPlayIntent → autoResume=true, RESUME → PLAYING + play.
TEST_F(CdcStreamStateMachineTest, M20_UserPlayInDelayedWindow)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    // User plays during delayed window → stay TRANSIENT_PAUSED, autoResume=true.
    Dispatch(A2DP_SNK_STREAM_USER_PLAY_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
    EXPECT_TRUE(streamSM_->autoResume_);
    // RESUME → RequestRemotePlay (autoResume=true).
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_RESUME));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::PLAYING);
}

// PauseRequested_SkipsDelayedPause: PLAYING(pauseRequested via SINK_PAUSE) + PAUSE → autoResume=false.
TEST_F(CdcStreamStateMachineTest, PauseRequested_SkipsDelayedPause)
{
    ReachPlayingState();
    // User paused first → autoResume=false.
    Dispatch(A2DP_SNK_STREAM_USER_PAUSE_EVT);
    ASSERT_FALSE(streamSM_->autoResume_);
    // LOSS_TRANSIENT: → TRANSIENT_PAUSED. autoResume stays false.
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
    EXPECT_FALSE(streamSM_->autoResume_);
}

// TransientPaused_Stop_ToNoFocus: TRANSIENT_PAUSED + STOP hint → NO_FOCUS.
TEST_F(CdcStreamStateMachineTest, TransientPaused_Stop_ToNoFocus)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_STOP));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::NO_FOCUS);
}

// ===========================================================================
// §7.3.4 — Policy layer notifications
// ===========================================================================

// A2dpConnected_Allowed_RequestsFocus: IDLE + A2dpConnected(true) → READY.
TEST_F(CdcStreamStateMachineTest, A2dpConnected_Allowed_RequestsFocus)
{
    EXPECT_CALL(*focusMock_, ActivateFocus()).WillOnce(Return(0));
    Dispatch(A2DP_SNK_STREAM_A2DP_CONNECTED_EVT, 1);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::READY);
}

// A2dpConnected_Disallowed_StaysIdle: IDLE + A2dpConnected(false) → IDLE (P09).
TEST_F(CdcStreamStateMachineTest, A2dpConnected_Disallowed_StaysIdle)
{
    Dispatch(A2DP_SNK_STREAM_A2DP_CONNECTED_EVT, 0);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::IDLE);
}

// PlaybackDisallowed_Playing_CallsRemotePause: PLAYING + PlaybackDisallowed → stay PLAYING + RequestRemotePause.
TEST_F(CdcStreamStateMachineTest, PlaybackDisallowed_Playing_CallsRemotePause)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT);
    // Stays PLAYING; actual transition to IDLE happens on subsequent DISCONNECT_CLEANUP_EVT.
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::PLAYING);
}

// PlaybackDisallowed_Ready_NoRemotePause: READY + PlaybackDisallowed → no pause (no stream).
TEST_F(CdcStreamStateMachineTest, PlaybackDisallowed_Ready_NoRemotePause)
{
    ReachReadyState();
    // streamAvailable_ is false in READY → HandlePlaybackDisallowed skips RequestRemotePause.
    Dispatch(A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::READY);
}

// ===========================================================================
// §7.3.5 — Device disconnect
// ===========================================================================

// DeviceDisconnected_Playing_ToIdle: PLAYING + DeviceDisconnected → IDLE + DeactivateFocus.
TEST_F(CdcStreamStateMachineTest, DeviceDisconnected_Playing_ToIdle)
{
    ReachPlayingState();
    EXPECT_CALL(*focusMock_, DeactivateFocus()).Times(AtLeast(1));
    Dispatch(A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::IDLE);
}

// DeviceDisconnected_Ready_ToIdle: READY + DeviceDisconnected → IDLE.
TEST_F(CdcStreamStateMachineTest, DeviceDisconnected_Ready_ToIdle)
{
    ReachReadyState();
    EXPECT_CALL(*focusMock_, DeactivateFocus()).Times(AtLeast(1));
    Dispatch(A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::IDLE);
}

// DeviceDisconnected_TransientPaused_CancelsTimer_ToIdle: TRANSIENT_PAUSED + Disconnect → IDLE.
TEST_F(CdcStreamStateMachineTest, DeviceDisconnected_TransientPaused_CancelsTimer_ToIdle)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,
        static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE));
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::TRANSIENT_PAUSED);
    EXPECT_CALL(*focusMock_, DeactivateFocus()).Times(AtLeast(1));
    Dispatch(A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::IDLE);
}

// DeviceDisconnected_NoFocus_ToIdle: NO_FOCUS + Disconnect → IDLE.
TEST_F(CdcStreamStateMachineTest, DeviceDisconnected_NoFocus_ToIdle)
{
    ReachNoFocusState();
    Dispatch(A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT);
    EXPECT_EQ(streamSM_->CurrentStateName(), CdcStreamStateMachine::IDLE);
}

// ===========================================================================
// §7.3.7 — D9 dual-path (user intent does NOT call RequestRemotePlay/Pause)
// ===========================================================================

// UserPlayIntent_DoesNotCallRemotePlay: any state + UserPlayIntent → no RequestRemotePlay.
TEST_F(CdcStreamStateMachineTest, UserPlayIntent_DoesNotCallRemotePlay)
{
    ReachReadyState();
    Dispatch(A2DP_SNK_STREAM_USER_PLAY_EVT);
}

// UserPauseIntent_DoesNotCallRemotePause: PLAYING + UserPauseIntent → no RequestRemotePause.
TEST_F(CdcStreamStateMachineTest, UserPauseIntent_DoesNotCallRemotePause)
{
    ReachPlayingState();
    Dispatch(A2DP_SNK_STREAM_USER_PAUSE_EVT);
}

// ===========================================================================
// §7.3.8 — Cleanup lifecycle
// ===========================================================================

// Cleanup_CallsDeactivateAndDeinit: Cleanup() → DeactivateAndDeinit invoked.
TEST_F(CdcStreamStateMachineTest, Cleanup_CallsDeactivateAndDeinit)
{
    ReachPlayingState();
    EXPECT_CALL(*focusMock_, DeactivateAndDeinit()).Times(1);
    streamSM_->Cleanup();
    // Prevent TearDown from double-calling Cleanup.
    streamSM_.reset();
    focusMock_ = nullptr;
}

// Cleanup_ResetsFocusState: After Cleanup, CanPlay=false, IsPlaying=false.
TEST_F(CdcStreamStateMachineTest, Cleanup_ResetsFocusState)
{
    ReachPlayingState();
    EXPECT_TRUE(streamSM_->CanPlay());
    streamSM_->Cleanup();
    EXPECT_FALSE(streamSM_->CanPlay());
    EXPECT_FALSE(streamSM_->IsPlaying());
    // Prevent TearDown from double-calling Cleanup.
    streamSM_.reset();
    focusMock_ = nullptr;
}
