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

#include <gtest/gtest.h>
#include <mutex>
#include <thread>

#include "bluetooth_access_token_mock.h"
#include "bluetooth_a2dp_src.h"
#include "bluetooth_def.h"
#include "bluetooth_device_class.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_utils.h"
#include "bluetooth_device.h"
#include "bluetooth_log.h"
#include "bluetooth_host_observer_stub.h"
#include "bluetooth_proxy_manager.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_no_destructor.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

sptr<IRemoteObject> hostObject;
BluetoothHost *host_;
int g_btState = -1;
constexpr int TIME = 4;
constexpr int TIME_10 = 10;

class TestBluetoothRemoteDeviceObserver : public BluetoothRemoteDeviceObserver {
public:
    TestBluetoothRemoteDeviceObserver();
    ~TestBluetoothRemoteDeviceObserver() override;

    void OnAclStateChanged(const BluetoothRemoteDevice &device, int state, unsigned int reason) override;
    void OnPairStatusChanged(const BluetoothRemoteDevice &device, int status, int cause) override;
    void OnRemoteUuidChanged(const BluetoothRemoteDevice &device, const std::vector<ParcelUuid> &uuids) override;
    void OnRemoteNameChanged(const BluetoothRemoteDevice &device, const std::string &deviceName) override;
    void OnRemoteAliasChanged(const BluetoothRemoteDevice &device, const std::string &alias) override;
    void OnRemoteCodChanged(const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod) override;
    void OnRemoteBatteryLevelChanged(const BluetoothRemoteDevice &device, int batteryLevel) override;
    void OnReadRemoteRssiEvent(const BluetoothRemoteDevice &device, int rssi, int status) override;
    void OnPairStatusChangedWithMessage(
        const BluetoothRemoteDevice &device, int status, int cause, const std::string &causeMessage) override;
};

class TestBluetoothHostObserverStub : public BluetoothHostObserverStub {
public:
    TestBluetoothHostObserverStub();
    ~TestBluetoothHostObserverStub() override;
    void OnStateChanged(int32_t transport, int32_t status) override;

    // ON_DIS_STA_CHANGE_CODE
    void OnDiscoveryStateChanged(int32_t status) override;
    void OnDiscoveryResult(const BluetoothRawAddress &device, int rssi, const std::string deviceName,
        int deviceClass, int transport) override;
    void OnPairRequested(const int32_t transport, const BluetoothRawAddress &device) override;
    void OnPairConfirmed(
        const int32_t transport, const BluetoothRawAddress &device, int reqType, int number) override;
    void OnScanModeChanged(int32_t mode) override;
    void OnDeviceNameChanged(const std::string &deviceName) override;
    void OnDeviceAddrChanged(const std::string &address) override;
    void OnBluetoothStateChanged(int32_t state) override
    {}
    void OnRefusePolicyChanged(const int32_t pid, const int64_t prohibitedSecondsTime) override
    {}
    void OnBluetoothSwitchAction(int32_t action, const std::string &callingName) override
    {}
};

class TestBluetoothResourceManagerObserver : public BluetoothResourceManagerObserver {
public:
    TestBluetoothResourceManagerObserver();
    ~TestBluetoothResourceManagerObserver() override;

    void OnSensingStateChanged(uint8_t eventId, const SensingInfo &info) override;
    void OnBluetoothResourceDecision(uint8_t eventId, const SensingInfo &info, uint32_t &result) override;
};

TestBluetoothHostObserverStub::TestBluetoothHostObserverStub() {};
TestBluetoothHostObserverStub::~TestBluetoothHostObserverStub() {};
void TestBluetoothHostObserverStub::OnStateChanged(int32_t transport, int32_t status) {};
void TestBluetoothHostObserverStub::OnDiscoveryStateChanged(int32_t status) {};
void TestBluetoothHostObserverStub::OnDiscoveryResult(const BluetoothRawAddress &device, int rssi,
    const std::string deviceName, int deviceClass, int transport) {};
void TestBluetoothHostObserverStub::OnPairRequested(const int32_t transport, const BluetoothRawAddress &device) {};
void TestBluetoothHostObserverStub::OnPairConfirmed(
    const int32_t transport, const BluetoothRawAddress &device, int reqType, int number) {};
void TestBluetoothHostObserverStub::OnScanModeChanged(int32_t mode) {};
void TestBluetoothHostObserverStub::OnDeviceNameChanged(const std::string &deviceName) {};
void TestBluetoothHostObserverStub::OnDeviceAddrChanged(const std::string &address) {};

TestBluetoothRemoteDeviceObserver::TestBluetoothRemoteDeviceObserver() {};
TestBluetoothRemoteDeviceObserver::~TestBluetoothRemoteDeviceObserver() {};

void TestBluetoothRemoteDeviceObserver::OnAclStateChanged(const BluetoothRemoteDevice &device,
    int state, unsigned int reason) {};
void TestBluetoothRemoteDeviceObserver::OnPairStatusChanged(const BluetoothRemoteDevice &device,
    int status, int cause) {};
void TestBluetoothRemoteDeviceObserver::OnRemoteUuidChanged(const BluetoothRemoteDevice &device,
    const std::vector<ParcelUuid> &uuids) {};
void TestBluetoothRemoteDeviceObserver::OnRemoteNameChanged(const BluetoothRemoteDevice &device,
    const std::string &deviceName) {};
void TestBluetoothRemoteDeviceObserver::OnRemoteAliasChanged(const BluetoothRemoteDevice &device,
    const std::string &alias) {};
void TestBluetoothRemoteDeviceObserver::OnRemoteCodChanged(const BluetoothRemoteDevice &device,
    const BluetoothDeviceClass &cod) {};
void TestBluetoothRemoteDeviceObserver::OnRemoteBatteryLevelChanged(const BluetoothRemoteDevice &device,
    int batteryLevel) {};
void TestBluetoothRemoteDeviceObserver::OnReadRemoteRssiEvent(const BluetoothRemoteDevice &device,
    int rssi, int status) {};
void TestBluetoothRemoteDeviceObserver::OnPairStatusChangedWithMessage(
    const BluetoothRemoteDevice &device, int status, int cause, const std::string &causeMessage) {};

TestBluetoothResourceManagerObserver::TestBluetoothResourceManagerObserver() {};
TestBluetoothResourceManagerObserver::~TestBluetoothResourceManagerObserver() {};

void TestBluetoothResourceManagerObserver::OnSensingStateChanged(uint8_t eventId, const SensingInfo &info) {};
void TestBluetoothResourceManagerObserver::OnBluetoothResourceDecision(
    uint8_t eventId, const SensingInfo &info, uint32_t &result) {};

class BluetoothHostObserverCommon : public BluetoothHostObserver {
public:
    BluetoothHostObserverCommon() = default;
    virtual ~BluetoothHostObserverCommon() = default;
    static std::shared_ptr<BluetoothHostObserverCommon> GetInstance();
    void OnStateChanged(const int transport, const int status) override;
    void OnBluetoothStateChanged(int state) override{};
    void OnDiscoveryStateChanged(int status) override;
    void OnDiscoveryResult(const BluetoothRemoteDevice &device, int rssi,
        const std::string deviceName, int deviceClass, int transport) override{};
    void OnPairRequested(const BluetoothRemoteDevice &device) override{};
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) override{};
    void OnDeviceNameChanged(const std::string &deviceName) override{};
    void OnScanModeChanged(int mode) override{};
    void OnDeviceAddrChanged(const std::string &address) override{};
};

class TestBluetoothOobCallback : public BluetoothOobCallback {
public:
    TestBluetoothOobCallback() = default;
    virtual ~TestBluetoothOobCallback() = default;

    void OnGenerateLocalOobData(int32_t ret, const OobData &oobData) override
    {
        HILOGI("TestBluetoothOobCallback::OnGenerateLocalOobData called");
    }
};

std::shared_ptr<BluetoothHostObserverCommon> BluetoothHostObserverCommon::GetInstance()
{
    static std::shared_ptr<BluetoothHostObserverCommon> instance = std::make_shared<BluetoothHostObserverCommon>();
    return instance;
}

void BluetoothHostObserverCommon::OnStateChanged(const int transport, const int status)
{
    if (transport == BT_TRANSPORT_BLE) {
        switch (status) {
            case STATE_TURNING_ON:
                HILOGI("BLE:Turning on ...");
                break;
            case STATE_TURN_ON:
                HILOGI("BLE:Turn on");
                break;
            case STATE_TURNING_OFF:
                HILOGI("BLE:Turning off ...");
                break;
            case STATE_TURN_OFF:
                HILOGI("BLE:Turn off");
                break;
            default:
                break;
        }
        return;
    } else {
        switch (status) {
            case STATE_TURNING_ON:
                HILOGI("BREDR:Turning on ...");
                break;
            case STATE_TURN_ON:
                HILOGI("BREDR:Turn on");
                break;
            case STATE_TURNING_OFF:
                HILOGI("BREDR:Turning off ...");
                break;
            case STATE_TURN_OFF:
                HILOGI("BREDR:Turn off");
                break;
            default:
                break;
        }
        return;
    }
}

void BluetoothHostObserverCommon::OnDiscoveryStateChanged(int status)
{
    switch (status) {
        case 0x01:
            HILOGI("discovery_start");
            break;
        case 0x02:
            HILOGI("discoverying");
            break;
        case 0x03:
            HILOGI("discovery_done");
            break;
        default:
            break;
    }
}

class HostTest : public testing::Test {
public:
    HostTest()
    {}
    ~HostTest()
    {}

    std::shared_ptr<BluetoothHostObserverCommon> btObserver_ = BluetoothHostObserverCommon::GetInstance();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HostTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
    host_ = &BluetoothHost::GetDefaultHost();
    hostObject = BluetoothProfileManager::GetInstance().GetProfileRemote("HostObject");

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}

void HostTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    host_ = nullptr;
    hostObject = nullptr;
}

void HostTest::SetUp()
{
}

void HostTest::TearDown()
{
}

/**
 * @tc.number: Host_ModuleTest_GetBtState_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtState_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtState_00100 start");

    EXPECT_EQ(host_->GetBtState(), BTStateID::STATE_TURN_ON);

    HILOGI("Host_ModuleTest_GetBtState_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetProfileList_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetProfileList_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetProfileList_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_FALSE(host_->GetProfileList().empty());

    HILOGI("Host_ModuleTest_GetProfileList_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetMaxNumConnectedAudioDevices_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetMaxNumConnectedAudioDevices_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetMaxNumConnectedAudioDevices_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_EQ(host_->GetMaxNumConnectedAudioDevices(), 6);

    HILOGI("Host_ModuleTest_GetMaxNumConnectedAudioDevices_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBtProfileConnState_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtProfileConnState_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtProfileConnState_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    host_->GetBtProfileConnState(PROFILE_ID_GATT_CLIENT, state);
    EXPECT_EQ(state, (int)BTConnectState::DISCONNECTED);

    HILOGI("Host_ModuleTest_GetBtProfileConnState_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBtConnectionState_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtConnectionState_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtConnectionState_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_EQ(host_->GetBtConnectionState(), (int)BTConnectState::DISCONNECTED);

    HILOGI("Host_ModuleTest_GetBtConnectionState_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_Start_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_Start_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_Start_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_TRUE(host_->Start());
    host_->Stop();
    HILOGI("Host_ModuleTest_Start_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetLocalDeviceClass_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetLocalDeviceClass_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetLocalDeviceClass_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    BluetoothDeviceClass remotCod(0x2A0104);
    EXPECT_TRUE(host_->SetLocalDeviceClass(remotCod));
    BluetoothDeviceClass getLocalcod = host_->GetLocalDeviceClass();
    EXPECT_EQ(remotCod.GetMajorClass(), getLocalcod.GetMajorClass());
    EXPECT_EQ(remotCod.GetMajorMinorClass(), getLocalcod.GetMajorMinorClass());

    HILOGI("Host_ModuleTest_GetLocalDeviceClass_00100 end");
}

/**
 * @tc.number: BluetoothDeviceClass::IsProfileSupported
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_IsProfileSupported_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_IsProfileSupported_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    BluetoothDeviceClass remotCod(0x2A0104);
    EXPECT_TRUE(host_->SetLocalDeviceClass(remotCod));
    BluetoothDeviceClass getLocalcod = host_->GetLocalDeviceClass();

    EXPECT_FALSE(getLocalcod.IsProfileSupported(BluetoothDevice::PROFILE_A2DP));
    EXPECT_TRUE(getLocalcod.IsProfileSupported(BluetoothDevice::PROFILE_A2DP_SINK));
    EXPECT_FALSE(getLocalcod.IsProfileSupported(BluetoothDevice::PROFILE_HEADSET));
    EXPECT_FALSE(getLocalcod.IsProfileSupported(BluetoothDevice::PROFILE_HID));
    EXPECT_TRUE(getLocalcod.IsProfileSupported(BluetoothDevice::PROFILE_PANU));
    EXPECT_TRUE(getLocalcod.IsProfileSupported(BluetoothDevice::PROFILE_NAP));
    EXPECT_FALSE(getLocalcod.IsProfileSupported(-1));

    HILOGI("Host_ModuleTest_IsProfileSupported_00100 end");
}

/**
 * @tc.number: BluetoothDeviceClass::IsServiceSupported
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_IsServiceSupported_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_IsServiceSupported_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    BluetoothDeviceClass remotCod(0x2A0104);
    EXPECT_TRUE(host_->SetLocalDeviceClass(remotCod));
    BluetoothDeviceClass getLocalcod = host_->GetLocalDeviceClass();

    EXPECT_FALSE(getLocalcod.IsServiceSupported(BluetoothDevice::SERVICE_LIMITED_DISCOVERABILITY));

    HILOGI("Host_ModuleTest_IsServiceSupported_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_SetLocalDeviceClass_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_SetLocalDeviceClass_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_SetLocalDeviceClass_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    BluetoothDeviceClass remotCod(0x10100);
    EXPECT_TRUE(host_->SetLocalDeviceClass(remotCod));
    BluetoothDeviceClass getLocalcod = host_->GetLocalDeviceClass();
    EXPECT_EQ(remotCod.GetMajorClass(), getLocalcod.GetMajorClass());

    HILOGI("Host_ModuleTest_SetLocalDeviceClass_00100 end");
}

/**
 * @tc.number: BluetoothDeviceClass::IsXXXXSupported
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_IsXXXXSupported_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_IsXXXXSupported_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    BluetoothDeviceClass remotCod(0x10100);
    EXPECT_TRUE(host_->SetLocalDeviceClass(remotCod));

    HILOGI("Host_ModuleTest_IsXXXXSupported_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetLocalAddress_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetLocalAddress_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetLocalAddress_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    std::string addr = "";
    host_->GetLocalAddress(addr);
    EXPECT_NE(addr, "");

    HILOGI("Host_ModuleTest_GetLocalAddress_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GenerateLocalOobData_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GenerateLocalOobData_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GenerateLocalOobData_00100 start");
    int transport = 0; // BT_TRANSPORT_BREDR
    auto callback = std::make_shared<BluetoothOobCallback>();
    int ret = host_->GenerateLocalOobData(transport, callback);
    sleep(1);
    EXPECT_EQ(ret, BT_NO_ERROR);
    std::shared_ptr<TestBluetoothOobCallback> oobCallback = std::make_shared<TestBluetoothOobCallback>();
    OobData oobData;
    callback->OnGenerateLocalOobData(ret, oobData);
    oobCallback->OnGenerateLocalOobData(ret, oobData);
    HILOGI("Host_ModuleTest_GenerateLocalOobData_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetLocalName_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetLocalName_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetLocalName_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    std::string localName = "localName";
    EXPECT_EQ(host_->SetLocalName(localName), BT_NO_ERROR);
    host_->GetLocalName();

    HILOGI("Host_ModuleTest_GetLocalName_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_SetLocalName_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_SetLocalName_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_SetLocalName_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    std::string localName = "localName";
    EXPECT_EQ(host_->SetLocalName(localName), BT_NO_ERROR);
    host_->GetLocalName();

    HILOGI("Host_ModuleTest_SetLocalName_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_SetBtScanMode_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_SetBtScanMode_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_SetBtScanMode_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_EQ(host_->SetBtScanMode(SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE, 0), BT_NO_ERROR);

    HILOGI("Host_ModuleTest_SetBtScanMode_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_SetBtScanMode_00200
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_SetBtScanMode_00200, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_SetBtScanMode_00200 start");

    const int scanModeInvalid = -1;
    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_NE(host_->SetBtScanMode(scanModeInvalid, 0), BT_NO_ERROR);

    HILOGI("Host_ModuleTest_SetBtScanMode_00200 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBtScanMode_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtScanMode_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtScanMode_00100 start");

    sleep(1);
    host_ = &BluetoothHost::GetDefaultHost();
    int scanmode = 0;
    EXPECT_EQ(host_->GetBtScanMode(scanmode), BT_NO_ERROR);

    HILOGI("Host_ModuleTest_GetBtScanMode_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_SetBondableMode_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_SetBondableMode_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_SetBondableMode_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_TRUE(host_->SetBondableMode(BT_TRANSPORT_BREDR, BONDABLE_MODE_OFF));

    HILOGI("Host_ModuleTest_SetBondableMode_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBtScanMode_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBondableMode_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBondableMode_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_EQ(host_->GetBondableMode(BTTransport::ADAPTER_BREDR), BONDABLE_MODE_OFF);

    HILOGI("Host_ModuleTest_GetBondableMode_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_StartBtDiscovery_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_StartBtDiscovery_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_StartBtDiscovery_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_EQ(host_->StartBtDiscovery(), BT_NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    bool isDiscovering = false;
    EXPECT_EQ(host_->IsBtDiscovering(isDiscovering, BTTransport::ADAPTER_BREDR), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_StartBtDiscovery_00100 end");
}


/**
 * @tc.number: Host_ModuleTest_CancelBtDiscovery_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_CancelBtDiscovery_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_CancelBtDiscovery_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_EQ(host_->CancelBtDiscovery(), BT_NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    HILOGI("Host_ModuleTest_CancelBtDiscovery_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBtDiscoveryEndMillis_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtDiscoveryEndMillis_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtDiscoveryEndMillis_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    long time = host_->GetBtDiscoveryEndMillis();
    EXPECT_NE(time, -1);
    HILOGI("Host_ModuleTest_GetBtDiscoveryEndMillis_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetPairedDevices_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetPairedDevices_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetPairedDevices_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    std::vector<BluetoothRemoteDevice> remotedeviceList;
    host_->GetPairedDevices(BT_TRANSPORT_BREDR, remotedeviceList);
    EXPECT_EQ((int)remotedeviceList.size(), 0);

    HILOGI("Host_ModuleTest_GetPairedDevices_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_RemovePair_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_RemovePair_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_RemovePair_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);

    EXPECT_NE(host_->RemovePair(device_), BT_NO_ERROR);

    HILOGI("Host_ModuleTest_RemovePair_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_RemoveAllPairs_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_RemoveAllPairs_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_RemoveAllPairs_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_TRUE(host_->RemoveAllPairs());

    HILOGI("Host_ModuleTest_RemoveAllPairs_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_IsValidBluetoothAddr_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_IsValidBluetoothAddr_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_IsValidBluetoothAddr_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_TRUE(host_->IsValidBluetoothAddr("00:00:00:00:00:00"));

    HILOGI("Host_ModuleTest_IsValidBluetoothAddr_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_IsValidBluetoothAddr_00200
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_IsValidBluetoothAddr_00200, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_IsValidBluetoothAddr_00200 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_FALSE(host_->IsValidBluetoothAddr("00"));

    HILOGI("Host_ModuleTest_IsValidBluetoothAddr_00200 end");
}

/**
 * @tc.number: DeregisterObserver
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_DeregisterObserver_00100 start");

    host_->DeregisterObserver(btObserver_);
    EXPECT_NE(nullptr, host_);

    HILOGI("Host_ModuleTest_DeregisterObserver_00100 end");
}

/**
 * @tc.number: GetRandomAddress
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetRandomAddress_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetRandomAddress_00100 start");

    std::string randomAddr;
    int ret = host_->GetRandomAddress("11:22:33:44:55:66", randomAddr);
    EXPECT_NE(randomAddr, "");

    HILOGI("Host_ModuleTest_GetRandomAddress_00100 end ret:%{public}d", ret);
}

/**
 * @tc.number: GetRandomAddress
 * @tc.name:
 * @tc.desc: IsValidBluetoothAddr()
 */
HWTEST_F(HostTest, Host_ModuleTest_GetRandomAddress_00200, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetRandomAddress_00200 start");

    std::string randomAddr;
    int ret = host_->GetRandomAddress("QQ:QQ:QQ#00#00#00", randomAddr);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("Host_ModuleTest_GetRandomAddress_00200 end ret:%{public}d", ret);
}

/**
 * @tc.number: GetRealAddress
 * @tc.name:
 * @tc.desc: GetRealAddress()
 */
HWTEST_F(HostTest, Host_ModuleTest_GetRealAddress_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetRealAddress_00100 start");

    std::string realAddr;
    int ret = host_->GetRealAddress("11:22:33:xx:55:66", realAddr);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);
    HILOGI("Host_ModuleTest_GetRealAddress_00100 end ret:%{public}d", ret);
}

/**
 * @tc.number: Host_ModuleTest_GetBtState_00300
 * @tc.name: GetBtState(int)
 * @tc.desc: function test
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtState_00300, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtState_00300 start");

    int state;
    host_ = &BluetoothHost::GetDefaultHost();
    int ret = host_->GetBtState(state);
    EXPECT_EQ(ret, BTStateID::STATE_TURNING_ON);

    HILOGI("Host_ModuleTest_GetBtState_00300 end ret:%{public}d", ret);
}

/**
 * @tc.number: GetVirtualAddressByHash_00100
 * @tc.name:GetVirtualAddressByHash
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetVirtualAddressByHash_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetVirtualAddressByHash_00100 start");
    int hashType = 0;
    const std::string hashValue = "11111111";
    std::string virtualAddress;
    EXPECT_EQ(host_->GetVirtualAddressByHash(hashType, hashValue, virtualAddress), BT_ERR_PARAM_NOT_COMPLIANT);
    HILOGI("Host_ModuleTest_GetVirtualAddressByHash_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_IsValidBluetoothAddr_00300
 * @tc.name:
 * @tc.desc: addr.empty():true
 */
HWTEST_F(HostTest, Host_ModuleTest_IsValidBluetoothAddr_00300, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_IsValidBluetoothAddr_00300 start");

    bool ret = host_->IsValidBluetoothAddr("");
    EXPECT_FALSE(ret);

    HILOGI("Host_ModuleTest_IsValidBluetoothAddr_00300 end ret:%{public}d", ret);
}

/**
 * @tc.number: Host_ModuleTest_IsValidBluetoothAddr_00400
 * @tc.name:
 * @tc.desc: (c >= 'A' && c <= 'F'):false
 */
HWTEST_F(HostTest, Host_ModuleTest_IsValidBluetoothAddr_00400, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_IsValidBluetoothAddr_00400 start");

    bool ret = host_->IsValidBluetoothAddr("QQ:QQ:QQ#00#00#00");
    EXPECT_FALSE(ret);

    HILOGI("Host_ModuleTest_IsValidBluetoothAddr_00400 end ret:%{public}d", ret);
}

/**
 * @tc.number: Host_ModuleTest_GetBtConnectionState_00400
 * @tc.name: GetBtConnectionState(int)
 * @tc.desc: function test
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtConnectionState_00400, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtConnectionState_00400 start");
    int state;
    int ret = host_->GetBtConnectionState(state);
    EXPECT_EQ(state, (int)BTConnectState::DISCONNECTED);

    HILOGI("Host_ModuleTest_GetBtConnectionState_00400 end state:%{public}d, ret:%{public}d", state, ret);
}

/**
 * @tc.number: Host_ModuleTest_GetLocalSupportedUuids_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetLocalSupportedUuids_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetLocalSupportedUuids_00100 start");

    std::vector<ParcelUuid> stringUuids;
    host_->GetLocalSupportedUuids(stringUuids);
    EXPECT_NE(nullptr, &stringUuids);

    HILOGI("Host_ModuleTest_GetLocalSupportedUuids_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetLocalName_00300
 * @tc.name: GetLocalName(std::string)
 * @tc.desc: function test
 */
HWTEST_F(HostTest, Host_ModuleTest_GetLocalName_00300, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetLocalName_00300 start");

    std::string localName = "localName";
    std::string name;
    host_->SetLocalName(localName);
    sleep(1);
    EXPECT_EQ(host_->GetLocalName(name), BT_NO_ERROR);

    HILOGI("Host_ModuleTest_GetLocalName_00300 end");
}

/**
 * @tc.number: Host_ModuleTest_RemovePair_00300
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(HostTest, Host_ModuleTest_RemovePair_00300, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_RemovePair_00300 start");

    host_ = &BluetoothHost::GetDefaultHost();
    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);

    EXPECT_NE(host_->RemovePair(device_), BT_NO_ERROR);

    HILOGI("Host_ModuleTest_RemovePair_00300 end");
}

/**
 * @tc.number: Host_ModuleTest_DeregisterRemoteDeviceObserver_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_DeregisterRemoteDeviceObserver_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_DeregisterRemoteDeviceObserver_00100 start");

    std::shared_ptr<TestBluetoothRemoteDeviceObserver> remoteDeviceObserver =
        std::make_shared<TestBluetoothRemoteDeviceObserver>();
    host_->DeregisterRemoteDeviceObserver(remoteDeviceObserver);
    EXPECT_NE(host_, nullptr);
    HILOGI("Host_ModuleTest_DeregisterRemoteDeviceObserver_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetLocalProfileUuids_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetLocalProfileUuids_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetLocalProfileUuids_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    std::vector<std::string> uuids{};
    EXPECT_EQ(host_->GetLocalProfileUuids(uuids), BT_NO_ERROR);

    HILOGI("Host_ModuleTest_GetLocalProfileUuids_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBleMaxAdvertisingDataLength_00100
 * @tc.name: GetBleMaxAdvertisingDataLength
 * @tc.desc: Ble device GetBleMaxAdvertisingDataLength
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBleMaxAdvertisingDataLength_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBleMaxAdvertisingDataLength_00100 start");
    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_NE(host_->GetBleMaxAdvertisingDataLength(), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_GetBleMaxAdvertisingDataLength_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetConnectedBLEDevices_001
 * @tc.name: GetConnectedBLEDevices
 * @tc.desc: Ble device GetConnectedBLEDevices
 */
HWTEST_F(HostTest, Host_ModuleTest_GetConnectedBLEDevices_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetConnectedBLEDevices_001 start");
    host_ = &BluetoothHost::GetDefaultHost();
    std::vector<std::string> connectedDevices;
    int32_t bleProfile = 1;
    EXPECT_EQ(host_->GetConnectedBLEDevices(bleProfile, connectedDevices), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_GetConnectedBLEDevices_001 end");
}

/*
 * @tc.number: Host_ModuleTest_SetFastScan_00100
 * @tc.name: SetFastScan
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_SetFastScan_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_SetFastScan_00100 start");

    host_ = &BluetoothHost::GetDefaultHost();
    EXPECT_EQ(host_->SetFastScan(true), BT_NO_ERROR);

    HILOGI("Host_ModuleTest_SetFastScan_00100 end");
}

/**
 * @tc.number: ConnectAllowedProfiles_00100
 * @tc.name: ConnectAllowedProfiles
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_ConnectAllowedProfiles_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_ConnectAllowedProfiles_00100 start");

    std::string addr("00:00:00:00:00:00");
    EXPECT_EQ(host_->ConnectAllowedProfiles(addr), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_ConnectAllowedProfiles_00100 end");
}

/**
 * @tc.number: DisconnectAllowedProfiles_00100
 * @tc.name: DisconnectAllowedProfiles
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_DisconnectAllowedProfiles_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_DisconnectAllowedProfiles_00100 start");

    std::string addr("00:00:00:00:00:00");
    EXPECT_EQ(host_->DisconnectAllowedProfiles(addr), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_DisconnectAllowedProfiles_00100 end");
}

/**
 * @tc.number: LoadSystemAbilityFail
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_LoadSystemAbilityFail_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_LoadSystemAbilityFail_00100 start");

    host_->LoadSystemAbilityFail();
    EXPECT_NE(host_, nullptr);
    HILOGI("Host_ModuleTest_LoadSystemAbilityFail_00100 end");
}

/**
 * @tc.number: OnPairRequestedInner
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnPairRequestedInner_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnPairRequestedInner_00100 start");
    TestBluetoothHostObserverStub bluetoothHostObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(BluetoothHostObserverStub::OnPairRequestedInner(&bluetoothHostObserverStubTest,
        data, reply), 1);
    sleep(1);
    HILOGI("Host_ModuleTest_OnPairRequestedInner_00100 end");
}

/**
 * @tc.number: OnPairConfirmedInner
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnPairConfirmedInner_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnPairConfirmedInner_00100 start");
    TestBluetoothHostObserverStub bluetoothHostObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(bluetoothHostObserverStubTest.OnPairConfirmedInner(&bluetoothHostObserverStubTest,
        data, reply), 1);
    sleep(1);
    HILOGI("Host_ModuleTest_OnPairConfirmedInner_00100 end");
}

/**
 * @tc.number: OnScanModeChangedInner
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnScanModeChangedInner_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnScanModeChangedInner_00100 start");
    TestBluetoothHostObserverStub bluetoothHostObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(bluetoothHostObserverStubTest.OnRemoteRequest(
        BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_SCAN_MODE_CHANGED,
        data, reply, option), 10);
    HILOGI("Host_ModuleTest_OnScanModeChangedInner_00100 end");
}

/**
 * @tc.number: OnDeviceNameChangedInner
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnDeviceNameChangedInner_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnDeviceNameChangedInner_00100 start");
    TestBluetoothHostObserverStub bluetoothHostObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(bluetoothHostObserverStubTest.OnRemoteRequest(
        BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DEVICE_NAME_CHANGED,
        data, reply, option), 10);
    HILOGI("Host_ModuleTest_OnDeviceNameChangedInner_00100 end");
}

/**
 * @tc.number: OnDeviceAddrChangedInner
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnDeviceAddrChangedInner_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnDeviceAddrChangedInner_00100 start");
    TestBluetoothHostObserverStub bluetoothHostObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(bluetoothHostObserverStubTest.OnRemoteRequest(
        BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DEVICE_ADDR_CHANGED,
        data, reply, option), 10);
    HILOGI("Host_ModuleTest_OnDeviceAddrChangedInner_00100 end");
}

/**
 * @tc.number: GenerateRandomMacAddress_00100
 * @tc.name: GenerateRandomMacAddress
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GenerateRandomMacAddress_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GenerateRandomMacAddress_00100 start");
    EXPECT_FALSE(GenerateRandomMacAddress().empty());
    HILOGI("Host_ModuleTest_GenerateRandomMacAddress_00100 end");
}

/**
 * @tc.number: GetBtTransportName_00100
 * @tc.name: GetBtTransportName
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtTransportName_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtTransportName_00100 start");
    int transport = 3;
    string name;
    name = GetBtTransportName(transport);
    EXPECT_EQ("Unknown", name);
    HILOGI("Host_ModuleTest_GetBtTransportName_00100 end");
}

/**
 * @tc.number: GetErrorCode_00100
 * @tc.name: GetErrorCode
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetErrorCode_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetErrorCode_00100 start");
    int errorCode = 0;
    string code;
    code = GetErrorCode(errorCode);
    EXPECT_EQ("BT_NO_ERROR(0)", code);
    HILOGI("Host_ModuleTest_GetErrorCode_00100 end");
}

/**
 * @tc.number: GetBtStateName_00100
 * @tc.name: GetBtStateName
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBtStateName_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtStateName_00100 start");
    int state = 4;
    string stateName;
    stateName = GetBtStateName(state);
    EXPECT_EQ("Unknown", stateName);
    HILOGI("Host_ModuleTest_GetBtStateName_00100 end");
}

/**
 * @tc.number: VerifyMultiPermissions_00100
 * @tc.name:VerifyMultiPermissions
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_VerifyMultiPermissions_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_VerifyMultiPermissions_00100 start");
    bool systemHapNeeded = false;
    std::set<std::string> permissions;
    EXPECT_EQ(host_->VerifyMultiPermissions(systemHapNeeded, permissions), BT_ERR_PERMISSION_FAILED);
    HILOGI("Host_ModuleTest_VerifyMultiPermissions_00100 end");
}

/**
 * @tc.number: GetProfileConnStateName_00100
 * @tc.name: GetProfileConnStateName
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetProfileConnStateName_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetProfileConnStateName_00100 start");
    string connStateName;
    connStateName = GetProfileConnStateName(static_cast<int>(BTConnectState::CONNECTED));
    EXPECT_EQ("CONNECTED(1)", connStateName);
    HILOGI("Host_ModuleTest_GetProfileConnStateName_00100 end");
}

/**
 * @tc.number: GetProfileConnStateName_00200
 * @tc.name: GetProfileConnStateName
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetProfileConnStateName_00200, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetProfileConnStateName_00200 start");
    string connStateName;
    connStateName = GetProfileConnStateName(static_cast<int>(BTConnectState::DISCONNECTING));
    EXPECT_EQ("DISCONNECTING(2)", connStateName);
    HILOGI("Host_ModuleTest_GetProfileConnStateName_00200 end");
}

/**
 * @tc.number: GetProfileConnStateName_00300
 * @tc.name: GetProfileConnStateName
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetProfileConnStateName_00300, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetProfileConnStateName_00300 start");
    string connStateName;
    connStateName = GetProfileConnStateName(static_cast<int>(BTConnectState::DISCONNECTED));
    EXPECT_EQ("DISCONNECTED(3)", connStateName);
    HILOGI("Host_ModuleTest_GetProfileConnStateName_00300 end");
}

/**
 * @tc.number: GetProfileConnStateName_00400
 * @tc.name: GetProfileConnStateName
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetProfileConnStateName_00400, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetProfileConnStateName_00400 start");
    int state = 4;
    string connStateName;
    connStateName = GetProfileConnStateName(state);
    EXPECT_EQ("Unknown", connStateName);
    HILOGI("Host_ModuleTest_GetProfileConnStateName_00400 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBrAddressByBleAddress_00100
 * @tc.name: GetBrAddressByBleAddress
 * @tc.desc: Test converting a valid BLE address to BR address.
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBrAddressByBleAddress_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBrAddressByBleAddress_00100 start");
    std::string bleAddr = "1A:2B:3C:4D:5E:6F";
    std::string brAddr;
    int ret = host_->GetBrAddressByBleAddress(bleAddr, brAddr);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("Host_ModuleTest_GetBrAddressByBleAddress_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBrAddressByBleAddress_00200
 * @tc.name: GetBrAddressByBleAddress
 * @tc.desc: Test converting an empty BLE address to BR address.
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBrAddressByBleAddress_00200, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBrAddressByBleAddress_00200 start");
    std::string bleAddr = "";
    std::string brAddr;
    int ret = host_->GetBrAddressByBleAddress(bleAddr, brAddr);
    EXPECT_NE(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("Host_ModuleTest_GetBrAddressByBleAddress_00200 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBrAddressByBleAddress_00300
 * @tc.name: GetBrAddressByBleAddress
 * @tc.desc: Test converting an invalid format BLE address to BR address.
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBrAddressByBleAddress_00300, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBrAddressByBleAddress_00300 start");
    std::string bleAddr = "INVALID_ADDR";
    std::string brAddr;
    int ret = host_->GetBrAddressByBleAddress(bleAddr, brAddr);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("Host_ModuleTest_GetBrAddressByBleAddress_00300 end");
}

/**
 * @tc.number: Host_ModuleTest_GetBleMacByBrMac_00100
 * @tc.name: GetBleMacByBrMac
 * @tc.desc: Test converting a valid BR address to BLE address.
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBleMacByBrMac_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBleMacByBrMac_00100 start");
    std::string brMac = "1A:2B:3C:4D:5E:6F";
    std::string bleMac;
    int ret = host_->GetBleMacByBrMac(brMac, bleMac);
    EXPECT_NE(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("Host_ModuleTest_GetBleMacByBrMac_00100 end");
}
 
/**
 * @tc.number: Host_ModuleTest_GetBleMacByBrMac_00200
 * @tc.name: GetBleMacByBrMac
 * @tc.desc: Test converting an empty BR address to BLE address.
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBleMacByBrMac_00200, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBleMacByBrMac_00200 start");
    std::string brMac = "";
    std::string bleMac;
    int ret = host_->GetBleMacByBrMac(brMac, bleMac);
    EXPECT_NE(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("Host_ModuleTest_GetBleMacByBrMac_00200 end");
}
 
/**
 * @tc.number: Host_ModuleTest_GetBleMacByBrMac_00300
 * @tc.name: GetBleMacByBrMac
 * @tc.desc: Test converting an invalid format BR address to BLE address.
 */
HWTEST_F(HostTest, Host_ModuleTest_GetBleMacByBrMac_00300, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBleMacByBrMac_00300 start");
    std::string brMac = "INVALID_ADDR";
    std::string bleMac;
    int ret = host_->GetBleMacByBrMac(brMac, bleMac);
    EXPECT_NE(ret, BT_NO_ERROR);
    HILOGI("Host_ModuleTest_GetBleMacByBrMac_00300 end");
}

/**
 * @tc.number: OnRemoveBluetoothSystemAbility_00100
 * @tc.name: OnRemoveBluetoothSystemAbility
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnRemoveBluetoothSystemAbility_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnRemoveBluetoothSystemAbility_00100 start");
    host_->OnRemoveBluetoothSystemAbility();
    host_->DeregisterObserver(btObserver_);
    host_->OnRemoveBluetoothSystemAbility();
    EXPECT_TRUE(host_ != nullptr);
    HILOGI("Host_ModuleTest_OnRemoveBluetoothSystemAbility_00100 end");
}

/**
 * @tc.number: FreezeByRss_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, BluetoothProxyManager_ModuleTest_FreezeByRss_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnRemoteBatteryChanged_00100 start");
    std::set<int> pidSet;
    BluetoothProxyManager& proxyManager = BluetoothProxyManager::GetInstance();
    EXPECT_TRUE(proxyManager.FreezeByRss(pidSet, true));
    EXPECT_TRUE(proxyManager.FreezeByRss(pidSet, false));
    HILOGI("Host_ModuleTest_OnRemoteBatteryChanged_00100 end");
}

/**
 * @tc.number: ResetAllProxy_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, BluetoothProxyManager_ModuleTest_ResetAllProxy_00100, TestSize.Level1)
{
    HILOGI("BluetoothProxyManager_ModuleTest_ResetAllProxy_00100 start");
    BluetoothProxyManager proxyManager;
    EXPECT_TRUE(proxyManager.ResetAllProxy());
    HILOGI("BluetoothProxyManager_ModuleTest_ResetAllProxy_00100 end");
}

/**
 * @tc.number: OnRemoteDeviceCommonInfoReport_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnRemoteDeviceCommonInfoReport_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnRemoteDeviceCommonInfoReport_00100 start");
    std::vector<uint8_t> value = {1, 2, 3};
    std::string address = "11:22:33:44:55:66";
    BluetoothRemoteDevice device(address);
    EXPECT_TRUE(device.IsValidBluetoothRemoteDevice());
    TestBluetoothRemoteDeviceObserver bluetoothRemoteDeviceObserver;
    bluetoothRemoteDeviceObserver.OnRemoteDeviceCommonInfoReport(device, value);
    DeviceBatteryInfo batteryInfo;
    bluetoothRemoteDeviceObserver.OnRemoteBatteryChanged(device, batteryInfo);
    HILOGI("Host_ModuleTest_OnRemoteDeviceCommonInfoReport_00100 end");
}

/**
 * @tc.number: GetUpdateOutputStackActionName_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, GetUpdateOutputStackActionName_001, TestSize.Level1)
{
    HILOGI("GetUpdateOutputStackActionName_001 start");
    EXPECT_EQ(GetUpdateOutputStackActionName(UpdateOutputStackAction::ACTION_WEAR), "WEAR(0)");
    EXPECT_EQ(GetUpdateOutputStackActionName(UpdateOutputStackAction::ACTION_UNWEAR), "UNWEAR(1)");
    EXPECT_EQ(GetUpdateOutputStackActionName(UpdateOutputStackAction::ACTION_ENABLE_FROM_REMOTE),
        "ENABLE_FROM_REMOTE(2)");
    EXPECT_EQ(GetUpdateOutputStackActionName(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE),
        "DISABLE_FROM_REMOTE(3)");
    EXPECT_EQ(GetUpdateOutputStackActionName(UpdateOutputStackAction::ACTION_ENABLE_WEAR_DETECTION),
        "ENABLE_WEAR_DETECTION(4)");
    EXPECT_EQ(GetUpdateOutputStackActionName(UpdateOutputStackAction::ACTION_DISABLE_WEAR_DETECTION),
        "DISABLE_WEAR_DETECTION(5)");
    EXPECT_EQ(GetUpdateOutputStackActionName(UpdateOutputStackAction::ACTION_USER_OPERATION),
        "USER_OPERATION(6)");
    EXPECT_EQ(GetUpdateOutputStackActionName(UpdateOutputStackAction::ACTION_STOP_VIRTUAL_CALL),
        "STOP_VIRTUAL_CALL(7)");
    EXPECT_EQ(GetUpdateOutputStackActionName(-1), "Unknown");
    HILOGI("GetUpdateOutputStackActionName_001 end");
}

/**
 * @tc.number: Host_ModuleTest_UpdateVirtualDevice_00100
 * @tc.name:UpdateVirtualDevice
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_UpdateVirtualDevice_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_UpdateVirtualDevice_00100 start");
    int32_t action = bluetooth::VIRTUAL_DEVICE_ADD;
    std::string address = "11:22:33:44:55:66";
    host_->UpdateVirtualDevice(action, address);
    action = bluetooth::VIRTUAL_DEVICE_REMOVE;
    host_->UpdateVirtualDevice(action, address);
    EXPECT_TRUE(host_ != nullptr);

    HILOGI("Host_ModuleTest_UpdateVirtualDevice_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_SetFastScanLevel_00100
 * @tc.name:SetFastScanLevel
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_SetFastScanLevel_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_SetFastScanLevel_00100 start");
    int32_t level = 0;
    int32_t result = host_->SetFastScanLevel(level);
    EXPECT_EQ(result, BT_NO_ERROR);
    HILOGI("Host_ModuleTest_SetFastScanLevel_00100 end");
}

/**
 * @tc.name: Host_ModuleTest_UpdateCloudBluetoothDevice_001
 * @tc.desc: UpdateCloudBluetoothDevice:
 * @tc.type:
 */
HWTEST_F(HostTest, Host_ModuleTest_UpdateCloudBluetoothDevice_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_UpdateCloudBluetoothDevice_001 enter");
    std::vector<TrustPairDeviceParam> cloudDevices;
    EXPECT_EQ(host_->UpdateCloudBluetoothDevice(cloudDevices), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_UpdateCloudBluetoothDevice_001 end");
}

/**
 * @tc.name: Host_ModuleTest_EnableBluetoothToRestrictMode_001
 * @tc.desc: EnableBluetoothToRestrictMode:
 * @tc.type:
 */
HWTEST_F(HostTest, Host_ModuleTest_EnableBluetoothToRestrictMode_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_EnableBluetoothToRestrictMode_001 enter");
    EXPECT_EQ(host_->EnableBluetoothToRestrictMode(), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_EnableBluetoothToRestrictMode_001 end");
}

/**
 * @tc.name: Host_ModuleTest_UpdateRefusePolicy_001
 * @tc.desc: UpdateRefusePolicy:
 * @tc.type:
 */
HWTEST_F(HostTest, Host_ModuleTest_UpdateRefusePolicy_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_UpdateRefusePolicy_001 enter");
    int32_t protocolType = 1;
    int32_t pid = 1;
    int64_t prohibitedSecondsTime = 1;
    EXPECT_EQ(host_->UpdateRefusePolicy(protocolType, pid, prohibitedSecondsTime), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_UpdateRefusePolicy_001 end");
}

/**
 * @tc.name: Host_ModuleTest_GetRefusePolicyProhibitedTime_001
 * @tc.desc: GetRefusePolicyProhibitedTime:
 * @tc.type:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetRefusePolicyProhibitedTime_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetRefusePolicyProhibitedTime_001 enter");
    EXPECT_EQ(host_->GetRefusePolicyProhibitedTime(), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_GetRefusePolicyProhibitedTime_001 end");
}

/**
 * @tc.name: Host_ModuleTest_Constructor_001
 * @tc.desc: BluetoothNoDestructor:
 * @tc.type:
 */
HWTEST_F(HostTest, Host_ModuleTest_Constructor_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_Constructor_001 enter");
    // 测试默认构造函数
    BluetoothNoDestructor<int> intObj;
    EXPECT_NE(intObj.get(), nullptr);

    // 测试带参数的构造函数
    BluetoothNoDestructor<std::string> strObj("Hello, World!");
    EXPECT_NE(strObj.get(), nullptr);
    EXPECT_EQ(*strObj, "Hello, World!");
    HILOGI("Host_ModuleTest_Constructor_001 end");
}

/**
 * @tc.name: Host_ModuleTest_OperatorStarAndGet_001
 * @tc.desc: BluetoothNoDestructor:
 * @tc.type:
 */
HWTEST_F(HostTest, Host_ModuleTest_OperatorStarAndGet_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OperatorStarAndGet_001 start");
    // 测试非常量`operator*()`
    BluetoothNoDestructor<int> intObj(5);
    EXPECT_EQ(*intObj, 5);

    // 修改对象的值
    *intObj = 10;
    EXPECT_EQ(*intObj, 10);

    // 测试`get()`方法
    EXPECT_NE(intObj.get(), nullptr);
    EXPECT_EQ(*intObj.get(), 10);

    // 修改对象的值
    *intObj.get() = 5;
    EXPECT_EQ(*intObj, 5);

    // 测试常量`operator*()`
    const BluetoothNoDestructor<int> constIntObj(20);
    EXPECT_EQ(*constIntObj, 20);
    HILOGI("Host_ModuleTest_OperatorStarAndGet_001 end");
}

/**
 * @tc.name: Host_ModuleTest_ProcessRandomDeviceIdCommand_001
 * @tc.desc: ProcessRandomDeviceIdCommand:
 * @tc.type:
 */
HWTEST_F(HostTest, Host_ModuleTest_ProcessRandomDeviceIdCommand_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_ProcessRandomDeviceIdCommand_001 enter");
    int32_t command = 1;
    std::vector<std::string> deviceIdVec {};
    bool isValid = true;
    EXPECT_NE(host_->ProcessRandomDeviceIdCommand(command, deviceIdVec, isValid), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_ProcessRandomDeviceIdCommand_001 end");
}

/**
 * @tc.name: Host_ModuleTest_GetCarKeyDfxData_001
 * @tc.desc: GetCarKeyDfxData:
 * @tc.type:
 */
HWTEST_F(HostTest, Host_ModuleTest_GetCarKeyDfxData_001, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetCarKeyDfxData_001 enter");
    std::string dfxData = "testDfxData";
    int32_t action = 1;
    host_->SetCarKeyCardData(dfxData, action);
    EXPECT_EQ(host_->GetCarKeyDfxData(dfxData), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_GetCarKeyDfxData_001 end");
}

/**
 * @tc.number: Host_ModuleTest_RegisterBtResourceManagerObserver_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_RegisterBtResourceManagerObserver_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_RegisterBtResourceManagerObserver_00100 start");
    std::shared_ptr<TestBluetoothResourceManagerObserver> resourceManagerObserver =
        std::make_shared<TestBluetoothResourceManagerObserver>();
    host_->RegisterBtResourceManagerObserver(resourceManagerObserver);
    host_->DeregisterBtResourceManagerObserver(resourceManagerObserver);
    EXPECT_NE(host_, nullptr);
    HILOGI("Host_ModuleTest_RegisterBtResourceManagerObserver_00100 end");
}

/**
 * @tc.number: OnBluetoothResourceDecision_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnBluetoothResourceDecision_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnBluetoothResourceDecision_00100 start");
    uint8_t eventId = bluetooth::SPP_CLIENT_CONNECT_DONE;
    SensingInfo sensingInfo;
    TestBluetoothResourceManagerObserver resourceManagerObserver;
    resourceManagerObserver.OnSensingStateChanged(eventId, sensingInfo);
    eventId = bluetooth::BLE_FASTEST_CONN_DECISION;
    uint32_t result = bluetooth::CONNECTION_ACCEPT;
    resourceManagerObserver.OnBluetoothResourceDecision(eventId, sensingInfo, result);
    EXPECT_NE(host_, nullptr);
    HILOGI("Host_ModuleTest_OnBluetoothResourceDecision_00100 end");
}

/**
 * @tc.name: SetBtChannelScan_00100
 * @tc.desc: Set bt channel scan
 * @tc.type: FUNC
 */
HWTEST_F(HostTest, SetBtChannelScan_00100, TestSize.Level1)
{
    HILOGI("SetBtChannelScan_00100 enter");
    int invalidInterval = CHANNEL_SCAN_INTERVAL_100MS - 1;
    EXPECT_EQ(host_->SetBtChannelScan(true, invalidInterval), BT_ERR_INVALID_PARAM);
    EXPECT_EQ(host_->SetBtChannelScan(true, CHANNEL_SCAN_INTERVAL_100MS), BT_NO_ERROR);
    EXPECT_EQ(host_->SetBtChannelScan(true, CHANNEL_SCAN_INTERVAL_1000MS), BT_NO_ERROR);
    EXPECT_EQ(host_->SetBtChannelScan(false, CHANNEL_SCAN_INTERVAL_BYPASS), BT_NO_ERROR);
    HILOGI("SetBtChannelScan_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_BluetoothFactoryReset_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_BluetoothFactoryReset_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_BluetoothFactoryReset_00100 start");
    EXPECT_EQ(host_->BluetoothFactoryReset(), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_BluetoothFactoryReset_00100 end");
}

/**
 * @tc.number: Host_ModuleTest_RestrictBluetooth_00100
 * @tc.name:RestrictBluetooth
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_RestrictBluetooth_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_RestrictBluetooth_00100 start");
    EXPECT_EQ(host_->RestrictBluetooth(), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_RestrictBluetooth_00100 end");
}

/**
 * @tc.number: Close_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_Close_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_Close_00100 start");
    host_->Close();
    EXPECT_NE(host_, nullptr);
    HILOGI("Host_ModuleTest_Close_00100 end");
}

/**
 * @tc.number: NotifyDialogResult_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_NotifyDialogResult_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_NotifyDialogResult_00100 start");
    uint32_t dialogType = 0;
    bool dialogResult = true;
    EXPECT_EQ(host_->NotifyDialogResult(dialogType, dialogResult), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_NotifyDialogResult_00100 end");
}

/**
 * @tc.number: SetCallingPackageName_00100
 * @tc.name:SetCallingPackageName
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_SetCallingPackageName_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_SetCallingPackageName_00100 start");
    std::string address = "22:33:44:55:66:99";
    std::string packageName = "bluetooth_test";
    EXPECT_EQ(host_->SetCallingPackageName(address, packageName), BT_NO_ERROR);
    HILOGI("Host_ModuleTest_SetCallingPackageName_00100 end");
}

/**
 * @tc.name: SetConnectionPriority_00100
 * @tc.desc: set connection priority
 * @tc.type: FUNC
 */
HWTEST_F(HostTest, SetConnectionPriority_00100, TestSize.Level1)
{
    HILOGI("SetConnectionPriority_00100 enter");
    std::string addr = "00:11:22:33:44:55";
    int ret = host_->SetConnectionPriority(addr, Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NON_PREEMPTIBLE);
    EXPECT_EQ(ret, BT_NO_ERROR);
    ret = host_->SetConnectionPriority(addr, Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NO_REFUSE_FREQUENT_CONNECT);
    EXPECT_EQ(ret, BT_NO_ERROR);
    ret = host_->SetConnectionPriority(addr, Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_DEFAULT);
    EXPECT_EQ(ret, BT_NO_ERROR);
    int invalidPriority = 0XFF;
    ret = host_->SetConnectionPriority(addr, invalidPriority);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("SetConnectionPriority_00100 end");
}

/**
 * @tc.name: OnStateChanged_00100
 * @tc.desc: OnStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(HostTest, OnStateChanged_00100, TestSize.Level1)
{
    HILOGI("OnStateChanged_00100 enter");
    std::string addr = "00:11:22:33:44:55";
    int transport = 1;
    int status = 1;
    EXPECT_NE(btObserver_, nullptr);
    btObserver_->OnStateChanged(transport, status);
    int state = 0;
    btObserver_->OnBluetoothStateChanged(state);
    btObserver_->OnDiscoveryStateChanged(status);
    BluetoothRemoteDevice device(addr, BT_TRANSPORT_BREDR);
    int rssi = -60;
    std::string deviceName = "testDeviceName";
    int deviceClass = 1;
    btObserver_->OnDiscoveryResult(device, rssi, deviceName, deviceClass, 0);
    btObserver_->OnPairRequested(device);
    int reqType = 1;
    int number = 2;
    btObserver_->OnPairConfirmed(device, reqType, number);
    btObserver_->OnDeviceNameChanged(deviceName);
    int mode = 0;
    btObserver_->OnScanModeChanged(mode);
    btObserver_->OnDeviceAddrChanged(addr);
    HILOGI("OnStateChanged_00100 end");
}

/**
 * @tc.number: OnPairStatusChangedWithMessage_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostTest, Host_ModuleTest_OnPairStatusChangedWithMessage_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_OnPairStatusChangedWithMessage_00100 start");
    std::string address = "11:22:33:44:55:66";
    BluetoothRemoteDevice device(address);
    EXPECT_TRUE(device.IsValidBluetoothRemoteDevice());
    TestBluetoothRemoteDeviceObserver bluetoothRemoteDeviceObserver;
    bluetoothRemoteDeviceObserver.OnPairStatusChangedWithMessage(device, 0, 0, BOND_MSG_NO_ERROR);
    HILOGI("Host_ModuleTest_OnPairStatusChangedWithMessage_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS