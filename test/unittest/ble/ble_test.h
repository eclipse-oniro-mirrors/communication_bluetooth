/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef BLE_TEST_H
#define BLE_TEST_H

#include <gtest/gtest.h>
#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_socket.h"
#include "bluetooth_ble_central_manager_callback_stub.h"
#include "bluetooth_ble_central_manager_proxy.h"
#include "bluetooth_ble_advertiser_proxy.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {
using namespace testing::ext;

class BleAdvertiseCallbackTest : public Bluetooth::BleAdvertiseCallback {
public:
    BleAdvertiseCallbackTest(){};
    ~BleAdvertiseCallbackTest(){};

private:
    void OnStartResultEvent(int result, int advHandle) override;
    void OnEnableResultEvent(int result, int advHandle) override;
    void OnDisableResultEvent(int result, int advHandle) override;
    void OnStopResultEvent(int result, int advHandle) override;
    void OnSetAdvDataEvent(int result) override;
    void OnGetAdvHandleEvent(int result, int advHandle) override;
    void OnChangeAdvResultEvent(int result, int advHandle) override;
};

class BleHostObserverTest : public Bluetooth::BluetoothHostObserver {
public:
    BleHostObserverTest(){};
    ~BleHostObserverTest(){};

private:
    void OnStateChanged(const int transport, const int status);
    void OnDiscoveryStateChanged(int status)
    {}
    void OnDiscoveryResult(const BluetoothRemoteDevice &device, int rssi,
        const std::string deviceName, int deviceClass, int transport)
    {}
    void OnPairRequested(const BluetoothRemoteDevice &device)
    {}
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number)
    {}
    void OnScanModeChanged(int mode)
    {}
    void OnDeviceNameChanged(const std::string &device)
    {}
    void OnDeviceAddrChanged(const std::string &address)
    {}
};

class BleCentralManagerCallbackTest : public Bluetooth::BleCentralManagerCallback {
public:
    BleCentralManagerCallbackTest(){};
    ~BleCentralManagerCallbackTest(){};

private:
    void OnScanCallback(const Bluetooth::BleScanResult &result)
    {}
    void OnFoundOrLostCallback(const Bluetooth::BleScanResult &result, uint8_t callbackType)
    {}
    void OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult> &results)
    {}
    void OnStartOrStopScanEvent(int resultCode, bool isStartScan)
    {}
};

class BleTest : public testing::Test {
public:
    BleTest();
    ~BleTest();

    static BleTest *bleInstance_;
    Bluetooth::BleAdvertiserSettings bleAdvertiserSettings_ {};

    std::shared_ptr<BleAdvertiseCallbackTest> bleAdvertiseCallbackTest_ =
        std::make_shared<BleAdvertiseCallbackTest>();
    Bluetooth::BleScanSettings bleScanSettings_ {};
    Bluetooth::BleScanResult bleScanResult_ {};
    BleAdvertiserData advData1;
    BleAdvertiserData advData2;
    BleAdvertiserData scanData1;
    BleAdvertiserData scanData2;

    BleLpDeviceParamSet paramSet;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void InitAdvertiseSettings();
    void InitScanSettings();
    void InitScanResult();
    bool HaveUuid(Bluetooth::BleAdvertiserData advData);
    bool ManufacturerData(Bluetooth::BleAdvertiserData advData);
    bool ServiceData(Bluetooth::BleAdvertiserData advData);
    bool EnableBle();
    bool DisableBle();
    bool SetLocalName(const std::string &name);
    void InitAdvData();
    void InitScanData();
    void InitAdvFilterData();

private:
    bool isHaveUuid = false;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLE_TEST_H