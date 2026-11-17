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
 * 文件说明：定义蓝牙 A2DP（Advanced Audio Distribution Profile，高级音频分发规范）
 * 协议栈的硬件抽象层（HAL）接口。内容包括：A2DP 连接状态与音频数据通路状态、
 * 支持的音频编解码器索引/优先级/采样率/采样位深/声道模式等能力枚举、
 * SCMST 内容保护状态、编解码配置结构体，以及 A2DP Source（音频源）和
 * A2DP Sink（音频接收端）两种角色的回调结构体与接口结构体。
 */

#ifndef VENDOR_INCLUDE_BT_AV_H
#define VENDOR_INCLUDE_BT_AV_H

#include <vector>

#include <hardware/bluetooth.h>

extern "C" { //__BEGIN_DECLS

/* Bluetooth AV connection states */
/* 蓝牙AV连接状态 */
typedef enum {
  BTAV_CONNECTION_STATE_DISCONNECTED = 0, /* 已断开 */
  BTAV_CONNECTION_STATE_CONNECTING, /* 连接中 */
  BTAV_CONNECTION_STATE_CONNECTED, /* 已连接 */
  BTAV_CONNECTION_STATE_DISCONNECTING /* 正在断开 */
} btav_connection_state_t;

/* Bluetooth AV datapath states */
/* 蓝牙AV音频数据通路状态 */
typedef enum {
  BTAV_AUDIO_STATE_REMOTE_SUSPEND = 0, /* 远端已挂起音频 */
  BTAV_AUDIO_STATE_STOPPED, /* 音频已停止 */
  BTAV_AUDIO_STATE_STARTED, /* 音频已启动 */
  BTAV_AUDIO_STATE_LATENCY_CHANGED, /* 音频时延已变化 */
} btav_audio_state_t;

/*
 * Enum values for each A2DP supported codec.
 * There should be a separate entry for each A2DP codec that is supported
 * for encoding (SRC), and for decoding purpose (SINK).
 */
/* A2DP支持的编解码器索引枚举（源端SRC用于编码，终端SINK用于解码） */
typedef enum {
  BTAV_A2DP_CODEC_INDEX_SOURCE_MIN = 0, /* 源端编解码器索引最小值 */

  // Add an entry for each source codec here.
  // NOTE: The values should be same as those listed in the following file:
  //   BluetoothCodecConfig.java
  BTAV_A2DP_CODEC_INDEX_SOURCE_SBC = 0, /* SBC编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_AAC, /* AAC编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX, /* aptX编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_HD, /* aptX HD编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC, /* LDAC编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_LHDC, /* LHDC编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_LHDC_LL, /* LHDC低时延编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_LC3, /* LC3编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC, /* L2HC编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_LC3_LL, /* LC3低时延编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_V2, /* L2HC V2编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST, /* L2HC ST编解码器（源端） */
  BTAV_A2DP_CODEC_INDEX_SOURCE_MAX, /* 源端编解码器索引最大值 */

  BTAV_A2DP_CODEC_INDEX_SINK_MIN = BTAV_A2DP_CODEC_INDEX_SOURCE_MAX, /* 终端编解码器索引最小值 */

  // Add an entry for each sink codec here
  BTAV_A2DP_CODEC_INDEX_SINK_SBC = BTAV_A2DP_CODEC_INDEX_SINK_MIN, /* SBC编解码器（终端） */
  BTAV_A2DP_CODEC_INDEX_SINK_AAC, /* AAC编解码器（终端） */
  BTAV_A2DP_CODEC_INDEX_SINK_LDAC, /* LDAC编解码器（终端） */
  BTAV_A2DP_CODEC_INDEX_SINK_L2HC, /* L2HC编解码器（终端） */
  BTAV_A2DP_CODEC_INDEX_SINK_MAX, /* 终端编解码器索引最大值 */

  BTAV_A2DP_CODEC_INDEX_MIN = BTAV_A2DP_CODEC_INDEX_SOURCE_MIN, /* 编解码器索引最小值 */
  BTAV_A2DP_CODEC_INDEX_MAX = BTAV_A2DP_CODEC_INDEX_SINK_MAX /* 编解码器索引最大值 */
} btav_a2dp_codec_index_t;

/* A2DP编解码器优先级枚举 */
typedef enum {
  // Disable the codec.
  // NOTE: This value can be used only during initialization when
  // function btav_source_interface_t::init() is called.
  BTAV_A2DP_CODEC_PRIORITY_DISABLED = -1, /* 禁用该编解码器（仅在初始化时可用） */

  // Reset the codec priority to its default value.
  BTAV_A2DP_CODEC_PRIORITY_DEFAULT = 0, /* 将编解码器优先级恢复为默认值 */
  A2DP_CODEC_PRIORITY_L2HCST = 8000, /* L2HC ST编解码器优先级 */
  // Highest codec priority.
  BTAV_A2DP_CODEC_PRIORITY_HIGHEST = 1000 * 1000 /* 最高编解码器优先级 */
} btav_a2dp_codec_priority_t;

/* A2DP编解码器采样率枚举（位掩码，可组合表示支持的多种采样率） */
typedef enum {
  BTAV_A2DP_CODEC_SAMPLE_RATE_NONE = 0x0, /* 无/不支持 */
  BTAV_A2DP_CODEC_SAMPLE_RATE_44100 = 0x1 << 0, /* 44.1kHz */
  BTAV_A2DP_CODEC_SAMPLE_RATE_48000 = 0x1 << 1, /* 48kHz */
  BTAV_A2DP_CODEC_SAMPLE_RATE_88200 = 0x1 << 2, /* 88.2kHz */
  BTAV_A2DP_CODEC_SAMPLE_RATE_96000 = 0x1 << 3, /* 96kHz */
  BTAV_A2DP_CODEC_SAMPLE_RATE_176400 = 0x1 << 4, /* 176.4kHz */
  BTAV_A2DP_CODEC_SAMPLE_RATE_192000 = 0x1 << 5, /* 192kHz */
  BTAV_A2DP_CODEC_SAMPLE_RATE_16000 = 0x1 << 6, /* 16kHz */
  BTAV_A2DP_CODEC_SAMPLE_RATE_24000 = 0x1 << 7 /* 24kHz */
} btav_a2dp_codec_sample_rate_t;

/* A2DP编解码器每采样位数（位深）枚举（位掩码，可组合表示支持的多种位深） */
typedef enum {
  BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE = 0x0, /* 无/不支持 */
  BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 = 0x1 << 0, /* 16位 */
  BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 = 0x1 << 1, /* 24位 */
  BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32 = 0x1 << 2 /* 32位 */
} btav_a2dp_codec_bits_per_sample_t;

/* A2DP编解码器声道模式枚举（位掩码，可组合表示支持的多种声道模式） */
typedef enum {
  BTAV_A2DP_CODEC_CHANNEL_MODE_NONE = 0x0, /* 无/不支持 */
  BTAV_A2DP_CODEC_CHANNEL_MODE_MONO = 0x1 << 0, /* 单声道 */
  BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO = 0x1 << 1 /* 立体声 */
} btav_a2dp_codec_channel_mode_t;

/* A2DP SCMST（串行拷贝管理系统）内容保护使能状态枚举 */
typedef enum {
  BTAV_A2DP_SCMST_DISABLED = 0x00, /* SCMST禁用 */
  BTAV_A2DP_SCMST_ENABLED = 0x01 /* SCMST使能 */
} btav_a2dp_scmst_enable_status_t;

/*
 * Structure for representing codec capability or configuration.
 * It is used for configuring A2DP codec preference, and for reporting back
 * current configuration or codec capability.
 * For codec capability, fields "sample_rate", "bits_per_sample" and
 * "channel_mode" can contain bit-masks with all supported features.
 */
/* 编解码器能力或配置结构体：用于配置A2DP编解码偏好，也用于回报当前配置或编解码能力 */
typedef struct {
  btav_a2dp_codec_index_t codec_type; /* 编解码器类型 */
  btav_a2dp_codec_priority_t
      codec_priority;  /* 编解码器优先级 */ // Codec selection priority
                       // relative to other codecs: larger value
                       // means higher priority. If 0, reset to
                       // default.
  btav_a2dp_codec_sample_rate_t sample_rate; /* 采样率 */
  btav_a2dp_codec_bits_per_sample_t bits_per_sample; /* 每采样位数（位深） */
  btav_a2dp_codec_channel_mode_t channel_mode; /* 声道模式 */
  int64_t codec_specific_1 = 0;  /* 编解码器特定值1 */ // Codec-specific value 1
  int64_t codec_specific_2 = 0;  /* 编解码器特定值2 */ // Codec-specific value 2
  uint64_t codec_specific_3 = 0;  /* 编解码器特定值3 */ // Codec-specific value 3
  int64_t codec_specific_4 = 0;  /* 编解码器特定值4 */ // Codec-specific value 4

  /* 将当前编解码配置信息转换为可读字符串 */
  std::string ToString() const {
    /* 编解码器名称字符串 */
    std::string codec_name_str;

    /* 根据编解码器类型索引映射为标准名称：先匹配源端(Source)编解码器 */
    switch (codec_type) {
      /* SBC编解码器(源端) */
      case BTAV_A2DP_CODEC_INDEX_SOURCE_SBC:
        codec_name_str = "SBC";
        break;
      /* AAC编解码器(源端) */
      case BTAV_A2DP_CODEC_INDEX_SOURCE_AAC:
        codec_name_str = "AAC";
        break;
      /* aptX编解码器(源端) */
      case BTAV_A2DP_CODEC_INDEX_SOURCE_APTX:
        codec_name_str = "aptX";
        break;
      /* aptX HD编解码器(源端) */
      case BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_HD:
        codec_name_str = "aptX HD";
        break;
      /* LDAC编解码器(源端) */
      case BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC:
        codec_name_str = "LDAC";
        break;
      /* LHDC编解码器(源端) */
      case BTAV_A2DP_CODEC_INDEX_SOURCE_LHDC:
        codec_name_str = "LHDC";
        break;
      /* LHDC低时延变体(源端) */
      case BTAV_A2DP_CODEC_INDEX_SOURCE_LHDC_LL:
        codec_name_str = "LHDC_LL";
        break;
      /* SBC编解码器(接收端，名称加Sink后缀区分) */
      case BTAV_A2DP_CODEC_INDEX_SINK_SBC:
        codec_name_str = "SBC (Sink)";
        break;
      /* AAC编解码器(接收端) */
      case BTAV_A2DP_CODEC_INDEX_SINK_AAC:
        codec_name_str = "AAC (Sink)";
        break;
      /* LDAC编解码器(接收端) */
      case BTAV_A2DP_CODEC_INDEX_SINK_LDAC:
        codec_name_str = "LDAC (Sink)";
        break;
      case BTAV_A2DP_CODEC_INDEX_SINK_L2HC:
        codec_name_str = "L2HCV1 (Sink)";
        break;
      case BTAV_A2DP_CODEC_INDEX_SOURCE_LC3:
        codec_name_str = "LC3";
        break;
      case BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC:
        codec_name_str = "L2HC";
        break;
      case BTAV_A2DP_CODEC_INDEX_SOURCE_LC3_LL:
        codec_name_str = "LC3_LL";
        break;
      case BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_V2:
        codec_name_str = "L2HCV2";
        break;
      case BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST:
        codec_name_str = "L2HCST";
        break;
      /* 索引超出已定义范围（最大边界值），视为未知编解码器 */
      case BTAV_A2DP_CODEC_INDEX_MAX:
        codec_name_str = "Unknown(CODEC_INDEX_MAX)";
        break;
    }

    /* 拼接采样率能力列表：NONE用'=='判断（表示完全不支持采样率能力），
     * 其余采样率按位掩码'&'逐项匹配，命中则以'|'分隔追加显示 */
    std::string sample_rate_str; /* 采样率能力列表字符串 */
    AppendCapability(sample_rate_str,
                     (sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_NONE),
                     "NONE"); /* 无采样率能力 */
    AppendCapability(sample_rate_str,
                     (sample_rate & BTAV_A2DP_CODEC_SAMPLE_RATE_44100),
                     "44100"); /* 44.1kHz */
    AppendCapability(sample_rate_str,
                     (sample_rate & BTAV_A2DP_CODEC_SAMPLE_RATE_48000),
                     "48000"); /* 48kHz */
    AppendCapability(sample_rate_str,
                     (sample_rate & BTAV_A2DP_CODEC_SAMPLE_RATE_88200),
                     "88200"); /* 88.2kHz */
    AppendCapability(sample_rate_str,
                     (sample_rate & BTAV_A2DP_CODEC_SAMPLE_RATE_96000),
                     "96000"); /* 96kHz */
    AppendCapability(sample_rate_str,
                     (sample_rate & BTAV_A2DP_CODEC_SAMPLE_RATE_176400),
                     "176400"); /* 176.4kHz */
    AppendCapability(sample_rate_str,
                     (sample_rate & BTAV_A2DP_CODEC_SAMPLE_RATE_192000),
                     "192000"); /* 192kHz */
    AppendCapability(sample_rate_str,
                     (sample_rate & BTAV_A2DP_CODEC_SAMPLE_RATE_16000),
                     "16000"); /* 16kHz */
    AppendCapability(sample_rate_str,
                     (sample_rate & BTAV_A2DP_CODEC_SAMPLE_RATE_24000),
                     "24000"); /* 24kHz */

    /* 拼接位深（每采样位数）能力列表：NONE用'=='判断（表示完全不支持位深能力），
     * 其余位深按位掩码'&'逐项匹配 */
    std::string bits_per_sample_str; /* 位深能力列表字符串 */
    AppendCapability(bits_per_sample_str,
                     (bits_per_sample == BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE),
                     "NONE"); /* 无位深能力 */
    AppendCapability(bits_per_sample_str,
                     (bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16),
                     "16"); /* 16位 */
    AppendCapability(bits_per_sample_str,
                     (bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24),
                     "24"); /* 24位 */
    AppendCapability(bits_per_sample_str,
                     (bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
                     "32"); /* 32位 */

    /* 拼接声道模式能力列表：NONE用'=='判断（表示完全不支持声道模式能力），
     * MONO/STEREO按位掩码'&'逐项匹配 */
    std::string channel_mode_str; /* 声道模式能力列表字符串 */
    AppendCapability(channel_mode_str,
                     (channel_mode == BTAV_A2DP_CODEC_CHANNEL_MODE_NONE),
                     "NONE"); /* 无声道模式能力 */
    AppendCapability(channel_mode_str,
                     (channel_mode & BTAV_A2DP_CODEC_CHANNEL_MODE_MONO),
                     "MONO"); /* 单声道 */
    AppendCapability(channel_mode_str,
                     (channel_mode & BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO),
                     "STEREO"); /* 立体声 */

    /* 将以上各项配置汇总为一条可读的日志字符串并返回 */
    return "codec: " + codec_name_str + /* 编解码器名称 */
           " priority: " + std::to_string(codec_priority) + /* 编解码器优先级 */
           " sample_rate: " + sample_rate_str + /* 采样率 */
           " bits_per_sample: " + bits_per_sample_str + /* 位深 */
           " channel_mode: " + channel_mode_str + /* 声道模式 */
           " codec_specific_1: " + std::to_string(codec_specific_1) + /* 编解码器特定值1 */
           " codec_specific_2: " + std::to_string(codec_specific_2) + /* 编解码器特定值2 */
           " codec_specific_3: " + std::to_string(codec_specific_3) + /* 编解码器特定值3 */
           " codec_specific_4: " + std::to_string(codec_specific_4); /* 编解码器特定值4 */
  }

 private:
  /* 按需将能力名称追加到结果字符串：
   * append为false时不做任何修改直接返回原字符串；
   * 为true时以'|'分隔追加name(首项前不加分隔符)，
   * 用于拼接采样率/位深/声道模式等多值能力列表 */
  static std::string AppendCapability(std::string& result, bool append,
                                      const std::string& name) {
    /* 能力未命中，直接返回原字符串 */
    if (!append) return result;
    /* 已有内容时先追加分隔符'|' */
    if (!result.empty()) result += "|";
    /* 追加能力名称 */
    result += name;
    return result;
  }
} btav_a2dp_codec_config_t;

/* A2DP SCMST内容保护信息结构体 */
typedef struct {
  btav_a2dp_scmst_enable_status_t enable_status; /* SCMST使能状态 */
  uint8_t cp_header; /* 内容保护（CP）头字节 */
} btav_a2dp_scmst_info_t;

/** Callback for connection state change.
 *  state will have one of the values from btav_connection_state_t
 */
typedef void (*btav_connection_state_callback)(const RawAddress& bd_addr,
                                                btav_connection_state_t state); /* A2DP连接状态变化回调 */

/** Callback for audiopath state change.
 *  state will have one of the values from btav_audio_state_t
 */
typedef void (*btav_audio_state_callback)(const RawAddress& bd_addr,
                                           btav_audio_state_t state); /* A2DP音频数据通路状态变化回调 */

/** Callback for audio configuration change.
 *  Used only for the A2DP Source interface.
 */
typedef void (*btav_audio_source_config_callback)(
    const RawAddress& bd_addr, btav_a2dp_codec_config_t codec_config,
    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities,
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities); /* 音频配置变化回调（仅A2DP Source使用） */

/**
 * Callback for audio configuration change.
 *  Used only for the A2DP Sink interface.
 *
 * @param bd_addr remote device address
 * @param sample_rate current sample rate in Hz
 * @param channel_count number of channels: 1 for mono, 2 for stereo
 */
/* 音频配置变化回调（仅A2DP Sink使用）：上报当前采样率(Hz)及声道数(1单声道/2立体声) */
typedef void (*btav_audio_sink_config_callback)(const RawAddress& bd_addr,
                                                uint32_t sample_rate,
                                                uint8_t channel_count); /* 音频配置变化回调（仅A2DP Sink使用） */

/** Callback for querying whether the mandatory codec is more preferred.
 *  Used only for the A2DP Source interface.
 *  Return true if optional codecs are not preferred.
 */
typedef bool (*btav_mandatory_codec_preferred_callback)(
    const RawAddress& bd_addr); /* 查询是否更偏好强制编解码器（仅A2DP Source使用），返回true表示不偏好可选编解码器 */

/** BT-AV A2DP Source callback structure. */
/* A2DP源端（Source）回调结构体 */
typedef struct {
  /** set to sizeof(btav_source_callbacks_t) */
  size_t size; /* 结构体大小，置为sizeof(btav_source_callbacks_t) */
  btav_connection_state_callback connection_state_cb; /* 连接状态变化回调 */
  btav_audio_state_callback audio_state_cb; /* 音频状态变化回调 */
  btav_audio_source_config_callback audio_config_cb; /* 音频配置变化回调 */
  btav_mandatory_codec_preferred_callback mandatory_codec_preferred_cb; /* 是否更偏好强制编解码器回调 */
} btav_source_callbacks_t;

/** BT-AV A2DP Sink callback structure. */
/* A2DP接收端（Sink）回调结构体 */
typedef struct {
  /** set to sizeof(btav_sink_callbacks_t) */
  size_t size; /* 结构体大小，置为sizeof(btav_sink_callbacks_t) */
  btav_connection_state_callback connection_state_cb; /* 连接状态变化回调 */
  btav_audio_state_callback audio_state_cb; /* 音频状态变化回调 */
  btav_audio_sink_config_callback audio_config_cb; /* 音频配置变化回调 */
} btav_sink_callbacks_t;

/* 备注：
 * 1. 初始仅支持AVRCP 1.0，AVRCP透传（passthrough）命令由协议栈内部通过uinput处理；
 * 2. A2DP音频数据通路通过socket管道在AudioFlinger的VENDOR_audio_hw音频库
 *    与蓝牙协议栈之间传输。 */

/** Represents the standard BT-AV A2DP Source interface.
 */
/* 标准BT-AV A2DP源端（Source）接口结构体 */
typedef struct {
  /** set to sizeof(btav_source_interface_t) */
  size_t size; /* 结构体大小，置为sizeof(btav_source_interface_t) */
  /**
   * Register the BtAv callbacks.
   */
  bt_status_t (*init)(
      btav_source_callbacks_t* callbacks, int max_connected_audio_devices,
      const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
      const std::vector<btav_a2dp_codec_config_t>& offloading_preference);  /* 注册回调并初始化A2DP Source接口 */

  /** connect to headset */
  bt_status_t (*connect)(const RawAddress& bd_addr);  /* 连接音频设备 */

  /** dis-connect from headset */
  bt_status_t (*disconnect)(const RawAddress& bd_addr);  /* 断开音频设备连接 */

  /** sets the connected device silence state */
  bt_status_t (*set_silence_device)(const RawAddress& bd_addr, bool silence);  /* 设置已连接设备的静音状态 */

  /** sets the connected device as active */
  bt_status_t (*set_active_device)(const RawAddress& bd_addr);  /* 将已连接设备设为活跃设备 */

  /** configure the codecs settings preferences */
  bt_status_t (*config_codec)(
      const RawAddress& bd_addr,
      std::vector<btav_a2dp_codec_config_t> codec_preferences);  /* 配置编解码器设置偏好 */

  /** Closes the interface. */
  void (*cleanup)(void);  /* 关闭接口并释放资源 */

} btav_source_interface_t;

/** Represents the standard BT-AV A2DP Sink interface.
 */
/* 标准BT-AV A2DP接收端（Sink）接口结构体 */
typedef struct {
  /** set to sizeof(btav_sink_interface_t) */
  size_t size; /* 结构体大小，置为sizeof(btav_sink_interface_t) */
  /**
   * Register the BtAv callbacks
   */
  bt_status_t (*init)(btav_sink_callbacks_t* callbacks,
                      int max_connected_audio_devices);  /* 注册回调并初始化A2DP Sink接口 */

  /** connect to headset */
  bt_status_t (*connect)(const RawAddress& bd_addr);  /* 连接音频设备 */

  /** dis-connect from headset */
  bt_status_t (*disconnect)(const RawAddress& bd_addr);  /* 断开音频设备连接 */

  /** Closes the interface. */
  void (*cleanup)(void);  /* 关闭接口并释放资源 */

  /** Sends Audio Focus State. */
  void (*set_audio_focus_state)(int focus_state);  /* 发送音频焦点状态 */

  /** Sets the audio track gain. */
  void (*set_audio_track_gain)(float gain);  /* 设置音频轨道增益 */

  /** sets the connected device as active */
  bt_status_t (*set_active_device)(const RawAddress& bd_addr);  /* 将已连接设备设为活跃设备 */
} btav_sink_interface_t;

} //__END_DECLS

#endif /* VENDOR_INCLUDE_BT_AV_H */
