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

#include "bt_chr_airtime_fuzzer.h"

#include "bt_chr_airtime.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "bluetooth_log.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace bluetooth {
namespace {
constexpr size_t FUZZ_STRING_MAX_LEN = 32;
constexpr int32_t FUZZ_HANDLE_MAX = 100;
constexpr int32_t FUZZ_LEN_MIN = -1;
constexpr int32_t FUZZ_LEN_MAX = 256;

std::string ConsumeOptionalString(FuzzedDataProvider &provider)
{
    return provider.ConsumeRandomLengthString(FUZZ_STRING_MAX_LEN);
}

void FuzzMediaAirtime(const std::string &addr, FuzzedDataProvider &provider)
{
    int32_t value = provider.ConsumeIntegral<int32_t>();
    BtChr2g4BleConnInterval(addr, provider.ConsumeIntegral<uint16_t>());
    BtChr2g4A2dpStart(addr);
    BtChr2g4A2dpActive(addr, value, value, value);
    BtChr2g4A2dpSinkCodec(addr, value, value);
    BtChr2g4A2dpStop(addr);
    BtChr2g4A2dpFail(addr, value, value);
    BtChr2g4ScoStart(addr);
    BtChr2g4ScoActive(addr, value, value, value);
    BtChr2g4ScoStop(addr, value, value);
    BtChr2g4ScoFail(addr, value, value);
}

void FuzzHidAirtime(const std::string &addr, FuzzedDataProvider &provider)
{
    int32_t value = provider.ConsumeIntegral<int32_t>();
    int32_t len = provider.ConsumeIntegralInRange<int32_t>(FUZZ_LEN_MIN, FUZZ_LEN_MAX);
    BtChr2g4BrHidStart(addr);
    BtChr2g4BrHidActive(addr);
    BtChr2g4BrHidReportData(addr, len);
    BtChr2g4BrHidStop(addr, value);
    BtChr2g4BrHidFail(addr, value);
    BtChr2g4BleHogpStart(addr);
    BtChr2g4BleHogpActive(addr);
    BtChr2g4BleHogpReportData(addr, len);
    BtChr2g4BleHogpStop(addr, value);
    BtChr2g4BleHogpFail(addr, value);
}

void FuzzSocketAirtime(const std::string &addr, FuzzedDataProvider &provider)
{
    int32_t handle = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_HANDLE_MAX);
    int32_t value = provider.ConsumeIntegral<int32_t>();
    int32_t len = provider.ConsumeIntegralInRange<int32_t>(FUZZ_LEN_MIN, FUZZ_LEN_MAX);
    BtChr2g4BrSocketStart(handle, addr, value, value);
    BtChr2g4BrSocketActive(handle, addr);
    BtChr2g4BrSocketData(handle, len);
    BtChr2g4BrSocketStop(handle, value);
    BtChr2g4BrSocketFail(addr, value, value);
}

void FuzzGattAirtime(const std::string &addr, FuzzedDataProvider &provider)
{
    int32_t value = provider.ConsumeIntegral<int32_t>();
    int32_t len = provider.ConsumeIntegralInRange<int32_t>(FUZZ_LEN_MIN, FUZZ_LEN_MAX);
    std::string pkgName = ConsumeOptionalString(provider);
    BtChr2g4BleGattPkgNameSet(addr, pkgName);
    BtChr2g4BleGattStart(addr, pkgName);
    BtChr2g4BleGattActive(addr);
    BtChr2g4BleGattTx(addr, len);
    BtChr2g4BleGattRx(addr, len);
    BtChr2g4BleGattStop(addr, value);
    BtChr2g4BleGattFail(addr, value, value);
}

void FuzzScanAirtime(FuzzedDataProvider &provider)
{
    int32_t value = provider.ConsumeIntegral<int32_t>();
    std::string pkgName = ConsumeOptionalString(provider);
    BtChr2g4BleScanPkgNameSet(pkgName);
    BtChr2g4BleScanParams(value, value, value, pkgName);
    BtChr2g4BleScanEnable(provider.ConsumeBool(), pkgName);
    BtChr2g4BleScanDisable();
    BtChr2g4BleScanFlush();
}

void FuzzAdvAirtime(FuzzedDataProvider &provider)
{
    int32_t handle = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_HANDLE_MAX);
    int32_t value = provider.ConsumeIntegral<int32_t>();
    std::string pkgName = ConsumeOptionalString(provider);
    BtChr2g4BleAdvPkgNameSet(handle, pkgName);
    BtChr2g4BleAdvStart(handle, value, value, value, pkgName);
    BtChr2g4BleAdvActive(handle);
    BtChr2g4BleAdvStop(handle);
    BtChr2g4BleAdvFail(handle, value);
}

void FuzzDiscAirtime(FuzzedDataProvider &provider)
{
    int32_t value = provider.ConsumeIntegral<int32_t>();
    BtChr2g4DiscStart(value, value, value);
    BtChr2g4DiscActive(value);
    BtChr2g4DiscStop(value, value);
    BtChr2g4DiscFlushAll();
}
}  // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string addr = Bluetooth::BuildAddressString(provider);
    FuzzMediaAirtime(addr, provider);
    FuzzHidAirtime(addr, provider);
    FuzzSocketAirtime(addr, provider);
    FuzzGattAirtime(addr, provider);
    FuzzScanAirtime(provider);
    FuzzAdvAirtime(provider);
    FuzzDiscAirtime(provider);
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
