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
 * 文件说明：蓝牙 GATT 服务端（BT-GATT Server）协议栈接口定义。
 * 定义 GATT 服务端注册/注销、连接/断开、服务添加/停止/删除、
 * 读/写请求响应、通知/指示发送、PHY 设置与读取等接口函数指针，
 * 以及对应的值/响应参数结构与回调结构体。
 */

/* 头文件保护宏，防止头文件被重复包含 */
#ifndef VENDOR_INCLUDE_BT_GATT_SERVER_H
#define VENDOR_INCLUDE_BT_GATT_SERVER_H

/* C标准库头文件：定长整型类型 */
#include <stdint.h>
/* C++标准库头文件：动态数组容器 */
#include <vector>

/* 本地头文件：GATT相关类型及公共数据结构定义 */
#include "bt_gatt_types.h"

/* 以C链接方式声明后续接口，保证C/C++混合编译时符号名一致 */
__BEGIN_DECLS


/**
 * Buffer sizes for maximum attribute length and maximum read/write
 * operation buffer size.
 */
#define BTGATT_MAX_ATTR_LEN 600 /* 属性值最大长度（读/写操作缓冲区大小） */

/** GATT value type used in response to remote read requests */ /* 响应远端读请求的 GATT 值类型 */
typedef struct {
  uint8_t value[BTGATT_MAX_ATTR_LEN]; /* 属性数据 */
  uint16_t handle; /* 属性句柄 */
  uint16_t offset; /* 数据偏移 */
  uint16_t len; /* 数据长度 */
  uint8_t auth_req; /* 鉴权请求标志 */
} btgatt_value_t;

/** GATT remote read request response type */ /* 远端读请求响应类型（联合体） */
typedef union {
  btgatt_value_t attr_value; /* 属性值（正常响应） */
  uint16_t handle; /* 属性句柄（错误响应时使用） */
} btgatt_response_t;

/** BT-GATT Server callback structure. */ /* BT-GATT 服务端回调结构体 */

/** Callback invoked in response to register_server */ /* 注册 GATT 服务端的结果回调 */
typedef void (*register_server_callback)(int status, int server_if,
                                         const bluetooth::Uuid& app_uuid);

/** Callback indicating that a remote device has connected or been disconnected
 */ /* 远端设备连接/断开状态变化回调 */
typedef void (*connection_callback)(int conn_id, int server_if, int connected,
                                         const RawAddress& bda, int reason);

/** Callback invoked in response to create_service */ /* 添加服务的结果回调 */
typedef void (*service_added_callback)(
    int status, int server_if, std::vector<btgatt_db_element_t> service);

/** Callback invoked in response to stop_service */ /* 停止服务的结果回调 */
typedef void (*service_stopped_callback)(int status, int server_if,
                                         int srvc_handle);

/** Callback triggered when a service has been deleted */ /* 服务删除结果回调 */
typedef void (*service_deleted_callback)(int status, int server_if,
                                         int srvc_handle);

/**
 * Callback invoked when a remote device has requested to read a
 * characteristic or descriptor. The application must respond by
 * calling send_response.
 *
 * @param conn_id connection identifier
 * @param trans_id transaction id, used when calling send_response
 * @param bda remote device address
 * @param attr_handle handle of the characteristic/descriptor to read
 * @param offset read offset within the attribute value
 * @param is_long true for a long (partial) read with a non-zero offset
 */
/* 远端设备读特征值/描述符请求回调，应用须调用 send_response 响应 */
typedef void (*request_read_callback)(int conn_id, int trans_id,
                                      const RawAddress& bda, int attr_handle,
                                      int offset, bool is_long);

/**
 * Callback invoked when a remote device has requested to write to a
 * characteristic or descriptor. The application must respond by
 * calling send_response if need_rsp is true.
 *
 * @param conn_id connection identifier
 * @param trans_id transaction id, used when calling send_response
 * @param bda remote device address
 * @param attr_handle handle of the characteristic/descriptor to write
 * @param offset write offset within the attribute value
 * @param need_rsp true if a response is required for this write
 * @param is_prep true for a prepared write (execute via
 *                request_exec_write_callback)
 * @param value data to be written
 */
/* 远端设备写特征值/描述符请求回调：need_rsp为true时须应答，is_prep为true时
 * 为预备写(需经执行预备写回调确认) */
typedef void (*request_write_callback)(int conn_id, int trans_id,
                                       const RawAddress& bda, int attr_handle,
                                       int offset, bool need_rsp, bool is_prep,
                                       std::vector<uint8_t> value);

/**
 * Callback invoked when a previously prepared write is to be executed
 * or canceled.
 *
 * @param conn_id connection identifier
 * @param trans_id transaction id, used when calling send_response
 * @param bda remote device address
 * @param exec_write true to execute the queued prepared writes,
 *                   false to cancel them
 */
/* 执行/取消预备写请求回调：exec_write为true时执行已排队的预备写，false时取消 */
typedef void (*request_exec_write_callback)(int conn_id, int trans_id,
                                            const RawAddress& bda,
                                            int exec_write);

/**
 * Callback triggered in response to send_response if the remote device
 * sends a confirmation (e.g. confirmation of an indication).
 *
 * @param status status of the response/confirmation
 * @param handle handle of the attribute the response was sent for
 */
/* send_response 的响应确认回调（如指示的确认） */
typedef void (*response_confirmation_callback)(int status, int handle);

/**
 * Callback confirming that a notification or indication has been sent
 * to a remote device.
 *
 * @param conn_id connection identifier
 * @param status status of the notification/indication transmission
 */
/* 通知/指示已发送至远端设备的确认回调 */
typedef void (*indication_sent_callback)(int conn_id, int status);

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

/** Callback invoked when the MTU for a given connection changes */ /* 连接 MTU 变更回调 */
typedef void (*mtu_changed_callback)(int conn_id, int mtu);

/** Callback invoked when the PHY for a given connection changes */ /* 连接 PHY 更新回调 */
typedef void (*phy_updated_callback)(int conn_id, uint8_t tx_phy,
                                     uint8_t rx_phy, uint8_t status);

/** Callback invoked when the connection parameters for a given connection
 * changes */ /* 连接参数更新回调 */
typedef void (*conn_updated_callback)(int conn_id, uint16_t interval,
                                      uint16_t latency, uint16_t timeout,
                                       uint8_t status);
/* GATT 服务端回调集合结构体 */
typedef struct {
  register_server_callback register_server_cb; /* 注册服务端结果回调 */
  connection_callback connection_cb; /* 连接/断开状态回调 */
  service_added_callback service_added_cb; /* 服务添加回调 */
  service_stopped_callback service_stopped_cb; /* 服务停止回调 */
  service_deleted_callback service_deleted_cb; /* 服务删除回调 */
  request_read_callback request_read_characteristic_cb; /* 读特征值请求回调 */
  request_read_callback request_read_descriptor_cb; /* 读描述符请求回调 */
  request_write_callback request_write_characteristic_cb; /* 写特征值请求回调 */
  request_write_callback request_write_descriptor_cb; /* 写描述符请求回调 */
  request_exec_write_callback request_exec_write_cb; /* 执行预备写请求回调 */
  response_confirmation_callback response_confirmation_cb; /* 响应确认回调 */
  indication_sent_callback indication_sent_cb; /* 通知/指示发送完成回调 */
  congestion_callback congestion_cb; /* 拥塞状态回调 */
  mtu_changed_callback mtu_changed_cb; /* MTU 变更回调 */
  phy_updated_callback phy_updated_cb; /* PHY 更新回调 */
  conn_updated_callback conn_updated_cb; /* 连接参数更新回调 */
} btgatt_server_callbacks_t;

/** Represents the standard BT-GATT server interface. */ /* 标准 GATT 服务端接口结构体 */
typedef struct {
  /** Registers a GATT server application with the stack */ /* 向协议栈注册 GATT 服务端应用 */
  bt_status_t (*register_server)(const bluetooth::Uuid& uuid, bool eatt_support);

  /** Unregister a server application from the stack */ /* 从协议栈注销服务端应用 */
  bt_status_t (*unregister_server)(int server_if);

  /** Create a connection to a remote peripheral */ /* 连接远端外围设备 */
  bt_status_t (*connect)(int server_if, const RawAddress& bd_addr,
                         bool is_direct, int transport);

  /** Disconnect an established connection or cancel a pending one */ /* 断开已建立的连接或取消待定连接 */
  bt_status_t (*disconnect)(int server_if, const RawAddress& bd_addr,
                            int conn_id);

  /** Create a new service */ /* 创建新服务 */
  bt_status_t (*add_service)(int server_if,
                             std::vector<btgatt_db_element_t> service);

  /** Stops a local service */ /* 停止本地服务 */
  bt_status_t (*stop_service)(int server_if, int service_handle);

  /** Delete a local service */ /* 删除本地服务 */
  bt_status_t (*delete_service)(int server_if, int service_handle);

  /** Send value indication to a remote device */ /* 向远端设备发送指示（indication） */
  bt_status_t (*send_indication)(int server_if, int attribute_handle,
                                 int conn_id, int confirm,
                                 std::vector<uint8_t> value);

  /** Send a response to a read/write operation */ /* 对读/写操作发送响应 */
  bt_status_t (*send_response)(int conn_id, int trans_id, int status,
                               const btgatt_response_t& response);

  bt_status_t (*set_preferred_phy)(const RawAddress& bd_addr, uint8_t tx_phy,
                                   uint8_t rx_phy, uint16_t phy_options); /* 设置指定设备的首选 PHY */

  bt_status_t (*read_phy)(
      const RawAddress& bd_addr,
      std::function<void(uint8_t tx_phy, uint8_t rx_phy, uint8_t status)> cb); /* 读取当前 PHY，结果经回调返回 */

} btgatt_server_interface_t;

__END_DECLS

#endif /* VENDOR_INCLUDE_BT_GATT_CLIENT_H */
