/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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
#include "satellite_manager.h"

#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hw_interface.h"
#include "datetime_ex.h"
#include "preferences_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

static const int SATELLITE_CONTROL_MODE_ANTENNA = 0;
static const int SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH = 1;

static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

void HwSatelliteModeSwitch(int handoverDirection)
{}

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwSatelliteModeSwitch = HwSatelliteModeSwitch,
};

namespace OHOS {
namespace bluetooth {
class SatelliteManagerTest : public ::testing::Test {
protected:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}

    void SetUp() override
    {
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
        satelliteManager_ = std::make_shared<SatelliteManager>();
    }

    void TearDown() override
    {
        satelliteManager_ = nullptr;
    }

    std::shared_ptr<SatelliteManager> satelliteManager_ = nullptr;
};

/*
 * @tc.number: satellite_manager_test_001
 * @tc.name: SatelliteManager_SatelliteControl_success
 * @tc.desc:
 */
HWTEST_F(SatelliteManagerTest, satellite_manager_test_001, TestSize.Level1)
{
    HILOGI("satellite_manager_test_001 enter");
    satelliteManager_->Init();
    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_ANTENNA, 1), BT_NO_ERROR);
    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_ANTENNA, 0), BT_NO_ERROR);

    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH, 1), BT_NO_ERROR);
    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH, 0), BT_NO_ERROR);
    HILOGI("satellite_manager_test_001 end");
}

/*
 * @tc.number: satellite_manager_test_002
 * @tc.name: SatelliteManager_SatelliteControl_failed
 * @tc.desc:
 */
HWTEST_F(SatelliteManagerTest, satellite_manager_test_002, TestSize.Level1)
{
    HILOGI("satellite_manager_test_002 enter");
    // no timer trigger
    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH, 1), BT_NO_ERROR);
    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH, 0), BT_NO_ERROR);
    EXPECT_EQ(satelliteManager_->SatelliteControl(-1, 1), BT_ERR_INTERNAL_ERROR);
    HILOGI("satellite_manager_test_002 end");
}

/*
 * @tc.number: satellite_manager_test_003
 * @tc.name: SatelliteManager_Timer
 * @tc.desc:
 */
HWTEST_F(SatelliteManagerTest, satellite_manager_test_003, TestSize.Level1)
{
    HILOGI("satellite_manager_test_003 enter");
    satelliteManager_->Init();
    satelliteManager_->timeoutMs_ = 10; // 10ms timeout

    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH, 1), BT_NO_ERROR);
    EXPECT_TRUE(satelliteManager_->IsSatelliteControlBtSwitch());

    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    EXPECT_FALSE(satelliteManager_->IsSatelliteControlBtSwitch());

    HILOGI("satellite_manager_test_003 end");
}

/*
 * @tc.number: satellite_manager_test_004
 * @tc.name: SatelliteManager_Init
 * @tc.desc:
 */
HWTEST_F(SatelliteManagerTest, satellite_manager_test_004, TestSize.Level1)
{
    HILOGI("satellite_manager_test_004 enter");

    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH, 1), BT_NO_ERROR);
    // Check time error scene
    int64_t nextTime = GetSecondsSince1970ToNow() + 10;
    PreferencesManager::Save(
        satelliteManager_->BT_SWITCH_TIME_KEY, nextTime, PreferencesManagerType::SATELLITE_CONTROL);
    satelliteManager_->Init();
    EXPECT_FALSE(satelliteManager_->IsSatelliteControlBtSwitch());
    EXPECT_FALSE(satelliteManager_->timer_->IsStarted());

    HILOGI("satellite_manager_test_004 end");
}

/*
 * @tc.number: satellite_manager_test_005
 * @tc.name: SatelliteManager_Init
 * @tc.desc:
 */
HWTEST_F(SatelliteManagerTest, satellite_manager_test_005, TestSize.Level1)
{
    HILOGI("satellite_manager_test_005 enter");

    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH, 1), BT_NO_ERROR);
    // Check timeout scene
    int64_t expiredTime = GetSecondsSince1970ToNow() - 300;  // More than 120 seconds
    PreferencesManager::Save(
        satelliteManager_->BT_SWITCH_TIME_KEY, expiredTime, PreferencesManagerType::SATELLITE_CONTROL);
    satelliteManager_->Init();
    EXPECT_FALSE(satelliteManager_->IsSatelliteControlBtSwitch());
    EXPECT_FALSE(satelliteManager_->timer_->IsStarted());

    HILOGI("satellite_manager_test_005 end");
}

/*
 * @tc.number: satellite_manager_test_006
 * @tc.name: SatelliteManager_Init
 * @tc.desc:
 */
HWTEST_F(SatelliteManagerTest, satellite_manager_test_006, TestSize.Level1)
{
    HILOGI("satellite_manager_test_006 enter");

    EXPECT_EQ(satelliteManager_->SatelliteControl(SATELLITE_CONTROL_MODE_BLUETOOTH_SWITCH, 1), BT_NO_ERROR);
    // Check not timeout scene
    int64_t expiredTime = GetSecondsSince1970ToNow() - 10;
    PreferencesManager::Save(
        satelliteManager_->BT_SWITCH_TIME_KEY, expiredTime, PreferencesManagerType::SATELLITE_CONTROL);
    satelliteManager_->Init();
    EXPECT_TRUE(satelliteManager_->IsSatelliteControlBtSwitch());
    EXPECT_TRUE(satelliteManager_->timer_->IsStarted());

    HILOGI("satellite_manager_test_006 end");
}
}  // namespace bluetooth
}  // namespace OHOS
