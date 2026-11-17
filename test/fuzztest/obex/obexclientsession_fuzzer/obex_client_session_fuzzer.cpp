/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "obex_client_session_fuzzer.h"
#include "permission_manager.h"
#include "obex_client_session.h"
#include "obex_header.h"
#include "obex_server_transport.h"
#include "obex_body.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {

static std::shared_ptr<bluetooth::ObexClientSession> CreateSession()
{
    std::string address("11:22:33:44:55:66");
    int32_t socketFd = 51;
    std::shared_ptr<bluetooth::ObexSocketDevice> socketDevice = std::make_shared<bluetooth::ObexSocketDevice>(
        socketFd, bluetooth::MAX_PACKET_SIZE, bluetooth::MAX_PACKET_SIZE,
        bluetooth::SocketType::TYPE_L2CAP, address);
    std::shared_ptr<bluetooth::ObexServerTransport> transport =
        std::make_shared<bluetooth::ObexServerTransport>(socketDevice);
    return std::make_shared<bluetooth::ObexClientSession>(transport, nullptr);
}

void TestSetPath(FuzzedDataProvider &provider)
{
    auto session = CreateSession();
    uint8_t flags = provider.ConsumeIntegral<uint8_t>();

    std::u16string path;
    int pathLen = provider.ConsumeIntegralInRange<int>(0, 32);
    for (int i = 0; i < pathLen; i++) {
        path.push_back(static_cast<char16_t>(provider.ConsumeIntegral<uint16_t>()));
    }

    bluetooth::ObexHeader resp;
    session->SetPath(flags, path, resp);
}

void TestGet(FuzzedDataProvider &provider)
{
    auto session = CreateSession();

    bluetooth::ObexHeader req;
    uint32_t connectionId = provider.ConsumeIntegral<uint32_t>();
    req.SetConnectionId(connectionId);

    std::string name = provider.ConsumeRandomLengthString(64);
    req.SetName(name);

    bluetooth::ObexHeader resp;
    session->Get(req, resp, nullptr);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);
    TestSetPath(provider);
    TestGet(provider);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
