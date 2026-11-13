/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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
#include "cloud_device_manager.h"

#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "bluetooth_log.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hw_interface.h"
#include "datetime_ex.h"
#include "preferences_manager.h"
#include "bluetooth.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;

class CloudDeviceManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
    }
    void SetUp();
    void TearDown();
    void AddCloudBluetoothDevTest();
    void DelCloudBluetoothDevTest();
    void AddCloudDevicePropertiesTest(std::shared_ptr<BluetoothDevice> remoteDevice, RawAddress device);
    void DelCloudDevicePropertiesTest(std::shared_ptr<BluetoothDevice> remoteDevice, RawAddress device);
    void DeleteCloudBtDeviceTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void SetDeviceClassPropTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void SetDeviceNamePropTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void SetCloudDevicePropTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void SetDeviceTypePropTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void SetDeviceUuidsPropTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void AddDeviceCloudPropTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void UpdateCloudDevicePropTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void SetAdvKeyTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void RemoveAdvKeyTest(std::shared_ptr<BluetoothDevice> remoteDevice);
    void BtAclTimerTest(RawAddress device);
    void SetDeviceProductIdPropTest(std::shared_ptr<BluetoothDevice> remoteDevice);
public:
    std::shared_ptr<CloudDeviceManager> cloudDeviceManager_;
};

void CloudDeviceManagerTest::SetUp()
{
    cloudDeviceManager_ = std::make_unique<CloudDeviceManager>();
}

void CloudDeviceManagerTest::TearDown()
{
    cloudDeviceManager_ = nullptr;
}

void CloudDeviceManagerTest::AddCloudBluetoothDevTest()
{
    HILOGI("cloud_device_manager_test_AddCloudBluetoothDev_001 enter");
    std::vector<Bluetooth::BluetoothTrustPairDevice> cloudList {};
    cloudDeviceManager_->AddCloudBluetoothDev(cloudList);
    Bluetooth::BluetoothTrustPairDevice element;
    cloudList.push_back(element);
    cloudDeviceManager_->AddCloudBluetoothDev(cloudList);
    element.SetMacAddress("11:22:33:44:55:66");
    cloudList.push_back(element);
    cloudDeviceManager_->AddCloudBluetoothDev(cloudList);
    std::vector<std::string> uuids{};
    uuids.push_back("0000110B-0000-1000-8000-00805F9B34FB");
    uuids.push_back("0000110D-0000-1000-8000-00805F9B34FB");
    element.SetUuid(uuids);
    cloudList.push_back(element);
    cloudDeviceManager_->AddCloudBluetoothDev(cloudList);
    cloudDeviceManager_->AddCloudBluetoothDev(cloudList);
    HILOGI("cloud_device_manager_test_AddCloudBluetoothDev_001 end");
}

void CloudDeviceManagerTest::DelCloudBluetoothDevTest()
{
    HILOGI("cloud_device_manager_test_DelCloudBluetoothDev_001 enter");
    std::vector<Bluetooth::BluetoothTrustPairDevice> cloudList {};
    Bluetooth::BluetoothTrustPairDevice element;
    element.SetMacAddress("11:22:33:44:55:66");
    cloudList.push_back(element);
    std::vector<std::string> uuids{};
    uuids.push_back("0000110B-0000-1000-8000-00805F9B34FB");
    uuids.push_back("0000110D-0000-1000-8000-00805F9B34FB");
    element.SetUuid(uuids);
    cloudDeviceManager_->DelCloudBluetoothDev(cloudList);
    HILOGI("cloud_device_manager_test_DelCloudBluetoothDev_001 end");
}

void CloudDeviceManagerTest::AddCloudDevicePropertiesTest(std::shared_ptr<BluetoothDevice> remoteDevice, RawAddress device)
{
    HILOGI("AddCloudDevicePropertiesTest enter");
    cloudDeviceManager_->AddCloudDeviceProperties(nullptr);
    cloudDeviceManager_->AddCloudDeviceProperties(remoteDevice);
    cloudDeviceManager_->AddCloudRemoteDevice(device);
    cloudDeviceManager_->AddCloudDeviceProperties(remoteDevice);
    HILOGI("AddCloudDevicePropertiesTest end");
}

void CloudDeviceManagerTest::DelCloudDevicePropertiesTest(std::shared_ptr<BluetoothDevice> remoteDevice, RawAddress device)
{
    HILOGI("DelCloudDevicePropertiesTest enter");
    cloudDeviceManager_->DelCloudDeviceProperties(nullptr);
    cloudDeviceManager_->DelCloudDeviceProperties(remoteDevice);
    cloudDeviceManager_->DelCloudDeviceProperties(remoteDevice);
    cloudDeviceManager_->AddCloudRemoteDevice(device);
    cloudDeviceManager_->DelCloudDeviceProperties(remoteDevice);
    HILOGI("DelCloudDevicePropertiesTest end");
}

void CloudDeviceManagerTest::DeleteCloudBtDeviceTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("cloud_device_manager_test_DeleteCloudBtDevice_001 enter");
    std::string address = "11:22:33:44:55:66";
    cloudDeviceManager_->DeleteCloudBtDevice(nullptr, address);
    std::string address1;
    cloudDeviceManager_->DeleteCloudBtDevice(remoteDevice, address1);
    cloudDeviceManager_->DeleteCloudBtDevice(remoteDevice, address);
    address1 = "22:22:33:44:55:66";
    cloudDeviceManager_->DeleteCloudBtDevice(remoteDevice, address1);
    if (remoteDevice != nullptr) {
        remoteDevice->SetPairedStatus(PAIR_PAIRING);
        cloudDeviceManager_->DeleteCloudBtDevice(remoteDevice, address1);
    }
    HILOGI("cloud_device_manager_test_DeleteCloudBtDevice_001 end");
}

void CloudDeviceManagerTest::SetDeviceClassPropTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("SetDeviceClassPropTest enter");
    int32_t deviceClass = 230644;
    cloudDeviceManager_->SetDeviceClassProp(nullptr, deviceClass);
    cloudDeviceManager_->SetDeviceClassProp(remoteDevice, deviceClass);
    HILOGI("SetDeviceClassPropTest end");
}

void CloudDeviceManagerTest::SetDeviceNamePropTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("SetDeviceNamePropTest enter");
    std::string deviceName;
    cloudDeviceManager_->SetDeviceNameProp(nullptr, deviceName);
    cloudDeviceManager_->SetDeviceNameProp(remoteDevice, deviceName);
    deviceName = "deviceNameTest";
    cloudDeviceManager_->SetDeviceNameProp(nullptr, deviceName);
    cloudDeviceManager_->SetDeviceNameProp(remoteDevice, deviceName);
    HILOGI("SetDeviceNamePropTest end");
}

void CloudDeviceManagerTest::SetCloudDevicePropTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("SetCloudDevicePropTest enter");
    bool isCloudDev = false;
    cloudDeviceManager_->SetCloudDeviceProp(nullptr, isCloudDev);
    cloudDeviceManager_->SetCloudDeviceProp(remoteDevice, isCloudDev);
    isCloudDev = true;
    cloudDeviceManager_->SetCloudDeviceProp(remoteDevice, isCloudDev);
    HILOGI("SetCloudDevicePropTest end");
}

void CloudDeviceManagerTest::SetDeviceTypePropTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("SetDeviceTypePropTest enter");
    int32_t devType = 3;
    cloudDeviceManager_->SetDeviceTypeProp(nullptr, devType);
    cloudDeviceManager_->SetDeviceTypeProp(remoteDevice, devType);
    HILOGI("SetDeviceTypePropTest end");
}

void CloudDeviceManagerTest::SetDeviceUuidsPropTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("SetDeviceUuidsPropTest enter");
    cloudDeviceManager_->SendCloudUuidIntent(nullptr);
    cloudDeviceManager_->SendCloudUuidIntent(remoteDevice);
    std::vector<Uuid> uuids {};
    cloudDeviceManager_->SetDeviceUuidsProp(nullptr, uuids);
    Uuid uuid_1 = Uuid::ConvertFromString("0000110B-0000-1000-8000-00805F9B34FB");
    Uuid uuid_2 = Uuid::ConvertFromString("0000110D-0000-1000-8000-00805F9B34FB");
    uuids.push_back(uuid_1);
    uuids.push_back(uuid_2);
    remoteDevice->SetDeviceUuids(uuids);
    cloudDeviceManager_->SendCloudUuidIntent(remoteDevice);
    cloudDeviceManager_->SetDeviceUuidsProp(remoteDevice, uuids);
    HILOGI("SetDeviceUuidsPropTest end");
}

void CloudDeviceManagerTest::AddDeviceCloudPropTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("AddDeviceCloudPropTest enter");
    Bluetooth::BluetoothTrustPairDevice element;
    cloudDeviceManager_->AddDeviceCloudProp(nullptr, element);
    cloudDeviceManager_->AddDeviceCloudProp(remoteDevice, element);
    std::vector<uint8_t> token {};
    token.push_back(0);
    element.SetToken(token);
    cloudDeviceManager_->AddDeviceCloudProp(remoteDevice, element);
    HILOGI("AddDeviceCloudPropTest end");
}

void CloudDeviceManagerTest::UpdateCloudDevicePropTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("UpdateCloudDevicePropTest enter");
    Bluetooth::BluetoothTrustPairDevice element;
    cloudDeviceManager_->UpdateCloudDeviceProp(nullptr, element);
    cloudDeviceManager_->UpdateCloudDeviceProp(remoteDevice, element);
    std::vector<uint8_t> token {};
    token.push_back(1);
    remoteDevice->SetToken(token);
    remoteDevice->SetCloudBondState(CLOUD_BOND_NONE);
    cloudDeviceManager_->UpdateCloudDeviceProp(remoteDevice, element);
    remoteDevice->SetCloudBondState(CLOUD_BOND_BONDING);
    cloudDeviceManager_->UpdateCloudDeviceProp(remoteDevice, element);
    remoteDevice->SetCloudBondState(CLOUD_BOND_CREATE_BOND);
    cloudDeviceManager_->UpdateCloudDeviceProp(remoteDevice, element);
    element.SetToken(token);
    cloudDeviceManager_->UpdateCloudDeviceProp(remoteDevice, element);
    token.push_back(2);
    element.SetToken(token);
    cloudDeviceManager_->UpdateCloudDeviceProp(remoteDevice, element);
    HILOGI("UpdateCloudDevicePropTest end");
}

void CloudDeviceManagerTest::SetAdvKeyTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("SetAdvKeyTest enter");
    std::vector<uint8_t> advInfo {};
    cloudDeviceManager_->SetAdvKey(advInfo, nullptr);
    cloudDeviceManager_->SetAdvKey(advInfo, remoteDevice);
    HILOGI("SetAdvKeyTest end");
}

void CloudDeviceManagerTest::RemoveAdvKeyTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("RemoveAdvKeyTest enter");
    cloudDeviceManager_->RemoveAdvKey(nullptr);
    cloudDeviceManager_->RemoveAdvKey(remoteDevice);
    HILOGI("RemoveAdvKeyTest end");
}

void CloudDeviceManagerTest::BtAclTimerTest(RawAddress device)
{
    HILOGI("BtAclTimerTest enter");
    cloudDeviceManager_->StartBtAclTimer(device);
    cloudDeviceManager_->StopBtAclTimer(device);
    cloudDeviceManager_->HandleCloudPairAclConnectTimeOut(device);
    HILOGI("BtAclTimerTest end");
}

void CloudDeviceManagerTest::SetDeviceProductIdPropTest(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    HILOGI("SetDeviceProductIdPropTest enter");
    cloudDeviceManager_->SetDeviceProductIdProp(nullptr, "00A1FF_FF_0001");
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "");
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "00A1FF");
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "123__456");
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "00A1FF_FF_GG");
    EXPECT_NE(remoteDevice->GetIcon(), 0x0001);
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "00A1FF_FF_0001");
    EXPECT_EQ(remoteDevice->GetIcon(), 0x0001);
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "FFFFFF_FF_FFFF");
    EXPECT_EQ(remoteDevice->GetIcon(), 0x0001);
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "FFFFFF_FF_0002");
    EXPECT_EQ(remoteDevice->GetIcon(), 0x0002);
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "00A1FF_FF_0003");
    EXPECT_EQ(remoteDevice->GetIcon(), 0x0003);
    cloudDeviceManager_->SetDeviceProductIdProp(remoteDevice, "00A1FF_FF_100000000");
    EXPECT_EQ(remoteDevice->GetIcon(), 0x0003);
    HILOGI("SetDeviceProductIdPropTest end");
}

/*
 * @tc.number: cloud_device_manager_test_001
 * @tc.name: cloud_device_manager_test_001
 * @tc.desc:
 */
HWTEST_F(CloudDeviceManagerTest, cloud_device_manager_test_001, TestSize.Level1)
{
    HILOGI("cloud_device_manager_test_001 enter");
    AddCloudBluetoothDevTest();
    EXPECT_FALSE(cloudDeviceManager_->cloudDevicesMap_.IsEmpty());
    DelCloudBluetoothDevTest();
    HILOGI("cloud_device_manager_test_001 end");
}

/*
 * @tc.number: cloud_device_manager_test_002
 * @tc.name: cloud_device_manager_test_002
 * @tc.desc:
 */
HWTEST_F(CloudDeviceManagerTest, cloud_device_manager_test_002, TestSize.Level1)
{
    HILOGI("cloud_device_manager_test_002 enter");
    std::string addr("11:22:33:44:55:66");
    RawAddress device(addr);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    AddCloudDevicePropertiesTest(remoteDevice, device);
    cloudDeviceManager_->AddCloudRemoteDevice(device);
    EXPECT_FALSE(cloudDeviceManager_->cloudDevicesMap_.IsEmpty());
    auto dev = cloudDeviceManager_->GetCloudBluetoothDeviceFromMap(device);
    EXPECT_TRUE(dev != nullptr);
    DelCloudDevicePropertiesTest(remoteDevice, device);
    DeleteCloudBtDeviceTest(remoteDevice);
    EXPECT_FALSE(cloudDeviceManager_->cloudDevicesMap_.Find(addr, remoteDevice));
    HILOGI("cloud_device_manager_test_002 end");
}

/*
 * @tc.number: cloud_device_manager_test_003
 * @tc.name: cloud_device_manager_test_003
 * @tc.desc:
 */
HWTEST_F(CloudDeviceManagerTest, cloud_device_manager_test_003, TestSize.Level1)
{
    HILOGI("cloud_device_manager_test_003 enter");
    RawAddress device("11:22:33:44:55:66");
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    SetDeviceClassPropTest(remoteDevice);
    SetDeviceNamePropTest(remoteDevice);
    SetDeviceUuidsPropTest(remoteDevice);
    SetCloudDevicePropTest(remoteDevice);
    SetDeviceTypePropTest(remoteDevice);
    AddDeviceCloudPropTest(remoteDevice);
    UpdateCloudDevicePropTest(remoteDevice);
    EXPECT_TRUE(remoteDevice->IsCloudDevice());
    cloudDeviceManager_->SendCloudDevState(device);
    HILOGI("cloud_device_manager_test_003 end");
}

/*
 * @tc.number:cloud_device_manager_test_004
 * @tc.name: cloud_device_manager_test_004
 * @tc.desc:
 */
HWTEST_F(CloudDeviceManagerTest, cloud_device_manager_test_004, TestSize.Level1)
{
    HILOGI("cloud_device_manager_test_004 enter");
    EXPECT_FALSE(cloudDeviceManager_->IsDeviceHasLinkkey(nullptr));
    RawAddress device("11:22:33:44:55:66");
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    cloudDeviceManager_->IsDeviceHasLinkkey(remoteDevice);
    SetAdvKeyTest(remoteDevice);
    RemoveAdvKeyTest(remoteDevice);
    BtAclTimerTest(device);
    HILOGI("cloud_device_manager_test_004 end");
}

/*
 * @tc.number:cloud_device_manager_test_005
 * @tc.name: IsValidAddress
 * @tc.desc:
 */
HWTEST_F(CloudDeviceManagerTest, cloud_device_manager_test_005, TestSize.Level1)
{
    HILOGI("cloud_device_manager_test_005 enter");
    std::string addr;
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress(addr));
    addr = "11:22:33";
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress(addr));
    addr = "11:22:33:44:55:66";
    EXPECT_TRUE(cloudDeviceManager_->IsValidAddress(addr));
    HILOGI("cloud_device_manager_test_005 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_IsAdvIrkInvalid_001, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_IsAdvIrkInvalid_001 enter");
    std::vector<uint8_t> advInfo1(1, 0);
    EXPECT_TRUE(IsAdvIrkInvalid(advInfo1));
    std::vector<uint8_t> advInfo2(ADV_INFO_BYTE_ARRY_LEN_49, 0);
    EXPECT_TRUE(IsAdvIrkInvalid(advInfo2));
    std::vector<uint8_t> advInfo3(ADV_INFO_BYTE_ARRY_LEN_49, 1);
    EXPECT_FALSE(IsAdvIrkInvalid(advInfo3));

    HILOGI("CloudDeviceManagerTest_UnitTest_IsAdvIrkInvalid_001 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_IsAdvIrkChanged_001, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_IsAdvIrkChanged_001 enter");
    std::vector<uint8_t> advInfo1(1, 0);
    std::vector<uint8_t> advInfo2(ADV_INFO_BYTE_ARRY_LEN_49, 0);
    std::vector<uint8_t> advInfo3(ADV_INFO_BYTE_ARRY_LEN_49, 1);
    std::vector<uint8_t> irk(ADV_INFO_BYTE_ARRY_LEN_16, 1);
    EXPECT_FALSE(IsAdvIrkChanged(advInfo1, irk));
    EXPECT_TRUE(IsAdvIrkChanged(advInfo2, irk));
    EXPECT_FALSE(IsAdvIrkChanged(advInfo3, irk));

    HILOGI("CloudDeviceManagerTest_UnitTest_IsAdvIrkChanged_001 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_IsUuidChanged_001, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_IsUuidChanged_001 enter");
    std::vector<Uuid> uuids {};
    EXPECT_FALSE(IsUuidChanged(uuids, uuids));
    std::vector<Uuid> uuids2 {};
    Uuid uuid_1 = Uuid::ConvertFromString("0000110B-0000-1000-8000-00805F9B34FB");
    uuids2.push_back(uuid_1);
    EXPECT_TRUE(IsUuidChanged(uuids, uuids2));
    EXPECT_TRUE(IsUuidChanged(uuids2, uuids));

    HILOGI("CloudDeviceManagerTest_UnitTest_IsUuidChanged_001 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_001, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_001 enter");
    std::vector<uint8_t> token;
    std::string tokenStr = "0a03f936741181f04d5f538c2a5a6a6af33f5c32cea6e04d07882366cb51e8fe";
    std::string anonymizedTokenStr = "0a0**********************************************************8fe";
    for (size_t i = 0; i < tokenStr.size(); i += 2) {
        std::string byte_str = tokenStr.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
        token.push_back(byte);
    }

    auto logStr = CloudTokenToLogString(token);
    EXPECT_EQ(logStr, anonymizedTokenStr);

    HILOGI("CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_001 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_002, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_002 enter");
    std::vector<uint8_t> token;
    std::string tokenStr = "0a03f9";
    std::string anonymizedTokenStr = "******";
    for (size_t i = 0; i < tokenStr.size(); i += 2) {
        std::string byte_str = tokenStr.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
        token.push_back(byte);
    }

    auto logStr = CloudTokenToLogString(token);
    EXPECT_EQ(logStr, anonymizedTokenStr);

    HILOGI("CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_002 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_003, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_003 enter");
    std::vector<uint8_t> token {};
    std::string anonymizedTokenStr = "";

    auto logStr = CloudTokenToLogString(token);
    EXPECT_EQ(logStr, anonymizedTokenStr);

    HILOGI("CloudDeviceManagerTest_UnitTest_CloudTokenToLogString_003 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_IsAnyDeviceInCloudPairing_001, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_IsAnyDeviceInCloudPairing_001 enter");
    EXPECT_FALSE(cloudDeviceManager_->IsAnyDeviceInCloudPairing());

    RawAddress device("11:22:33:44:55:66");
    auto classicProperties = AdapterProperties::GetInstance();
    auto remoteDeviceProperties = RemoteDeviceProperties::GetInstance();
    if (classicProperties != nullptr && remoteDeviceProperties != nullptr) {
        classicProperties->AddPairedAddrList(device);
        auto bluetoothDevice = remoteDeviceProperties->FindRemoteDevice(device);

        bluetoothDevice->SetPairedStatus(PAIR_PAIRED);
        EXPECT_FALSE(cloudDeviceManager_->IsAnyDeviceInCloudPairing());
        remoteDeviceProperties->SetCloudBondState(device, CLOUD_BOND_BONDING);
        EXPECT_TRUE(cloudDeviceManager_->IsAnyDeviceInCloudPairing());
        bluetoothDevice->SetPairedStatus(PAIR_PAIRING);
        remoteDeviceProperties->SetCloudBondState(device, CLOUD_BOND_CREATE_BOND);
        EXPECT_TRUE(cloudDeviceManager_->IsAnyDeviceInCloudPairing());
        remoteDeviceProperties->SetCloudBondState(device, CLOUD_BOND_BONDED);
        EXPECT_TRUE(cloudDeviceManager_->IsAnyDeviceInCloudPairing());
        remoteDeviceProperties->SetCloudBondState(device, CLOUD_BOND_NONE);
        EXPECT_FALSE(cloudDeviceManager_->IsAnyDeviceInCloudPairing());
    }

    HILOGI("CloudDeviceManagerTest_UnitTest_IsAnyDeviceInCloudPairing_001 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_IsAnyDeviceInCloudPairing_002, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_IsAnyDeviceInCloudPairing_002 enter");
    RawAddress device("00:22:33:44:55:66");
    auto classicProperties = AdapterProperties::GetInstance();
    auto remoteDeviceProperties = RemoteDeviceProperties::GetInstance();
    if (classicProperties != nullptr && remoteDeviceProperties != nullptr) {
        // GetBluetoothDeviceFromMap返回空指针场景分支
        classicProperties->AddPairedAddrList(device);
        remoteDeviceProperties->SetCloudBondState(device, CLOUD_BOND_BONDING);
        EXPECT_FALSE(cloudDeviceManager_->IsAnyDeviceInCloudPairing());
    }
    HILOGI("CloudDeviceManagerTest_UnitTest_IsAnyDeviceInCloudPairing_002 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_IsValidAddress_001, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_IsValidAddress_001 enter");
    EXPECT_TRUE(cloudDeviceManager_->IsValidAddress("08:f4:58:65:91:09"));
    EXPECT_TRUE(cloudDeviceManager_->IsValidAddress("08:F4:58:65:91:09"));
    EXPECT_TRUE(cloudDeviceManager_->IsValidAddress("AA:BB:CC:DD:EE:FF"));
    EXPECT_TRUE(cloudDeviceManager_->IsValidAddress("00:1a:2b:3c:4d:5e"));
    HILOGI("CloudDeviceManagerTest_UnitTest_IsValidAddress_001 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_IsValidAddress_002, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_IsValidAddress_002 enter");
    std::string addr;
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress(addr));
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress("08:F4:58"));
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress("08:F4:58:65:91:09:AA"));
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress("08F458659109"));
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress("08-F4-58-65-91-09"));
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress("0!:F4:58:65:91:09"));
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress("08::F4:58:65:91:09"));
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress(" 08:F4:58:65:91:09"));
    EXPECT_FALSE(cloudDeviceManager_->IsValidAddress("08:F4:58:65:91:09 "));
    HILOGI("CloudDeviceManagerTest_UnitTest_IsValidAddress_002 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_SetDeviceProductIdProp_001, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_SetDeviceProductIdProp_001 enter");
    RawAddress device("11:22:33:44:55:66");
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    SetCloudDevicePropTest(remoteDevice);
    SetDeviceProductIdPropTest(remoteDevice);
    HILOGI("CloudDeviceManagerTest_UnitTest_SetDeviceProductIdProp_001 end");
}

HWTEST_F(CloudDeviceManagerTest, CloudDeviceManagerTest_UnitTest_DeleteCloudBtDevice_Pairing, TestSize.Level1)
{
    HILOGI("CloudDeviceManagerTest_UnitTest_DeleteCloudBtDevice_Pairing enter");
    std::string addr("11:22:33:44:55:66");
    RawAddress device(addr);
    auto remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    ASSERT_TRUE(remoteDevice != nullptr);
    cloudDeviceManager_->AddCloudRemoteDevice(device);
    auto cloudDevice = cloudDeviceManager_->GetCloudBluetoothDeviceFromMap(device);
    ASSERT_TRUE(cloudDevice != nullptr);
    EXPECT_NE(remoteDevice.get(), cloudDevice.get());

    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    EXPECT_EQ(cloudDevice->GetPairedStatus(), PAIR_NONE);
    EXPECT_EQ(remoteDevice->GetPairedStatus(), PAIR_PAIRING);

    cloudDeviceManager_->DeleteCloudBtDevice(cloudDevice, addr);
    EXPECT_EQ(remoteDevice->GetPairedStatus(), PAIR_PAIRING);
    HILOGI("CloudDeviceManagerTest_UnitTest_DeleteCloudBtDevice_Pairing end");
}

}  // namespace bluetooth
}  // namespace OHOS
