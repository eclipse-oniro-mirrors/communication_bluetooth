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
 * 文件说明：BLE 扫描器（Scanner）协议栈硬件抽象接口定义。
 * 提供 LE 扫描的启动/停止、扫描参数设置、扫描过滤器（APCF）参数与
 * 条件配置、批量扫描（Batchscan）存储/使能/报告读取、周期广播同步
 * （Sync）等接口，并定义扫描结果、批量扫描报告、被跟踪广播设备
 * 事件等回调通知。
 */

#pragma once

#include <functional>
#include <stdint.h>
#include <vector>
#include "bt_common_types.h"
#include "bt_gatt_client.h"
#include "bt_gatt_types.h"

/** Callback invoked when batchscan reports are obtained */ /* 获取到批量扫描报告时的回调 */
typedef void (*batchscan_reports_callback)(int client_if, int status,
                                           int report_format, int num_records,
                                           std::vector<uint8_t> data);

/** Callback invoked when batchscan storage threshold limit is crossed */ /* 批量扫描存储阈值被超过时的回调 */
typedef void (*batchscan_threshold_callback)(int client_if);

/** Track ADV VSE callback invoked when tracked device is found or lost */ /* 被跟踪广播设备发现/丢失时的回调 */
typedef void (*track_adv_event_callback)(
    btgatt_track_adv_info_t* p_track_adv_info);

/**
 * Callback invoked for each scan result (advertising report).
 *
 * @param event_type advertising report event type (e.g. legacy /
 *                   extended ADV/SCAN_RSP)
 * @param addr_type advertiser address type (public/random, etc.)
 * @param bda advertiser device address
 * @param primary_phy primary advertising PHY (1M/2M/coded)
 * @param secondary_phy secondary advertising PHY (1M/2M/coded)
 * @param advertising_sid advertising set ID of the extended advertising
 * @param tx_power advertiser TX power in dBm, 127 if not reported
 * @param rssi received signal strength in dBm
 * @param periodic_adv_int periodic advertising interval, 0 if not
 *                         periodic advertising
 * @param adv_data raw advertising data payload
 */
/* 扫描结果回调：每收到一条广播报告触发一次，携带广播者地址及类型、
 * PHY、AID、发射功率、RSSI、周期广播间隔和原始广播数据 */
typedef void (*scan_result_callback)(uint16_t event_type, uint8_t addr_type,
                                     RawAddress* bda, uint8_t primary_phy,
                                     uint8_t secondary_phy,
                                     uint8_t advertising_sid, int8_t tx_power,
                                     int8_t rssi, uint16_t periodic_adv_int,
                                     std::vector<uint8_t> adv_data); /* 扫描结果回调 */

/* BLE 扫描回调集合结构体 */
typedef struct {
  scan_result_callback scan_result_cb; /* 扫描结果回调 */
  batchscan_reports_callback batchscan_reports_cb; /* 批量扫描报告回调 */
  batchscan_threshold_callback batchscan_threshold_cb; /* 批量扫描存储阈值回调 */
  track_adv_event_callback track_adv_event_cb; /* 被跟踪广播设备事件回调 */
} btgatt_scanner_callbacks_t;

class AdvertisingTrackInfo { /* 被跟踪广播设备信息 */
 public:
  uint8_t scanner_id; /* 扫描器 ID */
  uint8_t filter_index; /* 过滤器索引 */
  uint8_t advertiser_state; /* 广播设备状态 */
  uint8_t advertiser_info_present; /* 广播设备信息是否有效 */
  RawAddress advertiser_address; /* 广播设备地址 */
  uint8_t advertiser_address_type; /* 广播设备地址类型 */
  uint8_t tx_power; /* 发射功率 */
  int8_t rssi; /* 接收信号强度 */
  uint16_t time_stamp; /* 时间戳 */
  uint8_t adv_packet_len; /* 广播包长度 */
  std::vector<uint8_t> adv_packet; /* 广播包数据 */
  uint8_t scan_response_len; /* 扫描响应长度 */
  std::vector<uint8_t> scan_response; /* 扫描响应数据 */
};

/**
 * LE Scanning related callbacks invoked from from the Bluetooth native stack
 * All callbacks are invoked on the JNI thread
 */ /* LE 扫描相关回调，由蓝牙原生协议栈调用，均在 JNI 线程上触发 */
class ScanningCallbacks { /* 扫描回调接口，由上层实现并注册给协议栈 */
 public:
  virtual ~ScanningCallbacks() = default;
  virtual void OnScannerRegistered(const bluetooth::Uuid app_uuid,
                                   uint8_t scannerId, uint8_t status) = 0; /* 扫描器注册结果回调 */
  virtual void OnScanResult(uint16_t event_type, uint8_t addr_type,
                            RawAddress bda, uint8_t primary_phy,
                            uint8_t secondary_phy, uint8_t advertising_sid,
                            int8_t tx_power, int8_t rssi,
                            uint16_t periodic_adv_int,
                            std::vector<uint8_t> adv_data) = 0; /* 扫描结果回调 */
  virtual void OnTrackAdvFoundLost(
      AdvertisingTrackInfo advertising_track_info) = 0; /* 被跟踪广播设备发现/丢失回调 */
  virtual void OnBatchScanReports(int client_if, int status, int report_format,
                                  int num_records,
                                  std::vector<uint8_t> data) = 0; /* 批量扫描报告回调 */
  virtual void OnBatchScanThresholdCrossed(int client_if) = 0; /* 批量扫描存储阈值被超过回调 */
};

class BleScannerInterface { /* BLE 扫描器接口，由协议栈实现、供上层调用 */
 public:
  virtual ~BleScannerInterface() = default;

  using RegisterCallback =
      std::function<void(uint8_t /* scanner_id */, uint8_t /* status */)>; /* 扫描器注册结果回调 */

  using Callback = std::function<void(uint8_t /* status */)>; /* 通用操作结果回调 */

  using EnableCallback =
      std::function<void(uint8_t /* action */, uint8_t /* status */)>; /* 过滤器使能/去使能结果回调 */

  using FilterParamSetupCallback =
      std::function<void(uint8_t /* avbl_space */, uint8_t /* action_type */,
                          uint8_t /* status */)>;
/* 过滤器参数设置结果回调 */
  using FilterConfigCallback =
      std::function<void(uint8_t /* filt_type */, uint8_t /* avbl_space */,
                          uint8_t /* action */, uint8_t /* status */)>;
/* 过滤器条件配置结果回调 */
  /** Registers a scanner with the stack */ /* 向协议栈注册扫描器 */
  virtual void RegisterScanner(RegisterCallback) = 0;

  /** Unregister a scanner from the stack */ /* 从协议栈注销指定扫描器 */
  virtual void Unregister(int scanner_id) = 0;

  /** Start or stop LE device scanning */ /* 启动/停止 LE 扫描 */
  virtual void Scan(bool start) = 0;

  /** Setup scan filter params */ /* 设置扫描过滤器参数 */
  virtual void ScanFilterParamSetup(
      uint8_t client_if, uint8_t action, uint8_t filt_index,
      std::unique_ptr<btgatt_filt_param_setup_t> filt_param,
      FilterParamSetupCallback cb) = 0;

  /** Configure a scan filter condition  */ /* 配置扫描过滤条件 */
  virtual void ScanFilterAdd(int filter_index, std::vector<ApcfCommand> filters,
                             FilterConfigCallback cb) = 0;

  /** Clear all scan filter conditions for specific filter index*/ /* 清除指定过滤器索引的全部过滤条件 */
  virtual void ScanFilterClear(int filt_index, FilterConfigCallback cb) = 0;

  /** Enable / disable scan filter feature*/ /* 使能/去使能扫描过滤器功能 */
  virtual void ScanFilterEnable(bool enable, EnableCallback cb) = 0;

  /** Sets the LE scan interval and window in units of N*0.625 msec
   * OH fork signature: (interval, window, legacy, phy, callback) */ /* 设置 LE 扫描间隔与窗口，单位 N*0.625 毫秒 */
  virtual void SetScanParameters(uint32_t scan_interval, uint32_t scan_window, uint8_t legacy,
                                 uint8_t scan_phy, std::function<void(uint8_t status)> cb) = 0;

  /* Configure the batchscan storage */ /* 配置批量扫描存储参数 */
  virtual void BatchscanConfigStorage(int client_if, int batch_scan_full_max,
                                      int batch_scan_trunc_max,
                                      int batch_scan_notify_threshold,
                                      Callback cb) = 0;

  /* Enable batchscan */ /* 使能批量扫描 */
  virtual void BatchscanEnable(int scan_mode, int scan_interval,
                               int scan_window, int addr_type, int discard_rule,
                               Callback cb) = 0;

  /* Disable batchscan */ /* 去使能批量扫描 */
  virtual void BatchscanDisable(Callback cb) = 0;

  /* Read out batchscan reports */ /* 读出批量扫描报告 */
  virtual void BatchscanReadReports(int client_if, int scan_mode) = 0;

  using StartSyncCb =
      std::function<void(uint8_t status, uint16_t sync_handle,
                          uint8_t advertising_sid, uint8_t address_type,
                          RawAddress address, uint8_t phy, uint16_t interval)>; /* 周期广播同步建立结果回调 */
  using SyncReportCb =
      std::function<void(uint16_t sync_handle, int8_t tx_power, int8_t rssi,
                          uint8_t status, std::vector<uint8_t> data)>; /* 周期广播同步报告回调 */
  using SyncLostCb = std::function<void(uint16_t sync_handle)>; /* 周期广播同步丢失回调 */
  virtual void StartSync(uint8_t sid, RawAddress address, uint16_t skip,
                         uint16_t timeout, StartSyncCb start_cb,
                         SyncReportCb report_cb, SyncLostCb lost_cb) = 0; /* 开始与指定周期广播建立同步 */
  virtual void StopSync(uint16_t handle) = 0; /* 停止周期广播同步 */

  virtual void RegisterCallbacks(ScanningCallbacks* callbacks) = 0; /* 注册扫描回调接口 */
};


/* btif entry: implemented by the stack_adapter, resolved at link/load time */ /* btif 入口：由 stack_adapter 实现，链接/加载时解析 */
#ifdef __cplusplus
extern "C" {
#endif

BleScannerInterface* BtGetBleScannerInstance(void); /* 获取 BLE 扫描器接口实例 */

#ifdef __cplusplus
}
#endif

