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
 * 文件说明：蓝牙公共类型定义头文件。
 * 本文件定义了 JNI 层与蓝牙协议栈层之间共用的公共常量与数据结构，
 * 包括被跟踪广播设备信息、GATT 数据库属性元素、扫描过滤参数以及
 * 广播包内容过滤（APCF）命令等类型，供 GATT/广播/扫描等功能模块共同使用。
 */
/******************************************************************************
 *
 * This file contains constants and definitions that can be used commonly
 * between JNI and stack layer
 *
 ******************************************************************************/
#ifndef VENDOR_INCLUDE_BT_COMMON_TYPES_H
#define VENDOR_INCLUDE_BT_COMMON_TYPES_H

#include <vector>

#include "hardware/bluetooth.h"
#include "types/bluetooth/uuid.h"
#include "types/raw_address.h"

/* 被跟踪广播设备信息结构，用于硬件广播过滤跟踪回调上报 */
typedef struct {
  uint8_t client_if;                     /* 客户端接口标识 */
  uint8_t filt_index;                    /* 过滤器索引 */
  uint8_t advertiser_state;              /* 广播者状态 */
  uint8_t advertiser_info_present;       /* 是否携带广播者信息 */
  uint8_t addr_type;                     /* 地址类型（公共/随机） */
  uint8_t tx_power;                      /* 发射功率 */
  int8_t rssi_value;                     /* 接收信号强度指示值 */
  uint16_t time_stamp;                   /* 时间戳 */
  RawAddress bd_addr;                    /* 蓝牙设备地址 */
  uint8_t adv_pkt_len;                   /* 广播包数据长度 */
  uint8_t* p_adv_pkt_data;               /* 广播包数据指针 */
  uint8_t scan_rsp_len;                  /* 扫描响应数据长度 */
  uint8_t* p_scan_rsp_data;              /* 扫描响应数据指针 */
} btgatt_track_adv_info_t;

/* GATT 数据库属性元素类型枚举 */
typedef enum {
  BTGATT_DB_PRIMARY_SERVICE,   /* 主服务 */
  BTGATT_DB_SECONDARY_SERVICE, /* 次服务 */
  BTGATT_DB_INCLUDED_SERVICE,  /* 包含服务 */
  BTGATT_DB_CHARACTERISTIC,    /* 特征值 */
  BTGATT_DB_DESCRIPTOR,        /* 描述符 */
} bt_gatt_db_attribute_type_t;

/* GATT 数据库元素结构，描述服务发现结果中的单个属性 */
typedef struct {
  uint16_t id;                            /* 属性 ID */
  bluetooth::Uuid uuid;                   /* 属性 UUID */
  bt_gatt_db_attribute_type_t type;       /* 属性类型 */
  uint16_t attribute_handle;              /* 属性句柄 */

  /*
   * If |type| is |BTGATT_DB_PRIMARY_SERVICE|, or
   * |BTGATT_DB_SECONDARY_SERVICE|, this contains the start and end attribute
   * handles.
   */
  uint16_t start_handle;                  /* 起始属性句柄（服务类型时有效） */
  uint16_t end_handle;                    /* 结束属性句柄（服务类型时有效） */

  /*
   * If |type| is |BTGATT_DB_CHARACTERISTIC|, this contains the properties of
   * the characteristic.
   */
  uint8_t properties;                     /* 特征值属性（特征值类型时有效） */
  uint16_t extended_properties;           /* 扩展属性 */

  uint16_t permissions;                   /* 属性访问权限 */
} btgatt_db_element_t;

/* GATT 扫描过滤参数设置结构，用于硬件过滤条件配置 */
typedef struct {
  uint16_t feat_seln;                   /* 特性选择位掩码 */
  uint16_t list_logic_type;             /* 过滤列表逻辑类型（与/或） */
  uint8_t filt_logic_type;              /* 过滤器逻辑类型 */
  uint8_t rssi_high_thres;              /* RSSI 高门限 */
  uint8_t rssi_low_thres;               /* RSSI 低门限 */
  uint8_t dely_mode;                    /* 延迟模式（快速/低延迟） */
  uint16_t found_timeout;               /* 设备被发现超时时间 */
  uint16_t lost_timeout;                /* 设备丢失超时时间 */
  uint8_t found_timeout_cnt;            /* 被发现超时计数 */
  uint16_t num_of_tracking_entries;     /* 跟踪表项数量 */
} btgatt_filt_param_setup_t;

// Advertising Packet Content Filter
/* 广播包内容过滤（APCF）命令结构，用于硬件扫描过滤器配置 */
struct ApcfCommand {
  uint8_t type;                    /* 过滤类型 */
  RawAddress address;              /* 过滤地址 */
  uint8_t addr_type;               /* 地址类型 */
  bluetooth::Uuid uuid;            /* 服务 UUID 过滤条件 */
  bluetooth::Uuid uuid_mask;       /* 服务 UUID 掩码 */
  std::vector<uint8_t> name;       /* 完整/缩短本地名称过滤条件 */
  uint16_t company;                /* 厂商 ID 过滤条件 */
  uint16_t company_mask;           /* 厂商 ID 掩码 */
  std::vector<uint8_t> data;       /* 广播数据过滤条件 */
  std::vector<uint8_t> data_mask;  /* 广播数据掩码 */
  std::array<uint8_t, 16> irk;  // 128 bit/16 octet IRK /* 128 位/16 字节身份解析密钥 */
};

#endif /* VENDOR_INCLUDE_BT_COMMON_TYPES_H */
