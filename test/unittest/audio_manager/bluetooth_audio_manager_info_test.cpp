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
#include "bluetooth_audio_manager_info.h"
#include "bluetooth_host.h"
#include "bluetooth_audio_manager_info.h"
#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;

class BluetoothAudioManagerInfoTest : public testing::Test {
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
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

/*
 * @tc.number:audio_manager_test_001
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_001, TestSize.Level1)
{
    HILOGI("audio_manager_test_001 enter");
    int action = 0;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "ACTION_NONE");
    HILOGI("audio_manager_test_001 end");
}

/*
 * @tc.number:audio_manager_test_002
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_002, TestSize.Level1)
{
    HILOGI("audio_manager_test_002 enter");
    int action = 1;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "ACTION_ADD_DOUBLE");
    HILOGI("audio_manager_test_002 end");
}

/*
 * @tc.number:audio_manager_test_003
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_003, TestSize.Level1)
{
    HILOGI("audio_manager_test_003 enter");
    int action = 2;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "ACTION_ADD_LEFT");
    HILOGI("audio_manager_test_003 end");
}

/*
 * @tc.number:audio_manager_test_004
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_004, TestSize.Level1)
{
    HILOGI("audio_manager_test_004 enter");
    int action = 3;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "ACTION_ADD_RIGHT");
    HILOGI("audio_manager_test_004 end");
}

/*
 * @tc.number:audio_manager_test_005
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_005, TestSize.Level1)
{
    HILOGI("audio_manager_test_005 enter");
    int action = 4;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "ACTION_REMOVE_DOUBLE");
    HILOGI("audio_manager_test_005 end");
}

/*
 * @tc.number:audio_manager_test_006
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_006, TestSize.Level1)
{
    HILOGI("audio_manager_test_006 enter");
    int action = 5;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "ACTION_REMOVE_LEFT");
    HILOGI("audio_manager_test_006 end");
}

/*
 * @tc.number:audio_manager_test_007
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_007, TestSize.Level1)
{
    HILOGI("audio_manager_test_007 enter");
    int action = 6;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "ACTION_REMOVE_RIGHT");
    HILOGI("audio_manager_test_007 end");
}

/*
 * @tc.number:audio_manager_test_008
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_008, TestSize.Level1)
{
    HILOGI("audio_manager_test_008 enter");
    int action = 7;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "ACTION_SWITCH");
    HILOGI("audio_manager_test_008 end");
}

/*
 * @tc.number:audio_manager_test_009
 * @tc.name: GetActionString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_009, TestSize.Level1)
{
    HILOGI("audio_manager_test_009 enter");
    int action = 8;
    std::string ret = BluetoothAudioManagerInfo::GetActionString(action);
    EXPECT_EQ(ret, "UNKNOWN");
    HILOGI("audio_manager_test_009 end");
}

/*
 * @tc.number:audio_manager_test_010
 * @tc.name: GetPauseReasonString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_010, TestSize.Level1)
{
    HILOGI("audio_manager_test_010 enter");
    int pauseReason = -1;
    std::string ret = BluetoothAudioManagerInfo::GetPauseReasonString(pauseReason);
    EXPECT_EQ(ret, "PAUSE_NO_NEED");
    HILOGI("audio_manager_test_010 end");
}

/*
 * @tc.number:audio_manager_test_011
 * @tc.name: GetPauseReasonString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_011, TestSize.Level1)
{
    HILOGI("audio_manager_test_011 enter");
    int pauseReason = 1;
    std::string ret = BluetoothAudioManagerInfo::GetPauseReasonString(pauseReason);
    EXPECT_EQ(ret, "PAUSE_REASON_LEFT_REMOVE_LEFT");
    HILOGI("audio_manager_test_011 end");
}

/*
 * @tc.number:audio_manager_test_012
 * @tc.name: GetPauseReasonString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_012, TestSize.Level1)
{
    HILOGI("audio_manager_test_012 enter");
    int pauseReason = 2;
    std::string ret = BluetoothAudioManagerInfo::GetPauseReasonString(pauseReason);
    EXPECT_EQ(ret, "PAUSE_REASON_RIGHT_REMOVE_RIGHT");
    HILOGI("audio_manager_test_012 end");
}

/*
 * @tc.number:audio_manager_test_013
 * @tc.name: GetPauseReasonString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_013, TestSize.Level1)
{
    HILOGI("audio_manager_test_013 enter");
    int pauseReason = 3;
    std::string ret = BluetoothAudioManagerInfo::GetPauseReasonString(pauseReason);
    EXPECT_EQ(ret, "PAUSE_REASON_DOUBLE_REMOVE_LEFT");
    HILOGI("audio_manager_test_013 end");
}

/*
 * @tc.number:audio_manager_test_014
 * @tc.name: GetPauseReasonString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_014, TestSize.Level1)
{
    HILOGI("audio_manager_test_014 enter");
    int pauseReason = 4;
    std::string ret = BluetoothAudioManagerInfo::GetPauseReasonString(pauseReason);
    EXPECT_EQ(ret, "PAUSE_REASON_DOUBLE_REMOVE_RIGHT");
    HILOGI("audio_manager_test_014 end");
}

/*
 * @tc.number:audio_manager_test_015
 * @tc.name: GetPauseReasonString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_015, TestSize.Level1)
{
    HILOGI("audio_manager_test_015 enter");
    int pauseReason = 5;
    std::string ret = BluetoothAudioManagerInfo::GetPauseReasonString(pauseReason);
    EXPECT_EQ(ret, "PAUSE_REASON_DOUBLE_REMOVE_DOUBLE");
    HILOGI("audio_manager_test_015 end");
}

/*
 * @tc.number:audio_manager_test_016
 * @tc.name: GetPauseReasonString
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_016, TestSize.Level1)
{
    HILOGI("audio_manager_test_016 enter");
    int pauseReason = 6;
    std::string ret = BluetoothAudioManagerInfo::GetPauseReasonString(pauseReason);
    EXPECT_EQ(ret, "UNKNOWN");
    HILOGI("audio_manager_test_016 end");
}

/*
 * @tc.number:audio_manager_test_017
 * @tc.name: GetWearActionRecordWearNone
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_017, TestSize.Level1)
{
    HILOGI("audio_manager_test_017 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearNone(0, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_NONE);
    HILOGI("audio_manager_test_017 end");
}

/*
 * @tc.number:audio_manager_test_018
 * @tc.name: GetWearActionRecordWearNone
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_018, TestSize.Level1)
{
    HILOGI("audio_manager_test_018 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearNone(0, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_RIGHT);
    HILOGI("audio_manager_test_018 end");
}

/*
 * @tc.number:audio_manager_test_019
 * @tc.name: GetWearActionRecordWearNone
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_019, TestSize.Level1)
{
    HILOGI("audio_manager_test_019 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearNone(1, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_LEFT);
    HILOGI("audio_manager_test_019 end");
}

/*
 * @tc.number:audio_manager_test_020
 * @tc.name: GetWearActionRecordWearNone
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_020, TestSize.Level1)
{
    HILOGI("audio_manager_test_020 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearNone(1, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_DOUBLE);
    HILOGI("audio_manager_test_020 end");
}

/*
 * @tc.number:audio_manager_test_021
 * @tc.name: GetWearActionRecordWearDouble
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_021, TestSize.Level1)
{
    HILOGI("audio_manager_test_021 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearDouble(0, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_DOUBLE);
    HILOGI("audio_manager_test_021 end");
}

/*
 * @tc.number:audio_manager_test_022
 * @tc.name: GetWearActionRecordWearDouble
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_022, TestSize.Level1)
{
    HILOGI("audio_manager_test_022 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearDouble(0, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_LEFT);
    HILOGI("audio_manager_test_022 end");
}

/*
 * @tc.number:audio_manager_test_023
 * @tc.name: GetWearActionRecordWearDouble
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_023, TestSize.Level1)
{
    HILOGI("audio_manager_test_023 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearDouble(1, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_RIGHT);
    HILOGI("audio_manager_test_023 end");
}

/*
 * @tc.number:audio_manager_test_024
 * @tc.name: GetWearActionRecordWearDouble
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_024, TestSize.Level1)
{
    HILOGI("audio_manager_test_024 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearDouble(1, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_NONE);
    HILOGI("audio_manager_test_024 end");
}

/*
 * @tc.number:audio_manager_test_026
 * @tc.name: GetWearActionRecordWearLeft
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_026, TestSize.Level1)
{
    HILOGI("audio_manager_test_026 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearLeft(0, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_LEFT);
    HILOGI("audio_manager_test_026 end");
}

/*
 * @tc.number:audio_manager_test_027
 * @tc.name: GetWearActionRecordWearLeft
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_027, TestSize.Level1)
{
    HILOGI("audio_manager_test_027 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearLeft(0, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_SWITCH);
    HILOGI("audio_manager_test_027 end");
}

/*
 * @tc.number:audio_manager_test_028
 * @tc.name: GetWearActionRecordWearLeft
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_028, TestSize.Level1)
{
    HILOGI("audio_manager_test_028 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearLeft(1, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_NONE);
    HILOGI("audio_manager_test_028 end");
}

/*
 * @tc.number:audio_manager_test_029
 * @tc.name: GetWearActionRecordWearLeft
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_029, TestSize.Level1)
{
    HILOGI("audio_manager_test_029 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearLeft(1, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_RIGHT);
    HILOGI("audio_manager_test_029 end");
}

/*
 * @tc.number:audio_manager_test_030
 * @tc.name: GetWearActionRecordWearRight
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_030, TestSize.Level1)
{
    HILOGI("audio_manager_test_030 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearRight(0, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_RIGHT);
    HILOGI("audio_manager_test_030 end");
}

/*
 * @tc.number:audio_manager_test_031
 * @tc.name: GetWearActionRecordWearRight
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_031, TestSize.Level1)
{
    HILOGI("audio_manager_test_031 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearRight(0, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_NONE);
    HILOGI("audio_manager_test_031 end");
}

/*
 * @tc.number:audio_manager_test_032
 * @tc.name: GetWearActionRecordWearRight
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_032, TestSize.Level1)
{
    HILOGI("audio_manager_test_032 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearRight(1, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_SWITCH);
    HILOGI("audio_manager_test_032 end");
}

/*
 * @tc.number:audio_manager_test_033
 * @tc.name: GetWearActionRecordWearRight
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_033, TestSize.Level1)
{
    HILOGI("audio_manager_test_033 enter");
    int ret = BluetoothAudioManagerInfo::GetWearActionRecordWearRight(1, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_LEFT);
    HILOGI("audio_manager_test_033 end");
}

/*
 * @tc.number:audio_manager_test_034
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_034, TestSize.Level1)
{
    HILOGI("audio_manager_test_034 enter");
    BluetoothAudioManager::WearingStateRecord record(false, false);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 0, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_NONE);
    HILOGI("audio_manager_test_034 end");
}

/*
 * @tc.number:audio_manager_test_035
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_035, TestSize.Level1)
{
    HILOGI("audio_manager_test_035 enter");
    BluetoothAudioManager::WearingStateRecord record(false, false);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 0, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_RIGHT);
    HILOGI("audio_manager_test_035 end");
}

/*
 * @tc.number:audio_manager_test_036
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_036, TestSize.Level1)
{
    HILOGI("audio_manager_test_036 enter");
    BluetoothAudioManager::WearingStateRecord record(false, false);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 1, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_LEFT);
    HILOGI("audio_manager_test_036 end");
}

/*
 * @tc.number:audio_manager_test_037
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_037, TestSize.Level1)
{
    HILOGI("audio_manager_test_036 enter");
    BluetoothAudioManager::WearingStateRecord record(false, false);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 1, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_DOUBLE);
    HILOGI("audio_manager_test_036 end");
}


/*
 * @tc.number:audio_manager_test_038
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_038, TestSize.Level1)
{
    HILOGI("audio_manager_test_038 enter");
    BluetoothAudioManager::WearingStateRecord record(false, true);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 0, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_RIGHT);
    HILOGI("audio_manager_test_038 end");
}

/*
 * @tc.number:audio_manager_test_039
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_039, TestSize.Level1)
{
    HILOGI("audio_manager_test_039 enter");
    BluetoothAudioManager::WearingStateRecord record(false, true);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 0, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_NONE);
    HILOGI("audio_manager_test_039 end");
}

/*
 * @tc.number:audio_manager_test_040
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_040, TestSize.Level1)
{
    HILOGI("audio_manager_test_040 enter");
    BluetoothAudioManager::WearingStateRecord record(false, true);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 1, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_SWITCH);
    HILOGI("audio_manager_test_040 end");
}

/*
 * @tc.number:audio_manager_test_041
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_041, TestSize.Level1)
{
    HILOGI("audio_manager_test_041 enter");
    BluetoothAudioManager::WearingStateRecord record(false, true);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 1, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_LEFT);
    HILOGI("audio_manager_test_041 end");
}

/*
 * @tc.number:audio_manager_test_042
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_042, TestSize.Level1)
{
    HILOGI("audio_manager_test_042 enter");
    BluetoothAudioManager::WearingStateRecord record(true, false);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 0, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_LEFT);
    HILOGI("audio_manager_test_042 end");
}

/*
 * @tc.number:audio_manager_test_043
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_043, TestSize.Level1)
{
    HILOGI("audio_manager_test_043 enter");
    BluetoothAudioManager::WearingStateRecord record(true, false);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 0, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_SWITCH);
    HILOGI("audio_manager_test_043 end");
}

/*
 * @tc.number:audio_manager_test_044
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_044, TestSize.Level1)
{
    HILOGI("audio_manager_test_044 enter");
    BluetoothAudioManager::WearingStateRecord record(true, false);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 1, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_NONE);
    HILOGI("audio_manager_test_044 end");
}

/*
 * @tc.number:audio_manager_test_045
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_045, TestSize.Level1)
{
    HILOGI("audio_manager_test_045 enter");
    BluetoothAudioManager::WearingStateRecord record(true, false);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 1, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_ADD_RIGHT);
    HILOGI("audio_manager_test_045 end");
}

/*
 * @tc.number:audio_manager_test_046
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_046, TestSize.Level1)
{
    HILOGI("audio_manager_test_046 enter");
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 0, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_DOUBLE);
    HILOGI("audio_manager_test_046 end");
}

/*
 * @tc.number:audio_manager_test_047
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_047, TestSize.Level1)
{
    HILOGI("audio_manager_test_047 enter");
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 0, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_LEFT);
    HILOGI("audio_manager_test_047 end");
}

/*
 * @tc.number:audio_manager_test_048
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_048, TestSize.Level1)
{
    HILOGI("audio_manager_test_048 enter");
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 1, 0);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_REMOVE_RIGHT);
    HILOGI("audio_manager_test_048 end");
}

/*
 * @tc.number:audio_manager_test_049
 * @tc.name: GetWearAction
 * @tc.desc:
 */
HWTEST_F(BluetoothAudioManagerInfoTest, audio_manager_test_049, TestSize.Level1)
{
    HILOGI("audio_manager_test_049 enter");
    BluetoothAudioManager::WearingStateRecord record(true, true);
    int ret = BluetoothAudioManagerInfo::GetWearAction(record, 1, 1);
    EXPECT_EQ(ret, BluetoothAudioManagerInfo::ACTION_NONE);
    HILOGI("audio_manager_test_049 end");
}
}  // namespace bluetooth
}  // namespace OHOS