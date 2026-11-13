/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <cstring>
#include "bluetooth_log.h"
#include "obex_header.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
ObexHeader obexHeader;

class ObexHeaderTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {
        obexHeader.obexHeaderMap_.clear();
    }
};

/*
 * @tc.number: ObexHeader_ParseHeaderValue_001
 * @tc.name: ParseHeaderValue
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_ParseHeaderValue_001, TestSize.Level1)
{
    HILOGI("ObexHeader_ParseHeaderValue_001 start");
    std::vector<uint8_t> headerValue;
    // headerValue empty
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_TRUE(obexHeader.obexHeaderMap_.empty());

    headerValue.push_back(0x80);
    headerValue.push_back(0x01);
    // parse BYTE
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_EQ(obexHeader.obexHeaderMap_.size(), 1);

    headerValue.clear();
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    headerValue.push_back(0x05);
    headerValue.push_back(0x00);
    headerValue.push_back(0x01);
    // parse UNICODE_TEXT
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_EQ(obexHeader.obexHeaderMap_.size(), 2);

    headerValue.clear();
    headerValue.push_back(0x78);
    headerValue.push_back(0x00);
    headerValue.push_back(0x06);
    headerValue.push_back(0x00);
    headerValue.push_back(0x01);
    headerValue.push_back(0x02);
    // parse BYTES
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_EQ(obexHeader.obexHeaderMap_.size(), 3);

    headerValue.clear();
    headerValue.push_back(0xCB);
    headerValue.push_back(0x00);
    headerValue.push_back(0x00);
    headerValue.push_back(0x00);
    headerValue.push_back(0x01);
    // parse WORD
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_EQ(obexHeader.obexHeaderMap_.size(), 4);
    HILOGI("ObexHeader_ParseHeaderValue_001 end");
}

/*
 * @tc.number: ObexHeader_ParseHeaderValue_002
 * @tc.name: ParseHeaderValue
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_ParseHeaderValue_002, TestSize.Level1)
{
    HILOGI("ObexHeader_ParseHeaderValue_002 start");
    std::vector<uint8_t> headerValue;
    headerValue.push_back(0x80);
    // ParseByteArray error
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_TRUE(obexHeader.obexHeaderMap_.empty());

    headerValue.clear();
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    // ParseBytes error1
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_TRUE(obexHeader.obexHeaderMap_.empty());

    headerValue.clear();
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    headerValue.push_back(0x02);
    // ParseBytes error2
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_TRUE(obexHeader.obexHeaderMap_.empty());

    headerValue.clear();
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    headerValue.push_back(0x03);
    // ParseBytes error3
    obexHeader.ParseHeaderValue(headerValue);
    EXPECT_TRUE(obexHeader.obexHeaderMap_.empty());
    HILOGI("ObexHeader_ParseHeaderValue_002 end");
}

/*
 * @tc.number: ObexHeader_SetHeader_001
 * @tc.name: SetHeader
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_SetHeader_001, TestSize.Level1)
{
    HILOGI("ObexHeader_SetHeader_001 start");
    std::vector<uint8_t> headerValue;
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    uint8_t headerId = 0x01;
    obexHeader.SetHeader(headerId, headerValue);
    EXPECT_EQ(obexHeader.obexHeaderMap_.size(), 1);
    std::vector<uint8_t> retValue = obexHeader.GetHeader(headerId);
    EXPECT_EQ(retValue.size(), 2);

    headerValue.push_back(0x02);
    obexHeader.SetHeader(headerId, headerValue);
    EXPECT_EQ(obexHeader.obexHeaderMap_.size(), 1);
    retValue = obexHeader.GetHeader(headerId);
    EXPECT_EQ(retValue.size(), 3);
    HILOGI("ObexHeader_SetHeader_001 end");
}

/*
 * @tc.number: ObexHeader_GenerateHeaderValue_001
 * @tc.name: GenerateHeaderValue
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_GenerateHeaderValue_001, TestSize.Level1)
{
    HILOGI("ObexHeader_GenerateHeaderValue_001 start");
    std::vector<uint8_t> retValue;
    obexHeader.GenerateHeaderValue(retValue);
    EXPECT_TRUE(retValue.empty());

    std::vector<uint8_t> headerValue;
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    obexHeader.SetHeader(CONNECTION_ID, headerValue);
    obexHeader.GenerateHeaderValue(retValue);
    EXPECT_EQ(retValue.size(), 3);
    HILOGI("ObexHeader_GenerateHeaderValue_001 end");
}

/*
 * @tc.number: ObexHeader_GenerateHeaderValue_002
 * @tc.name: GenerateHeaderValue
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_GenerateHeaderValue_002, TestSize.Level1)
{
    HILOGI("ObexHeader_GenerateHeaderValue_002 start");
    std::vector<uint8_t> retValue;
    std::vector<uint8_t> headerValue;
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    obexHeader.SetHeader(TARGET, headerValue);
    obexHeader.GenerateHeaderValue(retValue);
    EXPECT_EQ(retValue.size(), 5);

    retValue.clear();
    obexHeader.SetHeader(CONNECTION_ID, headerValue);
    obexHeader.GenerateHeaderValue(retValue);
    EXPECT_EQ(retValue.size(), 5);

    retValue.clear();
    obexHeader.SetHeader(COUNT, headerValue);
    obexHeader.GenerateHeaderValue(retValue);
    EXPECT_EQ(retValue.size(), 8);

    retValue.clear();
    obexHeader.SetHeader(TYPE, headerValue);
    obexHeader.GenerateHeaderValue(retValue);
    EXPECT_EQ(retValue.size(), 14);
    HILOGI("ObexHeader_GenerateHeaderValue_002 end");
}

/*
 * @tc.number: ObexHeader_FindHeaderEnd_001
 * @tc.name: FindHeaderEnd
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_FindHeaderEnd_001, TestSize.Level1)
{
    HILOGI("ObexHeader_FindHeaderEnd_001 start");
    std::vector<uint8_t> headerValue;
    uint32_t result = obexHeader.FindHeaderEnd(headerValue, 0, 10);
    EXPECT_EQ(result, 0);

    headerValue.push_back(0x78);
    result = obexHeader.FindHeaderEnd(headerValue, 0, 10);
    EXPECT_EQ(result, 0);

    headerValue.clear();
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    headerValue.push_back(0x02);
    result = obexHeader.FindHeaderEnd(headerValue, 0, 10);
    EXPECT_EQ(result, 0);

    headerValue.clear();
    headerValue.push_back(0x01);
    headerValue.push_back(0x00);
    headerValue.push_back(0x05);
    headerValue.push_back(0x00);
    headerValue.push_back(0x01);
    result = obexHeader.FindHeaderEnd(headerValue, 0, 10);
    EXPECT_EQ(result, 5);
    HILOGI("ObexHeader_FindHeaderEnd_001 end");
}

/*
 * @tc.number: ObexHeader_FindHeaderEnd_002
 * @tc.name: FindHeaderEnd
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_FindHeaderEnd_002, TestSize.Level1)
{
    HILOGI("ObexHeader_FindHeaderEnd_002 start");
    std::vector<uint8_t> headerValue;
    headerValue.push_back(0x80);
    headerValue.push_back(0x01);
    uint32_t result = obexHeader.FindHeaderEnd(headerValue, 0, 10);
    EXPECT_EQ(result, 2);

    headerValue.clear();
    headerValue.push_back(0xCB);
    headerValue.push_back(0x00);
    headerValue.push_back(0x00);
    headerValue.push_back(0x00);
    headerValue.push_back(0x01);
    result = obexHeader.FindHeaderEnd(headerValue, 0, 10);
    EXPECT_EQ(result, 5);
    HILOGI("ObexHeader_FindHeaderEnd_002 end");
}

/*
 * @tc.number: ObexHeader_GetConnectionId_001
 * @tc.name: GetConnectionId
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_GetConnectionId_001, TestSize.Level1)
{
    HILOGI("ObexHeader_GetConnectionId_001 start");
    int32_t connectId = obexHeader.GetConnectionId();
    EXPECT_EQ(connectId, -1);

    std::vector<uint8_t> headerValue;
    headerValue.push_back(0x00);
    headerValue.push_back(0x01);
    obexHeader.SetHeader(CONNECTION_ID, headerValue);
    connectId = obexHeader.GetConnectionId();
    EXPECT_EQ(connectId, -1);

    obexHeader.SetConnectionId(12);
    connectId = obexHeader.GetConnectionId();
    EXPECT_EQ(connectId, 12);
    HILOGI("ObexHeader_GetConnectionId_001 end");
}

/*
 * @tc.number: ObexHeader_GetName_001
 * @tc.name: GetName
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_GetName_001, TestSize.Level1)
{
    HILOGI("ObexHeader_GetName_001 start");
    EXPECT_EQ(obexHeader.GetName(), "");

    std::vector<uint8_t> headerValue;
    headerValue.push_back(0x01);
    obexHeader.SetHeader(NAME, headerValue);
    EXPECT_EQ(obexHeader.GetName(), "");

    std::vector<uint8_t> headerValue1 = {
        0x00, 0x74, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x2F,
        0x00, 0x63, 0x00, 0x63, 0x00, 0x68, 0x00, 0x2E, 0x00, 0x76, 0x00, 0x63, 0x00, 0x66};
    obexHeader.SetHeader(NAME, headerValue1);
    EXPECT_EQ(obexHeader.GetName(), "telecom/cch.vcf");
    HILOGI("ObexHeader_GetName_001 end");
}

/*
 * @tc.number: ObexHeader_GetType_001
 * @tc.name: GetType
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_GetType_001, TestSize.Level1)
{
    HILOGI("ObexHeader_GetType_001 start");
    EXPECT_EQ(obexHeader.GetType(), "");

    std::vector<uint8_t> headerValue({
        0x78, 0x2D, 0x62, 0x74, 0x2F, 0x70, 0x68, 0x6F, 0x6E, 0x65, 0x62, 0x6F, 0x6F, 0x6B });
    obexHeader.SetHeader(TYPE, headerValue);
    EXPECT_EQ(obexHeader.GetType(), "x-bt/phonebook");
    HILOGI("ObexHeader_GetType_001 end");
}

/*
 * @tc.number: ObexHeader_ConvertUnicodeToVector_001
 * @tc.name: ConvertUnicodeToVector
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_ConvertUnicodeToVector_001, TestSize.Level1)
{
    HILOGI("ObexHeader_ConvertUnicodeToVector_001 start");

    std::vector<uint8_t> value;
    obexHeader.ConvertUnicodeToVector("hello", value);
    EXPECT_EQ(value.size(), 12);
    HILOGI("ObexHeader_ConvertUnicodeToVector_001 end");
}

/*
 * @tc.number: ObexHeader_Clear_001
 * @tc.name: Clear
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_Clear_001, TestSize.Level1)
{
    HILOGI("ObexHeader_Clear_001 start");
    obexHeader.Clear();
    EXPECT_EQ(obexHeader.obexHeaderMap_.size(), 0);
    HILOGI("ObexHeader_Clear_001 end");
}

/*
 * @tc.number: ObexHeader_SetLengthAndClear_001
 * @tc.name: SetLength
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_SetLengthAndClear_001, TestSize.Level1)
{
    HILOGI("ObexHeader_SetLengthAndClear_001 start");
    obexHeader.SetLength(0xFF);
    uint32_t length = 0;
    obexHeader.GetLength(length);
    EXPECT_EQ(length, 0xFF);
    obexHeader.Clear();
    length = 0;
    obexHeader.GetLength(length);
    EXPECT_EQ(length, 0);
    HILOGI("ObexHeader_SetLengthAndClear_001 end");
}

/*
 * @tc.number: ObexHeader_RecvTimeoutMs_001
 * @tc.name: RecvTimeoutMs
 * @tc.desc:
 */
HWTEST_F(ObexHeaderTest, ObexHeader_RecvTimeoutMs_001, TestSize.Level1)
{
    HILOGI("ObexHeader_RecvTimeoutMs_001 start");
    obexHeader.SetRecvTimeoutMs(1);
    EXPECT_EQ(obexHeader.GetRecvTimeoutMs(), 1);
    HILOGI("ObexHeader_RecvTimeoutMs_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS