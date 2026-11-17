/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <sys/socket.h>
#include <cstring>
#include <chrono>
#include <pthread.h>
#include <thread>

#include "bluetooth_log.h"
#include "thread_util.h"
#include "obex_server_socket_thread.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
const int TIME = 2;
static int g_fdPair[2] = {0, 0};
static std::shared_ptr<ObexServerSocketThread> threadObj;
class ObexServerSocketThreadTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        socketpair(AF_UNIX, SOCK_STREAM, 0, g_fdPair);
        threadObj = std::make_shared<ObexServerSocketThread>();
        sleep(TIME);
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        threadObj = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

class TestSocketObserver : public IObexSocketObserver {
public:
    void OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice) override
    {
        HILOGI("TestSocketObserver OnConnect Called");
    }
};

/*
 * @tc.number: ObexServerSocketThread_DeInitThread_002
 * @tc.name: DeInitThread
 * @tc.desc: 线程对象多次被调用后正常去初始化
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_DeInitThread_002, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_DeInitThread_002 start");
    threadObj->InitThread();
    // 重复调用，进入异常流程
    threadObj->InitThread();
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    threadObj->InitThread();
    threadObj->usedIndex_ = -1;
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    threadObj->InitThread();
    threadObj->usedIndex_ = 0;
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    threadObj->InitThread();
    threadObj->cmdFdw_ = -1;
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_TRUE(threadObj != nullptr);
    HILOGI("ObexServerSocketThread_DeInitThread_002 end");
}

/*
 * @tc.number: ObexServerSocketThread_InitCmdFd_001
 * @tc.name: InitCmdFd
 * @tc.desc: 
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_InitCmdFd_001, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_InitCmdFd_001 start");
    threadObj->InitThread();
    threadObj->cmdFdr_ = 1;
    threadObj->InitCmdFd();
    threadObj->cmdFdw_ = 1;
    threadObj->InitCmdFd();
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_TRUE(threadObj != nullptr);
    HILOGI("ObexServerSocketThread_InitCmdFd_001 end");
}

/*
 * @tc.number: ObexServerSocketThread_ThreadAddFd_001
 * @tc.name: ThreadAddFd
 * @tc.desc: 正常添加fd监听
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_ThreadAddFd_001, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_ThreadAddFd_001 start");
    threadObj->InitThread();
    bool res = threadObj->ThreadAddFd(g_fdPair[0], 0, OBEX_SOCK_THREAD_FD_RD, std::make_shared<TestSocketObserver>());
    EXPECT_TRUE(res);
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("ObexServerSocketThread_ThreadAddFd_001 end");
}

/*
 * @tc.number: ObexServerSocketThread_ThreadAddFd_002
 * @tc.name: ThreadAddFd
 * @tc.desc: 添加即时同步的fd监听
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_ThreadAddFd_002, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_ThreadAddFd_002 start");
    threadObj->InitThread();
    int flag = OBEX_SOCK_THREAD_FD_RD | OBEX_SOCK_THREAD_FD_RD;
    bool res = threadObj->ThreadAddFd(g_fdPair[0], 0, flag, std::make_shared<TestSocketObserver>());
    EXPECT_TRUE(res);
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("ObexServerSocketThread_ThreadAddFd_002 end");
}

/*
 * @tc.number: ObexServerSocketThread_ThreadAddFd_003
 * @tc.name: ThreadAddFd
 * @tc.desc: 添加fd监听但未设置callback
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_ThreadAddFd_003, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_ThreadAddFd_003 start");
    threadObj->InitThread();
    bool res = threadObj->ThreadAddFd(g_fdPair[0], 0, OBEX_SOCK_THREAD_FD_RD, nullptr);
    EXPECT_FALSE(res);
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("ObexServerSocketThread_ThreadAddFd_003 end");
}

/*
 * @tc.number: ObexServerSocketThread_ThreadAddFd_004
 * @tc.name: ThreadAddFd
 * @tc.desc: 添加fd监听但未进行线程初始化
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_ThreadAddFd_004, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_ThreadAddFd_004 start");
    threadObj->cmdFdw_ = -1;
    bool res = threadObj->ThreadAddFd(g_fdPair[0], 0, OBEX_SOCK_THREAD_FD_RD, std::make_shared<TestSocketObserver>());
    EXPECT_FALSE(res);
    HILOGI("ObexServerSocketThread_ThreadAddFd_004 end");
}

/*
 * @tc.number: ObexServerSocketThread_ThreadRemoveFdAndClose_001
 * @tc.name: ThreadRemoveFdAndClose
 * @tc.desc: 正常去除Fd监听
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_ThreadRemoveFdAndClose_001, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_ThreadRemoveFdAndClose_001 start");
    int fdPairTemp[2] = {0, 0};
    socketpair(AF_UNIX, SOCK_STREAM, 0, fdPairTemp);
    threadObj->InitThread();
    threadObj->ThreadAddFd(fdPairTemp[0], 0, OBEX_SOCK_THREAD_FD_RD, std::make_shared<TestSocketObserver>());
    bool res = threadObj->ThreadRemoveFdAndClose(fdPairTemp[0]);
    EXPECT_TRUE(res);
    close(fdPairTemp[1]);
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("ObexServerSocketThread_ThreadRemoveFdAndClose_001 end");
}

/*
 * @tc.number: ObexServerSocketThread_ThreadRemoveFdAndClose_002
 * @tc.name: ThreadRemoveFdAndClose
 * @tc.desc: 异常去除id为-1的fd
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_ThreadRemoveFdAndClose_002, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_ThreadRemoveFdAndClose_002 start");
    threadObj->InitThread();
    bool res = threadObj->ThreadRemoveFdAndClose(-1);
    EXPECT_FALSE(res);
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("ObexServerSocketThread_ThreadRemoveFdAndClose_002 end");
}

/*
 * @tc.number: ObexServerSocketThread_AddPoll_001
 * @tc.name: AddPoll
 * @tc.desc: AddPoll超过MAX_POLL的限度
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_AddPoll_001, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_AddPoll_001 start");
    threadObj->InitThread();
    bool res = true;
    int offset = 1000;
    for (int i = 0; i < MAX_POLL + 1; ++i) {
        res = threadObj->ThreadAddFd(offset + i, 0, OBEX_SOCK_THREAD_FD_RD, std::make_shared<TestSocketObserver>());
    }
    EXPECT_TRUE(res);
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("ObexServerSocketThread_AddPoll_001 end");
}

/*
 * @tc.number: ObexServerSocketThread_SetPoll_002
 * @tc.name: SetPoll
 * @tc.desc: 
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_SetPoll_002, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_SetPoll_002 start");
    threadObj->InitThread();
    bool ret = threadObj->ThreadAddFd(g_fdPair[0], 0, OBEX_SOCK_THREAD_FD_RD, std::make_shared<TestSocketObserver>());
    EXPECT_TRUE(ret);
    ret = threadObj->ThreadAddFd(g_fdPair[0], 1, OBEX_SOCK_THREAD_FD_RD, std::make_shared<TestSocketObserver>());
    EXPECT_TRUE(ret);
    threadObj->SetPoll(nullptr, 1, 0, OBEX_SOCK_THREAD_FD_RD);
    threadObj->AddPoll(-1, 0, OBEX_SOCK_THREAD_FD_RD);
    threadObj->RemovePoll(nullptr);
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("ObexServerSocketThread_SetPoll_002 end");
}

/*
 * @tc.number: ObexServerSocketThread_HandleMsg_001
 * @tc.name: HandleMsg1
 * @tc.desc: HandleMsg时错误连续超过10次，触发异常处理
 */
HWTEST_F(ObexServerSocketThreadTest, ObexServerSocketThread_HandleMsg_001, TestSize.Level1)
{
    HILOGI("ObexServerSocketThread_HandleMsg_001 start");
    threadObj->InitThread();
    std::shared_ptr<TestSocketObserver> observer = std::make_shared<TestSocketObserver>();
    bool ret = threadObj->ThreadAddFd(g_fdPair[0], 0, OBEX_SOCK_THREAD_FD_RD, observer);
    EXPECT_TRUE(ret);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    for (int i = 0; i < 11; ++i) {
        // 传送空数据给poll，导致线程读取数据后解析数据失败
        send(g_fdPair[1], "", 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    threadObj->DeInitThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    HILOGI("ObexServerSocketThread_HandleMsg_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS