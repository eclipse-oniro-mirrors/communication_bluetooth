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

#include "btgatt_client_interface_mock.h"
#include "gatt_client_application.h"
#include "gatt_client_service.h"
#include "bluetooth_gatt_interface.h"
#include "btgatt_interface_inner.h"
#include "gatt_client_callback_mock.h"
#include "profile_service_manager.h"
#include "bluetooth_host.h"
#include "gatt_data.h"
#include "bluetooth_log.h"
#include "thread_util.h"
#include "common/bluetooth_hw_interface.h"
#include "adapter_config.h"
#include "igatt_api.h"

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

static uint8_t g_hwConnAttrSetTransportMock;
static STACK::RawAddress g_hwConnAttrSetAddrMock;
static HwConnAttr g_hwConnAttrSetAtrrMock;
static void HwConnAttrSetMock(uint8_t transport, const STACK::RawAddress &addr, const HwConnAttr *attr)
{
    g_hwConnAttrSetTransportMock = transport;
    g_hwConnAttrSetAddrMock = addr;
    (void)memcpy_s(&g_hwConnAttrSetAtrrMock, sizeof(HwConnAttr), attr, sizeof(HwConnAttr));
}

static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

static std::pair<uint32_t, uint32_t> HwGetBrTransTxRxRate()
{
    std::pair<uint32_t, uint32_t> p;
    return p;
}

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwConnAttrSet = HwConnAttrSetMock,
    .hwGetBrTransTxRxRate = HwGetBrTransTxRxRate,
};
} // extern "C"

namespace OHOS {
namespace bluetooth {
const RawAddress DUMMY_ADDRESS("11:22:33:44:55:66");
const RawAddress EMPTY_ADDRESS("");
static Bluetooth::BluetoothHost *host_;
constexpr int TIME = 2;

class GattClientApplicationTest : public testing::Test {
public:
    GattClientApplicationTest() = default;
    ~GattClientApplicationTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    void SetConnStateConnected();

    std::shared_ptr<GattClientApplication> gattClientApplication_ {nullptr};
    GattClientService *gattClientService_ {nullptr};
    OHOS::bluetooth::BtgattClientInterfaceMock gattClientInterfaceMock_;
    int clientIf = 1;
    GattDevice device;
    std::shared_ptr<GattClientCallbackMock> callback_ {nullptr};
private:
};

void GattClientApplicationTest::SetUpTestSuite(void)
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
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    AdapterConfig::GetInstance()->Load();
    HILOGI("SetUpTestCase end");
}

void GattClientApplicationTest::TearDownTestSuite(void)
{
    BluetoothGattInterface::GetInstance()->ClearGattClientInValidObserver();
    sleep(TIME);
    host_ = nullptr;
}

void GattClientApplicationTest::SetUp()
{
    callback_ = std::make_shared<GattClientCallbackMock>();

    // new GattClientService
    auto manager = ProfileServiceManager::GetInstance();
    std::string profileName = "GattClientService";
    manager->SetProfileService(ADAPTER_BLE, profileName);
    gattClientService_ = static_cast<GattClientService *>(manager->GetProfileService(profileName));

    RegisterGattClientInterfaceMock(
        [this](const btgatt_client_interface_t *interface) {
           gattClientApplication_ = std::make_shared<GattClientApplication>(clientIf, interface, device, callback_);
           BluetoothGattInterface::GetInstance()->AddGattClientObserver(gattClientApplication_);
        },
        &gattClientInterfaceMock_);
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::ENABLED);
    if (gattClientService_ == nullptr) {
        HILOGI("gattClientService_ is nullptr");
    }
    // Wait for started
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_NE(g_btgattCallbacks, nullptr);

    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::ENABLED);
}

void GattClientApplicationTest::TearDown()
{
    EXPECT_CALL(gattClientInterfaceMock_, UnregisterClient(_))
        .WillRepeatedly(Return(BT_STATUS_SUCCESS));
    UnRegisterGattClientInterfaceMock();

    // delete GattClientService
    delete gattClientService_;
    gattClientService_ = nullptr;

    callback_ = nullptr;
    gattClientApplication_ = nullptr;
}

void GattClientApplicationTest::SetConnStateConnected()
{
    int clientId = 1;
    int connId = 1;
    int status = GattStatus::GATT_SUCCESS;
    gattClientApplication_->ConnectCallback(connId, status, clientId, STACK::RawAddress());
}

static constexpr int WAIT_TIMEOUT = 1100; // 1.1s
/**
 * @tc.name: gatt_client_application_ConnectCallback_001
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ConnectCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ConnectCallback_001 start");
    int clientIf = 2;
    int connId = 1;
    int status = GattStatus::GATT_SUCCESS;
    gattClientApplication_->ConnectCallback(connId, status, clientIf, STACK::RawAddress());
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ConnectCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_ConnectCallback_002
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ConnectCallback_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_ConnectCallback_002 start");
    SetConnStateConnected();
    EXPECT_TRUE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ConnectCallback_002 end");
}

/**
 * @tc.name: gatt_client_application_ConnectCallback_003
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ConnectCallback_003, TestSize.Level1)
{
    HILOGI("gatt_client_application_ConnectCallback_003 start");
    int clientIf = 1;
    int connId = 1;
    int status = GattStatus::REQUEST_NOT_SUPPORT;
    gattClientApplication_->ConnectCallback(connId, status, clientIf, STACK::RawAddress());
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->ConnectCallback(connId, status, clientIf, STACK::RawAddress());
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ConnectCallback_003 end");
}

/**
 * @tc.name: gatt_client_application_Connect_001
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_Connect_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_Connect_001 start");
    gattClientApplication_->addr_ = DUMMY_ADDRESS;
    EXPECT_CALL(gattClientInterfaceMock_, Connect(_, _, _, _, _, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->Connect(false);
    gattClientApplication_->addr_ = EMPTY_ADDRESS;
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_Connect_001 end");
}

/**
 * @tc.name: gatt_client_application_Connect_002
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_Connect_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_Connect_002 start");
    gattClientApplication_->addr_ = DUMMY_ADDRESS;
    EXPECT_CALL(gattClientInterfaceMock_, Connect(_, _, _, _, _, _))
        .WillOnce(Return(BT_STATUS_FAIL));
    gattClientApplication_->Connect(false);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->Connect(false);
    gattClientApplication_->addr_ = EMPTY_ADDRESS;
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_Connect_002 end");
}

/**
 * @tc.name: gatt_client_application_Connect_003
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_Connect_003, TestSize.Level1)
{
    HILOGI("gatt_client_application_Connect_003 start");
    SetConnStateConnected();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    gattClientApplication_->Connect(false);
    EXPECT_TRUE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_Connect_003 end");
}

/**
 * @tc.name: gatt_client_application_DisconnectCallback_001
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_DisconnectCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_DisconnectCallback_001 start");
    int clientIf = 2;
    int connId = 1;
    int status = GattStatus::GATT_SUCCESS;
    int reason = 1;
    gattClientApplication_->DisconnectCallback(connId, status, clientIf, STACK::RawAddress(), reason);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_DisconnectCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_DisconnectCallback_002
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_DisconnectCallback_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_DisconnectCallback_002 start");
    int clientIf = 1;
    int connId = 1;
    int status = GattStatus::GATT_SUCCESS;
    int reason = 1;
    gattClientApplication_->DisconnectCallback(connId, status, clientIf, STACK::RawAddress(), reason);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_DisconnectCallback_002 end");
}

/**
 * @tc.name: gatt_client_application_DisconnectCallback_003
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_DisconnectCallback_003, TestSize.Level1)
{
    HILOGI("gatt_client_application_DisconnectCallback_003 start");
    int clientIf = 1;
    int connId = 1;
    int status = GattStatus::REQUEST_NOT_SUPPORT;
    int reason = 1;
    gattClientApplication_->DisconnectCallback(connId, status, clientIf, STACK::RawAddress(), reason);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->DisconnectCallback(connId, status, clientIf, STACK::RawAddress(), reason);
    EXPECT_TRUE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_DisconnectCallback_003 end");
}

/**
 * @tc.name: gatt_client_application_Disconnect_001
 * @tc.desc: gatt_client_application_Disconnect_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_Disconnect_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_Disconnect_001 start");
    gattClientApplication_->Disconnect();
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_Disconnect_001 end");
}

/**
 * @tc.name: gatt_client_application_Disconnect_002
 * @tc.desc:gatt_client_application_Disconnect_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_Disconnect_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_Disconnect_002 start");
    SetConnStateConnected();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_CALL(gattClientInterfaceMock_, Disconnect(_, _, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->Disconnect();
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_Disconnect_002 end");
}

/**
 * @tc.name: gatt_client_application_Disconnect_003
 * @tc.desc: gatt_client_application_Disconnect_003.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_Disconnect_003, TestSize.Level1)
{
    HILOGI("gatt_client_application_Disconnect_003 start");
    SetConnStateConnected();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_CALL(gattClientInterfaceMock_, Disconnect(_, _, _))
        .WillOnce(Return(BT_STATUS_FAIL));
    gattClientApplication_->Disconnect();
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->Disconnect();
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_Disconnect_003 end");
}

/**
 * @tc.name: gatt_client_application_Disconnect_004
 * @tc.desc: gatt_client_application_Disconnect_004.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_Disconnect_004, TestSize.Level1)
{
    HILOGI("gatt_client_application_Disconnect_004 start");
    gattClientApplication_->addr_ = DUMMY_ADDRESS;
    EXPECT_CALL(gattClientInterfaceMock_, Connect(_, _, _, _, _, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->Connect(false);
    EXPECT_CALL(gattClientInterfaceMock_, Disconnect(_, _, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->Disconnect();
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_Disconnect_004 end");
}

/**
 * @tc.name: gatt_client_application_SearchCompleteCallback_001
 * @tc.desc: gatt_client_application_SearchCompleteCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_SearchCompleteCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_SearchCompleteCallback_001 start");
    int connId = 1;
    int status = GattStatus::GATT_SUCCESS;
    gattClientApplication_->SearchCompleteCallback(connId, status);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_SearchCompleteCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_SearchCompleteCallback_002
 * @tc.desc: gatt_client_application_SearchCompleteCallback_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_SearchCompleteCallback_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_SearchCompleteCallback_002 start");
    int connId = GATT_INVALID_CONN_ID;
    int status = GattStatus::GATT_SUCCESS;
    gattClientApplication_->SearchCompleteCallback(connId, status);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_SearchCompleteCallback_002 end");
}

/**
 * @tc.name: gatt_client_application_SearchCompleteCallback_003
 * @tc.desc: gatt_client_application_SearchCompleteCallback_003.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_SearchCompleteCallback_003, TestSize.Level1)
{
    HILOGI("gatt_client_application_SearchCompleteCallback_003 start");
    int connId = GATT_INVALID_CONN_ID;
    int status = GattStatus::REQUEST_NOT_SUPPORT;
    gattClientApplication_->SearchCompleteCallback(connId, status);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->SearchCompleteCallback(connId, status);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_SearchCompleteCallback_003 end");
}

/**
 * @tc.name: gatt_client_application_GetGattDatabase_001
 * @tc.desc: gatt_client_application_GetGattDatabase_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetGattDatabase_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetGattDatabase_001 start");
    int connId = GATT_INVALID_CONN_ID;
    EXPECT_CALL(gattClientInterfaceMock_, GetGattDb(_))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->GetGattDatabase(connId);
    HILOGI("gatt_client_application_GetGattDatabase_001 end");
}

/**
 * @tc.name: gatt_client_application_GetGattDatabase_002
 * @tc.desc: gatt_client_application_GetGattDatabase_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetGattDatabase_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetGattDatabase_002 start");
    int connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->GetGattDatabase(connId);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->GetGattDatabase(connId);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_GetGattDatabase_002 end");
}

/**
 * @tc.name: gatt_client_application_DiscoveryServices_001
 * @tc.desc: gatt_client_application_DiscoveryServices_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_DiscoveryServices_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_DiscoveryServices_001 start");
    SetConnStateConnected();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_CALL(gattClientInterfaceMock_, SearchService(_, _))
        .WillOnce(Return(BT_STATUS_FAIL));
    gattClientApplication_->DiscoveryServices();
    HILOGI("gatt_client_application_DiscoveryServices_001 end");
}

/**
 * @tc.name: gatt_client_application_DiscoveryServices_002
 * @tc.desc: gatt_client_application_DiscoveryServices_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_DiscoveryServices_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_DiscoveryServices_002 start");
    gattClientApplication_->DiscoveryServices();
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_DiscoveryServices_002 end");
}

/**
 * @tc.name: gatt_client_application_DiscoveryServices_003
 * @tc.desc: gatt_client_application_DiscoveryServices_003.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_DiscoveryServices_003, TestSize.Level1)
{
    HILOGI("gatt_client_application_DiscoveryServices_003 start");
    SetConnStateConnected();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_CALL(gattClientInterfaceMock_, SearchService(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS))
        .WillOnce(Return(BT_STATUS_SUCCESS));;
    gattClientApplication_->DiscoveryServices();
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->DiscoveryServices();
    HILOGI("gatt_client_application_DiscoveryServices_003 end");
}

/**
 * @tc.name: gatt_client_application_GetGattDbCallback_001
 * @tc.desc: gatt_client_application_GetGattDbCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetGattDbCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetGattDbCallback_001 start");
    int connId = 1;
    btgatt_db_element_t db;
    int count = -1;
    gattClientApplication_->GetGattDbCallback(connId, &db, count);
    connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->GetGattDbCallback(connId, &db, count);
    count = 65536;
    gattClientApplication_->GetGattDbCallback(connId, &db, count);
    count = 1;
    gattClientApplication_->GetGattDbCallback(connId, &db, count);
    count = 0;
    gattClientApplication_->GetGattDbCallback(connId, &db, count);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->GetGattDbCallback(connId, &db, -1);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_GetGattDbCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_GetGattDbCallback_002
 * @tc.desc: gatt_client_application_GetGattDbCallback_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetGattDbCallback_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetGattDbCallback_002 start");
    int connId = GATT_INVALID_CONN_ID;
    btgatt_db_element_t db;
    db.type = BTGATT_DB_INCLUDED_SERVICE;
    gattClientApplication_->GetGattDbCallback(connId, &db, 1);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_GetGattDbCallback_002 end");
}

#ifdef BLUETOOTH_WATCH_ENABLE
/**
 * @tc.name: gatt_client_application_GetGattDbCallback_002
 * @tc.desc: gatt_client_application_GetGattDbCallback_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetGattDbCallback_003, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetGattDbCallback_003 start");
    int connId = GATT_INVALID_CONN_ID;
    btgatt_db_element_t db;
    db.type = BTGATT_DB_INCLUDED_SERVICE;
    db.attribute_handle = 0;
    db.uuid = ::bluetooth::Uuid::From128BitBE(
    Uuid::UUID128Bit{{0x69, 0xD1, 0xD8, 0xF3, 0x45, 0xE1, 0x49, 0xA8, 0x98, 0x21,
                0x9B, 0xBD, 0xFD, 0xAA, 0xD9, 0xD9}});
    gattClientApplication_->GetGattDbCallback(connId, &db, 1);

    db.uuid = ::bluetooth::Uuid::From128BitBE(
    Uuid::UUID128Bit{{0x9F, 0xBF, 0x12, 0x0D, 0x63, 0x01, 0x42, 0xD9, 0x8C, 0x58,
                0x25, 0xE6, 0x99, 0xA2, 0x1D, 0xBD}});
    gattClientApplication_->GetGattDbCallback(connId, &db, 1);

    db.uuid = ::bluetooth::Uuid::From128BitBE(
    Uuid::UUID128Bit{{0x22, 0xEA, 0xC6, 0xE9, 0x24, 0xD6, 0x4B, 0xB5, 0xBE, 0x44,
                0xB3, 0x6A, 0xCE, 0x7C, 0x7B, 0xFB}});
    gattClientApplication_->GetGattDbCallback(connId, &db, 1);
    EXPECT_TRUE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_GetGattDbCallback_003 end");
}
#endif
/**
 * @tc.name: gatt_client_application_ReadCharacteristicCallback_001
 * @tc.desc: gatt_client_application_ReadCharacteristicCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadCharacteristicCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadCharacteristicCallback_001 start");
    int connId = 1;
    btgatt_read_params_t pData;
    int status = GattStatus::REQUEST_NOT_SUPPORT;
    gattClientApplication_->ReadCharacteristicCallback(connId, status, &pData);
    connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->ReadCharacteristicCallback(connId, status, &pData);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->ReadCharacteristicCallback(connId, status, &pData);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ReadCharacteristicCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_ReadCharacteristic_001
 * @tc.desc: gatt_client_application_ReadCharacteristic_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadCharacteristic_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadCharacteristic_001 start");
    uint16_t handle = 1;
    EXPECT_CALL(gattClientInterfaceMock_, ReadCharacteristic(_, _, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->ReadCharacteristic(handle);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->ReadCharacteristic(handle);
    HILOGI("gatt_client_application_ReadCharacteristic_001 end");
}

/**
 * @tc.name: gatt_client_application_ReadCharacteristic_002
 * @tc.desc: gatt_client_application_ReadCharacteristic_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadCharacteristic_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadCharacteristic_002 start");
    uint16_t handle = 1;
    EXPECT_CALL(gattClientInterfaceMock_, ReadCharacteristic(_, _, _))
        .WillOnce(Return(BT_STATUS_FAIL));
    gattClientApplication_->ReadCharacteristic(handle);
    HILOGI("gatt_client_application_ReadCharacteristic_002 end");
}

/**
 * @tc.name: gatt_client_application_ReadCharacteristicByUuid_001
 * @tc.desc: gatt_client_application_ReadCharacteristicByUuid_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadCharacteristicByUuid_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadCharacteristicByUuid_001 start");
    Uuid uuid = Uuid::Random();
    EXPECT_CALL(gattClientInterfaceMock_, ReadUsingCharacteristicUuid(_, _, _, _, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->ReadCharacteristicByUuid(uuid);
    HILOGI("gatt_client_application_ReadCharacteristicByUuid_001 end");
}

/**
 * @tc.name: gatt_client_application_ReadCharacteristicByUuid_002
 * @tc.desc: gatt_client_application_ReadCharacteristicByUuid_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadCharacteristicByUuid_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadCharacteristicByUuid_002 start");
    Uuid uuid = Uuid::Random();
    EXPECT_CALL(gattClientInterfaceMock_, ReadUsingCharacteristicUuid(_, _, _, _, _))
        .WillOnce(Return(BT_STATUS_FAIL));
    gattClientApplication_->ReadCharacteristicByUuid(uuid);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->ReadCharacteristicByUuid(uuid);
    HILOGI("gatt_client_application_ReadCharacteristicByUuid_002 end");
}

/**
 * @tc.name: gatt_client_application_WriteCharacteristicCallback_001
 * @tc.desc: gatt_client_application_WriteCharacteristicCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_WriteCharacteristicCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_WriteCharacteristicCallback_001 start");
    int connId = 1;
    uint16_t handle = 1;
    int status = GattStatus::REQUEST_NOT_SUPPORT;
    btgatt_rsp_params_t rspContext;
    rspContext.timeStamp = 0;
    gattClientApplication_->WriteCharacteristicCallback(connId, status, handle, rspContext);
    connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->WriteCharacteristicCallback(connId, status, handle, rspContext);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->WriteCharacteristicCallback(connId, status, handle, rspContext);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_WriteCharacteristicCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_WriteCharacteristicNoRespond_001
 * @tc.desc: gatt_client_application_WriteCharacteristicNoRespond_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_WriteCharacteristicNoRespond_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_WriteCharacteristicNoRespond_001 start");
    EXPECT_CALL(gattClientInterfaceMock_, WriteCharacteristic(_, _, _, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_FAIL))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    std::vector<uint8_t> value;
    uint16_t handle = 1;
    std::shared_ptr<utility::Semaphore> semaphore = std::make_shared<utility::Semaphore>(0);
    gattClientApplication_->WriteCharacteristicNoRespond(handle, value, semaphore);
    gattClientApplication_->WriteCharacteristicNoRespond(handle, value, semaphore);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->WriteCharacteristicNoRespond(handle, value, semaphore);
    HILOGI("gatt_client_application_WriteCharacteristicNoRespond_001 end");
}

/**
 * @tc.name: gatt_client_application_ReadDescriptorCallback_001
 * @tc.desc: gatt_client_application_ReadDescriptorCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadDescriptorCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadDescriptorCallback_001 start");
    int connId = 1;
    btgatt_read_params_t data;
    int status = GattStatus::REQUEST_NOT_SUPPORT;
    gattClientApplication_->ReadDescriptorCallback(connId, status, data);
    connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->ReadDescriptorCallback(connId, status, data);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->ReadDescriptorCallback(connId, status, data);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ReadDescriptorCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_ReadDescriptor_001
 * @tc.desc: gatt_client_application_ReadDescriptor_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadDescriptor_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadDescriptor_001 start");
    EXPECT_CALL(gattClientInterfaceMock_, ReadDescriptor(_, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_FAIL))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    uint16_t handle = 1;
    gattClientApplication_->ReadDescriptor(handle);
    gattClientApplication_->ReadDescriptor(handle);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->ReadDescriptor(handle);
    HILOGI("gatt_client_application_ReadDescriptor_001 end");
}

/**
 * @tc.name: gatt_client_application_WriteDescriptorCallback_001
 * @tc.desc: gatt_client_application_WriteDescriptorCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_WriteDescriptorCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_WriteDescriptorCallback_001 start");
    int connId = 1;
    uint16_t handle = 1;
    int status = GattStatus::REQUEST_NOT_SUPPORT;
    gattClientApplication_->WriteDescriptorCallback(connId, status, handle);
    connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->WriteDescriptorCallback(connId, status, handle);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->WriteDescriptorCallback(connId, status, handle);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_WriteDescriptorCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_WriteDescriptor_001
 * @tc.desc: gatt_client_application_WriteDescriptor_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_WriteDescriptor_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_WriteDescriptor_001 start");
    EXPECT_CALL(gattClientInterfaceMock_, WriteDescriptor(_, _, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_FAIL))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    uint16_t handle = 1;
    std::vector<uint8_t> value;
    gattClientApplication_->WriteDescriptor(handle, value);
    gattClientApplication_->WriteDescriptor(handle, value);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->WriteDescriptor(handle, value);
    HILOGI("gatt_client_application_WriteDescriptor_001 end");
}

/**
 * @tc.name: gatt_client_application_RequestNotification_001
 * @tc.desc: gatt_client_application_RequestNotification_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_RequestNotification_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_RequestNotification_001 start");
    uint16_t characteristicHandle = 1;
    bool enable = false;
    gattClientApplication_->RequestNotification(characteristicHandle, enable);
    enable = true;
    gattClientApplication_->RequestNotification(characteristicHandle, enable);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->RequestNotification(characteristicHandle, enable);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_RequestNotification_001 end");
}

/**
 * @tc.name: gatt_client_application_ConfigureMtuCallback_001
 * @tc.desc: gatt_client_application_ConfigureMtuCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ConfigureMtuCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ConfigureMtuCallback_001 start");
    int connId = 1;
    int mtu = 1;
    int status = GattStatus::REQUEST_NOT_SUPPORT;
    gattClientApplication_->ConfigureMtuCallback(connId, status, mtu);
    connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->ConfigureMtuCallback(connId, status, mtu);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->ConfigureMtuCallback(connId, status, mtu);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ConfigureMtuCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_RequestExchangeMtu_001
 * @tc.desc: gatt_client_application_RequestExchangeMtu_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_RequestExchangeMtu_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_RequestExchangeMtu_001 start");
    EXPECT_CALL(gattClientInterfaceMock_, ConfigureMtu(_, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_FAIL))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    int mtu = 1;
    gattClientApplication_->RequestExchangeMtu(mtu);
    gattClientApplication_->RequestExchangeMtu(mtu);
    HILOGI("gatt_client_application_RequestExchangeMtu_001 end");
}

/**
 * @tc.name: gatt_client_application_RequestExchangeMtu_002
 * @tc.desc: gatt_client_application_RequestExchangeMtu_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_RequestExchangeMtu_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_RequestExchangeMtu_002 start");
    int mtu = 1;
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->RequestExchangeMtu(mtu);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_RequestExchangeMtu_002 end");
}

/**
 * @tc.name: gatt_client_application_NotifyCallback_001
 * @tc.desc: gatt_client_application_NotifyCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_NotifyCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_NotifyCallback_001 start");
    int connId = 1;
    btgatt_notify_params_t data;
    gattClientApplication_->NotifyCallback(connId, data);
    connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->NotifyCallback(connId, data);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->NotifyCallback(connId, data);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_NotifyCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_ConnUpdatedCallback_001
 * @tc.desc: gatt_client_application_ConnUpdatedCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ConnUpdatedCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ConnUpdatedCallback_001 start");
    int connId = 1;
    uint16_t interval = 1;
    uint16_t latency = 1;
    uint16_t timeout = 1;
    uint8_t status = 1;
    gattClientApplication_->ConnUpdatedCallback(connId, interval, latency, timeout, status);
    connId = GATT_INVALID_CONN_ID;
    gattClientApplication_->ConnUpdatedCallback(connId, interval, latency, timeout, status);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->ConnUpdatedCallback(connId, interval, latency, timeout, status);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ConnUpdatedCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_RequestConnectionPriority_001
 * @tc.desc: gatt_client_application_RequestConnectionPriority_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_RequestConnectionPriority_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_RequestConnectionPriority_001 start");
    int connPriority = 2;
    SetConnStateConnected();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_CALL(gattClientInterfaceMock_, ConnParameterUpdate(_, _, _, _, _, _, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_FAIL))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->RequestConnectionPriority(connPriority);
    gattClientApplication_->RequestConnectionPriority(connPriority);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->RequestConnectionPriority(connPriority);
    HILOGI("gatt_client_application_RequestConnectionPriority_001 end");
}

/**
 * @tc.name: gatt_client_application_RequestConnectionPriority_002
 * @tc.desc: gatt_client_application_RequestConnectionPriority_002.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_RequestConnectionPriority_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_RequestConnectionPriority_002 start");
    SetConnStateConnected();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    int connPriority = static_cast<int>(GattConnectionPriority::HIGH);
    gattClientApplication_->RequestConnectionPriority(connPriority);
    connPriority = static_cast<int>(GattConnectionPriority::BALANCED);
    gattClientApplication_->RequestConnectionPriority(connPriority);
    EXPECT_TRUE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_RequestConnectionPriority_002 end");
}

/**
 * @tc.name: gatt_client_application_RequestConnectionPriority_003
 * @tc.desc: gatt_client_application_RequestConnectionPriority_003.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_RequestConnectionPriority_003, TestSize.Level1)
{
    HILOGI("gatt_client_application_RequestConnectionPriority_003 start");
    int connPriority = 2;
    gattClientApplication_->RequestConnectionPriority(connPriority);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_RequestConnectionPriority_003 end");
}

/**
 * @tc.name: gatt_client_application_ReadRemoteRssiValue_001
 * @tc.desc: gatt_client_application_ReadRemoteRssiValue_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadRemoteRssiValue_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadRemoteRssiValue_001 start");
    int appId = 1;
    EXPECT_CALL(gattClientInterfaceMock_, ReadRemoteRssi(_, _))
        .Times(2)
        .WillOnce(Return(BT_STATUS_FAIL))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientApplication_->ReadRemoteRssiValue(appId);
    gattClientApplication_->ReadRemoteRssiValue(appId);
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->ReadRemoteRssiValue(appId);
    HILOGI("gatt_client_application_ReadRemoteRssiValue_001 end");
}

/**
 * @tc.name: gatt_client_application_ReadRemoteRssiValueCallback_001
 * @tc.desc: gatt_client_application_ReadRemoteRssiValueCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadRemoteRssiValueCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadRemoteRssiValueCallback_001 start");
    int clientIf = 1;
    int rssi = 1;
    int status = 1;
    gattClientApplication_->ReadRemoteRssiValueCallback(clientIf, STACK::RawAddress(), rssi, status);
    clientIf = 2;
    gattClientApplication_->ReadRemoteRssiValueCallback(clientIf, STACK::RawAddress(), rssi, status);
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ThreadState::DISABLED);
    gattClientApplication_->ReadRemoteRssiValueCallback(clientIf, STACK::RawAddress(), rssi, status);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ReadRemoteRssiValueCallback_001 end");
}

/**
 * @tc.name: gatt_client_application_ServicesChangedCallback_001
 * @tc.desc: gatt_client_application_ServicesChangedCallback_001.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_ServicesChangedCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ServicesChangedCallback_001 start");
    int connId = 1;
    gattClientApplication_->ServicesChangedCallback(connId);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ServicesChangedCallback_001 end");
}

/*
 * @tc.number: gatt_client_application_GattTransportFromStack_001
 * @tc.name: gatt_client_application_GattTransportFromStack_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_GattTransportFromStack_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GattTransportFromStack_001 start");
    GattServiceBase::GattTransportFromStack(3);
    GattServiceBase::GattTransportFromStack(0);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_GattTransportFromStack_001 start");
}

/*
 * @tc.number: gatt_client_application_GattTransportToStack_001
 * @tc.name: gatt_client_application_GattTransportToStack_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_GattTransportToStack_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GattTransportToStack_001 start");
    GattServiceBase::GattTransportToStack(3);
    GattServiceBase::GattTransportToStack(0);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_GattTransportToStack_001 start");
}

/**
 * @tc.number: gatt_client_application_GattStatusFromStack_001
 * @tc.name: gatt_client_application_GattStatusFromStack_001
 */
HWTEST_F(GattClientApplicationTest, gatt_client_application_GattStatusFromStack_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GattStatusFromStack_001 start");
    int ret = GattServiceBase::GattStatusFromStack(STACK::GATT_READ_NOT_PERMIT);
    EXPECT_EQ(ret, READ_NOT_PERMITTED);
    int ret1 = GattServiceBase::GattStatusFromStack(STACK::GATT_AUTH_FAIL);
    EXPECT_EQ(ret1, AUTHENTICATION_FAILED);
    int ret2 = GattServiceBase::GattStatusFromStack(STACK::GATT_MORE);
    EXPECT_EQ(ret2, GATT_MORE_REASON);
    int ret3 = GattServiceBase::GattStatusFromStack(STACK::GATT_OUT_OF_RANGE);
    EXPECT_EQ(ret3, OUT_OF_RANGE);
    HILOGI("gatt_client_application_GattStatusFromStack_001 end");
}

/*
 * @tc.number: gatt_client_application_KeepBleScan_001
 * @tc.name: gatt_client_application_KeepBleScan_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_KeepBleScan_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_KeepBleScan_001 start");
    STACK::RawAddress rawAddr;
    STACK::RawAddress::FromString(device.addr_.GetAddress(), rawAddr);
    gattClientApplication_->Disconnect();

    // pkg name: softbus_server, uid: 1024, match
    gattClientApplication_->KeepBleScan("softbus_server", 1024);
    EXPECT_EQ(g_hwConnAttrSetTransportMock, BT_TRANSPORT_LE);
    EXPECT_EQ(g_hwConnAttrSetAddrMock, rawAddr);
    EXPECT_EQ(g_hwConnAttrSetAtrrMock.type, HW_CONN_ATTR_TYPE_KEEP_BLE_SCAN_IN_CONN);
    EXPECT_EQ(g_hwConnAttrSetAtrrMock.keepBleScanInConn, true);

    // pkg name: softbus_server, uid: 201024, not root user but match
    gattClientApplication_->KeepBleScan("softbus_server", 201024);
    EXPECT_EQ(g_hwConnAttrSetTransportMock, BT_TRANSPORT_LE);
    EXPECT_EQ(g_hwConnAttrSetAddrMock, rawAddr);
    EXPECT_EQ(g_hwConnAttrSetAtrrMock.type, HW_CONN_ATTR_TYPE_KEEP_BLE_SCAN_IN_CONN);
    EXPECT_EQ(g_hwConnAttrSetAtrrMock.keepBleScanInConn, true);

    g_hwConnAttrSetTransportMock = 0;
    // pkg name: softbus_server, uid: 21024, uid not match
    gattClientApplication_->KeepBleScan("softbus_server", 21024);
    EXPECT_EQ(g_hwConnAttrSetTransportMock, 0);

    // pkg name: softbus_server2, uid: 1024, pkgName not match
    gattClientApplication_->KeepBleScan("softbus_server2", 1024);
    EXPECT_EQ(g_hwConnAttrSetTransportMock, 0);

    // pkg name: softbus_server2, uid: 1025, pkgName and uid not match
    gattClientApplication_->KeepBleScan("softbus_server2", 1024);
    EXPECT_EQ(g_hwConnAttrSetTransportMock, 0);

    HILOGI("gatt_client_application_KeepBleScan_001 start");
}

/*
 * @tc.number: gatt_client_application_GetBleMinConnectionInterval_001
 * @tc.name: gatt_client_application_GetBleMinConnectionInterval_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetBleMinConnectionInterval_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetBleMinConnectionInterval_001 start");
    int connPriority = static_cast<int>(GattConnectionPriority::HIGH);
    int result = gattClientApplication_->GetBleMinConnectionInterval(connPriority);
    EXPECT_EQ(result, 0x9);  // 0x9 is the value corresponding to GattConnectionPriority::HIGH

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_WEAK);
    result = gattClientApplication_->GetBleMinConnectionInterval(connPriority);
    EXPECT_EQ(result, 0x50);  // 0x50 is the value corresponding to GattConnectionPriority::LOW_POWER_WEAK

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_ENHANCE);
    result = gattClientApplication_->GetBleMinConnectionInterval(connPriority);
    EXPECT_EQ(result, 0xA0);  // 0xA0 is the value corresponding to GattConnectionPriority::LOW_POWER_ENHANCE

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_ULTRA);
    result = gattClientApplication_->GetBleMinConnectionInterval(connPriority);
    EXPECT_EQ(result, 0xF0);  // 0xF0 is the value corresponding to GattConnectionPriority::LOW_POWER_ULTRA

    HILOGI("gatt_client_application_GetBleMinConnectionInterval_001 start");
}

/*
 * @tc.number: gatt_client_application_GetBleMaxConnectionInterval_001
 * @tc.name: gatt_client_application_GetBleMaxConnectionInterval_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetBleMaxConnectionInterval_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetBleMaxConnectionInterval_001 start");
    int connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_WEAK);
    int result = gattClientApplication_->GetBleMaxConnectionInterval(connPriority);
    EXPECT_EQ(result, 0x64);  // 0x64 is the value corresponding to GattConnectionPriority::LOW_POWER_WEAK

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_ENHANCE);
    result = gattClientApplication_->GetBleMaxConnectionInterval(connPriority);
    EXPECT_EQ(result, 0xC8);  // 0xC8 is the value corresponding to GattConnectionPriority::LOW_POWER_ENHANCE

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_ULTRA);
    result = gattClientApplication_->GetBleMaxConnectionInterval(connPriority);
    EXPECT_EQ(result, 0x108);  // 0x108 is the value corresponding to GattConnectionPriority::LOW_POWER_ULTRA

    HILOGI("gatt_client_application_GetBleMaxConnectionInterval_001 start");
}

/*
 * @tc.number: gatt_client_application_GetBleConnectionLatency_001
 * @tc.name: gatt_client_application_GetBleConnectionLatency_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetBleConnectionLatency_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetBleConnectionLatency_001 start");
    int connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_WEAK);
    int result = gattClientApplication_->GetBleConnectionLatency(connPriority);
    EXPECT_EQ(result, 0x0);  // 0x64 is the value corresponding to GattConnectionPriority::LOW_POWER_WEAK

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_ENHANCE);
    result = gattClientApplication_->GetBleConnectionLatency(connPriority);
    EXPECT_EQ(result, 0x2);  // 0x2 is the value corresponding to GattConnectionPriority::LOW_POWER_ENHANCE

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_ULTRA);
    result = gattClientApplication_->GetBleConnectionLatency(connPriority);
    EXPECT_EQ(result, 0x2);  // 0x2 is the value corresponding to GattConnectionPriority::LOW_POWER_ULTRA

    HILOGI("gatt_client_application_GetBleConnectionLatency_001 start");
}

/*
 * @tc.number: gatt_client_application_GetBleConnectionSupervisionTimeout_001
 * @tc.name: gatt_client_application_GetBleConnectionSupervisionTimeout_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_GetBleConnectionSupervisionTimeout_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_GetBleConnectionSupervisionTimeout_001 start");
    int connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_WEAK);
    int result = gattClientApplication_->GetBleConnectionSupervisionTimeout(connPriority);
    EXPECT_EQ(result, 0x1F4);  // 0x1F4 is the value corresponding to GattConnectionPriority::LOW_POWER_WEAK

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_ENHANCE);
    result = gattClientApplication_->GetBleConnectionSupervisionTimeout(connPriority);
    EXPECT_EQ(result, 0x1F4);  // 0x1F4 is the value corresponding to GattConnectionPriority::LOW_POWER_ENHANCE

    connPriority = static_cast<int>(GattConnectionPriority::LOW_POWER_ULTRA);
    result = gattClientApplication_->GetBleConnectionSupervisionTimeout(connPriority);
    EXPECT_EQ(result, 0x1F4);  // 0x1F4 is the value corresponding to GattConnectionPriority::LOW_POWER_ULTRA

    HILOGI("gatt_client_application_GetBleConnectionSupervisionTimeout_001 start");
}

/*
 * @tc.number: gatt_client_application_CheckAuthentication_001
 * @tc.name: gatt_client_application_CheckAuthentication_001
 * @tc.desc: 测试当status为OHOS_GATT_INSUFFICIENT_AUTHENTICATION, authRetryState_不为AUTH_RETRY_STATE_MITM时, 返回 true
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_CheckAuthentication_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_CheckAuthentication_001 start");
    int connId = 1;
    int status = OHOS_GATT_INSUFFICIENT_AUTHENTICATION;
    uint16_t handle = 1;
    int gattType = GattType::GATT_TYPE_WRITE_CHARACTERISTIC;
    gattClientApplication_->authRetryState_ = AUTH_RETRY_STATE_IDLE;
    bool ret = gattClientApplication_->CheckAuthentication(connId, status, handle, gattType);
    EXPECT_TRUE(ret);
    HILOGI("gatt_client_application_CheckAuthentication_001 end");
}

/*
 * @tc.number: gatt_client_application_CheckAuthentication_002
 * @tc.name: gatt_client_application_CheckAuthentication_002
 * @tc.desc: 测试当status不为OHOS_GATT_INSUFFICIENT_AUTHENTICATION或OHOS_GATT_INSUFFICIENT_ENCRYPTION时, 返回false;
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_CheckAuthentication_002, TestSize.Level1)
{
    HILOGI("gatt_client_application_CheckAuthentication_002 start");
    int connId = 1;
    int status = 0x01;
    uint16_t handle = 1;
    int gattType = GattType::GATT_TYPE_WRITE_CHARACTERISTIC;
    gattClientApplication_->authRetryState_ = AUTH_RETRY_STATE_IDLE;
    bool ret = gattClientApplication_->CheckAuthentication(connId, status, handle, gattType);
    EXPECT_FALSE(ret);
    HILOGI("gatt_client_application_CheckAuthentication_002 end");
}

/*
 * @tc.number: gatt_client_application_PhyUpdatedCallback_001
 * @tc.name: gatt_client_application_PhyUpdatedCallback_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_PhyUpdatedCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_PhyUpdatedCallback_001 start");
    int connId = 1;
    int status = 0;
    gattClientApplication_->PhyUpdatedCallback(connId, 1, 1, status);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_PhyUpdatedCallback_001 end");
}
 
/*
 * @tc.number: gatt_client_application_SetPhy_001
 * @tc.name: gatt_client_application_SetPhy_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_SetPhy_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_SetPhy_001 start");
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->SetPhy(1, 1, 0);
    EXPECT_EQ(ConvertToBlePhyMask(1), 1);
    HILOGI("gatt_client_application_SetPhy_001 end");
}
 
/*
 * @tc.number: gatt_client_application_ReadPhyCallback_001
 * @tc.name: gatt_client_application_ReadPhyCallback_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadPhyCallback_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadPhyCallback_001 start");
    int status = 0;
    gattClientApplication_->ReadPhyCallback(1, 1, status);
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ReadPhyCallback_001 end");
}
 
/*
 * @tc.number: gatt_client_application_ReadPhy_001
 * @tc.name: gatt_client_application_ReadPhy_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_application_ReadPhy_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ReadPhy_001 start");
    gattClientApplication_->btIfGattClient_ = nullptr;
    gattClientApplication_->ReadPhy();
    EXPECT_FALSE(gattClientApplication_->IsConnected());
    HILOGI("gatt_client_application_ReadPhy_001 end");
}

/*
 * @tc.number: gatt_client_ConvertDisconnectReason_001
 * @tc.name: gatt_client_ConvertDisconnectReason_001
*/
HWTEST_F(GattClientApplicationTest, gatt_client_ConvertDisconnectReason_001, TestSize.Level1)
{
    HILOGI("gatt_client_application_ConvertDisconnectReason_001 start");
    int convertReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
    std::string reasonMessage = "";
    int reason = tGATT_DISCONN_REASON::GATT_CONN_TIMEOUT;
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    EXPECT_EQ(reasonMessage, GATT_DIS_MSG_0X08_CONN_TIMEOUT);
    reason = tGATT_DISCONN_REASON::GATT_CONN_TERMINATE_PEER_USER;
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    EXPECT_EQ(reasonMessage, GATT_DIS_MSG_0X13_PEER_USER);
    reason = tGATT_DISCONN_REASON::GATT_CONN_OK;
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    EXPECT_EQ(reasonMessage, GATT_DIS_MSG_0X16_LOCAL_HOST);
    reason = tGATT_DISCONN_REASON::GATT_CONN_TERMINATE_LOCAL_HOST;
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    EXPECT_EQ(reasonMessage, GATT_DIS_MSG_0X16_LOCAL_HOST);
    reason = tGATT_DISCONN_REASON::GATT_CONN_L2C_FAILURE;
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    EXPECT_EQ(reasonMessage, GATT_DIS_MSG_L2C_FAILURE);
    reason = tGATT_DISCONN_REASON::GATT_CONN_LMP_TIMEOUT;
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    EXPECT_EQ(reasonMessage, GATT_DIS_MSG_0X22_LMP_TIMEOUT);
    reason = tGATT_DISCONN_REASON::BTA_GATT_CONN_NONE;
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    EXPECT_EQ(reasonMessage, GATT_DIS_MSG_NO_CONN_TO_CANCEL);
    HILOGI("gatt_client_application_ConvertDisconnectReason_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS
