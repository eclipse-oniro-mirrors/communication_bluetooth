/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "bluetooth_a2dp_src_observer_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
BluetoothA2dpSrcObserverStub::BluetoothA2dpSrcObserverStub()
{
    HILOGD("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothA2dpSourceObserverInterfaceCode::BT_A2DP_SRC_OBSERVER_CONNECTION_STATE_CHANGED)] =
        &BluetoothA2dpSrcObserverStub::OnConnectionStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothA2dpSourceObserverInterfaceCode::BT_A2DP_SRC_OBSERVER_PLAYING_STATUS_CHANGED)] =
        &BluetoothA2dpSrcObserverStub::OnPlayingStatusChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothA2dpSourceObserverInterfaceCode::BT_A2DP_SRC_OBSERVER_CONFIGURATION_CHANGED)] =
        &BluetoothA2dpSrcObserverStub::OnConfigurationChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothA2dpSourceObserverInterfaceCode::BT_A2DP_SRC_OBSERVER_MEDIASTACK_CHANGED)] =
        &BluetoothA2dpSrcObserverStub::OnMediaStackChangedInner;
}

BluetoothA2dpSrcObserverStub::~BluetoothA2dpSrcObserverStub()
{
    HILOGD("start.");
    memberFuncMap_.clear();
}

int BluetoothA2dpSrcObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    if (BluetoothA2dpSrcObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t BluetoothA2dpSrcObserverStub::OnConnectionStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int state = data.ReadInt32();

    OnConnectionStateChanged(RawAddress(addr), state);

    return NO_ERROR;
}

int32_t BluetoothA2dpSrcObserverStub::OnPlayingStatusChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int playingState = data.ReadInt32();
    int error = data.ReadInt32();

    OnPlayingStatusChanged(RawAddress(addr), playingState, error);

    return NO_ERROR;
}

int32_t BluetoothA2dpSrcObserverStub::OnConfigurationChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::shared_ptr<BluetoothA2dpCodecInfo> info(data.ReadParcelable<BluetoothA2dpCodecInfo>());
    if (!info) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int error = data.ReadInt32();

    OnConfigurationChanged(RawAddress(addr), *info, error);
    return NO_ERROR;
}

int32_t BluetoothA2dpSrcObserverStub::OnMediaStackChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int action = data.ReadInt32();
    OnMediaStackChanged(RawAddress(addr), action);
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS