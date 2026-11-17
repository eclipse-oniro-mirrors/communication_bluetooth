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

#include <gtest/gtest.h>
#include <cstring>
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_socket_observer_stub.h"
#include "bluetooth_socket_server.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_bt_uuid.h"
#include "profile_service_manager.h"
#include "interface_profile_socket.h"
#include "socket_service.h"
#include "uuid.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
static std::unique_ptr<BluetoothSocketServer> socketServer_ {nullptr};

const int TIME = 4;

class BluetoothServerSocketObserverTest : public BluetoothServerSocketObserverStub {
public:
    BluetoothServerSocketObserverTest() = default;
    ~BluetoothServerSocketObserverTest() override = default;
};

class BluetoothClientSocketObserverTest : public BluetoothClientSocketObserverStub {
public:
    BluetoothClientSocketObserverTest() = default;
    ~BluetoothClientSocketObserverTest() override = default;
    void OnConnectionStateChanged(const CallbackParam &callbackParam) override
    {}
};

class SppServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase start");
        HILOGI("TearDownTestCase end");
    }
    void SetUp()
    {
        socketServer_ = std::make_unique<BluetoothSocketServer>();
        auto manager = ProfileServiceManager::GetInstance();
        string name = "SocketService";
        manager->SetProfileService(ADAPTER_BREDR, name);
    }
    void TearDown()
    {
        socketServer_ = nullptr;
    }
};

/*
 * @tc.number: Spp_Stub_OnRemoteRequest_001
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(SppServiceStubTest, Spp_Stub_OnRemoteRequest_001, TestSize.Level1)
{
    HILOGI("Spp_Stub_OnRemoteRequest_001 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    socketServer_->OnRemoteRequest(static_cast<uint32_t>(BluetoothSocketInterfaceCode::SOCKET_CONNECT),
        data, reply, option);

    data.WriteInterfaceToken(BluetoothSocketStub::GetDescriptor());
    int32_t ret = socketServer_->OnRemoteRequest(99, data, reply, option);
    EXPECT_EQ(ret, 305);
    HILOGI("Spp_Stub_OnRemoteRequest_001 end");
}

/*
 * @tc.number: Spp_Stub_OnRemoteRequest_Connect_001
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(SppServiceStubTest, Spp_Stub_OnRemoteRequest_Connect_001, TestSize.Level1)
{
    HILOGI("Spp_Stub_OnRemoteRequest_Connect_001 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    data.WriteInterfaceToken(BluetoothSocketStub::GetDescriptor());
    int32_t ret = socketServer_->OnRemoteRequest(static_cast<uint32_t>(BluetoothSocketInterfaceCode::SOCKET_CONNECT),
        data, reply, option);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Spp_Stub_OnRemoteRequest_Connect_001 end");
}

/*
 * @tc.number: Spp_Stub_OnRemoteRequest_Connect_002
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(SppServiceStubTest, Spp_Stub_OnRemoteRequest_Connect_002, TestSize.Level1)
{
    HILOGI("Spp_Stub_OnRemoteRequest_Connect_002 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    UUID uuid_ = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    ConnectSocketParam param {
        .addr = "00:01:02:03:04:05",
        .uuid = bluetooth::Uuid::ConvertFrom128Bits(uuid_.ConvertTo128Bits()),
        .securityFlag = 1,
        .type = 0,
        .psm = 0x80
    };

    data.WriteInterfaceToken(BluetoothSocketStub::GetDescriptor());
    data.WriteString(param.addr);
    BluetoothUuid btUuid(param.uuid);
    data.WriteParcelable(&btUuid);
    data.WriteInt32(param.securityFlag);
    data.WriteInt32(param.type);
    data.WriteInt32(param.psm);

    int32_t ret = socketServer_->OnRemoteRequest(
        static_cast<uint32_t>(BluetoothSocketInterfaceCode::SOCKET_CONNECT), data, reply, option);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Spp_Stub_OnRemoteRequest_Connect_002 end");
}

/*
 * @tc.number: Spp_Stub_OnRemoteRequest_ListenInner_001
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(SppServiceStubTest, Spp_Stub_OnRemoteRequest_ListenInner_001, TestSize.Level1)
{
    HILOGI("Spp_Stub_OnRemoteRequest_ListenInner_001 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    data.WriteInterfaceToken(BluetoothSocketStub::GetDescriptor());
    int32_t ret = socketServer_->OnRemoteRequest(static_cast<uint32_t>(BluetoothSocketInterfaceCode::SOCKET_LISTEN),
        data, reply, option);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Spp_Stub_OnRemoteRequest_ListenInner_001 end");
}

/*
 * @tc.number: Spp_Stub_OnRemoteRequest_ListenInner_002
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(SppServiceStubTest, Spp_Stub_OnRemoteRequest_ListenInner_002, TestSize.Level1)
{
    HILOGI("Spp_Stub_OnRemoteRequest_ListenInner_002 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    UUID uuid_ = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    ListenSocketParam param {
        .name = "listen",
        .uuid = bluetooth::Uuid::ConvertFrom128Bits(uuid_.ConvertTo128Bits()),
        .securityFlag = 1,
        .type = 0
    };

    data.WriteInterfaceToken(BluetoothSocketStub::GetDescriptor());
    data.WriteString(param.name);
    BluetoothUuid btUuid(param.uuid);
    data.WriteParcelable(&btUuid);
    data.WriteInt32(param.securityFlag);
    data.WriteInt32(param.type);

    data.WriteInterfaceToken(BluetoothSocketStub::GetDescriptor());
    int32_t ret = socketServer_->OnRemoteRequest(static_cast<uint32_t>(BluetoothSocketInterfaceCode::SOCKET_LISTEN),
        data, reply, option);
    EXPECT_EQ(ret, BT_NO_ERROR);

    HILOGI("Spp_Stub_OnRemoteRequest_ListenInner_002 end");
}

/*
 * @tc.number: Spp_Connect_001
 * @tc.name: Connect
 * @tc.desc: spp client connect test
 */
HWTEST_F(SppServiceStubTest, Spp_Connect_001, TestSize.Level1)
{
    HILOGI("Spp_Connect_001 start");

    UUID uuid = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    ConnectSocketParam param {
        .addr = "00:01:02:03:04:05",
        .uuid = bluetooth::Uuid::ConvertFrom128Bits(uuid.ConvertTo128Bits()),
        .securityFlag = 1,
        .type = 0,
        .psm = 0x80
    };
    int fd = -1;

    ProfileServiceManager::GetInstance()->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, "SocketService");
    int ret = socketServer_->Connect(param, fd);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("Spp_Connect_001 end");
}

/*
 * @tc.number: Spp_Listen_001
 * @tc.name: Listen
 * @tc.desc: spp server listen test
 */
HWTEST_F(SppServiceStubTest, Spp_Listen_001, TestSize.Level1)
{
    HILOGI("Spp_Listen_001 start");

    UUID uuid = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    ListenSocketParam param {
        .name = "TMP_SERVICE",
        .uuid = bluetooth::Uuid::ConvertFrom128Bits(uuid.ConvertTo128Bits()),
        .securityFlag = 1,
        .type = 0,
        .observer = nullptr
    };
    int fd = -1;

    ProfileServiceManager::GetInstance()->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, "SocketService");
    int ret = socketServer_->Listen(param, fd);
    EXPECT_EQ(ret, NO_ERROR);

    fd = 20;
    socketServer_->Listen(param, fd);

    sptr<IBluetoothServerSocketObserver> observer = new BluetoothServerSocketObserverTest();
    param.observer = observer;
    socketServer_->Listen(param, fd);

    socketServer_->DeregisterServerObserver(observer);
    socketServer_->DeregisterServerObserver(nullptr);

    HILOGI("Spp_Listen_001 end");
}

/*
 * @tc.number: Spp_test_001
 * @tc.name:
 * @tc.desc: Spp_test_001
 */
HWTEST_F(SppServiceStubTest, Spp_test_001, TestSize.Level1)
{
    HILOGI("Spp_test_001 start");

    BluetoothSocketCocInfo info;
    socketServer_->UpdateCocConnectionParams(info);

    BluetoothRawAddress addr("11:22:33:44:55:66");
    Uuid uuid = Uuid::ConvertFromString("00003301-0000-1000-8000-00805F9B34FB");
    sptr<IBluetoothClientSocketObserver> observer = new BluetoothClientSocketObserverTest();
    socketServer_->RegisterClientObserver(addr, uuid, observer);

    Uuid uuidError = Uuid::ConvertFromString("00003303-0000-1000-8000-00805F9B34FB");
    socketServer_->DeregisterClientObserver(addr, uuidError, observer);
    socketServer_->DeregisterClientObserver(addr, uuid, observer);
    EXPECT_TRUE(socketServer_ != nullptr);
    HILOGI("Spp_test_001 end");
}

/*
 * @tc.number: Spp_IsAllowSocketConnect_001
 * @tc.name: IsAllowSocketConnect
 * @tc.desc: spp client IsAllowSocketConnect test
 */
HWTEST_F(SppServiceStubTest, Spp_IsAllowSocketConnect_001, TestSize.Level1)
{
    HILOGI("Spp_IsAllowSocketConnect_001 start");
    const int SOCK_L2CAP_LE = 0x02;
    std::string addr = "11:22:33:44:55:66";
    bool isAllowed = true;
    socketServer_->IsAllowSocketConnect(SOCK_L2CAP_LE, addr, isAllowed);
    EXPECT_TRUE(isAllowed);
    const int SOCK_RFCOMM = 0x00;
    socketServer_->IsAllowSocketConnect(SOCK_RFCOMM, addr, isAllowed);
    EXPECT_TRUE(isAllowed);
    HILOGI("Spp_IsAllowSocketConnect_001 end");
}

/*
 * @tc.number: UUID_Constructor_Default_001
 * @tc.name: UUID默认构造函数测试
 * @tc.desc: 验证默认构造函数创建的UUID对象是否正确初始化
 */
HWTEST_F(SppServiceStubTest, UUID_Constructor_Default_001, TestSize.Level1)
{
    HILOGI("UUID_Constructor_Default_001 start");
    UUID uuid;
    std::string uuidStr = uuid.ToString();
    EXPECT_EQ(uuidStr, "00000000-0000-0000-0000-000000000000");
    HILOGI("UUID_Constructor_Default_001 end");
}

/*
 * @tc.number: UUID_Constructor_Param_001
 * @tc.name: UUID带参数构造函数测试
 * @tc.desc: 验证通过mostSigBits和leastSigBits构造UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_Constructor_Param_001, TestSize.Level1)
{
    HILOGI("UUID_Constructor_Param_001 start");
    const long mostSigBits = 0x0000000000001000L;
    const long leastSigBits = 0x800000005F9B34FBL;
    UUID uuid(mostSigBits, leastSigBits);
    std::string uuidStr = uuid.ToString();
    EXPECT_EQ(uuidStr, "00000000-0000-1000-8000-00005F9B34FB");
    HILOGI("UUID_Constructor_Param_001 end");
}

/*
 * @tc.number: UUID_Constructor_Param_002
 * @tc.name: UUID带参数构造函数全零测试
 * @tc.desc: 验证通过全零参数构造UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_Constructor_Param_002, TestSize.Level1)
{
    HILOGI("UUID_Constructor_Param_002 start");
    const long mostSigBits = 0L;
    const long leastSigBits = 0L;
    UUID uuid(mostSigBits, leastSigBits);
    std::string uuidStr = uuid.ToString();
    EXPECT_EQ(uuidStr, "00000000-0000-0000-0000-000000000000");
    HILOGI("UUID_Constructor_Param_002 end");
}

/*
 * @tc.number: UUID_Constructor_Param_003
 * @tc.name: UUID带参数构造函数最大值测试
 * @tc.desc: 验证通过最大值参数构造UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_Constructor_Param_003, TestSize.Level1)
{
    HILOGI("UUID_Constructor_Param_003 start");
    const long mostSigBits = 0xFFFFFFFFFFFFFFFFL;
    const long leastSigBits = 0xFFFFFFFFFFFFFFFFL;
    UUID uuid(mostSigBits, leastSigBits);
    std::string uuidStr = uuid.ToString();
    EXPECT_EQ(uuidStr, "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    HILOGI("UUID_Constructor_Param_003 end");
}

/*
 * @tc.number: UUID_FromString_001
 * @tc.name: FromString方法测试-有效UUID字符串
 * @tc.desc: 验证从标准UUID字符串创建UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_FromString_001, TestSize.Level1)
{
    HILOGI("UUID_FromString_001 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34FB";
    UUID uuid = UUID::FromString(uuidStr);
    EXPECT_EQ(uuid.ToString(), uuidStr);
    HILOGI("UUID_FromString_001 end");
}

/*
 * @tc.number: UUID_FromString_002
 * @tc.name: FromString方法测试-空字符串
 * @tc.desc: 验证从空字符串创建UUID对象返回默认值
 */
HWTEST_F(SppServiceStubTest, UUID_FromString_002, TestSize.Level1)
{
    HILOGI("UUID_FromString_002 start");
    std::string uuidStr = "";
    UUID uuid = UUID::FromString(uuidStr);
    EXPECT_EQ(uuid.ToString(), "00000000-0000-0000-0000-000000000000");
    HILOGI("UUID_FromString_002 end");
}

/*
 * @tc.number: UUID_FromString_003
 * @tc.name: FromString方法测试-无效格式
 * @tc.desc: 验证从无效格式字符串创建UUID对象返回默认值
 */
HWTEST_F(SppServiceStubTest, UUID_FromString_003, TestSize.Level1)
{
    HILOGI("UUID_FromString_003 start");
    std::string uuidStr = "invalid-uuid-format";
    UUID uuid = UUID::FromString(uuidStr);
    EXPECT_EQ(uuid.ToString(), "00000000-0000-0000-0000-000000000000");
    HILOGI("UUID_FromString_003 end");
}

/*
 * @tc.number: UUID_FromString_004
 * @tc.name: FromString方法测试-小写字母
 * @tc.desc: 验证从小写字母UUID字符串创建UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_FromString_004, TestSize.Level1)
{
    HILOGI("UUID_FromString_004 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805f9b34fb";
    UUID uuid = UUID::FromString(uuidStr);
    std::string result = uuid.ToString();
    EXPECT_EQ(result, "00001101-0000-1000-8000-00805F9B34FB");
    HILOGI("UUID_FromString_004 end");
}

/*
 * @tc.number: UUID_FromString_005
 * @tc.name: FromString方法测试-混合大小写
 * @tc.desc: 验证从混合大小写UUID字符串创建UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_FromString_005, TestSize.Level1)
{
    HILOGI("UUID_FromString_005 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9b34Fb";
    UUID uuid = UUID::FromString(uuidStr);
    std::string result = uuid.ToString();
    EXPECT_EQ(result, "00001101-0000-1000-8000-00805F9B34FB");
    HILOGI("UUID_FromString_005 end");
}

/*
 * @tc.number: UUID_RandomUUID_001
 * @tc.name: RandomUUID方法测试
 * @tc.desc: 验证生成随机UUID是否成功且格式正确
 */
HWTEST_F(SppServiceStubTest, UUID_RandomUUID_001, TestSize.Level1)
{
    HILOGI("UUID_RandomUUID_001 start");
    UUID uuid1 = UUID::RandomUUID();
    sleep(1);  // 添加延迟确保时间戳不同
    UUID uuid2 = UUID::RandomUUID();
    std::string uuidStr1 = uuid1.ToString();
    std::string uuidStr2 = uuid2.ToString();
    EXPECT_NE(uuidStr1, uuidStr2);
    EXPECT_TRUE(IsValidUuid(uuidStr1));
    EXPECT_TRUE(IsValidUuid(uuidStr2));
    HILOGI("UUID_RandomUUID_001 end");
}

/*
 * @tc.number: UUID_ConvertFrom128Bits_001
 * @tc.name: ConvertFrom128Bits方法测试
 * @tc.desc: 验证从128位数组创建UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_ConvertFrom128Bits_001, TestSize.Level1)
{
    HILOGI("UUID_ConvertFrom128Bits_001 start");
    std::array<uint8_t, 16> bytes = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    UUID uuid = UUID::ConvertFrom128Bits(bytes);
    std::string uuidStr = uuid.ToString();
    EXPECT_EQ(uuidStr, "00010203-0405-0607-0809-0A0B0C0D0E0F");
    HILOGI("UUID_ConvertFrom128Bits_001 end");
}

/*
 * @tc.number: UUID_ConvertFrom128Bits_002
 * @tc.name: ConvertFrom128Bits方法测试-全零
 * @tc.desc: 验证从全零数组创建UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_ConvertFrom128Bits_002, TestSize.Level1)
{
    HILOGI("UUID_ConvertFrom128Bits_002 start");
    std::array<uint8_t, 16> bytes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    UUID uuid = UUID::ConvertFrom128Bits(bytes);
    std::string uuidStr = uuid.ToString();
    EXPECT_EQ(uuidStr, "00000000-0000-0000-0000-000000000000");
    HILOGI("UUID_ConvertFrom128Bits_002 end");
}

/*
 * @tc.number: UUID_ConvertFrom128Bits_003
 * @tc.name: ConvertFrom128Bits方法测试-最大值
 * @tc.desc: 验证从最大值数组创建UUID对象是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_ConvertFrom128Bits_003, TestSize.Level1)
{
    HILOGI("UUID_ConvertFrom128Bits_003 start");
    std::array<uint8_t, 16> bytes = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    UUID uuid = UUID::ConvertFrom128Bits(bytes);
    std::string uuidStr = uuid.ToString();
    EXPECT_EQ(uuidStr, "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
    HILOGI("UUID_ConvertFrom128Bits_003 end");
}

/*
 * @tc.number: UUID_ToString_001
 * @tc.name: ToString方法测试
 * @tc.desc: 验证将UUID对象转换为字符串是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_ToString_001, TestSize.Level1)
{
    HILOGI("UUID_ToString_001 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34FB";
    UUID uuid = UUID::FromString(uuidStr);
    std::string result = uuid.ToString();
    EXPECT_EQ(result, uuidStr);
    HILOGI("UUID_ToString_001 end");
}

/*
 * @tc.number: UUID_ToString_002
 * @tc.name: ToString方法测试-默认UUID
 * @tc.desc: 验证默认UUID转换为字符串是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_ToString_002, TestSize.Level1)
{
    HILOGI("UUID_ToString_002 start");
    UUID uuid;
    std::string result = uuid.ToString();
    EXPECT_EQ(result, "00000000-0000-0000-0000-000000000000");
    HILOGI("UUID_ToString_002 end");
}

/*
 * @tc.number: UUID_CompareTo_001
 * @tc.name: CompareTo方法测试-相等
 * @tc.desc: 验证比较两个相同UUID返回0
 */
HWTEST_F(SppServiceStubTest, UUID_CompareTo_001, TestSize.Level1)
{
    HILOGI("UUID_CompareTo_001 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34FB";
    UUID uuid1 = UUID::FromString(uuidStr);
    UUID uuid2 = UUID::FromString(uuidStr);
    int result = uuid1.CompareTo(uuid2);
    EXPECT_EQ(result, 0);
    HILOGI("UUID_CompareTo_001 end");
}

/*
 * @tc.number: UUID_CompareTo_002
 * @tc.name: CompareTo方法测试-大于
 * @tc.desc: 验证比较两个不同UUID返回正确值
 */
HWTEST_F(SppServiceStubTest, UUID_CompareTo_002, TestSize.Level1)
{
    HILOGI("UUID_CompareTo_002 start");
    UUID uuid1 = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    UUID uuid2 = UUID::FromString("00001102-0000-1000-8000-00805F9B34FB");
    int result = uuid1.CompareTo(uuid2);
    EXPECT_LT(result, 0);
    HILOGI("UUID_CompareTo_002 end");
}

/*
 * @tc.number: UUID_CompareTo_003
 * @tc.name: CompareTo方法测试-小于
 * @tc.desc: 验证比较两个不同UUID返回正确值
 */
HWTEST_F(SppServiceStubTest, UUID_CompareTo_003, TestSize.Level1)
{
    HILOGI("UUID_CompareTo_003 start");
    UUID uuid1 = UUID::FromString("00001102-0000-1000-8000-00805F9B34FB");
    UUID uuid2 = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    int result = uuid1.CompareTo(uuid2);
    EXPECT_GT(result, 0);
    HILOGI("UUID_CompareTo_003 end");
}

/*
 * @tc.number: UUID_Equals_001
 * @tc.name: Equals方法测试-相等
 * @tc.desc: 验证两个相同UUID的Equals方法返回true
 */
HWTEST_F(SppServiceStubTest, UUID_Equals_001, TestSize.Level1)
{
    HILOGI("UUID_Equals_001 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34FB";
    UUID uuid1 = UUID::FromString(uuidStr);
    UUID uuid2 = UUID::FromString(uuidStr);
    bool result = uuid1.Equals(uuid2);
    EXPECT_TRUE(result);
    HILOGI("UUID_Equals_001 end");
}

/*
 * @tc.number: UUID_Equals_002
 * @tc.name: Equals方法测试-不相等
 * @tc.desc: 验证两个不同UUID的Equals方法返回false
 */
HWTEST_F(SppServiceStubTest, UUID_Equals_002, TestSize.Level1)
{
    HILOGI("UUID_Equals_002 start");
    UUID uuid1 = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    UUID uuid2 = UUID::FromString("00001102-0000-1000-8000-00805F9B34FB");
    bool result = uuid1.Equals(uuid2);
    EXPECT_FALSE(result);
    HILOGI("UUID_Equals_002 end");
}

/*
 * @tc.number: UUID_Equals_003
 * @tc.name: Equals方法测试-默认UUID
 * @tc.desc: 验证两个默认UUID的Equals方法返回true
 */
HWTEST_F(SppServiceStubTest, UUID_Equals_003, TestSize.Level1)
{
    HILOGI("UUID_Equals_003 start");
    UUID uuid1;
    UUID uuid2;
    bool result = uuid1.Equals(uuid2);
    EXPECT_TRUE(result);
    HILOGI("UUID_Equals_003 end");
}

/*
 * @tc.number: UUID_GetLeastSignificantBits_001
 * @tc.name: GetLeastSignificantBits方法测试
 * @tc.desc: 验证获取UUID低位64位是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_GetLeastSignificantBits_001, TestSize.Level1)
{
    HILOGI("UUID_GetLeastSignificantBits_001 start");
    const long mostSigBits = 0x0000000000001000L;
    const long leastSigBits = 0x800000005F9B34FBL;
    UUID uuid(mostSigBits, leastSigBits);
    uint64_t result = uuid.GetLeastSignificantBits();
    EXPECT_EQ(result, static_cast<uint64_t>(leastSigBits));
    HILOGI("UUID_GetLeastSignificantBits_001 end");
}

/*
 * @tc.number: UUID_GetMostSignificantBits_001
 * @tc.name: GetMostSignificantBits方法测试
 * @tc.desc: 验证获取UUID高位64位是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_GetMostSignificantBits_001, TestSize.Level1)
{
    HILOGI("UUID_GetMostSignificantBits_001 start");
    const long mostSigBits = 0x0000000000001000L;
    const long leastSigBits = 0x800000005F9B34FBL;
    UUID uuid(mostSigBits, leastSigBits);
    uint64_t result = uuid.GetMostSignificantBits();
    EXPECT_EQ(result, static_cast<uint64_t>(mostSigBits));
    HILOGI("UUID_GetMostSignificantBits_001 end");
}

/*
 * @tc.number: UUID_ConvertTo128Bits_001
 * @tc.name: ConvertTo128Bits方法测试
 * @tc.desc: 验证将UUID对象转换为128位数组是否正确
 */
HWTEST_F(SppServiceStubTest, UUID_ConvertTo128Bits_001, TestSize.Level1)
{
    HILOGI("UUID_ConvertTo128Bits_001 start");
    std::array<uint8_t, 16> bytes = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    UUID uuid = UUID::ConvertFrom128Bits(bytes);
    std::array<uint8_t, 16> result = uuid.ConvertTo128Bits();
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(result[i], bytes[i]);
    }
    HILOGI("UUID_ConvertTo128Bits_001 end");
}

/*
 * @tc.number: UUID_ConvertTo128Bits_002
 * @tc.name: ConvertTo128Bits方法测试-往返转换
 * @tc.desc: 验证UUID往返转换的正确性
 */
HWTEST_F(SppServiceStubTest, UUID_ConvertTo128Bits_002, TestSize.Level1)
{
    HILOGI("UUID_ConvertTo128Bits_002 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34FB";
    UUID uuid1 = UUID::FromString(uuidStr);
    std::array<uint8_t, 16> bytes = uuid1.ConvertTo128Bits();
    UUID uuid2 = UUID::ConvertFrom128Bits(bytes);
    EXPECT_EQ(uuid1.ToString(), uuid2.ToString());
    HILOGI("UUID_ConvertTo128Bits_002 end");
}

/*
 * @tc.number: UUID_OperatorEqual_001
 * @tc.name: operator==运算符测试-相等
 * @tc.desc: 验证两个相同UUID的==运算符返回true
 */
HWTEST_F(SppServiceStubTest, UUID_OperatorEqual_001, TestSize.Level1)
{
    HILOGI("UUID_OperatorEqual_001 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34FB";
    UUID uuid1 = UUID::FromString(uuidStr);
    UUID uuid2 = UUID::FromString(uuidStr);
    bool result = (uuid1 == uuid2);
    EXPECT_TRUE(result);
    HILOGI("UUID_OperatorEqual_001 end");
}

/*
 * @tc.number: UUID_OperatorEqual_002
 * @tc.name: operator==运算符测试-不相等
 * @tc.desc: 验证两个不同UUID的==运算符返回false
 */
HWTEST_F(SppServiceStubTest, UUID_OperatorEqual_002, TestSize.Level1)
{
    HILOGI("UUID_OperatorEqual_002 start");
    UUID uuid1 = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    UUID uuid2 = UUID::FromString("00001102-0000-1000-8000-00805F9B34FB");
    bool result = (uuid1 == uuid2);
    EXPECT_FALSE(result);
    HILOGI("UUID_OperatorEqual_002 end");
}

/*
 * @tc.number: UUID_OperatorEqual_003
 * @tc.name: operator==运算符测试-默认UUID
 * @tc.desc: 验证两个默认UUID的==运算符返回true
 */
HWTEST_F(SppServiceStubTest, UUID_OperatorEqual_003, TestSize.Level1)
{
    HILOGI("UUID_OperatorEqual_003 start");
    UUID uuid1;
    UUID uuid2;
    bool result = (uuid1 == uuid2);
    EXPECT_TRUE(result);
    HILOGI("UUID_OperatorEqual_003 end");
}

/*
 * @tc.number: UUID_OperatorLess_001
 * @tc.name: operator<运算符测试-不相等
 * @tc.desc: 验证两个不同UUID的<运算符返回true
 */
HWTEST_F(SppServiceStubTest, UUID_OperatorLess_001, TestSize.Level1)
{
    HILOGI("UUID_OperatorLess_001 start");
    UUID uuid1 = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    UUID uuid2 = UUID::FromString("00001102-0000-1000-8000-00805F9B34FB");
    bool result = (uuid1 < uuid2);
    EXPECT_TRUE(result);
    HILOGI("UUID_OperatorLess_001 end");
}

/*
 * @tc.number: UUID_OperatorLess_002
 * @tc.name: operator<运算符测试-相等
 * @tc.desc: 验证两个相同UUID的<运算符返回false
 */
HWTEST_F(SppServiceStubTest, UUID_OperatorLess_002, TestSize.Level1)
{
    HILOGI("UUID_OperatorLess_002 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34FB";
    UUID uuid1 = UUID::FromString(uuidStr);
    UUID uuid2 = UUID::FromString(uuidStr);
    bool result = (uuid1 < uuid2);
    EXPECT_FALSE(result);
    HILOGI("UUID_OperatorLess_002 end");
}

/*
 * @tc.number: UUID_OperatorLess_003
 * @tc.name: operator<运算符测试-反向比较
 * @tc.desc: 验证反向比较两个不同UUID的<运算符返回false
 */
HWTEST_F(SppServiceStubTest, UUID_OperatorLess_003, TestSize.Level1)
{
    HILOGI("UUID_OperatorLess_003 start");
    UUID uuid1 = UUID::FromString("00001102-0000-1000-8000-00805F9B34FB");
    UUID uuid2 = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    bool result = (uuid1 < uuid2);
    EXPECT_TRUE(result);
    HILOGI("UUID_OperatorLess_003 end");
}

/*
 * @tc.number: UUID_IsValidUuid_001
 * @tc.name: IsValidUuid函数测试-有效UUID
 * @tc.desc: 验证有效的UUID字符串返回true
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_001, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_001 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34FB";
    bool result = IsValidUuid(uuidStr);
    EXPECT_TRUE(result);
    HILOGI("UUID_IsValidUuid_001 end");
}

/*
 * @tc.number: UUID_IsValidUuid_002
 * @tc.name: IsValidUuid函数测试-小写字母
 * @tc.desc: 验证小写字母的UUID字符串返回true
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_002, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_002 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805f9b34fb";
    bool result = IsValidUuid(uuidStr);
    EXPECT_TRUE(result);
    HILOGI("UUID_IsValidUuid_002 end");
}

/*
 * @tc.number: UUID_IsValidUuid_003
 * @tc.name: IsValidUuid函数测试-混合大小写
 * @tc.desc: 验证混合大小写的UUID字符串返回true
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_003, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_003 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34Fb";
    bool result = IsValidUuid(uuidStr);
    EXPECT_TRUE(result);
    HILOGI("UUID_IsValidUuid_003 end");
}

/*
 * @tc.number: UUID_IsValidUuid_004
 * @tc.name: IsValidUuid函数测试-空字符串
 * @tc.desc: 验证空字符串返回false
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_004, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_004 start");
    std::string uuidStr = "";
    bool result = IsValidUuid(uuidStr);
    EXPECT_FALSE(result);
    HILOGI("UUID_IsValidUuid_004 end");
}

/*
 * @tc.number: UUID_IsValidUuid_005
 * @tc.name: IsValidUuid函数测试-缺少分隔符
 * @tc.desc: 验证缺少分隔符的字符串返回false
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_005, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_005 start");
    std::string uuidStr = "0000110100001000800000805F9B34FB";
    bool result = IsValidUuid(uuidStr);
    EXPECT_FALSE(result);
    HILOGI("UUID_IsValidUuid_005 end");
}

/*
 * @tc.number: UUID_IsValidUuid_006
 * @tc.name: IsValidUuid函数测试-长度不足
 * @tc.desc: 验证长度不足的字符串返回false
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_006, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_006 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805F9B34F";
    bool result = IsValidUuid(uuidStr);
    EXPECT_FALSE(result);
    HILOGI("UUID_IsValidUuid_006 end");
}

/*
 * @tc.number: UUID_IsValidUuid_007
 * @tc.name: IsValidUuid函数测试-包含非法字符
 * @tc.desc: 验证包含非法字符的字符串返回false
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_007, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_007 start");
    std::string uuidStr = "00001101-0000-1000-8000-00805G9B34FB";
    bool result = IsValidUuid(uuidStr);
    EXPECT_FALSE(result);
    HILOGI("UUID_IsValidUuid_007 end");
}

/*
 * @tc.number: UUID_IsValidUuid_008
 * @tc.name: IsValidUuid函数测试-全零UUID
 * @tc.desc: 验证全零UUID字符串返回true
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_008, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_008 start");
    std::string uuidStr = "00000000-0000-0000-0000-000000000000";
    bool result = IsValidUuid(uuidStr);
    EXPECT_TRUE(result);
    HILOGI("UUID_IsValidUuid_008 end");
}

/*
 * @tc.number: UUID_IsValidUuid_009
 * @tc.name: IsValidUuid函数测试-最大值UUID
 * @tc.desc: 验证最大值UUID字符串返回true
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_009, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_009 start");
    std::string uuidStr = "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF";
    bool result = IsValidUuid(uuidStr);
    EXPECT_TRUE(result);
    HILOGI("UUID_IsValidUuid_009 end");
}

/*
 * @tc.number: UUID_IsValidUuid_010
 * @tc.name: IsValidUuid函数测试-分隔符位置错误
 * @tc.desc: 验证分隔符位置错误的字符串返回false
 */
HWTEST_F(SppServiceStubTest, UUID_IsValidUuid_010, TestSize.Level1)
{
    HILOGI("UUID_IsValidUuid_010 start");
    std::string uuidStr = "0000-1101-0000-1000-8000-00805F9B34FB";
    bool result = IsValidUuid(uuidStr);
    EXPECT_FALSE(result);
    HILOGI("UUID_IsValidUuid_010 end");
}
}  // namespace bluetooth
}  // namespace OHOS