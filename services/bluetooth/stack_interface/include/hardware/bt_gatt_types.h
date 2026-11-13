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

/* 本文件定义 GATT 相关基础数据类型：GATT 服务类型（主/次服务）、GATT ID、服务ID 结构体，
 * 以及 GATT 连接首选物理传输方式（BR/EDR、LE、自动）等类型定义。 */

#ifndef VENDOR_INCLUDE_BT_GATT_TYPES_H
#define VENDOR_INCLUDE_BT_GATT_TYPES_H

#include <sys/cdefs.h>
#include <stdbool.h>
#include <stdint.h>

#include <bluetooth/uuid.h>

__BEGIN_DECLS

/**
 * GATT Service types
 */
/* GATT 服务类型定义 */
#define BTGATT_SERVICE_TYPE_PRIMARY 0    /* 主服务 */
#define BTGATT_SERVICE_TYPE_SECONDARY 1  /* 次服务 */

/** GATT ID adding instance id tracking to the UUID */
/* GATT 标识：在 UUID 基础上附加实例ID跟踪 */
typedef struct {
  bluetooth::Uuid uuid;  /* 服务/特征的UUID */
  uint8_t inst_id;       /* 实例ID */
} btgatt_gatt_id_t;

/** GATT Service ID also identifies the service type (primary/secondary) */
/* GATT 服务ID：同时标识服务类型（主服务/次服务） */
typedef struct {
  btgatt_gatt_id_t id;  /* GATT 标识（UUID + 实例ID） */
  uint8_t is_primary;   /* 是否为主服务 */
} btgatt_srvc_id_t;

/** Preferred physical Transport for GATT connection */
/* GATT 连接首选物理传输方式 */
typedef enum {
  GATT_TRANSPORT_AUTO,   /* 自动选择传输方式 */
  GATT_TRANSPORT_BREDR,  /* 经典蓝牙（BR/EDR）传输 */
  GATT_TRANSPORT_LE      /* 低功耗（LE）传输 */
} btgatt_transport_t;

__END_DECLS

#endif /* VENDOR_INCLUDE_BT_GATT_TYPES_H */
