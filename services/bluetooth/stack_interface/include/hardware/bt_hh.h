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
 * 文件说明：蓝牙 HID Host（BT-HH）协议栈接口定义。
 * 定义 HID 主机与 HID 设备（键盘/鼠标/手柄等）交互所需的连接状态、
 * 状态码、协议模式、报告类型、HID 设备信息等数据结构，以及
 * 连接/断开、协议模式与报告读写、空闲时间、虚拟拔插等接口
 * 函数指针与回调结构体。
 */

#ifndef VENDOR_INCLUDE_BT_HH_H
#define VENDOR_INCLUDE_BT_HH_H

#include <stdint.h>
#include <string>

extern "C" { //__BEGIN_DECLS

#define BTHH_MAX_DSC_LEN 884 /* HID 描述符列表最大长度 */

/* HH connection states */ /* HID Host 连接状态枚举 */
typedef enum {
  BTHH_CONN_STATE_CONNECTED = 0, /* 已连接 */
  BTHH_CONN_STATE_CONNECTING = 1, /* 连接中 */
  BTHH_CONN_STATE_DISCONNECTED = 2, /* 已断开 */
  BTHH_CONN_STATE_DISCONNECTING = 3, /* 断开中 */
  BTHH_CONN_STATE_FAILED_MOUSE_FROM_HOST, /* 主机侧鼠标连接失败 */
  BTHH_CONN_STATE_FAILED_KBD_FROM_HOST, /* 主机侧键盘连接失败 */
  BTHH_CONN_STATE_FAILED_TOO_MANY_DEVICES, /* 连接设备数超限失败 */
  BTHH_CONN_STATE_FAILED_NO_BTHID_DRIVER, /* 缺少 BTHID 驱动导致失败 */
  BTHH_CONN_STATE_FAILED_GENERIC, /* 一般性连接失败 */
  BTHH_CONN_STATE_UNKNOWN = 0xff, /* 未知状态 */
} bthh_connection_state_t;

} //__END_DECLS
#define CASE_RETURN_TEXT(code) \
  case code:                   \
    return #code

/* 将 HID Host 连接状态枚举值转换为可读字符串 */
inline std::string bthh_connection_state_text(
    const bthh_connection_state_t& state) {
  switch (state) {
    CASE_RETURN_TEXT(BTHH_CONN_STATE_CONNECTED);
    CASE_RETURN_TEXT(BTHH_CONN_STATE_CONNECTING);
    CASE_RETURN_TEXT(BTHH_CONN_STATE_DISCONNECTED);
    CASE_RETURN_TEXT(BTHH_CONN_STATE_DISCONNECTING);
    CASE_RETURN_TEXT(BTHH_CONN_STATE_UNKNOWN);
    default:
      return std::string("UNKNOWN[%u]", state);
  }
}
#undef CASE_RETURN_TEXT
extern "C" { //__BEGIN_DECLS

/* HID Host 操作状态码枚举 */
typedef enum {
  BTHH_OK = 0, /* 成功 */
  BTHH_HS_HID_NOT_READY,  /* handshake error : device not ready */ /* 设备未就绪 */
  BTHH_HS_INVALID_RPT_ID, /* handshake error : invalid report ID */ /* 无效报告 ID */
  BTHH_HS_TRANS_NOT_SPT,  /* handshake error : transaction not spt */ /* 事务不支持 */
  BTHH_HS_INVALID_PARAM,  /* handshake error : invalid paremter */ /* 无效参数 */
  BTHH_HS_ERROR,          /* handshake error : unspecified HS error */ /* 未指定的握手错误 */
  BTHH_ERR,               /* general BTA HH error */ /* 一般 BTA HH 错误 */
  BTHH_ERR_SDP,           /* SDP error */ /* SDP 错误 */
  BTHH_ERR_PROTO,         /* SET_Protocol error,
                                                        only used in BTA_HH_OPEN_EVT
                             callback */ /* SET_Protocol 错误，仅用于 BTA_HH_OPEN_EVT 回调 */
  BTHH_ERR_DB_FULL,       /* device database full error, used  */ /* 设备数据库已满 */
  BTHH_ERR_TOD_UNSPT,     /* type of device not supported */ /* 设备类型不支持 */
  BTHH_ERR_NO_RES,        /* out of system resources */ /* 系统资源不足 */
  BTHH_ERR_AUTH_FAILED,   /* authentication fail */ /* 鉴权失败 */
  BTHH_ERR_HDL /* 句柄错误 */
} bthh_status_t;

/* Protocol modes */ /* 协议模式枚举 */
typedef enum {
  BTHH_REPORT_MODE = 0x00, /* 报告模式 */
  BTHH_BOOT_MODE = 0x01, /* 引导模式 */
  BTHH_UNSUPPORTED_MODE = 0xff /* 不支持的模式 */
} bthh_protocol_mode_t;

/* Report types */ /* 报告类型枚举 */
typedef enum {
  BTHH_INPUT_REPORT = 1, /* 输入报告 */
  BTHH_OUTPUT_REPORT, /* 输出报告 */
  BTHH_FEATURE_REPORT /* 特征报告 */
} bthh_report_type_t;

/* HID 设备信息结构体 */
typedef struct {
  int attr_mask; /* 属性掩码 */
  uint8_t sub_class; /* HID 子类 */
  uint8_t app_id; /* 应用 ID */
  int vendor_id; /* 厂商 ID */
  int product_id; /* 产品 ID */
  int version; /* 版本号 */
  uint8_t ctry_code; /* 国家代码 */
  int dl_len; /* 描述符列表长度 */
  uint8_t dsc_list[BTHH_MAX_DSC_LEN]; /* 描述符列表 */
} bthh_hid_info_t;

/** Callback for connection state change.
 *  state will have one of the values from bthh_connection_state_t
 */ /* HID 设备连接状态变化回调 */
typedef void (*bthh_connection_state_callback)(RawAddress* bd_addr,
                                               bthh_connection_state_t state);

/** Callback for vitual unplug api.
 *  the status of the vitual unplug
 */ /* 虚拟拔插（VUP）结果回调 */
typedef void (*bthh_virtual_unplug_callback)(RawAddress* bd_addr,
                                             bthh_status_t hh_status);

/** Callback for get hid info
 *  hid_info will contain attr_mask, sub_class, app_id, vendor_id, product_id,
 *  version, ctry_code, len
 */ /* 获取 HID 设备信息回调 */
typedef void (*bthh_hid_info_callback)(RawAddress* bd_addr,
                                       bthh_hid_info_t hid_info);

/** Callback for get protocol api.
 *  the protocol mode is one of the value from bthh_protocol_mode_t
 */ /* 协议模式查询结果回调 */
typedef void (*bthh_protocol_mode_callback)(RawAddress* bd_addr,
                                            bthh_status_t hh_status,
                                            bthh_protocol_mode_t mode);

/** Callback for get/set_idle_time api.
 */ /* 空闲时间获取/设置结果回调 */
typedef void (*bthh_idle_time_callback)(RawAddress* bd_addr,
                                        bthh_status_t hh_status, int idle_rate);

/** Callback for get report api.
 *  if staus is ok rpt_data contains the report data
 */ /* 获取报告结果回调 */
typedef void (*bthh_get_report_callback)(RawAddress* bd_addr,
                                         bthh_status_t hh_status,
                                         uint8_t* rpt_data, int rpt_size);

/** Callback for set_report/set_protocol api and if error
 *  occurs for get_report/get_protocol api.
 */ /* set_report/set_protocol 结果及错误时的握手回调 */
typedef void (*bthh_handshake_callback)(RawAddress* bd_addr,
                                        bthh_status_t hh_status);

/** BT-HH callback structure. */ /* BT-HH 回调结构体 */
typedef struct {
  /** set to sizeof(BtHfCallbacks) */ /* 结构体大小，设置为 sizeof(BtHfCallbacks) */
  size_t size;
  bthh_connection_state_callback connection_state_cb; /* 连接状态变化回调 */
  bthh_hid_info_callback hid_info_cb; /* HID 设备信息回调 */
  bthh_protocol_mode_callback protocol_mode_cb; /* 协议模式结果回调 */
  bthh_idle_time_callback idle_time_cb; /* 空闲时间结果回调 */
  bthh_get_report_callback get_report_cb; /* 获取报告结果回调 */
  bthh_virtual_unplug_callback virtual_unplug_cb; /* 虚拟拔插结果回调 */
  bthh_handshake_callback handshake_cb; /* 握手结果回调 */

} bthh_callbacks_t;

/** Represents the standard BT-HH interface. */ /* 标准 BT-HH（HID Host）接口结构体 */
typedef struct {
  /** set to sizeof(BtHhInterface) */ /* 结构体大小，设置为 sizeof(BtHhInterface) */
  size_t size;

  /**
   * Register the BtHh callbacks
   */ /* 初始化接口并注册 BT-HH 回调 */
  bt_status_t (*init)(bthh_callbacks_t* callbacks);

  /** connect to hid device */ /* 连接 HID 设备 */
  bt_status_t (*connect)(RawAddress* bd_addr);

  /** dis-connect from hid device */ /* 断开 HID 设备 */
  bt_status_t (*disconnect)(RawAddress* bd_addr);

  /** Virtual UnPlug (VUP) the specified HID device */ /* 虚拟拔插（VUP）指定 HID 设备 */
  bt_status_t (*virtual_unplug)(RawAddress* bd_addr);

  /** Set the HID device descriptor for the specified HID device. */ /* 设置指定 HID 设备的描述符信息 */
  bt_status_t (*set_info)(RawAddress* bd_addr, bthh_hid_info_t hid_info);

  /** Get the HID proto mode. */ /* 获取 HID 协议模式 */
  bt_status_t (*get_protocol)(RawAddress* bd_addr,
                              bthh_protocol_mode_t protocolMode);

  /** Set the HID proto mode. */ /* 设置 HID 协议模式 */
  bt_status_t (*set_protocol)(RawAddress* bd_addr,
                              bthh_protocol_mode_t protocolMode);

  /** Get the HID Idle Time */ /* 获取 HID 空闲时间 */
  bt_status_t (*get_idle_time)(RawAddress* bd_addr);

  /** Set the HID Idle Time */ /* 设置 HID 空闲时间 */
  bt_status_t (*set_idle_time)(RawAddress* bd_addr, uint8_t idleTime);

  /** Send a GET_REPORT to HID device. */ /* 向 HID 设备发送 GET_REPORT 请求 */
  bt_status_t (*get_report)(RawAddress* bd_addr, bthh_report_type_t reportType,
                            uint8_t reportId, int bufferSize);

  /** Send a SET_REPORT to HID device. */ /* 向 HID 设备发送 SET_REPORT 请求 */
  bt_status_t (*set_report)(RawAddress* bd_addr, bthh_report_type_t reportType,
                            char* report);

  /** Send data to HID device. */ /* 向 HID 设备发送数据 */
  bt_status_t (*send_data)(RawAddress* bd_addr, char* data);

  /** Closes the interface. */ /* 关闭接口并释放资源 */
  void (*cleanup)(void);

} bthh_interface_t;
} //__END_DECLS

#endif /* VENDOR_INCLUDE_BT_HH_H */
