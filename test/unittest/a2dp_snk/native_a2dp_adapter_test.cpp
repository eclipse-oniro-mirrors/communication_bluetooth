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

#include "native_a2dp_adapter.h"
#include "bt_av.h"
#include "service_util.h"
#include "raw_address.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace {
// 用于 Init 测试的空回调实现
class DummyA2dpSinkCallback : public INativeA2dpSinkCallback {
public:
    void OnNativeConnectionStateChanged(const RawAddress &device, btav_connection_state_t state) override {}
    void OnNativeAudioStateChanged(const RawAddress &device, btav_audio_state_t state) override {}
    void OnNativeAudioConfigChanged(const RawAddress &device, uint32_t sampleRate, uint8_t channelCount) override {}
};
}  // namespace

namespace {
constexpr char TEST_ADDR[] = "00:11:22:33:44:55";

// Test callback counters for fake btav_sink_interface_t.
static int g_initCallCount = 0;
static int g_connectCallCount = 0;
static int g_disconnectCallCount = 0;
static int g_cleanupCallCount = 0;
static int g_setFocusCallCount = 0;
static int g_setGainCallCount = 0;
static int g_setActiveDeviceCallCount = 0;
static int g_lastFocusState = -1;
static float g_lastGain = 0.0f;

// Return values configurable per-test.
static bt_status_t g_initReturn = BT_STATUS_SUCCESS;
static bt_status_t g_connectReturn = BT_STATUS_SUCCESS;
static bt_status_t g_disconnectReturn = BT_STATUS_SUCCESS;
static bt_status_t g_setActiveReturn = BT_STATUS_SUCCESS;

bt_status_t FakeInit(btav_sink_callbacks_t* cb, int max)
{
    g_initCallCount++;
    return g_initReturn;
}

bt_status_t FakeConnect(const STACK::RawAddress& addr)
{
    g_connectCallCount++;
    return g_connectReturn;
}

bt_status_t FakeDisconnect(const STACK::RawAddress& addr)
{
    g_disconnectCallCount++;
    return g_disconnectReturn;
}

void FakeCleanup()
{
    g_cleanupCallCount++;
}

void FakeSetAudioFocusState(int state)
{
    g_setFocusCallCount++;
    g_lastFocusState = state;
}

void FakeSetAudioTrackGain(float gain)
{
    g_setGainCallCount++;
    g_lastGain = gain;
}

bt_status_t FakeSetActiveDevice(const STACK::RawAddress& addr)
{
    g_setActiveDeviceCallCount++;
    return g_setActiveReturn;
}

void ResetCounters()
{
    g_initCallCount = 0;
    g_connectCallCount = 0;
    g_disconnectCallCount = 0;
    g_cleanupCallCount = 0;
    g_setFocusCallCount = 0;
    g_setGainCallCount = 0;
    g_setActiveDeviceCallCount = 0;
    g_lastFocusState = -1;
    g_lastGain = 0.0f;
    g_initReturn = BT_STATUS_SUCCESS;
    g_connectReturn = BT_STATUS_SUCCESS;
    g_disconnectReturn = BT_STATUS_SUCCESS;
    g_setActiveReturn = BT_STATUS_SUCCESS;
}

btav_sink_interface_t MakeFakeInterface()
{
    btav_sink_interface_t iface = {};
    iface.size = sizeof(btav_sink_interface_t);
    iface.init = FakeInit;
    iface.connect = FakeConnect;
    iface.disconnect = FakeDisconnect;
    iface.cleanup = FakeCleanup;
    iface.set_audio_focus_state = FakeSetAudioFocusState;
    iface.set_audio_track_gain = FakeSetAudioTrackGain;
    iface.set_active_device = FakeSetActiveDevice;
    return iface;
}

}  // namespace

class NativeA2dpAdapterTest : public Test {
protected:
    void SetUp() override
    {
        ResetCounters();
        iface_ = MakeFakeInterface();
        adapter_ = std::make_unique<NativeA2dpAdapter>(&iface_);
    }

    void TearDown() override
    {
        adapter_.reset();
    }

    btav_sink_interface_t iface_;
    std::unique_ptr<NativeA2dpAdapter> adapter_;
};

// --- Init ---

HWTEST_F(NativeA2dpAdapterTest, Init_CallsInterface, TestSize.Level1)
{
    DummyA2dpSinkCallback cb;
    bt_status_t ret = adapter_->Init(&cb, 6);
    EXPECT_EQ(ret, BT_STATUS_SUCCESS);
    EXPECT_EQ(g_initCallCount, 1);
}

HWTEST_F(NativeA2dpAdapterTest, Init_Fail_ReturnsError, TestSize.Level1)
{
    g_initReturn = BT_STATUS_FAIL;
    DummyA2dpSinkCallback cb;
    bt_status_t ret = adapter_->Init(&cb, 6);
    EXPECT_EQ(ret, BT_STATUS_FAIL);
    EXPECT_EQ(g_initCallCount, 1);
}

// --- Connect ---

HWTEST_F(NativeA2dpAdapterTest, Connect_CallsInterface, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR);
    bt_status_t ret = adapter_->Connect(addr);
    EXPECT_EQ(ret, BT_STATUS_SUCCESS);
    EXPECT_EQ(g_connectCallCount, 1);
}

HWTEST_F(NativeA2dpAdapterTest, Connect_Fail_ReturnsError, TestSize.Level1)
{
    g_connectReturn = BT_STATUS_FAIL;
    RawAddress addr(TEST_ADDR);
    bt_status_t ret = adapter_->Connect(addr);
    EXPECT_EQ(ret, BT_STATUS_FAIL);
}

// --- Disconnect ---

HWTEST_F(NativeA2dpAdapterTest, Disconnect_CallsInterface, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR);
    bt_status_t ret = adapter_->Disconnect(addr);
    EXPECT_EQ(ret, BT_STATUS_SUCCESS);
    EXPECT_EQ(g_disconnectCallCount, 1);
}

// --- Cleanup ---

HWTEST_F(NativeA2dpAdapterTest, Cleanup_CallsInterface, TestSize.Level1)
{
    adapter_->Cleanup();
    EXPECT_EQ(g_cleanupCallCount, 1);
}

// --- SetAudioFocusState ---

HWTEST_F(NativeA2dpAdapterTest, SetAudioFocusState_CallsInterface, TestSize.Level1)
{
    adapter_->SetAudioFocusState(STATE_FOCUS_GRANTED);
    EXPECT_EQ(g_setFocusCallCount, 1);
    EXPECT_EQ(g_lastFocusState, STATE_FOCUS_GRANTED);
}

// --- SetAudioTrackGain ---

HWTEST_F(NativeA2dpAdapterTest, SetAudioTrackGain_CallsInterface, TestSize.Level1)
{
    adapter_->SetAudioTrackGain(0.5f);
    EXPECT_EQ(g_setGainCallCount, 1);
    EXPECT_FLOAT_EQ(g_lastGain, 0.5f);
}

// --- SetActiveDevice ---

HWTEST_F(NativeA2dpAdapterTest, SetActiveDevice_CallsInterface, TestSize.Level1)
{
    RawAddress addr(TEST_ADDR);
    bt_status_t ret = adapter_->SetActiveDevice(addr);
    EXPECT_EQ(ret, BT_STATUS_SUCCESS);
    EXPECT_EQ(g_setActiveDeviceCallCount, 1);
}

HWTEST_F(NativeA2dpAdapterTest, SetActiveDevice_Fail_ReturnsError, TestSize.Level1)
{
    g_setActiveReturn = BT_STATUS_FAIL;
    RawAddress addr(TEST_ADDR);
    bt_status_t ret = adapter_->SetActiveDevice(addr);
    EXPECT_EQ(ret, BT_STATUS_FAIL);
}

// --- Null interface guards ---

HWTEST_F(NativeA2dpAdapterTest, NullInterface_Init_ReturnsFail, TestSize.Level1)
{
    auto nullAdapter = std::make_unique<NativeA2dpAdapter>(nullptr);
    DummyA2dpSinkCallback cb;
    EXPECT_EQ(nullAdapter->Init(&cb, 6), BT_STATUS_FAIL);
    EXPECT_EQ(g_initCallCount, 0);
}

HWTEST_F(NativeA2dpAdapterTest, NullInterface_Connect_ReturnsFail, TestSize.Level1)
{
    auto nullAdapter = std::make_unique<NativeA2dpAdapter>(nullptr);
    RawAddress addr(TEST_ADDR);
    EXPECT_EQ(nullAdapter->Connect(addr), BT_STATUS_FAIL);
}

HWTEST_F(NativeA2dpAdapterTest, NullInterface_Disconnect_ReturnsFail, TestSize.Level1)
{
    auto nullAdapter = std::make_unique<NativeA2dpAdapter>(nullptr);
    RawAddress addr(TEST_ADDR);
    EXPECT_EQ(nullAdapter->Disconnect(addr), BT_STATUS_FAIL);
}

HWTEST_F(NativeA2dpAdapterTest, NullInterface_SetActiveDevice_ReturnsFail, TestSize.Level1)
{
    auto nullAdapter = std::make_unique<NativeA2dpAdapter>(nullptr);
    RawAddress addr(TEST_ADDR);
    EXPECT_EQ(nullAdapter->SetActiveDevice(addr), BT_STATUS_FAIL);
}

HWTEST_F(NativeA2dpAdapterTest, NullInterface_Cleanup_NoCrash, TestSize.Level1)
{
    auto nullAdapter = std::make_unique<NativeA2dpAdapter>(nullptr);
    EXPECT_NO_FATAL_FAILURE(nullAdapter->Cleanup());
}

HWTEST_F(NativeA2dpAdapterTest, NullInterface_SetAudioFocusState_NoCrash, TestSize.Level1)
{
    auto nullAdapter = std::make_unique<NativeA2dpAdapter>(nullptr);
    EXPECT_NO_FATAL_FAILURE(nullAdapter->SetAudioFocusState(1));
}

HWTEST_F(NativeA2dpAdapterTest, NullInterface_SetAudioTrackGain_NoCrash, TestSize.Level1)
{
    auto nullAdapter = std::make_unique<NativeA2dpAdapter>(nullptr);
    EXPECT_NO_FATAL_FAILURE(nullAdapter->SetAudioTrackGain(0.5f));
}

// --- Partial null (interface ptr set, function ptr null) ---

HWTEST_F(NativeA2dpAdapterTest, PartialNull_Init_ReturnsFail, TestSize.Level1)
{
    btav_sink_interface_t partial = {};
    partial.size = sizeof(btav_sink_interface_t);
    partial.init = nullptr;  // init is null
    auto partialAdapter = std::make_unique<NativeA2dpAdapter>(&partial);
    DummyA2dpSinkCallback cb;
    EXPECT_EQ(partialAdapter->Init(&cb, 6), BT_STATUS_FAIL);
}
