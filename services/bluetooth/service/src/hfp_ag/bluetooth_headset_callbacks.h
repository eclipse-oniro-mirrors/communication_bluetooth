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
 * 本文件定义 HFP AG(音频网关，如本机手机侧)的耳机服务回调接口，
 * 由蓝牙协议栈原生层调用，用于将 HF(免提端)上报的连接状态、
 * 通话控制、AT命令等事件通知给上层服务处理。
 */

#pragma once

#include "bt_hf.h"

namespace bluetooth {
namespace headset {

/**
 * Headset (HF) related callbacks invoked from the Bluetooth native
 * stack to report events received from the remote HF device.
 * Implemented and registered by the upper layer.
 * All callbacks are invoked on the JNI thread.
 */
class Callbacks { /* 耳机(HF)事件回调接口类：由上层实现并注册，协议栈在JNI线程上调用，用于上报HF侧事件 */
 public:
  /* 虚析构函数，保证通过基类指针释放派生类对象时行为正确 */
  virtual ~Callbacks() = default;
  /**
   * Callback for HFP service-level connection state change.
   *
   * @param state current connection state, one of the
   *              bthf_connection_state_t values
   * @param bd_addr remote device address
   */
  virtual void ConnectionStateCallback(bthf_connection_state_t state,
                                        RawAddress* bd_addr) = 0; /* HFP服务层连接状态变化回调 */

  /**
   * Callback for audio connection state change.
   *
   * @param state one of the values from bthf_audio_state_t
   * @param bd_addr remote device address
   */
  virtual void AudioStateCallback(bthf_audio_state_t state,
                                  RawAddress* bd_addr) = 0; /* SCO音频连接状态变化回调 */

  /**
   * Callback for VR connection state change.
   *
   * @param state one of the values from bthf_vr_state_t
   * @param bd_addr
   */
  virtual void VoiceRecognitionCallback(bthf_vr_state_t state,
                                         RawAddress* bd_addr) = 0; /* 语音识别启动/停止请求回调 */

  /**
   * Callback for answer incoming call (ATA)
   *
   * @param bd_addr remote device address
   */
  virtual void AnswerCallCallback(RawAddress* bd_addr) = 0; /* HF请求接听来电(ATA)回调 */

  /**
   * Callback for disconnect call (AT+CHUP)
   *
   * @param bd_addr remote device address
   */
  virtual void HangupCallCallback(RawAddress* bd_addr) = 0; /* HF请求挂断通话(AT+CHUP)回调 */

  /**
   * Callback for volume control request from the HF device
   * (AT+VGS/AT+VGM).
   *
   * @param type target audio path: Speaker or Mic, one of the
   *             bthf_volume_type_t values
   * @param volume volume level, 0 to 15, p69, HFP 1.7.1 spec
   * @param bd_addr remote device address
   */
  virtual void VolumeControlCallback(bthf_volume_type_t type, int volume,
                                      RawAddress* bd_addr) = 0; /* HF音量调节请求回调(AT+VGS/AT+VGM，扬声器/麦克风，取值0~15) */

  /**
   * Callback for dialing an outgoing call
   *
   * @param number intended phone number, if number is NULL, redial
   * @param bd_addr remote device address
   */
  virtual void DialCallCallback(char* number, RawAddress* bd_addr) = 0; /* HF请求拨号回调，number为NULL时重拨上次号码 */

  /**
   * Callback for sending DTMF tones
   *
   * @param tone contains the dtmf character to be sent
   * @param bd_addr remote device address
   */
  virtual void DtmfCmdCallback(char tone, RawAddress* bd_addr) = 0; /* HF请求发送DTMF按键音回调 */

  /**
   * Callback for enabling/disabling noise reduction/echo cancellation
   *
   * @param nrec 1 to enable, 0 to disable
   * @param bd_addr remote device address
   */
  virtual void NoiseReductionCallback(bthf_nrec_t nrec,
                                      RawAddress* bd_addr) = 0; /* HF请求使能/去使能降噪回声消除(NREC)回调 */

  /**
   * Callback for AT+BCS and event from BAC
   *
   * @param wbs WBS enable, WBS disable
   * @param bd_addr remote device address
   */
  virtual void WbsCallback(bthf_wbs_config_t wbs, RawAddress* bd_addr) = 0; /* 宽带语音(WBS)编解码协商回调(AT+BCS/BAC) */

  /**
   * Callback for call hold handling (AT+CHLD)
   *
   * @param chld the call hold command (0, 1, 2, 3)
   * @param bd_addr remote device address
   */
  virtual void AtChldCallback(bthf_chld_type_t chld, RawAddress* bd_addr) = 0; /* HF通话保持/多方通话操作回调(AT+CHLD) */

  /**
   * Callback for CNUM (subscriber number)
   *
   * @param bd_addr remote device address
   */
  virtual void AtCnumCallback(RawAddress* bd_addr) = 0; /* HF查询本机用户号码回调(AT+CNUM) */

  /**
   * Callback for indicators (CIND)
   *
   * @param bd_addr remote device address
   */
  virtual void AtCindCallback(RawAddress* bd_addr) = 0; /* HF查询指示器状态回调(AT+CIND) */

  /**
   * Callback for operator selection (COPS)
   *
   * @param bd_addr remote device address
   */
  virtual void AtCopsCallback(RawAddress* bd_addr) = 0; /* HF查询当前运营商名称回调(AT+COPS) */

  /**
   * Callback for call list (AT+CLCC)
   *
   * @param bd_addr remote device address
   */
  virtual void AtClccCallback(RawAddress* bd_addr) = 0; /* HF查询当前通话列表回调(AT+CLCC) */

  /**
   * Callback for unknown AT command recd from HF
   *
   * @param at_string he unparsed AT string
   * @param bd_addr remote device address
   */
  virtual void UnknownAtCallback(char* at_string, RawAddress* bd_addr) = 0; /* 收到HF发来的无法识别的AT命令回调 */

  /**
   * Callback for keypressed (HSP) event.
   *
   * @param bd_addr remote device address
   */
  virtual void KeyPressedCallback(RawAddress* bd_addr) = 0; /* HSP按键事件回调 */

  /**
   * Callback for BIND. Pass the remote HF Indicators supported.
   *
   * @param at_string unparsed AT command string
   * @param bd_addr remote device address
   */
  virtual void AtBindCallback(char* at_string, RawAddress* bd_addr) = 0; /* HF上报其支持的HF指示器列表回调(AT+BIND) */

  /**
   * Callback for AT+BIEV. Reports a change of an HF indicator value
   * on the remote HF device.
   *
   * @param ind_id HF indicator id, one of the bthf_hf_ind_type_t values
   * @param ind_value new value of the HF indicator
   * @param bd_addr remote device address
   */
  virtual void AtBievCallback(bthf_hf_ind_type_t ind_id, int ind_value,
                               RawAddress* bd_addr) = 0; /* HF指示器值变化上报回调(AT+BIEV) */

  /**
   * Callback for AT+BIA. Reports the HF request to activate/deactivate
   * the AG indicator notifications.
   * NOTE: Call, Call Setup and Call Held indicators are mandatory and
   *       cannot be disabled, thus they are not included here.
   *
   * @param service whether HF should receive network service state updates
   * @param roam whether HF should receive roaming state updates
   * @param signal whether HF should receive signal strength updates
   * @param battery whether HF should receive AG battery level updates
   * @param bd_addr remote HF device address
   */
  virtual void AtBiaCallback(bool service, bool roam, bool signal, bool battery,
                              RawAddress* bd_addr) = 0; /* HF请求激活/去激活AG指示器上报回调(AT+BIA，通话类必选指示器不在此列) */

  /**
   * Callback for BCC. Notify the upper layer when receiving BCC command, which is used to switch the sound device.
   *
   * @param bd_addr remote HF device address
   */
#ifdef BLUETOOTH_SCO_NORMALIZED_FEATURE_ENABLE
  virtual void AtBccCallback(RawAddress* bd_addr) = 0; /* 收到BCC命令(请求切换音频设备/建立SCO)时通知上层 */
#endif
#ifdef COMMUNICATION_L2
    virtual void AtBrsfCallback(uint32_t features, RawAddress* bd_addr) = 0; /* HF上报其支持的特性(RFMI, AT+BRSF)回调 */
#endif
};

}  // namespace headset
}  // namespace bluetooth