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
#include <cstring>
#include "common/raw_address.h"
#include "profile_service_manager.h"
#include "adapter_manager.h"
#include "hid_device_service.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"
#include "profile_config.h"
#include "thread_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

class HidDeviceObserverTest : public HidDeviceObserver {
public:
    HidDeviceObserverTest() = default;
    virtual ~HidDeviceObserverTest() = default;
    virtual void OnConnectionStateChanged(bluetooth::RawAddress device, int state)
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
    virtual void OnInterruptDataReceived(int reportId, std::vector<uint8_t> data)
    {
        changeEvent_ = "OnInterruptDataReceived";
    }
    virtual void OnSetProtocol(int protocol)
    {
        changeEvent_ = "OnSetProtocol";
    }
    virtual void OnSetReport(int type, int id, std::vector<uint8_t> data)
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

static BluetoothHost *host_;
static std::shared_ptr<HidDeviceObserverTest> hidDeviceObserverTest = std::make_shared<HidDeviceObserverTest>();
const int TIME = 2;

static int GetRemoteServicesMock(STACK::RawAddress* remote_addr)
{
    HILOGI("GetRemoteServicesMock start");
    return 0;
}

class HidDeviceServiceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

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
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase start");
        auto hidDeviceService = HidDeviceService::GetService();
        hidDeviceService->SetHidDeviceAppInfoEmpty();
        hidDeviceService->SetIsConnected(false);
        host_ = nullptr;
        HILOGI("TearDownTestCase end");
    }
    void SetUp()
    {
        // new HidDeviceService
        auto manager = ProfileServiceManager::GetInstance();
        string name = "HidDeviceService";
        manager->SetProfileService(ADAPTER_BREDR, name);
        // Disable hid thread
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_HID_DEVICE, ThreadUtil::DISABLED);
    }
    void TearDown()
    {
        // delete HidDeviceService
        auto hidDeviceService = HidDeviceService::GetService();
        delete hidDeviceService;
    }

    void RegisterAppInfo()
    {
        auto hidDeviceService = HidDeviceService::GetService();
        hidDeviceService->hidDeviceAppInfo_.tokenId = 1111;
        hidDeviceService->hidDeviceAppInfo_.appId = "111";
        hidDeviceService->hidDeviceAppInfo_.name = "111";
    }

};

/*
 * @tc.number: Hid_Device_Service_SetIsForeground_00100
 * @tc.name: SetIsForeground
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_SetIsForeground_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_SetIsForeground_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    hidDeviceService->SetIsForeground(true);
    EXPECT_EQ(hidDeviceService->GetIsForeground(), true);
    hidDeviceService->SetIsForeground(false);
    EXPECT_EQ(hidDeviceService->GetIsForeground(), false);
    HILOGI("Hid_Device_Service_SetIsForeground_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_SetIsConnected_00100
 * @tc.name: SetIsConnected
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_SetIsConnected_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_SetIsConnected_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    hidDeviceService->SetIsConnected(true);
    EXPECT_EQ(hidDeviceService->IsConnected(), true);
    hidDeviceService->SetIsConnected(false);
    EXPECT_EQ(hidDeviceService->IsConnected(), false);
    HILOGI("Hid_Device_Service_SetIsConnected_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_SetAddress_00100
 * @tc.name: SetAddress
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_SetAddress_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_SetAddress_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    std::string device = "11:22:33:44:55:66";
    hidDeviceService->SetAddress(device);
    EXPECT_EQ(hidDeviceService->device_.GetAddress() == device, true);
    HILOGI("Hid_Device_Service_SetAddress_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_SetHidDeviceAppInfoEmpty_00100
 * @tc.name: SetHidDeviceAppInfoEmpty
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_SetHidDeviceAppInfoEmpty_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_SetHidDeviceAppInfoEmpty_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    hidDeviceService->SetHidDeviceAppInfoEmpty();
    EXPECT_TRUE(hidDeviceService->hidDeviceAppInfo_.tokenId == 0);
    EXPECT_TRUE(hidDeviceService->hidDeviceAppInfo_.appId.empty());
    EXPECT_TRUE(hidDeviceService->hidDeviceAppInfo_.name.empty());
    EXPECT_FALSE(hidDeviceService->IsAppRegistered());
    HILOGI("Hid_Device_Service_SetHidDeviceAppInfoEmpty_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_GetConnectState_00100
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_GetConnectState_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    int state = hidDeviceService->GetConnectState();
    EXPECT_EQ(state, PROFILE_STATE_DISCONNECTED);
    HILOGI("Hid_Device_Service_GetConnectState_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_GetMaxConnectNum_00100
 * @tc.name: GetMaxConnectNum
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_GetMaxConnectNum_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_GetMaxConnectNum_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    int maxConnectNum = hidDeviceService->GetMaxConnectNum();
    EXPECT_EQ(maxConnectNum, 1);
    HILOGI("Hid_Device_Service_GetMaxConnectNum_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_IsRemoteHidHostSupported_00100
 * @tc.name: IsRemoteHidHostSupported
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_IsRemoteHidHostSupported_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_IsRemoteHidHostSupported_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    RawAddress rawAddr("11:22:33:44:55:66");
    bool isSupport = hidDeviceService->IsRemoteHidHostSupported(rawAddr);
    EXPECT_FALSE(isSupport);
    HILOGI("Hid_Device_Service_IsRemoteHidHostSupported_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_GetConnectDevices_00100
 * @tc.name: GetConnectDevices
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_GetConnectDevices_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_GetConnectDevices_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    EXPECT_EQ(hidDeviceService->GetConnectDevices().size(), 0);
    HILOGI("Hid_Device_Service_GetConnectDevices_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_GetConnectionState_00100
 * @tc.name: GetConnectionState
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_GetConnectionState_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_GetConnectionState_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    RawAddress device("11:22:33:44:55:66");
    EXPECT_EQ(hidDeviceService->GetConnectionState(device), static_cast<int>(BTConnectState::DISCONNECTED));
    hidDeviceService->SetIsConnected(true);
    hidDeviceService->SetAddress(device.GetAddress());
    EXPECT_EQ(hidDeviceService->GetConnectionState(device), static_cast<int>(BTConnectState::CONNECTED));
    HILOGI("Hid_Device_Service_GetConnectionState_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_SetConnectStrategy_00100
 * @tc.name: SetConnectStrategy
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_SetConnectStrategy_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_SetConnectStrategy_00100 start");
    RawAddress device("00:11:22:33:44:55:66");
    auto hidDeviceService = HidDeviceService::GetService();
    EXPECT_EQ(hidDeviceService->SetConnectStrategy(device, 1), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hid_Device_Service_SetConnectStrategy_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_GetConnectStrategy_00100
 * @tc.name: GetConnectStrategy
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_GetConnectStrategy_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_GetConnectStrategy_00100 start");
    RawAddress device("00:11:22:33:44:55:66");
    auto hidDeviceService = HidDeviceService::GetService();
    EXPECT_EQ(hidDeviceService->GetConnectStrategy(device), static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN));
    HILOGI("Hid_Device_Service_GetConnectStrategy_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_Connect_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_Connect_00100 start");
    RawAddress device("00:11:22:33:44:55:66");
    auto hidDeviceService = HidDeviceService::GetService();
    EXPECT_EQ(hidDeviceService->Connect(device), BT_ERR_HID_APP_NOT_REGISTER);
    RegisterAppInfo();
    EXPECT_TRUE(hidDeviceService->IsAppRegistered());
    EXPECT_EQ(hidDeviceService->Connect(device), BT_ERR_INTERNAL_ERROR);
    hidDeviceService->SetIsConnected(true);
    EXPECT_EQ(hidDeviceService->Connect(device), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hid_Device_Service_Connect_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_Disconnect_00100 start");
    RawAddress device("00:11:22:33:44:55:66");
    auto hidDeviceService = HidDeviceService::GetService();
    EXPECT_EQ(hidDeviceService->Disconnect(device), BT_NO_ERROR);
    HILOGI("Hid_Device_Service_Disconnect_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_DisconnectHidDevice_00100
 * @tc.name: DisconnectHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_DisconnectHidDevice_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_DisconnectHidDevice_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    EXPECT_EQ(hidDeviceService->DisconnectHidDevice(), BT_ERR_HID_APP_NOT_REGISTER);
    RegisterAppInfo();
    EXPECT_EQ(hidDeviceService->DisconnectHidDevice(), BT_NO_ERROR);
    hidDeviceService->SetIsConnected(true);
    EXPECT_EQ(hidDeviceService->DisconnectHidDevice(), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hid_Device_Service_DisconnectHidDevice_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_SendReport_00100
 * @tc.name: SendReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_SendReport_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_SendReport_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    int id = 0;
    std::vector<uint8_t> data = {};
    EXPECT_EQ(hidDeviceService->SendReport(id, data), BT_ERR_HID_APP_NOT_REGISTER);
    RegisterAppInfo();
    EXPECT_EQ(hidDeviceService->SendReport(id, data), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hid_Device_Service_SendReport_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_ReplyReport_00100
 * @tc.name: ReplyReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_ReplyReport_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_ReplyReport_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    ReportType type = REPORT_TYPE_INPUT;
    int id = 0;
    std::vector<uint8_t> data = {};

    EXPECT_EQ(hidDeviceService->ReplyReport(type, id, data), BT_ERR_HID_APP_NOT_REGISTER);
    RegisterAppInfo();
    EXPECT_EQ(hidDeviceService->ReplyReport(type, id, data), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hid_Device_Service_ReplyReport_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_ReportError_00100
 * @tc.name: ReportError
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_ReportError_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_ReportError_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    ErrorReason type = RSP_SUCCESS;

    EXPECT_EQ(hidDeviceService->ReportError(type), BT_ERR_HID_APP_NOT_REGISTER);
    RegisterAppInfo();
    EXPECT_EQ(hidDeviceService->ReportError(type), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hid_Device_Service_ReportError_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_RegisterHidDevice_00100
 * @tc.name: RegisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_RegisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_RegisterHidDevice_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
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

    RegisterAppInfo();
    EXPECT_EQ(hidDeviceService->RegisterHidDevice(sdpSetting, inQos, outQos), BT_ERR_HID_APP_HAS_BEEN_REGISTERED);
    HILOGI("Hid_Device_Service_RegisterHidDevice_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_UnregisterHidDevice_00100
 * @tc.name: UnregisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_UnregisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_UnregisterHidDevice_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    EXPECT_EQ(hidDeviceService->UnregisterHidDevice(), BT_NO_ERROR);
    RegisterAppInfo();
    EXPECT_EQ(hidDeviceService->UnregisterHidDevice(), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hid_Device_Service_UnregisterHidDevice_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_AutoUnregisterHidDevice_00100
 * @tc.name: AutoUnregisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_AutoUnregisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_AutoUnregisterHidDevice_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    EXPECT_EQ(hidDeviceService->AutoUnregisterHidDevice(), BT_NO_ERROR);
    HILOGI("Hid_Device_Service_AutoUnregisterHidDevice_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_ProcessEvent_00100
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_ProcessEvent_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_ProcessEvent_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    RawAddress device("11:22:33:44:55:66");

    HidDeviceMessage EnableEvent(HID_DEVICE_SERVICE_STARTUP_EVT);
    hidDeviceService->ProcessEvent(EnableEvent);

    HidDeviceMessage DisableEvent(HID_DEVICE_SERVICE_SHUTDOWN_EVT);
    hidDeviceService->ProcessEvent(DisableEvent);

    HidDeviceMessage ConnectEvent(HID_DEVICE_CONNECT_EVT);
    ConnectEvent.dev_ = device;
    hidDeviceService->ProcessEvent(ConnectEvent);

    HidDeviceMessage DisconnectEvent(HID_DEVICE_DISCONNECT_EVT);
    DisconnectEvent.dev_ = device;
    hidDeviceService->ProcessEvent(DisconnectEvent);

    HidDeviceMessage UnregisterEvent(HID_DEVICE_UNREGISTER_HID_DEVICE_EVT);
    hidDeviceService->ProcessEvent(UnregisterEvent);

    HidDeviceMessage SendReportEvent(HID_DEVICE_SEND_REPORT_EVT);
    SendReportEvent.data_ = {};
    SendReportEvent.id_ = 0;
    hidDeviceService->ProcessEvent(SendReportEvent);

    HidDeviceMessage ReplyReportEvent(HID_DEVICE_REPLY_REPORT_EVT);
    ReplyReportEvent.data_ = {};
    ReplyReportEvent.id_ = 0;
    ReplyReportEvent.type_ = REPORT_TYPE_INPUT;
    hidDeviceService->ProcessEvent(ReplyReportEvent);

    HidDeviceMessage ReportErrorEvent(HID_DEVICE_REPORT_ERROR_EVT);
    ReportErrorEvent.type_ = RSP_SUCCESS;
    hidDeviceService->ProcessEvent(ReportErrorEvent);
    EXPECT_TRUE(hidDeviceService != nullptr);

    HILOGI("Hid_Device_Service_ProcessEvent_00100 end");
}

/*
 * @tc.number: Hid_Device_Service__OnConnectionStateChanged_00100
 * @tc.name: OnConnectionStateChanged
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service__OnConnectionStateChanged_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service__OnConnectionStateChanged_00100 start");
    RawAddress device("11:2:33:44:55:66");
    int ret = 0;
    hidDeviceObserverTest->OnConnectionStateChanged(device, ret);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnConnectionStateChanged");
    HILOGI("Hid_Device_Service__OnConnectionStateChanged_00100 end");
}
 
/*
 * @tc.number: Hid_Device_Service_OnAppStatusChanged_00100
 * @tc.name: OnAppStatusChanged
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_OnAppStatusChanged_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_OnAppStatusChanged_00100 start");
    int state = 0; // register failed
    hidDeviceObserverTest->OnAppStatusChanged(state);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnAppStatusChanged");
    HILOGI("Hid_Device_Service_OnAppStatusChanged_00100 end");
}
 
/*
 * @tc.number: Hid_Device_Service_OnGetReport_00100
 * @tc.name: OnGetReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_OnGetReport_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_OnGetReport_00100 start");
    int bufferSize = 100;
    hidDeviceObserverTest->OnGetReport(0, 0, bufferSize);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnGetReport");
    HILOGI("Hid_Device_Service_OnGetReport_00100 end");
}
 
/*
 * @tc.number: Hid_Device_Service_OnSetProtocol_00100
 * @tc.name: OnSetProtocol
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_OnSetProtocol_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_OnSetProtocol_00100 start");
    hidDeviceObserverTest->OnSetProtocol(0);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnSetProtocol");
    HILOGI("Hid_Device_Service_OnSetProtocol_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_OnSetReport_00100
 * @tc.name: OnSetReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_OnSetReport_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_OnSetReport_00100 start");
    int type = 0;
    int id = 0;
    std::vector<uint8_t> data = {};
    hidDeviceObserverTest->OnSetReport(type, id, data);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnSetReport");
    HILOGI("Hid_Device_Service_OnSetReport_00100 end");
}
 
/*
 * @tc.number: Hid_Device_Service_OnInterruptDataReceived_00100
 * @tc.name: OnInterruptDataReceived
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_OnInterruptDataReceived_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_OnInterruptDataReceived_00100 start");
    int id = 0;
    std::vector<uint8_t> data = {};
    hidDeviceObserverTest->OnInterruptDataReceived(id, data);
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnInterruptDataReceived");
    HILOGI("Hid_Device_Service_OnInterruptDataReceived_00100 end");
}
 
/*
 * @tc.number: Hid_Device_Service_OnVirtualCableUnplug_00100
 * @tc.name: OnVirtualCableUnplug
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_OnVirtualCableUnplug_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_OnVirtualCableUnplug_00100 start");
    hidDeviceObserverTest->OnVirtualCableUnplug();
    EXPECT_EQ(hidDeviceObserverTest->changeEvent_, "OnVirtualCableUnplug");
    HILOGI("Hid_Device_Service_OnVirtualCableUnplug_00100 end");
}

/*
 * @tc.number: Hid_Device_Service_HidProcessBtChrEvent_00100
 * @tc.name: HidProcessBtChrEvent
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceTest, Hid_Device_Service_HidProcessBtChrEvent_00100, TestSize.Level1)
{
    HILOGI("Hid_Device_Service_HidProcessBtChrEvent_00100 start");
    auto hidDeviceService = HidDeviceService::GetService();
    int state = static_cast<int>(BTConnectState::CONNECTING);
    hidDeviceService->HidProcessBtChrEvent(state);
    state = static_cast<int>(BTConnectState::CONNECTED);
    hidDeviceService->HidProcessBtChrEvent(state);
    state = static_cast<int>(BTConnectState::DISCONNECTED);
    hidDeviceService->HidProcessBtChrEvent(state);
    EXPECT_TRUE(hidDeviceService != nullptr); 
    HILOGI("Hid_Device_Service_HidProcessBtChrEvent_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS