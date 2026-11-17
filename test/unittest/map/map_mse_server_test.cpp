/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_map_mse_server.h"
#include "profile_service_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_map_mse_observer_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {

static BluetoothHost *host_;
const int TIME = 4;
static std::unique_ptr<BluetoothMapMseServer> mapMseServer_ {nullptr};

class MapMseServerTest : public testing::Test {
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
    {}
    void SetUp()
    {
        mapMseServer_ = std::make_unique<BluetoothMapMseServer>();
        auto manager = ProfileServiceManager::GetInstance();
        string name = "MapMseService";
        manager->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, name);
    }
    void TearDown()
    {
        mapMseServer_ = nullptr;
    }
};

class BluetoothMapMseObserverTest : public BluetoothMapMseObserverStub {
public:
    BluetoothMapMseObserverTest() = default;
    ~BluetoothMapMseObserverTest() override = default;
    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override {}
};

/*
 * @tc.number: Map_mse_test_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(MapMseServerTest, Map_mse_test_001, TestSize.Level1)
{
    HILOGI("Map_mse_test_001 start");

    mapMseServer_->RegisterObserver(nullptr);
    mapMseServer_->DeregisterObserver(nullptr);

    sptr<IBluetoothMapMseObserver> observer = new BluetoothMapMseObserverTest();
    mapMseServer_->RegisterObserver(observer);
    mapMseServer_->DeregisterObserver(observer);

    std::vector<int32_t> states {};
    std::vector<BluetoothRawAddress> devices {};
    EXPECT_EQ(mapMseServer_->GetDevicesByStates(states, devices), BT_ERR_INTERNAL_ERROR);

    int32_t state = 0;
    BluetoothRawAddress device("11:22:33:44:55:66");
    EXPECT_EQ(mapMseServer_->GetDeviceState(device, state), BT_ERR_INTERNAL_ERROR);

    EXPECT_EQ(mapMseServer_->Disconnect(device), BT_ERR_INTERNAL_ERROR);

    int32_t strategy = 0;
    EXPECT_EQ(mapMseServer_->SetConnectionStrategy(device, strategy), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(mapMseServer_->GetConnectionStrategy(device, strategy), BT_ERR_INTERNAL_ERROR);

    int32_t accessAuthorization = 0;
    EXPECT_EQ(mapMseServer_->SetMessageAccessAuthorization(device, accessAuthorization), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(mapMseServer_->GetMessageAccessAuthorization(device, accessAuthorization), BT_ERR_INTERNAL_ERROR);

    std::unique_ptr<BluetoothMapMseServer> mapMseServerTest = std::make_unique<BluetoothMapMseServer>();
    for(int i = 0; i <= 1001; i++) {
        sptr<IBluetoothMapMseObserver> ob;
        mapMseServerTest->RegisterObserver(ob);
    }

    HILOGI("Map_mse_test_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
