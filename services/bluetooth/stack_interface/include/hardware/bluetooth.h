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

/* 头文件保护宏，防止头文件被重复包含 */
#ifndef VENDOR_INCLUDE_BLUETOOTH_H
#define VENDOR_INCLUDE_BLUETOOTH_H

/* 标准C库头文件：布尔类型、定长整型、系统数据类型 */
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/* AVRCP(音视频远程控制协议)接口定义 */
#include "avrcp/avrcp.h"
/* 蓝牙UUID(通用唯一识别码)定义 */
#include "bluetooth/uuid.h"
/* 蓝牙传输类型定义(BR/EDR或LE) */
#include "bt_transport.h"
/* 蓝牙设备地址类型定义 */
#include "types/raw_address.h"

/* 蓝牙硬件模块ID，用于加载蓝牙硬件模块 */
#define BT_HARDWARE_MODULE_ID "bluetooth"
/* 蓝牙协议栈模块ID，用于获取蓝牙协议栈接口 */
#define BT_STACK_MODULE_ID "bluetooth"

/** Bluetooth profile interface IDs */
/* 免提(Handsfree)协议服务端接口ID */
#define BT_PROFILE_HANDSFREE_ID "handsfree"
/* 免提协议客户端接口ID */
#define BT_PROFILE_HANDSFREE_CLIENT_ID "handsfree_client"
/* A2DP(高级音频分发)源端接口ID */
#define BT_PROFILE_ADVANCED_AUDIO_ID "a2dp"
/* A2DP音频接收端(Sink)接口ID */
#define BT_PROFILE_ADVANCED_AUDIO_SINK_ID "a2dp_sink"
/* 健康(Health)设备协议接口ID */
#define BT_PROFILE_HEALTH_ID "health"
/* 蓝牙Socket(套接字)协议接口ID */
#define BT_PROFILE_SOCKETS_ID "socket"
/* HID(人机接口设备)主机侧协议接口ID */
#define BT_PROFILE_HIDHOST_ID "hidhost"
/* HID设备侧协议接口ID */
#define BT_PROFILE_HIDDEV_ID "hiddev"
/* PAN(个人区域网络)协议接口ID */
#define BT_PROFILE_PAN_ID "pan"
/* MAP(消息访问)客户端协议接口ID */
#define BT_PROFILE_MAP_CLIENT_ID "map_client"
/* SDP(服务发现)客户端协议接口ID */
#define BT_PROFILE_SDP_CLIENT_ID "sdp"
/* GATT(通用属性)协议接口ID */
#define BT_PROFILE_GATT_ID "gatt"
/* AVRCP(音视频远程控制)目标端接口ID */
#define BT_PROFILE_AV_RC_ID "avrcp"
/* AVRCP控制器端接口ID */
#define BT_PROFILE_AV_RC_CTRL_ID "avrcp_ctrl"
/* 助听(HearingAid)协议接口ID */
#define BT_PROFILE_HEARING_AID_ID "hearing_aid"
/* LEAudio(低功耗音频)协议接口ID */
#define BT_PROFILE_LE_AUDIO_ID "le_audio"
/* 蓝牙密钥库接口ID */
#define BT_KEYSTORE_ID "bluetooth_keystore"
/* 蓝牙活动归因(流量统计归因)接口ID */
#define BT_ACTIVITY_ATTRIBUTION_ID "activity_attribution"
/* 音量控制(VolumeControl)协议接口ID */
#define BT_PROFILE_VC_ID "volume_control"
/* 蓝牙厂商自定义接口ID */
#define BT_VENDOR_INTERFACE_ID "bt_vender_interface"
/* 手表类厂商接口ID */
#define BT_VENDOR_WATCH_ID "watch"

/** Bluetooth test interface IDs */
/* MCAP测试接口ID，用于蓝牙测试 */
#define BT_TEST_INTERFACE_MCAP_ID "mcap_test"

const int BT_MAX_REMOTE_PROPERTY =  25;

/** Bluetooth Device Name */
typedef struct { uint8_t name[249]; } __attribute__((packed)) bt_bdname_t;

/** Bluetooth Adapter Visibility Modes*/
typedef enum {
  BT_SCAN_MODE_NONE,                    /* 不可见且不可连接 */
  BT_SCAN_MODE_CONNECTABLE,             /* 可连接但不可见 */
  BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE /* 可见且可连接 */
} bt_scan_mode_t;

/** Bluetooth Adapter State */
typedef enum { BT_STATE_OFF, BT_STATE_ON } bt_state_t; /* 蓝牙适配器状态: 关闭/开启 */

/** Bluetooth Adapter Input Output Capabilities which determine Pairing/Security
 */
/* 蓝牙适配器输入输出能力枚举，决定配对/安全模式(SSP)的鉴权方式 */
typedef enum {
  BT_IO_CAP_OUT,    /* DisplayOnly - 仅显示能力，可显示配对码 */
  BT_IO_CAP_IO,     /* DisplayYesNo - 显示+确认能力，可显示并确认配对码 */
  BT_IO_CAP_IN,     /* KeyboardOnly - 仅键盘输入能力，可输入配对码 */
  BT_IO_CAP_NONE,   /* NoInputNoOutput - 无输入无输出能力(Just Works模式) */
  BT_IO_CAP_KBDISP, /* Keyboard display - 键盘+显示能力，可输入并显示配对码 */
  BT_IO_CAP_MAX,    /* IO能力枚举上限，用于边界校验 */
  BT_IO_CAP_UNKNOWN = 0xFF /* Unknown value - 未知能力值 */
} bt_io_cap_t;

/** Bluetooth Error Status */
/** We need to build on this */

typedef enum {
  BT_STATUS_SUCCESS = 0,           /* 成功 */
  BT_STATUS_FAIL,                  /* 失败 */
  BT_STATUS_NOT_READY,             /* 蓝牙未就绪 */
  BT_STATUS_NOMEM,                 /* 内存不足 */
  BT_STATUS_BUSY,                  /* 忙碌 */
  BT_STATUS_DONE, /* request already completed */ /* 请求已完成 */
  BT_STATUS_UNSUPPORTED,           /* 不支持 */
  BT_STATUS_PARM_INVALID,          /* 参数无效 */
  BT_STATUS_UNHANDLED,             /* 未处理 */
  BT_STATUS_AUTH_FAILURE,          /* 认证失败 */
  BT_STATUS_RMT_DEV_DOWN,          /* 远端设备已下线 */
  BT_STATUS_AUTH_REJECTED,         /* 认证被拒绝 */
  BT_STATUS_JNI_ENVIRONMENT_ERROR, /* JNI环境错误 */
  BT_STATUS_JNI_THREAD_ATTACH_ERROR, /* JNI线程附着错误 */
  BT_STATUS_WAKELOCK_ERROR         /* 唤醒锁错误 */
} bt_status_t;

inline std::string bt_status_text(const bt_status_t& status) {
  switch (status) {
    case BT_STATUS_SUCCESS:
      return std::string("success");
    case BT_STATUS_FAIL:
      return std::string("fail");
    case BT_STATUS_NOT_READY:
      return std::string("not_ready");
    case BT_STATUS_NOMEM:
      return std::string("no_memory");
    case BT_STATUS_BUSY:
      return std::string("busy");
    case BT_STATUS_DONE:
      return std::string("already_done");
    case BT_STATUS_UNSUPPORTED:
      return std::string("unsupported");
    case BT_STATUS_PARM_INVALID:
      return std::string("parameter_invalid");
    case BT_STATUS_UNHANDLED:
      return std::string("unhandled");
    case BT_STATUS_AUTH_FAILURE:
      return std::string("failure");
    case BT_STATUS_RMT_DEV_DOWN:
      return std::string("remote_device_down");
    case BT_STATUS_AUTH_REJECTED:
      return std::string("rejected");
    case BT_STATUS_JNI_ENVIRONMENT_ERROR:
      return std::string("jni_env_error");
    case BT_STATUS_JNI_THREAD_ATTACH_ERROR:
      return std::string("jni_thread_error");
    case BT_STATUS_WAKELOCK_ERROR:
      return std::string("wakelock_error");
    default:
      return std::string("UNKNOWN");
  }
}

/** Bluetooth HCI Error Codes */
/** Corresponding to [Vol 2] Part D, "Error Codes" of Core_v5.1 specs */
typedef uint8_t bt_hci_error_code_t;

/** Bluetooth PinKey Code */
/* 传统蓝牙配对PIN码结构体，最长16字节 */
typedef struct { uint8_t pin[16]; } __attribute__((packed)) bt_pin_code_t;

/* 蓝牙能耗统计信息结构体，由协议栈上报 */
typedef struct {
  uint8_t status;      /* 状态码，0表示数据有效 */
  uint8_t ctrl_state;   /* stack reported state - 控制器当前状态(Active/Scan/Idle) */
  uint64_t tx_time;     /* in ms - 发送耗时(毫秒) */
  uint64_t rx_time;     /* in ms - 接收耗时(毫秒) */
  uint64_t idle_time;   /* in ms - 空闲耗时(毫秒) */
  uint64_t energy_used; /* a product of mA, V and ms - 消耗能量(mA*V*ms) */
} __attribute__((packed)) bt_activity_energy_info;

typedef struct {
  int32_t app_uid;  /* 应用UID */
  uint64_t tx_bytes; /* 发送字节数 */
  uint64_t rx_bytes; /* 接收字节数 */
} __attribute__((packed)) bt_uid_traffic_t;

typedef struct {
  RawAddress addr = RawAddress::kEmpty;          /* 远端设备地址 */
  bluetooth::Uuid uuid = bluetooth::Uuid::kEmpty; /* 服务UUID */
  uint32_t resourceId = 0;                       /* 资源标识ID */
  uint16_t interval = 0;                         /* 资源事件上报间隔 */
} bt_sensing_info_t;

/** Bluetooth Adapter Discovery state */
typedef enum {
  BT_DISCOVERY_STOPPED, /* 设备发现已停止 */
  BT_DISCOVERY_STARTED  /* 设备发现进行中 */
} bt_discovery_state_t;

/** Bluetooth ACL connection state */
typedef enum {
  BT_ACL_STATE_CONNECTED,   /* ACL链路已连接 */
  BT_ACL_STATE_DISCONNECTED /* ACL链路已断开 */
} bt_acl_state_t;

/** Bluetooth SDP service record */
typedef struct {
  bluetooth::Uuid uuid;    /* 服务UUID */
  uint16_t channel;        /* RFCOMM通道号 */
  char name[256];  // what's the maximum length
} bt_service_record_t;

/** Bluetooth RemoteVersion info */
typedef struct {
  int version;      /* 远端设备LMP版本 */
  int sub_ver;      /* 远端设备LMP子版本 */
  int manufacturer; /* 远端设备制造商ID */
} bt_remote_version_t;

typedef struct {
  uint16_t version_supported;               /* 支持的LE控制器版本 */
  uint8_t local_privacy_enabled;            /* 本地隐私(地址解析)使能标志 */
  uint8_t max_adv_instance;                 /* 最大广播实例数 */
  uint8_t rpa_offload_supported;            /* RPA(可解析私有地址)卸载支持 */
  uint8_t max_irk_list_size;                /* 最大IRK列表大小 */
  uint8_t max_adv_filter_supported;         /* 最大广播过滤器数 */
  uint8_t activity_energy_info_supported;   /* 是否支持能耗信息统计 */
  uint16_t scan_result_storage_size;        /* 扫描结果存储大小 */
  uint16_t total_trackable_advertisers;     /* 可跟踪广播设备总数 */
  bool extended_scan_support;               /* 是否支持扩展扫描 */
  bool debug_logging_supported;             /* 是否支持调试日志 */
  bool le_2m_phy_supported;                 /* 是否支持LE 2M PHY */
  bool le_coded_phy_supported;              /* 是否支持LE Coded PHY */
  bool le_extended_advertising_supported;   /* 是否支持LE扩展广播 */
  bool le_periodic_advertising_supported;   /* 是否支持LE周期广播 */
  uint16_t le_maximum_advertising_data_length; /* LE最大广播数据长度 */
  uint32_t dynamic_audio_buffer_supported;  /* 是否支持动态音频缓冲 */
} bt_local_le_features_t;

/* Stored the default/maximum/minimum buffer time for dynamic audio buffer.
 * For A2DP offload usage, the unit is millisecond.
 * For A2DP legacy usage, the unit is buffer queue size*/
typedef struct {
  uint16_t default_buffer_time;
  uint16_t maximum_buffer_time;
  uint16_t minimum_buffer_time;
} bt_dynamic_audio_buffer_type_t;

typedef struct {
  bt_dynamic_audio_buffer_type_t dab_item[32];
} bt_dynamic_audio_buffer_item_t;

/** Bluetooth Bond state */
typedef enum {
  CLOUD_BOND_NONE = 1,       /* 云端未绑定 */
  CLOUD_BOND_BONDING = 2,    /* 云端绑定中 */
  CLOUD_BOND_BONDED = 3,     /* 云端已绑定 */
  CLOUD_BOND_REMOVING = 4,   /* 云端解绑中 */
  CLOUD_BOND_CREATE_BOND = 5 /* 云端发起绑定 */
} bt_cloud_bond_state_t;

#define ADV_INFO_BYTE_ARRY_LEN_49 49
#define ADV_INFO_BYTE_ARRY_LEN_16 16
/* Bluetooth Adapter and Remote Device property types */
/* 蓝牙适配器与远端设备的属性类型枚举 */
typedef enum {
  /* Properties common to both adapter and remote device */
  /* 适配器和远端设备共有的属性 */
  /**
   * Description - Bluetooth Device Name
   * Access mode - Adapter name can be GET/SET. Remote device can be GET
   * Data type   - bt_bdname_t
   */
  /* 蓝牙设备名称：适配器名称可读/写，远端设备名称仅可读 */
  BT_PROPERTY_BDNAME = 0x1,
  /**
   * Description - Bluetooth Device Address
   * Access mode - Only GET.
   * Data type   - RawAddress
   */
  /* 蓝牙设备地址：仅可读，数据类型为RawAddress */
  BT_PROPERTY_BDADDR,
  /* 蓝牙服务128位UUID列表：仅可读，数据为bluetooth::Uuid数组，数组大小由属性长度推算 */
  BT_PROPERTY_UUIDS,
  /**
   * Description - Bluetooth Class of Device as found in Assigned Numbers
   * Access mode - Only GET.
   * Data type   - uint32_t.
   */
  /* 设备类别(CoD)：仅可读，取值见Assigned Numbers */
  BT_PROPERTY_CLASS_OF_DEVICE,
  /**
   * Description - Device Type - BREDR, BLE or DUAL Mode
   * Access mode - Only GET.
   * Data type   - bt_device_type_t
   */
  /* 设备类型(经典/BLE/双模)：仅可读 */
  BT_PROPERTY_TYPE_OF_DEVICE,
  /**
   * Description - Bluetooth Service Record
   * Access mode - Only GET.
   * Data type   - bt_service_record_t
   */
  /* 蓝牙服务记录(SDP)：仅可读 */
  BT_PROPERTY_SERVICE_RECORD,

  /* Properties unique to adapter */
  /* 仅适配器特有的属性 */
  /**
   * Description - Bluetooth Adapter scan mode
   * Access mode - GET and SET
   * Data type   - bt_scan_mode_t.
   */
  /* 适配器扫描模式(可见性/可连接性)：可读/写 */
  BT_PROPERTY_ADAPTER_SCAN_MODE,
  /* 已绑定设备地址列表：仅可读，数据为远端设备RawAddress数组，数组大小由属性长度推算 */
  BT_PROPERTY_ADAPTER_BONDED_DEVICES,
  /**
   * Description - Bluetooth Adapter Discovery timeout (in seconds)
   * Access mode - GET and SET
   * Data type   - uint32_t
   */
  /* 适配器可发现超时时间(秒)：可读/写 */
  BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,

  /* Properties unique to remote device */
  /* 仅远端设备特有的属性 */
  /**
   * Description - User defined friendly name of the remote device
   * Access mode - GET and SET
   * Data type   - bt_bdname_t.
   */
  /* 远端设备自定义友好名称：可读/写 */
  BT_PROPERTY_REMOTE_FRIENDLY_NAME,
  /**
   * Description - RSSI value of the inquired remote device
   * Access mode - Only GET.
   * Data type   - int8_t.
   */
  /* 被查询远端设备的RSSI信号强度：仅可读 */
  BT_PROPERTY_REMOTE_RSSI,
  /**
   * Description - Remote version info
   * Access mode - SET/GET.
   * Data type   - bt_remote_version_t.
   */
  /* 远端设备版本信息(LMP版本/制造商等)：可读/写 */
  BT_PROPERTY_REMOTE_VERSION_INFO,

  /**
   * Description - Local LE features
   * Access mode - GET.
   * Data type   - bt_local_le_features_t.
   */
  /* 本地LE(低功耗蓝牙)特性：仅可读 */
  BT_PROPERTY_LOCAL_LE_FEATURES,

  /**
   * Description - Local Input/Output Capabilities for classic Bluetooth
   * Access mode - GET and SET
   * Data Type - bt_io_cap_t.
   */
  /* 经典蓝牙本地输入/输出能力：可读/写 */
  BT_PROPERTY_LOCAL_IO_CAPS,

  /**
   * Description - Local Input/Output Capabilities for BLE
   * Access mode - GET and SET
   * Data Type - bt_io_cap_t.
   */
  /* BLE本地输入/输出能力：可读/写 */
  BT_PROPERTY_LOCAL_IO_CAPS_BLE,

  BT_PROPERTY_DYNAMIC_AUDIO_BUFFER,

  /**
   * Description - Whether remote device supports Secure Connections mode
   * Access mode - GET and SET.
   * Data Type - uint8_t.
   */
  BT_PROPERTY_REMOTE_SECURE_CONNECTIONS_SUPPORTED,

  /**
   * Description - Maximum observed session key for remote device
   * Access mode - GET and SET.
   * Data Type - uint8_t.
   */
  BT_PROPERTY_REMOTE_MAX_SESSION_KEY_SIZE,

  BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP = 0xFF,
  BT_PROPERTY_REMOTE_MANUFACTURE_HW = 0x100,
  BT_PROPERTY_SCAN_UUIDS_HW = 0x101,
  BT_PROPERTY_AUTO_CONNECT_SWITCH = 0x102,
  BT_PROPERTY_TWS_CAP = 0x103,
  BT_PROPERTY_ICARRY = 0x104,
  BT_PROPERTY_ICARRY_GROUP_DEVICE = 0x105,
  BT_PROPERTY_ClOUD_CAP = 0x200,
  BT_PROPERTY_REMOTE_LINKKEY = 0x201,
  BT_PROPERTY_VENDOR_ID = 0x202,
  BT_PROPERTY_PRODUCT_ID = 0x203,
  BT_PROPERTY_HID_VERSION = 0x204,
  BT_PROPERTY_HIECHO = 0x205,
  BT_PROPERTY_TIMESTAMP = 0x206,
  BT_PROPERTY_NAME_CHANGE_VENDOR_ID = 0x207,
  BT_PROPERTY_NAME_CHANGE_PRODUCT_ID = 0x208,
  BT_PROPERTY_NAME_CHANGE_VERSION = 0x209,
  BT_PROPERTY_DEVICE_INFO = 0x20A,

  /**
   * Description - Remote device model id
   * Access mode - GET and SET
   * Data Type - uint32_t.
   */
  BT_PROPERTY_REMOTE_MODEL_ID = 0X20B,

  /**
   * Description - Remote device new model id
   * Access mode - GET and SET
   * Data Type - uint32_t.
   */
  BT_PROPERTY_REMOTE_NEW_MODEL_ID = 0X20C,

  /**
   * Description - Remote device sub model id
   * Access mode - GET and SET
   * Data Type - uint8_t.
   */
  BT_PROPERTY_REMOTE_SUB_MODEL_ID = 0X20D,

  /**
   * Description - Remote device nearby features
   * Access mode - GET and SET
   * Data Type - uint8_t.
   */
  BT_PROPERTY_REMOTE_NEARBY_FEATURE = 0X20E,

  /**
   * Description - Remote adv key for security BLE adv
   * Access mode - GET and SET
   * Data Type - advKey.
   */
  HW_BT_PROPERTY_REMOTE_ADV_KEY = 0x20F,

  /**
   * Description - Remote device is hid device
   * Access mode - GET and SET
   * Data Type - uint8_t.
   */
  HW_BT_PROPERTY_IS_HID_DEVICE = 0x210,
  HW_BT_PROPERTY_ABS_VOLUM_KEY = 0x211,
  HW_BT_PROPERTY_RMT_IO_CAP_KEY = 0x212,
  HW_BT_PROPERTY_A2DP_OWNED_SHARE_CAP_KEY = 0x213,
  HW_BT_PROPERTY_HW_DEV_TYPE_KEY = 0x214,
  HW_BT_PROPERTY_CUSTOM_TYPE = 0x215,
  BT_PROPERTY_CONNECTION_TIME = 0x216,
  BT_PROPERTY_DEVICE_TYPE_ID = 0x217,
} bt_property_type_t;

/** Bluetooth Adapter Property data structure */
typedef struct {
  bt_property_type_t type; /* 属性类型 */
  int len;                 /* 属性数据长度(字节) */
  void* val;               /* 属性数据指针 */
} bt_property_t;

// OOB_ADDRESS_SIZE is 6 bytes address + 1 byte address type
#define OOB_ADDRESS_SIZE 7
#define OOB_C_SIZE 16
#define OOB_R_SIZE 16
#define OOB_NAME_MAX_SIZE 256
// Classic
#define OOB_DATA_LEN_SIZE 2
#define OOB_COD_SIZE 3
// LE
#define OOB_TK_SIZE 16
#define OOB_LE_FLAG_SIZE 1
#define OOB_LE_ROLE_SIZE 1
#define OOB_LE_APPEARANCE_SIZE 2
/** Represents the actual Out of Band data itself */
typedef struct {
  // Both
  bool is_valid = false; /* Default to invalid data; force caller to verify */
  uint8_t address[OOB_ADDRESS_SIZE];
  uint8_t c[OOB_C_SIZE];      /* Simple Pairing Hash C-192/256 (Classic or LE) */
  uint8_t r[OOB_R_SIZE];      /* Simple Pairing Randomizer R-192/256 (Classic or LE) */
  uint8_t device_name[OOB_NAME_MAX_SIZE]; /* Name of the device */

  // Classic
  uint8_t oob_data_length[OOB_DATA_LEN_SIZE]; /* Classic only data Length. Value includes this
                                 in length */
  uint8_t class_of_device[OOB_COD_SIZE]; /* Class of Device (Classic or LE) */

  // LE
  uint8_t le_device_role;   /* Supported and preferred role of device */
  uint8_t sm_tk[OOB_TK_SIZE];        /* Security Manager TK Value (LE Only) */
  uint8_t le_flags;         /* LE Flags for discoverability and features */
  uint8_t le_appearance[OOB_LE_APPEARANCE_SIZE]; /* For the appearance of the device */
} bt_oob_data_t;

/** Bluetooth Device Type */
typedef enum {
  BT_DEVICE_DEVTYPE_BREDR = 0x1, /* 经典蓝牙(BR/EDR)设备 */
  BT_DEVICE_DEVTYPE_BLE,         /* 低功耗蓝牙(BLE)设备 */
  BT_DEVICE_DEVTYPE_DUAL         /* 双模(BR/EDR + BLE)设备 */
} bt_device_type_t;

/** Bluetooth Bond state */
typedef enum {
  BT_BOND_STATE_NONE,    /* 未绑定 */
  BT_BOND_STATE_BONDING, /* 绑定中 */
  BT_BOND_STATE_BONDED   /* 已绑定 */
} bt_bond_state_t;

/** Bluetooth SSP Bonding Variant */
typedef enum {
  BT_SSP_VARIANT_PASSKEY_CONFIRMATION, /* 密钥数字确认 */
  BT_SSP_VARIANT_PASSKEY_ENTRY,        /* 密钥输入 */
  BT_SSP_VARIANT_CONSENT,              /* 用户同意(Just Works) */
  BT_SSP_VARIANT_PASSKEY_NOTIFICATION  /* 密钥通知 */
} bt_ssp_variant_t;

#define BT_MAX_NUM_UUIDS 32

/** Bluetooth Resource Sensing Event */
typedef enum {
  INVALID_SENSING_EVENT = 0x00,                 /* 无效感知事件 */
  SPP_CLIENT_CONNECT_DONE = 0x01,               /* SPP客户端连接完成 */
  SPP_SERVER_CONNECT_DONE = 0x02,               /* SPP服务端连接完成 */
  SPP_PORT_CLOSE = 0x03,                        /* SPP端口关闭 */
  SPP_SERVER_CLOSE = 0x04,                      /* SPP服务端关闭 */
  SPP_SERVER_LISTEN_DONE = 0x05,                /* SPP服务端监听完成 */
  GATT_SERVER_REGISTER_DONE = 0x10,             /* GATT服务端注册完成 */
  GATT_SERVER_UNREGISTER_DONE = 0x11,           /* GATT服务端注销完成 */
  GATT_CLIENT_CONNECT_DONE = 0x012,             /* GATT客户端连接完成 */
  GATT_CLIENT_CONNECT_CLOSE = 0x13,             /* GATT客户端连接关闭 */
  GATT_SERVER_CONNECT_DONE = 0x14,              /* GATT服务端连接完成 */
  GATT_SERVER_CONNECT_CLOSE = 0x15,             /* GATT服务端连接关闭 */
  SOCKET_REGISTER_APPLICATION_PKGNAME = 0x20,   /* Socket注册应用包名 */
  GATT_REGISTER_APPLICATION_PKGNAME = 0x21,     /* GATT注册应用包名 */
  BLE_CONN_INTERVAL_UPDATE = 0X30,              /* BLE连接间隔更新 */
  BLE_ADV_STARTED = 0x40,                       /* BLE广播已启动 */
  BLE_ADV_STOPPED = 0x41,                       /* BLE广播已停止 */
  BLE_SCAN_STARTED_EVENT = 0x50,                /* BLE扫描已启动 */
  BLE_SCAN_STOPPED_EVENT = 0x51,                /* BLE扫描已停止 */
  BLE_SCAN_ENHANCE_MODE_STARTED_EVENT = 0x52,   /* BLE增强扫描模式已启动 */
  BLE_SCAN_ENHANCE_MODE_STOPPED_EVENT = 0x53,   /* BLE增强扫描模式已停止 */
  A2DP_CHANNEL_MODE_CHANGE = 0x60,              /* A2DP通道模式变化 */
  A2DP_POWER_LEVEL_CHANGE = 0x61,               /* A2DP功耗等级变化 */
} bt_sensing_event_t;

/** Bluetooth Resource Decision Event */
typedef enum {
  INVALID_DECISION_EVENT = 0x00, /* 无效决策事件 */
  BLE_FASTEST_CONN_DECISION = 0x01, /* BLE最快连接资源决策 */
} bt_decision_event_t;

typedef enum {
  CONNECTION_REJECT = 0x00,             /* 拒绝连接 */
  CONNECTION_ACCEPT,                    /* 接受连接 */
  CONNECTION_ACCEPT_WITH_PARAM_ADJUST,  /* 接受连接并调整连接参数 */
  CONNECTION_PREEMPTION,                /* 抢占连接 */
} bt_resource_decision_result_t;

/** Bluetooth Interface callbacks */
/* 蓝牙接口回调函数定义集合 */

/** Bluetooth Enable/Disable Callback. */
/* 蓝牙使能/去使能状态变化回调 */
typedef void (*adapter_state_changed_callback)(bt_state_t state);

/* 适配器属性获取/设置结果回调
 * TODO: 若同一适配器属性存在并发的GET/SET调用，可能需要会话标识符来关联
 * 调用与回调；若该问题由上层框架处理，则无需在此管理会话 */
typedef void (*adapter_properties_callback)(bt_status_t status,
                                            int num_properties,
                                            bt_property_t* properties);

/* 远端设备属性获取/设置结果回调
 * TODO: 远端设备属性无需批量获取/设置，num_properties应为1 */
typedef void (*remote_device_properties_callback)(bt_status_t status,
                                                  RawAddress* bd_addr,
                                                  int num_properties,
                                                  bt_property_t* properties);

/* 发现新设备回调：若无EIR数据，则BD_NAME为NULL且RSSI为-1 */
typedef void (*device_found_callback)(int num_properties,
                                      bt_property_t* properties);

/* 设备发现(扫描)状态变化回调 */
typedef void (*discovery_state_changed_callback)(bt_discovery_state_t state);

/* 传统配对PIN码请求回调，min_16_digit指示是否要求16位数字PIN码 */
typedef void (*pin_request_callback)(RawAddress* remote_bd_addr,
                                     bt_bdname_t* bd_name, uint32_t cod,
                                     bool min_16_digit);

/* SSP安全简易配对请求回调(Just Works及数字比较)：
 * 对于CONSENT及PASSKEY_ENTRY变体，pass_key应为0；
 * TODO: 具备显示能力的设备理论上无需此回调，为完整性协议栈仍需支持 */
typedef void (*ssp_request_callback)(RawAddress* remote_bd_addr,
                                     bt_bdname_t* bd_name, uint32_t cod,
                                     bt_ssp_variant_t pairing_variant,
                                     uint32_t pass_key);

/* 绑定状态变化回调：由create_bond/cancel_bond/remove_bond触发 */
typedef void (*bond_state_changed_callback)(bt_status_t status,
                                            RawAddress* remote_bd_addr,
                                            bt_bond_state_t state);

/** Bluetooth ACL connection state changed callback */
/* ACL链路连接状态变化回调，hci_reason为HCI层断连原因码，link_type为链路传输类型 */
typedef void (*acl_state_changed_callback)(bt_status_t status,
                                           RawAddress* remote_bd_addr,
                                           bt_acl_state_t state,
                                           bt_hci_error_code_t hci_reason,
                                           tBT_TRANSPORT link_type);

/** Bluetooth link quality report callback */
/* 蓝牙链路质量报告回调：包含时间戳、RSSI、信噪比及丢包/重传统计信息 */
typedef void (*link_quality_report_callback)(
    uint64_t timestamp, int report_id, int rssi, int snr,
    int retransmission_count, int packets_not_receive_count,
    int negative_acknowledgement_count);

/* JVM关联/解除关联事件类型 */
typedef enum { ASSOCIATE_JVM, DISASSOCIATE_JVM } bt_cb_thread_evt;

/** Thread Associate/Disassociate JVM Callback */
/* Callback that is invoked by the callback thread to allow upper layer to
 * attach/detach to/from the JVM */
/* 回调线程关联/解除关联JVM事件回调，供上层附加/脱离JVM环境 */
typedef void (*callback_thread_event)(bt_cb_thread_evt evt);

/** Bluetooth Test Mode Callback */
/* Receive any HCI event from controller. Must be in DUT Mode for this callback
 * to be received */
/* DUT测试模式回调：接收控制器上报的任意HCI事件，须处于DUT模式 */
typedef void (*dut_mode_recv_callback)(uint16_t opcode, uint8_t* buf,
                                       uint8_t len);

/* LE测试模式结果回调：由le_tx_test/le_rx_test/le_test_end触发，
 * num_packets仅在le_test_end命令结束时有效 */
typedef void (*le_test_mode_callback)(bt_status_t status, uint16_t num_packets);

/* 能耗信息获取回调：
 * ctrl_state-控制器状态(1-Active/2-Scan/3-Idle，0表示read_energy_info调用失败)
 * 时间字段单位为毫秒，能耗值与状态由控制器及API调用结果给出
 * uid_data为bt_uid_traffic_t数组，以app_uid为-1的元素结尾 */
typedef void (*energy_info_callback)(bt_activity_energy_info* energy_info,
                                     bt_uid_traffic_t* uid_data);

/** Callback invoked when OOB data is returned from the controller */
typedef void (*generate_local_oob_data_callback)(tBT_TRANSPORT transport,
                                                 bt_oob_data_t oob_data);

/* bluetooth resource sensing callback
 * This callback shall be invoked when sening event happened in stack,
 * and sensing data shall be callback to upper layer
 */
typedef void (*sensing_state_changed_callback)(
    uint8_t eventId, bt_sensing_info_t* info);

/** TODO: Add callbacks for Link Up/Down and other generic
 *  notifications/callbacks */

/** Bluetooth DM callback structure. */
typedef struct {
  /** set to sizeof(bt_callbacks_t) */
  size_t size; /* 结构体大小，用于版本兼容性检查 */
  adapter_state_changed_callback adapter_state_changed_cb; /* 蓝牙开关状态变化回调 */
  adapter_properties_callback adapter_properties_cb; /* 适配器属性变化回调 */
  remote_device_properties_callback remote_device_properties_cb; /* 远端设备属性变化回调 */
  device_found_callback device_found_cb; /* 发现新设备回调 */
  discovery_state_changed_callback discovery_state_changed_cb; /* 扫描/发现状态变化回调 */
  pin_request_callback pin_request_cb; /* 传统配对PIN码请求回调 */
  ssp_request_callback ssp_request_cb; /* SSP安全简易配对请求回调 */
  bond_state_changed_callback bond_state_changed_cb; /* 绑定状态变化回调 */
  acl_state_changed_callback acl_state_changed_cb; /* ACL连接状态变化回调 */
  callback_thread_event thread_evt_cb; /* 回调线程关联/解除关联JVM事件回调 */
  dut_mode_recv_callback dut_mode_recv_cb; /* DUT模式下HCI事件接收回调 */
  le_test_mode_callback le_test_mode_cb; /* LE测试模式结果回调 */
  energy_info_callback energy_info_cb; /* 能耗信息上报回调 */
  link_quality_report_callback link_quality_report_cb; /* 链路质量报告回调 */
  generate_local_oob_data_callback generate_local_oob_data_cb; /* 本地OOB数据生成结果回调 */
  sensing_state_changed_callback sensing_state_changed_cb; /* 资源感知状态变化回调 */
} bt_callbacks_t;

/* 闹钟到期回调函数类型，data为注册时传入的用户数据 */
typedef void (*alarm_cb)(void* data);
/* 设置唤醒闹钟callout：delay_millis为延时(毫秒)，should_wake指示是否唤醒设备 */
typedef bool (*set_wake_alarm_callout)(uint64_t delay_millis, bool should_wake,
                                       alarm_cb cb, void* data);
/* 获取唤醒锁callout，lock_name为锁名，返回0表示成功 */
typedef int (*acquire_wake_lock_callout)(const char* lock_name);
/* 释放唤醒锁callout，lock_name为锁名，返回0表示成功 */
typedef int (*release_wake_lock_callout)(const char* lock_name);

/* 协议栈所需的OS callout函数集，用于设置唤醒闹钟及持有/释放唤醒锁，
 * 通过bt_interface_t的set_os_callouts注入协议栈 */
typedef struct {
  /* 结构体大小，用于版本兼容性检查 */
  size_t size;

  /* 设置唤醒闹钟函数指针 */
  set_wake_alarm_callout set_wake_alarm;
  /* 获取唤醒锁函数指针 */
  acquire_wake_lock_callout acquire_wake_lock;
  /* 释放唤醒锁函数指针 */
  release_wake_lock_callout release_wake_lock;
} bt_os_callouts_t;

/* 备注：init/enable时默认不初始化任何Profile。当应用调用某Profile的'init' API时：
 * 1. 若蓝牙未使能，蓝牙核心将其标记为使能；待应用调用蓝牙'enable'时，在使能
 *    流程中调用相应协议栈API使能被标记的Profile，并通过adapter_properties_cb
 *    返回已使能Profile的UUID列表；
 * 2. 若蓝牙已使能，蓝牙核心直接调用协议栈Profile API初始化该Profile，并触发
 *    adapter_properties_cb返回包含新增Profile UUID的当前UUID列表。
 * Profile的'cleanup' API被调用时，执行与上述相反的流程。 */

/** Represents the standard Bluetooth DM interface. */
/* 蓝牙标准DM(设备管理)接口结构体，包含协议栈对外提供的全部管理API */
typedef struct {
  /** set to sizeof(bt_interface_t) */
  /* 结构体大小，用于版本兼容性检查 */
  size_t size;
  /* 初始化接口并注册回调：
   * guest_mode-访客模式(受限模式，此模式下新绑定的设备被标记为受限，退出时删除)
   * is_common_criteria_mode-通用准则(安全认证)模式
   * config_compare_result-通用准则模式下的配置校验结果
   * init_flags-运行期间不可变更的配置标志
   * is_atv-是否为TV类设备 */
  int (*init)(bt_callbacks_t* callbacks, bool guest_mode,
              bool is_common_criteria_mode, int config_compare_result,
              const char** init_flags, bool is_atv);

  /** Enable Bluetooth. */
  /* 使能蓝牙 */
  int (*enable)();

  /** Disable Bluetooth. */
  /* 去使能蓝牙 */
  int (*disable)(void);

  /** Closes the interface. */
  /* 关闭接口并释放资源 */
  void (*cleanup)(void);

  /** Get all Bluetooth Adapter properties at init */
  /* 初始化时获取适配器全部属性 */
  int (*get_adapter_properties)(void);

  /** Get Bluetooth Adapter property of 'type' */
  /* 获取指定类型的适配器属性 */
  int (*get_adapter_property)(bt_property_type_t type);

  /** Set Bluetooth Adapter property of 'type' */
  /* Based on the type, val shall be one of
   * RawAddress or bt_bdname_t or bt_scanmode_t etc
   */
  /* 设置指定类型的适配器属性，val类型由属性类型决定 */
  int (*set_adapter_property)(const bt_property_t* property);

  /** Get all Remote Device properties */
  /* 获取远端设备全部属性 */
  int (*get_remote_device_properties)(RawAddress* remote_addr);

  /** Get Remote Device property of 'type' */
  /* 获取指定类型的远端设备属性 */
  int (*get_remote_device_property)(RawAddress* remote_addr,
                                    bt_property_type_t type);

  /** Set Remote Device property of 'type' */
  /* 设置指定类型的远端设备属性 */
  int (*set_remote_device_property)(RawAddress* remote_addr,
                                    const bt_property_t* property);

  /** Get Remote Device's service record  for the given UUID */
  /* 获取远端设备指定UUID的服务记录 */
  int (*get_remote_service_record)(const RawAddress& remote_addr,
                                   const bluetooth::Uuid& uuid);

  /** Start SDP to get remote services */
  /* 发起SDP服务发现，获取远端设备服务列表 */
  int (*get_remote_services)(RawAddress* remote_addr);

  /** Start Discovery */
  /* 启动设备发现(查询) */
  int (*start_discovery)(void);

  /** Cancel Discovery */
  /* 取消设备发现(查询) */
  int (*cancel_discovery)(void);

  /** Create Bluetooth Bonding */
  /* 发起与远端设备的绑定(配对)，transport指定传输类型 */
  int (*create_bond)(const RawAddress* bd_addr, int transport);

  /** Create Bluetooth Bond using out of band data */
  /* 使用OOB(带外)数据发起绑定，p192_data/p256_data分别为192/256位OOB数据 */
  int (*create_bond_out_of_band)(const RawAddress* bd_addr, int transport,
                                 const bt_oob_data_t* p192_data,
                                 const bt_oob_data_t* p256_data);

  /** Remove Bond */
  /* 移除与远端设备的绑定 */
  int (*remove_bond)(const RawAddress* bd_addr);

  /** Cancel Bond */
  /* 取消正在进行的绑定流程 */
  int (*cancel_bond)(const RawAddress* bd_addr);

  /**
   * Get the connection status for a given remote device.
   * return value of 0 means the device is not connected,
   * non-zero return status indicates an active connection.
   */
  /* 获取远端设备连接状态：返回0表示未连接，非0表示已连接 */
  int (*get_connection_state)(const RawAddress* bd_addr);

  /* 传统配对PIN码应答：若accept为FALSE，则pin_len和pin_code应为0 */
  int (*pin_reply)(const RawAddress* bd_addr, uint8_t accept, uint8_t pin_len,
                   bt_pin_code_t* pin_code);

  /* SSP简易配对应答(Just Works/数字比较/密钥输入)：
   * 数字比较与CONSENT变体下passkey应为0；
   * 密钥输入变体下若accept为FALSE，则passkey应为0 */
  int (*ssp_reply)(const RawAddress* bd_addr, bt_ssp_variant_t variant,
                   uint8_t accept, uint32_t passkey);

  /** Get Bluetooth profile interface */
  /* 根据profile_id获取蓝牙Profile(协议)接口 */
  const void* (*get_profile_interface)(const char* profile_id);

  /** Bluetooth Test Mode APIs - Bluetooth must be enabled for these APIs */
  /* Configure DUT Mode - Use this mode to enter/exit DUT mode */
  /* 蓝牙测试模式API(须先使能蓝牙)：进入/退出DUT(被测设备)模式 */
  int (*dut_mode_configure)(uint8_t enable);

  /* Send any test HCI (vendor-specific) command to the controller. Must be in
   * DUT Mode */
  /* 向控制器发送测试HCI(厂商自定义)命令，须处于DUT模式 */
  int (*dut_mode_send)(uint16_t opcode, uint8_t* buf, uint8_t len);
  /** BLE Test Mode APIs */
  /* opcode MUST be one of: LE_Receiver_Test, LE_Transmitter_Test, LE_Test_End
   */
  /* BLE测试模式API：opcode须为LE接收/发送测试或测试结束命令 */
  int (*le_test_mode)(uint16_t opcode, uint8_t* buf, uint8_t len);

  /* 设置协议栈所需的OS callout(闹钟与唤醒锁)函数，应在init成功后立即调用 */
  int (*set_os_callouts)(bt_os_callouts_t* callouts);

  /** Read Energy info details - return value indicates BT_STATUS_SUCCESS or
   * BT_STATUS_NOT_READY Success indicates that the VSC command was sent to
   * controller
   */
  /* 读取能耗信息：返回BT_STATUS_SUCCESS表示VSC命令已发送至控制器 */
  int (*read_energy_info)();

  /**
   * Native support for metrics protobuf dumping. The dumping format will be
   * raw byte array
   *
   * @param output an externally allocated string to dump serialized protobuf
   */
  /* 指标(度量)数据protobuf序列化导出：output为外部分配的序列化结果字符串 */
  void (*dumpMetrics)(std::string* output);

  /**
   * Clear /data/misc/bt_config.conf and erase all stored connections
   */
  /* 清除蓝牙配置文件并删除所有已存储的连接信息 */
  int (*config_clear)(void);

  /* 重置设备互操作性数据库中的动态数据 */
  void (*interop_database_clear)(void);

  /* 为地址前len字节与addr匹配的远端设备新增互操作性规避项，
   * feature须为interop.h中interop_feature_t已定义的值 */
  void (*interop_database_add)(uint16_t feature, const RawAddress* addr,
                               size_t len);

  /**
   * Get the AvrcpTarget Service interface to interact with the Avrcp Service
   */
  /* 获取AVRCP目标端服务接口，用于与Avrcp服务交互 */
  bluetooth::avrcp::ServiceInterface* (*get_avrcp_service)(void);

  /**
   * Obfuscate Bluetooth MAC address into a PII free ID string
   *
   * @param address Bluetooth MAC address to be obfuscated
   * @return a string of uint8_t that is unique to this MAC address
   */
  /* 将蓝牙MAC地址混淆为不含PII(个人身份信息)的唯一ID字符串 */
  std::string (*obfuscate_address)(const RawAddress& address);

  /**
   * Get an incremental id for as primary key for Bluetooth metric and log
   *
   * @param address Bluetooth MAC address of Bluetooth device
   * @return int incremental Bluetooth id
   */
  int (*get_metric_id)(const RawAddress& address);

  /**
   * Set the dynamic audio buffer size to the Controller
   */
  int (*set_dynamic_audio_buffer_size)(int codec, int size);

  /**
   * Fetches the local Out of Band data.
   */
  int (*generate_local_oob_data)(tBT_TRANSPORT transport);
  void (*enable_bluetooth_highpower)(uint8_t highpwr_level); /* 设置蓝牙大功率等级 */
  void (*enable_fast_scan)(bool is_enabled); /* 使能/去使能快速扫描 */
  void (*setFastScan)(int level); /* 设置快速扫描等级 */

  /** Create Bluetooth Bonding */
  int (*create_acl_connection)(const RawAddress* bd_addr);

  /** Cleanup profile connection request queue */
  void (*cleanup_profiles)();

  /** Cleanup background connection request map */
  void (*clean_background_connection)();

  void (*send_antenna_status_msg)(bool isFixed);

  /* send ble start adv msg */
  void (*sendbleadvstartmsg)();

  /* send ble stop adv msg */
  void (*sendbleadvstopmsg)();

  /* register ranging ant switch callback for BLE ranging business */
  int (*register_ranging_ant_switch_callback)(void (*handler)(uint8_t macID, uint8_t antID));

  /* unregister ranging ant switch callback */
  void (*unregister_ranging_ant_switch_callback)();

  /* send ble antenna info query msg */
  void (*send_antenna_info_query_msg)();

  /* send adv scan msg */
  void (*send_ble_adv_msg)(bool isStarted);

  /* map BLE random pseudo address to BR/EDR identity address */
  bool (*get_br_address_by_ble_address)(RawAddress* random_pseudo, uint8_t* p_identity_addr_type);

  /* map BR/EDR identity address to BLE random pseudo address */
  bool (*get_ble_mac_by_br_mac)(RawAddress* bd_addr, uint8_t* p_addr_type);

  /* send bt wearable device connect/disconnect status msg to signalhub */
  void (*send_bt_wearable_status_msg)(bool isConnected);
} bt_interface_t;

#define BLUETOOTH_INTERFACE_STRING "bluetoothInterface"

#endif /* VENDOR_INCLUDE_BLUETOOTH_H */
