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
#include <gmock/gmock.h>
#include <cstring>
#include "power_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
RawAddress rawAddr = RawAddress("00:11:22:33:44:55");
std::shared_ptr<PowerDevice> powerDevice {nullptr};

class PowerDeviceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        powerDevice = std::make_shared<PowerDevice>(rawAddr);
        sleep(TIME);

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
        sleep(TIME);
        host_ = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number:power_device_test_001
 * @tc.name: AutoConnect
 * @tc.desc: NO_ACTION
 */
HWTEST_F(PowerDeviceTest, power_device_SetPowerMode_test_001, TestSize.Level1)
{
    HILOGI("power_device_test_001 enter");
    powerDevice->SetPowerMode();
    HILOGI("power_device_test_001 end");
}

/*
 * @tc.number:power_device_test_002
 * @tc.name: AutoConnect
 * @tc.desc: SetPowerMode_ActiveMode
 */
HWTEST_F(PowerDeviceTest, power_device_SetPowerMode_test_002, TestSize.Level1)
{
    HILOGI("power_device_test_002 enter");
    powerDevice->SetRequestPower(PROFILE_NAME_A2DP_SRC, RequestStatus::BUSY);
    powerDevice->SetPowerMode();
    powerDevice->DeleteRequestPower(PROFILE_NAME_A2DP_SRC);
    HILOGI("power_device_test_002 end");
}

/*
 * @tc.number:power_device_test_003
 * @tc.name: AutoConnect
 * @tc.desc: SetPowerMode_SniffMode
 */
HWTEST_F(PowerDeviceTest, power_device_SetPowerMode_test_003, TestSize.Level1)
{
    HILOGI("power_device_test_003 enter");
    powerDevice->SetRequestPower(PROFILE_NAME_A2DP_SRC, RequestStatus::CONNECT_ON);
    powerDevice->SetPowerMode();
    powerDevice->DeleteRequestPower(PROFILE_NAME_A2DP_SRC);
    HILOGI("power_device_test_003 enter");
}

/*
 * @tc.number:power_SetSniffMode_test_001
 * @tc.name: SetSniffMode
 * @tc.desc:
 */
HWTEST_F(PowerDeviceTest, power_SetSniffMode_test_001, TestSize.Level1)
{
    HILOGI("power_SetSniffMode_test_001 enter");
    PowerInfo requestPower = PowerInfo(PowerModeLevel::NO_ACTION, 0);
    powerDevice->SetSniffMode(requestPower);
    HILOGI("power_SetSniffMode_test_001 end");
}

/*
 * @tc.number:power_SetSniffMode_test_002
 * @tc.name: SetSniffMode
 * @tc.desc:
 */
HWTEST_F(PowerDeviceTest, power_SetSniffMode_test_002, TestSize.Level1)
{
    HILOGI("power_SetSniffMode_test_002 enter");
    powerDevice->UpdateControlSniffSubrating(PowerSsrLevel::SSR3);
    powerDevice->UpdatecontrolPowerLevel(PowerModeLevel::LEVEL_LOW);
    PowerInfo requestPower = PowerInfo(PowerModeLevel::NO_ACTION, 0);
    powerDevice->SetSniffMode(requestPower);
    HILOGI("power_SetSniffMode_test_002 end");
}

/*
 * @tc.number:power_device_GetPowerMode_test_001
 * @tc.name: GetPowerMode
 * @tc.desc: BTPowerMode::MODE_ACTIVE
 */
HWTEST_F(PowerDeviceTest, power_device_GetPowerMode_test_001, TestSize.Level1)
{
    HILOGI("power_device_GetPowerMode_test_001 enter");
    powerDevice->UpdatecontrolPowerLevel(PowerModeLevel::NO_ACTION);
    BTPowerMode ret = powerDevice->GetPowerMode();
    EXPECT_EQ(ret, BTPowerMode::MODE_ACTIVE);
    HILOGI("power_device_GetPowerMode_test_001 end");
}

/*
 * @tc.number:power_device_GetPowerMode_test_002
 * @tc.name: GetPowerMode
 * @tc.desc: BTPowerMode::MODE_ACTIVE
 */
HWTEST_F(PowerDeviceTest, power_device_GetPowerMode_test_002, TestSize.Level1)
{
    HILOGI("power_device_GetPowerMode_test_002 enter");
    powerDevice->UpdatecontrolPowerLevel(PowerModeLevel::LEVEL_ACTIVE);
    BTPowerMode ret = powerDevice->GetPowerMode();
    EXPECT_EQ(ret, BTPowerMode::MODE_ACTIVE);
    HILOGI("power_device_GetPowerMode_test_002 end");
}

/*
 * @tc.number:power_device_GetPowerMode_test_003
 * @tc.name: GetPowerMode
 * @tc.desc: BTPowerMode::MODE_SNIFF_LEVEL_HIG
 */
HWTEST_F(PowerDeviceTest, power_device_GetPowerMode_test_003, TestSize.Level1)
{
    HILOGI("power_device_GetPowerMode_test_003 enter");
    powerDevice->UpdatecontrolPowerLevel(PowerModeLevel::LEVEL_HIG);
    BTPowerMode ret = powerDevice->GetPowerMode();
    EXPECT_EQ(ret, BTPowerMode::MODE_SNIFF_LEVEL_HIG);
    HILOGI("power_device_GetPowerMode_test_003 end");
}

/*
 * @tc.number:power_device_GetPowerMode_test_004
 * @tc.name: GetPowerMode
 * @tc.desc: BTPowerMode::MODE_SNIFF_LEVEL_MID
 */
HWTEST_F(PowerDeviceTest, power_device_GetPowerMode_test_004, TestSize.Level1)
{
    HILOGI("power_device_GetPowerMode_test_004 enter");
    powerDevice->UpdatecontrolPowerLevel(PowerModeLevel::LEVEL_MID);
    BTPowerMode ret = powerDevice->GetPowerMode();
    EXPECT_EQ(ret, BTPowerMode::MODE_SNIFF_LEVEL_MID);
    HILOGI("power_device_GetPowerMode_test_004 end");
}

/*
 * @tc.number:power_device_GetPowerMode_test_005
 * @tc.name: GetPowerMode
 * @tc.desc: BTPowerMode::MODE_SNIFF_LEVEL_LOW
 */
HWTEST_F(PowerDeviceTest, power_device_GetPowerMode_test_005, TestSize.Level1)
{
    HILOGI("power_device_GetPowerMode_test_005 enter");
    powerDevice->UpdatecontrolPowerLevel(PowerModeLevel::LEVEL_LOW);
    BTPowerMode ret = powerDevice->GetPowerMode();
    EXPECT_EQ(ret, BTPowerMode::MODE_SNIFF_LEVEL_LOW);
    HILOGI("power_device_GetPowerMode_test_005 end");
}

/*
 * @tc.number:power_device_test_004
 * @tc.name: SniffSubratingCompleteCallback
 * @tc.desc:
 */
HWTEST_F(PowerDeviceTest, power_device_test_004, TestSize.Level1)
{
    HILOGI("power_device_test_004 enter");
    uint8_t status = 0;
    powerDevice->SniffSubratingCompleteCallback(status);
    sleep(1);
    status = 1;
    powerDevice->SniffSubratingCompleteCallback(status);
    HILOGI("power_device_test_004 end");
}

/*
 * @tc.number: PowerDeviceTest_UnitTest_CalcMaxPower_001
 * @tc.name: CalcMaxPower
 * @tc.desc: pimpl->requestPower_ is null by default.
*/
HWTEST_F(PowerDeviceTest, PowerDeviceTest_UnitTest_CalcMaxPower_001, TestSize.Level1)
{
    HILOGI("PowerDeviceTest_UnitTest_CalcMaxPower_001 start");
    powerDevice->SetRequestPower(PROFILE_NAME_A2DP_SRC, RequestStatus::SCO_ON);
    PowerInfo info = powerDevice->CalcMaxPower();
    EXPECT_TRUE(info.powerMode_ == PowerModeLevel::NO_ACTION);
    powerDevice->DeleteRequestPower(PROFILE_NAME_A2DP_SRC);
    HILOGI("PowerDeviceTest_UnitTest_CalcMaxPower_001 end");
}

/*
 * @tc.number: PowerDeviceTest_UnitTest_CalcMaxPower_002
 * @tc.name: CalcMaxPower
 * @tc.desc: pimpl->requestPower_ is set, itSpec.powerMode_ is greater than NO_ACTION
*/
HWTEST_F(PowerDeviceTest, PowerDeviceTest_UnitTest_CalcMaxPower_002, TestSize.Level1)
{
    HILOGI("PowerDeviceTest_UnitTest_CalcMaxPower_002 start");
    powerDevice->SetRequestPower(PROFILE_NAME_HFP_AG, RequestStatus::CONNECT_ON);

    PowerInfo info = powerDevice->CalcMaxPower();
    EXPECT_TRUE(info.powerMode_ != PowerModeLevel::NO_ACTION);
    powerDevice->DeleteRequestPower(PROFILE_NAME_HFP_AG);
    HILOGI("PowerDeviceTest_UnitTest_CalcMaxPower_002 end");
}

/*
 * @tc.number: PowerDeviceTest_UnitTest_CalcMaxPower_003
 * @tc.name: CalcMaxPower
 * @tc.desc: pimpl->requestPower_ is set, itSpec.powerMode_ is greater than NO_ACTION
*/
HWTEST_F(PowerDeviceTest, PowerDeviceTest_UnitTest_CalcMaxPower_003, TestSize.Level1)
{
    HILOGI("PowerDeviceTest_UnitTest_CalcMaxPower_003 start");
    //delete old settings
    powerDevice->DeleteRequestPower(PROFILE_NAME_HFP_AG);
    //set new settings
    powerDevice->SetRequestPower(PROFILE_NAME_A2DP_SRC, RequestStatus::SCO_ON);

    PowerInfo info = powerDevice->CalcMaxPower();
    HILOGI("info.powerMode_ is:%{public}d", info.powerMode_);
    EXPECT_TRUE(info.powerMode_ == PowerModeLevel::NO_ACTION);
    powerDevice->DeleteRequestPower(PROFILE_NAME_A2DP_SRC);
    HILOGI("PowerDeviceTest_UnitTest_CalcMaxPower_003 end");
}

/*
 * @tc.number:power_device_CalcLowestSsrLevel_test_001
 * @tc.name: CalcLowestSsrLevel
 * @tc.desc: PowerSsrLevel::NO_ACTION
 */
HWTEST_F(PowerDeviceTest, power_device_CalcLowestSsrLevel_test_001, TestSize.Level1)
{
    HILOGI("power_device_CalcLowestSsrLevel_test_001 enter");
    PowerSsrLevel ret = powerDevice->CalcLowestSsrLevel();
    EXPECT_EQ(ret, PowerSsrLevel::NO_ACTION);
    HILOGI("power_device_CalcLowestSsrLevel_test_001 end");
}

/*
 * @tc.number:power_device_CalcLowestSsrLevel_test_002
 * @tc.name: CalcLowestSsrLevel
 * @tc.desc: PowerSsrLevel::NO_ACTION
 */
HWTEST_F(PowerDeviceTest, power_device_CalcLowestSsrLevel_test_002, TestSize.Level1)
{
    HILOGI("power_device_CalcLowestSsrLevel_test_002 enter");
    powerDevice->SetRequestPower(PROFILE_NAME_A2DP_SRC, RequestStatus::SCO_OFF);
    PowerSsrLevel ret = powerDevice->CalcLowestSsrLevel();
    EXPECT_EQ(ret, PowerSsrLevel::SSR2);
    powerDevice->DeleteRequestPower(PROFILE_NAME_A2DP_SRC);
    HILOGI("power_device_CalcLowestSsrLevel_test_002 end");
}

/*
 * @tc.number:power_device_CalcLowestSsrLevel_test_003
 * @tc.name: CalcLowestSsrLevel
 * @tc.desc: PowerSsrLevel::NO_ACTION
 */
HWTEST_F(PowerDeviceTest, power_device_CalcLowestSsrLevel_test_003, TestSize.Level1)
{
    HILOGI("power_device_CalcLowestSsrLevel_test_003 enter");
    powerDevice->SetRequestPower("test", RequestStatus::IDLE);
    PowerSsrLevel ret = powerDevice->CalcLowestSsrLevel();
    EXPECT_EQ(ret, PowerSsrLevel::NO_ACTION);
    HILOGI("power_device_CalcLowestSsrLevel_test_003 end");
}
}  // namespace bluetooth
}  // namespace OHOS