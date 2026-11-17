/*
 * Copyright (c) 2012 Huawei Device Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file EXPECT in compliance with the License.
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

#undef private
#define private public

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "raw_address.h"
#include "common/raw_address.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "power_manager.h"
#include "bluetooth_log.h"
#include "power_manager.cpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

constexpr int TIME = 2;
OHOS::bluetooth::RawAddress addr("11:22:33:44:55:66");
static PowerManager *powerManager;
static Bluetooth::BluetoothHost *host_;

class PowerManagerTest : public testing::Test {
public:
    PowerManagerTest() = default;
    ~PowerManagerTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void PowerManagerTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    powerManager = PowerManager::GetInstance();
    host_ = &Bluetooth::BluetoothHost::GetDefaultHost();

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

void PowerManagerTest::TearDownTestSuite(void)
{
    sleep(TIME);
    host_ = nullptr;
    powerManager = nullptr;
}

void PowerManagerTest::SetUp() {}

void PowerManagerTest::TearDown() {}

/**
 * @tc.name: Enable
 * @tc.desc: Enable
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_Enable_001, TestSize.Level1)
{
    HILOGI("PowerManager_Enable_001 start");

    powerManager->Enable();

    HILOGI("PowerManager_Enable_001 end");
}

/**
 * @tc.name: StatusUpdate
 * @tc.desc: StatusUpdate
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_StatusUpdate_001, TestSize.Level1)
{
    HILOGI("PowerManager_StatusUpdate_001 start");

    std::string profileName = "PROFILE_NAME_HFP_AG";
    powerManager->StatusUpdate(RequestStatus::SCO_ON, profileName, addr);

    HILOGI("PowerManager_StatusUpdate_001 end");
}

/**
 * @tc.name: GetPowerMode
 * @tc.desc: GetPowerMode
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_GetPowerMode_001, TestSize.Level1)
{
    HILOGI("PowerManager_GetPowerMode_001 start");

    powerManager->GetPowerMode(addr);

    HILOGI("PowerManager_GetPowerMode_001 end");
}

/**
 * @tc.name: PowerProcess
 * @tc.desc: PowerProcess
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_PowerProcess_001, TestSize.Level1)
{
    HILOGI("PowerManager_PowerProcess_001 start");

    std::string profileName = "PROFILE_NAME_HFP_AG";
    PowerManager powerManager;
    auto pimpl = powerManager.pimpl.get();
    pimpl->isEnabled_ = true;
    pimpl->PowerProcess(RequestStatus::SCO_ON, profileName, addr);

    HILOGI("PowerManager_PowerProcess_001 end");
}

/**
 * @tc.name: UpdatePowerDevicesInfo
 * @tc.desc: UpdatePowerDevicesInfo
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_UpdatePowerDevicesInfo_001, TestSize.Level1)
{
    HILOGI("PowerManager_UpdatePowerDevicesInfo_001 start");

    std::string profileName = "PROFILE_NAME_HFP_AG";
    PowerManager powerManager;
    auto pimpl = powerManager.pimpl.get();
    pimpl->isEnabled_ = true;
    pimpl->UpdatePowerDevicesInfo(addr, profileName, RequestStatus::CONNECT_OFF);
    pimpl->UpdatePowerDevicesInfo(addr, profileName, RequestStatus::SCO_ON);

    HILOGI("PowerManager_UpdatePowerDevicesInfo_001 end");
}

/**
 * @tc.name: ModeChangeCallBackProcess
 * @tc.desc: ModeChangeCallBackProcess
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_ModeChangeCallBackProcess_001, TestSize.Level1)
{
    HILOGI("PowerManager_ModeChangeCallBackProcess_001 start");

    PowerManager powerManager;
    auto pimpl = powerManager.pimpl.get();
    pimpl->isEnabled_ = true;
    pimpl->ModeChangeCallBackProcess(0, addr, 0, 0);

    HILOGI("PowerManager_ModeChangeCallBackProcess_001 end");
}

/**
 * @tc.name: SsrCompleteCallBackProcess
 * @tc.desc: SsrCompleteCallBackProcess
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_SsrCompleteCallBackProcess_001, TestSize.Level1)
{
    HILOGI("PowerManager_SsrCompleteCallBackProcess_001 start");

    PowerManager powerManager;
    auto pimpl = powerManager.pimpl.get();
    pimpl->isEnabled_ = true;
    pimpl->SsrCompleteCallBackProcess(0, addr);

    HILOGI("PowerManager_ModeChangeCallBackProcess_001 end");
}

/**
 * @tc.name: ConnectionCompleteCallBackProcess
 * @tc.desc: ConnectionCompleteCallBackProcess
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_ConnectionCompleteCallBackProcess_001, TestSize.Level1)
{
    HILOGI("PowerManager_ConnectionCompleteCallBackProcess_001 start");

    PowerManager powerManager;
    auto pimpl = powerManager.pimpl.get();
    pimpl->isEnabled_ = true;
    pimpl->ConnectionCompleteCallBackProcess(addr, 0);

    HILOGI("PowerManager_ConnectionCompleteCallBackProcess_001 end");
}

/**
 * @tc.name: DisconnectionCompleteCallBackProcess
 * @tc.desc: DisconnectionCompleteCallBackProcess
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_DisconnectionCompleteCallBackProcess_001, TestSize.Level1)
{
    HILOGI("PowerManager_DisconnectionCompleteCallBackProcess_001 start");

    PowerManager powerManager;
    auto pimpl = powerManager.pimpl.get();
    pimpl->isEnabled_ = true;
    pimpl->DisconnectionCompleteCallBackProcess(0, 0, 0);

    HILOGI("PowerManager_DisconnectionCompleteCallBackProcess_001 end");
}

/**
 * @tc.name: Disable
 * @tc.desc: Disable
 * @tc.type: FUNC
 */
HWTEST_F(PowerManagerTest, PowerManager_Disable_001, TestSize.Level1)
{
    HILOGI("PowerManager_Disable_001 start");

    powerManager->Disable();

    HILOGI("PowerManager_Disable_001 end");
}
}
}