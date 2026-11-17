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


/*
 * 本文件定义蓝牙 PAN(个人区域网络，Personal Area Network)规范接口，
 * 用于通过蓝牙在网络设备间建立 IP 网络连接(网络共享/蓝牙组网)。
 */

#ifndef VENDOR_INCLUDE_BT_PAN_H
#define VENDOR_INCLUDE_BT_PAN_H

#include "hardware/bluetooth.h"
#include "types/raw_address.h"

extern "C" { //__BEGIN_DECLS

#define BTPAN_ROLE_NONE 0 /* 未指定角色 */
#define BTPAN_ROLE_PANNAP 1 /* PAN角色：网络接入点(NAP，提供网络共享) */
#define BTPAN_ROLE_PANU 2 /* PAN角色：PAN用户(PANU，使用网络接入) */

/* PAN 连接状态 */
typedef enum {
  BTPAN_STATE_CONNECTED = 0, /* 已连接 */
  BTPAN_STATE_CONNECTING = 1, /* 连接中 */
  BTPAN_STATE_DISCONNECTED = 2, /* 已断开 */
  BTPAN_STATE_DISCONNECTING = 3 /* 断开中 */
} btpan_connection_state_t;

/* PAN 服务开关状态 */
typedef enum {
  BTPAN_STATE_ENABLED = 0, /* PAN服务已开启 */
  BTPAN_STATE_DISABLED = 1 /* PAN服务已关闭 */
} btpan_control_state_t;

/**
 * Callback for pan connection state
 */
typedef void (*btpan_connection_state_callback)(btpan_connection_state_t state, /* PAN连接状态变化回调 */
                                                bt_status_t error, /* 连接操作结果状态 */
                                                const RawAddress* bd_addr, /* 远端设备地址 */
                                                int local_role, /* 本端PAN角色 */
                                                int remote_role); /* 对端PAN角色 */
typedef void (*btpan_control_state_callback)(btpan_control_state_t state, /* PAN服务开关状态变化回调 */
                                             int local_role, bt_status_t error, /* 本端角色及开关操作结果 */
                                             const char* ifname); /* 服务开启后创建的网络接口名 */

/* PAN 服务回调结构体 */
typedef struct {
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  btpan_control_state_callback control_state_cb; /* 服务开关状态变化回调 */
  btpan_connection_state_callback connection_state_cb; /* 连接状态变化回调 */
} btpan_callbacks_t;
/* PAN 服务接口结构体 */
typedef struct {
  /** set to size of this struct*/
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  /**
   * Initialize the pan interface and register the btpan callbacks
   */
  bt_status_t (*init)(const btpan_callbacks_t* callbacks); /* 初始化PAN接口并注册回调 */
  /*
   * enable the pan service by specified role. The result state of
   * enabl will be returned by btpan_control_state_callback. when pan-nap is
   * enabled, the state of connecting panu device will be notified by
   * btpan_connection_state_callback
   */
  bt_status_t (*enable)(int local_role);
  /*
   * get current pan local role
   */
  int (*get_local_role)(void);
  /**
   * start bluetooth pan connection to the remote device by specified pan role.
   * The result state will be returned by btpan_connection_state_callback
   */
  bt_status_t (*connect)(const RawAddress* bd_addr, int local_role,
                         int remote_role);
  /**
   * stop bluetooth pan connection. The result state will be returned by
   * btpan_connection_state_callback
   */
  bt_status_t (*disconnect)(const RawAddress* bd_addr);

  /**
   * Cleanup the pan interface
   */
  void (*cleanup)(void);

} btpan_interface_t;

} //__END_DECLS

#endif /* VENDOR_INCLUDE_BT_PAN_H */
