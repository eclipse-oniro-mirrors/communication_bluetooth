
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
#include "map_mse_content.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "../../map_mse/service/data_share_helper_impl_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

constexpr int32_t TIME = 4;
static BluetoothHost *host_;

class MapMseContentTest : public testing::Test {
public:
    MapMseContentTest() {}
    ~MapMseContentTest() {}
    std::unique_ptr<MapMseContent> mapMseContent = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MapMseContentTest::SetUpTestCase(void)
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
}

void MapMseContentTest::TearDownTestCase(void) {}

void MapMseContentTest::SetUp()
{
    mapMseContent = std::make_unique<MapMseContent>();
    mapMseContent->smsDataShareHelper_ = std::make_shared<DataShareHelperImplTest>();
}

void MapMseContentTest::TearDown() {}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_GetMessageListing_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_GetMessageListing_001 start");
    std::string folderName = "";
    std::vector<uint8_t> myParams {50, 0};
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>(myParams);
    std::string version = "";
    int32_t listSize = 1;
    bool hasUnRead = true;
    std::string res = mapMseContent->GetMessageListing(folderName, appParams, version, listSize, hasUnRead);
    HILOGI("MapMseContentTest_UnitTest_GetMessageListing_001BJJ %{public}s", res.c_str());
    EXPECT_NE(res, "");
    mapMseContent->smsDataShareHelper_ = nullptr;
    res = mapMseContent->GetMessageListing(folderName, appParams, version, listSize, hasUnRead);
    EXPECT_EQ(res, "");
    mapMseContent->CreateSmsDataShareHelper();
    HILOGI("MapMseContentTest_UnitTest_GetMessageListing_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_GetMessageListingSize_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_GetMessageListingSize_001 start");
    std::string folderName = "";
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    bool hasUnRead = true;
    int32_t res = mapMseContent->GetMessageListingSize(folderName, appParams, hasUnRead);
    HILOGI("MapMseContentTest_UnitTest_GetMessageListingSize_001 %{public}d", res);
    EXPECT_EQ(res, 0);
    mapMseContent->smsDataShareHelper_ = nullptr;
    res = mapMseContent->GetMessageListingSize(folderName, appParams, hasUnRead);
    EXPECT_EQ(res, 0);
    mapMseContent->CreateSmsDataShareHelper();
    HILOGI("MapMseContentTest_UnitTest_GetMessageListingSize_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_GetMessage_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_GetMessage_001 start");
    std::string handleStr = "0x01";
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    std::string version = "";
    std::string res = mapMseContent->GetMessage(handleStr, appParams, version);
    HILOGI("MapMseContentTest_UnitTest_GetMessage_001 %{public}s", res.c_str());
    EXPECT_EQ(res, "");
    handleStr = "";
    res = mapMseContent->GetMessage(handleStr, appParams, version);
    EXPECT_EQ(res, "");
    handleStr = "0xtest";
    res = mapMseContent->GetMessage(handleStr, appParams, version);
    EXPECT_EQ(res, "");
    handleStr = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    res = mapMseContent->GetMessage(handleStr, appParams, version);
    EXPECT_EQ(res, "");
    handleStr = "8000000000000000";
    res = mapMseContent->GetMessage(handleStr, appParams, version);
    EXPECT_EQ(res, "");
    handleStr = "7FFFFFFFFFFFFFFF";
    res = mapMseContent->GetMessage(handleStr, appParams, version);
    EXPECT_EQ(res, "");
    HILOGI("MapMseContentTest_UnitTest_GetMessage_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_GetSmsMessageListingSize_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_GetSmsMessageListingSize_001 start");
    std::string folderName = "";
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    bool hasUnRead = true;
    int32_t  res = mapMseContent->GetSmsMessageListingSize(folderName, appParams, hasUnRead);
    HILOGI("MapMseContentTest_UnitTest_GetMessageListing_001 %{public}d", res);
    EXPECT_EQ(res, 0);
    HILOGI("MapMseContentTest_UnitTest_GetSmsMessageListingSize_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_GetSmsMessageListing_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_GetSmsMessageListing_001 start");
    std::string folderName = "";
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    std::vector<std::unique_ptr<MseMessageElement>> messageList;
    bool hasUnRead = true;
    messageList = mapMseContent->GetSmsMessageListing(folderName, appParams, hasUnRead);
    EXPECT_EQ(messageList.size(), 0);
    folderName = MAP_MSE_FOLDER_NAME_DRAFT;
    messageList = mapMseContent->GetSmsMessageListing(folderName, appParams, hasUnRead);
    EXPECT_EQ(messageList.size(), 0);
    hasUnRead = false;
    messageList = mapMseContent->GetSmsMessageListing(folderName, appParams, hasUnRead);
    EXPECT_EQ(messageList.size(), 0);
    folderName = MAP_MSE_FOLDER_NAME_INBOX;
    messageList = mapMseContent->GetSmsMessageListing(folderName, appParams, hasUnRead);
    EXPECT_EQ(messageList.size(), 0);
    HILOGI("MapMseContentTest_UnitTest_GetSmsMessageListing_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_GetSmsMessage_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_GetSmsMessage_001 start");
    int64_t handle = 0x01;
    std::string res = mapMseContent->GetSmsMessage(handle);
    EXPECT_NE(res, "");
    HILOGI("MapMseContentTest_UnitTest_GetSmsMessage_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_SetSmsMessageVcardData_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_SetSmsMessageVcardData_001 start");
    std::shared_ptr<MapMseBMessageSms> message = std::make_shared<MapMseBMessageSms>();
    std::string number = "";
    bool hasUnRead = true;
    mapMseContent->SetSmsMessageVcardData(message, number, hasUnRead);
    number = "0x01";
    mapMseContent->SetSmsMessageVcardData(message, number, hasUnRead);
    hasUnRead = false;
    mapMseContent->SetSmsMessageVcardData(message, number, hasUnRead);
    HILOGI("MapMseContentTest_UnitTest_SetSmsMessageVcardData_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_GetConversationListing_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_GetConversationListing_001 start");
    EXPECT_EQ(mapMseContent->GetConversationListing(), "");
    HILOGI("MapMseContentTest_UnitTest_GetConversationListing_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_EncodeSmsMessageListing_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_EncodeSmsMessageListing_001 start");
    std::vector<std::unique_ptr<MseMessageElement>> messageList;
    std::string version = "";
    EXPECT_NE(mapMseContent->EncodeSmsMessageListing(messageList, version), "");
    HILOGI("MapMseContentTest_UnitTest_EncodeSmsMessageListing_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_ParseSmsMessageListingSizeResult_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_ParseSmsMessageListingSizeResult_001 start");
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = std::make_shared<DataShare::DataShareResultSet>();
    std::string folderName = "";
    int32_t count = 1;
    bool hasUnRead;
    mapMseContent->ParseSmsMessageListingSizeResult(resultSet, folderName, count, hasUnRead);
    HILOGI("MapMseContentTest_UnitTest_ParseSmsMessageListingSizeResult_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_ParseSmsMessageListingResult_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_ParseSmsMessageListingResult_001 start");
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = std::make_shared<DataShare::DataShareResultSet>();
    std::string folderName = "draft";
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    std::vector<std::unique_ptr<MseMessageElement>> messageList;
    bool hasUnRead;
    mapMseContent->ParseSmsMessageListingResult(resultSet, folderName, appParams, messageList, hasUnRead);
    HILOGI("MapMseContentTest_UnitTest_ParseSmsMessageListingResult_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_ParseSmsMessageListingResult_002, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_ParseSmsMessageListingResult_002 start");
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = std::make_shared<DataShare::DataShareResultSet>();
    std::string folderName = "";
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    std::vector<std::unique_ptr<MseMessageElement>> messageList;
    bool hasUnRead;
    mapMseContent->ParseSmsMessageListingResult(resultSet, folderName, appParams, messageList, hasUnRead);
    HILOGI("MapMseContentTest_UnitTest_ParseSmsMessageListingResult_002 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_SetSmsMessageListingQueryParam_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_SetSmsMessageListingQueryParam_001 start");
    std::string folderName = "draft";
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>();
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    Uri uri{""};
    mapMseContent->SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);

    folderName = "inbox";
    mapMseContent->SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);

    folderName = "outbox";
    mapMseContent->SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);

    folderName = "sent";
    mapMseContent->SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);

    folderName = "deleted";
    mapMseContent->SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);

    folderName = "";
    mapMseContent->SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);

    folderName = "deleted";
    mapMseContent->SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);
    HILOGI("MapMseContentTest_UnitTest_SetSmsMessageListingQueryParam_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_001 start");
    std::string folderName = "draft";
    std::vector<uint8_t> raw{PARAM_FILTER_READ_STATUS, TAG_LEN_FILTER_READ_STATUS, 0x10};
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>(raw);
    DataShare::DataSharePredicates predicates;
    mapMseContent->SetOtherFiltersQueryParam(folderName, appParams, predicates);
    HILOGI("MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_002, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_002 start");
    std::string folderName = "draft";
    std::vector<uint8_t> raw{PARAM_FILTER_PERIOD_BEGIN, 0x02, 0x10, 0x10};
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>(raw);
    DataShare::DataSharePredicates predicates;
    mapMseContent->SetOtherFiltersQueryParam(folderName, appParams, predicates);

    folderName = "";
    mapMseContent->SetOtherFiltersQueryParam(folderName, appParams, predicates);
    HILOGI("MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_002 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_003, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_003 start");
    std::string folderName = "draft";
    std::vector<uint8_t> raw{PARAM_FILTER_PERIOD_END, 0x02, 0x10, 0x10};
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>(raw);
    DataShare::DataSharePredicates predicates;
    mapMseContent->SetOtherFiltersQueryParam(folderName, appParams, predicates);

    folderName = "";
    mapMseContent->SetOtherFiltersQueryParam(folderName, appParams, predicates);
    HILOGI("MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_003 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_004, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_004 start");
    std::string folderName = "draft";
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
    std::shared_ptr<MapMseAppParams> appParams = std::make_shared<MapMseAppParams>(raw);
    DataShare::DataSharePredicates predicates;
    mapMseContent->SetOtherFiltersQueryParam(folderName, appParams, predicates);

    folderName = "";
    mapMseContent->SetOtherFiltersQueryParam(folderName, appParams, predicates);
    HILOGI("MapMseContentTest_UnitTest_SetOtherFiltersQueryParam_004 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_QueryContactNameByNumber_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_QueryContactNameByNumber_001 start");
    std::string name = mapMseContent->QueryContactNameByNumber("");
    EXPECT_EQ(name, "");
    HILOGI("MapMseContentTest_UnitTest_QueryContactNameByNumber_001 end");
}

HWTEST_F(MapMseContentTest, MapMseContentTest_UnitTest_QueryContactEmailsByContactId_001, TestSize.Level1)
{
    HILOGI("MapMseContentTest_UnitTest_QueryContactEmailsByContactId_001 start");
    std::vector<std::string> result = mapMseContent->QueryContactEmailsByContactId(0);
    EXPECT_EQ(result.size(), 0);
    HILOGI("MapMseContentTest_UnitTest_QueryContactEmailsByContactId_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS