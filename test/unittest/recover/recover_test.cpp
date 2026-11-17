/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "a2dp_hfp_recover.h"
#include "bluetooth_host.h"
#include "bluetooth_device.h"
#include "remote_device_properties.h"
#include "log.h"
#include "bluetooth_access_token_mock.h"
#include "class_creator.h"
#include "profile_service_manager.h"
#include "hfp_ag_service.h"
#include "a2dp_src_service.h"
#include "thread_util.h"

using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

void *A2dpHfpRecoverCreate(void)
{
    return new A2dpHfpRecover();
}

void *HfpAgServiceCreate(void)
{
    return new HfpAgService();
}

void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

constexpr int TIME = 1;
static BluetoothHost *host_ = nullptr;

class RecoverTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        BluetoothAccessTokenMock::SetNativeTokenInfo();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        ClassFactory::RegisterClass("A2dpHfpRecover", A2dpHfpRecoverCreate);
        ClassFactory::RegisterClass("HfpAgService", HfpAgServiceCreate);
        ClassFactory::RegisterClass("A2dpSrcService", A2dpSrcServiceCreate);
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase(void)
    {
        utility::CloseAllTimer();
        host_ = nullptr;
    }
    void SetUp()
    {
        auto manager = ProfileServiceManager::GetInstance();
        manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_HFP_AG);
        manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_A2DP_SRC);
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_A2DP, ThreadUtil::DISABLED);
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_HFP, ThreadUtil::DISABLED);
    }
    void TearDown()
    {
        auto manager = ProfileServiceManager::GetInstance();
        A2dpSrcService *a2dpSrcService = static_cast<A2dpSrcService *>(manager->GetProfileService("A2dpSrcService"));
        delete a2dpSrcService;
        HfpAgService *hfpAgService = static_cast<HfpAgService *>(manager->GetProfileService("HfpAgService"));
        delete hfpAgService;
    }
};

/**
 * @tc.number:recover_UnitTest001
 * @tc.name: SetTimerForRecoverA2dpService
 * @tc.desc:
 */
HWTEST_F(RecoverTest, recover_UnitTest_SetTimerForRecoverA2dpService, TestSize.Level1)
{
    HILOGI("A2dpHfpRecover::SetTimerForRecoverA2dpService starts");
    std::string device = "12:34:56:78:9A:BC";
    A2dpHfpRecover::GetInstance().SetTimerForRecoverA2dpService(device);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverA2dpService_, nullptr);
    HILOGI("A2dpHfpRecover::SetTimerForRecoverA2dpService ends");
}

/**
 * @tc.number:recover_UnitTest002
 * @tc.name: SetTimerForRecoverHfpService
 * @tc.desc:
 */
HWTEST_F(RecoverTest, recover_UnitTest_SetTimerForRecoverHfpService, TestSize.Level1)
{
    HILOGI("A2dpHfpRecover::SetTimerForRecoverHfpService starts");
    std::string device = "12:34:56:78:9A:BC";
    A2dpHfpRecover::GetInstance().SetTimerForRecoverHfpService(device);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverHfpService_, nullptr);
    HILOGI("A2dpHfpRecover::SetTimerForRecoverHfpService ends");
}

/**
 * @tc.number:recover_UnitTest003
 * @tc.name: SetTimerForRecoverA2dpandHfpService
 * @tc.desc:
 */
HWTEST_F(RecoverTest, recover_UnitTest_SetTimerForRecoverA2dpandHfpService, TestSize.Level1)
{
    HILOGI("SetTimerForRecoverA2dpandHfpService starts");
    std::string device = "12:34:56:78:9A:BC";
    A2dpHfpRecover::GetInstance().SetTimerForRecoverA2dpService(device);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverA2dpService_, nullptr);
    A2dpHfpRecover::GetInstance().SetTimerForRecoverHfpService(device);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverHfpService_, nullptr);
    HILOGI("SetTimerForRecoverA2dpandHfpService ends");
}

/**
 * @tc.number:recover_UnitTest004
 * @tc.name: SetTimerForRecoverHfpandA2dpService
 * @tc.desc:
 */
HWTEST_F(RecoverTest, recover_UnitTest_SetTimerForRecoverHfpandA2dpService, TestSize.Level1)
{
    HILOGI("SetTimerForRecoverHfpandA2dpService starts");
    std::string device = "12:34:56:78:9A:BC";
    A2dpHfpRecover::GetInstance().SetTimerForRecoverHfpService(device);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverHfpService_, nullptr);
    A2dpHfpRecover::GetInstance().SetTimerForRecoverA2dpService(device);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverA2dpService_, nullptr);
    HILOGI("SetTimerForRecoverHfpandA2dpService ends");
}

/**
 * @tc.number:recover_UnitTest005
 * @tc.name: A2dpOrHfpTimerCallback
 * @tc.desc:
 */
HWTEST_F(RecoverTest, recover_UnitTest_A2dpOrHfpTimerCallback, TestSize.Level1)
{
    HILOGI("timerForRecoverHfpService_ starts");
    std::string device = "12:34:56:78:9A:BC";
    A2dpHfpRecover::GetInstance().A2dpOrHfpTimerCallback(RecoverType::HfpRecover, device);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverHfpService_, nullptr);
    A2dpHfpRecover::GetInstance().A2dpOrHfpTimerCallback(RecoverType::A2dpRecover, device);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverHfpService_, nullptr);
    HILOGI("A2dpOrHfpTimerCallback ends");
}

/**
 * @tc.number:recover_UnitTest006
 * @tc.name: SetTimerForRecoverA2dpService001
 * @tc.desc:
 */
HWTEST_F(RecoverTest, recover_UnitTest_SetTimerForRecoverA2dpService001, TestSize.Level1)
{
    HILOGI("A2dpHfpRecover::SetTimerForRecoverA2dpService starts");
    std::string addr = "12:34:56:78:9A:BC";
    std::shared_ptr<BluetoothDevice> device = std::make_shared<BluetoothDevice>(addr);
    // CLASS_OF_DEVICE_AV_HEADSETS
    device->SetDeviceClass(0x0418);
    RemoteDeviceProperties::GetInstance()->remoteDevicesMap_.insert(std::make_pair(addr, device));
    A2dpHfpRecover::GetInstance().SetTimerForRecoverA2dpService(addr);
    EXPECT_EQ(A2dpHfpRecover::GetInstance().timerForRecoverA2dpService_, nullptr);
    HILOGI("A2dpHfpRecover::SetTimerForRecoverA2dpService ends");
}
}  // namespace Bluetooth
}  // namespace OHOS