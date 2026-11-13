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

/* 本文件定义蓝牙 HCI 错误码（tHCI_ERROR_CODE）及其与 tHCI_STATUS、tHCI_REASON 的等价别名，
 * 并提供错误码转文本、原始字节转错误码等辅助函数，用于协议栈各层事件/断连原因的统一描述。 */

#pragma once

#include <hardware/bluetooth.h>

#include <cstdint>
#include <string>

#include "bt_macros.h"

/*
 *  Definitions for HCI Error Codes that are passed in the events
 */
/* HCI 错误码定义（事件中传递的错误码，取值参考蓝牙 Core 规范） */
typedef enum : uint8_t {
  HCI_SUCCESS = 0x00,                         /* 成功 */
  HCI_ERR_ILLEGAL_COMMAND = 0x01,             /* 未知HCI命令 */
  HCI_ERR_NO_CONNECTION = 0x02,               /* 无对应连接 */
  HCI_ERR_HW_FAILURE = 0x03,                  /* 硬件故障 */
  HCI_ERR_PAGE_TIMEOUT = 0x04,                /* 寻呼超时 */
  HCI_ERR_AUTH_FAILURE = 0x05,                /* 鉴权/配对失败 */
  HCI_ERR_KEY_MISSING = 0x06,                 /* 密钥缺失 */
  HCI_ERR_MEMORY_FULL = 0x07,                 /* 内存不足 */
  HCI_ERR_CONNECTION_TOUT = 0x08,             /* 连接超时 */
  HCI_ERR_MAX_NUM_OF_CONNECTIONS = 0x09,      /* 连接数达到上限 */
  HCI_ERR_MAX_NUM_OF_SCOS = 0x0A,             /* SCO连接数达到上限 */
  HCI_ERR_CONNECTION_EXISTS = 0x0B,           /* 连接已存在 */
  HCI_ERR_COMMAND_DISALLOWED = 0x0C,          /* 命令不允许 */
  HCI_ERR_HOST_REJECT_RESOURCES = 0x0D,       /* 主机因资源不足拒绝连接 */
  HCI_ERR_HOST_REJECT_SECURITY = 0x0E,        /* 主机因安全原因拒绝连接 */
  HCI_ERR_HOST_REJECT_DEVICE = 0x0F,          /* 主机因不接受远端设备而拒绝连接 */
  HCI_ERR_HOST_TIMEOUT = 0x10,  // stack/btm/btm_ble_gap,  /* 主机超时 */
  HCI_ERR_ILLEGAL_PARAMETER_FMT = 0x12,       /* 非法命令参数 */
  HCI_ERR_PEER_USER = 0x13,                   /* 远端用户终止连接 */
  HCI_ERR_REMOTE_LOW_RESOURCE = 0x14,         /* 远端设备资源不足 */
  HCI_ERR_REMOTE_POWER_OFF = 0x15,            /* 远端设备关机 */
  HCI_ERR_CONN_CAUSE_LOCAL_HOST = 0x16,       /* 本地主机关闭连接 */
  HCI_ERR_REPEATED_ATTEMPTS = 0x17,           /* 配对尝试过于频繁 */
  HCI_ERR_PAIRING_NOT_ALLOWED = 0x18,         /* 不允许配对 */
  HCI_ERR_UNSUPPORTED_REM_FEATURE = 0x1A,  // stack/btm/btm_ble_gap  /* 远端不支持该特性 */
  HCI_ERR_UNSPECIFIED = 0x1F,                 /* 未指定的错误 */
  HCI_ERR_LMP_RESPONSE_TIMEOUT = 0x22,     // GATT_CONN_LMP_TIMEOUT  /* LMP响应超时 */
  HCI_ERR_LMP_ERR_TRANS_COLLISION = 0x23,  // TODO remove  /* LMP/LMP 过程冲突 */
  HCI_ERR_ENCRY_MODE_NOT_ACCEPTABLE = 0x25,   /* 不接受该加密模式 */
  HCI_ERR_UNIT_KEY_USED = 0x26,               /* 使用了单元密钥 */
  HCI_ERR_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED = 0x29,  /* 不支持与单元密钥配对 */
  HCI_ERR_DIFF_TRANSACTION_COLLISION = 0x2A,  // stack/btm/btm_sec  /* 不同过程冲突 */
  HCI_ERR_INSUFFCIENT_SECURITY = 0x2F,        // btif/btu  /* 安全等级不足 */
  HCI_ERR_ROLE_SWITCH_PENDING = 0x32,         // stack/btm/btm_sco  /* 角色切换挂起 */
  HCI_ERR_ROLE_SWITCH_FAILED = 0x35,          /* 角色切换失败 */
  HCI_ERR_HOST_BUSY_PAIRING = 0x38,           // stack/btm/btm_sec  /* 主机配对忙 */
  HCI_ERR_UNACCEPT_CONN_INTERVAL = 0x3B,      // stack/l2cap/l2c_ble  /* 不可接受的连接间隔 */
  HCI_ERR_ADVERTISING_TIMEOUT = 0x3C,         // stack/btm/btm_ble  /* 有界广播超时 */
  HCI_ERR_CONN_FAILED_ESTABLISHMENT = 0x3E,   // GATT_CONN_FAIL_ESTABLISH  /* 连接建立失败 */
  HCI_ERR_LIMIT_REACHED = 0x43,               // stack/btm/btm_ble_multi_adv.cc  /* 达到处理能力上限 */
  HCI_ERR_CANCELLED_BY_LOCAL_HOST = 0x44,     // stack/btm/btm_iso_impl.h  /* 被本地主机取消 */

  _HCI_ERR_MAX_ERR = 0x44,                    /* 错误码最大值 */
  HCI_ERR_UNDEFINED = 0xff,                   /* 未定义错误码 */
} tHCI_ERROR_CODE;

#define HCI_ERR_MAX_ERR _HCI_ERR_MAX_ERR  // HACK for now for SMP

inline std::string hci_error_code_text(const tHCI_ERROR_CODE& error_code) {
  switch (error_code) {
    CASE_RETURN_TEXT(HCI_SUCCESS);
    CASE_RETURN_TEXT(HCI_ERR_ILLEGAL_COMMAND);
    CASE_RETURN_TEXT(HCI_ERR_NO_CONNECTION);
    CASE_RETURN_TEXT(HCI_ERR_HW_FAILURE);
    CASE_RETURN_TEXT(HCI_ERR_PAGE_TIMEOUT);
    CASE_RETURN_TEXT(HCI_ERR_AUTH_FAILURE);
    CASE_RETURN_TEXT(HCI_ERR_KEY_MISSING);
    CASE_RETURN_TEXT(HCI_ERR_MEMORY_FULL);
    CASE_RETURN_TEXT(HCI_ERR_CONNECTION_TOUT);
    CASE_RETURN_TEXT(HCI_ERR_MAX_NUM_OF_CONNECTIONS);
    CASE_RETURN_TEXT(HCI_ERR_MAX_NUM_OF_SCOS);
    CASE_RETURN_TEXT(HCI_ERR_CONNECTION_EXISTS);
    CASE_RETURN_TEXT(HCI_ERR_COMMAND_DISALLOWED);
    CASE_RETURN_TEXT(HCI_ERR_HOST_REJECT_RESOURCES);
    CASE_RETURN_TEXT(HCI_ERR_HOST_REJECT_SECURITY);
    CASE_RETURN_TEXT(HCI_ERR_HOST_REJECT_DEVICE);
    CASE_RETURN_TEXT(HCI_ERR_HOST_TIMEOUT);
    CASE_RETURN_TEXT(HCI_ERR_ILLEGAL_PARAMETER_FMT);
    CASE_RETURN_TEXT(HCI_ERR_PEER_USER);
    CASE_RETURN_TEXT(HCI_ERR_REMOTE_LOW_RESOURCE);
    CASE_RETURN_TEXT(HCI_ERR_REMOTE_POWER_OFF);
    CASE_RETURN_TEXT(HCI_ERR_CONN_CAUSE_LOCAL_HOST);
    CASE_RETURN_TEXT(HCI_ERR_REPEATED_ATTEMPTS);
    CASE_RETURN_TEXT(HCI_ERR_PAIRING_NOT_ALLOWED);
    CASE_RETURN_TEXT(HCI_ERR_UNSUPPORTED_REM_FEATURE);
    CASE_RETURN_TEXT(HCI_ERR_UNSPECIFIED);
    CASE_RETURN_TEXT(HCI_ERR_LMP_RESPONSE_TIMEOUT);
    CASE_RETURN_TEXT(HCI_ERR_LMP_ERR_TRANS_COLLISION);
    CASE_RETURN_TEXT(HCI_ERR_ENCRY_MODE_NOT_ACCEPTABLE);
    CASE_RETURN_TEXT(HCI_ERR_UNIT_KEY_USED);
    CASE_RETURN_TEXT(HCI_ERR_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED);
    CASE_RETURN_TEXT(HCI_ERR_DIFF_TRANSACTION_COLLISION);
    CASE_RETURN_TEXT(HCI_ERR_INSUFFCIENT_SECURITY);
    CASE_RETURN_TEXT(HCI_ERR_ROLE_SWITCH_PENDING);
    CASE_RETURN_TEXT(HCI_ERR_ROLE_SWITCH_FAILED);
    CASE_RETURN_TEXT(HCI_ERR_HOST_BUSY_PAIRING);
    CASE_RETURN_TEXT(HCI_ERR_UNACCEPT_CONN_INTERVAL);
    CASE_RETURN_TEXT(HCI_ERR_ADVERTISING_TIMEOUT);
    CASE_RETURN_TEXT(HCI_ERR_CONN_FAILED_ESTABLISHMENT);
    CASE_RETURN_TEXT(HCI_ERR_LIMIT_REACHED);
    CASE_RETURN_TEXT(HCI_ERR_CANCELLED_BY_LOCAL_HOST);
    default:
      return "UNKNOWN[0x" + std::to_string(static_cast<uint8_t>(error_code)) + "]";
  }
}

// Context equivalence
/* 状态码类型：与 tHCI_ERROR_CODE 等价（上下文等价别名） */
using tHCI_STATUS = tHCI_ERROR_CODE; /* HCI 状态码类型别名 */
/* 将状态码转换为可读文本 */
inline std::string hci_status_code_text(const tHCI_STATUS& status_code) {
  return hci_error_code_text(status_code);
}

/* 原因码类型：与 tHCI_ERROR_CODE 等价（上下文等价别名） */
using tHCI_REASON = tHCI_ERROR_CODE; /* HCI 原因码类型别名 */
/* 将原因码转换为可读文本 */
inline std::string hci_reason_code_text(const tHCI_REASON& reason_code) {
  return hci_error_code_text(reason_code);
}

// Conversion from raw packet value
/* 将报文中的原始字节值转换为 HCI 错误码，超出最大值时返回未定义错误码 */
inline tHCI_ERROR_CODE to_hci_error_code(const uint8_t& error_code) {
  if (error_code > _HCI_ERR_MAX_ERR) {
    return HCI_ERR_UNDEFINED;
  }
  return static_cast<tHCI_ERROR_CODE>(error_code);
}

inline tHCI_STATUS to_hci_status_code(const uint8_t& status_code) {
  if (status_code > _HCI_ERR_MAX_ERR) {
    return HCI_ERR_UNDEFINED;
  }
  return static_cast<tHCI_STATUS>(status_code);
}

inline tHCI_REASON to_hci_reason_code(const uint8_t& reason_code) {
  if (reason_code > _HCI_ERR_MAX_ERR) {
    return HCI_ERR_UNDEFINED;
  }
  return static_cast<tHCI_REASON>(reason_code);
}

#if __has_include(<bluetooth/log.h>)
namespace std {
template <>
struct formatter<tHCI_ERROR_CODE> : enum_formatter<tHCI_ERROR_CODE> {};
}  // namespace std
#endif  // __has_include(<bluetooth/log.h>)
