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
 * 文件说明：蓝牙 GATT 客户端（BT-GATT Client）协议栈接口定义。
 * 定义 GATT 客户端注册/连接/断开、服务发现、特征值与描述符读写、
 * 通知/指示注册、MTU 配置、PHY 设置、连接参数更新、GATT 数据库
 * 获取等接口函数指针，以及对应的参数结构与回调结构体。
 */

#ifndef VENDOR_INCLUDE_BT_GATT_CLIENT_H
#define VENDOR_INCLUDE_BT_GATT_CLIENT_H

#include <stdint.h>
#include <vector>
/* 模块公共头文件：蓝牙UUID定义 */
#include <bluetooth/uuid.h>

#include "bt_common_types.h"
#include "bt_gatt_types.h"

extern "C" { //__BEGIN_DECLS

/**
 * Buffer sizes for maximum attribute length and maximum read/write
 * operation buffer size.
 */
#define BTGATT_MAX_ATTR_LEN 600 /* 属性值最大长度（读/写操作缓冲区大小） */

/** Buffer type for unformatted reads/writes */ /* 非格式化读/写操作的缓冲类型 */
typedef struct {
  uint8_t value[BTGATT_MAX_ATTR_LEN]; /* 数据缓冲区 */
  uint16_t len; /* 数据长度 */
} btgatt_unformatted_value_t;

/** Parameters for GATT read operations */ /* GATT 读操作结果参数 */
typedef struct {
  uint16_t handle; /* 属性句柄 */
  btgatt_unformatted_value_t value; /* 读到的数据 */
  uint16_t value_type; /* 值类型 */
  uint8_t status; /* 操作状态 */
} btgatt_read_params_t;

/** Parameters for GATT write operations */ /* GATT 写操作结果参数 */
typedef struct {
  btgatt_srvc_id_t srvc_id; /* 服务 ID */
  btgatt_gatt_id_t char_id; /* 特征值 ID */
  btgatt_gatt_id_t descr_id; /* 描述符 ID */
  uint8_t status; /* 操作状态 */
} btgatt_write_params_t;

/** Parameters for GATT response context */ /* GATT 响应上下文参数 */
typedef struct {
  uint64_t timeStamp; /* 时间戳 */
} btgatt_rsp_params_t;

/** Attribute change notification parameters */ /* 属性变化通知参数 */
typedef struct {
  uint8_t value[BTGATT_MAX_ATTR_LEN]; /* 通知数据 */
  RawAddress bda; /* 远端设备地址 */
  uint16_t handle; /* 属性句柄 */
  uint16_t len; /* 数据长度 */
  uint8_t is_notify; /* 是否为 notify（否则为 indication） */
} btgatt_notify_params_t;

/* GATT 测试模式参数结构体 */
typedef struct {
  RawAddress* bda1; /* 设备地址 */
  bluetooth::Uuid* uuid1; /* UUID */
  uint16_t u1; /* 通用参数 1 */
  uint16_t u2; /* 通用参数 2 */
  uint16_t u3; /* 通用参数 3 */
  uint16_t u4; /* 通用参数 4 */
  uint16_t u5; /* 通用参数 5 */
} btgatt_test_params_t;

/* BT GATT client error codes */ /* GATT 客户端错误码枚举 */
typedef enum {
  BT_GATTC_COMMAND_SUCCESS = 0, /* 0  Command succeeded                 */ /* 命令执行成功 */
  BT_GATTC_COMMAND_STARTED,     /* 1  Command started OK.               */ /* 命令已开始执行 */
  BT_GATTC_COMMAND_BUSY,        /* 2  Device busy with another command  */ /* 设备忙于其他命令 */
  BT_GATTC_COMMAND_STORED,      /* 3 request is stored in control block */ /* 请求已存入控制块 */
  BT_GATTC_NO_RESOURCES,        /* 4  No resources to issue command     */ /* 无资源执行命令 */
  BT_GATTC_MODE_UNSUPPORTED,    /* 5  Request for 1 or more unsupported modes */ /* 请求包含不支持的模式 */
  BT_GATTC_ILLEGAL_VALUE,       /* 6  Illegal command /parameter value  */ /* 非法命令/参数值 */
  BT_GATTC_INCORRECT_STATE,     /* 7  Device in wrong state for request  */ /* 设备状态错误 */
  BT_GATTC_UNKNOWN_ADDR,        /* 8  Unknown remote BD address         */ /* 未知远端设备地址 */
  BT_GATTC_DEVICE_TIMEOUT,      /* 9  Device timeout                    */ /* 设备超时 */
  BT_GATTC_INVALID_CONTROLLER_OUTPUT, /* 10  An incorrect value was received
                                         from HCI */ /* 收到不正确的 HCI 返回值 */
  BT_GATTC_SECURITY_ERROR, /* 11 Authorization or security failure or not
                              authorized  */ /* 鉴权/安全失败或未授权 */
  BT_GATTC_DELAYED_ENCRYPTION_CHECK, /*12 Delayed encryption check */ /* 延迟加密检查 */
  BT_GATTC_ERR_PROCESSING            /* 12 Generic error                     */ /* 一般错误 */
} btgattc_error_t;

/** BT-GATT Client callback structure. */ /* BT-GATT 客户端回调结构体 */

/** Callback invoked in response to register_client */ /* 注册 GATT 客户端的结果回调 */
typedef void (*register_client_callback)(int status, int client_if,
                                         const bluetooth::Uuid& app_uuid);

/** GATT open callback invoked in response to open */ /* 发起连接（open）的结果回调 */
typedef void (*connect_callback)(int conn_id, int status, int client_if,
                                 const RawAddress& bda);

/** Callback invoked in response to close */ /* 断开连接的结果回调 */
typedef void (*disconnect_callback)(int conn_id, int status, int client_if,
                                    const RawAddress& bda, int reason);

/** Callback to cancel open */ /* 取消连接的结果回调 */
typedef void (*cancel_open_callback)(int conn_id, int status, int client_if, const RawAddress& bda);

/**
 * Invoked in response to search_service when the GATT service search
 * has been completed.
 */ /* 服务搜索完成回调 */
typedef void (*search_complete_callback)(int conn_id, int status);

/** Callback invoked in response to [de]register_for_notification */ /* 注册/注销通知的结果回调 */
typedef void (*register_for_notification_callback)(int conn_id, int registered,
                                                   int status, uint16_t handle);

/**
 * Remote device notification callback, invoked when a remote device sends
 * a notification or indication that a client has registered for.
 */ /* 远端设备通知/指示回调 */
typedef void (*notify_callback)(int conn_id,
                                const btgatt_notify_params_t& p_data);

/** Reports result of a GATT read operation */ /* 读特征值操作结果回调 */
typedef void (*read_characteristic_callback)(int conn_id, int status,
                                             btgatt_read_params_t* p_data);

/** GATT write characteristic operation callback */ /* 写特征值操作结果回调 */
typedef void (*write_characteristic_callback)(int conn_id, int status,
                                              uint16_t handle, const btgatt_rsp_params_t& p_data);

/** GATT execute prepared write callback */ /* 执行预备写操作结果回调 */
typedef void (*execute_write_callback)(int conn_id, int status);

/** Callback invoked in response to read_descriptor */ /* 读描述符操作结果回调 */
typedef void (*read_descriptor_callback)(int conn_id, int status,
                                         const btgatt_read_params_t& p_data);

/** Callback invoked in response to write_descriptor */ /* 写描述符操作结果回调 */
typedef void (*write_descriptor_callback)(int conn_id, int status,
                                          uint16_t handle);

/** Callback triggered in response to read_remote_rssi */ /* 读取远端 RSSI 结果回调 */
typedef void (*read_remote_rssi_callback)(int client_if, const RawAddress& bda,
                                          int rssi, int status);

/** Callback invoked when the MTU for a given connection changes */ /* 连接 MTU 变更回调 */
typedef void (*configure_mtu_callback)(int conn_id, int status, int mtu);

/**
 * Callback notifying the application of the congestion status of a
 * remote device connection. When congested is true, the connection
 * flow is congested and cannot accept more outgoing data; the
 * application should pause sending until a subsequent callback
 * reports congested as false.
 *
 * @param conn_id connection identifier
 * @param congested true if the connection is congested, false if the
 *                  congestion has been cleared
 */
/* 连接拥塞状态变化回调：congested为true时应暂停发送数据，收到false后可恢复 */
typedef void (*congestion_callback)(int conn_id, bool congested);

/** GATT get database callback */ /* 获取 GATT 数据库回调 */
typedef void (*get_gatt_db_callback)(int conn_id, const btgatt_db_element_t* db,
                                     int count);

/** GATT services between start_handle and end_handle were removed */ /* 指定句柄范围的服务被移除回调 */
typedef void (*services_removed_callback)(int conn_id, uint16_t start_handle,
                                          uint16_t end_handle);

/** GATT services were added */ /* 服务新增回调 */
typedef void (*services_added_callback)(int conn_id,
                                        const btgatt_db_element_t& added,
                                        int added_count);

/** Callback invoked when the PHY for a given connection changes */ /* 连接 PHY 更新回调 */
typedef void (*phy_updated_callback)(int conn_id, uint8_t tx_phy,
                                     uint8_t rx_phy, uint8_t status);

/** Callback invoked when the connection parameters for a given connection
 * changes */ /* 连接参数更新回调 */
typedef void (*conn_updated_callback)(int conn_id, uint16_t interval,
                                      uint16_t latency, uint16_t timeout,
                                      uint8_t status);

/** Callback when services are changed */ /* 服务变更（service changed）回调 */
typedef void (*service_changed_callback)(int conn_id);

/** Callback when gatt connection state changed */ /* GATT 连接状态变化回调 */
typedef void (*conn_state_callback)(int client_if, const RawAddress& bda, int conn_id,
                                    bool connected, int reason);

/* GATT 客户端回调集合结构体 */
typedef struct {
  register_client_callback register_client_cb; /* 注册客户端结果回调 */
  connect_callback open_cb; /* 连接建立回调 */
  disconnect_callback close_cb; /* 断开连接回调 */
  cancel_open_callback cancel_open_cb; /* 取消连接回调 */
  search_complete_callback search_complete_cb; /* 服务搜索完成回调 */
  register_for_notification_callback register_for_notification_cb; /* 注册/注销通知回调 */
  notify_callback notify_cb; /* 通知/指示回调 */
  read_characteristic_callback read_characteristic_cb; /* 读特征值回调 */
  write_characteristic_callback write_characteristic_cb; /* 写特征值回调 */
  read_descriptor_callback read_descriptor_cb; /* 读描述符回调 */
  write_descriptor_callback write_descriptor_cb; /* 写描述符回调 */
  execute_write_callback execute_write_cb; /* 执行预备写回调 */
  read_remote_rssi_callback read_remote_rssi_cb; /* 读远端 RSSI 回调 */
  configure_mtu_callback configure_mtu_cb; /* MTU 配置回调 */
  congestion_callback congestion_cb; /* 拥塞状态回调 */
  get_gatt_db_callback get_gatt_db_cb; /* 获取 GATT 数据库回调 */
  services_removed_callback services_removed_cb; /* 服务移除回调 */
  services_added_callback services_added_cb; /* 服务新增回调 */
  phy_updated_callback phy_updated_cb; /* PHY 更新回调 */
  conn_updated_callback conn_updated_cb; /* 连接参数更新回调 */
  service_changed_callback service_changed_cb; /* 服务变更回调 */
  conn_state_callback conn_state_cb; /* 连接状态回调 */
} btgatt_client_callbacks_t;

/** Represents the standard BT-GATT client interface. */ /* 标准 GATT 客户端接口结构体 */

typedef struct {
  /** Registers a GATT client application with the stack */ /* 向协议栈注册 GATT 客户端应用 */
  bt_status_t (*register_client)(const bluetooth::Uuid& uuid,
                                 bool eatt_support);

  /** Unregister a client application from the stack */ /* 从协议栈注销客户端应用 */
  bt_status_t (*unregister_client)(int client_if);

  /** Create a connection to a remote LE or dual-mode device */ /* 连接远端 LE 或双模设备 */
  bt_status_t (*connect)(int client_if, const RawAddress& bd_addr,
                         bool is_direct, int transport, bool opportunistic,
                         int initiating_phys);

  /** Disconnect a remote device or cancel a pending connection */ /* 断开远端设备或取消待定连接 */
  bt_status_t (*disconnect)(int client_if, const RawAddress& bd_addr,
                            int conn_id);

  /** Clear the attribute cache for a given device */ /* 清除指定设备的属性缓存 */
  bt_status_t (*refresh)(int client_if, const RawAddress& bd_addr);

  /**
   * Enumerate all GATT services on a connected device.
   * Optionally, the results can be filtered for a given UUID.
   */ /* 枚举已连接设备的全部 GATT 服务，可按 UUID 过滤 */
  bt_status_t (*search_service)(int conn_id,
                                const bluetooth::Uuid* filter_uuid);

  /**
   * Sead "Find service by UUID" request. Used only for PTS tests.
   */ /* 发送按 UUID 查找服务请求（仅用于 PTS 测试） */
  void (*btif_gattc_discover_service_by_uuid)(int conn_id,
                                              const bluetooth::Uuid& uuid);

  /** Read a characteristic on a remote device */ /* 按句柄读远端设备特征值 */
  bt_status_t (*read_characteristic)(int conn_id, uint16_t handle,
                                     int auth_req);

  /** Read a characteristic on a remote device */ /* 按 UUID 读远端设备特征值 */
  bt_status_t (*read_using_characteristic_uuid)(int conn_id,
                                                const bluetooth::Uuid& uuid,
                                                uint16_t s_handle,
                                                uint16_t e_handle,
                                                int auth_req);

  /** Write a remote characteristic */ /* 写远端特征值 */
  bt_status_t (*write_characteristic)(int conn_id, uint16_t handle,
                                      int write_type, int auth_req,
                                      std::vector<uint8_t> value);

  /** Read the descriptor for a given characteristic */ /* 读指定特征值的描述符 */
  bt_status_t (*read_descriptor)(int conn_id, uint16_t handle, int auth_req);

  /** Write a remote descriptor for a given characteristic */ /* 写指定特征值的远端描述符 */
  bt_status_t (*write_descriptor)(int conn_id, uint16_t handle, int auth_req,
                                  std::vector<uint8_t> value);

  /** Execute a prepared write operation */ /* 执行预备写操作 */
  bt_status_t (*execute_write)(int conn_id, int execute);

  /**
   * Register to receive notifications or indications for a given
   * characteristic
   */ /* 注册接收指定特征值的通知/指示 */
  bt_status_t (*register_for_notification)(int client_if,
                                           const RawAddress& bd_addr,
                                           uint16_t handle);

  /** Deregister a previous request for notifications/indications */ /* 注销先前的通知/指示注册 */
  bt_status_t (*deregister_for_notification)(int client_if,
                                             const RawAddress& bd_addr,
                                             uint16_t handle);

  /** Request RSSI for a given remote device */ /* 请求读取远端设备 RSSI */
  bt_status_t (*read_remote_rssi)(int client_if, const RawAddress& bd_addr);

  /** Determine the type of the remote device (LE, BR/EDR, Dual-mode) */ /* 获取远端设备类型（LE/BR-EDR/双模） */
  int (*get_device_type)(const RawAddress& bd_addr);

  /** Configure the MTU for a given connection */ /* 配置指定连接的 MTU */
  bt_status_t (*configure_mtu)(int conn_id, int mtu);

  /**
   * Request a connection parameter update (LE connection parameter
   * update procedure) for the given remote device.
   *
   * @param bd_addr remote device address
   * @param min_interval minimum connection interval, in 1.25ms units
   * @param max_interval maximum connection interval, in 1.25ms units
   * @param latency secondary latency, in connection events
   * @param timeout supervision timeout, in 10ms units
   * @param min_ce_len minimum connection event length, in 0.625ms units
   * @param max_ce_len maximum connection event length, in 0.625ms units
   */
  /* 请求LE连接参数更新：连接间隔(min/max，单位1.25ms)、从机延迟(事件数)、
   * 监督超时(单位10ms)及连接事件长度(min/max，单位0.625ms) */
  bt_status_t (*conn_parameter_update)(const RawAddress& bd_addr,
                                       int min_interval, int max_interval,
                                       int latency, int timeout,
                                       uint16_t min_ce_len,
                                       uint16_t max_ce_len);

  /**
   * Set the preferred PHY (LE transmitter/receiver PHY) for the given
   * remote device.
   *
   * @param bd_addr remote device address
   * @param tx_phy preferred transmitter PHY (e.g. 1M, 2M, coded)
   * @param rx_phy preferred receiver PHY (e.g. 1M, 2M, coded)
   * @param phy_options preferred PHY option, e.g. LE coding of 2M/S=2
   */
  /* 设置指定设备的首选PHY(LE发射/接收PHY及编码选项) */
  bt_status_t (*set_preferred_phy)(const RawAddress& bd_addr, uint8_t tx_phy,
                                   uint8_t rx_phy, uint16_t phy_options); /* 设置指定设备的首选 PHY */

  bt_status_t (*read_phy)(
      const RawAddress& bd_addr,
      std::function<void(uint8_t tx_phy, uint8_t rx_phy, uint8_t status)> cb); /* 读取当前 PHY，结果经回调返回 */

  /** Test mode interface */ /* 测试模式接口 */
  bt_status_t (*test_command)(int command, const btgatt_test_params_t& params);

  /** Get gatt db content */ /* 获取 GATT 数据库内容 */
  bt_status_t (*get_gatt_db)(int conn_id);

  bt_status_t (*register_conn_state_cb)(); /* 注册连接状态变化回调 */

  bt_status_t (*deregister_conn_state_cb)(); /* 注销连接状态变化回调 */
} btgatt_client_interface_t;

} //__END_DECLS

#endif /* VENDOR_INCLUDE_BT_GATT_CLIENT_H */
