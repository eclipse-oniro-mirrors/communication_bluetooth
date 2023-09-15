/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_IBLUETOOTHOPP_H
#define OHOS_BLUETOOTH_IBLUETOOTHOPP_H

#include <vector>
#include <string_ex.h>
#include <iremote_broker.h>
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_opp_observer.h"

using OHOS::Bluetooth::IBluetoothOppObserver;

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

class IBluetoothOpp : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Bluetooth.IBluetoothOpp");

    virtual ErrCode SendFile(std::string &device, std::vector<std::string> &filePaths,
        std::vector<std::string> &mimeTypes, bool& result) = 0;
    virtual ErrCode SetIncomingFileConfirmation(bool &accept, bool &result) = 0;
    virtual ErrCode GetCurrentTransferInformation(BluetoothIOppTransferInformation &transferInformation) = 0;
    virtual ErrCode CancelTransfer(bool &result) = 0;
    virtual ErrCode RegisterObserver(const sptr<IBluetoothOppObserver> observer) = 0;
    virtual ErrCode DeregisterObserver(const sptr<IBluetoothOppObserver> observer) = 0;
    virtual ErrCode GetDeviceState(const BluetoothRawAddress &device, int& result) = 0;
    virtual ErrCode GetDevicesByStates(const std::vector<int32_t> &states,
        std::vector<BluetoothRawAddress>& result) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_IBLUETOOTHOPP_H