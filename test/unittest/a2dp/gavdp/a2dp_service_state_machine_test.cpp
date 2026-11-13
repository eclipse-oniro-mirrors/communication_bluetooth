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
#include "adapter_config.h"
#include "a2dp_service.h"
#include "a2dp_src/a2dp_src_service.h"
#include "a2dp_service_state_machine.h"
#include "a2dp_src_btif_interface_mock.h"
#include "bluetooth_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hw_interface.h"
#include "class_creator.h"
#include "classic_adapter.h"
#include "profile_service_manager.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;

extern "C" {
static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

static bool HwGetDeviceInfoMock(const STACK::RawAddress &addr, bt_property_t &prop)
{
    return true;
}

static void HwBtsndHcicDisconnectMock(const STACK::RawAddress &bdAddr)
{
    HILOGI("enter");
}

static bool IsIncomingConnectionMock(const STACK::RawAddress &bdAddr)
{
    HILOGI("enter");
    return true;
}

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwGetDeviceInfo = HwGetDeviceInfoMock,
    .hwBtsndHcicDisconnect = HwBtsndHcicDisconnectMock,
    .isIncomingConnection = IsIncomingConnectionMock,
};
}
namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;
using namespace OHOS::bluetoothMock;

void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

constexpr int TIME = 4;
constexpr int TIME_2 = 2;
static BluetoothHost *host_;
A2dpStateManager stateMachine;

class A2dpServStateMachineTest : public testing::Test {
public:
    A2dpServStateMachineTest()
    {}
    ~A2dpServStateMachineTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::unique_ptr<A2dpDisconnected> disconnected;
    std::unique_ptr<A2dpDisconnecting> disconnecting;
    std::unique_ptr<A2dpConnected> connected;
    std::unique_ptr<A2dpConnecting> connecting;

    A2dpService *a2dpService = nullptr;
};

void A2dpServStateMachineTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBrEnable = %{public}d", isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    ClassFactory::RegisterClass("A2dpSrcService", A2dpSrcServiceCreate);

    HILOGI("SetUpTestCase end");
}

void A2dpServStateMachineTest::TearDownTestCase(void)
{
    host_ = nullptr;
    utility::CloseAllTimer();
}

void A2dpServStateMachineTest::SetUp()
{
    HILOGI("SetUp start");
    disconnected =
        std::make_unique<A2dpDisconnected>(A2DP_STATE_DISCONNECTED, stateMachine);
    disconnecting =
        std::make_unique<A2dpDisconnecting>(A2DP_STATE_DISCONNECTING, stateMachine);
    connected =
        std::make_unique<A2dpConnected>(A2DP_STATE_CONNECTED, stateMachine);
    connecting =
        std::make_unique<A2dpConnecting>(A2DP_STATE_CONNECTING, stateMachine);

    // new A2dpSrcService
    auto manager = ProfileServiceManager::GetInstance();
    string name = "A2dpSrcService";
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, name);

    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    HILOGI("a2dpService1: %{public}p", a2dpService);

    A2dpService *a2dpService2 = static_cast<A2dpService *>(manager->GetProfileService(name));
    HILOGI("a2dpService2: %{public}p", a2dpService2);

    HILOGI("SetUp end");
}

void A2dpServStateMachineTest::TearDown(void)
{
    HILOGI("TearDown start");
    disconnected.reset();
    disconnecting.reset();
    connected.reset();
    connecting.reset();

    // delete A2dpSrcService
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    delete a2dpService;
    sleep(TIME_2);
    HILOGI("TearDown end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpDisconnectedDispatch_001
 * @tc.name: A2dpDisconnected::Dispatch
 * @tc.desc: service == nullptr.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectedDispatch_001, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectedDispatch_001 start");
    RawAddress device;
    uint8_t role = 6;
    utility::Message msg(A2DP_MSG_CONNECT, role, &device);
    int ret = disconnected->Dispatch(msg);
    EXPECT_TRUE(ret == false);
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectedDispatch_001 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpDisconnectedDispatch_002
 * @tc.name: A2dpDisconnected::Dispatch
 * @tc.desc: service is not nullptr
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectedDispatch_002, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectedDispatch_002 start");

    RawAddress device = RawAddress("11:22:33:44:55:66");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        btav_source_interface_t interface;
        interface.connect = ConnectMock;
        interface.disconnect = DisconnectMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;

        uint8_t role = A2DP_ROLE_SOURCE;
        utility::Message msg(A2DP_MSG_CONNECT, role, &device);
        int ret = disconnected->Dispatch(msg);
        EXPECT_TRUE(ret == true);

        utility::Message msg1(A2DP_MSG_PROFILE_CONNECTED, role, &device);
        ret = disconnected->Dispatch(msg1);
        EXPECT_TRUE(ret == true);

        utility::Message msg2(A2DP_MSG_PROFILE_CONNECTING, role, &device);
        ret = disconnected->Dispatch(msg2);
        EXPECT_TRUE(ret == true);

        utility::Message msg3(A2DP_MSG_CONNECT_FORBIDDEN, role, &device);
        ret = disconnected->Dispatch(msg3);
        EXPECT_TRUE(ret == true);

        utility::Message msg4(A2DP_MSG_PROFILE_DISCONNECTED, role, &device);
        ret = disconnected->Dispatch(msg4);
        EXPECT_TRUE(ret == true);
    }
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectedDispatch_002 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_001
 * @tc.name: A2dpDisconnecting::Dispatch
 * @tc.desc: msg.arg2_ is nullptr.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_001, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_001 start");
    uint8_t role = A2DP_ROLE_SOURCE;
    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTED, role, nullptr);
    int ret = disconnecting->Dispatch(msg);
    EXPECT_TRUE(ret == false);
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_001 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_002
 * @tc.name: A2dpDisconnecting::Dispatch
 * @tc.desc: service is nullptr.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_002, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_002 start");
    RawAddress device;
    uint8_t role = 6;
    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTED, role, &device);
    int ret = disconnecting->Dispatch(msg);
    EXPECT_TRUE(ret == false);
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_002 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_003
 * @tc.name: A2dpDisconnecting::Dispatch
 * @tc.desc: msg.what_ = A2DP_MSG_CONNECT_TIMEOUT.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_003, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_003 start");

    RawAddress device = RawAddress("11:22:33:44:55:66");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);

        btav_source_interface_t interface;
        interface.connect = ConnectMock;
        interface.disconnect = DisconnectMock;
        interface.set_silence_device = SetSilenceDeviceMock;
        interface.set_active_device = SetActiveDeviceMock;
        interface.config_codec = ConfigCodecMock;
        interface.cleanup = CleanupMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;

        uint8_t role = A2DP_ROLE_SOURCE;
        utility::Message msg(A2DP_MSG_CONNECT_TIMEOUT, role, &device);
        bool ret = disconnecting->Dispatch(msg);
        EXPECT_TRUE(ret);
        EXPECT_EQ(deviceInfo->GetConnectState(), static_cast<int>(BTConnectState::DISCONNECTED));
    }
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_003 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_004
 * @tc.name: A2dpDisconnecting::Dispatch
 * @tc.desc: msg.what_ = A2DP_MSG_PROFILE_DISCONNECTED.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_004, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_004 start");

    RawAddress device = RawAddress("11:22:33:44:55:66");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);

        btav_source_interface_t interface;
        interface.connect = ConnectMock;
        interface.disconnect = DisconnectMock;
        interface.set_silence_device = SetSilenceDeviceMock;
        interface.set_active_device = SetActiveDeviceMock;
        interface.config_codec = ConfigCodecMock;
        interface.cleanup = CleanupMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;

        uint8_t role = A2DP_ROLE_SOURCE;
        utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTED, role, &device);
        bool ret = disconnecting->Dispatch(msg);
        EXPECT_TRUE(ret);
    }
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_004 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_005
 * @tc.name: A2dpDisconnecting::Dispatch
 * @tc.desc: msg.what_ = default.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_005, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_005 start");

    RawAddress device = RawAddress("11:22:33:44:55:66");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);

        btav_source_interface_t interface;
        interface.connect = ConnectMock;
        interface.disconnect = DisconnectMock;
        interface.set_silence_device = SetSilenceDeviceMock;
        interface.set_active_device = SetActiveDeviceMock;
        interface.config_codec = ConfigCodecMock;
        interface.cleanup = CleanupMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;

        uint8_t role = A2DP_ROLE_SOURCE;
        utility::Message msg(A2DP_MSG_PROFILE_AUDIO_PLAY_SUSPEND, role, &device);
        bool ret = disconnecting->Dispatch(msg);
        EXPECT_TRUE(ret);
    }
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectingDispatch_005 end");
}


/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpConnectedDispatch_001
 * @tc.name: A2dpConnected::Dispatch
 * @tc.desc: service == nullptr.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpConnectedDispatch_001, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpConnectedDispatch_001 start");
    RawAddress device;
    uint8_t role = 6;
    utility::Message msg(A2DP_MSG_CONNECT, role, &device);
    int ret = connected->Dispatch(msg);
    EXPECT_TRUE(ret == false);
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpConnectedDispatch_001 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpConnectedDispatch_002
 * @tc.name: A2dpConnected::Dispatch
 * @tc.desc: service is not nullptr
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpConnectedDispatch_002, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpConnectedDispatch_002 start");

    RawAddress device = RawAddress("11:22:33:44:55:66");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        btav_source_interface_t interface;
        interface.connect = ConnectMock;
        interface.disconnect = DisconnectMock;
        interface.set_silence_device = SetSilenceDeviceMock;
        interface.set_active_device = SetActiveDeviceMock;
        interface.config_codec = ConfigCodecMock;
        interface.cleanup = CleanupMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;
        BluetoothHwInterface::GetInstance()->SetBtHwInterface(&g_btHwIfTest);

        uint8_t role = A2DP_ROLE_SOURCE;
        utility::Message msg(A2DP_MSG_DISCONNECT, role, &device);
        int ret = connected->Dispatch(msg);
        EXPECT_TRUE(ret == true);

        utility::Message msg1(A2DP_MSG_PROFILE_DISCONNECTED, role, &device);
        ret = connected->Dispatch(msg1);
        EXPECT_TRUE(ret == true);

        utility::Message msg2(A2DP_MSG_PROFILE_DISCONNECTING, role, &device);
        ret = connected->Dispatch(msg2);
        EXPECT_TRUE(ret == true);

        utility::Message msg3(A2DP_MSG_PROFILE_CONNECTING, role, &device);
        ret = connected->Dispatch(msg3);
        EXPECT_TRUE(ret == true);

        utility::Message msg4(A2DP_MSG_PROFILE_CODEC_CHANGE, role, &device);
        ret = connected->Dispatch(msg4);
        EXPECT_TRUE(ret == true);
    }
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpConnectedDispatch_002 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_UpdateDeviceInformation_001
 * @tc.name: A2dpConnected::UpdateDeviceInformation
 * @tc.desc: service == nullptr.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_UpdateDeviceInformation_001, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_UpdateDeviceInformation_001 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    uint8_t role = 6;
    connected->UpdateDeviceInformation(device, false, role);

    role = A2DP_ROLE_SOURCE;
    connected->UpdateDeviceInformation(device, false, role);
    EXPECT_TRUE(connected != NULL);
    HILOGI("A2dpServStateMachineTest_UnitTest_UpdateDeviceInformation_001 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpConnectingDispatch_001
 * @tc.name: A2dpConnecting::Dispatch
 * @tc.desc: msg.arg2_ is nullptr
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpConnectingDispatch_001, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpConnectingDispatch_001 start");
    uint8_t role = A2DP_ROLE_SOURCE;
    utility::Message msg(A2DP_MSG_CONNECT_TIMEOUT, role, nullptr);
    int ret = connecting->Dispatch(msg);
    EXPECT_TRUE(ret == false);
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpConnectingDispatch_001 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpConnectingDispatch_002
 * @tc.name: A2dpConnecting::Dispatch
 * @tc.desc: service is not nullptr
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpConnectingDispatch_002, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpConnectingDispatch_002 start");

    RawAddress device = RawAddress("11:22:33:44:55:66");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        btav_source_interface_t interface;
        interface.connect = ConnectMock;
        interface.disconnect = DisconnectMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;

        uint8_t role = A2DP_ROLE_SOURCE;
        utility::Message msg(A2DP_MSG_CONNECT_TIMEOUT, role, &device);
        int ret = connecting->Dispatch(msg);
        EXPECT_TRUE(ret == true);

        utility::Message msg1(A2DP_MSG_PROFILE_CONNECTED, role, &device);
        ret = connecting->Dispatch(msg1);
        EXPECT_TRUE(ret == true);

        utility::Message msg2(A2DP_MSG_PROFILE_DISCONNECTED, role, &device);
        ret = connecting->Dispatch(msg2);
        EXPECT_TRUE(ret == true);

        utility::Message msg3(A2DP_MSG_PROFILE_DISCONNECTING, role, &device);
        ret = connecting->Dispatch(msg3);
        EXPECT_TRUE(ret == true);

        utility::Message msg4(A2DP_MSG_DISCONNECT, role, &device);
        ret = connecting->Dispatch(msg4);
        EXPECT_TRUE(ret == true);

        utility::Message msg5(A2DP_MSG_CONNECT_FORBIDDEN, role, &device);
        ret = connecting->Dispatch(msg5);
        EXPECT_TRUE(ret == true);

        utility::Message msg6(A2DP_MSG_CONNECT, role, &device);
        ret = connecting->Dispatch(msg6);
        EXPECT_TRUE(ret == true);
    }
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpConnectingDispatch_002 end");
}

/*
 * @tc.number: A2dpServStateMachineTest_UnitTest_A2dpDisconnectedEntry_001
 * @tc.name: A2dpDisconnected::Entry
 * @tc.desc: service == nullptr.
*/
HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectedEntry_001, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectedEntry_001 start");
    RawAddress device = RawAddress("11:22:33:44:55:67");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    if (a2dpService != nullptr && bluetoothDevice != nullptr) {
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
            deviceInfo->SetTryReconnectA2dp(true);
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
            stateMachine.SetDevice(device);
            stateMachine.GetDevice();
            bluetoothDevice->SetAclConnectState(1);
            bluetoothDevice->SetPairedStatus(3);
            bluetoothDevice->SetHwTalkBandBandMode(1);
            bluetoothDevice->SetAutoConnSwitch(1);
            disconnected->Entry();
            bluetoothDevice->SetAutoConnSwitch(0);
            disconnected->Entry();
            deviceInfo->SetTryReconnectA2dp(false);
            disconnected->Entry();
        }
    }
    EXPECT_TRUE(disconnected != NULL);
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectedEntry_001 end");
    sleep(TIME);
}

HWTEST_F(A2dpServStateMachineTest, A2dpServStateMachineTest_UnitTest_A2dpDisconnectedEntry_002, TestSize.Level1)
{
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectedEntry_002 start");
    RawAddress device = RawAddress("11:22:33:44:55:67");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    if (a2dpService != nullptr && bluetoothDevice != nullptr) {
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
            deviceInfo->SetTryReconnectA2dp(true);
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
            stateMachine.SetDevice(device);
            stateMachine.GetDevice();
            bluetoothDevice->SetAclConnectState(1);
            bluetoothDevice->SetPairedStatus(3);
            bluetoothDevice->SetHwTalkBandBandMode(1);
            bluetoothDevice->SetAutoConnSwitch(1);
            disconnected->Entry();
            bluetoothDevice->SetAclConnectState(0);
        }
    }
    EXPECT_TRUE(disconnected != NULL);
    HILOGI("A2dpServStateMachineTest_UnitTest_A2dpDisconnectedEntry_002 end");
    sleep(TIME);
}
}  // namespace Bluetooth
}  // namespace OHOS