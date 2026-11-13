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

#include "bluetooth_hw_interface.h"
#include "common_event_manager.h"
#include "bt_chr_ue_manager.h"
#include "bluetooth_common_event_subscriber.h"
#include "btcommon/timer_manager.h"
#include "ble_defs.h"
#include <map>

using namespace OHOS::EventFwk;

namespace OHOS {
namespace bluetooth {

namespace STACK {
const RawAddress RawAddress::kEmpty{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
}

using SensorhubObserver = BluetoothHwInterface::SensorhubObserver;
static std::unique_ptr<BaseObserverList<SensorhubObserver>> g_sensorhubObservers = nullptr;

BluetoothHwInterface* BluetoothHwInterface::GetInstance(void)
{
    static BluetoothHwInterface singleton;
    return &singleton;
}

bool BluetoothHwInterface::InitBtHwInterface(const bthwif_interface_t *bthwif)
{
    bthwInterface_ = bthwif;
    return true;
}

const bthwif_interface_t* BluetoothHwInterface::GetBtHwInterface(void)
{
    return bthwInterface_;
}

void BluetoothHwInterface::SetBtHwInterface(const bthwif_interface_t* interface)
{
    bthwInterface_ = interface;
}

BluetoothHwInterface::BluetoothHwInterface(void) : bthwInterface_(nullptr)
{
    g_sensorhubObservers = std::make_unique<BaseObserverList<SensorhubObserver>>();
}

void BluetoothHwInterface::AddSensorhubObserver(SensorhubObserver &observer)
{
    if (g_sensorhubObservers) {
        g_sensorhubObservers->Register(observer);
    }
}

void BluetoothHwInterface::RemoveSensorhubObserver(SensorhubObserver &observer)
{
    if (g_sensorhubObservers) {
        g_sensorhubObservers->Deregister(observer);
    }
}

void HwSensorhubCollaborationCb(const std::vector<uint8_t> &notifyValue)
{
    if (g_sensorhubObservers) {
        g_sensorhubObservers->ForEach([data = notifyValue](SensorhubObserver &observer) {
            observer.SensorhubCollaborationCallback(data);
        });
    }
}

void ConvertBleScanMode(int scanMode, uint16_t &outScanWindow, uint16_t &outScanInterval)
{
    // scanMode <-> { interval, window }
    static std::map<int, std::pair<uint16_t, uint16_t>> table {
        { SCAN_MODE_LOW_POWER, { BLE_SCAN_MODE_LOW_POWER_INTERVAL_MS, BLE_SCAN_MODE_LOW_POWER_WINDOW_MS }},
        { SCAN_MODE_BALANCED, { BLE_SCAN_MODE_BALANCED_INTERVAL_MS, BLE_SCAN_MODE_BALANCED_WINDOW_MS }},
        { SCAN_MODE_LOW_LATENCY, { BLE_SCAN_MODE_LOW_LATENCY_INTERVAL_MS, BLE_SCAN_MODE_LOW_LATENCY_WINDOW_MS }},
        { SCAN_MODE_OP_P2_60_3000, { BLE_SCAN_MODE_OP_P2_60_3000_INTERVAL_MS, BLE_SCAN_MODE_OP_P2_60_3000_WINDOW_MS }},
        { SCAN_MODE_OP_P10_60_600, { BLE_SCAN_MODE_OP_P10_60_600_INTERVAL_MS, BLE_SCAN_MODE_OP_P10_60_600_WINDOW_MS }},
        { SCAN_MODE_OP_P25_60_240, { BLE_SCAN_MODE_OP_P25_60_240_INTERVAL_MS, BLE_SCAN_MODE_OP_P25_60_240_WINDOW_MS }},
        { SCAN_MODE_OP_P100_1000_1000,
            { BLE_SCAN_MODE_OP_P100_1000_1000_INTERVAL_MS, BLE_SCAN_MODE_OP_P100_1000_1000_WINDOW_MS }},
        { SCAN_MODE_OP_P50_100_200,
            {BLE_SCAN_MODE_OP_P50_100_200_INTERVAL_MS, BLE_SCAN_MODE_OP_P50_100_200_WINDOW_MS}},
        { SCAN_MODE_OP_P10_30_300, { BLE_SCAN_MODE_OP_P10_30_300_INTERVAL_MS, BLE_SCAN_MODE_OP_P10_30_300_WINDOW_MS }},
        { SCAN_MODE_OP_P2_30_1500, {BLE_SCAN_MODE_OP_P2_30_1500_INTERVAL_MS, BLE_SCAN_MODE_OP_P2_30_1500_WINDOW_MS }},
        { SCAN_MODE_OP_P75_30_40, {BLE_SCAN_MODE_OP_P75_30_40_INTERVAL_MS, BLE_SCAN_MODE_OP_P75_30_40_WINDOW_MS }},
        { SCAN_MODE_OP_P50_30_60, {BLE_SCAN_MODE_OP_P50_30_60_INTERVAL_MS, BLE_SCAN_MODE_OP_P50_30_60_WINDOW_MS }},
        { SCAN_MODE_OP_P10_25_250, {BLE_SCAN_MODE_OP_P10_25_250_INTERVAL_MS, BLE_SCAN_MODE_OP_P10_25_250_WINDOW_MS }}
    };
    auto iter = table.find(scanMode);
    if (iter != table.end()) {
        auto [interval, window] = iter->second;
        outScanInterval = interval;
        outScanWindow = window;
    } else {
        HILOGW("Unknown scan mode: %{public}d, set default balanced mode", scanMode);
        outScanInterval = BLE_SCAN_MODE_BALANCED_INTERVAL_MS;
        outScanWindow = BLE_SCAN_MODE_BALANCED_WINDOW_MS;
    }
}

void BluetoothCommonEventSubscriberEx::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    if (action_ == data.action) {
        callback_(data);
    }
}

}  // namespace bluetooth
}  // namespace OHOS

std::vector<std::shared_ptr<CommonEventSubscriber>> g_testCommonEvent;

void TestSetScreenStatus(const std::string &status)
{
    CommonEventData data = {status};
    for (uint32_t i = 0; i < g_testCommonEvent.size(); i++) {
        g_testCommonEvent[i]->OnReceiveEvent(data);
    }
}

int CommonEventManager::SubscribeCommonEvent(std::shared_ptr<CommonEventSubscriber> s)
{
    g_testCommonEvent.emplace_back(s);
    return 0;
}

void CommonEventManager::UnSubscribeCommonEvent(std::shared_ptr<CommonEventSubscriber> s)
{
    for (uint32_t i = 0; i < g_testCommonEvent.size(); i++) {
        if (g_testCommonEvent[i] == s) {
            g_testCommonEvent.erase(g_testCommonEvent.begin() + i);
            return;
        }
    }
}

BtChrUeManager::BtChrUeManager()
{}

BtChrUeManager* BtChrUeManager::GetInstance()
{
    static BtChrUeManager instance;
    return &instance;
}

void BtChrUeManager::WriteSensorhubCollaborationUe(const std::string &eventName,
    const SensorhubCollaborationUe &shColl)
{
}

utility::Timer::Timer(const std::function<void()> &callback)
{
    callback_ = callback;
}

void utility::Timer::Stop()
{
}

bool utility::Timer::Start(int a, bool b)
{
    return true;
}

utility::Timer::~Timer()
{
}