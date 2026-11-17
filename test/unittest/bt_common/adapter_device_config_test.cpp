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
#include "adapter_device_config.h"
#include "bluetooth_host.h"
#include "log.h"
#include "gmock/gmock.h"

using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;
using ::testing::Matcher;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;

class AdapterDeviceConfigTest : public testing::Test {
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
private:
    std::string addr_ = "00:11:22:33:44:55";
};

/**
 * @tc.name: Adapter_Device_Config_SetPbapShareType_test_0100
 * @tc.desc: AdapterDeviceConfigTest:SetPbapShareType
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_SetPbapShareType_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_SetPbapShareType_test_0100 enter");
    int32_t shareType = 0;
    SetPbapShareType(addr_, shareType);
    EXPECT_NE(addr_, "00");
    HILOGI("Adapter_Device_Config_SetPbapShareType_test_0100 end");
}

/**
 * @tc.name: Adapter_Device_Config_GetPbapShareType_test_0100
 * @tc.desc: AdapterDeviceConfigTest:GetPbapShareType
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_GetPbapShareType_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_GetPbapShareType_test_0100 enter");
    int32_t shareType = 0;
    EXPECT_FALSE(GetPbapShareType(addr_, shareType));
    shareType = -1;
    EXPECT_FALSE(GetPbapShareType(addr_, shareType));
    HILOGI("Adapter_Device_Config_GetPbapShareType_test_0100 end");
}

/**
 * @tc.name: Adapter_Device_Config_SetPbapPermission_test_0100
 * @tc.desc: AdapterDeviceConfigTest:SetPbapPermission
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_SetPbapPermission_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_SetPbapPermission_test_0100 enter");
    int32_t permission = 0;
    SetPbapPermission(addr_, permission);
    EXPECT_NE(addr_, "00");
    HILOGI("Adapter_Device_Config_SetPbapPermission_test_0100 end");
}

/**
 * @tc.name: Adapter_Device_Config_GetPbapPermission_test_0100
 * @tc.desc: AdapterDeviceConfigTest:GetPbapPermission
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_GetPbapPermission_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_GetPbapPermission_test_0100 enter");
    int32_t permission = 0;
    EXPECT_FALSE(GetPbapPermission(addr_, permission));
    HILOGI("Adapter_Device_Config_GetPbapPermission_test_0100 end");
}

/**
 * @tc.name: Adapter_Device_Config_SetMessagePermission_test_0100
 * @tc.desc: AdapterDeviceConfigTest:SetMessagePermission
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_SetMessagePermission_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_SetMessagePermission_test_0100 enter");
    int32_t permission = 0;
    SetMessagePermission(addr_, permission);
    EXPECT_NE(addr_, "00");
    HILOGI("Adapter_Device_Config_SetMessagePermission_test_0100 end");
}

/**
 * @tc.name: classic_config_test_057
 * @tc.desc: AdapterDeviceConfigTest:GetMessagePermission
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_GetMessagePermission_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_GetMessagePermission_test_0100 enter");
    int32_t permission = 0;
    EXPECT_FALSE(GetMessagePermission(addr_, permission));
    HILOGI("Adapter_Device_Config_GetMessagePermission_test_0100 end");
}

/**
 * @tc.name: classic_config_test_058
 * @tc.desc: AdapterDeviceConfigTest:SaveAutoPlayValue
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_SaveAutoPlayValue_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_SaveAutoPlayValue_test_0100 enter");
    int32_t autoPlayValue = 1;
    EXPECT_FALSE(SaveAutoPlayValue(addr_, autoPlayValue));
    HILOGI("Adapter_Device_Config_SaveAutoPlayValue_test_0100 end");
}

/**
 * @tc.name: classic_config_test_059
 * @tc.desc: AdapterDeviceConfigTest:GetAutoPlayValue
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_GetAutoPlayValue_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_GetAutoPlayValue_test_0100 enter");
    int32_t autoPlayValue = 1;
    EXPECT_FALSE(GetAutoPlayValue(addr_, autoPlayValue));
    HILOGI("Adapter_Device_Config_GetAutoPlayValue_test_0100 end");
}

/**
 * @tc.name: classic_config_test_060
 * @tc.desc: AdapterDeviceConfigTest:SetRestrictedDuration
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_SetRestrictedDuration_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_SetRestrictedDuration_test_0100 enter");
    int32_t duration = 3000;
    EXPECT_FALSE(SetRestrictedDuration(addr_, duration));
    HILOGI("Adapter_Device_Config_SetRestrictedDuration_test_0100 end");
}

/**
 * @tc.name: classic_config_test_061
 * @tc.desc: AdapterDeviceConfigTest:GetRestrictedDuration
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_GetRestrictedDuration_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_GetRestrictedDuration_test_0100 enter");
    int32_t duration = 3000;
    EXPECT_FALSE(GetRestrictedDuration(addr_, duration));
    HILOGI("Adapter_Device_Config_GetRestrictedDuration_test_0100 end");
}

/**
 * @tc.name: Adapter_Device_Config_SaveLearnedAutoPlayDuration_test_0100
 * @tc.desc: AdapterDeviceConfigTest:SaveLearnedAutoPlayDuration
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_SaveLearnedAutoPlayDuration_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_SaveLearnedAutoPlayDuration_test_0100 enter");
    int32_t duration = 3000;
    EXPECT_FALSE(SaveLearnedAutoPlayDuration(addr_, duration));
    HILOGI("Adapter_Device_Config_SaveLearnedAutoPlayDuration_test_0100 end");
}

/**
 * @tc.name: Adapter_Device_Config_GetLearnedAutoPlayDuration_test_0100
 * @tc.desc: AdapterDeviceConfigTest:GetLearnedAutoPlayDuration
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_GetLearnedAutoPlayDuration_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_GetLearnedAutoPlayDuration_test_0100 enter");
    int32_t duration = 0;
    EXPECT_FALSE(GetLearnedAutoPlayDuration(addr_, duration));
    HILOGI("Adapter_Device_Config_GetLearnedAutoPlayDuration_test_0100 end");
}

/**
 * @tc.name: classic_config_test_061
 * @tc.desc: AdapterDeviceConfigTest:SetHfpAgVgsSupport
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_SetHfpAgVgsSupport_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_SetHfpAgVgsSupport_test_0100 enter");
    bool vgsSupport = false;
    EXPECT_FALSE(SetHfpAgVgsSupport(addr_, vgsSupport));
    HILOGI("Adapter_Device_Config_SetHfpAgVgsSupport_test_0100 end");
}

/**
 * @tc.name: classic_config_test_061
 * @tc.desc: AdapterDeviceConfigTest:HasPbapOrMapPermission
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_HasPbapOrMapPermission_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_HasPbapOrMapPermission_test_0100 enter");
    EXPECT_FALSE(HasPbapOrMapPermission());
    HILOGI("Adapter_Device_Config_HasPbapOrMapPermission_test_0100 end");
}

/**
 * @tc.name: classic_config_test_061
 * @tc.desc: AdapterDeviceConfigTest:GetHfpAgVgsSupport
 * @tc.type: FUNC
 */
HWTEST_F(AdapterDeviceConfigTest, Adapter_Device_Config_GetHfpAgVgsSupport_test_0100, TestSize.Level1)
{
    HILOGI("Adapter_Device_Config_GetHfpAgVgsSupport_test_0100 enter");
    bool vgsSupport = false;
    EXPECT_FALSE(GetHfpAgVgsSupport(addr_, vgsSupport));
    HILOGI("Adapter_Device_Config_GetHfpAgVgsSupport_test_0100 end");
}
}  // namespace bluetooth
}  // namespace OHOS