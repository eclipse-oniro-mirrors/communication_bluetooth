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

#include "pbappceservice_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "pbap_pce_service.h"
#include "pbap_pce_service_impl.h"
#include "pbap_pce_state_machine.h"
#include "pbap_pce_message.h"
#include "pbap_pce_header_msg.h"
#include "pbap_pce_def.h"
#include "bt_chr_dft_exception.h"
#include "bluetooth_log.h"
#include "raw_address.h"
#include "profile_service_manager.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {

static constexpr int PBAP_PCE_PHONEBOOK_STATE_MIN = PBAP_PCE_PHONEBOOK_STATE_DISCONNECTED;
static constexpr int PBAP_PCE_PHONEBOOK_STATE_MAX = PBAP_PCE_PHONEBOOK_STATE_DOWNLOAD_ERROR;
static constexpr int PBAP_PCE_CONNECT_EVT_MIN = PBAP_PCE_CONNECT_EVT;
static constexpr int PBAP_PCE_CONNECT_EVT_MAX = PBAP_PCE_ABORT_DOWNLOADING_EVT;
static constexpr int PBAP_PCE_STM_MSG_MIN = PCE_SDP_FINISH;
static constexpr int PBAP_PCE_STM_MSG_MAX = PCE_REQ_DOWNLOAD;

class PbapPceFuzzObserver : public IPbapPceObserver {
public:
    PbapPceFuzzObserver() = default;
    ~PbapPceFuzzObserver() override = default;
    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override {}
    void OnSyncStateChange(const RawAddress &device, int syncState) override {}
};

static PbapPceService* GetPbapPceServiceInstance()
{
    auto manager = ProfileServiceManager::GetInstance();
    std::string profileName = "PbapPceService";
    manager->SetProfileService(ADAPTER_BREDR, profileName);
    return static_cast<PbapPceService *>(manager->GetProfileService(profileName));
}

static void TestPbapPceServiceConnectDisconnect(FuzzedDataProvider &provider)
{
    PbapPceService* service = GetPbapPceServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);

    service->Connect(rawAddr);
    service->Disconnect(rawAddr);

    service->GetDeviceState(rawAddr);
    service->GetConnectState();
    service->GetConnectDevices();
    service->IsDownloading(rawAddr);
    service->PullPhoneBook(rawAddr);
    service->AbortDownloading(rawAddr);
}

static void TestPbapPceServicePhoneBookSyncState(FuzzedDataProvider &provider)
{
    PbapPceService* service = GetPbapPceServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);
    int32_t state = provider.ConsumeIntegralInRange<int32_t>(
        PBAP_PCE_PHONEBOOK_STATE_MIN, PBAP_PCE_PHONEBOOK_STATE_MAX);

    service->SetPhoneBookSyncState(addr, static_cast<PbapPcePhoneBookStateType>(state));
    service->GetPhoneBookSyncState(rawAddr);
}

static void TestPbapPceServiceStrategy(FuzzedDataProvider &provider)
{
    PbapPceService* service = GetPbapPceServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);
    int32_t strategy = provider.ConsumeIntegralInRange<int32_t>(0, 2);

    service->SetConnectionStrategy(rawAddr, strategy);
    service->GetConnectionStrategy(rawAddr);
}

static void TestPbapPceServiceDevicesByStates(FuzzedDataProvider &provider)
{
    PbapPceService* service = GetPbapPceServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::vector<int> states = {
        static_cast<int>(BTConnectState::DISCONNECTED),
        static_cast<int>(BTConnectState::CONNECTING),
        static_cast<int>(BTConnectState::CONNECTED),
        static_cast<int>(BTConnectState::DISCONNECTING)
    };
    service->GetDevicesByStates(states);

    int maxNum = service->GetMaxConnectNum();
    HILOGI("TestPbapPceServiceDevicesByStates maxNum=%{public}d", maxNum);
}

static void TestPbapPceServiceObserver(FuzzedDataProvider &provider)
{
    PbapPceService* service = GetPbapPceServiceInstance();
    if (service == nullptr) {
        return;
    }
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);
    int32_t state = provider.ConsumeIntegralInRange<int32_t>(
        static_cast<int32_t>(BTConnectState::DISCONNECTED),
        static_cast<int32_t>(BTConnectState::DISCONNECTING));

    PbapPceFuzzObserver observer;
    service->RegisterObserver(observer);
    service->NotifyStateChanged(rawAddr, state, provider.ConsumeIntegral<int32_t>());
    service->DeregisterObserver(observer);
}

static void TestPbapPceServiceLifecycle(FuzzedDataProvider &provider)
{
    PbapPceService* service = GetPbapPceServiceInstance();
    if (service == nullptr) {
        return;
    }
    bool enableFirst = provider.ConsumeBool();
    if (enableFirst) {
        service->Enable();
    } else {
        service->Disable();
    }
}

static void TestPbapPceImplInterfaces(FuzzedDataProvider &provider)
{
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);
    PbapPceServiceImpl impl;

    impl.Connect(rawAddr);
    impl.Disconnect(rawAddr);
    impl.GetDeviceState(rawAddr);
    impl.GetConnectState();
    impl.GetDevicesByStates({static_cast<int32_t>(BTConnectState::CONNECTED)});
    impl.IsDownloading(rawAddr);
    impl.IsBusy(rawAddr);
    impl.SetBusy(rawAddr, provider.ConsumeBool());
    impl.IsConnected(rawAddr);
    impl.SetPowerStatusBusy(rawAddr, provider.ConsumeBool());
    impl.SetConnectionStrategy(rawAddr, provider.ConsumeIntegralInRange<int32_t>(0, 2));
    impl.GetConnectionStrategy(rawAddr);
    impl.GetPceConfig();

    std::shared_ptr<IPbapPceObserver> observer = std::make_shared<PbapPceFuzzObserver>();
    impl.RegisterObserver(observer);
    impl.DeregisterObserver(observer);

    bool enableFirst = provider.ConsumeBool();
    if (enableFirst) {
        impl.Enable();
    } else {
        impl.Disable();
    }
}

static void TestPbapPceImplMessages(FuzzedDataProvider &provider)
{
    std::string addr = BuildAddressString(provider);
    RawAddress rawAddr(addr);
    PbapPceServiceImpl impl;

    int32_t evtWhat = provider.ConsumeIntegralInRange<int32_t>(PBAP_PCE_CONNECT_EVT_MIN, PBAP_PCE_CONNECT_EVT_MAX);
    PbapPceMessage event(evtWhat);
    event.dev_ = rawAddr;
    event.arg1_ = provider.ConsumeIntegral<int32_t>();
    impl.ProcessConnectEvent(event);

    int32_t msgWhat = provider.ConsumeIntegralInRange<int32_t>(PBAP_PCE_STM_MSG_MIN, PBAP_PCE_STM_MSG_MAX);
    utility::Message msg(msgWhat, provider.ConsumeIntegral<int32_t>());
    impl.ProcessObexRespMessage(rawAddr, msg);
    impl.ProcessPhonebookMessage(rawAddr, msg);
    impl.ProcessSdpMessage(rawAddr, msg);
    impl.ForwardMsgToStm(rawAddr, msg);

    // invalid message id out of enum range
    utility::Message invalidMsg(provider.ConsumeIntegral<int32_t>());
    impl.ProcessObexRespMessage(rawAddr, invalidMsg);
    impl.ProcessPhonebookMessage(rawAddr, invalidMsg);
    impl.ProcessSdpMessage(rawAddr, invalidMsg);
    impl.ForwardMsgToStm(rawAddr, invalidMsg);
}

static void TestPbapPceContactsChr(PbapPceStateMachine &sm, FuzzedDataProvider &provider)
{
    sm.SetConnectFailReason(provider.ConsumeIntegralInRange<int32_t>(
        PBAP_PCE_CONN_FAIL_NONE, PBAP_PCE_CONN_FAIL_TRANSPORT));
    sm.GetConnectFailReason();
    sm.RecordContactsConnecting();
    sm.MarkChrConnConnected();
    sm.MarkChrSyncStart();
    sm.MarkChrSyncFinish(provider.ConsumeBool(),
        provider.ConsumeIntegralInRange<int32_t>(PBAP_PCE_SYNC_FAIL_NONE, PBAP_PCE_SYNC_FAIL_DB_TIMEOUT));
    sm.RecordContactsConnectFailed(provider.ConsumeIntegralInRange<int32_t>(
        PBAP_PCE_CONN_FAIL_NONE, PBAP_PCE_CONN_FAIL_TRANSPORT));
    sm.RecordContactsSyncFinish(provider.ConsumeBool(),
        provider.ConsumeIntegralInRange<int32_t>(PBAP_PCE_SYNC_FAIL_NONE, PBAP_PCE_SYNC_FAIL_DB_TIMEOUT));
    sm.ReportContactsSynUeEvent(provider.ConsumeBool(),
        provider.ConsumeIntegralInRange<int32_t>(PBAP_PCE_CONN_FAIL_NONE, PBAP_PCE_CONN_FAIL_TRANSPORT));
}

static void TestPbapPceStateMachine(FuzzedDataProvider &provider)
{
    std::string addr = BuildAddressString(provider);
    std::shared_ptr<IPbapPceObserver> observer = std::make_shared<PbapPceFuzzObserver>();
    PbapPceServiceImpl impl;
    PbapPceStateMachine sm(RawAddress(addr), impl, *impl.pbapPceSdp_, observer);

    sm.SetVersionNumber(provider.ConsumeIntegral<uint16_t>());
    sm.GetVersionNumber();
    sm.SetSupportedRes(provider.ConsumeIntegral<uint8_t>());
    sm.GetSupportedRes();
    sm.SetSupportedFeature(provider.ConsumeIntegral<uint32_t>());
    sm.GetSupportedFeature();
    sm.SetFeatureFlag(provider.ConsumeBool());
    sm.GetFeatureFlag();
    sm.SetConnectId(provider.ConsumeIntegral<uint32_t>());
    sm.SetTargetState(
        provider.ConsumeIntegralInRange<int32_t>(PCE_TARGET_STATE_UNKNOWN, PCE_TARGET_STATE_DISCONNECTED));
    sm.GetTargetState();
    sm.TransitTargetState();
    sm.ResetObexClient();
    sm.GetObexClientPtr();
    sm.IsBusy();
    sm.SetBusy(provider.ConsumeBool());
    sm.AbortDownloading();
    sm.ForceCloseObexClient();

    TestPbapPceContactsChr(sm, provider);

    int32_t what = provider.ConsumeIntegralInRange<int32_t>(PBAP_PCE_STM_MSG_MIN, PBAP_PCE_STM_MSG_MAX);
    sm.ProcessMessage(utility::Message(what, provider.ConsumeIntegral<int32_t>()));

    utility::Message msg(what, provider.ConsumeIntegral<int32_t>());
    PceDisconnectedState disconnected(PCE_DISCONNECTED_STATE, sm, observer);
    PceConnectingState connecting(PCE_CONNECTING_STATE, sm, observer);
    PceConnectedState connected(PCE_CONNECTED_STATE, sm, observer);
    PceDisconnectingState disconnecting(PCE_DISCONNECTING_STATE, sm, observer);
    disconnected.Dispatch(msg);
    connecting.Dispatch(msg);
    connected.Dispatch(msg);
    disconnecting.Dispatch(msg);

    // clear global sdp callback to avoid dangling after impl destruction
    SdpAdapter::GetInstance().SetPbapPceSdpFoundCallback(nullptr);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("PbapPceServiceFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestPbapPceServiceConnectDisconnect(provider);
    TestPbapPceServicePhoneBookSyncState(provider);
    TestPbapPceServiceStrategy(provider);
    TestPbapPceServiceDevicesByStates(provider);
    //TestPbapPceServiceObserver(provider);
    TestPbapPceServiceLifecycle(provider);
    TestPbapPceImplInterfaces(provider);
    TestPbapPceImplMessages(provider);
    TestPbapPceStateMachine(provider);

    HILOGI("PbapPceServiceFuzzTest success");
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }

    PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
