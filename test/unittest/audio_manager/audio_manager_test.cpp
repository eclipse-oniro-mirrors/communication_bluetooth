/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

constexpr int TIME = 4;
static BluetoothHost *host_;

class AudioManagerTest : public testing::Test {
public:
    AudioManagerTest();
    ~AudioManagerTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void DisableBt();

private:
};

AudioManagerTest::AudioManagerTest()
{
}

AudioManagerTest::~AudioManagerTest()
{
}

void AudioManagerTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}
void AudioManagerTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void AudioManagerTest::SetUp()
{
}
void AudioManagerTest::TearDown()
{
}

void AudioManagerTest::DisableBt()
{
    int btState = host_->GetBtState();
    HILOGI("DisableBt start, btState = %{public}d", btState);
    if ((btState != BTStateID::STATE_TURNING_OFF) && (btState != BTStateID::STATE_TURN_OFF)) {
        host_->DisableBt();
        sleep(TIME);
    }
}

/*
 * @tc.number: AudioManager_IsWearDetectionSupported_001
 * @tc.name: IsWearDetectionSupported
 * @tc.desc:
 */
HWTEST_F(AudioManagerTest, AudioManager_IsWearDetectionSupported_001, TestSize.Level1)
{
    HILOGI("AudioManager_IsWearDetectionSupported_001 start");
    bool isSupported = false;
    BluetoothRemoteDevice device("11:11:11:11:11:11", BT_TRANSPORT_BREDR);
    BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device, isSupported);
    EXPECT_FALSE(isSupported);
    HILOGI("isSupported = %{public}d", isSupported);
    HILOGI("AudioManager_IsWearDetectionSupported_001 end");
}

/*
 * @tc.number: AudioManager_IsWearDetectionSupported_002
 * @tc.name: IsWearDetectionSupported
 * @tc.desc:
 */
HWTEST_F(AudioManagerTest, AudioManager_IsWearDetectionSupported_002, TestSize.Level1)
{
    HILOGI("AudioManager_IsWearDetectionSupported_002 start");
    bool isSupported = false;
    BluetoothRemoteDevice device("abcde", BT_TRANSPORT_BREDR);
    int error = BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device, isSupported);
    EXPECT_EQ(error, BT_ERR_INVALID_PARAM);
    HILOGI("AudioManager_IsWearDetectionSupported_002 end");
}

HWTEST_F(AudioManagerTest, AudioManager_IsDeviceWearing_001, TestSize.Level1)
{
    HILOGI("AudioManager_IsDeviceWearing_001 start");
    BluetoothRemoteDevice device("11:11:11:11:11:11", BT_TRANSPORT_BREDR);
    int error = BluetoothAudioManager::GetInstance().IsDeviceWearing(device);
    EXPECT_EQ(error, BT_NO_ERROR);
    HILOGI("AudioManager_IsDeviceWearing_001 end");
}

HWTEST_F(AudioManagerTest, AudioManager_IsDeviceWearing_002, TestSize.Level1)
{
    HILOGI("AudioManager_IsDeviceWearing_002 start");
    BluetoothRemoteDevice device("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int error = BluetoothAudioManager::GetInstance().IsDeviceWearing(device);
    EXPECT_EQ(error, BT_ERR_INTERNAL_ERROR);
    HILOGI("AudioManager_IsDeviceWearing_002 end");
}

/*
 * @tc.number: AudioManager_SendDeviceSelection_001
 * @tc.name: IsWearDetectionSupported
 * @tc.desc:
 */
HWTEST_F(AudioManagerTest, AudioManager_SendDeviceSelection_001, TestSize.Level1)
{
    HILOGI("AudioManager_SendDeviceSelection_001 start");
    BluetoothRemoteDevice device("11:11:11:11:11:11", BT_TRANSPORT_BREDR);
    int error = BluetoothAudioManager::GetInstance().SendDeviceSelection(device, 1, 2, 3);
    EXPECT_EQ(error, BT_ERR_INTERNAL_ERROR);
    HILOGI("AudioManager_SendDeviceSelection_001 end");
}

/*
 * @tc.number: AudioManager_SendDeviceSelection_002
 * @tc.name: IsWearDetectionSupported
 * @tc.desc:
 */
HWTEST_F(AudioManagerTest, AudioManager_SendDeviceSelection_002, TestSize.Level1)
{
    HILOGI("AudioManager_SendDeviceSelection_002 start");
    BluetoothRemoteDevice device("abcdef", BT_TRANSPORT_BREDR);
    int error = BluetoothAudioManager::GetInstance().SendDeviceSelection(device, 1, 2, 3);
    EXPECT_EQ(error, BT_ERR_INVALID_PARAM);
    HILOGI("AudioManager_SendDeviceSelection_002 end");
}

/*
 * @tc.number: AudioManager_IsWearDetectionSupported_003
 * @tc.name: IsWearDetectionSupported
 * @tc.desc:
 */
HWTEST_F(AudioManagerTest, AudioManager_IsWearDetectionSupported_003, TestSize.Level1)
{
    HILOGI("AudioManager_IsWearDetectionSupported_003 start");
    host_ = &BluetoothHost::GetDefaultHost();
    host_->DisableBt();
    host_->DisableBle();
    sleep(TIME);

    bool isSupported = false;
    BluetoothRemoteDevice device("11:11:11:11:11:11", BT_TRANSPORT_BREDR);
    int error = BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device, isSupported);
    EXPECT_EQ(error, BT_ERR_INVALID_STATE);
    HILOGI("AudioManager_IsWearDetectionSupported_003 end");
}

/*
 * @tc.number: AudioManager_SendDeviceSelection_003
 * @tc.name: IsWearDetectionSupported
 * @tc.desc:
 */
HWTEST_F(AudioManagerTest, AudioManager_SendDeviceSelection_003, TestSize.Level1)
{
    HILOGI("AudioManager_SendDeviceSelection_003 start");
    host_ = &BluetoothHost::GetDefaultHost();
    host_->DisableBt();
    host_->DisableBle();
    sleep(TIME);

    BluetoothRemoteDevice device("11:11:11:11:11:11", BT_TRANSPORT_BREDR);
    int error = BluetoothAudioManager::GetInstance().SendDeviceSelection(device, 1, 2, 3);
    EXPECT_EQ(error, BT_ERR_INVALID_STATE);
    HILOGI("AudioManager_SendDeviceSelection_003 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
