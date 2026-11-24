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

#ifndef BT_STATE_CHECK_H
#define BT_STATE_CHECK_H

#include <cstdint>

#include "bluetooth_errorcode.h"
#include "bt_caller_identity.h"
#include "interface_adapter_manager.h"
#include "ipc_skeleton.h"
#include "permission_manager.h"

namespace OHOS {
namespace bluetooth {

/**
 * @brief BLE 半开态（受限模式）调用方判定。
 *
 * 注意：受限（半开）态下 BREDR 的原始状态仍为 STATE_TURN_ON，仅上报被屏蔽，
 * 因此必须以 IsBluetoothRestricted() 判断是否处于半开，不能用 BREDR 状态代替。
 *
 * 非受限（全开）      → 放行；
 * 受限（半开）+ BLE 开 → 仅 native / 系统 HAP / BLE owner 白名单放行，其余返回
 *                        Bluetooth::BT_ERR_RESTRICT_STATE；
 * 关闭或切换中        → Bluetooth::BT_ERR_INVALID_STATE。
 *
 * @return Bluetooth::BT_NO_ERROR 放行；否则返回对应错误码。
 */
inline int32_t CheckBleHalfState(const CallerIdentity *caller)
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
    if (caller == nullptr) {
        int32_t pid = IPCSkeleton::GetCallingPid();
        uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
        if (adapterManager->IsBleOwnerMode()) {
            return adapterManager->IsBleOwnerModeApp(pid) ?
                Bluetooth::BT_NO_ERROR : Bluetooth::BT_ERR_RESTRICT_STATE;
        }
        // IsSystemHap 对 native/shell 亦返回 true（非 HAP 类型视为系统侧），故无需再判 IsNativeCaller
        if (Bluetooth::PermissionManager::IsSystemHap(fullTokenId)) {
            return Bluetooth::BT_NO_ERROR;
        }
        return Bluetooth::BT_ERR_RESTRICT_STATE;
    }
    if (adapterManager->IsBleOwnerMode()) {
        return adapterManager->IsBleOwnerModeApp(caller->pid) ?
            Bluetooth::BT_NO_ERROR : Bluetooth::BT_ERR_RESTRICT_STATE;
    }
    // IsSystemHap 对 native/shell 亦返回 true（非 HAP 类型视为系统侧），故无需再判 IsNativeCaller
    if (caller->IsSystemHap()) {
        return Bluetooth::BT_NO_ERROR;
    }
    return Bluetooth::BT_ERR_RESTRICT_STATE;
}

inline int32_t CheckBleHalfStateForCaller()
{
    return CheckBleHalfState(nullptr);
}

/**
 * @brief 复用入口采集的调用方身份，避免半开态下重复读取 IPCSkeleton/AccessTokenKit。
 */
inline int32_t CheckBleHalfStateForCaller(const CallerIdentity &caller)
{
    return CheckBleHalfState(&caller);
}

} // namespace bluetooth
} // namespace OHOS

#endif // BT_STATE_CHECK_H
