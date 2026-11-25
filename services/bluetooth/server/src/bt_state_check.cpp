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

#include "bt_state_check.h"

#include "interface_adapter_manager.h"
#include "ipc_skeleton.h"
#include "permission_manager.h"

namespace OHOS {
namespace bluetooth {

namespace {

int32_t CheckBleRestrictedCaller(CallerIdentity *caller)
{
    IAdapterManager *adapterManager = IAdapterManager::GetInstance();
    int32_t pid = (caller != nullptr) ? caller->pid : IPCSkeleton::GetCallingPid();
    // owner 白名单优先；未命中时回落到系统 HAP / native，与 service 层判据一致。
    if (adapterManager->IsBleOwnerMode() && adapterManager->IsBleOwnerModeApp(pid)) {
        return Bluetooth::BT_NO_ERROR;
    }
    bool isSystemHap = (caller != nullptr) ? caller->IsSystemHap() :
        Bluetooth::PermissionManager::IsSystemHap(IPCSkeleton::GetCallingFullTokenID());
    return isSystemHap ? Bluetooth::BT_NO_ERROR : Bluetooth::BT_ERR_RESTRICT_STATE;
}

int32_t CheckBleHalfState(CallerIdentity *caller)
{
    IAdapterManager *adapterManager = IAdapterManager::GetInstance();
    if (adapterManager == nullptr) {
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    bool isBleOn = adapterManager->GetState(BTTransport::ADAPTER_BLE) == BTStateID::STATE_TURN_ON;
    bool isBrOn = adapterManager->GetState(BTTransport::ADAPTER_BREDR) == BTStateID::STATE_TURN_ON;
    if (!adapterManager->IsBluetoothRestricted()) {
        return (isBrOn || isBleOn) ? Bluetooth::BT_NO_ERROR : Bluetooth::BT_ERR_INVALID_STATE;
    }
    if (!isBleOn) {
        return Bluetooth::BT_ERR_INVALID_STATE;
    }
    return CheckBleRestrictedCaller(caller);
}

} // namespace

int32_t CheckBleHalfStateForCaller()
{
    return CheckBleHalfState(nullptr);
}

int32_t CheckBleHalfStateForCaller(CallerIdentity &caller)
{
    return CheckBleHalfState(&caller);
}

} // namespace bluetooth
} // namespace OHOS
