/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bt_chr_pkgname_manager.h"
 
using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
class BtChrPkgNameManagerTest : public testing::Test {
public:
    BtChrPkgNameManagerTest() = default;
    ~BtChrPkgNameManagerTest() = default;
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {
        std::lock_guard<std::mutex> lock(BtChrPkgNameManager::GetInstance().renderPkgNameSetMutex_);
        BtChrPkgNameManager::GetInstance().renderPkgNameSet_.clear();
    };
    void TearDown() {
        std::lock_guard<std::mutex> lock(BtChrPkgNameManager::GetInstance().renderPkgNameSetMutex_);
        BtChrPkgNameManager::GetInstance().renderPkgNameSet_.clear();
    };
};

/*
 * @tc.name: Test_UpdateRenderPkgName_001
 * @tc.desc: Verify the UpdateRenderPkgName function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrPkgNameManagerTest, Test_UpdateRenderPkgName_001, TestSize.Level1)
{
    HILOGI("Test_UpdateRenderPkgName_001 enter");
    std::string mockPkgName = "com.example.callapp";
    bool isRendererRunning = true;
    BtChrBusinessType businessType = BUSINESS_TYPE_CALL;
    uint32_t streamId = 100016;
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, isRendererRunning, businessType, streamId);

    std::string result = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_CALL);
    EXPECT_EQ(result, mockPkgName);
    HILOGI("Test_UpdateRenderPkgName_001 end");
}

/*
 * @tc.name: Test_UpdateRenderPkgName_002
 * @tc.desc: Verify the UpdateRenderPkgName function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrPkgNameManagerTest, Test_UpdateRenderPkgName_002, TestSize.Level1)
{
    HILOGI("Test_UpdateRenderPkgName_002 enter");

    bool isRendererRunning = true;
    std::string mockPkgName = "com.example.mediaapp";
    BtChrBusinessType businessType = BUSINESS_TYPE_MEDIA;
    uint32_t streamId = 100016;

    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, isRendererRunning, businessType, streamId);
    std::string result1 = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_MEDIA);
    EXPECT_EQ(result1, mockPkgName);

    mockPkgName = "com.example.mediaapp2";
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, isRendererRunning, businessType, streamId);
    std::string result2 = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_MEDIA);
    EXPECT_EQ(result2, "com.example.mediaapp,com.example.mediaapp2");

    isRendererRunning = false;
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, isRendererRunning, businessType, streamId);
    std::string result3 = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_MEDIA);
    EXPECT_EQ(result3, "com.example.mediaapp");

    HILOGI("Test_UpdateRenderPkgName_002 end");
}

/*
 * @tc.name: Test_UpdateRenderPkgName_003
 * @tc.desc: Verify the UpdateRenderPkgName function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrPkgNameManagerTest, Test_UpdateRenderPkgName_003, TestSize.Level1)
{
    HILOGI("Test_UpdateRenderPkgName_003 enter");
    int clientUid = 1003;
    bool isRendererRunning = true;
    uint32_t streamId = 100016;
    std::string mockPkgName = "com.example.mediaapp";
    BtChrBusinessType businessType = BUSINESS_TYPE_MEDIA;
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, isRendererRunning, businessType, streamId);
    
    mockPkgName = "com.example.callapp";
    businessType = BUSINESS_TYPE_CALL;
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, isRendererRunning, businessType, streamId);
    
    mockPkgName = "com.example.mediaapp2";
    businessType = BUSINESS_TYPE_MEDIA;
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, isRendererRunning, businessType, streamId);
 
    isRendererRunning = false;
    mockPkgName = "com.example.mediaapp";
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, isRendererRunning, businessType, streamId);
 
    std::string result1 = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_MEDIA);
    EXPECT_EQ(result1, "com.example.mediaapp2");
    std::string result2 = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_CALL);
    EXPECT_EQ(result2, "com.example.callapp");
 
    HILOGI("Test_UpdateRenderPkgName_003 end");
}

/*
 * @tc.name: Test_UpdateRenderPkgName_004
 * @tc.desc: Verify the UpdateRenderPkgName function. one apkName to multiple streamIds
 * @tc.type: FUNC
 */
HWTEST_F(BtChrPkgNameManagerTest, Test_UpdateRenderPkgName_004, TestSize.Level1)
{
    HILOGI("Test_UpdateRenderPkgName_004 enter");
    uint32_t streamId1 = 100016;
    uint32_t streamId2 = 100017;
    std::string mockPkgName = "com.example.mediaapp";

    BtChrBusinessType businessType = BUSINESS_TYPE_MEDIA;
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, true, businessType, streamId1);
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, true, businessType, streamId2);

    std::string result = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_MEDIA);
    EXPECT_EQ(result, mockPkgName);
    
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, false, businessType, streamId1);
    result = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_MEDIA);
    EXPECT_EQ(result, mockPkgName);
 
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(mockPkgName, false, businessType, streamId2);
    result = BtChrPkgNameManager::GetInstance().GetRenderPkgName(BUSINESS_TYPE_MEDIA);
    EXPECT_EQ(result, "");
    HILOGI("Test_UpdateRenderPkgName_004 end");
}
 
}  // namespace bluetooth
}  // namespace OHOS