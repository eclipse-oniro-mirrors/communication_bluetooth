/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "taihe_bluetooth_ble_central_manager_callback.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "taihe_native_object.h"
#include "taihe_bluetooth_error.h"

namespace OHOS {
namespace Bluetooth {

TaiheBluetoothBleCentralManagerCallback::TaiheBluetoothBleCentralManagerCallback(bool isLatestTaiheBleScannerObj)
    : eventSubscribe_(REGISTER_BLE_FIND_DEVICE_TYPE, "bluetoothBleCentral_taihe")
{
    isLatestTaiheBleScannerObj_ = isLatestTaiheBleScannerObj;
}

TaiheBluetoothBleCentralManagerCallback &TaiheBluetoothBleCentralManagerCallback::GetInstance(void)
{
    static TaiheBluetoothBleCentralManagerCallback instance(false);
    return instance;
}

static ::ohos::bluetooth::ble::ScanResult ConvertScanResultItem(const BleScanResult &result)
{
    auto payload = result.GetPayload();
    BleScanResult &mutableResult = const_cast<BleScanResult &>(result);
    return ::ohos::bluetooth::ble::ScanResult{
        taihe::string(result.GetPeripheralDevice().GetDeviceAddr()),
        result.GetRssi(),
        taihe::array<uint8_t>(taihe::copy_data, payload.data(), payload.size()),
        taihe::string(mutableResult.GetName()),
        result.IsConnectable()
    };
}

static ::ohos::bluetooth::ble::ScanReport ConvertScanResult(const BleScanResult &result,
    ::ohos::bluetooth::ble::ScanReportType reportType)
{
    std::vector<::ohos::bluetooth::ble::ScanResult> resultVec;
    resultVec.push_back(ConvertScanResultItem(result));
    ::ohos::bluetooth::ble::ScanReport scanReport{
        reportType,
        taihe::array<::ohos::bluetooth::ble::ScanResult>(
            taihe::move_data, resultVec.data(), resultVec.size())
    };
    return scanReport;
}

void TaiheBluetoothBleCentralManagerCallback::OnScanCallback(const BleScanResult &result)
{
    HILOGD("enter, remote device address: %{public}s", GET_ENCRYPT_ADDR(result.GetPeripheralDevice()));
    if (isLatestTaiheBleScannerObj_) {
        auto scanReport = ConvertScanResult(result,
            ::ohos::bluetooth::ble::ScanReportType(::ohos::bluetooth::ble::ScanReportType::key_t::ON_FOUND));
        auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::ScanReport const&)
            >(REGISTER_BLE_FIND_DEVICE_TYPE);
        module.PublishEvent(scanReport);
    } else {
        std::vector<::ohos::bluetooth::ble::ScanResult> resultVec;
        resultVec.push_back(ConvertScanResultItem(result));
        taihe::array<::ohos::bluetooth::ble::ScanResult> results(
            taihe::move_data, resultVec.data(), resultVec.size());
        auto &module = eventSubscribe_.GetModule<void(::taihe::array_view<::ohos::bluetooth::ble::ScanResult>)
            >(REGISTER_BLE_FIND_DEVICE_TYPE);
        module.PublishEvent(results);
    }
}

void TaiheBluetoothBleCentralManagerCallback::OnFoundOrLostCallback(const BleScanResult &result, uint8_t callbackType)
{
    HILOGD("enter, remote device address: %{public}s", GET_ENCRYPT_ADDR(result.GetPeripheralDevice()));
    ScanReportType scanReportType = ScanReportType::ON_FOUND;
    ConvertScanReportType(scanReportType, callbackType);
    if (isLatestTaiheBleScannerObj_) {
        auto scanReport = ConvertScanResult(result,
            ::ohos::bluetooth::ble::ScanReportType::from_value(static_cast<int32_t>(scanReportType)));
        auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::ScanReport const&)
            >(REGISTER_BLE_FIND_DEVICE_TYPE);
        module.PublishEvent(scanReport);
    }
}

void TaiheBluetoothBleCentralManagerCallback::OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results)
{
    HILOGI("enter, scan result size: %{public}zu", results.size());
    if (isLatestTaiheBleScannerObj_ && !results.empty()) {
        std::vector<::ohos::bluetooth::ble::ScanResult> resultVec;
        resultVec.reserve(results.size());
        for (size_t i = 0; i < results.size(); i++) {
            resultVec.push_back(ConvertScanResultItem(results[i]));
        }
        ::ohos::bluetooth::ble::ScanReport scanReport{
            ::ohos::bluetooth::ble::ScanReportType(::ohos::bluetooth::ble::ScanReportType::key_t::ON_BATCH),
            taihe::array<::ohos::bluetooth::ble::ScanResult>(
                taihe::move_data, resultVec.data(), resultVec.size())
        };
        auto &module = eventSubscribe_.GetModule<void(::ohos::bluetooth::ble::ScanReport const&)
            >(REGISTER_BLE_FIND_DEVICE_TYPE);
        module.PublishEvent(scanReport);
    }
}

void TaiheBluetoothBleCentralManagerCallback::OnStartOrStopScanEvent(int resultCode, bool isStartScan)
{
    HILOGI("resultCode: %{public}d, isStartScan: %{public}d", resultCode, isStartScan);
    if (isStartScan) {
        AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::BLE_START_SCAN, nullptr, resultCode);
    } else {
        std::vector<int32_t> validErrCodes = {
            BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
            BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_BLE_SCAN_NO_RESOURCE,
            BT_ERR_INTERNAL_ERROR, BT_ERR_BLE_SCAN_ALREADY_STARTED
        };
        validErrCodes.emplace_back(BT_NO_ERROR); // BT_NO_ERROR is valid error code
        auto processResult = ProcessErrCode(resultCode, validErrCodes);
        HILOGI("stop scan result: %{public}d -> %{public}d", resultCode, static_cast<int>(processResult.errCode));
        AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::BLE_STOP_SCAN, nullptr,
            static_cast<int>(processResult.errCode));
    }
}

void TaiheBluetoothBleCentralManagerCallback::ConvertScanReportType(
    ScanReportType &scanReportType, uint8_t callbackType)
{
    if (callbackType == BLE_SCAN_CALLBACK_TYPE_FIRST_MATCH) {
        scanReportType = ScanReportType::ON_FOUND;
    } else if (callbackType == BLE_SCAN_CALLBACK_TYPE_LOST_MATCH) {
        scanReportType = ScanReportType::ON_LOST;
    }
}
}  // namespace Bluetooth
}  // namespace OHOS