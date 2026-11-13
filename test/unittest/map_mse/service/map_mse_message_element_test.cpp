/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd. All rights reserved.
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
#include <vector>
#include <cstdint>
#include <optional>
#include <string>
#include "map_mse_message_element.h"

namespace OHOS {
namespace bluetooth {
using namespace testing;
using namespace testing::ext;
class MapMseMessageElementTest : public testing::Test {
public:
    MapMseMessageElementTest() {}
    ~MapMseMessageElementTest() {}
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(){};
    void TearDown(){};
};

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetHandle_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);

    mseMessageElement.SetHandle(1);
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetSubject_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);
    // appParams_->subjectLength_.has_value() false
    mseMessageElement.SetSubject("qwertyuiop");

    // appParams_->subjectLength_.has_value() true
    appParams->subjectLength_ = 256;
    mseMessageElement.SetSubject("qwertyuiop");

    // appParams_->subjectLength_.value() < 255
    appParams->subjectLength_ = 128;
    mseMessageElement.SetSubject("qwertyuiop");

    // appParams_->subjectLength_.value() > 255
    appParams->subjectLength_ = 256;
    mseMessageElement.SetSubject("qwertyuiop");

    // appParams_->parameterMask_.has_value() false
    mseMessageElement.SetSubject("qwertyuiop");

    // appParams_->parameterMask_.has_value() true && (appParams_->parameterMask_.value() & MASK_SUBJECT) != 0
    appParams->parameterMask_ = 0x01;
    mseMessageElement.SetSubject("qwertyuiop");

    // (appParams_->parameterMask_.value() & MASK_SUBJECT) == 0
    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetSubject("qwertyuiop");

    // value.length() > subjectLength
    appParams->subjectLength_ = 5;
    mseMessageElement.SetSubject("qwertyuiop");

    // value.length() > maxLength
    mseMessageElement.SetSubject("qwertyuiopasdfghjkl");

    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetDateTime_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetDateTime(1997151236);

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetDateTime(1997151236);

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetDateTime(1997151236);

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetDateTime(1997151236);
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetSenderName_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetSenderName("");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetSenderName("");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetSenderName("123456789");

    appParams->parameterMask_ = 0x04;
    mseMessageElement.SetSenderName("123456789");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetSenderAddressing_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetSenderAddressing("");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetSenderAddressing("");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetSenderAddressing("123456789");

    appParams->parameterMask_ = 0x08;
    mseMessageElement.SetSenderAddressing("123456789");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetRecipientName_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetRecipientName("");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetRecipientName("");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetRecipientName("123456789");

    appParams->parameterMask_ = 0x10;
    mseMessageElement.SetRecipientName("123456789");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetRecipientAddressing_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetRecipientAddressing("");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetRecipientAddressing("");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetRecipientAddressing("123456789");

    appParams->parameterMask_ = 0x20;
    mseMessageElement.SetRecipientAddressing("123456789");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetType_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetType(TYPE_SMS_GSM);

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetType(TYPE_SMS_GSM);

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetType(TYPE_SMS_GSM);

    appParams->parameterMask_ = 0x40;
    mseMessageElement.SetType(TYPE_SMS_GSM);
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetSize_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetSize(1);

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetSize(1);

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetSize(1);

    appParams->parameterMask_ = 0x80;
    mseMessageElement.SetSize(1);
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetText_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetText("123456789");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetText("123456789");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetText("123456789");

    appParams->parameterMask_ = 0x200;
    mseMessageElement.SetText("123456789");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetReceptionStatus_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetReceptionStatus("complete");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetReceptionStatus("complete");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetReceptionStatus("complete");

    appParams->parameterMask_ = 0x100;
    mseMessageElement.SetReceptionStatus("complete");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetDeliveryStatus_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetDeliveryStatus("delivered");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetDeliveryStatus("delivered");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetDeliveryStatus("delivered");

    appParams->parameterMask_ = 0x00010000;
    mseMessageElement.SetDeliveryStatus("delivered");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetAttachmentSize_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetAttachmentSize(0);

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetAttachmentSize(0);

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetAttachmentSize(0);

    appParams->parameterMask_ = 0x400;
    mseMessageElement.SetAttachmentSize(0);
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetPriority_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetPriority("no");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetPriority("no");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetPriority("no");

    appParams->parameterMask_ = 0x800;
    mseMessageElement.SetPriority("no");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetRead_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetRead(true);

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetRead(true);

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetRead(true);

    appParams->parameterMask_ = MASK_READ;
    mseMessageElement.SetRead(true);
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetSent_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetSent("sent");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetSent("sent");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetSent("sent");

    appParams->parameterMask_ = MASK_SENT;
    mseMessageElement.SetSent("sent");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetProtect_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetProtect("no");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetProtect("no");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetProtect("no");

    appParams->parameterMask_ = MASK_PROTECTED;
    mseMessageElement.SetProtect("no");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetThreadId_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetThreadId("1");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetThreadId("1");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetThreadId("1");

    appParams->parameterMask_ = MASK_CONVERSATION_ID;
    mseMessageElement.SetThreadId("1");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_SetThreadName_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.SetThreadName("no");

    appParams->parameterMask_ = 0x02;
    mseMessageElement.SetThreadName("no");

    appParams->parameterMask_ = 0x00;
    mseMessageElement.SetThreadName("no");

    appParams->parameterMask_ = MASK_CONVERSATION_NAME;
    mseMessageElement.SetThreadName("no");
    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_GetMessageTypeFromHandle_001, TestSize.Level1)
{
    int64_t handle = 0x01 | HANDLE_TYPE_MMS_MASK;
    EXPECT_EQ(TYPE_MMS, MseMessageElement::GetMessageTypeFromHandle(handle));

    handle = 0x01 | HANDLE_TYPE_EMAIL_MASK;
    EXPECT_EQ(TYPE_EMAIL, MseMessageElement::GetMessageTypeFromHandle(handle));

    handle = 0x01 | HANDLE_TYPE_SMS_GSM_MASK;
    EXPECT_EQ(TYPE_SMS_GSM, MseMessageElement::GetMessageTypeFromHandle(handle));

    handle = 0x01 | HANDLE_TYPE_SMS_CDMA_MASK;
    EXPECT_EQ(TYPE_SMS_CDMA, MseMessageElement::GetMessageTypeFromHandle(handle));

    handle = 0x01 | HANDLE_TYPE_IM_MASK;
    EXPECT_EQ(TYPE_IM, MseMessageElement::GetMessageTypeFromHandle(handle));
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_GetMessageTypeStr_001, TestSize.Level1)
{
    EXPECT_EQ("NONE", MseMessageElement::GetMessageTypeStr(TYPE_NONE));

    EXPECT_EQ("EMAIL", MseMessageElement::GetMessageTypeStr(TYPE_EMAIL));

    EXPECT_EQ("SMS_GSM", MseMessageElement::GetMessageTypeStr(TYPE_SMS_GSM));

    EXPECT_EQ("SMS_CDMA", MseMessageElement::GetMessageTypeStr(TYPE_SMS_CDMA));

    EXPECT_EQ("IM", MseMessageElement::GetMessageTypeStr(TYPE_IM));

    MessageType defaultType = static_cast<MessageType>(0x1F);
    EXPECT_EQ("NONE", MseMessageElement::GetMessageTypeStr(defaultType));
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_GetParsedMessageHandle_001, TestSize.Level1)
{
    int64_t handle = 0x01 | HANDLE_TYPE_MMS_MASK;
    EXPECT_EQ(0x01, MseMessageElement::GetParsedMessageHandle(handle));
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_GenerateStringHandle_001, TestSize.Level1)
{
    int64_t handle = 0x01;
    EXPECT_EQ("400000000000001", MseMessageElement::GenerateStringHandle(handle, TYPE_SMS_GSM));

    EXPECT_EQ("200000000000001", MseMessageElement::GenerateStringHandle(handle, TYPE_EMAIL));

    EXPECT_EQ("800000000000001", MseMessageElement::GenerateStringHandle(handle, TYPE_SMS_CDMA));

    EXPECT_EQ("100000000000001", MseMessageElement::GenerateStringHandle(handle, TYPE_MMS));

    EXPECT_EQ("1000000000000001", MseMessageElement::GenerateStringHandle(handle, TYPE_IM));

    EXPECT_EQ("-1", MseMessageElement::GenerateStringHandle(handle, TYPE_NONE));
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_GetEncodeData_001, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, true);
    mseMessageElement.SetHandle(1);
    mseMessageElement.SetType(TYPE_SMS_GSM);
    mseMessageElement.SetMsgTypeAppParamSet(true);

    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_GetEncodeData_002, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);

    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}

HWTEST_F(MapMseMessageElementTest, MapMseMessageElementTest_UnitTest_GetEncodeData_003, TestSize.Level1)
{
    std::string xml;
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    MseMessageElement mseMessageElement(appParams, false);
    mseMessageElement.SetHandle(1);
    mseMessageElement.SetMsgTypeAppParamSet(false);

    mseMessageElement.GetEncodeData(xml);
    EXPECT_NE(xml.size(), 0);
}
}  // namespace bluetooth
}  // namespace OHOS