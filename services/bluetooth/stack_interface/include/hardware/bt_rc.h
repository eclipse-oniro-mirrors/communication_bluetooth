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
 * 本文件定义蓝牙 AVRCP(音视频远程控制规范)的 CT(控制器)/TG(目标)接口，
 * 包含媒体信息查询、播放控制、播放器应用设置、媒体浏览、绝对音量等
 * 命令/响应与回调接口定义。
 */

#ifndef VENDOR_INCLUDE_BT_RC_H
#define VENDOR_INCLUDE_BT_RC_H

#include "hardware/bluetooth.h"
#include "types/raw_address.h"

#ifndef STACK
#define STACK
#endif

extern "C" { //__BEGIN_DECLS

/* Change this macro to use multiple RC */
#define BT_RC_NUM_APP 6 /* 支持的最大远端遥控应用实例数 */

/* Macros */
#define BTRC_MAX_ATTR_STR_LEN (1 << 16) /* 属性字符串最大长度 */
#define BTRC_UID_SIZE 8 /* 媒体条目UID长度(字节) */
#define BTRC_MAX_APP_SETTINGS 8 /* 播放器应用设置最大属性个数 */
#define BTRC_MAX_FOLDER_DEPTH 4 /* 浏览目录最大层级深度 */
#define BTRC_MAX_APP_ATTR_SIZE 16 /* 播放器应用属性最大个数 */
#define BTRC_MAX_ELEM_ATTR_SIZE 8 /* 媒体元素属性最大个数 */
#define BTRC_FEATURE_BIT_MASK_SIZE 16 /* 播放器特性位掩码长度(字节) */

/* Macros for valid scopes in get_folder_items */
/* get_folder_items(AT+AVRCP浏览命令)可查询的媒体浏览范围(scope)取值宏 */
#define BTRC_SCOPE_PLAYER_LIST 0x00 /* Media Player List - 媒体播放器列表 */
#define BTRC_SCOPE_FILE_SYSTEM 0x01 /* Virtual File System - 虚拟文件系统(逐级浏览文件夹) */
#define BTRC_SCOPE_SEARCH 0x02      /* Search - 搜索结果列表 */
#define BTRC_SCOPE_NOW_PLAYING 0x03 /* Now Playing - 正在播放列表 */

/* Macros for supported character encoding */
/* AVRCP浏览通道字符串使用的字符集编码ID(与IANA注册值一致) */
#define BTRC_CHARSET_ID_UTF8 0x006A /* UTF-8字符集编码ID */

/* Macros for item types */
#define BTRC_ITEM_PLAYER 0x01 /* Media Player */
#define BTRC_ITEM_FOLDER 0x02 /* Folder */
#define BTRC_ITEM_MEDIA 0x03  /* Media File */

/* Macros for media attribute IDs */
#define BTRC_MEDIA_ATTR_ID_INVALID (-1) /* 无效属性ID */
#define BTRC_MEDIA_ATTR_ID_TITLE 0x00000001 /* 曲目标题 */
#define BTRC_MEDIA_ATTR_ID_ARTIST 0x00000002 /* 艺术家 */
#define BTRC_MEDIA_ATTR_ID_ALBUM 0x00000003 /* 专辑 */
#define BTRC_MEDIA_ATTR_ID_TRACK_NUM 0x00000004 /* 曲目编号 */
#define BTRC_MEDIA_ATTR_ID_NUM_TRACKS 0x00000005 /* 总曲目数 */
#define BTRC_MEDIA_ATTR_ID_GENRE 0x00000006 /* 流派 */
#define BTRC_MEDIA_ATTR_ID_PLAYING_TIME 0x00000007 /* in miliseconds */ /* 曲目时长(ms) */
#define BTRC_MEDIA_ATTR_ID_COVER_ARTWORK_HANDLE 0x00000008 /* 封面图片句柄 */

/* Macros for folder types */
#define BTRC_FOLDER_TYPE_MIXED 0x00 /* 混合类型文件夹 */
#define BTRC_FOLDER_TYPE_TITLES 0x01 /* 按标题分类 */
#define BTRC_FOLDER_TYPE_ALBUMS 0x02 /* 按专辑分类 */
#define BTRC_FOLDER_TYPE_ARTISTS 0x03 /* 按艺术家分类 */
#define BTRC_FOLDER_TYPE_GENRES 0x04 /* 按流派分类 */
#define BTRC_FOLDER_TYPE_PLAYLISTS 0x05 /* 播放列表 */
#define BTRC_FOLDER_TYPE_YEARS 0x06 /* 按年份分类 */

/* Macros for media types */
#define BTRC_MEDIA_TYPE_AUDIO 0x00 /* audio */
#define BTRC_MEDIA_TYPE_VIDEO 0x01 /* video */

/* Macros for num attributes */
#define BTRC_NUM_ATTR_NONE 0xFF /* No attributes required */ /* 不需要任何属性 */
#define BTRC_NUM_ATTR_ALL 0X00  /* All attributes required */ /* 需要全部属性 */

#define BTRC_HANDLE_NONE 0xFF /* 无效句柄 */

typedef uint8_t btrc_uid_t[BTRC_UID_SIZE]; /* 媒体条目UID(8字节) */

/* AVRCP 连接状态 */
typedef enum {
  BTRC_CONNECTION_STATE_DISCONNECTED = 0, /* 已断开 */
  BTRC_CONNECTION_STATE_CONNECTED /* 已连接 */
} btrc_connection_state_t;

/* 远端设备AVRCP特性位掩码(可用位运算组合)，反映其支持的AVRCP版本及能力 */
typedef enum {
  BTRC_FEAT_NONE = 0x00,            /* 不支持以上任何特性(仅AVRCP 1.0基本控制) */
  BTRC_FEAT_METADATA = 0x01,        /* 支持元数据传输(AVRCP 1.3) */
  BTRC_FEAT_ABSOLUTE_VOLUME = 0x02, /* 支持TG角色及绝对音量同步(AVRCP 1.4) */
  BTRC_FEAT_BROWSE = 0x04, /* 支持媒体浏览(AVRCP 1.4及以上) */
  BTRC_FEAT_COVER_ARTWORK = 0x8,    /* 支持封面图片(AVRCP 1.6及以上) */
} btrc_remote_features_t;

/* 播放器播放状态 */
typedef enum {
  BTRC_PLAYSTATE_STOPPED = 0x00,  /* 已停止 */
  BTRC_PLAYSTATE_PLAYING = 0x01,  /* 正在播放 */
  BTRC_PLAYSTATE_PAUSED = 0x02,   /* 已暂停 */
  BTRC_PLAYSTATE_FWD_SEEK = 0x03, /* 快进中 */
  BTRC_PLAYSTATE_REV_SEEK = 0x04, /* 快退中 */
  BTRC_PLAYSTATE_ERROR = 0xFF,    /* 播放状态错误 */
} btrc_play_status_t;

/* AVRCP 可注册的通知事件ID */
typedef enum {
  BTRC_EVT_PLAY_STATUS_CHANGED = 0x01, /* 播放状态变化 */
  BTRC_EVT_TRACK_CHANGE = 0x02, /* 当前曲目变化 */
  BTRC_EVT_TRACK_REACHED_END = 0x03, /* 曲目播放到末尾 */
  BTRC_EVT_TRACK_REACHED_START = 0x04, /* 曲目播放到开头 */
  BTRC_EVT_PLAY_POS_CHANGED = 0x05, /* 播放进度变化 */
  BTRC_EVT_APP_SETTINGS_CHANGED = 0x08, /* 播放器应用设置变化 */
  BTRC_EVT_NOW_PLAYING_CONTENT_CHANGED = 0x09, /* 正在播放列表内容变化 */
  BTRC_EVT_AVAL_PLAYER_CHANGE = 0x0a, /* 可用播放器集合变化 */
  BTRC_EVT_ADDR_PLAYER_CHANGE = 0x0b, /* 寻址播放器变化 */
  BTRC_EVT_UIDS_CHANGED = 0x0c, /* 媒体条目UID变化 */
  BTRC_EVT_VOL_CHANGED = 0x0d, /* 音量变化 */
} btrc_event_id_t;

/* 通知响应类型 */
typedef enum {
  BTRC_NOTIFICATION_TYPE_INTERIM = 0, /* 临时响应(注册通知时的初始响应) */
  BTRC_NOTIFICATION_TYPE_CHANGED = 1, /* 状态变化响应(事件触发后上报) */
} btrc_notification_type_t;

/* 播放器应用设置属性ID */
typedef enum {
  BTRC_PLAYER_ATTR_EQUALIZER = 0x01, /* 均衡器开关 */
  BTRC_PLAYER_ATTR_REPEAT = 0x02, /* 重复播放模式 */
  BTRC_PLAYER_ATTR_SHUFFLE = 0x03, /* 随机播放模式 */
  BTRC_PLAYER_ATTR_SCAN = 0x04, /* 扫描播放模式 */
} btrc_player_attr_t;

/* 媒体元素属性ID */
typedef enum {
  BTRC_MEDIA_ATTR_TITLE = 0x01, /* 曲目标题 */
  BTRC_MEDIA_ATTR_ARTIST = 0x02, /* 艺术家 */
  BTRC_MEDIA_ATTR_ALBUM = 0x03, /* 专辑 */
  BTRC_MEDIA_ATTR_TRACK_NUM = 0x04, /* 曲目编号 */
  BTRC_MEDIA_ATTR_NUM_TRACKS = 0x05, /* 总曲目数 */
  BTRC_MEDIA_ATTR_GENRE = 0x06, /* 流派 */
  BTRC_MEDIA_ATTR_PLAYING_TIME = 0x07, /* 曲目时长(ms) */
} btrc_media_attr_t;

/* 重复播放模式取值 */
typedef enum {
  BTRC_PLAYER_VAL_OFF_REPEAT = 0x01, /* 关闭重复 */
  BTRC_PLAYER_VAL_SINGLE_REPEAT = 0x02, /* 单曲循环 */
  BTRC_PLAYER_VAL_ALL_REPEAT = 0x03, /* 全部循环 */
  BTRC_PLAYER_VAL_GROUP_REPEAT = 0x04 /* 分组循环 */
} btrc_player_repeat_val_t;

/* 随机播放模式取值 */
typedef enum {
  BTRC_PLAYER_VAL_OFF_SHUFFLE = 0x01, /* 关闭随机 */
  BTRC_PLAYER_VAL_ALL_SHUFFLE = 0x02, /* 全部随机 */
  BTRC_PLAYER_VAL_GROUP_SHUFFLE = 0x03 /* 分组随机 */
} btrc_player_shuffle_val_t;

/* AVRCP TG 响应状态码(浏览/控制命令处理结果，取值见AVRCP 1.6规范) */
typedef enum {
  BTRC_STS_BAD_CMD = 0x00,       /* 无效命令 */
  BTRC_STS_BAD_PARAM = 0x01,     /* 无效参数 */
  BTRC_STS_NOT_FOUND = 0x02,     /* 指定参数错误或未找到 */
  BTRC_STS_INTERNAL_ERR = 0x03,  /* TG内部错误 */
  BTRC_STS_NO_ERROR = 0x04,      /* 操作成功 */
  BTRC_STS_UID_CHANGED = 0x05,   /* 媒体条目UID已变化，需刷新浏览 */
  BTRC_STS_RESERVED = 0x06,      /* 保留值 */
  BTRC_STS_INV_DIRN = 0x07,      /* 无效方向(快进/快退方向错误) */
  BTRC_STS_INV_DIRECTORY = 0x08, /* 无效文件夹 */
  BTRC_STS_INV_ITEM = 0x09,      /* 无效媒体条目 */
  BTRC_STS_INV_SCOPE = 0x0a,     /* 无效浏览范围(scope) */
  BTRC_STS_INV_RANGE = 0x0b,     /* 无效条目范围(start/end item越界) */
  BTRC_STS_DIRECTORY = 0x0c,     /* 指定UID是文件夹(不可作为媒体条目操作) */
  BTRC_STS_MEDIA_IN_USE = 0x0d,  /* 媒体正被占用 */
  BTRC_STS_PLAY_LIST_FULL = 0x0e, /* 正在播放列表已满 */
  BTRC_STS_SRCH_NOT_SPRTD = 0x0f, /* 不支持搜索操作 */
  BTRC_STS_SRCH_IN_PROG = 0x10,   /* 搜索正在进行中 */
  BTRC_STS_INV_PLAYER = 0x11,     /* 无效播放器 */
  BTRC_STS_PLAY_NOT_BROW = 0x12,  /* 播放器不可浏览 */
  BTRC_STS_PLAY_NOT_ADDR = 0x13,  /* 播放器未被寻址 */
  BTRC_STS_INV_RESULTS = 0x14,    /* 无效搜索结果 */
  BTRC_STS_NO_AVBL_PLAY = 0x15,   /* 没有可用播放器 */
  BTRC_STS_ADDR_PLAY_CHGD = 0x16, /* 寻址播放器已变化 */
} btrc_status_t;

/* 寻址播放器变化事件参数 */
typedef struct {
  uint16_t player_id; /* 新寻址播放器ID */
  uint16_t uid_counter; /* UID计数器 */
} btrc_addr_player_changed_t;

/* 播放器应用设置(属性ID与对应值) */
typedef struct {
  uint8_t num_attr; /* 属性个数 */
  uint8_t attr_ids[BTRC_MAX_APP_SETTINGS]; /* 属性ID数组 */
  uint8_t attr_values[BTRC_MAX_APP_SETTINGS]; /* 属性值数组 */
} btrc_player_settings_t;

/* 播放器应用扩展属性的取值文本 */
typedef struct {
  uint8_t val; /* 属性值ID */
  uint16_t charset_id; /* 字符集编码ID */
  uint16_t str_len; /* 字符串长度 */
  uint8_t* p_str; /* 字符串内容指针 */
} btrc_player_app_ext_attr_val_t;

/* 播放器应用扩展属性(含属性文本及各取值文本) */
typedef struct {
  uint8_t attr_id; /* 属性ID */
  uint16_t charset_id; /* 字符集编码ID */
  uint16_t str_len; /* 属性名长度 */
  uint8_t* p_str; /* 属性名字符串 */
  uint8_t num_val; /* 取值个数 */
  btrc_player_app_ext_attr_val_t ext_attr_val[BTRC_MAX_APP_ATTR_SIZE]; /* 各取值及文本 */
} btrc_player_app_ext_attr_t;

/* 播放器应用属性(属性ID及支持的取值列表) */
typedef struct {
  uint8_t attr_id; /* 属性ID */
  uint8_t num_val; /* 取值个数 */
  uint8_t attr_val[BTRC_MAX_APP_ATTR_SIZE]; /* 取值列表 */
} btrc_player_app_attr_t;

/* GET_FOLDER_ITEMS 请求参数 */
typedef struct {
  uint32_t start_item; /* 起始条目索引 */
  uint32_t end_item; /* 结束条目索引 */
  uint32_t size; /* 条目总数 */
  uint32_t attrs[BTRC_MAX_ELEM_ATTR_SIZE]; /* 请求的属性ID列表 */
  uint8_t attr_count; /* 属性个数 */
} btrc_getfolderitem_t;

/* UID变化事件参数 */
typedef struct {
  uint16_t type; /* 事件类型 */
  uint16_t uid_counter; /* 新UID计数器 */
} btrc_uids_changed_t;

/* 正在播放内容变化事件参数 */
typedef struct { uint16_t type; } btrc_now_playing_changed_t;

/* 注册通知事件携带的参数(依事件类型取其一) */
typedef union {
  btrc_play_status_t play_status; /* 播放状态 */
  btrc_uid_t track; /* queue position in NowPlaying */ /* 当前曲目UID */
  uint32_t song_pos; /* 播放进度(ms) */
  uint16_t uid_counter; /* UID计数器 */
  btrc_player_settings_t player_setting; /* 播放器应用设置 */
  btrc_addr_player_changed_t addr_player_changed; /* 寻址播放器变化参数 */
  btrc_uids_changed_t uids_changed; /* UID变化参数 */
  btrc_now_playing_changed_t now_playing_changed; /* 正在播放内容变化参数 */
} btrc_register_notification_t;

/* 播放器应用属性/取值的显示文本 */
typedef struct {
  uint8_t id; /* can be attr_id or value_id */ /* 属性ID或取值ID */
  uint8_t text[BTRC_MAX_ATTR_STR_LEN]; /* 显示文本 */
} btrc_player_setting_text_t;

/* 媒体元素属性值 */
typedef struct {
  uint32_t attr_id; /* 属性ID */
  uint8_t text[BTRC_MAX_ATTR_STR_LEN]; /* 属性值文本 */
} btrc_element_attr_val_t;

/* 浏览列表中的播放器条目 */
typedef struct {
  uint16_t player_id; /* 播放器ID */
  uint8_t major_type; /* 播放器主类型 */
  uint32_t sub_type; /* 播放器子类型 */
  uint8_t play_status; /* 播放状态 */
  uint8_t features[BTRC_FEATURE_BIT_MASK_SIZE]; /* 播放器特性位掩码 */
  uint16_t charset_id; /* 字符集编码ID */
  uint8_t name[BTRC_MAX_ATTR_STR_LEN]; /* 播放器名称 */
} btrc_item_player_t;

/* 浏览列表中的文件夹条目 */
typedef struct {
  uint8_t uid[BTRC_UID_SIZE]; /* 文件夹UID */
  uint8_t type; /* 文件夹类型 */
  uint8_t playable; /* 是否可播放 */
  uint16_t charset_id; /* 字符集编码ID */
  uint8_t name[BTRC_MAX_ATTR_STR_LEN]; /* 文件夹名称 */
} btrc_item_folder_t;

/* 浏览列表中的媒体条目 */
typedef struct {
  uint8_t uid[BTRC_UID_SIZE]; /* 媒体条目UID */
  uint8_t type; /* 媒体类型(音频/视频) */
  uint16_t charset_id; /* 字符集编码ID */
  uint8_t name[BTRC_MAX_ATTR_STR_LEN]; /* 媒体名称 */
  int num_attrs; /* 属性个数 */
  btrc_element_attr_val_t* p_attrs; /* 属性值数组指针 */
} btrc_item_media_t;

/* 浏览列表条目(播放器/文件夹/媒体三种类型之一的联合体) */
typedef struct {
  uint8_t item_type; /* 条目类型(BTRC_ITEM_PLAYER/FOLDER/MEDIA) */
  union {
    btrc_item_player_t player; /* 播放器条目 */
    btrc_item_folder_t folder; /* 文件夹条目 */
    btrc_item_media_t media; /* 媒体条目 */
  };
} btrc_folder_items_t;

/* 浏览目录的文件夹名称(路径栈元素) */
typedef struct {
  uint16_t str_len; /* 名称长度 */
  uint8_t p_str[BTRC_MAX_ATTR_STR_LEN]; /* 名称字符串 */
} btrc_br_folder_name_t;

/** Callback for the controller's supported feautres */
typedef void (*btrc_remote_features_callback)(const STACK::RawAddress& bd_addr,
                                              btrc_remote_features_t features); /* 上报对端CT支持的AVRCP特性 */

/** Callback for play status request */
typedef void (*btrc_get_play_status_callback)(const STACK::RawAddress& bd_addr); /* CT请求查询播放状态 */

/** Callback for list player application attributes (Shuffle, Repeat,...) */
typedef void (*btrc_list_player_app_attr_callback)(const STACK::RawAddress& bd_addr); /* CT请求查询播放器应用属性列表 */

/** Callback for list player application attributes (Shuffle, Repeat,...) */
typedef void (*btrc_list_player_app_values_callback)(btrc_player_attr_t attr_id,
                                                     const STACK::RawAddress& bd_addr); /* CT请求查询指定属性的可取值列表 */

/** Callback for getting the current player application settings value
**  num_attr: specifies the number of attribute ids contained in p_attrs
*/
typedef void (*btrc_get_player_app_value_callback)(uint8_t num_attr,
                                                   btrc_player_attr_t* p_attrs,
                                                   const STACK::RawAddress& bd_addr); /* CT请求查询播放器应用设置当前值 */

/** Callback for getting the player application settings attributes' text
**  num_attr: specifies the number of attribute ids contained in p_attrs
*/
typedef void (*btrc_get_player_app_attrs_text_callback)(
    uint8_t num_attr, btrc_player_attr_t* p_attrs, const STACK::RawAddress& bd_addr); /* CT请求查询应用属性显示文本 */

/** Callback for getting the player application settings values' text
**  num_attr: specifies the number of value ids contained in p_vals
*/
typedef void (*btrc_get_player_app_values_text_callback)(
    uint8_t attr_id, uint8_t num_val, uint8_t* p_vals,
    const STACK::RawAddress& bd_addr); /* CT请求查询应用属性取值显示文本 */

/** Callback for setting the player application settings values */
typedef void (*btrc_set_player_app_value_callback)(
    btrc_player_settings_t* p_vals, const STACK::RawAddress& bd_addr); /* CT请求设置播放器应用属性值 */

/** Callback to fetch the get element attributes of the current song
**  num_attr: specifies the number of attributes requested in p_attrs
*/
typedef void (*btrc_get_element_attr_callback)(uint8_t num_attr,
                                               btrc_media_attr_t* p_attrs,
                                               const STACK::RawAddress& bd_addr); /* CT请求查询当前曲目媒体元素属性 */

/** Callback for register notification (Play state change/track change/...)
**  param: Is only valid if event_id is BTRC_EVT_PLAY_POS_CHANGED
*/
typedef void (*btrc_register_notification_callback)(btrc_event_id_t event_id,
                                                    uint32_t param,
                                                    const STACK::RawAddress& bd_addr); /* CT注册事件通知 */

/* AVRCP 1.4 Enhancements */
/** Callback for volume change on CT
**  volume: Current volume setting on the CT (0-127)
*/
typedef void (*btrc_volume_change_callback)(uint8_t volume, uint8_t ctype,
                                            const STACK::RawAddress& bd_addr); /* CT绝对音量变化通知(0-127) */

/** Callback for passthrough commands */
typedef void (*btrc_passthrough_cmd_callback)(int id, int key_state,
                                              const STACK::RawAddress& bd_addr); /* CT按键直通命令(播放/暂停等) */

/** Callback for set addressed player response on TG **/
typedef void (*btrc_set_addressed_player_callback)(uint16_t player_id,
                                                   const STACK::RawAddress& bd_addr); /* CT请求设置寻址播放器 */

/** Callback for set browsed player response on TG **/
typedef void (*btrc_set_browsed_player_callback)(uint16_t player_id,
                                                 const STACK::RawAddress& bd_addr); /* CT请求设置被浏览播放器 */

/** Callback for get folder items on TG
**  num_attr: specifies the number of attributes requested in p_attr_ids
*/
typedef void (*btrc_get_folder_items_callback)(
    uint8_t scope, uint32_t start_item, uint32_t end_item, uint8_t num_attr,
    uint32_t* p_attr_ids, const STACK::RawAddress& bd_addr); /* CT请求获取浏览列表条目 */

/** Callback for changing browsed path on TG **/
typedef void (*btrc_change_path_callback)(uint8_t direction,
                                          uint8_t* folder_uid,
                                          const STACK::RawAddress& bd_addr); /* CT请求切换浏览目录路径 */

/**
 * Callback invoked when the CT requests the attributes of a media
 * item (GetItemAttributes) in the given browsing scope.
 *
 * @param scope browsing scope of the media item, one of the
 *              BTRC_SCOPE_* values
 * @param uid UID of the target media item
 * @param uid_counter UID counter of the media item
 * @param num_attr number of attributes requested in p_attrs
 * @param p_attrs array of the requested attribute IDs
 * @param bd_addr remote CT device address
 */
/* CT请求获取指定媒体条目属性(GetItemAttributes)：按scope+uid定位条目，
 * 返回p_attrs中请求的num_attr个属性值 */
typedef void (*btrc_get_item_attr_callback)(uint8_t scope, uint8_t* uid,
                                            uint16_t uid_counter,
                                            uint8_t num_attr,
                                            btrc_media_attr_t* p_attrs,
                                            const STACK::RawAddress& bd_addr); /* CT请求获取指定媒体条目属性 */

/** Callback for play request for the media item indicated by an identifier */
typedef void (*btrc_play_item_callback)(uint8_t scope, uint16_t uid_counter,
                                        uint8_t* uid,
                                        const STACK::RawAddress& bd_addr); /* CT请求播放指定媒体条目 */

/** Callback to fetch total number of items from a folder **/
typedef void (*btrc_get_total_num_of_items_callback)(uint8_t scope,
                                                     const STACK::RawAddress& bd_addr); /* CT请求获取指定范围条目总数 */
/** Callback to set shuffle and repeat mode. */
typedef void (*btrc_app_settings_callback)(uint8_t shuffle_mode, uint8_t repeat_mode); /* CT设置随机/重复播放模式 */

/** Callback for conducting recursive search on a current browsed path for a
 * specified string */
typedef void (*btrc_search_callback)(uint16_t charset_id, uint16_t str_len,
                                     uint8_t* p_str, const STACK::RawAddress& bd_addr); /* CT请求在当前浏览目录递归搜索 */

/** Callback to add a specified media item indicated by an identifier to now
 * playing queue. */
typedef void (*btrc_add_to_now_playing_callback)(uint8_t scope, uint8_t* uid,
                                                 uint16_t uid_counter,
                                                 const STACK::RawAddress& bd_addr); /* CT请求将媒体条目加入正在播放队列 */

/** BT-RC Target callback structure. */
typedef struct {
  /** set to sizeof(BtRcCallbacks) */
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  btrc_remote_features_callback remote_features_cb; /* 对端CT特性上报回调 */
  btrc_get_play_status_callback get_play_status_cb; /* 播放状态查询请求回调 */
  btrc_list_player_app_attr_callback list_player_app_attr_cb; /* 应用属性列表查询回调 */
  btrc_list_player_app_values_callback list_player_app_values_cb; /* 属性取值列表查询回调 */
  btrc_get_player_app_value_callback get_player_app_value_cb; /* 应用设置当前值查询回调 */
  btrc_get_player_app_attrs_text_callback get_player_app_attrs_text_cb; /* 应用属性文本查询回调 */
  btrc_get_player_app_values_text_callback get_player_app_values_text_cb; /* 应用取值文本查询回调 */
  btrc_set_player_app_value_callback set_player_app_value_cb; /* 应用设置值设置回调 */
  btrc_get_element_attr_callback get_element_attr_cb; /* 媒体元素属性查询回调 */
  btrc_register_notification_callback register_notification_cb; /* 注册通知回调 */
  btrc_volume_change_callback volume_change_cb; /* 绝对音量变化回调 */
  btrc_passthrough_cmd_callback passthrough_cmd_cb; /* 按键直通命令回调 */
  btrc_set_addressed_player_callback set_addressed_player_cb; /* 设置寻址播放器回调 */
  btrc_set_browsed_player_callback set_browsed_player_cb; /* 设置被浏览播放器回调 */
  btrc_get_folder_items_callback get_folder_items_cb; /* 获取浏览条目回调 */
  btrc_change_path_callback change_path_cb; /* 切换目录路径回调 */
  btrc_get_item_attr_callback get_item_attr_cb; /* 获取媒体条目属性回调 */
  btrc_play_item_callback play_item_cb; /* 播放媒体条目回调 */
  btrc_get_total_num_of_items_callback get_total_num_of_items_cb; /* 获取条目总数回调 */
  btrc_search_callback search_cb; /* 目录搜索回调 */
  btrc_add_to_now_playing_callback add_to_now_playing_cb; /* 加入正在播放队列回调 */
  btrc_app_settings_callback app_settings_cb; /* 设置随机/重复模式回调 */
} btrc_callbacks_t;

/** Represents the standard BT-RC AVRCP Target interface. */
typedef struct {
  /** set to sizeof(BtRcInterface) */
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  /**
   * Register the BtRc callbacks
   */
  bt_status_t (*init)(btrc_callbacks_t* callbacks); /* 初始化TG接口并注册回调 */

  /**
   * Send the response to a GetPlayStatus request from the CT,
   * reporting the current playback status, song length and
   * song position.
   *
   * @param bd_addr remote CT device address
   * @param play_status current play status, one of the
   *                    btrc_play_status_t values
   * @param song_len total song duration, in milliseconds
   * @param song_pos current song position, in milliseconds
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  /* 播放状态查询(GetPlayStatus)响应：上报当前播放状态、曲目总时长及播放进度(毫秒) */
  bt_status_t (*get_play_status_rsp)(const STACK::RawAddress& bd_addr,
                                     btrc_play_status_t play_status,
                                     uint32_t song_len, uint32_t song_pos); /* 播放状态查询响应(状态/时长/进度) */

  /** Lists the support player application attributes (Shuffle/Repeat/...)
  **  num_attr: Specifies the number of attributes contained in the pointer
  *p_attrs
  */
  bt_status_t (*list_player_app_attr_rsp)(const STACK::RawAddress& bd_addr,
                                          int num_attr,
                                          btrc_player_attr_t* p_attrs); /* 应用属性列表查询响应 */

  /** Lists the support player application attributes (Shuffle Off/On/Group)
  **  num_val: Specifies the number of values contained in the pointer p_vals
  */
  bt_status_t (*list_player_app_value_rsp)(const STACK::RawAddress& bd_addr,
                                           int num_val, uint8_t* p_vals); /* 属性取值列表查询响应 */

  /** Returns the current application attribute values for each of the specified
   * attr_id */
  bt_status_t (*get_player_app_value_rsp)(const STACK::RawAddress& bd_addr,
                                          btrc_player_settings_t* p_vals); /* 应用设置当前值查询响应 */

  /** Returns the application attributes text ("Shuffle"/"Repeat"/...)
  **  num_attr: Specifies the number of attributes' text contained in the
  *pointer p_attrs
  */
  bt_status_t (*get_player_app_attr_text_rsp)(
      const STACK::RawAddress& bd_addr, int num_attr,
      btrc_player_setting_text_t* p_attrs); /* 应用属性文本查询响应 */

  /** Returns the application attributes text ("Shuffle"/"Repeat"/...)
  **  num_attr: Specifies the number of attribute values' text contained in the
  *pointer p_vals
  */
  bt_status_t (*get_player_app_value_text_rsp)(
      const STACK::RawAddress& bd_addr, int num_val,
      btrc_player_setting_text_t* p_vals); /* 应用取值文本查询响应 */

  /**
   * Send the response to a GetElementAttributes request, returning
   * the text of the current song's element attributes
   * (e.g. "Title", "Album", "Artist").
   *
   * @param bd_addr remote CT device address
   * @param num_attr number of attribute entries contained in p_attrs
   * @param p_attrs array of attribute id and text value pairs
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  /* 媒体元素属性查询(GetElementAttributes)响应：返回当前曲目各元素属性文本
   * (如标题/专辑/艺术家)，共num_attr个"属性ID+文本"条目 */
  bt_status_t (*get_element_attr_rsp)(const STACK::RawAddress& bd_addr,
                                      uint8_t num_attr,
                                      btrc_element_attr_val_t* p_attrs); /* 媒体元素属性查询响应 */

  /** Response to set player attribute request ("Shuffle"/"Repeat")
  **  rsp_status: Status of setting the player attributes for the current media
  *player
  */
  bt_status_t (*set_player_app_value_rsp)(const STACK::RawAddress& bd_addr,
                                          btrc_status_t rsp_status); /* 应用属性设置响应 */

  /**
   * Send the response to a register notification request from the CT
   * (play state change, track change, etc.).
   *
   * @param event_id event id this notification response corresponds to,
   *                 one of the btrc_event_id_t values
   * @param type response type: interim (initial reply on registration)
   *             or changed (event-triggered update)
   * @param p_param event-specific parameters, populated according to
   *                event_id
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  /* 注册通知(RegisterNotification)响应：type为临时响应(注册时初始应答)或
   * 变化响应(事件触发后上报)，p_param按event_id填充事件参数 */
  bt_status_t (*register_notification_rsp)(
      btrc_event_id_t event_id, btrc_notification_type_t type,
      btrc_register_notification_t* p_param); /* 注册通知响应(临时/变化) */

  /* AVRCP 1.4 enhancements */

  /**
   * Send the current volume setting to the remote side. Currently
   * limited to SetAbsoluteVolume; could be enhanced to support
   * Relative Volume (AVRCP 1.0), which would send VOLUME_UP/
   * VOLUME_DOWN instead of an absolute volume level.
   *
   * @param volume absolute volume level, 0 to 127; bit 7 is reserved
   *               and cannot be set
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  /* 向对端CT发送绝对音量(SetAbsoluteVolume)：volume取值0-127，bit7为保留位不可设置 */
  bt_status_t (*set_volume)(uint8_t volume); /* 向对端CT设置绝对音量(0-127) */

  /* Set addressed player response from TG to CT */
  bt_status_t (*set_addressed_player_rsp)(const STACK::RawAddress& bd_addr,
                                          btrc_status_t rsp_status); /* 设置寻址播放器响应 */

  /* Set browsed player response from TG to CT */
  bt_status_t (*set_browsed_player_rsp)(const STACK::RawAddress& bd_addr,
                                        btrc_status_t rsp_status,
                                        uint32_t num_items, uint16_t charset_id,
                                        uint8_t folder_depth,
                                        btrc_br_folder_name_t* p_folders); /* 设置被浏览播放器响应(含目录路径栈) */

  /* Get folder item list response from TG to CT */
  bt_status_t (*get_folder_items_list_rsp)(const STACK::RawAddress& bd_addr,
                                           btrc_status_t rsp_status,
                                           uint16_t uid_counter,
                                           uint8_t num_items,
                                           btrc_folder_items_t* p_items); /* 浏览条目列表查询响应 */

  /* Change path response from TG to CT */
  bt_status_t (*change_path_rsp)(const STACK::RawAddress& bd_addr,
                                 btrc_status_t rsp_status, uint32_t num_items); /* 目录路径切换响应 */

  /** Returns the element's attributes num_attr: Specifies the number of
   * attributes' text contained in the pointer p_attrs
   */
  bt_status_t (*get_item_attr_rsp)(const STACK::RawAddress& bd_addr,
                                   btrc_status_t rsp_status, uint8_t num_attr,
                                   btrc_element_attr_val_t* p_attrs); /* 媒体条目属性查询响应 */

  /* play media item response from TG to CT */
  bt_status_t (*play_item_rsp)(const STACK::RawAddress& bd_addr,
                               btrc_status_t rsp_status); /* 播放媒体条目响应 */

  /* get total number of items response from TG to CT*/
  bt_status_t (*get_total_num_of_items_rsp)(const STACK::RawAddress& bd_addr,
                                            btrc_status_t rsp_status,
                                            uint32_t uid_counter,
                                            uint32_t num_items); /* 条目总数查询响应 */

  /* Search VFS response from TG to CT */
  bt_status_t (*search_rsp)(const STACK::RawAddress& bd_addr, btrc_status_t rsp_status,
                            uint32_t uid_counter, uint32_t num_items); /* 目录搜索响应 */

  /* add_to_now playing list response from TG to CT */
  bt_status_t (*add_to_now_playing_rsp)(const STACK::RawAddress& bd_addr,
                                        btrc_status_t rsp_status); /* 加入正在播放队列响应 */

  /** Closes the interface. */
  void (*cleanup)(void); /* 清理并关闭TG接口 */
} btrc_interface_t;

typedef void (*btrc_passthrough_rsp_callback)(const STACK::RawAddress& bd_addr, int id,
                                              int key_state); /* 直通命令响应回调 */

typedef void (*btrc_groupnavigation_rsp_callback)(int id, int key_state); /* 组导航命令响应回调 */

typedef void (*btrc_connection_state_callback)(bool rc_connect, bool bt_connect,
                                               const STACK::RawAddress& bd_addr); /* AVRCP/ACL连接状态变化回调 */

typedef void (*btrc_ctrl_getrcfeatures_callback)(const STACK::RawAddress& bd_addr,
                                                 int features); /* TG支持的AVRCP特性上报回调 */

typedef void (*btrc_ctrl_setabsvol_cmd_callback)(const STACK::RawAddress& bd_addr,
                                                 uint8_t abs_vol,
                                                 uint8_t label); /* TG设置绝对音量命令回调 */

typedef void (*btrc_ctrl_registernotification_abs_vol_callback)(
    const STACK::RawAddress& bd_addr, uint8_t label); /* TG注册绝对音量通知回调 */

typedef void (*btrc_ctrl_setplayerapplicationsetting_rsp_callback)(
    const STACK::RawAddress& bd_addr, uint8_t accepted); /* 应用属性设置命令响应回调 */

typedef void (*btrc_ctrl_playerapplicationsetting_callback)(
    const STACK::RawAddress& bd_addr, uint8_t num_attr,
    btrc_player_app_attr_t* app_attrs, uint8_t num_ext_attr,
    btrc_player_app_ext_attr_t* ext_attrs); /* TG应用设置属性上报回调 */

typedef void (*btrc_ctrl_playerapplicationsetting_changed_callback)(
    const STACK::RawAddress& bd_addr, const btrc_player_settings_t& vals); /* TG应用设置变化通知回调 */

typedef void (*btrc_ctrl_track_changed_callback)(
    const STACK::RawAddress& bd_addr, uint8_t num_attr,
    btrc_element_attr_val_t* p_attrs); /* TG当前曲目变化(含元数据)回调 */

typedef void (*btrc_ctrl_play_position_changed_callback)(
    const STACK::RawAddress& bd_addr, uint32_t song_len, uint32_t song_pos); /* TG播放进度变化回调 */

typedef void (*btrc_ctrl_play_status_changed_callback)(
    const STACK::RawAddress& bd_addr, btrc_play_status_t play_status); /* TG播放状态变化回调 */

typedef void (*btrc_ctrl_get_folder_items_callback)(
    const STACK::RawAddress& bd_addr, btrc_status_t status,
    const btrc_folder_items_t* folder_items, uint8_t count); /* TG浏览条目列表响应回调 */

typedef void (*btrc_ctrl_change_path_callback)(const STACK::RawAddress& bd_addr,
                                               uint32_t count); /* TG目录路径切换响应回调 */

typedef void (*btrc_ctrl_set_browsed_player_callback)(const STACK::RawAddress& bd_addr,
                                                      uint8_t num_items,
                                                      uint8_t depth); /* TG设置被浏览播放器响应回调 */
typedef void (*btrc_ctrl_set_addressed_player_callback)(
    const STACK::RawAddress& bd_addr, uint8_t status); /* TG设置寻址播放器响应回调 */
typedef void (*btrc_ctrl_addressed_player_changed_callback)(
    const STACK::RawAddress& bd_addr, uint16_t id); /* TG寻址播放器变化通知回调 */
typedef void (*btrc_ctrl_now_playing_contents_changed_callback)(
    const STACK::RawAddress& bd_addr); /* TG正在播放内容变化通知回调 */
typedef void (*btrc_ctrl_available_player_changed_callback)(
    const STACK::RawAddress& bd_addr); /* TG可用播放器集合变化通知回调 */

typedef void (*btrc_ctrl_get_cover_art_psm_callback)(const STACK::RawAddress& bd_addr,
    const uint16_t psm); /* TG封面图片PSM(L2CAP通道)上报回调 */

/** BT-RC Controller callback structure. */
typedef struct {
  /** set to sizeof(BtRcCallbacks) */
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  btrc_passthrough_rsp_callback passthrough_rsp_cb; /* 直通命令响应回调 */
  btrc_groupnavigation_rsp_callback groupnavigation_rsp_cb; /* 组导航命令响应回调 */
  btrc_connection_state_callback connection_state_cb; /* 连接状态变化回调 */
  btrc_ctrl_getrcfeatures_callback getrcfeatures_cb; /* TG特性上报回调 */
  btrc_ctrl_setplayerapplicationsetting_rsp_callback setplayerappsetting_rsp_cb; /* 应用属性设置响应回调 */
  btrc_ctrl_playerapplicationsetting_callback playerapplicationsetting_cb; /* 应用设置属性上报回调 */
  btrc_ctrl_playerapplicationsetting_changed_callback
      playerapplicationsetting_changed_cb; /* 应用设置变化通知回调 */
  btrc_ctrl_setabsvol_cmd_callback setabsvol_cmd_cb; /* 设置绝对音量命令回调 */
  btrc_ctrl_registernotification_abs_vol_callback
      registernotification_absvol_cb; /* 注册绝对音量通知回调 */
  btrc_ctrl_track_changed_callback track_changed_cb; /* 曲目变化回调 */
  btrc_ctrl_play_position_changed_callback play_position_changed_cb; /* 播放进度变化回调 */
  btrc_ctrl_play_status_changed_callback play_status_changed_cb; /* 播放状态变化回调 */
  btrc_ctrl_get_folder_items_callback get_folder_items_cb; /* 浏览条目列表响应回调 */
  btrc_ctrl_change_path_callback change_folder_path_cb; /* 目录路径切换响应回调 */
  btrc_ctrl_set_browsed_player_callback set_browsed_player_cb; /* 设置被浏览播放器响应回调 */
  btrc_ctrl_set_addressed_player_callback set_addressed_player_cb; /* 设置寻址播放器响应回调 */
  btrc_ctrl_addressed_player_changed_callback addressed_player_changed_cb; /* 寻址播放器变化通知回调 */
  btrc_ctrl_now_playing_contents_changed_callback
      now_playing_contents_changed_cb; /* 正在播放内容变化通知回调 */
  btrc_ctrl_available_player_changed_callback available_player_changed_cb; /* 可用播放器变化通知回调 */
  btrc_ctrl_get_cover_art_psm_callback get_cover_art_psm_cb; /* 封面图片PSM上报回调 */
} btrc_ctrl_callbacks_t;

/** Represents the standard BT-RC AVRCP Controller interface. */
typedef struct {
  /** set to sizeof(BtRcInterface) */
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  /**
   * Register the BtRc callbacks
   */
  bt_status_t (*init)(btrc_ctrl_callbacks_t* callbacks); /* 初始化CT接口并注册回调 */

  /** send pass through command to target */
  bt_status_t (*send_pass_through_cmd)(const STACK::RawAddress& bd_addr,
                                       uint8_t key_code, uint8_t key_state); /* 向TG发送按键直通命令 */

  /** send group navigation command to target */
  bt_status_t (*send_group_navigation_cmd)(const STACK::RawAddress& bd_addr,
                                           uint8_t key_code, uint8_t key_state); /* 向TG发送组导航命令 */

  /** send command to set player applicaiton setting attributes to target */
  bt_status_t (*set_player_app_setting_cmd)(const STACK::RawAddress& bd_addr,
                                            uint8_t num_attrib,
                                            uint8_t* attrib_ids,
                                            uint8_t* attrib_vals); /* 向TG设置播放器应用属性值 */

  /** send command to play a particular item */
  bt_status_t (*play_item_cmd)(const STACK::RawAddress& bd_addr, uint8_t scope,
                               uint8_t* uid, uint16_t uid_counter); /* 请求TG播放指定媒体条目 */

  /** get the current track's media metadata */
  bt_status_t (*get_current_metadata_cmd)(const STACK::RawAddress& bd_addr); /* 查询当前曲目元数据 */

  /** get the playback state */
  bt_status_t (*get_playback_state_cmd)(const STACK::RawAddress& bd_addr); /* 查询播放状态 */

  /** get the now playing list */
  bt_status_t (*get_now_playing_list_cmd)(const STACK::RawAddress& bd_addr,
                                          uint32_t start, uint32_t end); /* 查询正在播放列表条目 */

  /** get the folder list */
  bt_status_t (*get_folder_list_cmd)(const STACK::RawAddress& bd_addr, uint32_t start,
                                     uint32_t end); /* 查询目录文件夹列表条目 */

  /** get the player list */
  bt_status_t (*get_player_list_cmd)(const STACK::RawAddress& bd_addr, uint32_t start,
                                     uint32_t end); /* 查询播放器列表条目 */

  /** change the folder path */
  bt_status_t (*change_folder_path_cmd)(const STACK::RawAddress& bd_addr,
                                        uint8_t direction, uint8_t* uid); /* 切换TG浏览目录路径 */

  /** set browsed player */
  bt_status_t (*set_browsed_player_cmd)(const STACK::RawAddress& bd_addr,
                                        uint16_t player_id); /* 设置TG被浏览播放器 */

  /** set addressed player */
  bt_status_t (*set_addressed_player_cmd)(const STACK::RawAddress& bd_addr,
                                          uint16_t player_id); /* 设置TG寻址播放器 */

  /** send rsp to set_abs_vol received from target */
  bt_status_t (*set_volume_rsp)(const STACK::RawAddress& bd_addr, uint8_t abs_vol,
                                uint8_t label); /* 响应TG的设置绝对音量命令 */

  /** send notificaiton rsp for abs vol to target */
  bt_status_t (*register_abs_vol_rsp)(const STACK::RawAddress& bd_addr,
                                      btrc_notification_type_t rsp_type,
                                      uint8_t abs_vol, uint8_t label); /* 向TG发送绝对音量通知响应 */

  /** Closes the interface. */
  void (*cleanup)(void); /* 清理并关闭CT接口 */
} btrc_ctrl_interface_t;

} //__END_DECLS

#endif /* VENDOR_INCLUDE_BT_RC_H */
