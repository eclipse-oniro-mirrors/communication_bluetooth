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
#include <future>
#include <memory>
#include <thread>

#include "btgatt_client_interface_mock.h"
#include "gatt_server_application.h"
#include "gatt_server_service.h"
#include "bluetooth_gatt_interface.h"
#include "btgatt_interface_inner.h"
#include "bluetooth_host.h"
#include "gatt_server_callback_mock.h"
#include "profile_service_manager.h"
#include "gatt_data.h"
#include "bluetooth_log.h"
#include "thread_util.h"
#include "btif_gatt.h"

using namespace testing;
using namespace testing::ext;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

extern "C" {
static const btgatt_callbacks_t* g_btgattCallbacks = nullptr;
static bt_status_t BtgattInterfaceInitMock(const btgatt_callbacks_t* callbacks)
{
    g_btgattCallbacks = callbacks;
    return BT_STATUS_SUCCESS;
}
static void BtgattInterfaceCleanupMock(void)
{
}

static btgatt_interface_t g_btgattInterface = {
    .size = sizeof(g_btgattInterface),
    .init = BtgattInterfaceInitMock,
    .cleanup = BtgattInterfaceCleanupMock,
    .client = BtGetGattClientInterface(),
    .server = BtGetGattServerInterface(),
    .scanner = nullptr,    // filled in btif_gatt_get_interface
    .advertiser = nullptr  // filled in btif_gatt_get_interface
};
} // extern "C"

namespace OHOS {
namespace bluetooth {
const uint8_t DUMMY_ADDRESS[6] { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
const int DUMMY_HANDLE = 0xFF;
const int DUMMY_CONN_ID = 0xEE;
const int DUMMY_TRANS_ID = 0x04;
static Bluetooth::BluetoothHost *host_;
constexpr int TIME = 2;
class GattServerApplicationTest : public testing::Test {
public:
    GattServerApplicationTest() = default;
    ~GattServerApplicationTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    void SetConnStateConnected(int connectId);

    std::shared_ptr<GattServerApplication> gattServerApplication_ {nullptr};
    GattServerService *gattServerService_ {nullptr};
    int serverIf = 1;
    GattDevice device;
    std::shared_ptr<GattServerCallbackMock> callback_ {nullptr};
    int tokenId_ = 1;
    OHOS::bluetooth::BtgattServerInterfaceMock gBtgattServerInterfaceMock;
private:
};

void GattServerApplicationTest::SetUpTestSuite(void)
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

    BluetoothGattInterface::GetInstance()->Initialize(&g_btgattInterface);

    HILOGI("SetUpTestCase end");
}

void GattServerApplicationTest::TearDownTestSuite(void)
{
    BluetoothGattInterface::GetInstance()->ClearGattServerInValidObserver();
    sleep(TIME);
    host_ = nullptr;
}

void GattServerApplicationTest::SetUp()
{
    callback_ = std::make_shared<GattServerCallbackMock>();

    auto manager = ProfileServiceManager::GetInstance();
    std::string profileName = "GattServerService";
    manager->SetProfileService(ADAPTER_BLE, profileName);
    gattServerService_ = static_cast<GattServerService *>(manager->GetProfileService(profileName));

    RegisterGattServerInterfaceMock(
        [this](const btgatt_server_interface_t *interface) {
           gattServerApplication_ = std::make_shared<GattServerApplication>(serverIf, interface, tokenId_, callback_,
               [](int appId, const Service &service) {
                   HILOGI("TestFunction %d", appId);
               });
            BluetoothGattInterface::GetInstance()->AddGattServerObserver(gattServerApplication_);
        },
        &gBtgattServerInterfaceMock);

    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::ENABLED);
    // Wait for started
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_NE(g_btgattCallbacks, nullptr);

    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::ENABLED);
}

void GattServerApplicationTest::TearDown()
{
    EXPECT_CALL(gBtgattServerInterfaceMock, UnregisterServer(_))
        .WillRepeatedly(Return(BT_STATUS_SUCCESS));
    UnRegisterGattServerInterfaceMock();
    gattServerService_ = nullptr;
    callback_ = nullptr;
    gattServerApplication_ = nullptr;
}

static constexpr int WAIT_TIMEOUT = 1100; // 1.1s

void GattServerApplicationTest::SetConnStateConnected(int connectId)
{
    int connId = connectId;
    int connected = 1;
    int serverId = 1;
    int reason = 1;
    gattServerApplication_->ConnectionCallback(
        connId, serverId, connected, STACK::RawAddress(DUMMY_ADDRESS), reason);
}

/**
 * @tc.name: gatt_server_application_BuildGattService_001
 * @tc.desc: gatt_server_application_BuildGattService_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_BuildGattService_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_BuildGattService_001 start");
    std::vector<btgatt_db_element_t> svc;
    btgatt_db_element_t element;
    element.id = 0;
    element.type = BTGATT_DB_PRIMARY_SERVICE;
    element.attribute_handle = 0;
    element.start_handle = 0x0001;
    element.end_handle = 0x0007;
    element.properties = 0;
    element.extended_properties = 0;
    element.permissions = 0;
    svc.push_back(element);
    btgatt_db_element_t element1;
    element.type = BTGATT_DB_SECONDARY_SERVICE;
    svc.push_back(element1);
    Service service;
    gattServerApplication_->BuildGattService(svc, service);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_BuildGattService_001 end");
}

/**
 * @tc.name: gatt_server_application_BuildGattService_002
 * @tc.desc: gatt_server_application_BuildGattService_002
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_BuildGattService_002, TestSize.Level1)
{
    HILOGI("gatt_server_application_BuildGattService_002 start");
    std::vector<btgatt_db_element_t> svc;
    btgatt_db_element_t element;
    element.id = 0;
    element.type = BTGATT_DB_CHARACTERISTIC;
    element.attribute_handle = 0;
    element.start_handle = 0x0001;
    element.end_handle = 0x0007;
    element.properties = 0;
    element.extended_properties = 0;
    element.permissions = 0;
    svc.push_back(element);
    btgatt_db_element_t element1;
    element1.id = 0;
    element1.type = BTGATT_DB_CHARACTERISTIC;
    element1.attribute_handle = 0;
    element1.start_handle = 0x0001;
    element1.end_handle = 0x0007;
    element1.properties = 0;
    element1.extended_properties = 1;
    element1.permissions = 1;
    svc.push_back(element1);
    btgatt_db_element_t element2;
    element2.type = BTGATT_DB_DESCRIPTOR;
    svc.push_back(element2);
    Service service;
    gattServerApplication_->BuildGattService(svc, service);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_BuildGattService_002 end");
}

/**
 * @tc.name: gatt_server_application_BuildGattService_005
 * @tc.desc: gatt_server_application_BuildGattService_005
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_BuildGattService_005, TestSize.Level1)
{
    HILOGI("gatt_server_application_BuildGattService_005 start");
    std::vector<btgatt_db_element_t> svc;
    btgatt_db_element_t element;
    element.id = 0;
    element.type = BTGATT_DB_INCLUDED_SERVICE;
    element.attribute_handle = 0;
    element.start_handle = 0x0001;
    element.end_handle = 0x0007;
    element.properties = 0;
    element.extended_properties = 0;
    element.permissions = 0;
    svc.push_back(element);
    Service service;
    gattServerApplication_->BuildGattService(svc, service);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_BuildGattService_005 end");
}

/**
 * @tc.name: gatt_server_application_ServiceAddedCallback_001
 * @tc.desc: gatt_server_application_ServiceAddedCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ServiceAddedCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ServiceAddedCallback_001 start");
    std::vector<btgatt_db_element_t> svc;
    btgatt_db_element_t element;
    element.id = 0;
    element.type = BTGATT_DB_INCLUDED_SERVICE;
    element.attribute_handle = 0;
    element.start_handle = 0x0001;
    element.end_handle = 0x0007;
    element.properties = 0;
    element.extended_properties = 0;
    element.permissions = 0;
    svc.push_back(element);
    int status = GATT_SUCCESS;
    int serverIf = 0;
    gattServerApplication_->ServiceAddedCallback(status, serverIf, svc);
    serverIf = 1;
    gattServerApplication_->ServiceAddedCallback(status, serverIf, svc);
    sleep(1);
    status = GATT_FAILURE;
    gattServerApplication_->ServiceAddedCallback(status, serverIf, svc);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->ServiceAddedCallback(status, serverIf, svc);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ServiceAddedCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_AddService_001
 * @tc.desc: gatt_server_application_AddService_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_AddService_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_AddService_001 start");
    EXPECT_CALL(gBtgattServerInterfaceMock, AddService(_, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    Service service;
    gattServerApplication_->AddService(service);
    gattServerApplication_->AddService(service);
    gattServerApplication_->btIfGattServer_ = nullptr;
    gattServerApplication_->AddService(service);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_AddService_001 end");
}

/**
 * @tc.name: gatt_server_application_ServiceDeletedCallback_001
 * @tc.desc: gatt_server_application_ServiceDeletedCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ServiceDeletedCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ServiceDeletedCallback_001 start");
    int serviceHandle = 1;
    int status = GATT_SUCCESS;
    int serverIf = 0;
    gattServerApplication_->ServiceDeletedCallback(status, serverIf, serviceHandle);
    serverIf = 1;
    gattServerApplication_->ServiceDeletedCallback(status, serverIf, serviceHandle);
    status = GATT_FAILURE;
    sleep(1);
    gattServerApplication_->ServiceDeletedCallback(status, serverIf, serviceHandle);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->ServiceDeletedCallback(status, serverIf, serviceHandle);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ServiceDeletedCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_RemoveService_001
 * @tc.desc: gatt_server_application_RemoveService_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RemoveService_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RemoveService_001 start");
    EXPECT_CALL(gBtgattServerInterfaceMock, DeleteService(_, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    uint16_t handle = 1;
    gattServerApplication_->RemoveService(handle);
    gattServerApplication_->RemoveService(handle);
    gattServerApplication_->btIfGattServer_ = nullptr;
    gattServerApplication_->RemoveService(handle);
    HILOGI("gatt_server_application_RemoveService_001 end");
}

/**
 * @tc.name: gatt_server_application_ConnectionCallback_001
 * @tc.desc: gatt_server_application_ConnectionCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ConnectionCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ConnectionCallback_001 start");
    int connId = 1;
    int connected = 1;
    int serverIf = 0;
    int reason = 1;
    gattServerApplication_->ConnectionCallback(
        connId, serverIf, connected, STACK::RawAddress(DUMMY_ADDRESS), reason);
    serverIf = 1;
    gattServerApplication_->ConnectionCallback(
        connId, serverIf, connected, STACK::RawAddress(DUMMY_ADDRESS), reason);
    sleep(1);
    connected = 0;
    gattServerApplication_->ConnectionCallback(
        connId, serverIf, connected, STACK::RawAddress(DUMMY_ADDRESS), reason);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->ConnectionCallback(
        connId, serverIf, connected, STACK::RawAddress(DUMMY_ADDRESS), reason);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ConnectionCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_RequestReadCallback_001
 * @tc.desc: gatt_server_application_RequestReadCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RequestReadCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RequestReadCallback_001 start");
    RawAddress addr("22:33:44:55:66:77");
    GattServerApplication::RequestReadCallbackContext context = {
    .connId = 1,
    .transId = 456,
    .addr = addr,
    .handle = 789,
    .offset = 0,
    .isLong = true
    };
    GattServerApplication::GattElement element = GattServerApplication::GattElement::SERVICE;
    gattServerApplication_->RequestReadCallback(context, element);
    context.isLong = false;
    gattServerApplication_->RequestReadCallback(context, element);
    SetConnStateConnected(context.connId);
    sleep(1);
    gattServerApplication_->RequestReadCallback(context, element);
    element = GattServerApplication::GattElement::CHARACTERISTIC;
    gattServerApplication_->RequestReadCallback(context, element);
    element = GattServerApplication::GattElement::DESCRIPTOR;
    gattServerApplication_->RequestReadCallback(context, element);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RequestReadCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_RequestReadCharacteristicCallback_001
 * @tc.desc: gatt_server_application_RequestReadCharacteristicCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RequestReadCharacteristicCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RequestReadCharacteristicCallback_001 start");
    int connId = 1;
    int transId = 2;
    STACK::RawAddress bda;
    int attrHandle = 0x1234;
    int offset = 0;
    bool isLong = false;
    gattServerApplication_->RequestReadCharacteristicCallback(connId, transId, bda, attrHandle, offset, isLong);
    connId = 2;
    SetConnStateConnected(connId);
    sleep(1);
    gattServerApplication_->RequestReadCharacteristicCallback(connId, transId, bda, attrHandle, offset, isLong);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->RequestReadCharacteristicCallback(connId, transId, bda, attrHandle, offset, isLong);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RequestReadCharacteristicCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_RespondCharacteristicRead_001
 * @tc.desc: gatt_server_application_RespondCharacteristicRead_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RespondCharacteristicRead_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RespondCharacteristicRead_001 start");
    RawAddress addr("11:22:33:44:55:66");
    uint16_t handle = 1;
    std::vector<uint8_t> value;
    int ret = 1;
    gattServerApplication_->RespondCharacteristicRead(addr, handle, value, ret);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->RespondCharacteristicRead(addr, handle, value, ret);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RespondCharacteristicRead_001 end");
}

/**
 * @tc.name: gatt_server_application_RequestReadDescriptorCallback_001
 * @tc.desc: gatt_server_application_RequestReadDescriptorCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RequestReadDescriptorCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RequestReadDescriptorCallback_001 start");
    int connId = 1;
    int transId = 2;
    STACK::RawAddress bda;
    int attrHandle = 0x1234;
    int offset = 0;
    bool isLong = false;
    gattServerApplication_->RequestReadDescriptorCallback(connId, transId, bda, attrHandle, offset, isLong);
    SetConnStateConnected(connId);
    sleep(1);
    gattServerApplication_->RequestReadDescriptorCallback(connId, transId, bda, attrHandle, offset, isLong);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->RequestReadDescriptorCallback(connId, transId, bda, attrHandle, offset, isLong);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RequestReadDescriptorCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_RespondDescriptorRead_001
 * @tc.desc: gatt_server_application_RespondDescriptorRead_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RespondDescriptorRead_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RespondDescriptorRead_001 start");
    RawAddress addr("11:22:33:44:55:66");
    uint16_t handle = 1;
    std::vector<uint8_t> value;
    int ret = 1;
    gattServerApplication_->RespondDescriptorRead(addr, handle, value, ret);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->RespondDescriptorRead(addr, handle, value, ret);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RespondDescriptorRead_001 end");
}

/**
 * @tc.name: gatt_server_application_ReportWriteRequest_001
 * @tc.desc: gatt_server_application_ReportWriteRequest_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ReportWriteRequest_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ReportWriteRequest_001 start");
    GattDevice device;
    uint16_t handle = 1;
    std::vector<uint8_t> value;
    bool needRsp = true;
    GattServerApplication::GattElement type = GattServerApplication::GattElement::CHARACTERISTIC;
    gattServerApplication_->ReportWriteRequest(device, handle, value, needRsp, type);
    type = GattServerApplication::GattElement::SERVICE;
    gattServerApplication_->ReportWriteRequest(device, handle, value, needRsp, type);
    type = GattServerApplication::GattElement::DESCRIPTOR;
    gattServerApplication_->ReportWriteRequest(device, handle, value, needRsp, type);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ReportWriteRequest_001 end");
}

/**
 * @tc.name: gatt_server_application_RequestWriteCallback_001
 * @tc.desc: gatt_server_application_RequestWriteCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RequestWriteCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ReportWriteRequest_001 start");
    RawAddress addr;
    GattServerApplication::RequestWriteCallbackContext callbackContext = {
    .connId = 123,
    .transId = 456,
    .addr = addr,
    .handle = 789,
    .offset = 0,
    .needRsp = true,
    .isPrep = true,
    .value = {0x01, 0x02, 0x03, 0x04}
    };
    GattServerApplication::GattElement type = GattServerApplication::GattElement::DESCRIPTOR;
    gattServerApplication_->RequestWriteCallback(callbackContext, type);
    callbackContext.isPrep = false;
    gattServerApplication_->RequestWriteCallback(callbackContext, type);
    callbackContext.connId = 1;
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->RequestWriteCallback(callbackContext, type);
    callbackContext.needRsp = false;
    gattServerApplication_->RequestWriteCallback(callbackContext, type);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RequestWriteCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_RequestWriteCharacteristicCallback_001
 * @tc.desc: gatt_server_application_RequestWriteCharacteristicCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RequestWriteCharacteristicCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RequestWriteCharacteristicCallback_001 start");
    int connId = 1;
    int transId = 2;
    STACK::RawAddress bda;
    int attrHandle = 0x1234;
    int offset = 0;
    bool needRsp = true;
    bool isPrep = false;
    std::vector<uint8_t> value = {0x01, 0x02, 0x03};
    gattServerApplication_->RequestWriteCharacteristicCallback(
        connId, transId, bda, attrHandle, offset, needRsp, isPrep, value);
    SetConnStateConnected(connId);
    sleep(1);
    gattServerApplication_->RequestWriteCharacteristicCallback(
        connId, transId, bda, attrHandle, offset, needRsp, isPrep, value);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->RequestWriteCharacteristicCallback(
        connId, transId, bda, attrHandle, offset, needRsp, isPrep, value);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RequestWriteCharacteristicCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_RespondCharacteristicWrite_001
 * @tc.desc: gatt_server_application_RespondCharacteristicWrite_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RespondCharacteristicWrite_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RespondCharacteristicWrite_001 start");
    RawAddress addr("11:22:33:44:55:66");
    uint16_t handle = 1;
    int ret = 1;
    gattServerApplication_->RespondCharacteristicWrite(addr, handle, ret);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->RespondCharacteristicWrite(addr, handle, ret);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RespondCharacteristicWrite_001 end");
}

/**
 * @tc.name: gatt_server_application_RequestWriteDescriptorCallback_001
 * @tc.desc: gatt_server_application_RequestWriteDescriptorCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RequestWriteDescriptorCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RequestWriteDescriptorCallback_001 start");
    int connId = 1;
    int transId = 2;
    STACK::RawAddress bda;
    int attrHandle = 3;
    int offset = 4;
    bool needRsp = true;
    bool isPrep = false;
    std::vector<uint8_t> value = {0x01, 0x02, 0x03};
    gattServerApplication_->RequestWriteDescriptorCallback(
        connId, transId, bda, attrHandle, offset, needRsp, isPrep, value);
    SetConnStateConnected(connId);
    sleep(1);
    gattServerApplication_->RequestWriteDescriptorCallback(
        connId, transId, bda, attrHandle, offset, needRsp, isPrep, value);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->RequestWriteDescriptorCallback(
        connId, transId, bda, attrHandle, offset, needRsp, isPrep, value);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RequestWriteDescriptorCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_RespondDescriptorWrite_001
 * @tc.desc: gatt_server_application_RespondDescriptorWrite_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RespondDescriptorWrite_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RespondDescriptorWrite_001 start");
    RawAddress addr("11:22:33:44:55:66");
    uint16_t handle = 1;
    int ret = 1;
    gattServerApplication_->RespondDescriptorWrite(addr, handle, ret);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->RespondDescriptorWrite(addr, handle, ret);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RespondDescriptorWrite_001 end");
}

/**
 * @tc.name: gatt_server_application_ProcessLongValueRead_001
 * @tc.desc: gatt_server_application_ProcessLongValueRead_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ProcessLongValueRead_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ProcessLongValueRead_001 start");
    RawAddress addr;
    GattServerApplication::RequestReadCallbackContext context = {
    .connId = 1,
    .transId = 456,
    .addr = addr,
    .handle = 789,
    .offset = -1,
    .isLong = true
    };
    gattServerApplication_->ProcessLongValueRead(context);
    context.offset = 0;
    gattServerApplication_->ProcessLongValueRead(context);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->ProcessLongValueRead(context);
    context.offset = 1;
    gattServerApplication_->ProcessLongValueRead(context);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ProcessLongValueRead_001 end");
}

/**
 * @tc.name: gatt_server_application_ProcessLongValueWrite_001
 * @tc.desc: gatt_server_application_ProcessLongValueWrite_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ProcessLongValueWrite_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ProcessLongValueWrite_001 start");
    RawAddress addr;
    GattServerApplication::RequestWriteCallbackContext callbackContext = {
    .connId = 1,
    .transId = 456,
    .addr = addr,
    .handle = 789,
    .offset = 0,
    .needRsp = true,
    .isPrep = true,
    .value = {0x01, 0x02, 0x03, 0x04}
    };
    GattServerApplication::GattElement type = GattServerApplication::GattElement::CHARACTERISTIC;
    gattServerApplication_->ProcessLongValueWrite(callbackContext, type);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->ProcessLongValueWrite(callbackContext, type);
    callbackContext.offset = -1;
    gattServerApplication_->ProcessLongValueWrite(callbackContext, type);
    callbackContext.offset = 1;
    gattServerApplication_->ProcessLongValueWrite(callbackContext, type);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ProcessLongValueWrite_001 end");
}

/**
 * @tc.name: gatt_server_application_RequestExecWriteCallback_001
 * @tc.desc: gatt_server_application_RequestExecWriteCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_RequestExecWriteCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_RequestExecWriteCallback_001 start");
    int connId = 1;
    int transId = 1;
    STACK::RawAddress bda;
    int execWrite = 0;
    gattServerApplication_->RequestExecWriteCallback(connId, transId, bda, execWrite);
    SetConnStateConnected(connId);
    sleep(1);
    gattServerApplication_->RequestExecWriteCallback(connId, transId, bda, execWrite);
    sleep(1);
    execWrite = 1;
    gattServerApplication_->RequestExecWriteCallback(connId, transId, bda, execWrite);
    sleep(1);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->RequestExecWriteCallback(connId, transId, bda, execWrite);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_RequestExecWriteCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_SendResponse_001
 * @tc.desc: gatt_server_application_SendResponse_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_SendResponse_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_SendResponse_001 start");
    GattServerApplication::SendResponseContext ctx = {
    .connId = 1,
    .transId = 456,
    .handle = 789,
    .ret = 1
    };
    std::vector<uint8_t> value;
    int offset = 0;
    gattServerApplication_->SendResponse(ctx, value, offset);
    sleep(1);
    value.push_back(1);
    gattServerApplication_->SendResponse(ctx, value, offset);
    sleep(1);
    ctx.ret = 0;
    gattServerApplication_->SendResponse(ctx, value, offset);
    sleep(1);
    EXPECT_CALL(gBtgattServerInterfaceMock, SendResponse(_, _, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    gattServerApplication_->SendResponse(ctx, value, offset);
    sleep(1);
    gattServerApplication_->SendResponse(ctx, value, offset);
    sleep(1);
    gattServerApplication_->btIfGattServer_ = nullptr;
    gattServerApplication_->SendResponse(ctx, value, offset);
    sleep(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_SendResponse_001 end");
}

/**
 * @tc.name: gatt_server_application_IndicationSentCallback_001
 * @tc.desc: gatt_server_application_IndicationSentCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_IndicationSentCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_IndicationSentCallback_001 start");
    int connId = 1;
    int status = -1;
    gattServerApplication_->IndicationSentCallback(connId, status);
    SetConnStateConnected(connId);
    sleep(1);
    gattServerApplication_->IndicationSentCallback(connId, status);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->IndicationSentCallback(connId, status);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_IndicationSentCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_NotifyClientInner_001
 * @tc.desc: gatt_server_application_NotifyClientInner_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_NotifyClientInner_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_NotifyClientInner_001 start");
    RawAddress addr("11:22:33:44:55:66");
    uint16_t handle = 1;
    std::vector<uint8_t> value;
    bool needConfirm = false;
    gattServerApplication_->NotifyClientInner(addr, handle, value, needConfirm);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->NotifyClientInner(addr, handle, value, needConfirm);
    handle = GATT_HANDLE_MAX;
    EXPECT_CALL(gBtgattServerInterfaceMock, SendIndication(_, _, _, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    EXPECT_TRUE(gattServerApplication_->NotifyClientInner(addr, handle, value, needConfirm));
    EXPECT_FALSE(gattServerApplication_->NotifyClientInner(addr, handle, value, needConfirm));
    gattServerApplication_->btIfGattServer_ = nullptr;
    EXPECT_TRUE(gattServerApplication_->NotifyClientInner(addr, handle, value, needConfirm));
    HILOGI("gatt_server_application_NotifyClientInner_001 end");
}

/**
 * @tc.name: gatt_server_application_NotifyClientNoCfm_001
 * @tc.desc: gatt_server_application_NotifyClientNoCfm_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_NotifyClientNoCfm_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_NotifyClientNoCfm_001 start");
    RawAddress addr("11:22:33:44:55:66");
    uint16_t handle = 1;
    std::vector<uint8_t> value;
    std::shared_ptr<utility::Semaphore> semaphore = nullptr;
    gattServerApplication_->NotifyClientNoCfm(addr, handle, value, semaphore);
    semaphore = std::make_shared<utility::Semaphore>();
    gattServerApplication_->NotifyClientNoCfm(addr, handle, value, semaphore);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->NotifyClientNoCfm(addr, handle, value, semaphore);
    EXPECT_CALL(gBtgattServerInterfaceMock, SendIndication(_, _, _, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    gattServerApplication_->NotifyClientNoCfm(addr, handle, value, semaphore);
    gattServerApplication_->NotifyClientNoCfm(addr, handle, value, semaphore);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_NotifyClientNoCfm_001 end");
}

/**
 * @tc.name: gatt_server_application_MtuChangedCallback_001
 * @tc.desc: gatt_server_application_MtuChangedCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_MtuChangedCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_MtuChangedCallback_001 start");
    int connId = 1;
    int mtu = 1;
    gattServerApplication_->MtuChangedCallback(connId, mtu);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->MtuChangedCallback(connId, mtu);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->MtuChangedCallback(connId, mtu);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_MtuChangedCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_ConnUpdatedCallback_001
 * @tc.desc: gatt_server_application_ConnUpdatedCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ConnUpdatedCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ConnUpdatedCallback_001 start");
    int connId = 1;
    uint16_t interval = 1;
    uint16_t latency = 1;
    uint16_t timeout = 1;
    uint8_t status = 1;
    gattServerApplication_->ConnUpdatedCallback(connId, interval, latency, timeout, status);
    SetConnStateConnected(1);
    sleep(1);
    gattServerApplication_->ConnUpdatedCallback(connId, interval, latency, timeout, status);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattServerApplication_->ConnUpdatedCallback(connId, interval, latency, timeout, status);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ConnUpdatedCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_Connect_001
 * @tc.desc: gatt_server_application_Connect_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_Connect_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_Connect_001 start");
    RawAddress addr("11:22:33:44:55:66");
    gattServerApplication_->Connect(addr, true);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_Connect_001 end");
}

/**
 * @tc.name: gatt_server_application_CancelConnection_001
 * @tc.desc: gatt_server_application_CancelConnection_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_CancelConnection_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_CancelConnection_001 start");
    RawAddress addr("11:22:33:44:55:66");
    gattServerApplication_->CancelConnection(addr);
    SetConnStateConnected(1);
    sleep(1);
    EXPECT_CALL(gBtgattServerInterfaceMock, Disconnect(_, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_FAIL));
    gattServerApplication_->CancelConnection(addr);
    gattServerApplication_->CancelConnection(addr);
    gattServerApplication_->btIfGattServer_ = nullptr;
    gattServerApplication_->CancelConnection(addr);
    HILOGI("gatt_server_application_CancelConnection_001 end");
}

/**
 * @tc.name: gatt_server_application_001
 * @tc.desc: gatt_server_application_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_001 start");
    std::unique_ptr<GattServerApplication> gattServerApplication = std::make_unique<GattServerApplication>(
        serverIf, nullptr, tokenId_, callback_, [](int appId, const Service &service) {
            HILOGI("TestFunction %d", appId);
        });
    gattServerApplication = nullptr;
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_001 end");
}

/**
 * @tc.name: gatt_server_application_ServiceDeletedCallback_002
 * @tc.desc: gatt_server_application_ServiceDeletedCallback_002
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ServiceDeletedCallback_002, TestSize.Level1)
{
    HILOGI("gatt_server_application_ServiceDeletedCallback_002 start");
    int serviceHandle = 1;
    int status = GATT_SUCCESS;
    int serverIf = 0;
    {
        std::shared_ptr<GattServerApplication> gattServerApplication = gattServerApplication_;
        gattServerApplication->ServiceDeletedCallback(status, serverIf, serviceHandle);
        gattServerApplication = nullptr;
    }
    sleep(2);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ServiceDeletedCallback_001 end");
}

/**
 * @tc.name: gatt_server_application_PhyUpdatedCallback_001
 * @tc.desc: gatt_server_application_PhyUpdatedCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_PhyUpdatedCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_PhyUpdatedCallback_001 start");
    int connId = 1;
    int txPhy = 1;
    int rxPhy = 1;
    int status = 0;
    gattServerApplication_->PhyUpdatedCallback(connId, txPhy, rxPhy, status);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_PhyUpdatedCallback_001 end");
}
 
/**
 * @tc.name: gatt_server_application_SetPhy_001
 * @tc.desc: gatt_server_application_SetPhy_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_SetPhy_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_SetPhy_001 start");
    RawAddress addr("11:22:33:44:55:66");
    int txPhy = 1;
    int rxPhy = 1;
    int phyOptions = 0;
    gattServerApplication_->SetPhy(addr, txPhy, rxPhy, phyOptions);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_SetPhy_001 end");
}
 
/**
 * @tc.name: gatt_server_application_ReadPhyCallback_001
 * @tc.desc: gatt_server_application_ReadPhyCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ReadPhyCallback_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ReadPhyCallback_001 start");
    int txPhy = 1;
    int rxPhy = 1;
    int status = 0;
    gattServerApplication_->ReadPhyCallback(txPhy, rxPhy, status);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ReadPhyCallback_001 end");
}
 
/**
 * @tc.name: gatt_server_application_ReadPhy_001
 * @tc.desc: gatt_server_application_ReadPhy_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerApplicationTest, gatt_server_application_ReadPhy_001, TestSize.Level1)
{
    HILOGI("gatt_server_application_ReadPhy_001 start");
    RawAddress addr("11:22:33:44:55:66");
    gattServerApplication_->ReadPhy(addr);
    EXPECT_TRUE(gattServerApplication_ != nullptr);
    HILOGI("gatt_server_application_ReadPhy_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS