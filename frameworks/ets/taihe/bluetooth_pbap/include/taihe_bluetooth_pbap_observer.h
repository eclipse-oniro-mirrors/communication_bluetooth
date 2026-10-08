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

#ifndef TAIHE_BLUETOOTH_PBAP_OBSERVER_H_
#define TAIHE_BLUETOOTH_PBAP_OBSERVER_H_

#include "bluetooth_pbap_pse.h"
#include "ohos.bluetooth.pbap.proj.hpp"
#include "ohos.bluetooth.pbap.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "taihe_event_module.h"

namespace OHOS {
namespace Bluetooth {

class TaiheBluetoothPbapObserver : public PbapPseObserver {
public:
    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int32_t state, int32_t cause) override;

    TaiheBluetoothPbapObserver();
    ~TaiheBluetoothPbapObserver() override = default;

private:
    friend class PbapServerProfileImpl;
    EventModule<void(::ohos::bluetooth::baseProfile::StateChangeParam const& data)> connectionStateChange_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_BLUETOOTH_PBAP_OBSERVER_H_
