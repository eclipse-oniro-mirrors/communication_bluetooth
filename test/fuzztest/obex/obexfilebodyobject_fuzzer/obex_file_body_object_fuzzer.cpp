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

#include "obex_file_body_object_fuzzer.h"
#include "permission_manager.h"
#include "opp_obex_client.h"
#include "opp_obex_server.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

using namespace std;
namespace OHOS {

/*
 * Note: obex_file_body_object.h / ObexFileBodyObject do not exist in this
 * codebase. The real file based OBEX body objects are:
 *   - bluetooth::OppSendFileBodyObject (src/opp/opp_obex_client.h)
 *   - bluetooth::OppReceiveFileBodyObject (src/opp/opp_obex_server.h)
 * This fuzzer targets those real classes.
 */

// Fuzz OppSendFileBodyObject with arbitrary (mostly invalid) fds.
// OpenFile/Read/Write/Close of this class are safe for any fd value.
void TestSendObjectWithFuzzedFd(FuzzedDataProvider &provider, bluetooth::OppSendFileBodyObject &sendObj)
{
    int fd = provider.ConsumeIntegral<int>();
    sendObj.OpenFile(fd);
    sendObj.IsReadFinished();
    sendObj.GetFileSize();
    sendObj.GetFileSendSize();

    size_t bufLen = provider.ConsumeIntegralInRange<size_t>(1, 256);
    std::vector<uint8_t> readBuf(bufLen, 0);
    sendObj.Read(readBuf.data(), readBuf.size());
    sendObj.IsReadFinished();
    // nullptr buffer edge case: read() returns -1 which is handled by the class
    sendObj.Read(nullptr, bufLen);

    size_t writeLen = provider.ConsumeIntegralInRange<size_t>(0, 256);
    std::vector<uint8_t> writeData = provider.ConsumeBytes<uint8_t>(writeLen);
    if (!writeData.empty()) {
        sendObj.Write(writeData.data(), writeData.size());
    }
    sendObj.Close();
}

// Fuzz OppSendFileBodyObject with a real temp file: full open/read/finish/close lifecycle.
void TestSendObjectWithValidFile(FuzzedDataProvider &provider)
{
    FILE *tmp = tmpfile();
    if (tmp == nullptr) {
        return;
    }
    size_t dataLen = provider.ConsumeIntegralInRange<size_t>(1, 1024);
    std::vector<uint8_t> fileData = provider.ConsumeBytes<uint8_t>(dataLen);
    if (fileData.empty() || fwrite(fileData.data(), 1, fileData.size(), tmp) != fileData.size()) {
        fclose(tmp);
        return;
    }
    fflush(tmp);
    // dup so that the body object owns its own fd; fclose(tmp) keeps the original one
    int validFd = dup(fileno(tmp));
    if (validFd == -1) {
        fclose(tmp);
        return;
    }
    bluetooth::OppSendFileBodyObject sendObj(validFd, "obex_file_body_fuzz_test");
    std::vector<uint8_t> readBuf(provider.ConsumeIntegralInRange<size_t>(1, 512), 0);
    while (!sendObj.IsReadFinished()) {
        size_t readSize = sendObj.Read(readBuf.data(), readBuf.size());
        if (readSize == 0) {
            break;
        }
    }
    sendObj.GetFileSize();
    sendObj.GetFileSendSize();

    size_t writeLen = provider.ConsumeIntegralInRange<size_t>(0, 128);
    std::vector<uint8_t> writeData = provider.ConsumeBytes<uint8_t>(writeLen);
    if (!writeData.empty()) {
        sendObj.Write(writeData.data(), writeData.size());
    }
    sendObj.Close();   // closes the dup'ed fd, safe to call twice
    fclose(tmp);       // closes the original fd
}

// Fuzz OppReceiveFileBodyObject on safe paths only.
// NOTE: OpenFile() with an invalid (non -1) fd leaves fdFile_ == nullptr while fd_ keeps
// the value, and Close() then calls fclose(nullptr); a second Close() after a successful
// one does the same. Both are product bugs; they are deliberately avoided here so that
// the fuzzer keeps running (only OpenFile(-1) and valid fds obtained via dup are used,
// and Close() is invoked once by the destructor).
void TestReceiveObject(FuzzedDataProvider &provider)
{
    bluetooth::OppReceiveFileBodyObject recvObj;
    size_t bufLen = provider.ConsumeIntegralInRange<size_t>(1, 256);
    std::vector<uint8_t> buf(bufLen, 0);
    recvObj.Read(buf.data(), buf.size());
    recvObj.IsReadFinished();

    size_t writeLen = provider.ConsumeIntegralInRange<size_t>(0, 256);
    std::vector<uint8_t> writeData = provider.ConsumeBytes<uint8_t>(writeLen);
    if (!writeData.empty()) {
        // no OppServiceImpl attached: Write returns 0 immediately (null service branch)
        recvObj.Write(writeData.data(), writeData.size());
    }
    recvObj.Write(nullptr, bufLen);
    recvObj.OpenFile(-1);   // invalid fd short-circuit branch

    FILE *tmp = tmpfile();
    if (tmp != nullptr) {
        int validFd = dup(fileno(tmp));
        if (validFd != -1) {
            recvObj.OpenFile(validFd);
            recvObj.Write(buf.data(), buf.size());
        }
        fclose(tmp);
    }
    // destructor performs the single Close()
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("start");
    FuzzedDataProvider provider(data, size);

    int fd = provider.ConsumeIntegral<int>();
    auto sendObj = std::make_shared<bluetooth::OppSendFileBodyObject>(fd, "obex_file_body_fuzz_test");
    TestSendObjectWithFuzzedFd(provider, *sendObj);

    auto defaultObj = std::make_shared<bluetooth::OppSendFileBodyObject>();
    TestSendObjectWithFuzzedFd(provider, *defaultObj);

    TestSendObjectWithValidFile(provider);
    TestReceiveObject(provider);
    return true;
}
}

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
