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

namespace OHOS {
namespace bluetooth {

/**
 * @brief BLE 半开态（受限模式）调用方判定。
 *
 * 注意：受限（半开）态下 BREDR 的原始状态仍为 STATE_TURN_ON，仅上报被屏蔽，
 * 因此必须以 IsBluetoothRestricted() 判断是否处于半开，不能用 BREDR 状态代替。
 *
 * 非受限（全开）      → BREDR 或 BLE 为 ON 放行；否则 BT_ERR_INVALID_STATE；
 * 受限（半开）+ BLE 开 → BLE owner 白名单、系统 HAP / native 放行；其余
 *                        BT_ERR_RESTRICT_STATE；
 * 关闭或切换中        → BT_ERR_INVALID_STATE。
 *
 * 语义边界：
 * 1. 以 IPC 入口时刻的状态/身份为准，进入 GATT 等业务线程后不再二次判定；
 * 2. BT_ERR_RESTRICT_STATE 在 NAPI/Taihe 层统一映射为 BT_ERR_INTERNAL_ERROR，
 *    对外不区分“策略拒绝”与“内部错误”；
 * 3. 判定语义变更需与 gatt_client_service 的受限分支保持同步。
 *
 * @return Bluetooth::BT_NO_ERROR 放行；否则返回对应错误码。
 */
int32_t CheckBleHalfStateForCaller();

/**
 * @brief 复用入口采集的调用方身份，避免半开态下重复读取 IPCSkeleton/AccessTokenKit。
 *        判定语义与无参版本一致。
 */
int32_t CheckBleHalfStateForCaller(CallerIdentity &caller);

} // namespace bluetooth
} // namespace OHOS

#endif // BT_STATE_CHECK_H
