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
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "profile_service_manager.h"
#include "raw_address.h"
#include "bt_def.h"
#include "thread_util.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

std::string g_pbapPceConnAddress("00:11:22:33:44:55");
const int PBAP_PCE_CONN_TIME = 2;
static Bluetooth::BluetoothHost *g_pbapPceConnHost_ = nullptr;
static PbapPceServiceImpl *g_pbapPceConnServiceImpl_ = nullptr;

class PbapPceConnectionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("PbapPceConnectionTest SetUpTestCase start");
        g_pbapPceConnHost_ = &Bluetooth::BluetoothHost::GetDefaultHost();

        bool isBleEnable = g_pbapPceConnHost_->IsBleEnabled();
        bool isBrEnable = g_pbapPceConnHost_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            g_pbapPceConnHost_->EnableBle();
            g_pbapPceConnHost_->EnableBt();
            sleep(PBAP_PCE_CONN_TIME);
        }

        HILOGI("PbapPceConnectionTest SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("PbapPceConnectionTest TearDownTestCase start");
        g_pbapPceConnHost_ = nullptr;
        HILOGI("PbapPceConnectionTest TearDownTestCase end");
    }
    void SetUp()
    {
        g_pbapPceConnServiceImpl_ = new PbapPceServiceImpl();

        auto manager = ProfileServiceManager::GetInstance();
        manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_PBAP_PCE);

        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_PBAP, ThreadUtil::NOT_SWITCH_THREAD);
        sleep(1);
    }
    void TearDown()
    {
        sleep(1);
        if (g_pbapPceConnServiceImpl_ != nullptr) {
            g_pbapPceConnServiceImpl_->machineMap_.clear();
            delete g_pbapPceConnServiceImpl_;
            g_pbapPceConnServiceImpl_ = nullptr;
        }
    }
};

/*
 * @tc.number: Pbap_Pce_Connection_GetDeviceState_00100
 * @tc.name: GetDeviceState when device not in map
 * @tc.desc: Verify GetDeviceState returns DISCONNECTED for unknown device.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_GetDeviceState_00100 start");
    RawAddress device("AA:BB:CC:DD:EE:FF");
    int state = g_pbapPceConnServiceImpl_->GetDeviceState(device);
    EXPECT_EQ(state, static_cast<int>(BTConnectState::DISCONNECTED));
    HILOGI("Pbap_Pce_Connection_GetDeviceState_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_GetConnectState_00100
 * @tc.name: GetConnectState with empty map
 * @tc.desc: Verify GetConnectState returns 0 when machineMap_ is empty.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_GetConnectState_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    EXPECT_EQ(g_pbapPceConnServiceImpl_->GetConnectState(), 0);
    HILOGI("Pbap_Pce_Connection_GetConnectState_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates with empty map
 * @tc.desc: Verify GetDevicesByStates returns empty when machineMap_ is empty.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_GetDevicesByStates_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    std::vector<int32_t> states = {static_cast<int32_t>(BTConnectState::CONNECTED)};
    std::vector<RawAddress> devices = g_pbapPceConnServiceImpl_->GetDevicesByStates(states);
    EXPECT_EQ(devices.size(), 0u);
    HILOGI("Pbap_Pce_Connection_GetDevicesByStates_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_GetConnectedDevices_00100
 * @tc.name: GetConnectedDevices with empty map
 * @tc.desc: Verify GetConnectedDevices returns empty when machineMap_ is empty.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_GetConnectedDevices_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_GetConnectedDevices_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    std::vector<RawAddress> devices = g_pbapPceConnServiceImpl_->GetConnectedDevices();
    EXPECT_EQ(devices.size(), 0u);
    HILOGI("Pbap_Pce_Connection_GetConnectedDevices_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_IsBusy_00100
 * @tc.name: IsBusy with device not in map
 * @tc.desc: Verify IsBusy returns false when device is not in machineMap_.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_IsBusy_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_IsBusy_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    EXPECT_FALSE(g_pbapPceConnServiceImpl_->IsBusy(RawAddress(g_pbapPceConnAddress)));
    HILOGI("Pbap_Pce_Connection_IsBusy_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_IsDownloading_00100
 * @tc.name: IsDownloading with device not in map
 * @tc.desc: Verify IsDownloading returns false when device is not in machineMap_.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_IsDownloading_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_IsDownloading_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    EXPECT_FALSE(g_pbapPceConnServiceImpl_->IsDownloading(RawAddress(g_pbapPceConnAddress)));
    HILOGI("Pbap_Pce_Connection_IsDownloading_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_IsConnected_00100
 * @tc.name: IsConnected with device not in map
 * @tc.desc: Verify IsConnected returns false when device is not in machineMap_.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_IsConnected_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_IsConnected_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    EXPECT_FALSE(g_pbapPceConnServiceImpl_->IsConnected(RawAddress(g_pbapPceConnAddress)));
    HILOGI("Pbap_Pce_Connection_IsConnected_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_SetBusy_00100
 * @tc.name: SetBusy with device not in map
 * @tc.desc: Verify SetBusy returns false when device is not in machineMap_.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_SetBusy_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_SetBusy_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    EXPECT_FALSE(g_pbapPceConnServiceImpl_->SetBusy(RawAddress(g_pbapPceConnAddress), true));
    HILOGI("Pbap_Pce_Connection_SetBusy_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_Disconnect_00100
 * @tc.name: Disconnect with no state machine
 * @tc.desc: Verify Disconnect returns RET_BAD_STATUS when device is DISCONNECTED.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_Disconnect_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_Disconnect_00100 start");
    RawAddress device(g_pbapPceConnAddress);
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    int ret = g_pbapPceConnServiceImpl_->Disconnect(device);
    EXPECT_EQ(ret, RET_BAD_STATUS);
    HILOGI("Pbap_Pce_Connection_Disconnect_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_ForwardMsgToStm_00100
 * @tc.name: ForwardMsgToStm with device not in map
 * @tc.desc: Verify ForwardMsgToStm does not crash when device is not in machineMap_.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_ForwardMsgToStm_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_ForwardMsgToStm_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    utility::Message msg(PCE_REQ_DOWNLOAD);
    g_pbapPceConnServiceImpl_->ForwardMsgToStm(RawAddress(g_pbapPceConnAddress), msg);
    EXPECT_TRUE(g_pbapPceConnServiceImpl_->machineMap_.empty());
    HILOGI("Pbap_Pce_Connection_ForwardMsgToStm_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_SetPowerStatusBusy_00100
 * @tc.name: SetPowerStatusBusy with device not in map
 * @tc.desc: Verify SetPowerStatusBusy does not crash when device is not in machineMap_.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_SetPowerStatusBusy_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_SetPowerStatusBusy_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    g_pbapPceConnServiceImpl_->SetPowerStatusBusy(RawAddress(g_pbapPceConnAddress), true);
    g_pbapPceConnServiceImpl_->SetPowerStatusBusy(RawAddress(g_pbapPceConnAddress), false);
    EXPECT_TRUE(g_pbapPceConnServiceImpl_->machineMap_.empty());
    HILOGI("Pbap_Pce_Connection_SetPowerStatusBusy_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_ProcessConnectEvent_00100
 * @tc.name: ProcessConnectEvent with CONNECT event
 * @tc.desc: Verify ProcessConnectEvent handles CONNECT_EVT without crash.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_ProcessConnectEvent_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_ProcessConnectEvent_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    PbapPceMessage eventConnect(PBAP_PCE_CONNECT_EVT);
    eventConnect.dev_ = RawAddress(g_pbapPceConnAddress);
    g_pbapPceConnServiceImpl_->ProcessConnectEvent(eventConnect);
    EXPECT_TRUE(g_pbapPceConnServiceImpl_->machineMap_.empty());
    HILOGI("Pbap_Pce_Connection_ProcessConnectEvent_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_ProcessConnectEvent_00200
 * @tc.name: ProcessConnectEvent with DISCONNECT event
 * @tc.desc: Verify ProcessConnectEvent handles DISCONNECT_EVT without crash.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_ProcessConnectEvent_00200, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_ProcessConnectEvent_00200 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    PbapPceMessage eventDisconnect(PBAP_PCE_DISCONNECT_EVT);
    eventDisconnect.dev_ = RawAddress(g_pbapPceConnAddress);
    g_pbapPceConnServiceImpl_->ProcessConnectEvent(eventDisconnect);
    EXPECT_TRUE(g_pbapPceConnServiceImpl_->machineMap_.empty());
    HILOGI("Pbap_Pce_Connection_ProcessConnectEvent_00200 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_ProcessConnectEvent_00300
 * @tc.name: ProcessConnectEvent with unknown event
 * @tc.desc: Verify ProcessConnectEvent handles unknown event without crash.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_ProcessConnectEvent_00300, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_ProcessConnectEvent_00300 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    PbapPceMessage eventUnknown(0xFF);
    eventUnknown.dev_ = RawAddress(g_pbapPceConnAddress);
    g_pbapPceConnServiceImpl_->ProcessConnectEvent(eventUnknown);
    EXPECT_TRUE(g_pbapPceConnServiceImpl_->machineMap_.empty());
    HILOGI("Pbap_Pce_Connection_ProcessConnectEvent_00300 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_ProcessObexRespMessage_00100
 * @tc.name: ProcessObexRespMessage with various msg types
 * @tc.desc: Verify ProcessObexRespMessage handles various msg types without crash.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_ProcessObexRespMessage_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_ProcessObexRespMessage_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    RawAddress device(g_pbapPceConnAddress);

    utility::Message msgConnected(PCE_OBEX_CONNECTED);
    g_pbapPceConnServiceImpl_->ProcessObexRespMessage(device, msgConnected);

    utility::Message msgDisconnected(PCE_OBEX_DISCONNECTED);
    g_pbapPceConnServiceImpl_->ProcessObexRespMessage(device, msgDisconnected);

    utility::Message msgConnectFailed(PCE_OBEX_CONNECT_FAILED);
    g_pbapPceConnServiceImpl_->ProcessObexRespMessage(device, msgConnectFailed);

    utility::Message msgTransportFailed(PCE_TRANSPORT_FAILED);
    g_pbapPceConnServiceImpl_->ProcessObexRespMessage(device, msgTransportFailed);

    utility::Message msgDownloadComplete(PCE_DOWNLOAD_COMPLETE);
    g_pbapPceConnServiceImpl_->ProcessObexRespMessage(device, msgDownloadComplete);

    utility::Message msgUnknown(0xFFFF);
    g_pbapPceConnServiceImpl_->ProcessObexRespMessage(device, msgUnknown);
    EXPECT_TRUE(g_pbapPceConnServiceImpl_->machineMap_.empty());
    HILOGI("Pbap_Pce_Connection_ProcessObexRespMessage_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_ProcessPhonebookMessage_00100
 * @tc.name: ProcessPhonebookMessage with various msg types
 * @tc.desc: Verify ProcessPhonebookMessage handles various msg types without crash.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_ProcessPhonebookMessage_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_ProcessPhonebookMessage_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    RawAddress device(g_pbapPceConnAddress);

    utility::Message msgSizeCompleted(PCE_PULLPHONEBOOKSIZE_COMPLETED);
    g_pbapPceConnServiceImpl_->ProcessPhonebookMessage(device, msgSizeCompleted);

    utility::Message msgPullCompleted(PCE_PULLPHONEBOOK_COMPLETED);
    g_pbapPceConnServiceImpl_->ProcessPhonebookMessage(device, msgPullCompleted);

    utility::Message msgAbortCompleted(PCE_ABORTDOWNLOADING_COMPLETED);
    g_pbapPceConnServiceImpl_->ProcessPhonebookMessage(device, msgAbortCompleted);

    utility::Message msgUnknown(0xFFFF);
    g_pbapPceConnServiceImpl_->ProcessPhonebookMessage(device, msgUnknown);
    EXPECT_TRUE(g_pbapPceConnServiceImpl_->machineMap_.empty());
    HILOGI("Pbap_Pce_Connection_ProcessPhonebookMessage_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_ProcessSdpMessage_00100
 * @tc.name: ProcessSdpMessage with various msg types
 * @tc.desc: Verify ProcessSdpMessage handles various msg types without crash.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_ProcessSdpMessage_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_ProcessSdpMessage_00100 start");
    g_pbapPceConnServiceImpl_->machineMap_.clear();
    RawAddress device(g_pbapPceConnAddress);

    utility::Message msgSdpFinish(PCE_SDP_FINISH);
    g_pbapPceConnServiceImpl_->ProcessSdpMessage(device, msgSdpFinish);

    utility::Message msgSdpFailed(PCE_SDP_FAILED);
    g_pbapPceConnServiceImpl_->ProcessSdpMessage(device, msgSdpFailed);

    utility::Message msgUnknown(0xFFFF);
    g_pbapPceConnServiceImpl_->ProcessSdpMessage(device, msgUnknown);
    EXPECT_TRUE(g_pbapPceConnServiceImpl_->machineMap_.empty());
    HILOGI("Pbap_Pce_Connection_ProcessSdpMessage_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_GetMaxConnectNum_00100
 * @tc.name: GetMaxConnectNum
 * @tc.desc: Verify GetMaxConnectNum returns a non-negative value.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_GetMaxConnectNum_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_GetMaxConnectNum_00100 start");
    EXPECT_GE(g_pbapPceConnServiceImpl_->GetPceConfig().pceMaxDevices_, 0);
    HILOGI("Pbap_Pce_Connection_GetMaxConnectNum_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_GetPceConfig_00100
 * @tc.name: GetPceConfig
 * @tc.desc: Verify GetPceConfig returns config with valid pceMaxDevices_.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_GetPceConfig_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_GetPceConfig_00100 start");
    const PbapPceConfig &config = g_pbapPceConnServiceImpl_->GetPceConfig();
    EXPECT_GE(config.pceMaxDevices_, 0);
    HILOGI("Pbap_Pce_Connection_GetPceConfig_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_RegisterDeregisterObserver_00100
 * @tc.name: RegisterObserver and DeregisterObserver
 * @tc.desc: Verify Register and Deregister observer without crash.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_RegisterDeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_RegisterDeregisterObserver_00100 start");
    class TestObserver : public IPbapPceObserver {
    public:
        void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override {}
        void OnSyncStateChange(const RawAddress &device, int syncState) override {}
    };
    std::shared_ptr<IPbapPceObserver> observer = std::make_shared<TestObserver>();
    g_pbapPceConnServiceImpl_->RegisterObserver(observer);
    g_pbapPceConnServiceImpl_->DeregisterObserver(observer);
    // observer 生命周期后 impl 仍功能正常（空 map 断连返回 RET_BAD_STATUS）
    int ret = g_pbapPceConnServiceImpl_->Disconnect(RawAddress(g_pbapPceConnAddress));
    EXPECT_EQ(ret, RET_BAD_STATUS);
    HILOGI("Pbap_Pce_Connection_RegisterDeregisterObserver_00100 end");
}

/*
 * @tc.number: Pbap_Pce_Connection_EnableDisable_00100
 * @tc.name: Enable and Disable service impl
 * @tc.desc: Verify Enable/Disable service impl lifecycle does not crash.
 */
HWTEST_F(PbapPceConnectionTest, Pbap_Pce_Connection_EnableDisable_00100, TestSize.Level1)
{
    HILOGI("Pbap_Pce_Connection_EnableDisable_00100 start");
    g_pbapPceConnServiceImpl_->Enable();
    g_pbapPceConnServiceImpl_->Disable();
    // Disable 后空 map 断连路径保持功能正常
    int ret = g_pbapPceConnServiceImpl_->Disconnect(RawAddress(g_pbapPceConnAddress));
    EXPECT_EQ(ret, RET_BAD_STATUS);
    HILOGI("Pbap_Pce_Connection_EnableDisable_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS
