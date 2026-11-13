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

#include "hfphfcallmanager_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#undef private
#define private public
#include "hfp_hf_call_manager.h"
#include "hfp_hf_defines.h"
#include "hfp_hf_service.h"
#include "hands_free_unit_calls.h"
#include "bluetooth_log.h"
#include "raw_address.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {

// Stub for HfpHfService::GetService() — returns nullptr to avoid pulling in
// hfp_hf_service.cpp (which has REGISTER_CLASS_CREATOR causing static init crash).
// HfpHfCallManager::NotifyCallChanged handles nullptr gracefully.
HfpHfService *HfpHfService::GetService()
{
    return nullptr;
}

// Stub for HfpHfService::NotifyCallChanged — empty body, avoids linking hfp_hf_service.cpp.
void HfpHfService::NotifyCallChanged(const RawAddress &device, const HandsFreeUnitCalls &call) {}

static constexpr int HFP_CALL_STATE_MIN = HFP_CALL_STATE_ACTIVE;
static constexpr int HFP_CALL_STATE_MAX = HFP_CALL_STATE_FINISHED;
// Fuzz input bounds for call id — covers outgoing-id (-1), default action (-1), and regular indices.
static constexpr int FUZZ_CALL_ID_MIN = HFP_HF_OUTGOING_CALL_ID - 1;
static constexpr int FUZZ_CALL_ID_MAX = 10;
// Fuzz input bounds for string length consumed from the provider.
static constexpr size_t FUZZ_STR_MAX_LEN = 32;
// Fuzz input bounds for the UpdateCall loop count.
static constexpr int FUZZ_UPDATE_CALL_COUNT_MIN = 1;
static constexpr int FUZZ_UPDATE_CALL_COUNT_MAX = 5;

static HandsFreeUnitCalls BuildCall(FuzzedDataProvider &provider, const std::string &address)
{
    int state = provider.ConsumeIntegralInRange<int>(HFP_CALL_STATE_MIN, HFP_CALL_STATE_MAX);
    int id = provider.ConsumeIntegralInRange<int>(FUZZ_CALL_ID_MIN, FUZZ_CALL_ID_MAX);
    std::string number = provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN);
    bool multiParty = provider.ConsumeBool();
    bool outgoing = provider.ConsumeBool();
    bool inBandRing = provider.ConsumeBool();
    return HandsFreeUnitCalls(address, id, state, number, multiParty, outgoing, inBandRing);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("HfpHfCallManagerFuzzTest start");
    FuzzedDataProvider provider(data, size);
    std::string addr = BuildAddressString(provider);

    HfpHfCallManager manager(addr);

    // --- Query paths: HasCallState / GetCurrentCalls / SetScoConnected ---
    int state = provider.ConsumeIntegralInRange<int>(HFP_CALL_STATE_MIN, HFP_CALL_STATE_MAX);
    manager.HasCallState(state);

    auto calls = manager.GetCurrentCalls();
    HILOGI("HfpHfCallManagerFuzzTest current call size=%{public}zu", calls.size());

    bool isScoConnected = provider.ConsumeBool();
    manager.SetScoConnected(isScoConnected);
    manager.SetScoConnected(!isScoConnected);

    // --- Update paths: UpdateCall (outgoing-id & regular-id) + UpdateCallDone ---
    std::string addrOut = BuildAddressString(provider);
    HandsFreeUnitCalls outgoing(addrOut, HFP_HF_OUTGOING_CALL_ID,
        provider.ConsumeIntegralInRange<int>(HFP_CALL_STATE_MIN, HFP_CALL_STATE_MAX),
        provider.ConsumeRandomLengthString(FUZZ_STR_MAX_LEN),
        provider.ConsumeBool(), provider.ConsumeBool(), provider.ConsumeBool());
    manager.UpdateCall(outgoing);

    int updateCount = provider.ConsumeIntegralInRange<int>(FUZZ_UPDATE_CALL_COUNT_MIN, FUZZ_UPDATE_CALL_COUNT_MAX);
    for (int i = 0; i < updateCount; i++) {
        std::string callAddr = BuildAddressString(provider);
        HandsFreeUnitCalls call = BuildCall(provider, callAddr);
        manager.UpdateCall(call);
    }
    manager.UpdateCallDone();

    // Second UpdateCallDone: clear outgoing-id and exercise the finish branch.
    std::string addrRegular = BuildAddressString(provider);
    HandsFreeUnitCalls regular = BuildCall(provider, addrRegular);
    manager.UpdateCall(regular);
    manager.UpdateCallDone();

    // --- Notify path: NotifyCallChanged (service may be nullptr) ---
    std::string notifyAddr = BuildAddressString(provider);
    HandsFreeUnitCalls notifyCall = BuildCall(provider, notifyAddr);
    manager.NotifyCallChanged(notifyCall);

    HILOGI("HfpHfCallManagerFuzzTest success");
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }

    PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
