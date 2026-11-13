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

#include "a2dp_snk_audio_render_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "a2dp_snk_audio_render.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int RENDER_FREQ_MIN = 8000;
static constexpr int RENDER_FREQ_MAX = 192000;
static constexpr int RENDER_BITS_MIN = 8;
static constexpr int RENDER_BITS_MAX = 32;
static constexpr int RENDER_CHANNELS_MIN = 1;
static constexpr int RENDER_CHANNELS_MAX = 8;
static constexpr size_t RENDER_BUFFER_SIZE = 64;

static void TestAudioRenderLifecycle(FuzzedDataProvider &provider)
{
    auto audioRender = A2dpSnkAudioRender::GetInstance();
    if (audioRender == nullptr) {
        return;
    }
    int freq = provider.ConsumeIntegralInRange<int>(RENDER_FREQ_MIN, RENDER_FREQ_MAX);
    int bits = provider.ConsumeIntegralInRange<int>(RENDER_BITS_MIN, RENDER_BITS_MAX);
    int channels = provider.ConsumeIntegralInRange<int>(RENDER_CHANNELS_MIN, RENDER_CHANNELS_MAX);

    audioRender->CreateRender(freq, bits, channels);
    audioRender->StartRender();
    audioRender->PauseRender();
    audioRender->StartRender();
    audioRender->StopRender();
    audioRender->ReleaseRender();
}

static void TestAudioRenderWriteStream(FuzzedDataProvider &provider)
{
    auto audioRender = A2dpSnkAudioRender::GetInstance();
    if (audioRender == nullptr) {
        return;
    }
    uint8_t buffer[RENDER_BUFFER_SIZE] = {};
    for (size_t i = 0; i < RENDER_BUFFER_SIZE; i++) {
        buffer[i] = provider.ConsumeIntegral<uint8_t>();
    }

    audioRender->WriteStream(buffer, RENDER_BUFFER_SIZE);
    audioRender->WriteStream(buffer, 0);
    audioRender->WriteStream(nullptr, 0);
}

static void TestAudioRenderWithoutCreate(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    auto audioRender = A2dpSnkAudioRender::GetInstance();
    if (audioRender == nullptr) {
        return;
    }
    audioRender->StartRender();
    audioRender->StopRender();
    audioRender->PauseRender();
    audioRender->ReleaseRender();
    uint8_t buffer[RENDER_BUFFER_SIZE] = {};
    audioRender->WriteStream(buffer, RENDER_BUFFER_SIZE);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("A2dpSnkAudioRenderFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestAudioRenderLifecycle(provider);
    TestAudioRenderWriteStream(provider);
    TestAudioRenderWithoutCreate(provider);

    HILOGI("A2dpSnkAudioRenderFuzzTest success");
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
