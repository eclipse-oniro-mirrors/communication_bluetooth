/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#pragma once

#ifdef __cplusplus
#include <cstdint>
#include <string>
#else
#include <stdint.h>
#endif

/* Device Types
 */
/* 蓝牙设备类型标志位，可用位运算组合 */
enum {
  BT_DEVICE_TYPE_BREDR = (1 << 0), /* 仅支持经典蓝牙(BR/EDR) */
  BT_DEVICE_TYPE_BLE = (1 << 1), /* 仅支持低功耗蓝牙(BLE) */
  BT_DEVICE_TYPE_DUMO = BT_DEVICE_TYPE_BREDR | BT_DEVICE_TYPE_BLE, /* 双模设备(同时支持BR/EDR和BLE) */
};
typedef uint8_t tBT_DEVICE_TYPE; /* 设备类型掩码类型 */

#ifdef __cplusplus
/* 将设备类型掩码转换为可读字符串，用于日志/调试输出 */
inline std::string DeviceTypeText(tBT_DEVICE_TYPE type) {
  /* 根据掩码值匹配设备类型并返回对应文本 */
  switch (type) {
    /* 仅经典蓝牙设备 */
    case BT_DEVICE_TYPE_BREDR:
      return std::string("BR_EDR");
    /* 仅低功耗蓝牙设备 */
    case BT_DEVICE_TYPE_BLE:
      return std::string("BLE");
    /* 双模设备(经典+低功耗) */
    case BT_DEVICE_TYPE_DUMO:
      return std::string("DUAL");
    /* 未匹配到任何已知类型时返回Unknown */
    default:
      return std::string("Unknown"); /* 未知/非法类型 */
  }
}
#endif  // __cplusplus
