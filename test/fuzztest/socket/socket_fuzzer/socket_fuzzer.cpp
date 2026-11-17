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

#include "socket_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "bluetooth_socket_stub.h"
#include "bluetooth_socket_server.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
using namespace OHOS::bluetooth;
namespace OHOS {
constexpr uint32_t MESSAGE_SIZE = BluetoothSocketInterfaceCode::SOCKET_BUTT;

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    auto addr = BuildAddressString(provider);
    bluetooth::RawAddress rawAddr(addr);
    BluetoothRawAddress bluetoothAddr(rawAddr);
    BluetoothUuid *bluetoothUuid = new BluetoothUuid();

    BluetoothSocketServer socketServer;
    for (uint32_t code = 0; code < MESSAGE_SIZE + 1; code++) {
        MessageParcel datas;
        std::u16string descriptor = BluetoothSocketStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteString(addr.c_str());
        datas.WriteParcelable(&bluetoothAddr);
        datas.WriteParcelable(bluetoothUuid);
        datas.WriteInt32(provider.ConsumeIntegral<int32_t>());
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;

        socketServer.OnRemoteRequest(code, datas, reply, option);
    }
    HILOGI("OnRemoteRequest success");
    return true;
}

bool TestForSocketServer(const uint8_t* data, size_t size)
{
    HILOGI("start");
    std::shared_ptr<BluetoothSocketServer> socketServer = std::make_shared<BluetoothSocketServer>();
    ConnectSocketParam param;
    FuzzedDataProvider provider(data, size);
    auto addr = BuildAddressString(provider);
    bluetooth::RawAddress rawAddr(addr);
    BluetoothRawAddress bluetoothAddr(rawAddr);
    param.addr = addr;
    param.securityFlag = provider.ConsumeIntegral<int32_t>();
    param.type = provider.ConsumeIntegral<int32_t>();
    param.psm = provider.ConsumeIntegral<int32_t>();
    int fd = provider.ConsumeIntegral<int>();
    socketServer->Connect(param, fd);
    ListenSocketParam listenParam;
    sptr<IBluetoothServerSocketObserver> serverObserver;
    listenParam.observer = serverObserver;
    socketServer->Listen(listenParam, fd);
    BluetoothSocketCocInfo info;
    socketServer->UpdateCocConnectionParams(info);
    Uuid uuid = Uuid::ConvertFromString("00003301-0000-1000-8000-00805F9B34FB");
    sptr<IBluetoothClientSocketObserver> clientObserver;
    socketServer->RegisterClientObserver(bluetoothAddr, uuid, clientObserver);
    int32_t socketType = provider.ConsumeIntegral<int32_t>();
    bool isAllowed = provider.ConsumeBool();
    socketServer->IsAllowSocketConnect(socketType, addr, isAllowed);
    socketServer->DeregisterClientObserver(bluetoothAddr, uuid, clientObserver);
    socketServer->DeregisterServerObserver(serverObserver);
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
    OHOS::TestForSocketServer(data, size);
    return 0;
}