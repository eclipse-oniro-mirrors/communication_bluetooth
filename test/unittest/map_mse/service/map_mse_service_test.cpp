
/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <vector>
#include <cstdint>
#include <optional>
#include "map_mse_service.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "../../map_mse/service/data_share_helper_impl_test.h"
#include "connect_strategy_manager.h"
#include "bluetooth_device.h"
#include "class_creator.h"
#include "map_mse_service_impl.h"
#include "bluetooth_errorcode.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

constexpr int32_t TIME = 4;
const int32_t MAS_ID_SMS_MMS = 0;
static BluetoothHost *host_;
void *MapMseServiceCreate(void)
{
    return new MapMseService();
}

class MapMseServiceTest : public testing::Test {
public:
    MapMseServiceTest() {}
    ~MapMseServiceTest() {}
    std::shared_ptr<MapMseService> mapMseService = nullptr;
    std::shared_ptr<MapMseServiceImpl> mapMseServiceImpl_ = nullptr;
    std::shared_ptr<IMapMseObserver> mapMseConnectionObserver_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MapMseServiceTest::SetUpTestCase(void)
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
    ClassFactory::RegisterClass("MapMseService", MapMseServiceCreate);
}

void MapMseServiceTest::TearDownTestCase(void) {
    utility::CloseAllTimer();
}

void MapMseServiceTest::SetUp()
{
    mapMseService = std::make_shared<MapMseService>();
    mapMseServiceImpl_ = std::make_shared<MapMseServiceImpl>();
    mapMseConnectionObserver_ = std::make_shared<MapMseConnectionObserver>();
}

void MapMseServiceTest::TearDown()
{
    mapMseService = nullptr;
    mapMseServiceImpl_ = nullptr;
    mapMseConnectionObserver_ = nullptr;
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetService_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetService_001 start");
    MapMseService::GetService();
    HILOGI("MapMseServiceTest_UnitTest_GetService_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_Disconnect_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_001 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    int ret = mapMseServiceImpl_->Disconnect(device);
    EXPECT_EQ(ret, 0);
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_Disconnect_002, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_002 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    mapMseServiceImpl_->connectDevice_ = device.GetAddress();
    int ret = mapMseServiceImpl_->Disconnect(device);
    EXPECT_EQ(ret, 0);
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_002 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_Disconnect_003, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_003 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    mapMseServiceImpl_->connectState_ = static_cast<int32_t>(BTConnectState::CONNECTED);
    int ret = mapMseServiceImpl_->Disconnect(device);
    EXPECT_EQ(ret, 0);
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_003 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_Disconnect_004, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_004 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    mapMseServiceImpl_->socketDevice_ = nullptr;
    mapMseServiceImpl_->connectState_ = static_cast<int32_t>(BTConnectState::CONNECTED);
    int ret = mapMseServiceImpl_->Disconnect(device);
    EXPECT_EQ(ret, Bluetooth::BT_ERR_INTERNAL_ERROR);
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_004 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetConnectDevices_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_001 start");
    std::list<RawAddress> ret = mapMseServiceImpl_->GetConnectDevices();
    EXPECT_EQ(ret.size(), 0);
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetConnectDevices_002, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_002 start");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    mapMseServiceImpl_->connectState_ = static_cast<int32_t>(BTConnectState::CONNECTED);
    std::list<RawAddress> ret = mapMseServiceImpl_->GetConnectDevices();
    EXPECT_EQ(ret.size(), 0);
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_002 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetConnectDevices_003, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_003 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    mapMseServiceImpl_->connectDevice_ = device.GetAddress();
    std::list<RawAddress> ret = mapMseServiceImpl_->GetConnectDevices();
    EXPECT_EQ(ret.size(), 0);
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_003 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetConnectDevices_004, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_004 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "12:23:34:45:56:67";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    mapMseServiceImpl_->connectDevice_ = device.GetAddress();
    mapMseServiceImpl_->connectState_ = static_cast<int32_t>(BTConnectState::CONNECTED);
    std::list<RawAddress> ret = mapMseServiceImpl_->GetConnectDevices();
    EXPECT_EQ(ret.size(), 1);
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_004 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetConnectDevices_005, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_005 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    mapMseServiceImpl_->socketDevice_ = nullptr;
    mapMseServiceImpl_->connectDevice_ = device.GetAddress();
    mapMseServiceImpl_->connectState_ = static_cast<int32_t>(BTConnectState::CONNECTED);
    std::list<RawAddress> ret = mapMseServiceImpl_->GetConnectDevices();
    EXPECT_EQ(ret.size(), 0);
    HILOGI("MapMseServiceTest_UnitTest_GetConnectDevices_005 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetDeviceState_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetDeviceState_001 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    mapMseServiceImpl_->connectDevice_ = device.GetAddress();
    int ret = mapMseServiceImpl_->GetDeviceState(device);
    EXPECT_EQ(ret, 3);
    RawAddress device1 = RawAddress("11:22:33:44:55:77");
    ret = mapMseServiceImpl_->GetDeviceState(device1);
    EXPECT_EQ(ret, 3);
    HILOGI("MapMseServiceTest_UnitTest_GetDeviceState_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetDeviceState_002, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetDeviceState_002 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    mapMseServiceImpl_->socketDevice_ = nullptr;
    mapMseServiceImpl_->connectDevice_ = device.GetAddress();
    int ret = mapMseServiceImpl_->GetDeviceState(device);
    EXPECT_EQ(ret, 3);
    HILOGI("MapMseServiceTest_UnitTest_GetDeviceState_002 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_SetConnectionStrategy_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_SetConnectionStrategy_001 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int strategy = 0;
    int ret = mapMseService->SetConnectionStrategy(device, strategy);
    EXPECT_NE(ret, 0);
    HILOGI("MapMseServiceTest_UnitTest_SetConnectionStrategy_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_SetConnectionStrategy_002, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_SetConnectionStrategy_002 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int strategy = 0;
    std::shared_ptr<BluetoothDevice> remoteDevice = std::make_shared<BluetoothDevice>(device.GetAddress());
    ConnectStrategyManager::GetInstance()->remoteDeviceProperties_->remoteDevicesMap_
        .insert(std::make_pair(device.GetAddress(), remoteDevice));
    int ret = mapMseService->SetConnectionStrategy(device, strategy);
    EXPECT_NE(ret, 0);
    HILOGI("MapMseServiceTest_UnitTest_SetConnectionStrategy_002 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_SetConnectionStrategy_003, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_SetConnectionStrategy_003 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int strategy = 2;
    std::shared_ptr<BluetoothDevice> remoteDevice = std::make_shared<BluetoothDevice>(device.GetAddress());
    ConnectStrategyManager::GetInstance()->remoteDeviceProperties_->remoteDevicesMap_
        .insert(std::make_pair(device.GetAddress(), remoteDevice));
    int ret = mapMseService->SetConnectionStrategy(device, strategy);
    EXPECT_NE(ret, 0);
    HILOGI("MapMseServiceTest_UnitTest_SetConnectionStrategy_003 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_SetMessageAccessAuthorization_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_SetMessageAccessAuthorization_001 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t accessAuthorization = 0;
    int ret = mapMseService->SetMessageAccessAuthorization(device, accessAuthorization);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("MapMseServiceTest_UnitTest_SetMessageAccessAuthorization_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_SetMessageAccessAuthorization_002, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_SetMessageAccessAuthorization_002 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t accessAuthorization = 0;
    mapMseService->EnableNative();
    mapMseService->requestPermissionFlag = true;
    int ret = mapMseService->SetMessageAccessAuthorization(device, accessAuthorization);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("MapMseServiceTest_UnitTest_SetMessageAccessAuthorization_002 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_GetMessageAccessAuthorization_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_GetMessageAccessAuthorization_001 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    int32_t accessAuthorization = 0;
    int ret = mapMseService->GetMessageAccessAuthorization(device, accessAuthorization);
    EXPECT_NE(ret, 0);
    HILOGI("MapMseServiceTest_UnitTest_GetMessageAccessAuthorization_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_SdpFoundCallback_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_SdpFoundCallback_001 start");
    MapMnsSdpFoundRecord record;
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseService->socketDevice_ = socketDevice;
    mapMseService->SdpFoundCallback(record);
    record.status = -1;
    mapMseService->SdpFoundCallback(record);
    mapMseService->sdpSearchDone_ = false;
    mapMseService->SdpFoundCallback(record);
    HILOGI("MapMseServiceTest_UnitTest_SdpFoundCallback_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_DealOnConnect_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_DealOnConnect_001 start");
    auto device = std::make_shared<ObexSocketDevice>(-1, 0, 0, SocketType::TYPE_RFCOMM, "00:00:00:00:00:00");
    int32_t masInstanceId = 0;
    mapMseService->OnConnect(device, masInstanceId);
    mapMseService->DealOnConnect(device, masInstanceId);
    HILOGI("MapMseServiceTest_UnitTest_DealOnConnect_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_DealOnConnect_002, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_DealOnConnect_002 start");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    int32_t masInstanceId = 0;
    MapMseServiceImpl mapMseServiceImpl;
    std::shared_ptr<MapMseMasInstance> masInstance = std::make_shared<MapMseMasInstance>(0, &mapMseServiceImpl);
    mapMseService->EnableNative();
    mapMseService->DealOnConnect(socketDevice, masInstanceId);
    HILOGI("MapMseServiceTest_UnitTest_DealOnConnect_002 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_DealOnClose_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_DealOnClose_001 start");
    int32_t masInstanceId = 0;
    MapMseServiceImpl mapMseServiceImpl;
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    std::shared_ptr<MapMseMasInstance> masInstance = std::make_shared<MapMseMasInstance>(0, &mapMseServiceImpl);
    mapMseServiceImpl_->masInstanceMap_.EnsureInsert(masInstanceId, masInstance);
    mapMseServiceImpl_->DealOnClose(masInstanceId);
    HILOGI("MapMseServiceTest_UnitTest_DealOnClose_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_DealOnClose_002, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_DealOnClose_002 start");
    int32_t masInstanceId = 0;
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->socketDevice_ = socketDevice;
    mapMseServiceImpl_->DealOnClose(masInstanceId);
    HILOGI("MapMseServiceTest_UnitTest_DealOnClose_002 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_CheckOrGetPermission_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_CheckOrGetPermission_001 start");
    std::string address = "";
    mapMseService->EnableNative();
    mapMseService->CheckOrGetPermission(address);
    HILOGI("MapMseServiceTest_UnitTest_CheckOrGetPermission_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_RejectConnection_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_RejectConnection_001 start");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseService->socketDevice_ = socketDevice;
    mapMseService->RejectConnection();
    sleep(2);
    EXPECT_TRUE(mapMseService->obexRejectServerSession_ == nullptr);
    HILOGI("MapMseServiceTest_UnitTest_RejectConnection_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_OnConnectionStateChanged_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_OnConnectionStateChanged_001 start");
    RawAddress device{""};
    mapMseConnectionObserver_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::CONNECTED), 2);

    RawAddress device1{"11:22:33:44:55:66"};
    mapMseConnectionObserver_->OnConnectionStateChanged(device1, static_cast<int>(BTConnectState::DISCONNECTED), 1);
    HILOGI("MapMseServiceTest_UnitTest_OnConnectionStateChanged_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_UpdateDbIdentifier_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_UpdateDbIdentifier_001 start");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseService->socketDevice_ = socketDevice;
    mapMseService->UpdateDbIdentifier();
    int ret = mapMseService->GetDbIdentifier();
    EXPECT_NE(ret, 1);
    HILOGI("MapMseServiceTest_UnitTest_UpdateDbIdentifier_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_StopSocketListener_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_StopSocketListener_001 start");
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseService->socketDevice_ = socketDevice;
    mapMseService->StopSocketListener();
    int ret = mapMseService->GetDbIdentifier();
    EXPECT_NE(ret, 1);
    HILOGI("MapMseServiceTest_UnitTest_StopSocketListener_001 end");
}


HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_SetIsNeedReconnClient_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_SetIsNeedReconnClient_001 start");
    MapMnsSdpFoundRecord record;
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseService->socketDevice_ = socketDevice;
    mapMseService->sdpSearchDone_ = false;
    mapMseService->SdpFoundCallback(record);
    mapMseService->SetIsNeedReconnClient(true);
    mapMseService->SdpFoundCallback(record);
    EXPECT_TRUE(mapMseService->sdpSearchDone_ == false);
    HILOGI("MapMseServiceTest_UnitTest_SetIsNeedReconnClient_001 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_StartUp_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_StartUp_001 start");
    MapMnsSdpFoundRecord record;
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "11:22:33:44:55:66";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseServiceImpl_->StartUp(record, socketDevice, 1);
    mapMseServiceImpl_->Disconnect(RawAddress(deviceAddress));
    EXPECT_TRUE(mapMseServiceImpl_->socketDevice_ != nullptr);
    HILOGI("MapMseServiceTest_UnitTest_Disconnect_003 end");
}

HWTEST_F(MapMseServiceTest, MapMseServiceTest_UnitTest_StartSocketListener_001, TestSize.Level1)
{
    HILOGI("MapMseServiceTest_UnitTest_StartSocketListener_001 start");
    std::shared_ptr<MapMseService> mapMseServiceTest = std::make_shared<MapMseService>();
    mapMseServiceTest->ReStartWhenSocketServerException(MAS_ID_SMS_MMS);
    EXPECT_TRUE(mapMseServiceImpl_->socketDevice_ == nullptr);
    HILOGI("MapMseServiceTest_UnitTest_StartSocketListener_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS