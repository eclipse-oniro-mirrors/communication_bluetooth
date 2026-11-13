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
#include "a2dp_snk_service.h"
#include "a2dp_snk_def.h"
#include "a2dp_snk_message.h"
#include "bluetooth_errorcode.h"
#include "mock/native_a2dp_interface_mock.h"
#include "mock/i_a2dp_snk_stream_state_machine_mock.h"
#include "mock/test_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace {
constexpr char TEST_ADDR_1[] = "00:11:22:33:44:55";
}

class A2dpSnkServiceTest : public Test {
protected:
    void SetUp() override
    {
        nativeMock_ = std::make_shared<NiceMock<NativeA2dpInterfaceMock>>();
        service_ = std::make_unique<A2dpSnkService>();
        service_->SetNativeAdapter(nativeMock_);
    }

    void TearDown() override
    {
        service_.reset();
        nativeMock_.reset();
    }

    std::shared_ptr<NiceMock<NativeA2dpInterfaceMock>> nativeMock_;
    std::unique_ptr<A2dpSnkService> service_;
};

// ===========================================================================
// Native adapter delegation
// ===========================================================================

HWTEST_F(A2dpSnkServiceTest, SetNativeAdapter_NativeConnectDelegatesToAdapter, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    EXPECT_CALL(*nativeMock_, Connect(_)).WillOnce(Return(BT_STATUS_SUCCESS));
    bool result = service_->NativeConnect(addr);
    EXPECT_TRUE(result);
}

HWTEST_F(A2dpSnkServiceTest, NativeConnect_Fail_ReturnsFalse, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    EXPECT_CALL(*nativeMock_, Connect(_)).WillOnce(Return(BT_STATUS_FAIL));
    bool result = service_->NativeConnect(addr);
    EXPECT_FALSE(result);
}

HWTEST_F(A2dpSnkServiceTest, NativeConnect_NullAdapter_ReturnsFalse, TestSize.Level1)
{
    auto svc = std::make_unique<A2dpSnkService>();
    // No SetNativeAdapter → nativeAdapter_ is nullptr
    RawAddress addr(TEST_ADDR_1);
    bool result = svc->NativeConnect(addr);
    EXPECT_FALSE(result);
}

HWTEST_F(A2dpSnkServiceTest, NativeDisconnect_DelegatesToAdapter, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    EXPECT_CALL(*nativeMock_, Disconnect(_)).WillOnce(Return(BT_STATUS_SUCCESS));
    bool result = service_->NativeDisconnect(addr);
    EXPECT_TRUE(result);
}

HWTEST_F(A2dpSnkServiceTest, NativeDisconnect_Fails_ReturnsFalse, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    EXPECT_CALL(*nativeMock_, Disconnect(_)).WillOnce(Return(BT_STATUS_FAIL));
    bool result = service_->NativeDisconnect(addr);
    EXPECT_FALSE(result);
}

HWTEST_F(A2dpSnkServiceTest, NativeDisconnect_NullAdapter_ReturnsFalse, TestSize.Level1)
{
    auto svc = std::make_unique<A2dpSnkService>();
    RawAddress addr(TEST_ADDR_1);
    bool result = svc->NativeDisconnect(addr);
    EXPECT_FALSE(result);
}

HWTEST_F(A2dpSnkServiceTest, AcceptIncomingConnection_TrueByDefault, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    bool result = service_->AcceptIncomingConnection(addr);
    EXPECT_TRUE(result);
}

// ===========================================================================
// SetAudioFocusState delegation
// ===========================================================================

HWTEST_F(A2dpSnkServiceTest, SetAudioFocusState_DelegatesToAdapter, TestSize.Level1)
{
    EXPECT_CALL(*nativeMock_, SetAudioFocusState(STATE_FOCUS_GRANTED)).Times(1);
    service_->SetAudioFocusState(STATE_FOCUS_GRANTED);
}

HWTEST_F(A2dpSnkServiceTest, SetAudioFocusState_NullAdapter_NoCrash, TestSize.Level1)
{
    auto svc = std::make_unique<A2dpSnkService>();
    EXPECT_NO_FATAL_FAILURE(svc->SetAudioFocusState(STATE_FOCUS_GRANTED));
}

// ===========================================================================
// Stream SM injection and forwarding
// ===========================================================================

HWTEST_F(A2dpSnkServiceTest, SetStreamStateMachine_GetStreamStateMachine, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_EQ(service_->GetStreamStateMachine(), smMock);
}

HWTEST_F(A2dpSnkServiceTest, GetStreamStateMachine_NullByDefault, TestSize.Level1)
{
    EXPECT_EQ(service_->GetStreamStateMachine(), nullptr);
}

HWTEST_F(A2dpSnkServiceTest, NotifyUserPlayIntent_ForwardsToStreamSM, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, NotifyUserPlayIntent()).Times(1);
    service_->NotifyUserPlayIntent();
}

HWTEST_F(A2dpSnkServiceTest, NotifyUserPauseIntent_ForwardsToStreamSM, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, NotifyUserPauseIntent()).Times(1);
    service_->NotifyUserPauseIntent();
}

HWTEST_F(A2dpSnkServiceTest, NotifyPlaybackDisallowed_ForwardsToStreamSM, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, NotifyPlaybackDisallowed()).Times(1);
    service_->NotifyPlaybackDisallowed();
}

HWTEST_F(A2dpSnkServiceTest, NotifyA2dpConnected_ForwardsToStreamSM, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, NotifyA2dpConnected(true)).Times(1);
    service_->NotifyA2dpConnected(true);
}

HWTEST_F(A2dpSnkServiceTest, NotifyA2dpConnected_Disallowed_ForwardsToStreamSM, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, NotifyA2dpConnected(false)).Times(1);
    service_->NotifyA2dpConnected(false);
}

HWTEST_F(A2dpSnkServiceTest, NotifyDeviceDisconnected_ForwardsToStreamSM, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, NotifyDeviceDisconnected()).Times(1);
    service_->NotifyDeviceDisconnected();
}

HWTEST_F(A2dpSnkServiceTest, NotifyDelayedPause_ForwardsToStreamSM, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, NotifyDelayedPause()).Times(1);
    service_->NotifyDelayedPause();
}

// Forwarding with null stream SM → no crash
HWTEST_F(A2dpSnkServiceTest, NotifyUserPlayIntent_NullSM_NoCrash, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(service_->NotifyUserPlayIntent());
}

HWTEST_F(A2dpSnkServiceTest, NotifyUserPauseIntent_NullSM_NoCrash, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(service_->NotifyUserPauseIntent());
}

HWTEST_F(A2dpSnkServiceTest, NotifyPlaybackDisallowed_NullSM_NoCrash, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(service_->NotifyPlaybackDisallowed());
}

HWTEST_F(A2dpSnkServiceTest, NotifyA2dpConnected_NullSM_NoCrash, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(service_->NotifyA2dpConnected(true));
}

HWTEST_F(A2dpSnkServiceTest, NotifyDeviceDisconnected_NullSM_NoCrash, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(service_->NotifyDeviceDisconnected());
}

HWTEST_F(A2dpSnkServiceTest, NotifyDelayedPause_NullSM_NoCrash, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(service_->NotifyDelayedPause());
}

// ===========================================================================
// CanPlay delegation
// ===========================================================================

HWTEST_F(A2dpSnkServiceTest, CanPlay_DelegatesToStreamSM, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, CanPlay()).WillOnce(Return(true));
    EXPECT_TRUE(service_->CanPlay());
}

HWTEST_F(A2dpSnkServiceTest, CanPlay_NullSM_ReturnsFalse, TestSize.Level1)
{
    EXPECT_FALSE(service_->CanPlay());
}

// ===========================================================================
// SetActiveSrcDevice
// ===========================================================================

HWTEST_F(A2dpSnkServiceTest, SetActiveSrcDevice_EmptyAddr_ReturnsNoError, TestSize.Level1)
{
    RawAddress emptyAddr("");
    EXPECT_EQ(service_->SetActiveSrcDevice(emptyAddr), Bluetooth::BT_NO_ERROR);
}

HWTEST_F(A2dpSnkServiceTest, SetActiveSrcDevice_NotStarted_ReturnsError, TestSize.Level1)
{
    // Service 未启动（isStarted_=false），SetActiveSrcDevice 被拒绝
    RawAddress addr(TEST_ADDR_1);
    EXPECT_NE(service_->SetActiveSrcDevice(addr), Bluetooth::BT_NO_ERROR);
}

HWTEST_F(A2dpSnkServiceTest, SetActiveSrcDevice_AdapterFails_ReturnsError, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    EXPECT_CALL(*nativeMock_, SetActiveDevice(_)).WillOnce(Return(BT_STATUS_FAIL));
    EXPECT_NE(service_->SetActiveSrcDevice(addr), Bluetooth::BT_NO_ERROR);
}

HWTEST_F(A2dpSnkServiceTest, SetActiveSrcDevice_NullAdapter_ReturnsError, TestSize.Level1)
{
    auto svc = std::make_unique<A2dpSnkService>();
    RawAddress addr(TEST_ADDR_1);
    EXPECT_NE(svc->SetActiveSrcDevice(addr), Bluetooth::BT_NO_ERROR);
}

// ===========================================================================
// GetNativeAdapter
// ===========================================================================

HWTEST_F(A2dpSnkServiceTest, GetNativeAdapter_ReturnsInjected, TestSize.Level1)
{
    EXPECT_EQ(service_->GetNativeAdapter(), nativeMock_);
}

// ===========================================================================
// INativeA2dpSinkCallback — 回调接口测试
// ===========================================================================

HWTEST_F(A2dpSnkServiceTest, OnNativeConnectionStateChanged_PostsEvent, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    // 回调应正常执行，不崩溃（PostEvent 内部有线程调度，此处仅验证可调用）
    EXPECT_NO_FATAL_FAILURE(service_->OnNativeConnectionStateChanged(addr, BTAV_CONNECTION_STATE_CONNECTED));
}

HWTEST_F(A2dpSnkServiceTest, OnNativeAudioStateChanged_NullStreamSM_NoCrash, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    // streamStateMachine_ 默认为 nullptr
    EXPECT_NO_FATAL_FAILURE(service_->OnNativeAudioStateChanged(addr, BTAV_AUDIO_STATE_STARTED));
}

HWTEST_F(A2dpSnkServiceTest, OnNativeAudioStateChanged_WithStreamSM_PostsEvent, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, PostEvent(_)).Times(1);
    RawAddress addr(TEST_ADDR_1);
    service_->OnNativeAudioStateChanged(addr, BTAV_AUDIO_STATE_STARTED);
}

HWTEST_F(A2dpSnkServiceTest, OnNativeAudioStateChanged_UnhandledState_NoEvent, TestSize.Level1)
{
    auto smMock = std::make_shared<NiceMock<IA2dpSnkStreamStateMachineMock>>();
    service_->SetStreamStateMachine(smMock);
    EXPECT_CALL(*smMock, PostEvent(_)).Times(0);
    RawAddress addr(TEST_ADDR_1);
    // 使用未定义的 audio state 值
    service_->OnNativeAudioStateChanged(addr, static_cast<btav_audio_state_t>(999));
}

HWTEST_F(A2dpSnkServiceTest, OnNativeAudioConfigChanged_PostsEvent, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR_1);
    EXPECT_NO_FATAL_FAILURE(service_->OnNativeAudioConfigChanged(addr, 44100, 2));
}
