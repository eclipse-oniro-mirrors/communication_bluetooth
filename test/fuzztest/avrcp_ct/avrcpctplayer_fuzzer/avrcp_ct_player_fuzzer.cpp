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

#include "avrcp_ct_player_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_player.h"
#include "avrcp_ct_media_item.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int PLAYER_TYPE_MAX = 3;
static constexpr int PLAY_STATUS_MAX = 5;

static void TestPlayerBasicInfo(FuzzedDataProvider &provider)
{
    AvrcpCtPlayer player;
    int id = provider.ConsumeIntegral<int>();
    std::string name = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    int type = provider.ConsumeIntegralInRange<int>(0, PLAYER_TYPE_MAX);
    int playStatus = provider.ConsumeIntegralInRange<int>(0, PLAY_STATUS_MAX);
    std::string address = Bluetooth::BuildAddressString(provider);

    player.SetPlayerId(id);
    player.GetPlayerId();
    player.SetPlayerName(name);
    player.GetPlayerName();
    player.SetPlayerType(type);
    player.GetPlayerType();
    player.SetPlayStatus(playStatus);
    player.GetPlayStatus();
    player.SetDeviceAddress(address);
    player.GetDeviceAddress();
}

static void TestPlayerPlaybackInfo(FuzzedDataProvider &provider)
{
    AvrcpCtPlayer player;
    int64_t playTime = provider.ConsumeIntegral<int64_t>();
    float playSpeed = provider.ConsumeFloatingPoint<float>();
    int64_t duration = provider.ConsumeIntegral<int64_t>();
    int64_t playPosition = provider.ConsumeIntegral<int64_t>();

    player.SetPlayTime(playTime);
    player.GetPlayTime();
    player.SetPlaySpeed(playSpeed);
    player.GetPlaySpeed();
    player.SetDuration(duration);
    player.GetDuration();
    player.SetPlayPosition(playPosition);
    player.GetPlayPosition();
    player.ToAVPlaybackState();
}

static void TestPlayerFeatures(FuzzedDataProvider &provider)
{
    AvrcpCtPlayer player;
    int featureId = provider.ConsumeIntegralInRange<int>(0, 63);
    std::vector<uint8_t> features;
    size_t featureSize = provider.ConsumeIntegralInRange<int>(0, 16);
    for (size_t i = 0; i < featureSize; i++) {
        features.push_back(provider.ConsumeIntegral<uint8_t>());
    }

    player.SetSupportedFeatures(features);
    player.SetSupportedFeature(featureId);
    player.SupportsFeature(featureId);
}

static void TestPlayerCurrentTrack(FuzzedDataProvider &provider)
{
    AvrcpCtPlayer player;
    auto track = std::make_shared<AvrcpCtMediaItem>();
    track->SetTitle(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16)));
    track->SetCoverArtHandle(provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 10)));

    player.UpdateCurrentTrack(track);
    player.GetCurrentTrack();
    player.HasCoverArt();
    player.GetCoverArtHandle();
}

static void TestPlayerNotifyImageDownload(FuzzedDataProvider &provider)
{
    AvrcpCtPlayer player;
    std::string uuid = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));
    std::string uri = provider.ConsumeBytesAsString(provider.ConsumeIntegralInRange<int>(0, 16));

    player.NotifyImageDownload(uuid, uri);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtPlayerFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestPlayerBasicInfo(provider);
    TestPlayerPlaybackInfo(provider);
    TestPlayerFeatures(provider);
    TestPlayerCurrentTrack(provider);
    TestPlayerNotifyImageDownload(provider);

    HILOGI("AvrcpCtPlayerFuzzTest success");
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
