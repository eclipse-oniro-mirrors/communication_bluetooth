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
#include <thread>
#include "adapter_config.h"
#include "a2dp_service.h"
#include "a2dp_src_btif_interface_mock.h"
#include "a2dp_snk_btif_interface_mock.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "btif_interface_mock.h"
#include "bluetooth_errorcode.h"
#include "connect_strategy_manager.h"
#include "profile_service_manager.h"
#include "profile_config.h"
#include "remote_device_properties.h"
#include "adapter_manager.h"
#include "a2dp_src/a2dp_src_service.h"
#include "class_creator.h"
#include "thread_util.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_hw_interface.h"
#include "a2dp_service_mock.h"
#include "hfp_ag_system_interface.h"
#include "../../../../services/bluetooth/service/src/bluetooth_audio_manager/bluetooth_audio_manager.h"
#include "bluetooth_state_manager.h"
#include "bt_chr_business_event.h"
#include "battery_srv_client.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "bluetooth_audio_framework_adapter.h"
#include "system_ability_processer.h"

using namespace testing;
using namespace testing::ext;
constexpr int TIME = 2;
constexpr int TIME_10 = 10;

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
static BluetoothHost *host_;

class A2dpServiceTest : public testing::Test {
public:
    A2dpServiceTest()
    {}
    ~A2dpServiceTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void A2dpServiceTest::SetUpTestCase(void)
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

    ProfileConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("A2dpSrcService", A2dpSrcServiceCreate);

    HILOGI("SetUpTestCase end");
}

void A2dpServiceTest::TearDownTestCase(void)
{
    utility::CloseAllTimer();
    sleep(TIME);
}

void A2dpServiceTest::SetUp()
{
    HILOGI("SetUp start");
    auto manager = ProfileServiceManager::GetInstance();
    string name = "A2dpSrcService";
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, name);
    // Disable a2dp thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_A2DP, ThreadUtil::DISABLED);
    HILOGI("SetUp end");
}

void A2dpServiceTest::TearDown(void)
{
    auto manager = ProfileServiceManager::GetInstance();
    A2dpSrcService *src = static_cast<A2dpSrcService *>(manager->GetProfileService("A2dpSrcService"));
    delete src;
}

static void *ClassicAdapterCreate(void)
{
    return new ClassicAdapter();
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_EnableService_001
 * @tc.name: EnableService
 * @tc.desc: for src role, bt_interface is null.
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_EnableService_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_EnableService_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        btav_source_interface_t interface;
        interface.init = InitMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;

        AdapterManager::GetInstance()->setBluetoothInterface(NULL);
        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_EnableService_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_EnableService_002
 * @tc.name: EnableService
 * @tc.desc: for src role
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_EnableService_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_EnableService_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMock;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_EnableService_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_ClearDeviceMap_001
 * @tc.name: ClearDeviceMap
 * @tc.desc: Clear Device Map
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ClearDeviceMap_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ClearDeviceMap_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->ClearDeviceMap();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_ClearDeviceMap_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_DisableService_001
 * @tc.name: DisableService
 * @tc.desc: for src role and a2dpDevices_ is null
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_DisableService_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_DisableService_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        btav_source_interface_t interface;
        interface.cleanup = CleanupMock;
        interface.set_active_device = SetActiveDeviceMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;
        a2dpService->DisableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_DisableService_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_DisableService_002
 * @tc.name: DisableService
 * @tc.desc: for src role and a2dpDevices_ is not null
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_DisableService_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_DisableService_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        btav_source_interface_t interface;
        interface.cleanup = CleanupMock;
        interface.set_active_device = SetActiveDeviceMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;
        a2dpService->DisableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_DisableService_002 end");
}

#ifdef BLUETOOTH_A2DP_SINK_ENABLE
/*
 * @tc.number: A2dpServiceTest_UnitTest_DisableService_003
 * @tc.name: DisableService
 * @tc.desc: for snk role, sink feature is not open, so a2dpService is null
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_DisableService_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_DisableService_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SINK);
    if (a2dpService != nullptr) {
        btav_sink_interface_t interface;
        interface.cleanup = CleanupSnkMock;
        interface.set_active_device = SetActiveDeviceMock;
        a2dpService->sBluetoothA2dpSnkInterface = &interface;
        a2dpService->DisableService();
    }
    EXPECT_FALSE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_DisableService_003 end");
}
#endif

/*
 * @tc.number: A2dpServiceTest_UnitTest_Connect_001
 * @tc.name: Connect
 * @tc.desc:
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Connect_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Connect_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device;
    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Connect(device);
    }
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_Connect_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Connect_002
 * @tc.name: Connect
 * @tc.desc: device state is CONNECTED, return BAD STATUS
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Connect_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Connect_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }

    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Connect(device);
    }
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_Connect_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Connect_003
 * @tc.name: Connect
 * @tc.desc: device state is CONNECTED, return BAD STATUS
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Connect_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Connect_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }

    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Connect(device);
    }
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_Connect_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Connect_004
 * @tc.name: Connect
 * @tc.desc: connection strategy is forbidden, return RET_NO_SUPPORT
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Connect_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Connect_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    bool ret1 = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(
        device, PROPERTY_A2DP_CONNECTION_POLICY, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    HILOGI("A2dpServiceTest_UnitTest_Connect_004 SaveConnectStrategy:%d", ret1);

    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Connect(device);
    }
    EXPECT_TRUE(a2dpService != nullptr);
    HILOGI("A2dpServiceTest_UnitTest_Connect_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Connect_005
 * @tc.name: Connect
 * @tc.desc: connection strategy is accept, return RET_NO_SUPPORT
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Connect_005, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Connect_005 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    a2dpService->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));

    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Connect(device);
    }
    EXPECT_EQ(ret, BT_ERR_PROFILE_DISABLED);
    HILOGI("A2dpServiceTest_UnitTest_Connect_005 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Disconnect_001
 * @tc.name: Disconnect
 * @tc.desc: could find the device, but the connecState is DISCONNECTED, return BAD_STATUS
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Disconnect_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Disconnect_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }

    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Disconnect(device);
    }
    EXPECT_TRUE(ret == RET_BAD_STATUS);
    HILOGI("A2dpServiceTest_UnitTest_Disconnect_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Disconnect_002
 * @tc.name: Disconnect
 * @tc.desc: could find the device, but the connecState is DISCONNECTING, return BAD_STATUS
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Disconnect_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Disconnect_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }

    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Disconnect(device);
    }
    EXPECT_TRUE(ret == RET_BAD_STATUS);
    HILOGI("A2dpServiceTest_UnitTest_Disconnect_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Disconnect_003
 * @tc.name: Disconnect
 * @tc.desc: could find the device, but the connecState is DISCONNECTING, return BAD_STATUS
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Disconnect_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Disconnect_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }

    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Disconnect(device);
    }
    EXPECT_TRUE(ret == RET_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_Disconnect_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Disconnect_004
 * @tc.name: Disconnect
 * @tc.desc: could not find the device, return RET_BAD_STATUS
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Disconnect_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Disconnect_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("00:00:00:00:00:00");
    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Disconnect(device);
    }
    EXPECT_TRUE(ret == RET_BAD_STATUS);
    HILOGI("A2dpServiceTest_UnitTest_Disconnect_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetConnectDevices_001
 * @tc.name: GetConnectDevices
 * @tc.desc:
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetConnectDevices_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetConnectDevices_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->GetConnectDevices();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetConnectDevices_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetConnectDevices_002
 * @tc.name: GetConnectDevices
 * @tc.desc: connectionState is CONNECTED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetConnectDevices_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetConnectDevices_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");

        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        a2dpService->GetConnectDevices();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetConnectDevices_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetConnectDevices_003
 * @tc.name: GetConnectDevices
 * @tc.desc: connectionState is not CONNECTED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetConnectDevices_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetConnectDevices_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");

        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        a2dpService->GetConnectDevices();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetConnectDevices_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetConnectDevices_004
 * @tc.name: GetConnectDevices
 * @tc.desc: it->second is nullptr
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetConnectDevices_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetConnectDevices_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("22:22:22:22:22:22");
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), nullptr);

        a2dpService->GetConnectDevices();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetConnectDevices_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_001
 * @tc.name: GetConnectingOrConnectionDevices
 * @tc.desc:
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->GetConnectingOrConnectionDevices();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_002
 * @tc.name: GetConnectingOrConnectionDevices
 * @tc.desc: connectionState is CONNECTED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");

        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        a2dpService->GetConnectingOrConnectionDevices();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_003
 * @tc.name: GetConnectingOrConnectionDevices
 * @tc.desc: connectionState is not CONNECTED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");

        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        a2dpService->GetConnectingOrConnectionDevices();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_004
 * @tc.name: GetConnectingOrConnectionDevices
 * @tc.desc: it->second is nullptr
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("22:22:22:22:22:22");
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), nullptr);

        a2dpService->GetConnectingOrConnectionDevices();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetConnectingOrConnectionDevices_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetDevicesByStates_001
 * @tc.name: GetDevicesByStates
 * @tc.desc: state matched
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetDevicesByStates_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetDevicesByStates_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");

        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
        a2dpService->GetDevicesByStates(states);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetDevicesByStates_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetDevicesByStates_002
 * @tc.name: GetDevicesByStates
 * @tc.desc: state not matched
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetDevicesByStates_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetDevicesByStates_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTING)};
        a2dpService->GetDevicesByStates(states);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetDevicesByStates_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetDeviceState_001
 * @tc.name: GetDeviceState
 * @tc.desc: could find the device
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetDeviceState_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetDeviceState_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        a2dpService->GetDeviceState(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetDeviceState_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetDeviceState_002
 * @tc.name: GetDeviceState
 * @tc.desc: could not find the device
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetDeviceState_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetDeviceState_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:00:00:00:11:22");
        a2dpService->GetDeviceState(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetDeviceState_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetActiveSinkDevice_001
 * @tc.name: SetActiveSinkDevice
 * @tc.desc: device is found in a2dpDevices_ and GetConnectState is not CONNECTED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetActiveSinkDevice_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetActiveSinkDevice_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }

    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->SetActiveSinkDevice(device);
    }
    EXPECT_TRUE(ret == RET_BAD_STATUS);
    HILOGI("A2dpServiceTest_UnitTest_SetActiveSinkDevice_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetActiveSinkDevice_002
 * @tc.name: SetActiveSinkDevice
 * @tc.desc: the device is already active
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetActiveSinkDevice_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetActiveSinkDevice_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        a2dpService->UpdateActiveDevice(device);
    }
    btav_source_interface_t interface;
    interface.set_active_device = SetActiveDeviceMock;
    a2dpService->sBluetoothA2dpSrcInterface = &interface;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);

    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->SetActiveSinkDevice(device);
    }
    EXPECT_TRUE(ret == RET_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetActiveSinkDevice_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetActiveSinkDevice_003
 * @tc.name: SetActiveSinkDevice
 * @tc.desc: ready to set_active_device from A to B
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetActiveSinkDevice_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetActiveSinkDevice_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    RawAddress device = RawAddress("11:22:33:44:55:66");
    RawAddress activeDevice_ = RawAddress("00:00:00:00:00:00");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        a2dpService->UpdateActiveDevice(activeDevice_);
    }

    btav_source_interface_t interface;
    interface.set_active_device = SetActiveDeviceMock;
    a2dpService->sBluetoothA2dpSrcInterface = &interface;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);

    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->SetActiveSinkDevice(device);
    }
    EXPECT_TRUE(ret == RET_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetActiveSinkDevice_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetActiveSinkDevice_004
 * @tc.name: SetActiveSinkDevice
 * @tc.desc: ready to set_active_device from A to empty
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetActiveSinkDevice_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetActiveSinkDevice_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    EXPECT_TRUE(a2dpService != nullptr);

    RawAddress device = RawAddress("10:21:25:44:55:66");
    RawAddress activeDevice_ = RawAddress("00:00:00:00:00:00");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        deviceInfo->SetIsHdapConnected(true);
        a2dpService->SetHdapDeviceInfo(deviceInfo);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        a2dpService->UpdateActiveDevice(device);
    }

    EXPECT_TRUE(a2dpService->GetHdapDeviceInfo() != nullptr);

    btav_source_interface_t interface;
    interface.set_active_device = SetActiveDeviceMock;
    a2dpService->sBluetoothA2dpSrcInterface = &interface;
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);

    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        deviceInfo->SetIsHdapConnected(false);
        ret = a2dpService->SetActiveSinkDevice(activeDevice_);
    }
    EXPECT_TRUE(ret == RET_NO_ERROR);
    EXPECT_TRUE(a2dpService->GetHdapDeviceInfo() != nullptr);
    HILOGI("A2dpServiceTest_UnitTest_SetActiveSinkDevice_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetConnectStrategy_001
 * @tc.name: SetConnectStrategy
 * @tc.desc: SaveConnectStrategy failed, return BT_ERR_INTERNAL_ERROR
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    int ret = Bluetooth::BT_NO_ERROR;
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:00:00:00:00:00");
        ret = a2dpService->SetConnectStrategy(device, strategy);
    }
    EXPECT_TRUE(ret == Bluetooth::BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetConnectStrategy_002
 * @tc.name: SetConnectStrategy
 * @tc.desc: CONNECTION_ALLOWED && DISCONNECTED, return BT_NO_ERROR
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetConnectStrategy_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);

    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        AdapterConfig::GetInstance()->Load();
        ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
        AdapterManager::GetInstance()->CreateAdapters();
        ret = a2dpService->SetConnectStrategy(device, strategy);
    }
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetConnectStrategy_003
 * @tc.name: SetConnectStrategy
 * @tc.desc: CONNECTION_ALLOWED && CONNECTED, return BT_NO_ERROR
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetConnectStrategy_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);

    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        AdapterConfig::GetInstance()->Load();
        ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
        AdapterManager::GetInstance()->CreateAdapters();
        ret = a2dpService->SetConnectStrategy(device, strategy);
    }
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetConnectStrategy_004
 * @tc.name: SetConnectStrategy
 * @tc.desc: CONNECTION_FORBIDDEN && DISCONNECTED, return BT_NO_ERROR
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetConnectStrategy_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);

    int strategy = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->SetConnectStrategy(device, strategy);
    }
    EXPECT_TRUE(ret == BT_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetConnectStrategy_005
 * @tc.name: SetConnectStrategy
 * @tc.desc: CONNECTION_FORBIDDEN && DISCONNECTING, return BT_NO_ERROR
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetConnectStrategy_005, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_005 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);

    int strategy = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->SetConnectStrategy(device, strategy);
    }
    EXPECT_TRUE(ret == BT_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_005 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetConnectStrategy_006
 * @tc.name: SetConnectStrategy
 * @tc.desc: CONNECTION_FORBIDDEN && CONNECTING, return BT_NO_ERROR
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetConnectStrategy_006, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_006 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);

    int strategy = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->SetConnectStrategy(device, strategy);
    }
    EXPECT_TRUE(ret == BT_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_006 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetConnectStrategy_007
 * @tc.name: SetConnectStrategy
 * @tc.desc: CONNECTION_UNKNOWN, return BT_ERR_INTERNAL_ERROR
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetConnectStrategy_007, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_007 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);

    int strategy = static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
    int ret = BT_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->SetConnectStrategy(device, strategy);
    }
    EXPECT_TRUE(ret == BT_ERR_INTERNAL_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_007 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetConnectStrategy_008
 * @tc.name: SetConnectStrategy
 * @tc.desc: 
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetConnectStrategy_008, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_008 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
        int currentDeviceType = remoteDevice->GetHwRemoteDeviceType();
        remoteDevice->SetHwRemoteDeviceType(HW_CAR_DEVICE_TYPE);
        AdapterConfig::GetInstance()->Load();
        ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
        AdapterManager::GetInstance()->CreateAdapters();
        int ret = a2dpService->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
        EXPECT_TRUE(ret == BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_SetConnectStrategy_008 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetCodecStatus_001
 * @tc.name: GetCodecStatus
 * @tc.desc: device is not found in list
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetCodecStatus_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetCodecStatus_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:00:00:00:00:00");
        a2dpService->GetCodecStatus(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetCodecStatus_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetCodecStatus_002
 * @tc.name: GetCodecStatus
 * @tc.desc: device is found in list
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetCodecStatus_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetCodecStatus_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }
        a2dpService->GetCodecStatus(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetCodecStatus_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetDeviceFromList_001
 * @tc.name: GetDeviceFromList
 * @tc.desc: Can't find the statemachine
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetDeviceFromList_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetDeviceFromList_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:00:00:00:00:00");
        info = a2dpService->GetDeviceFromList(device);
    }
    EXPECT_TRUE(info == nullptr);
    HILOGI("A2dpServiceTest_UnitTest_GetDeviceFromList_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetDeviceFromList_002
 * @tc.name: GetDeviceFromList
 * @tc.desc: Can't find the statemachine
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetDeviceFromList_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetDeviceFromList_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        info = a2dpService->GetDeviceFromList(device);
    }
    EXPECT_TRUE(info != nullptr);
    HILOGI("A2dpServiceTest_UnitTest_GetDeviceFromList_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_ProcessEvent_001
 * @tc.name: ProcessEvent
 * @tc.desc:
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ProcessEvent_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ProcessEvent_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        btav_source_interface_t interface;
        interface.connect = ConnectMock;
        interface.disconnect = DisconnectMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;

        utility::Message event(A2DP_CONNECT_EVT);
        a2dpService->ProcessEvent(event, device);

        utility::Message event1(A2DP_DISCONNECT_EVT);
        a2dpService->ProcessEvent(event1, device);

        utility::Message event2(A2DP_INVALID_EVT);
        a2dpService->ProcessEvent(event2, device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_ProcessEvent_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_CheckDisable_001
 * @tc.name: CheckDisable
 * @tc.desc:
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_CheckDisable_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_CheckDisable_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->CheckDisable();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_CheckDisable_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_prepareCodecPreferences_001
 * @tc.name: prepareCodecPreferences
 * @tc.desc:
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_prepareCodecPreferences_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_prepareCodecPreferences_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        AdapterConfig::GetInstance()->Load();
        a2dpService->prepareCodecPreferences();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_prepareCodecPreferences_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_PreferenceL2hcStCodec_001
 * @tc.name: PreferenceL2hcStCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_PreferenceL2hcStCodec_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_PreferenceL2hcStCodec_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    std::vector<btav_a2dp_codec_config_t> codec_preferences;
    if (a2dpService != nullptr) {
        PreferenceL2hcStCodec(codec_preferences);
        for (auto item = codec_preferences.begin(); item != codec_preferences.end(); item++) {
            if (item->codec_type == BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST) {
                HILOGI("A2dpServiceTest_UnitTest_PreferenceL2hcStCodec_001 success!");
            }
        }
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_PreferenceL2hcStCodec_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_connection_state_callback_001
 * @tc.name: bta2dp_connection_state_callback
 * @tc.desc: callback will be triggered by init operation for simple,
 *           state == BTAV_CONNECTION_STATE_CONNECTING, enter if branch
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_connection_state_callback_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_connection_state_callback_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_connection_state_callback_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_connection_state_callback_002
 * @tc.name: bta2dp_connection_state_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           state == BTAV_CONNECTION_STATE_CONNECTING, enter else branch
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_connection_state_callback_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_connection_state_callback_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = GetProfileInterfaceMockForCallback2;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);
        // delete a2dpService EnableService() to avoid cfi check
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_connection_state_callback_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_connection_state_callback_004
 * @tc.name: bta2dp_connection_state_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null branch, connectState is DISCONNECTED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_connection_state_callback_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_connection_state_callback_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback4;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_connection_state_callback_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_connection_state_callback_005
 * @tc.name: bta2dp_connection_state_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null branch, connectState is CONNECTING
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_connection_state_callback_005, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_connection_state_callback_005 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback4;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_connection_state_callback_005 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_state_callback_001
 * @tc.name: bta2dp_audio_state_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is null
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_state_callback_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_state_callback_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback5;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_state_callback_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_state_callback_002
 * @tc.name: bta2dp_audio_state_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, state is BTAV_AUDIO_STATE_STARTED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_state_callback_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_state_callback_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback5;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_state_callback_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_state_callback_003
 * @tc.name: bta2dp_audio_state_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, state is not BTAV_AUDIO_STATE_STARTED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_state_callback_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_state_callback_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback6;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_state_callback_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_state_callback_004
 * @tc.name: bta2dp_audio_state_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, state is BTAV_AUDIO_STATE_LATENCY_CHANGED
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_state_callback_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_state_callback_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback13;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_state_callback_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_config_callback_002
 * @tc.name: bta2dp_audio_config_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, wrong codec info(priority is always return true, codec type return false)
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_config_callback_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback7;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_config_callback_003
 * @tc.name: bta2dp_audio_config_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, wrong codec info(priority & codec type return true, sampleRate return false)
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_config_callback_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback8;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_config_callback_004
 * @tc.name: bta2dp_audio_config_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null,
 *           wrong codec info(priority & codectype & sampleRate return true, bitsper return false)
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_config_callback_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback9;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_config_callback_005
 * @tc.name: bta2dp_audio_config_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null,
 *           wrong codec info(priority & codectype & sampleRate & bitsper return true, channel mode return false)
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_config_callback_005, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_005 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback10;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_005 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_config_callback_006
 * @tc.name: bta2dp_audio_config_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, correct codec info
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_config_callback_006, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_006 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback11;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_006 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_config_callback_007
 * @tc.name: bta2dp_audio_config_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, correct codec info
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_config_callback_007, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_007 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback12;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_007 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_config_callback_008
 * @tc.name: bta2dp_audio_config_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, correct codec info
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_config_callback_008, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_008 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback14;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_008 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_audio_config_callback_009
 * @tc.name: bta2dp_audio_config_callback
 * @tc.desc: callback will be triggered by init operation for simple
 *           deviceInfo is not null, correct ldac codec info
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_audio_config_callback_009, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_009 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        bt_interface_t *btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
        btInterface->get_profile_interface = NewGetProfileInterfaceMockForCallback15;
        AdapterManager::GetInstance()->setBluetoothInterface(btInterface);

        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_audio_config_callback_009 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_get_connect_state_001
 * @tc.name: GetConnectState
 * @tc.desc: no device connection for a2dp, then GetConnectState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_get_connect_state_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        a2dpService->DeleteDeviceFromList(device);

        int connectState = a2dpService->GetConnectState();
        EXPECT_EQ(connectState, 0);
    }
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_get_connect_state_002
 * @tc.name: GetConnectState
 * @tc.desc: second value is nullptr, then GetConnectState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_get_connect_state_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), nullptr);

        int connectState = a2dpService->GetConnectState();
        EXPECT_EQ(connectState, PROFILE_STATE_DISCONNECTED);
    }
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_get_connect_state_003
 * @tc.name: GetConnectState
 * @tc.desc: the a2dp connection of the device is CONNECTED, then GetConnectState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_get_connect_state_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }


        int connectState = a2dpService->GetConnectState();
        EXPECT_EQ(connectState, PROFILE_STATE_CONNECTED);
    }
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_get_connect_state_004
 * @tc.name: GetConnectState
 * @tc.desc: the a2dp connection of the device is CONNECTING, then GetConnectState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_get_connect_state_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }


        int connectState = a2dpService->GetConnectState();
        EXPECT_EQ(connectState, PROFILE_STATE_CONNECTING);
    }
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_get_connect_state_005
 * @tc.name: GetConnectState
 * @tc.desc: the a2dp connection of the device is DISCONNECTING, then GetConnectState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_get_connect_state_005, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_005 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }


        int connectState = a2dpService->GetConnectState();
        EXPECT_EQ(connectState, PROFILE_STATE_DISCONNECTING);
    }
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_005 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_get_connect_state_006
 * @tc.name: GetConnectState
 * @tc.desc: the a2dp connection of the device is DISCONNECTED, then GetConnectState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_get_connect_state_006, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_006 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        int connectState = a2dpService->GetConnectState();
        EXPECT_EQ(connectState, PROFILE_STATE_DISCONNECTED);
    }
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_006 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_get_connect_state_007
 * @tc.name: GetConnectState
 * @tc.desc: the a2dp connection of the device is CONNECTING, then GetConnectState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_get_connect_state_007, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_007 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }
        int connectState = a2dpService->GetConnectState();
        EXPECT_EQ(connectState, PROFILE_STATE_CONNECTED);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo1 = a2dpService->GetDeviceFromList(device);
        deviceInfo1->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
        connectState = a2dpService->GetConnectState();
        EXPECT_EQ(connectState, PROFILE_STATE_CONNECTED);
    }
    HILOGI("A2dpServiceTest_UnitTest_get_connect_state_007 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetCodecPreference_001
 * @tc.name: GetCodecPreference
 * @tc.desc: A2dpServiceTest_UnitTest_GetCodecPreference_001
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetCodecPreference_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetCodecPreference_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        A2dpSrcCodecInfo a2dpSrcCodecInfo;
        int ret = a2dpService->GetCodecPreference(device, a2dpSrcCodecInfo);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_INVALID_PARAM);
    }
    HILOGI("A2dpServiceTest_UnitTest_GetCodecPreference_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetCodecPreference_002
 * @tc.name: GetCodecPreference
 * @tc.desc: A2dpServiceTest_UnitTest_GetCodecPreference_002
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetCodecPreference_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetCodecPreference_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        A2dpSrcCodecInfo a2dpSrcCodecInfo;
        int ret = a2dpService->GetCodecPreference(device, a2dpSrcCodecInfo);
        EXPECT_EQ(ret, Bluetooth::BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_GetCodecPreference_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetCodecPreference_001
 * @tc.name: GetCodecPreference
 * @tc.desc: A2dpServiceTest_UnitTest_SetCodecPreference_001
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetCodecPreference_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetCodecPreference_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        A2dpSrcCodecInfo a2dpSrcCodecInfo;
        int ret = a2dpService->SetCodecPreference(device, a2dpSrcCodecInfo);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_INVALID_STATE);
    }
    HILOGI("A2dpServiceTest_UnitTest_SetCodecPreference_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetCodecPreference_002
 * @tc.name: GetCodecPreference
 * @tc.desc: A2dpServiceTest_UnitTest_SetCodecPreference_002
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetCodecPreference_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetCodecPreference_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        std::shared_ptr<A2dpDeviceInfo> deviceInfo1 = a2dpService->GetDeviceFromList(device);
        A2dpSrcCodecStatus codecStatus;
        A2dpSrcCodecInfo a2dpSrcCodecInfo;
        codecStatus.codecInfo = a2dpSrcCodecInfo;
        deviceInfo1->SetCodecStatus(codecStatus);
        int ret = a2dpService->SetCodecPreference(device, a2dpSrcCodecInfo);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_INVALID_PARAM);
    }
    HILOGI("A2dpServiceTest_UnitTest_SetCodecPreference_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetCodecPreference_003
 * @tc.name: GetCodecPreference
 * @tc.desc: A2dpServiceTest_UnitTest_SetCodecPreference_003
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetCodecPreference_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetCodecPreference_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        std::shared_ptr<A2dpDeviceInfo> deviceInfo1 = a2dpService->GetDeviceFromList(device);
        A2dpSrcCodecInfo a2dpSrcCodecInfo;
        int ret = a2dpService->SetCodecPreference(device, a2dpSrcCodecInfo);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_INVALID_PARAM);
    }
    HILOGI("A2dpServiceTest_UnitTest_SetCodecPreference_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetCodecPreference_004
 * @tc.name: SetCodecPreference codec switch
 * @tc.desc: A2dpServiceTest_UnitTest_SetCodecPreference_004
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetCodecPreference_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetCodecPreference_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));

        A2dpSrcCodecStatus codecStatus;
        A2dpSrcCodecInfo a2dpSrcCodecInfo;
        a2dpSrcCodecInfo.codecType = 10;
        a2dpSrcCodecInfo.sampleRate = 2;
        a2dpSrcCodecInfo.bitsPerSample = 2;
        a2dpSrcCodecInfo.channelMode = 2;
        codecStatus.codecInfo = a2dpSrcCodecInfo;
        codecStatus.codecInfoConfirmedCap.push_back(a2dpSrcCodecInfo);

        std::shared_ptr<A2dpDeviceInfo> deviceInfo1 = a2dpService->GetDeviceFromList(device);
        deviceInfo1->SetCodecStatus(codecStatus);
        a2dpSrcCodecInfo.codecType = 10;
        int ret = a2dpService->SetCodecPreference(device, a2dpSrcCodecInfo);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_INVALID_PARAM);
    }
    HILOGI("A2dpServiceTest_UnitTest_SetCodecPreference_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_OffloadStartPlaying_001
 * @tc.name: OffloadStartPlaying
 * @tc.desc: OffloadStartPlaying
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OffloadStartPlaying_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<int> sessionsId;
        int ret = a2dpService->OffloadStartPlaying(device, sessionsId);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_INVALID_PARAM);
    }
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_OffloadStartPlaying_002
 * @tc.name: OffloadStartPlaying
 * @tc.desc: OffloadStartPlaying
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OffloadStartPlaying_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<int> sessionsId;
        sessionsId.push_back(1);
        int ret = a2dpService->OffloadStartPlaying(device, sessionsId);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_DEVICE_DISCONNECTED);
    }
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_OffloadStartPlaying_003
 * @tc.name: OffloadStartPlaying
 * @tc.desc: OffloadStartPlaying
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OffloadStartPlaying_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<int> sessionsId;
        sessionsId.push_back(1);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        int ret = a2dpService->OffloadStartPlaying(device, sessionsId);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_INVALID_STATE);
    }
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_OffloadStartPlaying_004
 * @tc.name: OffloadStartPlaying
 * @tc.desc: OffloadStartPlaying
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OffloadStartPlaying_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 1;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        a2dpService->a2dpStreamSessions_.clear();
        a2dpService->A2dpOffloadAddRequestSessions(streamsInfo);
        std::vector<int> sessionsId;
        sessionsId.push_back(1);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        deviceInfo->currentConnectState_ = static_cast<int>(BTConnectState::CONNECTED);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        int ret = a2dpService->OffloadStartPlaying(device, sessionsId);
        EXPECT_EQ(ret, Bluetooth::BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_OffloadStopPlaying_001
 * @tc.name: OffloadStopPlaying
 * @tc.desc: OffloadStopPlaying
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OffloadStopPlaying_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OffloadStopPlaying_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<int> sessionsId;
        int ret = a2dpService->OffloadStopPlaying(device, sessionsId);
        EXPECT_EQ(ret, Bluetooth::BT_ERR_INVALID_PARAM);
    }
    HILOGI("A2dpServiceTest_UnitTest_OffloadStopPlaying_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_OffloadStopPlaying_002
 * @tc.name: OffloadStopPlaying
 * @tc.desc: OffloadStopPlaying
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OffloadStopPlaying_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OffloadStopPlaying_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<int> sessionsId;
        sessionsId.push_back(1);
        int ret = a2dpService->OffloadStopPlaying(device, sessionsId);
        EXPECT_EQ(ret, Bluetooth::BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_OffloadStopPlaying_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_OffloadStopPlaying_003
 * @tc.name: OffloadStopPlaying
 * @tc.desc: OffloadStopPlaying
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OffloadStopPlaying_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OffloadStartPlaying_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<int> sessionsId;
        sessionsId.push_back(1);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        int ret = a2dpService->OffloadStopPlaying(device, sessionsId);
        EXPECT_EQ(ret, Bluetooth::BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_OffloadStopPlaying_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_OffloadStopPlaying_004
 * @tc.name: OffloadStopPlaying
 * @tc.desc: OffloadStopPlaying
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OffloadStopPlaying_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OffloadStopPlaying_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<int> sessionsId;
        sessionsId.push_back(1);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->currentConnectState_ = static_cast<int>(BTConnectState::CONNECTED);
        int ret = a2dpService->OffloadStopPlaying(device, sessionsId);
        EXPECT_EQ(ret, Bluetooth::BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_OffloadStopPlaying_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_001
 * @tc.name: A2dpOffloadSessionPathRequest
 * @tc.desc: A2dpOffloadSessionPathRequest
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        SetOffloadEncodePathValue(1);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 1);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_002
 * @tc.name: A2dpOffloadSessionPathRequest
 * @tc.desc: A2dpOffloadSessionPathRequest
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 1;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
        HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
        systemInterface.SetCallState(HFP_AG_CALL_STATE_IDLE);
        a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        systemInterface.SetCallState(HFP_AG_CALL_STATE_INCOMING);
        SetOffloadEncodePathValue(1);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 1);
        SetOffloadEncodePathValue(2);
        ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 2);
        a2dpService->SetCurrentDeviceMute();
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_003
 * @tc.name: A2dpOffloadSessionPathRequest
 * @tc.desc: A2dpOffloadSessionPathRequest
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 100000;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
        HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
        systemInterface.SetCallState(HFP_AG_CALL_STATE_IDLE);
        SetOffloadEncodePathValue(1);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 1);
        a2dpService->NotifyWearDetectionActionAfterConnected(device);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_004
 * @tc.name: A2dpOffloadSessionPathRequest
 * @tc.desc: A2dpOffloadSessionPathRequest
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 100000;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
        HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
        systemInterface.SetCallState(HFP_AG_CALL_STATE_IDLE);
        SetOffloadEncodePathValue(1);
        a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        SetOffloadEncodePathValue(2);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 2);
        a2dpService->NotifyProfileState(device);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_005
 * @tc.name: A2dpOffloadSessionPathRequest
 * @tc.desc: A2dpOffloadSessionPathRequest
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_005, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_005 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("22:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 100000;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 1);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_005 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_006
 * @tc.name: A2dpOffloadSessionPathRequest
 * @tc.desc: A2dpOffloadSessionPathRequest
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_006, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_006 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 100000;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
        HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
        systemInterface.SetCallState(HFP_AG_CALL_STATE_ACTIVE);
        SetOffloadEncodePathValue(1);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 1);
        SetOffloadEncodePathValue(2);
        ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 2);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_006 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_007
 * @tc.name: A2dpOffloadSessionPathRequest
 * @tc.desc: A2dpOffloadSessionPathRequest
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_007, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_007 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 100000;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
        HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
        systemInterface.SetCallState(HFP_AG_CALL_STATE_IDLE);
        SetOffloadEncodePathValue(1);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 1);
        int offloadFlag = BtChrBusinessEvent::GetInstance().GetOffloadFlag();
        EXPECT_EQ(offloadFlag, 0);
        SetOffloadEncodePathValue(2);
        ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        offloadFlag = BtChrBusinessEvent::GetInstance().GetOffloadFlag();
        EXPECT_EQ(ret, 2);
        EXPECT_EQ(offloadFlag, 1);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_007 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetA2dpOffloadCodecStatus_001
 * @tc.name: GetA2dpOffloadCodecStatus
 * @tc.desc: GetA2dpOffloadCodecStatus
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetA2dpOffloadCodecStatus_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetA2dpOffloadCodecStatus_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        A2dpSrcOffloadCodecStatus offloadCodecStatus;
        A2dpSrcOffloadCodecStatus ret = a2dpService->GetA2dpOffloadCodecStatus(device);
        EXPECT_EQ(ret.codecInfo.mtu, offloadCodecStatus.codecInfo.mtu);
    }
    HILOGI("A2dpServiceTest_UnitTest_GetA2dpOffloadCodecStatus_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetA2dpOffloadCodecStatus_002
 * @tc.name: GetA2dpOffloadCodecStatus
 * @tc.desc: GetA2dpOffloadCodecStatus
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetA2dpOffloadCodecStatus_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetA2dpOffloadCodecStatus_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        A2dpSrcOffloadCodecStatus offloadCodecStatus;
        A2dpSrcOffloadCodecStatus ret = a2dpService->GetA2dpOffloadCodecStatus(device);
        offloadCodecStatus.codecInfo.mediaPacketHeader = 0x10;
        offloadCodecStatus.codecInfo.mtu = 1000;
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetOffloadCodecStatus(offloadCodecStatus);
        ret = a2dpService->GetA2dpOffloadCodecStatus(device);
        EXPECT_EQ(ret.codecInfo.mediaPacketHeader, 0x10);
        EXPECT_EQ(ret.codecInfo.mtu, 1000);
    }
    HILOGI("A2dpServiceTest_UnitTest_GetA2dpOffloadCodecStatus_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadAddRequestSessions_001
 * @tc.name: A2dpOffloadAddRequestSessions
 * @tc.desc: A2dpOffloadAddRequestSessions
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadAddRequestSessions_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadAddRequestSessions_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 1;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        a2dpService->a2dpStreamSessions_.clear();
        a2dpService->A2dpOffloadAddRequestSessions(streamsInfo);
        EXPECT_EQ(a2dpService->a2dpStreamSessions_.size(), 1);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadAddRequestSessions_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadAddRequestSessions_002
 * @tc.name: A2dpOffloadAddRequestSessions
 * @tc.desc: A2dpOffloadAddRequestSessions
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadAddRequestSessions_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadAddRequestSessions_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 2;
        stream.streamType = 15;
        streamsInfo.push_back(stream);
        a2dpService->a2dpStreamSessions_.clear();
        a2dpService->A2dpOffloadAddRequestSessions(streamsInfo);
        EXPECT_EQ(a2dpService->a2dpStreamSessions_.size(), 1);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadAddRequestSessions_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadClearNotActiveSessions_001
 * @tc.name: A2dpOffloadClearNotActiveSessions
 * @tc.desc: A2dpOffloadClearNotActiveSessions
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadClearNotActiveSessions_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadClearNotActiveSessions_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 1;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        stream.sessionId = 2;
        stream.streamType = 15;
        streamsInfo.push_back(stream);
        a2dpService->a2dpStreamSessions_.clear();
        a2dpService->A2dpOffloadAddRequestSessions(streamsInfo);
        EXPECT_EQ(a2dpService->a2dpStreamSessions_.size(), 2);
        std::vector<A2dpSrcStreamInfo> streamsInfo2;
        stream.sessionId = 1;
        stream.streamType = 1;
        streamsInfo2.push_back(stream);
        a2dpService->A2dpOffloadClearNotActiveSessions(streamsInfo2);
        EXPECT_EQ(a2dpService->a2dpStreamSessions_.size(), 1);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadClearNotActiveSessions_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadUpdateSessionsState_001
 * @tc.name: A2dpOffloadUpdateSessionsState
 * @tc.desc: A2dpOffloadUpdateSessionsState
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadUpdateSessionsState_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadUpdateSessionsState_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 1;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        stream.sessionId = 2;
        stream.streamType = 15;
        streamsInfo.push_back(stream);
        a2dpService->a2dpStreamSessions_.clear();
        a2dpService->A2dpOffloadAddRequestSessions(streamsInfo);
        EXPECT_EQ(a2dpService->a2dpStreamSessions_.size(), 2);
        std::vector<int> sessionsId;
        a2dpService->A2dpOffloadUpdateSessionsState(sessionsId, true);
        bool isPlaying = false;
        for (auto streamInfo : a2dpService->a2dpStreamSessions_) {
            if (streamInfo.isPlaying) {
                isPlaying = true;
                break;
            }
        }
        EXPECT_EQ(isPlaying, false);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadUpdateSessionsState_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadUpdateSessionsState_002
 * @tc.name: A2dpOffloadUpdateSessionsState
 * @tc.desc: A2dpOffloadUpdateSessionsState
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadUpdateSessionsState_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadUpdateSessionsState_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 1;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        stream.sessionId = 2;
        stream.streamType = 15;
        streamsInfo.push_back(stream);
        a2dpService->a2dpStreamSessions_.clear();
        a2dpService->A2dpOffloadAddRequestSessions(streamsInfo);
        EXPECT_EQ(a2dpService->a2dpStreamSessions_.size(), 2);
        std::vector<int> sessionsId;
        bool isPlaying = false;
        sessionsId.push_back(1);
        sessionsId.push_back(2);
        a2dpService->A2dpOffloadUpdateSessionsState(sessionsId, true);
        for (auto streamInfo : a2dpService->a2dpStreamSessions_) {
            if (streamInfo.isPlaying) {
                isPlaying = true;
                break;
            }
        }
        isPlaying = false;
        a2dpService->A2dpOffloadUpdateSessionsState(sessionsId, false);
        for (auto streamInfo : a2dpService->a2dpStreamSessions_) {
            if (streamInfo.isPlaying) {
                isPlaying = true;
                break;
            }
        }
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadUpdateSessionsState_002 end");
}

HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadHandleDelayStop_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadHandleDelayStop_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        a2dpService->A2dpOffloadHandleDelayStop(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadHandleDelayStop_001 end");
}

HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadHandleDelayStop_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadHandleDelayStop_002 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    if (a2dpService) {
        a2dpService->A2dpOffloadHandleDelayStop(device);
        EXPECT_TRUE(a2dpService->GetDeviceFromList(device) == nullptr);
        EXPECT_TRUE(a2dpService->A2dpOffloadIsSessionOnPlaying() == false);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadHandleDelayStop_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionsConcurrent_001
 * @tc.name: A2dpOffloadSessionsConcurrent
 * @tc.desc: concurrent access a2dpStreamSessions_ protected by g_a2dpServiceMutex
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionsConcurrent_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionsConcurrent_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    EXPECT_TRUE(a2dpService != nullptr);
    if (a2dpService == nullptr) {
        HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionsConcurrent_001 end");
        return;
    }
    RawAddress device = RawAddress("11:22:33:44:55:66");
    a2dpService->a2dpStreamSessions_.clear();
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);

    constexpr int LOOP_TIMES = 100;
    constexpr int MAX_SESSION_ID = 10;
    std::vector<int> sessionsId = {1};

    std::thread addThread([a2dpService]() {
        for (int i = 0; i < LOOP_TIMES; ++i) {
            A2dpSrcStreamInfo stream = {};
            stream.sessionId = i % MAX_SESSION_ID + 1;
            stream.streamType = 1;
            std::vector<A2dpSrcStreamInfo> streamsInfo;
            streamsInfo.push_back(stream);
            a2dpService->A2dpOffloadAddRequestSessions(streamsInfo);
        }
    });
    std::thread updateThread([a2dpService, sessionsId]() {
        for (int i = 0; i < LOOP_TIMES; ++i) {
            a2dpService->A2dpOffloadUpdateSessionsState(sessionsId, i % 2 == 0);
        }
    });
    std::thread clearThread([a2dpService, sessionsId]() {
        for (int i = 0; i < LOOP_TIMES; ++i) {
            A2dpSrcStreamInfo stream = {};
            stream.sessionId = sessionsId.front();
            stream.streamType = 1;
            std::vector<A2dpSrcStreamInfo> streamsInfo;
            streamsInfo.push_back(stream);
            a2dpService->A2dpOffloadClearNotActiveSessions(streamsInfo);
        }
    });
    std::thread queryThread([a2dpService]() {
        for (int i = 0; i < LOOP_TIMES; ++i) {
            a2dpService->A2dpOffloadIsSessionOnPlaying();
        }
    });
    std::thread startPlayingThread([a2dpService, device, sessionsId]() {
        for (int i = 0; i < LOOP_TIMES; ++i) {
            a2dpService->OffloadStartPlaying(device, sessionsId);
        }
    });
    addThread.join();
    updateThread.join();
    clearThread.join();
    queryThread.join();
    startPlayingThread.join();

    size_t sessionSize = a2dpService->a2dpStreamSessions_.size();
    EXPECT_TRUE(sessionSize <= static_cast<size_t>(MAX_SESSION_ID));
    for (auto &session : a2dpService->a2dpStreamSessions_) {
        EXPECT_TRUE(session.sessionId >= 1 && session.sessionId <= MAX_SESSION_ID);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionsConcurrent_001 end, session size:%{public}zu", sessionSize);
}

HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetNavigationMode_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetNavigationMode_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {

        a2dpService->SetNavigationMode(false);
        a2dpService->SetNavigationMode(true);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_SetNavigationMode_001 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_ForceStopOffloadPlaying_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ForceStopOffloadPlaying_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ForceStopOffloadPlaying_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
        a2dpService->ForceStopOffloadPlaying(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_ForceStopOffloadPlaying_001 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_OnServiceStatusReceived_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OnServiceStatusReceived_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OnServiceStatusReceived_001 enter");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->UpdateActiveDevice(device);
        btav_source_interface_t interface;
        interface.init = InitMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;
        AdapterManager::GetInstance()->setBluetoothInterface(NULL);
        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_OnServiceStatusReceived_001 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_RegisterHdiServiceStatusListener_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_RegisterHdiServiceStatusListener_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_RegisterHdiServiceStatusListener_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        btav_source_interface_t interface;
        interface.init = InitMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;

        AdapterManager::GetInstance()->setBluetoothInterface(NULL);
        a2dpService->EnableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_RegisterHdiServiceStatusListener_001 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_UnRegisterDeviceStatusListener_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_UnRegisterDeviceStatusListener_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_UnRegisterDeviceStatusListener_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }

        btav_source_interface_t interface;
        interface.cleanup = CleanupMock;
        interface.set_active_device = SetActiveDeviceMock;
        a2dpService->sBluetoothA2dpSrcInterface = &interface;
        a2dpService->DisableService();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_UnRegisterDeviceStatusListener_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Enable_001
 * @tc.name: Enable
 * @tc.desc: Enable
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Enable_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Enable_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->Enable();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_Enable_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_WriteFrame_001
 * @tc.name: WriteFrame
 * @tc.desc: WriteFrame
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_WriteFrame_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_WriteFrame_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        uint8_t data;
        a2dpService->WriteFrame(&data, 1);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_WriteFrame_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_DeregisterObserver_001
 * @tc.name: DeregisterObserver
 * @tc.desc: DeregisterObserver
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_DeregisterObserver_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_DeregisterObserver_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        IA2dpObserver *observerPtr = nullptr;
        a2dpService->DeregisterObserver(observerPtr);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_DeregisterObserver_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_RegisterObserver_001
 * @tc.name: RegisterObserver
 * @tc.desc: RegisterObserver
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_RegisterObserver_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_RegisterObserver_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        IA2dpObserver *observerPtr = nullptr;
        a2dpService->RegisterObserver(observerPtr);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_RegisterObserver_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_StopPlaying_001
 * @tc.name: StopPlaying
 * @tc.desc: StopPlaying
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_StopPlaying_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_StopPlaying_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        a2dpService->StopPlaying(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_StopPlaying_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SuspendPlaying_001
 * @tc.name: SuspendPlaying
 * @tc.desc: SuspendPlaying
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SuspendPlaying_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SuspendPlaying_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        a2dpService->SuspendPlaying(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_SuspendPlaying_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_StartPlaying_001
 * @tc.name: StartPlaying
 * @tc.desc: StartPlaying
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_StartPlaying_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_StartPlaying_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        a2dpService->StartPlaying(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_StartPlaying_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetOptionalCodecsSupportState_001
 * @tc.name: GetOptionalCodecsSupportState
 * @tc.desc: GetOptionalCodecsSupportState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetOptionalCodecsSupportState_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetOptionalCodecsSupportState_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        a2dpService->GetOptionalCodecsSupportState(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetOptionalCodecsSupportState_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SwitchOptionalCodecs_001
 * @tc.name: SwitchOptionalCodecs
 * @tc.desc: SwitchOptionalCodecs
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SwitchOptionalCodecs_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SwitchOptionalCodecs_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        a2dpService->SwitchOptionalCodecs(device, true);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_SwitchOptionalCodecs_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_IsConfirmCodecInfo_001
 * @tc.name: IsConfirmCodecInfo
 * @tc.desc: IsConfirmCodecInfo
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_IsConfirmCodecInfo_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_IsConfirmCodecInfo_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        A2dpSrcCodecStatus codecStatus;
        A2dpSrcCodecInfo a2dpSrcCodecInfo;
        codecStatus.codecInfo = a2dpSrcCodecInfo;
        a2dpService->IsConfirmCodecInfo(codecStatus, a2dpSrcCodecInfo);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_IsConfirmCodecInfo_001 end");
}

HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_IsConfirmCodecInfo_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_IsConfirmCodecInfo_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        A2dpSrcCodecStatus codecStatus;
        A2dpSrcCodecInfo a2dpSrcCodecInfo;
        a2dpSrcCodecInfo.codecType = 10;
        a2dpSrcCodecInfo.sampleRate = 2;
        codecStatus.codecInfo = a2dpSrcCodecInfo;
        codecStatus.codecInfoConfirmedCap.push_back(a2dpSrcCodecInfo);
        EXPECT_TRUE(a2dpService->IsConfirmCodecInfo(codecStatus, a2dpSrcCodecInfo));
    }
    HILOGI("A2dpServiceTest_UnitTest_IsConfirmCodecInfo_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SendDelay_001
 * @tc.name: SendDelay
 * @tc.desc: SendDelay
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SendDelay_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SendDelay_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        uint16_t delayValue = 10;
        a2dpService->SendDelay(device, delayValue);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_SendDelay_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetPlayingState_001
 * @tc.name: GetPlayingState
 * @tc.desc: GetPlayingState
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetPlayingState_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetPlayingState_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        int state = 1;
        a2dpService->GetPlayingState(device, state);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetPlayingState_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_NotifyMediaStackChanged_001
 * @tc.name: NotifyMediaStackChanged
 * @tc.desc: NotifyMediaStackChanged
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_NotifyMediaStackChanged_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_NotifyMediaStackChanged_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        a2dpService->NotifyMediaStackChanged(1, device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_NotifyMediaStackChanged_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_BluetoothNotifyMediaStackChanged_001
 * @tc.name: BluetoothNotifyMediaStackChanged
 * @tc.desc: BluetoothNotifyMediaStackChanged
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_BluetoothNotifyMediaStackChanged_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_BluetoothNotifyMediaStackChanged_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        a2dpService->BluetoothNotifyMediaStackChanged(1, device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_BluetoothNotifyMediaStackChanged_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_IsActiveDeviceInPlayToPauseDenyList_001
 * @tc.name: IsActiveDeviceInPlayToPauseDenyList
 * @tc.desc: IsActiveDeviceInPlayToPauseDenyList
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_IsActiveDeviceInPlayToPauseDenyList_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_IsActiveDeviceInPlayToPauseDenyList_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->IsActiveDeviceInPlayToPauseDenyList();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_IsActiveDeviceInPlayToPauseDenyList_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Disable_001
 * @tc.name: Disable
 * @tc.desc: Disable
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Disable_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Disable_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->Disable();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_Disable_001 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_EnableAutoPlay_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_EnableAutoPlay_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_EnableAutoPlay_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        int ret = a2dpService->EnableAutoPlay(device);
        EXPECT_EQ(ret, BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_EnableAutoPlay_001 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_DisableAutoPlay_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_DisableAutoPlay_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_DisableAutoPlay_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        int duration = -1;
        int res = a2dpService->DisableAutoPlay(device, duration);
        EXPECT_EQ(res, BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_DisableAutoPlay_001 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_DisableAutoPlay_002
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_DisableAutoPlay_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_DisableAutoPlay_002 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        int duration = 3000;
        int res = a2dpService->DisableAutoPlay(device, duration);
        EXPECT_EQ(res, BT_NO_ERROR);
    }
    HILOGI("A2dpServiceTest_UnitTest_DisableAutoPlay_002 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_GetAutoPlayDisabledDuration_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetAutoPlayDisabledDuration_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetAutoPlayDisabledDuration_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("A4:3B:0E:7B:D3:36");
        int duration;
        int res = a2dpService->GetAutoPlayDisabledDuration(device, duration);
        EXPECT_EQ(res, BT_NO_ERROR);
        EXPECT_EQ(duration, -1);
    }
    HILOGI("A2dpServiceTest_UnitTest_GetAutoPlayDisabledDuration_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_RegisterObserver_002
 * @tc.name: RegisterObserver
 * @tc.desc: RegisterObserver
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_RegisterObserver_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_RegisterObserver_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        IA2dpObserver *observerPtr = nullptr;
        a2dpService->RegisterObserver(observerPtr);
    }
    if (a2dpService != nullptr) {
        IA2dpObserver *observerPtr = nullptr;
        a2dpService->DeregisterObserver(observerPtr);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_RegisterObserver_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_DeregisterObserver_002
 * @tc.name: DeregisterObserver
 * @tc.desc: DeregisterObserver
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_DeregisterObserver_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_DeregisterObserver_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        IA2dpObserver *observerPtr = nullptr;
        a2dpService->DeregisterObserver(observerPtr);
    }
    if (a2dpService != nullptr) {
        IA2dpObserver *observerPtr = nullptr;
        a2dpService->RegisterObserver(observerPtr);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_DeregisterObserver_002 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_OnRenderSceneChange_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OnRenderSceneChange_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OnRenderSceneChange_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    bool anyRenderRunning = false;
    bool gameRunning = false;
    bool musicRunning = false;
    if (a2dpService != nullptr) {
        a2dpService->OnRenderSceneChange(anyRenderRunning, gameRunning, musicRunning);
        EXPECT_EQ(a2dpService->codecChangeScene_, CodecChangeAppScene::APP_TYPE_OTHER);

        anyRenderRunning = true;
        musicRunning = true;
        a2dpService->OnRenderSceneChange(anyRenderRunning, gameRunning, musicRunning);
        EXPECT_EQ(a2dpService->codecChangeScene_, CodecChangeAppScene::APP_TYPE_MUSIC);

        // scene no change and notify again to test the log
        a2dpService->OnRenderSceneChange(anyRenderRunning, gameRunning, musicRunning);
        EXPECT_EQ(a2dpService->codecChangeScene_, CodecChangeAppScene::APP_TYPE_MUSIC);

        anyRenderRunning = true;
        gameRunning = true;
        a2dpService->OnRenderSceneChange(anyRenderRunning, gameRunning, musicRunning);
        EXPECT_EQ(a2dpService->codecChangeScene_, CodecChangeAppScene::APP_TYPE_GAME);

        anyRenderRunning = true;
        gameRunning = false;
        musicRunning = false;
        a2dpService->OnRenderSceneChange(anyRenderRunning, gameRunning, musicRunning);
        EXPECT_EQ(a2dpService->codecChangeScene_, CodecChangeAppScene::APP_TYPE_OTHER);
    }
    HILOGI("A2dpServiceTest_UnitTest_OnRenderSceneChange_001 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_OnRenderSceneChange_002
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OnRenderSceneChange_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OnRenderSceneChange_002 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    bool anyRenderRunning = false;
    bool gameRunning = false;
    bool musicRunning = false;
    if (a2dpService != nullptr) {

        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        A2dpSrcCodecStatus codecStatus;
        codecStatus.codecInfo.codecType = A2DP_CODEC_TYPE_L2HCV2_USER;
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        a2dpService->UpdateActiveDevice(device);

        anyRenderRunning = true;
        gameRunning = true;
        a2dpService->OnRenderSceneChange(anyRenderRunning, gameRunning, musicRunning);
        EXPECT_EQ(a2dpService->codecChangeScene_, CodecChangeAppScene::APP_TYPE_GAME);

        anyRenderRunning = true;
        gameRunning = false;
        a2dpService->OnRenderSceneChange(anyRenderRunning, gameRunning, musicRunning);
        EXPECT_EQ(a2dpService->codecChangeScene_, CodecChangeAppScene::APP_TYPE_OTHER);

        std::shared_ptr<A2dpDeviceInfo> deviceInfo1 = a2dpService->GetDeviceFromList(device);
        codecStatus.codecInfo.codecType = A2DP_CODEC_TYPE_LDAC_USER;
        deviceInfo1->SetCodecStatus(codecStatus);
        anyRenderRunning = true;
        gameRunning = true;
        a2dpService->OnRenderSceneChange(anyRenderRunning, gameRunning, musicRunning);
        EXPECT_EQ(a2dpService->codecChangeScene_, CodecChangeAppScene::APP_TYPE_GAME);
    }
    HILOGI("A2dpServiceTest_UnitTest_OnRenderSceneChange_002 end");
}

/**
 * @tc.name: A2dpServiceTest_UnitTest_ChangeCodecWithSceneSwitch_001
 * @tc.desc: A2dpServiceTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ChangeCodecWithSceneSwitch_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ChangeCodecWithSceneSwitch_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    if (a2dpService != nullptr) {
        SetOffloadRunning(false);
        a2dpService->codecChangeScene_ = CodecChangeAppScene::APP_TYPE_OTHER;
        a2dpService->ChangeCodecWithSceneSwitch();

        const bthwif_interface_t *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        if (bthwif != nullptr) {
            bthwif->hwSendSceneType(8); //video
            a2dpService->codecChangeScene_ = CodecChangeAppScene::APP_TYPE_OTHER;
            a2dpService->ChangeCodecWithSceneSwitch();

            bthwif->hwSendSceneType(9); //game
            a2dpService->codecChangeScene_ = CodecChangeAppScene::APP_TYPE_OTHER;
            a2dpService->ChangeCodecWithSceneSwitch();

            bthwif->hwSendSceneType(-1); //unknown type
        }

        a2dpService->codecChangeScene_ = CodecChangeAppScene::APP_TYPE_MUSIC;
        a2dpService->ChangeCodecWithSceneSwitch();

        a2dpService->codecChangeScene_ = CodecChangeAppScene::APP_TYPE_GAME;
        a2dpService->ChangeCodecWithSceneSwitch();

        SetOffloadRunning(true);
        a2dpService->ChangeCodecWithSceneSwitch();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_ChangeCodecWithSceneSwitch_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SendCommandPause_001
 * @tc.name: SendCommandPause
 * @tc.desc: the device is already active, SendCommandPause
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SendCommandPause_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SendCommandPause_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        a2dpService->OnRenderStateChange(true);
        a2dpService->SendCommandPause();
        deviceInfo->SetPlayingState(true);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        EXPECT_EQ(a2dpService->GetDeviceFromList(device), deviceInfo);
        a2dpService->UpdateActiveDevice(device);
        a2dpService->OnRenderStateChange(false);
        a2dpService->SendCommandPause();
    }
    HILOGI("A2dpServiceTest_UnitTest_SendCommandPause_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_ProcessA2dpHdfLoad_002
 * @tc.name: ProcessA2dpHdfLoad
 * @tc.desc: ProcessA2dpHdfLoad
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ProcessA2dpHdfLoad_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ProcessA2dpHdfLoad_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    if (a2dpService != nullptr) {
        int state = static_cast<int>(BTConnectState::DISCONNECTED);
        a2dpService->ProcessA2dpHdfLoad(state, device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_ProcessA2dpHdfLoad_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_ProcessA2dpHdfLoad_003
 * @tc.name: ProcessA2dpHdfLoad
 * @tc.desc: ProcessA2dpHdfLoad
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ProcessA2dpHdfLoad_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ProcessA2dpHdfLoad_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    if (a2dpService != nullptr) {
        int state = static_cast<int>(BTConnectState::DISCONNECTING);
        a2dpService->ProcessA2dpHdfLoad(state, device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_ProcessA2dpHdfLoad_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetProfile_001
 * @tc.name: GetProfile
 * @tc.desc: GetProfile
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetProfile_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetProfile_001 start");
    A2dpSrcService *a2dpSrcService = new A2dpSrcService();
    if (a2dpSrcService != nullptr) {
        a2dpSrcService->GetProfile();
    }
    EXPECT_TRUE(a2dpSrcService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetProfile_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_GetTryReconnectA2dp_001
 * @tc.name: GetTryReconnectA2dp
 * @tc.desc: the device is already active, GetTryReconnectA2dp
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetTryReconnectA2dp_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetTryReconnectA2dp_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetTryReconnectA2dp(true);
        EXPECT_TRUE(deviceInfo->GetTryReconnectA2dp());
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        EXPECT_EQ(deviceInfo->GetPreConnectState(), static_cast<int>(BTConnectState::CONNECTED));
    }
    a2dpService->postA2dpConnectEvent(0, device);
    sleep(TIME);
    EXPECT_TRUE(deviceInfo->GetTryReconnectA2dp());
    HILOGI("A2dpServiceTest_UnitTest_GetTryReconnectA2dp_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_SetActiveMode_001
 * @tc.name: HwSetActiveMode
 * @tc.desc: the device is already active, HwSetActiveMode
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetActiveMode_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetActiveMode_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        EXPECT_EQ(a2dpService->GetDeviceFromList(device), deviceInfo);
        a2dpService->UpdateActiveDevice(device);
        deviceInfo->SetPlayingState(false);
        a2dpService->HwSetActiveMode();
    }
    HILOGI("A2dpServiceTest_UnitTest_SetActiveMode_001 end");
}

HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ProcessAvrcpDynamicLoad, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ProcessAvrcpDynamicLoad start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress rawAddr = RawAddress("11:22:33:44:55:66");
    if (a2dpService != nullptr) {
        // Connecting start load avrcp
        a2dpService->a2dpLoadedDevice_.Push(rawAddr.GetAddress());
        int state = static_cast<int>(BTConnectState::CONNECTING);
        a2dpService->ProcessAvrcpDynamicLoad(state, rawAddr);
        EXPECT_EQ(a2dpService->a2dpLoadedDevice_.Size(), 1);

        // connected cannot load avrcp
        state = static_cast<int>(BTConnectState::CONNECTED);
        a2dpService->ProcessAvrcpDynamicLoad(state, rawAddr);
        EXPECT_EQ(a2dpService->a2dpLoadedDevice_.Size(), 1);

        // disconnected start unload avrcp
        a2dpService->a2dpLoadedDevice_.Clear();
        state = static_cast<int>(BTConnectState::DISCONNECTED);
        a2dpService->ProcessAvrcpDynamicLoad(state, rawAddr);
        EXPECT_EQ(a2dpService->a2dpLoadedDevice_.Size(), 0);

        a2dpService->avrcpLoadedDevice_.Push(rawAddr.GetAddress());
        state = static_cast<int>(BTConnectState::CONNECTED);
        a2dpService->ProcessAvrcpDynamicLoad(state, rawAddr, "avrcp");
        EXPECT_EQ(a2dpService->avrcpLoadedDevice_.Size(), 1);

        // connected cannot load avrcp
        state = static_cast<int>(BTConnectState::CONNECTED);
        a2dpService->ProcessAvrcpDynamicLoad(state, rawAddr, "avrcp");
        EXPECT_EQ(a2dpService->avrcpLoadedDevice_.Size(), 1);

        // disconnected start unload avrcp
        a2dpService->avrcpLoadedDevice_.Clear();
        state = static_cast<int>(BTConnectState::DISCONNECTED);
        a2dpService->ProcessAvrcpDynamicLoad(state, rawAddr, "avrcp");
        EXPECT_EQ(a2dpService->avrcpLoadedDevice_.Size(), 0);
    }
    HILOGI("A2dpServiceTest_UnitTest_ProcessAvrcpDynamicLoad end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_IsA2dpPlaying_001
 * @tc.name: IsA2dpPlaying
 * @tc.desc: IsA2dpPlaying
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_IsA2dpPlaying_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_IsA2dpPlaying_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    RawAddress device = RawAddress("11:22:33:44:55:66");
    bool res = a2dpService->IsA2dpPlaying(device);
    EXPECT_FALSE(res);
    HILOGI("A2dpServiceTest_UnitTest_IsA2dpPlaying_001 end");
}

/**
 * @tc.name  : NotifyCaptureConnStateChangedInner
 * @tc.number: A2dpServiceTest_UnitTest_NotifyCaptureConnStateChangedInner_001
 * @tc.desc  : Test when hdap device is connected and device address matches, it should notify connected
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_NotifyCaptureConnStateChangedInner_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_NotifyCaptureConnStateChangedInner_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress  device = RawAddress("00:11:22:33:44:55");
    std::shared_ptr<A2dpDeviceInfo> hdapDeviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    hdapDeviceInfo->SetIsHdapConnected(true);
    a2dpService->SetHdapDeviceInfo(hdapDeviceInfo);
    a2dpService->NotifyCaptureConnStateChangedInner(device);
    EXPECT_TRUE(a2dpService->GetHdapDeviceInfo() != nullptr);
    HILOGI("A2dpServiceTest_UnitTest_NotifyCaptureConnStateChangedInner_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_ReceiveA2dpAbnormalError_001
 * @tc.name: ReceiveA2dpAbnormalError
 * @tc.desc: received a2dp error from stack, ReceiveA2dpAbnormalError
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ReceiveA2dpAbnormalError_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ReceiveA2dpAbnormalError_001 start");
    int errCode = 8; /* 8 ERR_CODE_AVDT_ERR_TIMEOUT */
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress  device = RawAddress("00:11:22:33:44:55");
    a2dpService->ConnectManager().ReceiveA2dpAbnormalError(device, 0);
    a2dpService->ConnectManager().ReceiveA2dpAbnormalError(device, errCode);
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_ReceiveA2dpAbnormalError_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpStreamControl_001
 * @tc.name: A2dpStreamControl
 * @tc.desc:
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpStreamControl_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpStreamControl_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress  device = RawAddress("00:11:22:33:44:55");
    int event = A2DP_MSG_PROFILE_AUDIO_PLAY_START;
    EXPECT_FALSE(a2dpService->ConnectManager().A2dpStreamControl(device, 0));
    event = A2DP_MSG_PROFILE_AUDIO_PLAY_STOP;
    EXPECT_FALSE(a2dpService->ConnectManager().A2dpStreamControl(device, 0));
    event = A2DP_MSG_PROFILE_AUDIO_PLAY_SUSPEND;
    EXPECT_FALSE(a2dpService->ConnectManager().A2dpStreamControl(device, 0));
    HILOGI("A2dpServiceTest_UnitTest_A2dpStreamControl_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_RemoveA2dpAbnormalVirtualDevice_001
 * @tc.name: RemoveA2dpAbnormalVirtualDevice
 * @tc.desc: remove virtual device which added when a2dp happen error, RemoveA2dpAbnormalVirtualDevice
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_RemoveA2dpAbnormalVirtualDevice_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_RemoveA2dpAbnormalVirtualDevice_001 start");
    int errCode = 8; /* ERR_CODE_AVDT_ERR_TIMEOUT */
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress  device = RawAddress("00:11:22:33:44:55");

    if (a2dpService != nullptr) {
        a2dpService->ConnectManager().RemoveA2dpAbnormalVirtualDevice(device);
        a2dpService->ConnectManager().abnomalDevices_.Push(device.GetAddress());
        a2dpService->ConnectManager().RemoveA2dpAbnormalVirtualDevice(device);
        a2dpService->virtualDevices_.Push(device.GetAddress());
        a2dpService->ConnectManager().abnomalDevices_.Push(device.GetAddress());
        a2dpService->ConnectManager().RemoveA2dpAbnormalVirtualDevice(device);
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_RemoveA2dpAbnormalVirtualDevice_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_HandleA2dpAbnormalError_001
 * @tc.name: HandleA2dpAbnormalError
 * @tc.desc: handle a2dp error when a2dp happen error, HandleA2dpAbnormalError
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_HandleA2dpAbnormalError_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_HandleA2dpAbnormalError_001 start");
    int errCode = 8; /* ERR_CODE_AVDT_ERR_TIMEOUT */
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    a2dpService->ConnectManager().HandleA2dpAbnormalError(device, 0);
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    EXPECT_EQ(a2dpService->GetDeviceFromList(device), deviceInfo);
    a2dpService->ConnectManager().HandleA2dpAbnormalError(device, errCode);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
    a2dpService->ConnectManager().HandleA2dpAbnormalError(device, errCode);
    HILOGI("A2dpServiceTest_UnitTest_HandleA2dpAbnormalError_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_HandleA2dpAbnormalError_002
 * @tc.name: HandleA2dpAbnormalError
 * @tc.desc: handle a2dp error when a2dp happen error, HandleA2dpAbnormalError
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_HandleA2dpAbnormalError_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_HandleA2dpAbnormalError_002 start");
    int errCode = 8; /* ERR_CODE_AVDT_ERR_TIMEOUT */
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:66");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
    a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    EXPECT_EQ(a2dpService->GetDeviceFromList(device), deviceInfo);
    a2dpService->ConnectManager().HandleA2dpAbnormalError(device, errCode);
    // new HfpAgService
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(ADAPTER_BREDR, "HfpAgService");
    auto hfpAgService = manager->GetProfileService("HfpAgService");
    BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(device.GetAddress(), false);
    a2dpService->ConnectManager().HandleA2dpAbnormalError(device, errCode);
    int state = static_cast<int>(HfpAgConnectState::HFP_AG_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(device.GetAddress(), state);
    a2dpService->ConnectManager().HandleA2dpAbnormalError(device, errCode);
    BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(device.GetAddress(), true);
    a2dpService->virtualDevices_.Push(device.GetAddress());
    a2dpService->ConnectManager().HandleA2dpAbnormalError(device, errCode);
    a2dpService->virtualDevices_.Erase(device.GetAddress());
    a2dpService->ConnectManager().HandleA2dpAbnormalError(device, errCode);
    a2dpService->ConnectManager().RemoveA2dpAbnormalVirtualDevice(device);
    a2dpService->ConnectManager().DeleteDevice(device);
    BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(device.GetAddress(), false);
    delete hfpAgService;
    sleep(TIME);
    HILOGI("A2dpServiceTest_UnitTest_HandleA2dpAbnormalError_002 end");
}

/**
 * @tc.name  : SetA2dpPlayingStateCallback
 * @tc.number: A2dpServiceTest_UnitTest_SubscribeCommonEvent_001
 * @tc.desc  : Test SetA2dpPlayingStateCallback
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetA2dpPlayingStateCallback_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetA2dpPlayingStateCallback_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        auto func = [](int state){};
        a2dpService->SetA2dpPlayingStateCallback(func);
        EXPECT_TRUE(a2dpService->a2dpStatecallback_);
    }
    HILOGI("A2dpServiceTest_UnitTest_SetA2dpPlayingStateCallback_001 end");
}

/**
 * @tc.name  : HandleA2dpPlayingStateChange
 * @tc.number: A2dpServiceTest_UnitTest_HandleA2dpPlayingStateChange_001
 * @tc.desc  : Test HandleA2dpPlayingStateChange
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_HandleA2dpPlayingStateChange_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_HandleA2dpPlayingStateChange_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->a2dpStatecallback_ = [](int state){};
        a2dpService->HandleA2dpPlayingStateChange(BTAV_AUDIO_STATE_STARTED);
        EXPECT_TRUE(a2dpService->a2dpStatecallback_);
        a2dpService->isLastOnLowLatency_ = true;
        a2dpService->HandleA2dpPlayingStateChange(BTAV_AUDIO_STATE_STOPPED);
        EXPECT_FALSE(a2dpService->isLastOnLowLatency_);
    }
    HILOGI("A2dpServiceTest_UnitTest_HandleA2dpPlayingStateChange_001 end");
}

/**
 * @tc.name  : ProcessConnectFrameworkCallback
 * @tc.number: A2dpServiceTest_UnitTest_ProcessConnectFrameworkCallback_001
 * @tc.desc  : Test when bt turn off, notify capture state change to upper
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_ProcessConnectFrameworkCallback_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_ProcessConnectFrameworkCallback_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("00:11:22:33:44:55");
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    std::shared_ptr<A2dpDeviceInfo> hdapDeviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    hdapDeviceInfo->SetIsHdapConnected(true);
    a2dpService->SetHdapDeviceInfo(hdapDeviceInfo);
    a2dpService->ProcessConnectFrameworkCallback(state, device);
    EXPECT_TRUE(hdapDeviceInfo->IsHdapConnected() == false);
    EXPECT_TRUE(a2dpService->GetHdapDeviceInfo() == nullptr);
    
    RawAddress device2 = RawAddress("11:11:22:33:44:55");
    hdapDeviceInfo->SetIsHdapConnected(true);
    a2dpService->SetHdapDeviceInfo(hdapDeviceInfo);
    a2dpService->ProcessConnectFrameworkCallback(state, device2);
    EXPECT_TRUE(hdapDeviceInfo->IsHdapConnected() == true);
    RawAddress inactiveDevice = RawAddress("11:11:22:33:44:55");
    a2dpService->ProcessConnectFrameworkCallback(state, inactiveDevice);
    HILOGI("A2dpServiceTest_UnitTest_ProcessConnectFrameworkCallback_001 end");
}

/**
 * @tc.name  : SetCurrentDeviceMute
 * @tc.number: A2dpServiceTest_UnitTest_SetCurrentDeviceMute_001
 * @tc.desc  : Test SetCurrentDeviceMute
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SetCurrentDeviceMute_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SetCurrentDeviceMute_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    std::string addr = "11:22:33:44:55:66";
    RawAddress device = RawAddress(addr);
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        a2dpService->UpdateActiveDevice(device);

        BluetoothAudioManager::GetInstance().SetProfileStatus(addr, 2, 2);
#ifdef AUDIO_FRAMEWORK
        BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioPreferredOutPutDeviceChangeListener();
        auto audioGroupManager = AudioStandard::AudioVolumeClientManager::GetInstance().GetGroupManager(
            OHOS::AudioStandard::DEFAULT_VOLUME_GROUP_ID);
        if (audioGroupManager != nullptr) {
            int ret = audioGroupManager->SetMute(AudioStandard::AudioVolumeType::STREAM_MUSIC, false);
            a2dpService->SetCurrentDeviceMute();
            if (ret == 0) {
                EXPECT_TRUE(a2dpService->timerForRestoreVolumeIfPaused_ != nullptr);
            } else {
                EXPECT_TRUE(a2dpService->timerForRestoreVolumeIfPaused_ == nullptr);
            }
        }
        BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioPreferredOutPutDeviceChangeListener();
#endif
    }

    HILOGI("A2dpServiceTest_UnitTest_SetCurrentDeviceMute_001 end");
}

/**
 * @tc.name  : FetchCurrentRenderScene
 * @tc.number: A2dpServiceTest_UnitTest_FetchCurrentRenderScene_001
 * @tc.desc  : Test FetchCurrentRenderScene
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_FetchCurrentRenderScene_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_FetchCurrentRenderScene_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->codecChangeScene_ = CodecChangeAppScene::APP_TYPE_MUSIC;
        CodecChangeAppScene curScene = CodecChangeAppScene::APP_TYPE_OTHER;
        curScene = a2dpService->FetchCurrentRenderScene(false, false, false, false);
        EXPECT_EQ(curScene, CodecChangeAppScene::APP_TYPE_MUSIC);

        a2dpService->gameRunning_ = false;
        curScene = a2dpService->FetchCurrentRenderScene(true, true, true, false);
        EXPECT_EQ(curScene, CodecChangeAppScene::APP_TYPE_GAME);

        a2dpService->anyRenderRunning_ = false;
        curScene = a2dpService->FetchCurrentRenderScene(true, true, false, true);
        EXPECT_EQ(curScene, CodecChangeAppScene::APP_TYPE_MUSIC);

        curScene = a2dpService->FetchCurrentRenderScene(true, false, false, false);
        EXPECT_EQ(curScene, CodecChangeAppScene::APP_TYPE_OTHER);

    }
    HILOGI("A2dpServiceTest_UnitTest_FetchCurrentRenderScene_001 end");
}

/**
 * @tc.name  : IsA2dpSceneChanged
 * @tc.number: A2dpServiceTest_UnitTest_IsA2dpSceneChanged_001
 * @tc.desc  : Test IsA2dpSceneChanged
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_IsA2dpSceneChanged_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_IsA2dpSceneChanged_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    if (a2dpService != nullptr) {
        a2dpService->lastIwareAppType_ = -1;
        const bthwif_interface_t *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        if (bthwif != nullptr) {
            bthwif->hwSendSceneType(9); // game
            CodecChangeAppScene lastScene = CodecChangeAppScene::APP_TYPE_OTHER;
            a2dpService->codecChangeScene_ = CodecChangeAppScene::APP_TYPE_OTHER;
            // 流变化，且有流，other变游戏，当前前台刷新为游戏，返回true
            auto isChange = a2dpService->IsA2dpSceneChanged(true, true, lastScene);
            EXPECT_EQ(isChange, true);
            // 流不变化，且有流，前台保持为游戏，返回false
            isChange = a2dpService->IsA2dpSceneChanged(false, true, lastScene);
            EXPECT_EQ(isChange, false);
            // 重复通知，音乐变为游戏
            lastScene = CodecChangeAppScene::APP_TYPE_MUSIC;
            isChange = a2dpService->IsA2dpSceneChanged(false, true, lastScene);
            EXPECT_EQ(isChange, true);
            // 流不变化，无流，前台从音乐变为游戏，返回false
            a2dpService->lastIwareAppType_ = DDM_IWARE_MUSIC;
            isChange = a2dpService->IsA2dpSceneChanged(false, false, lastScene);
            EXPECT_EQ(isChange, false);
            // 流变化，无流，前台从音乐变为游戏，返回false
            a2dpService->lastIwareAppType_ = DDM_IWARE_MUSIC;
            isChange = a2dpService->IsA2dpSceneChanged(true, false, lastScene);
            EXPECT_EQ(isChange, false);
            // 流不变化，有流，前台从音乐变为游戏，返回true
            isChange = a2dpService->IsA2dpSceneChanged(false, true, lastScene);
            EXPECT_EQ(isChange, true);
            // 流不变化，有流，前台保持为音乐，流类型从music变为other，返回true
            a2dpService->lastIwareAppType_ = DDM_IWARE_MUSIC;
            bthwif->hwSendSceneType(7); // music
            isChange = a2dpService->IsA2dpSceneChanged(false, true, lastScene);
            EXPECT_EQ(isChange, true);
        }
    }
    HILOGI("A2dpServiceTest_UnitTest_IsA2dpSceneChanged_001 end");
}

/**
 * @tc.name  : NotifyBondStateChanged
 * @tc.number: A2dpServiceTest_UnitTest_NotifyBondStateChanged_001
 * @tc.desc  : Test NotifyBondStateChanged
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_NotifyBondStateChanged_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_NotifyBondStateChanged_001 enter");
    bt_status_t status = BT_STATUS_SUCCESS;
    RawAddress device = RawAddress("00:11:22:33:44:55");
    STACK::RawAddress bd_addr = ServiceUtil::AddrToStack(device);
    bt_bond_state_t state = BT_BOND_STATE_NONE;
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_A2DP, ThreadUtil::ENABLED);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo = a2dpService->GetDeviceFromList(device);
        EXPECT_TRUE(deviceInfo != nullptr);
        a2dpService->NotifyBondStateChanged(status, device, state);
        sleep(1); // 异步处理，等待1s
        deviceInfo = a2dpService->GetDeviceFromList(device);
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_A2DP, ThreadUtil::DISABLED);
        EXPECT_TRUE(deviceInfo == nullptr);
    }
    HILOGI("A2dpServiceTest_UnitTest_NotifyBondStateChanged_001 end");
}

/**
 * @tc.name  : UpdateActiveDevice
 * @tc.number: A2dpServiceTest_UnitTest_UpdateActiveDevice_001
 * @tc.desc  : Test UpdateActiveDevice
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_UpdateActiveDevice_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_UpdateActiveDevice_001 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->UpdateActiveDevice(device);
        EXPECT_EQ(a2dpService->GetActiveSinkDevice().GetAddress(), device.GetAddress());
        RawAddress emptyDevice = RawAddress("");
        a2dpService->UpdateActiveDevice(emptyDevice);
        EXPECT_EQ(a2dpService->GetActiveSinkDevice().GetAddress(), emptyDevice.GetAddress());
    }
    HILOGI("A2dpServiceTest_UnitTest_UpdateActiveDevice_001 end");
}

/**
 * @tc.name  : UpdateActiveDeviceAfterAudioServerExit
 * @tc.number: A2dpServiceTest_UnitTest_UpdateActiveDeviceAfterAudioServerExit_001
 * @tc.desc  : Test UpdateActiveDeviceAfterAudioServerExit
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_UpdateActiveDeviceAfterAudioServerExit_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_UpdateActiveDeviceAfterAudioServerExit_001 enter");
    std::string deviceAddr = "00:11:22:33:44:55";
    RawAddress device = RawAddress(deviceAddr);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->UpdateActiveDevice(device);
        EXPECT_EQ(a2dpService->GetActiveSinkDevice().GetAddress(), device.GetAddress());
        sptr<SystemAbilityStatusChange> statusChangeListener = new SystemAbilityStatusChange();
        statusChangeListener->OnRemoveSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, deviceAddr);
    }
    HILOGI("A2dpServiceTest_UnitTest_UpdateActiveDeviceAfterAudioServerExit_001 end");
}

/**
 * @tc.name  : UpdateVirtualDeviceByA2dpConnectState
 * @tc.number: A2dpServiceTest_UnitTest_UpdateVirtualDeviceByA2dpConnectState_001
 * @tc.desc  : Test UpdateVirtualDeviceByA2dpConnectState
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_UpdateVirtualDeviceByA2dpConnectState_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_UpdateVirtualDeviceByA2dpConnectState_001 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        int state = a2dpService->GetRemoteDeviceConnectStatus(device);
        EXPECT_EQ(state, static_cast<int>(BTConnectState::DISCONNECTED));
    }
    HILOGI("A2dpServiceTest_UnitTest_UpdateVirtualDeviceByA2dpConnectState_001 end");
}

/**
 * @tc.name  : SubscribeCommonEvent
 * @tc.number: A2dpServiceTest_UnitTest_SubscribeCommonEvent_001
 * @tc.desc  : Test SubscribeCommonEvent
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_SubscribeCommonEvent_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_SubscribeCommonEvent_001 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        EXPECT_TRUE(a2dpService->SubscribeCommonEvent());
    }
    HILOGI("A2dpServiceTest_UnitTest_SubscribeCommonEvent_001 end");
}

/**
 * @tc.name  : OnReceiveEvent
 * @tc.number: A2dpServiceTest_UnitTest_OnReceiveEvent_001
 * @tc.desc  : Test OnReceiveEvent
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_OnReceiveEvent_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_OnReceiveEvent_001 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        EventFwk::CommonEventData eventData;
        AAFwk::Want want;
        want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        eventData.SetWant(want);
        eventData.SetCode(static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS));
        OHOS::EventFwk::CommonEventSubscribeInfo eventSubscribeInfo;
        std::shared_ptr<A2dpService::ReceiveMessage> Subscriber =
            std::make_shared<A2dpService::ReceiveMessage>(eventSubscribeInfo, *a2dpService);
        Subscriber->OnReceiveEvent(eventData);
        a2dpService->SetWirelessChargingFlag(true);
        EXPECT_TRUE(a2dpService->IsInWirelessCharging());
        Subscriber->OnReceiveEvent(eventData);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
            deviceInfo->SetPlayingState(true);
        }
        Subscriber->OnReceiveEvent(eventData);
        eventData.SetCode(static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB));
        Subscriber->OnReceiveEvent(eventData);
        want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        eventData.SetWant(want);
        Subscriber->OnReceiveEvent(eventData);
    }
    HILOGI("A2dpServiceTest_UnitTest_OnReceiveEvent_001 end");
}

HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetMusicState_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetMusicState_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->GetMusicState();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetMusicState_001 end");
}

HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_GetGameState_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_GetGameState_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->GetGameState();
    }
    EXPECT_TRUE(a2dpService != NULL);
    HILOGI("A2dpServiceTest_UnitTest_GetGameState_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_008
 * @tc.name: A2dpOffloadSessionPathRequest
 * @tc.desc: A2dpOffloadSessionPathRequest
 */
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_008, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_008 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("11:22:33:44:55:66");
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = 100000;
        stream.streamType = 1;
        streamsInfo.push_back(stream);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
        BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::SetAdaptiveSwitchStatus(
            true);
        SetOffloadEncodePathValue(2);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(device, streamsInfo);
        EXPECT_EQ(ret, 1);
        BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::SetAdaptiveSwitchStatus(
            false);
    }
    HILOGI("A2dpServiceTest_UnitTest_A2dpOffloadSessionPathRequest_008 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpDisconnect_001
 * @tc.name: A2dpDisconnect
 * @tc.desc: device already DISCONNECTED, skip disconnect and keep state unchanged
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpDisconnect_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpDisconnect_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    ASSERT_NE(a2dpService, nullptr);
    RawAddress device = RawAddress("11:22:33:44:55:74");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    ASSERT_NE(deviceInfo, nullptr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
    a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);

    bool ret = a2dpService->ConnectManager().A2dpDisconnect(device);
    EXPECT_TRUE(ret == true);
    // The state must stay DISCONNECTED instead of becoming a ghost DISCONNECTING
    EXPECT_EQ(deviceInfo->GetConnectState(), static_cast<int>(BTConnectState::DISCONNECTED));
    HILOGI("A2dpServiceTest_UnitTest_A2dpDisconnect_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_A2dpDisconnect_002
 * @tc.name: A2dpDisconnect
 * @tc.desc: device not in list, return false
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_A2dpDisconnect_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_A2dpDisconnect_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    ASSERT_NE(a2dpService, nullptr);
    RawAddress device = RawAddress("11:22:33:44:55:75");
    bool ret = a2dpService->ConnectManager().A2dpDisconnect(device);
    EXPECT_TRUE(ret == false);
    HILOGI("A2dpServiceTest_UnitTest_A2dpDisconnect_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_JudgeConnectedNum_001
 * @tc.name: JudgeConnectedNum
 * @tc.desc: empty device list, return true
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_JudgeConnectedNum_001, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_001 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    ASSERT_NE(a2dpService, nullptr);
    bool ret = a2dpService->ConnectManager().JudgeConnectedNum();
    EXPECT_TRUE(ret == true);
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_001 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_JudgeConnectedNum_002
 * @tc.name: JudgeConnectedNum
 * @tc.desc: device stuck in DISCONNECTING (ghost state) does not occupy the connection quota
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_JudgeConnectedNum_002, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_002 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    ASSERT_NE(a2dpService, nullptr);
    RawAddress device = RawAddress("11:22:33:44:55:71");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    ASSERT_NE(deviceInfo, nullptr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
    a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);

    int maxNum = a2dpService->GetMaxConnectNum();
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_002 maxNum: %{public}d", maxNum);
    bool ret = a2dpService->ConnectManager().JudgeConnectedNum();
    // A DISCONNECTING device must not block new connections even if max connect num is reached
    EXPECT_TRUE(ret == true);
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_002 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_JudgeConnectedNum_003
 * @tc.name: JudgeConnectedNum
 * @tc.desc: CONNECTED device occupies the connection quota
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_JudgeConnectedNum_003, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_003 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    ASSERT_NE(a2dpService, nullptr);
    RawAddress device = RawAddress("11:22:33:44:55:72");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    ASSERT_NE(deviceInfo, nullptr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);

    int maxNum = a2dpService->GetMaxConnectNum();
    bool ret = a2dpService->ConnectManager().JudgeConnectedNum();
    if (maxNum <= 1) {
        EXPECT_TRUE(ret == false);
    } else {
        EXPECT_TRUE(ret == true);
    }
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_003 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_JudgeConnectedNum_004
 * @tc.name: JudgeConnectedNum
 * @tc.desc: DISCONNECTED device does not occupy the connection quota
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_JudgeConnectedNum_004, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_004 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    ASSERT_NE(a2dpService, nullptr);
    RawAddress device = RawAddress("11:22:33:44:55:73");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    ASSERT_NE(deviceInfo, nullptr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
    a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);

    bool ret = a2dpService->ConnectManager().JudgeConnectedNum();
    EXPECT_TRUE(ret == true);
    HILOGI("A2dpServiceTest_UnitTest_JudgeConnectedNum_004 end");
}

/*
 * @tc.number: A2dpServiceTest_UnitTest_Connect_006
 * @tc.name: Connect
 * @tc.desc: device state is DISCONNECTING, retry connect with delay and return RET_NO_ERROR
*/
HWTEST_F(A2dpServiceTest, A2dpServiceTest_UnitTest_Connect_006, TestSize.Level1)
{
    HILOGI("A2dpServiceTest_UnitTest_Connect_006 start");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = RawAddress("11:22:33:44:55:76");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService && deviceInfo != nullptr) {
        // Simulate a ghost state produced by a stale disconnect event
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }
    // The remote device must support a2dp sink, otherwise Connect returns BT_ERR_PROFILE_DISABLED
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    if (remoteDevice != nullptr) {
        std::vector<Uuid> remoteUuids;
        remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_A2DP_SINK));
        remoteDevice->SetDeviceUuids(remoteUuids);
    }
    a2dpService->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));

    int ret = RET_NO_ERROR;
    if (a2dpService != nullptr) {
        ret = a2dpService->Connect(device);
    }
    // The connect request is accepted and delayed to retry after the disconnect completes
    EXPECT_EQ(ret, RET_NO_ERROR);
    HILOGI("A2dpServiceTest_UnitTest_Connect_006 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
