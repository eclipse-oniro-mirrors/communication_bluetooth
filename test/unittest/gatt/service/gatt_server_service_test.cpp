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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "btgatt_interface_inner.h"
#include "btgatt_server_interface_mock.h"
#include "gatt_server_service.h"
#include "bluetooth_gatt_interface.h"
#include "gatt_server_callback_mock.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"
#include "service_util.h"
#include "bluetooth_host.h"
#include "ipc_skeleton.h"

#include "gatt_data.h"
#include "thread_util.h"
#include "btif_gatt.h"

using namespace testing;
using namespace testing::ext;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

extern "C" {
const btgatt_callbacks_t* g_btgattCallbacks;
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

class GattServerServiceTest : public testing::Test {
public:
    GattServerServiceTest() = default;
    ~GattServerServiceTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();

    GattServerService *gattService_ {nullptr};
    int appId_ = -1;
    std::shared_ptr<GattServerCallbackMock> callback_ {nullptr};
    OHOS::bluetooth::BtgattServerInterfaceMock gBtgattServerInterfaceMock;
private:
};

void GattServerServiceTest::SetUpTestSuite(void)
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

void GattServerServiceTest::TearDownTestSuite(void)
{
    sleep(TIME);
    host_ = nullptr;
}

void GattServerServiceTest::SetUp()
{
    // new GattServerService
    auto manager = ProfileServiceManager::GetInstance();
    std::string profileName = "GattServerService";
    manager->SetProfileService(ADAPTER_BLE, profileName);
    gattService_ = static_cast<GattServerService *>(manager->GetProfileService(profileName));

    RegisterGattServerInterfaceMock(
        [this](const btgatt_server_interface_t *interface) {
            gattService_->SetBtifInterface(interface);
        },
        &gBtgattServerInterfaceMock);

    gattService_->Enable();
    // Wait for started
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_NE(g_btgattCallbacks, nullptr);

    // Register application, appId is 1.
    EXPECT_CALL(gBtgattServerInterfaceMock, RegisterServer(_, _))
        .WillOnce(Invoke([](const STACK::bluetooth::Uuid& uuid, bool eattSupport) -> bt_status_t {
            int serverIf = 1;
            auto future = std::async(
                std::launch::async,
                [serverIf, uuid]() { g_btgattCallbacks->server->register_server_cb(0, serverIf, uuid); });
            return BT_STATUS_SUCCESS;
        }));
    callback_ = std::make_shared<GattServerCallbackMock>();
    appId_ = gattService_->RegisterApplication(callback_);
    EXPECT_NE(gattService_, nullptr);
}

void GattServerServiceTest::TearDown()
{
    EXPECT_CALL(gBtgattServerInterfaceMock, UnregisterServer(_))
        .WillRepeatedly(Return(BT_STATUS_SUCCESS));
    UnRegisterGattServerInterfaceMock();

    // delete GattServerService
    delete gattService_;
    gattService_ = nullptr;
    appId_ = -1;
    callback_ = nullptr;
    uint8_t sleepTimeMs = 30;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimeMs));
}

/**
 * @tc.name: upstream_to_server_callback_001
 * @tc.desc: Verify the OnConnectionStateChanged callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_001, TestSize.Level1)
{
    g_btgattCallbacks->server->connection_cb(DUMMY_CONN_ID, appId_, 1, STACK::RawAddress(DUMMY_ADDRESS), 0);
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}
/**
 * @tc.name: upstream_to_server_callback_002
 * @tc.desc: Verify the OnAddService & OnServiceChanged callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_002, TestSize.Level1)
{
    EXPECT_CALL(*callback_, OnAddService(_, _))
        .Times(1);
    EXPECT_CALL(*callback_, OnServiceChanged(_))
        .Times(0);

    g_btgattCallbacks->server->service_added_cb(0, appId_, std::vector<btgatt_db_element_t>{});
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}
/**
 * @tc.name: upstream_to_server_callback_003
 * @tc.desc: Verify the OnServiceChanged callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_003, TestSize.Level1)
{
    EXPECT_CALL(*callback_, OnServiceChanged(_))
        .Times(0);

    g_btgattCallbacks->server->service_deleted_cb(0, appId_, DUMMY_HANDLE);
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}
/**
 * @tc.name: upstream_to_server_callback_004
 * @tc.desc: Verify the OnCharacteristicReadRequest callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_004, TestSize.Level1)
{
    g_btgattCallbacks->server->request_read_characteristic_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, false);
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}
/**
 * @tc.name: upstream_to_server_callback_005
 * @tc.desc: Verify the OnDescriptorReadRequest callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_005, TestSize.Level1)
{
    g_btgattCallbacks->server->request_read_descriptor_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, false);
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}
/**
 * @tc.name: upstream_to_server_callback_006
 * @tc.desc: Verify the OnCharacteristicWriteRequest callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_006, TestSize.Level1)
{
    g_btgattCallbacks->server->request_write_characteristic_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, true, false, std::vector<uint8_t>{});
    g_btgattCallbacks->server->request_write_characteristic_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, false, false, std::vector<uint8_t>{});
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}
/**
 * @tc.name: upstream_to_server_callback_007
 * @tc.desc: Verify the OnDescriptorWriteRequest callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_007, TestSize.Level1)
{
    g_btgattCallbacks->server->request_write_descriptor_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, true, false, std::vector<uint8_t>{});
    g_btgattCallbacks->server->request_write_descriptor_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, false, false, std::vector<uint8_t>{});
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}
/**
 * @tc.name: upstream_to_server_callback_008
 * @tc.desc: Verify the OnMtuChanged callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_008, TestSize.Level1)
{
    HILOGI("upstream_to_server_callback_008 enter");
    int mtu = 100;
    g_btgattCallbacks->server->mtu_changed_cb(DUMMY_CONN_ID, mtu);
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
    HILOGI("upstream_to_server_callback_008 end");
}
/**
 * @tc.name: upstream_to_server_callback_009
 * @tc.desc: Verify the OnConnectionParameterChanged callback.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_009, TestSize.Level1)
{
    int interval = 100;
    int latency = 100;
    int timeout = 100;
    g_btgattCallbacks->server->conn_updated_cb(DUMMY_CONN_ID, interval, latency, timeout, 0);
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}
/**
 * @tc.name: upstream_to_server_callback_010
 * @tc.desc: Verify the callback is nullptr, weak_ptr.lock() failed.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, upstream_to_server_callback_010, TestSize.Level1)
{
    int dummy = 100;
    callback_ = nullptr;  // Clear callback, check weak_ptr.

    g_btgattCallbacks->server->service_added_cb(0, appId_, std::vector<btgatt_db_element_t>{});
    g_btgattCallbacks->server->service_deleted_cb(0, appId_, 1);
    g_btgattCallbacks->server->service_stopped_cb(0, appId_, 1);
    g_btgattCallbacks->server->connection_cb(DUMMY_CONN_ID, appId_, 1, STACK::RawAddress(DUMMY_ADDRESS), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    g_btgattCallbacks->server->request_read_characteristic_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, false);
    g_btgattCallbacks->server->request_read_descriptor_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, false);
    g_btgattCallbacks->server->request_write_characteristic_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, true, false, std::vector<uint8_t>{});
    g_btgattCallbacks->server->request_write_descriptor_cb(DUMMY_CONN_ID, DUMMY_TRANS_ID,
        STACK::RawAddress(DUMMY_ADDRESS), DUMMY_HANDLE, 0, true, false, std::vector<uint8_t>{});
    g_btgattCallbacks->server->mtu_changed_cb(DUMMY_CONN_ID, dummy);
    g_btgattCallbacks->server->conn_updated_cb(DUMMY_CONN_ID, dummy, dummy, dummy, 0);
    g_btgattCallbacks->server->request_exec_write_cb(1, 1, STACK::RawAddress(DUMMY_ADDRESS), 0);
    g_btgattCallbacks->server->response_confirmation_cb(1, 1);
    g_btgattCallbacks->server->indication_sent_cb(1, 1);
    g_btgattCallbacks->server->congestion_cb(1, false);
    g_btgattCallbacks->server->phy_updated_cb(1, 1, 1, 1);
    // Wait for callback executed.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_NE(g_btgattCallbacks->server, nullptr);
}

/**
 * @tc.name: gatt_server_timeout_002
 * @tc.desc: Verify DeregisterApplication timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_timeout_002, TestSize.Level1)
{
    HILOGI("gatt_server_timeout_002 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattService_->DeregisterApplication(appId_);
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_timeout_002 end");
}
/**
 * @tc.name: gatt_server_timeout_003
 * @tc.desc: Verify DeregisterApplication success.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_timeout_003, TestSize.Level1)
{
    HILOGI("gatt_server_timeout_003 start");
    int ret = gattService_->DeregisterApplication(appId_);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_server_timeout_003 end");
}
/**
 * @tc.name: gatt_server_timeout_004
 * @tc.desc: Verify DeregisterApplication timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_timeout_004, TestSize.Level1)
{
    HILOGI("gatt_server_timeout_004 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    auto gattDevice = GattDevice();
    auto character = Characteristic();
    gattService_->NotifyClient(gattDevice, character, false);

    EXPECT_NE(gattService_, nullptr);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_timeout_004 end");
}

/**
 * @tc.name: gatt_server_RegisterApplication_001
 * @tc.desc: Verify RegisterApplication - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RegisterApplication_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_RegisterApplication_001 start");
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->RegisterApplication(callback_);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_RegisterApplication_001 end");
}

/**
 * @tc.name: gatt_server_RegisterApplication_002
 * @tc.desc: Verify RegisterApplication - btIfGattServer_->register_server != BT_STATUS_SUCCESS.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RegisterApplication_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_RegisterApplication_002 start");
    EXPECT_CALL(gBtgattServerInterfaceMock, RegisterServer(_, _))
        .WillOnce(Return(BT_STATUS_FAIL));
    int ret = gattService_->RegisterApplication(callback_);
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    HILOGI("gatt_server_service_RegisterApplication_002 end");
}

/**
 * @tc.name: gatt_server_RegisterApplication_003
 * @tc.desc: Verify RegisterApplication - btIfGattServer_->register_server != BT_STATUS_SUCCESS.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_RegisterApplication_003, TestSize.Level1)
{
    HILOGI("gatt_server_RegisterApplication_003 start");
    gattService_->btIfGattServer_ = nullptr;
    int ret = gattService_->RegisterApplication(callback_);
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    HILOGI("gatt_server_RegisterApplication_003 end");
}

/**
 * @tc.name: gatt_server_DeregisterApplication_001
 * @tc.desc: Verify DeregisterApplication - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_DeregisterApplication_001, TestSize.Level1)
{
    HILOGI("gatt_server_DeregisterApplication_001 start");
    int tempAppId = 1;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->DeregisterApplication(tempAppId);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_DeregisterApplication_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_DeregisterApplication_002
 * @tc.desc: Verify DeregisterApplication - !pimpl->IsValidApplication().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_DeregisterApplication_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_DeregisterApplication_002 start");
    int tempAppId = -1;
    int ret = gattService_->DeregisterApplication(tempAppId);
    EXPECT_EQ(ret, GattStatus::INVALID_PARAMETER);
    HILOGI("gatt_server_service_DeregisterApplication_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_DeregisterApplication_003
 * @tc.desc: Verify DeregisterApplication - !pimpl->IsValidApplication().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_DeregisterApplication_003, TestSize.Level1)
{
    HILOGI("gatt_server_service_DeregisterApplication_003 start");
    int ret = gattService_->DeregisterApplication(appId_);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_server_service_DeregisterApplication_003 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_DeregisterApplication_004
 * @tc.desc: Verify DeregisterApplication(appId, pid) - pid mismatch is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_DeregisterApplication_004, TestSize.Level1)
{
    HILOGI("gatt_server_service_DeregisterApplication_004 start");
    int32_t wrongPid = -1;
    int ret = gattService_->DeregisterApplication(appId_, wrongPid);
    EXPECT_EQ(ret, GattStatus::INVALID_PARAMETER);
    HILOGI("gatt_server_service_DeregisterApplication_004 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_DeregisterApplication_005
 * @tc.desc: Verify DeregisterApplication(appId, pid) - pid recorded at registration
 *           succeeds, same as the app death cleanup path.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_DeregisterApplication_005, TestSize.Level1)
{
    HILOGI("gatt_server_service_DeregisterApplication_005 start");
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int ret = gattService_->DeregisterApplication(appId_, callerPid);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_server_service_DeregisterApplication_005 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_AddService_001
 * @tc.desc: Verify AddServic - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_AddService_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_AddService_001 start");
    int tempAppId = 1;
    Service service;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->AddService(tempAppId, service);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_AddService_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_AddServic_002
 * @tc.desc: Verify AddServic - !pimpl->IsValidApplication().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_AddService_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_AddService_002 start");
    int tempAppId = -1;
    Service service;
    int ret = gattService_->AddService(tempAppId, service);
    EXPECT_EQ(ret, GattStatus::INVALID_PARAMETER);
    HILOGI("gatt_server_service_AddService_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_AddService_003
 * @tc.desc: gatt_server_service_AddService_003
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_AddService_003, TestSize.Level1)
{
    HILOGI("gatt_server_service_AddService_003 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Service service;
    int ret = gattService_->AddService(appId_, service);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_service_AddService_003 end ret:%d", ret);
}


/**
 * @tc.name: gatt_server_RemoveService_001
 * @tc.desc: Verify RemoveService - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RemoveService_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_RemoveService_001 start");
    int tempAppId = 1;
    Service service;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->RemoveService(tempAppId, service);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_RemoveService_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_RemoveService_002
 * @tc.desc: Verify RemoveService - !pimpl->IsValidApplication().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RemoveService_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_RemoveService_002 start");
    int tempAppId = -1;
    Service service;
    int ret = gattService_->RemoveService(tempAppId, service);
    EXPECT_EQ(ret, GattStatus::INVALID_PARAMETER);
    HILOGI("gatt_server_service_RemoveService_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_RemoveService_003
 * @tc.desc: gatt_server_RemoveService_003
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_RemoveService_003, TestSize.Level1)
{
    HILOGI("gatt_server_RemoveService_003 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Service service;
    int ret = gattService_->RemoveService(appId_, service);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_RemoveService_003 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_RemoveService_004
 * @tc.desc: Verify RemoveService - a wrong pid cannot strip the ownership of
 *           the application, so the rightful process can still remove service.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RemoveService_004, TestSize.Level1)
{
    HILOGI("gatt_server_service_RemoveService_004 start");
    // Another process (wrong pid) fails to deregister the application.
    int32_t wrongPid = -1;
    int ret = gattService_->DeregisterApplication(appId_, wrongPid);
    EXPECT_EQ(ret, GattStatus::INVALID_PARAMETER);

    // The rightful process can still remove its service.
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Service service;
    ret = gattService_->RemoveService(appId_, service);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_service_RemoveService_004 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_ClearServices_001
 * @tc.desc: Verify ClearServices - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_ClearServices_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_ClearServices_001 start");
    int tempAppId = 1;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->ClearServices(tempAppId);
    EXPECT_EQ(ret, 2900001);
    HILOGI("gatt_server_service_ClearServices_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_ClearServices_002
 * @tc.desc: Verify ClearServices - !pimpl->IsValidApplication().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_ClearServices_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_ClearServices_002 start");
    int tempAppId = -1;
    int ret = gattService_->ClearServices(tempAppId);
    EXPECT_EQ(ret, 2900099);
    HILOGI("gatt_server_service_ClearServices_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_ClearServices_003
 * @tc.desc: gatt_server_service_ClearServices_003
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_ClearServices_003, TestSize.Level1)
{
    HILOGI("gatt_server_service_ClearServices_003 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattService_->ClearServices(appId_);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_service_ClearServices_003 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_ClearServices_004
 * @tc.desc: Verify ClearServices(appId, pid) - pid mismatch is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_ClearServices_004, TestSize.Level1)
{
    HILOGI("gatt_server_service_ClearServices_004 start");
    int32_t wrongPid = -1;
    int ret = gattService_->ClearServices(appId_, wrongPid);
    EXPECT_EQ(ret, 2900099);
    HILOGI("gatt_server_service_ClearServices_004 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_ClearServices_005
 * @tc.desc: Verify ClearServices(appId, pid) - pid recorded at registration
 *           succeeds, same as the app death cleanup path.
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_ClearServices_005, TestSize.Level1)
{
    HILOGI("gatt_server_service_ClearServices_005 start");
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int ret = gattService_->ClearServices(appId_, callerPid);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_server_service_ClearServices_005 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_Connect_001
 * @tc.desc: Verify Connect - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_Connect_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_Connect_001 start");
    GattDevice gattDevice;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->Connect(0, gattDevice, true);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_Connect_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_Connect_002
 * @tc.desc: Verify Connect - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_Connect_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_Connect_002 start");
    GattDevice gattDevice;
    int ret = gattService_->Connect(-1, gattDevice, true);
    EXPECT_EQ(ret, GattStatus::INVALID_PARAMETER);
    HILOGI("gatt_server_service_Connect_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_CancelConnection_001
 * @tc.desc: Verify CancelConnection - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_CancelConnection_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_CancelConnection_001 start");
    GattDevice gattDevice;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->CancelConnection(0, gattDevice);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_CancelConnection_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_CancelConnection_002
 * @tc.desc: gatt_server_service_CancelConnection_002
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_CancelConnection_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_CancelConnection_002 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    GattDevice gattDevice;
    int ret = gattService_->CancelConnection(-1, gattDevice);
    EXPECT_EQ(ret, GattStatus::INVALID_PARAMETER);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_service_CancelConnection_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_CancelConnection_003
 * @tc.desc: gatt_server_service_CancelConnection_003
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_CancelConnection_003, TestSize.Level1)
{
    HILOGI("gatt_server_service_CancelConnection_003 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    GattDevice gattDevice;
    int ret = gattService_->CancelConnection(appId_, gattDevice);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_service_CancelConnection_003 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_RespondCharacteristicRead_001
 * @tc.desc: Verify RespondCharacteristicRead - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RespondCharacteristicRead_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_RespondCharacteristicRead_001 start");
    GattDevice gattDevice;
    Characteristic character;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->RespondCharacteristicRead(gattDevice, character, 1);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_RespondCharacteristicRead_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_RespondCharacteristicRead_002
 * @tc.desc: gatt_server_service_RespondCharacteristicRead_002
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RespondCharacteristicRead_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_RespondCharacteristicRead_002 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    GattDevice gattDevice;
    Characteristic character;
    int ret = gattService_->RespondCharacteristicRead(gattDevice, character, 1);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_service_RespondCharacteristicRead_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_RespondDescriptorRead_001
 * @tc.desc: Verify RespondDescriptorRead - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RespondDescriptorRead_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_RespondDescriptorRead_001 start");
    GattDevice gattDevice;
    Descriptor descriptor;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->RespondDescriptorRead(gattDevice, descriptor, 1);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_RespondDescriptorRead_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_RespondDescriptorRead_002
 * @tc.desc: gatt_server_service_RespondDescriptorRead_002
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RespondDescriptorRead_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_RespondDescriptorRead_002 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    GattDevice gattDevice;
    Descriptor descriptor;
    int ret = gattService_->RespondDescriptorRead(gattDevice, descriptor, 1);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_service_RespondDescriptorRead_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_RespondCharacteristicWrite_001
 * @tc.desc: Verify RespondCharacteristicWrite - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RespondCharacteristicWrite_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_RespondCharacteristicWrite_001 start");
    GattDevice gattDevice;
    Characteristic character;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->RespondCharacteristicWrite(gattDevice, character, 1);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_RespondCharacteristicWrite_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_RespondCharacteristicWrite_002
 * @tc.desc: gatt_server_service_RespondCharacteristicWrite_002
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_RespondCharacteristicWrite_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_RespondCharacteristicWrite_002 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    GattDevice gattDevice;
    Characteristic character;
    int ret = gattService_->RespondCharacteristicWrite(gattDevice, character, 1);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_service_RespondCharacteristicWrite_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_RespondDescriptorWrite_001
 * @tc.desc: Verify RespondDescriptorWrite - !pimpl->InRunningState().
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_RespondDescriptorWrite_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_RespondDescriptorWrite_001 start");
    GattDevice gattDevice;
    Descriptor descriptor;
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->RespondDescriptorWrite(gattDevice, descriptor, 1);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_RespondDescriptorWrite_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_RespondDescriptorWrite_002
 * @tc.desc: gatt_server_RespondDescriptorWrite_002
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_RespondDescriptorWrite_002, TestSize.Level1)
{
    HILOGI("gatt_server_RespondDescriptorWrite_002 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    GattDevice gattDevice;
    Descriptor descriptor;
    int ret = gattService_->RespondDescriptorWrite(gattDevice, descriptor, 1);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_server_RespondDescriptorWrite_002 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_NotifyClient_001
 * @tc.desc: gatt_server_service_NotifyClient_001
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_NotifyClient_001, TestSize.Level1)
{
    HILOGI("gatt_server_service_NotifyClient_001 start");
    auto gattDevice = GattDevice();
    auto character = Characteristic();
    gattService_->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->NotifyClient(gattDevice, character, false);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_server_service_NotifyClient_001 end ret:%d", ret);
}

/**
 * @tc.name: gatt_server_service_NotifyClient_002
 * @tc.desc: gatt_server_service_NotifyClient_002
 * @tc.type: FUNC
 */
HWTEST_F(GattServerServiceTest, gatt_server_service_NotifyClient_002, TestSize.Level1)
{
    HILOGI("gatt_server_service_NotifyClient_002 start");
    auto gattDevice = GattDevice();
    auto character = Characteristic();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int ret = gattService_->NotifyClient(gattDevice, character, true);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_server_service_NotifyClient_002 end ret:%d", ret);
}

/*
 * @tc.number: gatt_server_service_GetConnectedState001
 * @tc.name: GetConnectedState
*/
HWTEST_F(GattServerServiceTest, gatt_server_service_GetConnectedState001, TestSize.Level1)
{
    HILOGI("gatt_server_service_GetConnectedState001 start");
    RawAddress deviceId("11:22:33:44:55:66");
    EXPECT_EQ(gattService_->GetConnectedState(deviceId), 3);
    HILOGI("gatt_server_service_GetConnectedState001 end");
}
}  // namespace bluetooth
}  // namespace OHOS