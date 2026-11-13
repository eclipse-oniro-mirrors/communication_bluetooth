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

#include "opp_obex_client_fuzzer.h"
#include "permission_manager.h"
#include "opp_obex_client.h"
#include "opp_service_impl.h"
#include "opp_def.h"
#include "obex_header.h"
#include "obex_server_transport.h"
#include "obex_client_session.h"
#include "obex_socket_device.h"
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

static std::shared_ptr<OppObexClient> CreateOppObexClient(OppServiceImpl *svcImpl)
{
    auto obexConfig = std::make_shared<ObexClientConfig>();
    obexConfig->addr = TEST_ADDR;
    obexConfig->l2capLocalPsm = OPP_L2CAP_PSM;
    obexConfig->l2capPsm = OPP_L2CAP_PSM;
    obexConfig->rfCommChannel = OPP_RFCOMM_CHANNEL;
    obexConfig->mtu = 1024;
    obexConfig->fileCount = 1;
    obexConfig->isL2capPSM = false;
    obexConfig->isSupportSrm = false;
    obexConfig->isSupportReliableSession = false;
    obexConfig->serviceUUID = "00001105-0000-1000-8000-00805F9B34FB";
    return std::make_shared<OppObexClient>(obexConfig, svcImpl);
}

static int CreateUnownedFd()
{
    FILE *fp = tmpfile();
    if (fp == nullptr) {
        return -1;
    }
    int fd = dup(fileno(fp));
    fclose(fp);
    return fd;
}

static void SetupClientSession(std::shared_ptr<OppObexClient> &client)
{
    int fd = CreateUnownedFd();
    if (fd < 0) {
        return;
    }
    auto device = std::make_shared<ObexSocketDevice>(
        fd, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_RFCOMM, TEST_ADDR);
    auto transport = std::make_shared<ObexServerTransport>(device);
    client->clientSession_ = std::make_shared<ObexClientSession>(transport, nullptr);
    client->connectHeader_.SetConnectionId(1);
}

// Release clientSession_ and fileObject_ before client destruction to avoid
// ObexClientSession destructor running in libbt_obex.z.so with invalid transport state.
static void CleanupClient(std::shared_ptr<OppObexClient> &client)
{
    if (client) {
        if (client->clientSession_ != nullptr) {
            client->clientSession_->CloseTransport();
        }
        client->clientSession_ = nullptr;
        client->fileObject_ = nullptr;
    }
}

// Fuzz OppSendFileBodyObject
void TestSendFileBodyObject(FuzzedDataProvider &provider)
{
    // Create a temp file, write data, then dup() the fd so that
    // fdopen() in OpenFile() gets an unowned fd (avoids FDSAN error).
    FILE *tmpFile = tmpfile();
    if (tmpFile == nullptr) {
        return;
    }
    size_t bufLen = provider.ConsumeIntegralInRange<size_t>(0, 256);
    std::vector<uint8_t> buf(bufLen);
    if (bufLen > 0) {
        provider.ConsumeData(buf.data(), bufLen);
        fwrite(buf.data(), 1, bufLen, tmpFile);
        fflush(tmpFile);
    }
    int tmpFd = dup(fileno(tmpFile));
    fclose(tmpFile);  // close original FILE*, dup'd fd remains valid and unowned
    if (tmpFd < 0) {
        return;
    }

    std::string fileName = provider.ConsumeRandomLengthString(64);
    auto fileObj = std::make_shared<OppSendFileBodyObject>(tmpFd, fileName);

    size_t readSize = fileObj->Read(buf.data(), bufLen);
    (void)readSize;

    bool isFinished = fileObj->IsReadFinished();
    (void)isFinished;

    size_t writeSize = fileObj->Write(buf.data(), bufLen);
    (void)writeSize;

    size_t fileSize = fileObj->GetFileSize();
    (void)fileSize;

    size_t fileSendSize = fileObj->GetFileSendSize();
    (void)fileSendSize;

    fileObj->Close();

    // Test OpenFile with a fresh unowned fd
    FILE *tmpFile2 = tmpfile();
    if (tmpFile2 != nullptr) {
        int tmpFd2 = dup(fileno(tmpFile2));
        fclose(tmpFile2);
        if (tmpFd2 >= 0) {
            fileObj->OpenFile(tmpFd2);
            if (bufLen > 0) {
                fileObj->Write(buf.data(), bufLen);
            }
            fileObj->Close();
        }
    }

    // Test Write with nullptr
    fileObj->Write(nullptr, bufLen);

    // Test Write with zero length
    if (bufLen > 0) {
        fileObj->Write(buf.data(), 0);
    }
}

// Fuzz Connect / Disconnect / DisconnectLocal
// NOTE: Only use withObexReq=true to avoid PostEvent which dispatches async
// FFRT tasks that outlive the test function and access freed OppServiceImpl.
void TestConnectDisconnect(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);
    SetupClientSession(client);

    client->Disconnect(true);

    // Test with session restored
    SetupClientSession(client);
    client->DisconnectLocal(true);
    CleanupClient(client);
}

// Fuzz OnConnectResp / OnDisConnectResp
// NOTE: Avoid OBEX_HTTP_OK (0xA0) in OnConnectResp and skip OnDisConnectResp
// because both trigger PostEvent → async state machine transitions that crash
// when OppServiceImpl is freed before the FFRT task runs.
void TestConnectResp(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);
    SetupClientSession(client);

    ObexHeader reply;
    // Exclude 0xA0 (OBEX_HTTP_OK) to avoid PostEvent
    uint8_t responseCode = provider.ConsumeIntegralInRange<uint8_t>(0x00, 0x9F);
    reply.SetConnectionId(provider.ConsumeIntegral<uint32_t>());
    client->OnConnectResp(reply, responseCode);

    // Skip OnDisConnectResp — it always posts OPP_DISCONNECTED_EVT
    CleanupClient(client);
}

// Fuzz OnPutDateAvailableResp / OnAbortResp / OnActionCompleted
void TestResponseHandlers(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);
    SetupClientSession(client);

    uint8_t responseCode = provider.ConsumeIntegral<uint8_t>();
    ObexHeader reply;
    client->OnPutDateAvailableResp(reply, responseCode);
    client->OnAbortResp();

    // OnActionCompleted requires fileObject_ to be set
    int fd = CreateUnownedFd();
    if (fd >= 0) {
        client->fileObject_ = std::make_shared<OppSendFileBodyObject>(fd, "fuzz_test.txt");
    }

    // Test various response codes to cover different branches
    uint8_t codes[] = {
        static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_CONTINUE),
        static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_OK),
        static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_LENGTH_REQUIRED),
        static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_FORBIDDEN),
        static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE),
        static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_UNSUPPORTED_TYPE),
        static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_UNAVAILABLE),
        static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_BAD_REQUEST),
    };
    int codeIdx = provider.ConsumeIntegralInRange<int>(0, sizeof(codes) - 1);
    client->status_ = provider.ConsumeIntegralInRange<int>(0, 3);
    client->OnActionCompleted(reply, codes[codeIdx]);
    CleanupClient(client);
}

// Fuzz SetBusy / OnBusy
void TestBusy(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);
    SetupClientSession(client);

    bool isBusy = provider.ConsumeBool();
    client->SetBusy(isBusy);
    client->OnBusy(isBusy);
    CleanupClient(client);
}

// Fuzz CancelSendFile
void TestCancelSendFile(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);
    SetupClientSession(client);

    bool isImmediately = provider.ConsumeBool();
    client->CancelSendFile(isImmediately);
    CleanupClient(client);
}

// Fuzz CheckBeforeRequest with various opcodes
void TestCheckBeforeRequest(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);

    uint8_t opcodes[] = {
        OBEX_OPCODE_CONNECT,
        OBEX_OPCODE_DISCONNECT,
        OBEX_OPCODE_SESSION,
        OBEX_OPCODE_PUT,
        OBEX_OPCODE_GET,
        OBEX_OPCODE_ABORT,
    };
    int idx = provider.ConsumeIntegralInRange<int>(0, sizeof(opcodes) - 1);
    // Toggle isObexConnected_ to cover both branches
    client->isObexConnected_ = provider.ConsumeBool();
    client->isProcessing_ = provider.ConsumeBool();
    bool result = client->CheckBeforeRequest(opcodes[idx]);
    (void)result;
    CleanupClient(client);
}

// Fuzz Abort / SendAbortRequest
void TestAbort(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);
    SetupClientSession(client);

    client->isProcessing_ = provider.ConsumeBool();
    client->isWaitingSendAbort_ = provider.ConsumeBool();
    client->isAbortSended_ = provider.ConsumeBool();
    client->isObexConnected_ = true;
    client->Abort();
    client->SendAbortRequest();
    CleanupClient(client);
}

// Fuzz OnTransferStateChangeFaild / OnOperationActionSync
void TestStateChange(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);

    int reasons[] = {
        OPP_TRANSFER_RESULT_ERROR_TRANSFER_FAILED,
        OPP_TRANSFER_RESULT_ERROR_UNKNOWN,
        OPP_TRANSFER_RESULT_ERROR_CANCELED,
        OPP_TRANSFER_RESULT_ERROR_LENGTH,
    };
    int idx = provider.ConsumeIntegralInRange<int>(0, sizeof(reasons) - 1);
    client->OnTransferStateChangeFaild(reasons[idx]);

    uint8_t responseCode = provider.ConsumeIntegral<uint8_t>();
    client->OnOperationActionSync(responseCode);
    CleanupClient(client);
}

// Fuzz SendFile
void TestSendFile(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);
    SetupClientSession(client);

    IOppTransferInformation info;
    info.SetFileName(provider.ConsumeRandomLengthString(64));
    info.SetFileType(provider.ConsumeRandomLengthString(16));
    info.SetTotalBytes(provider.ConsumeIntegral<uint64_t>());
    info.SetFileFd(-1);
    int ret = client->SendFile(std::make_shared<IOppTransferInformation>(info));
    (void)ret;
    CleanupClient(client);
}

// Fuzz RecvSocketDevice
void TestRecvSocketDevice(FuzzedDataProvider &provider, OppServiceImpl *svcImpl)
{
    auto client = CreateOppObexClient(svcImpl);

    int typeIdx = provider.ConsumeIntegralInRange<int>(0, 2);
    SocketType type = SocketType::TYPE_RFCOMM;
    if (typeIdx == 0) {
        type = SocketType::TYPE_L2CAP;
    } else if (typeIdx == 1) {
        type = SocketType::TYPE_RFCOMM;
    } else {
        type = static_cast<SocketType>(99);
    }
    client->socketFd_ = provider.ConsumeIntegral<int32_t>();
    auto device = client->RecvSocketDevice(type);
    (void)device;
    CleanupClient(client);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);

    auto svcImpl = new OppServiceImpl();
    svcImpl->isAllowedOppReadFunc_ = [](const std::string &addr) { return true; };

    TestSendFileBodyObject(provider);
    TestConnectDisconnect(provider, svcImpl);
    TestConnectResp(provider, svcImpl);
    TestResponseHandlers(provider, svcImpl);
    TestBusy(provider, svcImpl);
    TestCancelSendFile(provider, svcImpl);
    TestCheckBeforeRequest(provider, svcImpl);
    TestAbort(provider, svcImpl);
    TestStateChange(provider, svcImpl);
    TestSendFile(provider, svcImpl);
    TestRecvSocketDevice(provider, svcImpl);

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
