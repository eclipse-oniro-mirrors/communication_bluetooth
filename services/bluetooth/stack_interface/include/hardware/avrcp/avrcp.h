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
 * 文件说明：AVRCP（音视频远程控制规范）接口定义头文件。
 * 定义了 AVRCP 服务所使用的媒体信息结构（歌曲/播放状态/播放器/文件夹等）、
 * 媒体回调接口（MediaCallbacks）、媒体层接口（MediaInterface）、
 * 音量接口（VolumeInterface）以及服务接口（ServiceInterface），
 * 供 JNI/媒体层与 AVRCP 协议栈之间进行播放控制、浏览、绝对音量等交互。
 */
#pragma once


#include <functional>
#include <set>
#include <string>
#include <vector>

#include "hardware/avrcp/avrcp_common.h"
#include "types/raw_address.h"

namespace bluetooth {
namespace avrcp {

#define UNUSED_ATTR __attribute__((unused))

/* 歌曲信息结构，包含媒体 ID 及各属性条目（标题/歌手/专辑等） */
struct SongInfo {
  std::string media_id = "";  // This gets converted to a UID in the native service /* 媒体 ID，会在原生服务中转换为 UID */
  std::set<AttributeEntry> attributes { /* 歌曲属性集合，含默认值 */
    AttributeEntry(Attribute::TITLE, "Not Provided"),              /* 标题 */
    AttributeEntry(Attribute::ARTIST_NAME, ""),                    /* 歌手名 */
    AttributeEntry(Attribute::ALBUM_NAME, ""),                     /* 专辑名 */
    AttributeEntry(Attribute::TRACK_NUMBER, "1"),                  /* 音轨号 */
    AttributeEntry(Attribute::TOTAL_NUMBER_OF_TRACKS, "1"),        /* 总音轨数 */
    AttributeEntry(Attribute::GENRE, ""),                          /* 流派 */
    AttributeEntry(Attribute::PLAYING_TIME, "0"),                  /* 播放时长 */
    AttributeEntry(Attribute::DEFAULT_COVER_ART, "0000001")        /* 默认封面艺术 */
  };
};

/* 播放状态枚举 */
enum PlayState : uint8_t {
  STOPPED = 0x00, /* 已停止 */
  PLAYING,        /* 播放中 */
  PAUSED,         /* 已暂停 */
  FWD_SEEK,       /* 快进寻道 */
  REV_SEEK,       /* 快退寻道 */
  ERROR = 0xFF,   /* 错误状态 */
};

/* 播放状态信息结构 */
struct PlayStatus {
  uint32_t position = 0; /* 当前播放位置（毫秒） */
  uint32_t duration = 0; /* 歌曲总时长（毫秒） */
  PlayState state;       /* 当前播放状态 */
};

/* 媒体播放器信息结构 */
struct MediaPlayerInfo {
  uint16_t id;               /* 播放器 ID */
  std::string name;          /* 播放器名称 */
  bool browsing_supported;   /* 是否支持浏览功能 */
};

/* 文件夹信息结构 */
struct FolderInfo {
  std::string media_id;   /* 文件夹媒体 ID */
  bool is_playable;       /* 文件夹是否可播放 */
  std::string name;       /* 文件夹名称 */
};

// TODO (apanicke): Convert this to a union
/* 列表项结构，表示浏览列表中的一个条目（文件夹或歌曲） */
struct ListItem {
  /* 列表项类型枚举：文件夹或歌曲 */
  enum : uint8_t {
    FOLDER, /* 文件夹 */
    SONG,   /* 歌曲 */
  } type;   /* 条目类型 */

  FolderInfo folder; /* 文件夹信息（type 为 FOLDER 时有效） */
  SongInfo song;     /* 歌曲信息（type 为 SONG 时有效） */
};

/* 媒体更新回调接口类，由 AVRCP 服务实现，用于向媒体层发送变化通知 */
class MediaCallbacks {
 public:
  virtual void SendMediaUpdate(bool track_changed, bool play_state,
                               bool queue) = 0; /* 通知媒体更新（歌曲变化/播放状态/播放队列） */
  virtual void SendFolderUpdate(bool available_players, bool addressed_players,
                                bool uids_changed) = 0; /* 通知文件夹浏览更新（可用播放器/寻址播放器/UID 变化） */
  virtual void SendActiveDeviceChanged(const RawAddress& address) = 0; /* 通知活跃设备发生变化 */
#ifdef MULTI_A2DP_ENABLED
  // Multi A2DP car: send the update only to the device designated by |bdaddr|.
  // Default fallback broadcasts to all devices (regression safe when the
  // upper layer has not adopted the ByAddress variants yet).
  virtual void SendMediaUpdateByAddress(bool track_changed, bool play_state,
                                        bool queue, const RawAddress& bdaddr) {
    SendMediaUpdate(track_changed, play_state, queue);
  }
  virtual void SendFolderUpdateByAddress(bool available_players,
                                         bool addressed_players,
                                         bool uids_changed,
                                         const RawAddress& bdaddr) {
    SendFolderUpdate(available_players, addressed_players, uids_changed);
  }
#endif
  virtual ~MediaCallbacks() = default;
};

// The classes below are used by the JNI and are loaded dynamically with the
// Bluetooth library. All classes must be pure virtual otherwise a compiler
// error occurs when trying to link the function implementation.

// MediaInterface defines the class that the AVRCP Service uses in order
// communicate with the media layer. The media layer will define its own
// implementation of this object and register it with the service using
// Avrcp::ServiceInterface::Init(). At this point the AVRCP Service will
// call RegisterUpdateCallbacks() to provide an handle to use to send
// notifications about changes in the Media Interface.
//
// NOTES: The current implementation has the native service handle all the
// thread switching. It will call the interface functions on the btif/jni
// thread and the callback will post its results to the bta thread.
// In the future the interface the JNI registered with the
// service should post all its tasks to the JNI thread itself so that the native
// service isn't aware of the thread the interface functions need to be called
// on. It can then supply callbacks that post results to the correct thread
// allowing the threading model to be totally encapsulated and allow correct
// behavior in case the threading model changes on either side.
class MediaInterface { /* 媒体层接口类，AVRCP 服务通过它与媒体层通信（由媒体层实现并注册） */
 public:
  virtual void SendKeyEvent(uint8_t key, KeyState state) = 0; /* 发送按键事件（播放/暂停/上下曲等） */

  using SongInfoCallback = std::function<void(SongInfo)>; /* 歌曲信息回调类型 */
  virtual void GetSongInfo(SongInfoCallback info_cb) = 0; /* 获取当前歌曲信息，结果通过回调返回 */

  using PlayStatusCallback = std::function<void(PlayStatus)>; /* 播放状态回调类型 */
  virtual void GetPlayStatus(PlayStatusCallback status_cb) = 0; /* 获取当前播放状态，结果通过回调返回 */

  // Contains the current queue and the media ID of the currently playing item
  // in the queue
  using NowPlayingCallback = std::function<void(std::string, std::vector<SongInfo>)>; /* 正在播放队列回调类型（含当前播放项媒体 ID） */
  virtual void GetNowPlayingList(NowPlayingCallback now_playing_cb) = 0; /* 获取正在播放队列列表，结果通过回调返回 */

  // TODO (apanicke): Use a map with the ID as the key instead of vector
  // in follow up cleanup patches. This allows simplification of the
  // MediaPlayerInfo object
  using MediaListCallback =
          std::function<void(uint16_t curr_player, std::vector<MediaPlayerInfo>)>; /* 媒体播放器列表回调类型 */
  virtual void GetMediaPlayerList(MediaListCallback list_cb) = 0; /* 获取媒体播放器列表，结果通过回调返回 */

  using FolderItemsCallback = std::function<void(std::vector<ListItem>)>; /* 文件夹条目回调类型 */
  virtual void GetFolderItems(uint16_t player_id, std::string media_id,
                              FolderItemsCallback folder_cb) = 0; /* 获取指定播放器与文件夹下的条目列表，结果通过回调返回 */

  using SetBrowsedPlayerCallback = std::function<void(
      bool success, std::string root_id, uint32_t num_items)>; /* 设置浏览播放器回调类型（成功标志/根 ID/条目数） */
  virtual void SetBrowsedPlayer(uint16_t player_id,
                                SetBrowsedPlayerCallback browse_cb) = 0; /* 设置当前浏览的媒体播放器，结果通过回调返回 */

  virtual void PlayItem(uint16_t player_id, bool now_playing,
                        std::string media_id) = 0; /* 播放指定媒体项 */

  virtual void SetActiveDevice(const RawAddress& address) = 0; /* 设置当前活跃媒体设备 */

  virtual void RegisterUpdateCallback(MediaCallbacks* callback) = 0; /* 注册媒体更新回调 */

  virtual void UnregisterUpdateCallback(MediaCallbacks* callback) = 0; /* 注销媒体更新回调 */

  using AppSettingsCallback = std::function<void(uint8_t shuffle_mode, uint8_t repeat_mode)>; /* 应用设置回调类型（随机/重复模式） */
  virtual void AppSettingsSupport(AppSettingsCallback app_cb) = 0; /* 查询应用设置支持情况，结果通过回调返回 */
  virtual void SetAppSettingsMode(uint8_t shuffle_mode, uint8_t repeat_mode) = 0; /* 设置应用随机播放/重复播放模式 */
#ifdef MULTI_A2DP_ENABLED
  virtual void SendKeyEventByAddr(uint8_t key, KeyState state,
                                  const RawAddress& address) {}
  virtual void GetSongInfoByAddr(SongInfoCallback info_cb, const RawAddress& address) {
    info_cb(SongInfo{});
  }
  virtual void GetPlayStatusByAddr(PlayStatusCallback status_cb, const RawAddress& address) {
    PlayStatus status = {0, 0, PlayState::STOPPED};
    status_cb(status);
  }
  virtual void GetNowPlayingListByAddr(NowPlayingCallback now_playing_cb, const RawAddress& address) {
    now_playing_cb(std::string{}, std::vector<SongInfo>{});
  }
  virtual void GetMediaPlayerListByAddr(MediaListCallback list_cb, const RawAddress& address) {
    list_cb(0, std::vector<MediaPlayerInfo>{});
  }
  virtual void GetFolderItemsByAddr(uint16_t player_id, std::string media_id,
                                    FolderItemsCallback folder_cb, const RawAddress& address) {
    folder_cb(std::vector<ListItem>{});
  }
  virtual void SetBrowsedPlayerByAddr(uint16_t player_id,
                                      SetBrowsedPlayerCallback browse_cb, const RawAddress& address) {
    browse_cb(false, std::string{}, 0);
  }
  virtual void PlayItemByAddr(uint16_t player_id, bool now_playing,
                              std::string media_id, const RawAddress& address) {}
  virtual void AppSettingsSupportByAddr(AppSettingsCallback app_cb, const RawAddress& address) {}
  virtual void SetAppSettingsModeByAddr(uint8_t shuffle_mode, uint8_t repeat_mode,
                                        const RawAddress& address) {}
#endif

#ifdef MULTI_A2DP_ENABLED
  // This is an empty function, and its implementation is in avrcp_service.cc
  virtual void SetAddress(const RawAddress& address) {};

  // This is an empty function, and its implementation is in avrcp_service.cc
  virtual MediaInterface* getWrapper() {
    return nullptr;
  };
#endif

  MediaInterface() = default;
  virtual ~MediaInterface() = default;
};

class VolumeInterface { /* 绝对音量接口类，由 AVRCP 服务用于与音量控制层交互 */
public:
  // TODO (apanicke): Investigate the best value type for volume. Right now it
  // is a value from 0-127 because thats what AVRCP uses.
  using VolumeChangedCb = std::function<void(RawAddress addr, int8_t volume)>; /* 音量变化回调类型（设备地址/音量值 0-127） */

  // Indicate that a device has been connected that does not support absolute
  // volume.
  virtual void DeviceConnected(const RawAddress& bdaddr) = 0; /* 通知不支持绝对音量的设备已连接 */

  // Indicate that a device has been connected that does support absolute
  // volume. The callback will be immediately called with the current volume
  // which will be sent to the device.
  virtual void DeviceConnected(const RawAddress& bdaddr,
                               VolumeChangedCb cb) = 0; /* 通知支持绝对音量的设备已连接，并通过回调返回当前音量 */

  // Register the SetVolume method to jni
  virtual void DeviceEmplaceSetVolume(const RawAddress& bdaddr,
                                      VolumeChangedCb cb) = 0; /* 向 JNI 注册设备的设置音量方法 */

  // Indicate that a device has been disconnected from AVRCP. Will unregister
  // any callbacks if absolute volume is supported.
  virtual void DeviceDisconnected(const RawAddress& bdaddr) = 0; /* 通知设备已从 AVRCP 断开，并注销相关回调 */

  virtual void SetVolume(int8_t volume) = 0; /* 设置音量 */

  // for volume store, we must set volume with device
  // vendor's repo contain cbg and hisi, hisi do not include jni
  // implement, so we impl it in .h, use override to trick
  /* 按设备设置音量（默认调用 SetVolume，供厂商覆写） */
  virtual void setVolumeOfDevice(int8_t volume, UNUSED_ATTR const RawAddress& bdaddr) {
    SetVolume(volume);
  }

#ifdef MULTI_A2DP_ENABLED
  // This is an empty function, and its implementation is in avrcp_service.cc
  virtual void SetAddress(const RawAddress& address) {};

  // This is an empty function, and its implementation is in avrcp_service.cc
  virtual VolumeInterface* getWrapper() {
    return nullptr;
  };
#endif

  virtual ~VolumeInterface() = default;
};

class ServiceInterface { /* AVRCP 服务接口类，由 AVRCP 服务实现，供上层（JNI）调用 */
public:
  // mediaInterface can not be null. If volumeInterface is null then Absolute
  // Volume is disabled.
  virtual void Init(MediaInterface* mediaInterface,
                    VolumeInterface* volumeInterface) = 0; /* 初始化服务（volumeInterface 为空则禁用绝对音量） */
  virtual void RegisterBipServer(int psm) = 0; /* 注册 BIP 服务端（指定 PSM） */
  virtual void UnregisterBipServer() = 0; /* 注销 BIP 服务端 */
  virtual bool ConnectDevice(const RawAddress& bdaddr) = 0; /* 连接 AVRCP 设备 */
  virtual bool DisconnectDevice(const RawAddress& bdaddr) = 0; /* 断开 AVRCP 设备 */
  virtual void SetBipClientStatus(const RawAddress& bdaddr, bool connected) = 0; /* 设置设备 BIP 客户端连接状态 */
  virtual bool Cleanup() = 0; /* 清理服务 */

protected:
  virtual ~ServiceInterface() = default;
};

}  // namespace avrcp
}  // namespace bluetooth
