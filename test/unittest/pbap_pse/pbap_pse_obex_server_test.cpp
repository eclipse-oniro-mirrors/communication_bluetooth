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

#include <gtest/gtest.h>

#include "btcommon/timer_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "pbap_pse_obex_server.h"
#include "pbap_pse_def.h"
#include "class_creator.h"
#include "pbap_pse_service.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace OHOS::Bluetooth;

void *PbapPseServiceCreate(void)
{
    return new PbapPseService();
}

const std::vector<uint8_t> typePb = {
    0x78, 0x2D, 0x62, 0x74, 0x2F, 0x70, 0x68, 0x6F,
    0x6E, 0x65, 0x62, 0x6F, 0x6F, 0x6B, 0x00};

const std::vector<uint8_t> namePb = {
    0x00, 0x74, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x65,
    0x00, 0x63, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x2F,
    0x00, 0x70, 0x00, 0x62, 0x00, 0x2E, 0x00, 0x76,
    0x00, 0x63, 0x00, 0x66, 0x00, 0x00};

constexpr int32_t TIME = 4;
static BluetoothHost *host_;
class PbapPseObexServerTest : public testing::Test {
public:
    PbapPseObexServerTest();
    ~PbapPseObexServerTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
    std::string addr_ = "00:11:22:33:44:55";
    std::unique_ptr<PbapPseObexServer> pbapPseObexServerTest_ = nullptr;
};

PbapPseObexServerTest::PbapPseObexServerTest()
{
    pbapPseObexServerTest_ = std::make_unique<PbapPseObexServer>(addr_, nullptr);
}

PbapPseObexServerTest::~PbapPseObexServerTest() {}

void PbapPseObexServerTest::SetUpTestCase(void)
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
    ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
}

void PbapPseObexServerTest::TearDownTestCase(void) {
    utility::CloseAllTimer();
}

void PbapPseObexServerTest::SetUp() {}

void PbapPseObexServerTest::TearDown() {}

/*
 * @tc.number: PbapPse_ModuleTest_IsNameMatchTarget_00100
 * @tc.name: IsNameMatchTarget
 * @tc.desc: IsNameMatchTarget
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_IsNameMatchTarget_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_IsNameMatchTarget_00100 start");
    EXPECT_FALSE(pbapPseObexServerTest_->IsNameMatchTarget("", "pb"));
    EXPECT_FALSE(pbapPseObexServerTest_->IsNameMatchTarget(".vcf", PB));
    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("pb.vcf", PB));
    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("pb", PB));
    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/pb", PB));
    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/pb.vcf", PB));
    EXPECT_FALSE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/pb.vdf", PB));

    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/fav.vcf", FAV));
    EXPECT_FALSE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/pb.vcf", FAV));

    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/ich.vcf", ICH));
    EXPECT_FALSE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/pb.vcf", ICH));

    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/och.vcf", OCH));
    EXPECT_FALSE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/pb.vcf", OCH));

    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/mch.vcf", MCH));
    EXPECT_FALSE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/pb.vcf", MCH));

    EXPECT_TRUE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/cch.vcf", CCH));
    EXPECT_FALSE(pbapPseObexServerTest_->IsNameMatchTarget("telecom/pb.vcf", CCH));
    HILOGI("PbapPse_ModuleTest_IsNameMatchTarget_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_OnConnect_00100
 * @tc.name: OnConnect
 * @tc.desc: OnConnect
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_OnConnect_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_OnConnect_00100 start");
    ObexHeader request;
    ObexHeader reply;
    EXPECT_EQ(pbapPseObexServerTest_->OnConnect(request, reply), ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE);

    const std::vector<uint8_t> targetUuid1 = {
        0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8,
        0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a};
    request.SetHeader(TARGET, targetUuid1);
    EXPECT_EQ(pbapPseObexServerTest_->OnConnect(request, reply), ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE);

    const std::vector<uint8_t> targetUuid2 = {
        0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8,
        0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x67};
    request.SetHeader(TARGET, targetUuid2);
    EXPECT_EQ(pbapPseObexServerTest_->OnConnect(request, reply), ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE);

    const std::vector<uint8_t> targetUuid3 = {
        0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8,
        0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    request.SetHeader(TARGET, targetUuid3);
    EXPECT_EQ(pbapPseObexServerTest_->OnConnect(request, reply), ResponseCodes::OBEX_HTTP_OK);
    HILOGI("PbapPse_ModuleTest_OnConnect_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_OnSetPath_00100
 * @tc.name: OnSetPath
 * @tc.desc: OnSetPath
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_OnSetPath_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_OnSetPath_00100 start");
    ObexHeader request;
    ObexHeader reply;
    pbapPseObexServerTest_->currentPath_ = "";
    EXPECT_EQ(pbapPseObexServerTest_->OnSetPath(request, reply, true, false), ResponseCodes::OBEX_HTTP_OK);

    pbapPseObexServerTest_->currentPath_ = "/telecom/pb";
    EXPECT_EQ(pbapPseObexServerTest_->OnSetPath(request, reply, true, false), ResponseCodes::OBEX_HTTP_OK);
    EXPECT_EQ(pbapPseObexServerTest_->OnSetPath(request, reply, false, false), ResponseCodes::OBEX_HTTP_OK);

    pbapPseObexServerTest_->currentPath_ = "/telecob";
    request.SetHeader(NAME, namePb);
    EXPECT_EQ(pbapPseObexServerTest_->OnSetPath(request, reply, false, false), ResponseCodes::OBEX_HTTP_NOT_FOUND);
    EXPECT_EQ(pbapPseObexServerTest_->OnSetPath(request, reply, false, true), ResponseCodes::OBEX_HTTP_FORBIDDEN);
    HILOGI("PbapPse_ModuleTest_OnSetPath_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_OnGet_00100
 * @tc.name: OnGet
 * @tc.desc: OnGet
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_OnGet_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_OnGet_00100 start");
    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> output;
    bool sendBodyHeader;
    EXPECT_EQ(pbapPseObexServerTest_->OnGet(request, reply, output, sendBodyHeader),
        ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE);
    HILOGI("PbapPse_ModuleTest_OnGet_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_IsLegalPath_00100
 * @tc.name: IsLegalPath
 * @tc.desc: IsLegalPath
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_IsLegalPath_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_IsLegalPath_00100 start");
    std::string path = "";
    EXPECT_TRUE(pbapPseObexServerTest_->IsLegalPath(path));

    path = "/telecom/pb";
    EXPECT_TRUE(pbapPseObexServerTest_->IsLegalPath(path));

    path = "/telecom/pd";
    EXPECT_FALSE(pbapPseObexServerTest_->IsLegalPath(path));
    HILOGI("PbapPse_ModuleTest_IsLegalPath_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_PullVcardListing_00100
 * @tc.name: PullVcardListing
 * @tc.desc: PullVcardListing
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_PullVcardListing_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_PullVcardListing_00100 start");
    ObexHeader reply;
    std::vector<uint8_t> output;
    bool sendBodyHeader;
    PbapPseAppParam appParam;
    appParam.searchAttribute_ = SEARCH_ATTR_SOUND;
    EXPECT_EQ(pbapPseObexServerTest_->PullVcardListing(appParam, reply, output, sendBodyHeader),
        ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED);

    appParam.searchAttribute_ = 3;
    EXPECT_EQ(pbapPseObexServerTest_->PullVcardListing(appParam, reply, output, sendBodyHeader),
        ResponseCodes::OBEX_HTTP_PRECON_FAILED);
    
    appParam.searchAttribute_ = SEARCH_ATTR_NUMBER;
    appParam.order_ = ORDER_BY_PHONETIC;
    EXPECT_EQ(pbapPseObexServerTest_->PullVcardListing(appParam, reply, output, sendBodyHeader),
        ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED);

    appParam.order_ = 3;
    EXPECT_EQ(pbapPseObexServerTest_->PullVcardListing(appParam, reply, output, sendBodyHeader),
        ResponseCodes::OBEX_HTTP_PRECON_FAILED);

    appParam.order_ = ORDER_BY_INDEX;
    appParam.contentType_ = PbapContentType::PHONEBOOK;
    EXPECT_EQ(pbapPseObexServerTest_->PullVcardListing(appParam, reply, output, sendBodyHeader),
        ResponseCodes::OBEX_HTTP_OK);
    HILOGI("PbapPse_ModuleTest_PullVcardListing_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_SendVcardListingXml_00100
 * @tc.name: SendVcardListingXml
 * @tc.desc: SendVcardListingXml
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_SendVcardListingXml_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_SendVcardListingXml_00100 start");
    ObexHeader reply;
    std::vector<uint8_t> output;
    PbapPseAppParam appParam;
    appParam.contentType_ = PbapContentType::PHONEBOOK;
    appParam.searchAttribute_ = SEARCH_ATTR_SOUND;
    appParam.maxListCount_ = 0;
    EXPECT_EQ(pbapPseObexServerTest_->SendVcardListingXml(reply, appParam, output),
        ResponseCodes::OBEX_HTTP_OK);
    appParam.maxListCount_ = 5;
    EXPECT_EQ(pbapPseObexServerTest_->SendVcardListingXml(reply, appParam, output),
        ResponseCodes::OBEX_HTTP_OK);
    appParam.contentType_ = PbapContentType::FAVORITES;
    appParam.searchAttribute_ = SEARCH_ATTR_NUMBER;
    EXPECT_EQ(pbapPseObexServerTest_->SendVcardListingXml(reply, appParam, output),
        ResponseCodes::OBEX_HTTP_OK);

    appParam.contentType_ = PbapContentType::COMBINED_CALL_HISTORY;
    appParam.listStartOffset_ = 2;
    EXPECT_EQ(pbapPseObexServerTest_->SendVcardListingXml(reply, appParam, output),
        ResponseCodes::OBEX_HTTP_OK);
    HILOGI("PbapPse_ModuleTest_SendVcardListingXml_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_CreateNumSearchType_00100
 * @tc.name: CreateNumSearchType
 * @tc.desc: CreateNumSearchType
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_CreateNumSearchType_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_CreateNumSearchType_00100 start");
    std::vector<std::string> nameList = {"ceshi"};
    std::vector<std::string> names = {"ceshi"};
    std::vector<std::string> selectNameList;
    std::vector<uint32_t> selectNameListPos;
    pbapPseObexServerTest_->CreateNumSearchType(nameList, names, selectNameList, selectNameListPos);
    EXPECT_EQ(selectNameList.size(), 1);

    nameList = {"ceshi1"};
    selectNameList.clear();
    selectNameListPos.clear();
    pbapPseObexServerTest_->CreateNumSearchType(nameList, names, selectNameList, selectNameListPos);
    EXPECT_EQ(selectNameList.size(), 0);
    HILOGI("PbapPse_ModuleTest_CreateNumSearchType_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_CreateOtherSearchType_00100
 * @tc.name: CreateOtherSearchType
 * @tc.desc: CreateOtherSearchType
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_CreateOtherSearchType_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_CreateOtherSearchType_00100 start");
    PbapPseAppParam appParam;
    std::vector<std::string> nameList = {"ceshi"};
    std::vector<std::string> selectNameList;
    std::vector<uint32_t> selectNameListPos;
    pbapPseObexServerTest_->CreateOtherSearchType(appParam, nameList, selectNameList, selectNameListPos);
    EXPECT_EQ(selectNameList.size(), 0);

    appParam.searchValue_ = "ceshi";
    pbapPseObexServerTest_->CreateOtherSearchType(appParam, nameList, selectNameList, selectNameListPos);
    HILOGI("PbapPse_ModuleTest_CreateOtherSearchType_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetVcardEntryIndex_00100
 * @tc.name: GetVcardEntryIndex
 * @tc.desc: GetVcardEntryIndex
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_GetVcardEntryIndex_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_GetVcardEntryIndex_00100 start");
    std::string name = "";
    int32_t index;
    EXPECT_FALSE(pbapPseObexServerTest_->GetVcardEntryIndex(name, index));

    name = "asdasdadasd";
    EXPECT_FALSE(pbapPseObexServerTest_->GetVcardEntryIndex(name, index));

    name = "5.vcf";
    EXPECT_TRUE(pbapPseObexServerTest_->GetVcardEntryIndex(name, index));
    EXPECT_EQ(index, 5);
    HILOGI("PbapPse_ModuleTest_GetVcardEntryIndex_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_CalcCallLogParam_00100
 * @tc.name: CalcCallLogParam
 * @tc.desc: CalcCallLogParam
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_CalcCallLogParam_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_CalcCallLogParam_00100 start");
    PbapPseAppParam appParam;
    appParam.maxListCount_ = 10;
    appParam.listStartOffset_ = 0;
    appParam.contentType_ = PbapContentType::PHONEBOOK;
    CallLogSelectParam param;
    param.pbSize = 5;
    EXPECT_TRUE(pbapPseObexServerTest_->CalcCallLogParam(appParam, param));

    appParam.contentType_ = PbapContentType::COMBINED_CALL_HISTORY;
    appParam.listStartOffset_ = 6;
    EXPECT_FALSE(pbapPseObexServerTest_->CalcCallLogParam(appParam, param));
    HILOGI("PbapPse_ModuleTest_CalcCallLogParam_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_CheckNameIncludeVcf_00100
 * @tc.name: CheckNameIncludeVcf
 * @tc.desc: CheckNameIncludeVcf
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_CheckNameIncludeVcf_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_CheckNameIncludeVcf_00100 start");
    std::string name = "";
    EXPECT_TRUE(pbapPseObexServerTest_->CheckNameIncludeVcf(name));
    name = "pb";
    EXPECT_TRUE(pbapPseObexServerTest_->CheckNameIncludeVcf(name));
    name = "pb.vcf";
    EXPECT_TRUE(pbapPseObexServerTest_->CheckNameIncludeVcf(name));
    name = "pb.vdf";
    EXPECT_FALSE(pbapPseObexServerTest_->CheckNameIncludeVcf(name));
    HILOGI("PbapPse_ModuleTest_CheckNameIncludeVcf_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetContentTypeForNoNameOrVcard_00100
 * @tc.name: GetContentTypeForNoNameOrVcard
 * @tc.desc: GetContentTypeForNoNameOrVcard
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_GetContentTypeForNoNameOrVcard_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_GetContentTypeForNoNameOrVcard_00100 start");
    int32_t contentType;
    pbapPseObexServerTest_->currentPath_ = "";
    std::string type = TYPE_PB;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(false, type, contentType),
        ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE);

    pbapPseObexServerTest_->currentPath_ = PB_PATH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(true, type, contentType),
        ResponseCodes::OBEX_HTTP_OK);
    
    pbapPseObexServerTest_->currentPath_ = FAV_PATH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(true, type, contentType),
        ResponseCodes::OBEX_HTTP_OK);
    
    pbapPseObexServerTest_->currentPath_ = ICH_PATH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(true, type, contentType),
        ResponseCodes::OBEX_HTTP_OK);

    pbapPseObexServerTest_->currentPath_ = OCH_PATH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(true, type, contentType),
        ResponseCodes::OBEX_HTTP_OK);

    pbapPseObexServerTest_->currentPath_ = MCH_PATH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(true, type, contentType),
        ResponseCodes::OBEX_HTTP_OK);

    pbapPseObexServerTest_->currentPath_ = CCH_PATH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(true, type, contentType),
        ResponseCodes::OBEX_HTTP_OK);
    
    pbapPseObexServerTest_->currentPath_ = TELECOM_PATH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(true, type, contentType),
        ResponseCodes::OBEX_HTTP_OK);
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(false, type, contentType),
        ResponseCodes::OBEX_HTTP_OK);
    type = TYPE_LISTING;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeForNoNameOrVcard(false, type, contentType),
        ResponseCodes::OBEX_HTTP_NOT_FOUND);

    HILOGI("PbapPse_ModuleTest_GetContentTypeForNoNameOrVcard_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetContentTypeAndCounter_00100
 * @tc.name: GetContentTypeAndCounter
 * @tc.desc: GetContentTypeAndCounter
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_GetContentTypeAndCounter_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_HandleAppParaForResponse_00100 start");
    PbapPseAppParam appParam;
    std::string name = PB;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeAndCounter(name, appParam), ResponseCodes::OBEX_HTTP_OK);

    name = FAV;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeAndCounter(name, appParam), ResponseCodes::OBEX_HTTP_OK);

    name = ICH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeAndCounter(name, appParam), ResponseCodes::OBEX_HTTP_OK);

    name = OCH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeAndCounter(name, appParam), ResponseCodes::OBEX_HTTP_OK);

    name = MCH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeAndCounter(name, appParam), ResponseCodes::OBEX_HTTP_OK);

    name = CCH;
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeAndCounter(name, appParam), ResponseCodes::OBEX_HTTP_OK);

    name = "uch";
    EXPECT_EQ(pbapPseObexServerTest_->GetContentTypeAndCounter(name, appParam), ResponseCodes::OBEX_HTTP_NOT_FOUND);
    HILOGI("PbapPse_ModuleTest_GetContentTypeAndCounter_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_HandleAppParaForResponse_00100
 * @tc.name: HandleAppParaForResponse
 * @tc.desc: HandleAppParaForResponse
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_HandleAppParaForResponse_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_HandleAppParaForResponse_00100 start");
    ObexHeader reply;
    pbapPseObexServerTest_->needPhonebookSize_ = false;
    pbapPseObexServerTest_->needNewMissedCallsNum_ = false;
    EXPECT_EQ(pbapPseObexServerTest_->HandleAppParaForResponse(0, reply), -1);

    pbapPseObexServerTest_->needNewMissedCallsNum_ = true;
    EXPECT_EQ(pbapPseObexServerTest_->HandleAppParaForResponse(0, reply), -1);

    pbapPseObexServerTest_->needPhonebookSize_ = true;
    pbapPseObexServerTest_->needNewMissedCallsNum_ = false;
    EXPECT_EQ(pbapPseObexServerTest_->HandleAppParaForResponse(0, reply), ResponseCodes::OBEX_HTTP_OK);

    pbapPseObexServerTest_->needPhonebookSize_ = true;
    pbapPseObexServerTest_->needNewMissedCallsNum_ = true;
    EXPECT_EQ(pbapPseObexServerTest_->HandleAppParaForResponse(0, reply), ResponseCodes::OBEX_HTTP_OK);
    HILOGI("PbapPse_ModuleTest_HandleAppParaForResponse_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_SetConnectionId_00100
 * @tc.name: SetConnectionId
 * @tc.desc: SetConnectionId
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_SetConnectionId_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_SetConnectionId_00100 start");
    pbapPseObexServerTest_->SetConnectionId(1);
    EXPECT_EQ(pbapPseObexServerTest_->GetConnectionId(), 1);
    HILOGI("PbapPse_ModuleTest_SetConnectionId_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_OnDisconnect_00100
 * @tc.name: OnDisconnect
 * @tc.desc: OnDisconnect
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_OnDisconnect_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_OnDisconnect_00100 start");
    ObexHeader request;
    ObexHeader reply;
    int res = pbapPseObexServerTest_->OnDisconnect(request, reply);
    EXPECT_EQ(res, OBEX_HTTP_OK);
    HILOGI("PbapPse_ModuleTest_OnDisconnect_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_OnAbort_00100
 * @tc.name: OnAbort
 * @tc.desc: OnAbort
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_OnAbort_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_OnAbort_00100 start");
    ObexHeader request;
    ObexHeader reply;
    int res = pbapPseObexServerTest_->OnAbort(request, reply);
    EXPECT_EQ(res, OBEX_HTTP_OK);
    HILOGI("PbapPse_ModuleTest_OnAbort_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_OnPut_00100
 * @tc.name: OnPut
 * @tc.desc: OnPut
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_OnPut_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_OnPut_00100 start");
    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> input;
    bool requestFinished;
    int res = pbapPseObexServerTest_->OnPut(request, reply, input, requestFinished);
    EXPECT_EQ(res, OBEX_HTTP_BAD_REQUEST);
    HILOGI("PbapPse_ModuleTest_OnPut_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_OnDelete_00100
 * @tc.name: OnDelete
 * @tc.desc: OnDelete
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_OnDelete_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_OnDelete_00100 start");
    ObexHeader request;
    ObexHeader reply;
    int res = pbapPseObexServerTest_->OnDelete(request, reply);
    EXPECT_EQ(res, OBEX_HTTP_BAD_REQUEST);
    pbapPseObexServerTest_->OnClose(true);
    HILOGI("PbapPse_ModuleTest_OnDelete_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_PullVcardEntry_00100
 * @tc.name: PullVcardEntry
 * @tc.desc: PullVcardEntry
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_PullVcardEntry_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_PullVcardEntry_00100 start");
    PbapPseAppParam appParam;
    ObexHeader reply;
    std::string name = "test";
    std::vector<uint8_t> output;
    bool sendBodyHeader = false;
    int res = pbapPseObexServerTest_->PullVcardEntry(appParam, reply, name, output, sendBodyHeader);
    EXPECT_EQ(res, OBEX_HTTP_NOT_ACCEPTABLE);
    HILOGI("PbapPse_ModuleTest_PullVcardEntry_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_PullVcardEntry_00101
 * @tc.name: PullVcardEntry
 * @tc.desc: PullVcardEntry
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_PullVcardEntry_00101, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_PullVcardEntry_00101 start");
    PbapPseAppParam appParam;
    ObexHeader reply;
    std::string name = "5.vcf";
    std::vector<uint8_t> output;
    bool sendBodyHeader = false;
    int res = pbapPseObexServerTest_->PullVcardEntry(appParam, reply, name, output, sendBodyHeader);
    EXPECT_EQ(res, OBEX_HTTP_OK);
    HILOGI("PbapPse_ModuleTest_PullVcardEntry_00101 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_PullPhonebook_00100
 * @tc.name: PullPhonebook
 * @tc.desc: PullPhonebook
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_PullPhonebook_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_PullPhonebook_00100 start");
    PbapPseAppParam appParam;
    ObexHeader reply;
    std::string name = "";
    std::vector<uint8_t> output;
    bool sendBodyHeader = false;
    int res = pbapPseObexServerTest_->PullPhonebook(appParam, reply, name, output, sendBodyHeader);
    EXPECT_EQ(res, OBEX_HTTP_OK);
    HILOGI("PbapPse_ModuleTest_PullPhonebook_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_PullPhonebook_00101
 * @tc.name: PullPhonebook
 * @tc.desc: PullPhonebook
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_PullPhonebook_00101, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_PullPhonebook_00101 start");
    PbapPseAppParam appParam;
    ObexHeader reply;
    std::string name = "pb.vdf";
    std::vector<uint8_t> output;
    bool sendBodyHeader = false;
    int res = pbapPseObexServerTest_->PullPhonebook(appParam, reply, name, output, sendBodyHeader);
    EXPECT_EQ(res, OBEX_HTTP_NOT_ACCEPTABLE);
    HILOGI("PbapPse_ModuleTest_PullPhonebook_00101 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_AppendAppHeaderData_00100
 * @tc.name: AppendAppHeaderData
 * @tc.desc: AppendAppHeaderData
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_AppendAppHeaderData_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_AppendAppHeaderData_00100 start");
    uint8_t tag = 1;
    uint8_t length = 2;
    std::vector<uint8_t> data;
    std::vector<uint8_t> retValue;
    pbapPseObexServerTest_->AppendAppHeaderData(tag, length, data, retValue);
    EXPECT_EQ(retValue.size(), 2);
    HILOGI("PbapPse_ModuleTest_AppendAppHeaderData_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_CheckAccessAuthorization_00100
 * @tc.name: CheckAccessAuthorization
 * @tc.desc: CheckAccessAuthorization
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_CheckAccessAuthorization_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_CheckAccessAuthorization_00100 start");
    EXPECT_FALSE(pbapPseObexServerTest_->CheckAccessAuthorization());
    HILOGI("PbapPse_ModuleTest_CheckAccessAuthorization_00100 end");
}

/*
 * @tc.number: PbapPse_ModuleTest_GetRawContactIdByNameIndex_00100
 * @tc.name: GetRawContactIdByNameIndex
 * @tc.desc: GetRawContactIdByNameIndex
*/
HWTEST_F(PbapPseObexServerTest, PbapPse_ModuleTest_GetRawContactIdByNameIndex_00100, TestSize.Level0)
{
    HILOGI("PbapPse_ModuleTest_GetRawContactIdByNameIndex_00100 start");
    int32_t index = 0;
    EXPECT_EQ(pbapPseObexServerTest_->GetRawContactIdByNameIndex(index, ORDER_BY_ALPHABETICAL), 0);

    index = 1;
    EXPECT_EQ(pbapPseObexServerTest_->GetRawContactIdByNameIndex(index, ORDER_BY_ALPHABETICAL), 0);
    HILOGI("PbapPse_ModuleTest_GetRawContactIdByNameIndex_00100 end");
}

}  // namespace Bluetooth
}  // namespace OHOS
