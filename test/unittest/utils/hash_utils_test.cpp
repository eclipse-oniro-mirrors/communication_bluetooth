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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bluetooth_log.h"
#include "hash_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
class HashUtilTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/**
 * @tc.number: HashUtilTest_EncodeString_001
 * @tc.name: EncodeString
 * @tc.desc: FUNC
 */
HWTEST_F(HashUtilTest, HashUtilTest_EncodeString_001, TestSize.Level0)
{
    HILOGI("HashUtilTest_EncodeString_001 enter");
    int hashAlgorithmType = 0;
    int unKnownHashType = -1;
    std::string virtualAddress = "11:22:33:44:55:66";
    std::string ret = HashUtil::EncodeString(hashAlgorithmType, virtualAddress);
    EXPECT_FALSE(ret.empty());
    ret = HashUtil::EncodeString(unKnownHashType, virtualAddress);
    EXPECT_TRUE(ret.empty());
    HILOGI("HashUtilTest_EncodeString_001 end");
}

/**
 * @tc.number: HashUtilTest_ConvertSha256_001
 * @tc.name: GetString
 * @tc.desc: FUNC
 */
HWTEST_F(HashUtilTest, HashUtilTest_ConvertSha256_001, TestSize.Level0)
{
    HILOGI("HashUtilTest_ConvertSha256_001 enter");
    std::string s1 = "11111";
    std::string ret = HashUtil::ConvertSha256(s1);
    EXPECT_FALSE(ret.empty());
    std::string s2;
    ret = HashUtil::ConvertSha256(s2);
    EXPECT_TRUE(ret.empty());
    HILOGI("HashUtilTest_ConvertSha256_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS