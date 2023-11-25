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

#ifndef OHOS_BLUETOOTH_STANDARD_HOST_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_HOST_PROXY_H

#include "bluetooth_raw_address.h"
#include "i_bluetooth_host.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHostProxy : public IRemoteProxy<IBluetoothHost> {
public:
    explicit BluetoothHostProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBluetoothHost>(impl)
    {}
    ~BluetoothHostProxy()
    {}

    void RegisterObserver(const sptr<IBluetoothHostObserver> &observer) override;
    void DeregisterObserver(const sptr<IBluetoothHostObserver> &observer) override;
    int32_t EnableBt() override;
    int32_t DisableBt() override;
    sptr<IRemoteObject> GetProfile(const std::string &name) override;
    sptr<IRemoteObject> GetBleRemote(const std::string &name) override;
    bool BluetoothFactoryReset() override;
    int32_t GetBtState(int &state) override;
    std::string GetLocalAddress() override;
    int32_t DisableBle() override;
    int32_t EnableBle() override;
    bool IsBrEnabled() override;
    bool IsBleEnabled() override;
    std::vector<uint32_t> GetProfileList() override;
    int32_t GetMaxNumConnectedAudioDevices() override;
    int32_t GetBtConnectionState(int &state) override;
    int32_t GetBtProfileConnState(uint32_t profileId, int &state) override;
    int32_t GetLocalDeviceClass() override;
    bool SetLocalDeviceClass(const int32_t &deviceClass) override;
    int32_t GetLocalName(std::string &name) override;
    int32_t SetLocalName(const std::string &name) override;
    int32_t GetBtScanMode(int32_t &scanMode) override;
    int32_t SetBtScanMode(int32_t mode, int32_t duration) override;
    int32_t GetBondableMode(const int32_t transport) override;
    bool SetBondableMode(int32_t transport, int32_t mode) override;
    int32_t StartBtDiscovery() override;
    int32_t CancelBtDiscovery() override;
    int32_t IsBtDiscovering(bool &isDisCovering, const int32_t transport) override;
    long GetBtDiscoveryEndMillis() override;
    int32_t GetPairedDevices(std::vector<BluetoothRawAddress> &pairedAddr) override;
    int32_t RemovePair(const int32_t transport, const sptr<BluetoothRawAddress> &device) override;
    bool RemoveAllPairs() override;
    void RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer) override;
    void DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer) override;
    int32_t GetBleMaxAdvertisingDataLength() override;
    int32_t GetDeviceType(int32_t transport, const std::string &address) override;
    int32_t GetPhonebookPermission(const std::string &address) override;
    bool SetPhonebookPermission(const std::string &address, int32_t permission) override;
    int32_t GetMessagePermission(const std::string &address) override;
    bool SetMessagePermission(const std::string &address, int32_t permission) override;
    int32_t GetPowerMode(const std::string &address) override;
    int32_t GetDeviceName(int32_t transport, const std::string &address, std::string &name) override;
    std::string GetDeviceAlias(const std::string &address) override;
    bool SetDeviceAlias(const std::string &address, const std::string &aliasName) override;
    int32_t GetDeviceBatteryLevel(const std::string &address) override;
    int32_t GetPairState(int32_t transport, const std::string &address, int32_t &pairState) override;
    int32_t StartPair(int32_t transport, const std::string &address) override;
    int32_t StartCrediblePair(int32_t transport, const std::string &address) override;
    bool CancelPairing(int32_t transport, const std::string &address) override;
    bool IsBondedFromLocal(int32_t transport, const std::string &address) override;
    bool IsAclConnected(int32_t transport, const std::string &address) override;
    bool IsAclEncrypted(int32_t transport, const std::string &address) override;
    int32_t GetDeviceClass(const std::string &address, int &cod) override;
    int32_t SetDevicePin(const std::string &address, const std::string &pin) override;
    int32_t SetDevicePairingConfirmation(int32_t transport, const std::string &address, bool accept) override;
    bool SetDevicePasskey(int32_t transport, const std::string &address, int32_t passkey, bool accept) override;
    bool PairRequestReply(int32_t transport, const std::string &address, bool accept) override;
    bool ReadRemoteRssiValue(const std::string &address) override;
    void GetLocalSupportedUuids(std::vector<std::string> &uuids) override;
    int32_t GetDeviceUuids(const std::string &address, std::vector<std::string> &uuids) override;
    int32_t GetLocalProfileUuids(std::vector<std::string> &uuids) override;
    void RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer) override;
    void DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer) override;
    void RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer) override;
    void DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer) override;
    int32_t SetFastScan(bool isEnable) override;
    int32_t GetRandomAddress(const std::string &realAddr, std::string &randomAddr) override;
    int32_t SyncRandomAddress(const std::string &realAddr, const std::string &randomAddr) override;
    int32_t CountEnableTimes(bool enable) override;
    int32_t ConnectAllowedProfiles(const std::string &remoteAddr) override;
    int32_t DisconnectAllowedProfiles(const std::string &remoteAddr) override;
private:
    int32_t InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);
    static inline BrokerDelegator<BluetoothHostProxy> delegator_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_HOST_PROXY_H
