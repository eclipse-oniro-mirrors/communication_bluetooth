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
#include <thread>

#include "bluetooth_log.h"
#include "bluetooth_device_manager.h"
#include "raw_address.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "classic_config.h"
#include "btcommon/timer_manager.h"

using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;

class BluetoothDeviceManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBleEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp()
    {
        BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    }
    void TearDown()
    {}
public:
};

/**
 * @tc.name: bluetooth_device_manager_test_001
 * @tc.desc: BluetoothDeviceManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_001, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_001 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:55");
    RawAddress realAddr2 = RawAddress("11:11:22:33:44:55");
    RawAddress realAddr3 = RawAddress("22:11:22:33:44:55");
    RawAddress realAddr4 = RawAddress("33:11:22:33:44:55");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    RawAddress randomAddr1 = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr1);
    RawAddress randomAddr2 = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr2);
    RawAddress randomAddr3 = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr3);
    RawAddress testAddr;
    int32_t ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(realAddr1, testAddr);
    EXPECT_EQ(ret, RET_SUCCESS);
    EXPECT_EQ(testAddr, randomAddr1);

    ret = BluetoothDeviceManager::GetInstance()->GetDeviceRealAddr(randomAddr1, testAddr);
    EXPECT_EQ(ret, RET_SUCCESS);
    EXPECT_EQ(testAddr, realAddr1);

    HILOGI("bluetooth_device_manager_test_001 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_002
 * @tc.desc: BluetoothDeviceManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_002, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_002 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:55");
    RawAddress realAddr2 = RawAddress("11:11:22:33:44:55");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    RawAddress randomAddr1 = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr1);
    int ret = BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(realAddr1, true);
    EXPECT_EQ(ret, RET_SUCCESS);

    ret = BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(realAddr2, true);
    EXPECT_EQ(ret, RET_NO_EXIST);

    ret = BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(realAddr1, false);
    EXPECT_EQ(ret, RET_SUCCESS);

    ret = BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(realAddr2, false);
    EXPECT_EQ(ret, RET_NO_EXIST);
    HILOGI("bluetooth_device_manager_test_002 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_003
 * @tc.desc: BluetoothDeviceManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_003, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_003 enter");
    std::vector<RawAddress> rawAddrVec;
    std::vector<RawAddress> randomAddrVec;
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(rawAddrVec, randomAddrVec);
    HILOGI("bluetooth_device_manager_test_003 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_004
 * @tc.desc: BluetoothDeviceManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_004, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_004 enter");
    std::vector<RawAddress> rawAddrVec;
    std::vector<OHOS::Bluetooth::BluetoothRawAddress> randomAddrVec;
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(rawAddrVec, randomAddrVec);
    HILOGI("bluetooth_device_manager_test_004 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_005
 * @tc.desc: BluetoothDeviceManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_005, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_005 enter");
    std::vector<RawAddress> rawAddrVec;
    RawAddress realAddr1("00:11:22:33:44:55");
    RawAddress realAddr2("11:11:22:33:44:55");
    rawAddrVec.emplace_back(realAddr1);
    rawAddrVec.emplace_back(realAddr2);
    std::vector<RawAddress> randomAddrVec;

    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(rawAddrVec, randomAddrVec);
    HILOGI("bluetooth_device_manager_test_005 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_006
 * @tc.desc: BluetoothDeviceManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_006, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_006 enter");
    std::vector<RawAddress> rawAddrVec;
    RawAddress realAddr1("00:11:22:33:44:55");
    RawAddress realAddr2("11:11:22:33:44:55");
    rawAddrVec.emplace_back(realAddr1);
    rawAddrVec.emplace_back(realAddr2);

    std::vector<OHOS::Bluetooth::BluetoothRawAddress> randomAddrVec;
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(rawAddrVec, randomAddrVec);
    HILOGI("bluetooth_device_manager_test_006 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_007
 * @tc.desc: Check BluetoothDeviceManager timer started when call AddDeviceInfo.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_007, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_007 enter");

    auto *deviceManager = BluetoothDeviceManager::GetInstance();
    auto &timer = deviceManager->bluetoothDeviceManagerTimer;
    auto timeoutFunc = [deviceManager]() {
        deviceManager->ScheduleCleanDeviceInfo();
    };
    deviceManager->timerMs_ = 10; // 10ms
    timer = std::make_shared<utility::Timer>(timeoutFunc);

    EXPECT_FALSE(timer->IsStarted());
    RawAddress rawAddr("11:11:22:22:33:33");
    deviceManager->AddDeviceInfo(rawAddr);
    EXPECT_TRUE(timer->IsStarted());

    HILOGI("bluetooth_device_manager_test_007 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_008
 * @tc.desc: Check BluetoothDeviceManager timer started when call SetDeviceRetentionFlag.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_008, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_008 enter");

    auto *deviceManager = BluetoothDeviceManager::GetInstance();
    auto &timer = deviceManager->bluetoothDeviceManagerTimer;
    auto timeoutFunc = [deviceManager]() {
        deviceManager->ScheduleCleanDeviceInfo();
    };
    deviceManager->timerMs_ = 10; // 10ms
    timer = std::make_shared<utility::Timer>(timeoutFunc);

    EXPECT_FALSE(timer->IsStarted());

    RawAddress rawAddr("11:11:22:22:33:33");
    RawAddress realAddr("00:11:22:33:44:55");
    deviceManager->AddRetentionDeviceInfo(realAddr, rawAddr);
    RawAddress realAddr1("00:11:22:33:44:5A");
    RawAddress realAddr2("00:11:22:33:44:5a");
    deviceManager->AddRetentionDeviceInfo(realAddr1, rawAddr);
    deviceManager->AddRetentionDeviceInfo(realAddr2, rawAddr);
    EXPECT_FALSE(timer->IsStarted());

    deviceManager->SetDeviceRetentionFlag(realAddr, false);
    EXPECT_TRUE(timer->IsStarted());

    HILOGI("bluetooth_device_manager_test_008 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_009
 * @tc.desc: Check BluetoothDeviceManager timer stopped when call SetDeviceRetentionFlag.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_009, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_009 enter");

    auto *deviceManager = BluetoothDeviceManager::GetInstance();
    auto &timer = deviceManager->bluetoothDeviceManagerTimer;
    auto timeoutFunc = [deviceManager]() {
        deviceManager->ScheduleCleanDeviceInfo();
    };
    deviceManager->timerMs_ = 10; // 10ms
    timer = std::make_shared<utility::Timer>(timeoutFunc);

    EXPECT_FALSE(timer->IsStarted());

    RawAddress realAddr("00:11:22:33:44:55");
    deviceManager->AddDeviceInfo(realAddr);
    EXPECT_TRUE(timer->IsStarted());

    deviceManager->SetDeviceRetentionFlag(realAddr, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(timer->IsStarted());

    HILOGI("bluetooth_device_manager_test_009 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_010
 * @tc.desc: Check BluetoothDeviceManager GetRealUsedAddress function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_010, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_010 enter");

    RawAddress realAddr = RawAddress("00:11:FF:33:44:55");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    RawAddress randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr);

    BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(randomAddr);
    EXPECT_EQ(realAddr, BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(realAddr));

    HILOGI("bluetooth_device_manager_test_010 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_011
 * @tc.desc: Check BluetoothDeviceManager AddPersistRandomDeviceId function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_011, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_011 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    RawAddress realAddr = RawAddress("00:11:FF:33:44:55");
    manager->ClearDevicesInfo();

    EXPECT_EQ(manager->AddPersistRandomDeviceId(realAddr, 0), BT_ERR_INVALID_PARAM);
    RawAddress randomAddr = manager->AddDeviceInfo(realAddr);
    EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr, 0), BT_NO_ERROR);
    EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr, 0), BT_NO_ERROR);
    EXPECT_TRUE(manager->IsValidRandomDeviceId(randomAddr));
    std::vector<std::string> vec {};
    manager->GetPersistDeviceIds(0, vec);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], randomAddr.GetAddress());

    EXPECT_EQ(manager->DeletePersistRandomDeviceId(randomAddr, 0), BT_NO_ERROR);
    EXPECT_EQ(manager->DeletePersistRandomDeviceId(randomAddr, 0), BT_NO_ERROR);
    manager->GetPersistDeviceIds(0, vec);
    EXPECT_EQ(vec.size(), 0);
    HILOGI("bluetooth_device_manager_test_011 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_012
 * @tc.desc: Check BluetoothDeviceManager AddPersistRandomDeviceId max function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_012, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_012 enter");
    const uint32_t PERSISTENT_RANDOM_DEVICE_ID_MAX_COUNT = 100;
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    for (uint32_t i = 0; i < PERSISTENT_RANDOM_DEVICE_ID_MAX_COUNT; i++) {
        std::vector<uint8_t> vec {0x55, 0x44, 0x33, 0x22, 0x11};
        vec.push_back(static_cast<uint8_t>(i));
        RawAddress realAddr = RawAddress::ConvertToString(vec.data(), vec.size());
        RawAddress randomAddr = manager->AddDeviceInfo(realAddr);
        EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr, 0), BT_NO_ERROR);
    }

    std::vector<std::string> vec {};
    manager->GetPersistDeviceIds(0, vec);
    EXPECT_EQ(vec.size(), PERSISTENT_RANDOM_DEVICE_ID_MAX_COUNT);

    RawAddress randomAddr = manager->AddDeviceInfo(RawAddress("11:22:33:44:55:66"));
    EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr, 0), BT_ERR_MAX_RESOURCES);

    // 另一个应用能成功添加固化虚拟地址
    manager->GetPersistDeviceIds(1, vec);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr, 1), BT_NO_ERROR);
    HILOGI("bluetooth_device_manager_test_012 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_013
 * @tc.desc: Check BluetoothDeviceManager Clear retention flag which address is persistent.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_013, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_013 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    RawAddress realAddr = RawAddress("00:11:FF:33:44:55");
    manager->ClearDevicesInfo();

    EXPECT_EQ(manager->AddPersistRandomDeviceId(realAddr, 0), BT_ERR_INVALID_PARAM);
    EXPECT_EQ(manager->SetDeviceRetentionFlag(realAddr, false), RET_NO_EXIST);

    HILOGI("bluetooth_device_manager_test_013 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_014
 * @tc.desc: BluetoothDeviceManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_014, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_014 enter");
    std::vector<std::string> pairedDevices = OHOS::bluetooth::ClassicConfig::GetInstance().GetPairedAddrList();
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    if (!pairedDevices.empty()) {
        manager->AddPairedDevInfo();
        EXPECT_TRUE(!manager->btDevicesMap.empty());
    }
    HILOGI("bluetooth_device_manager_test_014 end");
}

#define CHECK_APP_PERSIST_DEVICE_ID_COUNT(tokenId, count) \
{ \
    std::vector<std::string> vec {}; \
    manager->GetPersistDeviceIds(tokenId, vec); \
    EXPECT_EQ(vec.size(), count); \
}

/**
 * @tc.name: bluetooth_device_manager_test_015
 * @tc.desc: 检查多应用固化不同的虚拟地址，互不干扰
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_015, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_015 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    RawAddress realAddr1 = RawAddress("00:11:FF:33:44:55");
    EXPECT_EQ(manager->AddPersistRandomDeviceId(realAddr1, 0), BT_ERR_INVALID_PARAM);
    RawAddress randomAddr1 = manager->AddDeviceInfo(realAddr1);
    EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr1, 0), BT_NO_ERROR);

    RawAddress realAddr2 = RawAddress("11:11:FF:33:44:55");
    EXPECT_EQ(manager->AddPersistRandomDeviceId(realAddr2, 1), BT_ERR_INVALID_PARAM);
    RawAddress randomAddr2 = manager->AddDeviceInfo(realAddr2);
    EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr2, 1), BT_NO_ERROR);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(0, 1);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(1, 1);

    EXPECT_EQ(manager->DeletePersistRandomDeviceId(randomAddr1, 0), BT_NO_ERROR);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(0, 0);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(1, 1);

    EXPECT_EQ(manager->DeletePersistRandomDeviceId(randomAddr2, 1), BT_NO_ERROR);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(0, 0);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(1, 0);
    HILOGI("bluetooth_device_manager_test_015 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_016
 * @tc.desc: 检查多应用固化相同的虚拟地址
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_016, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_016 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    RawAddress realAddr1 = RawAddress("00:11:FF:33:44:55");
    EXPECT_EQ(manager->AddPersistRandomDeviceId(realAddr1, 0), BT_ERR_INVALID_PARAM);
    RawAddress randomAddr1 = manager->AddDeviceInfo(realAddr1);
    EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr1, 0), BT_NO_ERROR);
    EXPECT_EQ(manager->AddPersistRandomDeviceId(randomAddr1, 1), BT_NO_ERROR);

    auto &deviceInfo = manager->btDevicesMap.find(realAddr1.GetAddress())->second;
    EXPECT_EQ(deviceInfo.appTokenIdSet.size(), 2);
    EXPECT_TRUE(deviceInfo.isRetention);
    EXPECT_TRUE(deviceInfo.isPersistRandomAddr);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(0, 1);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(1, 1);

    EXPECT_EQ(manager->DeletePersistRandomDeviceId(randomAddr1, 0), BT_NO_ERROR);
    EXPECT_TRUE(deviceInfo.isRetention);
    EXPECT_TRUE(deviceInfo.isPersistRandomAddr);
    EXPECT_EQ(deviceInfo.appTokenIdSet.size(), 1);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(0, 0);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(1, 1);

    EXPECT_EQ(manager->DeletePersistRandomDeviceId(randomAddr1, 1), BT_NO_ERROR);
    EXPECT_FALSE(deviceInfo.isRetention);
    EXPECT_FALSE(deviceInfo.isPersistRandomAddr);
    EXPECT_EQ(deviceInfo.appTokenIdSet.size(), 0);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(0, 0);
    CHECK_APP_PERSIST_DEVICE_ID_COUNT(1, 0);
    HILOGI("bluetooth_device_manager_test_016 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_017
 * @tc.desc: 检查从文件里获取到无效地址信息，默认不处理的的行为
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_017, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_017 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    std::vector<BluetoothDeviceManager::BluetoothDeviceInfo> vec;
    std::string key = "00"; // invalid_key
    std::string value = "11"; // invalid_value
    manager->GetPersistRandomDeviceIdFromFile(vec, key, value);
    EXPECT_TRUE(vec.empty());

    HILOGI("bluetooth_device_manager_test_017 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_018
 * @tc.desc: 检查从文件里获取到有效地址信息，添加到设备列表
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_018, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_018 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    std::vector<BluetoothDeviceManager::BluetoothDeviceInfo> vec;
    std::string key = "00:11:22:33:44:55"; // valid_key
    std::string value = "11:22:33:44:55:66"; // valid_value
    manager->GetPersistRandomDeviceIdFromFile(vec, key, value);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].realMacAddr, key);
    EXPECT_EQ(vec[0].randomMacAddr, value);

    HILOGI("bluetooth_device_manager_test_018 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_019
 * @tc.desc: 检查从文件里获取到重复地址信息，仅添加一条内容到设备列表
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_019, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_019 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    std::vector<BluetoothDeviceManager::BluetoothDeviceInfo> vec;
    std::string key = "00:11:22:33:44:55"; // valid_key
    std::string value = "11:22:33:44:55:66"; // valid_value
    BluetoothDeviceManager::BluetoothDeviceInfo deviceInfo(key, value, 0, true);
    vec.push_back(deviceInfo);

    manager->GetPersistRandomDeviceIdFromFile(vec, key, value);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].realMacAddr, key);
    EXPECT_EQ(vec[0].randomMacAddr, value);

    HILOGI("bluetooth_device_manager_test_019 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_020
 * @tc.desc: 检查从文件里获取到无效用户信息，默认不处理的的行为
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_020, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_020 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    std::vector<BluetoothDeviceManager::BluetoothDeviceInfo> vec;
    std::string key = "00"; // invalid_key
    std::vector<std::string> values = { "11" }; // invalid_value
    manager->GetPersistRandomDeviceIdAppInfoFromFile(vec, key, values);
    EXPECT_TRUE(vec.empty());

    key = "00_xx";
    manager->GetPersistRandomDeviceIdAppInfoFromFile(vec, key, values);
    EXPECT_TRUE(vec.empty());

    key = "00:11:22:33:44:55_AppIdentifiers";
    values = { "xxxxxx" };
    manager->GetPersistRandomDeviceIdAppInfoFromFile(vec, key, values);
    EXPECT_TRUE(vec[0].appTokenIdSet.empty());

    HILOGI("bluetooth_device_manager_test_020 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_021
 * @tc.desc: 检查从文件里获取到有效地址信息，添加到设备列表
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_021, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_021 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    std::vector<BluetoothDeviceManager::BluetoothDeviceInfo> vec;
    std::string key = "00:11:22:33:44:55_AppIdentifiers"; // valid_key
    std::vector<std::string> values = { "123456" }; // valid_value
    manager->GetPersistRandomDeviceIdAppInfoFromFile(vec, key, values);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].realMacAddr, "00:11:22:33:44:55");
    EXPECT_EQ(vec[0].randomMacAddr, "");
    EXPECT_TRUE(vec[0].appTokenIdSet.find(123456) != vec[0].appTokenIdSet.end());  // 123456 is test app toeknId

    HILOGI("bluetooth_device_manager_test_021 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_022
 * @tc.desc: 检查从文件里获取到重复地址信息，仅添加一条内容到设备列表
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_022, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_022 enter");
    BluetoothDeviceManager *manager = BluetoothDeviceManager::GetInstance();
    manager->ClearDevicesInfo();

    std::vector<BluetoothDeviceManager::BluetoothDeviceInfo> vec;
    std::string key = "00:11:22:33:44:55"; // valid_key
    std::string value = "11:22:33:44:55:66"; // valid_value
    BluetoothDeviceManager::BluetoothDeviceInfo deviceInfo(key, value, 0, true);
    vec.push_back(deviceInfo);

    std::string appKey = "00:11:22:33:44:55_AppIdentifiers"; // valid_key
    std::vector<std::string> appValues = { "111111", "222222" }; // valid_value
    manager->GetPersistRandomDeviceIdAppInfoFromFile(vec, appKey, appValues);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].realMacAddr, key);
    EXPECT_EQ(vec[0].randomMacAddr, value);
    EXPECT_TRUE(vec[0].appTokenIdSet.find(111111) != vec[0].appTokenIdSet.end());  // 111111 is test app toeknId
    EXPECT_TRUE(vec[0].appTokenIdSet.find(222222) != vec[0].appTokenIdSet.end());  // 222222 is test app toeknId

    HILOGI("bluetooth_device_manager_test_022 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_023
 * @tc.desc: Check BluetoothDeviceManager GetRealUsedAddress function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_023, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_023 enter");

    RawAddress randAddr = RawAddress("00:11:AF:FA:44:00");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    RawAddress realUsedAddr;
    bool ret = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(randAddr, realUsedAddr);
    EXPECT_TRUE(ret);  // the testsuit has IsUseRealAddr() permission, so return true here

    HILOGI("bluetooth_device_manager_test_023 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_024
 * @tc.desc: Check BluetoothDeviceManager GetRealUsedAddress function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_024, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_024 enter");

    RawAddress inputAddr = RawAddress("00:11:FF:33:44:55");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    RawAddress randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(inputAddr);
    RawAddress realUsedAddr;
    bool ret = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(randomAddr, realUsedAddr);
    EXPECT_TRUE(ret);
    EXPECT_EQ(inputAddr, realUsedAddr);

    HILOGI("bluetooth_device_manager_test_024 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_025
 * @tc.desc: Check BluetoothDeviceManager GetRealUsedAddress function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_025, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_025 enter");

    RawAddress inputAddr = RawAddress("00:11:FF:33:44:55");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    RawAddress randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(inputAddr);
    RawAddress realUsedAddr;
    bool ret = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(inputAddr, realUsedAddr);
    EXPECT_TRUE(ret);
    EXPECT_EQ(inputAddr, realUsedAddr);

    HILOGI("bluetooth_device_manager_test_025 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_026
 * @tc.desc: Check BluetoothDeviceManager IsRealAddrInBtDeviceMap function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_026, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_026 enter");

    RawAddress inputAddr = RawAddress("00:11:FF:33:44:55");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    RawAddress randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(inputAddr);
    bool ret = BluetoothDeviceManager::GetInstance()->IsRealAddrInBtDeviceMap(inputAddr);
    EXPECT_TRUE(ret);

    HILOGI("bluetooth_device_manager_test_026 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_027
 * @tc.desc: Check BluetoothDeviceManager IsRealAddrInBtDeviceMap function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_027, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_027 enter");

    RawAddress inputAddr = RawAddress("00:11:FF:33:44:55");
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    bool ret = BluetoothDeviceManager::GetInstance()->IsRealAddrInBtDeviceMap(inputAddr);
    EXPECT_FALSE(ret);

    HILOGI("bluetooth_device_manager_test_027 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_028
 * @tc.desc: Check BluetoothDeviceManager GetRealUsedAddrByAddrType function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_028, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_028 enter");

    RawAddress inputAddr = RawAddress("00:11:AF:FA:44:00");
    RawAddress realAddr;
    int32_t addrType = AddressType::UNSET_ADDRESS;
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    bool ret = BluetoothDeviceManager::GetInstance()->GetRealUsedAddrByAddrType(inputAddr, addrType, realAddr);
    EXPECT_TRUE(ret); // the testsuit has IsUseRealAddr() permission, so return true here.

    HILOGI("bluetooth_device_manager_test_028 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_029
 * @tc.desc: Check BluetoothDeviceManager GetRealUsedAddrByAddrType function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_029, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_029 enter");

    RawAddress inputAddr = RawAddress("00:11:FF:33:44:55");
    RawAddress realAddr;
    int32_t addrType = AddressType::VIRTUAL_ADDRESS;
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    bool ret = BluetoothDeviceManager::GetInstance()->GetRealUsedAddrByAddrType(inputAddr, addrType, realAddr);
    EXPECT_FALSE(ret);

    HILOGI("bluetooth_device_manager_test_029 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_030
 * @tc.desc: Check BluetoothDeviceManager GetRealUsedAddrByAddrType function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_030, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_030 enter");

    RawAddress inputAddr = RawAddress("00:11:FF:33:44:55");
    RawAddress realAddr;
    int32_t addrType = AddressType::REAL_ADDRESS;
    BluetoothDeviceManager::GetInstance()->ClearDevicesInfo();
    bool ret = BluetoothDeviceManager::GetInstance()->GetRealUsedAddrByAddrType(inputAddr, addrType, realAddr);
    EXPECT_TRUE(ret);
    EXPECT_EQ(inputAddr, realAddr);

    HILOGI("bluetooth_device_manager_test_030 end");
}

/**
 * @tc.name: bluetooth_device_manager_test_031
 * @tc.desc: Check BluetoothDeviceManager GetHashValue function.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceManagerTest, bluetooth_device_manager_test_031, TestSize.Level1)
{
    HILOGI("bluetooth_device_manager_test_031 enter");
    int hashAlgorithmType = 0;
    int unKnownHashType = -1;
    std::string virtualAddress = "11:22:33:44:55:66";
    std::string ret = BluetoothDeviceManager::GetInstance()->GetHashValue(hashAlgorithmType, virtualAddress);
    EXPECT_FALSE(ret.empty());
    ret = BluetoothDeviceManager::GetInstance()->GetHashValue(unKnownHashType, virtualAddress);
    EXPECT_TRUE(ret.empty());
    HILOGI("bluetooth_device_manager_test_031 end");
}
}  // namespace bluetooth
}  // namespace OHOS