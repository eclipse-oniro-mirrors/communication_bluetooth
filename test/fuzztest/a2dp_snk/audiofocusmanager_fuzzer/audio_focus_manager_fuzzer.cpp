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

#include "audio_focus_manager_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "audio_focus_manager.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static void TestAudioFocusManagerLifecycle(FuzzedDataProvider &provider)
{
    AudioFocusManager focusManager;
    bool initFirst = provider.ConsumeBool();
    if (initFirst) {
        focusManager.InitOnly([](AudioStandard::InterruptHint hint) {
            HILOGI("focus hint=%{public}d", static_cast<int>(hint));
        });
    }
    focusManager.GetSessionId();
    focusManager.ActivateFocus();
    focusManager.GetSessionId();
    focusManager.DeactivateFocus();
    focusManager.DeactivateAndDeinit();
    focusManager.GetSessionId();
}

static void TestAudioFocusManagerWithoutInit(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    AudioFocusManager focusManager;
    focusManager.ActivateFocus();
    focusManager.DeactivateFocus();
    focusManager.DeactivateAndDeinit();
    focusManager.GetSessionId();
}

static void TestAudioFocusManagerCallback(FuzzedDataProvider &provider)
{
    AudioFocusManager focusManager;
    bool received = false;
    int32_t ret = focusManager.InitOnly([&received](AudioStandard::InterruptHint hint) {
        received = true;
        HILOGI("TestAudioFocusManagerCallback hint=%{public}d", static_cast<int>(hint));
    });
    if (ret == 0) {
        focusManager.ActivateFocus();
        focusManager.DeactivateFocus();
    }
    focusManager.DeactivateAndDeinit();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AudioFocusManagerFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestAudioFocusManagerLifecycle(provider);
    TestAudioFocusManagerWithoutInit(provider);
    TestAudioFocusManagerCallback(provider);

    HILOGI("AudioFocusManagerFuzzTest success");
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
