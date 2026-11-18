/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "adapter_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_host_server.h"
#include "bluetooth_host_server.cpp"
#include "bluetooth_host.h"
#include "bluetooth_host_observer_stub.h"
#include "bluetooth_ble_peripheral_observer_stub.h"
#include "bluetooth_remote_device_observer_stub.h"
#include "bluetooth_resource_manager_observer_stub.h"
#include "bluetooth_oob_observer_stub.h"
#include "bluetooth_remote_device_info.h"

#include "adapter_manager.h"
#include "class_creator.h"
#include "ble_adapter.h"
#include "classic_adapter.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_access_token_mock.h"
#include "adapter_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {

const int TIME = 2;
static Bluetooth::BluetoothHost *host_;

class BluetoothHostObserverStubTest : public BluetoothHostObserverStub {
public:
    void OnStateChanged(int32_t transport, int32_t status) override
    {}
    void OnDiscoveryStateChanged(int32_t status) override
    {}
    void OnDiscoveryResult(const BluetoothRawAddress &device, int rssi, const std::string deviceName,
        int deviceClass, int transport) override
    {}
    void OnPairRequested(const int32_t transport, const BluetoothRawAddress &device) override
    {}
    void OnPairConfirmed(const int32_t transport, const BluetoothRawAddress &device, int reqType, int number) override
    {}
    void OnScanModeChanged(int32_t mode) override
    {}
    void OnDeviceNameChanged(const std::string &deviceName) override
    {}
    void OnDeviceAddrChanged(const std::string &address) override
    {}
    void OnBluetoothStateChanged(int32_t state) override
    {}
    void OnRefusePolicyChanged(const int32_t pid, const int64_t prohibitedSecondsTime) override
    {}
    void OnBluetoothSwitchAction(int32_t action, const std::string &callingName) override
    {}

    explicit BluetoothHostObserverStubTest()
    {}
    ~BluetoothHostObserverStubTest() override
    {}

private:
};

class BluetoothRemoteDeviceObserverstubTest : public BluetoothRemoteDeviceObserverstub {
public:
    void OnAclStateChanged(const BluetoothRawAddress &device, int32_t state, uint32_t reason) override
    {}
    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device,
        int32_t status, int32_t cause, const std::string &causeMessage) override
    {}
    void OnRemoteUuidChanged(const BluetoothRawAddress &device, const std::vector<bluetooth::Uuid> uuids) override
    {}
    void OnRemoteNameChanged(const BluetoothRawAddress &device, const std::string deviceName) override
    {}
    void OnRemoteAliasChanged(const BluetoothRawAddress &device, const std::string alias) override
    {}
    void OnRemoteCodChanged(const BluetoothRawAddress &device, int32_t cod) override
    {}
    void OnRemoteBatteryChanged(const BluetoothRawAddress &device, const BluetoothBatteryInfo &batteryInfo) override
    {}
    void OnRemoteDeviceCommonInfoReport(const BluetoothRawAddress &device, const std::vector<uint8_t> &value) override
    {}

    explicit BluetoothRemoteDeviceObserverstubTest()
    {}
    ~BluetoothRemoteDeviceObserverstubTest() override
    {}

private:
};

class BluetoothBlePeripheralObserverStubTest : public BluetoothBlePeripheralObserverStub {
public:
    void OnReadRemoteRssiEvent(const BluetoothRawAddress &device, int rssi, int status) override
    {}
    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device, int status, int cause,
        const std::string &causeMessage) override
    {}
    void OnAclStateChanged(const BluetoothRawAddress &device, int state, unsigned int reason) override
    {}

    explicit BluetoothBlePeripheralObserverStubTest()
    {}
    ~BluetoothBlePeripheralObserverStubTest() override
    {}

private:
};

class BluetoothResourceManagerObserverstubTest : public BluetoothResourceManagerObserverStub {
public:
    void OnSensingStateChanged(uint8_t eventId, const BluetoothSensingInfo &info) override
    {}
    void OnBluetoothResourceDecision(uint8_t eventId, const BluetoothSensingInfo &info, uint32_t &result) override
    {}

    BluetoothResourceManagerObserverstubTest()
    {}
    ~BluetoothResourceManagerObserverstubTest() override
    {}

private:
};

class BluetoothOobObserverStubTest : public BluetoothOobObserverStub {
public:
    void OnGenerateLocalOobData(int32_t ret, const BluetoothOobData &oobData) override
    {}

    BluetoothOobObserverStubTest()
    {}
    ~BluetoothOobObserverStubTest() override
    {}

private:
};

static void *BleAdapterCreate(void)
{
    return new BleAdapter();
}

static void *ClassicAdapterCreate(void)
{
    return new ClassicAdapter();
}

class HostServerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase");
        BluetoothAccessTokenMock::SetNativeTokenInfo();
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
        AdapterConfig::GetInstance()->Load();
        ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
        ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
        AdapterManager::GetInstance()->CreateAdapters();
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        sleep(TIME);
        utility::CloseAllTimer();
        host_ = nullptr;
    }
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<BluetoothHostServer> hostServer_;
};

void HostServerTest::SetUp()
{
    hostServer_ = std::make_unique<BluetoothHostServer>();
}

void HostServerTest::TearDown()
{
    hostServer_ = nullptr;
}

/**
 * @tc.name: host_server_register_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_register_001, TestSize.Level1)
{
    HILOGI("host_server_register_001 enter");
    sptr<BluetoothHostObserverStubTest> hostObserver = nullptr;
    hostServer_->RegisterObserver(hostObserver);
    hostServer_->DeregisterObserver(hostObserver);
    hostServer_->RegisterBleAdapterObserver(hostObserver);
    hostServer_->DeregisterBleAdapterObserver(hostObserver);

    hostObserver = new BluetoothHostObserverStubTest();
    hostServer_->RegisterObserver(hostObserver);
    hostServer_->DeregisterObserver(hostObserver);
    hostServer_->RegisterBleAdapterObserver(hostObserver);
    hostServer_->DeregisterBleAdapterObserver(hostObserver);

    sptr<BluetoothRemoteDeviceObserverstubTest> deviceObserver = nullptr;
    hostServer_->RegisterRemoteDeviceObserver(deviceObserver);
    hostServer_->DeregisterRemoteDeviceObserver(deviceObserver);

    deviceObserver = new BluetoothRemoteDeviceObserverstubTest();
    hostServer_->RegisterRemoteDeviceObserver(deviceObserver);
    hostServer_->DeregisterRemoteDeviceObserver(deviceObserver);

    sptr<BluetoothBlePeripheralObserverStubTest> pheralObserver = nullptr;
    hostServer_->RegisterBlePeripheralCallback(pheralObserver);
    hostServer_->DeregisterBlePeripheralCallback(pheralObserver);

    pheralObserver = new BluetoothBlePeripheralObserverStubTest();
    hostServer_->RegisterBlePeripheralCallback(pheralObserver);
    hostServer_->DeregisterBlePeripheralCallback(pheralObserver);

    sptr<IBluetoothResourceManagerObserver> resourceManagerObserver = nullptr;
    hostServer_->RegisterBtResourceManagerObserver(resourceManagerObserver);
    hostServer_->DeregisterBtResourceManagerObserver(resourceManagerObserver);

    int32_t state = 0;
    EXPECT_EQ(hostServer_->GetBtState(state), BT_NO_ERROR);
    HILOGI("host_server_register_001 end");
}

/**
 * @tc.name: host_server_getprofile_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_getprofile_001, TestSize.Level1)
{
    HILOGI("host_server_getprofile_001 enter");
    hostServer_->GetProfile("GattClientServer");
    hostServer_->GetBleRemote("BleAdvertiserServer");
    // delete hostServer OnStart to avoid libcesfwk_core cfi check
    hostServer_->GetProfile("GattClientServer");
    sptr<IRemoteObject> object = hostServer_->GetBleRemote("BleAdvertiserServer");
    EXPECT_TRUE(object == nullptr);
    HILOGI("host_server_getprofile_001 end");
}

/**
 * @tc.name: host_server_getdevicetype_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_getdevicetype_001, TestSize.Level1)
{
    HILOGI("host_server_getdevicetype_001 enter");
    std::string addr = "11:22:33:44:55:66";
    EXPECT_EQ(hostServer_->GetDeviceType(0, addr), INVALID_TYPE);
    EXPECT_EQ(hostServer_->GetDeviceType(1, addr), INVALID_TYPE);
    EXPECT_EQ(hostServer_->GetDeviceType(2, addr), INVALID_TYPE);
    HILOGI("host_server_getdevicetype_001 end");
}

/**
 * @tc.name: host_server_GetDeviceName_001
 * @tc.desc: HostServerTest:GetDeviceName
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetDeviceName_001, TestSize.Level1)
{
    HILOGI("host_server_GetDeviceName_001 enter");
    std::string addr = "11:22:33:44:55:66";
    std::string name;
    EXPECT_EQ(hostServer_->GetDeviceName(0, addr, name), BT_NO_ERROR);
    EXPECT_EQ(hostServer_->GetDeviceName(1, addr, name), BT_NO_ERROR);
    EXPECT_EQ(hostServer_->GetDeviceName(2, addr, name), BT_NO_ERROR);
    HILOGI("host_server_GetDeviceName_001 end");
}

/**
 * @tc.name: host_server_GetLocalAddress_001
 * @tc.desc: HostServerTest:GetLocalAddress
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetLocalAddress_001, TestSize.Level1)
{
    HILOGI("host_server_GetLocalAddress_001 enter");
    std::string addr = "";
    EXPECT_EQ(hostServer_->GetLocalAddress(addr), BT_NO_ERROR);
    HILOGI("host_server_GetLocalAddress_001 end");
}

/**
 * @tc.name: host_server_GenerateLocalOobData_001
 * @tc.desc: HostServerTest:GenerateLocalOobData
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GenerateLocalOobData_001, TestSize.Level1)
{
    HILOGI("host_server_GenerateLocalOobData_001 enter");
    int32_t transport = -2; // invalid transport
    sptr<BluetoothOobObserverStubTest> oobObserver = new BluetoothOobObserverStubTest();
    EXPECT_EQ(hostServer_->GenerateLocalOobData(transport, oobObserver), BT_ERR_INVALID_PARAM);
    transport = 0; // BT_TRANSPORT_BREDR
    EXPECT_EQ(hostServer_->GenerateLocalOobData(transport, oobObserver), BT_ERR_INTERNAL_ERROR);
    int32_t ret = BT_NO_ERROR;
    Bluetooth::BluetoothOobData oobData;
    hostServer_->pimpl->classicObserverImp_->OnGenerateLocalOobData(ret, oobData);
    HILOGI("host_server_GenerateLocalOobData_001 end");
}

/**
 * @tc.number: BluetoothOobData_001
 * @tc.name: BluetoothOobData
 * @tc.desc:
 */
HWTEST_F(HostServerTest, BluetoothOobData_001, TestSize.Level1)
{
    HILOGI("host_server_BluetoothOobData_001 start");
    BluetoothOobData oobData;
    std::vector<uint8_t> confirmHash = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10};
    std::vector<uint8_t> randomHash = {0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10, 0x11};
    std::vector<uint8_t> addressWithType = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01};
    oobData.SetConfirmationHash(confirmHash);
    oobData.SetRandomizerHash(randomHash);
    oobData.SetAddressWithType(addressWithType);
    oobData.SetOobDataType(OobDataType::P192);
    oobData.SetDeviceRole(LeDeviceRole::DEVICE_ROLE_BOTH_PREFER_CENTRAL);
    oobData.SetDeviceName("testName");
    EXPECT_TRUE(oobData.HasOobData());
    EXPECT_EQ(oobData.GetRandomizerHash(), randomHash);
    EXPECT_EQ(oobData.GetConfirmationHash(), confirmHash);
    EXPECT_EQ(oobData.GetAddressWithType(), addressWithType);
    EXPECT_EQ(oobData.GetOobDataType(), OobDataType::P192);
    EXPECT_EQ(oobData.GetDeviceRole(), LeDeviceRole::DEVICE_ROLE_BOTH_PREFER_CENTRAL);
    EXPECT_EQ(oobData.GetDeviceName(), "testName");
    HILOGI("host_server_BluetoothOobData_001 end");
}

/**
 * @tc.name: host_server_GetConnectedBLEDevices_001
 * @tc.desc: HostServerTest:GetConnectedBLEDevices
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetConnectedBLEDevices_001, TestSize.Level1)
{
    HILOGI("host_server_GetConnectedBLEDevices_001 enter");
    std::vector<std::string> connectedDevices;
    int32_t bleProfile = 0;
    hostServer_->GetConnectedBLEDevices(bleProfile, connectedDevices);
    EXPECT_EQ(connectedDevices.size(), 0);
    HILOGI("host_server_GetConnectedBLEDevices_001 end");
}


/**
 * @tc.name: host_server_get_set_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_get_set_001, TestSize.Level1)
{
    HILOGI("host_server_get_set_001 enter");
    EXPECT_EQ(hostServer_->GetBleMaxAdvertisingDataLength(), BT_NO_ERROR);

    std::string addr = "11:22:33:44:55:66";
    BluetoothBatteryInfo batteryInfo;
    hostServer_->GetRemoteDeviceBatteryInfo(addr, batteryInfo);

    hostServer_->SetDeviceCustomType(addr, 0);
    std::shared_ptr<BluetoothRemoteDeviceInfo> deviceInfo;
    hostServer_->GetRemoteDeviceInfo(addr, deviceInfo, bluetooth::DeviceInfoType::DEVICE_INFO_UNKNOWN);
    hostServer_->GetRemoteDeviceInfo(addr, deviceInfo, bluetooth::DeviceInfoType::DEVICE_VENDOR_ID);
    hostServer_->GetRemoteDeviceInfo(addr, deviceInfo, bluetooth::DeviceInfoType::DEVICE_PRODUCT_ID);
    hostServer_->GetRemoteDeviceInfo(addr, deviceInfo, bluetooth::DeviceInfoType::DEVICE_MODEL_ID);
    hostServer_->GetRemoteDeviceInfo(addr, deviceInfo, bluetooth::DeviceInfoType::DEVICE_CUSTOM_TYPE);
    HILOGI("host_server_get_set_001 end");
}

/**
 * @tc.name: host_server_GetDeviceAlias_001
 * @tc.desc: HostServerTest:GetDeviceAlias
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetDeviceAlias_001, TestSize.Level1)
{
    HILOGI("host_server_GetDeviceAlias_001 enter");
    std::string addr = "11:22:33:44:55:66";
    std::string aliasName = "bluetooth";
    EXPECT_EQ(hostServer_->SetDeviceAlias(addr, aliasName), BT_ERR_INVALID_PARAM);
    hostServer_->GetDeviceAlias(addr);
    HILOGI("host_server_GetDeviceAlias_001 end");
}

/**
 * @tc.name: host_server_GetLocalDeviceClass_001
 * @tc.desc: HostServerTest:GetLocalDeviceClass
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetLocalDeviceClass_001, TestSize.Level1)
{
    HILOGI("host_server_GetLocalDeviceClass_001 enter");
    EXPECT_FALSE(hostServer_->SetLocalDeviceClass(1));
    hostServer_->GetLocalDeviceClass();
    HILOGI("host_server_GetLocalDeviceClass_001 end");
}

/**
 * @tc.name: host_server_GetLocalName_001
 * @tc.desc: HostServerTest:GetLocalName
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetLocalName_001, TestSize.Level1)
{
    HILOGI("host_server_GetLocalName_001 enter");
    EXPECT_EQ(hostServer_->SetLocalName("OHOS_Bluetooth"), BT_ERR_INTERNAL_ERROR);
    std::string name;
    EXPECT_EQ(hostServer_->GetLocalName(name), BT_NO_ERROR);
    HILOGI("host_server_GetLocalName_001 end");
}

/**
 * @tc.name: host_server_GetBtScanMode_001
 * @tc.desc: HostServerTest:GetBtScanMode
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetBtScanMode_001, TestSize.Level1)
{
    HILOGI("host_server_GetBtScanMode_001 enter");
    EXPECT_EQ(hostServer_->SetBtScanMode(1, 100), BT_ERR_INTERNAL_ERROR);
    int32_t scanMode;
    EXPECT_EQ(hostServer_->GetBtScanMode(scanMode), BT_NO_ERROR);
    HILOGI("host_server_GetBtScanMode_001 end");
}

/**
 * @tc.name: host_server_btdiscovery_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_btdiscovery_001, TestSize.Level1)
{
    HILOGI("host_server_btdiscovery_001 enter");
    bool isDisCovering;
    // delete hostServer OnStart to avoid libcesfwk_core cfi check
    EXPECT_EQ(hostServer_->StartBtDiscovery(), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(hostServer_->IsBtDiscovering(isDisCovering, 0), BT_NO_ERROR);
    EXPECT_EQ(hostServer_->IsBtDiscovering(isDisCovering, 1), BT_NO_ERROR);
    EXPECT_EQ(hostServer_->IsBtDiscovering(isDisCovering, 2), BT_NO_ERROR);
    hostServer_->GetBtDiscoveryEndMillis();
    EXPECT_EQ(hostServer_->CancelBtDiscovery(), BT_ERR_INTERNAL_ERROR);
    HILOGI("host_server_btdiscovery_001 end");
}

/**
 * @tc.name: host_server_bonded_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_bonded_001, TestSize.Level1)
{
    HILOGI("host_server_bonded_001 enter");
    // delete hostServer OnStart to avoid libcesfwk_core cfi check
    EXPECT_TRUE(hostServer_->SetBondableMode(0, 1));
    hostServer_->GetBondableMode(0);
    EXPECT_TRUE(hostServer_->SetBondableMode(1, 1));
    hostServer_->GetBondableMode(1);
    EXPECT_TRUE(hostServer_->SetBondableMode(2, 1));
    hostServer_->GetBondableMode(2);
    HILOGI("host_server_bonded_001 end");
}

/**
 * @tc.name: host_server_GetCloudBondState_001
 * @tc.desc: HostServerTest:GetCloudBondState
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetCloudBondState_001, TestSize.Level1)
{
    HILOGI("host_server_GetCloudBondState_001 enter");
    std::string addr = "11:22:33:44:55:66";
    EXPECT_FALSE(hostServer_->IsBondedFromLocal(0, addr));
    EXPECT_FALSE(hostServer_->IsBondedFromLocal(1, addr));
    EXPECT_FALSE(hostServer_->IsBondedFromLocal(2, addr));
    int32_t cloudBondState = 0;
    EXPECT_EQ(hostServer_->GetCloudBondState(addr, cloudBondState), BT_ERR_INVALID_STATE);
    HILOGI("host_server_GetCloudBondState_001 end");
}

/**
 * @tc.name: host_server_btdiscovery_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_pair_001, TestSize.Level1)
{
    HILOGI("host_server_btdiscovery_001 enter");
    std::string addr = "11:22:33:44:55:66";
    int32_t addrType = AddressType::UNSET_ADDRESS;
    BluetoothRawAddress bluetoothRawAddress(addrType, addr);
    std::vector<BluetoothRawAddress> pairedAddr {};
    Bluetooth::BluetoothOobData dummyOobData;
    hostServer_->StartPair(0, bluetoothRawAddress, dummyOobData);
    hostServer_->StartPair(1, bluetoothRawAddress, dummyOobData);
    hostServer_->StartPair(2, bluetoothRawAddress, dummyOobData);
    hostServer_->GetPairedDevices(pairedAddr);
    hostServer_->StartCrediblePair(0, addr);
    hostServer_->StartCrediblePair(1, addr);
    hostServer_->StartCrediblePair(2, addr);
    int pairState;
    hostServer_->GetPairState(0, addr, pairState);
    hostServer_->GetPairState(1, addr, pairState);
    hostServer_->GetPairState(2, addr, pairState);
    hostServer_->CancelPairing(0, addr);
    hostServer_->CancelPairing(1, addr);
    hostServer_->CancelPairing(2, addr);
    sptr<BluetoothRawAddress> device = new BluetoothRawAddress(addr);
    hostServer_->RemovePair(0, device);
    hostServer_->RemovePair(1, device);
    hostServer_->RemovePair(2, device);
    hostServer_->RemovePair(2, nullptr);
    EXPECT_FALSE(hostServer_->RemoveAllPairs());
    HILOGI("host_server_btdiscovery_001 end");
}

/**
 * @tc.name: host_server_cancel_pairing_001
 * @tc.desc: Cancel a device which is not in the pairing state via CancelPairing,
 *           expect BT_ERR_DEVICE_NOT_PAIRING (2900019)
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_cancel_pairing_001, TestSize.Level1)
{
    HILOGI("host_server_cancel_pairing_001 enter");
    EXPECT_EQ(BT_ERR_PAIRING_INITIATED_BY_OTHER, 2900018);
    EXPECT_EQ(BT_ERR_DEVICE_NOT_PAIRING, 2900019);
    std::string addr = "11:22:33:44:55:66";
    EXPECT_EQ(hostServer_->CancelPairing(0, addr), BT_ERR_DEVICE_NOT_PAIRING);
    EXPECT_EQ(hostServer_->CancelPairing(1, addr), BT_ERR_DEVICE_NOT_PAIRING);
    EXPECT_EQ(hostServer_->CancelPairing(2, addr), BT_ERR_DEVICE_NOT_PAIRING);
    HILOGI("host_server_cancel_pairing_001 end");
}

/**
 * @tc.name: host_server_cancel_pairing_002
 * @tc.desc: A system application cancels a pairing initiated by another application,
 *           the originator check is bypassed
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_cancel_pairing_002, TestSize.Level1)
{
    HILOGI("host_server_cancel_pairing_002 enter");
    std::string addr = "11:22:33:44:55:66";
    EXPECT_EQ(hostServer_->CheckPairingInitiator(addr, true, "", ""), BT_NO_ERROR);
    HILOGI("host_server_cancel_pairing_002 end");
}

/**
 * @tc.name: host_server_cancel_pairing_003
 * @tc.desc: A third-party application cancels a pairing without an initiator record,
 *           expect BT_ERR_PAIRING_INITIATED_BY_OTHER (2900018)
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_cancel_pairing_003, TestSize.Level1)
{
    HILOGI("host_server_cancel_pairing_003 enter");
    std::string addr = "11:22:33:44:55:66";
    EXPECT_EQ(hostServer_->CheckPairingInitiator(addr, false, "appIdentifierA", "pkgA"),
        BT_ERR_PAIRING_INITIATED_BY_OTHER);
    HILOGI("host_server_cancel_pairing_003 end");
}

/**
 * @tc.name: host_server_cancel_pairing_004
 * @tc.desc: A third-party application cancels a pairing initiated by another application,
 *           expect BT_ERR_PAIRING_INITIATED_BY_OTHER (2900018)
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_cancel_pairing_004, TestSize.Level1)
{
    HILOGI("host_server_cancel_pairing_004 enter");
    std::string addr = "11:22:33:44:55:66";
    hostServer_->pimpl->pairingInitiatorMap_.EnsureInsert(addr,
        PairRecord {GetNowTimestamp(), "appIdentifierA", "pkgA", true});
    EXPECT_EQ(hostServer_->CheckPairingInitiator(addr, false, "appIdentifierB", "pkgB"),
        BT_ERR_PAIRING_INITIATED_BY_OTHER);
    HILOGI("host_server_cancel_pairing_004 end");
}

/**
 * @tc.name: host_server_cancel_pairing_005
 * @tc.desc: The initiator application cancels its own pairing, the originator check passes
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_cancel_pairing_005, TestSize.Level1)
{
    HILOGI("host_server_cancel_pairing_005 enter");
    std::string addr = "11:22:33:44:55:66";
    hostServer_->pimpl->pairingInitiatorMap_.EnsureInsert(addr,
        PairRecord {GetNowTimestamp(), "appIdentifierA", "pkgA", true});
    EXPECT_EQ(hostServer_->CheckPairingInitiator(addr, false, "appIdentifierA", "pkgA"), BT_NO_ERROR);
    HILOGI("host_server_cancel_pairing_005 end");
}

/**
 * @tc.name: host_server_cancel_pairing_006
 * @tc.desc: A stale initiator record left by an interrupted pairing cannot authorize cancellation,
 *           expect BT_ERR_PAIRING_INITIATED_BY_OTHER (2900018)
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_cancel_pairing_006, TestSize.Level1)
{
    HILOGI("host_server_cancel_pairing_006 enter");
    std::string addr = "11:22:33:44:55:66";
    // The record is 10 minutes old, far beyond the 120s validity period.
    hostServer_->pimpl->pairingInitiatorMap_.EnsureInsert(addr,
        PairRecord {GetNowTimestamp() - 10 * 60 * 1000, "appIdentifierA", "pkgA", true});
    EXPECT_EQ(hostServer_->CheckPairingInitiator(addr, false, "appIdentifierA", "pkgA"),
        BT_ERR_PAIRING_INITIATED_BY_OTHER);
    HILOGI("host_server_cancel_pairing_006 end");
}

/**
 * @tc.name: host_server_acl_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_acl_001, TestSize.Level1)
{
    HILOGI("host_server_acl_001 enter");
    std::string addr = "11:22:33:44:55:66";
    hostServer_->IsAclConnected(0, addr);
    hostServer_->IsAclConnected(1, addr);
    hostServer_->IsAclConnected(2, addr);

    hostServer_->IsAclEncrypted(0, addr);
    hostServer_->IsAclEncrypted(1, addr);
    EXPECT_FALSE(hostServer_->IsAclEncrypted(2, addr));
    HILOGI("host_server_acl_001 end");
}

/**
 * @tc.name: host_server_acl_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_Device_001, TestSize.Level1)
{
    HILOGI("host_server_acl_001 enter");
    int32_t cod;
    std::string addr = "11:22:33:44:55:66";
    hostServer_->GetDeviceClass(addr, cod);
    std::vector<std::string> uuids {};
    hostServer_->GetDeviceUuids(addr, uuids);
    hostServer_->GetLocalProfileUuids(uuids);
    EXPECT_TRUE(hostServer_->ReadRemoteRssiValue(addr));
    HILOGI("host_server_acl_001 end");
}

/**
 * @tc.name: host_server_pin_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_pin_001, TestSize.Level1)
{
    HILOGI("host_server_pin_001 enter");
    std::string addr = "11:22:33:44:55:66";
    hostServer_->SetDevicePin(addr, "123456");
    hostServer_->SetDevicePairingConfirmation(0, addr, true);
    hostServer_->SetDevicePairingConfirmation(1, addr, true);
    hostServer_->SetDevicePairingConfirmation(2, addr, true);

    hostServer_->SetDevicePasskey(0, addr, 1, true);
    hostServer_->SetDevicePasskey(1, addr, 1, true);
    hostServer_->SetDevicePasskey(2, addr, 1, true);

    hostServer_->PairRequestReply(0, addr, true);
    hostServer_->PairRequestReply(1, addr, true);
    EXPECT_FALSE(hostServer_->PairRequestReply(2, addr, true));
    HILOGI("host_server_pin_001 end");
}

/**
 * @tc.name: host_server_dump_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_dump_001, TestSize.Level1)
{
    HILOGI("host_server_dump_001 enter");
    int32_t fd = 1;
    std::vector<std::u16string> args;
    EXPECT_EQ(hostServer_->Dump(fd, args), 0);
    HILOGI("host_server_dump_001 end");
}

/**
 * @tc.name: host_server_fastscan_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_fastscan_001, TestSize.Level1)
{
    HILOGI("host_server_fastscan_001 enter");
    EXPECT_EQ(hostServer_->SetFastScan(true), BT_ERR_INVALID_STATE);
    EXPECT_EQ(hostServer_->SetFastScanLevel(0x01), BT_ERR_INVALID_STATE);
    HILOGI("host_server_fastscan_001 end");
}

/**
 * @tc.name: host_server_ConnectAllowedProfiles_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_ConnectAllowedProfiles_001, TestSize.Level1)
{
    HILOGI("host_server_ConnectAllowedProfiles_001 enter");
    // delete hostServer OnStart to avoid libcesfwk_core cfi check
    bluetooth::RawAddress device = bluetooth::RawAddress("00:11:22:33:44:66");
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetNeedDelayConnectFlag(false);
    remoteDevice->SetDeviceType(1);
    hostServer_->ConnectAllowedProfiles("00:11:22:33:44:66");
    remoteDevice->SetNeedDelayConnectFlag(true);
    EXPECT_EQ(hostServer_->ConnectAllowedProfiles("00:11:22:33:44:66"), BT_NO_ERROR);
    HILOGI("host_server_ConnectAllowedProfiles_001 end");
}

/**
 * @tc.name: host_server_DisconnectAllowedProfiles_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_DisconnectAllowedProfiles_001, TestSize.Level1)
{
    HILOGI("host_server_DisconnectAllowedProfiles_001 enter");
    // delete hostServer OnStart to avoid libcesfwk_core cfi check
    EXPECT_EQ(hostServer_->DisconnectAllowedProfiles("11:22:33:44:55:66"), BT_NO_ERROR);
    HILOGI("host_server_DisconnectAllowedProfiles_001 end");
}

/**
 * @tc.name: host_server_RemovePairInner_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_RemovePairInner_001, TestSize.Level1)
{
    HILOGI("host_server_RemovePairInner_001 enter");
    MessageParcel dataEmpty;
    MessageParcel reply;
    EXPECT_EQ(hostServer_->RemovePairInner(dataEmpty, reply), BT_ERR_IPC_TRANS_FAILED);
    HILOGI("host_server_RemovePairInner_001 end");
}

/**
 * @tc.name: host_server_IsSupportVirtualAutoConnect_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_IsSupportVirtualAutoConnect_001, TestSize.Level1)
{
    HILOGI("host_server_IsSupportVirtualAutoConnect_001 enter");
    std::string addr = "00:11:22:33:44:55";
    bool support = false;
    EXPECT_EQ(hostServer_->IsSupportVirtualAutoConnect(addr, support), BT_NO_ERROR);
    HILOGI("host_server_IsSupportVirtualAutoConnect_001 end");
}

/**
 * @tc.name: host_server_SetVirtualAutoConnectType_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_SetVirtualAutoConnectType_001, TestSize.Level1)
{
    HILOGI("host_server_SetVirtualAutoConnectType_001 enter");
    std::string addr = "00:11:22:33:44:55";
    hostServer_->SetVirtualAutoConnectType(addr, 0x01, 0);
    hostServer_->SetVirtualAutoConnectType(addr, 0x02, 3);
    EXPECT_EQ(hostServer_->SetVirtualAutoConnectType(addr, 0x03, 0), BT_NO_ERROR);
    HILOGI("host_server_SetVirtualAutoConnectType_001 end");
}

/**
 * @tc.name: host_server_FreezeObservers_OnFunctions_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_FreezeObservers_OnFunctions_001, TestSize.Level1)
{
    HILOGI("host_server_FreezeObservers_OnFunctions_001 enter");
    sptr<BluetoothResourceManagerObserverstubTest> resourceMgrObserver = new BluetoothResourceManagerObserverstubTest();
    sptr<BluetoothBlePeripheralObserverStubTest> pheralObserver  = new BluetoothBlePeripheralObserverStubTest();
    sptr<BluetoothHostObserverStubTest> classicObserver = new BluetoothHostObserverStubTest();
    sptr<BluetoothHostObserverStubTest> bleObserver = new BluetoothHostObserverStubTest();
    sptr<BluetoothRemoteDeviceObserverstubTest> deviceObserver = new BluetoothRemoteDeviceObserverstubTest();

    hostServer_->RegisterBtResourceManagerObserver(resourceMgrObserver);
    hostServer_->RegisterBlePeripheralCallback(pheralObserver);
    hostServer_->RegisterObserver(classicObserver);
    hostServer_->RegisterBleAdapterObserver(bleObserver);
    hostServer_->RegisterRemoteDeviceObserver(deviceObserver);

    int32_t pid = hostServer_->pimpl->resourceMgrAppContainer_->GetRemotePid(resourceMgrObserver->AsObject());

    OHOS::bluetooth::SensingInfo info;
    std::string strAddr = "11:22:33:44:55:66";
    RawAddress rawAddr(strAddr);
    std::string deviceName = "testDevice";
    std::string alias = "testAlias";
    uint8_t eventId = 0;
    uint32_t result = 0;
    int rssi = 0, status = 0, cause = 0, state = 0;
    int32_t mode = 0;
    bluetooth::BTTransport transport = bluetooth::BTTransport::ADAPTER_BREDR;

    BluetoothBleCentralManagerServer::proxyPids_.clear();
    EXPECT_TRUE(BluetoothBleCentralManagerServer::proxyPids_.count(pid) == 0);

    hostServer_->pimpl->observerImp_->OnStateChange(transport, bluetooth::BTStateID::STATE_TURN_ON);
    hostServer_->pimpl->observerImp_->OnBluetoothStateChanged(state);
    hostServer_->pimpl->observerImp_->OnRefusePolicyChanged(pid, 100);

    hostServer_->pimpl->classicObserverImp_->OnDiscoveryStateChanged(status);
    hostServer_->pimpl->classicObserverImp_->OnDiscoveryResult(rawAddr, rssi, deviceName, 0, 0);
    hostServer_->pimpl->classicObserverImp_->OnPairRequested(transport, rawAddr);
    hostServer_->pimpl->classicObserverImp_->OnPairConfirmed(transport, rawAddr, 0, 0);
    hostServer_->pimpl->classicObserverImp_->OnScanModeChanged(mode);
    hostServer_->pimpl->classicObserverImp_->OnDeviceNameChanged(deviceName);
    hostServer_->pimpl->classicObserverImp_->OnDeviceAddrChanged(strAddr);

    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    hostServer_->pimpl->remoteObserverImp_->CheckDiscoverOrAccessPermission(tokenId);
    hostServer_->pimpl->remoteObserverImp_->OnAclStateChanged(rawAddr, state, cause, 0);
    hostServer_->pimpl->remoteObserverImp_->OnPairStatusChanged(transport, rawAddr, status, cause, "");
    std::vector<Uuid> uuids;
    hostServer_->pimpl->remoteObserverImp_->OnRemoteUuidChanged(rawAddr, uuids);
    hostServer_->pimpl->remoteObserverImp_->OnRemoteNameChanged(rawAddr, deviceName);
    hostServer_->pimpl->remoteObserverImp_->OnRemoteAliasChanged(rawAddr, alias);
    hostServer_->pimpl->remoteObserverImp_->OnRemoteCodChanged(rawAddr, 0);
    BatteryInfo batteryInfo;
    hostServer_->pimpl->remoteObserverImp_->OnRemoteBatteryChanged(rawAddr, batteryInfo);
    std::vector<uint8_t> value = {0x01, 0x09, 0x20, 0x02};
    hostServer_->pimpl->remoteObserverImp_->OnRemoteDeviceCommonInfoReport(rawAddr, value);

    hostServer_->pimpl->bleObserverImp_->CheckDiscoverOrAccessPermission(tokenId);
    hostServer_->pimpl->bleObserverImp_->OnDiscoveryStateChanged(0);
    hostServer_->pimpl->bleObserverImp_->OnDiscoveryResult(rawAddr, rssi, deviceName, 0, 0);
    hostServer_->pimpl->bleObserverImp_->OnPairRequested(transport, rawAddr);
    hostServer_->pimpl->bleObserverImp_->OnPairConfirmed(transport, rawAddr, 0, 1);
    hostServer_->pimpl->bleObserverImp_->OnScanModeChanged(mode);
    hostServer_->pimpl->bleObserverImp_->OnDeviceNameChanged(deviceName);
    hostServer_->pimpl->bleObserverImp_->OnDeviceAddrChanged(strAddr);

    hostServer_->pimpl->bleRemoteObserverImp_->OnReadRemoteRssiEvent(rawAddr, rssi, status);
    hostServer_->pimpl->bleRemoteObserverImp_->OnPairStatusChanged(transport, rawAddr, status, cause, "");
    hostServer_->pimpl->bleRemoteObserverImp_->OnAclStateChanged(rawAddr, state, cause, state);

    hostServer_->pimpl->resourceMgrObserverImp_->OnSensingStateChanged(eventId, info);
    hostServer_->pimpl->resourceMgrObserverImp_->OnBluetoothResourceDecision(eventId, info, result);

    BluetoothBleCentralManagerServer::proxyPids_[pid] = 0;
    EXPECT_TRUE(BluetoothBleCentralManagerServer::proxyPids_.count(pid) == 1);

    hostServer_->pimpl->resourceMgrObserverImp_->OnSensingStateChanged(eventId, info);
    hostServer_->pimpl->resourceMgrObserverImp_->OnBluetoothResourceDecision(eventId, info, result);
    hostServer_->pimpl->bleRemoteObserverImp_->OnReadRemoteRssiEvent(rawAddr, rssi, status);
    hostServer_->pimpl->classicObserverImp_->OnDeviceAddrChanged(strAddr);
    hostServer_->pimpl->bleObserverImp_->OnDeviceAddrChanged(strAddr);
    hostServer_->pimpl->classicObserverImp_->OnDeviceNameChanged(deviceName);
    hostServer_->pimpl->bleObserverImp_->OnDeviceNameChanged(deviceName);
    hostServer_->pimpl->classicObserverImp_->OnScanModeChanged(mode);
    hostServer_->pimpl->bleObserverImp_->OnScanModeChanged(mode);
    hostServer_->pimpl->remoteObserverImp_->OnRemoteAliasChanged(rawAddr, alias);

    sptr<BluetoothHostServer> object = new BluetoothHostServer();
    sptr<IRemoteObject> remote = object->AsObject();
    sptr<BluetoothHostObserverStubTest> hostObserver = nullptr;
    hostObserver = new BluetoothHostObserverStubTest();
    hostServer_->RegisterObserver(hostObserver);
    if (hostServer_->pimpl->fusionConnectivityObserverDeathRecipient_ != nullptr) {
        hostServer_->pimpl->fusionConnectivityObserverDeathRecipient_->OnRemoteDied(remote);
        EXPECT_TRUE(hostServer_->pimpl->fusionConnectivityObserver_ == nullptr);
    }
    hostServer_->DeregisterObserver(hostObserver);

    HILOGI("host_server_FreezeObservers_OnFunctions_001 end");
}

/**
 * @tc.name: host_server_ConnectionState_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_ConnectionState_001, TestSize.Level1)
{
    HILOGI("host_server_ConnectionState_001 enter");
    int32_t state;
    EXPECT_EQ(hostServer_->GetBtConnectionState(state), BT_ERR_INVALID_STATE);
    hostServer_->GetBtProfileConnState(1, state);

    int64_t connectionTime = 0;
    std::string addr = "11:22:33:44:55:66";
    EXPECT_EQ(hostServer_->GetLastConnectionTime(addr, connectionTime), BT_ERR_INVALID_STATE);
    HILOGI("host_server_ConnectionState_001 end");
}

/*
 * @tc.name: host_server_EnableBleNoAutoConnect_001
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_EnableBle_001, TestSize.Level1)
{
    HILOGI("host_server_EnableBleNoAutoConnect_001 enter");
    hostServer_->EnableBle(false);
    EXPECT_FALSE(AdapterManager::GetInstance()->GetQuietMode());

    hostServer_->EnableBle(true);
    EXPECT_TRUE(AdapterManager::GetInstance()->GetQuietMode());
    HILOGI("host_server_EnableBleNoAutoConnect_001 end");
}

/**
 * @tc.name: host_server_register_002
 * @tc.desc: HostServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_register_002, TestSize.Level1)
{
    HILOGI("host_server_register_001 enter");
    std::unique_ptr<BluetoothHostServer> hostServerTest = std::make_unique<BluetoothHostServer>();
    for (int i = 0; i <= 1001; i++) {
        sptr<IBluetoothHostObserver> observer;
        hostServer_->RegisterObserver(observer);
    }
    std::vector<BluetoothTrustPairDevice> cloudDevices;
    EXPECT_EQ(hostServer_->UpdateCloudBluetoothDevice(cloudDevices), BT_NO_ERROR);
    HILOGI("host_server_register_002 end");
}

/**
 * @tc.name: host_server_GetPairedAppIdentifier_001
 * @tc.desc: get appIdentifier of hap
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetPairedAppIdentifier_001, TestSize.Level1)
{
    HILOGI("host_server_GetPairedAppIdentifier_001 enter");
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    std::string result = hostServer_->GetPairedAppIdentifier(tokenId);
    EXPECT_TRUE(result.empty());
    HILOGI("host_server_GetPairedAppIdentifier_001 end");
}

/**
 * @tc.name: host_server_GetRandomAddress_001
 * @tc.desc: get random address
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetRandomAddress_001, TestSize.Level1)
{
    HILOGI("host_server_GetRandomAddress_001 enter");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:55");
    RawAddress randomAddr1 = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr1);
    std::string randomAddr2 = "";
    int ret = hostServer_->GetRandomAddress(realAddr1.GetAddress(), randomAddr2);
    EXPECT_EQ(ret, BT_NO_ERROR);
    ret = hostServer_->GetRandomAddress(realAddr1.GetAddress(), randomAddr2, 1);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("host_server_GetRandomAddress_001 end");
}

/**
 * @tc.name: host_server_OnSvcCmd_001
 * @tc.desc: disable bluetooth
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_OnSvcCmd_001, TestSize.Level1)
{
    HILOGI("host_server_OnSvcCmd_001 enter");
    std::vector<std::u16string> args = {u"disable"};
    int32_t svcResult = hostServer_->OnSvcCmd(1, args);
    EXPECT_EQ(svcResult, 0);
    HILOGI("host_server_OnSvcCmd_001 end");
}

/**
 * @tc.name: host_server_OnSvcCmd_002
 * @tc.desc: enable bluetooth
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_OnSvcCmd_002, TestSize.Level1)
{
    HILOGI("host_server_OnSvcCmd_002 enter");
    std::vector<std::u16string> args = {u"enable"};
    int32_t svcResult = hostServer_->OnSvcCmd(1, args);
    EXPECT_EQ(svcResult, -1);
    HILOGI("host_server_OnSvcCmd_002 end");
}

/**
 * @tc.name: host_server_OnSvcCmd_003
 * @tc.desc: help bluetooth
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_OnSvcCmd_003, TestSize.Level1)
{
    HILOGI("host_server_OnSvcCmd_003 enter");
    std::vector<std::u16string> args = {u"help"};
    int32_t svcResult = hostServer_->OnSvcCmd(1, args);
    EXPECT_EQ(svcResult, 0);
    HILOGI("host_server_OnSvcCmd_003 end");
}

/**
 * @tc.name: host_server_OnSvcCmd_004
 * @tc.desc: wrong parameter
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_OnSvcCmd_004, TestSize.Level1)
{
    HILOGI("host_server_OnSvcCmd_004 enter");
    std::vector<std::u16string> args1 = {u"wrong"};
    int32_t svcResult = hostServer_->OnSvcCmd(1, args1);
    EXPECT_EQ(svcResult, -1);
    std::vector<std::u16string> args2 = {u"wrong", u"wrong2"};
    svcResult = hostServer_->OnSvcCmd(1, args2);
    EXPECT_EQ(svcResult, -1);
    HILOGI("host_server_OnSvcCmd_004 end");
}

/**
 * @tc.name: host_server_StartRemoteSdpSearch_001
 * @tc.desc: StartRemoteSdpSearch
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_StartRemoteSdpSearch_001, TestSize.Level1)
{
    HILOGI("host_server_StartRemoteSdpSearch_001 enter");
    std::string address = "00:11:22:33:44:55";
    std::string uuid = "00000000-0000-1000-8000-00805f9b34fb";
    EXPECT_EQ(hostServer_->StartRemoteSdpSearch(address, uuid), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(hostServer_->GetRemoteServices(address), BT_ERR_INTERNAL_ERROR);
    HILOGI("host_server_StartRemoteSdpSearch_001 end");
}

/**
 * @tc.name: host_server_IsProfileExist_001
 * @tc.desc: IsProfileExist
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_IsProfileExist_001, TestSize.Level1)
{
    HILOGI("host_server_IsProfileExist_001 enter");
    std::string profileName = "INVALID NAME";
    bool isProfileExist = true;
    int res = hostServer_->IsProfileExist(profileName, isProfileExist);
    EXPECT_FALSE(isProfileExist);
    HILOGI("host_server_IsProfileExist_001 end");
}

/**
 * @tc.name: host_server_GetVirtualAddressByHash_001
 * @tc.desc: GetVirtualAddressByHash
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetVirtualAddressByHash_001, TestSize.Level1)
{
    HILOGI("host_server_GetVirtualAddressByHash_001 enter");
    int hashType = 0;
    const std::string hashValue = "11111111";
    std::string virtualAddress;
    EXPECT_EQ(hostServer_->GetVirtualAddressByHash(hashType, hashValue, virtualAddress), BT_ERR_UNPAIRED_DEVICE);
    HILOGI("host_server_GetVirtualAddressByHash_001 end");
}

/**
 * @tc.name: host_server_IsAllowedConnectProfile_001
 * @tc.desc: IsAllowedConnectProfile
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_IsAllowedConnectProfile_001, TestSize.Level1)
{
    HILOGI("host_server_IsAllowedConnectProfile_001 enter");
    std::string address = "00:11:22:33:44:66";
    EXPECT_TRUE(hostServer_->IsAllowedConnectProfile(address));
    HILOGI("host_server_IsAllowedConnectProfile_001 end");
}

/**
 * @tc.name: host_server_IsAllowedDisconnectProfile_001
 * @tc.desc: IsAllowedDisconnectProfile
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_IsAllowedDisconnectProfile_001, TestSize.Level1)
{
    HILOGI("host_server_IsAllowedDisconnectProfile_001 enter");
    std::string address = "00:11:22:33:44:AA";
    bool isSystemHap = true;
    EXPECT_TRUE(hostServer_->IsAllowedDisconnectProfile(isSystemHap, address));
    isSystemHap = false;
    EXPECT_TRUE(hostServer_->IsAllowedDisconnectProfile(isSystemHap, address));
    HILOGI("host_server_IsAllowedDisconnectProfile_001 end");
}

/**
 * @tc.name: host_server_DumpRssFreezeInfo_001
 * @tc.desc: DumpRssFreezeInfo
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_DumpRssFreezeInfo_001, TestSize.Level1)
{
    HILOGI("host_server_DumpRssFreezeInfo_001 enter");
    std::string ret = hostServer_->DumpRssFreezeInfo();
    EXPECT_NE(ret, "\nInfo ERROR\n");
    HILOGI("host_server_DumpRssFreezeInfo_001 end");
}

/**
 * @tc.name: host_server_EnableBluetoothToRestrictMode_001
 * @tc.desc: EnableBluetoothToRestrictMode
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_EnableBluetoothToRestrictMode_001, TestSize.Level1)
{
    HILOGI("host_server_EnableBluetoothToRestrictMode_001 enter");
    std::string callingName = "com.test.callingName";
    EXPECT_EQ(hostServer_->EnableBluetoothToRestrictMode(callingName, false), BT_ERR_INTERNAL_ERROR);
    HILOGI("host_server_EnableBluetoothToRestrictMode_001 end");
}

/**
 * @tc.name: host_server_GetDeviceTransport_001
 * @tc.desc: GetDeviceTransport
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetDeviceTransport_001, TestSize.Level1)
{
    HILOGI("host_server_GetDeviceTransport_001 enter");
    std::string address = "00:11:22:33:44:55";
    int32_t transport = 0;
    EXPECT_EQ(hostServer_->GetDeviceTransport(address, transport), BT_NO_ERROR);
    HILOGI("host_server_GetDeviceTransport_001 end");
}

/**
 * @tc.name: host_server_ProcessRandomDeviceIdCommand_001
 * @tc.desc: ProcessRandomDeviceIdCommand
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_ProcessRandomDeviceIdCommand_001, TestSize.Level1)
{
    HILOGI("host_server_ProcessRandomDeviceIdCommand_001 enter");
    int32_t command = static_cast<int>(RandomDeviceIdCommand::GET);
    std::vector<std::string> deviceIdVec { "1", "2"};
    bool isValid = true;
    EXPECT_EQ(hostServer_->ProcessRandomDeviceIdCommand(command, deviceIdVec, isValid), BT_NO_ERROR);
    command = static_cast<int>(RandomDeviceIdCommand::ADD);
    EXPECT_EQ(hostServer_->ProcessRandomDeviceIdCommand(command, deviceIdVec, isValid), BT_ERR_INVALID_PARAM);
    command = static_cast<int>(RandomDeviceIdCommand::DELETE);
    EXPECT_EQ(hostServer_->ProcessRandomDeviceIdCommand(command, deviceIdVec, isValid), BT_ERR_INVALID_PARAM);
    command = static_cast<int>(RandomDeviceIdCommand::IS_VALID);
    EXPECT_EQ(hostServer_->ProcessRandomDeviceIdCommand(command, deviceIdVec, isValid), BT_ERR_INVALID_PARAM);
    HILOGI("host_server_ProcessRandomDeviceIdCommand_001 end");
}

/**
 * @tc.name: host_server_SetRemoteDeviceBatteryInfo_001
 * @tc.desc: SetRemoteDeviceBatteryInfo
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_SetRemoteDeviceBatteryInfo_001, TestSize.Level1)
{
    HILOGI("host_server_SetRemoteDeviceBatteryInfo_001 enter");
    std::string address = "11:22:33:44:55:66";
    BluetoothBatteryInfo batteryInfo;
    EXPECT_EQ(hostServer_->SetRemoteDeviceBatteryInfo(address, batteryInfo), BT_ERR_PERMISSION_FAILED);
    HILOGI("host_server_SetRemoteDeviceBatteryInfo_001 end");
}

/**
 * @tc.name: host_server_SetBtChannelScan_001
 * @tc.desc: SetBtChannelScan
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_SetBtChannelScan_001, TestSize.Level1)
{
    HILOGI("host_server_SetBtChannelScan_001 enter");
    int invalidInterval = CHANNEL_SCAN_INTERVAL_100MS - 1;
    EXPECT_EQ(hostServer_->SetBtChannelScan(true, invalidInterval), BT_ERR_INVALID_PARAM);
    EXPECT_EQ(hostServer_->SetBtChannelScan(true, CHANNEL_SCAN_INTERVAL_100MS), BT_NO_ERROR);
    EXPECT_EQ(hostServer_->SetBtChannelScan(true, CHANNEL_SCAN_INTERVAL_1000MS), BT_NO_ERROR);
    EXPECT_EQ(hostServer_->SetBtChannelScan(false, CHANNEL_SCAN_INTERVAL_BYPASS), BT_NO_ERROR);
    HILOGI("host_server_SetBtChannelScan_001 end");
}

/**
 * @tc.name: host_server_GetBrAddressByBleAddress_001
 * @tc.desc: GetBrAddressByBleAddress with valid BLE address
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetBrAddressByBleAddress_001, TestSize.Level1)
{
    HILOGI("host_server_GetBrAddressByBleAddress_001 enter");
    std::string bleAddr = "1A:2B:3C:4D:5E:6F";
    std::string brAddr;
    int32_t ret = hostServer_->GetBrAddressByBleAddress(bleAddr, brAddr);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("host_server_GetBrAddressByBleAddress_001 end");
}

/**
 * @tc.name: host_server_GetBrAddressByBleAddress_002
 * @tc.desc: GetBrAddressByBleAddress with empty BLE address
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetBrAddressByBleAddress_002, TestSize.Level1)
{
    HILOGI("host_server_GetBrAddressByBleAddress_002 enter");
    std::string bleAddr = "";
    std::string brAddr;
    int32_t ret = hostServer_->GetBrAddressByBleAddress(bleAddr, brAddr);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("host_server_GetBrAddressByBleAddress_002 end");
}

/**
 * @tc.name: host_server_GetBleMacByBrMac_001
 * @tc.desc: GetBleMacByBrMac with empty BR address
 * @tc.type: FUNC
 */
HWTEST_F(HostServerTest, host_server_GetBleMacByBrMac_001, TestSize.Level1)
{
    HILOGI("host_server_GetBleMacByBrMac_001 enter");
    std::string brMac = "";
    std::string bleMac;
    int32_t ret = hostServer_->GetBleMacByBrMac(brMac, bleMac);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("host_server_GetBleMacByBrMac_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS