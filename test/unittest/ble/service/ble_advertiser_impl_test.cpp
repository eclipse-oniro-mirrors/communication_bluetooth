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
#define private public
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <memory>
#include "ble_advertiser_impl.h"
#include "ble_adapter.h"
#include "bluetooth_host.h"
#include "class_creator.h"
#include "thread_util.h"

#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace OHOS {
namespace bluetooth {
class BleAdvertiserCallbackTest : public IBleAdvertiserCallback {
public:
    ~BleAdvertiserCallbackTest() override{};
    void OnStartResultEvent(int result, uint8_t advHandle, int opcode = BLE_ADV_DEFAULT_OP_CODE) override {}
    void OnEnableResultEvent(int result, uint8_t advHandle) override {}
    void OnDisableResultEvent(int result, uint8_t advHandle) override {}
    void OnStopResultEvent(int result, uint8_t advHandle) override {}
    void OnAutoStopAdvEvent(uint8_t advHandle) override {}
    void OnSetAdvDataEvent(int result, uint8_t advHandle, bluetooth::SetAdvDataType type) override {}
    void OnChangeAdvResultEvent(int result, uint8_t advHandle) override {}
};
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;
class BleAdvertiserImplTest : public testing::Test {
public:
    BleAdvertiserImplTest() = default;
    ~BleAdvertiserImplTest() = default;

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    BleAdvertiserCallbackTest bleAdvertiserCallbackTest;
    BleAdapter *bleAdapter;
    std::unique_ptr<BleAdvertiserImpl> bleAdvertiserImpl;
private:
};

void BleAdvertiserImplTest::SetUpTestSuite(void)
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

void BleAdvertiserImplTest::TearDownTestSuite(void)
{
    host_ = nullptr;
}

void BleAdvertiserImplTest::SetUp()
{
    bleAdapter = ClassCreator<BleAdapter>::NewInstance(ADAPTER_NAME_BLE);
    bleAdvertiserImpl = std::make_unique<BleAdvertiserImpl>(bleAdvertiserCallbackTest, *bleAdapter);
    // Enable ble thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ThreadState::ENABLED);
    sleep(1);
}

void BleAdvertiserImplTest::TearDown()
{
    sleep(1);
    delete bleAdapter;
    bleAdapter = nullptr;
    bleAdvertiserImpl = nullptr;
}

/**
 * @tc.name: ble_advertiser_CreateAdvertiserSetHandle_001
 * @tc.desc: ble_advertiser_CreateAdvertiserSetHandle_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_CreateAdvertiserSetHandle_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_CreateAdvertiserSetHandle_001 start");
    int status = -1;
    EXPECT_EQ(bleAdvertiserImpl->CreateAdvertiserSetHandle(status), 0xFF);
    HILOGI("ble_advertiser_CreateAdvertiserSetHandle_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvStartedEvent_001
 * @tc.desc: ble_advertiser_OnAdvStartedEvent_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvStartedEvent_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvStartedEvent_001 start");
    uint8_t advHandle = 1;
    uint8_t status = 0;
    bleAdvertiserImpl->OnAdvStartedEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvStartedEvent_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvStartedEvent_002
 * @tc.desc: ble_advertiser_OnAdvStartedEvent_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvStartedEvent_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvStartedEvent_002 start");
    uint8_t advHandle = 1;
    uint8_t status = 1;
    bleAdvertiserImpl->OnAdvStartedEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvStartedEvent_002 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvStoppedEvent_001
 * @tc.desc: ble_advertiser_OnAdvStoppedEvent_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvStoppedEvent_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvStoppedEvent_001 start");
    uint8_t advHandle = 1;
    uint8_t status = 0;
    bleAdvertiserImpl->OnAdvStoppedEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvStoppedEvent_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvStoppedEvent_002
 * @tc.desc: ble_advertiser_OnAdvStoppedEvent_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvStoppedEvent_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvStoppedEvent_002 start");
    uint8_t advHandle = 1;
    uint8_t status = 1;
    bleAdvertiserImpl->OnAdvStoppedEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvStoppedEvent_002 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvStoppedEvent_004
 * @tc.desc: ble_advertiser_OnAdvStoppedEvent_004
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvStoppedEvent_004, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvStoppedEvent_004 start");
    uint8_t advHandle = 1;
    uint8_t status = 0;
    bleAdvertiserImpl->bleAdapter_ = nullptr;
    bleAdvertiserImpl->OnAdvStoppedEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvStoppedEvent_004 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvEnabledEvent_001
 * @tc.desc: ble_advertiser_OnAdvEnabledEvent_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvEnabledEvent_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvEnabledEvent_001 start");
    uint8_t advHandle = 1;
    uint8_t status = 0;
    bleAdvertiserImpl->OnAdvEnabledEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvEnabledEvent_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvEnabledEvent_002
 * @tc.desc: ble_advertiser_OnAdvEnabledEvent_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvEnabledEvent_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvEnabledEvent_002 start");
    uint8_t advHandle = 1;
    uint8_t status = 1;
    bleAdvertiserImpl->OnAdvEnabledEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvEnabledEvent_002 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvDisabledEvent_001
 * @tc.desc: ble_advertiser_OnAdvDisabledEvent_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvDisabledEvent_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvDisabledEvent_001 start");
    uint8_t advHandle = 1;
    uint8_t status = 0;
    bleAdvertiserImpl->OnAdvDisabledEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvDisabledEvent_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvDisabledEvent_002
 * @tc.desc: ble_advertiser_OnAdvDisabledEvent_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvDisabledEvent_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvDisabledEvent_002 start");
    uint8_t advHandle = 1;
    uint8_t status = 1;
    bleAdvertiserImpl->OnAdvDisabledEvent(advHandle, status);
    HILOGI("ble_advertiser_OnAdvDisabledEvent_002 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvStartedEventFromStack_001
 * @tc.desc: ble_advertiser_OnAdvStartedEventFromStack_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvStartedEventFromStack_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvStartedEventFromStack_001 start");
    bool enable = true;
    uint8_t advHandle = 1;
    uint8_t status = 0;
    bleAdvertiserImpl->OnAdvStartedEventFromStack(advHandle, status);
    HILOGI("ble_advertiser_OnAdvStartedEventFromStack_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvEnabledEventFromStack_001
 * @tc.desc: ble_advertiser_OnAdvEnabledEventFromStack_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvEnabledEventFromStack_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvEnabledEventFromStack_001 start");
    bool enable = true;
    uint8_t advHandle = 1;
    uint8_t status = 0;
    bleAdvertiserImpl->OnAdvEnabledEventFromStack(advHandle, status);
    HILOGI("ble_advertiser_OnAdvEnabledEventFromStack_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvStartedEventFromStack_002
 * @tc.desc: ble_advertiser_OnAdvStartedEventFromStack_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvStartedEventFromStack_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvStartedEventFromStack_002 start");
    bool enable = true;
    uint8_t advHandle = 1;
    uint8_t status = 0;
    // Dsiable ble thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ThreadState::DISABLED);
    sleep(2);
    bleAdvertiserImpl->OnAdvStartedEventFromStack(advHandle, status);
    HILOGI("ble_advertiser_OnAdvStartedEventFromStack_002 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvEnabledEventFromStack_002
 * @tc.desc: ble_advertiser_OnAdvEnabledEventFromStack_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvEnabledEventFromStack_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvEnabledEventFromStack_002 start");
    bool enable = true;
    uint8_t advHandle = 1;
    uint8_t status = 0;
    // Dsiable ble thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ThreadState::DISABLED);
    bleAdvertiserImpl->OnAdvEnabledEventFromStack(advHandle, status);
    HILOGI("ble_advertiser_OnAdvEnabledEventFromStack_002 end");
}

/**
 * @tc.name: ble_advertiser_IsAdvStarted_001
 * @tc.desc: ble_advertiser_IsAdvStarted_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_IsAdvStarted_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_IsAdvStarted_001 start");
    uint8_t advHandle = 1;
    EXPECT_FALSE(bleAdvertiserImpl->IsAdvStarted(advHandle));
    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(advHandle);
    bleAdvertiserInstance.advStatus = 3;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance);
    EXPECT_TRUE(bleAdvertiserImpl->IsAdvStarted(advHandle));
    HILOGI("ble_advertiser_IsAdvStarted_001 end");
}

/**
 * @tc.name: ble_advertiser_ChangeAdvStatus_001
 * @tc.desc: ble_advertiser_ChangeAdvStatus_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_ChangeAdvStatus_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_ChangeAdvStatus_001 start");
    uint8_t advHandle = 1;
    int advStatus = 1;
    bleAdvertiserImpl->ChangeAdvStatus(advHandle, advStatus);
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->ChangeAdvStatus(advHandle, advStatus);
    HILOGI("ble_advertiser_ChangeAdvStatus_001 end");
}

/**
 * @tc.name: ble_advertiser_ParseSettings_001
 * @tc.desc: ble_advertiser_ParseSettings_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_ParseSettings_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_ParseSettings_001 start");
    BleAdvertiserSettingsImpl settings;
    settings.SetConnectable(true);
    bool hasScanRsp = false;
    AdvertiseParameters outParams;
    bleAdvertiserImpl->ParseSettings(settings, hasScanRsp, &outParams);
    HILOGI("ble_advertiser_ParseSettings_001 end");
}

/**
 * @tc.name: ble_advertiser_ParseSettings_002
 * @tc.desc: ble_advertiser_ParseSettings_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_ParseSettings_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_ParseSettings_002 start");
    BleAdvertiserSettingsImpl settings;
    settings.SetConnectable(false);
    bool hasScanRsp = false;
    AdvertiseParameters outParams;
    bleAdvertiserImpl->ParseSettings(settings, hasScanRsp, &outParams);
    HILOGI("ble_advertiser_ParseSettings_002 end");
}

/**
 * @tc.name: ble_advertiser_ParseSettings_003
 * @tc.desc: ble_advertiser_ParseSettings_003
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_ParseSettings_003, TestSize.Level1)
{
    HILOGI("ble_advertiser_ParseSettings_003 start");
    BleAdvertiserSettingsImpl settings;
    settings.SetConnectable(false);
    bool hasScanRsp = true;
    AdvertiseParameters outParams;
    bleAdvertiserImpl->ParseSettings(settings, hasScanRsp, &outParams);
    HILOGI("ble_advertiser_ParseSettings_003 end");
}

/**
 * @tc.name: ble_advertiser_BleAdvertiserDataLog_001
 * @tc.desc: ble_advertiser_BleAdvertiserDataLog_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_BleAdvertiserDataLog_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_BleAdvertiserDataLog_001 start");
    std::string data;
    bleAdvertiserImpl->BleAdvertiserDataLog(data, false);
    HILOGI("ble_advertiser_BleAdvertiserDataLog_001 end");
}

/**
 * @tc.name: ble_advertiser_BleAdvertiserDataLog_002
 * @tc.desc: ble_advertiser_BleAdvertiserDataLog_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_BleAdvertiserDataLog_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_BleAdvertiserDataLog_002 start");
    std::string data = "testdata";
    bleAdvertiserImpl->BleAdvertiserDataLog(data, false);
    HILOGI("ble_advertiser_BleAdvertiserDataLog_002 end");
}

/**
 * @tc.name: ble_advertiser_RemoveAdFlag_001
 * @tc.desc: ble_advertiser_RemoveAdFlag_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_RemoveAdFlag_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_RemoveAdFlag_001 start");
    std::vector<uint8_t> data;
    bleAdvertiserImpl->RemoveAdFlag(data);
    HILOGI("ble_advertiser_RemoveAdFlag_001 end");
}

/**
 * @tc.name: ble_advertiser_RemoveAdFlag_002
 * @tc.desc: ble_advertiser_RemoveAdFlag_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_RemoveAdFlag_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_RemoveAdFlag_001 start");
    std::vector<uint8_t> data;
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    bleAdvertiserImpl->RemoveAdFlag(data);
    HILOGI("ble_advertiser_RemoveAdFlag_001 end");
}

/**
 * @tc.name: ble_advertiser_RemoveAdFlag_003
 * @tc.desc: ble_advertiser_RemoveAdFlag_003
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_RemoveAdFlag_003, TestSize.Level1)
{
    HILOGI("ble_advertiser_RemoveAdFlag_003 start");
    std::vector<uint8_t> data;
    data.push_back(0);
    data.push_back(1);
    bleAdvertiserImpl->RemoveAdFlag(data);
    HILOGI("ble_advertiser_RemoveAdFlag_003 end");
}

/**
 * @tc.name: ble_advertiser_RemoveAdFlag_004
 * @tc.desc: ble_advertiser_RemoveAdFlag_004
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_RemoveAdFlag_004, TestSize.Level1)
{
    HILOGI("ble_advertiser_RemoveAdFlag_004 start");
    std::vector<uint8_t> data;
    data.push_back(0);
    data.push_back(1);
    data.push_back(1);
    data.push_back(1);
    bleAdvertiserImpl->RemoveAdFlag(data);
    HILOGI("ble_advertiser_RemoveAdFlag_004 end");
}

/**
 * @tc.name: ble_advertiser_StartAdvertising_001
 * @tc.desc: ble_advertiser_StartAdvertising_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StartAdvertising_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_StartAdvertising_001 start");
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdvertiserImpl->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    HILOGI("ble_advertiser_StartAdvertising_001 end");
}

/**
 * @tc.name: ble_advertiser_StartAdvertising_002
 * @tc.desc: ble_advertiser_StartAdvertising_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StartAdvertising_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_StartAdvertising_002 start");
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(advHandle);
    bleAdvertiserInstance.advStatus = 3;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance);
    bleAdvertiserImpl->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    HILOGI("ble_advertiser_StartAdvertising_002 end");
}

/**
 * @tc.name: ble_advertiser_StartAdvertising_003
 * @tc.desc: ble_advertiser_StartAdvertising_003
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StartAdvertising_003, TestSize.Level1)
{
    HILOGI("ble_advertiser_StartAdvertising_003 start");
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    HILOGI("ble_advertiser_StartAdvertising_003 end");
}

/**
 * @tc.name: ble_advertiser_StartAdvertising_004
 * @tc.desc: Test Extended PDU mode with both connectable and scannable should fail
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StartAdvertising_004, TestSize.Level1)
{
    HILOGI("ble_advertiser_StartAdvertising_004 start");
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetConnectable(true);
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    scanResponse.SetPayload(std::string({0x02, 0x01, 0x05}));
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    EXPECT_FALSE(bleAdvertiserImpl->IsAdvStarted(advHandle));
    HILOGI("ble_advertiser_StartAdvertising_004 end");
}

/**
 * @tc.name: ble_advertiser_StartAdvertising_005
 * @tc.desc: Test Legacy mode with both connectable and scannable should succeed
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StartAdvertising_005, TestSize.Level1)
{
    HILOGI("ble_advertiser_StartAdvertising_005 start");
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(true);
    settings.SetConnectable(true);
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    scanResponse.SetPayload(std::string({0x02, 0x01, 0x05}));
    uint8_t advHandle = 2;
    uint16_t duration = 0;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
    EXPECT_TRUE(bleAdvertiserImpl->IsAdvStarted(advHandle));
    HILOGI("ble_advertiser_StartAdvertising_005 end");
}

/**
 * @tc.name: ble_advertiser_EnableAdvertising_001
 * @tc.desc: ble_advertiser_EnableAdvertising_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_EnableAdvertising_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_EnableAdvertising_001 start");
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdvertiserImpl->EnableAdvertising(advHandle, duration);

    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance1(advHandle);
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance1);
    bleAdvertiserImpl->EnableAdvertising(advHandle, duration);
    bleAdvertiserImpl->advInstances_.Erase(advHandle);

    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance2(advHandle);
    bleAdvertiserInstance2.advStatus = 3; // 3 mean already started
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance2);
    bleAdvertiserImpl->EnableAdvertising(advHandle, duration);

    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->EnableAdvertising(advHandle, duration);

    HILOGI("ble_advertiser_EnableAdvertising_001 end");
}

/**
 * @tc.name: ble_advertiser_DisableAdvertising_001
 * @tc.desc: ble_advertiser_DisableAdvertising_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_DisableAdvertising_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_DisableAdvertising_001 start");
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdvertiserImpl->DisableAdvertising(advHandle);

    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance1(advHandle);
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance1);
    bleAdvertiserImpl->DisableAdvertising(advHandle);
    bleAdvertiserImpl->advInstances_.Erase(advHandle);

    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance2(advHandle);
    bleAdvertiserInstance2.advStatus = 3; // 3 mean already started
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance2);
    bleAdvertiserImpl->DisableAdvertising(advHandle);

    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->DisableAdvertising(advHandle);

    HILOGI("ble_advertiser_DisableAdvertising_001 end");
}

/**
 * @tc.name: ble_advertiser_StopAdvertising_001
 * @tc.desc: ble_advertiser_StopAdvertising_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StopAdvertising_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_StopAdvertising_001 start");
    uint8_t advHandle = 1;
    bleAdvertiserImpl->StopAdvertising(advHandle);
    HILOGI("ble_advertiser_StopAdvertising_001 end");
}

/**
 * @tc.name: ble_advertiser_StopAdvertising_002
 * @tc.desc: ble_advertiser_StopAdvertising_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StopAdvertising_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_StopAdvertising_002 start");
    uint8_t advHandle = 1;
    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(advHandle);
    bleAdvertiserInstance.advStatus = 3;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance);
    bleAdvertiserImpl->StopAdvertising(advHandle);
    HILOGI("ble_advertiser_StopAdvertising_002 end");
}

/**
 * @tc.name: ble_advertiser_StopAdvertising_003
 * @tc.desc: ble_advertiser_StopAdvertising_003
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StopAdvertising_003, TestSize.Level1)
{
    HILOGI("ble_advertiser_StopAdvertising_003 start");
    uint8_t advHandle = 1;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->StopAdvertising(advHandle);
    sleep(1);
    HILOGI("ble_advertiser_StopAdvertising_003 end");
}

/**
 * @tc.name: ble_advertiser_StopAdvertising_004
 * @tc.desc: ble_advertiser_StopAdvertising_004
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_StopAdvertising_004, TestSize.Level1)
{
    HILOGI("ble_advertiser_StopAdvertising_004 start");
    uint8_t advHandle = 1;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->StopAdvertising(advHandle);
    sleep(1);
    HILOGI("ble_advertiser_StopAdvertising_004 end");
}

/**
 * @tc.name: ble_advertiser_GetAdvertisingStatus_001
 * @tc.desc: ble_advertiser_GetAdvertisingStatus_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_GetAdvertisingStatus_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_GetAdvertisingStatus_001 start");
    EXPECT_EQ(bleAdvertiserImpl->GetAdvertisingStatus(), -1);
    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(1);
    bleAdvertiserInstance.advStatus = 3;
    bleAdvertiserImpl->advInstances_.EnsureInsert(1, bleAdvertiserInstance);
    EXPECT_EQ(bleAdvertiserImpl->GetAdvertisingStatus(), 3);
    HILOGI("ble_advertiser_GetAdvertisingStatus_001 end");
}

/**
 * @tc.name: ble_advertiser_OnSetAdvertisingDataEvent_001
 * @tc.desc: ble_advertiser_OnSetAdvertisingDataEvent_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnSetAdvertisingDataEvent_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_001 start");
    uint8_t advHandle = 1;
    SetAdvDataState state = SetAdvDataState::ADV;
    std::vector<uint8_t> rspData;
    uint8_t status = 1;
    bleAdvertiserImpl->OnSetAdvertisingDataEvent(advHandle, state, rspData, status);
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_001 end");
}

/**
 * @tc.name: ble_advertiser_OnSetAdvertisingDataEvent_002
 * @tc.desc: ble_advertiser_OnSetAdvertisingDataEvent_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnSetAdvertisingDataEvent_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_002 start");
    uint8_t advHandle = 1;
    SetAdvDataState state = SetAdvDataState::ADV;
    std::vector<uint8_t> rspData;
    uint8_t status = 0;
    bleAdvertiserImpl->OnSetAdvertisingDataEvent(advHandle, state, rspData, status);
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_002 end");
}

/**
 * @tc.name: ble_advertiser_OnSetAdvertisingDataEvent_003
 * @tc.desc: ble_advertiser_OnSetAdvertisingDataEvent_003
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnSetAdvertisingDataEvent_003, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_003 start");
    uint8_t advHandle = 1;
    SetAdvDataState state = SetAdvDataState::COMPLETE;
    std::vector<uint8_t> rspData;
    uint8_t status = 0;
    bleAdvertiserImpl->OnSetAdvertisingDataEvent(advHandle, state, rspData, status);
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_003 end");
}

/**
 * @tc.name: ble_advertiser_OnSetAdvertisingDataEvent_004
 * @tc.desc: ble_advertiser_OnSetAdvertisingDataEvent_004
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnSetAdvertisingDataEvent_004, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_004 start");
    uint8_t advHandle = 1;
    SetAdvDataState state = SetAdvDataState::SCAN_RSP;
    std::vector<uint8_t> rspData;
    uint8_t status = 0;
    bleAdvertiserImpl->OnSetAdvertisingDataEvent(advHandle, state, rspData, status);
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_004 end");
}

/**
 * @tc.name: ble_advertiser_OnSetAdvertisingDataEvent_005
 * @tc.desc: ble_advertiser_OnSetAdvertisingDataEvent_005
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnSetAdvertisingDataEvent_005, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_005 start");
    uint8_t advHandle = 1;
    SetAdvDataState state = SetAdvDataState::SCAN_RSP;
    std::vector<uint8_t> rspData;
    uint8_t status = 0;
    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->OnSetAdvertisingDataEvent(advHandle, state, rspData, status);
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_005 end");
}

/**
 * @tc.name: ble_advertiser_OnSetAdvertisingDataEvent_006
 * @tc.desc: ble_advertiser_OnSetAdvertisingDataEvent_006
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnSetAdvertisingDataEvent_006, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_006 start");
    uint8_t advHandle = 1;
    SetAdvDataState state = SetAdvDataState::COMPLETE;
    std::vector<uint8_t> rspData;
    uint8_t status = 0;
    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->OnSetAdvertisingDataEvent(advHandle, state, rspData, status);
    HILOGI("ble_advertiser_OnSetAdvertisingDataEvent_006 end");
}

/**
 * @tc.name: ble_advertiser_SetAdvertisingData_001
 * @tc.desc: ble_advertiser_SetAdvertisingData_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_SetAdvertisingData_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_SetAdvertisingData_001 start");
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    bleAdvertiserImpl->SetAdvertisingData(advData, scanResponse, advHandle);
    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(advHandle);
    bleAdvertiserInstance.advStatus = 3;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance);
    bleAdvertiserImpl->SetAdvertisingData(advData, scanResponse, advHandle);
    HILOGI("ble_advertiser_SetAdvertisingData_001 end");
}

/**
 * @tc.name: ble_advertiser_SetAdvertisingData_002
 * @tc.desc: ble_advertiser_SetAdvertisingData_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_SetAdvertisingData_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_SetAdvertisingData_002 start");
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    bleAdvertiserImpl->SetAdvertisingData(advData, scanResponse, advHandle);
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->SetAdvertisingData(advData, scanResponse, advHandle);
    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->SetAdvertisingData(advData, scanResponse, advHandle);
    HILOGI("ble_advertiser_SetAdvertisingData_002 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvDisabledEventFromStack_001
 * @tc.desc: ble_advertiser_OnAdvDisabledEventFromStack_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvDisabledEventFromStack_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvDisabledEventFromStack_001 start");
    bool enable = true;
    uint8_t advHandle = 1;
    uint8_t status = 0;
    bleAdvertiserImpl->OnAdvDisabledEventFromStack(advHandle, status);
    sleep(1);
    HILOGI("ble_advertiser_OnAdvDisabledEventFromStack_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvDisabledEventFromStack_002
 * @tc.desc: ble_advertiser_OnAdvDisabledEventFromStack_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvDisabledEventFromStack_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvDisabledEventFromStack_002 start");
    bool enable = true;
    uint8_t advHandle = 1;
    uint8_t status = 0;
    // Dsiable ble thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ThreadState::DISABLED);
    bleAdvertiserImpl->OnAdvDisabledEventFromStack(advHandle, status);
    sleep(1);
    HILOGI("ble_advertiser_OnAdvDisabledEventFromStack_002 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvChangeParamsEventFromStack_001
 * @tc.desc: ble_advertiser_OnAdvChangeParamsEventFromStack_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvChangeParamsEventFromStack_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvChangeParamsEventFromStack_001 start");
    bool enable = true;
    uint8_t advHandle = 1;
    uint8_t status = 0;
    int8_t txPower = 0;
    // Dsiable ble thread
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_BLE, ThreadUtil::ThreadState::DISABLED);
    bleAdvertiserImpl->OnAdvChangeParamsEventFromStack(advHandle, status, txPower);
    sleep(1);
    HILOGI("ble_advertiser_OnAdvChangeParamsEventFromStack_001 end");
}

/**
 * @tc.name: ble_advertiser_OnAdvChangeParamsEvent_001
 * @tc.desc: ble_advertiser_OnAdvChangeParamsEvent_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_OnAdvChangeParamsEvent_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_OnAdvChangeParamsEvent_001 start");
    uint8_t advHandle = 1;
    uint8_t status = 0;
    int8_t txPower = 0;
    bleAdvertiserImpl->OnAdvChangeParamsEvent(advHandle, status, txPower);
    status = 1;
    bleAdvertiserImpl->OnAdvChangeParamsEvent(advHandle, status, txPower);
    HILOGI("ble_advertiser_OnAdvChangeParamsEvent_001 end");
}

/**
 * @tc.name: ble_advertiser_ChangeAdvertisingParams_001
 * @tc.desc: ble_advertiser_ChangeAdvertisingParams_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_ChangeAdvertisingParams_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_ChangeAdvertisingParams_001 start");
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 1;
    uint16_t duration = 0;
    bleAdvertiserImpl->ChangeAdvertisingParams(advHandle, settings);
    HILOGI("ble_advertiser_ChangeAdvertisingParams_001 end");
}

/**
 * @tc.name: ble_advertiser_ChangeAdvertisingParams_002
 * @tc.desc: ble_advertiser_ChangeAdvertisingParams_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_ChangeAdvertisingParams_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_ChangeAdvertisingParams_002 start");
    BleAdvertiserSettingsImpl settings;
    uint8_t advHandle = 1;
    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(advHandle);
    bleAdvertiserInstance.advStatus = 3;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance);
    bleAdvertiserImpl->ChangeAdvertisingParams(advHandle, settings);
    HILOGI("ble_advertiser_ChangeAdvertisingParams_002 end");
}

/**
 * @tc.name: ble_advertiser_ChangeAdvertisingParams_003
 * @tc.desc: ble_advertiser_ChangeAdvertisingParams_003
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_ChangeAdvertisingParams_003, TestSize.Level1)
{
    HILOGI("ble_advertiser_ChangeAdvertisingParams_003 start");
    BleAdvertiserSettingsImpl settings;
    uint8_t advHandle = 1;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->ChangeAdvertisingParams(advHandle, settings);
    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->ChangeAdvertisingParams(advHandle, settings);
    HILOGI("ble_advertiser_ChangeAdvertisingParams_003 end");
}

/**
 * @tc.name: ble_advertiser_ChangeAdvertisingParams_004
 * @tc.desc: Test Extended PDU mode with connectable should fail
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_ChangeAdvertisingParams_004, TestSize.Level1)
{
    HILOGI("ble_advertiser_ChangeAdvertisingParams_004 start");
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetConnectable(true);
    uint8_t advHandle = 1;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, BleAdvertiserImpl::BleAdvertiserInstance(advHandle));
    bleAdvertiserImpl->ChangeAdvertisingParams(advHandle, settings);
    EXPECT_FALSE(bleAdvertiserImpl->IsAdvStarted(advHandle));
    HILOGI("ble_advertiser_ChangeAdvertisingParams_004 end");
}

/**
 * @tc.name: ble_advertiser_IsBurstAdvertise_001
 * @tc.desc: ble_advertiser_IsBurstAdvertise_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_IsBurstAdvertise_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_IsBurstAdvertise_001 start");
    EXPECT_FALSE(bleAdvertiserImpl->IsBurstAdvertise(0, 0));
    HILOGI("ble_advertiser_IsBurstAdvertise_001 end");
}

/**
 * @tc.name: ble_advertiser_SetAdvOrRspData_001
 * @tc.desc: ble_advertiser_SetAdvOrRspData_001
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_SetAdvOrRspData_001, TestSize.Level1)
{
    HILOGI("ble_advertiser_SetAdvOrRspData_001 start");
    BleAdvertiserDataImpl advData;
    uint8_t advHandle = 2;
    bool isAdv = true;
    bleAdvertiserImpl->SetAdvOrRspData(advData, isAdv, advHandle);

    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(advHandle);
    bleAdvertiserInstance.advStatus = 3;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance);
    bleAdvertiserImpl->SetAdvOrRspData(advData, isAdv, advHandle);

    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->SetAdvOrRspData(advData, isAdv, advHandle);
    EXPECT_EQ(bleAdvertiserImpl->GetAdvertisingStatus(), 3);
    HILOGI("ble_advertiser_SetAdvOrRspData_001 end");
}

/**
 * @tc.name: ble_advertiser_SetAdvOrRspData_002
 * @tc.desc: ble_advertiser_SetAdvOrRspData_002
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_SetAdvOrRspData_002, TestSize.Level1)
{
    HILOGI("ble_advertiser_SetAdvOrRspData_002 start");
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 3;
    bool isAdv = false;
    bleAdvertiserImpl->SetAdvOrRspData(scanResponse, isAdv, advHandle);

    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(advHandle);
    bleAdvertiserInstance.advStatus = 3;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance);
    bleAdvertiserImpl->SetAdvOrRspData(scanResponse, isAdv, advHandle);

    bleAdvertiserImpl->btifBleAdvertiser_ = nullptr;
    bleAdvertiserImpl->SetAdvOrRspData(scanResponse, isAdv, advHandle);
    EXPECT_EQ(bleAdvertiserImpl->GetAdvertisingStatus(), 3);
    HILOGI("ble_advertiser_SetAdvOrRspData_002 end");
}

/**
 * @tc.name: ble_advertiser_SetAdvOrRspData_003
 * @tc.desc: ble_advertiser_SetAdvOrRspData_003
 * @tc.type: FUNC
 */
HWTEST_F(BleAdvertiserImplTest, ble_advertiser_SetAdvOrRspData_003, TestSize.Level1)
{
    HILOGI("ble_advertiser_SetAdvOrRspData_003 start");
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    uint8_t advHandle = 4;
    bool isAdv = true;

    BleAdvertiserImpl::BleAdvertiserInstance bleAdvertiserInstance(advHandle);
    bleAdvertiserInstance.advStatus = -1;
    bleAdvertiserImpl->advInstances_.EnsureInsert(advHandle, bleAdvertiserInstance);
    bleAdvertiserImpl->SetAdvOrRspData(advData, isAdv, advHandle);
    bleAdvertiserImpl->SetAdvOrRspData(scanResponse, !isAdv, advHandle);
    EXPECT_EQ(bleAdvertiserImpl->GetAdvertisingStatus(), -1);
    HILOGI("ble_advertiser_SetAdvOrRspData_003 end");
}
}  // namespace bluetooth
}  // namespace OHOS