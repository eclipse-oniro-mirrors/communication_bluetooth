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
#include "bluetooth_pbap_pse_server.h"
#include "bluetooth_errorcode.h"
#include "profile_service_manager.h"
#include "bluetooth_pbap_pse_observer_stub.h"
#include "bt_def.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {

static BluetoothHost *host_;
const int TIME = 4;
static std::unique_ptr<BluetoothPbapPseServer> pbapPseServer_ {nullptr};

class BluetoothPbapPseObserverTest : public BluetoothPbapPseObserverStub {
public:
    BluetoothPbapPseObserverTest() = default;
    ~BluetoothPbapPseObserverTest() override = default;
    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override {}
};

class PbapPseServerTest : public testing::Test {
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
        pbapPseServer_ = std::make_unique<BluetoothPbapPseServer>();
        auto manager = ProfileServiceManager::GetInstance();
        string name = "PbapPseService";
        manager->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, name);
    }
    void TearDown()
    {
        pbapPseServer_ = nullptr;
    }
};

/*
 * @tc.number: Pbap_pse_test_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(PbapPseServerTest, Pbap_pse_test_001, TestSize.Level1)
{
    HILOGI("Pbap_pse_test_001 start");

    pbapPseServer_->RegisterObserver(nullptr);
    pbapPseServer_->DeregisterObserver(nullptr);

    sptr<IBluetoothPbapPseObserver> observer = new BluetoothPbapPseObserverTest();
    pbapPseServer_->RegisterObserver(observer);
    pbapPseServer_->DeregisterObserver(observer);

    std::vector<int32_t> states {};
    std::vector<BluetoothRawAddress> devices {};
    pbapPseServer_->GetDevicesByStates(states, devices);

    int32_t state = 0;
    BluetoothRawAddress device("11:22:33:44:55:66");
    pbapPseServer_->GetDeviceState(device, state);

    pbapPseServer_->Disconnect(device);

    int32_t strategy = 0;
    pbapPseServer_->SetConnectionStrategy(device, strategy);
    pbapPseServer_->GetConnectionStrategy(device, strategy);

    int32_t shareType = 0;
    pbapPseServer_->SetShareType(device, shareType);
    pbapPseServer_->GetShareType(device, shareType);

    int32_t accessAuthorization = 0;
    pbapPseServer_->SetPhoneBookAccessAuthorization(device, accessAuthorization);
    int ret = pbapPseServer_->GetPhoneBookAccessAuthorization(device, accessAuthorization);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    HILOGI("Pbap_pse_test_001 end");
}

/*
 * @tc.number: Pbap_pse_RegisterObserver_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(PbapPseServerTest, Pbap_pse_RegisterObserver_001, TestSize.Level1)
{
    HILOGI("Pbap_pse_RegisterObserver_001 start");
    for(int i = 0; i <= 1001; i++) {
        sptr<IBluetoothPbapPseObserver> observer;
        pbapPseServer_->RegisterObserver(observer);
    }
    EXPECT_TRUE(pbapPseServer_ != nullptr);
    HILOGI("Pbap_pse_RegisterObserver_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
