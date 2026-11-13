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
 * 本文件定义蓝牙 HFP(Hands-Free Profile，免提规范)客户端(HF)接口，
 * 用于本设备作为免提终端与远端音频网关(AG，如手机)建立连接，
 * 实现通话控制、音量调节、语音识别、网络状态查询等免提功能。
 */

#ifndef VENDOR_INCLUDE_BT_HF_CLIENT_H
#define VENDOR_INCLUDE_BT_HF_CLIENT_H
#include <vector>
#include <hardware/bluetooth.h>

extern "C" { //__BEGIN_DECLS

typedef enum {
  BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED = 0, /* 已断开 */
  BTHF_CLIENT_CONNECTION_STATE_CONNECTING,   /* 连接中 */
  BTHF_CLIENT_CONNECTION_STATE_CONNECTED,    /* 已连接(RFCOMM) */
  BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED, /* 服务层连接(SLC)已建立 */
  BTHF_CLIENT_CONNECTION_STATE_DISCONNECTING /* 断开中 */
} bthf_client_connection_state_t;

typedef enum {
  BTHF_CLIENT_AUDIO_STATE_DISCONNECTED = 0, /* 音频已断开 */
  BTHF_CLIENT_AUDIO_STATE_CONNECTING,  /* 音频连接中 */
  BTHF_CLIENT_AUDIO_STATE_CONNECTED,   /* 音频已连接(CVSD) */
  BTHF_CLIENT_AUDIO_STATE_CONNECTED_MSBC, /* 音频已连接(mSBC编码) */
} bthf_client_audio_state_t;

typedef enum {
  BTHF_CLIENT_VR_STATE_STOPPED = 0, /* 语音识别已停止 */
  BTHF_CLIENT_VR_STATE_STARTED /* 语音识别已启动 */
} bthf_client_vr_state_t;

typedef enum {
  BTHF_CLIENT_VOLUME_TYPE_SPK = 0, /* 扬声器音量 */
  BTHF_CLIENT_VOLUME_TYPE_MIC /* 麦克风音量 */
} bthf_client_volume_type_t;

typedef enum {
  BTHF_CLIENT_NETWORK_STATE_NOT_AVAILABLE = 0, /* 网络不可用(无服务) */
  BTHF_CLIENT_NETWORK_STATE_AVAILABLE /* 网络可用 */
} bthf_client_network_state_t;

typedef enum {
  BTHF_CLIENT_SERVICE_TYPE_HOME = 0, /* 归属网络(非漫游) */
  BTHF_CLIENT_SERVICE_TYPE_ROAMING /* 漫游网络 */
} bthf_client_service_type_t;

typedef enum {
  BTHF_CLIENT_CALL_STATE_ACTIVE = 0, /* 通话激活 */
  BTHF_CLIENT_CALL_STATE_HELD,  /* 通话保持 */
  BTHF_CLIENT_CALL_STATE_DIALING, /* 正在拨出 */
  BTHF_CLIENT_CALL_STATE_ALERTING, /* 对方振铃(拨出已接续) */
  BTHF_CLIENT_CALL_STATE_INCOMING, /* 来电振铃 */
  BTHF_CLIENT_CALL_STATE_WAITING,  /* 来电等待 */
  BTHF_CLIENT_CALL_STATE_HELD_BY_RESP_HOLD, /* 被响应与保持挂起 */
} bthf_client_call_state_t;

typedef enum {
  BTHF_CLIENT_CALL_NO_CALLS_IN_PROGRESS = 0, /* 无进行中的通话 */
  BTHF_CLIENT_CALL_CALLS_IN_PROGRESS /* 有进行中的通话 */
} bthf_client_call_t;

typedef enum {
  BTHF_CLIENT_CALLSETUP_NONE = 0, /* 无呼叫建立过程 */
  BTHF_CLIENT_CALLSETUP_INCOMING, /* 来电建立中 */
  BTHF_CLIENT_CALLSETUP_OUTGOING, /* 去电拨号中 */
  BTHF_CLIENT_CALLSETUP_ALERTING /* 对方振铃 */
} bthf_client_callsetup_t;

typedef enum {
  BTHF_CLIENT_CALLHELD_NONE = 0, /* 无保持的通话 */
  BTHF_CLIENT_CALLHELD_HOLD_AND_ACTIVE, /* 既有保持通话又有激活通话 */
  BTHF_CLIENT_CALLHELD_HOLD, /* 仅有保持通话，无激活通话 */
} bthf_client_callheld_t;

typedef enum {
  BTHF_CLIENT_RESP_AND_HOLD_HELD = 0, /* 响应与保持：已挂起 */
  BTRH_CLIENT_RESP_AND_HOLD_ACCEPT, /* 响应与保持：接受 */
  BTRH_CLIENT_RESP_AND_HOLD_REJECT, /* 响应与保持：拒绝 */
} bthf_client_resp_and_hold_t;

typedef enum {
  BTHF_CLIENT_CALL_DIRECTION_OUTGOING = 0, /* 去电 */
  BTHF_CLIENT_CALL_DIRECTION_INCOMING /* 来电 */
} bthf_client_call_direction_t;

typedef enum {
  BTHF_CLIENT_CALL_MPTY_TYPE_SINGLE = 0, /* 单方通话 */
  BTHF_CLIENT_CALL_MPTY_TYPE_MULTI /* 多方通话 */
} bthf_client_call_mpty_type_t;

typedef enum {
  BTHF_CLIENT_CMD_COMPLETE_OK = 0, /* 命令执行成功 */
  BTHF_CLIENT_CMD_COMPLETE_ERROR, /* 命令执行错误 */
  BTHF_CLIENT_CMD_COMPLETE_ERROR_NO_CARRIER, /* 错误：无载波 */
  BTHF_CLIENT_CMD_COMPLETE_ERROR_BUSY, /* 错误：忙碌 */
  BTHF_CLIENT_CMD_COMPLETE_ERROR_NO_ANSWER, /* 错误：无应答 */
  BTHF_CLIENT_CMD_COMPLETE_ERROR_DELAYED, /* 错误：延迟 */
  BTHF_CLIENT_CMD_COMPLETE_ERROR_REJECTLISTED, /* 错误：被列入黑名单 */
  BTHF_CLIENT_CMD_COMPLETE_ERROR_CME /* 错误：CME扩展错误码(见cme参数) */
} bthf_client_cmd_complete_t;

typedef enum {
  BTHF_CLIENT_CALL_ACTION_CHLD_0 = 0, /* CHLD 0: 释放等待或保持的通话 */
  BTHF_CLIENT_CALL_ACTION_CHLD_1, /* CHLD 1: 释放激活通话并接听其他通话 */
  BTHF_CLIENT_CALL_ACTION_CHLD_2, /* CHLD 2: 激活通话保持并接听其他通话 */
  BTHF_CLIENT_CALL_ACTION_CHLD_3, /* CHLD 3: 将保持通话加入多方通话 */
  BTHF_CLIENT_CALL_ACTION_CHLD_4, /* CHLD 4: 连接两路通话并退出多方通话 */
  BTHF_CLIENT_CALL_ACTION_CHLD_1x, /* CHLD 1x: 释放指定索引的激活通话 */
  BTHF_CLIENT_CALL_ACTION_CHLD_2x, /* CHLD 2x: 与指定通话私密模式(其余保持) */
  BTHF_CLIENT_CALL_ACTION_ATA, /* ATA: 接听来电 */
  BTHF_CLIENT_CALL_ACTION_CHUP, /* CHUP: 挂断当前通话 */
  BTHF_CLIENT_CALL_ACTION_BTRH_0, /* BTRH 0: 挂起来电(响应与保持) */
  BTHF_CLIENT_CALL_ACTION_BTRH_1, /* BTRH 1: 接受被挂起的来电 */
  BTHF_CLIENT_CALL_ACTION_BTRH_2, /* BTRH 2: 拒绝被挂起的来电 */
} bthf_client_call_action_t;

typedef enum {
  BTHF_CLIENT_SERVICE_UNKNOWN = 0, /* 未知业务类型 */
  BTHF_CLIENT_SERVICE_VOICE, /* 语音业务 */
  BTHF_CLIENT_SERVICE_FAX /* 传真业务 */
} bthf_client_subscriber_service_type_t;

typedef enum {
  BTHF_CLIENT_IN_BAND_RINGTONE_NOT_PROVIDED = 0, /* 不支持带内铃声 */
  BTHF_CLIENT_IN_BAND_RINGTONE_PROVIDED, /* 支持带内铃声 */
} bthf_client_in_band_ring_state_t;

/* Peer features masks */
#define BTHF_CLIENT_PEER_FEAT_3WAY 0x00000001 /* Three-way calling */
#define BTHF_CLIENT_PEER_FEAT_ECNR \
  0x00000002 /* Echo cancellation and/or noise reduction */
#define BTHF_CLIENT_PEER_FEAT_VREC 0x00000004   /* Voice recognition */
#define BTHF_CLIENT_PEER_FEAT_INBAND 0x00000008 /* In-band ring tone */
#define BTHF_CLIENT_PEER_FEAT_VTAG \
  0x00000010 /* Attach a phone number to a voice tag */
#define BTHF_CLIENT_PEER_FEAT_REJECT \
  0x00000020 /* Ability to reject incoming call */
#define BTHF_CLIENT_PEER_FEAT_ECS 0x00000040    /* Enhanced Call Status */
#define BTHF_CLIENT_PEER_FEAT_ECC 0x00000080    /* Enhanced Call Control */
#define BTHF_CLIENT_PEER_FEAT_EXTERR 0x00000100 /* Extended error codes */
#define BTHF_CLIENT_PEER_FEAT_CODEC 0x00000200  /* Codec Negotiation */

/* Peer call handling features masks */
#define BTHF_CLIENT_CHLD_FEAT_REL \
  0x00000001 /* 0  Release waiting call or held calls */
#define BTHF_CLIENT_CHLD_FEAT_REL_ACC                    \
  0x00000002 /* 1  Release active calls and accept other \
                   (waiting or held) cal */
#define BTHF_CLIENT_CHLD_FEAT_REL_X \
  0x00000004 /* 1x Release specified active call only */
#define BTHF_CLIENT_CHLD_FEAT_HOLD_ACC                   \
  0x00000008 /* 2  Active calls on hold and accept other \
                   (waiting or held) call */
#define BTHF_CLIENT_CHLD_FEAT_PRIV_X                   \
  0x00000010 /* 2x Request private mode with specified \
                   call (put the rest on hold) */
#define BTHF_CLIENT_CHLD_FEAT_MERGE \
  0x00000020 /* 3  Add held call to multiparty */
#define BTHF_CLIENT_CHLD_FEAT_MERGE_DETACH     \
  0x00000040 /* 4  Connect two calls and leave \
                   (disconnect from) multiparty */

/** Callback for connection state change.
 *  state will have one of the values from BtHfConnectionState
 *  peer/chld_features are valid only for
 * BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED state
 */
typedef void (*bthf_client_connection_state_callback)(
    const RawAddress* bd_addr, bthf_client_connection_state_t state,
    unsigned int peer_feat, unsigned int chld_feat);

/** Callback for audio connection state change.
 *  state will have one of the values from BtHfAudioState
 */
typedef void (*bthf_client_audio_state_callback)(
    const RawAddress* bd_addr, bthf_client_audio_state_t state);

/** Callback for VR connection state change.
 *  state will have one of the values from BtHfVRState
 */
typedef void (*bthf_client_vr_cmd_callback)(const RawAddress* bd_addr,
                                            bthf_client_vr_state_t state);

/** Callback for network state change
 */
typedef void (*bthf_client_network_state_callback)(
    const RawAddress* bd_addr, bthf_client_network_state_t state);

/** Callback for network roaming status change
 */
typedef void (*bthf_client_network_roaming_callback)(
    const RawAddress* bd_addr, bthf_client_service_type_t type);

/**
 * Callback for the signal strength indicator reported by the AG
 * (AT+CIEV signal).
 *
 * @param bd_addr remote AG device address
 * @param signal_strength signal strength, 0 to 5
 */
typedef void (*bthf_client_network_signal_callback)(const RawAddress* bd_addr,
                                                    int signal_strength); /* AG上报信号强度指示回调 */

/**
 * Callback for the battery level indicator reported by the AG
 * (AT+CIEV batt_chg).
 *
 * @param bd_addr remote AG device address
 * @param battery_level AG battery level, 0 to 5
 */
typedef void (*bthf_client_battery_level_callback)(const RawAddress* bd_addr,
                                                   int battery_level); /* AG上报电池电量指示回调 */

/**
 * Callback for the current operator name reported by the AG
 * (response to AT+COPS).
 *
 * @param bd_addr remote AG device address
 * @param name current network operator name
 */
typedef void (*bthf_client_current_operator_callback)(const RawAddress* bd_addr,
                                                      const char* name); /* AG上报当前运营商名称回调 */

/**
 * Callback for the call indicator reported by the AG (AT+CIEV call),
 * indicating whether there is at least one call in progress.
 *
 * @param bd_addr remote AG device address
 * @param call current call indicator, one of the bthf_client_call_t values
 */
typedef void (*bthf_client_call_callback)(const RawAddress* bd_addr,
                                          bthf_client_call_t call); /* AG上报通话指示回调(是否有通话进行中) */

/** Callback for callsetup indicator
 */
typedef void (*bthf_client_callsetup_callback)(
    const RawAddress* bd_addr, bthf_client_callsetup_t callsetup);

/** Callback for callheld indicator
 */
typedef void (*bthf_client_callheld_callback)(const RawAddress* bd_addr,
                                              bthf_client_callheld_t callheld);

/** Callback for response and hold
 */
typedef void (*bthf_client_resp_and_hold_callback)(
    const RawAddress* bd_addr, bthf_client_resp_and_hold_t resp_and_hold);

/** Callback for Calling Line Identification notification
 *  Will be called only when there is an incoming call and number is provided.
 */
typedef void (*bthf_client_clip_callback)(const RawAddress* bd_addr,
                                          const char* number);

/**
 * Callback for Call Waiting notification
 */
typedef void (*bthf_client_call_waiting_callback)(const RawAddress* bd_addr,
                                                  const char* number);

/**
 * Callback for the current call list reported by the AG
 * (response to AT+CLCC). Invoked once per call entry and may be
 * called multiple times until the list is complete.
 *
 * @param bd_addr remote AG device address
 * @param index index of the call entry
 * @param dir direction of the call (outgoing/incoming)
 * @param state current state of the call
 * @param mpty whether the call is part of a multi-party call
 * @param number phone number of the call, NULL if unknown
 */
typedef void (*bthf_client_current_calls)(const RawAddress* bd_addr, int index,
                                          bthf_client_call_direction_t dir,
                                          bthf_client_call_state_t state,
                                          bthf_client_call_mpty_type_t mpty,
                                          const char* number); /* AG上报当前通话列表回调(AT+CLCC，逐条上报，number未知时为NULL) */

/**
 * Callback for audio volume change reported by the AG (+VGS/+VGM).
 *
 * @param bd_addr remote AG device address
 * @param type target audio path: Speaker or Mic
 * @param volume volume level, 0 to 15
 */
typedef void (*bthf_client_volume_change_callback)(
    const RawAddress* bd_addr, bthf_client_volume_type_t type,
    int volume); /* AG上报音量变化回调(+VGS/+VGM，取值0~15) */

/** Callback for command complete event
 *  cme is valid only for BTHF_CLIENT_CMD_COMPLETE_ERROR_CME type
 */
typedef void (*bthf_client_cmd_complete_callback)(
    const RawAddress* bd_addr, bthf_client_cmd_complete_t type, int cme);

/** Callback for subscriber information
 */
typedef void (*bthf_client_subscriber_info_callback)(
    const RawAddress* bd_addr, const char* name,
    bthf_client_subscriber_service_type_t type);

/** Callback for in-band ring tone settings
 */
typedef void (*bthf_client_in_band_ring_tone_callback)(
    const RawAddress* bd_addr, bthf_client_in_band_ring_state_t state);

/**
 * Callback for requested number from AG
 */
typedef void (*bthf_client_last_voice_tag_number_callback)(
    const RawAddress* bd_addr, const char* number);

/**
 * Callback for sending ring indication to app
 */
typedef void (*bthf_client_ring_indication_callback)(const RawAddress* bd_addr);

/**
 * Callback for sending unknown (vendor specific) event
 */
typedef void (*bthf_client_unknown_event_callback)(const RawAddress* bd_addr,
                                                   const char* unknow_event);

/** BT-HF callback structure. */
typedef struct {
  /** set to sizeof(BtHfClientCallbacks) */
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  bthf_client_connection_state_callback connection_state_cb; /* 连接状态变化回调 */
  bthf_client_audio_state_callback audio_state_cb; /* 音频连接状态变化回调 */
  bthf_client_vr_cmd_callback vr_cmd_cb; /* 语音识别状态变化回调 */
  bthf_client_network_state_callback network_state_cb; /* 网络可用状态变化回调 */
  bthf_client_network_roaming_callback network_roaming_cb; /* 网络漫游状态变化回调 */
  bthf_client_network_signal_callback network_signal_cb; /* 网络信号强度变化回调 */
  bthf_client_battery_level_callback battery_level_cb; /* AG电池电量上报回调 */
  bthf_client_current_operator_callback current_operator_cb; /* 当前运营商名称上报回调 */
  bthf_client_call_callback call_cb; /* 通话指示(有无通话)变化回调 */
  bthf_client_callsetup_callback callsetup_cb; /* 呼叫建立状态变化回调 */
  bthf_client_callheld_callback callheld_cb; /* 通话保持状态变化回调 */
  bthf_client_resp_and_hold_callback resp_and_hold_cb; /* 响应与保持状态回调 */
  bthf_client_clip_callback clip_cb; /* 来电号码识别(CLIP)通知回调 */
  bthf_client_call_waiting_callback call_waiting_cb; /* 来电等待(CCWA)通知回调 */
  bthf_client_current_calls current_calls_cb; /* 当前通话列表上报回调 */
  bthf_client_volume_change_callback volume_change_cb; /* 音量变化回调 */
  bthf_client_cmd_complete_callback cmd_complete_cb; /* AT命令执行结果回调 */
  bthf_client_subscriber_info_callback subscriber_info_cb; /* 用户号码订阅信息回调 */
  bthf_client_in_band_ring_tone_callback in_band_ring_tone_cb; /* 带内铃声设置回调 */
  bthf_client_last_voice_tag_number_callback last_voice_tag_number_callback; /* 语音标签对应号码查询结果回调 */
  bthf_client_ring_indication_callback ring_indication_cb; /* 振铃指示回调 */
  bthf_client_unknown_event_callback unknown_event_cb; /* 未知(厂商私有)事件回调 */
} bthf_client_callbacks_t;

/** Represents the standard BT-HF interface. */
typedef struct {
  /** set to sizeof(BtHfClientInterface) */
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  /**
   * Register the BtHf callbacks
   */
  bt_status_t (*init)(bthf_client_callbacks_t* callbacks); /* 初始化HF客户端并注册回调 */

  /** connect to audio gateway */
  bt_status_t (*connect)(RawAddress* bd_addr); /* 连接音频网关(AG)，建立RFCOMM连接 */

  /** disconnect from audio gateway */
  bt_status_t (*disconnect)(const RawAddress* bd_addr); /* 断开与音频网关的连接 */

  /** create an audio connection */
  bt_status_t (*connect_audio)(const RawAddress* bd_addr); /* 建立SCO音频连接 */

  /** close the audio connection */
  bt_status_t (*disconnect_audio)(const RawAddress* bd_addr); /* 断开SCO音频连接 */

  /** start voice recognition */
  bt_status_t (*start_voice_recognition)(const RawAddress* bd_addr); /* 启动语音识别 */

  /** stop voice recognition */
  bt_status_t (*stop_voice_recognition)(const RawAddress* bd_addr); /* 停止语音识别 */

  /** volume control */
  bt_status_t (*volume_control)(const RawAddress* bd_addr,
                                bthf_client_volume_type_t type, int volume); /* 音量控制(扬声器/麦克风) */

  /** place a call with number a number
   * if number is NULL last called number is called (aka re-dial)*/
  bt_status_t (*dial)(const RawAddress* bd_addr, const char* number); /* 拨打电话，number为NULL时重拨上次号码 */

  /** place a call with number specified by location (speed dial) */
  bt_status_t (*dial_memory)(const RawAddress* bd_addr, int location); /* 按存储位置快速拨号 */

  /** perform specified call related action
   * idx is limited only for enhanced call control related action
   */
  bt_status_t (*handle_call_action)(const RawAddress* bd_addr,
                                    bthf_client_call_action_t action, int idx); /* 执行指定通话操作(CHLD/ATA/CHUP等) */

  /** query list of current calls */
  bt_status_t (*query_current_calls)(const RawAddress* bd_addr); /* 查询当前通话列表 */

  /** query name of current selected operator */
  bt_status_t (*query_current_operator_name)(const RawAddress* bd_addr); /* 查询当前运营商名称 */

  /** Retrieve subscriber information */
  bt_status_t (*retrieve_subscriber_info)(const RawAddress* bd_addr); /* 获取本机用户号码订阅信息 */

  /** Send DTMF code*/
  bt_status_t (*send_dtmf)(const RawAddress* bd_addr, char code); /* 发送DTMF码 */

  /** Request a phone number from AG corresponding to last voice tag recorded */
  bt_status_t (*request_last_voice_tag_number)(const RawAddress* bd_addr); /* 向AG查询最近语音标签对应的电话号码 */

  /** Closes the interface. */
  void (*cleanup)(void); /* 清理并关闭HF客户端接口 */

  /** Send AT Command. */
  bt_status_t (*send_at_cmd)(const RawAddress* bd_addr, int cmd, int val1,
                             int val2, const char* arg); /* 发送厂商扩展AT命令 */
} bthf_client_interface_t;

} //__END_DECLS

#endif /* VENDOR_INCLUDE_BT_HF_CLIENT_H */
