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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "bluetooth_log.h"
#include "pc_power_manager.h"
#include <vector>
#include "bt_def.h"
#include "interface_adapter_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hw_interface.h"
#include "service_util.h"
#include "connect_strategy_manager.h"
#include "bluetooth_common_event_subscriber.h"
#include "adapter_manager.h"
#include "bluetooth_access_token_mock.h"
#include "bluetooth_host.h"
#include "bluetooth_host_server.h"

extern "C" {
static void HwVendorSpecificCommandMock(SpecificCmdToHisi *cmd)
{}

static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

static void HwSetConnReasonFlag(const RawAddress &bdAddr, int connType,
    const uint8_t *connReasonData, int connReasonDataLen) {}

static void hwBtsndHcicDisconnectMock(const RawAddress &bdAddr)
{}

static void HwSetLeConnectionScanToFastMock(bool enable) {}

static bool HwGetDeviceInfoMock(const RawAddress &bdAddr, bt_property_t &prop)
{
    return true;
}


uint8_t HwGetLocalDeviceEchoTypeMock()
{
#ifdef PC_FEATURE_FORCESLEEP_SUPPORT
    // pc
    return 0x29;
#else
    // tablet
    return 0x7;
#endif
}

static const bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwVendorSpecificCommand = HwVendorSpecificCommandMock,
    .hwBtsndHcicDisconnect = hwBtsndHcicDisconnectMock,
    .hwSetConnReasonFlag = HwSetConnReasonFlag,
    .hwSetLeConnectionScanToFast = HwSetLeConnectionScanToFastMock,
    .hwGetLocalDeviceEchoType = HwGetLocalDeviceEchoTypeMock,
    .hwGetDeviceInfo = HwGetDeviceInfoMock,
};
}  // extern "C"

namespace OHOS {
namespace bluetooth {
using namespace testing;
using namespace testing::ext;

constexpr int TIME = 2;
constexpr int TIMEOUT_TEST = 6;
constexpr int SCREENOFF_TIMEOUT_TEST = 120;

class PcPowerManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        Bluetooth::BluetoothAccessTokenMock::SetNativeTokenInfo();
        OHOS::bluetooth::IAdapterManager::GetInstance()->Stop();
        OHOS::bluetooth::IAdapterManager::GetInstance()->Start();
        OHOS::bluetooth::AdapterManager::GetInstance()->setBluetoothInterface(nullptr);
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}

    void AddBluetoothDevice()
    {
        // device br mouse
        std::string address = "11:11:11:11:11:11";
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
        auto device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
        if (device == nullptr)
        {
            HILOGI("device is nullptr");
            return;
        }
        device->SetPairedStatus(PAIR_PAIRED);
        device->SetConnectionHandle(0x80);
        device->SetDeviceClass(0x0580);
        device->SetAclConnectState(CONNECTION_STATE_CONNECTED);
        device->SetDeviceType(DEVICE_TYPE_BREDR);

        // device ble mouse
        address = "11:11:11:11:12:11";
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
        device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
        device->SetPairedStatus(PAIR_PAIRED);
        device->SetConnectionHandle(0x40);
        device->SetDeviceClass(0x0580);
        device->SetAclConnectState(CONNECTION_STATE_CONNECTED);
        device->SetDeviceType(DEVICE_TYPE_LE);

        // device br keyboard
        address = "11:11:11:11:11:12";
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
        device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
        device->SetPairedStatus(PAIR_PAIRED);
        device->SetConnectionHandle(0x80);
        device->SetDeviceClass(0x0540);
        device->SetAclConnectState(CONNECTION_STATE_CONNECTED);
        device->SetDeviceType(DEVICE_TYPE_BREDR);

        // device ble  earphone
        address = "11:11:11:11:11:13";
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
        device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
        device->SetPairedStatus(PAIR_PAIRED);
        device->SetConnectionHandle(0x81);
        device->SetDeviceClass(0x0640);
        device->SetAclConnectState(CONNECTION_STATE_CONNECTED);
        device->SetDeviceType(DEVICE_TYPE_LE);

        // device ble  earphone not aclconnect
        address = "11:11:11:11:11:14";
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
        device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
        device->SetPairedStatus(PAIR_PAIRED);
        device->SetConnectionHandle(0x81);
        device->SetDeviceClass(0x0640);
        device->SetAclConnectState(CONNECTION_STATE_DISCONNECTED);
        device->SetDeviceType(DEVICE_TYPE_LE);
    }
};

/**
 * @tc.name: Pc_Power_Manager_Test_Ability_Start_001
 * @tc.desc: PcPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Ability_Start_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Ability_Start_001 enter");
    AddBluetoothDevice();
    std::string eventData = "0";
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(static_cast<uint8_t>(PowerEvent::STATE_SWITCH_OFF), eventData);
    HILOGI("Pc_Power_Manager_Test_Ability_Start_001 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_Ability_Start_001
 * @tc.desc: PcPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Ability_Start_002, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Ability_Start_001 enter");
    AddBluetoothDevice();
    std::string eventData = "0";
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(static_cast<uint8_t>(PowerEvent::STATE_SWITCH_ON), eventData);
    HILOGI("Pc_Power_Manager_Test_Ability_Start_001 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_Switch_Event_001
 * @tc.desc: PcPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Switch_Event_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Switch_Event_001 enter");
    AddBluetoothDevice();
    std::string eventData = "0";
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(static_cast<uint8_t>(PowerEvent::STATE_SWITCH_OFF), eventData);
    sleep(TIME);
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(static_cast<uint8_t>(PowerEvent::STATE_SWITCH_ON), eventData);
    HILOGI("Pc_Power_Manager_Test_Switch_Event_001 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_Force_Sleep_Event_001
 * @tc.desc: PcPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Force_Sleep_Event_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Force_Sleep_Event_001 enter");
    AddBluetoothDevice();
    std::string eventData = "0";
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_ENTER_FORCESLEEP), eventData);
    sleep(TIME);
    ConnectStrategyManager::GetInstance()->SaveLastActiveDevice("");
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_EXIT_FORCESLEEP), eventData);
    HILOGI("Pc_Power_Manager_Test_Force_Sleep_Event_001 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_Force_Sleep_Event_002
 * @tc.desc: PcPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Force_Sleep_Event_002, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Force_Sleep_Event_002 enter");
    std::string eventData = "0";
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_ENTER_FORCESLEEP), eventData);
    sleep(TIME);
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_EXIT_FORCESLEEP), eventData);
    sleep(TIME);
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_ENTER_FORCESLEEP), eventData);
    sleep(TIME);
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_EXIT_FORCESLEEP), eventData);
    HILOGI("Pc_Power_Manager_Test_Force_Sleep_Event_002 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_Force_Sleep_Event_003
 * @tc.desc: PcPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Force_Sleep_Event_003, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Force_Sleep_Event_003 enter");
    AddBluetoothDevice();
    std::string eventData = "0";
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_ENTER_FORCESLEEP), eventData);
    sleep(TIMEOUT_TEST);
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_EXIT_FORCESLEEP), eventData);
    sleep(TIMEOUT_TEST);
    HILOGI("Pc_Power_Manager_Test_Force_Sleep_Event_003 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_external_screen_Event_001
 * @tc.desc: PcPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_external_screen_Event_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_external_screen_Event_001 enter");
    std::string eventData = "0";
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_POWER_START), eventData);
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_INSERT_SCREEN), eventData);
    sleep(TIMEOUT_TEST);
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_REMOVEING_SCREEN), eventData);
    HILOGI("Pc_Power_Manager_Test_external_screen_Event_001 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_Set_Connection_Scan_Event_001
 * @tc.desc: SetLeConnectionScanToFast:
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Set_Connection_Scan_Event_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Set_Connection_Scan_Event_001 enter");
    AddBluetoothDevice();
    bool ret = ForceSleep::GetInstance()->IsPairedMouseOrKeyboard();
    EXPECT_EQ(ret, true);
    ForceSleep::GetInstance()->SetLeConnectionScanToFast(false);
    ForceSleep::GetInstance()->SetLeConnectionScanToFast(true);
    HILOGI("Pc_Power_Manager_Test_Set_Connection_Scan_Event_001 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_Hid_Host_Callback_Event_001
 * @tc.desc: HidHostCallback::OnConnectionStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Hid_Host_Callback_Event_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Hid_Host_Callback_Event_001 enter");
    std::string eventData = "0";
    // enter foresleep
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_ENTER_FORCESLEEP), eventData);
    std::string address = "11:11:11:11:11:11";
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
    auto device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
    if (device != nullptr) {
        // device earphone
        device->SetDeviceClass(0x0640);
        HidHostCallback *hidHostCallback = new(std::nothrow)HidHostCallback();
        if (hidHostCallback != nullptr) {
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::CONNECTED), 0);
            // device mouse
            device->SetDeviceClass(0x0580);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::CONNECTED), 0);
            // exit foresleep
            PowerMgrHelper::GetInstance()->OnPowerStateChanged(
                static_cast<uint8_t>(PowerEvent::STATE_EXIT_FORCESLEEP), eventData);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::CONNECTED), 0);
            if (hidHostCallback != nullptr) {
                delete(hidHostCallback);
                hidHostCallback = nullptr;
            }
        }
    }
    EXPECT_TRUE(ForceSleep::GetInstance()->IsPairedMouseOrKeyboard());
    HILOGI("Pc_Power_Manager_Test_Hid_Host_Callback_Event_001 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_Hid_Host_Callback_Event_002
 * @tc.desc: HidHostCallback::OnConnectionStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_Hid_Host_Callback_Event_002, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_Hid_Host_Callback_Event_002 enter");
    std::string eventData = "0";
    // enter foresleep
    PowerMgrHelper::GetInstance()->OnPowerStateChanged(
        static_cast<uint8_t>(PowerEvent::STATE_SWITCH_OFF), eventData);
    std::string address = "11:22:33:11:11:11";
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
    auto device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
    if (device != nullptr) {
        // device earphone
        device->SetDeviceClass(0x0640);
        HidHostCallback *hidHostCallback = new(std::nothrow)HidHostCallback();
        if (hidHostCallback != nullptr) {
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::CONNECTED), 0);
            // device mouse
            device->SetDeviceClass(0x0580);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::CONNECTED), 0);
            // insert screen
            PowerMgrHelper::GetInstance()->OnPowerStateChanged(
                static_cast<uint8_t>(PowerEvent::STATE_INSERT_SCREEN), eventData);
            sleep(TIMEOUT_TEST);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::CONNECTED), 0);
            PowerMgrHelper::GetInstance()->OnPowerStateChanged(
                static_cast<uint8_t>(PowerEvent::STATE_REMOVEING_SCREEN), eventData);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
            hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::CONNECTED), 0);
            if (hidHostCallback != nullptr) {
                delete(hidHostCallback);
                hidHostCallback = nullptr;
            }
        }
    }
    EXPECT_TRUE(ForceSleep::GetInstance()->IsPairedMouseOrKeyboard());
    HILOGI("Pc_Power_Manager_Test_Hid_Host_Callback_Event_001 end");
}

/**
 * @tc.name: Pc_Power_Manager_Test_BluetoothState_001
 * @tc.desc: HidHostCallback::OnConnectionStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_BluetoothState_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_BluetoothState_001 enter");
    std::string address = "44:44:44:11:11:11";
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
    auto device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
    EXPECT_EQ(device != nullptr, true);
    device->SetDeviceClass(0x0580);
    HidHostCallback *hidHostCallback = new (std::nothrow) HidHostCallback();
    EXPECT_EQ(hidHostCallback != nullptr, true);
    hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
    // enter screen off
    PowerMgrHelper::GetInstance()->OnPowerConnectStateChanged(
        EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    AdapterStateCallback *adapterStateCallback = new (std::nothrow) AdapterStateCallback();
    EXPECT_EQ(adapterStateCallback != nullptr, true);
    adapterStateCallback->OnBluetoothStateChanged(BluetoothSwitchState::STATE_ON);
    if (hidHostCallback != nullptr) {
        delete(hidHostCallback);
        hidHostCallback = nullptr;
    }
    if (adapterStateCallback != nullptr) {
        delete(adapterStateCallback);
        adapterStateCallback = nullptr;
    }
}

/**
 * @tc.name: Pc_Power_Manager_Test_PowerConnectState_001
 * @tc.desc: OnPowerConnectStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_PowerConnectState_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_PowerConnectState_001 enter");
    std::string address = "22:22:22:11:11:11";
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address));
    auto device = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(address));
    EXPECT_EQ(device != nullptr, true);
    device->SetDeviceClass(0x0580);
    HidHostCallback *hidHostCallback = new(std::nothrow)HidHostCallback();
    EXPECT_EQ(hidHostCallback != nullptr, true);
    hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
    PowerMgrHelper::GetInstance()->OnPowerConnectStateChanged(
        EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    sleep(SCREENOFF_TIMEOUT_TEST);
    hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::CONNECTED), 0);
    hidHostCallback->OnConnectionStateChanged(RawAddress(address),
                static_cast<int>(BTConnectState::DISCONNECTED), 0);
    PowerMgrHelper::GetInstance()->OnPowerConnectStateChanged(
        EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
}

/**
 * @tc.name: Pc_Power_Manager_Test_ScreenState_001
 * @tc.desc: OnScreenStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_ScreenState_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_ScreenState_001 enter");
    sptr<Bluetooth::BluetoothHostServer> hostServer = Bluetooth::BluetoothHostServer::GetInstance();
    EXPECT_EQ(hostServer != nullptr, true);
    // quietMode, needReconnectActiveDevice
    hostServer->EnableBle(true, false, "resource_schedule_service");
    EXPECT_TRUE(AdapterManager::GetInstance()->GetQuietMode());
    EXPECT_TRUE(PowerMgrHelper::GetInstance()->GetNeedReconnectActiveDevice());
    PowerMgrHelper::GetInstance()->OnScreenStateChanged(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    EXPECT_FALSE(AdapterManager::GetInstance()->GetQuietMode());
    // quietMode, no needReconnectActiveDevice
    hostServer->EnableBle(true);
    EXPECT_TRUE(AdapterManager::GetInstance()->GetQuietMode());
    EXPECT_FALSE(PowerMgrHelper::GetInstance()->GetNeedReconnectActiveDevice());
    // no quietMode, no needReconnectActiveDevice
    hostServer->EnableBle(false);
    EXPECT_FALSE(PowerMgrHelper::GetInstance()->GetNeedReconnectActiveDevice());
}

/**
 * @tc.name: Pc_Power_Manager_Test_CheckIfNeedAutoConnect_001
 * @tc.desc: CheckIfNeedAutoConnect
 * @tc.type: FUNC
 */
HWTEST_F(PcPowerManagerTest, Pc_Power_Manager_Test_CheckIfNeedAutoConnect_001, TestSize.Level1)
{
    HILOGI("Pc_Power_Manager_Test_CheckIfNeedAutoConnect_001 enter");
    std::string address = "11:22:33:44:55:66";
    ConnectStrategyManager::GetInstance()->SaveLastConnectedDevice(address);
    bool needAutoConnect = PowerMgrHelper::GetInstance()->CheckIfNeedAutoConnect();
    EXPECT_EQ(needAutoConnect, true);

    address = "";
    ConnectStrategyManager::GetInstance()->SaveLastConnectedDevice(address);
    needAutoConnect = PowerMgrHelper::GetInstance()->CheckIfNeedAutoConnect();
    EXPECT_EQ(needAutoConnect, false);
}
}  // namespace bluetooth
}  // namespace OHOS
