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
#include "hfp_hf_call_manager.h"
#ifdef BLUETOOTH_WATCH_ENABLE
#include "watch_service.h"
#else
#include "hfp_hf_defines.h"
#endif
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

std::string address("11:22:33:44:55:66");
HfpHfCallManager callManager(address);
static BluetoothHost *host_;
constexpr int TIME = 2;
class HfpHfCallManagerTest : public testing::Test {
public:
    HfpHfCallManagerTest()
    {}
    ~HfpHfCallManagerTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HfpHfCallManagerTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();
    sleep(TIME);

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    auto manager = ProfileServiceManager::GetInstance();
    std::string profileName = "HfpHfService";
    manager->SetProfileService(ADAPTER_BREDR, profileName);

    HILOGI("SetUpTestCase end");
}
void HfpHfCallManagerTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void HfpHfCallManagerTest::SetUp()
{
}

void HfpHfCallManagerTest::TearDown()
{
}

/*
 * @tc.number: Hfp_Hf_Call_Manager_HasCallState_0001
 * @tc.name: HasCallState
 * @tc.desc:
 */
HWTEST_F(HfpHfCallManagerTest, Hfp_Hf_Call_Manager_HasCallState_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Call_Manager_HasCallState_0001 start");

    EXPECT_EQ(false, callManager.HasCallState(HFP_CALL_STATE_INCOMING));

    HandsFreeUnitCalls call(address, 0, HFP_CALL_STATE_ACTIVE, "10086", false, false, true);
    callManager.UpdateCall(call);
    callManager.UpdateCallDone();

    EXPECT_NE(false, callManager.HasCallState(HFP_CALL_STATE_ACTIVE));

    EXPECT_EQ(false, callManager.HasCallState(1));

    callManager.UpdateCallDone();

    HILOGI("Hfp_Hf_Call_Manager_HasCallState_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Call_Manager_GetCurrentCalls_0001
 * @tc.name: GetCurrentCalls
 * @tc.desc:
 */
HWTEST_F(HfpHfCallManagerTest, Hfp_Hf_Call_Manager_GetCurrentCalls_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Call_Manager_GetCurrentCalls_0001 start");

    EXPECT_EQ(0, callManager.GetCurrentCalls().size());

    HandsFreeUnitCalls call(address, 0, HFP_CALL_STATE_INCOMING, "10086", false, false, true);
    callManager.UpdateCall(call);
    callManager.UpdateCallDone();
    EXPECT_EQ(1, callManager.GetCurrentCalls().size());

    HandsFreeUnitCalls callActive(address, 1, HFP_CALL_STATE_ACTIVE, "10086", false, false, true);
    callManager.UpdateCall(call);
    callManager.UpdateCall(callActive);
    callManager.UpdateCallDone();
    EXPECT_EQ(2, callManager.GetCurrentCalls().size());

    callManager.UpdateCallDone();

    HILOGI("Hfp_Hf_Call_Manager_GetCurrentCalls_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Call_Manager_NotifyCallChanged_0001
 * @tc.name: NotifyCallChanged
 * @tc.desc:
 */
HWTEST_F(HfpHfCallManagerTest, Hfp_Hf_Call_Manager_NotifyCallChanged_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Call_Manager_NotifyCallChanged_0001 start");

    HandsFreeUnitCalls call(address, 0, HFP_CALL_STATE_INCOMING, "10086", false, false, true);
    callManager.NotifyCallChanged(call);

    HandsFreeUnitCalls callActive(address, 1, HFP_CALL_STATE_ACTIVE, "10086", false, false, true);
    callManager.NotifyCallChanged(callActive);

    HILOGI("Hfp_Hf_Call_Manager_NotifyCallChanged_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Call_Manager_UpdateCall_0001
 * @tc.name: UpdateCall
 * @tc.desc:
 */
HWTEST_F(HfpHfCallManagerTest, Hfp_Hf_Call_Manager_UpdateCall_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Call_Manager_UpdateCall_0001 start");

    HandsFreeUnitCalls callIncoming(address, HFP_HF_OUTGOING_CALL_ID,
                                    HFP_CALL_STATE_DIALING, "10086", false, false, true);
    callManager.UpdateCall(callIncoming);
    EXPECT_EQ(callManager.calls_.Size(), 1);
    EXPECT_EQ(callManager.updateCalls_.Size(), 1);

    HandsFreeUnitCalls callActive(address, 0, HFP_CALL_STATE_DIALING, "10086", false, false, true);
    callManager.UpdateCall(callActive);
    EXPECT_EQ(callManager.calls_.Size(), 0);
    EXPECT_EQ(callManager.updateCalls_.Size(), 1);

    HILOGI("Hfp_Hf_Call_Manager_UpdateCall_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Call_Manager_UpdateCallDone_0001
 * @tc.name: UpdateCallDone
 * @tc.desc:
 */
HWTEST_F(HfpHfCallManagerTest, Hfp_Hf_Call_Manager_UpdateCallDone_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Call_Manager_UpdateCallDone_0001 start");

    HandsFreeUnitCalls callIncoming(address, 0, HFP_CALL_STATE_INCOMING, "10086", false, false, true);
    callManager.UpdateCall(callIncoming);
    callManager.UpdateCallDone();
    EXPECT_EQ(callManager.calls_.Size(), 1);

    HandsFreeUnitCalls callActive(address, 0, HFP_CALL_STATE_ACTIVE, "10086", false, false, true);
    callManager.UpdateCall(callActive);
    callManager.UpdateCallDone();
    EXPECT_EQ(callManager.calls_.Size(), 1);

#ifdef BLUETOOTH_WATCH_ENABLE
    HandsFreeUnitCalls callDailing(address, 1, HFP_CALL_STATE_DIALING, "10086", false, false, true);
    callManager.SetScoConnected(false);
    callManager.UpdateCall(callDailing);
    callManager.UpdateCallDone();
    EXPECT_FALSE(WatchService::GetInstance()->IsAllowHfScoConnect(address));
    EXPECT_EQ(callManager.calls_.Size(), 1);

    WatchService::GetInstance()->SetAllowHfScoConnect(address, true);
    WatchService::GetInstance()->SetDialingByWatch(address);
    callManager.UpdateCall(callDailing);
    callManager.UpdateCallDone();
    EXPECT_TRUE(WatchService::GetInstance()->IsAllowHfScoConnect(address));
    EXPECT_EQ(callManager.calls_.Size(), 1);

    HandsFreeUnitCalls callDailingOrg(address, -1, HFP_CALL_STATE_DIALING, "10086", false, false, true);
    callManager.UpdateCall(callDailingOrg);
    callManager.UpdateCallDone();
    EXPECT_EQ(callManager.calls_.Size(), 1);
#endif

    HILOGI("Hfp_Hf_Call_Manager_UpdateCallDone_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Call_Manager_ProcessUpdateCallTimeout_0001
 * @tc.name: ProcessUpdateCallTimeout
 * @tc.desc:
 */
HWTEST_F(HfpHfCallManagerTest, Hfp_Hf_Call_Manager_ProcessUpdateCallTimeout_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Call_Manager_ProcessUpdateCallTimeout_0001 start");

    callManager.calls_.Clear();
    callManager.updateCalls_.Clear();
    long currentTime = clock();
    long createTime = currentTime - OUTGOING_TIMEOUT_MS * CLOCKS_PER_SEC - 1000;
    Uuid uuid = Uuid::Random();
    HandsFreeUnitCalls callIncoming(address, HFP_HF_OUTGOING_CALL_ID, HFP_CALL_STATE_DIALING,
                                    "10086", uuid, false, false, true, createTime);
    callManager.UpdateCall(callIncoming);
    callManager.ProcessUpdateCallTimeout();
    EXPECT_EQ(callManager.calls_.Size(), 1);
    EXPECT_EQ(callManager.updateCalls_.Size(), 1);

    HILOGI("Hfp_Hf_Call_Manager_ProcessUpdateCallTimeout_0001 end");
}

/*
 * @tc.number: Hfp_Hf_Call_Manager_SetScoConnected_0001
 * @tc.name: SetScoConnected
 * @tc.desc:
 */
HWTEST_F(HfpHfCallManagerTest, Hfp_Hf_Call_Manager_SetScoConnected_0001, TestSize.Level1)
{
    HILOGI("Hfp_Hf_Call_Manager_SetScoConnected_0001 start");

    callManager.SetScoConnected(true);
    EXPECT_TRUE(callManager.isScoConnected_);

    callManager.SetScoConnected(false);
    EXPECT_FALSE(callManager.isScoConnected_);

    sleep(TIME);

    HILOGI("Hfp_Hf_Call_Manager_SetScoConnected_0001 end");
}
}  // namespace bluetooth
}  // namespace OHOS
