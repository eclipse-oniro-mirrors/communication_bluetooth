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

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bt_chr_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
static BluetoothHost *host_;
class BtChrUtilTest : public testing::Test {
public:
    BtChrUtilTest() = default;
    ~BtChrUtilTest() = default;

    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

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
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Test_GetEncryptAddr_001
 * @tc.desc: Verify the GetEncryptAddr function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUtilTest, Test_GetEncryptAddr_001, TestSize.Level1)
{
    HILOGI("Test_GetEncryptAddr_001 enter");
    std::string addr = "74:0a:12:d4:56:d5";
    std::string result = GetEncryptAddr(addr);
    EXPECT_STREQ(result.c_str(), "74:0a:**:**:**:d5");

    addr = "";
    result = GetEncryptAddr(addr);
    EXPECT_STREQ(result.c_str(), "");

    addr = "74:0a:12:d4:56:d5123";
    result = GetEncryptAddr(addr);
    EXPECT_STREQ(result.c_str(), "");
    HILOGI("Test_GetEncryptAddr_001 end");
}


/**
 * @tc.name: Test_GetEncryptEventKey_001
 * @tc.desc: Verify the GetEncryptEventKey function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUtilTest, Test_GetEncryptEventKey_001, TestSize.Level1)
{
    HILOGI("Test_GetEncryptEventKey_001 enter");
    std::string eventKey = "BT_USER_DISCONNECT&74:0a:12:d4:56:d5";
    std::string result = GetEncryptEventKey(eventKey);
    EXPECT_STREQ(result.c_str(), "BT_USER_DISCONNECT&74:0a:**:**:**:d5");

    eventKey = "";
    result = GetEncryptEventKey(eventKey);
    EXPECT_STREQ(result.c_str(), "");
    HILOGI("Test_GetEncryptEventKey_001 end");
}

/**
 * @tc.name: Test_GetCurrentTime_001
 * @tc.desc: Verify the GetCurrentTime function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUtilTest, Test_GetCurrentTime_001, TestSize.Level1)
{
    HILOGI("Test_GetCurrentTime_001 enter");
    EXPECT_NE(GetCurrentTime(), "");
    GetNowTimeMs();
    HILOGI("Test_GetCurrentTime_001 end");
}

/**
 * @tc.name: Test_GetCurrentTimeMs_001
 * @tc.desc: Verify the GetCurrentTimeMs function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUtilTest, Test_GetCurrentTimeMs_001, TestSize.Level1)
{
    HILOGI("Test_GetCurrentTimeMs_001 enter");
    std::string result = GetCurrentTimeMs();
    EXPECT_NE(result, "");
    ASSERT_EQ(result.length(), 23u);
    EXPECT_EQ(result[19], '.');
    HILOGI("GetCurrentTimeMs result: %{public}s", result.c_str());
    HILOGI("Test_GetCurrentTimeMs_001 end");
}

/**
 * @tc.name: Test_GetLowercaseAddr_001
 * @tc.desc: Verify the GetNowTimeMs function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrUtilTest, Test_GetLowercaseAddr_001, TestSize.Level1)
{
    HILOGI("Test_GetLowercaseAddr_001 enter");
    std::string result = GetLowercaseAddr("74:0A:12:D4:56:D5");
    EXPECT_STREQ(result.c_str(), "74:0a:12:d4:56:d5");
    HILOGI("Test_GetLowercaseAddr_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS