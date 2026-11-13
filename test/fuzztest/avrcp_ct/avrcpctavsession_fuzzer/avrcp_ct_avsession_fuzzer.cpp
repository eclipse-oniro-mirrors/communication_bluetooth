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

#include "avrcp_ct_avsession_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_avsession.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int32_t AV_SESSION_TYPE_MAX = 2;
static constexpr size_t IMAGE_DATA_SIZE = 32;

static void TestAVSessionCreateDestroy(FuzzedDataProvider &provider)
{
    AvrcpCtAVSession avSession;
    std::string tag = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    int32_t type = provider.ConsumeIntegralInRange<int32_t>(0, AV_SESSION_TYPE_MAX);
    AppExecFwk::ElementName elementName;
    elementName.SetAbilityName(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16)));
    elementName.SetBundleName(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16)));

    avSession.CreateAVSession(tag, type, elementName);
    avSession.Destroy();
    avSession.Destroy();
}

static void TestAVSessionCommands(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    AvrcpCtAVSession avSession;
    AppExecFwk::ElementName elementName;
    avSession.CreateAVSession("fuzz_avrcp_ct", 0, elementName);

    avSession.AddSupportCommands();
    avSession.DeleteSupportCommands();
    avSession.Activate();
    avSession.Deactivate();
    avSession.Activate();
    avSession.Destroy();
}

static void TestAVSessionUpdateInfo(FuzzedDataProvider &provider)
{
    AvrcpCtAVSession avSession;
    AppExecFwk::ElementName elementName;
    avSession.CreateAVSession("fuzz_avrcp_ct", 0, elementName);

    AVSession::AVMetaData metaData;
    metaData.SetTitle(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16)));
    metaData.SetArtist(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16)));
    avSession.UpdateMetaData(metaData);

    AVSession::AVPlaybackState playbackState;
    playbackState.SetState(provider.ConsumeIntegral<int32_t>());
    AVSession::AVPlaybackState::Position pos;
    pos.elapsedTime_ = provider.ConsumeIntegral<int32_t>();
    pos.updateTime_ = provider.ConsumeIntegral<int64_t>();
    playbackState.SetPosition(pos);
    avSession.UpdatePlaybackState(playbackState);
    avSession.GetPlaybackState();

    std::vector<AVSession::AVQueueItem> items;
    AVSession::AVQueueItem item;
    item.SetItemId(provider.ConsumeIntegral<int32_t>());
    items.push_back(item);
    avSession.UpdatePlayingQueueItems(items);
    avSession.ClearPlayingQueueItems();
    avSession.Destroy();
}

static void TestAVSessionWithoutCreate(FuzzedDataProvider &provider)
{
    (void)provider.ConsumeBool();
    AvrcpCtAVSession avSession;

    avSession.RegisterCallback(nullptr);
    avSession.AddSupportCommands();
    avSession.DeleteSupportCommands();
    avSession.Activate();
    avSession.Deactivate();
    AVSession::AVMetaData metaData;
    avSession.UpdateMetaData(metaData);
    AVSession::AVPlaybackState playbackState;
    avSession.UpdatePlaybackState(playbackState);
    avSession.GetPlaybackState();
    avSession.ClearPlayingQueueItems();
    avSession.Destroy();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtAVSessionFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestAVSessionCreateDestroy(provider);
    TestAVSessionCommands(provider);
    TestAVSessionUpdateInfo(provider);
    TestAVSessionWithoutCreate(provider);

    HILOGI("AvrcpCtAVSessionFuzzTest success");
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
