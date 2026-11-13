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

#include "obex_server_socket_thread_fuzzer.h"
#include "permission_manager.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "obex_server_socket_thread.h"
#include "obex_socket_observer.h"
#include "obex_socket_device.h"
#include "bluetooth_log.h"
#include "bt_sock.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

#include <sys/socket.h>
#include <unistd.h>

using namespace std;
using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {

// Fuzz input bounds
static constexpr int FUZZ_SOCKET_TYPE_MIN = static_cast<int>(SocketType::TYPE_RFCOMM);
static constexpr int FUZZ_SOCKET_TYPE_MAX = static_cast<int>(SocketType::TYPE_L2CAP_LE);
static constexpr unsigned int FUZZ_FLAGS_RD = static_cast<unsigned int>(OBEX_SOCK_THREAD_FD_RD);
static constexpr unsigned int FUZZ_FLAGS_WR = static_cast<unsigned int>(OBEX_SOCK_THREAD_FD_WR);
static constexpr unsigned int FUZZ_FLAGS_SYNC = static_cast<unsigned int>(OBEX_SOCK_THREAD_ADD_FD_SYNC);
static constexpr int FUZZ_FD_MIN = -1;
static constexpr int FUZZ_FD_MAX = 20;
static constexpr int FUZZ_POLL_SLOT_MIN = 0;
static constexpr int FUZZ_POLL_SLOT_MAX = MAX_POLL - 1;
static constexpr int FUZZ_ERR_TIMES_MIN = 0;
static constexpr int FUZZ_ERR_TIMES_MAX = 15;
static constexpr int FUZZ_ERR_TIMES_THRESHOLD = 10;
static constexpr int FUZZ_CMD_WAKEUP = 1;
static constexpr int FUZZ_CMD_EXIT = 2;
static constexpr int FUZZ_CMD_ADD_FD = 3;
static constexpr int FUZZ_CMD_REMOVE_FD = 4;
static constexpr int FUZZ_MAC_ADDR_LEN = 6;
static constexpr int FUZZ_FD_VALID_MIN = 0;
static constexpr size_t FUZZ_PREPARE_SIZE_MIN = 1;
static constexpr int FUZZ_UNKNOWN_CMD_MIN = 100;
static constexpr int FUZZ_UNKNOWN_CMD_MAX = 200;
// Wait up to 500ms for poll thread to exit after CMD_EXIT
static constexpr int FUZZ_THREAD_EXIT_RETRIES = 500;
static constexpr int FUZZ_THREAD_EXIT_SLEEP_US = 1000;

class FuzzSocketObserver : public IObexSocketObserver {
public:
    void OnConnect(std::shared_ptr<ObexSocketDevice> &device) override
    {
        HILOGI("FuzzSocketObserver::OnConnect called");
    }
};

// Wait for poll thread to exit; cmdFdw_ is the last member set in CloseCmdFd,
// then sleep to allow the thread to fully terminate after CloseCmdFd returns.
static void WaitForThreadExit(ObexServerSocketThread &thread)
{
    for (int i = 0; i < FUZZ_THREAD_EXIT_RETRIES; i++) {
        if (thread.cmdFdw_ == -1) {
            break;
        }
        usleep(FUZZ_THREAD_EXIT_SLEEP_US);
    }
    usleep(FUZZ_THREAD_EXIT_SLEEP_US);
};

// Thread lifecycle + InitCmdFd + AddPoll + SetPoll + Flags2Pevents
static void TestThreadLifecycle(FuzzedDataProvider &provider, ObexServerSocketThread &thread)
{
    thread.InitThread();
    thread.InitThread();  // double init, exercises usedIndex_ check
    thread.InitCmdFd();   // double init, exercises early-return

    thread.AddPoll(FUZZ_FD_MIN, provider.ConsumeIntegralInRange<int>(FUZZ_POLL_SLOT_MIN, FUZZ_POLL_SLOT_MAX),
        FUZZ_FLAGS_RD);

    unsigned int flags1 = provider.ConsumeIntegral<unsigned int>();
    thread.Flags2Pevents(flags1);
    unsigned int flags2 = FUZZ_FLAGS_RD | FUZZ_FLAGS_WR;
    thread.Flags2Pevents(flags2);

    thread.SetPoll(nullptr, provider.ConsumeIntegralInRange<int>(FUZZ_FD_MIN, FUZZ_FD_MAX),
        provider.ConsumeIntegralInRange<int>(FUZZ_POLL_SLOT_MIN, FUZZ_POLL_SLOT_MAX),
        provider.ConsumeIntegral<unsigned int>());

    thread.DeInitThread();  // 2->1, early return
    thread.DeInitThread();  // 1->0, sends CMD_EXIT
    WaitForThreadExit(thread);
}

// ThreadAddFd + ThreadRemoveFdAndClose
static void TestThreadAddRemoveFd(FuzzedDataProvider &provider, ObexServerSocketThread &thread)
{
    auto observer = std::make_shared<FuzzSocketObserver>();
    thread.ThreadAddFd(provider.ConsumeIntegralInRange<int>(FUZZ_FD_MIN, FUZZ_FD_MAX),
        provider.ConsumeIntegralInRange<int>(FUZZ_SOCKET_TYPE_MIN, FUZZ_SOCKET_TYPE_MAX),
        FUZZ_FLAGS_RD, observer);  // cmdFdw_ == -1, early return

    thread.ThreadRemoveFdAndClose(FUZZ_FD_MIN);  // invalid fd, early return

    thread.InitThread();

    thread.ThreadAddFd(provider.ConsumeIntegralInRange<int>(FUZZ_FD_VALID_MIN, FUZZ_FD_MAX),
        provider.ConsumeIntegralInRange<int>(FUZZ_SOCKET_TYPE_MIN, FUZZ_SOCKET_TYPE_MAX),
        FUZZ_FLAGS_RD, nullptr);  // null callback, early return

    int syncFd[2] = {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, syncFd) == 0) {
        thread.ThreadAddFd(syncFd[0],
            provider.ConsumeIntegralInRange<int>(FUZZ_SOCKET_TYPE_MIN, FUZZ_SOCKET_TYPE_MAX),
            FUZZ_FLAGS_SYNC | FUZZ_FLAGS_RD, observer);
        close(syncFd[0]);
        close(syncFd[1]);
    }

    int asyncFd[2] = {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, asyncFd) == 0) {
        thread.ThreadAddFd(asyncFd[0],
            provider.ConsumeIntegralInRange<int>(FUZZ_SOCKET_TYPE_MIN, FUZZ_SOCKET_TYPE_MAX),
            FUZZ_FLAGS_RD, observer);
        thread.ThreadRemoveFdAndClose(asyncFd[0]);
        close(asyncFd[0]);
        close(asyncFd[1]);
    }

    thread.DeInitThread();
    WaitForThreadExit(thread);
}

// Poll internals — no poll thread running (InitCmdFd, not InitThread)
static void TestPollInternals(FuzzedDataProvider &provider, ObexServerSocketThread &thread)
{
    thread.InitCmdFd();

    thread.RemovePoll(nullptr);  // null check

    thread.AddPoll(FUZZ_FD_MIN, provider.ConsumeIntegralInRange<int>(FUZZ_POLL_SLOT_MIN, FUZZ_POLL_SLOT_MAX),
        FUZZ_FLAGS_RD);

    int sv[2] = {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0) {
        thread.AddPoll(sv[0],
            provider.ConsumeIntegralInRange<int>(FUZZ_POLL_SLOT_MIN, FUZZ_POLL_SLOT_MAX),
            FUZZ_FLAGS_RD);
        thread.AddPoll(sv[0],  // same fd again, exercises "fd already exists"
            provider.ConsumeIntegralInRange<int>(FUZZ_POLL_SLOT_MIN, FUZZ_POLL_SLOT_MAX),
            FUZZ_FLAGS_WR);

        for (int i = 0; i < MAX_POLL; i++) {
            if (thread.pollSlotArray_[i].pfd.fd == sv[0]) {
                thread.RemovePoll(&thread.pollSlotArray_[i]);
                break;
            }
        }
        close(sv[0]);
        close(sv[1]);
    }

    constexpr int FUZZ_EXTRA_FD_COUNT = MAX_POLL + 1;
    int extraFds[FUZZ_EXTRA_FD_COUNT] = {-1};
    for (int i = 0; i < FUZZ_EXTRA_FD_COUNT; i++) {
        int pair[2] = {-1, -1};
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0) {
            extraFds[i] = pair[0];
            thread.AddPoll(pair[0], i % (FUZZ_POLL_SLOT_MAX + 1), FUZZ_FLAGS_RD | FUZZ_FLAGS_WR);
            close(pair[1]);
        } else {
            break;
        }
    }
    for (int i = 0; i < FUZZ_EXTRA_FD_COUNT; i++) {
        if (extraFds[i] > 0) {
            close(extraFds[i]);
        }
    }

    struct pollfd pfds[MAX_POLL];
    if (memset_s(pfds, sizeof(pfds), 0, sizeof(pfds)) == 0) {
        size_t prepareSize = provider.ConsumeIntegralInRange<size_t>(FUZZ_PREPARE_SIZE_MIN, MAX_POLL);
        thread.PreparePollFds(pfds, prepareSize);
    }

    if (thread.cmdFdw_ != -1) {
        int addFd[2] = {-1, -1};
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, addFd) == 0) {
            SockCmd addCmd = {FUZZ_CMD_ADD_FD, addFd[0],
                provider.ConsumeIntegralInRange<int>(FUZZ_POLL_SLOT_MIN, FUZZ_POLL_SLOT_MAX),
                static_cast<int>(FUZZ_FLAGS_RD)};
            ssize_t ret = send(thread.cmdFdw_, &addCmd, sizeof(addCmd), 0);
            if (ret > 0) {
                thread.ProcessCmdSock();
            }
            SockCmd removeCmd = {FUZZ_CMD_REMOVE_FD, addFd[0], 0, 0};
            ret = send(thread.cmdFdw_, &removeCmd, sizeof(removeCmd), 0);
            if (ret > 0) {
                thread.ProcessCmdSock();
            }
            close(addFd[1]);
        }
        SockCmd wakeupCmd = {FUZZ_CMD_WAKEUP, 0, 0, 0};
        ssize_t ret = send(thread.cmdFdw_, &wakeupCmd, sizeof(wakeupCmd), 0);
        if (ret > 0) {
            thread.ProcessCmdSock();
        }
        SockCmd exitCmd = {FUZZ_CMD_EXIT, 0, 0, 0};
        ret = send(thread.cmdFdw_, &exitCmd, sizeof(exitCmd), 0);
        if (ret > 0) {
            thread.ProcessCmdSock();
        }
        SockCmd unknownCmd = {provider.ConsumeIntegralInRange<int>(FUZZ_UNKNOWN_CMD_MIN, FUZZ_UNKNOWN_CMD_MAX),
            0, 0, 0};
        ret = send(thread.cmdFdw_, &unknownCmd, sizeof(unknownCmd), 0);
        if (ret > 0) {
            thread.ProcessCmdSock();
        }
    }

    struct pollfd dataPfds[MAX_POLL] = {};
    int dataCount = provider.ConsumeIntegralInRange<int>(FUZZ_FD_VALID_MIN, MAX_POLL);
    for (int i = 0; i < MAX_POLL; i++) {
        dataPfds[i].fd = provider.ConsumeIntegralInRange<int>(FUZZ_FD_MIN, FUZZ_FD_MAX);
        dataPfds[i].revents = provider.ConsumeIntegral<short>();
    }
    thread.ProcessDataSock(dataPfds, dataCount);

    thread.CloseCmdFd();
}

// HandleMsg + RecvSocketDevice
static void TestHandleMsgAndRecv(FuzzedDataProvider &provider, ObexServerSocketThread &thread)
{
    thread.InitThread();

    int recvType = provider.ConsumeIntegralInRange<int>(FUZZ_SOCKET_TYPE_MIN, FUZZ_SOCKET_TYPE_MAX);
    thread.RecvSocketDevice(provider.ConsumeIntegralInRange<int>(FUZZ_FD_MIN, FUZZ_FD_MAX),
        static_cast<SocketType>(recvType));  // invalid fd, recvmsg error

    int sv[2] = {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0) {
        sock_connect_info_t info = {};
        info.status = provider.ConsumeBool();
        info.max_tx_packet_size = provider.ConsumeIntegral<unsigned short>();
        info.max_rx_packet_size = provider.ConsumeIntegral<unsigned short>();

        for (int i = 0; i < FUZZ_MAC_ADDR_LEN; i++) {
            info.bd_addr.address[i] = provider.ConsumeIntegral<uint8_t>();
        }

        struct msghdr msg = {};
        struct iovec iov = {};
        iov.iov_base = &info;
        iov.iov_len = sizeof(info);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        char cmsgbuf[CMSG_SPACE(sizeof(int))];
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        int dummyFd = sv[1];
        memcpy_s(CMSG_DATA(cmsg), sizeof(dummyFd), &dummyFd, sizeof(dummyFd));

        ssize_t sent = sendmsg(sv[0], &msg, 0);
        if (sent > 0) {
            int recvType2 = provider.ConsumeIntegralInRange<int>(FUZZ_SOCKET_TYPE_MIN, FUZZ_SOCKET_TYPE_MAX);
            thread.RecvSocketDevice(sv[1], static_cast<SocketType>(recvType2));
        }
        close(sv[0]);
        close(sv[1]);
    }

    int handleFd = provider.ConsumeIntegralInRange<int>(FUZZ_FD_VALID_MIN, FUZZ_FD_MAX);
    thread.fdErrCountMap_.EnsureInsert(handleFd,
        provider.ConsumeIntegralInRange<int>(FUZZ_ERR_TIMES_MIN, FUZZ_ERR_TIMES_MAX));
    auto observer = std::make_shared<FuzzSocketObserver>();
    thread.onConnectMap_.EnsureInsert(handleFd, observer);
    int handleType = provider.ConsumeIntegralInRange<int>(FUZZ_SOCKET_TYPE_MIN, FUZZ_SOCKET_TYPE_MAX);
    thread.HandleMsg(handleFd, handleType);

    // error times >= threshold, exercises restart branch
    int errFd = provider.ConsumeIntegralInRange<int>(FUZZ_FD_VALID_MIN, FUZZ_FD_MAX);
    thread.fdErrCountMap_.EnsureInsert(errFd, FUZZ_ERR_TIMES_THRESHOLD);
    thread.onConnectMap_.EnsureInsert(errFd, observer);
    thread.HandleMsg(errFd, provider.ConsumeIntegralInRange<int>(FUZZ_SOCKET_TYPE_MIN, FUZZ_SOCKET_TYPE_MAX));

    thread.DeInitThread();
    WaitForThreadExit(thread);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    HILOGI("ObexServerSocketThreadFuzzTest start");
    FuzzedDataProvider provider(data, size);

    {
        ObexServerSocketThread thread1;
        TestThreadLifecycle(provider, thread1);
    }

    {
        ObexServerSocketThread thread2;
        TestThreadAddRemoveFd(provider, thread2);
    }

    {
        ObexServerSocketThread thread3;
        TestPollInternals(provider, thread3);
    }

    {
        ObexServerSocketThread thread4;
        TestHandleMsgAndRecv(provider, thread4);
    }

    HILOGI("ObexServerSocketThreadFuzzTest success");
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }

    PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
