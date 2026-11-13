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
#undef private
#define private public

#include "mapmseobexserver_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "bluetooth_log.h"
#include "obex_header.h"
#include "map_mse_obex_server.h"
#include "securec.h"

using namespace std;
using namespace OHOS::bluetooth;
const int FUZZ_MAX_NAME_LEN = 10;
const std::vector<uint8_t> MAP_TARGET = {
    0xBB, 0x58, 0x2B, 0x40, 0x42, 0x0C, 0x11, 0xDB, 0xB0, 0xDE, 0x08, 0x00, 0x20, 0x0C, 0x9A, 0x66};
const std::string TYPE_GET_FOLDER_LISTING = "x-obex/folder-listing";
const std::string TYPE_GET_MESSAGE_LISTING = "x-bt/MAP-msg-listing";
const std::string TYPE_GET_CONVO_LISTING = "x-bt/MAP-convo-listing";
const std::string TYPE_MESSAGE = "x-bt/message";
const std::string TYPE_SET_MESSAGE_STATUS = "x-bt/messageStatus";
const std::string TYPE_SET_NOTIFICATION_REGISTRATION = "x-bt/MAP-NotificationRegistration";
const std::string TYPE_MESSAGE_UPDATE = "x-bt/MAP-messageUpdate";
const std::string TYPE_GET_MAS_INSTANCE_INFORMATION = "x-bt/MASInstanceInformation";
const std::string TYPE_SET_OWNER_STATUS = "x-bt/participant";
const std::string TYPE_SET_NOTIFICATION_FILTER = "x-bt/MAP-notification-filter";
namespace OHOS {

void TestMapMseObexServer(std::shared_ptr<MapMseObexServer> obexServer,
    ObexHeader &request, ObexHeader &reply, const uint8_t* data, size_t size)
{
    std::vector<uint8_t> put;
    for (size_t i = 0; i < size; i++) {
        put.push_back(data[i]);
    }
    bool sendBodyHeader = true;
    std::string typeStr = TYPE_GET_FOLDER_LISTING;
    std::vector<uint8_t> type = std::vector<uint8_t>(typeStr.begin(), typeStr.end());
    request.SetHeader(TYPE, type);
    obexServer->OnGet(request, reply, put, sendBodyHeader);
    typeStr = TYPE_GET_FOLDER_LISTING;
    type = std::vector<uint8_t>(typeStr.begin(), typeStr.end());
    request.SetHeader(TYPE, type);
    obexServer->OnGet(request, reply, put, sendBodyHeader);
    typeStr = TYPE_GET_MESSAGE_LISTING;
    type = std::vector<uint8_t>(typeStr.begin(), typeStr.end());
    request.SetHeader(TYPE, type);
    obexServer->OnGet(request, reply, put, sendBodyHeader);
    typeStr = TYPE_GET_CONVO_LISTING;
    type = std::vector<uint8_t>(typeStr.begin(), typeStr.end());
    request.SetHeader(TYPE, type);
    obexServer->OnGet(request, reply, put, sendBodyHeader);
    typeStr = TYPE_GET_MAS_INSTANCE_INFORMATION;
    type = std::vector<uint8_t>(typeStr.begin(), typeStr.end());
    request.SetHeader(TYPE, type);
    obexServer->OnGet(request, reply, put, sendBodyHeader);
    typeStr = TYPE_MESSAGE;
    type = std::vector<uint8_t>(typeStr.begin(), typeStr.end());
    request.SetHeader(TYPE, type);
    obexServer->OnPut(request, reply, put, sendBodyHeader);
    obexServer->OnGet(request, reply, put, sendBodyHeader);
    typeStr = TYPE_SET_NOTIFICATION_REGISTRATION;
    type = std::vector<uint8_t>(typeStr.begin(), typeStr.end());
    request.SetHeader(TYPE, type);
    obexServer->OnPut(request, reply, put, sendBodyHeader);
    typeStr = TYPE_SET_MESSAGE_STATUS;
    type = std::vector<uint8_t>(typeStr.begin(), typeStr.end());
    request.SetHeader(TYPE, type);
    obexServer->OnPut(request, reply, put, sendBodyHeader);
    std::string folderName = "testFolderName";
    obexServer->GetFolderElementFromName(folderName);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    if ((data == nullptr) || (size < FUZZ_MAX_NAME_LEN)) {
        return false;
    }
    std::shared_ptr<MapMseMasInstance> masInstance = nullptr;
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    std::shared_ptr<MapMseObexServer> obexServer = std::make_shared<MapMseObexServer>(masInstance, mnsClient, nullptr);
    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> name{'0', '1', '2', '3'};
    request.SetHeader(NAME, name);
    obexServer->OnConnect(request, reply);
    std::vector<uint8_t> invalidTarget(MAP_TARGET.begin(), MAP_TARGET.end() - 2);
    request.SetHeader(TARGET, invalidTarget);
    obexServer->OnConnect(request, reply);
    std::vector<uint8_t> wrongTarget(MAP_TARGET.begin(), MAP_TARGET.end());
    wrongTarget[7] = 0xFF;
    request.SetHeader(TARGET, invalidTarget);
    obexServer->OnConnect(request, reply);
    request.SetHeader(TARGET, MAP_TARGET);
    obexServer->OnConnect(request, reply);
    std::vector<uint8_t> threadedMailKeyVec{0x01, 0x01, 0x01};
    request.SetHeader(THREADED_MAIL_HEADER_ID, threadedMailKeyVec);
    obexServer->OnConnect(request, reply);
    threadedMailKeyVec.push_back(0x01);
    threadedMailKeyVec.push_back(0x01);
    request.SetHeader(THREADED_MAIL_HEADER_ID, threadedMailKeyVec);
    obexServer->OnConnect(request, reply);
    std::vector<uint8_t> who{0x01, 0x02};
    request.SetHeader(WHO, who);
    std::vector<uint8_t> output;
    obexServer->OnConnect(request, reply);
    TestMapMseObexServer(obexServer, request, reply, data, size);
    obexServer->OnDelete(request, reply);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

