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

#include "opp_obex_server_fuzzer.h"
#include "permission_manager.h"
#include "opp_obex_server.h"
#include "opp_service_impl.h"
#include "opp_def.h"
#include "obex_header.h"
#include "obex_server_session.h"
#include "obex_body.h"
#include "obex_def.h"
#include "bluetooth_log.h"
#include "bt_chr_transaction_manager.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::bluetooth;
namespace OHOS {

static const std::string TEST_ADDR = "11:22:33:44:55:66";

// Fuzz OppReceiveFileBodyObject
void TestReceiveFileBodyObject(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    int32_t totalSize = provider.ConsumeIntegralInRange<int32_t>(0, 10000);
    auto bodyObj = std::make_shared<OppReceiveFileBodyObject>(TEST_ADDR, svcImpl, totalSize);

    // Test Write with fuzzed buffer
    size_t bufLen = provider.ConsumeIntegralInRange<size_t>(0, 256);
    std::vector<uint8_t> buf(bufLen);
    if (bufLen > 0) {
        provider.ConsumeData(buf.data(), bufLen);
    }
    size_t writeSize = bodyObj->Write(buf.data(), bufLen);
    (void)writeSize;

    // Test Read
    size_t readSize = bodyObj->Read(buf.data(), bufLen);
    (void)readSize;

    // Test IsReadFinished
    bool isFinished = bodyObj->IsReadFinished();
    (void)isFinished;

    // Test OpenFile with a valid unowned fd (avoid FDSAN from fdopen on system fds)
    FILE *tmpFile = tmpfile();
    if (tmpFile != nullptr) {
        if (bufLen > 0) {
            fwrite(buf.data(), 1, bufLen, tmpFile);
            fflush(tmpFile);
        }
        int fd = dup(fileno(tmpFile));
        fclose(tmpFile);
        if (fd >= 0) {
            bodyObj->OpenFile(fd);
        }
    }

    // Write again after OpenFile
    if (bufLen > 0) {
        bodyObj->Write(buf.data(), bufLen);
    }

    bodyObj->Close();

    // Test Write with nullptr
    bodyObj->Write(nullptr, bufLen);

    // Test Write with zero length
    if (bufLen > 0) {
        bodyObj->Write(buf.data(), 0);
    }
}

// Fuzz OnConnect
// Fuzz OnConnect
// NOTE: OnConnect calls SubscribTransactionChrEvent() which triggers
// CommonEventManager::SubscribeCommonEvent — a cross-DSO call into
// libcesfwk_core.z.so that fails CFI check. Skip OnConnect to avoid crash.
void TestOnConnect(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);
    server->SetObexServerSession(nullptr);
    // Only test that the server object can be constructed and destroyed safely
    (void)provider;
}

// Fuzz OnGet
void TestOnGet(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);

    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> output;
    bool sendBodyHeader = provider.ConsumeBool();
    int ret = server->OnGet(request, reply, output, sendBodyHeader);
    (void)ret;
}

// Fuzz OnPut with various header configurations
void TestOnPut(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);
    server->SetObexServerSession(nullptr);

    ObexHeader request;
    ObexHeader reply;

    // Set fuzzed type
    std::string type = provider.ConsumeRandomLengthString(32);
    if (!type.empty()) {
        std::vector<uint8_t> typeValue(type.begin(), type.end());
        request.SetHeader(TYPE, typeValue);
    }

    // Set fuzzed name
    std::string name = provider.ConsumeRandomLengthString(64);
    request.SetName(name);

    // Set fuzzed length
    uint32_t length = provider.ConsumeIntegral<uint32_t>();
    request.SetLength(length);

    // Set fuzzed body
    uint16_t bodyLen = provider.ConsumeIntegralInRange<uint16_t>(0, 128);
    std::vector<uint8_t> bodyData(bodyLen);
    if (bodyLen > 0) {
        provider.ConsumeData(bodyData.data(), bodyLen);
    }
    request.SetBody(bodyData, bodyLen);

    std::vector<uint8_t> input;
    bool requestFinished = provider.ConsumeBool();
    int ret = server->OnPut(request, reply, input, requestFinished);
    (void)ret;

    // Test with end of body
    ObexHeader request2;
    request2.SetName(provider.ConsumeRandomLengthString(32));
    request2.SetLength(provider.ConsumeIntegral<uint32_t>());
    std::vector<uint8_t> endBody(bodyLen);
    if (bodyLen > 0) {
        provider.ConsumeData(endBody.data(), bodyLen);
    }
    request2.SetEndOfBody(endBody, bodyLen);
    ret = server->OnPut(request2, reply, input, requestFinished);
    (void)ret;
}

// Fuzz OnDelete / OnAbort / OnSetPath
// NOTE: Skip OnDisconnect — it calls PostEvent(OPP_DISCONNECTED_EVT) which
// dispatches an async FFRT task that outlives svcImpl and dereferences the
// freed OppServiceImpl::mutex_ in ProcessDefaultEvent.
void TestSimpleHandlers(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);

    ObexHeader request;
    ObexHeader reply;

    server->OnDelete(request, reply);
    server->OnAbort(request, reply);

    bool backup = provider.ConsumeBool();
    bool create = provider.ConsumeBool();
    server->OnSetPath(request, reply, backup, create);
}

// Fuzz OnClose
// NOTE: OnClose(true) dereferences serverSession_->GetRemoteAddress() without
// null check. Since we set serverSession_ to nullptr to avoid cross-DSO CFI on
// real session destruction, only call OnClose(false) which early-returns.
void TestOnClose(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);
    server->SetObexServerSession(nullptr);

    (void)provider;
    server->OnClose(false);
}

// Fuzz SetConnectionId / GetConnectionId
void TestConnectionId(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);

    int connId = provider.ConsumeIntegral<int>();
    server->SetConnectionId(connId);
    int ret = server->GetConnectionId();
    (void)ret;
}

// Fuzz ReceiveFileHeader with isAllowedOppReadFunc_ set
void TestReceiveFileHeader(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);
    server->SetObexServerSession(nullptr);

    // Set the isAllowedOppReadFunc_ to allow reading
    svcImpl->isAllowedOppReadFunc_ = [](const std::string &a) { return true; };

    ObexHeader request;
    ObexHeader reply;

    std::string type = provider.ConsumeRandomLengthString(32);
    if (!type.empty()) {
        std::vector<uint8_t> typeValue(type.begin(), type.end());
        request.SetHeader(TYPE, typeValue);
    }

    std::string name = provider.ConsumeRandomLengthString(128);
    request.SetName(name);
    request.SetLength(provider.ConsumeIntegral<uint32_t>());

    int ret = server->ReceiveFileHeader(request, reply);
    (void)ret;

    // Test with isAllowedOppReadFunc_ returning false
    svcImpl->isAllowedOppReadFunc_ = [](const std::string &a) { return false; };
    ret = server->ReceiveFileHeader(request, reply);
    (void)ret;
}

// Fuzz ReceiveFileBody with null serverSession
void TestReceiveFileBody(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);
    server->SetObexServerSession(nullptr);

    ObexHeader request;
    ObexHeader reply;
    bool requestFinished = provider.ConsumeBool();

    int ret = server->ReceiveFileBody(request, reply, requestFinished);
    (void)ret;
}

// Fuzz SetObexServerSession
void TestSetObexServerSession(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    std::string addr = TEST_ADDR;
    auto server = std::make_shared<OppObexServer>(addr, svcImpl);

    server->SetObexServerSession(nullptr);
    server->SetObexServerSession(nullptr);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    if ((data == nullptr) || (size < 1)) {
        return false;
    }
    FuzzedDataProvider provider(data, size);

    auto svcImpl = new OppServiceImpl();
    svcImpl->isAllowedOppReadFunc_ = [](const std::string &addr) { return true; };

    TestReceiveFileBodyObject(provider, svcImpl);
    TestOnConnect(provider, svcImpl);
    TestOnGet(provider, svcImpl);
    TestOnPut(provider, svcImpl);
    TestSimpleHandlers(provider, svcImpl);
    TestOnClose(provider, svcImpl);
    TestConnectionId(provider, svcImpl);
    TestReceiveFileHeader(provider, svcImpl);
    TestReceiveFileBody(provider, svcImpl);
    TestSetObexServerSession(provider, svcImpl);

    delete svcImpl;

    // Stop BtChrTransactionManager timer to avoid static destructor order crash:
    // Timer::~Timer() calls TimerManager which may already be destroyed at exit.
    auto &chrMgr = BtChrTransactionManager::GetInstance();
    std::lock_guard<std::mutex> chrLk(chrMgr.counterGroupTimerMutex_);
    if (chrMgr.reportTimer_ != nullptr) {
        chrMgr.reportTimer_->Stop();
        chrMgr.reportTimer_ = nullptr;
    }

    return true;
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
