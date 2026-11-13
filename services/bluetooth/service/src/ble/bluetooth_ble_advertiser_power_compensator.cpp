/*
 * Copyright (C) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef LOG_TAG
#define LOG_TAG "bt_server_ble_tx_power_comp"
#endif

#include "bluetooth_ble_advertiser_power_compensator.h"

#include "bluetooth_datashare_utils.h"
#include "bluetooth_log.h"
#include "parameters.h"
#include "parameter.h"

namespace OHOS {
namespace Bluetooth {

static constexpr const char* DEDICATED_ANTENNA_SWITCH_STATE_PARAM =
    "persist.bluetooth.dedicated_antenna_switch_state";

void BleAdvTxPowerCompensator::AdjustBleAdvTxPower(
    bluetooth::BleAdvertiserSettingsImpl &settings)
{
    constexpr const char* C1_TX_POWER_COMPENSATION_KEY =
        "const.bluetooth.c1_ble_tx_power_compensation";
    int compensation = OHOS::system::GetIntParameter(C1_TX_POWER_COMPENSATION_KEY, 0);
    if (compensation == 0) {
        return;
    }

    if (!IsBleAdvOnDedicatedAnt()) {
        return;
    }

    if (!IsBleAdvOnMac1()) {
        return;
    }

    if (IsOOBEState()) {
        return;
    }

    int8_t currentTxPower = settings.GetTxPower();
    int16_t newTxPower = static_cast<int16_t>(currentTxPower) + compensation;
    if (newTxPower > maxTxPower) {
        newTxPower = maxTxPower;
        HILOGW("BleAdvTxPowerCompensator: clamped to max %{public}d", newTxPower);
    }
    if (newTxPower < minTxPower) {
        newTxPower = minTxPower;
        HILOGW("BleAdvTxPowerCompensator: clamped to min %{public}d", newTxPower);
    }
    HILOGI("BleAdvTxPowerCompensator: Dedicated antenna, %{public}d + %{public}d = %{public}d",
        currentTxPower, compensation, newTxPower);
    settings.SetTxPower(static_cast<int8_t>(newTxPower));
}

bool BleAdvTxPowerCompensator::IsBleAdvOnDedicatedAnt()
{
    constexpr const char* MLPS_ANT_INFO_DEFAULT = "0:0";
    constexpr int BT_DEDICATED_ANT_ID = 3;

    std::string antInfoStr = OHOS::system::GetParameter(DEDICATED_ANTENNA_SWITCH_STATE_PARAM, MLPS_ANT_INFO_DEFAULT);
    size_t pos = antInfoStr.find(':');
    if (pos == std::string::npos) {
        HILOGW("BleAdvTxPowerCompensator: invalid ant info '%{public}s'", antInfoStr.c_str());
        return false;
    }
    int antID = std::atoi(antInfoStr.substr(pos + 1).c_str());
    return antID == BT_DEDICATED_ANT_ID;
}

bool BleAdvTxPowerCompensator::IsBleAdvOnMac1()
{
    constexpr const char* RANGING_ANT_INFO_PARAM = "persist.bluetooth.ranging_ant_info";
    constexpr const char* RANGING_ANT_INFO_DEFAULT = "0:0";
    constexpr int MAC_ID_ONE = 1;

    std::string antInfoStr = OHOS::system::GetParameter(RANGING_ANT_INFO_PARAM, RANGING_ANT_INFO_DEFAULT);
    size_t pos = antInfoStr.find(':');
    if (pos == std::string::npos) {
        HILOGW("BleAdvTxPowerCompensator: invalid ranging ant info '%{public}s'", antInfoStr.c_str());
        return false;
    }
    int macID = std::atoi(antInfoStr.substr(0, pos).c_str());
    return macID == MAC_ID_ONE;
}

bool BleAdvTxPowerCompensator::IsOOBEState()
{
    if (oobeChecked_ && !isOOBE_) {
        return false;
    }

    constexpr const char* SETTINGS_DATA_BASE_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
    constexpr const char* KEY_DEVICE_PROVISIONED = "device_provisioned";
    constexpr const char* KEY_USER_SETUP_COMPLETE = "user_setup_complete";
    constexpr const char* KEY_IS_OTA_FINISHED = "is_ota_finished";
    constexpr const char* OOBE_READY = "1";

    std::string deviceProvisioned;
    bool ret = bluetooth::BluetoothDataShareQuery(
        SETTINGS_DATA_BASE_URI, KEY_DEVICE_PROVISIONED, deviceProvisioned);
    if (ret && deviceProvisioned != OOBE_READY) {
        return true;
    }

    std::string userSetupComplete;
    ret = bluetooth::BluetoothDataShareQuery(
        SETTINGS_DATA_BASE_URI, KEY_USER_SETUP_COMPLETE, userSetupComplete);
    if (ret && userSetupComplete != OOBE_READY) {
        return true;
    }

    std::string isOtaFinished;
    ret = bluetooth::BluetoothDataShareQuery(
        SETTINGS_DATA_BASE_URI, KEY_IS_OTA_FINISHED, isOtaFinished);
    if (ret && isOtaFinished == "0") {
        return true;
    }

    if (!ret) {
        return true;
    }

    oobeChecked_ = true;
    isOOBE_ = false;
    return false;
}

}  // namespace Bluetooth
}  // namespace OHOS
