/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <chrono>
#include <thread>

#include "play_helper.cpp"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
static BluetoothHost *host_;
static PlayHelper playHelper_;

class PlayHelperTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: PlayHelperTest_InitAudioRender_001
 * @tc.desc: InitAudioRender
 * @tc.type: FUNC
 */
HWTEST_F(PlayHelperTest, PlayHelperTest_InitAudioRender_001, TestSize.Level1)
{
    HILOGI("PlayHelperTest_InitAudioRender_001 enter");
    std::unique_ptr<AudioStandard::AudioRenderer> audioRenderer =
        AudioStandard::AudioRenderer::Create(AudioStandard::AudioStreamType::STREAM_MUSIC);
    EXPECT_TRUE(playHelper_.InitAudioRender(audioRenderer));
    playHelper_.PlaySilenceSound();
    audioRenderer->Release();

    audioRenderer = nullptr;
    EXPECT_FALSE(playHelper_.InitAudioRender(nullptr));
    HILOGI("PlayHelperTest_InitAudioRender_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS