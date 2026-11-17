/*
 * Copyright (c) 2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "permission_manager.h"
#include "ipc_skeleton.h"
#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Security::AccessToken;

ATokenTypeEnum AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_HAP;
HapTokenInfo AccessTokenKit::hapTokenInfo_;
int AccessTokenKit::accessTokenKitRet_ = -1;
bool TokenIdKit::isSystemApp_ = true;
std::string g_appIdMock = "";

namespace OHOS {
namespace Bluetooth {
using namespace OHOS;
using namespace Security::AccessToken;

std::string GetHapAppIdentifier(int32_t userId, const std::string &bundleName)
{
    return g_appIdMock;
}

class PermissionManagerIsUserRealAddrTest : public testing::Test {
public:
    PermissionManagerIsUserRealAddrTest() = default;
    ~PermissionManagerIsUserRealAddrTest() = default;

    static void SetUpTestSuite(void) {};
    static void TearDownTestSuite(void) {};
    void SetUp()
    {
        HILOGI("SetUpTestCase start");
        HILOGI("SetUpTestCase end");
    };
    void TearDown() {};
};

/**
 * @tc.name: Permission_Manager_IsUseRealAddr_Test__000
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerIsUserRealAddrTest, Permission_Manager_IsUseRealAddr_Test_001, TestSize.Level0)
{
    HILOGI("Permission_Manager_IsUseRealAddr_Test__000 start");
    AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_NATIVE;
    EXPECT_TRUE(PermissionManager::IsUseRealAddr(0));
    AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_SHELL;
    EXPECT_TRUE(PermissionManager::IsUseRealAddr(0));
    AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_HAP;
    EXPECT_TRUE(PermissionManager::IsUseRealAddr(0));
    TokenIdKit::isSystemApp_ = false;
    AccessTokenKit::hapTokenInfo_.apiVersion = 9;
    EXPECT_TRUE(PermissionManager::IsUseRealAddr(0));
    AccessTokenKit::hapTokenInfo_.apiVersion = 11;
    EXPECT_FALSE(PermissionManager::IsUseRealAddr(0));
    AccessTokenKit::hapTokenInfo_.apiVersion = 12;
    EXPECT_FALSE(PermissionManager::IsUseRealAddr(0));
    AccessTokenKit::accessTokenKitRet_ = 0;
    EXPECT_TRUE(PermissionManager::IsUseRealAddr(0));
    HILOGI("Permission_Manager_IsUseRealAddr_Test__000 end");
}

/**
 * @tc.name: Permission_Manager_IsNeededDiscarded_Test__000
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerIsUserRealAddrTest, Permission_Manager_IsNeededDiscarded_Test_001, TestSize.Level0)
{
    HILOGI("Permission_Manager_IsNeededDiscarded_Test__000 start");
    AccessTokenKit::hapTokenInfo_.apiVersion = 9;
    EXPECT_FALSE(PermissionManager::IsNeededDiscarded(ONLY_THIRDLY_HAP, 10));
    EXPECT_FALSE(PermissionManager::IsNeededDiscarded(SYSTEM_AND_THIRDLY_HAP, 10));
    AccessTokenKit::hapTokenInfo_.apiVersion = 12;
    TokenIdKit::isSystemApp_ = false;
    EXPECT_TRUE(PermissionManager::IsNeededDiscarded(ONLY_THIRDLY_HAP, 10));
    TokenIdKit::isSystemApp_ = true;
    EXPECT_FALSE(PermissionManager::IsNeededDiscarded(ONLY_THIRDLY_HAP, 10));
    EXPECT_TRUE(PermissionManager::IsNeededDiscarded(SYSTEM_AND_THIRDLY_HAP, 10));
    HILOGI("Permission_Manager_IsNeededDiscarded_Test__000 end");
}

/**
 * @tc.name: Permission_Manager_ChrReportSystemAppInfo_Test__000
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerIsUserRealAddrTest, Permission_Manager_ChrReportSystemAppInfo_Test__000, TestSize.Level0)
{
    HILOGI("Permission_Manager_ChrReportSystemAppInfo_Test__000 start");
    AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_HAP;
    PermissionManager::IsUseRealAddr(0);
    // ChrReportSystemAppInfo have the tokenId, no need to insert.
    PermissionManager::IsUseRealAddr(0);
    HILOGI("Permission_Manager_ChrReportSystemAppInfo_Test__000 end");
}

/**
 * @tc.name: Permission_Manager_IsInRealAddrTmpWhiteList_Test__001
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerIsUserRealAddrTest, Permission_Manager_IsInRealAddrTmpWhiteList_Test__001, TestSize.Level0)
{
    HILOGI("Permission_Manager_IsInRealAddrTmpWhiteList_Test__001 start");
    AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::accessTokenKitRet_ = -1;
    AccessTokenKit::hapTokenInfo_.apiVersion = 12;

    TokenIdKit::isSystemApp_ = false;
    AccessTokenKit::hapTokenInfo_.bundleName = "xxxx";
    EXPECT_FALSE(PermissionManager::IsUseRealAddr(0));

    TokenIdKit::isSystemApp_ = false;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.health";
    EXPECT_FALSE(PermissionManager::IsUseRealAddr(0));

    TokenIdKit::isSystemApp_ = true;
    AccessTokenKit::hapTokenInfo_.bundleName = "xxxx";
    EXPECT_FALSE(PermissionManager::IsUseRealAddr(0));

    TokenIdKit::isSystemApp_ = true;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.health";
    EXPECT_FALSE(PermissionManager::IsUseRealAddr(0));

    TokenIdKit::isSystemApp_ = true;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.health";
    g_appIdMock = "xxxx";
    EXPECT_FALSE(PermissionManager::IsUseRealAddr(0));

    TokenIdKit::isSystemApp_ = true;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.health";
    g_appIdMock = "1167174791690463936";
    EXPECT_FALSE(PermissionManager::IsUseRealAddr(0));

    HILOGI("Permission_Manager_IsInRealAddrTmpWhiteList_Test__001 end");
}

#ifdef MMI_NOT_DISCOVER_BLE
/**
 * @tc.name: Permission_Manager_IsInMmiTmpWhiteList_Test__001
 * @tc.desc: PublishEvent
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerIsUserRealAddrTest, Permission_Manager_IsInMmiTmpWhiteList_Test__001, TestSize.Level0)
{
    HILOGI("Permission_Manager_IsInMmiTmpWhiteList_Test__001 start");
    AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::accessTokenKitRet_ = -1;

    TokenIdKit::isSystemApp_ = false;
    AccessTokenKit::hapTokenInfo_.bundleName = "xxxx";
    EXPECT_FALSE(PermissionManager::IsInMmiTmpWhiteList(0));

    TokenIdKit::isSystemApp_ = false;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.mmitest";
    EXPECT_FALSE(PermissionManager::IsInMmiTmpWhiteList(0));

    TokenIdKit::isSystemApp_ = true;
    AccessTokenKit::hapTokenInfo_.bundleName = "xxxx";
    EXPECT_FALSE(PermissionManager::IsInMmiTmpWhiteList(0));

    TokenIdKit::isSystemApp_ = true;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.mmitest";
    EXPECT_FALSE(PermissionManager::IsInMmiTmpWhiteList(0));

    TokenIdKit::isSystemApp_ = true;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.mmitest";
    g_appIdMock = "xxxx";
    EXPECT_FALSE(PermissionManager::IsInMmiTmpWhiteList(0));

    TokenIdKit::isSystemApp_ = true;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.mmitest";
    g_appIdMock = "5765880207855265337";
    EXPECT_TRUE(PermissionManager::IsInMmiTmpWhiteList(0));

    HILOGI("Permission_Manager_IsInMmiTmpWhiteList_Test__001 end");
}
#endif

/**
 * @tc.name: Permission_Manager_GetCallingUserId_Test_001
 * @tc.desc: verify GetCallingUserId for hap caller, non-hap invalid uid and multi-user uid
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerIsUserRealAddrTest, Permission_Manager_GetCallingUserId_Test_001, TestSize.Level0)
{
    HILOGI("Permission_Manager_GetCallingUserId_Test_001 start");
    // hap caller: the hap user id is used to route the pair dialog
    AccessTokenKit::hapTokenInfo_.userID = 102;
    EXPECT_EQ(102, PermissionManager::GetCallingUserId());
    AccessTokenKit::hapTokenInfo_.userID = 0;

    // non-hap caller with invalid uid: no valid routing target
    AccessTokenKit::hapTokenInfoRet_ = RET_FAILED;
    IPCSkeleton::callingUid_ = 0;
    EXPECT_EQ(INVALID_USER_ID, PermissionManager::GetCallingUserId());

    // non-hap caller with multi-user uid: uid = 102 * BASE_USER_RANGE + 100 -> user 102
    IPCSkeleton::callingUid_ = 102 * 200000 + 100;
    EXPECT_EQ(102, PermissionManager::GetCallingUserId());

    // restore stub defaults
    IPCSkeleton::callingUid_ = 0;
    AccessTokenKit::hapTokenInfoRet_ = RET_SUCCESS;
    HILOGI("Permission_Manager_GetCallingUserId_Test_001 end");
}

/**
 * @tc.name: Permission_Manager_GetCallingName_Cache_Test_001
 * @tc.desc: verify GetCallingName hap branch inserts bundleName into cache and repeated calls hit cache,
 *           failed lookup does not insert into cache
 * @tc.type: FUNC
 */
HWTEST_F(PermissionManagerIsUserRealAddrTest, Permission_Manager_GetCallingName_Cache_Test_001, TestSize.Level0)
{
    HILOGI("Permission_Manager_GetCallingName_Cache_Test_001 start");
    PermissionManager::callingNameCache_.Clear();
    AccessTokenKit::hapTokenInfoRet_ = RET_SUCCESS;
    constexpr uint32_t hapTokenId = 123;
    AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::hapTokenInfo_.bundleName = "com.example.hap";
    EXPECT_EQ(static_cast<size_t>(0), PermissionManager::callingNameCache_.Size());
    EXPECT_STREQ("com.example.hap", PermissionManager::GetCallingName(hapTokenId).c_str());
    EXPECT_EQ(static_cast<size_t>(1), PermissionManager::callingNameCache_.Size());
    std::string cachedName = "";
    EXPECT_TRUE(PermissionManager::callingNameCache_.Find(hapTokenId, cachedName));
    EXPECT_STREQ("com.example.hap", cachedName.c_str());
    EXPECT_STREQ("com.example.hap", PermissionManager::GetCallingName(hapTokenId).c_str());
    EXPECT_EQ(static_cast<size_t>(1), PermissionManager::callingNameCache_.Size());

    // hap info query failed: no insert into cache
    AccessTokenKit::hapTokenInfoRet_ = RET_FAILED;
    EXPECT_STREQ("", PermissionManager::GetCallingName(hapTokenId + 1).c_str());
    EXPECT_EQ(static_cast<size_t>(1), PermissionManager::callingNameCache_.Size());

    // restore stub defaults
    AccessTokenKit::hapTokenInfoRet_ = RET_SUCCESS;
    AccessTokenKit::hapTokenInfo_.bundleName = "";
    AccessTokenKit::tokenType_ = ATokenTypeEnum::TOKEN_NATIVE;
    PermissionManager::callingNameCache_.Clear();
    HILOGI("Permission_Manager_GetCallingName_Cache_Test_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS