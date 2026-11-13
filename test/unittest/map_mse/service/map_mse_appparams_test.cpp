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
#include <vector>
#include <cstdint>
#include <optional>
#include "map_mse_appparams.h"

namespace OHOS {
namespace bluetooth {
using namespace testing;
using namespace testing::ext;
class MapMseAppParamsTest : public testing::Test {
public:
    MapMseAppParamsTest() {}
    ~MapMseAppParamsTest() {}
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(){};
    void TearDown(){};
};

static std::vector<uint8_t> output_{};
static int32_t pos_ = 0;
static int32_t tagLength_ = 0;

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseParams_001, TestSize.Level1)
{
    // pos + 2 > appParams.size()
    std::vector<uint8_t> raw{};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.maxListCount_.has_value(), false);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseParams_002, TestSize.Level1)
{
    // pos + 2 = appParams.size()
    std::vector<uint8_t> raw{0, 1};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.maxListCount_.has_value(), false);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseParams_003, TestSize.Level1)
{
    // pos + 2 > appParams.size()
    std::vector<uint8_t> raw{0x01, 0x02, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.maxListCount_.has_value(), true);
    EXPECT_EQ(params.maxListCount_.value(), ((0x10 << 8) | 0x10));
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseParams_004, TestSize.Level1)
{
    // pos + tagLength > appParams.size()
    std::vector<uint8_t> raw{0x01, 0x02, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.maxListCount_.has_value(), false);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseParams_005, TestSize.Level1)
{
    // parseFuncs.count(tagId) < 0
    std::vector<uint8_t> raw{0x50, 0x02, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.maxListCount_.has_value(), false);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseMaxListCount_001, TestSize.Level1)
{
    // tagLength != TAG_LEN_MAX_LIST_COUNT
    std::vector<uint8_t> raw{0x01, 0x01};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.maxListCount_.has_value(), false);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseMaxListCount_002, TestSize.Level1)
{
    // tagLength == TAG_LEN_MAX_LIST_COUNT
    std::vector<uint8_t> raw{0x01, 0x02, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.maxListCount_.has_value(), true);
    EXPECT_EQ(params.maxListCount_.value(), ((0x10 << 8) | 0x10));
    params.ParseMaxListCount(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseListStartOffSet_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_LIST_START_OFF_SET, 0xFF};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.listStartOffSet_.has_value(), false);
    params.ParseListStartOffSet(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseListStartOffSet_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_LIST_START_OFF_SET, TAG_LEN_LIST_START_OFF_SET, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.listStartOffSet_.has_value(), true);
    EXPECT_EQ(params.listStartOffSet_.value(), ((0x10 << 8) | 0x10));
    params.ParseListStartOffSet(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterMessageType_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_MESSAGE_TYPE, 0xFF};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterMessageType_.has_value(), false);
    params.ParseFilterMessageType(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterMessageType_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_MESSAGE_TYPE, TAG_LEN_FILTER_MESSAGE_TYPE, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterMessageType_.has_value(), true);
    EXPECT_EQ(params.filterMessageType_.value(), 0x10);
    params.ParseFilterMessageType(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterPeriodBegin_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_PERIOD_BEGIN, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterPeriodBegin_.has_value(), false);
    params.ParseFilterPeriodBegin(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterPeriodBegin_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_PERIOD_BEGIN, 0x02, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterPeriodBegin_.has_value(), true);
    params.ParseFilterPeriodBegin(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterPeriodEnd_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_PERIOD_END, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterPeriodEnd_.has_value(), false);
    params.ParseFilterPeriodEnd(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterPeriodEnd_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_PERIOD_END, 0x02, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterPeriodEnd_.has_value(), true);
    params.ParseFilterPeriodEnd(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterReadStatus_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_READ_STATUS, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterReadStatus_.has_value(), false);
    params.ParseFilterReadStatus(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterReadStatus_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_READ_STATUS, TAG_LEN_FILTER_READ_STATUS, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterReadStatus_.has_value(), true);
    params.ParseFilterReadStatus(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterRecipient_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_RECIPIENT, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterRecipient_.has_value(), false);
    params.ParseFilterRecipient(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterRecipient_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_RECIPIENT, 0x01, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterRecipient_.has_value(), true);
    params.ParseFilterRecipient(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterOriginator_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_ORIGINATOR, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterOriginator_.has_value(), false);
    params.ParseFilterOriginator(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterOriginator_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_ORIGINATOR, 0x01, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterOriginator_.has_value(), true);
    params.ParseFilterOriginator(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterPriority_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_PRIORITY, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterPriority_.has_value(), false);
    params.ParseFilterPriority(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterPriority_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_PRIORITY, TAG_LEN_FILTER_PRIORITY, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterPriority_.has_value(), true);
    params.ParseFilterPriority(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseAttachment_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_ATTACHMENT, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.attachment_.has_value(), false);
    params.ParseAttachment(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseAttachment_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_ATTACHMENT, TAG_LEN_ATTACHMENT, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.attachment_.has_value(), true);
    params.ParseAttachment(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseTransparent_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_TRANSPARENT, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.transparent_.has_value(), false);
    params.ParseTransparent(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseTransparent_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_TRANSPARENT, TAG_LEN_TRANSPARENT, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.transparent_.has_value(), true);
    params.ParseTransparent(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseRetry_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_RETRY, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.retry_.has_value(), false);
    params.ParseRetry(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseRetry_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_RETRY, TAG_LEN_RETRY, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.retry_.has_value(), true);
    params.ParseRetry(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseNewMessage_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_NEW_MESSAGE, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.newMessage_.has_value(), false);
    params.ParseNewMessage(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseNewMessage_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_NEW_MESSAGE, TAG_LEN_NEW_MESSAGE, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.newMessage_.has_value(), true);
    params.ParseNewMessage(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseNotificationStatus_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_NOTIFICATION_STATUS, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.notificationStatus_.has_value(), false);
    params.ParseNotificationStatus(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseNotificationStatus_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_NOTIFICATION_STATUS, TAG_LEN_NOTIFICATION_STATUS, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.notificationStatus_.has_value(), true);
    params.ParseNotificationStatus(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseMasInstanceid_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_MAS_INSTANCEID, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.masInstanceId_.has_value(), false);
    params.ParseMasInstanceid(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseMasInstanceid_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_MAS_INSTANCEID, TAG_LEN_MAS_INSTANCEID, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.masInstanceId_.has_value(), true);
    params.ParseMasInstanceid(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseParameterMask_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_PARAMETER_MASK, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.parameterMask_.has_value(), false);
    params.ParseParameterMask(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseParameterMask_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_PARAMETER_MASK, TAG_LEN_PARAMETER_MASK, 0x10, 0x10, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.parameterMask_.has_value(), true);
    params.ParseParameterMask(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFolderListingSize_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FOLDER_LISTING_SIZE, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.folderListingSize_.has_value(), false);
    params.ParseFolderListingSize(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFolderListingSize_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FOLDER_LISTING_SIZE, TAG_LEN_FOLDER_LISTING_SIZE, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.folderListingSize_.has_value(), true);
    params.ParseFolderListingSize(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseListingSize_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_LISTING_SIZE, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.messagesListingSize_.has_value(), false);
    params.ParseListingSize(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseListingSize_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_LISTING_SIZE, TAG_LEN_LISTING_SIZE, 0x10, 0x20};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.messagesListingSize_.has_value(), true);
    params.ParseListingSize(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseSubjectLength_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_SUBJECT_LENGTH, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.subjectLength_.has_value(), false);
    params.ParseSubjectLength(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseSubjectLength_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_SUBJECT_LENGTH, TAG_LEN_SUBJECT_LENGTH, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.subjectLength_.has_value(), true);
    params.ParseSubjectLength(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseCharset_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_CHARSET, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.charSet_.has_value(), false);
    params.ParseCharset(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseCharset_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_CHARSET, TAG_LEN_CHARSET, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.charSet_.has_value(), true);
    params.ParseCharset(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFractionRequest_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FRACTION_REQUEST, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.fractionRequest_.has_value(), false);
    params.ParseFractionRequest(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFractionRequest_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FRACTION_REQUEST, TAG_LEN_FRACTION_REQUEST, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.fractionRequest_.has_value(), true);
    params.ParseFractionRequest(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFractionDeliver_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FRACTION_DELIVER, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.fractionDeliver_.has_value(), false);
    params.ParseFractionDeliver(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFractionDeliver_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FRACTION_DELIVER, TAG_LEN_FRACTION_DELIVER, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.fractionDeliver_.has_value(), true);
    params.ParseFractionDeliver(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseStatusIndicator_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_STATUS_INDICATOR, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.statusIndicator_.has_value(), false);
    params.ParseStatusIndicator(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseStatusIndicator_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_STATUS_INDICATOR, TAG_LEN_STATUS_INDICATOR, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.statusIndicator_.has_value(), true);
    params.ParseStatusIndicator(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseStatusValue_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_STATUS_VALUE, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.statusValue_.has_value(), false);
    params.ParseStatusValue(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseStatusValue_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_STATUS_VALUE, TAG_LEN_STATUS_VALUE, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.statusValue_.has_value(), true);
    params.ParseStatusValue(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseConversationListingVersionCounter_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_CONVERSATION_LISTING_VERSION_COUNTER, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.conversationListingVersionCounterLow_.has_value(), false);
    params.ParseConversationListingVersionCounter(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseConversationListingVersionCounter_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{
        PARAM_CONVERSATION_LISTING_VERSION_COUNTER,
        TAG_LEN_CONVERSATION_LISTING_VERSION_COUNTER,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
    };
    MapMseAppParams params(raw);
    EXPECT_EQ(params.conversationListingVersionCounterLow_.has_value(), true);
    params.ParseConversationListingVersionCounter(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseConversationId_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_CONVERSATION_ID, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.conversationIdLow_.has_value(), false);
    params.ParseConversationId(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseConversationId_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{
        PARAM_CONVERSATION_ID,
        TAG_LEN_CONVERSATION_ID,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
        0x10,
    };
    MapMseAppParams params(raw);
    EXPECT_EQ(params.conversationIdLow_.has_value(), true);
    params.ParseConversationId(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterMessageHandle_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_FILTER_MESSAGE_HANDLE, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterMessageHandle_.has_value(), false);
    params.ParseFilterMessageHandle(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseFilterMessageHandle_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{
        PARAM_FILTER_MESSAGE_HANDLE, TAG_LEN_FILTER_MESSAGE_HANDLE, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.filterMessageHandle_.has_value(), true);
    params.ParseFilterMessageHandle(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseConvoListingSize_001, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_CONVO_LISTING_SIZE, 0x00};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.convoListingSize_.has_value(), false);
    params.ParseConvoListingSize(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_ParseConvoListingSize_002, TestSize.Level1)
{
    std::vector<uint8_t> raw{PARAM_CONVO_LISTING_SIZE, TAG_LEN_CONVO_LISTING_SIZE, 0x10, 0x10};
    MapMseAppParams params(raw);
    EXPECT_EQ(params.convoListingSize_.has_value(), true);
    params.ParseConvoListingSize(output_, pos_, tagLength_);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_GetEncodeData_001, TestSize.Level1)
{
    std::vector<uint8_t> output{};
    MapMseAppParams params;
    params.GetEncodeData(output);
    EXPECT_EQ(output.size(), 0);
}

HWTEST_F(MapMseAppParamsTest, MapMseAppParamsTest_UnitTest_GetEncodeData_002, TestSize.Level1)
{
    std::vector<uint8_t> output{};
    MapMseAppParams params;
    params.mseTime_ = "199825163356";
    params.maxListCount_ = 0x10;
    params.conversationIdLow_ = 0x10;
    params.conversationIdHigh_ = 0x20;
    params.GetEncodeData(output);
    EXPECT_NE(output.size(), 0);
}
}  // namespace bluetooth
}  // namespace OHOS