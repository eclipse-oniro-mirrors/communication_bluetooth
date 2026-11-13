/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef IGATT_API_H
#define IGATT_API_H

#include <cstdint>

#include "types/bt_transport.h"

typedef enum GattStatus : uint8_t {
   GATT_SUCCESS = 0x00,            /* 0x00 */
  GATT_INVALID_HANDLE = 0x01,      /* 0x01 */
  GATT_READ_NOT_PERMIT = 0x02,    /* 0x02 */
  GATT_WRITE_NOT_PERMIT = 0x03,   /* 0x03 */
  GATT_INVALID_PDU = 0x04,        /* 0x04 */
  GATT_INSUF_AUTHENTICATION = 0x05, /* 0x05 */
  GATT_REQ_NOT_SUPPORTED = 0x06,  /* 0x06 */
  GATT_INVALID_OFFSET = 0x07,     /* 0x07 */
  GATT_INSUF_AUTHORIZATION = 0x08, /* 0x08 */
  GATT_PREPARE_Q_FULL = 0x09,     /* 0x09 */
  GATT_NOT_FOUND = 0x0a,          /* 0x0a */
  GATT_NOT_LONG = 0x0b,           /* 0x0b */
  GATT_INSUF_KEY_SIZE = 0x0c,     /* 0x0c */
  GATT_INVALID_ATTR_LEN = 0x0d,   /* 0x0d */
  GATT_ERR_UNLIKELY = 0x0e,       /* 0x0e */
  GATT_INSUF_ENCRYPTION = 0x0f,   /* 0x0f */
  GATT_UNSUPPORT_GRP_TYPE = 0x10, /* 0x10 */
  GATT_INSUF_RESOURCE = 0x11,     /* 0x11 */
  GATT_DATABASE_OUT_OF_SYNC = 0x12, /* 0x12 */
  GATT_VALUE_NOT_ALLOWED = 0x13,  /* 0x13 */
  GATT_ILLEGAL_PARAMETER = 0x87,  /* 0x87 */
  GATT_TOO_SHORT = 0x7f,          /* 0x7f */
  GATT_NO_RESOURCES = 0x80,       /* 0x80 */
  GATT_INTERNAL_ERROR = 0x81,      /* 0x81 */
  GATT_WRONG_STATE = 0x82,        /* 0x82 */
  GATT_DB_FULL = 0x83,            /* 0x83 */
  GATT_BUSY = 0x84,               /* 0x84 */
  GATT_ERROR = 0x85,              /* 0x85 */
  GATT_CMD_STARTED = 0x86,        /* 0x86 */
  GATT_PENDING = 0x88,            /* 0x88 */
  GATT_AUTH_FAIL = 0x89,          /* 0x89 */
  GATT_MORE = 0x8a,               /* 0x8a */
  GATT_INVALID_CFG = 0x8b,        /* 0x8b */
  GATT_SERVICE_STARTED = 0x8c,    /* 0x8c */
  GATT_ENCRYPED_MITM = GATT_SUCCESS, /* 0x00 */
  GATT_ENCRYPED_NO_MITM = 0x8d,   /* 0x8d */
  GATT_NOT_ENCRYPTED = 0x8e,      /* 0x8e */
  GATT_CONGESTED = 0x8f,          /* 0x8f */
  GATT_DUP_REG = 0x90,            /* 0x090 */
  GATT_ALREADY_OPEN = 0x91,       /* 0x91 */
  GATT_CANCEL = 0x92,             /* 0x92 */

  /* = 0xE0 ~ 0xFC reserved for future use */
  /* Procedure Already in progresses */
  GATT_PRC_IN_PROGRESS = 0xFE,
  /* Attribute value out of range. */
  GATT_OUT_OF_RANGE = 0xFF,
/* Client Characteristic Configuration Descriptor Improperly Configured */
  GATT_CCC_CFG_ERR = 0xFD,
} tGATT_STATUS;

/* GATT disconnect reason (HCI/H4 error code carried on disconnect) */
#include "stack/include/hci_error_code.h"
typedef enum : uint16_t {
  GATT_CONN_OK = 0,
  GATT_CONN_UNKNOWN = 0,
  /* general L2cap failure  */
  GATT_CONN_L2C_FAILURE = 1,
  /* 0x13 connection terminate by peer user  */
  GATT_CONN_TERMINATE_PEER_USER = HCI_ERR_PEER_USER,
  /* 0x16 connectionterminated by local host  */
  GATT_CONN_TERMINATE_LOCAL_HOST = HCI_ERR_CONN_CAUSE_LOCAL_HOST,
  /* 0x08 connection timeout  */
  GATT_CONN_TIMEOUT = HCI_ERR_CONNECTION_TOUT,
  /* 0x22 connection fail for LMP response tout */
  GATT_CONN_LMP_TIMEOUT = HCI_ERR_LMP_RESPONSE_TIMEOUT,

  BTA_GATT_CONN_NONE = 0x0101, /* 0x0101 no connection to cancel  */

} tGATT_DISCONN_REASON;

/* GATT authentication requirement values */
#define GATT_AUTH_REQ_NO_MITM 1 /* unauthenticated encryption */
#define GATT_AUTH_REQ_MITM 2    /* authenticated encryption */
#define GATT_AUTH_REQ_SIGNED_NO_MITM 3
#define GATT_AUTH_REQ_SIGNED_MITM 4
#define GATT_AUTH_REQ_NONE 0
typedef uint8_t tGATT_AUTH_REQ;

/* GATT write types */
#ifndef GATT_INVALID_CONN_ID
#define GATT_INVALID_CONN_ID 0xFFFF
#endif

enum { GATT_WRITE_NO_RSP = 1, GATT_WRITE, GATT_WRITE_PREPARE };
typedef uint8_t tGATT_WRITE_TYPE;

/* OH extension: service-layer tuning constants (moved from gattdefs.h) */
#ifndef GATT_MAX_ATTR_LEN
#define GATT_MAX_ATTR_LEN 600
#endif

#endif /* IGATT_API_H */
