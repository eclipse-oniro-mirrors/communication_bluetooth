/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "permission_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
class PermissionManagerTest : public testing::Test {
public:
    PermissionManagerTest() = default;
    ~PermissionManagerTest() = default;

    static void SetUpTestSuite(void) {};
    static void TearDownTestSuite(void) {};
    void SetUp()
    {
        HILOGI("SetUpTestCase start");
        BluetoothAccessTokenMock::SetNativeTokenInfo();
        PermissionManager::callingNameCache_.Clear();
        HILOGI("SetUpTestCase end");
    };
    void TearDown()
    {
        PermissionManager::callingNameCache_.Clear();
    };
};

/**
 * @tc.name: permission_verifypermission_001
 * @tc.desc: verify VerifyPermission
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_verifypermission_001, TestSize.Level1)
{
    HILOGI("permission_verifypermission_001 enter");
    EXPECT_TRUE(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH));
    EXPECT_FALSE(PermissionManager::VerifyPermission(DISCOVER_BLUETOOTH));
    EXPECT_TRUE(PermissionManager::VerifyPermission(MANAGE_BLUETOOTH));
    EXPECT_FALSE(PermissionManager::VerifyPermission(LOCATION));
    EXPECT_FALSE(PermissionManager::VerifyPermission(APPROXIMATELY_LOCATION));
    EXPECT_TRUE(PermissionManager::VerifyPermission(GET_BLUETOOTH_LOCAL_MAC));
    HILOGI("permission_verifypermission_001 end");
}

/**
 * @tc.name: permission_verifypermission_002
 * @tc.desc: verify VerifyPermission with tokenId
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_verifypermission_002, TestSize.Level0)
{
    HILOGI("permission_verifypermission_002 enter");
    uint32_t tokenId = 0;
    EXPECT_FALSE(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId));
    EXPECT_FALSE(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId, true));
    EXPECT_FALSE(PermissionManager::VerifyPermission(DISCOVER_BLUETOOTH, tokenId));
    EXPECT_FALSE(PermissionManager::VerifyPermission(MANAGE_BLUETOOTH, tokenId));
    EXPECT_FALSE(PermissionManager::VerifyPermission(LOCATION, tokenId));
    EXPECT_FALSE(PermissionManager::VerifyPermission(APPROXIMATELY_LOCATION, tokenId));
    EXPECT_FALSE(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId));
    EXPECT_FALSE(PermissionManager::VerifyPermission(GET_BLUETOOTH_LOCAL_MAC, tokenId));
    HILOGI("permission_verifypermission_002 end");
}

/**
 * @tc.name: permission_ispermissionsgranted_001
 * @tc.desc: verify IsPermissionsGranted
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_ispermissionsgranted_001, TestSize.Level1)
{
    HILOGI("permission_ispermissionsgranted_001 enter");
    std::set<std::string> permissions;
    permissions.insert(ACCESS_BLUETOOTH);
    permissions.insert(MANAGE_BLUETOOTH);
    EXPECT_TRUE(PermissionManager::IsPermissionsGranted(permissions));
    HILOGI("permission_ispermissionsgranted_001 end");
}

/**
 * @tc.name: permission_verifymultipermissionsd_001
 * @tc.desc: verify VerifyMultiPermissions
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_verifymultipermissionsd_001, TestSize.Level1)
{
    HILOGI("permission_verifymultipermissionsd_001 enter");
    auto item = PermissionManager::CreateItem(true, {}, {ACCESS_BLUETOOTH});
    EXPECT_EQ(BT_NO_ERROR, PermissionManager::VerifyMultiPermissions(item));
    HILOGI("permission_verifymultipermissionsd_001 end");
}

/**
 * @tc.name: permission_getapiversion_001
 * @tc.desc: verify GetApiVersion
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getapiversion_001, TestSize.Level0)
{
    HILOGI("permission_getapiversion_001 enter");
    EXPECT_EQ(API_VERSION_INVALID, PermissionManager::GetApiVersion());
    uint32_t tokenId = 0;
    EXPECT_EQ(API_VERSION_INVALID, PermissionManager::GetApiVersion(tokenId));
    HILOGI("permission_getapiversion_001 end");
}

/**
 * @tc.name: permission_getcallingName_001
 * @tc.desc: verify GetCallingName
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getcallingName_001, TestSize.Level1)
{
    HILOGI("permission_getcallingName_001 enter");
    EXPECT_STRNE("", PermissionManager::GetCallingName().c_str());
    uint32_t tokenId = 0;
    EXPECT_STREQ("", PermissionManager::GetCallingName(tokenId).c_str());
    HILOGI("permission_getcallingName_001 end");
}

/**
 * @tc.name: permission_getcallingName_002
 * @tc.desc: verify GetCallingName returns cached value on cache hit, without querying AccessTokenKit
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getcallingName_002, TestSize.Level1)
{
    HILOGI("permission_getcallingName_002 enter");
    constexpr uint32_t fakeTokenId = 0xFFFFFFFF;
    const std::string cachedName = "com.example.cache.hit";
    PermissionManager::callingNameCache_.EnsureInsert(fakeTokenId, cachedName);
    EXPECT_STREQ(cachedName.c_str(), PermissionManager::GetCallingName(fakeTokenId).c_str());
    EXPECT_EQ(static_cast<size_t>(1), PermissionManager::callingNameCache_.Size());
    HILOGI("permission_getcallingName_002 end");
}

/**
 * @tc.name: permission_getcallingName_003
 * @tc.desc: verify failed lookup on invalid tokenId does not pollute cache, including on retry
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getcallingName_003, TestSize.Level1)
{
    HILOGI("permission_getcallingName_003 enter");
    uint32_t tokenId = 0;
    EXPECT_STREQ("", PermissionManager::GetCallingName(tokenId).c_str());
    EXPECT_STREQ("", PermissionManager::GetCallingName(tokenId).c_str());
    EXPECT_EQ(static_cast<size_t>(0), PermissionManager::callingNameCache_.Size());
    std::string cachedName = "not_empty";
    EXPECT_FALSE(PermissionManager::callingNameCache_.Find(tokenId, cachedName));
    EXPECT_STREQ("not_empty", cachedName.c_str());
    HILOGI("permission_getcallingName_003 end");
}

/**
 * @tc.name: permission_getcallingName_004
 * @tc.desc: verify repeated calls for the same native caller hit cache and return consistent name
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getcallingName_004, TestSize.Level1)
{
    HILOGI("permission_getcallingName_004 enter");
    std::string firstName = PermissionManager::GetCallingName();
    EXPECT_STREQ("bluetooth_unittest", firstName.c_str());
    std::string secondName = PermissionManager::GetCallingName();
    EXPECT_STREQ(firstName.c_str(), secondName.c_str());
    EXPECT_EQ(static_cast<size_t>(1), PermissionManager::callingNameCache_.Size());
    HILOGI("permission_getcallingName_004 end");
}

/**
 * @tc.name: permission_getcallingName_005
 * @tc.desc: verify cache is cleared when size exceeds MAX_CALLING_NAME_CACHE_SIZE,
 *           and the new caller name is re-queried and re-inserted
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getcallingName_005, TestSize.Level1)
{
    HILOGI("permission_getcallingName_005 enter");
    constexpr uint32_t dummyTokenBase = 0x20000000;
    constexpr size_t overCount = 5;
    for (size_t i = 0; i < MAX_CALLING_NAME_CACHE_SIZE + overCount; i++) {
        PermissionManager::callingNameCache_.EnsureInsert(
            dummyTokenBase + static_cast<uint32_t>(i), "dummy" + std::to_string(i));
    }
    EXPECT_EQ(MAX_CALLING_NAME_CACHE_SIZE + overCount, PermissionManager::callingNameCache_.Size());
    uint32_t tokenId = 0;
    EXPECT_STREQ("", PermissionManager::GetCallingName(tokenId).c_str());
    EXPECT_EQ(static_cast<size_t>(0), PermissionManager::callingNameCache_.Size());
    std::string staleName;
    EXPECT_FALSE(PermissionManager::callingNameCache_.Find(dummyTokenBase, staleName));
    std::string firstName = PermissionManager::GetCallingName();
    EXPECT_STREQ("bluetooth_unittest", firstName.c_str());
    std::string secondName = PermissionManager::GetCallingName();
    EXPECT_STREQ(firstName.c_str(), secondName.c_str());
    EXPECT_EQ(static_cast<size_t>(1), PermissionManager::callingNameCache_.Size());
    HILOGI("permission_getcallingName_005 end");
}

/**
 * @tc.name: permission_getcallingName_006
 * @tc.desc: verify cache is not cleared when size reaches exactly MAX_CALLING_NAME_CACHE_SIZE
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getcallingName_006, TestSize.Level1)
{
    HILOGI("permission_getcallingName_006 enter");
    constexpr uint32_t dummyTokenBase = 0x20000000;
    for (size_t i = 0; i < MAX_CALLING_NAME_CACHE_SIZE; i++) {
        PermissionManager::callingNameCache_.EnsureInsert(
            dummyTokenBase + static_cast<uint32_t>(i), "dummy" + std::to_string(i));
    }
    EXPECT_EQ(MAX_CALLING_NAME_CACHE_SIZE, PermissionManager::callingNameCache_.Size());
    uint32_t tokenId = 0;
    EXPECT_STREQ("", PermissionManager::GetCallingName(tokenId).c_str());
    EXPECT_EQ(MAX_CALLING_NAME_CACHE_SIZE, PermissionManager::callingNameCache_.Size());
    HILOGI("permission_getcallingName_006 end");
}

/**
 * @tc.name: permission_getcallinguserid_001
 * @tc.desc: verify GetCallingUserId returns INVALID_USER_ID for native caller,
 *           which means no multi-user routing target for the pair dialog
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getcallinguserid_001, TestSize.Level0)
{
    HILOGI("permission_getcallinguserid_001 enter");
    EXPECT_EQ(INVALID_USER_ID, PermissionManager::GetCallingUserId());
    HILOGI("permission_getcallinguserid_001 end");
}

/**
 * @tc.name: permission_ishapcaller_001
 * @tc.desc: verify IsHapCaller
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_ishapcaller_001, TestSize.Level0)
{
    HILOGI("permission_ishapcaller_001 enter");
    EXPECT_FALSE(PermissionManager::IsHapCaller());
    uint32_t tokenId = 0;
    EXPECT_FALSE(PermissionManager::IsHapCaller(tokenId));
    HILOGI("permission_ishapcaller_001 end");
}

/**
 * @tc.name: permission_isnativecaller_001
 * @tc.desc: verify IsNativeCaller
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_isnativecaller_001, TestSize.Level0)
{
    HILOGI("permission_isnativecaller_001 enter");
    EXPECT_TRUE(PermissionManager::IsNativeCaller());
    HILOGI("permission_isnativecaller_001 end");
}

/**
 * @tc.name: permission_issystemhap_001
 * @tc.desc: verify IsSystemHap
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_issystemhap_001, TestSize.Level0)
{
    HILOGI("permission_issystemhap_001 enter");
    EXPECT_TRUE(PermissionManager::IsSystemHap());
    HILOGI("permission_issystemhap_001 end");
}

/**
 * @tc.name: permission_isuserealAddr_001
 * @tc.desc: verify IsUseRealAddr
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_isuserealAddr_001, TestSize.Level0)
{
    HILOGI("permission_isuserealAddr_001 enter");
    EXPECT_TRUE(PermissionManager::IsUseRealAddr());
    HILOGI("permission_isuserealAddr_001 end");
}

/**
 * @tc.name: permission_getcallingapptlabel_001
 * @tc.desc: verify GetCallingAppLabel fallback with empty bundleName and no proxy
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerTest, permission_getcallingapptlabel_001, TestSize.Level1)
{
    HILOGI("permission_getcallingapptlabel_001 enter");
    EXPECT_STREQ("", PermissionManager::GetCallingAppLabel("", 100).c_str());
    EXPECT_STREQ("", PermissionManager::GetCallingAppLabel("com.bluetoothdemo", 100).c_str());
    HILOGI("permission_getcallingapptlabel_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS