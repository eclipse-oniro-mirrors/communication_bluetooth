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

#include "obex_server_actions_fuzzer.h"
#include "permission_manager.h"
#include "obex_server_actions.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "btcommon/timer_manager.h"

using namespace std;
using namespace OHOS::Bluetooth;
const int FUZZ_DATA_LEN = 6;
namespace OHOS {
    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        if ((data == nullptr) || (size < FUZZ_DATA_LEN)) {
            return false;
        }
        std::string address("11:22:33:44:55:66");
        int32_t socketFd = 51;
        std::shared_ptr<bluetooth::ObexSocketDevice> socketDevice = std::make_shared<bluetooth::ObexSocketDevice>(
            socketFd, bluetooth::MAX_PACKET_SIZE, bluetooth::MAX_PACKET_SIZE,
            bluetooth::SocketType::TYPE_L2CAP, address);
        std::shared_ptr<bluetooth::ObexServerTransport> obexTransport
            = std::make_shared<bluetooth::ObexServerTransport>(socketDevice);
        std::unique_ptr<bluetooth::ObexServerGetAction> getAction =
            std::make_unique<bluetooth::ObexServerGetAction>(obexTransport, nullptr);

        uint8_t responseCode = 0;
        uint8_t requestType = 0;
        getAction->SendReply(responseCode);
        getAction->HandleRequest(requestType);
        getAction->HandleClientGetContinue();
        getAction->HandleNextGetOp();
        getAction->NeedSubcontractPackage();
        getAction->SendSubcontractPackage();

        std::vector<uint8_t> headerdata;
        for (size_t i = 0; i < size; i++) {
            headerdata.push_back(data[i]);
        }
        getAction->SendBigHeader(headerdata);

        uint8_t code = 0;
        uint32_t sendLength = 0;
        getAction->SendPackage(code, sendLength);

        std::vector<uint8_t> sendData;
        for (size_t i = 0; i < size; i++) {
            sendData.push_back(data[i]);
        }
        uint32_t headSize = 0;
        getAction->FillSendData(code, sendLength, headSize, sendData);
        getAction->ReadRequestPackageLength();
        bool isTrue = true;
        getAction->HandleSendReplyContinue(isTrue);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    utility::CloseAllTimer();
    return 0;
}

