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

#include "hfphfserver_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "bluetooth_hfp_hf_stub.h"
#include "bluetooth_hfp_hf_server.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "raw_address.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
constexpr uint32_t MESSAGE_SIZE = BT_HFP_HF_BUTT;

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    auto addr = BuildAddressString(provider);
    bluetooth::RawAddress rawAddr(addr);
    BluetoothRawAddress bluetoothAddr(rawAddr);

    BluetoothHfpHfServer hfpHfServer;
    for (uint32_t code = 0; code < MESSAGE_SIZE + 1; code++) {
        MessageParcel datas;
        std::u16string descriptor = BluetoothHfpHfStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteParcelable(&bluetoothAddr);
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        hfpHfServer.OnRemoteRequest(code, datas, reply, option);
    }
    HILOGI("OnRemoteRequest success");
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        HILOGI("invalid data");
        return 0;
    }

    /* Run your code on data */
    PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}