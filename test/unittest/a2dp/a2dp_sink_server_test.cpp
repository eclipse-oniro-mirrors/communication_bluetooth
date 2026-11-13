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
#include "bluetooth_a2dp_sink_server.h"

namespace OHOS {
namespace Bluetooth {

static BluetoothHost *host_;
static std::unique_ptr<BluetoothA2dpSinkServer> a2dpSinkServer_ {nullptr};

class A2dpSinkServerTest : public testing::Test {
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
        a2dpSinkServer_ = std::make_unique<BluetoothA2dpSinkServer>();
        auto manager = ProfileServiceManager::GetInstance();
        string name = "A2dpSnkService";
        manager->SetProfileService(ADAPTER_BREDR, name);
    }
    void TearDown()
    {
        a2dpSinkServer_ = nullptr;
    }
};

/*
 * @tc.number: Pbap_pse_test_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(A2dpSinkServerTest, Pbap_pse_test_001, TestSize.Level1)
{
    HILOGI("Pbap_pse_test_001 start");

    a2dpSinkServer_->RegisterObserver(nullptr);
    a2dpSinkServer_->DeregisterObserver(nullptr);

    sptr<IBluetoothA2dpSinkObserver> observer = new BluetoothA2dpSinkObserverStub();
    a2dpSinkServer_->RegisterObserver(observer);
    a2dpSinkServer_->DeregisterObserver(observer);

    BluetoothRawAddress device("11:22:33:44:55:66");
    a2dpSinkServer_->Connect(device);
    a2dpSinkServer_->Disconnect(device);
    a2dpSinkServer_->GetDeviceState(device);

    std::vector<int32_t> states {};
    a2dpSinkServer_->GetDevicesByStates(states);

    int32_t state = 0;
    int ret = a2dpSinkServer_->GetPlayingState(device, state);
    EXPECT_EQ(ret, BT_NO_ERROR);

    HILOGI("Pbap_pse_test_001 end");
}

/*
 * @tc.number: A2dpSinkServerTest_002
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(A2dpSinkServerTest, A2dpSinkServerTest_002, TestSize.Level1)
{
    HILOGI("A2dpSinkServerTest_002 start");
    std::unique_ptr<BluetoothA2dpSinkServer> mapMseServerTest = std::make_unique<BluetoothA2dpSinkServer>();
    for(int i = 0; i <= 1001; i++) {
        sptr<IBluetoothA2dpSinkObserver> observer;
        a2dpSinkServer_->RegisterObserver(observer);
    }
    int32_t strategy = 0;
    BluetoothRawAddress device("11:22:33:44:55:66");
    a2dpSinkServer_->SetConnectStrategy(device, strategy);
    int ret = a2dpSinkServer_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(ret, BT_NO_ERROR);
    a2dpSinkServer_->SendDelay(device, 0);
    HILOGI("A2dpSinkServerTest_002 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
