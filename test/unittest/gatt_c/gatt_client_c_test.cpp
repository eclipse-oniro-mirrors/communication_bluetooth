/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "ohos_bt_gatt_client.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
class GattClientCTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

static constexpr int BT_UUID_LEN = 2;
static constexpr int BT_UUID_1ST = 0xEE;
static constexpr int BT_UUID_2ND = 0xFD;

void GattClientCTest::SetUpTestCase(void) {}

void GattClientCTest::TearDownTestCase(void) {}

void GattClientCTest::SetUp() {}

void GattClientCTest::TearDown() {}

/*
 * @tc.name: GattClientCTest_001
 * @tc.desc: Test BleGattcSetPriority when clientId is not found
 * @tc.type: FUNC
 * @tc.require: issueI5V7QF
*/
HWTEST_F(GattClientCTest, GattClientCTest_001, TestSize.Level1)
{
    int clientId = 0;
    BdAddr bdAddr;
    BtGattPriority priority = OHOS_BT_GATT_PRIORITY_BALANCED;
    int res = BleGattcSetPriority(clientId, &bdAddr, priority);
    EXPECT_EQ(res, OHOS_BT_STATUS_FAIL);
}

/*
 * @tc.name: GattClientCTest_002
 * @tc.desc: Test BleGattcSetPriority after register GATT client with a specified application UUID
 * @tc.type: FUNC
 * @tc.require: issueI5V7QF
*/
HWTEST_F(GattClientCTest, GattClientCTest_002, TestSize.Level1)
{
    BtUuid appId;
    char uuid[BT_UUID_LEN] = { BT_UUID_1ST, BT_UUID_2ND };
    appId.uuidLen = BT_UUID_LEN;
    appId.uuid = uuid;
    int clientId = BleGattcRegister(appId);
    BdAddr bdAddr;
    BtGattPriority priority = OHOS_BT_GATT_PRIORITY_BALANCED;
    int res = BleGattcSetPriority(clientId, &bdAddr, priority);
    EXPECT_EQ(res, OHOS_BT_STATUS_FAIL);
    BleGattcUnRegister(clientId);
}

/*
 * @tc.name: GattClientCTest_003
 * @tc.desc: Test BleGattcSetFastestConn when clientId is not found
 * @tc.type: FUNC
 * @tc.require: issueI5V7QF
*/
HWTEST_F(GattClientCTest, GattClientCTest_003, TestSize.Level1)
{
    int clientId = 0;
    int res = BleGattcSetFastestConn(clientId, false);
    EXPECT_EQ(res, OHOS_BT_STATUS_FAIL);
}

/*
 * @tc.name: GattClientCTest_004
 * @tc.desc: Test BleGattcSetFastestConn when clientId is not found
 * @tc.type: FUNC
 * @tc.require: issueI5V7QF
*/
HWTEST_F(GattClientCTest, GattClientCTest_004, TestSize.Level1)
{
    int clientId = 0;
    int res = BleGattcSetFastestConn(clientId, true);
    EXPECT_EQ(res, OHOS_BT_STATUS_FAIL);
}

/*
 * @tc.name: GattClientCTest_005
 * @tc.desc: Test BleGattcSetFastestConn after register GATT client with a specified application UUID
 * @tc.type: FUNC
 * @tc.require: issueI5V7QF
*/
HWTEST_F(GattClientCTest, GattClientCTest_005, TestSize.Level1)
{
    BtUuid appId;
    char uuid[BT_UUID_LEN] = { BT_UUID_1ST, BT_UUID_2ND };
    appId.uuidLen = BT_UUID_LEN;
    appId.uuid = uuid;
    int clientId = BleGattcRegister(appId);
    int res = BleGattcSetFastestConn(clientId, false);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);
    BleGattcUnRegister(clientId);
}

/*
 * @tc.name: GattClientCTest_006
 * @tc.desc: Test BleGattcSetFastestConn after register GATT client with a specified application UUID
 * @tc.type: FUNC
 * @tc.require: issueI5V7QF
*/
HWTEST_F(GattClientCTest, GattClientCTest_006, TestSize.Level1)
{
    BtUuid appId;
    char uuid[BT_UUID_LEN] = { BT_UUID_1ST, BT_UUID_2ND };
    appId.uuidLen = BT_UUID_LEN;
    appId.uuid = uuid;
    int clientId = BleGattcRegister(appId);
    int res = BleGattcSetFastestConn(clientId, true);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);
    BleGattcUnRegister(clientId);
}

/*
 * @tc.name: GattClientCTest_007
 * @tc.desc: Test BleGattcConnect after set fastest connection with false
 * @tc.type: FUNC
 * @tc.require: issueI5V7QF
*/
HWTEST_F(GattClientCTest, GattClientCTest_007, TestSize.Level1)
{
    BtUuid appId;
    char uuid[BT_UUID_LEN] = { BT_UUID_1ST, BT_UUID_2ND };
    appId.uuidLen = BT_UUID_LEN;
    appId.uuid = uuid;
    int clientId = BleGattcRegister(appId);
    int res = BleGattcSetFastestConn(clientId, false);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);

    BdAddr bdAddr;
    BtGattClientCallbacks func;
    BleGattcConnect(clientId, &func, &bdAddr, false, OHOS_BT_TRANSPORT_TYPE_LE);
    BleGattcUnRegister(clientId);
}

/*
 * @tc.name: GattClientCTest_008
 * @tc.desc: Test BleGattcConnect after set fastest connection with true
 * @tc.type: FUNC
 * @tc.require: issueI5V7QF
*/
HWTEST_F(GattClientCTest, GattClientCTest_008, TestSize.Level1)
{
    BtUuid appId;
    char uuid[BT_UUID_LEN] = { BT_UUID_1ST, BT_UUID_2ND };
    appId.uuidLen = BT_UUID_LEN;
    appId.uuid = uuid;
    int clientId = BleGattcRegister(appId);
    int res = BleGattcSetFastestConn(clientId, true);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);

    BdAddr bdAddr;
    BtGattClientCallbacks func;
    BleGattcConnect(clientId, &func, &bdAddr, false, OHOS_BT_TRANSPORT_TYPE_LE);
    BleGattcUnRegister(clientId);
}
}  // namespace Bluetooth
}  // namespace OHOS
