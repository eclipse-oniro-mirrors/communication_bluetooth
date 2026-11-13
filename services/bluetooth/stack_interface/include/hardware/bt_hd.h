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
 * 本文件定义蓝牙 HID Device（BTHD）Profile 的对外 C 接口，
 * 包含 HID 报告类型、应用注册/连接状态、应用注册参数、QoS 参数、
 * 各类事件回调结构体以及 HID 设备接口函数表，
 * 供蓝牙协议栈上层实现 HID 设备侧（如键盘、鼠标等）功能使用。
 */
#ifndef VENDOR_INCLUDE_BT_HD_H
#define VENDOR_INCLUDE_BT_HD_H

#include <sys/cdefs.h>
#include <stdint.h>

#include "types/raw_address.h"

__BEGIN_DECLS

/* HID 报告类型 */
typedef enum {
  BTHD_REPORT_TYPE_OTHER = 0, /* 其他类型报告 */
  BTHD_REPORT_TYPE_INPUT, /* 输入报告（设备到主机方向） */
  BTHD_REPORT_TYPE_OUTPUT, /* 输出报告（主机到设备方向） */
  BTHD_REPORT_TYPE_FEATURE, /* 特性报告（配置设备属性） */
  // special value for reports to be sent on INTR(INPUT is assumed)
  BTHD_REPORT_TYPE_INTRDATA /* 中断通道数据的特殊值（默认按输入报告处理） */
} bthd_report_type_t;

/* HID 设备应用注册状态 */
typedef enum {
  BTHD_APP_STATE_NOT_REGISTERED, /* 应用未注册 */
  BTHD_APP_STATE_REGISTERED /* 应用已注册 */
} bthd_application_state_t;

/* HID 设备与主机的虚拟电缆连接状态 */
typedef enum {
  BTHD_CONN_STATE_CONNECTED, /* 已连接 */
  BTHD_CONN_STATE_CONNECTING, /* 连接中 */
  BTHD_CONN_STATE_DISCONNECTED, /* 已断开 */
  BTHD_CONN_STATE_DISCONNECTING, /* 断开中 */
  BTHD_CONN_STATE_UNKNOWN /* 未知状态 */
} bthd_connection_state_t;

/* HID 设备应用注册参数（对应 SDP 中的 HID Device Record 描述信息） */
typedef struct {
  const char* name; /* 应用名称 */
  const char* description; /* 应用描述 */
  const char* provider; /* 提供者名称 */
  uint8_t subclass; /* 设备子类码，标识具体 HID 设备类型 */
  uint8_t* desc_list; /* HID 描述符列表（Report Descriptor 等二进制数据） */
  int desc_list_len; /* 描述符列表长度 */
} bthd_app_param_t;

/* HID 设备 QoS（服务质量）参数 */
typedef struct {
  uint8_t service_type; /* 服务类型（尽力而为/保证带宽等） */
  uint32_t token_rate; /* 令牌速率，即数据传输速率（字节/秒） */
  uint32_t token_bucket_size; /* 令牌桶大小，允许的突发数据量 */
  uint32_t peak_bandwidth; /* 峰值带宽 */
  uint32_t access_latency; /* 访问最大时延 */
  uint32_t delay_variation; /* 时延抖动允许的最大变化量 */
} bthd_qos_param_t;

/* HID 应用注册状态变化回调（bd_addr 为对端主机地址，state 为新状态） */
typedef void (*bthd_application_state_callback)(RawAddress* bd_addr,
                                                bthd_application_state_t state);
/* HID 设备连接状态变化回调（bd_addr 为对端主机地址，state 为新状态） */
typedef void (*bthd_connection_state_callback)(RawAddress* bd_addr, bthd_connection_state_t state);
/* 主机请求获取报告回调（type 为报告类型，id 为报告 ID，buffer_size 为缓冲区大小） */
typedef void (*bthd_get_report_callback)(uint8_t type, uint8_t id, uint16_t buffer_size);
/* 主机请求设置报告回调（p_data 指向报告数据，len 为数据长度） */
typedef void (*bthd_set_report_callback)(uint8_t type, uint8_t id, uint16_t len, uint8_t* p_data);
/* 主机请求设置协议模式回调（protocol 为 BOOT/REPORT 协议模式） */
typedef void (*bthd_set_protocol_callback)(uint8_t protocol);
/* 主机在中断通道发送数据回调（report_id 为报告 ID，p_data 为数据，len 为长度） */
typedef void (*bthd_intr_data_callback)(uint8_t report_id, uint16_t len, uint8_t* p_data);
/* 主机请求虚拟电缆拔出回调 */
typedef void (*bthd_vc_unplug_callback)(void);

/** BT-HD callbacks */ /* BT-HD 事件回调结构体 */
typedef struct {
  size_t size; /* 结构体大小，用于版本兼容性检查 */

  bthd_application_state_callback application_state_cb; /* 应用注册状态变化回调 */
  bthd_connection_state_callback connection_state_cb; /* 连接状态变化回调 */
  bthd_get_report_callback get_report_cb; /* 主机获取报告请求回调 */
  bthd_set_report_callback set_report_cb; /* 主机设置报告请求回调 */
  bthd_set_protocol_callback set_protocol_cb; /* 主机设置协议模式请求回调 */
  bthd_intr_data_callback intr_data_cb; /* 中断通道数据回调 */
  bthd_vc_unplug_callback vc_unplug_cb; /* 虚拟电缆拔出请求回调 */
} bthd_callbacks_t;

/** BT-HD interface */ /* BT-HD 接口函数表 */
typedef struct {
  size_t size; /* 结构体大小，用于版本兼容性检查 */

  /** init interface and register callbacks */
  /* 初始化接口并注册回调 */
  bt_status_t (*init)(bthd_callbacks_t* callbacks); /* 初始化 HID 设备接口并注册回调 */

  /** close interface */
  /* 关闭接口 */
  void (*cleanup)(void); /* 清理并关闭接口 */

  /** register application */
  /* 注册应用 */
  bt_status_t (*register_app)(bthd_app_param_t* app_param, bthd_qos_param_t* in_qos,
                              bthd_qos_param_t* out_qos); /* 注册 HID 应用，携带应用参数及输入/输出 QoS 参数 */

  /** unregister application */
  /* 注销应用 */
  bt_status_t (*unregister_app)(void); /* 注销已注册的 HID 应用 */

  /** connects to host with virtual cable */
  /* 通过虚拟电缆连接主机 */
  bt_status_t (*connect)(RawAddress* bd_addr); /* 以虚拟电缆方式连接到指定地址的主机 */

  /** disconnects from currently connected host */
  /* 断开与当前已连接主机的连接 */
  bt_status_t (*disconnect)(void); /* 断开当前连接 */

  /** send report */
  /* 发送报告：向主机发送指定类型的 HID 报告 */
  bt_status_t (*send_report)(bthd_report_type_t type, uint8_t id, uint16_t len, uint8_t* p_data);

  /** notifies error for invalid SET_REPORT */
  /* 对无效的 SET_REPORT 请求上报错误 */
  bt_status_t (*report_error)(uint8_t error); /* 上报 SET_REPORT 处理错误码 */

  /** send Virtual Cable Unplug  */
  /* 发送虚拟电缆拔出请求 */
  bt_status_t (*virtual_cable_unplug)(void); /* 发送虚拟电缆拔出（VC Unplug）请求 */
} bthd_interface_t;

__END_DECLS

#if __has_include(<bluetooth/log.h>)

namespace std {
template <>
struct formatter<bthd_report_type_t> : enum_formatter<bthd_report_type_t> {};
}  // namespace std

#endif  // __has_include(<bluetooth/log.h>)

#endif /* VENDOR_INCLUDE_BT_HD_H */
