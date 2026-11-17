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
#include "profile_config.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
ProfileConfig profileConfig;

class ProfileConfigTest : public testing::Test {
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
 * @tc.number:profile_config_test_001
 * @tc.name: Load
 * @tc.desc:
 */
HWTEST_F(ProfileConfigTest, profile_config_test_001, TestSize.Level1)
{
    HILOGI("profile_config_test_001 enter");
    EXPECT_FALSE(profileConfig.Load());
    HILOGI("profile_config_test_001 end");
}

/*
 * @tc.number:profile_config_test_002
 * @tc.name: Reload
 * @tc.desc:
 */
HWTEST_F(ProfileConfigTest, profile_config_test_002, TestSize.Level1)
{
    HILOGI("profile_config_test_002 enter");
    EXPECT_FALSE(profileConfig.Reload());
    HILOGI("profile_config_test_002 end");
}

/*
 * @tc.number:profile_config_test_003
 * @tc.name: SetValue
 * @tc.desc:
 */
HWTEST_F(ProfileConfigTest, profile_config_test_003, TestSize.Level1)
{
    HILOGI("profile_config_test_003 enter");
    std::string addr = "00:11:22:33:44:55";
    std::string section = "profile_config_test_003";
    std::string property = "profile_config_test_003";
    int value = 1;
    EXPECT_FALSE(profileConfig.SetValue(addr, section, property, value));
    HILOGI("profile_config_test_003 end");
}

/*
 * @tc.number:profile_config_test_004
 * @tc.name: SetValue
 * @tc.desc:
 */
HWTEST_F(ProfileConfigTest, profile_config_test_004, TestSize.Level1)
{
    HILOGI("profile_config_test_004 enter");
    std::string addr = "00:11:22:33:44:55";
    std::string section = "profile_config_test_003";
    std::string property = "profile_config_test_003";
    bool value = true;
    EXPECT_FALSE(profileConfig.SetValue(addr, section, property, value));
    HILOGI("profile_config_test_004 end");
}

/*
 * @tc.number:profile_config_test_005
 * @tc.name: SetValue
 * @tc.desc:
 */
HWTEST_F(ProfileConfigTest, profile_config_test_005, TestSize.Level1)
{
    HILOGI("profile_config_test_005 enter");
    std::string section = "profile_config_test_005";
    std::string property = "profile_config_test_005";
    std::string value = "profile_config_test_005";
    EXPECT_FALSE(profileConfig.SetValue(section, property, value));
    HILOGI("profile_config_test_005 end");
}

/*
 * @tc.number:profile_config_test_006
 * @tc.name: RemoveAddr
 * @tc.desc:
 */
HWTEST_F(ProfileConfigTest, profile_config_test_006, TestSize.Level1)
{
    HILOGI("profile_config_test_006 enter");
    std::string addr = "00:11:22:33:44:55";
    EXPECT_FALSE(profileConfig.RemoveAddr(addr));
    HILOGI("profile_config_test_006 end");
}
}  // namespace bluetooth
}  // namespace OHOS