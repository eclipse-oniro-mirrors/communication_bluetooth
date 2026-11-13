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

#ifndef VENDOR_INCLUDE_BT_GATT_H
#define VENDOR_INCLUDE_BT_GATT_H

#include <stdint.h>
#include "ble_advertiser.h"
#include "ble_scanner.h"
#include "bt_gatt_client.h"
#include "bt_gatt_server.h"

__BEGIN_DECLS

/** BT-GATT callbacks —— GATT 协议栈回调注册结构体，用于向协议栈注册各类事件的回调接口 */
typedef struct {
  /** Set to sizeof(btgatt_callbacks_t) —— 结构体大小，用于版本/大小校验 */
  size_t size;

  /** GATT Client callbacks —— GATT 客户端（主设备）回调接口 */
  const btgatt_client_callbacks_t* client;

  /** GATT Server callbacks —— GATT 服务端（从设备）回调接口 */
  const btgatt_server_callbacks_t* server;

  /** LE scanner callbacks —— BLE 扫描回调接口 */
  const btgatt_scanner_callbacks_t* scanner;
} btgatt_callbacks_t;

/** Represents the standard Bluetooth GATT interface.
 *  GATT 标准接口结构体 —— 协议栈向上层暴露的 GATT 总入口，
 *  聚合了客户端、服务端、扫描和广播四个子接口 */
typedef struct {
  /** Set to sizeof(btgatt_interface_t) —— 结构体大小，用于版本/大小校验 */
  size_t size;

  /**
   * Initializes the interface and provides callback routines
   * 初始化 GATT 接口并注册回调
   */
  bt_status_t (*init)(const btgatt_callbacks_t* callbacks);

  /** Closes the interface —— 关闭/反初始化 GATT 接口，释放资源 */
  void (*cleanup)(void);

  /** Pointer to the GATT client interface methods.
   *  GATT 客户端接口：连接、服务发现、读写特征值、订阅通知等 */
  const btgatt_client_interface_t* client;

  /** Pointer to the GATT server interface methods.
   *  GATT 服务端接口：添加服务、响应读写请求、发送通知等 */
  const btgatt_server_interface_t* server;

  /** Pointer to the LE scanner interface methods.
   *  BLE 扫描接口：启动/停止扫描、配置过滤器等 */
  BleScannerInterface* scanner;

  /** Pointer to the advertiser interface methods.
   *  BLE 广播接口：启动/停止广播、设置广播参数/数据等 */
  BleAdvertiserInterface* advertiser;
} btgatt_interface_t;

__END_DECLS

#endif /* VENDOR_INCLUDE_BT_GATT_H */
