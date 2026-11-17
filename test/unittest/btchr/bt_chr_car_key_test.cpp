/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd. All rights reserved.
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bluetooth_log.h"
#include "bt_chr_car_key.h"
#include "preferences_manager.h"
#include "vendor_interface.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

class BtChrCarKeyTest : public testing::Test {
public:
    BtChrCarKeyTest() = default;
    ~BtChrCarKeyTest() = default;

    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}

    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Test_IsWalletDftEvent_001
 * @tc.desc: Verify the IsWalletDftEvent function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrCarKeyTest, Test_IsWalletDftEvent_001, TestSize.Level1)
{
    HILOGI("Test_IsWalletDftEvent_001 enter");
    BtChrEventParam param = BtChrEventParam("ENCRYPTIONSTATUS", 1);
    EXPECT_TRUE(CarKey::IsWalletDftEvent(CHR_BLE_DISCONNECT, param));
    EXPECT_FALSE(CarKey::IsWalletDftEvent(CHR_UE_BLE_START_SCAN, param));
    BtChrEventParam testParam = BtChrEventParam("test", 1);
    EXPECT_FALSE(CarKey::IsWalletDftEvent(CHR_BLE_DISCONNECT, testParam));
    HILOGI("Test_IsWalletDftEvent_001 end");
}

/**
 * @tc.name: Test_InitCardInfo_001
 * @tc.desc: Verify the InitCardInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrCarKeyTest, Test_InitCardInfo_001, TestSize.Level1)
{
    HILOGI("Test_InitCardInfo_001 enter");
    std::string addr = "11:22:33:44:55:66";
    PreferencesManager::SaveString(addr, "", PreferencesManagerType::CAR_KEY_CARD);
    CarKey::InitCardInfo();
    EXPECT_FALSE(PreferencesManager::GetAll(PreferencesManagerType::CAR_KEY_CARD).empty());
    HILOGI("Test_InitCardInfo_001 end");
}

/**
 * @tc.name: Test_SetCardChrInfo_001
 * @tc.desc: Verify the SetCardChrInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrCarKeyTest, Test_SetCardChrInfo_001, TestSize.Level1)
{
    HILOGI("Test_SetCardChrInfo_001 enter");
    std::string addr = "11:22:33:44:55:66";
    int32_t addAction = 1;
    CarKey::SetCardChrInfo(addr, addAction);
    const map<std::string, PreferencesValue> cards = PreferencesManager::GetAll(PreferencesManagerType::CAR_KEY_CARD);
    EXPECT_FALSE(cards.empty());
    HILOGI("Test_SetCardChrInfo_001 end");
}

/**
 * @tc.name: Test_GetChrData_001
 * @tc.desc: Verify the RecordWalletChr function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrCarKeyTest, Test_GetChrData_001, TestSize.Level1)
{
    HILOGI("Test_GetChrData_001 enter");
    const string &chrDataStr = CarKey::GetChrData();
    EXPECT_EQ(chrDataStr, "");
    std::string addr = "11:22:**:**:**:66";
    int32_t addAction = 1;
    CarKey::SetCardChrInfo(addr, addAction);
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("DEV_ADDRESS", addr));
    CarKey::RecordWalletChr(CHR_UE_BLE_START_SCAN, params);
    CarKey::RecordWalletChr(CHR_UE_START_PAIR, params);
    CarKey::RecordWalletChr(CHR_BT_PAIR_EXCEPTION, params);
    const string &chrData = CarKey::GetChrData();
    EXPECT_NE(chrData, "");
    addr = "11:22:**:**:66";
    addAction = 1;
    CarKey::SetCardChrInfo(addr, addAction);
    std::vector<BtChrEventParam> params1;
    params.push_back(BtChrEventParam("DEV_ADDRESS", addr));
    CarKey::RecordWalletChr(CHR_UE_BLE_START_SCAN, params1);
    CarKey::RecordWalletChr(CHR_UE_START_PAIR, params1);
    CarKey::RecordWalletChr(CHR_BT_PAIR_EXCEPTION, params1);
    const string &chrData1 = CarKey::GetChrData();
    EXPECT_EQ(chrData1, "");
    HILOGI("Test_GetChrData_001 end");
}

/**
 * @tc.name: Test_CarKeyProcessLogReport_001
 * @tc.desc: Verify the CarKeyProcessLogReport function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrCarKeyTest, Test_CarKeyProcessLogReport_001, TestSize.Level1)
{
    HILOGI("Test_CarKeyProcessLogReport_001 enter");
    // 构造未开卡设备进入上报流程时
    std::string addr = "11:22:**:**:**:66";
    CarKey::CarKeyProcessLogReport(CarKey::START_GATT_CONNECT, addr, VendorInterface::GetWalletServiceName());

    // 开卡后日志上报
    int32_t addAction = 1;
    CarKey::SetCardChrInfo(addr, addAction);
    const map<std::string, PreferencesValue> cards = PreferencesManager::GetAll(PreferencesManagerType::CAR_KEY_CARD);
    CarKey::CarKeyProcessLogReport(CarKey::START_GATT_CONNECT, addr, VendorInterface::GetWalletServiceName());

    CarKey::CarKeyProcessLogReport(CarKey::GATT_CONNECTED, addr);
    addAction = 0;
    CarKey::SetCardChrInfo(addr, addAction);
    EXPECT_FALSE(cards.empty());
    HILOGI("Test_CarKeyProcessLogReport_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS