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

#pragma once

#include <stdint.h>

#include <functional>
#include <vector>

#include "bt_common_types.h"
#include "bt_gatt_types.h"
#include "types/raw_address.h"

/* BLE 广播参数配置结构体 */
struct AdvertiseParameters {
  uint16_t advertising_event_properties; /* 广播事件属性(可连接/可扫描/传统或扩展广播等标志位) */
  uint32_t min_interval; /* 最小广播间隔，单位0.625ms */
  uint32_t max_interval; /* 最大广播间隔，单位0.625ms */
  uint8_t channel_map; /* 广播信道映射(bit0:ch37, bit1:ch38, bit2:ch39) */
  int8_t tx_power; /* 广播发射功率，单位dBm */
  uint8_t primary_advertising_phy; /* 主广播物理信道类型(1M/2M/Coded PHY) */
  uint8_t secondary_advertising_phy; /* 次广播物理信道类型(用于扩展广播) */
  uint8_t scan_request_notification_enable; /* 扫描请求通知使能标志 */
  uint8_t own_addr[RawAddress::kLength]; // defalut value is {}
  int8_t own_addr_type = -1; // defalut value is -1, means not configured
};

/* BLE 周期广播参数配置结构体 */
struct PeriodicAdvertisingParameters {
  uint8_t enable; /* 周期广播使能标志 */
  uint16_t min_interval; /* 最小周期广播间隔，单位1.25ms */
  uint16_t max_interval; /* 最大周期广播间隔，单位1.25ms */
  uint16_t periodic_advertising_properties; /* 周期广播属性标志 */
};

/**
 * LE Advertising related callbacks invoked from from the Bluetooth native stack
 * All callbacks are invoked on the JNI thread
 */
class AdvertisingCallbacks { /* BLE 广播事件回调接口类，由协议栈在JNI线程上调用 */
public:
  virtual ~AdvertisingCallbacks() = default;
  virtual void OnAdvertisingSetStarted(int reg_id, uint8_t advertiser_id, int8_t tx_power,
                                       uint8_t status) = 0; /* 广播集启动结果回调：reg_id为上层注册ID，advertiser_id为协议栈分配的广播句柄 */
  virtual void OnAdvertisingEnabled(uint8_t advertiser_id, bool enable, uint8_t status) = 0; /* 广播开启/停止操作结果回调 */
  virtual void OnAdvertisingDataSet(uint8_t advertiser_id, uint8_t status) = 0; /* 广播数据设置结果回调 */
  virtual void OnScanResponseDataSet(uint8_t advertiser_id, uint8_t status) = 0; /* 扫描响应数据(scan response)设置结果回调 */
  virtual void OnAdvertisingParametersUpdated(uint8_t advertiser_id, int8_t tx_power,
                                              uint8_t status) = 0; /* 广播参数更新结果回调，tx_power为协议栈实际使用的发射功率 */
  virtual void OnPeriodicAdvertisingParametersUpdated(uint8_t advertiser_id, uint8_t status) = 0; /* 周期广播参数更新结果回调 */
  virtual void OnPeriodicAdvertisingDataSet(uint8_t advertiser_id, uint8_t status) = 0; /* 周期广播数据设置结果回调 */
  /* 周期广播开启/停止操作结果回调 */
  virtual void OnPeriodicAdvertisingEnabled(uint8_t advertiser_id, bool enable, uint8_t status) = 0;
  virtual void OnOwnAddressRead(uint8_t advertiser_id, uint8_t address_type,
                                RawAddress address) = 0; /* 读取广播使用的自身地址结果回调，address_type为地址类型(公共/随机) */
};

class BleAdvertiserInterface {
public:
  virtual ~BleAdvertiserInterface() = default;

  /** Callback invoked when multi-adv operation has completed */
  using StatusCallback = std::function<void(uint8_t /* status */)>; /* 仅带操作状态的操作结果回调 */
  /* 带广播句柄和操作状态的操作结果回调 */
  using IdStatusCallback = std::function<void(uint8_t /* advertiser_id */, uint8_t /* status */)>;
  /* 带广播句柄、发射功率和操作状态的操作结果回调 */
  using IdTxPowerStatusCallback = std::function<void(uint8_t /* advertiser_id */,
                                                     int8_t /* tx_power */, uint8_t /* status */)>;
  /* 广播参数设置结果回调(状态+实际发射功率) */
  using ParametersCallback = std::function<void(uint8_t /* status */, int8_t /* tx_power */)>;

  /** Registers an advertiser with the stack */
  virtual void RegisterAdvertiser(IdStatusCallback) = 0;

  using GetAddressCallback =
          std::function<void(uint8_t /* address_type*/, RawAddress /*address*/)>;
  virtual void GetOwnAddress(uint8_t advertiser_id, GetAddressCallback cb) = 0;

  /* Set the parameters as per spec, user manual specified values */
  virtual void SetParameters(uint8_t advertiser_id, AdvertiseParameters params,
                             ParametersCallback cb) = 0;

  /* Setup the data */
  virtual void SetData(int advertiser_id, bool set_scan_rsp, std::vector<uint8_t> data,
                       StatusCallback cb) = 0;

  /* Enable the advertising instance */
  virtual void Enable(uint8_t advertiser_id, bool enable, StatusCallback cb, uint16_t duration,
                      uint8_t maxExtAdvEvents, StatusCallback timeout_cb) = 0;

  /*  Unregisters an advertiser */
  virtual void Unregister(uint8_t advertiser_id) = 0;

  virtual void StartAdvertising(uint8_t advertiser_id, StatusCallback cb,
                                AdvertiseParameters params, std::vector<uint8_t> advertise_data,
                                std::vector<uint8_t> scan_response_data, int timeout_s,
                                StatusCallback timeout_cb) = 0;

  /** Start the advertising set. This include registering, setting all
   * parameters and data, and enabling it. |register_cb| is called when the set
   * is advertising. |timeout_cb| is called when the timeout_s have passed.
   * |reg_id| is the callback id assigned from upper or native layer.
   * |client_id| is the callbacks client id for jni or native layer.
   */
  virtual void StartAdvertisingSet(uint8_t client_id, int reg_id,
                                   IdTxPowerStatusCallback register_cb, AdvertiseParameters params,
                                   std::vector<uint8_t> advertise_data,
                                   std::vector<uint8_t> scan_response_data,
                                   PeriodicAdvertisingParameters periodic_params,
                                   std::vector<uint8_t> periodic_data, uint16_t duration,
                                   uint8_t maxExtAdvEvents, IdStatusCallback timeout_cb) = 0;

  virtual void SetPeriodicAdvertisingParameters(int advertiser_id,
                                                PeriodicAdvertisingParameters parameters,
                                                StatusCallback cb) = 0;

  virtual void SetPeriodicAdvertisingData(int advertiser_id, std::vector<uint8_t> data,
                                          StatusCallback cb) = 0;

  virtual void SetPeriodicAdvertisingEnable(int advertiser_id, bool enable, bool include_adi,
                                            StatusCallback cb) = 0;
  virtual void RegisterCallbacks(AdvertisingCallbacks* callbacks) = 0;
};


/* btif entry: implemented by the stack_adapter, resolved at link/load time */
#ifdef __cplusplus
extern "C" {
#endif

BleAdvertiserInterface* BtGetBleAdvertiserInstance(void);

#ifdef __cplusplus
}
#endif

