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
#include <cstring>
#include "bluetooth_pan_server.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
static std::unique_ptr<BluetoothPanServer> panServer_ {nullptr};
const int TIME = 4;

class PanServerTest : public testing::Test {
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
        HILOGI("TearDownTestSuite start");
        host_ = nullptr;
        HILOGI("TearDownTestSuite end");
    }
    void SetUp()
    {
        panServer_ = std::make_unique<BluetoothPanServer>();
    }
    void TearDown()
    {
    }

    void SetRandomAddr(BluetoothRawAddress &device)
    {
        string randomAddr;
        host_->GetRandomAddress(device.GetAddress(), randomAddr);
        device.address_ = randomAddr;
    }
};

/*
 * @tc.number: Pan_Server_RegisterObserver_00100
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(PanServerTest, Pan_Server_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("Pan_Server_RegisterObserver_00100 start");
    EXPECT_EQ(panServer_->RegisterObserver(nullptr), BT_ERR_INTERNAL_ERROR);

    HILOGI("Pan_Server_RegisterObserver_00100 end");
}

/*
 * @tc.number: Pan_Server_DeregisterObserver_00100
 * @tc.name: DeregisterObserver
 * @tc.desc:
 */
HWTEST_F(PanServerTest, Pan_Server_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("Pan_Server_DeregisterObserver_00100 start");
    EXPECT_EQ(panServer_->DeregisterObserver(nullptr), BT_ERR_INTERNAL_ERROR);

    HILOGI("Pan_Server_DeregisterObserver_00100 end");
}

/*
 * @tc.number: Pan_Server_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(PanServerTest, Pan_Server_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("Pan_Server_GetDevicesByStates_00100 start");

    std::vector<BluetoothRawAddress> rawAdds;
    std::vector<int32_t> states = {};

    EXPECT_EQ(panServer_->GetDevicesByStates(states, rawAdds), BT_ERR_INTERNAL_ERROR);

    HILOGI("Pan_Server_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: Pan_Server_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(PanServerTest, Pan_Server_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("Pan_Server_GetDeviceState_00100 start");

    BluetoothRawAddress devices;
    int32_t states;

    EXPECT_EQ(panServer_->GetDeviceState(devices, states), BT_ERR_INTERNAL_ERROR);

    HILOGI("Pan_Server_GetDeviceState_00100 end");
}

/*
 * @tc.number: Pan_Server_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(PanServerTest, Pan_Server_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Pan_Server_Disconnect_00100 start");
    BluetoothRawAddress addr("11:55:66:44:33:22");
    EXPECT_EQ(panServer_->Disconnect(addr), BT_ERR_INTERNAL_ERROR);
    HILOGI("Pan_Server_Disconnect_00100 end");
}

/*
 * @tc.number: Pan_Server_SetTethering_00100
 * @tc.name: SetTethering
 * @tc.desc:
 */
HWTEST_F(PanServerTest, Pan_Server_SetTethering_00100, TestSize.Level1)
{
    HILOGI("Pan_Server_SetTethering_00100 start");
    EXPECT_EQ(panServer_->SetTethering(true), BT_ERR_INTERNAL_ERROR);
    HILOGI("Pan_Server_SetTethering_00100 end");
}

/*
 * @tc.number: Pan_Server_IsTetheringOn_00100
 * @tc.name: IsTetheringOn
 * @tc.desc:
 */
HWTEST_F(PanServerTest, Pan_Server_IsTetheringOn_00100, TestSize.Level1)
{
    HILOGI("Pan_Server_IsTetheringOn_00100 start");

    bool result;
    EXPECT_EQ(panServer_->IsTetheringOn(result), BT_ERR_INTERNAL_ERROR);

    HILOGI("Pan_Server_IsTetheringOn_00100 end");
}

/*
 * @tc.number: Pan_Server__ModuleTest_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(PanServerTest, Pan_Server_ModuleTest_Connect_00100, TestSize.Level1)
{
    HILOGI("Pan_Server_ModuleTest_Connect_00100 start");
    BluetoothRawAddress device("12:34:56:78:90:12");
    EXPECT_EQ(panServer_->Connect(device), BT_ERR_INTERNAL_ERROR);
    HILOGI("Pan_Server_ModuleTest_Connect_00100 end");
}

/*
 * @tc.number: Pan_Server_ModuleTest_GetConnectStrategy_001
 * @tc.name: GetConnectStrategy
 * @tc.desc: Get connect strategy by address
*/
HWTEST_F(PanServerTest, Pan_Server_ModuleTest_GetConnectStrategy_001, TestSize.Level1)
{
    HILOGI("Pan_Server_ModuleTest_GetConnectStrategy_001 start");
    BluetoothRawAddress device("11:22:33:44:55:66");
    BluetoothRawAddress invalidDevice("11:22");
    int ret = panServer_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    int strategy;
    ret = panServer_->GetConnectStrategy(device, strategy);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    ret = panServer_->SetConnectStrategy(invalidDevice, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    ret = panServer_->GetConnectStrategy(invalidDevice, strategy);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("Pan_Server_ModuleTest_GetConnectStrategy_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS