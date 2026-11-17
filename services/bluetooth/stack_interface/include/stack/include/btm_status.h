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
 * 本文件定义蓝牙协议栈管理（BTM）层的应用返回状态码枚举 tBTM_STATUS，
 * 并提供状态码与数值、文本之间的转换辅助函数，
 * 供协议栈各模块上报命令执行结果使用。
 */
#pragma once

#include <cstdint>

#include "bt_macros.h"

/* BTM application return status codes */ /* BTM 应用层返回状态码 */
enum tBTM_STATUS : uint8_t {
  BTM_SUCCESS = 0,                   /* 0  Command succeeded                 */ /* 命令执行成功 */
  BTM_CMD_STARTED,                   /* 1  Command started OK.               */ /* 命令已成功启动 */
  BTM_BUSY,                          /* 2  Device busy with another command  */ /* 设备正忙于处理其他命令 */
  BTM_NO_RESOURCES,                  /* 3  No resources to issue command     */ /* 无可用资源执行命令 */
  BTM_MODE_UNSUPPORTED,              /* 4  Request for 1 or more unsupported modes */ /* 请求了一个或多个不支持的模式 */
  BTM_ILLEGAL_VALUE,                 /* 5  Illegal parameter value           */ /* 非法参数值 */
  BTM_WRONG_MODE,                    /* 6  Device in wrong mode for request  */ /* 设备当前模式与请求不匹配 */
  BTM_UNKNOWN_ADDR,                  /* 7  Unknown remote BD address         */ /* 未知的远端蓝牙设备地址 */
  BTM_DEVICE_TIMEOUT,                /* 8  Device timeout                    */ /* 设备响应超时 */
  BTM_BAD_VALUE_RET,                 /* 9  A bad value was received from HCI */ /* 从 HCI 层收到了错误值 */
  BTM_ERR_PROCESSING,                /* 10 Generic error                     */ /* 通用处理错误 */
  BTM_NOT_AUTHORIZED,                /* 11 Authorization failed              */ /* 授权（鉴权）失败 */
  BTM_DEV_RESET,                     /* 12 Device has been reset             */ /* 设备已被复位 */
  BTM_CMD_STORED,                    /* 13 request is stored in control block */ /* 请求已存入控制块暂缓处理 */
  BTM_ILLEGAL_ACTION,                /* 14 state machine gets illegal command */ /* 状态机收到了非法命令 */
  BTM_DELAY_CHECK,                   /* 15 delay the check on encryption */ /* 延迟执行加密检查 */
  BTM_SCO_BAD_LENGTH,                /* 16 Bad SCO over HCI data length */ /* HCI 上 SCO 数据长度错误 */
  BTM_SUCCESS_NO_SECURITY,           /* 17 security passed, no security set  */ /* 成功且无需设置安全（链路已满足安全要求） */
  BTM_FAILED_ON_SECURITY,            /* 18 security failed                   */ /* 安全（鉴权/加密）检查失败 */
  BTM_REPEATED_ATTEMPTS,             /* 19 repeated attempts for LE security requests */ /* LE 安全请求重复尝试过于频繁 */
  BTM_MODE4_LEVEL4_NOT_SUPPORTED,    /* 20 Secure Connections Only Mode can't be
                                         supported */ /* 不支持"仅安全连接（Secure Connections Only）"模式 */
  BTM_DEV_RESTRICT_LISTED,           /* 21 The device is restrict listed */ /* 设备在受限名单（黑名单）中 */
  BTM_ERR_KEY_MISSING,               /* 22 Handle for Pin or Key Missing */ /* PIN 码或链路密钥缺失（对应 HCI Key Missing） */
  BTM_NOT_AUTHENTICATED,             /* 23 the link is not authenticated */ /* 链路未经过鉴权 */
  BTM_NOT_ENCRYPTED,                 /* 24 the link is not encrypted */ /* 链路未加密 */
  BTM_INSUFFICIENT_ENCRYPT_KEY_SIZE, /* 25 the encrypt key size not sufficient
                                      */ /* 加密密钥长度不足 */
  BTM_MAX_STATUS_VALUE, /* 有效状态码数量上限（越界判断用） */
  BTM_UNDEFINED = 0xFF, /* 未定义/无效状态值 */
};

/* 将 tBTM_STATUS 枚举转换为对应的 uint8_t 数值 */
inline uint8_t btm_status_value(const tBTM_STATUS& status) { return static_cast<uint8_t>(status); }

/* 将 uint8_t 数值安全转换为 tBTM_STATUS 枚举，越界时返回 BTM_UNDEFINED */
inline tBTM_STATUS to_btm_status(const uint8_t& value) {
  if (value >= static_cast<unsigned>(tBTM_STATUS::BTM_MAX_STATUS_VALUE)) {
    return tBTM_STATUS::BTM_UNDEFINED;
  }
  return static_cast<tBTM_STATUS>(value);
}

/* switch-case 转文本辅助宏：case 命中时返回状态码名字符串 */
#define CASE_RETURN_TEXT(code) \
  case code:                   \
    return #code

/* 将 tBTM_STATUS 枚举转换为可读的状态码名字符串，未知值返回 "UNKNOWN[...]" */
inline std::string btm_status_text(const tBTM_STATUS& status) {
  /* 逐一匹配各已定义状态码，命中即返回其枚举名字符串 */
  switch (status) {
    /* 命令执行成功 */
    CASE_RETURN_TEXT(BTM_SUCCESS);
    /* 命令已成功启动 */
    CASE_RETURN_TEXT(BTM_CMD_STARTED);
    /* 设备正忙于处理其他命令 */
    CASE_RETURN_TEXT(BTM_BUSY);
    /* 无可用资源执行命令 */
    CASE_RETURN_TEXT(BTM_NO_RESOURCES);
    /* 请求了一个或多个不支持的模式 */
    CASE_RETURN_TEXT(BTM_MODE_UNSUPPORTED);
    /* 非法参数值 */
    CASE_RETURN_TEXT(BTM_ILLEGAL_VALUE);
    /* 设备当前模式与请求不匹配 */
    CASE_RETURN_TEXT(BTM_WRONG_MODE);
    /* 未知的远端蓝牙设备地址 */
    CASE_RETURN_TEXT(BTM_UNKNOWN_ADDR);
    /* 设备响应超时 */
    CASE_RETURN_TEXT(BTM_DEVICE_TIMEOUT);
    /* 从 HCI 层收到了错误值 */
    CASE_RETURN_TEXT(BTM_BAD_VALUE_RET);
    /* 通用处理错误 */
    CASE_RETURN_TEXT(BTM_ERR_PROCESSING);
    /* 授权（鉴权）失败 */
    CASE_RETURN_TEXT(BTM_NOT_AUTHORIZED);
    /* 设备已被复位 */
    CASE_RETURN_TEXT(BTM_DEV_RESET);
    /* 请求已存入控制块暂缓处理 */
    CASE_RETURN_TEXT(BTM_CMD_STORED);
    /* 状态机收到了非法命令 */
    CASE_RETURN_TEXT(BTM_ILLEGAL_ACTION);
    /* 延迟执行加密检查 */
    CASE_RETURN_TEXT(BTM_DELAY_CHECK);
    /* HCI 上 SCO 数据长度错误 */
    CASE_RETURN_TEXT(BTM_SCO_BAD_LENGTH);
    /* 成功且无需设置安全（链路已满足安全要求） */
    CASE_RETURN_TEXT(BTM_SUCCESS_NO_SECURITY);
    /* 安全（鉴权/加密）检查失败 */
    CASE_RETURN_TEXT(BTM_FAILED_ON_SECURITY);
    /* LE 安全请求重复尝试过于频繁 */
    CASE_RETURN_TEXT(BTM_REPEATED_ATTEMPTS);
    /* 不支持"仅安全连接（Secure Connections Only）"模式 */
    CASE_RETURN_TEXT(BTM_MODE4_LEVEL4_NOT_SUPPORTED);
    /* 设备在受限名单（黑名单）中 */
    CASE_RETURN_TEXT(BTM_DEV_RESTRICT_LISTED);
    /* 未匹配到任何已知状态码时返回"UNKNOWN[原始值]" */
    default:
      return std::string("UNKNOWN[%hhu]", status);
  }
}

/* 取消 CASE_RETURN_TEXT 宏定义，避免污染包含本文件的其他编译单元 */
#undef CASE_RETURN_TEXT

