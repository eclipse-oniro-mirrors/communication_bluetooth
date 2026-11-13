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
#include <fstream>
#include "bluetooth_manager_proxy.h"
#include "bluetooth_log.h"
#include "cJSON.h"
#include "system_ability_definition.h"
#include "parameters.h"
#include "bluetooth_edm_manager.h"

using namespace OHOS::bluetooth;
using namespace OHOS::EDM;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
const std::string SET_EDM_SERVICE = "persist.edm.edm_enable";
static const char *const BLUETOOTH_ALLOWEDLIST_CHANGED_EVENT = "com.ohos.edm.bluetoothdeviceschanged";
static const char *const BLUETOOTH_DISALLOWEDLIST_CHANGED_EVENT = "com.ohos.edm.disallowedbluetoothdeviceschanged";
static const char *const BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT = "usual.event.EDM_CONFIG_CHANGED";
static BluetoothEdmManager &mgr_ = BluetoothEdmManager::GetInstance();

class BluetoothEdmManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<BluetoothManagerProxy> proxy_ = nullptr;
};


static void SetEdmServiceEnable()
{
    system::SetParameter(SET_EDM_SERVICE, "true");
}

void BluetoothEdmManagerTest::SetUp()
{
    proxy_ = BluetoothManagerProxy::GetBluetoothManagerProxy();
    SetEdmServiceEnable();
}

void BluetoothEdmManagerTest::TearDown()
{
    proxy_.reset();
}

/**
 * @tc.name: Edm_Mgr_test_IsAllowedConnect_001
 * @tc.desc: BluetoothEdmManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsAllowedConnect_001, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_IsAllowedConnect_001 enter");
    mgr_.Init();
    mgr_.Init();
    bool ret = mgr_.IsAllowedConnect("");
    EXPECT_FALSE(ret);
    ret = mgr_.IsAllowedConnect("11:22:33:44:55:66");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAllowedConnect("FF:FF:FF:44:55:66");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAllowedConnect("11:22:33:44:55:66:77");
    EXPECT_FALSE(ret);
    ret = mgr_.IsAllowedConnect("11:GG:33:44:55:66");
    EXPECT_FALSE(ret);
    HILOGI("Edm_Mgr_test_IsAllowedConnect_001 end");
}

/**
 * @tc.name: Edm_Mgr_test_IsAllowedConnect_002
 * @tc.desc: BluetoothEdmManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsAllowedConnect_002, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_IsAllowedConnect_002 enter");
    mgr_.Init();
    mgr_.allowedListMap_.clear();
    mgr_.realAllowedAddrSet_.clear();
    // No converter is registered, so the value equals the key.
    mgr_.allowedListMap_.emplace("11:22:33:44:55:66", "11:22:33:44:55:66");
    mgr_.realAllowedAddrSet_.insert("11:22:33:44:55:66");
    bool ret = mgr_.IsAllowedConnect("11:22:33:44:55:66");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAllowedConnect("FF:FF:FF:44:55:77");
    EXPECT_FALSE(ret);
    mgr_.allowedListMap_.clear();
    mgr_.realAllowedAddrSet_.clear();
    ret = mgr_.IsAllowedConnect("FF:FF:FF:44:55:77");
    EXPECT_TRUE(ret);
    HILOGI("Edm_Mgr_test_IsAllowedConnect_002 end");
}

/**
 * @tc.name: Edm_Mgr_test_IsAllowedConnect_003
 * @tc.desc: BluetoothEdmManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsAllowedConnect_003, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_IsAllowedConnect_003 enter");
    mgr_.Init();
    mgr_.disallowedListMap_.clear();
    mgr_.realDisallowedAddrSet_.clear();
    // No converter is registered, so the value equals the key.
    mgr_.disallowedListMap_.emplace("11:22:33:44:55:66", "11:22:33:44:55:66");
    mgr_.realDisallowedAddrSet_.insert("11:22:33:44:55:66");
    bool ret = mgr_.IsAllowedConnect("11:22:33:44:55:66");
    EXPECT_FALSE(ret);
    ret = mgr_.IsAllowedConnect("FF:FF:FF:44:55:77");
    EXPECT_TRUE(ret);
    mgr_.disallowedListMap_.clear();
    mgr_.realDisallowedAddrSet_.clear();
    ret = mgr_.IsAllowedConnect("FF:FF:FF:44:55:77");
    EXPECT_TRUE(ret);
    HILOGI("Edm_Mgr_test_IsAllowedConnect_003 end");
}

/**
 * @tc.name: Edm_Mgr_test_IsAccountConfigAllowedConnect_001
 * @tc.desc: BluetoothEdmManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsAccountConfigAllowedConnect_001, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_IsAccountConfigAllowedConnect_001 enter");
    mgr_.Init();
    std::vector<std::string> profiles;
    profiles.push_back("GATT");
    profiles.push_back("OPP");
    profiles.push_back("XXX");
    mgr_.accountDenylist_["100"]= profiles;
    ControlInterceptMessage msg {
        .addr = "11:22:33:44:55:66",
        .isSystemHap = true,
        .isNativeCaller = true,
    };
    bool ret = mgr_.IsAccountConfigAllowedConnect(msg, "GATT");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAccountConfigAllowedConnect(msg, "SPP");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAccountConfigAllowedConnect(msg, "OPP");
    EXPECT_FALSE(ret);
    ret = mgr_.IsAccountConfigAllowedConnect(msg, "XXX");
    EXPECT_TRUE(ret);
    mgr_.accountDenylist_.clear();
    ret = mgr_.IsAccountConfigAllowedConnect(msg, "OPP");
    EXPECT_TRUE(ret);
    HILOGI("Edm_Mgr_test_IsAccountConfigAllowedConnect_001 end");
}

/**
 * @tc.name: Edm_Mgr_test_IsValidAddress_001
 * @tc.desc: BluetoothEdmManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsValidAddress_001, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_IsValidAddress_001 enter");
    bool ret = mgr_.IsAllowedConnect("11:22:33:44:55:66");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAllowedConnect("11:22:33:44:55:66:77");
    EXPECT_FALSE(ret);
    HILOGI("Edm_Mgr_test_IsValidAddress_001 end");
}

/**
 * @tc.name: Edm_Mgr_test_GetDisallowedList_001
 * @tc.desc: BluetoothEdmManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_OnEdmListChanged_001, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_OnEdmListChanged_001 enter");
    mgr_.OnEdmListChanged(BLUETOOTH_ALLOWEDLIST_CHANGED_EVENT);
    mgr_.OnEdmListChanged(BLUETOOTH_DISALLOWEDLIST_CHANGED_EVENT);
    mgr_.OnEdmListChanged(BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT);
    HILOGI("Edm_Mgr_test_OnEdmListChanged_001 end");
}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_001
 * @tc.desc: Test ParseAccountDenylist with normal case: single account, single profile
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_001, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_001 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();
    cJSON *account = cJSON_CreateArray();
    cJSON_AddItemToArray(account, cJSON_CreateString("GATT"));
    cJSON_AddItemToObject(obj, "100", account);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 1);
    EXPECT_EQ(denylist.count("100"), 1);
    EXPECT_EQ(denylist["100"].size(), 1);
    EXPECT_EQ(denylist["100"][0], "GATT");

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_001 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_002
 * @tc.desc: Test ParseAccountDenylist with normal case: single account, multiple profiles
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_002, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_002 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();
    cJSON *account = cJSON_CreateArray();
    cJSON_AddItemToArray(account, cJSON_CreateString("GATT"));
    cJSON_AddItemToArray(account, cJSON_CreateString("OPP"));
    cJSON_AddItemToArray(account, cJSON_CreateString("SPP"));
    cJSON_AddItemToObject(obj, "100", account);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 1);
    EXPECT_EQ(denylist.count("100"), 1);
    EXPECT_EQ(denylist["100"].size(), 3);
    EXPECT_EQ(denylist["100"][0], "GATT");
    EXPECT_EQ(denylist["100"][1], "OPP");
    EXPECT_EQ(denylist["100"][2], "SPP");

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_002 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_003
 * @tc.desc: Test ParseAccountDenylist with normal case: multiple accounts, each with multiple profiles
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_003, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_003 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();

    cJSON *account1 = cJSON_CreateArray();
    cJSON_AddItemToArray(account1, cJSON_CreateString("GATT"));
    cJSON_AddItemToArray(account1, cJSON_CreateString("OPP"));
    cJSON_AddItemToObject(obj, "100", account1);

    cJSON *account2 = cJSON_CreateArray();
    cJSON_AddItemToArray(account2, cJSON_CreateString("SPP"));
    cJSON_AddItemToArray(account2, cJSON_CreateString("GATT"));
    cJSON_AddItemToObject(obj, "200", account2);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 2);
    EXPECT_EQ(denylist.count("100"), 1);
    EXPECT_EQ(denylist.count("200"), 1);
    EXPECT_EQ(denylist["100"].size(), 2);
    EXPECT_EQ(denylist["200"].size(), 2);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_003 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_004
 * @tc.desc: Test ParseAccountDenylist with boundary case: accountSize = 1000
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_004, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_004 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();

    for (int i = 0; i < 1000; i++) {
        cJSON *account = cJSON_CreateArray();
        cJSON_AddItemToArray(account, cJSON_CreateString("GATT"));
        cJSON_AddItemToObject(obj, std::to_string(i).c_str(), account);
    }

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 1000);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_004 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_005
 * @tc.desc: Test ParseAccountDenylist with abnormal case: accountSize = 1001 (exceeds MAX_ACCOUNT_SIZE)
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_005, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_005 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();

    for (int i = 0; i < 1001; i++) {
        cJSON *account = cJSON_CreateArray();
        cJSON_AddItemToArray(account, cJSON_CreateString("GATT"));
        cJSON_AddItemToObject(obj, std::to_string(i).c_str(), account);
    }

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, -1);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_005 end");
}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_006
 * @tc.desc: Test ParseAccountDenylist with abnormal case: childNode is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_006, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_006 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();
    cJSON *account = cJSON_CreateArray();
    cJSON_AddItemToArray(account, cJSON_CreateString("GATT"));
    cJSON_AddItemToObject(obj, "100", account);
    cJSON_AddItemToArray(obj, nullptr);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 1);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_006 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_007
 * @tc.desc: Test ParseAccountDenylist with abnormal case: childNode->string is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_007, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_007 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();
    cJSON *account = cJSON_CreateArray();
    cJSON_AddItemToArray(account, cJSON_CreateString("GATT"));
    cJSON_AddItemToArray(obj, account);
    account->string = nullptr;

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 0);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_007 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_008
 * @tc.desc: Test ParseAccountDenylist with boundary case: profileSize = 500
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_008, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_008 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();
    cJSON *account = cJSON_CreateArray();
    for (int i = 0; i < 500; i++) {
        cJSON_AddItemToArray(account, cJSON_CreateString("GATT"));
    }
    cJSON_AddItemToObject(obj, "100", account);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 1);
    EXPECT_EQ(denylist["100"].size(), 500);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_008 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_009
 * @tc.desc: Test ParseAccountDenylist with abnormal case: profileSize = 501 (exceeds MAX_PROFILE_SIZE, continue)
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_009, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_009 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();
    cJSON *account1 = cJSON_CreateArray();
    cJSON_AddItemToArray(account1, cJSON_CreateString("GATT"));
    cJSON_AddItemToObject(obj, "100", account1);

    cJSON *account2 = cJSON_CreateArray();
    for (int i = 0; i < 501; i++) {
        cJSON_AddItemToArray(account2, cJSON_CreateString("OPP"));
    }
    cJSON_AddItemToObject(obj, "200", account2);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 1);
    EXPECT_EQ(denylist.count("100"), 1);
    EXPECT_EQ(denylist.count("200"), 0);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_009 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_010
 * @tc.desc: Test ParseAccountDenylist with abnormal case: profileNode is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_010, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_010 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();
    cJSON *account = cJSON_CreateArray();
    cJSON_AddItemToArray(account, cJSON_CreateString("GATT"));
    cJSON_AddItemToArray(account, nullptr);
    cJSON_AddItemToObject(obj, "100", account);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 1);
    EXPECT_EQ(denylist["100"].size(), 1);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_010 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_011
 * @tc.desc: Test ParseAccountDenylist with abnormal case: profileNode->valuestring is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_011, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_011 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();
    cJSON *account = cJSON_CreateArray();
    cJSON *profile1 = cJSON_CreateString("GATT");
    cJSON_AddItemToArray(account, profile1);
    cJSON *profile2 = cJSON_CreateString("OPP");
    profile2->valuestring = nullptr;
    cJSON_AddItemToArray(account, profile2);
    cJSON_AddItemToObject(obj, "100", account);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 1);
    EXPECT_EQ(denylist["100"].size(), 1);
    EXPECT_EQ(denylist["100"][0], "GATT");

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_011 end");}

/**
 * @tc.name: Edm_Mgr_test_ParseAccountDenylist_012
 * @tc.desc: Test ParseAccountDenylist with mixed case: partial valid data, partial invalid data
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ParseAccountDenylist_012, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_012 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    cJSON *obj = cJSON_CreateArray();

    // Valid account
    cJSON *account1 = cJSON_CreateArray();
    cJSON_AddItemToArray(account1, cJSON_CreateString("GATT"));
    cJSON_AddItemToArray(account1, cJSON_CreateString("OPP"));
    cJSON_AddItemToObject(obj, "100", account1);

    // Invalid account (null string)
    cJSON *account2 = cJSON_CreateArray();
    cJSON_AddItemToArray(account2, cJSON_CreateString("SPP"));
    cJSON_AddItemToArray(obj, account2);
    account2->string = nullptr;

    // Valid account with some invalid profiles
    cJSON *account3 = cJSON_CreateArray();
    cJSON_AddItemToArray(account3, cJSON_CreateString("GATT"));
    cJSON *invalidProfile = cJSON_CreateString("OPP");
    invalidProfile->valuestring = nullptr;
    cJSON_AddItemToArray(account3, invalidProfile);
    cJSON_AddItemToArray(account3, cJSON_CreateString("SPP"));
    cJSON_AddItemToObject(obj, "300", account3);

    int ret = mgr_.ParseAccountDenylist(obj, denylist);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(denylist.size(), 2);
    EXPECT_EQ(denylist.count("100"), 1);
    EXPECT_EQ(denylist["100"].size(), 2);
    EXPECT_EQ(denylist.count("300"), 1);
    EXPECT_EQ(denylist["300"].size(), 2);

    cJSON_Delete(obj);
    HILOGI("Edm_Mgr_test_ParseAccountDenylist_012 end");}

/**
 * @tc.name: Edm_Mgr_test_IsAccountConfigAllowedReceive_001
 * @tc.desc: Test IsAccountConfigAllowedReceive with OPP and GATT profiles in denylist
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsAccountConfigAllowedReceive_001, TestSize.Level1){
    HILOGI("Edm_Mgr_test_IsAccountConfigAllowedReceive_001 enter");
    mgr_.Init();
    std::vector<std::string> profiles;
    profiles.push_back("GATT");
    profiles.push_back("OPP");
    profiles.push_back("SPP");
    mgr_.accountRecvDenylist_["100"]= profiles;
    ControlInterceptMessage msg {
        .addr = "11:22:33:44:55:66",
        .isSystemHap = true,
        .isNativeCaller = true,
    };
    bool ret = mgr_.IsAccountConfigAllowedReceive(msg, "GATT");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAccountConfigAllowedReceive(msg, "OPP");
    EXPECT_FALSE(ret);
    ret = mgr_.IsAccountConfigAllowedReceive(msg, "SPP");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAccountConfigAllowedReceive(msg, "XXX");
    EXPECT_TRUE(ret);
    mgr_.accountRecvDenylist_.clear();
    ret = mgr_.IsAccountConfigAllowedReceive(msg, "OPP");
    EXPECT_TRUE(ret);
    HILOGI("Edm_Mgr_test_IsAccountConfigAllowedReceive_001 end");
}

/**
 * @tc.name: Edm_Mgr_test_IsAccountConfigAllowedReceive_002
 * @tc.desc: Test IsAccountConfigAllowedReceive with empty denylist
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsAccountConfigAllowedReceive_002, TestSize.Level1){
    HILOGI("Edm_Mgr_test_IsAccountConfigAllowedReceive_002 enter");
    mgr_.Init();
    mgr_.accountRecvDenylist_.clear();
    ControlInterceptMessage msg {
        .addr = "11:22:33:44:55:66",
        .isSystemHap = true,
        .isNativeCaller = true,
    };
    bool ret = mgr_.IsAccountConfigAllowedReceive(msg, "GATT");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAccountConfigAllowedReceive(msg, "OPP");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAccountConfigAllowedReceive(msg, "SPP");
    EXPECT_TRUE(ret);
    HILOGI("Edm_Mgr_test_IsAccountConfigAllowedReceive_002 end");
}

/**
 * @tc.name: Edm_Mgr_test_ReportEdmAuditInfo_001
 * @tc.desc: Test ReportEdmAuditInfo with disallowedList device and filePath
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_ReportEdmAuditInfo_001, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_ReportEdmAuditInfo_001 enter");
    mgr_.Init();
    mgr_.disallowedListMap_.clear();
    mgr_.realDisallowedAddrSet_.clear();
    // No converter is registered, so the value equals the key.
    mgr_.disallowedListMap_.emplace("11:22:33:44:55:66", "11:22:33:44:55:66");
    mgr_.realDisallowedAddrSet_.insert("11:22:33:44:55:66");
    bool ret = mgr_.IsAllowedConnect("11:22:33:44:55:66");
    EXPECT_FALSE(ret);
    mgr_.ReportEdmAuditInfo("11:22:33:44:55:66", 0, 0, "/data/test_opp.txt");
    ret = mgr_.IsAllowedConnect("FF:FF:FF:66:88:77");
    EXPECT_TRUE(ret);
    mgr_.disallowedListMap_.clear();
    mgr_.realDisallowedAddrSet_.clear();
    ret = mgr_.IsAllowedConnect("FF:FF:FF:66:88:77");
    EXPECT_TRUE(ret);
    HILOGI("Edm_Mgr_test_ReportEdmAuditInfo_001 end");
}

/**
 * @tc.name: Edm_Mgr_test_GetAccountDenylistInternal_001
 * @tc.desc: Test GetAccountDenylistInternal with empty protocolDenyList * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_GetAccountDenylistInternal_001, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_GetAccountDenylistInternal_001 enter");
    std::map<std::string, std::vector<std::string>> denylist;
    int ret = mgr_.GetAccountDenylistInternal(mgr_.accountDenylistMutexLock_, denylist, "ProtocolDenyList");
    EXPECT_EQ(ret, -1);
    HILOGI("Edm_Mgr_test_GetAccountDenylistInternal_001 end");
}
/**
 * @tc.name: Edm_Mgr_test_GetAccountDenylistInternal_002
 * @tc.desc: Test GetAccountDenylistInternal with invalid JSON content
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_GetAccountDenylistInternal_002, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_GetAccountDenylistInternal_002 enter");
    std::ofstream file("/data/service/el1/public/edm/config/system/all/bluetooth/config.json");
    file << "{invalid json content";
    file.close();

    std::map<std::string, std::vector<std::string>> denylist;
    int ret = mgr_.GetAccountDenylistInternal(mgr_.accountDenylistMutexLock_, denylist, "ProtocolDenyList");
    EXPECT_EQ(ret, -1);

    std::remove("/data/service/el1/public/edm/config/system/all/bluetooth/config.json");
    HILOGI("Edm_Mgr_test_GetAccountDenylistInternal_002 end");
}

/**
 * @tc.name: Edm_Mgr_test_GetAccountDenylistInternal_003
 * @tc.desc: Test GetAccountDenylistInternal with valid JSON but missing key
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_GetAccountDenylistInternal_003, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_GetAccountDenylistInternal_003 enter");
    std::ofstream file("/data/service/el1/public/edm/config/system/all/bluetooth/config.json");
    file << "{\"OtherKey\": []}";
    file.close();

    std::map<std::string, std::vector<std::string>> denylist;
    int ret = mgr_.GetAccountDenylistInternal(mgr_.accountDenylistMutexLock_, denylist, "ProtocolDenyList");
    EXPECT_EQ(ret, -1);

    std::remove("/data/service/el1/public/edm/config/system/all/bluetooth/config.json");
    HILOGI("Edm_Mgr_test_GetAccountDenylistInternal_003 end");
}

/**
 * @tc.name: Edm_Mgr_test_GetAccountDenylistInternal_004
 * @tc.desc: Test GetAccountDenylistInternal with ParseAccountDenylist failure (accountSize exceeds limit)
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_GetAccountDenylistInternal_004, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_GetAccountDenylistInternal_004 enter");
    std::ofstream file("/data/service/el1/public/edm/config/system/all/bluetooth/config.json");
    file << "{\"ProtocolDenyList\": [";
    for (int i = 0; i < 1001; i++) {
        file << "[\"GATT\"],";
    }
    file.seekp(-1, std::ios::end);
    file << "]}";
    file.close();

    std::map<std::string, std::vector<std::string>> denylist;
    int ret = mgr_.GetAccountDenylistInternal(mgr_.accountDenylistMutexLock_, denylist, "ProtocolDenyList");
    EXPECT_EQ(ret, -1);

    std::remove("/data/service/el1/public/edm/config/system/all/bluetooth/config.json");
    HILOGI("Edm_Mgr_test_GetAccountDenylistInternal_004 end");
}

/**
 * @tc.name: Edm_Mgr_test_IsAllowedConnect_VirtualAddr_001
 * @tc.desc: Test allowedList/disallowedList control when virtual address is converted to real address at write time
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsAllowedConnect_VirtualAddr_001, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_IsAllowedConnect_VirtualAddr_001 enter");
    mgr_.Init();
    mgr_.allowedListMap_.clear();
    mgr_.realAllowedAddrSet_.clear();
    mgr_.disallowedListMap_.clear();
    mgr_.realDisallowedAddrSet_.clear();
    const std::string virtualAddr = "AA:BB:CC:DD:EE:FF";
    const std::string realAddr = "11:22:33:44:55:66";
    mgr_.RegisterRealAddrConvertFunc([virtualAddr, realAddr](const RawAddress &addr, RawAddress &outRealAddr) {
        if (addr.GetAddress() != virtualAddr) {
            return false;
        }
        outRealAddr.SetAddress(realAddr);
        return true;
    });

    // The key is the original address and the value is its converted real address.
    mgr_.allowedListMap_.emplace(virtualAddr, realAddr);
    mgr_.realAllowedAddrSet_.insert(realAddr);
    EXPECT_EQ(mgr_.allowedListMap_.count(virtualAddr), 1);
    EXPECT_EQ(mgr_.allowedListMap_.at(virtualAddr), realAddr);

    // allowedList mode: stored real address allows both virtual and real address
    bool ret = mgr_.IsAllowedConnect("AA:BB:CC:DD:EE:FF");
    EXPECT_TRUE(ret);
    ret = mgr_.IsAllowedConnect(realAddr);
    EXPECT_TRUE(ret);
    ret = mgr_.IsAllowedConnect("99:88:77:66:55:44");
    EXPECT_FALSE(ret);

    // disallowedList mode: stored real address restricts both virtual and real address
    mgr_.allowedListMap_.clear();
    mgr_.realAllowedAddrSet_.clear();
    mgr_.disallowedListMap_.emplace(virtualAddr, realAddr);
    mgr_.realDisallowedAddrSet_.insert(realAddr);
    EXPECT_EQ(mgr_.disallowedListMap_.count(virtualAddr), 1);
    EXPECT_EQ(mgr_.disallowedListMap_.at(virtualAddr), realAddr);
    ret = mgr_.IsAllowedConnect("AA:BB:CC:DD:EE:FF");
    EXPECT_FALSE(ret);
    ret = mgr_.IsAllowedConnect(realAddr);
    EXPECT_FALSE(ret);
    ret = mgr_.IsAllowedConnect("99:88:77:66:55:44");
    EXPECT_TRUE(ret);

    mgr_.disallowedListMap_.clear();
    mgr_.realDisallowedAddrSet_.clear();
    mgr_.RegisterRealAddrConvertFunc(nullptr);
    HILOGI("Edm_Mgr_test_IsAllowedConnect_VirtualAddr_001 end");
}

/**
 * @tc.name: Edm_Mgr_test_IsAllowedConnect_VirtualAddr_002
 * @tc.desc: Test converter registered after the list is populated does not retroactively convert stored addresses
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothEdmManagerTest, Edm_Mgr_test_IsAllowedConnect_VirtualAddr_002, TestSize.Level1)
{
    HILOGI("Edm_Mgr_test_IsAllowedConnect_VirtualAddr_002 enter");
    mgr_.Init();
    mgr_.allowedListMap_.clear();
    mgr_.realAllowedAddrSet_.clear();
    mgr_.disallowedListMap_.clear();
    mgr_.realDisallowedAddrSet_.clear();
    const std::string virtualAddr = "AA:BB:CC:DD:EE:FF";
    const std::string realAddr = "11:22:33:44:55:66";
    mgr_.RegisterRealAddrConvertFunc(nullptr);

    // No converter at load time: the value falls back to the original address.
    std::vector<std::string> addrList = {virtualAddr};
    mgr_.UpdateListMap(mgr_.allowedListMap_, mgr_.realAllowedAddrSet_, addrList);
    EXPECT_EQ(mgr_.allowedListMap_.at(virtualAddr), virtualAddr);
    bool ret = mgr_.IsAllowedConnect(virtualAddr);
    EXPECT_TRUE(ret);
    ret = mgr_.IsAllowedConnect(realAddr);
    EXPECT_FALSE(ret);

    // Converter registered later does not retroactively convert the already stored address.
    mgr_.RegisterRealAddrConvertFunc([virtualAddr, realAddr](const RawAddress &addr, RawAddress &outRealAddr) {
        if (addr.GetAddress() != virtualAddr) {
            return false;
        }
        outRealAddr.SetAddress(realAddr);
        return true;
    });
    EXPECT_EQ(mgr_.allowedListMap_.at(virtualAddr), virtualAddr);
    // Only the real address of the queried address is compared with the keys and the values. The
    // virtual address now converts to the real address, which is absent from both, so it is rejected.
    ret = mgr_.IsAllowedConnect(virtualAddr);
    EXPECT_FALSE(ret);
    // The real address matches neither the key nor the value, so it is rejected.
    ret = mgr_.IsAllowedConnect(realAddr);
    EXPECT_FALSE(ret);

    mgr_.allowedListMap_.clear();
    mgr_.realAllowedAddrSet_.clear();
    mgr_.RegisterRealAddrConvertFunc(nullptr);
    HILOGI("Edm_Mgr_test_IsAllowedConnect_VirtualAddr_002 end");
}

}  // namespace bluetooth
}  // namespace OHOS