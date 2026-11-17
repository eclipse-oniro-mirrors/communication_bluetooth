
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
#include "map_mse_masinstance.h"
#include "map_mse_service_impl.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

constexpr int32_t TIME = 4;
static BluetoothHost *host_;

class MapMseMasInstanceTest : public testing::Test {
public:
    MapMseMasInstanceTest() {}
    ~MapMseMasInstanceTest() {}
    MapMnsSdpFoundRecord record;
    std::shared_ptr<MapMseMasInstance> mapMseMasInstance = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MapMseMasInstanceTest::SetUpTestCase(void)
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
}

void MapMseMasInstanceTest::TearDownTestCase(void) {}

void MapMseMasInstanceTest::SetUp()
{
    mapMseMasInstance = std::make_shared<MapMseMasInstance>(0, nullptr);
}

void MapMseMasInstanceTest::TearDown() {
}

HWTEST_F(MapMseMasInstanceTest, MapMseMasInstanceTest_UnitTest_StartObexServerSession_001, TestSize.Level1)
{
    HILOGI("MapMseMasInstanceTest_UnitTest_StartObexServerSession_001 start");
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    int32_t socketFd = 0;
    uint32_t maxTxPacketSize = 0;
    uint32_t maxRxPacketSize = 0;
    SocketType transportType;
    std::string deviceAddress = "";
    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(socketFd, maxTxPacketSize,
        maxRxPacketSize, transportType, deviceAddress);
    mapMseMasInstance->StartObexServerSession(socketDevice, mnsClient);
    // obexServerSession_ is alread create
    EXPECT_TRUE(mapMseMasInstance->obexServerSession_ != nullptr);
    mapMseMasInstance->StopObexServerSession();
    HILOGI("MapMseMasInstanceTest_UnitTest_StartObexServerSession_001 end");
}

HWTEST_F(MapMseMasInstanceTest, MapMseMasInstanceTest_UnitTest_UpdateMnsMseClient_002, TestSize.Level1)
{
    HILOGI("MapMseMasInstanceTest_UnitTest_UpdateMnsMseClient_002 start");
    mapMseMasInstance->UpdateMnsMseClient(nullptr);
    MapMnsSdpFoundRecord record;
    MapMseServiceImpl mapMseServiceImpl;
    std::shared_ptr<MapMseMnsClient> mnsClient = std::make_shared<MapMseMnsClient>(record, &mapMseServiceImpl);
    EXPECT_TRUE(mapMseMasInstance->mapMseObexServer_ == nullptr);
    mapMseMasInstance->UpdateMnsMseClient(mnsClient);
    HILOGI("MapMseMasInstanceTest_UnitTest_UpdateMnsMseClient_002 end");
}

}  // namespace bluetooth
}  // namespace OHOS