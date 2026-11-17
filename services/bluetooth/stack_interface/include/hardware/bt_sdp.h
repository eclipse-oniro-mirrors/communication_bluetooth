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
 * 文件说明：蓝牙 SDP（Service Discovery Protocol，服务发现协议）接口定义。
 * 定义 SDP 记录类型枚举与各 profile（MAP/PBAP/OPP/SAP/DIP）的 SDP 记录
 * 结构体、SDP 搜索回调与回调集合，以及 SDP 初始化/注销、远端记录搜索、
 * SDP 记录创建/删除等接口函数指针。
 */

#pragma once

#include "bluetooth.h"

#define SDP_OPP_SUPPORTED_FORMATS_MAX_LENGTH 15 /* OPP 支持格式列表最大长度 */

extern "C" { //__BEGIN_DECLS

/**
 * These events are handled by the state machine
 */ /* SDP 记录类型枚举（由状态机处理） */
typedef enum {
  SDP_TYPE_RAW, /* 未知 UUID 时承载原始 SDP 搜索数据 */         // Used to carry raw SDP search data for unknown UUIDs
  SDP_TYPE_MAP_MAS, /* MAP 消息访问服务端 */     // Message Access Profile - Server
  SDP_TYPE_MAP_MNS, /* MAP 消息通知服务端 */     // Message Access Profile - Client (Notification Server)
  SDP_TYPE_PBAP_PSE, /* PBAP 电话簿服务端 */    // Phone Book Profile - Server
  SDP_TYPE_PBAP_PCE, /* PBAP 电话簿客户端 */    // Phone Book Profile - Client
  SDP_TYPE_OPP_SERVER, /* OPP 对象推送服务端 */  // Object Push Profile
  SDP_TYPE_SAP_SERVER, /* SIM 卡访问服务端 */   // SIM Access Profile
  SDP_TYPE_DIP /* 设备标识 profile */           // Device Identification Profile
} bluetooth_sdp_types;

/* SDP 记录头结构体 */
typedef struct _bluetooth_sdp_hdr {
  bluetooth_sdp_types type; /* 记录类型 */
  bluetooth::Uuid uuid; /* 服务 UUID */
  uint32_t service_name_length; /* 服务名长度 */
  char* service_name; /* 服务名 */
  int32_t rfcomm_channel_number; /* RFCOMM 通道号 */
  int32_t l2cap_psm; /* L2CAP PSM */
  int32_t profile_version; /* profile 版本 */
} bluetooth_sdp_hdr;

/**
 * Some signals need additional pointers, hence we introduce a
 * generic way to handle these pointers.
 */ /* 带扩展用户指针的 SDP 记录头结构体 */
typedef struct _bluetooth_sdp_hdr_overlay {
  bluetooth_sdp_types type; /* 记录类型 */
  bluetooth::Uuid uuid; /* 服务 UUID */
  uint32_t service_name_length; /* 服务名长度 */
  char* service_name; /* 服务名 */
  int32_t rfcomm_channel_number; /* RFCOMM 通道号 */
  int32_t l2cap_psm; /* L2CAP PSM */
  int32_t profile_version; /* profile 版本 */

  // User pointers, only used for some signals - see bluetooth_sdp_ops_record
  int user1_ptr_len; /* 用户指针 1 数据长度 */
  uint8_t* user1_ptr; /* 用户指针 1 */
  int user2_ptr_len; /* 用户指针 2 数据长度 */
  uint8_t* user2_ptr; /* 用户指针 2 */
} bluetooth_sdp_hdr_overlay;

/* MAP MAS（消息访问服务端）SDP 记录结构体 */
typedef struct _bluetooth_sdp_mas_record {
  bluetooth_sdp_hdr_overlay hdr; /* SDP 记录头 */
  uint32_t mas_instance_id; /* MAS 实例 ID */
  uint32_t supported_features; /* 支持的特性 */
  uint32_t supported_message_types; /* 支持的消息类型 */
} bluetooth_sdp_mas_record;

/* MAP MNS（消息通知服务端）SDP 记录结构体 */
typedef struct _bluetooth_sdp_mns_record {
  bluetooth_sdp_hdr_overlay hdr; /* SDP 记录头 */
  uint32_t supported_features; /* 支持的特性 */
} bluetooth_sdp_mns_record;

/* PBAP PSE（电话簿服务端）SDP 记录结构体 */
typedef struct _bluetooth_sdp_pse_record {
  bluetooth_sdp_hdr_overlay hdr; /* SDP 记录头 */
  uint32_t supported_features; /* 支持的特性 */
  uint32_t supported_repositories; /* 支持的电话簿存储库 */
} bluetooth_sdp_pse_record;

/* PBAP PCE（电话簿客户端）SDP 记录结构体 */
typedef struct _bluetooth_sdp_pce_record {
  bluetooth_sdp_hdr_overlay hdr; /* SDP 记录头 */
} bluetooth_sdp_pce_record;

/* OPP（对象推送）SDP 记录结构体 */
typedef struct _bluetooth_sdp_ops_record {
  bluetooth_sdp_hdr_overlay hdr; /* SDP 记录头 */
  int supported_formats_list_len; /* 支持格式列表长度 */
  uint8_t supported_formats_list[SDP_OPP_SUPPORTED_FORMATS_MAX_LENGTH]; /* 支持的对象格式列表 */
} bluetooth_sdp_ops_record;

/* SAP（SIM 卡访问）SDP 记录结构体 */
typedef struct _bluetooth_sdp_sap_record {
  bluetooth_sdp_hdr_overlay hdr; /* SDP 记录头 */
} bluetooth_sdp_sap_record;

/* DIP（设备标识）SDP 记录结构体 */
typedef struct _bluetooth_sdp_dip_record {
  bluetooth_sdp_hdr_overlay hdr; /* SDP 记录头 */
  uint16_t spec_id; /* 规范 ID */
  uint16_t vendor; /* 厂商 ID */
  uint16_t vendor_id_source; /* 厂商 ID 来源 */
  uint16_t product; /* 产品 ID */
  uint16_t version; /* 版本号 */
  bool primary_record; /* 是否为主记录 */
} bluetooth_sdp_dip_record;

/* SDP 记录联合体（按记录类型解释为具体 profile 记录） */
typedef union {
  bluetooth_sdp_hdr_overlay hdr; /* 通用记录头 */
  bluetooth_sdp_mas_record mas; /* MAP MAS 记录 */
  bluetooth_sdp_mns_record mns; /* MAP MNS 记录 */
  bluetooth_sdp_pse_record pse; /* PBAP PSE 记录 */
  bluetooth_sdp_pce_record pce; /* PBAP PCE 记录 */
  bluetooth_sdp_ops_record ops; /* OPP 记录 */
  bluetooth_sdp_sap_record sap; /* SAP 记录 */
  bluetooth_sdp_dip_record dip; /* DIP 记录 */
} bluetooth_sdp_record;

/** Callback for SDP search */ /* SDP 搜索结果回调 */
typedef void (*btsdp_search_callback)(bt_status_t status,
                                      const RawAddress& bd_addr,
                                      const bluetooth::Uuid& uuid,
                                      int num_records,
                                      bluetooth_sdp_record* records);

/* SDP 回调集合结构体 */
typedef struct {
  /** Set to sizeof(btsdp_callbacks_t) */ /* 结构体大小，设为 sizeof(btsdp_callbacks_t) */
  size_t size;
  btsdp_search_callback sdp_search_cb; /* SDP 搜索结果回调 */
} btsdp_callbacks_t;

/* 标准 BT SDP 接口结构体 */
typedef struct {
  /** Set to size of this struct */ /* 结构体大小，设为 sizeof(btsdp_interface_t) */
  size_t size;

  /** Register BT SDP search callbacks */ /* 注册 SDP 搜索回调并初始化 */
  bt_status_t (*init)(btsdp_callbacks_t* callbacks);

  /** Unregister BT SDP */ /* 注销 SDP */
  bt_status_t (*deinit)();

  /** Search for SDP records with specific uuid on remote device */ /* 在远端设备上按 UUID 搜索 SDP 记录 */
  bt_status_t (*sdp_search)(RawAddress* bd_addr, const bluetooth::Uuid& uuid);

  /**
   * Use listen in the socket interface to create rfcomm and/or l2cap PSM
   * channels, (without UUID and service_name and set the BTSOCK_FLAG_NO_SDP
   * flag in flags). Then use createSdpRecord to create the SDP record
   * associated with the rfcomm/l2cap channels.
   *
   * Returns a handle to the SDP record, which can be parsed to
   * remove_sdp_record.
   *
   * record           (in) The SDP record to create
   * record_handle    (out)The corresponding record handle will be written to
   * this pointer.
   */ /* 创建与 RFCOMM/L2CAP 通道关联的 SDP 记录，返回记录句柄 */
  bt_status_t (*create_sdp_record)(bluetooth_sdp_record* record,
                                   int* record_handle);

  /** Remove a SDP record created by createSdpRecord */ /* 删除由 create_sdp_record 创建的 SDP 记录 */
  bt_status_t (*remove_sdp_record)(int sdp_handle);
} btsdp_interface_t;

} //__END_DECLS
