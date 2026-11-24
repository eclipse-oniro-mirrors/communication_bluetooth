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
#include "class_creator.h"
#include "ble_adapter.h"
#include <future>
#include <memory>

#include "btgatt_client_interface_mock.h"
#include "gatt_client_service.h"
#include "adapter_manager.h"
#include "adapter_config.h"
#include "bluetooth_gatt_interface.h"
#include "btgatt_interface_inner.h"
#include "gatt_client_callback_mock.h"
#include "profile_service_manager.h"
#include "bluetooth_host.h"
#include "gatt_data.h"
#include "bluetooth_log.h"
#include "thread_util.h"
#include "bluetooth_errorcode.h"
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
const RawAddress DUMMY_ADDRESS("11:22:33:44:55:66");
static Bluetooth::BluetoothHost *host_;
constexpr int TIME = 2;
constexpr int SLEEP_TIME = 500;
GattClientService* gattClientService_ = nullptr;
int appId_ = -1;
std::shared_ptr<GattClientCallbackMock> callback_ = nullptr;
OHOS::bluetooth::BtgattClientInterfaceMock gBtgattClientInterfaceMock;

class GattClientServiceTest : public testing::Test {
public:
    GattClientServiceTest() = default;
    ~GattClientServiceTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();

private:
};

static void *BleAdapterCreate(void)
{
    return new BleAdapter();
}

void GattClientServiceTest::SetUpTestSuite(void)
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
    // 测试进程内的 AdapterManager 与设备 SA 不同进程，需显式创建 adapter 并置为开启
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    AdapterManager::GetInstance()->UpdateBluetoothState(
        bluetooth::BTTransport::ADAPTER_BLE, bluetooth::BTStateID::STATE_TURN_ON);
    BluetoothGattInterface::GetInstance()->Initialize(&g_btgattInterface);
    auto manager = ProfileServiceManager::GetInstance();
    std::string profileName = "GattClientService";
    manager->SetProfileService(ADAPTER_BLE, profileName);
    gattClientService_ = static_cast<GattClientService*>(manager->GetProfileService(profileName));
    RegisterGattClientInterfaceMock(
        [](const btgatt_client_interface_t *interface) {
           gattClientService_->SetBtifInterface(interface);
        },
        &gBtgattClientInterfaceMock);

    gattClientService_->Enable();
    // Wait for started
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_NE(g_btgattCallbacks, nullptr);

    // Register application, appId is 1.
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Invoke([](const STACK::bluetooth::Uuid& uuid, bool eattSupport) -> bt_status_t {
            int serverIf = 1;
            auto future = std::async(
                std::launch::async,
                [serverIf, uuid]() { g_btgattCallbacks->client->register_client_cb(0, serverIf, uuid); });
            return BT_STATUS_SUCCESS;
        }));
    callback_ = std::make_shared<GattClientCallbackMock>();
    appId_ = gattClientService_->RegisterApplication(callback_, DUMMY_ADDRESS, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    HILOGI("SetUpTestCase end");
}

void GattClientServiceTest::TearDownTestSuite(void)
{
    EXPECT_CALL(gBtgattClientInterfaceMock, UnregisterClient(_))
        .WillRepeatedly(Return(BT_STATUS_SUCCESS));
    UnRegisterGattClientInterfaceMock();
    gattClientService_ = nullptr;
    callback_ = nullptr;
    sleep(TIME);
    host_ = nullptr;
}

void GattClientServiceTest::SetUp()
{}

void GattClientServiceTest::TearDown()
{}

static constexpr int WAIT_TIMEOUT = 1100; // 1.1s
/**
 * @tc.name: gatt_client_timeout_001
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_001, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_001 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIMEOUT));
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    RawAddress addr("22:33:44:55:66:77");
    int ret = gattClientService_->RegisterApplication(callback_, addr, 0);
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    // 等待回调上报，避免对象生命周期问题
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    HILOGI("gatt_client_timeout_001 end");
}
/**
 * @tc.name: gatt_client_timeout_002
 * @tc.desc: Verify RegisterApplication no response.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_002, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_002 start");
    RawAddress addr("22:33:44:55:66:77");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->RegisterApplication(callback_, addr, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_timeout_002 end");
}
/**
 * @tc.name: gatt_client_timeout_003
 * @tc.desc: Verify RegisterSharedApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_003, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_003 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIMEOUT));
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    int ret = gattClientService_->RegisterSharedApplication(callback_, DUMMY_ADDRESS, 0);
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    // 等待回调上报，避免对象生命周期问题
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    HILOGI("gatt_client_timeout_003 end");
}
/**
 * @tc.name: gatt_client_timeout_004
 * @tc.desc: Verify RegisterSharedApplication no response.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_004, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_004 start");
    // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->RegisterSharedApplication(callback_, DUMMY_ADDRESS, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_timeout_004 end");
}
/**
 * @tc.name: gatt_client_timeout_005
 * @tc.desc: Verify DeregisterApplication no response.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_005, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_005 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->DeregisterApplication(appId_);
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_timeout_005 end");
}
/**
 * @tc.name: gatt_client_timeout_006
 * @tc.desc: Verify GetService no response.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_006, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_006 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    auto ret = gattClientService_->GetServices(appId_);
    EXPECT_TRUE(ret.empty());
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_timeout_006 end");
}
/**
 * @tc.name: gatt_client_timeout_007
 * @tc.desc: Verify GetAllDevice no response.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_007, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_007 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    auto ret = gattClientService_->GetAllDevice();
    EXPECT_TRUE(ret.empty());
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_timeout_007 end");
}
/**
 * @tc.name: gatt_client_timeout_008
 * @tc.desc: Verify WriteCharacteristicNoRsp error.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_008, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_008 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, WriteCharacteristic(_, _, _, _, _))
        .WillOnce(Return(BT_STATUS_FAIL));
    EXPECT_CALL(*callback_, OnCharacteristicWrite(GattStatus::GATT_FAILURE, _, _));
    Characteristic character;
    int ret = gattClientService_->WriteCharacteristic(appId_, character, true);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_client_timeout_008 end");
}
/**
 * @tc.name: gatt_client_timeout_009
 * @tc.desc: Verify WriteCharacteristicNoRsp no response.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_009, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_009 start");
    Characteristic character;
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->WriteCharacteristic(appId_, character, true);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_timeout_009 end");
}
/**
 * @tc.name: gatt_client_timeout_010
 * @tc.desc: Verify WriteCharacteristicNoRsp timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_timeout_010, TestSize.Level1)
{
    HILOGI("gatt_client_timeout_010 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, WriteCharacteristic(_, _, _, _, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    EXPECT_CALL(*callback_, OnCharacteristicWrite(_, _, _))
        .Times(0);
    Characteristic character;
    int ret = gattClientService_->WriteCharacteristic(appId_, character, true);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_client_timeout_010 end");
}

/**
 * @tc.name: gatt_client_connect_001
 * @tc.desc: gatt_client_connect_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_connect_001, TestSize.Level1)
{
    HILOGI("gatt_client_connect_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->Connect(appId_, true);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_connect_001 end");
}

/**
 * @tc.name: gatt_client_RegisterSharedApplication_001
 * @tc.desc: gatt_client_RegisterSharedApplication_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterSharedApplication_001, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterSharedApplication_001 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Invoke([](const STACK::bluetooth::Uuid& uuid, bool eattSupport) -> bt_status_t {
            int serverIf = 1;
            auto future = std::async(
                std::launch::async,
                [serverIf, uuid]() { g_btgattCallbacks->client->register_client_cb(0, serverIf, uuid); });
            return BT_STATUS_SUCCESS;
        }));
    callback_ = std::make_shared<GattClientCallbackMock>();
    gattClientService_->RegisterSharedApplication(callback_, DUMMY_ADDRESS, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    HILOGI("gatt_client_RegisterSharedApplication_001 end");
}

/**
 * @tc.name: gatt_client_connect_002
 * @tc.desc: gatt_client_connect_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_connect_002, TestSize.Level1)
{
    HILOGI("gatt_client_connect_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->Connect(-1, true);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_connect_002 end");
}

/**
 * @tc.name: gatt_client_disconnect_001
 * @tc.desc: gatt_client_disconnect_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_disconnect_001, TestSize.Level1)
{
    HILOGI("gatt_client_disconnect_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->Disconnect(appId_);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_disconnect_001 end");
}

/**
 * @tc.name: gatt_client_disconnect_002
 * @tc.desc: gatt_client_disconnect_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_disconnect_002, TestSize.Level1)
{
    HILOGI("gatt_client_disconnect_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->Disconnect(-1);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_disconnect_002 end");
}

/**
 * @tc.name: gatt_client_DiscoveryServices_001
 * @tc.desc: gatt_client_DiscoveryServices_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_DiscoveryServices_001, TestSize.Level1)
{
    HILOGI("gatt_client_DiscoveryServices_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->DiscoveryServices(appId_);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_DiscoveryServices_001 end");
}

/**
 * @tc.name: gatt_client_DiscoveryServices_002
 * @tc.desc: gatt_client_DiscoveryServices_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_DiscoveryServices_002, TestSize.Level1)
{
    HILOGI("gatt_client_DiscoveryServices_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->DiscoveryServices(-1);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_DiscoveryServices_002 end");
}

/**
 * @tc.name: gatt_client_ReadCharacteristic_001
 * @tc.desc: gatt_client_ReadCharacteristic_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_ReadCharacteristic_001, TestSize.Level1)
{
    HILOGI("gatt_client_ReadCharacteristic_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Characteristic character;
    int ret = gattClientService_->ReadCharacteristic(appId_, character);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_ReadCharacteristic_001 end");
}

/**
 * @tc.name: gatt_client_ReadCharacteristic_002
 * @tc.desc: gatt_client_ReadCharacteristic_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_ReadCharacteristic_002, TestSize.Level1)
{
    HILOGI("gatt_client_ReadCharacteristic_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Characteristic character;
    int ret = gattClientService_->ReadCharacteristic(-1, character);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_ReadCharacteristic_002 end");
}

/**
 * @tc.name: gatt_client_ReadCharacteristicByUuid_001
 * @tc.desc: gatt_client_ReadCharacteristicByUuid_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_ReadCharacteristicByUuid_001, TestSize.Level1)
{
    HILOGI("gatt_client_ReadCharacteristicByUuid_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Uuid uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    int ret = gattClientService_->ReadCharacteristicByUuid(appId_, uuid);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_ReadCharacteristicByUuid_001 end");
}

/**
 * @tc.name: gatt_client_ReadCharacteristicByUuid_002
 * @tc.desc: gatt_client_ReadCharacteristicByUuid_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_ReadCharacteristicByUuid_002, TestSize.Level1)
{
    HILOGI("gatt_client_ReadCharacteristicByUuid_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Uuid uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    int ret = gattClientService_->ReadCharacteristicByUuid(-1, uuid);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_ReadCharacteristicByUuid_002 end");
}

/**
 * @tc.name: gatt_client_ReadDescriptor_001
 * @tc.desc: gatt_client_ReadDescriptor_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_ReadDescriptor_001, TestSize.Level1)
{
    HILOGI("gatt_client_ReadDescriptor_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Descriptor descriptor;
    int ret = gattClientService_->ReadDescriptor(appId_, descriptor);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_ReadDescriptor_001 end");
}

/**
 * @tc.name: gatt_client_ReadDescriptor_002
 * @tc.desc: gatt_client_ReadDescriptor_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_ReadDescriptor_002, TestSize.Level1)
{
    HILOGI("gatt_client_ReadDescriptor_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Descriptor descriptor;
    int ret = gattClientService_->ReadDescriptor(-1, descriptor);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_ReadDescriptor_002 end");
}

/**
 * @tc.name: gatt_client_WriteDescriptor_001
 * @tc.desc: gatt_client_WriteDescriptor_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_WriteDescriptor_001, TestSize.Level1)
{
    HILOGI("gatt_client_WriteDescriptor_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Descriptor descriptor;
    int ret = gattClientService_->WriteDescriptor(appId_, descriptor);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_WriteDescriptor_001 end");
}

/**
 * @tc.name: gatt_client_WriteDescriptor_002
 * @tc.desc: gatt_client_WriteDescriptor_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_WriteDescriptor_002, TestSize.Level1)
{
    HILOGI("gatt_client_WriteDescriptor_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    Descriptor descriptor;
    int ret = gattClientService_->WriteDescriptor(-1, descriptor);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_WriteDescriptor_002 end");
}

/**
 * @tc.name: gatt_client_RequestExchangeMtu_001
 * @tc.desc: gatt_client_RequestExchangeMtu_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RequestExchangeMtu_001, TestSize.Level1)
{
    HILOGI("gatt_client_RequestExchangeMtu_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int mtu = -1;
    int ret = gattClientService_->RequestExchangeMtu(appId_, mtu);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_RequestExchangeMtu_001 end");
}

/**
 * @tc.name: gatt_client_RequestExchangeMtu_002
 * @tc.desc: gatt_client_RequestExchangeMtu_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RequestExchangeMtu_002, TestSize.Level1)
{
    HILOGI("gatt_client_RequestExchangeMtu_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int mtu = -1;
    int ret = gattClientService_->RequestExchangeMtu(-1, mtu);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_RequestExchangeMtu_002 end");
}

/**
 * @tc.name: gatt_client_GetAllDevice_001
 * @tc.desc: gatt_client_GetAllDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_GetAllDevice_001, TestSize.Level1)
{
    HILOGI("gatt_client_GetAllDevice_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int size = gattClientService_->GetAllDevice().size();
    EXPECT_EQ(size, 0);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_GetAllDevice_001 end");
}

/**
 * @tc.name: gatt_client_RequestConnectionPriority_001
 * @tc.desc: gatt_client_RequestConnectionPriority_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RequestConnectionPriority_001, TestSize.Level1)
{
    HILOGI("gatt_client_RequestConnectionPriority_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int connPriority = -1;
    int ret = gattClientService_->RequestConnectionPriority(appId_, connPriority);
    EXPECT_EQ(ret, Bluetooth::BtErrCode::BT_NO_ERROR);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_RequestConnectionPriority_001 end");
}

/**
 * @tc.name: gatt_client_RequestConnectionPriority_002
 * @tc.desc: gatt_client_RequestConnectionPriority_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RequestConnectionPriority_002, TestSize.Level1)
{
    HILOGI("gatt_client_RequestConnectionPriority_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int connPriority = -1;
    int ret = gattClientService_->RequestConnectionPriority(-1, connPriority);
    EXPECT_NE(ret, Bluetooth::BtErrCode::BT_NO_ERROR);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_RequestConnectionPriority_002 end");
}

/**
 * @tc.name: gatt_client_RequestFastestConn_001
 * @tc.desc: gatt_client_RequestFastestConn_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RequestFastestConn_001, TestSize.Level1)
{
    HILOGI("gatt_client_RequestFastestConn_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->RequestFastestConn(DUMMY_ADDRESS);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_RequestFastestConn_001 end");
}

/**
 * @tc.name: gatt_client_ReadRemoteRssiValue_001
 * @tc.desc: gatt_client_ReadRemoteRssiValue_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_ReadRemoteRssiValue_001, TestSize.Level1)
{
    HILOGI("gatt_client_ReadRemoteRssiValue_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->ReadRemoteRssiValue(appId_);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_ReadRemoteRssiValue_001 end");
}

/**
 * @tc.name: gatt_client_ReadRemoteRssiValue_002
 * @tc.desc: gatt_client_ReadRemoteRssiValue_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_ReadRemoteRssiValue_002, TestSize.Level1)
{
    HILOGI("gatt_client_ReadRemoteRssiValue_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->ReadRemoteRssiValue(-1);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_ReadRemoteRssiValue_002 end");
}

/**
 * @tc.name: gatt_client_RequestNotification_001
 * @tc.desc: gatt_client_RequestNotification_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RequestNotification_001, TestSize.Level1)
{
    HILOGI("gatt_client_RequestNotification_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->RequestNotification(appId_, -1, true);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_RequestNotification_001 end");
}

/**
 * @tc.name: gatt_client_RequestNotification_002
 * @tc.desc: gatt_client_RequestNotification_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RequestNotification_002, TestSize.Level1)
{
    HILOGI("gatt_client_RequestNotification_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->RequestNotification(-1, -1, true);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_RequestNotification_002 end");
}

/**
 * @tc.name: gatt_client_DeregisterApplication_001
 * @tc.desc: gatt_client_DeregisterApplication_001
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_DeregisterApplication_001, TestSize.Level1)
{
    HILOGI("gatt_client_DeregisterApplication_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    sleep(2);
    int ret = gattClientService_->DeregisterApplication(-1);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_DeregisterApplication_001 end");
}

/**
 * @tc.name: gatt_client_DeregisterApplication_002
 * @tc.desc: gatt_client_DeregisterApplication_002
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_DeregisterApplication_002, TestSize.Level1)
{
    HILOGI("gatt_client_DeregisterApplication_002 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    gattClientService_->Disable();
    sleep(2);
    int ret = gattClientService_->DeregisterApplication(-1);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_DeregisterApplication_002 end");
}

/**
 * @tc.name: gatt_client_GetServices_001
 * @tc.desc: Verify GetService no response.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_GetServices_001, TestSize.Level1)
{
    HILOGI("gatt_client_GetServices_001 start");
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    auto ret = gattClientService_->GetServices(-1);
    EXPECT_TRUE(ret.empty());
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_GetServices_001 end");
}

/**
 * @tc.name: gatt_client_WriteCharacteristic_001
 * @tc.desc: Verify WriteCharacteristicNoRsp no response.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_WriteCharacteristic_001, TestSize.Level1)
{
    HILOGI("gatt_client_WriteCharacteristic_001 start");
    Characteristic character;
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->WriteCharacteristic(-1, character, true);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_WriteCharacteristic_001 end");
}

/**
 * @tc.name: gatt_client_WriteCharacteristic_002
 * @tc.desc: Verify WriteCharacteristic
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_WriteCharacteristic_002, TestSize.Level1)
{
    HILOGI("gatt_client_WriteCharacteristic_002 start");
    Characteristic character;
        // Disable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::DISABLED);
    int ret = gattClientService_->WriteCharacteristic(appId_, character, false);
    EXPECT_EQ(ret, GattStatus::GATT_SUCCESS);
    // Enable gatt thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_GATT, ThreadUtil::ENABLED);
    HILOGI("gatt_client_WriteCharacteristic_002 end");
}

/**
 * @tc.name: gatt_client_RegisterApplication_001
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterApplication_001, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterApplication_001 start");
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    int ret = gattClientService_->RegisterApplication(callback_, DUMMY_ADDRESS, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(ret, GattStatus::GATT_FAILURE);
    HILOGI("gatt_client_RegisterApplication_001 end");
}

/**
 * @tc.name: gatt_client_RegisterApplication_002
 * @tc.desc: Verify RegisterApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterApplication_002, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterApplication_002 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    int ret = gattClientService_->RegisterApplication(callback_, DUMMY_ADDRESS, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_NE(ret, BT_STATUS_SUCCESS);
    HILOGI("gatt_client_RegisterApplication_002 end");
}

/**
 * @tc.name: gatt_client_RegisterApplication_003
 * @tc.desc: Verify RegisterApplication
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterApplication_003, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterApplication_003 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    RawAddress addr("22:33:44:55:66:77");
    int ret = gattClientService_->RegisterApplication(callback_, addr, 1);
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_client_RegisterApplication_003 end");
}

/**
 * @tc.name: gatt_client_RegisterApplication_004
 * @tc.desc: Verify RegisterApplication
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterApplication_004, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterApplication_004 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientService_->Disable();
    RawAddress addr("22:33:44:55:66:77");
    int ret = gattClientService_->RegisterApplication(callback_, addr, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_client_RegisterApplication_004 end");
}

/**
 * @tc.name: gatt_client_RegisterSharedApplication_002
 * @tc.desc: Verify RegisterSharedApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterSharedApplication_002, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterSharedApplication_002 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIMEOUT));
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    int ret = gattClientService_->RegisterSharedApplication(callback_, DUMMY_ADDRESS, 1);
    EXPECT_NE(ret, BT_STATUS_SUCCESS);
    // 等待回调上报，避免对象生命周期问题
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    HILOGI("gatt_client_RegisterSharedApplication_002 end");
}

/**
 * @tc.name: gatt_client_RegisterSharedApplication_003
 * @tc.desc: Verify RegisterSharedApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterSharedApplication_003, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterSharedApplication_003 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIMEOUT));
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    RawAddress addr("22:33:44:55:66:77");
    int ret = gattClientService_->RegisterSharedApplication(callback_, addr, 1);
    EXPECT_NE(ret, BT_STATUS_SUCCESS);
    // 等待回调上报，避免对象生命周期问题
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    HILOGI("gatt_client_RegisterSharedApplication_003 end");
}

/**
 * @tc.name: gatt_client_RegisterSharedApplication_004
 * @tc.desc: Verify RegisterSharedApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterSharedApplication_004, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterSharedApplication_004 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    RawAddress addr("22:33:44:55:66:77");
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    int ret = gattClientService_->RegisterSharedApplication(callback_, addr, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_NE(ret, BT_STATUS_SUCCESS);
    HILOGI("gatt_client_RegisterSharedApplication_004 end");
}

/**
 * @tc.name: gatt_client_RegisterSharedApplication_005
 * @tc.desc: Verify RegisterSharedApplication.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterSharedApplication_005, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterSharedApplication_004 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    gattClientService_->Disable();
    RawAddress addr("22:33:44:55:66:77");
    int ret = gattClientService_->RegisterSharedApplication(callback_, addr, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_NE(ret, GattStatus::GATT_SUCCESS);
    HILOGI("gatt_client_RegisterSharedApplication_005 end");
}

/**
 * @tc.name: gatt_client_RegisterSharedApplication_006
 * @tc.desc: Verify RegisterSharedApplication response timeout.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, gatt_client_RegisterSharedApplication_006, TestSize.Level1)
{
    HILOGI("gatt_client_RegisterSharedApplication_006 start");
    EXPECT_CALL(gBtgattClientInterfaceMock, RegisterClient(_, _))
        .WillOnce(Return(BT_STATUS_SUCCESS));
    RawAddress addr("22:33:44:55:66:77");
    int clientIf = 2;
    auto uuid = STACK::bluetooth::Uuid::GetRandom();
    auto future = std::async(
        std::launch::async,
        [clientIf, uuid]() {
            g_btgattCallbacks->client->register_client_cb(0, clientIf, uuid);
        }
    );
    int clientIf1 = 3;
    auto uuid1 = STACK::bluetooth::Uuid::GetRandom();
    auto future1 = std::async(
        std::launch::async,
        [clientIf1, uuid1]() {
            g_btgattCallbacks->client->register_client_cb(0, clientIf1, uuid1);
        }
    );
    int ret = gattClientService_->RegisterSharedApplication(callback_, addr, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_NE(ret, GATT_FAILURE);
    HILOGI("gatt_client_RegisterSharedApplication_006 end");
}

/**
 * @tc.name: upstream_to_client_callback_010
 * @tc.desc: Verify the callback is nullptr, weak_ptr.lock() failed.
 * @tc.type: FUNC
 */
HWTEST_F(GattClientServiceTest, upstream_to_client_callback_010, TestSize.Level1)
{
    HILOGI("upstream_to_client_callback_010 start");
    gattClientService_->Enable();
    btgatt_notify_params_t data;
    btgatt_read_params_t pData;
    btgatt_db_element_t db;
    btgatt_rsp_params_t pRsp;
    g_btgattCallbacks->client->open_cb(0, 1, 1, STACK::RawAddress());
    g_btgattCallbacks->client->close_cb(0, 1, 1, STACK::RawAddress(), 0);
    g_btgattCallbacks->client->search_complete_cb(0, 1);
    g_btgattCallbacks->client->register_for_notification_cb(0, 1, 1, 1);
    g_btgattCallbacks->client->notify_cb(0, data);
    g_btgattCallbacks->client->read_characteristic_cb(0, 1, &pData);
    g_btgattCallbacks->client->write_characteristic_cb(0, 1, 1, pRsp);
    g_btgattCallbacks->client->read_descriptor_cb(0, 1, pData);
    g_btgattCallbacks->client->write_descriptor_cb(0, 1, 1);
    g_btgattCallbacks->client->execute_write_cb(0, 1);
    g_btgattCallbacks->client->configure_mtu_cb(0, 1, 1);
    g_btgattCallbacks->client->read_remote_rssi_cb(0, STACK::RawAddress(), 1, 1);
    g_btgattCallbacks->client->get_gatt_db_cb(0, &db, 1);
    g_btgattCallbacks->client->services_removed_cb(0, 1, 1);
    g_btgattCallbacks->client->services_added_cb(0, db, 1);
    g_btgattCallbacks->client->conn_updated_cb(0, 1, 1, 1, 1);
    EXPECT_NE(g_btgattCallbacks->client, nullptr);
    HILOGI("upstream_to_client_callback_010 end");
}

/*
 * @tc.number: gatt_server_service_GetConnectedState001
 * @tc.name: GetConnectedState
*/
HWTEST_F(GattClientServiceTest, gatt_client_service_GetConnectedState001, TestSize.Level1)
{
    HILOGI("gatt_client_service_GetConnectedState001 start");
    RawAddress deviceId("11:22:33:44:55:66");
    EXPECT_EQ(gattClientService_->GetConnectedState(deviceId), 3);
    HILOGI("gatt_client_service_GetConnectedState001 end");
}

/*
 * @tc.number: gatt_client_service_SetPhy001
 * @tc.name: gatt_client_service_SetPhy001
*/
HWTEST_F(GattClientServiceTest, gatt_client_service_SetPhy001, TestSize.Level1)
{
    HILOGI("gatt_client_service_SetPhy001 start");
    int txPhy = 1;
    int rxPhy = 1;
    int phyOptions = 0;
    int ret = gattClientService_->SetPhy(-1, txPhy, rxPhy, phyOptions);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_client_service_SetPhy001 end");
}
 
/*
 * @tc.number: gatt_client_service_ReadPhy001
 * @tc.name: gatt_client_service_ReadPhy001
*/
HWTEST_F(GattClientServiceTest, gatt_client_service_ReadPhy001, TestSize.Level1)
{
    HILOGI("gatt_client_service_ReadPhy001 start");
    int ret = gattClientService_->ReadPhy(-1);
    EXPECT_EQ(ret, GattStatus::REQUEST_NOT_SUPPORT);
    HILOGI("gatt_client_service_ReadPhy001 end");
}
}  // namespace bluetooth
}  // namespace OHOS