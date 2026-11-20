/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <cstring>
#include "pbap_pce_service_impl.h"
#include "pbap_pce_state_machine.h"
#include "pbap_pce_app_params.h"
#include "pbap_pce_header_msg.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "profile_service_manager.h"
#include "raw_address.h"
#include "bt_def.h"
#include "bt_chr_dft_exception.h"
#include "thread_util.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

std::string g_pbapPceStmAddress("00:11:22:33:44:55");
const int PBAP_PCE_STM_TIME = 2;
static Bluetooth::BluetoothHost *g_pbapPceStmHost_ = nullptr;
static PbapPceServiceImpl *g_pbapPceStmServiceImpl_ = nullptr;
// weak_from_this（定时器/Entry 回调安全）要求对象由 shared_ptr 持有
static std::shared_ptr<PbapPceStateMachine> g_pbapPceStmStateMachine_ = nullptr;

class PbapPceStatemachineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("PbapPceStatemachineTest SetUpTestCase start");
        g_pbapPceStmHost_ = &Bluetooth::BluetoothHost::GetDefaultHost();

        bool isBleEnable = g_pbapPceStmHost_->IsBleEnabled();
        bool isBrEnable = g_pbapPceStmHost_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            g_pbapPceStmHost_->EnableBle();
            g_pbapPceStmHost_->EnableBt();
            sleep(PBAP_PCE_STM_TIME);
        }

        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_PBAP, ThreadUtil::DISABLED);

        g_pbapPceStmServiceImpl_ = new PbapPceServiceImpl();
        g_pbapPceStmStateMachine_ = std::make_shared<PbapPceStateMachine>(
            RawAddress(g_pbapPceStmAddress), *g_pbapPceStmServiceImpl_,
            *g_pbapPceStmServiceImpl_->pbapPceSdp_, nullptr);

        auto manager = ProfileServiceManager::GetInstance();
        manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_PBAP_PCE);

        HILOGI("PbapPceStatemachineTest SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("PbapPceStatemachineTest TearDownTestCase start");
        SdpAdapter::GetInstance().SetPbapPceSdpFoundCallback(nullptr);
        sleep(PBAP_PCE_STM_TIME);
        g_pbapPceStmStateMachine_.reset();
        if (g_pbapPceStmServiceImpl_ != nullptr) {
            g_pbapPceStmServiceImpl_->machineMap_.clear();
            delete g_pbapPceStmServiceImpl_;
            g_pbapPceStmServiceImpl_ = nullptr;
        }
        g_pbapPceStmHost_ = nullptr;
        HILOGI("PbapPceStatemachineTest TearDownTestCase end");
    }
    void SetUp()
    {
        disconnectedState =
            std::make_unique<PceDisconnectedState>(PCE_DISCONNECTED_STATE, *g_pbapPceStmStateMachine_, nullptr);
        connectingState =
            std::make_unique<PceConnectingState>(PCE_CONNECTING_STATE, *g_pbapPceStmStateMachine_, nullptr);
        connectedState =
            std::make_unique<PceConnectedState>(PCE_CONNECTED_STATE, *g_pbapPceStmStateMachine_, nullptr);
        disconnectingState =
            std::make_unique<PceDisconnectingState>(PCE_DISCONNECTING_STATE, *g_pbapPceStmStateMachine_, nullptr);
    }
    void TearDown()
    {
        disconnectedState.reset();
        connectingState.reset();
        connectedState.reset();
        disconnectingState.reset();
    }

public:
    std::unique_ptr<PceDisconnectedState> disconnectedState;
    std::unique_ptr<PceConnectingState> connectingState;
    std::unique_ptr<PceConnectedState> connectedState;
    std::unique_ptr<PceDisconnectingState> disconnectingState;
};

/**
 * @tc.number: Pbap_Pce_Statemachine_GetDevice_00100
 * @tc.name: GetDevice
 * @tc.desc: Verify GetDevice returns the device passed to constructor.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_GetDevice_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_GetDevice_00100 start");
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetDevice().GetAddress(), g_pbapPceStmAddress);
    HILOGI("Pbap_Pce_Statemachine_GetDevice_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_GetPceService_00100
 * @tc.name: GetPceService
 * @tc.desc: Verify GetPceService returns the service passed to constructor.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_GetPceService_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_GetPceService_00100 start");
    EXPECT_EQ(&g_pbapPceStmStateMachine_->GetPceService(), g_pbapPceStmServiceImpl_);
    HILOGI("Pbap_Pce_Statemachine_GetPceService_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_VersionNumber_00100
 * @tc.name: SetVersionNumber and GetVersionNumber
 * @tc.desc: Verify Set and Get version number.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_VersionNumber_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_VersionNumber_00100 start");
    g_pbapPceStmStateMachine_->SetVersionNumber(0x0102);
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetVersionNumber(), 0x0102);
    HILOGI("Pbap_Pce_Statemachine_VersionNumber_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_SupportedRes_00100
 * @tc.name: SetSupportedRes and GetSupportedRes
 * @tc.desc: Verify Set and Get supported repositories.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_SupportedRes_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_SupportedRes_00100 start");
    g_pbapPceStmStateMachine_->SetSupportedRes(0x03);
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetSupportedRes(), 0x03);
    HILOGI("Pbap_Pce_Statemachine_SupportedRes_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_SupportedFeature_00100
 * @tc.name: SetSupportedFeature and GetSupportedFeature
 * @tc.desc: Verify Set and Get supported features.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_SupportedFeature_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_SupportedFeature_00100 start");
    g_pbapPceStmStateMachine_->SetSupportedFeature(0x000000FF);
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetSupportedFeature(), 0x000000FF);
    HILOGI("Pbap_Pce_Statemachine_SupportedFeature_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_FeatureFlag_00100
 * @tc.name: SetFeatureFlag and GetFeatureFlag
 * @tc.desc: Verify Set and Get feature flag.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_FeatureFlag_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_FeatureFlag_00100 start");
    g_pbapPceStmStateMachine_->SetFeatureFlag(true);
    EXPECT_TRUE(g_pbapPceStmStateMachine_->GetFeatureFlag());
    g_pbapPceStmStateMachine_->SetFeatureFlag(false);
    EXPECT_FALSE(g_pbapPceStmStateMachine_->GetFeatureFlag());
    HILOGI("Pbap_Pce_Statemachine_FeatureFlag_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_ConnectId_00100
 * @tc.name: SetConnectId
 * @tc.desc: Verify SetConnectId does not crash.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ConnectId_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ConnectId_00100 start");
    g_pbapPceStmStateMachine_->SetConnectId(0x1234);
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetConnectId(), 0x1234U);
    HILOGI("Pbap_Pce_Statemachine_ConnectId_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_TargetState_00100
 * @tc.name: SetTargetState and GetTargetState
 * @tc.desc: Verify Set and Get target state.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_TargetState_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_TargetState_00100 start");
    g_pbapPceStmStateMachine_->SetTargetState(PCE_TARGET_STATE_CONNECTED);
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetTargetState(), PCE_TARGET_STATE_CONNECTED);
    g_pbapPceStmStateMachine_->SetTargetState(PCE_TARGET_STATE_DISCONNECTED);
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetTargetState(), PCE_TARGET_STATE_DISCONNECTED);
    HILOGI("Pbap_Pce_Statemachine_TargetState_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_IsBusy_00100
 * @tc.name: IsBusy with null obexClient
 * @tc.desc: Verify IsBusy returns false when obexClient is null.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_IsBusy_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_IsBusy_00100 start");
    g_pbapPceStmStateMachine_->ResetObexClient();
    EXPECT_FALSE(g_pbapPceStmStateMachine_->IsBusy());
    HILOGI("Pbap_Pce_Statemachine_IsBusy_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_SetBusy_00100
 * @tc.name: SetBusy with null obexClient
 * @tc.desc: Verify SetBusy returns false when obexClient is null.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_SetBusy_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_SetBusy_00100 start");
    g_pbapPceStmStateMachine_->ResetObexClient();
    EXPECT_FALSE(g_pbapPceStmStateMachine_->SetBusy(true));
    HILOGI("Pbap_Pce_Statemachine_SetBusy_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_AbortDownloading_00100
 * @tc.name: AbortDownloading with null obexClient
 * @tc.desc: Verify AbortDownloading returns RET_BAD_STATUS when obexClient is null.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_AbortDownloading_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_AbortDownloading_00100 start");
    g_pbapPceStmStateMachine_->ResetObexClient();
    EXPECT_EQ(g_pbapPceStmStateMachine_->AbortDownloading(), RET_BAD_STATUS);
    HILOGI("Pbap_Pce_Statemachine_AbortDownloading_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_ForceCloseObexClient_00100
 * @tc.name: ForceCloseObexClient with null obexClient
 * @tc.desc: Verify ForceCloseObexClient does not crash when obexClient is null.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ForceCloseObexClient_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ForceCloseObexClient_00100 start");
    g_pbapPceStmStateMachine_->ResetObexClient();
    g_pbapPceStmStateMachine_->ForceCloseObexClient();
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetObexClientPtr(), nullptr);
    HILOGI("Pbap_Pce_Statemachine_ForceCloseObexClient_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_ResetObexClient_00100
 * @tc.name: ResetObexClient
 * @tc.desc: Verify ResetObexClient sets obexClient to nullptr.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ResetObexClient_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ResetObexClient_00100 start");
    g_pbapPceStmStateMachine_->ResetObexClient();
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetObexClientPtr(), nullptr);
    HILOGI("Pbap_Pce_Statemachine_ResetObexClient_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_DisconnectedState_Dispatch_00100
 * @tc.name: PceDisconnectedState Dispatch
 * @tc.desc: Verify PceDisconnectedState Dispatch handles various messages.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_DisconnectedState_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_DisconnectedState_Dispatch_00100 start");
    utility::Message msgSetTarget(PCE_REQ_SET_TARGET_STATE, PCE_TARGET_STATE_CONNECTED);
    EXPECT_TRUE(disconnectedState->Dispatch(msgSetTarget));

    utility::Message msgTransit(PCE_REQ_TRANSIT_TARGET_STATE);
    EXPECT_TRUE(disconnectedState->Dispatch(msgTransit));

    utility::Message msgUnknown(0xFFFF);
    EXPECT_TRUE(disconnectedState->Dispatch(msgUnknown));
    HILOGI("Pbap_Pce_Statemachine_DisconnectedState_Dispatch_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_ConnectingState_Dispatch_00100
 * @tc.name: PceConnectingState Dispatch
 * @tc.desc: Verify PceConnectingState Dispatch handles various messages.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ConnectingState_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ConnectingState_Dispatch_00100 start");
    utility::Message msgSetTarget(PCE_REQ_SET_TARGET_STATE, PCE_TARGET_STATE_DISCONNECTED);
    EXPECT_TRUE(connectingState->Dispatch(msgSetTarget));

    utility::Message msgTransit(PCE_REQ_TRANSIT_TARGET_STATE);
    EXPECT_TRUE(connectingState->Dispatch(msgTransit));

    utility::Message msgUnknown(0xFFFF);
    EXPECT_FALSE(connectingState->Dispatch(msgUnknown));
    HILOGI("Pbap_Pce_Statemachine_ConnectingState_Dispatch_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_ConnectedState_Dispatch_00100
 * @tc.name: PceConnectedState Dispatch
 * @tc.desc: Verify PceConnectedState Dispatch handles various messages.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ConnectedState_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ConnectedState_Dispatch_00100 start");
    utility::Message msgSetTarget(PCE_REQ_SET_TARGET_STATE, PCE_TARGET_STATE_DISCONNECTED);
    EXPECT_TRUE(connectedState->Dispatch(msgSetTarget));

    utility::Message msgTransit(PCE_REQ_TRANSIT_TARGET_STATE);
    EXPECT_TRUE(connectedState->Dispatch(msgTransit));

    utility::Message msgAbort(PCE_REQ_ABORTDOWNLOADING);
    EXPECT_TRUE(connectedState->Dispatch(msgAbort));

    utility::Message msgDownloadComplete(PCE_DOWNLOAD_COMPLETE);
    EXPECT_TRUE(connectedState->Dispatch(msgDownloadComplete));
    HILOGI("Pbap_Pce_Statemachine_ConnectedState_Dispatch_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_DisconnectingState_Dispatch_00100
 * @tc.name: PceDisconnectingState Dispatch
 * @tc.desc: Verify PceDisconnectingState Dispatch handles various messages.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_DisconnectingState_Dispatch_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_DisconnectingState_Dispatch_00100 start");
    utility::Message msgSetTarget(PCE_REQ_SET_TARGET_STATE, PCE_TARGET_STATE_CONNECTED);
    EXPECT_TRUE(disconnectingState->Dispatch(msgSetTarget));

    utility::Message msgTransit(PCE_REQ_TRANSIT_TARGET_STATE);
    EXPECT_TRUE(disconnectingState->Dispatch(msgTransit));

    utility::Message msgDownloadComplete(PCE_DOWNLOAD_COMPLETE);
    EXPECT_TRUE(disconnectingState->Dispatch(msgDownloadComplete));

    utility::Message msgUnknown(0xFFFF);
    EXPECT_FALSE(disconnectingState->Dispatch(msgUnknown));
    HILOGI("Pbap_Pce_Statemachine_DisconnectingState_Dispatch_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_TryReleasePbapMsg_00100
 * @tc.name: TryReleasePbapMsg
 * @tc.desc: Verify TryReleasePbapMsg releases payload only for SDP/OBEX msg types.
 */
namespace {
class TestSdpMsg : public PbapPceHeaderSdpMsg {
public:
    static int dtorCount_;
    TestSdpMsg() : PbapPceHeaderSdpMsg(cfg_, 0, 0, 0, false) {}
    ~TestSdpMsg() override
    {
        dtorCount_++;
    }

private:
    static PbapPceObexConfig cfg_;
};
int TestSdpMsg::dtorCount_ = 0;
PbapPceObexConfig TestSdpMsg::cfg_ {};
}  // namespace

HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_TryReleasePbapMsg_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_TryReleasePbapMsg_00100 start");
    // unknown msg：不释放 payload
    TestSdpMsg::dtorCount_ = 0;
    TestSdpMsg notReleased;
    utility::Message msgUnknown(0xFFFF, 0, &notReleased);
    g_pbapPceStmStateMachine_->TryReleasePbapMsg(msgUnknown);
    EXPECT_EQ(TestSdpMsg::dtorCount_, 0);
    // PCE_SDP_FINISH：释放 payload（虚析构置标志）
    utility::Message msgSdpFinish(PCE_SDP_FINISH, 0, new TestSdpMsg());
    g_pbapPceStmStateMachine_->TryReleasePbapMsg(msgSdpFinish);
    EXPECT_EQ(TestSdpMsg::dtorCount_, 1);
    HILOGI("Pbap_Pce_Statemachine_TryReleasePbapMsg_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_PullPhoneBook_00100
 * @tc.name: PullPhoneBook with null obexClient
 * @tc.desc: Verify PullPhoneBook with null obexClient does not crash.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_PullPhoneBook_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_PullPhoneBook_00100 start");
    g_pbapPceStmStateMachine_->ResetObexClient();
    PbapPceAppParams appParams;
    g_pbapPceStmStateMachine_->PullPhoneBook(appParams);
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetObexClientPtr(), nullptr);
    HILOGI("Pbap_Pce_Statemachine_PullPhoneBook_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_TransitTargetState_00100
 * @tc.name: TransitTargetState with UNKNOWN target
 * @tc.desc: Verify TransitTargetState with UNKNOWN target does not crash.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_TransitTargetState_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_TransitTargetState_00100 start");
    g_pbapPceStmStateMachine_->SetTargetState(PCE_TARGET_STATE_UNKNOWN);
    g_pbapPceStmStateMachine_->TransitTargetState();
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetTargetState(), PCE_TARGET_STATE_UNKNOWN);
    HILOGI("Pbap_Pce_Statemachine_TransitTargetState_00100 end");
}
/**
 * @tc.number: Pbap_Pce_Statemachine_ConnectFailReason_00100
 * @tc.name: SetConnectFailReason and ClearConnectFailed
 * @tc.desc: Verify connect fail reason is stored, and cleared together with connect failed flag.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ConnectFailReason_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ConnectFailReason_00100 start");
    g_pbapPceStmStateMachine_->SetConnectFailReason(PBAP_PCE_CONN_FAIL_TIMEOUT);
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetConnectFailReason(), PBAP_PCE_CONN_FAIL_TIMEOUT);
    g_pbapPceStmStateMachine_->MarkConnectFailed();
    EXPECT_TRUE(g_pbapPceStmStateMachine_->IsConnectFailed());
    g_pbapPceStmStateMachine_->ClearConnectFailed();
    EXPECT_FALSE(g_pbapPceStmStateMachine_->IsConnectFailed());
    EXPECT_EQ(g_pbapPceStmStateMachine_->GetConnectFailReason(), PBAP_PCE_CONN_FAIL_NONE);
    HILOGI("Pbap_Pce_Statemachine_ConnectFailReason_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_ContactsConnecting_00100
 * @tc.name: RecordContactsConnecting
 * @tc.desc: Verify connecting time is recorded and sync fields are reset when a new connection starts.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ContactsConnecting_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ContactsConnecting_00100 start");
    g_pbapPceStmStateMachine_->RecordContactsConnecting();
    EXPECT_GT(g_pbapPceStmStateMachine_->chrConnConnectingTime_, 0);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrConnConnectedTime_, 0);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncStartTime_, 0);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncEndTime_, 0);
    EXPECT_FALSE(g_pbapPceStmStateMachine_->chrSyncStarted_);
    EXPECT_FALSE(g_pbapPceStmStateMachine_->chrSyncFailed_);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncFailReason_, PBAP_PCE_SYNC_FAIL_NONE);
    HILOGI("Pbap_Pce_Statemachine_ContactsConnecting_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_ContactsSync_00100
 * @tc.name: MarkChrSyncStart and MarkChrSyncFinish
 * @tc.desc: Verify sync start/finish times, result and fail reason are recorded.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ContactsSync_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ContactsSync_00100 start");
    g_pbapPceStmStateMachine_->RecordContactsConnecting();
    g_pbapPceStmStateMachine_->MarkChrConnConnected();
    g_pbapPceStmStateMachine_->MarkChrSyncStart();
    EXPECT_GT(g_pbapPceStmStateMachine_->chrConnConnectedTime_, 0);
    EXPECT_TRUE(g_pbapPceStmStateMachine_->chrSyncStarted_);
    EXPECT_GT(g_pbapPceStmStateMachine_->chrSyncStartTime_, 0);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncEndTime_, 0);

    g_pbapPceStmStateMachine_->MarkChrSyncFinish(true, PBAP_PCE_SYNC_FAIL_OBEX_ERR);
    EXPECT_TRUE(g_pbapPceStmStateMachine_->chrSyncFailed_);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncFailReason_, PBAP_PCE_SYNC_FAIL_OBEX_ERR);
    EXPECT_GT(g_pbapPceStmStateMachine_->chrSyncEndTime_, 0);

    g_pbapPceStmStateMachine_->RecordContactsSyncFinish(false, PBAP_PCE_SYNC_FAIL_NONE);
    EXPECT_FALSE(g_pbapPceStmStateMachine_->chrSyncFailed_);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncFailReason_, PBAP_PCE_SYNC_FAIL_NONE);
    HILOGI("Pbap_Pce_Statemachine_ContactsSync_00100 end");
}

/**
 * @tc.number: Pbap_Pce_Statemachine_ReportContactsSynUeEvent_00100
 * @tc.name: ReportContactsSynUeEvent
 * @tc.desc: Verify CONTACTS_SYN_STATUS_RECORD is reported and local record is reset after reporting.
 */
HWTEST_F(PbapPceStatemachineTest, Pbap_Pce_Statemachine_ReportContactsSynUeEvent_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Statemachine_ReportContactsSynUeEvent_00100 start");
    g_pbapPceStmStateMachine_->RecordContactsConnecting();
    g_pbapPceStmStateMachine_->MarkChrConnConnected();
    g_pbapPceStmStateMachine_->MarkChrSyncStart();
    g_pbapPceStmStateMachine_->RecordContactsSyncFinish(false, PBAP_PCE_SYNC_FAIL_NONE);
    g_pbapPceStmStateMachine_->SetConnectFailReason(PBAP_PCE_CONN_FAIL_TRANSPORT);
    g_pbapPceStmStateMachine_->MarkConnectFailed();

    g_pbapPceStmStateMachine_->ReportContactsSynUeEvent(true,
        g_pbapPceStmStateMachine_->GetConnectFailReason());
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrConnConnectingTime_, 0);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrConnConnectedTime_, 0);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncStartTime_, 0);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncEndTime_, 0);
    EXPECT_FALSE(g_pbapPceStmStateMachine_->chrSyncStarted_);
    EXPECT_FALSE(g_pbapPceStmStateMachine_->chrSyncFailed_);
    EXPECT_EQ(g_pbapPceStmStateMachine_->chrSyncFailReason_, PBAP_PCE_SYNC_FAIL_NONE);
    g_pbapPceStmStateMachine_->ClearConnectFailed();
    HILOGI("Pbap_Pce_Statemachine_ReportContactsSynUeEvent_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS
