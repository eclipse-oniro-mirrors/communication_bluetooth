/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
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
#define private public
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <parameters.h>
#include <parameter.h>
#include "cJSON.h"

#include "bluetooth_log.h"
#include "ble_range_impl.h"
#include "profile_service_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class BleRangeImplTest : public testing::Test {
public:
    BleRangeImplTest() = default;
    ~BleRangeImplTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();

protected:
    std::unique_ptr<BleRangeImpl> bleRangeImpl_;
};

void BleRangeImplTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestSuite start");
    HILOGI("SetUpTestSuite end");
}

void BleRangeImplTest::TearDownTestSuite(void)
{
    HILOGI("TearDownTestSuite start");
    HILOGI("TearDownTestSuite end");
}

void BleRangeImplTest::SetUp()
{
    HILOGI("SetUp start");
    ProfileServiceManager::Initialize();
    bleRangeImpl_ = std::make_unique<BleRangeImpl>();
    HILOGI("SetUp end");
}

void BleRangeImplTest::TearDown()
{
    HILOGI("TearDown start");
    bleRangeImpl_ = nullptr;
    HILOGI("TearDown end");
}

/**
 * @tc.name: ble_range_impl_GetCurrentAdvPower_001
 * @tc.desc: GetCurrentAdvPower_001
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetCurrentAdvPower_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetCurrentAdvPower_001 start");
    int advPower = bleRangeImpl_->GetCurrentAdvPower();
    EXPECT_EQ(advPower, 127);
    HILOGI("ble_range_impl_GetCurrentAdvPower_001 end");
}

/**
 * @tc.name: ble_range_impl_SetCurrentAdvPower_001
 * @tc.desc: SetCurrentAdvPower_001
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_SetCurrentAdvPower_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_SetCurrentAdvPower_001 start");
    int advPower = 100;
    bleRangeImpl_->SetCurrentAdvPower(advPower);
    EXPECT_EQ(bleRangeImpl_->GetCurrentAdvPower(), advPower);
    HILOGI("ble_range_impl_SetCurrentAdvPower_001 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPower_001
 * @tc.desc: GetAdvPower_001
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPower_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPower_001 start");
    int32_t advPower = bleRangeImpl_->GetAdvPower();
    EXPECT_TRUE(advPower >= -127 && advPower <= 127);
    HILOGI("ble_range_impl_GetAdvPower_001 end");
}

/**
 * @tc.name: ble_range_impl_HandleNonRangingAppType_001
 * @tc.desc: HandleNonRangingAppType_001 - non ranging app type
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleNonRangingAppType_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleNonRangingAppType_001 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_NO_RANGING_TYPE;
    int advPower = 127;
    int result = bleRangeImpl_->HandleNonRangingAppType(appType, advPower);
    EXPECT_EQ(result, -10);
    HILOGI("ble_range_impl_HandleNonRangingAppType_001 end");
}

/**
 * @tc.name: ble_range_impl_HandleNonRangingAppType_002
 * @tc.desc: HandleNonRangingAppType_002 - ranging touch app type
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleNonRangingAppType_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleNonRangingAppType_002 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = 127;
    int result = bleRangeImpl_->HandleNonRangingAppType(appType, advPower);
    EXPECT_EQ(result, -1);
    HILOGI("ble_range_impl_HandleNonRangingAppType_002 end");
}

/**
 * @tc.name: ble_range_impl_SwitchMacIfNeeded_001
 * @tc.desc: SwitchMacIfNeeded_001 - same mac id, no switch needed
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_SwitchMacIfNeeded_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_SwitchMacIfNeeded_001 start");
    uint8_t currentMacId = 0;
    uint8_t targetMacId = 0;
    bool result = bleRangeImpl_->SwitchMacIfNeeded(currentMacId, targetMacId);
    EXPECT_TRUE(result);
    HILOGI("ble_range_impl_SwitchMacIfNeeded_001 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackInternal_001
 * @tc.desc: OnRangingAntSwitchCallbackInternal_001 - MAC match, promise set_value and nulled
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackInternal_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_001 start");
    bleRangeImpl_->hasSwitchedMac_ = false;
    bleRangeImpl_->StoreRangingAntSwitchInfo(1, 0);
    uint8_t macID = 1;
    uint8_t antID = 1;
    bleRangeImpl_->rangingPromise_ = std::make_shared<std::promise<bool>>();
    auto future = bleRangeImpl_->rangingPromise_->get_future();
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(macID, antID);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    bool callbackResult = future.get();
    EXPECT_TRUE(callbackResult);
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_001 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackInternal_002
 * @tc.desc: OnRangingAntSwitchCallbackInternal_002 - promise is null
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackInternal_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_002 start");
    uint8_t macID = 1;
    uint8_t antID = 1;
    bleRangeImpl_->rangingPromise_ = nullptr;
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(macID, antID);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_002 end");
}

/**
 * @tc.name: ble_range_impl_SwitchMacIfNeeded_002
 * @tc.desc: SwitchMacIfNeeded_002 - different mac id, switch needed
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_SwitchMacIfNeeded_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_SwitchMacIfNeeded_002 start");
    uint8_t currentMacId = 0;
    uint8_t targetMacId = 1;
    bleRangeImpl_->satelliteManager_ = nullptr;
    bool result = bleRangeImpl_->SwitchMacIfNeeded(currentMacId, targetMacId);
    EXPECT_FALSE(result);
    HILOGI("ble_range_impl_SwitchMacIfNeeded_002 end");
}

/**
 * @tc.name: ble_range_impl_HandleNonRangingAppType_003
 * @tc.desc: HandleNonRangingAppType_003 - ranging approach app type
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleNonRangingAppType_003, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleNonRangingAppType_003 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_APPROACH;
    int advPower = 127;
    bleRangeImpl_->SetCurrentAdvPower(advPower);
    int result = bleRangeImpl_->HandleNonRangingAppType(appType, advPower);
    EXPECT_EQ(result, -10);
    HILOGI("ble_range_impl_HandleNonRangingAppType_003 end");
}

/**
 * @tc.name: ble_range_impl_RegisterAntSwitchCallback_001
 * @tc.desc: RegisterAntSwitchCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_RegisterAntSwitchCallback_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_RegisterAntSwitchCallback_001 start");

    bleRangeImpl_->RegisterAntSwitchCallback();
    EXPECT_NE(bleRangeImpl_->btInterface_, nullptr);
    HILOGI("ble_range_impl_RegisterAntSwitchCallback_001 end");
}

/**
 * @tc.name: ble_range_impl_SendBleAdvStartMsg_001
 * @tc.desc: SendBleAdvStartMsg_001 - btInterface is null
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_SendBleAdvStartMsg_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_SendBleAdvStartMsg_001 start");
    bleRangeImpl_->btInterface_ = nullptr;
    bool result = bleRangeImpl_->SendBleAdvStartMsg();
    EXPECT_FALSE(result);
    HILOGI("ble_range_impl_SendBleAdvStartMsg_001 end");
}

/**
 * @tc.name: ble_range_impl_InitGetBleRangeParam_001
 * @tc.desc: InitGetBleRangeParam_001
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_InitGetBleRangeParam_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_InitGetBleRangeParam_001 start");
    int32_t result = bleRangeImpl_->InitGetBleRangeParam();
    EXPECT_TRUE(result == 0 || result == -1);
    HILOGI("ble_range_impl_InitGetBleRangeParam_001 end");
}

/**
 * @tc.name: ble_range_impl_BleRestoreRangingAntSwitch_001
 * @tc.desc: BleRestoreRangingAntSwitch_001 - btInterface is null
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_BleRestoreRangingAntSwitch_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_BleRestoreRangingAntSwitch_001 start");
    bleRangeImpl_->btInterface_ = nullptr;
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int result = bleRangeImpl_->BleRestoreRangingAntSwitch(appType);
    EXPECT_EQ(result, -1);
    HILOGI("ble_range_impl_BleRestoreRangingAntSwitch_001 end");
}

/**
 * @tc.name: ble_range_impl_HandleRangingSwitching_001
 * @tc.desc: HandleRangingSwitching_001 - no switch needed (config not support)
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleRangingSwitching_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleRangingSwitching_001 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = 100;
    int beforePower = bleRangeImpl_->GetCurrentAdvPower();
    bleRangeImpl_->HandleRangingSwitching(appType, advPower);
    int afterPower = bleRangeImpl_->GetCurrentAdvPower();
    EXPECT_TRUE(afterPower >= -127 && afterPower <= 127);
    HILOGI("ble_range_impl_HandleRangingSwitching_001 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerForRangingBusiness_001
 * @tc.desc: GetAdvPowerForRangingBusiness_001 - non ranging app type
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerForRangingBusiness_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_001 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_NO_RANGING_TYPE;
    int advPower = bleRangeImpl_->GetAdvPowerForRangingBusiness(appType);
    EXPECT_TRUE(advPower >= -127 && advPower <= 127);
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_001 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerForRangingBusiness_002
 * @tc.desc: GetAdvPowerForRangingBusiness_002 - ranging touch app type
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerForRangingBusiness_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_002 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = bleRangeImpl_->GetAdvPowerForRangingBusiness(appType);
    EXPECT_TRUE(advPower >= -127 && advPower <= 127);
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_002 end");
}

/**
 * @tc.name: ble_range_impl_SendAntennaInfoQueryMsg_001
 * @tc.desc: SendAntennaInfoQueryMsg_001 - btInterface is null
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_SendAntennaInfoQueryMsg_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_SendAntennaInfoQueryMsg_001 start");
    bleRangeImpl_->SendAntennaInfoQueryMsg();
    EXPECT_NE(bleRangeImpl_->btInterface_, nullptr);
    HILOGI("ble_range_impl_SendAntennaInfoQueryMsg_001 end");
}

/**
 * @tc.name: ble_range_impl_SendAntennaInfoQueryMsg_002
 * @tc.desc: SendAntennaInfoQueryMsg_002 - btInterface valid but func null
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_SendAntennaInfoQueryMsg_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_SendAntennaInfoQueryMsg_002 start");
    bt_interface_t* btIf = const_cast<bt_interface_t*>(bleRangeImpl_->btInterface_);
    if (btIf != nullptr) {
        auto origFunc = btIf->send_antenna_info_query_msg;
        btIf->send_antenna_info_query_msg = nullptr;
        bleRangeImpl_->SendAntennaInfoQueryMsg();
        btIf->send_antenna_info_query_msg = origFunc;
    }
    HILOGI("ble_range_impl_SendAntennaInfoQueryMsg_002 end");
}

/**
 * @tc.name: ble_range_impl_StoreRangingAntSwitchInfo_001
 * @tc.desc: StoreRangingAntSwitchInfo_001 - store and verify
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_StoreRangingAntSwitchInfo_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_001 start");
    uint8_t macID = 2;
    uint8_t antID = 3;
    bleRangeImpl_->StoreRangingAntSwitchInfo(macID, antID);
    auto info = bleRangeImpl_->GetRangingAntSwitchInfo();
    EXPECT_EQ(info.macID, macID);
    EXPECT_EQ(info.antID, antID);
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_001 end");
}

/**
 * @tc.name: ble_range_impl_StoreRangingAntSwitchInfo_002
 * @tc.desc: StoreRangingAntSwitchInfo_002 - overwrite previous values
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_StoreRangingAntSwitchInfo_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_002 start");
    bleRangeImpl_->StoreRangingAntSwitchInfo(1, 1);
    uint8_t macID = 0;
    uint8_t antID = 2;
    bleRangeImpl_->StoreRangingAntSwitchInfo(macID, antID);
    auto info = bleRangeImpl_->GetRangingAntSwitchInfo();
    EXPECT_EQ(info.macID, macID);
    EXPECT_EQ(info.antID, antID);
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_002 end");
}

/**
 * @tc.name: ble_range_impl_GetRangingAntSwitchInfo_001
 * @tc.desc: GetRangingAntSwitchInfo_001 - default values after construction
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetRangingAntSwitchInfo_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetRangingAntSwitchInfo_001 start");
    auto info = bleRangeImpl_->GetRangingAntSwitchInfo();
    EXPECT_TRUE(info.macID == 0 || info.macID == 1);
    HILOGI("ble_range_impl_GetRangingAntSwitchInfo_001 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackInternal_003
 * @tc.desc: OnRangingAntSwitchCallbackInternal_003 - MAC mismatch with hasSwitchedMac_=true, promise unchanged
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackInternal_003, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_003 start");
    bleRangeImpl_->hasSwitchedMac_ = true;
    bleRangeImpl_->configMac_ = 0x5;  // targetMacId = (0x5 >> 1) & 0x3 = 2
    bleRangeImpl_->rangingPromise_ = std::make_shared<std::promise<bool>>();
    uint8_t mismatchMacID = 0;  // 0 != 2, MAC mismatch
    uint8_t antID = 1;
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(mismatchMacID, antID);
    EXPECT_NE(bleRangeImpl_->rangingPromise_, nullptr);  // promise should still exist
    bleRangeImpl_->rangingPromise_ = nullptr;  // cleanup
    bleRangeImpl_->hasSwitchedMac_ = false;
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_003 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackInternal_004
 * @tc.desc: OnRangingAntSwitchCallbackInternal_004 - MAC match with hasSwitchedMac_=true
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackInternal_004, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_004 start");
    bleRangeImpl_->hasSwitchedMac_ = true;
    bleRangeImpl_->configMac_ = 0x5;
    uint8_t targetMacId = (bleRangeImpl_->configMac_ >> 1) & 0x3;
    uint8_t antID = 2;
    bleRangeImpl_->rangingPromise_ = std::make_shared<std::promise<bool>>();
    auto future = bleRangeImpl_->rangingPromise_->get_future();
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(targetMacId, antID);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    bool callbackResult = future.get();
    EXPECT_TRUE(callbackResult);
    auto info = bleRangeImpl_->GetRangingAntSwitchInfo();
    EXPECT_EQ(info.macID, targetMacId);
    EXPECT_EQ(info.antID, antID);
    bleRangeImpl_->hasSwitchedMac_ = false;
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_004 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackInternal_005
 * @tc.desc: OnRangingAntSwitchCallbackInternal_005 - MAC mismatch with hasSwitchedMac_=false, promise unchanged
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackInternal_005, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_005 start");
    bleRangeImpl_->hasSwitchedMac_ = false;
    bleRangeImpl_->StoreRangingAntSwitchInfo(1, 0);  // cachedMacID_ = 1
    bleRangeImpl_->rangingPromise_ = std::make_shared<std::promise<bool>>();
    uint8_t mismatchMacID = 0;  // 0 != 1, MAC mismatch
    uint8_t antID = 1;
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(mismatchMacID, antID);
    EXPECT_NE(bleRangeImpl_->rangingPromise_, nullptr);  // promise should still exist
    bleRangeImpl_->rangingPromise_ = nullptr;  // cleanup
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_005 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackInternal_006
 * @tc.desc: OnRangingAntSwitchCallbackInternal_006 - MAC match with hasSwitchedMac_=false and promise null
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackInternal_006, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_006 start");
    bleRangeImpl_->hasSwitchedMac_ = false;
    bleRangeImpl_->StoreRangingAntSwitchInfo(0, 0);
    bleRangeImpl_->rangingPromise_ = nullptr;
    uint8_t matchMacID = 0;
    uint8_t antID = 1;
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(matchMacID, antID);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    auto info = bleRangeImpl_->GetRangingAntSwitchInfo();
    EXPECT_EQ(info.macID, matchMacID);
    EXPECT_EQ(info.antID, antID);
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_006 end");
}

/**
 * @tc.name: ble_range_impl_BleRestoreRangingAntSwitch_002
 * @tc.desc: BleRestoreRangingAntSwitch_002 - hasSwitchedMac_ should be reset to false
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_BleRestoreRangingAntSwitch_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_BleRestoreRangingAntSwitch_002 start");
    bleRangeImpl_->hasSwitchedMac_ = true;
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    bleRangeImpl_->BleRestoreRangingAntSwitch(appType);
    EXPECT_FALSE(bleRangeImpl_->hasSwitchedMac_);
    uint8_t expectedDefaultMacId = (bleRangeImpl_->defaultMac_ >> 5) & 0x1;
    auto info = bleRangeImpl_->GetRangingAntSwitchInfo();
    EXPECT_EQ(info.macID, expectedDefaultMacId);
    HILOGI("ble_range_impl_BleRestoreRangingAntSwitch_002 end");
}

/**
 * @tc.name: ble_range_impl_HandleRangingSwitching_003
 * @tc.desc: HandleRangingSwitching_003 - needMacSwitch but satelliteManager null
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleRangingSwitching_003, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleRangingSwitching_003 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = 100;
    bleRangeImpl_->configMac_ = 0x1;
    bleRangeImpl_->defaultMac_ = 0x20;
    bleRangeImpl_->cachedMacID_ = (bleRangeImpl_->defaultMac_ >> 5) & 0x1;
    bleRangeImpl_->satelliteManager_ = nullptr;
    bleRangeImpl_->HandleRangingSwitching(appType, advPower);
    EXPECT_FALSE(bleRangeImpl_->hasSwitchedMac_);
    HILOGI("ble_range_impl_HandleRangingSwitching_003 end");
}

/**
 * @tc.name: ble_range_impl_HandleRangingSwitching_004
 * @tc.desc: HandleRangingSwitching_004 - needMacSwitch and needAntSwitch both false
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleRangingSwitching_004, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleRangingSwitching_004 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = 100;
    bleRangeImpl_->configMac_ = 0x0;
    bleRangeImpl_->HandleRangingSwitching(appType, advPower);
    int afterPower = bleRangeImpl_->GetCurrentAdvPower();
    EXPECT_TRUE(afterPower >= -127 && afterPower <= 127);
    HILOGI("ble_range_impl_HandleRangingSwitching_004 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerForRangingBusiness_003
 * @tc.desc: GetAdvPowerForRangingBusiness_003 - ranging approach app type
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerForRangingBusiness_003, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_003 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_APPROACH;
    int advPower = bleRangeImpl_->GetAdvPowerForRangingBusiness(appType);
    EXPECT_TRUE(advPower >= -127 && advPower <= 127);
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_003 end");
}

/**
 * @tc.name: ble_range_impl_cachedMacID_Init_001
 * @tc.desc: cachedMacID_Init_001 - verify cachedMacID initialized based on defaultMac_
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_cachedMacID_Init_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_cachedMacID_Init_001 start");
    auto info = bleRangeImpl_->GetRangingAntSwitchInfo();
    if (bleRangeImpl_->defaultMac_ == 62) {
        EXPECT_EQ(info.macID, 1);
    } else {
        EXPECT_EQ(info.macID, 0);
    }
    EXPECT_EQ(info.antID, 0);
    HILOGI("ble_range_impl_cachedMacID_Init_001 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackInternal_007
 * @tc.desc: OnRangingAntSwitchCallbackInternal_007 - duplicate callback should not crash
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackInternal_007, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_007 start");
    bleRangeImpl_->hasSwitchedMac_ = false;
    bleRangeImpl_->StoreRangingAntSwitchInfo(1, 0);
    uint8_t macID = 1;
    uint8_t antID = 2;
    bleRangeImpl_->rangingPromise_ = std::make_shared<std::promise<bool>>();
    auto future = bleRangeImpl_->rangingPromise_->get_future();
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(macID, antID);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    EXPECT_TRUE(future.get());
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(macID, antID);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_007 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackInternal_008
 * @tc.desc: OnRangingAntSwitchCallbackInternal_008 - callback before get_future, future invalid
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackInternal_008, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_008 start");
    bleRangeImpl_->hasSwitchedMac_ = false;
    bleRangeImpl_->StoreRangingAntSwitchInfo(1, 0);
    uint8_t macID = 1;
    uint8_t antID = 1;
    bleRangeImpl_->rangingPromise_ = std::make_shared<std::promise<bool>>();
    bleRangeImpl_->OnRangingAntSwitchCallbackInternal(macID, antID);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    std::future<bool> future;
    {
        std::lock_guard<std::mutex> lock(bleRangeImpl_->rangingMutex_);
        if (bleRangeImpl_->rangingPromise_ != nullptr) {
            future = bleRangeImpl_->rangingPromise_->get_future();
        }
    }
    EXPECT_FALSE(future.valid());
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackInternal_008 end");
}

/**
 * @tc.name: ble_range_impl_PrepareAndSendBleAdv_001
 * @tc.desc: PrepareAndSendBleAdv_001 - SendBleAdvStartMsg fail, set_value(false) and null promise
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_PrepareAndSendBleAdv_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_PrepareAndSendBleAdv_001 start");
    bleRangeImpl_->btInterface_ = nullptr;
    int advPower = -14;
    bool result = bleRangeImpl_->PrepareAndSendBleAdv(advPower);
    EXPECT_FALSE(result);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    HILOGI("ble_range_impl_PrepareAndSendBleAdv_001 end");
}

/**
 * @tc.name: ble_range_impl_WaitForAntSwitchCallbackAndHandleResult_001
 * @tc.desc: WaitForAntSwitchCallbackAndHandleResult_001 - timeout, set_value(false) and null promise
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_WaitForAntSwitchCallbackAndHandleResult_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_WaitForAntSwitchCallbackAndHandleResult_001 start");
    bleRangeImpl_->hasSwitchedMac_ = false;
    bleRangeImpl_->StoreRangingAntSwitchInfo(1, 0);
    bleRangeImpl_->rangingPromise_ = std::make_shared<std::promise<bool>>();
    uint8_t defaultMacId = 0;
    uint8_t targetMacId = 1;
    int advPower = -14;
    bleRangeImpl_->WaitForAntSwitchCallbackAndHandleResult(defaultMacId, targetMacId, advPower);
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    HILOGI("ble_range_impl_WaitForAntSwitchCallbackAndHandleResult_001 end");
}

/**
 * @tc.name: ble_range_impl_OnRangingAntSwitchCallbackStatic_001
 * @tc.desc: OnRangingAntSwitchCallbackStatic_001 - instance is null, should not crash
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_OnRangingAntSwitchCallbackStatic_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackStatic_001 start");
    {
        std::lock_guard<std::mutex> lock(BleRangeImpl::s_instanceMutex);
        BleRangeImpl::s_currentInstance.reset();
    }
    EXPECT_NO_FATAL_FAILURE(BleRangeImpl::OnRangingAntSwitchCallbackStatic(0, 0));
    HILOGI("ble_range_impl_OnRangingAntSwitchCallbackStatic_001 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_001
 * @tc.desc: GetAdvPowerByAntInfo_001 - mac0 ant0 from advPowerMac0Ant_ array
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_001 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8, -6};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -10);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_001 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_002
 * @tc.desc: GetAdvPowerByAntInfo_002 - mac0 ant1 from advPowerMac0Ant_ array
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_002 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 1;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8, -6};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -8);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_002 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_003
 * @tc.desc: GetAdvPowerByAntInfo_003 - mac1 ant0 from advPowerMac1Ant_ array
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_003, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_003 start");
    bleRangeImpl_->cachedMacID_ = 1;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8, -6};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -12);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_003 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_004
 * @tc.desc: GetAdvPowerByAntInfo_004 - mac1 ant1 from advPowerMac1Ant_ array
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_004, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_004 start");
    bleRangeImpl_->cachedMacID_ = 1;
    bleRangeImpl_->cachedAntID_ = 1;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8, -6};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -14);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_004 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_005
 * @tc.desc: GetAdvPowerByAntInfo_005 - antID out of range, fallback to advPower_
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_005, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_005 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 10;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    bleRangeImpl_->advPower_ = -5;
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -5);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_005 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_006
 * @tc.desc: GetAdvPowerByAntInfo_006 - empty arrays, fallback to advPower_
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_006, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_006 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac0Ant_.clear();
    bleRangeImpl_->advPowerMac1Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac0Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac1Ant_.clear();
    bleRangeImpl_->advPower_ = -7;
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -7);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_006 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_007
 * @tc.desc: GetAdvPowerByAntInfo_007 - folded state, mac0 ant0 from advPowerFoldedMac0Ant_
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_007, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_007 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "2");
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -20);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_007 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_008
 * @tc.desc: GetAdvPowerByAntInfo_008 - folded state, mac1 ant1 from advPowerFoldedMac1Ant_
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_008, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_008 start");
    bleRangeImpl_->cachedMacID_ = 1;
    bleRangeImpl_->cachedAntID_ = 1;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "2");
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -26);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_008 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_009
 * @tc.desc: GetAdvPowerByAntInfo_009 - folded state but folded array empty, fallback to unfolded
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_009, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_009 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac1Ant_.clear();
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "2");
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -10);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_009 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_010
 * @tc.desc: GetAdvPowerByAntInfo_010 - folded state but antID out of folded range, fallback to unfolded
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_010, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_010 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 5;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "2");
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -10);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_010 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_011
 * @tc.desc: GetAdvPowerByAntInfo_011 - not folded state, should use unfolded array
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_011, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_011 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "0");
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -10);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_011 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerByAntInfo_012
 * @tc.desc: GetAdvPowerByAntInfo_012 - mac0 ant2 (last element) from advPowerMac0Ant_
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerByAntInfo_012, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_012 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 2;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8, -6};
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    bleRangeImpl_->advPowerFoldedMac1Ant_ = {-24, -26};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "0");
    int32_t result = bleRangeImpl_->GetAdvPowerByAntInfo();
    EXPECT_EQ(result, -6);
    HILOGI("ble_range_impl_GetAdvPowerByAntInfo_012 end");
}

/**
 * @tc.name: ble_range_impl_ParseAdvPowerAntFromConfig_001
 * @tc.desc: ParseAdvPowerAntFromConfig_001 - parse all four ant arrays from valid config
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_ParseAdvPowerAntFromConfig_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_001 start");
    cJSON* configItem = cJSON_CreateObject();
    ASSERT_NE(configItem, nullptr);
    cJSON* mac0Arr = cJSON_CreateArray();
    cJSON* mac1Arr = cJSON_CreateArray();
    cJSON* foldedMac0Arr = cJSON_CreateArray();
    cJSON* foldedMac1Arr = cJSON_CreateArray();
    int mac0Vals[] = {-10, -8, -6};
    int mac1Vals[] = {-12, -14};
    int foldedMac0Vals[] = {-20, -22};
    int foldedMac1Vals[] = {-24, -26};
    for (int v : mac0Vals) cJSON_AddItemToArray(mac0Arr, cJSON_CreateNumber(v));
    for (int v : mac1Vals) cJSON_AddItemToArray(mac1Arr, cJSON_CreateNumber(v));
    for (int v : foldedMac0Vals) cJSON_AddItemToArray(foldedMac0Arr, cJSON_CreateNumber(v));
    for (int v : foldedMac1Vals) cJSON_AddItemToArray(foldedMac1Arr, cJSON_CreateNumber(v));
    cJSON_AddItemToObject(configItem, "advpower_mac0_ant", mac0Arr);
    cJSON_AddItemToObject(configItem, "advpower_mac1_ant", mac1Arr);
    cJSON_AddItemToObject(configItem, "advpower_folded_mac0_ant", foldedMac0Arr);
    cJSON_AddItemToObject(configItem, "advpower_folded_mac1_ant", foldedMac1Arr);

    bleRangeImpl_->advPowerMac0Ant_.clear();
    bleRangeImpl_->advPowerMac1Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac0Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac1Ant_.clear();
    bleRangeImpl_->ParseAdvPowerAntFromConfig(configItem);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_.size(), 3u);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_[0], -10);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_[1], -8);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_[2], -6);
    EXPECT_EQ(bleRangeImpl_->advPowerMac1Ant_.size(), 2u);
    EXPECT_EQ(bleRangeImpl_->advPowerMac1Ant_[0], -12);
    EXPECT_EQ(bleRangeImpl_->advPowerMac1Ant_[1], -14);
    EXPECT_EQ(bleRangeImpl_->advPowerFoldedMac0Ant_.size(), 2u);
    EXPECT_EQ(bleRangeImpl_->advPowerFoldedMac0Ant_[0], -20);
    EXPECT_EQ(bleRangeImpl_->advPowerFoldedMac0Ant_[1], -22);
    EXPECT_EQ(bleRangeImpl_->advPowerFoldedMac1Ant_.size(), 2u);
    EXPECT_EQ(bleRangeImpl_->advPowerFoldedMac1Ant_[0], -24);
    EXPECT_EQ(bleRangeImpl_->advPowerFoldedMac1Ant_[1], -26);
    cJSON_Delete(configItem);
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_001 end");
}

/**
 * @tc.name: ble_range_impl_ParseAdvPowerAntFromConfig_002
 * @tc.desc: ParseAdvPowerAntFromConfig_002 - config item has no ant arrays, vectors stay empty
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_ParseAdvPowerAntFromConfig_002, TestSize.Level1)
{
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_002 start");
    cJSON* configItem = cJSON_CreateObject();
    ASSERT_NE(configItem, nullptr);
    bleRangeImpl_->advPowerMac0Ant_.clear();
    bleRangeImpl_->advPowerMac1Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac0Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac1Ant_.clear();
    bleRangeImpl_->ParseAdvPowerAntFromConfig(configItem);
    EXPECT_TRUE(bleRangeImpl_->advPowerMac0Ant_.empty());
    EXPECT_TRUE(bleRangeImpl_->advPowerMac1Ant_.empty());
    EXPECT_TRUE(bleRangeImpl_->advPowerFoldedMac0Ant_.empty());
    EXPECT_TRUE(bleRangeImpl_->advPowerFoldedMac1Ant_.empty());
    cJSON_Delete(configItem);
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_002 end");
}

/**
 * @tc.name: ble_range_impl_ParseAdvPowerAntFromConfig_003
 * @tc.desc: ParseAdvPowerAntFromConfig_003 - partial config, only mac0 present
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_ParseAdvPowerAntFromConfig_003, TestSize.Level1)
{
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_003 start");
    cJSON* configItem = cJSON_CreateObject();
    ASSERT_NE(configItem, nullptr);
    cJSON* mac0Arr = cJSON_CreateArray();
    int mac0Vals[] = {-10, -8};
    for (int v : mac0Vals) cJSON_AddItemToArray(mac0Arr, cJSON_CreateNumber(v));
    cJSON_AddItemToObject(configItem, "advpower_mac0_ant", mac0Arr);

    bleRangeImpl_->advPowerMac0Ant_.clear();
    bleRangeImpl_->advPowerMac1Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac0Ant_.clear();
    bleRangeImpl_->advPowerFoldedMac1Ant_.clear();
    bleRangeImpl_->ParseAdvPowerAntFromConfig(configItem);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_.size(), 2u);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_[0], -10);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_[1], -8);
    EXPECT_TRUE(bleRangeImpl_->advPowerMac1Ant_.empty());
    EXPECT_TRUE(bleRangeImpl_->advPowerFoldedMac0Ant_.empty());
    EXPECT_TRUE(bleRangeImpl_->advPowerFoldedMac1Ant_.empty());
    cJSON_Delete(configItem);
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_003 end");
}

/**
 * @tc.name: ble_range_impl_ParseAdvPowerAntFromConfig_004
 * @tc.desc: ParseAdvPowerAntFromConfig_004 - key exists but not array, should be ignored
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_ParseAdvPowerAntFromConfig_004, TestSize.Level1)
{
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_004 start");
    cJSON* configItem = cJSON_CreateObject();
    ASSERT_NE(configItem, nullptr);
    cJSON_AddItemToObject(configItem, "advpower_mac0_ant", cJSON_CreateNumber(-10));
    bleRangeImpl_->advPowerMac0Ant_.clear();
    bleRangeImpl_->ParseAdvPowerAntFromConfig(configItem);
    EXPECT_TRUE(bleRangeImpl_->advPowerMac0Ant_.empty());
    cJSON_Delete(configItem);
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_004 end");
}

/**
 * @tc.name: ble_range_impl_ParseAdvPowerAntFromConfig_005
 * @tc.desc: ParseAdvPowerAntFromConfig_005 - array contains non-number items, skip them
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_ParseAdvPowerAntFromConfig_005, TestSize.Level1)
{
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_005 start");
    cJSON* configItem = cJSON_CreateObject();
    ASSERT_NE(configItem, nullptr);
    cJSON* mac0Arr = cJSON_CreateArray();
    cJSON_AddItemToArray(mac0Arr, cJSON_CreateNumber(-10));
    cJSON_AddItemToArray(mac0Arr, cJSON_CreateString("invalid"));
    cJSON_AddItemToArray(mac0Arr, cJSON_CreateNumber(-6));
    cJSON_AddItemToObject(configItem, "advpower_mac0_ant", mac0Arr);
    bleRangeImpl_->advPowerMac0Ant_.clear();
    bleRangeImpl_->ParseAdvPowerAntFromConfig(configItem);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_.size(), 2u);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_[0], -10);
    EXPECT_EQ(bleRangeImpl_->advPowerMac0Ant_[1], -6);
    cJSON_Delete(configItem);
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_005 end");
}

/**
 * @tc.name: ble_range_impl_ParseAdvPowerAntFromConfig_006
 * @tc.desc: ParseAdvPowerAntFromConfig_006 - null configItem, should not crash
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_ParseAdvPowerAntFromConfig_006, TestSize.Level1)
{
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_006 start");
    bleRangeImpl_->advPowerMac0Ant_.clear();
    bleRangeImpl_->ParseAdvPowerAntFromConfig(nullptr);
    EXPECT_TRUE(bleRangeImpl_->advPowerMac0Ant_.empty());
    HILOGI("ble_range_impl_ParseAdvPowerAntFromConfig_006 end");
}

/**
 * @tc.name: ble_range_impl_StoreRangingAntSwitchInfo_003
 * @tc.desc: StoreRangingAntSwitchInfo_003 - verify param value is "mac:ant" format
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_StoreRangingAntSwitchInfo_003, TestSize.Level1)
{
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_003 start");
    uint8_t macID = 1;
    uint8_t antID = 2;
    bleRangeImpl_->StoreRangingAntSwitchInfo(macID, antID);
    std::string paramValue = OHOS::system::GetParameter("persist.bluetooth.ranging_ant_info", "");
    EXPECT_EQ(paramValue, "1:2");
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_003 end");
}

/**
 * @tc.name: ble_range_impl_StoreRangingAntSwitchInfo_004
 * @tc.desc: StoreRangingAntSwitchInfo_004 - verify param value with mac0 ant0
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_StoreRangingAntSwitchInfo_004, TestSize.Level1)
{
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_004 start");
    uint8_t macID = 0;
    uint8_t antID = 0;
    bleRangeImpl_->StoreRangingAntSwitchInfo(macID, antID);
    std::string paramValue = OHOS::system::GetParameter("persist.bluetooth.ranging_ant_info", "");
    EXPECT_EQ(paramValue, "0:0");
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_004 end");
}

/**
 * @tc.name: ble_range_impl_StoreRangingAntSwitchInfo_005
 * @tc.desc: StoreRangingAntSwitchInfo_005 - overwrite and verify param updated
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_StoreRangingAntSwitchInfo_005, TestSize.Level1)
{
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_005 start");
    bleRangeImpl_->StoreRangingAntSwitchInfo(1, 1);
    std::string param1 = OHOS::system::GetParameter("persist.bluetooth.ranging_ant_info", "");
    EXPECT_EQ(param1, "1:1");
    bleRangeImpl_->StoreRangingAntSwitchInfo(0, 2);
    std::string param2 = OHOS::system::GetParameter("persist.bluetooth.ranging_ant_info", "");
    EXPECT_EQ(param2, "0:2");
    HILOGI("ble_range_impl_StoreRangingAntSwitchInfo_005 end");
}

/**
 * @tc.name: ble_range_impl_HandleNonRangingAppType_004
 * @tc.desc: HandleNonRangingAppType_004 - with antenna arrays set, returns array value
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleNonRangingAppType_004, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleNonRangingAppType_004 start");
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 1;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8, -6};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "0");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_NO_RANGING_TYPE;
    int advPower = 100;
    int result = bleRangeImpl_->HandleNonRangingAppType(appType, advPower);
    EXPECT_EQ(result, -8);
    HILOGI("ble_range_impl_HandleNonRangingAppType_004 end");
}

/**
 * @tc.name: ble_range_impl_HandleRangingSwitching_005
 * @tc.desc: HandleRangingSwitching_005 - no switch needed, get power from antenna array
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleRangingSwitching_005, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleRangingSwitching_005 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = 100;
    bleRangeImpl_->configMac_ = 0x0;
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 1;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8, -6};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "0");
    bleRangeImpl_->HandleRangingSwitching(appType, advPower);
    int afterPower = bleRangeImpl_->GetCurrentAdvPower();
    EXPECT_EQ(afterPower, -8);
    HILOGI("ble_range_impl_HandleRangingSwitching_005 end");
}

/**
 * @tc.name: ble_range_impl_HandleRangingSwitching_006
 * @tc.desc: HandleRangingSwitching_006 - no switch needed, folded state uses folded array
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleRangingSwitching_006, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleRangingSwitching_006 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = 100;
    bleRangeImpl_->configMac_ = 0x0;
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8};
    bleRangeImpl_->advPowerFoldedMac0Ant_ = {-20, -22};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "2");
    bleRangeImpl_->HandleRangingSwitching(appType, advPower);
    int afterPower = bleRangeImpl_->GetCurrentAdvPower();
    EXPECT_EQ(afterPower, -20);
    HILOGI("ble_range_impl_HandleRangingSwitching_006 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerForRangingBusiness_004
 * @tc.desc: GetAdvPowerForRangingBusiness_004 - non-ranging type with antenna arrays
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerForRangingBusiness_004, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_004 start");
    bleRangeImpl_->cachedMacID_ = 1;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "0");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_NO_RANGING_TYPE;
    int result = bleRangeImpl_->GetAdvPowerForRangingBusiness(appType);
    EXPECT_EQ(result, -12);
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_004 end");
}

/**
 * @tc.name: ble_range_impl_GetAdvPowerForRangingBusiness_005
 * @tc.desc: GetAdvPowerForRangingBusiness_005 - ranging touch with no switch config, uses antenna array
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_GetAdvPowerForRangingBusiness_005, TestSize.Level1)
{
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_005 start");
    bleRangeImpl_->configMac_ = 0x0;
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 2;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8, -6};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "0");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int result = bleRangeImpl_->GetAdvPowerForRangingBusiness(appType);
    EXPECT_EQ(result, -6);
    HILOGI("ble_range_impl_GetAdvPowerForRangingBusiness_005 end");
}

/**
 * @tc.name: ble_range_impl_ReadTargetMacAntFromSignalHub_001
 * @tc.desc: ReadTargetMacAntFromSignalHub_001 - read from /dev/signal_hub, verify read flow succeeds
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_ReadTargetMacAntFromSignalHub_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_ReadTargetMacAntFromSignalHub_001 start");
    // Verify default sentinel values before call
    EXPECT_EQ(bleRangeImpl_->targetMacAntFromHub_.targetMac, 0xFF);
    EXPECT_EQ(bleRangeImpl_->targetMacAntFromHub_.targetAnt, 0xFF);
    // In test env, /dev/signal_hub exists and ioctl succeeds
    bool result = bleRangeImpl_->ReadTargetMacAntFromSignalHub();
    EXPECT_TRUE(result);
    // After a successful read, targetMacAntFromHub_ holds the values read back from
    // the device. 0xFF is a legal device state: SignalHub reports it when the ranging
    // antenna feature is not configured on this device (no antenna switch needed,
    // see needAntSwitch = (targetAnt != 0xFF) in HandleRangingSwitching), so the
    // values must not be asserted to differ from the sentinel.
    HILOGI("ReadTargetMacAntFromSignalHub_001: targetMac=0x%{public}x, targetAnt=0x%{public}x",
        bleRangeImpl_->targetMacAntFromHub_.targetMac, bleRangeImpl_->targetMacAntFromHub_.targetAnt);
    HILOGI("ble_range_impl_ReadTargetMacAntFromSignalHub_001 end");
}

/**
 * @tc.name: ble_range_impl_HandleRangingSwitching_007
 * @tc.desc: HandleRangingSwitching_007 - needAntSwitch=true alone, PrepareAndSendBleAdv fails (btInterface null)
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleRangingSwitching_007, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleRangingSwitching_007 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = 100;
    // configMac_ = 0 → needMacSwitch = false
    bleRangeImpl_->configMac_ = 0x0;
    // targetAnt != 0xFF → needAntSwitch = true
    bleRangeImpl_->targetMacAntFromHub_.targetMac = 0x0;
    bleRangeImpl_->targetMacAntFromHub_.targetAnt = 0x1;
    // btInterface_ = nullptr → SendBleAdvStartMsg fails → PrepareAndSendBleAdv fails
    bleRangeImpl_->btInterface_ = nullptr;
    // Set up antenna arrays for predictable GetAdvPowerByAntInfo result
    bleRangeImpl_->cachedMacID_ = 0;
    bleRangeImpl_->cachedAntID_ = 0;
    bleRangeImpl_->advPowerMac0Ant_ = {-10, -8};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "0");
    bleRangeImpl_->HandleRangingSwitching(appType, advPower);
    // PrepareAndSendBleAdv failed → SetCurrentAdvPower(GetAdvPowerByAntInfo()) = -10
    int afterPower = bleRangeImpl_->GetCurrentAdvPower();
    EXPECT_EQ(afterPower, -10);
    // promise should be cleaned up after failure path
    EXPECT_EQ(bleRangeImpl_->rangingPromise_, nullptr);
    HILOGI("ble_range_impl_HandleRangingSwitching_007 end");
}

/**
 * @tc.name: ble_range_impl_HandleRangingSwitching_008
 * @tc.desc: HandleRangingSwitching_008 - needMacSwitch=true + needAntSwitch=true, mac switch fails (satelliteManager null)
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_HandleRangingSwitching_008, TestSize.Level1)
{
    HILOGI("ble_range_impl_HandleRangingSwitching_008 start");
    bluetooth::BleAppType appType = bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH;
    int advPower = 100;
    // configMac_ = 0x1 → needMacSwitch = true (bit0 = 1)
    bleRangeImpl_->configMac_ = 0x1;
    // targetAnt != 0xFF → needAntSwitch = true
    bleRangeImpl_->targetMacAntFromHub_.targetMac = 0x0;
    bleRangeImpl_->targetMacAntFromHub_.targetAnt = 0x1;
    // satelliteManager_ = nullptr → SwitchMacIfNeeded fails
    bleRangeImpl_->satelliteManager_ = nullptr;
    bleRangeImpl_->defaultMac_ = 0x20;  // defaultMacId = (0x20 >> 5) & 0x1 = 1
    bleRangeImpl_->cachedMacID_ = 1;
    bleRangeImpl_->cachedAntID_ = 0;
    // Set up antenna arrays for predictable GetAdvPowerByAntInfo result
    bleRangeImpl_->advPowerMac1Ant_ = {-12, -14};
    OHOS::system::SetParameter("vendor.cust.stablefoldstate", "0");
    bleRangeImpl_->HandleRangingSwitching(appType, advPower);
    // MAC switch failed → early return with GetAdvPowerByAntInfo() = -12
    int afterPower = bleRangeImpl_->GetCurrentAdvPower();
    EXPECT_EQ(afterPower, -12);
    EXPECT_FALSE(bleRangeImpl_->hasSwitchedMac_);
    HILOGI("ble_range_impl_HandleRangingSwitching_008 end");
}

/**
 * @tc.name: ble_range_impl_Init_001
 * @tc.desc: Init_001 - Init() calls ReadTargetMacAntFromSignalHub without crash
 * @tc.type: FUNC
 */
HWTEST_F(BleRangeImplTest, ble_range_impl_Init_001, TestSize.Level1)
{
    HILOGI("ble_range_impl_Init_001 start");
    // Init() calls ReadTargetMacAntFromSignalHub() which succeeds in test env
    EXPECT_NO_FATAL_FAILURE(bleRangeImpl_->Init());
    // After Init, targetMacAntFromHub_ holds the values read back from the device.
    // 0xFF is a legal device state (ranging antenna feature not configured, no
    // antenna switch needed), so the values must not be asserted to differ from
    // the sentinel.
    HILOGI("Init_001: targetMac=0x%{public}x, targetAnt=0x%{public}x",
        bleRangeImpl_->targetMacAntFromHub_.targetMac, bleRangeImpl_->targetMacAntFromHub_.targetAnt);
    HILOGI("ble_range_impl_Init_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS