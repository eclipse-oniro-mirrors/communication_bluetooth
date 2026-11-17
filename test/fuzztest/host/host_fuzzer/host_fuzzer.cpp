/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "host_fuzzer.h"
#include "permission_manager.h"
#include "bluetooth_host_stub.h"
#include "bluetooth_host_server.h"
#include "bluetooth_errorcode.h"
#include "bt_def.h"
#include "bluetooth_log.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "securec.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <set>

using namespace std;
using namespace OHOS::Bluetooth;
using namespace OHOS::bluetooth;
namespace OHOS {
    constexpr uint32_t MESSAGE_SIZE = BluetoothHostInterfaceCode::BT_HOST_BUTT;

    static std::string generateFromTwoUint8(FuzzedDataProvider &provider)
    {
        uint8_t high_byte = provider.ConsumeIntegral<uint8_t>();
        uint8_t low_byte = provider.ConsumeIntegral<uint8_t>();
        uint16_t combined_16bit = (static_cast<uint16_t>(high_byte) << 8) | low_byte;
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0')
            << std::setw(4) << combined_16bit;
        std::string hex_16bit = ss.str();
        return "0000" + hex_16bit + "-0000-1000-8000-00805F9B34FB";
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::string address = BuildAddressString(provider);

        for (uint32_t code = 0; code < MESSAGE_SIZE; code++) {
            MessageParcel datas;
            std::u16string descriptor = BluetoothHostStub::GetDescriptor();
            datas.WriteInterfaceToken(descriptor);
            datas.WriteString(address.c_str());
            datas.WriteInt32(provider.ConsumeIntegral<int32_t>());
            datas.RewindRead(0);
            MessageParcel reply;
            MessageOption option;

            HILOGI("OnRemoteRequest code: %{public}d, size: %{public}zu", code, size);
            DelayedSingleton<BluetoothHostServer>::GetInstance()->OnRemoteRequest(
                code, datas, reply, option);
            HILOGI("OnRemoteRequest success");
        }
        return true;
    }

    bool GetRemoteDeviceInfoFuzzTest(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::string address = BuildAddressString(provider);
        MessageParcel datas;
        std::u16string descriptor = BluetoothHostStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteString(address);
        datas.WriteInt32(provider.ConsumeIntegral<int32_t>());
        MessageParcel reply;
        MessageOption option;
        uint32_t code = BluetoothHostInterfaceCode::GET_DEVICE_INFO_ID;

        HILOGI("start code: %{public}d, size: %{public}zu", code, size);
        sptr<BluetoothHostServer> hostServer = BluetoothHostServer::GetInstance();
        hostServer->OnRemoteRequest(code, datas, reply, option);
        int32_t result = reply.ReadInt32();
        if (result != BT_NO_ERROR) {
            HILOGI("result code: %{public}d", result);
            return false;
        }
        HILOGI("OnRemoteRequest success");
        return true;
    }

    bool IsSupportVirtualAutoConnectFuzzTest(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::string address = BuildAddressString(provider);
        MessageParcel datas;
        std::u16string descriptor = BluetoothHostStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteString(address);
        MessageParcel reply;
        MessageOption option;
        uint32_t code = BluetoothHostInterfaceCode::GET_VIRTUAL_AUTO_CONN_SWITCH;

        HILOGI("start code: %{public}d, size: %{public}zu", code, size);
        sptr<BluetoothHostServer> hostServer = BluetoothHostServer::GetInstance();
        hostServer->OnRemoteRequest(code, datas, reply, option);
        bool result = reply.ReadBool();
        HILOGI("result code: %{public}d", result);
        return true;
    }

    bool SetVirtualAutoConnectTypeFuzzTest(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::string address = BuildAddressString(provider);
        MessageParcel datas;
        std::u16string descriptor = BluetoothHostStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteString(address);
        datas.WriteInt32(provider.ConsumeIntegral<int32_t>());
        datas.WriteInt32(provider.ConsumeIntegral<int32_t>());
        MessageParcel reply;
        MessageOption option;
        uint32_t code = BluetoothHostInterfaceCode::SET_VIRTUAL_AUTO_CONN_TYPE;

        HILOGI("start code: %{public}d, size: %{public}zu", code, size);
        sptr<BluetoothHostServer> hostServer = BluetoothHostServer::GetInstance();
        hostServer->OnRemoteRequest(code, datas, reply, option);
        HILOGI("OnRemoteRequest success");
        return true;
    }

    bool SetFastScanLevel(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        std::string address = BuildAddressString(provider);
        MessageParcel datas;
        std::u16string descriptor = BluetoothHostStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteInt32(provider.ConsumeIntegral<int32_t>());
        MessageParcel reply;
        MessageOption option;
        uint32_t code = BluetoothHostInterfaceCode::SET_FAST_SCAN_LEVEL;

        HILOGI("start code: %{public}d, size: %{public}zu", code, size);
        sptr<BluetoothHostServer> hostServer = BluetoothHostServer::GetInstance();
        hostServer->OnRemoteRequest(code, datas, reply, option);
        HILOGI("OnRemoteRequest success");
        return true;
    }

    bool GetRemoteServices(const uint8_t* data, size_t size)
    {
        HILOGI("start GetRemoteServices");
        FuzzedDataProvider provider(data, size);
        std::string address = BuildAddressString(provider);
        MessageParcel datas;
        std::u16string descriptor = BluetoothHostStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteString(address);
        MessageParcel reply;
        MessageOption option;
        uint32_t code = BluetoothHostInterfaceCode::GET_REMOTE_SERVICES;

        HILOGI("start code: %{public}d, size: %{public}zu", code, size);
        sptr<BluetoothHostServer> hostServer = BluetoothHostServer::GetInstance();
        hostServer->OnRemoteRequest(code, datas, reply, option);
        HILOGI("GetRemoteServices, OnRemoteRequest success");
        return true;
    }

    bool StartRemoteSdpSearch(const uint8_t* data, size_t size)
    {
        HILOGI("start StartRemoteSdpSearch");
        FuzzedDataProvider provider(data, size);
        std::string address = BuildAddressString(provider);
        std::string uuid = generateFromTwoUint8(provider);
        MessageParcel datas;
        std::u16string descriptor = BluetoothHostStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteString(address);
        datas.WriteString(uuid);
        MessageParcel reply;
        MessageOption option;
        uint32_t code = BluetoothHostInterfaceCode::START_REMOTE_SDP_SEARCH;

        HILOGI("start code: %{public}d, size: %{public}zu", code, size);
        sptr<BluetoothHostServer> hostServer = BluetoothHostServer::GetInstance();
        hostServer->OnRemoteRequest(code, datas, reply, option);
        HILOGI("StartRemoteSdpSearch, OnRemoteRequest success");
        return true;
    }

    bool VerifyMultiPermissionsFuzzTest(const uint8_t* data, size_t size)
    {
        HILOGI("start VerifyMultiPermissionsFuzzTest");
        FuzzedDataProvider provider(data, size);
        bool systemHapNeeded = provider.ConsumeBool();
        int32_t dataSize = provider.ConsumeIntegralInRange<int32_t>(0, 100);
        std::set<std::string> permissionsSet;
        for (int32_t i = 0; i < dataSize; i++) {
            std::string permission = provider.ConsumeRandomLengthString();
            permissionsSet.insert(permission);
        }
        MessageParcel datas;
        std::u16string descriptor = BluetoothHostStub::GetDescriptor();
        datas.WriteInterfaceToken(descriptor);
        datas.WriteBool(systemHapNeeded);
        datas.WriteInt32(dataSize);
        for (const auto& permission : permissionsSet) {
            datas.WriteString(permission);
        }
        MessageParcel reply;
        MessageOption option;
        uint32_t code = BluetoothHostInterfaceCode::BT_VERIFY_MULTI_PERMISSIONS;

        HILOGI("start code: %{public}d, size: %{public}zu", code, size);
        sptr<BluetoothHostServer> hostServer = BluetoothHostServer::GetInstance();
        hostServer->OnRemoteRequest(code, datas, reply, option);
        HILOGI("VerifyMultiPermissionsFuzzTest, OnRemoteRequest success");
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        HILOGI("invalid data");
        return 0;
    }
    /* Run your code on data */
    PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::GetRemoteDeviceInfoFuzzTest(data, size);
    OHOS::IsSupportVirtualAutoConnectFuzzTest(data, size);
    OHOS::SetVirtualAutoConnectTypeFuzzTest(data, size);
    OHOS::SetFastScanLevel(data, size);
    OHOS::GetRemoteServices(data, size);
    OHOS::StartRemoteSdpSearch(data, size);
    OHOS::VerifyMultiPermissionsFuzzTest(data, size);
    return 0;
}