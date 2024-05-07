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

#ifndef OHOS_BLUETOOTH_STANDARD_HOST_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_HOST_INTERFACE_H

#include "bluetooth_raw_address.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "parcel_bt_uuid.h"
#include "i_bluetooth_ble_peripheral_observer.h"
#include "i_bluetooth_host_observer.h"
#include "i_bluetooth_remote_device_observer.h"
#include "iremote_broker.h"
#include "../parcel/bluetooth_device_battery_info.h"
#include "../parcel/bluetooth_remote_device_info.h"

namespace OHOS {
namespace Bluetooth {
namespace {
const std::string PROFILE_GATT_CLIENT = "GattClientServer";
const std::string PROFILE_GATT_SERVER = "GattServerServer";
const std::string PROFILE_A2DP_SRC = "A2dpSrcServer";
const std::string PROFILE_A2DP_SINK = "A2dpSnkServer";
const std::string PROFILE_AVRCP_CT = "AvrcpCtServer";
const std::string PROFILE_AVRCP_TG = "AvrcpTgServer";
const std::string PROFILE_HFP_AG = "HfpAgServer";
const std::string PROFILE_HFP_HF = "HfpHfServer";
const std::string PROFILE_MAP_MCE = "MapMceServer";
const std::string PROFILE_MAP_MSE = "MapMseServer";
const std::string PROFILE_PBAP_PCE = "PbapPceServer";
const std::string PROFILE_PBAP_PSE = "PbapPseServer";
const std::string PROFILE_SOCKET = "SocketServer";
const std::string PROFILE_DI = "DIServer";
const std::string BLE_ADVERTISER_SERVER = "BleAdvertiserServer";
const std::string BLE_CENTRAL_MANAGER_SERVER = "BleCentralMangerServer";
const std::string PROFILE_HID_HOST_SERVER = "BluetoothHidHostServer";
const std::string PROFILE_PAN_SERVER = "BluetoothPanServer";
const std::string PROFILE_OPP_SERVER = "BluetoothOppServer";
const std::string PROFILE_AUDIO_MANAGER = "BluetoothAudioManagerServer";
}  // namespace

class IBluetoothHost : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothHost");

    virtual void RegisterObserver(const sptr<IBluetoothHostObserver> &observer) = 0;
    virtual void DeregisterObserver(const sptr<IBluetoothHostObserver> &observer) = 0;
    virtual int32_t EnableBt() = 0;
    virtual int32_t DisableBt() = 0;
    virtual sptr<IRemoteObject> GetProfile(const std::string &name) = 0;
    virtual sptr<IRemoteObject> GetBleRemote(const std::string &name) = 0;
    virtual int32_t BluetoothFactoryReset() = 0;
    virtual int32_t GetBtState(int &state) = 0;
    virtual int32_t GetLocalAddress(std::string &addr) = 0;
    virtual int32_t DisableBle() = 0;
    virtual int32_t EnableBle() = 0;
    virtual bool IsBrEnabled() = 0;
    virtual bool IsBleEnabled() = 0;
    virtual std::vector<uint32_t> GetProfileList() = 0;
    virtual int32_t GetMaxNumConnectedAudioDevices() = 0;
    virtual int32_t GetBtConnectionState(int &state) = 0;
    virtual int32_t GetBtProfileConnState(uint32_t profileId, int &state) = 0;
    virtual int32_t GetLocalDeviceClass() = 0;
    virtual bool SetLocalDeviceClass(const int32_t &deviceClass) = 0;
    virtual int32_t GetLocalName(std::string &name) = 0;
    virtual int32_t SetLocalName(const std::string &name) = 0;
    virtual int32_t GetBtScanMode(int32_t &scanMode) = 0;
    virtual int32_t SetBtScanMode(int32_t mode, int32_t duration) = 0;
    virtual int32_t GetBondableMode(const int32_t transport) = 0;
    virtual bool SetBondableMode(int32_t transport, int32_t mode) = 0;
    virtual int32_t StartBtDiscovery() = 0;
    virtual int32_t CancelBtDiscovery() = 0;
    virtual int32_t IsBtDiscovering(bool &isDisCovering, const int32_t transport) = 0;
    virtual long GetBtDiscoveryEndMillis() = 0;
    virtual int32_t GetPairedDevices(std::vector<BluetoothRawAddress> &pairedAddr) = 0;
    virtual int32_t RemovePair(const int32_t transport, const sptr<BluetoothRawAddress> &device) = 0;
    virtual bool RemoveAllPairs() = 0;
    virtual void RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer) = 0;
    virtual void DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer) = 0;
    virtual int32_t GetBleMaxAdvertisingDataLength() = 0;
    virtual int32_t GetDeviceType(int32_t transport, const std::string &address) = 0;
    virtual int32_t GetPhonebookPermission(const std::string &address) = 0;
    virtual bool SetPhonebookPermission(const std::string &address, int32_t permission) = 0;
    virtual int32_t GetMessagePermission(const std::string &address) = 0;
    virtual bool SetMessagePermission(const std::string &address, int32_t permission) = 0;
    virtual int32_t GetPowerMode(const std::string &address) = 0;
    virtual int32_t GetDeviceName(int32_t transport, const std::string &address, std::string &name) = 0;
    virtual std::string GetDeviceAlias(const std::string &address) = 0;
    virtual int32_t SetDeviceAlias(const std::string &address, const std::string &aliasName) = 0;
    virtual int32_t GetRemoteDeviceBatteryInfo(const std::string &address, BluetoothBatteryInfo &batteryInfo) = 0;
    virtual int32_t GetPairState(int32_t transport, const std::string &address, int32_t &pairState) = 0;
    virtual int32_t StartPair(int32_t transport, const std::string &address) = 0;
    virtual int32_t StartCrediblePair(int32_t transport, const std::string &address) = 0;
    virtual bool CancelPairing(int32_t transport, const std::string &address) = 0;
    virtual bool IsBondedFromLocal(int32_t transport, const std::string &address) = 0;
    virtual bool IsAclConnected(int32_t transport, const std::string &address) = 0;
    virtual bool IsAclEncrypted(int32_t transport, const std::string &address) = 0;
    virtual int32_t GetDeviceClass(const std::string &address, int &cod) = 0;
    virtual int32_t SetDevicePin(const std::string &address, const std::string &pin) = 0;
    virtual int32_t SetDevicePairingConfirmation(int32_t transport, const std::string &address, bool accept) = 0;
    virtual bool SetDevicePasskey(int32_t transport, const std::string &address, int32_t passkey, bool accept) = 0;
    virtual bool PairRequestReply(int32_t transport, const std::string &address, bool accept) = 0;
    virtual bool ReadRemoteRssiValue(const std::string &address) = 0;
    virtual void GetLocalSupportedUuids(std::vector<std::string> &uuids) = 0;
    virtual int32_t GetDeviceUuids(const std::string &address, std::vector<std::string> &uuids) = 0;
    virtual int32_t GetLocalProfileUuids(std::vector<std::string> &uuids) = 0;
    virtual void RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer) = 0;
    virtual void DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer) = 0;
    virtual void RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer) = 0;
    virtual void DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer) = 0;
    virtual int32_t SetFastScan(bool isEnable) = 0;
    virtual int32_t GetRandomAddress(const std::string &realAddr, std::string &randomAddr) = 0;
    virtual int32_t SyncRandomAddress(const std::string &realAddr, const std::string &randomAddr) = 0;
    virtual int32_t CountEnableTimes(bool enable) = 0;
    virtual int32_t ConnectAllowedProfiles(const std::string &remoteAddr) = 0;
    virtual int32_t DisconnectAllowedProfiles(const std::string &remoteAddr) = 0;
    virtual int32_t GetDeviceProductId(const std::string &address, std::string &prodcutId) = 0;
    virtual int32_t SetDeviceCustomType(const std::string &address, int32_t deviceType) = 0;
    virtual int32_t GetRemoteDeviceInfo(const std::string &address,
        std::shared_ptr<BluetoothRemoteDeviceInfo> &deviceInfo) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_HOST_INTERFACE_H
