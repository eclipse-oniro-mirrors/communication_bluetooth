/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <singleton.h>

#include "bas_fuzzer.h"
#include "permission_manager.h"
#include "bluetooth_bas_stub.h"
#include "bluetooth_bas_server.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "bt_def.h"
#include "bluetooth_log.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "securec.h"
#include "raw_address.h"

using namespace std;
using namespace OHOS::Bluetooth;
using namespace OHOS::bluetooth;

namespace OHOS {
constexpr uint32_t MESSAGE_SIZE = BluetoothBasInterfaceCode::BT_BAS_BUTT;

bool DoSomethingInterestingWithBasAPI(FuzzedDataProvider &provider)
{
    std::string address = BuildAddressString(provider);

    BluetoothBasServer basServer;
    for (uint32_t code = 0; code < MESSAGE_SIZE + 1; code++) {
        MessageParcel datas;
        std::u16string descriptor = BluetoothBasStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteString(address.c_str());
        sptr<IRemoteObject> observer = nullptr;
        datas.WriteRemoteObject(observer);
        MessageParcel reply;
        MessageOption option;

        basServer.OnRemoteRequest(code, datas, reply, option);
    }
    HILOGI("OnRemoteRequest success");
    return true;
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        HILOGI("invalid data");
        return 0;
    }
    PermissionManager::SetAllGranted();
    FuzzedDataProvider provider(data, size);
    OHOS::DoSomethingInterestingWithBasAPI(provider);
    return 0;
}