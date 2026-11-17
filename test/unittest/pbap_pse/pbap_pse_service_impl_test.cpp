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
#include "adapter_device_config.h"
#include "pbap_pse_service_impl.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"
#include "class_creator.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
const int TIME = 4;

class PbapPseServiceImplTest : public testing::Test {
public:
    PbapPseServiceImplTest();
    ~PbapPseServiceImplTest();

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
    }
    static void TearDownTestCase()
    {
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }

private:
    std::string addr_ = "11:22:33:44:55:66";
    PbapPseServiceImpl *PbapPseServiceImplTest_ = nullptr;
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
};

PbapPseServiceImplTest::PbapPseServiceImplTest()
{
    PbapPseServiceImplTest_ = new PbapPseServiceImpl;
    socketDevice_ = std::make_shared<ObexSocketDevice>(-1, 0, 0, SocketType::TYPE_RFCOMM, addr_);

    std::shared_ptr<SafeMap<const std::string, int64_t>> permissionRequestTimeMap_ =
        std::make_shared<SafeMap<const std::string, int64_t>>();\
    PbapPseServiceImplTest_->DllRegisterFunc(nullptr, permissionRequestTimeMap_);
}

PbapPseServiceImplTest::~PbapPseServiceImplTest()
{
    PbapPseServiceImplTest_->stateMachines_.clear();
    PbapPseServiceImplTest_ = nullptr;
    socketDevice_ = nullptr;
    delete PbapPseServiceImplTest_;
}

/*
 * @tc.number: PbapPse_Service_Impl_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_Disconnect_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_Disconnect_00100 start");
    const RawAddress device = RawAddress(addr_);

    PbapPseServiceImplTest_->stateMachines_[addr_] = nullptr;
    EXPECT_EQ(PbapPseServiceImplTest_->Disconnect(device), BT_ERR_INTERNAL_ERROR);

    PbapPseServiceImplTest_->stateMachines_[addr_] = std::make_unique<PbapPseStateMachine>(socketDevice_,
        PbapPseServiceImplTest_);
    EXPECT_EQ(PbapPseServiceImplTest_->Disconnect(device), BT_ERR_INTERNAL_ERROR);
    HILOGI("PbapPse_Service_Impl_Disconnect_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Impl_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_GetDevicesByStates_00100 start");
    std::vector<int> states = {0};
    PbapPseServiceImplTest_->stateMachines_.clear();
    PbapPseServiceImplTest_->stateMachines_[addr_] = nullptr;
    std::vector<RawAddress> devices = PbapPseServiceImplTest_->GetDevicesByStates(states);
    EXPECT_EQ(devices.size(), 0);

    PbapPseServiceImplTest_->stateMachines_[addr_] = std::make_unique<PbapPseStateMachine>(socketDevice_,
        PbapPseServiceImplTest_);
    devices = PbapPseServiceImplTest_->GetDevicesByStates(states);
    EXPECT_EQ(devices.size(), 0);

    states = {1};
    devices = PbapPseServiceImplTest_->GetDevicesByStates(states);
    EXPECT_EQ(devices.size(), 1);
    HILOGI("PbapPse_Service_Impl_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: PbapPse_Service_GetDeviceState_00101
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_GetDeviceState_00100 start");
    const RawAddress device = RawAddress(addr_);
    PbapPseServiceImplTest_->stateMachines_.clear();
    EXPECT_EQ(PbapPseServiceImplTest_->GetDeviceState(device), static_cast<int>(BTConnectState::DISCONNECTED));

    PbapPseServiceImplTest_->stateMachines_[addr_] = nullptr;
    EXPECT_EQ(PbapPseServiceImplTest_->GetDeviceState(device), static_cast<int>(BTConnectState::DISCONNECTED));

    PbapPseServiceImplTest_->stateMachines_[addr_] = std::make_unique<PbapPseStateMachine>(socketDevice_,
        PbapPseServiceImplTest_);
    EXPECT_EQ(PbapPseServiceImplTest_->GetDeviceState(device), static_cast<int>(BTConnectState::DISCONNECTED));
    HILOGI("PbapPse_Service_Impl_GetDeviceState_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Impl_GetConnectState_00100
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_GetConnectState_00100 start");
    PbapPseServiceImplTest_->stateMachines_.clear();
    EXPECT_EQ(PbapPseServiceImplTest_->GetConnectState(), 0);

    PbapPseServiceImplTest_->stateMachines_[addr_] = nullptr;
    EXPECT_EQ(PbapPseServiceImplTest_->GetConnectState(), PROFILE_STATE_DISCONNECTED);

    PbapPseServiceImplTest_->stateMachines_[addr_] = std::make_unique<PbapPseStateMachine>(socketDevice_,
        PbapPseServiceImplTest_);
    EXPECT_EQ(PbapPseServiceImplTest_->GetConnectState(), PROFILE_STATE_DISCONNECTED);
    HILOGI("PbapPse_Service_Impl_GetConnectState_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Impl_ProcessEvent_00100
 * @tc.name: ProcessEvent
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_ProcessEvent_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_ProcessEvent_00100 start");
    PbapPseServiceImplTest_->stateMachines_.clear();
    PbapPseMessage eventRemove;
    eventRemove.dev_ = addr_;
    eventRemove.what_ = PBAP_PSE_REMOVE_STATE_MACHINE_EVT;
    PbapPseServiceImplTest_->ProcessEvent(eventRemove);
    eventRemove.what_ = PBAP_PSE_CONNECT_EVT;
    PbapPseServiceImplTest_->ProcessEvent(eventRemove);
    eventRemove.what_ = PBAP_PSE_REQUEST_PERMISSION_EVT;
    PbapPseServiceImplTest_->ProcessEvent(eventRemove);
    eventRemove.what_ = PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT;
    PbapPseServiceImplTest_->ProcessEvent(eventRemove);
    PbapPseServiceImplTest_->stateMachines_.clear();
    eventRemove.what_ = PBAP_PSE_VERIFY_RESULT_EVT;
    PbapPseServiceImplTest_->ProcessEvent(eventRemove);
    eventRemove.what_ = PBAP_PSE_DISCONNECT_EVT;
    PbapPseServiceImplTest_->ProcessEvent(eventRemove);
    eventRemove.what_ = PBAP_PSE_STATE_CHANGED;
    PbapPseServiceImplTest_->ProcessEvent(eventRemove);
    eventRemove.what_ = PBAP_PSE_STATE_CHANGED;
    eventRemove.state_ = PBAP_PSE_STATE_DISCONNECTED;
    PbapPseServiceImplTest_->ProcessEvent(eventRemove);
    PbapPseServiceImplTest_->NotifyStateChanged(RawAddress(addr_), PBAP_PSE_STATE_CONNECTED);
    PbapPseServiceImplTest_->NotifyStateChanged(RawAddress(addr_), 3);
    EXPECT_NE(PbapPseServiceImplTest_->permissionRequestTimeMap_, nullptr);
    HILOGI("PbapPse_Service_Impl_ProcessEvent_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Impl_PhoneBookAccessAuthorization_00100
 * @tc.name: PhoneBookAccessAuthorization
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_PhoneBookAccessAuthorization_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_PhoneBookAccessAuthorization_00100 start");
    int32_t accessAuthorization = 0;
    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), accessAuthorization);

    accessAuthorization = 1;
    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), accessAuthorization);

    accessAuthorization = 2;
    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), accessAuthorization);

    EXPECT_NE(PbapPseServiceImplTest_->permissionRequestTimeMap_, nullptr);
    HILOGI("PbapPse_Service_Impl_PhoneBookAccessAuthorization_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Impl_IsNeedSetPermissionToUnknown_00100
 * @tc.name: IsNeedSetPermissionToUnknown
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_IsNeedSetPermissionToUnknown_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_IsNeedSetPermissionToUnknown_00100 start");
    PbapPseServiceImplTest_->stateMachines_.clear();
    SetPbapRejectCount(addr_, 0);
    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), 2);
    EXPECT_FALSE(PbapPseServiceImplTest_->IsNeedSetPermissionToUnknown(addr_, 0));
    EXPECT_TRUE(PbapPseServiceImplTest_->IsNeedSetPermissionToUnknown(addr_, 2));

    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), 2);
    EXPECT_TRUE(PbapPseServiceImplTest_->IsNeedSetPermissionToUnknown(addr_, 2));
    HILOGI("PbapPse_Service_Impl_IsNeedSetPermissionToUnknown_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Impl_CheckOrGetPermission_00100
 * @tc.name: CheckOrGetPermission
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_CheckOrGetPermission_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_CheckOrGetPermission_00100 start");
    PbapPseServiceImplTest_->stateMachines_.clear();
    std::shared_ptr<utility::Timer> timer = nullptr;
    SetPbapRejectCount(addr_, 0);
    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), 0);
    EXPECT_FALSE(PbapPseServiceImplTest_->IsNeedSetPermissionToUnknown(addr_, 0));
    PbapPseServiceImplTest_->CheckOrGetPermission(addr_, timer);

    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), 1);
    PbapPseServiceImplTest_->CheckOrGetPermission(addr_, timer);

    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), 2);
    PbapPseServiceImplTest_->CheckOrGetPermission(addr_, timer);

    PbapPseServiceImplTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), 2);
    PbapPseServiceImplTest_->CheckOrGetPermission(addr_, timer);
    HILOGI("PbapPse_Service_Impl_CheckOrGetPermission_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Impl_ProcessReqPermissionTimeOut_00100
 * @tc.name: ProcessReqPermissionTimeOut
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_ProcessReqPermissionTimeOut_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_ProcessReqPermissionTimeOut_00100 start");
    PbapPseMessage event(PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT);
    event.dev_ = addr_;
    PbapPseServiceImplTest_->ProcessReqPermissionTimeOut(event);

    PbapPseServiceImplTest_->DllRegisterFunc(nullptr, nullptr);
    EXPECT_EQ(PbapPseServiceImplTest_->dialogFunc_, nullptr);
    PbapPseServiceImplTest_->ProcessReqPermissionTimeOut(event);
    HILOGI("PbapPse_Service_Impl_ProcessReqPermissionTimeOut_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Impl_VerifyPermissionResult_00100
 * @tc.name: VerifyPermissionResult
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceImplTest, PbapPse_Service_Impl_VerifyPermissionResult_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Impl_VerifyPermissionResult_00100 start");
    PbapPseServiceImplTest_->DllRegisterFunc(nullptr, nullptr);
    PbapPseServiceImplTest_->VerifyPermissionResult(addr_, true);
    PbapPseServiceImplTest_->VerifyPermissionResult(addr_, false);

    std::shared_ptr<SafeMap<const std::string, int64_t>> permissionRequestTimeMap_ =
        std::make_shared<SafeMap<const std::string, int64_t>>();\
    PbapPseServiceImplTest_->DllRegisterFunc(nullptr, permissionRequestTimeMap_);
    PbapPseServiceImplTest_->VerifyPermissionResult(addr_, true);
    PbapPseServiceImplTest_->VerifyPermissionResult(addr_, false);
    EXPECT_NE(PbapPseServiceImplTest_->permissionRequestTimeMap_, nullptr);
    HILOGI("PbapPse_Service_Impl_VerifyPermissionResult_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS