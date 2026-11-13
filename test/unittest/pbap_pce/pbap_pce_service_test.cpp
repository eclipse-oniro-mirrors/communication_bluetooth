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
#include <cstring>
#include "pbap_pce_service.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "profile_service_manager.h"
#include "raw_address.h"
#include "thread_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

std::string g_pbapPceSvcAddress("00:11:22:33:44:55");
const int PBAP_PCE_SVC_TIME = 2;
static BluetoothHost *g_pbapPceSvcHost_ = nullptr;
static PbapPceService *g_pbapPceService_ = nullptr;

class PbapPceServiceObserverTest : public IPbapPceObserver {
public:
    PbapPceServiceObserverTest() = default;
    virtual ~PbapPceServiceObserverTest() = default;
    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {
        connStateCount_++;
    }
    void OnSyncStateChange(const RawAddress &device, int syncState) override
    {
        syncStateCount_++;
    }
    int GetConnStateCount() const
    {
        return connStateCount_;
    }
    void ResetCount()
    {
        connStateCount_ = 0;
        syncStateCount_ = 0;
    }

private:
    int connStateCount_ = 0;
    int syncStateCount_ = 0;
};

static PbapPceServiceObserverTest g_pbapPceSvcObserver;

class PbapPceServiceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("PbapPceServiceTest SetUpTestCase start");
        g_pbapPceSvcHost_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = g_pbapPceSvcHost_->IsBleEnabled();
        bool isBrEnable = g_pbapPceSvcHost_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            g_pbapPceSvcHost_->EnableBle();
            g_pbapPceSvcHost_->EnableBt();
            sleep(PBAP_PCE_SVC_TIME);
        }

        HILOGI("PbapPceServiceTest SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("PbapPceServiceTest TearDownTestCase start");
        g_pbapPceSvcHost_ = nullptr;
        HILOGI("PbapPceServiceTest TearDownTestCase end");
    }
    void SetUp()
    {
        g_pbapPceService_ = new PbapPceService();
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_PBAP, ThreadUtil::DISABLED);
        sleep(1);
    }
    void TearDown()
    {
        sleep(1);
        if (g_pbapPceService_ != nullptr) {
            delete g_pbapPceService_;
            g_pbapPceService_ = nullptr;
        }
    }
};

/*
 * @tc.number: Pbap_Pce_Service_Connect_00100
 * @tc.name: Connect without loading impl
 * @tc.desc: Verify Connect returns BT_ERR_INTERNAL_ERROR when impl library is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_Connect_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_Connect_00100 start");
    EXPECT_EQ(g_pbapPceService_->Connect(RawAddress(g_pbapPceSvcAddress)), BT_ERR_INTERNAL_ERROR);
    HILOGI("Pbap_Pce_Service_Connect_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_Disconnect_00100
 * @tc.name: Disconnect without loading impl
 * @tc.desc: Verify Disconnect returns BT_ERR_INTERNAL_ERROR when impl library is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_Disconnect_00100 start");
    EXPECT_EQ(g_pbapPceService_->Disconnect(RawAddress(g_pbapPceSvcAddress)), BT_ERR_INTERNAL_ERROR);
    HILOGI("Pbap_Pce_Service_Disconnect_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_GetDeviceState_00100
 * @tc.name: GetDeviceState without loading impl
 * @tc.desc: Verify GetDeviceState returns DISCONNECTED when impl library is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_GetDeviceState_00100 start");
    EXPECT_EQ(g_pbapPceService_->GetDeviceState(RawAddress(g_pbapPceSvcAddress)),
        static_cast<int>(BTConnectState::DISCONNECTED));
    HILOGI("Pbap_Pce_Service_GetDeviceState_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates without loading impl
 * @tc.desc: Verify GetDevicesByStates returns empty when impl library is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_GetDevicesByStates_00100 start");
    std::vector<int> states = {0};
    std::vector<RawAddress> devices = g_pbapPceService_->GetDevicesByStates(states);
    EXPECT_EQ(devices.size(), 0u);
    HILOGI("Pbap_Pce_Service_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_GetConnectState_00100
 * @tc.name: GetConnectState without loading impl
 * @tc.desc: Verify GetConnectState returns 0 when impl library is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_GetConnectState_00100 start");
    EXPECT_EQ(g_pbapPceService_->GetConnectState(), 0);
    HILOGI("Pbap_Pce_Service_GetConnectState_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_GetConnectDevices_00100
 * @tc.name: GetConnectDevices without loading impl
 * @tc.desc: Verify GetConnectDevices returns empty when impl library is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_GetConnectDevices_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_GetConnectDevices_00100 start");
    std::list<RawAddress> devices = g_pbapPceService_->GetConnectDevices();
    EXPECT_EQ(devices.size(), 0u);
    HILOGI("Pbap_Pce_Service_GetConnectDevices_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_SetConnectionStrategy_00100
 * @tc.name: SetConnectionStrategy without loading impl
 * @tc.desc: Verify SetConnectionStrategy returns BT_ERR_INTERNAL_ERROR when impl is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_SetConnectionStrategy_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_SetConnectionStrategy_00100 start");
    EXPECT_EQ(g_pbapPceService_->SetConnectionStrategy(RawAddress(g_pbapPceSvcAddress), 0), BT_ERR_INTERNAL_ERROR);
    HILOGI("Pbap_Pce_Service_SetConnectionStrategy_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_GetConnectionStrategy_00100
 * @tc.name: GetConnectionStrategy without loading impl
 * @tc.desc: Verify GetConnectionStrategy returns BT_ERR_INTERNAL_ERROR when impl is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_GetConnectionStrategy_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_GetConnectionStrategy_00100 start");
    EXPECT_EQ(g_pbapPceService_->GetConnectionStrategy(RawAddress(g_pbapPceSvcAddress)), BT_ERR_INTERNAL_ERROR);
    HILOGI("Pbap_Pce_Service_GetConnectionStrategy_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_GetPhoneBookSyncState_00100
 * @tc.name: GetPhoneBookSyncState with no device in map
 * @tc.desc: Verify GetPhoneBookSyncState returns DISCONNECTED when device is not in map.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_GetPhoneBookSyncState_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_GetPhoneBookSyncState_00100 start");
    EXPECT_EQ(g_pbapPceService_->GetPhoneBookSyncState(RawAddress(g_pbapPceSvcAddress)),
        static_cast<int32_t>(PBAP_PCE_PHONEBOOK_STATE_DISCONNECTED));
    HILOGI("Pbap_Pce_Service_GetPhoneBookSyncState_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_SetPhoneBookSyncState_00100
 * @tc.name: SetPhoneBookSyncState then GetPhoneBookSyncState
 * @tc.desc: Verify SetPhoneBookSyncState then GetPhoneBookSyncState returns the set value.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_SetPhoneBookSyncState_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_SetPhoneBookSyncState_00100 start");
    g_pbapPceService_->SetPhoneBookSyncState(g_pbapPceSvcAddress, PBAP_PCE_PHONEBOOK_STATE_CONNECTED);
    EXPECT_EQ(g_pbapPceService_->GetPhoneBookSyncState(RawAddress(g_pbapPceSvcAddress)),
        static_cast<int32_t>(PBAP_PCE_PHONEBOOK_STATE_CONNECTED));
    HILOGI("Pbap_Pce_Service_SetPhoneBookSyncState_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_IsDownloading_00100
 * @tc.name: IsDownloading without loading impl
 * @tc.desc: Verify IsDownloading returns false when impl library is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_IsDownloading_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_IsDownloading_00100 start");
    EXPECT_FALSE(g_pbapPceService_->IsDownloading(RawAddress(g_pbapPceSvcAddress)));
    HILOGI("Pbap_Pce_Service_IsDownloading_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_AbortDownloading_00100
 * @tc.name: AbortDownloading without loading impl
 * @tc.desc: Verify AbortDownloading returns BT_ERR_INTERNAL_ERROR when impl is not loaded.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_AbortDownloading_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_AbortDownloading_00100 start");
    EXPECT_EQ(g_pbapPceService_->AbortDownloading(RawAddress(g_pbapPceSvcAddress)), BT_ERR_INTERNAL_ERROR);
    HILOGI("Pbap_Pce_Service_AbortDownloading_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_PullPhoneBook_00100
 * @tc.name: PullPhoneBook returns BT_NO_ERROR
 * @tc.desc: Verify PullPhoneBook returns BT_NO_ERROR (deferred).
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_PullPhoneBook_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_PullPhoneBook_00100 start");
    EXPECT_EQ(g_pbapPceService_->PullPhoneBook(RawAddress(g_pbapPceSvcAddress)), BT_NO_ERROR);
    HILOGI("Pbap_Pce_Service_PullPhoneBook_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_GetMaxConnectNum_00100
 * @tc.name: GetMaxConnectNum
 * @tc.desc: Verify GetMaxConnectNum returns 10.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_GetMaxConnectNum_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_GetMaxConnectNum_00100 start");
    EXPECT_EQ(g_pbapPceService_->GetMaxConnectNum(), 10);
    HILOGI("Pbap_Pce_Service_GetMaxConnectNum_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_NotifyStateChanged_00100
 * @tc.name: NotifyStateChanged with no observers
 * @tc.desc: Verify NotifyStateChanged is a no-op when observer list is empty.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_NotifyStateChanged_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_NotifyStateChanged_00100 start");
    // 注册后立即注销：空表派发应为 no-op，无残留通知
    g_pbapPceService_->RegisterObserver(g_pbapPceSvcObserver);
    g_pbapPceService_->DeregisterObserver(g_pbapPceSvcObserver);
    g_pbapPceSvcObserver.ResetCount();
    g_pbapPceService_->NotifyStateChanged(RawAddress(g_pbapPceSvcAddress),
        static_cast<int>(BTConnectState::CONNECTED), 0);
    EXPECT_EQ(g_pbapPceSvcObserver.GetConnStateCount(), 0);
    HILOGI("Pbap_Pce_Service_NotifyStateChanged_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_RegisterDeregisterObserver_00100
 * @tc.name: RegisterObserver and DeregisterObserver
 * @tc.desc: Verify observer receives notification after register and stops after deregister.
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_RegisterDeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_RegisterDeregisterObserver_00100 start");
    g_pbapPceSvcObserver.ResetCount();
    g_pbapPceService_->RegisterObserver(g_pbapPceSvcObserver);
    g_pbapPceService_->NotifyStateChanged(RawAddress(g_pbapPceSvcAddress),
        static_cast<int>(BTConnectState::CONNECTED), 0);
    EXPECT_EQ(g_pbapPceSvcObserver.GetConnStateCount(), 1);
    g_pbapPceService_->DeregisterObserver(g_pbapPceSvcObserver);
    g_pbapPceService_->NotifyStateChanged(RawAddress(g_pbapPceSvcAddress),
        static_cast<int>(BTConnectState::CONNECTED), 0);
    EXPECT_EQ(g_pbapPceSvcObserver.GetConnStateCount(), 1);
    HILOGI("Pbap_Pce_Service_RegisterDeregisterObserver_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Service_EnableDisable_00100
 * @tc.name: Enable and Disable
 * @tc.desc: Verify service remains queryable with default state after Enable/Disable (async).
 */
HWTEST_F(PbapPceServiceTest, Pbap_Pce_Service_EnableDisable_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Service_EnableDisable_00100 start");
    g_pbapPceService_->Enable();
    sleep(PBAP_PCE_SVC_TIME);
    g_pbapPceService_->Disable();
    sleep(1);
    EXPECT_EQ(g_pbapPceService_->GetPhoneBookSyncState(RawAddress(g_pbapPceSvcAddress)),
        static_cast<int32_t>(PBAP_PCE_PHONEBOOK_STATE_DISCONNECTED));
    HILOGI("Pbap_Pce_Service_EnableDisable_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS
