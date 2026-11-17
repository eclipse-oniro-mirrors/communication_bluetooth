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
 * 文件说明：HFP AG（音频网关）耳机服务接口定义头文件。
 * 定义了 Fluoride 软件栈中 Headset（HFP）规范的编程接口类，
 * 供 JNI/Profile 层调用以实现耳机连接、SCO 音频、语音识别、
 * 音量控制、AT 命令响应、通话状态通知等 HFP AG 功能。
 */
#pragma once

/* HFP AG 服务事件回调接口定义（连接/音频/通话/音量等事件通知） */
#include "bluetooth_headset_callbacks.h"
/* HFP AG 底层接口类型及AT命令相关数据类型定义 */
#include "bt_hf.h"

namespace bluetooth {
/* HFP耳机(免提)协议命名空间 */
namespace headset {

/**
 * Programming interface for the Headset (HFP AG) profile in the
 * Fluoride stack. Implemented by the stack and consumed by the
 * upper layers (JNI/Profile) to manage service-level connections,
 * SCO audio, volume control, AT command responses and call
 * state notifications.
 * All methods of this interface are thread-safe.
 */
class Interface {
 public:
  virtual ~Interface() = default;
  /**
   * Initialize the HFP AG service and register the BtHf callbacks.
   * Must be called before any other interface method.
   *
   * @param callbacks callback interface provided by the upper layer
   *                  to receive stack events
   * @param max_hf_clients maximum number of simultaneously connected
   *                       headset clients
   * @param inband_ringing_enabled whether the in-band ringtone feature
   *                               is enabled
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t Init(Callbacks* callbacks, int max_hf_clients,
                           bool inband_ringing_enabled) = 0; /* 初始化 HFP 服务并注册回调 */

  /**
   * Connect to headset
   *
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t Connect(RawAddress* bd_addr) = 0; /* 建立 HFP 服务层连接 */

  /**
   * Disconnect from headset
   *
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t Disconnect(RawAddress* bd_addr) = 0; /* 断开 HFP 服务层连接 */

  /**
   * Create an audio connection
   *
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t ConnectAudio(RawAddress* bd_addr) = 0; /* 建立 SCO 音频连接 */

  /**
   * Close the audio connection
   *
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t DisconnectAudio(RawAddress* bd_addr) = 0; /* 断开 SCO 音频连接 */

  /**
   * Checks whether the device support echo cancellation and/or noise reduction
   * via the AT+BRSF bitmask
   *
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t isNoiseReductionSupported(RawAddress* bd_addr) = 0; /* 查询远端是否支持回声消除/降噪（AT+BRSF 位图） */

  /**
   * Checks whether the device supports voice recognition via the AT+BRSF
   * bitmask
   *
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t isVoiceRecognitionSupported(RawAddress* bd_addr) = 0; /* 查询远端是否支持语音识别（AT+BRSF 位图） */

  /** start voice recognition */
  /**
   * Start voice recognition
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t StartVoiceRecognition(RawAddress* bd_addr) = 0; /* 启动语音识别 */

  /**
   * Stop voice recognition
   *
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t StopVoiceRecognition(RawAddress* bd_addr) = 0; /* 停止语音识别 */

  /**
   * Change the HFP-related volume level on the remote headset,
   * sent as the +VGS/+VGM AT command.
   *
   * @param type target audio path: Speaker (+VGS) or Mic (+VGM)
   * @param volume volume level in the range 0-15, p69, HFP 1.7.1 spec
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t VolumeControl(bthf_volume_type_t type, int volume,
                                    RawAddress* bd_addr) = 0; /* 调节远端耳机音量（扬声器+VGS/麦克风+VGM，音量范围0-15） */

  /**
   * Send a combined device status change notification to the remote
   * headset. Notifies the network availability, service type, signal
   * strength and battery level at once.
   *
   * @param ntk_state network state: available or not available
   * @param svc_type service type: roaming or home
   * @param signal signal strength, 0 to 5, p86, HFP 1.7.1 spec
   * @param batt_chg battery level of the AG device, 0 to 5, p87, HFP 1.7.1 spec
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t DeviceStatusNotification(bthf_network_state_t ntk_state,
                                               bthf_service_type_t svc_type,
                                               int signal, int batt_chg,
                                               RawAddress* bd_addr) = 0; /* 向远端耳机组合上报设备状态变化（网络/服务类型/信号/电池） */

  /**
   * Send the response for the COPS (Query Operator Selection) command.
   *
   * @param cops operator name, max length 16 chars, p32, HFP 1.7.1 spec
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t CopsResponse(const char* cops, RawAddress* bd_addr) = 0; /* 响应 COPS 命令，上报当前运营商名称（最长16字符） */

  /**
   * Send the response for the CIND (Indicator Update) command, reporting
   * the current values of all HFP device indicators.
   *
   * @param svc service availability: available or not available
   * @param num_active number of active calls
   * @param num_held number of held calls
   * @param call_setup_state current call setup state
   * @param signal signal strength, 0 to 5, p86, HFP 1.7.1 spec
   * @param roam roaming state: 1 for roaming, 0 for home, p86, HFP 1.7.1 spec
   * @param batt_chg AG battery charge, 0 to 5, p87, HFP 1.7.1 spec
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t CindResponse(int svc, int num_active, int num_held,
                                   bthf_call_state_t call_setup_state,
                                   int signal, int roam, int batt_chg,
                                   RawAddress* bd_addr) = 0; /* 响应 CIND 命令，上报全部设备指示器当前值（服务/通话/信号/漫游/电池） */

  /**
   * Send a pre-formatted AT response string to the remote headset.
   * Typically used to reply to AT commands not recognized by the stack.
   *
   * @param rsp pre-formatted AT response string
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t FormattedAtResponse(const char* rsp,
                                          RawAddress* bd_addr) = 0; /* 发送预格式化 AT 响应（通常用于应答协议栈无法识别的 AT 命令） */

  /**
   * Send an OK/ERROR response to conclude an AT command received from
   * the remote headset.
   *
   * @param response_code OK or ERROR
   * @param error_code extended error code, interpretation depends on use case
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t AtResponse(bthf_at_response_t response_code,
                                 int error_code, RawAddress* bd_addr) = 0; /* 发送 AT 命令的 OK/ERROR 应答，结束一条 AT 命令交互 */

  /**
   * Send the response for the CLCC (Current List of Calls) command.
   * Called iteratively once per call entry; a call index of 0 is treated
   * as NULL termination and completes the response.
   *
   * @param index index of the call entry, 0 to terminate the list
   * @param dir direction of the call (outgoing/incoming)
   * @param state current state of the call
   * @param mode mode of the call (voice/data/fax)
   * @param mpty whether the call is part of a multi-party call
   * @param number phone number of the call
   * @param type address type of the phone number
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t ClccResponse(
      int index, bthf_call_direction_t dir, bthf_call_state_t state,
      bthf_call_mode_t mode, bthf_call_mpty_type_t mpty, const char* number,
      bthf_call_addrtype_t type, RawAddress* bd_addr) = 0; /* 响应 CLCC 命令，逐条上报当前通话列表，index为0时结束上报 */

  /**
   * Notify the remote headset of a call state change.
   * Each update reports:
   *   1. The number of active/held/ringing calls
   *   2. call_setup_state: the state change that triggered this
   *      notification, one of the bthf_call_state_t values
   *   3. number, type and name: valid only for incoming and
   *      waiting calls
   *
   * @param num_active number of active calls
   * @param num_held number of held calls
   * @param call_setup_state current call setup state
   * @param number phone number of the call, valid for incoming/waiting calls
   * @param type address type of the phone number
   * @param name caller display name, valid for incoming/waiting calls
   * @param bd_addr remote device address
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  virtual bt_status_t PhoneStateChange(int num_active, int num_held,
                                       bthf_call_state_t call_setup_state,
                                       const char* number,
                                       bthf_call_addrtype_t type,
                                       const char* name,
                                       RawAddress* bd_addr) = 0; /* 通知远端耳机通话状态变化（激活/保持通话数、来电号码及来电人姓名等） */

  /**
   * Closes the interface.
   */
  virtual void Cleanup() = 0; /* 关闭并清理接口 */

  /**
   * Whether we are allowed to initiate SCO
   *
   * @param value true to allow, false to disallow
   * @return BT_STATUS_SUCCESS on success
   */
  virtual bt_status_t SetScoAllowed(bool value) = 0; /* 设置是否允许主动发起 SCO 连接 */

  /**
   * Send +BSIR response code to enable/disable in-band ringtone in an active
   * HFP service level connection
   *
   * @param value true for enabled, false for disable
   * @param bd_addr remote device address
   */
  virtual bt_status_t SendBsir(bool value, RawAddress* bd_addr) = 0; /* 发送 +BSIR 响应码，启用/禁用带内铃声 */

  /**
   * Set the current active headset device for SCO audio
   *
   * @param active_device_addr remote device address
   */
  virtual bt_status_t SetActiveDevice(RawAddress* active_device_addr) = 0; /* 设置 SCO 音频当前活跃的耳机设备 */
};

}  // namespace headset
}  // namespace bluetooth
