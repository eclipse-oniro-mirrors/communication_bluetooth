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
 * 文件说明：定义蓝牙 Socket（套接字）协议栈硬件抽象层（HAL）接口。
 * 支持 RFCOMM、SCO、L2CAP、BLE L2CAP（LE CoC）等 socket 类型的监听与连接，
 * 并定义 socket 连接信号/连接信息结构体及 socket 接口结构体（btsock_interface_t）。
 */

#pragma once

#include <stddef.h>

#include "bluetooth.h"
#include "bluetooth/uuid.h"
#include "types/raw_address.h"

extern "C" { //__BEGIN_DECLS

#define BTSOCK_FLAG_ENCRYPT 1 /* socket链路加密 */
#define BTSOCK_FLAG_AUTH (1 << 1) /* socket链路需要鉴权（配对） */
#define BTSOCK_FLAG_NO_SDP (1 << 2) /* 不创建SDP服务记录 */
#define BTSOCK_FLAG_AUTH_MITM (1 << 3) /* 鉴权要求中间人（MITM）防护 */
#define BTSOCK_FLAG_AUTH_16_DIGIT (1 << 4) /* 鉴权使用16位数字配对 */
#define BTSOCK_FLAG_LE_COC (1 << 5) /* BLE面向连接的通道（LE CoC） */

/* 蓝牙socket类型枚举 */
typedef enum {
  BTSOCK_RFCOMM = 1, /* RFCOMM socket（串口仿真协议） */
  BTSOCK_SCO = 2, /* SCO socket（同步面向连接音频链路） */
  BTSOCK_L2CAP = 3, /* L2CAP socket（逻辑链路控制与适配协议） */
  BTSOCK_L2CAP_LE = 4 /* BLE L2CAP socket（低功耗蓝牙L2CAP） */
} btsock_type_t;

/** Represents the standard BT SOCKET interface. */
/* socket连接信号结构体（socket连接建立后可从socket fd中读取） */
typedef struct {
  short size; /* 结构体大小 */
  RawAddress bd_addr; /* 远端设备地址 */
  int channel; /* socket通道号 */
  int status; /* 连接状态 */

  // The writer must make writes using a buffer of this maximum size
  // to avoid loosing data. (L2CAP only)
  unsigned short max_tx_packet_size; /* 最大发送包长度（仅L2CAP），写入方须按此最大长度写以避免丢数据 */

  // The reader must read using a buffer of at least this size to avoid
  // loosing data. (L2CAP only)
  unsigned short max_rx_packet_size; /* 最小接收缓冲大小（仅L2CAP），读取方须至少按此大小读以避免丢数据 */
} __attribute__((packed)) sock_connect_signal_t;

/* socket连接信息结构体（COMMUNICATION_L2场景使用） */
typedef struct {
    bool status; /* 连接状态 */
    RawAddress bd_addr; /* 远端设备地址 */

    // The writer must make writes using a buffer of this maximum size
    // to avoid loosing data. (L2CAP only)
    unsigned short max_tx_packet_size; /* 最大发送包长度（仅L2CAP） */

    // The reader must read using a buffer of at least this size to avoid
    // loosing data. (L2CAP only)
    unsigned short max_rx_packet_size; /* 最小接收缓冲大小（仅L2CAP） */
} __attribute__((packed)) sock_connect_info_t;

/* 标准蓝牙SOCKET接口结构体 */
typedef struct {
  /** set to size of this struct*/
  size_t size; /* 结构体大小，置为sizeof(btsock_interface_t) */

  /**
   * Listen for incoming connections on a RFCOMM/L2CAP UUID or channel.
   * Returns the socket fd from which a sock_connect_signal_t can be
   * read when a remote device connects.
   * If neither a UUID nor a channel is provided, a channel will be
   * allocated, and an SDP service record can be created afterwards by
   * passing the channel number to create_sdp_record(...) in bt_sdp.
   *
   * @param type socket type, one of the btsock_type_t values
   * @param service_name service name for the SDP record
   * @param service_uuid service UUID to listen on, NULL to use a raw channel
   * @param channel channel number, 0 to auto-allocate when listening by UUID
   * @param sock_fd output parameter for the returned socket fd
   * @param flags socket flags, OR-ed combination of BTSOCK_FLAG_* values
   * @param callingUid UID of the application requesting the socket,
   *                   used for traffic accounting
   * @param callingPid PID of the application requesting the socket
   * @param appName name of the application requesting the socket
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  /* 监听指定UUID/通道上的入连接：远端连入后可从返回的socket fd读取
   * 连接信号sock_connect_signal_t；未指定UUID和通道时自动分配通道，
   * 后续可将通道号传给bt_sdp的create_sdp_record创建SDP服务记录；
   * callingUid等用于流量统计 */
  bt_status_t (*listen)(btsock_type_t type, const char* service_name,
                        const bluetooth::Uuid* service_uuid, int channel,
                        int* sock_fd, int flags, int callingUid,
                        int callingPid, const char* appName);  /* 监听RFCOMM UUID/通道，返回socket fd用于读取远端连接信号 */

  /**
   * Connect to a remote device's RFCOMM/L2CAP UUID channel. Returns
   * the socket fd from which the sock_connect_signal_t and a new
   * socket fd to be accepted can be read out when the connection
   * is established.
   *
   * @param bd_addr remote device address to connect to
   * @param type socket type, one of the btsock_type_t values
   * @param uuid service UUID to connect to
   * @param channel channel number, 0 to look up via SDP by UUID
   * @param sock_fd output parameter for the returned socket fd
   * @param flags socket flags, OR-ed combination of BTSOCK_FLAG_* values
   * @param callingUid UID of the application requesting the socket,
   *                   used for traffic accounting
   * @param callingPid PID of the application requesting the socket
   * @param appName name of the application requesting the socket
   * @return BT_STATUS_SUCCESS on success, error code otherwise
   */
  /* 连接远端设备的UUID通道：连接建立后可从返回的socket fd读取连接信号
   * sock_connect_signal_t及待接受(accept)的新socket fd；
   * callingUid等用于流量统计 */
  bt_status_t (*connect)(const RawAddress* bd_addr, btsock_type_t type,
                         const bluetooth::Uuid* uuid, int channel, int* sock_fd,
                         int flags, int callingUid, int callingPid,
                         const char* appName);  /* 连接远端设备的RFCOMM UUID通道，返回socket fd用于读取连接信号及待接受的新socket fd */

  /**
   * Set the LE Data Length value to this connected peer to the
   * maximum supported by this BT controller. This command
   * suggests to the BT controller to set its maximum transmission
   * packet size.
   */
  void (*request_max_tx_data_length)(const RawAddress& bd_addr);  /* 请求将该连接对端的LE数据长度设为本端控制器支持的最大传输包长 */

} btsock_interface_t;

} //__END_DECLS
