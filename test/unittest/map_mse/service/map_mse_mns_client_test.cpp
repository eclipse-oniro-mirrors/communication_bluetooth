
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
#include "map_mse_mns_client.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "../../map_mse/service/data_share_helper_impl_test.h"
#include "map_mse_service.h"
#include "class_creator.h"
#include "map_mse_service_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

constexpr int32_t TIME = 4;
static BluetoothHost *host_;

void *MapMseServiceCreate1(void)
{
    return new MapMseService();
}
class MapMseMnsClientTest : public testing::Test {
public:
    MapMseMnsClientTest() {}
    ~MapMseMnsClientTest() {}
    MapMnsSdpFoundRecord record;
    MapMseServiceImpl mapMseServiceImpl;
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MapMseMnsClientTest::SetUpTestCase(void)
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
    ClassFactory::RegisterClass("MapMseService", MapMseServiceCreate1);
}

void MapMseMnsClientTest::TearDownTestCase(void) {}

void MapMseMnsClientTest::SetUp()
{
    mnsClient = std::make_shared<MapMseMnsClient>(record, &mapMseServiceImpl);
}

void MapMseMnsClientTest::TearDown() {
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_HandleRegistration_001, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_HandleRegistration_001 start");
    MapMseServiceImpl myMapMseServiceImpl;
    auto setIsNeedReconnClient = [this](const bool isNeedReconnClient) -> void {
        HILOGI("setIsNeedReconnClient");
        return;
    };
    myMapMseServiceImpl.setIsNeedReconnClientFunc_ = setIsNeedReconnClient;
    std::shared_ptr<MapMseMnsClient> myMnsClient = std::make_shared<MapMseMnsClient>(record, &myMapMseServiceImpl);
    int32_t masId = 0;
    uint8_t notificationStatus = 0;
    myMnsClient->HandleRegistration(masId, notificationStatus);
    notificationStatus = 1;
    myMnsClient->HandleRegistration(masId, notificationStatus);
    HILOGI("MapMseMnsClientTest_UnitTest_HandleRegistration_001 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_Connect_001, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_Connect_001 start");
    mnsClient->Connect();
    mnsClient->DisConnect();
    HILOGI("MapMseMnsClientTest_UnitTest_Connect_001 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_Connect_002, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_Connect_002 start");
    mnsClient->mnsRecord_.l2capPsm = 1;
    mnsClient->Connect();
    mnsClient->DisConnect();
    HILOGI("MapMseMnsClientTest_UnitTest_Connect_002 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_Connect_003, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_Connect_003 start");
    mnsClient->mnsRecord_.channel = 1;
    mnsClient->Connect();
    mnsClient->DisConnect();
    HILOGI("MapMseMnsClientTest_UnitTest_Connect_003 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_Connect_004, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_Connect_003 start");
    mnsClient->mnsRecord_.l2capPsm = 1;
    mnsClient->mnsRecord_.channel = 1;
    mnsClient->socketFd_ = -1;
    mnsClient->Connect();
    mnsClient->DisConnect();
    HILOGI("MapMseMnsClientTest_UnitTest_Connect_003 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_DisConnect_001, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_DisConnect_001 start");
    mnsClient->mnsRecord_.l2capPsm = 1;
    mnsClient->mnsRecord_.channel = 1;
    mnsClient->socketFd_ = 1;
    std::string deviceAddress = "00:11:22:33:44:55";
    auto socketDevice = std::make_shared<ObexSocketDevice>(225, 1024, 1024, SocketType::TYPE_L2CAP_LE, deviceAddress);
    std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(socketDevice);
    mnsClient->clientSession_ = std::make_shared<ObexClientSession>(transport, nullptr);
    mnsClient->Connect();
    EXPECT_TRUE(mnsClient->clientSession_ != nullptr);
    mnsClient->DisConnect();
    HILOGI("MapMseMnsClientTest_UnitTest_DisConnect_001 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_DisConnect_002, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_DisConnect_002 start");
    mnsClient->clientSession_ = nullptr;
    EXPECT_TRUE(mnsClient->clientSession_ == nullptr);
    mnsClient->DisConnect();
    HILOGI("MapMseMnsClientTest_UnitTest_DisConnect_002 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_RecvSocketDevice_001, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_RecvSocketDevice_001 start");
    SocketType socketType = SocketType::TYPE_RFCOMM;
    mnsClient->RecvSocketDevice(socketType);
    HILOGI("MapMseMnsClientTest_UnitTest_RecvSocketDevice_001 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_RecvSocketDevice_002, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_RecvSocketDevice_002 start");
    SocketType socketType = SocketType::TYPE_L2CAP;
    mnsClient->RecvSocketDevice(socketType);
    HILOGI("MapMseMnsClientTest_UnitTest_RecvSocketDevice_002 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_RecvSocketDevice_003, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_RecvSocketDevice_003 start");
    SocketType socketType = SocketType::TYPE_L2CAP_LE;
    mnsClient->RecvSocketDevice(socketType);
    HILOGI("MapMseMnsClientTest_UnitTest_RecvSocketDevice_003 end");
}

HWTEST_F(MapMseMnsClientTest, MapMseMnsClientTest_UnitTest_SendEvent_001, TestSize.Level1)
{
    HILOGI("MapMseMnsClientTest_UnitTest_SendEvent_001 start");
    std::string deviceAddress = "00:11:22:33:44:55";
    auto socketDevice = std::make_shared<ObexSocketDevice>(225, 1024, 1024, SocketType::TYPE_L2CAP_LE, deviceAddress);
    std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(socketDevice);
    mnsClient->clientSession_ = std::make_shared<ObexClientSession>(transport, nullptr);

    mnsClient->connected_ = false;
    mnsClient->SendEvent(0, "123");

    mnsClient->connected_ = true;
    mnsClient->SendEvent(0, "123");
    HILOGI("MapMseMnsClientTest_UnitTest_SendEvent_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS