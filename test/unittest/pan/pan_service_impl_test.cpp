/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#include <memory>
#include "pan_service_impl.h"
#include "pan_statemachine.h"
#include "pan_defines.h"
#include "raw_address.h"
#include "bluetooth_host.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

const int TIME = 4;
static BluetoothHost *host_;
static int g_notifyCallCount = 0;
static int g_postEventCallCount = 0;
static int g_getDeviceStateCallCount = 0;
static int g_setProactiveDisconnectFlagCallCount = 0;

class PanServiceImplTest : public testing::Test {
public:
    PanServiceImplTest();
    ~PanServiceImplTest();

    static void SetUpTestCase() {
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
    static void TearDownTestCase() {}
    void SetUp()
    {
        g_notifyCallCount = 0;
        g_postEventCallCount = 0;
        g_getDeviceStateCallCount = 0;
        g_setProactiveDisconnectFlagCallCount = 0;
    }
    void TearDown()
    {
        if (serviceImpl != nullptr) {
            serviceImpl->ClearStatemachine();
        }
    }

    PanServiceImpl* serviceImpl = nullptr;
};

PanServiceImplTest::PanServiceImplTest()
{
    serviceImpl = new PanServiceImpl();
}

PanServiceImplTest::~PanServiceImplTest()
{
    delete serviceImpl;
}


/*
 * @tc.number: Pan_Service_Impl_BringUpNetwork_00100
 * @tc.name: BringUpNetwork
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, BringUpNetwork, TestSize.Level1)
{
    int result = serviceImpl->BringUpNetwork();
    EXPECT_EQ(result, PAN_FAILURE);
}

/*
 * @tc.number: Pan_Service_Impl_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, GetDevicesByStates, TestSize.Level1)
{
    std::vector<int> states = {PAN_STATE_CONNECTED};
    auto devices = serviceImpl->GetDevicesByStates(states);
    EXPECT_TRUE(devices.empty());
}

/*
 * @tc.number: Pan_Service_Impl_GetConnectState_00100
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, GetConnectState, TestSize.Level1)
{
    int state = serviceImpl->GetConnectState();
    EXPECT_EQ(state, 0);
}

/*
 * @tc.number: Pan_Service_Impl_ProcessConnect_00100
 * @tc.name: ProcessConnect
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, ProcessConnect_NewDevice, TestSize.Level1)
{
    HILOGI("ProcessConnect_NewDevice start");
    serviceImpl->ClearStatemachine();
    EXPECT_TRUE(serviceImpl->IsStateMachineEmpty());
    
    PanMessage msg(PAN_CONNECT_EVT);
    msg.dev_ = "11:22:33:44:55:66";
    msg.localRole = LOCAL_NAP_ROLE;
    msg.remoteRole = REMOTE_PANU_ROLE;
    
    serviceImpl->ProcessConnect(msg);
    EXPECT_FALSE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("ProcessConnect_NewDevice end");
}

/*
 * @tc.number: Pan_Service_Impl_ProcessConnect_00200
 * @tc.name: ProcessConnect
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, ProcessConnect_ExistingDevice, TestSize.Level1)
{
    HILOGI("ProcessConnect_ExistingDevice start");
    serviceImpl->ClearStatemachine();
    PanMessage msg(PAN_CONNECT_EVT);
    msg.dev_ = "11:22:33:44:55:66";
    msg.localRole = LOCAL_NAP_ROLE;
    msg.remoteRole = REMOTE_PANU_ROLE;
    
    serviceImpl->ProcessConnect(msg);
    EXPECT_FALSE(serviceImpl->IsStateMachineEmpty());
    
    serviceImpl->ProcessConnect(msg);
    EXPECT_FALSE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("ProcessConnect_ExistingDevice end");
}

/*
 * @tc.number: Pan_Service_Impl_ProcessDisconnect_00100
 * @tc.name: ProcessDisconnect
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, ProcessDisconnect_Existing, TestSize.Level1)
{
    HILOGI("ProcessDisconnect_Existing start");
    serviceImpl->ClearStatemachine();
    PanMessage connectMsg(PAN_CONNECT_EVT);
    connectMsg.dev_ = "11:22:33:44:55:66";
    connectMsg.localRole = LOCAL_NAP_ROLE;
    connectMsg.remoteRole = REMOTE_PANU_ROLE;
    serviceImpl->ProcessConnect(connectMsg);
    
    auto setFlagFunc = [](const std::string& addr, uint32_t profileId, bool isProactive) {
        g_setProactiveDisconnectFlagCallCount++;
    };
    serviceImpl->DllSetDevProactiveDisconnectFlagFunc(setFlagFunc);
    
    PanMessage disconnectMsg(PAN_DISCONNECT_EVT);
    disconnectMsg.dev_ = "11:22:33:44:55:66";
    
    serviceImpl->ProcessDisconnect(disconnectMsg);
    EXPECT_EQ(g_setProactiveDisconnectFlagCallCount, 1);
    
    HILOGI("ProcessDisconnect_Existing end");
}

/*
 * @tc.number: Pan_Service_Impl_ProcessDisconnect_00200
 * @tc.name: ProcessDisconnect
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, ProcessDisconnect_NonExisting, TestSize.Level1)
{
    HILOGI("ProcessDisconnect_NonExisting start");
    serviceImpl->ClearStatemachine();
    EXPECT_TRUE(serviceImpl->IsStateMachineEmpty());
    
    PanMessage disconnectMsg(PAN_DISCONNECT_EVT);
    disconnectMsg.dev_ = "11:22:33:44:55:66";
    
    EXPECT_NO_FATAL_FAILURE(serviceImpl->ProcessDisconnect(disconnectMsg));
    EXPECT_TRUE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("ProcessDisconnect_NonExisting end");
}

/*
 * @tc.number: Pan_Service_Impl_ProcessConnectStateChange_00100
 * @tc.name: ProcessConnectStateChange
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, ProcessConnectStateChange_NewDevice, TestSize.Level1)
{
    HILOGI("ProcessConnectStateChange_NewDevice start");
    serviceImpl->ClearStatemachine();
    EXPECT_TRUE(serviceImpl->IsStateMachineEmpty());
    
    PanMessage stateChangeMsg(PAN_CONNECTED_IND_EVT);
    stateChangeMsg.dev_ = "11:22:33:44:55:66";
    stateChangeMsg.state = PAN_STATE_CONNECTED;
    stateChangeMsg.localRole = LOCAL_NAP_ROLE;
    stateChangeMsg.remoteRole = REMOTE_PANU_ROLE;
    
    serviceImpl->ProcessConnectStateChange(stateChangeMsg);
    EXPECT_FALSE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("ProcessConnectStateChange_NewDevice end");
}

/*
 * @tc.number: Pan_Service_Impl_ProcessConnectStateChange_00200
 * @tc.name: ProcessConnectStateChange
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, ProcessConnectStateChange_ExistingDevice, TestSize.Level1)
{
    HILOGI("ProcessConnectStateChange_ExistingDevice start");
    serviceImpl->ClearStatemachine();
    
    PanMessage connectMsg(PAN_CONNECT_EVT);
    connectMsg.dev_ = "11:22:33:44:55:66";
    connectMsg.localRole = LOCAL_NAP_ROLE;
    connectMsg.remoteRole = REMOTE_PANU_ROLE;
    serviceImpl->ProcessConnect(connectMsg);
    
    PanMessage stateChangeMsg(PAN_CONNECTED_IND_EVT);
    stateChangeMsg.dev_ = "11:22:33:44:55:66";
    stateChangeMsg.state = PAN_STATE_CONNECTED;
    stateChangeMsg.localRole = LOCAL_NAP_ROLE;
    stateChangeMsg.remoteRole = REMOTE_PANU_ROLE;
    
    serviceImpl->ProcessConnectStateChange(stateChangeMsg);
    EXPECT_FALSE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("ProcessConnectStateChange_ExistingDevice end");
}

/*
 * @tc.number: Pan_Service_Impl_RemoveStateMachine_00100
 * @tc.name: RemoveStateMachine
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, RemoveStateMachine, TestSize.Level1)
{
    HILOGI("RemoveStateMachine start");
    serviceImpl->ClearStatemachine();
    
    PanMessage msg(PAN_CONNECT_EVT);
    msg.dev_ = "11:22:33:44:55:66";
    msg.localRole = LOCAL_NAP_ROLE;
    msg.remoteRole = REMOTE_PANU_ROLE;
    serviceImpl->ProcessConnect(msg);
    EXPECT_FALSE(serviceImpl->IsStateMachineEmpty());
    
    serviceImpl->RemoveStateMachine(msg.dev_);
    EXPECT_TRUE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("RemoveStateMachine end");
}

/*
 * @tc.number: Pan_Service_Impl_IsStateMachineEmpty_00100
 * @tc.name: IsStateMachineEmpty
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, IsStateMachineEmpty_WhenEmpty, TestSize.Level1)
{
    HILOGI("IsStateMachineEmpty_WhenEmpty start");
    serviceImpl->ClearStatemachine();
    EXPECT_TRUE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("IsStateMachineEmpty_WhenEmpty end");
}

/*
 * @tc.number: Pan_Service_Impl_IsStateMachineEmpty_00200
 * @tc.name: IsStateMachineEmpty
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, IsStateMachineEmpty_WhenNotEmpty, TestSize.Level1)
{
    HILOGI("IsStateMachineEmpty_WhenNotEmpty start");
    serviceImpl->ClearStatemachine();
    
    PanMessage msg(PAN_CONNECT_EVT);
    msg.dev_ = "11:22:33:44:55:66";
    msg.localRole = LOCAL_NAP_ROLE;
    msg.remoteRole = REMOTE_PANU_ROLE;
    serviceImpl->ProcessConnect(msg);
    
    EXPECT_FALSE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("IsStateMachineEmpty_WhenNotEmpty end");
}

/*
 * @tc.number: Pan_Service_Impl_ClearStatemachine_00100
 * @tc.name: ClearStatemachine
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, ClearStatemachine, TestSize.Level1)
{
    HILOGI("ClearStatemachine start");
    PanMessage msg1(PAN_CONNECT_EVT);
    msg1.dev_ = "11:22:33:44:55:66";
    msg1.localRole = LOCAL_NAP_ROLE;
    msg1.remoteRole = REMOTE_PANU_ROLE;
    serviceImpl->ProcessConnect(msg1);
    
    PanMessage msg2(PAN_CONNECT_EVT);
    msg2.dev_ = "22:33:44:55:66:77";
    msg2.localRole = LOCAL_PANU_ROLE;
    msg2.remoteRole = REMOTE_NAP_ROLE;
    serviceImpl->ProcessConnect(msg2);
    
    EXPECT_FALSE(serviceImpl->IsStateMachineEmpty());
    
    serviceImpl->ClearStatemachine();
    EXPECT_TRUE(serviceImpl->IsStateMachineEmpty());
    
    HILOGI("ClearStatemachine end");
}

/*
 * @tc.number: Pan_Service_Impl_DllIsTetheringOnFunc_00100
 * @tc.name: DllIsTetheringOnFunc
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, DllIsTetheringOnFunc, TestSize.Level1)
{
    HILOGI("DllIsTetheringOnFunc start");
    bool isCalled = false;
    auto mockFunc = [&isCalled]() -> bool {
        isCalled = true;
        return true;
    };
    
    serviceImpl->DllIsTetheringOnFunc(mockFunc);
    EXPECT_TRUE(serviceImpl->IsTetheringOnFunc_ != nullptr);
    
    bool result = serviceImpl->IsTetheringOnFunc_();
    EXPECT_TRUE(isCalled);
    EXPECT_TRUE(result);
    
    HILOGI("DllIsTetheringOnFunc end");
}

/*
 * @tc.number: Pan_Service_Impl_DllNotifyStateChangedFunc_00100
 * @tc.name: DllNotifyStateChangedFunc
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, DllNotifyStateChangedFunc, TestSize.Level1)
{
    HILOGI("DllNotifyStateChangedFunc start");
    g_notifyCallCount = 0;
    auto mockFunc = [](const bluetooth::RawAddress& device, int state, int role) {
        g_notifyCallCount++;
    };
    
    serviceImpl->DllNotifyStateChangedFunc(mockFunc);
    EXPECT_TRUE(serviceImpl->NotifyStateChangedFunc_ != nullptr);
    
    bluetooth::RawAddress device("11:22:33:44:55:66");
    serviceImpl->NotifyStateChangedFunc_(device, PAN_STATE_CONNECTED, LOCAL_NAP_ROLE);
    EXPECT_EQ(g_notifyCallCount, 1);
    
    HILOGI("DllNotifyStateChangedFunc end");
}

/*
 * @tc.number: Pan_Service_Impl_DllPostEventFunc_00100
 * @tc.name: DllPostEventFunc
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, DllPostEventFunc, TestSize.Level1)
{
    HILOGI("DllPostEventFunc start");
    g_postEventCallCount = 0;
    auto mockFunc = [](const PanMessage& event) {
        g_postEventCallCount++;
    };
    
    serviceImpl->DllPostEventFunc(mockFunc);
    EXPECT_TRUE(serviceImpl->PostEventFunc_ != nullptr);
    
    PanMessage msg(PAN_CONNECT_EVT);
    serviceImpl->PostEventFunc_(msg);
    EXPECT_EQ(g_postEventCallCount, 1);
    
    HILOGI("DllPostEventFunc end");
}

/*
 * @tc.number: Pan_service_Impl_DllGetDeviceStateFunc_00100
 * @tc.name: DllGetDeviceStateFunc
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, DllGetDeviceStateFunc, TestSize.Level1)
{
    HILOGI("DllGetDeviceStateFunc start");
    g_getDeviceStateCallCount = 0;
    auto mockFunc = [](const bluetooth::RawAddress& device) -> int {
        g_getDeviceStateCallCount++;
        return PAN_STATE_CONNECTED;
    };
    
    serviceImpl->DllGetDeviceStateFunc(mockFunc);
    EXPECT_TRUE(serviceImpl->GetDeviceStateFunc_ != nullptr);
    
    bluetooth::RawAddress device("11:22:33:44:55:66");
    int state = serviceImpl->GetDeviceStateFunc_(device);
    EXPECT_EQ(g_getDeviceStateCallCount, 1);
    EXPECT_EQ(state, PAN_STATE_CONNECTED);
    
    HILOGI("DllGetDeviceStateFunc end");
}

/*
 * @tc.number: Pan_service_Impl_DllSetDevProactiveDisconnectFlagFunc_00100
 * @tc.name: DllSetDevProactiveDisconnectFlagFunc
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, DllSetDevProactiveDisconnectFlagFunc, TestSize.Level1)
{
    HILOGI("DllSetDevProactiveDisconnectFlagFunc start");
    g_setProactiveDisconnectFlagCallCount = 0;
    auto mockFunc = [](const std::string& addr, uint32_t profileId, bool isProactive) {
        g_setProactiveDisconnectFlagCallCount++;
    };
    
    serviceImpl->DllSetDevProactiveDisconnectFlagFunc(mockFunc);
    EXPECT_TRUE(serviceImpl->SetDevProactiveDisconnectFlagFunc_ != nullptr);
    
    serviceImpl->SetDevProactiveDisconnectFlagFunc_("11:22:33:44:55:66", 1, true);
    EXPECT_EQ(g_setProactiveDisconnectFlagCallCount, 1);
    
    HILOGI("DllSetDevProactiveDisconnectFlagFunc end");
}

/*
 * @tc.number: Pan_service_Impl_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(PanServiceImplTest, GetDevicesByStates_WithMockFunction, TestSize.Level1)
{
    HILOGI("GetDevicesByStates_WithMockFunction start");
    serviceImpl->ClearStatemachine();
    
    auto mockGetStateFunc = [](const bluetooth::RawAddress& device) -> int {
        return PAN_STATE_CONNECTED;
    };
    serviceImpl->DllGetDeviceStateFunc(mockGetStateFunc);
    std::vector<int> connectedStates = {PAN_STATE_CONNECTED};
    auto devices = serviceImpl->GetDevicesByStates(connectedStates);
    EXPECT_TRUE(devices.empty());
    HILOGI("GetDevicesByStates_WithMockFunction end");
}

}
}