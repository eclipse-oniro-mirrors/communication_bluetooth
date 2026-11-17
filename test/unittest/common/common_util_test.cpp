/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "common_util.h"
#include "bluetooth_code_convert_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;

class CommonUtilTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: CommonUtilTest_UnitTest_GenerateRandomNum_001
 * @tc.name: GenerateRandomNum
 * @tc.desc: could not found the type, return false
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_GenerateRandomNum_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_GenerateRandomNum_001 start");
    int beginNum = 1;
    int endNum = 100;
    int randomNum1 = GenerateRandomNum(beginNum, endNum);
    EXPECT_LE(randomNum1, endNum);
    EXPECT_GE(randomNum1, beginNum);
    EXPECT_EQ(GenerateRandomNum(endNum, beginNum), 0);
    beginNum = -100;
    endNum = -1;
    randomNum1 = GenerateRandomNum(beginNum, endNum);
    EXPECT_LE(randomNum1, endNum);
    EXPECT_GE(randomNum1, beginNum);
    HILOGI("CommonUtilTest_UnitTest_GenerateRandomNum_001 end");
}


/*
 * @tc.number: CommonUtilTest_UnitTest_IsValidAddr_001
 * @tc.name: IsValidAddr
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_IsValidAddr_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_IsValidAddr_001 start");
    std::string addr = "";
    EXPECT_FALSE(IsValidAddr(addr));

    addr = "00:00:00:00:00:00";
    EXPECT_FALSE(IsValidAddr(addr));

    addr = "08:F4:58:65:91:09";
    EXPECT_TRUE(IsValidAddr(addr));

    addr = "0!:F4:58:65:91:09";
    EXPECT_FALSE(IsValidAddr(addr));
    HILOGI("CommonUtilTest_UnitTest_IsValidAddr_001 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_IsValidAddr_002
 * @tc.name: IsValidAddr
 * @tc.desc:
 */
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_IsValidAddr_002, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_IsValidAddr_002 start");
    EXPECT_TRUE(IsValidAddr("08:f4:58:65:91:09"));
    EXPECT_TRUE(IsValidAddr("AA:BB:CC:DD:EE:FF"));
    EXPECT_TRUE(IsValidAddr("00:1a:2b:3c:4d:5e"));

    EXPECT_FALSE(IsValidAddr("08:F4:58"));
    EXPECT_FALSE(IsValidAddr("08:F4:58:65:91:09:AA"));
    EXPECT_FALSE(IsValidAddr("08F458659109"));
    EXPECT_FALSE(IsValidAddr("08-F4-58-65-91-09"));
    EXPECT_FALSE(IsValidAddr("08::F4:58:65:91:09"));
    EXPECT_FALSE(IsValidAddr(" 08:F4:58:65:91:09"));
    EXPECT_FALSE(IsValidAddr("08:F4:58:65:91:09 "));
    HILOGI("CommonUtilTest_UnitTest_IsValidAddr_002 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_IsNumber_001
 * @tc.name: IsNumber
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_IsNumber_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_IsNumber_001 start");
    EXPECT_TRUE(IsNumber("12"));

    EXPECT_FALSE(IsNumber("abc1"));
    EXPECT_FALSE(IsNumber(""));
    EXPECT_FALSE(IsNumber("12c"));
    HILOGI("CommonUtilTest_UnitTest_IsNumber_001 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_AsciiToUtf8_001
 * @tc.name: AsciiToUtf8
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_AsciiToUtf8_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_AsciiToUtf8_001 start");
    std::string str = "abc";
    std::string str1 = BluetoothCodeConvertUtils::AsciiToUtf8("abc");
    EXPECT_EQ(str.compare(str1), 0);
    HILOGI("CommonUtilTest_UnitTest_AsciiToUtf8_001 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_AsciiToUtf8_002
 * @tc.name: AsciiToUtf8
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_AsciiToUtf8_002, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_AsciiToUtf8_002 start");
    std::string str = "";
    std::string str1 = BluetoothCodeConvertUtils::AsciiToUtf8("");
    EXPECT_EQ(str.compare(str1), 0);
    HILOGI("CommonUtilTest_UnitTest_AsciiToUtf8_002 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_AsciiAndUtf8MixedToUtf8_001
 * @tc.name: AsciiAndUtf8MixedToUtf8
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_AsciiAndUtf8MixedToUtf8_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_AsciiAndUtf8MixedToUtf8_001 start");
    // 测试用例1：正常情况，输入为有效的UTF-8字符串
    EXPECT_EQ(BluetoothCodeConvertUtils::AsciiAndUtf8MixedToUtf8("Hello, 世界!"), "Hello, 世界!");

    // 测试用例2：空字符串
    EXPECT_EQ(BluetoothCodeConvertUtils::AsciiAndUtf8MixedToUtf8(""), "");

    // 测试用例3：全ASCII字符串
    EXPECT_EQ(BluetoothCodeConvertUtils::AsciiAndUtf8MixedToUtf8("Hello, World!"), "Hello, World!");

    // 测试用例4：边界情况，输入刚好达到处理能力
    string input = string(1024, 'a'); // 假设处理能力为1024字节
    EXPECT_EQ(BluetoothCodeConvertUtils::AsciiAndUtf8MixedToUtf8(input), string(1024, 'a'));
    HILOGI("CommonUtilTest_UnitTest_AsciiAndUtf8MixedToUtf8_001 end");
}


/*
 * @tc.number: CommonUtilTest_UnitTest_IsUtf8_001
 * @tc.name: IsUtf8
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_IsUtf8_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_IsUtf8_001 start");
    EXPECT_TRUE(BluetoothCodeConvertUtils::IsUtf8("abc"));
    HILOGI("CommonUtilTest_UnitTest_IsUtf8_001 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_IsUtf8Char_001
 * @tc.name: IsUtf8Char
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_IsUtf8Char_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_IsUtf8Char_001 start");
    int32_t nBytes = 0;
    EXPECT_TRUE(BluetoothCodeConvertUtils::IsUtf8Char('a', nBytes));
    HILOGI("CommonUtilTest_UnitTest_IsUtf8Char_001 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_IsUtf8Char_002
 * @tc.name: IsUtf8Char
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_IsUtf8Char_002, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_IsUtf8Char_002 start");
    int32_t nBytes = 2;
    EXPECT_FALSE(BluetoothCodeConvertUtils::IsUtf8Char('B', nBytes));
    HILOGI("CommonUtilTest_UnitTest_IsUtf8Char_002 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_TruncateString_001
 * @tc.name: TruncateString
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_TruncateString_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_TruncateString_001 start");
    std::string longString = "aa";
    EXPECT_EQ(TruncateString(longString, 4), "aa");
    longString = "aéaaaaaaaaa";
    EXPECT_EQ(TruncateString(longString, 6), "aé..."); //é为2字节
    longString = "a一aaaaaaaaa";
    EXPECT_EQ(TruncateString(longString, 7), "a一..."); //一为3字节
    longString = "a𐀀aaaaaaaaa";
    EXPECT_EQ(TruncateString(longString, 8), "a𐀀..."); //𐀀为4字节
    longString = "a𐀀éaaaaaaaa";
    EXPECT_EQ(TruncateString(longString, 9), "a𐀀..."); //𐀀为4字节
    HILOGI("CommonUtilTest_UnitTest_TruncateString_001 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_GetEncryptDeviceName_001
 * @tc.name: GetEncryptDeviceName
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_GetEncryptDeviceName_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_GetEncryptDeviceName_001 start");
    std::string deviceName = "";
    EXPECT_EQ(GetEncryptDeviceName(deviceName), "");
    deviceName = "a";
    EXPECT_EQ(GetEncryptDeviceName(deviceName), "*");
    deviceName = "ab";
    EXPECT_EQ(GetEncryptDeviceName(deviceName), "**");
    deviceName = "abc";
    EXPECT_EQ(GetEncryptDeviceName(deviceName), "a*c");
    HILOGI("CommonUtilTest_UnitTest_GetEncryptDeviceName_001 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_Convert_001
 * @tc.name: Convert
 * @tc.desc:
*/
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_Convert_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_Convert_001 start");
    // 测试用例1：正常情况
    string input = "测试";
    string fromCharset = "UTF-8";
    string toCharset = "GBK";
    EXPECT_EQ(BluetoothCodeConvertUtils::Convert(input, fromCharset, toCharset), "\xB2\xE2\xCA\xD4");

    // 测试用例2：空字符串
    string emptyInput = "";
    EXPECT_EQ(BluetoothCodeConvertUtils::Convert(emptyInput, fromCharset, toCharset), "");

    // 测试用例3：无效字符集
    string invalidCharset = "invalidCharset";
    EXPECT_EQ(BluetoothCodeConvertUtils::Convert(input, invalidCharset, toCharset), "测试");
    HILOGI("CommonUtilTest_UnitTest_Convert_001 end");
}
/*
 * @tc.number: CommonUtilTest_UnitTest_IsValidUuid_001
 * @tc.name: IsValidUuid
 * @tc.desc: Test valid and invalid UUID strings
 */
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_IsValidUuid_001, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_IsValidUuid_001 start");
    EXPECT_TRUE(IsValidUuid("00000000-0000-1000-8000-00805F9B34FB"));
    EXPECT_TRUE(IsValidUuid("12345678-ABCD-EF00-1234-567890ABCDEF"));
    EXPECT_TRUE(IsValidUuid("abcdef01-2345-6789-abcd-ef0123456789"));
    HILOGI("CommonUtilTest_UnitTest_IsValidUuid_001 end");
}

/*
 * @tc.number: CommonUtilTest_UnitTest_IsValidUuid_002
 * @tc.name: IsValidUuid
 * @tc.desc: Test empty and invalid UUID strings
 */
HWTEST_F(CommonUtilTest, CommonUtilTest_UnitTest_IsValidUuid_002, TestSize.Level0)
{
    HILOGI("CommonUtilTest_UnitTest_IsValidUuid_002 start");
    EXPECT_FALSE(IsValidUuid(""));
    EXPECT_FALSE(IsValidUuid("00000000-0000-1000-8000"));
    EXPECT_FALSE(IsValidUuid("00000000-0000-1000-8000-00805F9B34FB-extra"));
    EXPECT_FALSE(IsValidUuid("0000000000001000800000805F9B34FB"));
    EXPECT_FALSE(IsValidUuid("g0000000-0000-1000-8000-00805F9B34FB"));
    EXPECT_FALSE(IsValidUuid("0000000-0000-1000-8000-00805F9B34FB"));
    EXPECT_FALSE(IsValidUuid(" 00000000-0000-1000-8000-00805F9B34FB"));
    EXPECT_FALSE(IsValidUuid("00000000-0000-1000-8000-00805F9B34FB "));
    HILOGI("CommonUtilTest_UnitTest_IsValidUuid_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS