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

#ifndef TAIHE_BLUETOOTH_OPP_OBSERVER_H_
#define TAIHE_BLUETOOTH_OPP_OBSERVER_H_

#include "bluetooth_opp.h"
#include "ohos.bluetooth.opp.proj.hpp"
#include "ohos.bluetooth.opp.impl.hpp"
#include "taihe/runtime.hpp"
#include "taihe_event_module.h"

namespace OHOS {
namespace Bluetooth {

class TaiheBluetoothOppObserver : public OppObserver {
public:
    TaiheBluetoothOppObserver();
    ~TaiheBluetoothOppObserver() override = default;

    void OnReceiveIncomingFileChanged(const BluetoothOppTransferInformation &transferInformation) override;
    void OnTransferStateChanged(const BluetoothOppTransferInformation &transferInformation) override;

private:
    friend class OppServerProfileImpl;
    EventModule<void(::ohos::bluetooth::opp::OppTransferInformation const& data)> receiveIncomingFile_;
    EventModule<void(::ohos::bluetooth::opp::OppTransferInformation const& data)> transferStateChange_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_BLUETOOTH_OPP_OBSERVER_H_
