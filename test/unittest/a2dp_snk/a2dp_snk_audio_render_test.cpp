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
#include "a2dp_snk_audio_render.h"
#include "btif/a2dp_audio_render_interface.h"
#include "mock/ia2dp_audio_render_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

// ===========================================================================
// A2dpSnkAudioRenderTest
//
// Tests A2dpSnkAudioRender — the service-layer AudioRenderer wrapper.
// The class internally calls AudioRenderer::Create() (real audio_framework),
// so CreateRender is not unit-tested here. All other methods are tested
// in the "no renderer" state (no CreateRender called), which verifies
// the null-guard logic without depending on audio_framework.
//
// DI path: IA2dpAudioRender::SetInstance/GetInstance is also tested via
// the IA2dpAudioRenderMock to verify the injection mechanism works.
// ===========================================================================
class A2dpSnkAudioRenderTest : public Test {
protected:
    void SetUp() override
    {
        render_ = A2dpSnkAudioRender::GetInstance();
        ASSERT_NE(render_, nullptr);
    }

    void TearDown() override
    {
        render_->ReleaseRender();
    }

    std::shared_ptr<A2dpSnkAudioRender> render_;
};

// --- Singleton ---

HWTEST_F(A2dpSnkAudioRenderTest, GetInstance_NotNull, TestSize.Level1)
{
    EXPECT_NE(render_, nullptr);
}

HWTEST_F(A2dpSnkAudioRenderTest, GetInstance_SameInstance, TestSize.Level1)
{
    auto render2 = A2dpSnkAudioRender::GetInstance();
    EXPECT_EQ(render_.get(), render2.get());
}

// --- DI: IA2dpAudioRender interface injection ---

HWTEST_F(A2dpSnkAudioRenderTest, DI_SetInstance_GetInstance, TestSize.Level1)
{
    auto mock = std::make_shared<NiceMock<IA2dpAudioRenderMock>>();
    IA2dpAudioRender::SetInstance(mock);
    auto iface = IA2dpAudioRender::GetInstance();
    EXPECT_NE(iface, nullptr);
    EXPECT_EQ(iface.get(), mock.get());
    IA2dpAudioRender::SetInstance(nullptr);
}

HWTEST_F(A2dpSnkAudioRenderTest, DI_SetInstance_ConcreteRender, TestSize.Level1)
{
    IA2dpAudioRender::SetInstance(render_);
    auto iface = IA2dpAudioRender::GetInstance();
    EXPECT_NE(iface, nullptr);
    EXPECT_EQ(iface.get(), render_.get());
    IA2dpAudioRender::SetInstance(nullptr);
}

HWTEST_F(A2dpSnkAudioRenderTest, DI_GetInstance_NullAfterReset, TestSize.Level1)
{
    IA2dpAudioRender::SetInstance(nullptr);
    auto iface = IA2dpAudioRender::GetInstance();
    EXPECT_EQ(iface, nullptr);
}

// --- No renderer (no CreateRender) negative paths ---

HWTEST_F(A2dpSnkAudioRenderTest, StartRender_NoRenderer_ReturnsFalse, TestSize.Level1)
{
    EXPECT_FALSE(render_->StartRender());
}

HWTEST_F(A2dpSnkAudioRenderTest, StopRender_NoRenderer_ReturnsFalse, TestSize.Level1)
{
    EXPECT_FALSE(render_->StopRender());
}

HWTEST_F(A2dpSnkAudioRenderTest, PauseRender_NoRenderer_ReturnsFalse, TestSize.Level1)
{
    EXPECT_FALSE(render_->PauseRender());
}

HWTEST_F(A2dpSnkAudioRenderTest, WriteStream_NoRenderer_ReturnsFalse, TestSize.Level1)
{
    uint8_t buf[4] = {0};
    EXPECT_FALSE(render_->WriteStream(buf, sizeof(buf)));
}

HWTEST_F(A2dpSnkAudioRenderTest, ReleaseRender_NoRenderer_ReturnsTrue, TestSize.Level1)
{
    EXPECT_TRUE(render_->ReleaseRender());
}

// --- Idempotent release ---

HWTEST_F(A2dpSnkAudioRenderTest, ReleaseRender_Twice_NoCrash, TestSize.Level1)
{
    EXPECT_TRUE(render_->ReleaseRender());
    EXPECT_TRUE(render_->ReleaseRender());
}
