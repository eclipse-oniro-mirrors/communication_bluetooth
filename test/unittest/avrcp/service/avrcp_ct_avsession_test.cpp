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
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "avrcp_ct_avsession.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class AvrcpCtAvSessionTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        avSession_ = std::make_unique<AvrcpCtAVSession>();
    }
    void TearDown() override
    {
        avSession_ = nullptr;
    }

protected:
    std::unique_ptr<AvrcpCtAVSession> avSession_;
};

/*
 * @tc.number: AvrcpCtAvSessionTest_Initial_001
 * @tc.name: Initial state
 * @tc.desc: Test avsession is not created initially
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_Initial_001, TestSize.Level1)
{
    EXPECT_EQ(avSession_->avSession_, nullptr);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_Destroy_001
 * @tc.name: Destroy without session
 * @tc.desc: Test Destroy is a no-op when session is not created
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_Destroy_001, TestSize.Level1)
{
    avSession_->Destroy();
    EXPECT_EQ(avSession_->avSession_, nullptr);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_RegisterCallback_001
 * @tc.name: RegisterCallback without session
 * @tc.desc: Test RegisterCallback is a no-op when session is not created
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_RegisterCallback_001, TestSize.Level1)
{
    avSession_->RegisterCallback(nullptr);
    EXPECT_EQ(avSession_->avSession_, nullptr);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_SupportCommands_001
 * @tc.name: SupportCommands without session
 * @tc.desc: Test AddSupportCommands/DeleteSupportCommands are no-ops when session is not created
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_SupportCommands_001, TestSize.Level1)
{
    avSession_->AddSupportCommands();
    avSession_->DeleteSupportCommands();
    EXPECT_EQ(avSession_->avSession_, nullptr);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_Activate_001
 * @tc.name: Activate/Deactivate without session
 * @tc.desc: Test Activate/Deactivate are no-ops when session is not created
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_Activate_001, TestSize.Level1)
{
    avSession_->Activate();
    EXPECT_FALSE(avSession_->isActive_);
    avSession_->Deactivate();
    EXPECT_FALSE(avSession_->isActive_);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_UpdateMetaData_001
 * @tc.name: UpdateMetaData without session
 * @tc.desc: Test UpdateMetaData is a no-op when session is not created
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_UpdateMetaData_001, TestSize.Level1)
{
    AVSession::AVMetaData meta;
    meta.SetTitle("title");
    avSession_->UpdateMetaData(meta);
    EXPECT_EQ(avSession_->avSession_, nullptr);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_UpdatePlaybackState_001
 * @tc.name: UpdatePlaybackState without session
 * @tc.desc: Test UpdatePlaybackState is a no-op when session is not created
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_UpdatePlaybackState_001, TestSize.Level1)
{
    AVSession::AVPlaybackState state;
    state.SetState(AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY);
    avSession_->UpdatePlaybackState(state);
    EXPECT_EQ(avSession_->avSession_, nullptr);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_UpdatePlayingQueueItems_001
 * @tc.name: UpdatePlayingQueueItems without session
 * @tc.desc: Test UpdatePlayingQueueItems/ClearPlayingQueueItems are no-ops when session is not created
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_UpdatePlayingQueueItems_001, TestSize.Level1)
{
    std::vector<AVSession::AVQueueItem> items;
    avSession_->UpdatePlayingQueueItems(items);
    avSession_->ClearPlayingQueueItems();
    EXPECT_EQ(avSession_->avSession_, nullptr);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_CreateAVSession_001
 * @tc.name: CreateAVSession
 * @tc.desc: Test CreateAVSession does not crash in test environment
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_CreateAVSession_001, TestSize.Level1)
{
    OHOS::AppExecFwk::ElementName elementName;
    elementName.SetBundleName("bluetooth.ohos.avsession");
    elementName.SetAbilityName("bluetooth.ability");
    avSession_->CreateAVSession("bluetooth", 0, elementName);

    // Session creation may fail in unit test environment, all methods must be safe either way
    avSession_->RegisterCallback(nullptr);
    avSession_->AddSupportCommands();
    avSession_->Activate();
    avSession_->Deactivate();
    avSession_->DeleteSupportCommands();
    avSession_->Destroy();
    EXPECT_EQ(avSession_->avSession_, nullptr);
    EXPECT_FALSE(avSession_->isActive_);
}

/*
 * @tc.number: AvrcpCtAvSessionTest_CreateAVSession_002
 * @tc.name: CreateAVSession twice
 * @tc.desc: Test CreateAVSession keeps existing session
 */
HWTEST_F(AvrcpCtAvSessionTest, AvrcpCtAvSessionTest_CreateAVSession_002, TestSize.Level1)
{
    OHOS::AppExecFwk::ElementName elementName;
    avSession_->CreateAVSession("bluetooth", 0, elementName);
    auto firstSession = avSession_->avSession_;

    avSession_->CreateAVSession("bluetooth", 0, elementName);
    EXPECT_EQ(avSession_->avSession_, firstSession);

    avSession_->Destroy();
    EXPECT_EQ(avSession_->avSession_, nullptr);
}
} // namespace bluetooth
} // namespace OHOS
