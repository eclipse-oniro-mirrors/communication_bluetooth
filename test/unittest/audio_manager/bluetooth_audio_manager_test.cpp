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
#include "bluetooth_audio_manager.h"
#include "bluetooth_audio_manager_info.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "raw_address.h"
#include "remote_device_properties.h"
#include "class_creator.h"
#include "a2dp_src/a2dp_src_service.h"
#include "profile_service_manager.h"
#include "bluetooth_audio_framework_adapter.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;

constexpr int TIME = 2;
constexpr int DOUBLE_REMOVE_CHECK_TIME = 2000;
constexpr int MAX_RESUME_PLAY_TIME_SIMPLE = 180000L; // 180s
const std::string DEVICE_AUDIO_MANAGER_PATH = "/data/service/el1/public/bluetooth/bluetooth_audio_manager_config.xml";

void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

class BluetoothAudioManagerTest : public testing::Test {
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
        ClassFactory::RegisterClass("A2dpSrcService", A2dpSrcServiceCreate);
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

/*
 * @tc.number:audio_manager_test_WearStateHiechoProcess_001
 * @tc.name: WearStateHiechoProcess
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_WearStateHiechoProcess_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_WearStateHiechoProcess_001 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device(address);
    BluetoothAudioManager::GetInstance().WearStateHiechoProcess(device, 0xFF, 0xFF);
    BluetoothAudioManager::GetInstance().WearStateHiechoProcess(device, 0xFF, 1);
    BluetoothAudioManager::GetInstance().WearStateHiechoProcess(device, 1, 0xFF);
    BluetoothAudioManager::GetInstance().WearStateHiechoProcess(device, 1, 0);
    EXPECT_TRUE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    HILOGI("audio_manager_test_WearStateHiechoProcess_001 end");
}

/*
 * @tc.number:audio_manager_test_ChangeActiveForNotWearing_001
 * @tc.name: ChangeActiveForNotWearing
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_ChangeActiveForNotWearing_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_ChangeActiveForNotWearing_001 enter");
    std::string address = "00:01:02:03:04:05";
    BluetoothAudioManager::GetInstance().ChangeActiveForNotWearing(address, 1, 1);
    BluetoothAudioManager::GetInstance().ChangeActiveForNotWearing(address, 0, 0);
    RawAddress device = RawAddress(address);
    EXPECT_TRUE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    HILOGI("audio_manager_test_ChangeActiveForNotWearing_001 end");
}

/*
 * @tc.number:audio_manager_test_UpdateDeviceWearState_001
 * @tc.name: UpdateDeviceWearState
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_UpdateDeviceWearState_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_UpdateDeviceWearState_001 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    int32_t ret = BluetoothAudioManager::GetInstance().SetAudioManagerSupport(remoteDevice->GetAddress(), -1);
    EXPECT_EQ(ret, BluetoothAudioManager::SET_WEARDETECTION_SUCCESS);
    BluetoothAudioManager::GetInstance().UpdateDeviceWearState(remoteDevice, 0, 0);
    HILOGI("audio_manager_test_UpdateDeviceWearState_001 end");
}

/*
 * @tc.number:audio_manager_test_UpdateDeviceWearState_002
 * @tc.name: UpdateDeviceWearState
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_UpdateDeviceWearState_002, TestSize.Level1)
{
    HILOGI("audio_manager_test_UpdateDeviceWearState_002 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    int32_t ret = BluetoothAudioManager::GetInstance().SetAudioManagerSupport(remoteDevice->GetAddress(), 1);
    EXPECT_EQ(ret, BluetoothAudioManager::SET_WEARDETECTION_SUCCESS);
    BluetoothAudioManager::GetInstance().UpdateDeviceWearState(remoteDevice, 0, 0);
    HILOGI("audio_manager_test_UpdateDeviceWearState_002 end");
}

/*
 * @tc.number:audio_manager_test_UpdateDeviceWearState_003
 * @tc.name: UpdateDeviceWearState
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_UpdateDeviceWearState_003, TestSize.Level1)
{
    HILOGI("audio_manager_test_UpdateDeviceWearState_003 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    int32_t ret = BluetoothAudioManager::GetInstance().SetAudioManagerSupport(remoteDevice->GetAddress(), 0);
    EXPECT_EQ(ret, BluetoothAudioManager::SET_WEARDETECTION_SUCCESS);
    BluetoothAudioManager::GetInstance().UpdateDeviceWearState(remoteDevice, 0, 0);
    HILOGI("audio_manager_test_UpdateDeviceWearState_003 end");
}


/*
 * @tc.number:audio_manager_test_UpdateDeviceWearStateIfSupport_001
 * @tc.name: UpdateDeviceWearStateIfSupport
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_UpdateDeviceWearStateIfSupport_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_UpdateDeviceWearStateIfSupport_001 enter");
    std::string address = "00:01:02:03:04:05";
    BluetoothAudioManager::WearingStateRecord record(false, false);
    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.EnsureInsert(address, record);
    RawAddress device = RawAddress(address);
    EXPECT_FALSE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    BluetoothAudioManager::GetInstance().UpdateDeviceWearStateIfSupport(address, 1, 1);
    HILOGI("audio_manager_test_UpdateDeviceWearStateIfSupport_001 end");
}

/*
 * @tc.number:audio_manager_test_UpdateDeviceWearStateIfSupport_002
 * @tc.name: UpdateDeviceWearStateIfSupport
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_UpdateDeviceWearStateIfSupport_002, TestSize.Level1)
{
    HILOGI("audio_manager_test_UpdateDeviceWearStateIfSupport_002 enter");
    std::string address = "00:01:02:03:04:05";
    BluetoothAudioManager::WearingStateRecord record(false, false);
    std::string recordAddr = "00:01:02:07:05:A1";
    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.EnsureInsert(recordAddr, record);
    RawAddress device = RawAddress(recordAddr);
    EXPECT_FALSE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    BluetoothAudioManager::GetInstance().UpdateDeviceWearStateIfSupport(address, 1, 1);
    BluetoothAudioManager::GetInstance().UpdateDeviceWearStateIfSupport(address, 0, 0);
    HILOGI("audio_manager_test_UpdateDeviceWearStateIfSupport_002 end");
}

/*
 * @tc.number:adapter_manager_test_ChangePlayStateIfNeeded_001
 * @tc.name: ChangePlayStateIfNeeded
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_ChangePlayStateIfNeeded_001, TestSize.Level1)
{
    HILOGI("adapter_manager_test_ChangePlayStateIfNeeded_001 enter");
    std::string address = "00:01:02:03:04:05";
    BluetoothAudioManager::WearingStateRecord record(false, false);
    BluetoothAudioManager::GetInstance().ChangePlayStateIfNeeded(address, 1, record, true, false);

    BluetoothAudioManager::WearingStateRecord record1(true, true);
    ProfileServiceManager::GetInstance()->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, "A2dpSrcService");
    BluetoothAudioManager::GetInstance().ChangePlayStateIfNeeded(address, 0, record1, true, false);
    BluetoothAudioManager::GetInstance().ChangePlayStateIfNeeded(address, 4, record1, false, false);
    BluetoothAudioManager::GetInstance().ChangePlayStateIfNeeded(address, 4, record1, true, false);

    BluetoothAudioManager::WearingStateRecord record2(false, false);
    BluetoothAudioManager::GetInstance().ChangePlayStateIfNeeded(address, 1, record2, true, false);
    BluetoothAudioManager::GetInstance().ChangePlayStateIfNeeded(address, 1, record2, false, false);

    BluetoothAudioManager::WearingStateRecord record3(false, true);
    BluetoothAudioManager::GetInstance().ChangePlayStateIfNeeded(address, 7, record3, true, false);
    RawAddress device = RawAddress(address);
    EXPECT_FALSE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    HILOGI("adapter_manager_test_ChangePlayStateIfNeeded_001 end");
}

/*
 * @tc.number:adapter_manager_test_SetAudioManagerSupport_001
 * @tc.name: SetAudioManagerSupport
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_SetAudioManagerSupport_001, TestSize.Level1)
{
    HILOGI("adapter_manager_test_SetAudioManagerSupport_001 enter");
    std::string address = "00:01:02:03:04:05";
    int32_t ret = BluetoothAudioManager::GetInstance().SetAudioManagerSupport(address, false);
    EXPECT_EQ(ret, BluetoothAudioManager::SET_WEARDETECTION_SUCCESS);
    HILOGI("adapter_manager_test_SetAudioManagerSupport_001 end");
}

/*
 * @tc.number:adapter_manager_test_SetAudioManagerSupport_002
 * @tc.name: SetAudioManagerSupport
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_SetAudioManagerSupport_002, TestSize.Level1)
{
    HILOGI("adapter_manager_test_SetAudioManagerSupport_002 enter");
    std::string address = "00:01:02:03:04:05";
    int32_t ret = BluetoothAudioManager::GetInstance().SetAudioManagerSupport(address, true);
    EXPECT_EQ(ret, BluetoothAudioManager::SET_WEARDETECTION_SUCCESS);
    HILOGI("adapter_manager_test_SetAudioManagerSupport_002 end");
}

/*
 * @tc.number:adapter_manager_test_RemoveDisconnectDevice_001
 * @tc.name: RemoveDisconnectDevice
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_RemoveDisconnectDevice_001, TestSize.Level1)
{
    HILOGI("adapter_manager_test_RemoveDisconnectDevice_001 enter");
    std::string address = "00:01:02:03:04:05";
    BluetoothAudioManager::GetInstance().RemoveDisconnectDevice(address);
    uint8_t a2dpState = 1;
    uint8_t hfpState = 1;
    EXPECT_FALSE(BluetoothAudioManager::GetInstance().GetProfileStatus(address, a2dpState, hfpState));
    HILOGI("adapter_manager_test_RemoveDisconnectDevice_001 end");
}

/*
 * @tc.number:adapter_manager_test_IsDeviceWearing_001
 * @tc.name: IsDeviceWearing
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_IsDeviceWearing_001, TestSize.Level1)
{
    HILOGI("adapter_manager_test_IsDeviceWearing_001 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device = RawAddress(address);
    EXPECT_FALSE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    HILOGI("adapter_manager_test_IsDeviceWearing_001 end");
}

/*
 * @tc.number:adapter_manager_test_IsDeviceWearing_002
 * @tc.name: IsDeviceWearing
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_IsDeviceWearing_002, TestSize.Level1)
{
    HILOGI("adapter_manager_test_IsDeviceWearing_002 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device = RawAddress(address);
    std::shared_ptr<BluetoothAudioManagerInterface> it = std::make_shared<BluetoothAudioManagerInterface>();
    if (it != nullptr) {
        EXPECT_FALSE(it->IsDeviceWearing(device));
    }
    HILOGI("adapter_manager_test_IsDeviceWearing_002 end");
}

/*
 * @tc.number:adapter_manager_test_IsDeviceWearing_003
 * @tc.name: IsDeviceWearing
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_IsDeviceWearing_003, TestSize.Level1)
{
    HILOGI("adapter_manager_test_IsDeviceWearing_003 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    BluetoothAudioManager::GetInstance().UpdateDeviceWearState(remoteDevice, false, false);
    EXPECT_FALSE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    bool isActiveDevWill = false;
    BluetoothAudioManager::GetInstance().ChangeActiveDeviceIfNeeded(address, true, false, isActiveDevWill);
    EXPECT_TRUE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    HILOGI("adapter_manager_test_IsDeviceWearing_003 end");
}

/*
 * @tc.number:adapter_manager_test_GetWearState_001
 * @tc.name: GetWearState
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_GetWearState_001, TestSize.Level1)
{
    HILOGI("adapter_manager_test_GetWearState_001 enter");
    std::string address = "00:01:02:03:04:05";
    int support = -1;
    int res = BluetoothAudioManager::GetInstance().SetAudioManagerSupport(address, support);
    EXPECT_EQ(res, BluetoothAudioManager::SET_WEARDETECTION_SUCCESS);
    int ability = 0;
    int32_t ret = BluetoothAudioManager::GetInstance().GetWearState(address, ability);
    EXPECT_EQ(ret, NO_ERROR);
    std::shared_ptr<BluetoothAudioManagerInterface> it = std::make_shared<BluetoothAudioManagerInterface>();
    if (it != nullptr) {
        EXPECT_EQ(it->BtAudioManagerGetWearState(address, ability), NO_ERROR);
    }
    BluetoothAudioManager::GetInstance().ClearWearDetectionSupport(address);
    HILOGI("adapter_manager_test_GetWearState_001 end");
}

/*
 * @tc.number:audio_manager_test_GetAudioManagerSupport_001
 * @tc.name: GetAudioManagerSupport
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetAudioManagerSupport_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetAudioManagerSupport_001 enter");
    std::string address = "00:01:02:03:04:05";
    int support = 1;
    BluetoothAudioManager::GetInstance().ChangeActiveForSupportChange(address, support);
    int supportValue = BluetoothAudioManager::GetInstance().GetAudioManagerSupport(address);
    EXPECT_NE(supportValue, BluetoothAudioManager::SUPPORT_SYSTEM_ERROR);
    HILOGI("audio_manager_test_GetAudioManagerSupport_001 end");
}

/*
 * @tc.number:audio_manager_test_IsAudioOutputToBluetooth_001
 * @tc.name: IsAudioOutputToBluetooth
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_IsAudioOutputToBluetooth_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_IsAudioOutputToBluetooth_001 enter");
    bool res = BluetoothAudioManager::GetInstance().IsAudioOutputToBluetooth();
    EXPECT_EQ(res, false);
    HILOGI("audio_manager_test_IsAudioOutputToBluetooth_001 end");
}

/*
 * @tc.number:audio_manager_test_ShouldHandleAsWearAction_001
 * @tc.name: ShouldHandleAsWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_ShouldHandleAsWearAction_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_ShouldHandleAsWearAction_001 enter");
    std::string address = "00:01:02:03:04:05";
    bool res = BluetoothAudioManager::GetInstance().ShouldHandleAsWearAction(address, 0, 0);
    EXPECT_EQ(res, false);
    HILOGI("audio_manager_test_ShouldHandleAsWearAction_001 end");
}

/*
 * @tc.number:audio_manager_test_ShouldHandleAsWearAction_002
 * @tc.name: ShouldHandleAsWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_ShouldHandleAsWearAction_002, TestSize.Level1)
{
    HILOGI("audio_manager_test_ShouldHandleAsWearAction_002 enter");
    std::string address = "00:01:02:03:04:05";
    bool res = BluetoothAudioManager::GetInstance().ShouldHandleAsWearAction(address, 1, 1);
    EXPECT_EQ(res, false);
    HILOGI("audio_manager_test_ShouldHandleAsWearAction_002 end");
}

/*
 * @tc.number:audio_manager_test_ShouldHandleAsWearAction_003
 * @tc.name: ShouldHandleAsWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_ShouldHandleAsWearAction_003, TestSize.Level1)
{
    HILOGI("audio_manager_test_ShouldHandleAsWearAction_003 enter");
    std::string address = "00:01:02:03:04:05";
    bool res = BluetoothAudioManager::GetInstance().ShouldHandleAsWearAction(address, 1, 1);
    EXPECT_EQ(res, false);
    HILOGI("audio_manager_test_ShouldHandleAsWearAction_003 end");
}

/*
 * @tc.number:audio_manager_test_ResumePlayIfNeeded_001
 * @tc.name: ResumePlayIfNeeded
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_ResumePlayIfNeeded_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_ResumePlayIfNeeded_001 enter");
    std::string address = "00:01:02:03:04:05";
    BluetoothAudioManager::GetInstance().mPauseRecordMap.Clear();
    BluetoothAudioManager::GetInstance().ResumePlayIfNeeded(address, 1, 0);
    EXPECT_EQ(BluetoothAudioManager::GetInstance().mPauseRecordMap.Size(), 0);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_NO_NEED,
        GetTimeStamp() - MAX_RESUME_PLAY_TIME_SIMPLE));
    BluetoothAudioManager::GetInstance().ResumePlayIfNeeded(address,
        BluetoothAudioManagerInfo::ACTION_ADD_DOUBLE, GetTimeStamp());
    EXPECT_EQ(BluetoothAudioManager::GetInstance().mPauseRecordMap.Size(), 0);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_LEFT,
        GetTimeStamp() - MAX_RESUME_PLAY_TIME_SIMPLE));
    BluetoothAudioManager::GetInstance().ResumePlayIfNeeded(address,
        BluetoothAudioManagerInfo::ACTION_NONE, GetTimeStamp());
    EXPECT_EQ(BluetoothAudioManager::GetInstance().mPauseRecordMap.Size(), 0);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_LEFT,
        GetTimeStamp() - MAX_RESUME_PLAY_TIME_SIMPLE - DOUBLE_REMOVE_CHECK_TIME));
    BluetoothAudioManager::GetInstance().ResumePlayIfNeeded(address,
        BluetoothAudioManagerInfo::ACTION_NONE, GetTimeStamp());
    EXPECT_EQ(BluetoothAudioManager::GetInstance().mPauseRecordMap.Size(), 0);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_LEFT,
        GetTimeStamp() - MAX_RESUME_PLAY_TIME_SIMPLE));
    BluetoothAudioManager::GetInstance().ResumePlayIfNeeded(address,
        BluetoothAudioManagerInfo::ACTION_ADD_DOUBLE, GetTimeStamp());
    EXPECT_EQ(BluetoothAudioManager::GetInstance().mPauseRecordMap.Size(), 0);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_DOUBLE,
        GetTimeStamp() - DOUBLE_REMOVE_CHECK_TIME));
    BluetoothAudioManager::GetInstance().ResumePlayIfNeeded(address,
        BluetoothAudioManagerInfo::ACTION_NONE, GetTimeStamp());
    EXPECT_EQ(BluetoothAudioManager::GetInstance().mPauseRecordMap.Size(), 0);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_DOUBLE,
        GetTimeStamp() - MAX_RESUME_PLAY_TIME_SIMPLE - DOUBLE_REMOVE_CHECK_TIME));
    BluetoothAudioManager::GetInstance().ResumePlayIfNeeded(address,
        BluetoothAudioManagerInfo::ACTION_NONE, GetTimeStamp());
    EXPECT_EQ(BluetoothAudioManager::GetInstance().mPauseRecordMap.Size(), 0);

    HILOGI("audio_manager_test_ResumePlayIfNeeded_001 end");
}

/*
 * @tc.number:audio_manager_test_PauseMediaAndRecordIfNeeded_001
 * @tc.name: PauseMediaAndRecordIfNeeded
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_PauseMediaAndRecordIfNeeded_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_PauseMediaAndRecordIfNeeded_001 enter");
    std::string address = "00:01:02:03:04:05";
    BluetoothAudioManager::GetInstance().PauseMediaAndRecordIfNeeded(address,
        BluetoothAudioManagerInfo::PAUSE_NO_NEED, 0);
    BluetoothAudioManager::GetInstance().PauseMediaAndRecordIfNeeded(address,
        BluetoothAudioManagerInfo::PAUSE_REASON_LEFT_REMOVE_LEFT, 0);
    BluetoothAudioManager::PauseRecord pauseRecord;
    EXPECT_TRUE(BluetoothAudioManager::GetInstance().mPauseRecordMap.Find(address, pauseRecord));
    HILOGI("audio_manager_test_PauseMediaAndRecordIfNeeded_001 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReason_001
 * @tc.name: GetPauseReason
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReason_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReason_001 enter");
    std::string address = "00:01:02:03:04:05";
    int currentPlayState = 1;
    int action = 5;
    BluetoothAudioManager::WearingStateRecord record(true, false);
    int res = BluetoothAudioManager::GetInstance().GetPauseReason(address, action, record, currentPlayState);
    EXPECT_EQ(res, 1);
    HILOGI("audio_manager_test_GetPauseReason_001 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReason_002
 * @tc.name: GetPauseReason
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReason_002, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReason_002 enter");
    std::string address = "00:01:02:03:04:05";
    int currentPlayState = 1;
    int action = 6;
    BluetoothAudioManager::WearingStateRecord record(false, true);
    int res = BluetoothAudioManager::GetInstance().GetPauseReason(address, action, record, currentPlayState);
    EXPECT_EQ(res, 2);
    HILOGI("audio_manager_test_GetPauseReason_002 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReason_003
 * @tc.name: GetPauseReason
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReason_003, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReason_003 enter");
    std::string address = "00:01:02:03:04:05";
    int currentPlayState = 1;
    int action = 5;
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int res = BluetoothAudioManager::GetInstance().GetPauseReason(address, action, record, currentPlayState);
    EXPECT_EQ(res, 3);
    HILOGI("audio_manager_test_GetPauseReason_003 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReason_004
 * @tc.name: GetPauseReason
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReason_004, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReason_004 enter");
    std::string address = "00:01:02:03:04:05";
    int currentPlayState = 1;
    int action = 6;
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int res = BluetoothAudioManager::GetInstance().GetPauseReason(address, action, record, currentPlayState);
    EXPECT_EQ(res, 4);
    HILOGI("audio_manager_test_GetPauseReason_004 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReason_005
 * @tc.name: GetPauseReason
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReason_005, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReason_005 enter");
    std::string address = "00:01:02:03:04:05";
    int currentPlayState = 1;
    int action = 4;
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int res = BluetoothAudioManager::GetInstance().GetPauseReason(address, action, record, currentPlayState);
    EXPECT_EQ(res, 5);
    HILOGI("audio_manager_test_GetPauseReason_005 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReason_006
 * @tc.name: GetPauseReason
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReason_006, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReason_006 enter");
    std::string address = "00:01:02:03:04:05";
    int currentPlayState = 1;
    int action = -1;
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int res = BluetoothAudioManager::GetInstance().GetPauseReason(address, action, record, currentPlayState);
    EXPECT_EQ(res, -1);
    HILOGI("audio_manager_test_GetPauseReason_006 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReason_007
 * @tc.name: GetPauseReason
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReason_007, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReason_007 enter");
    std::string address = "00:01:02:03:04:05";
    int currentPlayState = 1;
    int action = -1;
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int res = BluetoothAudioManager::GetInstance().GetPauseReason(address, action, record, currentPlayState);
    EXPECT_EQ(res, -1);
    HILOGI("audio_manager_test_GetPauseReason_007 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReason_008
 * @tc.name: GetPauseReason
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReason_008, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReason_008 enter");
    std::string address = "00:01:02:03:04:05";
    int currentPlayState = 1;
    int action = -1;
    BluetoothAudioManager::WearingStateRecord record(false, false);
    int res = BluetoothAudioManager::GetInstance().GetPauseReason(address, action, record, currentPlayState);
    EXPECT_EQ(res, -1);
    HILOGI("audio_manager_test_GetPauseReason_008 end");
}

/*
 * @tc.number:audio_manager_test_GetPauseReasonNotPlaying_001
 * @tc.name: GetPauseReasonNotPlaying
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_GetPauseReasonNotPlaying_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_GetPauseReasonNotPlaying_001 enter");
    std::string address = "00:01:02:03:04:05";
    int action = -1;
    BluetoothAudioManager::WearingStateRecord record(false, false);
    BluetoothAudioManager::GetInstance().mPauseRecordMap.Clear();
    int res = BluetoothAudioManager::GetInstance().GetPauseReasonNotPlaying(address, action, record);
    EXPECT_EQ(res, -1);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_LEFT,
        GetTimeStamp() - MAX_RESUME_PLAY_TIME_SIMPLE));
    BluetoothAudioManager::WearingStateRecord record2(false, true);
    res = BluetoothAudioManager::GetInstance().GetPauseReasonNotPlaying(address,
        BluetoothAudioManagerInfo::ACTION_REMOVE_RIGHT, record2);
    EXPECT_EQ(res, BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_DOUBLE);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_RIGHT,
        GetTimeStamp() - MAX_RESUME_PLAY_TIME_SIMPLE));
    BluetoothAudioManager::WearingStateRecord record3(true, false);
    res = BluetoothAudioManager::GetInstance().GetPauseReasonNotPlaying(address,
        BluetoothAudioManagerInfo::ACTION_REMOVE_LEFT, record3);
    EXPECT_EQ(res, BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_DOUBLE);

    BluetoothAudioManager::GetInstance().mPauseRecordMap.EnsureInsert(address,
        BluetoothAudioManager::PauseRecord(BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_RIGHT,
        GetTimeStamp() - MAX_RESUME_PLAY_TIME_SIMPLE));
    BluetoothAudioManager::WearingStateRecord record4(true, false);
    res = BluetoothAudioManager::GetInstance().GetPauseReasonNotPlaying(address,
        BluetoothAudioManagerInfo::PAUSE_NO_NEED, record4);
    EXPECT_EQ(res, BluetoothAudioManagerInfo::PAUSE_NO_NEED);

    HILOGI("audio_manager_test_GetPauseReasonNotPlaying_001 end");
}

/*
 * @tc.number:audio_manager_test_IsWearDetectionSupported_001
 * @tc.name: IsWearDetectionSupported
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_IsWearDetectionSupported_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_IsWearDetectionSupported_001 enter");
    std::string address = "00:01:02:03:04:05";
    bool isActiveDevWill = false;
    BluetoothAudioManager::GetInstance().ChangeActiveDeviceIfNeeded(address, false, false, isActiveDevWill);
    RawAddress device(address);
    bool isSupport = false;
    int ret = BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device, isSupport);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("audio_manager_test_IsWearDetectionSupported_001 end");
}

/*
 * @tc.number:audio_manager_test_SetProfileStatus_001
 * @tc.name: UpdateProfileStatusMap
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_SetProfileStatus_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_SetProfileStatus_001 enter");
    std::string addr = "AE:08:33:FC:00:2B";
    uint8_t a2dpState = 2;
    uint8_t hfpState = 2;
    BluetoothAudioManager::GetInstance().SetProfileStatus(addr, a2dpState, hfpState);
    EXPECT_TRUE(BluetoothAudioManager::GetInstance().GetProfileStatus(addr, a2dpState, hfpState));
    HILOGI("audio_manager_test_SetProfileStatus_001 end");
}

/*
 * @tc.number:audio_manager_test_StopOffloadWhenA2dpReleased_001
 * @tc.name: StopOffloadWhenA2dpReleased
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_StopOffloadWhenA2dpReleased_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_StopOffloadWhenA2dpReleased_001 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device(address);
    int useA2dp = 2;
    BluetoothAudioManager::GetInstance().StopOffloadWhenA2dpReleased(device, useA2dp);
    BluetoothAudioManager::GetInstance().ProcessAudioRenderListenerWhenUserSelect(useA2dp);
    EXPECT_TRUE(BluetoothAudioFrameworkAdapter::GetInstance().audioOutputPipeCallback_ != nullptr);
    useA2dp = 1;
    BluetoothAudioManager::GetInstance().StopOffloadWhenA2dpReleased(device, useA2dp);
    BluetoothAudioManager::GetInstance().ProcessAudioRenderListenerWhenUserSelect(useA2dp);
    HILOGI("audio_manager_test_StopOffloadWhenA2dpReleased_001 end");
}

/*
 * @tc.number:audio_manager_test_SaveHiechoAvLatency_001
 * @tc.name: SaveHiechoAvLatency
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_SaveHiechoAvLatency_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_SaveHiechoAvLatency_001 enter");
    std::string address = "00:01:02:03:04:05";
    RawAddress device(address);
    BluetoothAudioManager::GetInstance().SaveHiechoAvLatency(device, 320);
    EXPECT_EQ(BluetoothAudioManager::GetInstance().avSyncManager_.GetAvSyncLatency(device), 320);
    BluetoothAudioManager::GetInstance().UpdateBtAudioLatency(device);
    BluetoothAudioManager::GetInstance().ClearLatencyInfoByAddr(device);
    BluetoothAudioManager::GetInstance().ClearLatencyInfo();
    HILOGI("audio_manager_test_SaveHiechoAvLatency_001 end");
}

/*
 * @tc.number:audio_manager_test_SetSharedPreVirtualAutoConnSwitch_001
 * @tc.name: SetSharedPreVirtualAutoConnSwitch
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_SetSharedPreVirtualAutoConnSwitch_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_SetSharedPreVirtualAutoConnSwitch_001 enter");
    std::string address = "00:01:02:03:04:05";
    BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(address, false);
    bool ret = BluetoothAudioManager::GetInstance().GetSharedPreVirtualAutoConnSwitchValue(address);
    EXPECT_EQ(ret, false);
    BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(address, true);
    ret = BluetoothAudioManager::GetInstance().GetSharedPreVirtualAutoConnSwitchValue(address);
    EXPECT_EQ(ret, true);
    HILOGI("audio_manager_test_SetSharedPreVirtualAutoConnSwitch_001 end");
}

/*
 * @tc.number:audio_manager_test_UnWearActionNotifyHdapDisConnected_001
 * @tc.name: UnWearActionNotifyHdapDisConnected
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerTest, audio_manager_test_UnWearActionNotifyHdapDisConnected_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_UnWearActionNotifyHdapDisConnected_001 enter");
    std::string addr = "00:01:02:03:04:05";
    RawAddress address = RawAddress(addr);
    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.Clear();
    BluetoothAudioManager::WearingStateRecord record1(true, true);
    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.EnsureInsert(addr, record1);
    BluetoothAudioManager::GetInstance().UnWearActionNotifyHdapDisConnected(address,
        UpdateOutputStackAction::ACTION_WEAR);

    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.Clear();
    BluetoothAudioManager::WearingStateRecord record2(true, true);
    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.EnsureInsert(addr, record2);
    BluetoothAudioManager::GetInstance().UnWearActionNotifyHdapDisConnected(address,
        UpdateOutputStackAction::ACTION_UNWEAR);

    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.Clear();
    BluetoothAudioManager::WearingStateRecord record3(false, false);
    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.EnsureInsert(addr, record3);
    BluetoothAudioManager::GetInstance().UnWearActionNotifyHdapDisConnected(address,
        UpdateOutputStackAction::ACTION_WEAR);

    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.Clear();
    BluetoothAudioManager::WearingStateRecord record4(false, false);
    BluetoothAudioManager::GetInstance().mAudioManagerRecordMap_.EnsureInsert(addr, record4);
    RawAddress device = RawAddress(addr);
    EXPECT_FALSE(BluetoothAudioManager::GetInstance().IsDeviceWearing(device));
    BluetoothAudioManager::GetInstance().UnWearActionNotifyHdapDisConnected(address,
        UpdateOutputStackAction::ACTION_UNWEAR);
    HILOGI("audio_manager_test_UnWearActionNotifyHdapDisConnected_001 end");
}

/**
 * @tc.name: adapter_manager_test_ChangePlayStateForRemoveAction_001
 * @tc.desc: ChangePlayStateForRemoveAction evaluates last-send-play window
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_ChangePlayStateForRemoveAction_001, TestSize.Level1)
{
    HILOGI("adapter_manager_test_ChangePlayStateForRemoveAction_001 enter");
    std::string address = "00:11:22:33:44:55";
    BluetoothAudioManager::WearingStateRecord record(false, false);
    BluetoothAudioManager::GetInstance().lastSendPlayTime_ = 100000;
    EXPECT_NO_FATAL_FAILURE(BluetoothAudioManager::GetInstance().ChangePlayStateForRemoveAction(
        address, 1, record, -1, 100500));
    EXPECT_NO_FATAL_FAILURE(BluetoothAudioManager::GetInstance().ChangePlayStateForRemoveAction(
        address, 1, record, -1, 200000));
    HILOGI("adapter_manager_test_ChangePlayStateForRemoveAction_001 end");
}


/**
 * @tc.name: adapter_manager_test_HandleWearStateChanged_001
 * @tc.desc: wear state changed computes active device by a2dp sink/wear state
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothAudioManagerTest, adapter_manager_test_HandleWearStateChanged_001, TestSize.Level1)
{
    HILOGI("adapter_manager_test_HandleWearStateChanged_001 enter");
    std::string address = "00:11:22:33:44:55";
    BluetoothAudioManager::WearingStateRecord record(false, false);
    EXPECT_NO_FATAL_FAILURE(BluetoothAudioManager::GetInstance().HandleWearStateChanged(
        address, true, true, record));
    EXPECT_NO_FATAL_FAILURE(BluetoothAudioManager::GetInstance().HandleWearStateChanged(
        address, false, false, BluetoothAudioManager::WearingStateRecord(true, true)));
    HILOGI("adapter_manager_test_HandleWearStateChanged_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS
