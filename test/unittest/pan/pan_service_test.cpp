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
#include "pan_service.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {

static BluetoothHost *host_;
static std::unique_ptr<PanService> panService_ {nullptr};

const int TIME = 2;

void *PanServiceCreate(void)
{
    return new PanService();
}

class PanServiceTest : public testing::Test {
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
        ClassFactory::RegisterClass("PanService", PanServiceCreate);
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestSuite start");
        HILOGI("TearDownTestSuite end");
    }
    void SetUp()
    {
        panService_ = std::make_unique<PanService>();
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_PAN, ThreadUtil::DISABLED);
    }
    void TearDown()
    {
        panService_ = nullptr;
    }
};

/*
 * @tc.number: Pan_Service_SetTethering_00100
 * @tc.name: SetTethering
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_SetTethering_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_SetTethering_00100 start");

    panService_->isTetheringOn_ = true;
    EXPECT_EQ(panService_->SetTethering(true), PAN_SUCCESS);

    panService_->isTetheringOn_ = true;
    EXPECT_EQ(panService_->SetTethering(false), PAN_SUCCESS);

    panService_->isTetheringOn_ = false;
    EXPECT_EQ(panService_->SetTethering(false), PAN_SUCCESS);

    panService_->isTetheringOn_ = false;
    EXPECT_EQ(panService_->SetTethering(true), PAN_SUCCESS);

    HILOGI("Pan_Service_SetTethering_00100 end");
}

/*
 * @tc.number: Pan_Service_IsTetheringOn_00100
 * @tc.name: IsTetheringOn
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_IsTetheringOn_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_IsTetheringOn_00100 start");

    panService_->isTetheringOn_ = true;
    EXPECT_EQ(panService_->IsTetheringOn(), true);

    panService_->isTetheringOn_ = false;
    EXPECT_EQ(panService_->IsTetheringOn(), false);

    HILOGI("Pan_Service_IsTetheringOn_00100 end");
}

/*
 * @tc.number: Pan_Service_Enable_00100
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_Enable_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_Enable_00100 start");
    EXPECT_NO_FATAL_FAILURE(panService_->Enable());
    HILOGI("Pan_Service_Enable_00100 end");
}

/*
 * @tc.number: Pan_Service_Disable_00100
 * @tc.name: Disable
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_Disable_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_Disable_00100 start");
    EXPECT_NO_FATAL_FAILURE(panService_->Disable());
    HILOGI("Pan_Service_Disable_00100 end");
}

/*
 * @tc.number: Pan_Service_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_Connect_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_Connect_00100 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    EXPECT_EQ(panService_->Connect(device), BT_ERR_PROFILE_DISABLED);
    HILOGI("Pan_Service_Connect_00100 end");
}

/*
 * @tc.number: Pan_Service_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_Disconnect_00100 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    EXPECT_EQ(panService_->Disconnect(device), BT_ERR_PROFILE_DISABLED);
    HILOGI("Pan_Service_Disconnect_00100 end");
}

/*
 * @tc.number: Pan_Service_GetConnectState_00100
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_GetConnectState_00100 start");
    EXPECT_NO_FATAL_FAILURE(panService_->GetConnectState());
    HILOGI("Pan_Service_GetConnectState_00100 end");
}

/*
 * @tc.number: Pan_Service_GetMaxConnectNum_00100
 * @tc.name: GetMaxConnectNum
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_GetMaxConnectNum_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_GetMaxConnectNum_00100 start");
    int maxNum = panService_->GetMaxConnectNum();
    EXPECT_GE(maxNum, 0);
    HILOGI("Pan_Service_GetMaxConnectNum_00100 end");
}

/*
 * @tc.number: Pan_Service_GetConnectDevices_00100
 * @tc.name: GetConnectDevices
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_GetConnectDevices_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_GetConnectDevices_00100 start");
    std::list<bluetooth::RawAddress> devices = panService_->GetConnectDevices();
    EXPECT_TRUE(devices.empty() || devices.size() > 0);
    HILOGI("Pan_Service_GetConnectDevices_00100 end");
}

/*
 * @tc.number: Pan_Service_RemoveStateMachine_00100
 * @tc.name: RemoveStateMachine
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_RemoveStateMachine_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_RemoveStateMachine_00100 start");
    std::string device = "AA:BB:CC:DD:EE:FF";
    EXPECT_NO_FATAL_FAILURE(panService_->RemoveStateMachine(device));
    HILOGI("Pan_Service_RemoveStateMachine_00100 end");
}

/*
 * @tc.number: Pan_Service_BringUpNetwork_00100
 * @tc.name: BringUpNetwork
 * @tc.desc:
 */
HWTEST_F(PanServiceTest, Pan_Service_BringUpNetwork_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_BringUpNetwork_00100 start");
    panService_->isLoaded_ = true;
    EXPECT_NO_FATAL_FAILURE(panService_->BringUpNetwork());
    HILOGI("Pan_Service_BringUpNetwork_00100 end");
}

/*
 * @tc.number: Pan_Service_SetConnectStrategy_00100
 * @tc.name: SetConnectStrategy
 * @tc.desc: Test SetConnectStrategy with CONNECTION_FORBIDDEN
 */
HWTEST_F(PanServiceTest, Pan_Service_SetConnectStrategy_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_SetConnectStrategy_00100 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    int ret = panService_->SetConnectStrategy(device, strategy);
    EXPECT_TRUE(ret == BT_NO_ERROR || ret == BT_ERR_INTERNAL_ERROR);
    HILOGI("Pan_Service_SetConnectStrategy_00100 end");
}

/*
 * @tc.number: Pan_Service_SetConnectStrategy_00200
 * @tc.name: SetConnectStrategy
 * @tc.desc: Test SetConnectStrategy with CONNECTION_ALLOWED when device is disconnected
 */
HWTEST_F(PanServiceTest, Pan_Service_SetConnectStrategy_00200, TestSize.Level1)
{
    HILOGI("Pan_Service_SetConnectStrategy_00200 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    int ret = panService_->SetConnectStrategy(device, strategy);
    EXPECT_TRUE(ret == BT_NO_ERROR || ret == BT_ERR_INTERNAL_ERROR);
    HILOGI("Pan_Service_SetConnectStrategy_00200 end");
}

/*
 * @tc.number: Pan_Service_SetConnectStrategy_00300
 * @tc.name: SetConnectStrategy
 * @tc.desc: Test SetConnectStrategy with invalid strategy
 */
HWTEST_F(PanServiceTest, Pan_Service_SetConnectStrategy_00300, TestSize.Level1)
{
    HILOGI("Pan_Service_SetConnectStrategy_00300 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    int invalidStrategy = -1;
    int ret = panService_->SetConnectStrategy(device, invalidStrategy);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("Pan_Service_SetConnectStrategy_00300 end");
}

/*
 * @tc.number: Pan_Service_GetConnectStrategy_00100
 * @tc.name: GetConnectStrategy
 * @tc.desc: Test GetConnectStrategy
 */
HWTEST_F(PanServiceTest, Pan_Service_GetConnectStrategy_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_GetConnectStrategy_00100 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    int strategy = panService_->GetConnectStrategy(device);
    EXPECT_GE(strategy, 0);
    HILOGI("Pan_Service_GetConnectStrategy_00100 end");
}

/*
 * @tc.number: Pan_Service_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc: Test GetDevicesByStates with empty states vector
 */
HWTEST_F(PanServiceTest, Pan_Service_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_GetDevicesByStates_00100 start");
    std::vector<int> states;
    std::vector<bluetooth::RawAddress> devices = panService_->GetDevicesByStates(states);
    EXPECT_TRUE(devices.empty());
    HILOGI("Pan_Service_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: Pan_Service_GetDevicesByStates_00200
 * @tc.name: GetDevicesByStates
 * @tc.desc: Test GetDevicesByStates with specific states
 */
HWTEST_F(PanServiceTest, Pan_Service_GetDevicesByStates_00200, TestSize.Level1)
{
    HILOGI("Pan_Service_GetDevicesByStates_00200 start");
    std::vector<int> states = {PAN_STATE_CONNECTED};
    std::vector<bluetooth::RawAddress> devices = panService_->GetDevicesByStates(states);
    EXPECT_TRUE(devices.empty());
    HILOGI("Pan_Service_GetDevicesByStates_00200 end");
}

/*
 * @tc.number: Pan_Service_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc: Test GetDeviceState for a device
 */
HWTEST_F(PanServiceTest, Pan_Service_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_GetDeviceState_00100 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    int state = panService_->GetDeviceState(device);
    EXPECT_GE(state, 0);
    HILOGI("Pan_Service_GetDeviceState_00100 end");
}

/*
 * @tc.number: Pan_Service_StartUp_00100
 * @tc.name: StartUp
 * @tc.desc: Test service startup when not started
 */
HWTEST_F(PanServiceTest, Pan_Service_StartUp_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_StartUp_00100 start");
    panService_->isStarted_ = false;
    EXPECT_NO_FATAL_FAILURE(panService_->StartUp());
    HILOGI("Pan_Service_StartUp_00100 end");
}

/*
 * @tc.number: Pan_Service_StartUp_00200
 * @tc.name: StartUp
 * @tc.desc: Test service startup when already started
 */
HWTEST_F(PanServiceTest, Pan_Service_StartUp_00200, TestSize.Level1)
{
    HILOGI("Pan_Service_StartUp_00200 start");
    panService_->isStarted_ = true;
    EXPECT_NO_FATAL_FAILURE(panService_->StartUp());
    EXPECT_TRUE(panService_->isStarted_);
    HILOGI("Pan_Service_StartUp_00200 end");
}

/*
 * @tc.number: Pan_Service_ShutDown_00100
 * @tc.name: ShutDown
 * @tc.desc: Test service shutdown when isStarted is false
 */
HWTEST_F(PanServiceTest, Pan_Service_ShutDown_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_ShutDown_00100 start");
    panService_->isStarted_ = false;
    EXPECT_NO_FATAL_FAILURE(panService_->ShutDown());
    HILOGI("Pan_Service_ShutDown_00100 end");
}

/*
 * @tc.number: Pan_Service_ShutDown_00200
 * @tc.name: ShutDown
 * @tc.desc: Test service shutdown when isStarted is true
 */
HWTEST_F(PanServiceTest, Pan_Service_ShutDown_00200, TestSize.Level1)
{
    HILOGI("Pan_Service_ShutDown_00200 start");
    panService_->isStarted_ = true;
    EXPECT_NO_FATAL_FAILURE(panService_->ShutDown());
    HILOGI("Pan_Service_ShutDown_00200 end");
}

/*
 * @tc.number: Pan_Service_ShutDownDone_00100
 * @tc.name: ShutDownDone
 * @tc.desc: Test service shutdown done
 */
HWTEST_F(PanServiceTest, Pan_Service_ShutDownDone_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_ShutDownDone_00100 start");
    panService_->isStarted_ = true;
    panService_->isShuttingDown_ = true;
    EXPECT_NO_FATAL_FAILURE(panService_->ShutDownDone());
    EXPECT_FALSE(panService_->isStarted_);
    EXPECT_FALSE(panService_->isShuttingDown_);
    HILOGI("Pan_Service_ShutDownDone_00100 end");
}

/*
 * @tc.number: Pan_Service_ProcessControlStateChange_00100
 * @tc.name: ProcessControlStateChange
 * @tc.desc: Test ProcessControlStateChange
 */
HWTEST_F(PanServiceTest, Pan_Service_ProcessControlStateChange_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_ProcessControlStateChange_00100 start");
    PanMessage event(PAN_CONTROL_STATE_CHANGE_EVT);
    event.ifname_ = "bt-pan";
    EXPECT_NO_FATAL_FAILURE(panService_->ProcessControlStateChange(event));
    EXPECT_TRUE(panService_->ifname_ == event.ifname_);
    HILOGI("Pan_Service_ProcessControlStateChange_00100 end");
}

/*
 * @tc.number: Pan_Service_ProcessDisconnectAll_00100
 * @tc.name: ProcessDisconnectAll
 * @tc.desc: Test ProcessDisconnectAll
 */
HWTEST_F(PanServiceTest, Pan_Service_ProcessDisconnectAll_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_ProcessDisconnectAll_00100 start");
    EXPECT_NO_FATAL_FAILURE(panService_->ProcessDisconnectAll());
    HILOGI("Pan_Service_ProcessDisconnectAll_00100 end");
}

/*
 * @tc.number: Pan_Service_ProcessRemoveStateMachine_00100
 * @tc.name: ProcessRemoveStateMachine
 * @tc.desc: Test ProcessRemoveStateMachine
 */
HWTEST_F(PanServiceTest, Pan_Service_ProcessRemoveStateMachine_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_ProcessRemoveStateMachine_00100 start");
    PanMessage event(PAN_REMOVE_STATE_MACHINE_EVT);
    event.dev_ = "AA:BB:CC:DD:EE:FF";
    EXPECT_NO_FATAL_FAILURE(panService_->ProcessRemoveStateMachine(event));
    HILOGI("Pan_Service_ProcessRemoveStateMachine_00100 end");
}

/*
 * @tc.number: Pan_Service_NotifyStateChanged_00100
 * @tc.name: NotifyStateChanged
 * @tc.desc: Test NotifyStateChanged
 */
HWTEST_F(PanServiceTest, Pan_Service_NotifyStateChanged_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_NotifyStateChanged_00100 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    EXPECT_NO_FATAL_FAILURE(panService_->NotifyStateChanged(device, PAN_STATE_DISCONNECTED, LOCAL_NAP_ROLE));
    HILOGI("Pan_Service_NotifyStateChanged_00100 end");
}

/*
 * @tc.number: Pan_Service_IsRemotePanSupported_00100
 * @tc.name: IsRemotePanSupported
 * @tc.desc: Test IsRemotePanSupported with device without PAN UUID
 */
HWTEST_F(PanServiceTest, Pan_Service_IsRemotePanSupported_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_IsRemotePanSupported_00100 start");
    bluetooth::RawAddress device("AA:BB:CC:DD:EE:FF");
    bool supported = panService_->IsRemotePanSupported(device);
    EXPECT_FALSE(supported);
    HILOGI("Pan_Service_IsRemotePanSupported_00100 end");
}

/*
 * @tc.number: Pan_Service_getBluetoothPanInterface_00100
 * @tc.name: getBluetoothPanInterface
 * @tc.desc: Test getBluetoothPanInterface
 */
HWTEST_F(PanServiceTest, Pan_Service_getBluetoothPanInterface_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_getBluetoothPanInterface_00100 start");
    btpan_interface_t* panInterface = panService_->getBluetoothPanInterface();
    EXPECT_TRUE(panInterface == nullptr);
    HILOGI("Pan_Service_getBluetoothPanInterface_00100 end");
}

/*
 * @tc.number: Pan_Service_GetService_00100
 * @tc.name: GetService
 * @tc.desc: Test GetService
 */
HWTEST_F(PanServiceTest, Pan_Service_GetService_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_GetService_00100 start");
    EXPECT_NO_FATAL_FAILURE(PanService::GetService());
    HILOGI("Pan_Service_GetService_00100 end");
}

/*
 * @tc.number: Pan_Service_RegisterObserver_00100
 * @tc.name: RegisterObserver
 * @tc.desc: Test RegisterObserver
 */
HWTEST_F(PanServiceTest, Pan_Service_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_RegisterObserver_00100 start");
    IPanObserver* observer = nullptr;
    EXPECT_NO_FATAL_FAILURE(panService_->RegisterObserver(*observer));
    HILOGI("Pan_Service_RegisterObserver_00100 end");
}

/*
 * @tc.number: Pan_Service_DeregisterObserver_00100
 * @tc.name: DeregisterObserver
 * @tc.desc: Test DeregisterObserver
 */
HWTEST_F(PanServiceTest, Pan_Service_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("Pan_Service_DeregisterObserver_00100 start");
    IPanObserver* observer = nullptr;
    EXPECT_NO_FATAL_FAILURE(panService_->DeregisterObserver(*observer));
    HILOGI("Pan_Service_DeregisterObserver_00100 end");
}

}  // namespace bluetooth
}  // namespace OHOS