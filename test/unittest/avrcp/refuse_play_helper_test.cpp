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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "refuse_play_helper.h"
#include "a2dp_src/a2dp_src_service.h"
#include "class_creator.h"
#include "profile_service_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "remote_device_properties.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
static BluetoothHost *host_;
static RefusePlayHelper *refusePlayHelper_;

void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

class RefusePlayHelperTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        refusePlayHelper_ = RefusePlayHelper::GetInstance();
        sleep(TIME);

        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
        ClassFactory::RegisterClass("A2dpSrcService", A2dpSrcServiceCreate);
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {}
    void SetUp();
    void TearDown();

    std::string device = "11:22:33:44:55:66";
};

void RefusePlayHelperTest::SetUp()
{}

void RefusePlayHelperTest::TearDown()
{
    RefusePlayHelper::GetInstance()->ClearRecord(device);
}

/**
 * @tc.name: RefusePlayHelperTest_SetLastAvrcpConnectTime_001
 * @tc.desc: SetLastAvrcpConnectTime
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_SetLastAvrcpConnectTime_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_SetLastAvrcpConnectTime_001 enter");
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    EXPECT_NE(refusePlayHelper_->GetLastAvrcpConnectTime(device), DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_SetLastAvrcpConnectTime_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_SetLastAvrcpConnectTime_002
 * @tc.desc: SetLastAvrcpConnectTime
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_SetLastAvrcpConnectTime_002, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_SetLastAvrcpConnectTime_002 enter");
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    EXPECT_NE(refusePlayHelper_->GetLastAvrcpConnectTime(device), DEFAULT_VALUE);

    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    EXPECT_NE(refusePlayHelper_->GetLastAvrcpConnectTime(device), DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_SetLastAvrcpConnectTime_002 end");
}

/**
 * @tc.name: RefusePlayHelperTest_GetLastAvrcpConnectTime_001
 * @tc.desc: GetLastAvrcpConnectTime
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_GetLastAvrcpConnectTime_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_GetLastAvrcpConnectTime_001 enter");
    int lastAvrcpConnectTime = refusePlayHelper_->GetLastAvrcpConnectTime(device);
    EXPECT_EQ(lastAvrcpConnectTime, DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_GetLastAvrcpConnectTime_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsAutoPlay_001
 * @tc.desc: IsAutoPlay
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsAutoPlay_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsAutoPlay_001 enter");
    int currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
    bool isAutoPlay = refusePlayHelper_->IsAutoPlay(device, currentTime);
    EXPECT_FALSE(isAutoPlay);
    HILOGI("RefusePlayHelperTest_IsAutoPlay_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsAutoPlay_002
 * @tc.desc: IsAutoPlay
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsAutoPlay_002, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsAutoPlay_002 enter");
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    int64_t currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
    bool isAutoPlay = refusePlayHelper_->IsAutoPlay(device, currentTime);
    EXPECT_TRUE(isAutoPlay);
    HILOGI("RefusePlayHelperTest_IsAutoPlay_002 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsAutoPlay_003
 * @tc.desc: IsAutoPlay
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsAutoPlay_003, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsAutoPlay_003 enter");
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    bool isAutoPlay = refusePlayHelper_->IsAutoPlay(device, 10000000000000);
    EXPECT_FALSE(isAutoPlay);
    HILOGI("RefusePlayHelperTest_IsAutoPlay_003 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsRefusePlayAfterConnect_001
 * @tc.desc: IsRefusePlayAfterConnect
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsRefusePlayAfterConnect_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsRefusePlayAfterConnect_001 enter");
    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(device));
    remoteDevice->SetIoCapability(GAP_IO_KEYBOARDDISPLAY);
    bool isRefuse = refusePlayHelper_->IsRefusePlayAfterConnect(device);
    EXPECT_FALSE(isRefuse);
    HILOGI("RefusePlayHelperTest_IsRefusePlayAfterConnect_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsRefusePlayAfterConnect_002
 * @tc.desc: IsRefusePlayAfterConnect
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsRefusePlayAfterConnect_002, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsRefusePlayAfterConnect_002 enter");
    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(device));
    remoteDevice->SetIoCapability(GAP_IO_KEYBOARDDISPLAY);
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    bool isRefuse = refusePlayHelper_->IsRefusePlayAfterConnect(device);
    EXPECT_TRUE(isRefuse);
    HILOGI("RefusePlayHelperTest_IsRefusePlayAfterConnect_002 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsNeedToSendPlay_001
 * @tc.desc: IsNeedToSendPlay
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsNeedToSendPlay_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsNeedToSendPlay_001 enter");
    EXPECT_FALSE(refusePlayHelper_->IsNeedToSendPlay(device));
    HILOGI("RefusePlayHelperTest_IsNeedToSendPlay_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsPauseAllowedByWhiteList_001
 * @tc.desc: IsPauseAllowedByWhiteList
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsPauseAllowedByWhiteList_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsPauseAllowedByWhiteList_001 enter");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    auto manager = ProfileServiceManager::GetInstance();
    string name = "A2dpSrcService";
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, name);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (a2dpService != nullptr) {
        deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
        a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
        EXPECT_FALSE(refusePlayHelper_->IsPauseAllowedByWhiteList());
    }
    HILOGI("RefusePlayHelperTest_IsPauseAllowedByWhiteList_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsPauseAllowedByWhiteList_002
 * @tc.desc: IsPauseAllowedByWhiteList
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsPauseAllowedByWhiteList_002, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsPauseAllowedByWhiteList_002 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:54:AF:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (a2dpService && deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }
        EXPECT_TRUE(refusePlayHelper_->IsPauseAllowedByWhiteList());
    }
    HILOGI("RefusePlayHelperTest_IsPauseAllowedByWhiteList_002 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsPauseAllowedByWhiteList_003
 * @tc.desc: IsPauseAllowedByWhiteList
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsPauseAllowedByWhiteList_003, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsPauseAllowedByWhiteList_003 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:54:AF:44:55:66");
        a2dpService->DeleteDeviceFromList(device);
        device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }
        std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
        if (remoteDevice != nullptr) {
            remoteDevice->SetAliasName("BT_CITROEN");
        }
        EXPECT_TRUE(refusePlayHelper_->IsPauseAllowedByWhiteList());
        if (remoteDevice != nullptr) {
            remoteDevice->SetAliasName("");
        }
    }
    HILOGI("RefusePlayHelperTest_IsPauseAllowedByWhiteList_003 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsPauseAllowedByWhiteList_004
 * @tc.desc: IsPauseAllowedByWhiteList
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsPauseAllowedByWhiteList_004, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsPauseAllowedByWhiteList_004 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress device = RawAddress("00:54:AF:44:55:66");
        a2dpService->DeleteDeviceFromList(device);
        device = RawAddress("11:22:33:44:55:66");
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
        }
        std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
        if (remoteDevice != nullptr) {
            remoteDevice->SetAliasName("BT_PEUGEOT");
        }
        EXPECT_TRUE(refusePlayHelper_->IsPauseAllowedByWhiteList());
        if (remoteDevice != nullptr) {
            remoteDevice->SetAliasName("");
        }
    }
    HILOGI("RefusePlayHelperTest_IsPauseAllowedByWhiteList_004 end");
}

/**
 * @tc.name: RefusePlayHelperTest_NeedIgnoreSetActive_001
 * @tc.desc: NeedIgnoreSetActive
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_NeedIgnoreSetActive_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_NeedIgnoreSetActive_001 enter");
    std::string device = "11:11:11:44:55:66";
    bool ret = refusePlayHelper_->NeedIgnoreSetActive();
    EXPECT_FALSE(ret);
    refusePlayHelper_->SetLastHangUpTime(device);
    this_thread::sleep_for(chrono::seconds(4));
    ret = refusePlayHelper_->NeedIgnoreSetActive();
    EXPECT_TRUE(ret);
    refusePlayHelper_->ClearRecord(device);
    HILOGI("RefusePlayHelperTest_NeedIgnoreSetActive_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_SetLastHangUpTime_001
 * @tc.desc: SetLastHangUpTime
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_SetLastHangUpTime_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_SetLastHangUpTime_001 enter");
    refusePlayHelper_->SetLastHangUpTime(device);
    refusePlayHelper_->SetLastHangUpTime(device);
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    EXPECT_NE(refusePlayHelper_->GetLastAvrcpConnectTime(device), DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_SetLastHangUpTime_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_NeedDropDueScoEnd_001
 * @tc.desc: NeedDropDueScoEnd
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_NeedDropDueScoEnd_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_NeedDropDueScoEnd_001 enter");
    std::string device = "11:22:33:44:55:66";
    refusePlayHelper_->ProcessPushed(device, true);
    bool ret = refusePlayHelper_->NeedDropDueScoEnd(device, true);
    EXPECT_TRUE(ret);
    refusePlayHelper_->SetLastHangUpTime(device);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress rawDevice = RawAddress(device);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(rawDevice);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device, deviceInfo);
            a2dpService->UpdateActiveDevice(rawDevice);
        }
        refusePlayHelper_->SavePauseTime(device);
        refusePlayHelper_->JudgeScoPause(device);
        ret = refusePlayHelper_->NeedDropDueScoEnd(device, true);
        EXPECT_FALSE(ret);
        refusePlayHelper_->NeedDropDueScoEnd(device, false);
        refusePlayHelper_->SetLastHangUpTime(device);
        ret = refusePlayHelper_->NeedDropDueScoEnd(device, false);
        EXPECT_TRUE(ret);
        refusePlayHelper_->ClearRecord(device);
        refusePlayHelper_->SavePauseTime(device);
        refusePlayHelper_->JudgeScoPause(device);
        ret = refusePlayHelper_->NeedDropDueScoEnd(device, false);
        EXPECT_FALSE(ret);
    }

    HILOGI("RefusePlayHelperTest_NeedDropDueScoEnd_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_ProcessPushed_001
 * @tc.desc: ProcessPushed
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_ProcessPushed_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_ProcessPushed_001 enter");
    refusePlayHelper_->ProcessPushed(device, true);
    EXPECT_TRUE(refusePlayHelper_->NeedDropDueScoEnd(device, true));
    refusePlayHelper_->ProcessPushed(device, false);
    EXPECT_FALSE(refusePlayHelper_->NeedDropDueScoEnd(device, false));
    HILOGI("RefusePlayHelperTest_ProcessPushed_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_SavePauseTime_001
 * @tc.desc: SavePauseTime
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_SavePauseTime_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_SavePauseTime_001 enter");
    refusePlayHelper_->SavePauseTime(device);
    refusePlayHelper_->SavePauseTime(device);
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    EXPECT_NE(refusePlayHelper_->GetLastAvrcpConnectTime(device), DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_SavePauseTime_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_JudgeScoPause_001
 * @tc.desc: JudgeScoPause
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_JudgeScoPause_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_JudgeScoPause_001 enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        RawAddress rawDevice = RawAddress(device);
        std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(rawDevice);
        if (deviceInfo != nullptr) {
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
            a2dpService->AddDeviceToList(device, deviceInfo);
            a2dpService->UpdateActiveDevice(rawDevice);
        }
        refusePlayHelper_->JudgeScoPause(device);
        refusePlayHelper_->SavePauseTime(device);
        refusePlayHelper_->SetLastAvrcpConnectTime(device);
        EXPECT_NE(refusePlayHelper_->GetLastAvrcpConnectTime(device), DEFAULT_VALUE);
        refusePlayHelper_->JudgeScoPause(device);
    }
    HILOGI("RefusePlayHelperTest_JudgeScoPause_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_ClearAuthingDevice_001
 * @tc.desc: ClearAuthingDevice
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_ClearAuthingDevice_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_ClearAuthingDevice_001 enter");
    refusePlayHelper_->ClearAuthingDevice(device);
    EXPECT_EQ(refusePlayHelper_->mWatingForAuthDevices_.Size(), 0);
    HILOGI("RefusePlayHelperTest_ClearAuthingDevice_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_IsShowCapsuleAfterHangUp_001
 * @tc.desc: IsShowCapsuleAfterHangUp
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_IsShowCapsuleAfterHangUp_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_IsShowCapsuleAfterHangUp_001 enter");
    std::string device1 = "01:22:33:44:55:66";
    EXPECT_FALSE(refusePlayHelper_->IsShowCapsuleAfterHangUp(device1));

    std::string device2 = "01:12:33:44:55:66";
    refusePlayHelper_->SetLastHangUpTime(device2);
    EXPECT_TRUE(refusePlayHelper_->IsShowCapsuleAfterHangUp(device2));

    std::string device3 = "01:12:23:44:55:66";
    refusePlayHelper_->SetLastHangUpTime(device2);
    sleep(2);
    EXPECT_FALSE(refusePlayHelper_->IsShowCapsuleAfterHangUp(device2));
    HILOGI("RefusePlayHelperTest_IsShowCapsuleAfterHangUp_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_GetLastHangUpTime_001
 * @tc.desc: GetLastHangUpTime
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_GetLastHangUpTime_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_GetLastHangUpTime_001 enter");
    std::string device1 = "01:22:33:44:55:66";
    EXPECT_EQ(refusePlayHelper_->GetLastHangUpTime(device1), DEFAULT_VALUE);

    std::string device2 = "01:12:33:44:55:66";
    refusePlayHelper_->SetLastHangUpTime(device2);
    EXPECT_NE(refusePlayHelper_->IsShowCapsuleAfterHangUp(device2), DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_GetLastHangUpTime_001 end");
}
/**
 * @tc.name: RefusePlayHelperTest_GetEffectiveAutoPlayDuration_001
 * @tc.desc: GetEffectiveAutoPlayDuration
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_GetEffectiveAutoPlayDuration_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_GetEffectiveAutoPlayDuration_001 enter");
    int duration = refusePlayHelper_->GetEffectiveAutoPlayDuration(device);
    EXPECT_EQ(duration, DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS);
    HILOGI("RefusePlayHelperTest_GetEffectiveAutoPlayDuration_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_RecordPlayOffset_001
 * @tc.desc: RecordPlayOffset
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_RecordPlayOffset_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_RecordPlayOffset_001 enter");
    refusePlayHelper_->RecordPlayOffset(device);
    TimeRecord timeRecord;
    EXPECT_FALSE(refusePlayHelper_->mTimeRecordMap_.Find(device, timeRecord));
    HILOGI("RefusePlayHelperTest_RecordPlayOffset_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_RecordPlayOffset_002
 * @tc.desc: RecordPlayOffset
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_RecordPlayOffset_002, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_RecordPlayOffset_002 enter");
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    refusePlayHelper_->RecordPlayOffset(device);
    TimeRecord timeRecord;
    EXPECT_TRUE(refusePlayHelper_->mTimeRecordMap_.Find(device, timeRecord));
    EXPECT_EQ(timeRecord.pendingPlayOffset, DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_RecordPlayOffset_002 end");
}

/**
 * @tc.name: RefusePlayHelperTest_TryLearnFromUserPause_001
 * @tc.desc: TryLearnFromUserPause
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_TryLearnFromUserPause_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_TryLearnFromUserPause_001 enter");
    int64_t currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
    TimeRecord timeRecord;
    int64_t learnOffset = refusePlayHelper_->TryLearnFromUserPause(device, timeRecord, currentTime);
    EXPECT_EQ(learnOffset, DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_TryLearnFromUserPause_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_TryLearnFromUserPause_003
 * @tc.desc: TryLearnFromUserPause
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_TryLearnFromUserPause_003, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_TryLearnFromUserPause_003 enter");
    int64_t currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
    TimeRecord timeRecord;
    timeRecord.lastAvrcpConnectTime = currentTime - 25000;
    timeRecord.pendingPlayTime = currentTime - 24000;
    timeRecord.pendingPlayOffset = 8000;
    int64_t learnOffset = refusePlayHelper_->TryLearnFromUserPause(device, timeRecord, currentTime);
    EXPECT_EQ(learnOffset, DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_TryLearnFromUserPause_003 end");
}

/**
 * @tc.name: RefusePlayHelperTest_TryLearnAndSave_001
 * @tc.desc: TryLearnAndSave
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_TryLearnAndSave_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_TryLearnAndSave_001 enter");
    int64_t currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
    TimeRecord timeRecord;
    refusePlayHelper_->TryLearnAndSave(device, timeRecord, currentTime);
    TimeRecord savedRecord;
    EXPECT_TRUE(refusePlayHelper_->mTimeRecordMap_.Find(device, savedRecord));
    EXPECT_EQ(savedRecord.pendingPlayOffset, DEFAULT_VALUE);
    HILOGI("RefusePlayHelperTest_TryLearnAndSave_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_HandlePauseEvent_001
 * @tc.desc: HandlePauseEvent
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_HandlePauseEvent_001, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_HandlePauseEvent_001 enter");
    refusePlayHelper_->HandlePauseEvent(device);
    TimeRecord timeRecord;
    EXPECT_FALSE(refusePlayHelper_->mTimeRecordMap_.Find(device, timeRecord));
    HILOGI("RefusePlayHelperTest_HandlePauseEvent_001 end");
}

/**
 * @tc.name: RefusePlayHelperTest_HandlePauseEvent_002
 * @tc.desc: HandlePauseEvent
 * @tc.type: FUNC
 */
HWTEST_F(RefusePlayHelperTest, RefusePlayHelperTest_HandlePauseEvent_002, TestSize.Level1)
{
    HILOGI("RefusePlayHelperTest_HandlePauseEvent_002 enter");
    refusePlayHelper_->SetLastAvrcpConnectTime(device);
    refusePlayHelper_->HandlePauseEvent(device);
    TimeRecord timeRecord;
    EXPECT_TRUE(refusePlayHelper_->mTimeRecordMap_.Find(device, timeRecord));
    EXPECT_EQ(timeRecord.lastPauseTime, DEFAULT_VALUE);
    EXPECT_EQ(timeRecord.pendingPlayOffset, DEFAULT_VALUE);
    // empty device is not processed
    refusePlayHelper_->HandlePauseEvent("");
    EXPECT_TRUE(refusePlayHelper_->mTimeRecordMap_.Find(device, timeRecord));
    HILOGI("RefusePlayHelperTest_HandlePauseEvent_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS
