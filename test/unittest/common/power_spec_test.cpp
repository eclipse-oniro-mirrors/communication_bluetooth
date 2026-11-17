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
#include "service_util.h"
#include "power_device.h"
#include "power_spec.h"
#include "class_creator.h"
#include "adapter_manager.h"
#include "bluetooth_host.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
PowerSpec powerSpec;

class PowerSpecTest : public testing::Test {
public:
    static void SetUpTestCase()
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
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number:PowerSpecTest_UnitTest_GetPowerSpec_001
 * @tc.name: GetPowerSpec
 * @tc.desc: profileName is not found, return PowerInfo object with powerMode_ is NO_ACTION.
 */
HWTEST_F(PowerSpecTest, PowerSpecTest_UnitTest_GetPowerSpec_001, TestSize.Level1)
{
    HILOGI("PowerSpecTest_UnitTest_GetPowerSpec_001 enter");
    std::string profileName = "power_spec_test_001";
    RequestStatus status{ RequestStatus::CONNECT_ON };
    PowerInfo power = powerSpec.GetPowerSpec(profileName, status);
    EXPECT_TRUE(power.powerMode_ == PowerModeLevel::NO_ACTION);
    HILOGI("PowerSpecTest_UnitTest_GetPowerSpec_001 end");
}

/*
 * @tc.number:PowerSpecTest_UnitTest_GetPowerSpec_002
 * @tc.name: GetPowerSpec
 * @tc.desc: profileName is found, return PowerInfo object with powerMode_ is LEVEL_LOW.
 */
HWTEST_F(PowerSpecTest, PowerSpecTest_UnitTest_GetPowerSpec_002, TestSize.Level1)
{
    HILOGI("PowerSpecTest_UnitTest_GetPowerSpec_002 enter");
    RequestStatus status{ RequestStatus::CONNECT_ON };
    PowerInfo power = powerSpec.GetPowerSpec(PROFILE_NAME_HFP_AG, status);
    EXPECT_TRUE(power.powerMode_ == PowerModeLevel::LEVEL_LOW);
    HILOGI("PowerSpecTest_UnitTest_GetPowerSpec_002 end");
}

/*
 * @tc.number:PowerSpecTest_UnitTest_GetPowerParam_001
 * @tc.name: GetPowerParam
 * @tc.desc: level is found in POWER_PARAM, return maxInterval_ is SNIFF_LEVEL_MID_MAX_INTERVAL.
 */
HWTEST_F(PowerSpecTest, PowerSpecTest_UnitTest_GetPowerParam_001, TestSize.Level1)
{
    HILOGI("PowerSpecTest_UnitTest_GetPowerParam_001 enter");
    PowerModeLevel level = PowerModeLevel::LEVEL_MID;
    PowerParam powerParam = powerSpec.GetPowerParam(level);
    EXPECT_TRUE(powerParam.maxInterval_ == SNIFF_LEVEL_MID_MAX_INTERVAL);
    HILOGI("PowerSpecTest_UnitTest_GetPowerParam_001 end");
}

/*
 * @tc.number:PowerSpecTest_UnitTest_GetPowerParam_002
 * @tc.name: GetPowerParam
 * @tc.desc: level is not found in POWER_PARAM, return maxInterval_ is 0.
 */
HWTEST_F(PowerSpecTest, PowerSpecTest_UnitTest_GetPowerParam_002, TestSize.Level1)
{
    HILOGI("PowerSpecTest_UnitTest_GetPowerParam_002 enter");
    PowerModeLevel level = PowerModeLevel::NO_ACTION;
    PowerParam powerParam = powerSpec.GetPowerParam(level);
    EXPECT_TRUE(powerParam.maxInterval_ == 0);
    HILOGI("PowerSpecTest_UnitTest_GetPowerParam_002 end");
}

/*
 * @tc.number:PowerSpecTest_UnitTest_GetPowerSsrLevel_001
 * @tc.name: GetPowerSsrLevel
 * @tc.desc: profileName is not found, return PowerSsrLevel::NO_ACTION
 */
HWTEST_F(PowerSpecTest, PowerSpecTest_UnitTest_GetPowerSsrLevel_001, TestSize.Level1)
{
    HILOGI("PowerSpecTest_UnitTest_GetPowerSsrLevel_001 enter");
    std::string profileName = "power_spec_test_001";
    RequestStatus status{ RequestStatus::CONNECT_ON };
    PowerSsrLevel level = powerSpec.GetPowerSsrLevel(profileName, status);
    EXPECT_TRUE(level == PowerSsrLevel::NO_ACTION);
    HILOGI("PowerSpecTest_UnitTest_GetPowerSsrLevel_001 end");
}

/*
 * @tc.number:PowerSpecTest_UnitTest_GetPowerSsrLevel_002
 * @tc.name: GetPowerSsrLevel
 * @tc.desc: profileName is found, return PowerSsrLevel::NO_ACTION
 */
HWTEST_F(PowerSpecTest, PowerSpecTest_UnitTest_GetPowerSsrLevel_002, TestSize.Level1)
{
    HILOGI("PowerSpecTest_UnitTest_GetPowerSsrLevel_002 enter");
    RequestStatus status{ RequestStatus::CONNECT_ON };
    PowerSsrLevel level = powerSpec.GetPowerSsrLevel(PROFILE_NAME_HFP_AG, status);
    EXPECT_TRUE(level == PowerSsrLevel::SSR2);
    HILOGI("PowerSpecTest_UnitTest_GetPowerSsrLevel_002 end");
}

/*
 * @tc.number:PowerSpecTest_UnitTest_GetPowerSsrParam_001
 * @tc.name: GetPowerSsrParam
 * @tc.desc: level is found.
 */
HWTEST_F(PowerSpecTest, PowerSpecTest_UnitTest_GetPowerSsrParam_001, TestSize.Level1)
{
    HILOGI("PowerSpecTest_UnitTest_GetPowerSsrParam_001 enter");
    PowerSsrLevel level = PowerSsrLevel::SSR1;
    PowerSsrParam param = powerSpec.GetPowerSsrParam(level);
    EXPECT_TRUE(param.maxLat_ == SSR1_MAX_LATENCY);
    HILOGI("PowerSpecTest_UnitTest_GetPowerSsrParam_001 end");
}

/*
 * @tc.number:PowerSpecTest_UnitTest_GetPowerSsrParam_002
 * @tc.name: GetPowerSsrParam
 * @tc.desc: level is not found.
 */
HWTEST_F(PowerSpecTest, PowerSpecTest_UnitTest_GetPowerSsrParam_002, TestSize.Level1)
{
    HILOGI("PowerSpecTest_UnitTest_GetPowerSsrParam_002 enter");
    PowerSsrLevel level = PowerSsrLevel::NO_ACTION;
    PowerSsrParam param = powerSpec.GetPowerSsrParam(level);
    EXPECT_TRUE(param.maxLat_ == 0);
    HILOGI("PowerSpecTest_UnitTest_GetPowerSsrParam_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS