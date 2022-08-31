/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "ohos_bt_gatt.h"

#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <sstream>

#include <vector>
#include "__config"

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_utils.h"

#include "cstdint"
#include "iosfwd"
#include "istream"
#include "new"
#include "ohos_bt_adapter_utils.h"
#include "ohos_bt_def.h"
#include "ostream"
#include "securec.h"
#include "streambuf"
#include "string"


#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
#define MAX_BLE_ADV_NUM 7

class BleCentralManagerCallbackWapper;
class BleAdvCallback;

static BtGattCallbacks *g_AppCallback;
static BleCentralManager *g_BleCentralManager;
static BleCentralManagerCallbackWapper *g_ScanCallback;

static BleAdvCallback *g_BleAdvCallbacks[MAX_BLE_ADV_NUM];
static BleAdvertiser *g_BleAdvertiser = nullptr;

class BleCentralManagerCallbackWapper : public BleCentralManagerCallback {
public:
    /**
     * @brief Scan result callback.
     *
     * @param result Scan result.
     * @since 6
     */
    void OnScanCallback(const BleScanResult &result) {
        BtScanResultData scanResult;
        scanResult.eventType = OHOS_BLE_EVT_LEGACY_CONNECTABLE; // result.IsConnectable();
        scanResult.dataStatus = OHOS_BLE_DATA_COMPLETE;
        scanResult.addrType = OHOS_BLE_RANDOM_DEVICE_ADDRESS;
        scanResult.primaryPhy = OHOS_BLE_SCAN_PHY_1M;
        scanResult.secondaryPhy = OHOS_BLE_SCAN_PHY_1M;
        scanResult.advSid = 1;
        scanResult.txPower = 1;
        scanResult.rssi = result.GetRssi();
        scanResult.directAddrType = OHOS_BLE_RANDOM_DEVICE_ADDRESS;
        GetAddrFromString(result.GetPeripheralDevice().GetDeviceAddr(), scanResult.addr.addr);
        vector<uint8_t> vec = result.GetPayload();
        scanResult.advData = vec.data();
        scanResult.advLen = vec.size();

        std::string strs;
        std::stringstream strStream;
        for (int i = 0; i < scanResult.advLen; i++) {
            char token[3] = {0};
            (void)sprintf_s(token, 3, "%02X", scanResult.advData[i]);
            strStream << token;
        }
        strStream >> strs;
        string address = result.GetPeripheralDevice().GetDeviceAddr();
        HILOGI("device: %{public}s, scan data: %{public}s", GetEncryptAddr(address).c_str(), strs.c_str());
        g_AppCallback->scanResultCb(&scanResult);
    }

    /**
     * @brief Batch scan results event callback.
     *
     * @param results Scan results.
     * @since 6
     */
    void OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results) {}

    /**
     * @brief Start or Stop scan event callback.
     *
     * @param resultCode Scan result code.
     * @param isStartScan true->start scan, false->stop scan.
     * @since 6
     */
    void OnStartOrStopScanEvent(int resultCode, bool isStartScan) {}
};

class BleAdvCallback : public BleAdvertiseCallback {
public:
    BleAdvCallback(int advId)
    {
        advId_ = advId;
    }

    void OnStartResultEvent(int result) {
        if (result != 0) {
            HILOGE("result : %{public}d", result);
            return;
        }

        HILOGI("adv started. advId_: %{public}d", advId_);
        if (g_AppCallback != nullptr && g_AppCallback->advEnableCb != nullptr) {
            g_AppCallback->advEnableCb(advId_, 0);
        }
    }

protected:
    BleAdvertiserData *advData;
    BleAdvertiserData *advResponseData;
    BleAdvertiserSettings *advSetting;

private:
    int advId_;
};

/**
 * @brief Initializes the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is initialized;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int InitBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Enables the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is enabled;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int EnableBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Disables the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is disabled;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int DisableBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets the Bluetooth device name.
 *
 * @param name Indicates the pointer to the name to set.
 * @param len Indicates the length of the name to set.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth device name is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int SetDeviceName(const char *name, unsigned int len) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets advertising data.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param data Indicates the pointer to the advertising data. For details, see {@link BleConfigAdvData}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising data is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetAdvData(int advId, const BleConfigAdvData *data)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Starts advertising.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param param Indicates the pointer to the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartAdv(int advId, const BleAdvParams *param)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

static bool IsAllAdvStopped()
{
    for (int i = 0; i < MAX_BLE_ADV_NUM; i++) {
        if (g_BleAdvCallbacks[i] != nullptr) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Stops advertising.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising is stopped;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStopAdv(int advId)
{
    HILOGI("BleStopAdv, advId: %{public}d.", advId);
    if (advId < 0 || advId >= MAX_BLE_ADV_NUM) {
        HILOGE("BleStopAdv fail, advId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }
    if (g_BleAdvertiser == nullptr || g_BleAdvCallbacks[advId] == nullptr) {
        HILOGE("BleStopAdv fail, the current adv is not started.");
        return OHOS_BT_STATUS_FAIL;
    }

    g_BleAdvertiser->StopAdvertising(*g_BleAdvCallbacks[advId]);

    usleep(100);
    if (g_AppCallback != nullptr && g_AppCallback->advDisableCb != nullptr) {
        HILOGI("adv stopped advId: %{public}d.", advId);
        g_AppCallback->advDisableCb(advId, 0);
    }
    delete g_BleAdvCallbacks[advId];
    g_BleAdvCallbacks[advId] = nullptr;

    if (IsAllAdvStopped()) {
        HILOGI("All adv have been stopped.");
        delete g_BleAdvertiser;
        g_BleAdvertiser = nullptr;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Updates advertising parameters.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param param Indicates the pointer to the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising parameters are updated;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleUpdateAdv(int advId, const BleAdvParams *param) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets the secure I/O capability mode.
 *
 * @param mode Indicates the capability mode to set. For details, see {@link BleIoCapMode}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the capability mode is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetSecurityIoCap(BleIoCapMode mode) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets the authentication mode for secure connection requests.
 *
 * @param mode Indicates the authentication mode to set. For details, see {@link BleAuthReqMode}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the authentication mode is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetSecurityAuthReq(BleAuthReqMode mode) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Responds to a secure connection request.
 *
 * @param bdAddr Indicates the address of the device that sends the request.
 * @param accept Specifies whether to accept the request. The value <b>true</b> means to accept the request,
 * and <b>false</b> means to reject the request.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the request is responded to;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattSecurityRsp(BdAddr bdAddr, bool accept) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Obtains the device MAC address.
 *
 * @param mac Indicates the pointer to the device MAC address.
 * @param len Indicates the length of the device MAC address.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the device MAC address is obtained;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int ReadBtMacAddr(unsigned char *mac, unsigned int len) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets scan parameters.
 *
 * @param clientId Indicates the client ID, which is obtained during client registration.
 * @param param Indicates the pointer to the scan parameters. For details, see {@link BleScanParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan parameters are set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetScanParameters(int clientId, BleScanParams *param)
{
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts a scan.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartScan(void)
{
    HILOGI("BleStartScan enter");
    if (g_BleCentralManager == nullptr) {
        HILOGE("BleStartScan fail, ble centra manager is null.");
        return OHOS_BT_STATUS_FAIL;
    }

    g_BleCentralManager->StartScan();
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts a scan with specific parameters and filters.
 *
 * @param params Indicates the pointer to the scan parameters. For details, see {@link BleScanParams}.
 * @param filter Indicates the pointer to the scan filter. For details, see {@link BleScanNativeFilter}.
 * @param filterSize Indicates the number of the scan filter.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartFilterScan(BleScanParams *params, BleScanNativeFilter *filter, unsigned int filterSize)
{
    HILOGI("BleStartFilterScan enter");
    if (g_BleCentralManager == NULL || params == NULL || filter == NULL) {
        return 1;
    }

    vector<BleScanFilter> scanFilters;
    for (unsigned int i = 0; i < filterSize; i++) {
        BleScanNativeFilter nativeScanFilter = filter[i];
        BleScanFilter scanFilter;
        scanFilter.SetName(nativeScanFilter.deviceName);
        UUID serviceUuid = UUID::FromString((char *)nativeScanFilter.serviceUuid);
        scanFilter.SetServiceUuid(serviceUuid);
        UUID serviceUuidMask = UUID::FromString((char *)nativeScanFilter.serviceUuidMask);
        scanFilter.SetServiceUuidMask(serviceUuidMask);

        std::vector<uint8_t> serviceData;
        std::vector<uint8_t> serviceDataMask;
        if (nativeScanFilter.manufactureData != NULL && nativeScanFilter.manufactureDataMask != NULL) {
            for (unsigned int i = 0; i < nativeScanFilter.serviceDataLength; i++) {
                serviceData.push_back(nativeScanFilter.serviceData[i]);
                serviceDataMask.push_back(nativeScanFilter.serviceDataMask[i]);
            }
        }
        scanFilter.SetServiceData(serviceData);
        scanFilter.SetServiceDataMask(serviceDataMask);

        std::vector<uint8_t> manufactureData;
        std::vector<uint8_t> manufactureDataMask;
        if (nativeScanFilter.manufactureData != NULL && nativeScanFilter.manufactureDataMask != NULL) {
            for (unsigned int i = 0; i < nativeScanFilter.manufactureDataLength; i++) {
                manufactureData.push_back(nativeScanFilter.manufactureData[i]);
                manufactureDataMask.push_back(nativeScanFilter.manufactureDataMask[i]);
            }
        }
        scanFilter.SetManufactureData(manufactureData);
        scanFilter.SetManufactureDataMask(manufactureDataMask);
        scanFilter.SetManufacturerId(nativeScanFilter.manufactureId);
        scanFilters.push_back(scanFilter);
    }

    BleScanSettings scanSettings;
    int scanMode = GetBleScanMode(params->scanInterval, params->scanWindow);
    scanSettings.SetScanMode(scanMode);
    int curScanPhy = static_cast<int>(params->scanPhy);
    if (curScanPhy >= PHY_LE_1M && curScanPhy <= PHY_LE_CODED) {
        scanSettings.SetPhy(curScanPhy);
    }

    g_BleCentralManager->ConfigScanFilter(scanFilters);
    g_BleCentralManager->StartScan(scanSettings);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Gets BLE scan mode.
 *
 * @param scanInterval Scan interval. It is calculated as follows: [N * 0.625 ms],
 * where N indicates the value of this field.
 * @param scanWindow Scan window. It is calculated as follows: [N * 0.625 ms],
 * where N indicates the value of this field.
 * @return Returns scan mode. For details, see {@link SCAN_MODE}.
 * @since 6
 */
int GetBleScanMode(unsigned short scanInterval, unsigned short scanWindow)
{
    int scanMode = SCAN_MODE_LOW_POWER;
    HILOGI("Get blue scan mode successfully, mode = %{public}d", scanMode);
    return scanMode;
}

/**
 * @brief Stops a scan.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is stopped;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStopScan(void)
{
    HILOGI("BleStopScan enter");
    if (g_BleCentralManager == nullptr) {
        HILOGE("BleStopScan fail, ble centra manager is null.");
        return OHOS_BT_STATUS_FAIL;
    }

    g_BleCentralManager->StopScan();
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Registers GATT callbacks.
 *
 * @param func Indicates the pointer to the callbacks to register. For details, see {@link BtGattCallbacks}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT callbacks are registered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattRegisterCallbacks(BtGattCallbacks *func)
{
    HILOGI("BleGattRegisterCallbacks enter");
    g_AppCallback = func;

    if (g_ScanCallback == nullptr) {
        g_ScanCallback = new BleCentralManagerCallbackWapper();
    }

    if (g_BleCentralManager == nullptr) {
        g_BleCentralManager = new BleCentralManager(*g_ScanCallback);
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets advertising data and parameters and starts advertising.
 *
 * This function is available for system applications only. \n
 *
 * @param advId Indicates the pointer to the advertisement ID.
 * @param rawData Indicates the advertising data. For details, see {@link StartAdvRawData}.
 * @param advParam Indicates the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartAdvEx(int *advId, const StartAdvRawData rawData, BleAdvParams advParam)
{
    HILOGI("BleStartAdvEx enter");
    if (g_BleAdvertiser == nullptr) {
        g_BleAdvertiser = new BleAdvertiser();
    }
    int i = 0;
    for (i = 0; i < MAX_BLE_ADV_NUM; i++) {
        if (g_BleAdvCallbacks[i] == nullptr) {
            g_BleAdvCallbacks[i] = new BleAdvCallback(i);
            break;
        }
        HILOGI("g_BleAdvCallbacks[%{public}d] = %{public}p.", i, g_BleAdvCallbacks[i]);
    }

    if (i == MAX_BLE_ADV_NUM) {
        return OHOS_BT_STATUS_UNHANDLED;
    }

    *advId = i;
    HILOGI("ret advId: %{public}d.", *advId);

    BleAdvertiserSettings settings;
    settings.SetInterval(advParam.minInterval);
    if (advParam.advType == OHOS_BLE_ADV_SCAN_IND ||
        advParam.advType == OHOS_BLE_ADV_NONCONN_IND) {
        settings.SetConnectable(false);
    }

    vector<uint8_t> advData;
    if (rawData.advData != nullptr) {
        for (unsigned int i = 0; i < rawData.advDataLen; i++) {
            advData.push_back(rawData.advData[i]);
        }
    }

    vector<uint8_t> scanResponse;
    if (rawData.rspData != nullptr) {
        for (unsigned int i = 0; i < rawData.rspDataLen; i++) {
            scanResponse.push_back(rawData.rspData[i]);
        }
    }

    g_BleAdvertiser->StartAdvertising(settings, advData, scanResponse, *g_BleAdvCallbacks[i]);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts a scan with BleScanConfigs.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartScanEx(BleScanConfigs *configs)
{
    HILOGI("BleStartScanEx enter");
    if (g_BleCentralManager == nullptr || configs == nullptr) {
        HILOGE("BleStartScanEx fail, ble centra manager is null or configs is null.");
        return OHOS_BT_STATUS_FAIL;
    }

    HILOGI("scanMode: %{public}d", configs->scanMode);
    BleScanSettings settings;
    settings.SetScanMode(configs->scanMode);
    g_BleCentralManager->StartScan(settings);
    return OHOS_BT_STATUS_SUCCESS;
}
}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
/** @} */
