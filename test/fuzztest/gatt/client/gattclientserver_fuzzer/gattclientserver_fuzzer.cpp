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

#include "gattclientserver_fuzzer.h"
#include "permission_manager.h"
#include "../../../bt_utils/fuzztest_utils.h"
#include "bluetooth_gatt_client_stub.h"
#include "bluetooth_gatt_client_server.h"
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "raw_address.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
constexpr uint32_t MESSAGE_SIZE = BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_BUTT;

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    uint32_t code = provider.ConsumeIntegralInRange<uint32_t>(0, MESSAGE_SIZE);
    auto addr = BuildAddressString(provider);
    bluetooth::RawAddress rawAddr(addr);
    BluetoothRawAddress bluetoothAddr(rawAddr);

    MessageParcel datas;
    std::u16string descriptor = BluetoothGattClientStub::GetDescriptor();
    datas.WriteInterfaceToken(descriptor);
    datas.WriteString(addr.c_str());
    datas.WriteInt32(provider.ConsumeIntegral<int32_t>());
    datas.RewindRead(0);
    datas.WriteParcelable(&bluetoothAddr);
    MessageParcel reply;
    MessageOption option;

    HILOGI("start code: %{public}d, size: %{public}zu", code, size);
    BluetoothGattClientServer gattClient;
    gattClient.OnRemoteRequest(code, datas, reply, option);
    HILOGI("OnRemoteRequest success");
    return true;
}
bool TestForCharacteristic(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    auto addr = BuildAddressString(provider);
    bluetooth::RawAddress rawAddr(addr);
    BluetoothRawAddress bluetoothAddr(rawAddr);

    BluetoothGattCharacteristic *characteristic = new BluetoothGattCharacteristic();
    
    BluetoothGattClientServer gattClient;
    uint32_t codes[] = {BT_GATT_CLIENT_WRITE_CHARACTERISTIC, BT_GATT_CLIENT_SIGNED_WRITE_CHARACTERISTIC};
    for (uint32_t code : codes) {
        MessageParcel datas;
        std::u16string descriptor = BluetoothGattClientStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteInt32(*(reinterpret_cast<const int32_t *>(data)));
        datas.WriteParcelable(characteristic);
        datas.WriteString(addr.c_str());
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        gattClient.OnRemoteRequest(code, datas, reply, option);
    }
    HILOGI("OnRemoteRequest success");
    return true;
}
bool TestForDescriptor(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    auto addr = BuildAddressString(provider);
    bluetooth::RawAddress rawAddr(addr);
    BluetoothRawAddress bluetoothAddr(rawAddr);

    BluetoothGattDescriptor *bluetoothGattDescriptor = new BluetoothGattDescriptor();
    
    BluetoothGattClientServer gattClient;
    uint32_t codes[] = {BT_GATT_CLIENT_READ_DESCRIPTOR, BT_GATT_CLIENT_WRITE_DESCRIPTOR};
    for (uint32_t code : codes) {
        MessageParcel datas;
        std::u16string descriptor = BluetoothGattClientStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteInt32(*(reinterpret_cast<const int32_t *>(data)));
        datas.WriteParcelable(bluetoothGattDescriptor);
        datas.WriteString(addr.c_str());
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        gattClient.OnRemoteRequest(code, datas, reply, option);
    }
    HILOGI("OnRemoteRequest success");
    return true;
}
bool TestForAddress(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    auto addr = BuildAddressString(provider);
    bluetooth::RawAddress rawAddr(addr);
    BluetoothRawAddress bluetoothAddr(rawAddr);

    MessageParcel datas;
    std::u16string descriptor = BluetoothGattClientStub::GetDescriptor();
    datas.WriteInterfaceToken(descriptor);
    datas.WriteParcelable(&bluetoothAddr);
    datas.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    BluetoothGattClientServer gattClient;
    gattClient.OnRemoteRequest(BT_GATT_CLIENT_REQUEST_FASTEST_CONNECTION, datas, reply, option);
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
    OHOS::TestForCharacteristic(data, size);
    OHOS::TestForDescriptor(data, size);
    OHOS::TestForAddress(data, size);
    return 0;
}