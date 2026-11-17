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

#include <gtest/gtest.h>
#include <cstring>
#include "preferences_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
std::string addr = "00:11:22:33:44:55";

class PreferencesManagerTest : public testing::Test {
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
 * @tc.number: PreferencesManagerTest_Save_001
 * @tc.name: Save
 * @tc.desc:
*/
HWTEST_F(PreferencesManagerTest, PreferencesManagerTest_Save_001, TestSize.Level1)
{
    HILOGI("PreferencesManagerTest_Save_001 start");
    PreferencesManager::Save(addr, true, PreferencesManagerType::ABS_VOLUME_SWITCH);
    EXPECT_TRUE(PreferencesManager::Get(addr, false, PreferencesManagerType::ABS_VOLUME_SWITCH));
    HILOGI("PreferencesManagerTest_Save_001 end");
}

/*
 * @tc.number: PreferencesManagerTest_Get_001
 * @tc.name: Get
 * @tc.desc:
*/
HWTEST_F(PreferencesManagerTest, PreferencesManagerTest_Get_001, TestSize.Level1)
{
    HILOGI("PreferencesManagerTest_Get_001 start");
    PreferencesManager::Save(addr, 12, PreferencesManagerType::ABS_VOLUME);
    EXPECT_TRUE(PreferencesManager::Get(addr, 0, PreferencesManagerType::ABS_VOLUME) == 12);
    HILOGI("PreferencesManagerTest_Get_001 end");
}

/*
 * @tc.number: PreferencesManagerTest_Delete_001
 * @tc.name: Delete
 * @tc.desc:
*/
HWTEST_F(PreferencesManagerTest, PreferencesManagerTest_Delete_001, TestSize.Level1)
{
    HILOGI("PreferencesManagerTest_Delete_001 start");
    PreferencesManager::Delete(addr, PreferencesManagerType::ABS_VOLUME_SWITCH);
    EXPECT_FALSE(PreferencesManager::Get(addr, false, PreferencesManagerType::ABS_VOLUME_SWITCH));
    HILOGI("PreferencesManagerTest_Delete_001 end");
}

/*
 * @tc.number: PreferencesManagerTest_SaveString_001
 * @tc.name: SaveString
 * @tc.desc:
*/
HWTEST_F(PreferencesManagerTest, PreferencesManagerTest_SaveString_001, TestSize.Level1)
{
    HILOGI("PreferencesManagerTest_SaveString_001 start");
    PreferencesManager::SaveString(addr, "test", PreferencesManagerType::CAR_KEY_DFX_DATA);
    EXPECT_FALSE(PreferencesManager::GetAll(PreferencesManagerType::CAR_KEY_DFX_DATA).empty());
    HILOGI("PreferencesManagerTest_SaveString_001 end");
}

/*
 * @tc.number: PreferencesManagerTest_GetString_001
 * @tc.name: GetString
 * @tc.desc:
*/
HWTEST_F(PreferencesManagerTest, PreferencesManagerTest_GetString_001, TestSize.Level1)
{
    HILOGI("PreferencesManagerTest_GetString_001 start");
    std::string device = "AA:11:22:33:44:55";
    PreferencesManager::SaveString(device, "true", PreferencesManagerType::CONNECT_ALL_PROFILE_APP_TYPE);
    std::string res = PreferencesManager::GetString(device, "false", PreferencesManagerType::CONNECT_ALL_PROFILE_APP_TYPE);
    EXPECT_TRUE(res == "true");
    HILOGI("PreferencesManagerTest_GetString_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS