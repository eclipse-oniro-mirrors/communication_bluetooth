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

#include "avrcp_ct_service_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "avrcp_ct_service.h"
#include "avrcp_ct_state_machine.h"
#include "avrcp_ct_message.h"
#include "avrcp_ct_define.h"
#include "profile_service_manager.h"
#include "bluetooth_log.h"
#include "raw_address.h"
#include "bt_def.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static constexpr int AVRCP_CT_EVT_MIN = AVRCP_CT_MSG_INTERNAL_ABS_VOL_TIMEOUT;
static constexpr int AVRCP_CT_EVT_MAX = AVRCP_CT_MSG_PROCESS_IMAGE_DOWNLOADED;

static AvrcpCtService *GetAvrcpCtServiceInstance()
{
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_AVRCP_CT);
    return static_cast<AvrcpCtService *>(manager->GetProfileService(PROFILE_NAME_AVRCP_CT));
}

static void TestAvrcpCtServiceLifecycle(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    bool enableFirst = provider.ConsumeBool();
    if (enableFirst) {
        service->Enable();
    } else {
        service->Disable();
    }
    service->IsEnabled();
}

static void TestAvrcpCtServiceConnect(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);

    service->Connect(rawAddr);
    service->Disconnect(rawAddr);
    service->GetDeviceState(rawAddr);
    service->GetConnectDevices();
    service->GetConnectedDevices();
    service->GetMaxConnectNum();
    service->GetConnectState();
    service->GetActiveDevice();
    service->GetConnectionState(addr);
    std::vector<int> states = {
        AVRCP_CT_STATE_DISCONNECTED,
        AVRCP_CT_STATE_CONNECTING,
        AVRCP_CT_STATE_CONNECTED,
        AVRCP_CT_STATE_DISCONNECTING,
    };
    service->GetDevicesByStates(states);
    service->GetDevicesMatchingConnectionStates();
    service->RemoveStateMachine(addr);
}

static void TestAvrcpCtServiceButton(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    uint8_t button = provider.ConsumeIntegral<uint8_t>();
    uint8_t keyCode = provider.ConsumeIntegral<uint8_t>();
    uint8_t keyState = provider.ConsumeIntegralInRange<uint8_t>(0, 1);

    service->PressButton(rawAddr, button);
    service->ReleaseButton(rawAddr, button);
    service->GetUnitInfo(rawAddr);
    service->GetSubUnitInfo(rawAddr);
    service->SendPassThroughCommand(rawAddr, keyCode, keyState);
}

static void TestAvrcpCtServiceMediaPlayer(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    uint16_t playerId = provider.ConsumeIntegral<uint16_t>();

    service->SetAddressedPlayer(rawAddr, playerId);
    service->SetBrowsedPlayer(rawAddr, playerId);
    service->GetSupportedCompanies(rawAddr);
    service->GetSupportedEvents(rawAddr);
    service->GetPlayStatus(rawAddr);
    service->RequestCurrentMetadata(rawAddr);
    service->RequestPlaybackState(rawAddr);
    service->RequestRemotePlay(rawAddr);
    service->RequestRemotePause(rawAddr);
    service->GetCurrentMetadataIfNoCoverArt(addr);
    service->HandleAddressedPlayerChanged(addr, playerId);
}

static void TestAvrcpCtServicePlayerAppSetting(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    uint8_t attribute = provider.ConsumeIntegral<uint8_t>();
    uint8_t attributeId = provider.ConsumeIntegral<uint8_t>();
    std::vector<uint8_t> attributes = {
        static_cast<uint8_t>(AVRCP_EQUALIZER_STATUS),
        static_cast<uint8_t>(AVRCP_REPEAT_STATUS),
        static_cast<uint8_t>(AVRCP_SHUFFLE_STATUS),
    };
    std::vector<uint8_t> values = {provider.ConsumeIntegral<uint8_t>()};

    service->GetPlayerAppSettingAttributes(rawAddr);
    service->GetPlayerAppSettingValues(rawAddr, attribute);
    service->GetPlayerAppSettingCurrentValue(rawAddr, attributes);
    service->SetPlayerAppSettingCurrentValue(rawAddr, attributes, values);
    service->GetPlayerAppSettingAttributeText(rawAddr, attributes);
    service->GetPlayerAppSettingValueText(rawAddr, attributeId, values);
}

static void TestAvrcpCtServiceMediaInfo(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    uint8_t scope = provider.ConsumeIntegral<uint8_t>();
    uint64_t uid = provider.ConsumeIntegral<uint64_t>();
    uint16_t uidCounter = provider.ConsumeIntegral<uint16_t>();
    uint32_t startItem = provider.ConsumeIntegral<uint32_t>();
    uint32_t endItem = provider.ConsumeIntegral<uint32_t>();
    std::vector<uint32_t> attributes = {provider.ConsumeIntegral<uint32_t>()};

    service->GetElementAttributes(rawAddr, uid, attributes);
    service->PlayItem(rawAddr, scope, uid, uidCounter);
    service->AddToNowPlaying(rawAddr, scope, uid, uidCounter);
    service->GetFolderItems(rawAddr, scope, startItem, endItem, attributes);
    service->GetItemAttributes(rawAddr, scope, uid, uidCounter, attributes);
    service->GetTotalNumberOfItems(rawAddr, scope);
    service->ChangePath(rawAddr, uidCounter, scope, uid);
}

static void TestAvrcpCtServiceVolume(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    uint8_t volume = provider.ConsumeIntegral<uint8_t>();
    uint8_t pduId = provider.ConsumeIntegral<uint8_t>();
    int absVol = provider.ConsumeIntegral<int>();
    int label = provider.ConsumeIntegral<int>();
    int rspType = provider.ConsumeIntegralInRange<int>(0, 1);
    int32_t streamType = provider.ConsumeIntegral<int32_t>();
    int32_t volumeValue = provider.ConsumeIntegral<int32_t>();
    std::vector<uint8_t> events = {provider.ConsumeIntegral<uint8_t>()};

    service->SetAbsoluteVolume(rawAddr, volume);
    service->SendAbsVolumeResponse(rawAddr, absVol, label);
    service->SendRegisterAbsVolResponse(rawAddr, rspType, absVol, label);
    service->RequestContinuingResponse(rawAddr, pduId);
    service->AbortContinuingResponse(rawAddr, pduId);
    service->EnableNotification(rawAddr, events);
    service->DisableNotification(rawAddr, events);
    service->NotifyAudioVolumeEvent(streamType, volumeValue);
}

static void TestAvrcpCtServiceProcessEvent(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    int32_t eventWhat = provider.ConsumeIntegralInRange<int32_t>(AVRCP_CT_EVT_MIN, AVRCP_CT_EVT_MAX);

    // 先建立连接状态机，再投递事件驱�?ProcessEvent 分发
    AvrcpCtMessage connectEvent(AVRCP_CT_CONNECT_EVT);
    connectEvent.dev_ = addr;
    connectEvent.connectFlag_.remoteCtlConnected = provider.ConsumeBool();
    connectEvent.connectFlag_.browsingConnected = provider.ConsumeBool();
    service->PostEvent(connectEvent);

    AvrcpCtMessage event(eventWhat);
    event.dev_ = addr;
    event.arg1_ = provider.ConsumeIntegral<int32_t>();
    event.eventThreeParameter_ = provider.ConsumeIntegral<int32_t>();
    event.rcConnect = provider.ConsumeBool();
    service->PostEvent(event);

    AvrcpCtMessage passthruEvent(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_PLAY);
    passthruEvent.dev_ = addr;
    service->PostEvent(passthruEvent);

    AvrcpCtMessage repeatEvent(AVRCP_MSG_SET_REPEAT_SHUFFLE, AVRCP_REPEAT_OFF);
    repeatEvent.dev_ = addr;
    service->PostEvent(repeatEvent);

    AvrcpCtMessage disconnectEvent(AVRCP_CT_DISCONNECT_EVT);
    disconnectEvent.dev_ = addr;
    service->PostEvent(disconnectEvent);
}

static void TestAvrcpCtServiceCoverArt(FuzzedDataProvider &provider)
{
    AvrcpCtService *service = GetAvrcpCtServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = Bluetooth::BuildAddressString(provider);
    std::string handle = provider.ConsumeBytesAsString(7);
    int psm = provider.ConsumeIntegral<int>();

    service->GetRcPsm(addr, psm);
    service->GetUuidForHandle(addr, handle);
    auto coverArtManager = service->GetCoverArtManager();
    if (coverArtManager != nullptr) {
        coverArtManager->IsConnected(addr);
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("AvrcpCtServiceFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestAvrcpCtServiceLifecycle(provider);
    TestAvrcpCtServiceConnect(provider);
    TestAvrcpCtServiceButton(provider);
    TestAvrcpCtServiceMediaPlayer(provider);
    TestAvrcpCtServicePlayerAppSetting(provider);
    TestAvrcpCtServiceMediaInfo(provider);
    TestAvrcpCtServiceVolume(provider);
    TestAvrcpCtServiceProcessEvent(provider);
    TestAvrcpCtServiceCoverArt(provider);

    HILOGI("AvrcpCtServiceFuzzTest success");
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
