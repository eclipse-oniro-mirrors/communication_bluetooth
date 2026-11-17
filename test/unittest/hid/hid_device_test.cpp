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
 
#include <gtest/gtest.h>
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hid_device.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_log.h"
 
using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace Bluetooth {
using namespace std;
 
class HidDeviceObserverTest : public HidDeviceObserver {
public:
    HidDeviceObserverTest() = default;
    virtual ~HidDeviceObserverTest() = default;
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state)
    {
        changeEvent_ = "OnConnectionStateChanged";
    }
    virtual void OnAppStatusChanged(int state)
    {
        changeEvent_ = "OnAppStatusChanged";
    }
    virtual void OnGetReport(int type, int id, uint16_t bufferSize)
    {
        changeEvent_ = "OnGetReport";
    }
    virtual void OnInterruptDataReceived(int reportId, const std::vector<uint8_t> &data)
    {
        changeEvent_ = "OnInterruptDataReceived";
    }
    virtual void OnSetProtocol(int protocol)
    {
        changeEvent_ = "OnSetProtocol";
    }
    virtual void OnSetReport(int type, int id, const std::vector<uint8_t> &data)
    {
        changeEvent_ = "OnSetReport";
    }
    virtual void OnVirtualCableUnplug()
    {
        changeEvent_ = "OnVirtualCableUnplug";
    }
private:
    std::string changeEvent_;
};
 
constexpr int TIME = 4;
static Bluetooth::HidDevice  *profile_;
static std::shared_ptr<HidDeviceObserverTest> hidDeviceObserverTest = std::make_shared<HidDeviceObserverTest>();
static BluetoothHost *host_;
 
class HidDeviceTest : public testing::Test {
public:
    HidDeviceTest();
    ~HidDeviceTest();
 
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
                       std::vector<std::string> bluetoothRemoteDeviceAddr);
    void DisableBt();
 
private:
};
 
HidDeviceTest::HidDeviceTest()
{
}
 
HidDeviceTest::~HidDeviceTest()
{
}
 
void HidDeviceTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
    host_ = &BluetoothHost::GetDefaultHost();
 
    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);
 
    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
 
    profile_ = HidDevice::GetProfile();
 
    HILOGI("SetUpTestCase end");
}
void HidDeviceTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void HidDeviceTest::SetUp()
{
}
void HidDeviceTest::TearDown()
{
}
 
void HidDeviceTest::DisableBt()
{
    int btState = host_->GetBtState();
    HILOGI("DisableBt start, btState = %{public}d", btState);
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        sleep(TIME);
    }
}
 
bool HidDeviceTest::CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
                                    std::vector<std::string> bluetoothRemoteDeviceAddr)
{
    if (bluetoothRemoteDeviceByState.size() != bluetoothRemoteDeviceAddr.size()) {
        return false;
    }
    for (int i = 0; i < bluetoothRemoteDeviceByState.size(); i++) {
        if (bluetoothRemoteDeviceByState[i].GetDeviceAddr().compare(bluetoothRemoteDeviceAddr[i]) != 0) {
            return false;
        }
    }
    return true;
}

/*
 * @tc.number: HID_Device_ModuleTest_RegisterHidDevice_00100
 * @tc.name: RegisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_RegisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_RegisterHidDevice_00100 start");
    BluetoothHidDeviceSdp sdpSetting;
    sdpSetting.name_ = "test";
    sdpSetting.description_ = "test";
    sdpSetting.provider_ = "test";
    sdpSetting.subclass_ = static_cast<int>(Subclass::SUBCLASS_MOUSE);
    sdpSetting.sdpSettings_ = {};
    BluetoothHidDeviceQos inQos;
    inQos.serviceType_ = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    inQos.tokenRate_ = DEFAULT_TOKEN_RATE;
    inQos.tokenBucketSize_ = DEFAULT_TOKEN_BUCKET_SIZE;
    inQos.peakBandwidth_ = DEFAULT_PEAK_BANDWIDTH;
    inQos.latency_ = DEFAULT_LATENCY;
    inQos.delayVariation_ = DEFAULT_DELAY_VARIATION;
    BluetoothHidDeviceQos outQos;
    outQos.serviceType_ = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    outQos.tokenRate_ = DEFAULT_TOKEN_RATE;
    outQos.tokenBucketSize_ = DEFAULT_TOKEN_BUCKET_SIZE;
    outQos.peakBandwidth_ = DEFAULT_PEAK_BANDWIDTH;
    outQos.latency_ = DEFAULT_LATENCY;
    outQos.delayVariation_ = DEFAULT_DELAY_VARIATION;
    profile_->RegisterHidDevice(sdpSetting, inQos, outQos);
    EXPECT_TRUE(profile_ != nullptr);
    HILOGI("HID_Device_ModuleTest_RegisterHidDevice_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_UnregisterHidDevice_00100
 * @tc.name: UnregisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_UnregisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_UnregisterHidDevice_00100 start");
    EXPECT_EQ(profile_->UnregisterHidDevice(), BT_NO_ERROR);
    HILOGI("HID_Device_ModuleTest_UnregisterHidDevice_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_GetConnectedDevices_00100
 * @tc.name: GetConnectedDevices
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_GetConnectedDevices_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetDevicesByStates_00100 start");
    std::vector<std::string> bluetoothRemoteDeviceAddr;
    std::vector<BluetoothRemoteDevice> devices;
    profile_->GetConnectedDevices(devices);
    EXPECT_TRUE(CompareDevice(devices, bluetoothRemoteDeviceAddr));
    HILOGI("HID_Device_ModuleTest_GetDevicesByStates_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_GetConnectionState_00100
 * @tc.name: GetConnectionState
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_GetConnectionState_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetConnectionState_00100 start");
 
    int success = static_cast<int>(BTConnectState::DISCONNECTED);
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    profile_->GetConnectionState(device, state);
    EXPECT_EQ(success, state);
 
    HILOGI("HID_Device_ModuleTest_GetDeviceState_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_GetConnectionState_00200
 * @tc.name: GetConnectionState
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_GetConnectionState_00200, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetConnectionState_00200 start");
    BluetoothRemoteDevice device("invalid addr", BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    EXPECT_EQ(profile_->GetConnectionState(device, state), BT_ERR_INVALID_PARAM);
    HILOGI("HID_Device_ModuleTest_GetConnectionState_00200 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_Connect_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_Connect_00100 start");
    AddressInfo device;
    device.SetAddress("11:22:33:44:55");
    EXPECT_EQ(profile_->Connect(device), BT_ERR_INVALID_PARAM);
    HILOGI("HID_Device_ModuleTest_Connect_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_Connect_00200
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_Connect_00200, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_Connect_00200 start");
    AddressInfo device;
    device.SetAddress("11:22:aa:44:55:66");
    device.SetAddressType(AddressType::VIRTUAL_ADDRESS);
    EXPECT_EQ(profile_->Connect(device), BT_ERR_INTERNAL_ERROR);
    HILOGI("HID_Device_ModuleTest_Connect_00200 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_Disconnect_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_Disconnect_00100 start");
    EXPECT_EQ(profile_->Disconnect(), BT_ERR_HID_APP_NOT_REGISTER);
    HILOGI("HID_Device_ModuleTest_Disconnect_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_RegisterObserver_00100
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_RegisterObserver_00100 start");
    profile_->RegisterObserver(hidDeviceObserverTest);
    EXPECT_NE(nullptr, profile_);
    profile_->DeregisterObserver(hidDeviceObserverTest);
    HILOGI("HID_Device_ModuleTest_RegisterObserver_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_OnConnectionStateChanged_00100
 * @tc.name: OnConnectionStateChanged
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_OnConnectionStateChanged_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_OnConnectionStateChanged_00100 start");
    BluetoothRemoteDevice device;
    int ret = 0;
    hidDeviceObserverTest->OnConnectionStateChanged(device, ret);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnConnectionStateChanged");
    HILOGI("HID_Device_ModuleTest_OnConnectionStateChanged_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_OnAppStatusChanged_00100
 * @tc.name: OnAppStatusChanged
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_OnAppStatusChanged_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_OnAppStatusChanged_00100 start");
    std::string device;
    int state = 0; // register failed
    hidDeviceObserverTest->OnAppStatusChanged(state);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnAppStatusChanged");
    HILOGI("HID_Device_ModuleTest_OnAppStatusChanged_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_OnGetReport_00100
 * @tc.name: OnGetReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_OnGetReport_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_OnGetReport_00100 start");
    int bufferSize = 100;
    hidDeviceObserverTest->OnGetReport(0, 0, bufferSize);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnGetReport");
    HILOGI("HID_Device_ModuleTest_OnGetReport_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_OnSetProtocol_00100
 * @tc.name: OnSetProtocol
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_OnSetProtocol_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_OnSetProtocol_00100 start");
    hidDeviceObserverTest->OnSetProtocol(0);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnSetProtocol");
    HILOGI("HID_Device_ModuleTest_OnSetProtocol_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_OnSetReport_00100
 * @tc.name: OnSetReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_OnSetReport_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_OnSetReport_00100 start");
    hidDeviceObserverTest->OnSetReport(0, 0, {});
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnSetReport");
    HILOGI("HID_Device_ModuleTest_OnSetReport_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_OnInterruptDataReceived_00100
 * @tc.name: OnInterruptDataReceived
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_OnInterruptDataReceived_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_OnInterruptDataReceived_00100 start");
    int id = 0;
    std::vector<uint8_t> data = {};
    hidDeviceObserverTest->OnInterruptDataReceived(id, data);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnInterruptDataReceived");
    HILOGI("HID_Device_ModuleTest_OnInterruptDataReceived_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_OnVirtualCableUnplug_00100
 * @tc.name: OnVirtualCableUnplug
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_OnVirtualCableUnplug_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_OnVirtualCableUnplug_00100 start");
    hidDeviceObserverTest->OnVirtualCableUnplug();
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnVirtualCableUnplug");
    HILOGI("HID_Device_ModuleTest_OnVirtualCableUnplug_00100 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_SendReport_00100
 * @tc.name: SendReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_SendReport_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_SendReport_00100 start");
    int id = 0;
    std::vector<uint8_t> data = {};
    EXPECT_EQ(profile_->SendReport(id, data), BT_ERR_HID_APP_NOT_REGISTER);
    HILOGI("HID_Device_ModuleTest_SendReport_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_ReplyReport_00100
 * @tc.name: ReplyReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_ReplyReport_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_ReplyReport_00100 start");
    ReportType type = REPORT_TYPE_INPUT;
    int id = 0;
    std::vector<uint8_t> data = {};
    EXPECT_EQ(profile_->ReplyReport(type, id, data), BT_ERR_HID_APP_NOT_REGISTER);
    HILOGI("HID_Device_ModuleTest_ReplyReport_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_ReportError_00100
 * @tc.name: ReportError
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_ReportError_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_ReportError_00100 start");
    ErrorReason type = RSP_SUCCESS;
    EXPECT_EQ(profile_->ReportError(type), BT_ERR_HID_APP_NOT_REGISTER);
    HILOGI("HID_Device_ModuleTest_ReportError_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_CheckProfileState_00100
 * @tc.name: CheckProfileState
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_CheckProfileState_00100, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_CheckProfileState_00100 start");
    EXPECT_EQ(profile_->CheckProfileState(), BT_NO_ERROR);
    HILOGI("HID_Device_ModuleTest_CheckProfileState_00100 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_GetConnectStrategy_001
 * @tc.name: GetConnectStrategy
 * @tc.desc: Get connect strategy by address
*/
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_GetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetConnectStrategy_001 start");
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();
    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    HILOGI("HID_Device_ModuleTest_GetConnectStrategy_001 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_GetConnectStrategy_002
 * @tc.name: GetConnectStrategy
 * @tc.desc: IS_BT_ENABLED():false
*/
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_GetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_GetConnectStrategy_002 start");
    BluetoothRemoteDevice device("00:00:00:00:00:01", BT_TRANSPORT_BREDR);
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN));
    HILOGI("HID_Device_ModuleTest_GetConnectStrategy_002 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_SetConnectStrategy_001
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_SetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_SetConnectStrategy_001 start");
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    EXPECT_EQ(profile_->SetConnectStrategy(device,
        static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN)), BT_ERR_INVALID_PARAM);
    HILOGI("HID_Device_ModuleTest_SetConnectStrategy_001 end");
}
 
/*
 * @tc.number: HID_Device_ModuleTest_SetConnectStrategy_002
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_SetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_SetConnectStrategy_002 start");
    sleep(TIME);
    BluetoothRemoteDevice device("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    device.StartPair();
    profile_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    int strategy = 0;
    EXPECT_EQ(profile_->GetConnectStrategy(device, strategy), BT_NO_ERROR);
    EXPECT_EQ(strategy, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    HILOGI("HID_Device_ModuleTest_SetConnectStrategy_002 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_Connect_00300
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_Connect_00300, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_Connect_00300 start");
    AddressInfo device;
    device.SetAddress("11:22:33:44:55:66");
    DisableBt();
    EXPECT_EQ(profile_->Connect(device), BT_ERR_INVALID_STATE);
    HILOGI("HID_Device_ModuleTest_Connect_00300 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_Disconnect_00200
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_Disconnect_00200, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_Disconnect_00200 start");
    DisableBt();
    EXPECT_EQ(profile_->Disconnect(), BT_ERR_INVALID_STATE);
    HILOGI("HID_Device_ModuleTest_Disconnect_00200 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_SendReport_00200
 * @tc.name: SendReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_SendReport_00200, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_SendReport_00200 start");
    int id = 0;
    std::vector<uint8_t> data = {};
    DisableBt();
    EXPECT_EQ(profile_->SendReport(id, data), BT_ERR_INVALID_STATE);
    HILOGI("HID_Device_ModuleTest_SendReport_00200 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_ReplyReport_00200
 * @tc.name: ReplyReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_ReplyReport_00200, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_ReplyReport_00200 start");
    ReportType type = REPORT_TYPE_INPUT;
    int id = 0;
    std::vector<uint8_t> data = {};
    DisableBt();
    EXPECT_EQ(profile_->ReplyReport(type, id, data), BT_ERR_INVALID_STATE);
    HILOGI("HID_Device_ModuleTest_ReplyReport_00200 end");
}

/*
 * @tc.number: HID_Device_ModuleTest_ReportError_00200
 * @tc.name: ReportError
 * @tc.desc:
 */
HWTEST_F(HidDeviceTest, HID_Device_ModuleTest_ReportError_00200, TestSize.Level1)
{
    HILOGI("HID_Device_ModuleTest_ReportError_00200 start");
    ErrorReason type = RSP_SUCCESS;
    DisableBt();
    EXPECT_EQ(profile_->ReportError(type), BT_ERR_INVALID_STATE);
    HILOGI("HID_Device_ModuleTest_ReportError_00200 end");
}
}  // namespace Bluetooth
}  // namespace OHOS