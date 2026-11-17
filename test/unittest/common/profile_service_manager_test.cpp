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
#include "profile_service_manager.h"
#include "bluetooth_access_token_mock.h"
#include "adapter_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
ProfileServiceManager profileServiceManager;

class ProfileServiceManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
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
 * @tc.number: profile_service_manager_test_001
 * @tc.name: Start
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_001, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_001 enter");
    // libsamgr_proxy.z.so cficheck 导致crash 暂时屏蔽AdapterManager adapterManager;
    // libsamgr_proxy.z.so cficheck 导致crash 暂时屏蔽adapterManager.Start();
    sleep(TIME);
    profileServiceManager.Start();
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_001 end");
}

/*
 * @tc.number: profile_service_manager_test_002
 * @tc.name: CreateClassicProfileServices
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_002, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_002 enter");
    profileServiceManager.CreateClassicProfileServices();
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    EXPECT_NE(profileServiceManager.GetProfileService("HfpHfService"), nullptr);
    HILOGI("profile_service_manager_test_002 end");
}

/*
 * @tc.number: profile_service_manager_test_004
 * @tc.name: CreateBleProfileServices
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_004, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_004 enter");
    profileServiceManager.CreateBleProfileServices();
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_004 end");
}

/*
 * @tc.number: profile_service_manager_test_005
 * @tc.name: Stop
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_005, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_005 enter");
    profileServiceManager.Stop();
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_005 end");
}

/*
 * @tc.number: profile_service_manager_test_006
 * @tc.name: GetProfileService
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_006, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_006 enter");
    std::string name = "profile_service_manager_test_006 test";
    profileServiceManager.GetProfileService(name);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_006 end");
}

/*
 * @tc.number: profile_service_manager_test_007
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_007, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_007 enter");
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_TRUE(profileServiceManager.Enable(transport));
    transport = ADAPTER_BLE;
    EXPECT_TRUE(profileServiceManager.Enable(transport));
    HILOGI("profile_service_manager_test_007 end");
}

/*
 * @tc.number: profile_service_manager_test_008
 * @tc.name: OnAllEnabled
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_008, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_008 enter");
    BTTransport transport = ADAPTER_BREDR;
    profileServiceManager.OnAllEnabled(transport);
    transport = ADAPTER_BLE;
    profileServiceManager.OnAllEnabled(transport);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_008 end");
}

/*
 * @tc.number: profile_service_manager_test_010
 * @tc.name: IsAllEnabled
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_010, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_010 enter");
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_TRUE(profileServiceManager.IsAllEnabled(transport));
    transport = ADAPTER_BLE;
    EXPECT_TRUE(profileServiceManager.IsAllEnabled(transport));
    HILOGI("profile_service_manager_test_010 end");
}

/*
 * @tc.number: profile_service_manager_test_012
 * @tc.name: OnEnable
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_012, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_012 enter");
    std::string name = "profile_service_manager_test_012";
    bool ret = true;
    profileServiceManager.OnEnable(name, ret);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_012 end");
}

/*
 * @tc.number: profile_service_manager_test_013
 * @tc.name: OnEnable
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_013, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_013 enter");
    std::string name = "profile_service_manager_test_013";
    bool ret = true;
    profileServiceManager.OnEnable(name, ret);
    profileServiceManager.Initialize();
    profileServiceManager.OnEnable(name, ret);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_013 end");
}

/*
 * @tc.number: profile_service_manager_test_014
 * @tc.name: EnableCompleteProcess
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_014, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_014 enter");
    std::string name = "profile_service_manager_test_014";
    bool ret = true;
    profileServiceManager.EnableCompleteProcess(name, ret);
    ret = false;
    profileServiceManager.EnableCompleteProcess(name, ret);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_014 end");
}

/*
 * @tc.number: profile_service_manager_test_015
 * @tc.name: EnableCompleteProcess
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_015, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_015 enter");
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_FALSE(profileServiceManager.IsProfilesTurning(transport));
    HILOGI("profile_service_manager_test_015 end");
}

/*
 * @tc.number: profile_service_manager_test_016
 * @tc.name: EnableCompleteNotify
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_016, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_016 enter");
    BTTransport transport = ADAPTER_BREDR;
    profileServiceManager.EnableCompleteNotify(transport);
    transport = ADAPTER_BLE;
    profileServiceManager.EnableCompleteNotify(transport);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_016 end");
}

/*
 * @tc.number: profile_service_manager_test_017
 * @tc.name: Disable
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_017, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_017 enter");
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_TRUE(profileServiceManager.Disable(transport));
    transport = ADAPTER_BLE;
    EXPECT_TRUE(profileServiceManager.Disable(transport));
    HILOGI("profile_service_manager_test_017 end");
}

/*
 * @tc.number: profile_service_manager_test_018
 * @tc.name: OnAllDisabled
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_018, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_018 enter");
    BTTransport transport = ADAPTER_BREDR;
    profileServiceManager.OnAllDisabled(transport);
    transport = ADAPTER_BLE;
    profileServiceManager.OnAllDisabled(transport);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_018 end");
}

/*
 * @tc.number: profile_service_manager_test_019
 * @tc.name: IsAllDisabled
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_019, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_019 enter");
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_TRUE(profileServiceManager.IsAllDisabled(transport));
    transport = ADAPTER_BLE;
    EXPECT_TRUE(profileServiceManager.IsAllDisabled(transport));
    HILOGI("profile_service_manager_test_019 end");
}

/*
 * @tc.number: profile_service_manager_test_021
 * @tc.name: OnDisable
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_021, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_021 enter");
    std::string name = "profile_service_manager_test_021";
    bool ret = true;
    profileServiceManager.OnDisable(name, ret);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_021 end");
}

/*
 * @tc.number: profile_service_manager_test_022
 * @tc.name: DisableCompleteProcess
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_022, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_022 enter");
    std::string name = "profile_service_manager_test_022";
    bool ret = true;
    profileServiceManager.DisableCompleteProcess(name, ret);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_022 end");
}

/*
 * @tc.number: profile_service_manager_test_023
 * @tc.name: DisableCompleteNotify
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_023, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_023 enter");
    BTTransport transport = ADAPTER_BREDR;
    profileServiceManager.DisableCompleteNotify(transport);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_023 end");
}

/*
 * @tc.number: profile_service_manager_test_024
 * @tc.name: CheckWaitEnableProfiles
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_024, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_024 enter");
    std::string name = "profile_service_manager_test_024";
    BTTransport transport = ADAPTER_BREDR;
    profileServiceManager.CheckWaitEnableProfiles(name, transport);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_024 end");
}

/*
 * @tc.number: profile_service_manager_test_025
 * @tc.name: GetProfileServicesSupportedUuids
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_025, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_025 enter");
    std::vector<std::string> v;
    v.push_back("00000000-0000-1000-8000-00805F9B34FB");
    profileServiceManager.GetProfileServicesSupportedUuids(v);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_025 end");
}

/*
 * @tc.number: profile_service_manager_test_026
 * @tc.name: GetProfileServicesList
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_026, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_026 enter");
    profileServiceManager.GetProfileServicesList();
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_026 end");
}

/*
 * @tc.number: profile_service_manager_test_027
 * @tc.name: GetProfileServiceConnectState
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_027, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_027 enter");
    uint32_t profileID = 147258;
    EXPECT_EQ(profileServiceManager.GetProfileServiceConnectState(profileID), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_027 end");
}

/*
 * @tc.number: profile_service_manager_test_028
 * @tc.name: GetProfileServiceConnectState
 * @tc.desc:
 */
HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_028, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_028 enter");
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_028 end");
}


HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_029, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_029 enter");
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_TRUE(profileServiceManager.IsAllEnabled(transport));
    transport = ADAPTER_BLE;
    EXPECT_TRUE(profileServiceManager.IsAllEnabled(transport));
    HILOGI("profile_service_manager_test_029 end");
}

HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_030, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_030 enter");
    std::string name = PROFILE_NAME_AVRCP_TG;
    EXPECT_TRUE(profileServiceManager.NeedWaitOtherProfileComplete(name));
    name = PROFILE_NAME_PBAP_PSE;
    EXPECT_TRUE(profileServiceManager.NeedWaitOtherProfileComplete(name));
    name = PROFILE_NAME_MAP_MSE;
    EXPECT_TRUE(profileServiceManager.NeedWaitOtherProfileComplete(name));
    name = "false";
    EXPECT_FALSE(profileServiceManager.NeedWaitOtherProfileComplete(name));
    HILOGI("profile_service_manager_test_030 end");
}

HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_032, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_032 enter");
    std::string name = PROFILE_NAME_AVRCP_TG;
    bool ret = false;
    profileServiceManager.OnEnable(name, ret);
    ret = true;
    profileServiceManager.OnEnable(name, ret);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_032 end");
}

HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_033, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_033 enter");
    std::string name = PROFILE_NAME_A2DP_SRC;
    profileServiceManager.CheckEnableOtherProfile(name);
    name = PROFILE_NAME_SPP;
    profileServiceManager.CheckEnableOtherProfile(name);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_033 end");
}

HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_034, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_034 enter");
    std::string name = PROFILE_NAME_AVRCP_TG;
    bool ret = false;
    profileServiceManager.EnableCompleteProcess(name, ret);
    ret = true;
    profileServiceManager.EnableCompleteProcess(name, ret);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_034 end");
}

HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_035, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_035 enter");
    BTTransport transport = ADAPTER_BREDR;
    profileServiceManager.IsProfilesTurning(transport);
    transport = ADAPTER_BLE;
    profileServiceManager.IsProfilesTurning(transport);
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("profile_service_manager_test_035 end");
}

HWTEST_F(ProfileServiceManagerTest, profile_service_manager_test_036, TestSize.Level1)
{
    HILOGI("profile_service_manager_test_036 enter");
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_EQ(profileServiceManager.GetProfileServicesConnectState(), BTConnectState::DISCONNECTED);
    std::string name = PROFILE_NAME_A2DP_SRC;
    profileServiceManager.SetProfileService(transport, name);
    transport = ADAPTER_BLE;
    profileServiceManager.SetProfileService(transport, name);
    HILOGI("profile_service_manager_test_036 end");
}
}  // namespace bluetooth
}  // namespace OHOS