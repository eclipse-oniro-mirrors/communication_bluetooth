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

#include "native_a2dp_adapter_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "native_a2dp_adapter.h"
#include "i_native_a2dp_adapter.h"
#include "a2dp_snk_def.h"
#include "bluetooth_log.h"
#include "raw_address.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
namespace OHOS {
namespace bluetooth {

static bt_status_t StubInit(btav_sink_callbacks_t *callbacks, int maxDevices)
{
    return BT_STATUS_SUCCESS;
}

static bt_status_t StubConnect(const STACK::RawAddress &bdAddr)
{
    return BT_STATUS_SUCCESS;
}

static bt_status_t StubDisconnect(const STACK::RawAddress &bdAddr)
{
    return BT_STATUS_SUCCESS;
}

static void StubCleanup(void) {}

static void StubSetAudioFocusState(int focusState) {}

static void StubSetAudioTrackGain(float gain) {}

static bt_status_t StubSetActiveDevice(const STACK::RawAddress &bdAddr)
{
    return BT_STATUS_SUCCESS;
}

static btav_sink_interface_t GetStubInterface()
{
    btav_sink_interface_t iface = {};
    iface.size = sizeof(btav_sink_interface_t);
    iface.init = StubInit;
    iface.connect = StubConnect;
    iface.disconnect = StubDisconnect;
    iface.cleanup = StubCleanup;
    iface.set_audio_focus_state = StubSetAudioFocusState;
    iface.set_audio_track_gain = StubSetAudioTrackGain;
    iface.set_active_device = StubSetActiveDevice;
    return iface;
}

class TestNativeA2dpSinkCallback : public INativeA2dpSinkCallback {
public:
    void OnNativeConnectionStateChanged(const RawAddress &device, btav_connection_state_t state) override
    {
        HILOGI("OnNativeConnectionStateChanged state=%{public}d", static_cast<int>(state));
    }

    void OnNativeAudioStateChanged(const RawAddress &device, btav_audio_state_t state) override
    {
        HILOGI("OnNativeAudioStateChanged state=%{public}d", static_cast<int>(state));
    }

    void OnNativeAudioConfigChanged(const RawAddress &device, uint32_t sampleRate, uint8_t channelCount) override
    {
        HILOGI("OnNativeAudioConfigChanged sampleRate=%{public}d", sampleRate);
    }
};

static void TestNativeAdapterWithStubIface(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    int maxDevices = provider.ConsumeIntegralInRange<int>(0, 8);
    int focusState = provider.ConsumeIntegralInRange<int>(STATE_FOCUS_LOST, STATE_FOCUS_GRANTED);
    float gain = provider.ConsumeFloatingPoint<float>();

    btav_sink_interface_t iface = GetStubInterface();
    TestNativeA2dpSinkCallback callback;
    NativeA2dpAdapter adapter(&iface);

    adapter.Init(&callback, maxDevices);
    adapter.Connect(rawAddr);
    adapter.Disconnect(rawAddr);
    adapter.SetActiveDevice(rawAddr);
    adapter.SetAudioFocusState(focusState);
    adapter.SetAudioTrackGain(gain);
    adapter.Cleanup();
}

static void TestNativeAdapterWithNullIface(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);

    TestNativeA2dpSinkCallback callback;
    NativeA2dpAdapter adapter(nullptr);

    adapter.Init(&callback, 0);
    adapter.Connect(rawAddr);
    adapter.Disconnect(rawAddr);
    adapter.SetActiveDevice(rawAddr);
    adapter.SetAudioFocusState(STATE_FOCUS_GRANTED);
    adapter.SetAudioTrackGain(1.0f);
    adapter.Cleanup();
}

static void TestNativeAdapterWithNullCallback(FuzzedDataProvider &provider)
{
    std::string addr = Bluetooth::BuildAddressString(provider);
    RawAddress rawAddr(addr);
    btav_sink_interface_t iface = GetStubInterface();

    NativeA2dpAdapter adapter(&iface);
    adapter.Init(nullptr, 1);
    adapter.Connect(rawAddr);
    adapter.Cleanup();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("NativeA2dpAdapterFuzzTest start");
    FuzzedDataProvider provider(data, size);

    TestNativeAdapterWithStubIface(provider);
    TestNativeAdapterWithNullIface(provider);
    TestNativeAdapterWithNullCallback(provider);

    HILOGI("NativeA2dpAdapterFuzzTest success");
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
