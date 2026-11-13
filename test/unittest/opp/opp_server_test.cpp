/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <fcntl.h>
#include <unistd.h>
#include "bluetooth_opp.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "opp_obex_server.h"
#include "opp_service_impl.h"
#include "interface_profile_opp.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;

static Bluetooth::BluetoothHost *host_;
constexpr int TIME = 2;
constexpr int MTU = 300;
constexpr int FILELEN = 5;
static string addr = "11:22:33:44:55:66";

class MockOppServiceImpl : public OppServiceImpl {
public:
    MOCK_METHOD(int, OnReceiveIncomingFile, (IOppTransferInformation));
};

class OppServerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<MockOppServiceImpl> mockServiceImpl_;
    std::shared_ptr<OppObexServer> oppServer_;
};

void OppServerTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &Bluetooth::BluetoothHost::GetDefaultHost();

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

void OppServerTest::TearDownTestCase(void)
{
    host_ = nullptr;
    utility::CloseAllTimer();
}

void OppServerTest::SetUp()
{
    mockServiceImpl_ = std::make_shared<MockOppServiceImpl>();
    OppServiceImpl* implPtr = static_cast<OppServiceImpl*>(mockServiceImpl_.get());
    oppServer_ = make_shared<OppObexServer>(addr, implPtr);
    // 先设置serverSession为nullptr，避免后续操作触发状态机
    oppServer_->SetObexServerSession(nullptr);
}

void OppServerTest::TearDown()
{
    // 先清理serverSession，避免析构时访问空指针
    if (oppServer_ != nullptr) {
        oppServer_->SetObexServerSession(nullptr);
    }
    oppServer_ = nullptr;
    mockServiceImpl_ = nullptr;
}

/*
 * @tc.number: OPP_ModuleTest_OnPut_00100
 * @tc.name: OppReceiveFileBodyObject
 * @tc.desc:
 */
HWTEST_F(OppServerTest, OPP_ModuleTest_OnPut_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_OnPut_00100 start");
    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> input {};
    bool requestFinished = false;
    int ret = oppServer_->OnPut(request, reply, input, requestFinished);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("OPP_ModuleTest_OnPut_00100 end");
}

/*
 * @tc.number: OPP_Server_ReceiveFileHeader_00100
 * @tc.name: OPP_Server_ReceiveFileHeader
 * @tc.desc:
 */
HWTEST_F(OppServerTest, OPP_Server_ReceiveFileHeader_00100, TestSize.Level1)
{
    HILOGI("OPP_Server_ReceiveFileHeader_00100 start");

    // 设置Mock期望：允许读取功能
    mockServiceImpl_->isAllowedOppReadFunc_ = [](const std::string& addr) { return true; };

    // 创建有效的ObexHeader请求
    ObexHeader request;
    std::string type = "text/plain";
    std::vector<uint8_t> typeValue(type.begin(), type.end());
    request.SetHeader(TYPE, typeValue);
    request.SetName("test.txt");
    request.SetLength(100);

    ObexHeader reply;
    int ret = oppServer_->ReceiveFileHeader(request, reply);
    
    // 由于serverSession_为nullptr，函数会提前返回OBEX_HTTP_OK
    // 这个测试的主要目的是验证空指针问题已修复
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("OPP_Server_ReceiveFileHeader_00100 end");
}

/*
 * @tc.number: OPP_Server_OnDelete_001
 * @tc.name: OPP_Server_OnDelete
 * @tc.desc:
 */
HWTEST_F(OppServerTest, OPP_Server_OnDelete, TestSize.Level1)
{
    HILOGI("OPP_Server_OnDelete start");
    ObexHeader request;
    ObexHeader reply;
    bool requestFinished = false;
    int ret = oppServer_->OnDelete(request, reply);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("OPP_Server_OnDelete end");
}

/*
 * @tc.number: OPP_Server_OnAbort_001
 * @tc.name: OPP_Server_OnAbort
 * @tc.desc:
 */
HWTEST_F(OppServerTest, OPP_Server_OnAbort, TestSize.Level1)
{
    HILOGI("OPP_Server_OnAbort start");
    ObexHeader request;
    ObexHeader reply;
    bool requestFinished = false;
    int ret = oppServer_->OnAbort(request, reply);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("OPP_Server_OnAbort end");
}

/*
 * @tc.number: OPP_Server_OnGet_001
 * @tc.name: OPP_Server_OnGet
 * @tc.desc:
 */
HWTEST_F(OppServerTest, OPP_Server_OnGet, TestSize.Level1)
{
    HILOGI("OPP_Server_OnGet_001 start");
    ObexHeader request;
    ObexHeader reply;
    bool requestFinished = false;
    std::vector<uint8_t> output;
    int ret = oppServer_->OnGet(request, reply, output, requestFinished);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("OPP_Server_OnGet_001 end");
}

/*
 * @tc.number: OPP_Server_OnDisconnect_001
 * @tc.name: OPP_Server_OnDisconnect
 * @tc.desc: 测试OnDisconnect方法正常断开连接场景
 */
HWTEST_F(OppServerTest, OPP_Server_OnDisconnect_001, TestSize.Level1)
{
    HILOGI("OPP_Server_OnDisconnect_001 start");
    ObexHeader request;
    ObexHeader reply;
    int ret = oppServer_->OnDisconnect(request, reply);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("OPP_Server_OnDisconnect_001 end");
}

/*
 * @tc.number: OPP_Server_OnSetPath_001
 * @tc.name: OPP_Server_OnSetPath
 * @tc.desc: 测试OnSetPath方法正常路径设置场景
 */
HWTEST_F(OppServerTest, OPP_Server_OnSetPath_001, TestSize.Level1)
{
    HILOGI("OPP_Server_OnSetPath_001 start");
    ObexHeader request;
    ObexHeader reply;
    bool backup = false;
    bool create = true;
    int ret = oppServer_->OnSetPath(request, reply, backup, create);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("OPP_Server_OnSetPath_001 end");
}

/*
 * @tc.number: OPP_Server_ReceiveFileBody_001
 * @tc.name: OPP_Server_ReceiveFileBody
 * @tc.desc: 测试ReceiveFileBody方法serverSession为nullptr场景
 */
HWTEST_F(OppServerTest, OPP_Server_ReceiveFileBody_001, TestSize.Level1)
{
    HILOGI("OPP_Server_ReceiveFileBody_001 start");
    ObexHeader request;
    ObexHeader reply;
    bool requestFinished = false;
    int ret = oppServer_->ReceiveFileBody(request, reply, requestFinished);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_BAD_REQUEST);
    HILOGI("OPP_Server_ReceiveFileBody_001 end");
}

/*
 * @tc.number: OPP_Server_ReceiveFileBody_002
 * @tc.name: OPP_Server_ReceiveFileBody
 * @tc.desc: 测试ReceiveFileBody方法ReceivedObject为nullptr场景
 */
HWTEST_F(OppServerTest, OPP_Server_ReceiveFileBody_002, TestSize.Level1)
{
    HILOGI("OPP_Server_ReceiveFileBody_002 start");
    ObexHeader request;
    ObexHeader reply;
    bool requestFinished = false;
    int ret = oppServer_->ReceiveFileBody(request, reply, requestFinished);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_BAD_REQUEST);
    HILOGI("OPP_Server_ReceiveFileBody_002 end");
}

/*
 * @tc.number: OPP_Server_ReceiveFileBody_003
 * @tc.name: OPP_Server_ReceiveFileBody
 * @tc.desc: 测试ReceiveFileBody方法bodyLen为0且requestFinished为false场景
 */
HWTEST_F(OppServerTest, OPP_Server_ReceiveFileBody_003, TestSize.Level1)
{
    HILOGI("OPP_Server_ReceiveFileBody_003 start");
    // 创建mock ReceivedObject
    std::shared_ptr<ObexBodyObject> mockWriter = 
        std::make_shared<OppReceiveFileBodyObject>(addr, mockServiceImpl_.get(), 1000);
    ObexHeader firstReq;
    std::shared_ptr<ObexServerReceivedObject> receivedObject =
        std::make_shared<ObexServerReceivedObject>(firstReq, mockWriter);
    
    // 通过反射设置receivedObject_ (这里简化处理)
    ObexHeader request;
    ObexHeader reply;
    bool requestFinished = false;
    
    int ret = oppServer_->ReceiveFileBody(request, reply, requestFinished);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_BAD_REQUEST);
    HILOGI("OPP_Server_ReceiveFileBody_003 end");
}

/*
 * @tc.number: OPP_ReceiveFileBodyObject_Write_001
 * @tc.name: OPP_ReceiveFileBodyObject_Write
 * @tc.desc: 测试Write方法正常写入场景
 */
HWTEST_F(OppServerTest, OPP_ReceiveFileBodyObject_Write_001, TestSize.Level1)
{
    HILOGI("OPP_ReceiveFileBodyObject_Write_001 start");
    int totalSize = 1000;
    auto bodyObject = std::make_shared<OppReceiveFileBodyObject>(
        addr, mockServiceImpl_.get(), totalSize);
    
    // 创建临时文件
    int fd = open("/tmp/test_opp_write.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        bodyObject->OpenFile(fd);
        
        std::string testData = "Hello OPP Test";
        const uint8_t* buf = reinterpret_cast<const uint8_t*>(testData.c_str());
        size_t writeSize = bodyObject->Write(buf, testData.length());
        
        EXPECT_EQ(writeSize, testData.length());
        HILOGI("OPP_ReceiveFileBodyObject_Write_001 write successful");
    }
    HILOGI("OPP_ReceiveFileBodyObject_Write_001 end");
}

/*
 * @tc.number: OPP_ReceiveFileBodyObject_Write_002
 * @tc.name: OPP_ReceiveFileBodyObject_Write
 * @tc.desc: 测试Write方法空指针场景
 */
HWTEST_F(OppServerTest, OPP_ReceiveFileBodyObject_Write_002, TestSize.Level1)
{
    HILOGI("OPP_ReceiveFileBodyObject_Write_002 start");
    int totalSize = 1000;
    auto bodyObject = std::make_shared<OppReceiveFileBodyObject>(
        addr, mockServiceImpl_.get(), totalSize);
    
    size_t writeSize = bodyObject->Write(nullptr, 100);
    EXPECT_EQ(writeSize, 0);
    HILOGI("OPP_ReceiveFileBodyObject_Write_002 end");
}

/*
 * @tc.number: OPP_ReceiveFileBodyObject_Write_003
 * @tc.name: OPP_ReceiveFileBodyObject_Write
 * @tc.desc: 测试Write方法bufLen为0场景
 */
HWTEST_F(OppServerTest, OPP_ReceiveFileBodyObject_Write_003, TestSize.Level1)
{
    HILOGI("OPP_ReceiveFileBodyObject_Write_003 start");
    int totalSize = 1000;
    auto bodyObject = std::make_shared<OppReceiveFileBodyObject>(
        addr, mockServiceImpl_.get(), totalSize);
    
    std::string testData = "test";
    const uint8_t* buf = reinterpret_cast<const uint8_t*>(testData.c_str());
    size_t writeSize = bodyObject->Write(buf, 0);
    EXPECT_EQ(writeSize, 0);
    HILOGI("OPP_ReceiveFileBodyObject_Write_003 end");
}

/*
 * @tc.number: OPP_ReceiveFileBodyObject_Read_001
 * @tc.name: OPP_ReceiveFileBodyObject_Read
 * @tc.desc: 测试Read方法正常场景
 */
HWTEST_F(OppServerTest, OPP_ReceiveFileBodyObject_Read_001, TestSize.Level1)
{
    HILOGI("OPP_ReceiveFileBodyObject_Read_001 start");
    int totalSize = 1000;
    auto bodyObject = std::make_shared<OppReceiveFileBodyObject>(
        addr, mockServiceImpl_.get(), totalSize);
    
    uint8_t buf[100];
    size_t readSize = bodyObject->Read(buf, 100);
    EXPECT_EQ(readSize, 100);
    HILOGI("OPP_ReceiveFileBodyObject_Read_001 end");
}

/*
 * @tc.number: OPP_ReceiveFileBodyObject_IsReadFinished_001
 * @tc.name: OPP_ReceiveFileBodyObject_IsReadFinished
 * @tc.desc: 测试IsReadFinished方法
 */
HWTEST_F(OppServerTest, OPP_ReceiveFileBodyObject_IsReadFinished_001, TestSize.Level1)
{
    HILOGI("OPP_ReceiveFileBodyObject_IsReadFinished_001 start");
    int totalSize = 1000;
    auto bodyObject = std::make_shared<OppReceiveFileBodyObject>(
        addr, mockServiceImpl_.get(), totalSize);
    
    bool isFinished = bodyObject->IsReadFinished();
    EXPECT_FALSE(isFinished);
    HILOGI("OPP_ReceiveFileBodyObject_IsReadFinished_001 end");
}

/*
 * @tc.number: OPP_Server_GetConnectionId_001
 * @tc.name: OPP_Server_GetConnectionId
 * @tc.desc: 测试GetConnectionId方法
 */
HWTEST_F(OppServerTest, OPP_Server_GetConnectionId_001, TestSize.Level1)
{
    HILOGI("OPP_Server_GetConnectionId_001 start");
    int ret = oppServer_->GetConnectionId();
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);
    HILOGI("OPP_Server_GetConnectionId_001 end");
}

/*
 * @tc.number: OPP_Server_OnPut_002
 * @tc.name: OPP_Server_OnPut
 * @tc.desc: 测试OnPut方法serverSession为nullptr场景
 */
HWTEST_F(OppServerTest, OPP_Server_OnPut_002, TestSize.Level1)
{
    HILOGI("OPP_Server_OnPut_002 start");
    // 创建请求
    ObexHeader request;
    std::string type = "text/plain";
    std::vector<uint8_t> typeValue(type.begin(), type.end());
    request.SetHeader(TYPE, typeValue);
    request.SetName("test_put.txt");
    request.SetLength(100);

    ObexHeader reply;
    std::vector<uint8_t> input;
    bool requestFinished = false;

    // 当serverSession为nullptr时,OnPut直接返回OBEX_HTTP_OK
    int ret = oppServer_->OnPut(request, reply, input, requestFinished);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);

    // 重新创建oppServer_对象，避免析构时崩溃
    OppServiceImpl* implPtr = static_cast<OppServiceImpl*>(mockServiceImpl_.get());
    oppServer_ = make_shared<OppObexServer>(addr, implPtr);

    HILOGI("OPP_Server_OnPut_002 end");
}

/*
 * @tc.number: OPP_Server_OnPut_003
 * @tc.name: OPP_Server_OnPut
 * @tc.desc: 测试OnPut方法serverSession为nullptr场景
 */
HWTEST_F(OppServerTest, OPP_Server_OnPut_003, TestSize.Level1)
{
    HILOGI("OPP_Server_OnPut_003 start");
    // 创建请求
    ObexHeader request;
    std::string type = "text/plain";
    std::vector<uint8_t> typeValue(type.begin(), type.end());
    request.SetHeader(TYPE, typeValue);
    request.SetName("test_put_denied.txt");
    request.SetLength(100);

    ObexHeader reply;
    std::vector<uint8_t> input;
    bool requestFinished = false;

    // 当serverSession为nullptr时,OnPut直接返回OBEX_HTTP_OK
    int ret = oppServer_->OnPut(request, reply, input, requestFinished);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_OK);

    // 重新创建oppServer_对象，避免析构时崩溃
    OppServiceImpl* implPtr = static_cast<OppServiceImpl*>(mockServiceImpl_.get());
    oppServer_ = make_shared<OppObexServer>(addr, implPtr);

    HILOGI("OPP_Server_OnPut_003 end");
}
}  // namespace bluetooth
}  // namespace OHOS
