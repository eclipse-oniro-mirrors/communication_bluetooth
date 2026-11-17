/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "ble_scanner_state_machine.h"
#include "bluetooth_application_container.h"
#include "bluetooth_ble_advertiser_application_container.h"
#include "bluetooth_ble_advertiser_server.h"
#include "bluetooth_ble_central_manager_server.h"
#include "bluetooth_ble_scanner_application_container.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "adapter_config.h"
#include "adapter_manager.h"
#include "ble_adapter.h"
#include "class_creator.h"
#include "ble_service_data.h"
#include "bt_def.h"
#include "btcommon/timer_manager.h"

using namespace OHOS;
using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
constexpr int TIME = 2;
class BluetoothApplicationsContainerTest : public testing::Test {
public:
    BluetoothApplicationsContainerTest() = default;
    ~BluetoothApplicationsContainerTest() override = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<BluetoothApplicationContainer<BluetoothApplication>> defaultContainer_ {nullptr};
    std::shared_ptr<BluetoothBleAdvertiserApplicationContainer> bleAdvertiserApplicationContainer_ {nullptr};
    std::shared_ptr<BluetoothBleScannerApplicationContainer> bleScannerApplicationContainer_ {nullptr};
    std::shared_ptr<BleScannerStateMachine> dummySM_ {nullptr};
};

void BluetoothApplicationsContainerTest::SetUpTestCase(void)
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

void BluetoothApplicationsContainerTest::TearDownTestCase(void)
{
    host_ = nullptr;
    utility::CloseAllTimer();
}

void BluetoothApplicationsContainerTest::SetUp()
{
    dummySM_ = std::make_shared<BleScannerStateMachine>();
    auto resetBatchScanCb = [this](int32_t pid, int32_t uid, int32_t scannerId) -> bool {
        return true;
    };
    defaultContainer_ = std::make_shared<BluetoothApplicationContainer<BluetoothApplication>>("defaultContainer");
    defaultContainer_->Init();
    bleAdvertiserApplicationContainer_ = std::make_shared<BluetoothBleAdvertiserApplicationContainer>();
    bleAdvertiserApplicationContainer_->Init();
    bleScannerApplicationContainer_ = std::make_shared<BluetoothBleScannerApplicationContainer>(dummySM_,
        resetBatchScanCb);
    bleScannerApplicationContainer_->Init();
}

void BluetoothApplicationsContainerTest::TearDown()
{
    dummySM_ = nullptr;
    defaultContainer_ = nullptr;
    bleAdvertiserApplicationContainer_ = nullptr;
    bleScannerApplicationContainer_ = nullptr;
}

static void *BleAdapterCreate(void)
{
    return new BleAdapter();
}

static void *BleAdapterCreateNull(void)
{
    return nullptr;
}

/**
 * @tc.name: bluetooth_applications_container_001
 * @tc.desc: Verify the BluetoothApplicationContainer Add/Remove/Get function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_applications_container_001, TestSize.Level1)
{
    HILOGI("bluetooth_applications_container_001 enter");
    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;

    defaultContainer_->AddRemoteObject(pid, uid, remote);
    EXPECT_TRUE(defaultContainer_->Contain(remote));
    EXPECT_EQ(defaultContainer_->GetRemotePid(remote), pid);
    EXPECT_EQ(defaultContainer_->GetRemoteUid(remote), uid);

    defaultContainer_->RemoveRemoteObject(remote);
    EXPECT_FALSE(defaultContainer_->Contain(remote));
    EXPECT_EQ(defaultContainer_->GetRemotePid(remote), 0);
    EXPECT_EQ(defaultContainer_->GetRemoteUid(remote), 0);

    HILOGI("bluetooth_applications_container_001 end");
}

/**
 * @tc.name: bluetooth_applications_container_002
 * @tc.desc: Verify the BluetoothApplicationContainer same pid/uid, different remote
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_applications_container_002, TestSize.Level1)
{
    HILOGI("bluetooth_applications_container_002 enter");
    sptr<BluetoothBleAdvertiserServer> object1 = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote1 = object1->AsObject();
    sptr<BluetoothBleAdvertiserServer> object2 = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote2 = object2->AsObject();
    int pid = 10;
    int uid = 20;

    defaultContainer_->AddRemoteObject(pid, uid, remote1);
    defaultContainer_->AddRemoteObject(pid, uid, remote2);
    EXPECT_TRUE(defaultContainer_->Contain(remote1));
    EXPECT_TRUE(defaultContainer_->Contain(remote2));

    EXPECT_EQ(defaultContainer_->GetRemotePid(remote1), pid);
    EXPECT_EQ(defaultContainer_->GetRemotePid(remote2), pid);

    defaultContainer_->RemoveRemoteObject(remote1);
    EXPECT_EQ(defaultContainer_->GetRemotePid(remote1), 0);
    EXPECT_EQ(defaultContainer_->GetRemotePid(remote2), pid);

    HILOGI("bluetooth_applications_container_002 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_applications_container_001
 * @tc.desc: Verify the BluetoothBleAdvertiserApplicationContainer Add/Remove advHandle success
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_applications_container_001, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_applications_container_001 enter");
    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    std::set<AdvHandleRecord> handles {AdvHandleRecord(2), AdvHandleRecord(3), AdvHandleRecord(4),
        AdvHandleRecord(5), AdvHandleRecord(6), AdvHandleRecord(7)};

    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote);
    for (auto handle : handles) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(remote, handle.advHandle);
    }
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote), handles);

    for (auto handle : handles) {
        bleAdvertiserApplicationContainer_->RemoveAdvHandle(handle.advHandle);
    }
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote), std::set<AdvHandleRecord>{});

    HILOGI("bluetooth_ble_advertiser_applications_container_001 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_applications_container_002
 * @tc.desc: Verify the BluetoothBleAdvertiserApplicationContainer Add/Remove advHandle failed
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_applications_container_002, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_applications_container_002 enter");
    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int handle = 2;

    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote);
    bleAdvertiserApplicationContainer_->AddAdvHandle(remote, handle);
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote),
        std::set<AdvHandleRecord>{AdvHandleRecord(handle)});

    bleAdvertiserApplicationContainer_->RemoveAdvHandle(0);
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote),
        std::set<AdvHandleRecord>{AdvHandleRecord(handle)});

    HILOGI("bluetooth_ble_advertiser_applications_container_002 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_applications_container_003
 * @tc.desc: Verify the BluetoothBleAdvertiserApplicationContainer OnRemoteDied success
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_applications_container_003, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_applications_container_003 enter");
    sptr<BluetoothBleAdvertiserServer> object = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    std::set<AdvHandleRecord> handles {AdvHandleRecord(10), AdvHandleRecord(11), AdvHandleRecord(12),
        AdvHandleRecord(13), AdvHandleRecord(14), AdvHandleRecord(15)};

    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote);
    for (auto handle : handles) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(remote, handle.advHandle);
    }
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote), handles);

    bleAdvertiserApplicationContainer_->OnRemoteDied(remote);
    EXPECT_FALSE(bleAdvertiserApplicationContainer_->Contain(remote));
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote), std::set<AdvHandleRecord>{});

    HILOGI("bluetooth_ble_advertiser_applications_container_003 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_applications_container_004
 * @tc.desc: Verify the BluetoothBleAdvertiserApplicationContainer OnRemoteDied failed
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_applications_container_004, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_applications_container_004 enter");
    sptr<BluetoothBleAdvertiserServer> object1 = new BluetoothBleAdvertiserServer();
    sptr<BluetoothBleAdvertiserServer> object2 = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote1 = object1->AsObject();
    sptr<IRemoteObject> remote2 = object2->AsObject();
    int pid = 10;
    int uid = 20;
    std::set<AdvHandleRecord> handles {AdvHandleRecord(10), AdvHandleRecord(11), AdvHandleRecord(12),
        AdvHandleRecord(13), AdvHandleRecord(14), AdvHandleRecord(15)};

    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote1);
    for (auto handle : handles) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(remote1, handle.advHandle);
    }
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote1), handles);
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote2), std::set<AdvHandleRecord>{});

    bleAdvertiserApplicationContainer_->OnRemoteDied(remote2);
    EXPECT_TRUE(bleAdvertiserApplicationContainer_->Contain(remote1));
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(remote1), handles);

    HILOGI("bluetooth_ble_advertiser_applications_container_004 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_applications_container_005
 * @tc.desc: Verify the BluetoothBleAdvertiserApplicationContainer GetAdvHandles
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_applications_container_005, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_applications_container_005 enter");
    sptr<BluetoothBleAdvertiserServer> object1 = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object1->AsObject();
    int pid = 10;
    int uid = 20;
    std::set<AdvHandleRecord> handles {AdvHandleRecord(10), AdvHandleRecord(11), AdvHandleRecord(12),
        AdvHandleRecord(13), AdvHandleRecord(14), AdvHandleRecord(15)};

    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote);
    for (auto handle : handles) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(remote, handle.advHandle);
    }
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(pid, uid), handles);
    EXPECT_EQ(bleAdvertiserApplicationContainer_->GetAdvHandles(11, 12), std::set<AdvHandleRecord>{});

    HILOGI("bluetooth_ble_advertiser_applications_container_005 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_applications_container_006
 * @tc.desc: Verify the BluetoothBleAdvertiserApplicationContainer IsApplicationAdv
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_applications_container_006, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_applications_container_006 enter");
    sptr<BluetoothBleAdvertiserServer> object1 = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object1->AsObject();
    int pid = 10;
    int uid = 20;
    int handle = 9;
    std::set<AdvHandleRecord> handles {AdvHandleRecord(10), AdvHandleRecord(11), AdvHandleRecord(12),
        AdvHandleRecord(13), AdvHandleRecord(14), AdvHandleRecord(15)};

    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote);
    for (auto tempHandle : handles) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(remote, tempHandle.advHandle);
    }
    EXPECT_FALSE(bleAdvertiserApplicationContainer_->IsApplicationAdv(pid, uid, handle));
    EXPECT_TRUE(bleAdvertiserApplicationContainer_->IsApplicationAdv(pid, uid, 15));

    HILOGI("bluetooth_ble_advertiser_applications_container_006 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_preemption_001
 * @tc.desc: Verify advertising preemption when non-softbus advertising num exceeded
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_preemption_001, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_preemption_001 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();

    sptr<BluetoothBleAdvertiserServer> object1 = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object1->AsObject();
    int pid = 10;
    int uid = 20;

    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote);
    bleAdvertiserApplicationContainer_->SetCallingName(remote, "AAA");
    int i = 0;
    for (; i < ADV_NUM_LIMIT; i++) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(remote, i);
    }
    // When non-softbus adv num reaching ADV_NUM_LIMIT, no adv is preemptible
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), BLE_INVALID_ADVERTISING_HANDLE);
    // When non-softbus adv num reaching ADV_NUM_LIMIT + 1, no adv is preemptible
    bleAdvertiserApplicationContainer_->AddAdvHandle(remote, ++i);
    auto future = std::async(std::launch::async, [this]() {
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        std::unique_lock<std::mutex> lock(bleAdvertiserApplicationContainer_->advPreemptionLock);
        bleAdvertiserApplicationContainer_->preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
        bleAdvertiserApplicationContainer_->advPreemptionCV.notify_all();
    });
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), 0);

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();

    HILOGI("bluetooth_ble_advertiser_preemption_001 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_preemption_002
 * @tc.desc: Verify advertising preemption when softbus advertising num exceeded
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_preemption_002, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_preemption_002 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();

    sptr<BluetoothBleAdvertiserServer> object1 = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object1->AsObject();
    int pid = 10;
    int uid = SOFTBUS_UID;
    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote);
    bleAdvertiserApplicationContainer_->SetCallingName(remote, "softbus_server");

    int i = 0;
    for (; i < SOFTBUS_ADV_NUM_LIMIT; i++) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(remote, i);
    }
    // When softbus adv num reaching SOFTBUS_ADV_NUM_LIMIT, no adv is preemptible
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), BLE_INVALID_ADVERTISING_HANDLE);
    // When softbus adv num reaching SOFTBUS_ADV_NUM_LIMIT + 1, adv 0 is preemptible
    bleAdvertiserApplicationContainer_->AddAdvHandle(remote, ++i);
    auto future = std::async(std::launch::async, [this]() {
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        std::unique_lock<std::mutex> lock(bleAdvertiserApplicationContainer_->advPreemptionLock);
        bleAdvertiserApplicationContainer_->preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
        bleAdvertiserApplicationContainer_->advPreemptionCV.notify_all();
    });
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), 0);

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();

    HILOGI("bluetooth_ble_advertiser_preemption_002 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_preemption_003
 * @tc.desc: Verify advertising preemption when softbus and non-softbus advertising num both exceeded
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_preemption_003, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_preemption_003 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();

    sptr<BluetoothBleAdvertiserServer> softbus = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> softbusRemote = softbus->AsObject();
    int softbusPid = 10;
    int softbusUid = SOFTBUS_UID;
    sptr<BluetoothBleAdvertiserServer> nonSoftbus = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> nonSoftbusRemote = nonSoftbus->AsObject();
    int nonSoftbusPid = 20;
    int nonSoftbusUid = 30;
    bleAdvertiserApplicationContainer_->AddRemoteObject(softbusPid, softbusUid, softbusRemote);
    bleAdvertiserApplicationContainer_->SetCallingName(softbusRemote, "softbus_server");
    bleAdvertiserApplicationContainer_->AddRemoteObject(nonSoftbusPid, nonSoftbusUid, nonSoftbusRemote);
    bleAdvertiserApplicationContainer_->SetCallingName(nonSoftbusRemote, "AAA");

    int i = 0;
    for (; i < ADV_NUM_LIMIT; i++) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(nonSoftbusRemote, i);
    }
    for (; i < SOFTBUS_ADV_NUM_LIMIT; i++) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(softbusRemote, i);
    }
    // When softbus and non-softbus adv num both not exceeded, no advertising is preemptible
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), BLE_INVALID_ADVERTISING_HANDLE);
    // When softbus and non-softbus adv num both exceeded, nonSoftbus adv will be preempted
    bleAdvertiserApplicationContainer_->AddAdvHandle(nonSoftbusRemote, ++i);
    bleAdvertiserApplicationContainer_->AddAdvHandle(softbusRemote, ++i);
    auto future = std::async(std::launch::async, [this]() {
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        std::unique_lock<std::mutex> lock(bleAdvertiserApplicationContainer_->advPreemptionLock);
        bleAdvertiserApplicationContainer_->preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
        bleAdvertiserApplicationContainer_->advPreemptionCV.notify_all();
    });
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), 0);

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();

    HILOGI("bluetooth_ble_advertiser_preemption_003 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_preemption_004
 * @tc.desc: Verify advertising preemption when non-softbus adv num not exceeded but softbus adv num exceeded
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_preemption_004, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_preemption_004 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();

    sptr<BluetoothBleAdvertiserServer> softbus = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> softbusRemote = softbus->AsObject();
    int softbusPid = 10;
    int softbusUid = SOFTBUS_UID;
    sptr<BluetoothBleAdvertiserServer> nonSoftbus = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> nonSoftbusRemote = nonSoftbus->AsObject();
    int nonSoftbusPid = 20;
    int nonSoftbusUid = 30;
    bleAdvertiserApplicationContainer_->AddRemoteObject(softbusPid, softbusUid, softbusRemote);
    bleAdvertiserApplicationContainer_->SetCallingName(softbusRemote, "softbus_server");
    bleAdvertiserApplicationContainer_->AddRemoteObject(nonSoftbusPid, nonSoftbusUid, nonSoftbusRemote);
    bleAdvertiserApplicationContainer_->SetCallingName(nonSoftbusRemote, "AAA");

    int i = 0;
    for (; i < SOFTBUS_ADV_NUM_LIMIT; i++) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(softbusRemote, i);
    }
    for (; i < ADV_NUM_LIMIT; i++) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(nonSoftbusRemote, i);
    }
    // When softbus and non-softbus adv num both not exceeded, no advertising is preemptible
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), BLE_INVALID_ADVERTISING_HANDLE);
    // When non-softbus adv num not exceeded but softbus adv num exceeded, softbus adv will be preempted
    bleAdvertiserApplicationContainer_->AddAdvHandle(softbusRemote, ++i);
    auto future = std::async(std::launch::async, [this]() {
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        std::unique_lock<std::mutex> lock(bleAdvertiserApplicationContainer_->advPreemptionLock);
        bleAdvertiserApplicationContainer_->preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
        bleAdvertiserApplicationContainer_->advPreemptionCV.notify_all();
    });
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), 0);

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();

    HILOGI("bluetooth_ble_advertiser_preemption_004 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_preemption_005
 * @tc.desc: Verify advertising preemption when softbus adv num not exceeded but non-softbus adv num exceeded
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_preemption_005, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_preemption_005 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();

    sptr<BluetoothBleAdvertiserServer> softbus = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> softbusRemote = softbus->AsObject();
    int softbusPid = 10;
    int softbusUid = SOFTBUS_UID;
    sptr<BluetoothBleAdvertiserServer> nonSoftbus = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> nonSoftbusRemote = nonSoftbus->AsObject();
    int nonSoftbusPid = 20;
    int nonSoftbusUid = 30;
    bleAdvertiserApplicationContainer_->AddRemoteObject(softbusPid, softbusUid, softbusRemote);
    bleAdvertiserApplicationContainer_->SetCallingName(softbusRemote, "softbus_server");
    bleAdvertiserApplicationContainer_->AddRemoteObject(nonSoftbusPid, nonSoftbusUid, nonSoftbusRemote);
    bleAdvertiserApplicationContainer_->SetCallingName(nonSoftbusRemote, "AAA");

    int i = 0;
    for (; i < ADV_NUM_LIMIT; i++) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(nonSoftbusRemote, i);
    }
    for (; i < SOFTBUS_ADV_NUM_LIMIT; i++) {
        bleAdvertiserApplicationContainer_->AddAdvHandle(softbusRemote, i);
    }
    // When softbus and non-softbus adv num both not exceeded, no advertising is preemptible
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), BLE_INVALID_ADVERTISING_HANDLE);
    // When softbus adv num not exceeded but non-softbus adv num exceeded, non-softbus adv will be preempted
    bleAdvertiserApplicationContainer_->AddAdvHandle(nonSoftbusRemote, ++i);
    auto future = std::async(std::launch::async, [this]() {
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        std::unique_lock<std::mutex> lock(bleAdvertiserApplicationContainer_->advPreemptionLock);
        bleAdvertiserApplicationContainer_->preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
        bleAdvertiserApplicationContainer_->advPreemptionCV.notify_all();
    });
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), 0);

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();

    HILOGI("bluetooth_ble_advertiser_preemption_005 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_preemption_006
 * @tc.desc: Verify advertising preemption when non-softbus advertising time exceeded
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_preemption_006, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_preemption_006 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();

    sptr<BluetoothBleAdvertiserServer> object1 = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> remote = object1->AsObject();
    int pid = 10;
    int uid = 20;
    bleAdvertiserApplicationContainer_->AddRemoteObject(pid, uid, remote);
    bleAdvertiserApplicationContainer_->SetCallingName(remote, "AAA");

    TimePoint now = std::chrono::system_clock::now();
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        remote, AdvHandleRecord(0, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT - 1)));
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        remote, AdvHandleRecord(1, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT + 1)));
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        remote, AdvHandleRecord(2, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT + 2)));
    auto future = std::async(std::launch::async, [this]() {
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        std::unique_lock<std::mutex> lock(bleAdvertiserApplicationContainer_->advPreemptionLock);
        bleAdvertiserApplicationContainer_->preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
        bleAdvertiserApplicationContainer_->advPreemptionCV.notify_all();
    });
    // When non-softbus adv time reaching ADV_HOLDING_TIME_LIMIT, preempt the adv with longest holding time
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), 2);

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();

    HILOGI("bluetooth_ble_advertiser_preemption_006 end");
}

/**
 * @tc.name: bluetooth_ble_advertiser_preemption_007
 * @tc.desc: Verify advertising preemption when softbus and non-softbus advertising time both exceeded
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_advertiser_preemption_007, TestSize.Level1)
{
    HILOGI("bluetooth_ble_advertiser_preemption_007 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();

    sptr<BluetoothBleAdvertiserServer> softbus = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> softbusRemote = softbus->AsObject();
    int softbusPid = 10;
    int softbusUid = SOFTBUS_UID;
    sptr<BluetoothBleAdvertiserServer> nonSoftbus = new BluetoothBleAdvertiserServer();
    sptr<IRemoteObject> nonSoftbusRemote = nonSoftbus->AsObject();
    int nonSoftbusPid = 20;
    int nonSoftbusUid = 30;
    bleAdvertiserApplicationContainer_->AddRemoteObject(softbusPid, softbusUid, softbusRemote);
    bleAdvertiserApplicationContainer_->SetCallingName(softbusRemote, "softbus_server");
    bleAdvertiserApplicationContainer_->AddRemoteObject(nonSoftbusPid, nonSoftbusUid, nonSoftbusRemote);
    bleAdvertiserApplicationContainer_->SetCallingName(nonSoftbusRemote, "AAA");

    TimePoint now = std::chrono::system_clock::now();
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        softbusRemote, AdvHandleRecord(0, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT - 10)));
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        softbusRemote, AdvHandleRecord(1, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT + 10)));
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        softbusRemote, AdvHandleRecord(2, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT + 20)));
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        nonSoftbusRemote, AdvHandleRecord(3, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT - 1)));
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        nonSoftbusRemote, AdvHandleRecord(4, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT + 1)));
    bleAdvertiserApplicationContainer_->AddAdvHandle(
        nonSoftbusRemote, AdvHandleRecord(5, now - std::chrono::milliseconds(ADV_HOLDING_TIME_LIMIT + 2)));
    auto future = std::async(std::launch::async, [this]() {
        const int delay = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        std::unique_lock<std::mutex> lock(bleAdvertiserApplicationContainer_->advPreemptionLock);
        bleAdvertiserApplicationContainer_->preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
        bleAdvertiserApplicationContainer_->advPreemptionCV.notify_all();
    });
    // When softbus and non-softbus adv time both reaching ADV_HOLDING_TIME_LIMIT, preempt non-softbus adv first
    EXPECT_EQ(bleAdvertiserApplicationContainer_->PreemptAdvHandle(), 5);

    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreateNull);
    AdapterManager::GetInstance()->CreateAdapters();

    HILOGI("bluetooth_ble_advertiser_preemption_007 end");
}

/**
 * @tc.name: bluetooth_ble_scanner_applications_container_001
 * @tc.desc: Verify the BluetoothBleScannerApplicationContainer TokenId
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_scanner_applications_container_001, TestSize.Level1)
{
    HILOGI("bluetooth_ble_scanner_applications_container_001 enter");
    sptr<BluetoothBleCentralManagerServer> object1 = new BluetoothBleCentralManagerServer();
    sptr<BluetoothBleCentralManagerServer> object2 = new BluetoothBleCentralManagerServer();
    sptr<IRemoteObject> remote1 = object1->AsObject();
    sptr<IRemoteObject> remote2 = object2->AsObject();
    int pid = 10;
    int uid = 20;
    uint32_t tokenId = 2000;

    bleScannerApplicationContainer_->AddRemoteObject(pid, uid, remote1);
    bleScannerApplicationContainer_->AddTokenId(remote1, tokenId);
    bleScannerApplicationContainer_->AddSdkVersion(remote1, 1);
    EXPECT_EQ(bleScannerApplicationContainer_->GetTokenId(remote1), tokenId);

    bleScannerApplicationContainer_->AddTokenId(remote2, tokenId);
    bleScannerApplicationContainer_->AddSdkVersion(remote2, 1);
    EXPECT_EQ(bleScannerApplicationContainer_->GetTokenId(remote2), 0);
    bleScannerApplicationContainer_->GetSdkVersion(remote2);

    HILOGI("bluetooth_ble_scanner_applications_container_001 end");
}

/**
 * @tc.name: bluetooth_ble_scanner_applications_container_002
 * @tc.desc: Verify the BluetoothBleScannerApplicationContainer ClientId
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_scanner_applications_container_002, TestSize.Level1)
{
    HILOGI("bluetooth_ble_scanner_applications_container_002 enter");
    sptr<BluetoothBleCentralManagerServer> object = new BluetoothBleCentralManagerServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 10;

    bleScannerApplicationContainer_->AddRemoteObject(pid, uid, remote);
    bleScannerApplicationContainer_->SetScannerId(remote, pid, uid, scannerId);  // SetScannerId
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote), scannerId);

    bleScannerApplicationContainer_->ClearScannerId();
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote), 0);

    HILOGI("bluetooth_ble_scanner_applications_container_002 end");
}

/**
 * @tc.name: bluetooth_ble_scanner_applications_container_003
 * @tc.desc: Verify the BluetoothBleScannerApplicationContainer OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_scanner_applications_container_003, TestSize.Level1)
{
    HILOGI("bluetooth_ble_scanner_applications_container_003 enter");
    sptr<BluetoothBleCentralManagerServer> object1 = new BluetoothBleCentralManagerServer();
    sptr<BluetoothBleCentralManagerServer> object2 = new BluetoothBleCentralManagerServer();
    sptr<IRemoteObject> remote1 = object1->AsObject();
    sptr<IRemoteObject> remote2 = object2->AsObject();
    int pid = 10;
    int uid = 20;
    int pid1 = 11;
    int uid1 = 21;
    int scannerId1 = 10;
    int scannerId2 = 20;

    bleScannerApplicationContainer_->AddRemoteObject(pid, uid, remote1);
    bleScannerApplicationContainer_->SetScannerId(remote1, pid, uid, scannerId1);

    bleScannerApplicationContainer_->OnRemoteDied(remote2);
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote1), scannerId1);

    bleScannerApplicationContainer_->OnRemoteDied(remote1);
    EXPECT_FALSE(bleAdvertiserApplicationContainer_->Contain(remote1));
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote1), 0);

    bleScannerApplicationContainer_->AddRemoteObject(pid, uid, remote1);
    bleScannerApplicationContainer_->SetScannerId(remote1, pid1, uid, scannerId1);
    bleScannerApplicationContainer_->AddRemoteObject(pid, uid, remote2);
    bleScannerApplicationContainer_->SetScannerId(remote1, pid, uid1, scannerId2);
    bleScannerApplicationContainer_->SetScannerId(remote1, pid1, uid1, scannerId2);
    bleScannerApplicationContainer_->OnRemoteDied(remote1);
    bleScannerApplicationContainer_->OnRemoteDied(remote2);
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote1), 0);
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote2), 0);

    HILOGI("bluetooth_ble_scanner_applications_container_003 end");
}

/**
 * @tc.name: bluetooth_ble_scanner_applications_container_004
 * @tc.desc: Verify the BluetoothBleScannerApplicationContainer scanSettings and remote
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_scanner_applications_container_004, TestSize.Level1)
{
    HILOGI("bluetooth_ble_scanner_applications_container_004 enter");
    sptr<BluetoothBleCentralManagerServer> object = new BluetoothBleCentralManagerServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 10;

    bleScannerApplicationContainer_->AddRemoteObject(pid, uid, remote);
    bleScannerApplicationContainer_->SetScannerId(remote, pid, uid, scannerId);
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote), scannerId);

    OHOS::bluetooth::BleScanSettingsImpl settings;
    settings.SetCallbackType(BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH);
    bleScannerApplicationContainer_->SetScanIdSetting(pid, uid, scannerId, settings);
    uint8_t callbackType = bleScannerApplicationContainer_->GetScanSetting(scannerId).GetCallbackType();
    EXPECT_EQ(callbackType, BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH);

    EXPECT_NE(bleScannerApplicationContainer_->GetRemote(scannerId), nullptr);

    bleScannerApplicationContainer_->ClearScannerId();
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote), 0);

    HILOGI("bluetooth_ble_scanner_applications_container_004 end");
}

/**
 * @tc.name: bluetooth_ble_scanner_applications_container_005
 * @tc.desc: Verify the BluetoothBleScannerApplicationContainer get BleScanFilter
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_scanner_applications_container_005, TestSize.Level1)
{
    HILOGI("bluetooth_ble_scanner_applications_container_005 enter");
    sptr<BluetoothBleCentralManagerServer> object = new BluetoothBleCentralManagerServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 10;

    bleScannerApplicationContainer_->AddRemoteObject(pid, uid, remote);
    bleScannerApplicationContainer_->SetScannerId(remote, pid, uid, scannerId);
    std::vector<OHOS::bluetooth::BleScanFilterImpl> bleScanFilters;
    bleScannerApplicationContainer_->SetBleScanFilter(scannerId, bleScanFilters);
    bleScannerApplicationContainer_->GetBleScanFilterSize(scannerId);
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote), scannerId);

    bleScannerApplicationContainer_->ClearScannerId();
    EXPECT_EQ(bleScannerApplicationContainer_->GetScannerId(remote), 0);
    bleScannerApplicationContainer_->GetBleScanFilterSize(scannerId);

    HILOGI("bluetooth_ble_scanner_applications_container_005 end");
}

/**
 * @tc.name: bluetooth_ble_scanner_applications_container_006
 * @tc.desc: Verify the BluetoothBleScannerApplicationContainer GetUseRealAddrFlag
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_scanner_applications_container_006, TestSize.Level1)
{
    HILOGI("bluetooth_ble_scanner_applications_container_006 enter");
    sptr<BluetoothBleCentralManagerServer> object = new BluetoothBleCentralManagerServer();
    sptr<IRemoteObject> remote = object->AsObject();
    bool flag = true;

    bleScannerApplicationContainer_->SetUseRealAddrFlag(remote, flag);
    EXPECT_TRUE(bleScannerApplicationContainer_->GetUseRealAddrFlag(remote));

    int scannerId = 10;
    std::string name = "testName";
    bleScannerApplicationContainer_->SetCallingName(scannerId, name);
    EXPECT_TRUE(bleScannerApplicationContainer_->GetUseRealAddrFlag(scannerId));

    HILOGI("bluetooth_ble_scanner_applications_container_006 end");
}

/**
 * @tc.name: bluetooth_ble_scanner_applications_container_007
 * @tc.desc: Verify the BluetoothBleScannerApplicationContainer IsBleScanFilterMatched
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_scanner_applications_container_007, TestSize.Level1)
{
    HILOGI("bluetooth_ble_scanner_applications_container_007 enter");
    int scannerId = 10;
    BluetoothBleScanResult bleScanResult;
    int32_t subErrCode = 0;
    bleScannerApplicationContainer_->ChrReportScanFilter(subErrCode);
    bluetooth::BleScanFilterImpl matchedFilter;
    EXPECT_FALSE(bleScannerApplicationContainer_->IsBleScanFilterMatched(scannerId, bleScanResult,
        matchedFilter));

    HILOGI("bluetooth_ble_scanner_applications_container_007 end");
}

/**
 * @tc.name: bluetooth_ble_scanner_applications_container_008
 * @tc.desc: Verify the BluetoothBleScannerApplicationContainer SetLpDeviceFlag
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothApplicationsContainerTest, bluetooth_ble_scanner_applications_container_008, TestSize.Level1)
{
    HILOGI("bluetooth_ble_scanner_applications_container_008 enter");

    sptr<BluetoothBleCentralManagerServer> object = new BluetoothBleCentralManagerServer();
    sptr<IRemoteObject> remote = object->AsObject();
    int pid = 10;
    int uid = 20;
    int scannerId = 10;
    bool enable = true;
    bleScannerApplicationContainer_->AddRemoteObject(pid, uid, remote);
    bleScannerApplicationContainer_->SetScannerId(remote, pid, uid, scannerId);

    std::vector<OHOS::bluetooth::BleScanFilterImpl> bleScanFilters;
    OHOS::bluetooth::BleScanFilterImpl filter;
    BluetoothBleScanResult bleScanResult;
    bleScanFilters.emplace_back(filter);
    bleScannerApplicationContainer_->SetBleScanFilter(scannerId, bleScanFilters);
    bleScannerApplicationContainer_->SetLpDeviceFlag(scannerId, enable);
    EXPECT_FALSE(bleScannerApplicationContainer_->IsBleScanFilterMatched(scannerId, bleScanResult, filter));

    HILOGI("bluetooth_ble_scanner_applications_container_008 end");
}
}
}