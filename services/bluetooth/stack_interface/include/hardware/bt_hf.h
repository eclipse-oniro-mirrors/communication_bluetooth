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

namespace bluetooth {
namespace headset {

/* AT 响应码 —— 标识 AG 对 HF 发来的 AT 命令的处理结果（OK / ERROR） */
typedef enum {
  BTHF_AT_RESPONSE_ERROR = 0,  // 命令执行失败
  BTHF_AT_RESPONSE_OK         // 命令执行成功
} bthf_at_response_t;

/* HFP 连接状态 —— 描述 HF 与 AG 之间 RFCOMM/服务层连接的建立与断开过程 */
typedef enum {
  BTHF_CONNECTION_STATE_DISCONNECTED = 0,  // 已断开
  BTHF_CONNECTION_STATE_CONNECTING,        // 正在建立连接
  BTHF_CONNECTION_STATE_CONNECTED,         // RFCOMM 已连接（SLC 尚未建立）
  BTHF_CONNECTION_STATE_SLC_CONNECTED,     // 服务层连接（SLC）已建立，可正常交互
  BTHF_CONNECTION_STATE_DISCONNECTING      // 正在断开连接
} bthf_connection_state_t;

/* SCO 音频连接状态 —— 描述通话音频链路（SCO/eSCO）的状态 */
typedef enum {
  BTHF_AUDIO_STATE_DISCONNECTED = 0,  // 音频链路已断开
  BTHF_AUDIO_STATE_CONNECTING,        // 正在建立音频链路
  BTHF_AUDIO_STATE_CONNECTED,         // 音频链路已建立，可进行通话
  BTHF_AUDIO_STATE_DISCONNECTING      // 正在断开音频链路
} bthf_audio_state_t;

/* 语音识别状态 —— 对应 AT+BVRA 语音识别的开启/关闭 */
typedef enum {
  BTHF_VR_STATE_STOPPED = 0,  // 语音识别已停止
  BTHF_VR_STATE_STARTED       // 语音识别已启动
} bthf_vr_state_t;

/* 音量调节类型 —— 对应 AT+VGS（扬声器）/AT+VGM（麦克风）音量控制 */
typedef enum {
  BTHF_VOLUME_TYPE_SPK = 0,  // 扬声器音量
  BTHF_VOLUME_TYPE_MIC       // 麦克风音量
} bthf_volume_type_t;

/* Noise Reduction and Echo Cancellation */
typedef enum { BTHF_NREC_STOP, BTHF_NREC_START } bthf_nrec_t;

/* WBS codec setting */
typedef enum { BTHF_WBS_NONE, BTHF_WBS_NO, BTHF_WBS_YES, BTHF_WBS_SUPER } bthf_wbs_config_t;

/* CHLD - Call held handling */
typedef enum {
  BTHF_CHLD_TYPE_RELEASEHELD,  // Terminate all held or set UDUB("busy") to a
                               // waiting call
  BTHF_CHLD_TYPE_RELEASEACTIVE_ACCEPTHELD,  // Terminate all active calls and
                                            // accepts a waiting/held call
  BTHF_CHLD_TYPE_HOLDACTIVE_ACCEPTHELD,  // Hold all active calls and accepts a
                                         // waiting/held call
  BTHF_CHLD_TYPE_ADDHELDTOCONF,          // Add all held calls to a conference
} bthf_chld_type_t;

/* HF Indicators HFP 1.7 —— HF 指示器类型（对应 AT+BIND 支持的指示器 ID） */
typedef enum {
  BTHF_HF_IND_ENHANCED_DRIVER_SAFETY = 1,  // 增强驾驶安全指示器
  BTHF_HF_IND_BATTERY_LEVEL_STATUS = 2,    // 电池电量状态指示器
} bthf_hf_ind_type_t;

/* HF 指示器使能状态 —— 对应 AT+BIEV 上报前的指示器开关 */
typedef enum {
  BTHF_HF_IND_DISABLED = 0,  // 指示器已禁用
  BTHF_HF_IND_ENABLED,       // 指示器已启用
} bthf_hf_ind_status_t;

/* 网络注册状态 —— 对应 AT+COPS 查询的运营商网络可用性 */
typedef enum {
  BTHF_NETWORK_STATE_NOT_AVAILABLE = 0,  // 网络不可用（未注册）
  BTHF_NETWORK_STATE_AVAILABLE         // 网络可用（已注册）
} bthf_network_state_t;

/* 漫游服务类型 —— 对应 AT+COPS 返回的注册类型 */
typedef enum {
  BTHF_SERVICE_TYPE_HOME = 0,     // 归属网络（非漫游）
  BTHF_SERVICE_TYPE_ROAMING      // 漫游网络
} bthf_service_type_t;

/* 当前呼叫状态 —— 对应 AT+CLCC 查询的各路通话状态 */
typedef enum {
  BTHF_CALL_STATE_ACTIVE = 0,      // 通话中（已接通）
  BTHF_CALL_STATE_HELD,            // 保持中
  BTHF_CALL_STATE_DIALING,         // 拨号中（去话）
  BTHF_CALL_STATE_ALERTING,        // 对方振铃（去话已接续）
  BTHF_CALL_STATE_INCOMING,        // 来电振铃
  BTHF_CALL_STATE_WAITING,         // 呼叫等待（通话中有新来电）
  BTHF_CALL_STATE_IDLE,            // 空闲（无活动呼叫）
  BTHF_CALL_STATE_DISCONNECTED     // 已断开
} bthf_call_state_t;

/* 呼叫方向 —— 呼出还是呼入 */
typedef enum {
  BTHF_CALL_DIRECTION_OUTGOING = 0,  // 去话（呼出）
  BTHF_CALL_DIRECTION_INCOMING       // 来话（呼入）
} bthf_call_direction_t;

/* 呼叫业务模式 —— 对应 AT+CLCC 返回的通话类型 */
typedef enum {
  BTHF_CALL_TYPE_VOICE = 0,  // 语音呼叫
  BTHF_CALL_TYPE_DATA,       // 数据呼叫
  BTHF_CALL_TYPE_FAX         // 传真呼叫
} bthf_call_mode_t;

/* 多方呼叫类型 —— 对应 AT+CLCC 中 MPTY 标志 */
typedef enum {
  BTHF_CALL_MPTY_TYPE_SINGLE = 0,  // 单路呼叫（非会议）
  BTHF_CALL_MPTY_TYPE_MULTI       // 多方会议呼叫
} bthf_call_mpty_type_t;

/* 号码地址类型 —— 对应 AT+CLCC 中号码的 TON（Type of Number）编码 */
typedef enum {
  BTHF_CALL_ADDRTYPE_UNKNOWN = 0x81,        // 未知类型号码
  BTHF_CALL_ADDRTYPE_INTERNATIONAL = 0x91   // 国际号码（带 + 前缀）
} bthf_call_addrtype_t;

}  // namespace headset
}  // namespace bluetooth