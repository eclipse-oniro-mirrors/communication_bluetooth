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
#undef private
#define private public

#include "pbap_pse_obex_server_fuzzer.h"
#include "permission_manager.h"
#include "pbap_pse_obex_server.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "btcommon/timer_manager.h"

using namespace std;
using namespace OHOS::bluetooth;
const int FUZZ_DATA_LEN = 10;
const int OTHER_BRANCHES = 3;
namespace OHOS {

    void PbapPseObexServerFuzzTest(std::shared_ptr<PbapPseObexServer> pbapPseObexServer, ObexHeader& reply,
        const uint8_t* data, std::vector<uint8_t> output, bool sendBodyHeader)
    {
        PbapPseAppParam appParam;
        appParam.searchAttribute_ = SEARCH_ATTR_SOUND;
        pbapPseObexServer->PullVcardListing(appParam, reply, output, sendBodyHeader);
        appParam.searchAttribute_ = OTHER_BRANCHES;
        pbapPseObexServer->PullVcardListing(appParam, reply, output, sendBodyHeader);
        appParam.searchAttribute_ = SEARCH_ATTR_NUMBER;
        pbapPseObexServer->PullVcardListing(appParam, reply, output, sendBodyHeader);
        appParam.order_ = ORDER_BY_PHONETIC;
        pbapPseObexServer->PullVcardListing(appParam, reply, output, sendBodyHeader);
        appParam.order_ = OTHER_BRANCHES;
        pbapPseObexServer->PullVcardListing(appParam, reply, output, sendBodyHeader);
        appParam.order_ = ORDER_BY_ALPHANUMERIC;
        pbapPseObexServer->PullVcardListing(appParam, reply, output, sendBodyHeader);
        appParam.contentType_ = PbapContentType::PHONEBOOK;
        pbapPseObexServer->SendVcardListingXml(reply, appParam, output);
        appParam.searchAttribute_ = SEARCH_ATTR_NUMBER;
        std::string xml = "testXml";
        pbapPseObexServer->CreatePhoneBookList(appParam, xml);
        std::vector<std::string> nameList = {"testName"};
        std::vector<std::string> selectNameList;
        std::vector<uint32_t> selectNameListPos;
        pbapPseObexServer->CreateNumSearchType(nameList, nameList, selectNameList, selectNameListPos);
        appParam.searchValue_ = "testValue";
        pbapPseObexServer->CreateOtherSearchType(appParam, nameList, selectNameList, selectNameListPos);
        pbapPseObexServer->CreateCallHistoryList(appParam, xml);
        std::string nameStr = "testName";
        pbapPseObexServer->PullVcardEntry(appParam, reply, nameStr, output, sendBodyHeader);
        int32_t index = static_cast<int32_t>(data[0]);
        pbapPseObexServer->GetVcardEntryIndex(nameStr, index);
        pbapPseObexServer->PullPhonebook(appParam, reply, nameStr, output, sendBodyHeader);
        CallLogSelectParam param;
        appParam.contentType_ = PbapContentType::PHONEBOOK;
        pbapPseObexServer->CalcCallLogParam(appParam, param);
        pbapPseObexServer->CheckNameIncludeVcf(nameStr);
        int32_t size = static_cast<int32_t>(data[1]);
        pbapPseObexServer->HandleAppParaForResponse(size, reply);
        pbapPseObexServer->GetContentTypeAndCounter(nameStr, appParam);
    }

    void PbapPseObexServerFuzzTest2(std::shared_ptr<PbapPseObexServer> pbapPseObexServer,
        ObexHeader& reply, std::string listingStr)
    {
        bool validName = true;
        int contentType = PbapContentType::PHONEBOOK;
        pbapPseObexServer->currentPath_ = PB_PATH;
        pbapPseObexServer->GetContentTypeForNoNameOrVcard(validName, listingStr, contentType);
        pbapPseObexServer->currentPath_ = PB_PATH;
        pbapPseObexServer->GetContentTypeForNoNameOrVcard(validName, listingStr, contentType);
        pbapPseObexServer->currentPath_ = PB_PATH;
        pbapPseObexServer->GetContentTypeForNoNameOrVcard(validName, listingStr, contentType);
        pbapPseObexServer->currentPath_ = PB_PATH;
        pbapPseObexServer->GetContentTypeForNoNameOrVcard(validName, listingStr, contentType);
        pbapPseObexServer->currentPath_ = PB_PATH;
        pbapPseObexServer->GetContentTypeForNoNameOrVcard(validName, listingStr, contentType);
        pbapPseObexServer->currentPath_ = PB_PATH;
        pbapPseObexServer->GetContentTypeForNoNameOrVcard(validName, listingStr, contentType);
        pbapPseObexServer->currentPath_ = PB_PATH;
        pbapPseObexServer->GetContentTypeForNoNameOrVcard(validName, listingStr, contentType);
        pbapPseObexServer->needPhonebookSize_ = true;
        pbapPseObexServer->needNewMissedCallsNum_ = true;
        pbapPseObexServer->needNewMissedCallsNum_ = true;
        std::vector<uint8_t> retValue = {1, 2};
        pbapPseObexServer->AppendNewMissdCallData(retValue);
        uint8_t tag = TRIPLET_TAGID_PHONEBOOKSIZE;
        uint8_t length = TRIPLET_LENGTH_PHONEBOOKSIZE;
        std::vector<uint8_t> headerData = {1, 2};
        pbapPseObexServer->AppendAppHeaderData(tag, length, headerData, retValue);
        pbapPseObexServer->CheckAccessAuthorization();
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        if ((data == nullptr) || (size < FUZZ_DATA_LEN)) {
            return false;
        }
        std::string device = "11:22:33:44:55:66";
        std::shared_ptr<PbapPseObexServer> pbapPseObexServer = std::make_shared<PbapPseObexServer>(device, nullptr);
        ObexHeader request;
        ObexHeader reply;
        const std::vector<uint8_t> targetHeaderUuid = {0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8,
            0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
        request.SetHeader(TARGET, targetHeaderUuid);
        pbapPseObexServer->OnConnect(request, reply);
        std::vector<uint8_t> output = {1, 2};
        bool sendBodyHeader = true;
        std::string vcardStr = "x-bt/vcard";
        std::vector<uint8_t> vcard(vcardStr.begin(), vcardStr.end());
        request.SetHeader(TYPE, vcard);
        pbapPseObexServer->OnGet(request, reply, output, sendBodyHeader);
        std::string listingStr = "x-bt/vcard-listing";
        std::vector<uint8_t> listing(listingStr.begin(), listingStr.end());
        request.SetHeader(TYPE, listing);
        std::vector<uint8_t> name;
        for (size_t i = 0; i < size; i++) {
            name.push_back(data[i]);
        }
        request.SetHeader(NAME, name);
        pbapPseObexServer->OnGet(request, reply, output, sendBodyHeader);
        pbapPseObexServer->IsLegalPath("");
        pbapPseObexServer->IsLegalPath("/telecom/pb");
        pbapPseObexServer->IsNameMatchTarget("", "pb");
        pbapPseObexServer->IsNameMatchTarget(".vcf", "pb");
        pbapPseObexServer->IsNameMatchTarget("pb.vcf", "pb");
        PbapPseObexServerFuzzTest(pbapPseObexServer, reply, data, output, sendBodyHeader);
        PbapPseObexServerFuzzTest2(pbapPseObexServer, reply, listingStr);
        pbapPseObexServer->OnDelete(request, reply);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    utility::CloseAllTimer();
    return 0;
}
