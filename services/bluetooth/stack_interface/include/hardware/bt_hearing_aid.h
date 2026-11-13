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
 * 文件说明：助听器规范（Hearing Aid Profile）接口定义头文件。
 * 定义了助听器规范的连接状态枚举、回调接口类（HearingAidCallbacks）与
 * 服务接口类（HearingAidInterface），供上层（JNI/Profile 层）与协议栈
 * 之间进行助听器设备连接、音量设置、音频发送等交互使用。
 */
#ifndef VENDOR_INCLUDE_BT_HEARING_AID_H
#define VENDOR_INCLUDE_BT_HEARING_AID_H

#include <hardware/bluetooth.h>

#include "types/raw_address.h"

namespace bluetooth {
namespace hearing_aid {

/* 助听器规范连接状态枚举 */
enum class ConnectionState {
  DISCONNECTED = 0, /* 已断开 */
  CONNECTING,       /* 连接中 */
  CONNECTED,        /* 已连接 */
  DISCONNECTING     /* 断开中 */
};

/* 助听器规范事件回调接口类，由上层实现并注册给协议栈 */
class HearingAidCallbacks {
 public:
  virtual ~HearingAidCallbacks() = default;

  /** Callback for profile connection state change */
  /* 连接状态变化回调 */
  virtual void OnConnectionState(ConnectionState state,
                                 const RawAddress& address) = 0; /* 连接状态变化通知 */

  /** Callback for device being available. Is executed when devices are loaded
   * from storage on stack bringup, and when new device is connected to profile.
   * Main purpose of this callback is to keep its users informed of device
   * capabilities and hiSyncId.
   */
  /* 设备可用回调：协议栈启动时从存储加载设备、或新设备接入规范时触发，
   * 用于向使用者同步设备能力与 hiSyncId */
  virtual void OnDeviceAvailable(uint8_t capabilities, uint64_t hiSyncId,
                                 const RawAddress& address) = 0; /* 设备可用通知（携带设备能力与 hiSyncId） */
};

/* 助听器规范服务接口类，由协议栈实现，供上层调用 */
class HearingAidInterface {
public:
  virtual ~HearingAidInterface() = default;

  /** Register the Hearing Aid callbacks */
  virtual void Init(HearingAidCallbacks* callbacks) = 0; /* 初始化并注册助听器回调 */

  /** Connect to Hearing Aid */
  virtual void Connect(const RawAddress& address) = 0; /* 连接助听器设备 */

  /** Disconnect from Hearing Aid */
  virtual void Disconnect(const RawAddress& address) = 0; /* 断开助听器设备连接 */

  /** Add a hearing aid device to acceptlist */
  virtual void AddToAcceptlist(const RawAddress& address) = 0; /* 将助听器设备加入接受列表 */

  /** Set the volume */
  virtual void SetVolume(int8_t volume) = 0; /* 设置音量 */

  /** Set the volume by device address */
  virtual void SetVolumeByAddr(const RawAddress& address, int8_t volume) = 0; /* 按设备地址设置音量 */

  /** Start hearing aid audio session by deivice address */
  virtual void StartSendingAudio(const RawAddress& address) = 0; /* 按设备地址启动助听器音频会话 */

  /** Closes the interface. */
  virtual void Cleanup(void) = 0; /* 关闭并清理接口 */

  /* Called when Hearing Aid is unbonded. */
  virtual void RemoveDevice(const RawAddress& address) = 0; /* 助听器设备解除配对时移除该设备 */
};

}  // namespace hearing_aid
}  // namespace bluetooth

#endif /* VENDOR_INCLUDE_BT_HEARING_AID_H */
