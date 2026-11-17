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
 * 本文件定义 AVRCP(音视频远程控制规范)协议的公共常量、枚举与数据结构，
 * 包括 AVCTP 帧 CType/Opcode、AVRCP 命令PDU、通知事件、媒体浏览(Browsing)
 * 相关的播放器/文件夹/媒体条目等公共定义，供 AVRCP CT/TG 模块共用。
 */

#pragma once

#include <set>

namespace bluetooth {
namespace avrcp {

constexpr uint32_t BLUETOOTH_COMPANY_ID = 0x001958; /* 蓝牙SIG分配的厂商Company ID */

constexpr uint8_t MAX_TRANSACTION_LABEL = 0xF; /* AVCTP事务标签(transaction label)最大值 */

/* AVCTP/AVRCP 帧CType(命令/响应类型) */
enum class CType : uint8_t {
  CONTROL = 0x0, /* 控制命令 */
  STATUS = 0x1, /* 状态查询命令 */
  NOTIFY = 0x3, /* 通知命令 */
  NOT_IMPLEMENTED = 0x8, /* 响应：未实现 */
  ACCEPTED = 0x9, /* 响应：已接受 */
  REJECTED = 0xa, /* 响应：已拒绝 */
  STABLE = 0xc, /* 响应：状态稳定(通用响应) */
  CHANGED = 0xd, /* 响应：状态已变化(通知确认) */
  INTERIM = 0xf, /* 响应：临时响应(注册通知的初始响应) */
};

/* AVCTP 帧Opcode(操作码)，区分不同类型的AVRCP命令分组 */
enum class Opcode : uint8_t {
  VENDOR = 0x00, /* 厂商自定义命令 */
  UNIT_INFO = 0x30, /* 单元信息查询 */
  SUBUNIT_INFO = 0x31, /* 子单元信息查询 */
  PASS_THROUGH = 0x7c, /* 按键直通命令(播放/暂停/上一曲/下一曲等) */
};

// Found in AVRCP_v1.6.1 Section 4.5 Table 4.5
// Searching can be done in the spec by Camel Casing the constant name
/* AVRCP 命令PDU(协议数据单元)类型 */
enum class CommandPdu : uint8_t {
  GET_CAPABILITIES = 0x10, /* 查询对端支持的能力(Company ID/事件) */
  LIST_APPLICATION_SETTING_ATTRIBUTES = 0x11, /* 查询播放器应用设置属性列表 */
  LIST_PLAYER_APP_VALUES = 0x12, /* 查询指定属性的可取值列表 */
  GET_CUR_PLAYER_APP_VALUE = 0x13, /* 查询播放器应用设置当前值 */
  SET_PLAYER_APP_VALUE = 0x14, /* 设置播放器应用属性值 */
  GET_PLAYER_APP_ATTR_TEXT = 0x15, /* 查询属性值的显示文本 */
  GET_PLAYER_APP_VALUE_TEXT = 0x16, /* 查询指定属性值的显示文本 */
  GET_ELEMENT_ATTRIBUTES = 0x20, /* 查询当前播放媒体元素属性(标题/歌手/专辑等) */
  GET_PLAY_STATUS = 0x30, /* 查询播放状态(歌曲长度/当前位置/播放状态) */
  REGISTER_NOTIFICATION = 0x31, /* 注册事件通知 */
  SET_ABSOLUTE_VOLUME = 0x50, /* 设置绝对音量 */
  SET_ADDRESSED_PLAYER = 0x60, /* 设置寻址播放器 */
  PLAY_ITEM = 0x74, /* 播放指定媒体条目 */
  COMMAND_PDU_NA, /* 无效/未定义PDU */
};

/* 播放器应用设置属性ID */
constexpr uint8_t ID_ILLEGAL = 0x00; /* 非法属性ID */
constexpr uint8_t ID_EQUALIZER = 0x01; /* 均衡器开关 */
constexpr uint8_t ID_REPEAT = 0x02; /* 重复播放模式 */
constexpr uint8_t ID_SHUFFLE = 0x03; /* 随机播放模式 */
constexpr uint8_t ID_SCAN = 0x04; /* 扫描播放模式 */

/* 重复播放模式取值 */
constexpr uint8_t REPEAT_OFF = 0x01; /* 关闭重复 */
constexpr uint8_t REPEAT_SINGLE_TRACK = 0x02; /* 单曲循环 */
constexpr uint8_t REPEAT_ALL_TRACKS = 0x03; /* 全部循环 */

/* 随机播放模式取值 */
constexpr uint8_t SHUFFLE_OFF = 0x01; /* 关闭随机 */
constexpr uint8_t SHUFFLE_ALL_TRACKS = 0x02; /* 全部随机 */
constexpr uint8_t SHUFFLE_QUERY = 0x05; /* 查询随机状态 */

/* UTF-8字符集编码标识（低/高字节） */
constexpr uint8_t UTF8L = 0x00;
constexpr uint8_t UTF8H = 0x6a;

constexpr uint8_t SUPPORTED_IDS = 2; /* 本端支持的属性个数(重复/随机) */
constexpr uint8_t REPEAT_MODES = 3; /* 重复模式可选值个数 */
constexpr uint8_t SHUFFLE_MODES = 2; /* 随机模式可选值个数 */

/* 播放器应用设置属性及取值的显示文本 */
constexpr char REPEAT[] = "Repeat";
constexpr char SHUFFLE[] = "Shuffle";
constexpr char OFF[] = "Off";
constexpr char NA[] = "NA";
constexpr char SINGLE_TRACK[] = "Single track";
constexpr char ALL_TRACKS[] = "All tracks";

/* AVRC 命令分包类型 */
enum class PacketType : uint8_t {
  SINGLE = 0x00, /* 单包（本实现不支持分包） */
};

/* 能力查询类型(GET_CAPABILITIES的capability参数) */
enum class Capability : uint8_t {
  COMPANY_ID = 0x02, /* 查询支持的Company ID列表 */
  EVENTS_SUPPORTED = 0x03, /* 查询支持的通知事件列表 */
};

// Found in AVRCP_v1.6.1 Section 28 Appendix H
/* AVRCP 可注册的通知事件ID */
enum class Event : uint8_t {
  PLAYBACK_STATUS_CHANGED = 0x01, /* 播放状态变化(播放/暂停/停止) */
  TRACK_CHANGED = 0x02, /* 当前曲目变化 */
  PLAYBACK_POS_CHANGED = 0x05, /* 播放进度变化 */
  PLAYER_APPLICATION_SETTING_CHANGED = 0x08, /* 播放器应用设置变化(重复/随机等) */
  NOW_PLAYING_CONTENT_CHANGED = 0x09, /* 正在播放列表内容变化 */
  AVAILABLE_PLAYERS_CHANGED = 0x0a, /* 可用播放器集合变化 */
  ADDRESSED_PLAYER_CHANGED = 0x0b, /* 寻址播放器变化 */
  UIDS_CHANGED = 0x0c, /* 媒体条目UID变化(浏览列表失效) */
  VOLUME_CHANGED = 0x0d, /* 音量变化(绝对音量) */
};

/* 媒体元素属性ID(GET_ELEMENT_ATTRIBUTES的属性) */
enum class Attribute : uint32_t {
  TITLE = 0x01, /* 曲目标题 */
  ARTIST_NAME = 0x02, /* 艺术家名称 */
  ALBUM_NAME = 0x03, /* 专辑名称 */
  TRACK_NUMBER = 0x04, /* 曲目编号 */
  TOTAL_NUMBER_OF_TRACKS = 0x05, /* 总曲目数 */
  GENRE = 0x06, /* 流派 */
  PLAYING_TIME = 0x07, /* 曲目时长(ms) */
  DEFAULT_COVER_ART = 0x08, /* 默认封面图片 */
};

/* AVRCP 响应状态码 */
enum class Status : uint8_t {
  INVALID_COMMAND = 0x00, /* 无效命令 */
  INVALID_PARAMETER = 0x01, /* 无效参数 */
  PARAMETER_CONTENT_ERROR = 0x02, /* 参数内容错误 */
  INTERNAL_ERROR = 0x03, /* 内部错误 */
  NO_ERROR = 0x04, /* 无错误，操作成功 */
  UIDS_CHANGED = 0x05, /* UID已变化 */
  RESERVED = 0x06, /* 保留 */
  INVALID_DIRECTION = 0x07, /* 无效的目录浏览方向 */
  NOT_A_DIRECTORY = 0x08, /* 不是目录 */
  DOES_NOT_EXIST = 0x09, /* 条目不存在 */
  INVALID_SCOPE = 0x0a, /* 无效的浏览范围 */
  RANGE_OUT_OF_BOUNDS = 0xb, /* 请求范围越界 */
  FOLDER_ITEM_NOT_PLAYABLE = 0x0c, /* 文件夹条目不可播放 */
  MEDIA_IN_USE = 0x0d, /* 媒体被占用 */
  NOW_PLAYING_LIST_FULL = 0x0e, /* 正在播放列表已满 */
  SEARCH_NOT_SUPPORTED = 0x0f, /* 不支持搜索 */
  SEARCH_IN_PROGRESS = 0x10, /* 搜索进行中 */
  INVALID_PLAYER_ID = 0x11, /* 无效播放器ID */
  PLAYER_NOT_BROWSABLE = 0x12, /* 播放器不支持浏览 */
  PLAYER_NOT_ADDRESSED = 0x13, /* 播放器未被寻址 */
  NO_VALID_SEARCH_RESULTS = 0x14, /* 无有效搜索结果 */
  NO_AVAILABLE_PLAYERS = 0x15, /* 无可用播放器 */
  ADDRESSED_PLAYER_CHANGED = 0x16, /* 寻址播放器已变化 */
};

/* AVRCP 浏览(Browsing)通道PDU类型 */
enum class BrowsePdu : uint8_t {
  SET_BROWSED_PLAYER = 0x70, /* 设置被浏览的播放器 */
  GET_FOLDER_ITEMS = 0x71, /* 获取指定范围内的媒体条目列表 */
  CHANGE_PATH = 0x72, /* 目录路径切换(进入/返回上级) */
  GET_ITEM_ATTRIBUTES = 0x73, /* 获取指定媒体条目的属性 */
  GET_TOTAL_NUMBER_OF_ITEMS = 0x75, /* 获取条目总数 */
  GENERAL_REJECT = 0xa0, /* 通用拒绝响应 */
};

/* 媒体浏览范围(GET_FOLDER_ITEMS的作用域) */
enum class Scope : uint8_t {
  MEDIA_PLAYER_LIST = 0x00, /* 媒体播放器列表 */
  VFS = 0x01, /* 虚拟文件系统(目录树) */
  SEARCH = 0x02, /* 搜索结果列表 */
  NOW_PLAYING = 0x03, /* 正在播放列表 */
};

/* 目录浏览方向(CHANGE_PATH参数) */
enum class Direction : uint8_t {
  UP = 0x00, /* 返回上级目录 */
  DOWN = 0x01, /* 进入子目录 */
};

/* 按键状态(PASS_THROUGH命令的按键状态) */
enum class KeyState : uint8_t {
  PUSHED = 0x00, /* 按下 */
  RELEASED = 0x01, /* 释放 */
};

/* 媒体元素属性条目：属性类型 + UTF-8字符串值，用于GET_ELEMENT_ATTRIBUTES等 */
class AttributeEntry {
 public:
  AttributeEntry(const Attribute& attribute, const std::string& value)
      : attribute_(attribute), value_(value) {}

  AttributeEntry(const Attribute& attribute) : attribute_(attribute) {} /* 构造仅含属性ID(不含值)的条目 */

  AttributeEntry(const AttributeEntry&) = default;

  Attribute attribute() const { return attribute_; } /* 获取属性ID */

  std::string value() const { return value_; } /* 获取属性值字符串 */

  /* 条目头固定字节数(不含属性值本身) */
  static constexpr size_t kHeaderSize() {
    size_t ret = 0;
    ret += 4;  // Size of attribute field - 属性ID字段4字节
    ret += 2;  // Size of length field - 属性值长度字段2字节
    ret += 2;  // Size of character encoding field - 字符集编码字段2字节
    return ret;
  }

  size_t size() const { return kHeaderSize() + value_.size(); } /* 条目总字节数(头+属性值) */

  void resize(size_t new_size) { /* 按总长度截断属性值，防止超过MTU/最大字段长度 */
    new_size = new_size < kHeaderSize() ? 0 : new_size - kHeaderSize();
    if (value_.size() > new_size) {
      value_.resize(new_size);
    }
  }

  bool empty() { return value_.empty(); } /* 属性值是否为空 */

  bool operator<(const AttributeEntry& rhs) const
  {
    return attribute_ < rhs.attribute_; /* 按属性ID比较，用于set排序 */
  }

 private:
  Attribute attribute_; /* 属性ID */
  std::string value_; /* 属性值(UTF-8字符串) */
};

constexpr size_t MAX_FIELD_LEN = 100; /* 名称/属性字段的最大长度(字节)，防止超过浏览MTU */

/* 媒体播放器条目(GET_FOLDER_ITEMS返回的播放器列表项) */
struct MediaPlayerItem {
  uint16_t id_; /* 播放器ID */
  std::string name_; /* 播放器名称 */
  bool browsable_; /* 是否支持浏览 */

  /* 构造播放器条目，名称超长时截断至MAX_FIELD_LEN，防止超过浏览MTU */
  MediaPlayerItem(uint16_t id, const std::string& name, bool browsable)
      : id_(id), name_(name), browsable_(browsable) {
    if (name_.size() > MAX_FIELD_LEN) {
      name_.resize(MAX_FIELD_LEN);
    }
  }

  /* 默认拷贝构造 */
  MediaPlayerItem(const MediaPlayerItem&) = default;

  /* 条目头固定字节数(不含播放器名称本身) */
  static constexpr size_t kHeaderSize() {
    size_t ret = 0;
    ret += 1;   // Media Player Type - 条目类型1字节
    ret += 2;   // Item Length - 条目总长度2字节
    ret += 2;   // Player Id - 播放器ID 2字节
    ret += 1;   // Player Type - 播放器类型1字节
    ret += 4;   // Player Subtype - 播放器子类型4字节
    ret += 1;   // Play Status - 播放状态1字节
    ret += 16;  // Features - 特性位掩码16字节
    ret += 2;   // UTF-8 character set - 字符集编码2字节
    ret += 2;   // Name Length - 名称长度2字节
    return ret;
  }

  /* 条目总字节数(头+播放器名称) */
  size_t size() const { return kHeaderSize() + name_.size(); }
};

/* 文件夹条目(GET_FOLDER_ITEMS返回的目录列表项) */
struct FolderItem {
  uint64_t uid_; /* 文件夹UID */
  uint8_t folder_type_; /* 文件夹类型(混合/文件夹/播放列表等) */
  bool is_playable_; /* 是否可播放 */
  std::string name_; /* 文件夹名称 */

  /* 构造文件夹条目
   * uid-文件夹UID folder_type-文件夹类型 is_playable-是否可播放 name-文件夹名称 */
  FolderItem(uint64_t uid,
             uint8_t folder_type,
             bool is_playable,
             const std::string& name)
      : uid_(uid),                /* 文件夹UID */
        folder_type_(folder_type), /* 文件夹类型 */
        is_playable_(is_playable), /* 是否可播放 */
        name_(name) {              /* 文件夹名称 */
    /* 名称超长时截断至MAX_FIELD_LEN，防止单个条目超过浏览通道MTU */
    if (name_.size() > MAX_FIELD_LEN) {
      name_.resize(MAX_FIELD_LEN);
    }
  }

  /* 默认拷贝构造 */
  FolderItem(const FolderItem&) = default;

  /* 条目头固定字节数(不含文件夹名称本身) */
  static constexpr size_t kHeaderSize() {
    size_t ret = 0;
    ret += 1;  // Folder Item Type - 条目类型1字节
    ret += 2;  // Item Length - 条目总长度2字节
    ret += 8;  // Folder UID - 文件夹UID 8字节
    ret += 1;  // Folder Type - 文件夹类型1字节
    ret += 1;  // Is Playable byte - 可播放标志1字节
    ret += 2;  // UTF-8 Character Set - 字符集编码2字节
    ret += 2;  // Name Length - 名称长度2字节
    return ret;
  }

  /* 条目总字节数(头+文件夹名称) */
  size_t size() const { return kHeaderSize() + name_.size(); }
};

// NOTE: We never use media type field because we only support audio types
/* 媒体元素条目(GET_FOLDER_ITEMS返回的歌曲/媒体文件列表项) */
struct MediaElementItem {
  uint64_t uid_ = 0; /* 媒体条目UID */
  std::string name_; /* 媒体名称 */
  std::set<AttributeEntry> attributes_; /* 媒体属性集合(标题/歌手/专辑等) */

  // Truncate the name and attribute fields so that we don't have a single item
  // that can exceed the Browsing MTU
  /* 构造媒体元素条目：名称及各属性值超长时截断至MAX_FIELD_LEN，
   * 防止单个条目超过浏览MTU */
  MediaElementItem(uint64_t uid, const std::string& name,
                   std::set<AttributeEntry> attributes)
      : uid_(uid), name_(name) {
    if (name_.size() > MAX_FIELD_LEN) {
      name_.resize(MAX_FIELD_LEN);
    }

    /* 逐个截断属性条目后插入集合 */
    for (AttributeEntry val : attributes) {
      val.resize(MAX_FIELD_LEN);
      attributes_.insert(val);
    }
  }

  /* 默认拷贝构造 */
  MediaElementItem(const MediaElementItem&) = default;

  /* 计算条目总字节数(头+名称+全部属性条目) */
  size_t size() const {
    size_t ret = 0;
    ret += 1;  // Media Element Item Type - 条目类型1字节
    ret += 2;  // Item Length - 条目总长度2字节
    ret += 8;  // Item UID - 媒体条目UID 8字节
    ret += 1;  // Media Type - 媒体类型1字节(仅支持audio)
    ret += 2;  // UTF-8 Character Set - 字符集编码2字节
    ret += 2;  // Name Length - 名称长度2字节
    ret += name_.size(); // 媒体名称字节数
    ret += 1;  // Number of Attributes - 属性个数1字节
    /* 累加各属性条目字节数 */
    for (const auto& entry : attributes_) {
      ret += entry.size();
    }

    return ret;
  }
};

/* 媒体列表条目：浏览通道列表项的联合体封装(播放器/文件夹/歌曲三种类型之一) */
struct MediaListItem {
  enum : uint8_t { PLAYER = 0x01, FOLDER = 0x02, SONG = 0x03 } type_; /* 条目类型: 播放器/文件夹/歌曲 */

  union {
    MediaPlayerItem player_; /* 播放器条目(type_==PLAYER时有效) */
    FolderItem folder_; /* 文件夹条目(type_==FOLDER时有效) */
    MediaElementItem song_; /* 歌曲条目(type_==SONG时有效) */
  };

  /* 由播放器条目构造(类型置为PLAYER) */
  MediaListItem(MediaPlayerItem item) : type_(PLAYER), player_(item) {}

  /* 由文件夹条目构造(类型置为FOLDER) */
  MediaListItem(FolderItem item) : type_(FOLDER), folder_(item) {}

  /* 由歌曲(媒体元素)条目构造(类型置为SONG) */
  MediaListItem(MediaElementItem item) : type_(SONG), song_(item) {}

  /* 拷贝构造：union成员含非平凡类型(std::string等)，编译器无法隐式生成，
   * 须先复制type_，再按type_在对应成员原始内存上placement new完成拷贝 */
  MediaListItem(const MediaListItem& item) {
    /* 先拷贝类型标记，决定union中哪个成员有效 */
    type_ = item.type_;
    /* 仅对有效成员做placement new拷贝，其余成员不初始化 */
    switch (item.type_) {
      case PLAYER:
        new (&player_) MediaPlayerItem(item.player_); /* 拷贝播放器条目 */
        return;
      case FOLDER:
        new (&folder_) FolderItem(item.folder_); /* 拷贝文件夹条目 */
        return;
      case SONG:
        new (&song_) MediaElementItem(item.song_); /* 拷贝歌曲条目 */
        return;
    }
  }

  /* 析构：union成员含非平凡类型，编译器无法隐式生成，
   * 须按type_仅对有效成员显式调用析构函数，避免误析构无效成员 */
  ~MediaListItem() {
    switch (type_) {
      case PLAYER:
        player_.~MediaPlayerItem(); /* 析构播放器条目 */
        return;
      case FOLDER:
        folder_.~FolderItem(); /* 析构文件夹条目 */
        return;
      case SONG:
        song_.~MediaElementItem(); /* 析构歌曲条目 */
        return;
    }
  }

  /* 计算条目总字节数：union中仅有type_对应的一种条目有效，
   * 按type_转发到对应条目的size()（头部+名称/属性等可变部分） */
  size_t size() const {
    switch (type_) {
      case PLAYER:
        return player_.size(); /* 播放器条目字节数 */
      case FOLDER:
        return folder_.size(); /* 文件夹条目字节数 */
      case SONG:
        return song_.size(); /* 歌曲条目字节数 */
    }
  }
};

constexpr size_t AVCT_HDR_LEN = 3; /* AVCTP帧头长度(字节) */

}  // namespace avrcp
}  // namespace bluetooth
