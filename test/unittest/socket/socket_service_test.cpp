/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "socket_service.h"
#include "socket_def.h"
#include "class_creator.h"
#include "thread_util.h"
#include "bluetooth_errorcode.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
constexpr int TIME = 4;

static BluetoothHost *host_;

void *SocketServiceCreate(void)
{
    return new SocketService();
}

class SocketServiceTest : public testing::Test {
public:
    SocketServiceTest();
    ~SocketServiceTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::unique_ptr<SocketService> socketService = nullptr;
};

SocketServiceTest::SocketServiceTest()
{
    HILOGI("SocketServiceTest CONSTRUCTOR");
    socketService = std::make_unique<SocketService>();
    sleep(1);
}

SocketServiceTest::~SocketServiceTest()
{
    HILOGI("###### ~SocketServiceTest ######");
    sleep(1);
    socketService = nullptr;
}

void SocketServiceTest::SetUpTestCase(void)
{
    HILOGI("SocketServiceTest SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("SocketServiceTest isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
    ClassFactory::RegisterClass("SocketService", SocketServiceCreate);
    HILOGI("SocketServiceTest SetUpTestCase end");
}
void SocketServiceTest::TearDownTestCase(void)
{
    HILOGI("SocketServiceTest TearDownTestCase start");
    sleep(TIME);
    host_ = nullptr;
    HILOGI("SocketServiceTest TearDownTestCase end");
}
void SocketServiceTest::SetUp()
{
}

void SocketServiceTest::TearDown()
{
}

/**
 * @tc.number:Socket_Service_UnitTest_Enable
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_Enable, TestSize.Level1)
{
    HILOGI("SocketServiceTest::Enable starts");
    socketService->Enable();
    socketService->EnableNative();
    EXPECT_EQ(socketService->GetMaxConnectNum(), 0);
    HILOGI("SocketServiceTest::Enable ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_Connect
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("SocketServiceTest::Connect starts");
    std::string addr = "11:22:33:44:55:66";
    Uuid uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    int psm = 1;
    int ret = socketService->Connect(addr, uuid, SOCK_FLAG_AUTHENTICATION, SOCK_L2CAP_LE, psm);
    EXPECT_EQ(ret, -1);
    HILOGI("SocketServiceTest::Connect ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_Listen
 * @tc.name: Listen
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_Listent, TestSize.Level1)
{
    HILOGI("SocketServiceTest::Listen starts");
    std::string name = "BluetoothDevice";
    Uuid uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    int channel = 20;
    int ret = socketService->Listen(name, uuid, SOCK_FLAG_AUTHENTICATION, SOCK_L2CAP_LE, channel);
    EXPECT_EQ(ret, -1);
    HILOGI("SocketServiceTest::Listen ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_UpdateCocConnectionParams
 * @tc.name: UpdateCocConnectionParams
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_UpdateCocConnectionParams, TestSize.Level1)
{
    HILOGI("SocketServiceTest::UpdateCocConnectionParams starts");
    BluetoothSocketCocInfo *codecData = new BluetoothSocketCocInfo();
    socketService->UpdateCocConnectionParams(*codecData);
    EXPECT_EQ(socketService->GetConnectState(), 0);
    HILOGI("SocketServiceTest::UpdateCocConnectionParams ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_RegisterConnectionObserver
 * @tc.name: RegisterConnectionObserver
 * @tc.desc: IBtClientSocketCallback callback : nullptr
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_RegisterConnectionObserver, TestSize.Level1)
{
    HILOGI("SocketServiceTest::RegisterConnectionObserver starts");
    std::string addr = "11:22:33:44:55:66";
    Uuid uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    int ret = socketService->RegisterConnectionObserver(addr, uuid, nullptr);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("SocketServiceTest::RegisterConnectionObserver ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_UnregisterConnectionObserver
 * @tc.name: UnregisterConnectionObserver
 * @tc.desc: IBtClientSocketCallback callback : nullptr
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_UnregisterConnectionObserver, TestSize.Level1)
{
    HILOGI("SocketServiceTest::UnregisterConnectionObserver starts");
    std::string addr = "11:22:33:44:55:66";
    Uuid uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    int ret = socketService->UnregisterConnectionObserver(addr, uuid, nullptr);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("SocketServiceTest::UnregisterConnectionObserver ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_Disable
 * @tc.name: Disable
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_Disable, TestSize.Level1)
{
    HILOGI("SocketServiceTest::Disable starts");
    socketService->Disable();
    EXPECT_EQ(socketService->GetMaxConnectNum(), 0);
    HILOGI("SocketServiceTest::Disable ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_Unmarshalling
 * @tc.name: Unmarshalling
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_Unmarshalling, TestSize.Level1)
{
    HILOGI("SocketServiceTest::Unmarshalling starts");
    BluetoothSocketCocInfo codecData;
    OHOS::Parcel parcel;
    EXPECT_TRUE(codecData.Marshalling(parcel));
    EXPECT_TRUE(codecData.Unmarshalling(parcel));
    HILOGI("SocketServiceTest::Unmarshalling ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_WriteToParcel
 * @tc.name: WriteToParcel
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_WriteToParcel, TestSize.Level1)
{
    HILOGI("SocketServiceTest::WriteToParcel starts");
    BluetoothSocketCocInfo codecData;
    OHOS::Parcel parcel;
    EXPECT_TRUE(codecData.WriteToParcel(parcel));
    EXPECT_TRUE(codecData.ReadFromParcel(parcel));
    HILOGI("SocketServiceTest::WriteToParcel ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_UnregisterConnectionObserver_001
 * @tc.name: UnregisterConnectionObserver
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_UnregisterConnectionObserver_001, TestSize.Level1)
{
    HILOGI("SocketServiceTest::UnregisterConnectionObserver_001 starts");
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_SOCKET, ThreadUtil::ThreadState::NOT_SWITCH_THREAD);

    std::string addr = "11:22:33:44:55:66";
    Uuid uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    int ret = socketService->RegisterConnectionObserver(addr, uuid, nullptr);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    sleep(1);

    std::string addr1 = "11:00:00:00:00:00";
    Uuid uuid1 = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    ret = socketService->UnregisterConnectionObserver(addr1, uuid1, nullptr);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);

    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_SOCKET, ThreadUtil::ThreadState::ENABLED);
    HILOGI("SocketServiceTest::UnregisterConnectionObserver_001 ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_UnregisterConnectionObserver_002
 * @tc.name: UnregisterConnectionObserver
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_UnregisterConnectionObserver_002, TestSize.Level1)
{
    HILOGI("SocketServiceTest::UnregisterConnectionObserver_002 starts");
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_SOCKET, ThreadUtil::ThreadState::NOT_SWITCH_THREAD);
    std::string addr = "11:22:33:44:55:66";
    Uuid uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    int ret = socketService->RegisterConnectionObserver(addr, uuid, nullptr);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    sleep(1);
    ret = socketService->UnregisterConnectionObserver(addr, uuid, nullptr);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_SOCKET, ThreadUtil::ThreadState::ENABLED);
    HILOGI("SocketServiceTest::UnregisterConnectionObserver_002 ends");
}

/**
 * @tc.number:Socket_Service_UnitTest_IsAllowSocketConnect
 * @tc.name: IsAllowSocketConnect
 * @tc.desc:
 */
HWTEST_F(SocketServiceTest, Socket_Service_UnitTest_IsAllowSocketConnect, TestSize.Level1)
{
    HILOGI("SocketServiceTest::IsAllowSocketConnect starts");
    const int SOCK_L2CAP_LE = 0x02;
    std::string addr = "11:22:33:44:55:66";
    bool ret = socketService->IsAllowSocketConnect(SOCK_L2CAP_LE, addr);
    EXPECT_TRUE(ret);
    const int SOCK_RFCOMM = 0x00;
    ret = socketService->IsAllowSocketConnect(SOCK_RFCOMM, addr);
    EXPECT_TRUE(ret);
    HILOGI("SocketServiceTest::IsAllowSocketConnect ends");
}
}  // namespace Bluetooth
}  // namespace OHOS