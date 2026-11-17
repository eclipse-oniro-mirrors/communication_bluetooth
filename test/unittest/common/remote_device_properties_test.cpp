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

#include <sstream>
#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>
#include "remote_device_properties.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_hw_interface.h"
#include "bluetooth_workspace_manager.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
RemoteDeviceProperties remoteDeviceProperties;

static std::string ToHexString(int value)
{
    std::ostringstream oss;
    oss << std::hex << value;
    return oss.str();
}

class RemoteDevicePropertiesTest : public testing::Test {
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
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: remote_device_properties_test_001
 * @tc.name: FreeMemory
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_001, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_001 enter");
    remoteDeviceProperties.FreeMemory();
    EXPECT_TRUE(remoteDeviceProperties.IsRemoteDeviceMapEmpty());
    HILOGI("remote_device_properties_test_001 end");
}

/*
 * @tc.number: remote_device_properties_test_002
 * @tc.name: FindRemoteDevice
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_002, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_002 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDeviceProperties.remoteDeviceQueue.size(), 1);
    HILOGI("remote_device_properties_test_002 end");
}

/*
 * @tc.number: remote_device_properties_test_003
 * @tc.name: GetDeviceName
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_003, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_003 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_EQ(remoteDeviceProperties.GetDeviceName(device), "");
    HILOGI("remote_device_properties_test_003 end");
}

/*
 * @tc.number: remote_device_properties_test_004
 * @tc.name: GetDeviceBatteryInfo
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_004, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_004 enter");
    remoteDeviceProperties.FreeMemory();
    BatteryDetailInfo batteryInfo;
    EXPECT_FALSE(remoteDeviceProperties.GetDeviceBatteryInfo("00:11:22:33:44:55", batteryInfo));

    RawAddress device = RawAddress("00:11:22:33:44:55");
    remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_TRUE(remoteDeviceProperties.GetDeviceBatteryInfo("00:11:22:33:44:55", batteryInfo));
    HILOGI("remote_device_properties_test_004 end");
}

/*
 * @tc.number: remote_device_properties_test_005
 * @tc.name: SetDeviceBatteryInfo
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_005, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_005 enter");
    remoteDeviceProperties.FreeMemory();
    BatteryDetailInfo batteryInfo;
    remoteDeviceProperties.SetDeviceBatteryInfo("00:11:22:33:44:55", batteryInfo);
    EXPECT_FALSE(remoteDeviceProperties.GetDeviceBatteryInfo("00:11:22:33:44:55", batteryInfo));
    remoteDeviceProperties.ClearDeviceBatteryInfo("00:11:22:33:44:55");

    RawAddress device = RawAddress("00:11:22:33:44:55");
    remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.SetDeviceBatteryInfo("00:11:22:33:44:55", batteryInfo);
    EXPECT_TRUE(remoteDeviceProperties.GetDeviceBatteryInfo("00:11:22:33:44:55", batteryInfo));
    remoteDeviceProperties.ClearDeviceBatteryInfo("00:11:22:33:44:55");
    HILOGI("remote_device_properties_test_005 end");
}

/*
 * @tc.number: remote_device_properties_test_006
 * @tc.name: GetDeviceUuids
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_006, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_006 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_TRUE(remoteDeviceProperties.GetDeviceUuids(device).empty());
    HILOGI("remote_device_properties_test_006 end");
}

/*
 * @tc.number: remote_device_properties_test_007
 * @tc.name: IsBondedFromLocal
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_007, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_007 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_FALSE(remoteDeviceProperties.IsBondedFromLocal(device));
    HILOGI("remote_device_properties_test_007 end");
}

/*
 * @tc.number: remote_device_properties_test_008
 * @tc.name: GetBtPairedDevices
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_008, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_008 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDeviceProperties.GetBtPairedDevices().size(), 0);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    EXPECT_EQ(remoteDeviceProperties.GetBtPairedDevices().size(), 1);
    remoteDevice->SetPairedStatus(PAIR_NONE);
    HILOGI("remote_device_properties_test_008 end");
}

/*
 * @tc.number: remote_device_properties_test_009
 * @tc.name: GetDeviceUuids
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_009, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_009 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    Uuid uuid = Uuid::ConvertFromString("0000110B-0000-1000-8000-00805F9B34FB");
    std::vector<Uuid> uuidData {};
    uuidData.push_back(uuid);
    remoteDevice->SetDeviceUuids(uuidData);
    EXPECT_EQ(remoteDeviceProperties.GetDeviceUuids(device).size(), 1);
    remoteDevice->SetPairedStatus(PAIR_NONE);
    EXPECT_EQ(remoteDeviceProperties.GetDeviceUuids(device).size(), 0);
    HILOGI("remote_device_properties_test_009 end");
}

/*
 * @tc.number: remote_device_properties_test_010
 * @tc.name: IsBondedFromLocal
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_010, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_010 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_TRUE(remoteDeviceProperties.GetBluetoothDeviceFromMap(device) != nullptr);
    HILOGI("remote_device_properties_test_010 end");
}

/*
 * @tc.number: remote_device_properties_test_011
 * @tc.name: CancelPairing
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_011, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_011 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_FALSE(remoteDeviceProperties.CancelPairing(device));
    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    EXPECT_FALSE(remoteDeviceProperties.CancelPairing(device));
    remoteDevice->SetPairedStatus(PAIR_NONE);
    HILOGI("remote_device_properties_test_011 end");
}

/*
 * @tc.number: remote_device_properties_test_012
 * @tc.name: CancelPairing
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_012, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_012 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_FALSE(remoteDeviceProperties.RemovePair(device));
    HILOGI("remote_device_properties_test_012 end");
}

/*
 * @tc.number: remote_device_properties_test_013
 * @tc.name: DeleteLinkKey
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_013, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_013 enter");
    std::shared_ptr<BluetoothDevice> remoteDevice = std::make_shared<BluetoothDevice>();
    remoteDeviceProperties.DeleteLinkKey(remoteDevice);
    EXPECT_EQ(remoteDevice->linkKeyType_, PAIR_INVALID_LINK_KEY_TYPE);
    HILOGI("remote_device_properties_test_013 end");
}

/*
 * @tc.number: remote_device_properties_test_014
 * @tc.name: removeAllDevicesFromMap
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_014, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_014 enter");
    EXPECT_TRUE(remoteDeviceProperties.removeAllDevicesFromMap().empty());
    HILOGI("remote_device_properties_test_014 end");
}

/*
 * @tc.number: remote_device_properties_test_015
 * @tc.name: GetPairState
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_015, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_015 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_EQ(remoteDeviceProperties.GetPairState(device), PAIR_NONE);
    HILOGI("remote_device_properties_test_015 end");
}

/*
 * @tc.number: remote_device_properties_test_016
 * @tc.name: SetDevicePairingConfirmation
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_016, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_016 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    bool accept = true;
    EXPECT_FALSE(remoteDeviceProperties.SetDevicePairingConfirmation(device, accept));
    HILOGI("remote_device_properties_test_016 end");
}

/*
 * @tc.number: remote_device_properties_test_017
 * @tc.name: GetDeviceType
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_017, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_017 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_NE(remoteDeviceProperties.GetDeviceType(device), INVALID_TYPE);
    HILOGI("remote_device_properties_test_017 end");
}

/*
 * @tc.number: remote_device_properties_test_018
 * @tc.name: IsBrDeviceType
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_018, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_018 enter");
    std::string address = "00:11:22:33:44:55";
    EXPECT_TRUE(remoteDeviceProperties.IsBrDeviceType(address));
    HILOGI("remote_device_properties_test_018 end");
}

/*
 * @tc.number: remote_device_properties_test_019
 * @tc.name: GetDeviceClass
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_019, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_019 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_EQ(remoteDeviceProperties.GetDeviceClass(device), INVALID_VALUE);
    HILOGI("remote_device_properties_test_019 end");
}

/*
 * @tc.number: remote_device_properties_test_020
 * @tc.name: GetAliasName
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_020, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_020 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    std::string name = "00:11:22:33:44:55";
    remoteDeviceProperties.SetAliasName(device, name);
    EXPECT_EQ(remoteDeviceProperties.GetAliasName(device), "");
    HILOGI("remote_device_properties_test_020 end");
}

/*
 * @tc.number: remote_device_properties_test_030
 * @tc.name: SetDevicePin
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_030, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_030 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    std::string pinCode = "123456";
    EXPECT_FALSE(remoteDeviceProperties.SetDevicePin(device, pinCode));
    pinCode = "1234567891012345678";
    EXPECT_FALSE(remoteDeviceProperties.SetDevicePin(device, pinCode));
    HILOGI("remote_device_properties_test_030 end");
}

/*
 * @tc.number: remote_device_properties_test_031
 * @tc.name: FindRemoteDevice
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_031, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_031 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    RawAddress device_ = RawAddress("01:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDeviceProperties.remoteDeviceQueue.size(), 1);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    std::shared_ptr<BluetoothDevice> remoteDevice_ = remoteDeviceProperties.FindRemoteDevice(device_);
    remoteDevice_->SetPairedStatus(PAIR_PAIRING);
    EXPECT_EQ(remoteDeviceProperties.remoteDeviceQueue.size(), 2);
    std::string address = "00:00:00:00:0";
    const int begin = 0;
    const int mid = 10;
    const int end = 98;
    for (int i = begin; i <= end; ++i) {
        std::string address0;
        if (i < mid) {
            address0 = address + "0:0" + std::to_string(i);
        } else {
            address0 = address + "0:" + std::to_string(i);
        }
        RawAddress device0 = RawAddress(address0);
        remoteDeviceProperties.FindRemoteDevice(device0);
        EXPECT_EQ(remoteDeviceProperties.remoteDeviceQueue.size(), i * 2 + 3);
        std::string address1 = address + "1:0" + std::to_string(i);
        RawAddress device1 = RawAddress(address1);
        remoteDeviceProperties.FindRemoteDevice(device1);
        EXPECT_EQ(remoteDeviceProperties.remoteDeviceQueue.size(), i * 2 + 4);
    }
    RawAddress device3 = RawAddress("03:11:22:33:44:66");
    remoteDeviceProperties.FindRemoteDevice(device3);
    EXPECT_TRUE(remoteDeviceProperties.remoteDevicesMap_.find("00:11:22:33:44:55")
        != remoteDeviceProperties.remoteDevicesMap_.end());
    EXPECT_EQ(remoteDeviceProperties.remoteDeviceQueue.size(), MAX_DEVICE_QUEUE_SIZE);
    RawAddress device4 = RawAddress("04:11:22:33:44:66");
    remoteDeviceProperties.FindRemoteDevice(device4);
    RawAddress device5 = RawAddress("05:11:22:33:44:66");
    remoteDeviceProperties.FindRemoteDevice(device5);
    EXPECT_TRUE(remoteDeviceProperties.remoteDevicesMap_.find("01:11:22:33:44:55")
        != remoteDeviceProperties.remoteDevicesMap_.end());
    EXPECT_FALSE(remoteDeviceProperties.remoteDevicesMap_.find("00:00:00:00:00:00")
        != remoteDeviceProperties.remoteDevicesMap_.end());
    EXPECT_EQ(remoteDeviceProperties.remoteDeviceQueue.size(), MAX_DEVICE_QUEUE_SIZE);
    HILOGI("remote_device_properties_test_031 end");
}

/*
 * @tc.number: remote_device_properties_test_033
 * @tc.name: GetDeviceAbsVolumeAbility
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_033, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_033 enter");
    RemoteDeviceProperties* remoteProperties = RemoteDeviceProperties::GetInstance();
    EXPECT_EQ(remoteProperties->GetDeviceAbsVolumeAbility(RawAddress("11:22:33:44:55:66")),
        DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT);
    HILOGI("remote_device_properties_test_033 end");
}

/*
 * @tc.number: remote_device_properties_test_034
 * @tc.name: SetRemoteDeviceProperty
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_034, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_034 enter");
    RemoteDeviceProperties* remoteProperties = RemoteDeviceProperties::GetInstance();

    remoteProperties->IsAclConnected(RawAddress("11:22:33:44:55:66"));
    remoteProperties->IsAclEncrypted(RawAddress("11:22:33:44:55:66"));

    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    STACK::RawAddress addr;
    STACK::RawAddress::FromString("11:22:33:44:55:66", addr);
    remoteProperties->SetRemoteDeviceProperty(addr, property);

    EXPECT_FALSE(remoteProperties->IsAclConnected(RawAddress("11:22:33:44:55:66")));
    EXPECT_FALSE(remoteProperties->IsAclEncrypted(RawAddress("11:22:33:44:55:66")));

    remoteProperties->GetRemoteDeviceProperty(addr, bt_property_type_t::BT_PROPERTY_BDNAME);

    HILOGI("remote_device_properties_test_034 end");
}

/*
 * @tc.number: remote_device_properties_test_035
 * @tc.name: GetNotPairNoneDevices
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_035, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_035 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    std::vector<std::string> notPairNoneDevices = remoteDeviceProperties.GetNotPairNoneDevices();
    EXPECT_EQ(notPairNoneDevices.size(), 0);

    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    notPairNoneDevices = remoteDeviceProperties.GetNotPairNoneDevices();
    EXPECT_EQ(notPairNoneDevices.size(), 1);
    HILOGI("remote_device_properties_test_035 end");
}

/*
 * @tc.number: remote_device_properties_test_036
 * @tc.name: GetCustomType
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_036, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_036 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    int32_t customType = remoteDeviceProperties.GetCustomType(device);
    EXPECT_EQ(customType, -1);
    HILOGI("remote_device_properties_test_036 end");
}

/*
 * @tc.number: remote_device_properties_test_037
 * @tc.name: RemovePairWhiteList
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_037, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_037 enter");
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    remoteDeviceProperties.RemovePairWhiteList(device);
    EXPECT_TRUE(remoteDeviceProperties.whiteListMap_.empty());
    HILOGI("remote_device_properties_test_037 end");
}


/**
 * @tc.name: remote_device_properties_test_038
 * @tc.desc: GetDeviceCustomType/SetDeviceCustomType
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_038, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_038");
    RawAddress device = RawAddress("00:11:22:33:44:66");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);

    EXPECT_EQ(remoteDeviceProperties.GetCustomType(device), 0);
    EXPECT_EQ(remoteDeviceProperties.SetCustomType(device, 2), BT_NO_ERROR);
    EXPECT_EQ(remoteDeviceProperties.GetCustomType(device), 2);
    HILOGI("remote_device_properties_test_038");
}

/**
 * @tc.name: remote_device_properties_test_039
 * @tc.desc: UpdateRemoteDeviceName
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_039, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_039 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::string newRemoteName = remoteDeviceProperties.adapterProperties_->ParseDeviceName(&property);
    remoteDeviceProperties.UpdateRemoteDeviceName(device, newRemoteName);
    EXPECT_EQ(remoteDeviceProperties.GetDeviceName(device), newRemoteName);
    HILOGI("remote_device_properties_test_039 end");
}

/**
 * @tc.name: remote_device_properties_test_040
 * @tc.desc: UpdateRemoteDeviceCod
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_040, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_040 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    int newCod = static_cast<int>(remoteDeviceProperties.adapterProperties_->
        ParseDeviceCod(&property) & CLASS_OF_DEVICE_RANGE);
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    remoteDeviceProperties.UpdateRemoteDeviceCod(device, newCod);
    EXPECT_EQ(remoteDeviceProperties.GetDeviceClass(device), newCod);
    HILOGI("remote_device_properties_test_040 end");
}

/**
 * @tc.name: remote_device_properties_test_041
 * @tc.desc: HandlePropertyRemoteDeviceName
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_041, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_041 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    EXPECT_NE(remoteDeviceProperties.adapterProperties_->ParseDeviceName(&property), INVALID_NAME);
    remoteDeviceProperties.HandlePropertyRemoteDeviceName(&property, device);
    HILOGI("remote_device_properties_test_041 end");
}

/**
 * @tc.name: remote_device_properties_test_042
 * @tc.desc: HandlePropertyRemoteDeviceUuid
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_042, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_042 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetAddress(), "aa:11:22:33:44:55");
    remoteDeviceProperties.HandlePropertyRemoteDeviceUuid(&property, remoteDevice);
    HILOGI("remote_device_properties_test_042 end");
}

/**
 * @tc.name: remote_device_properties_test_043
 * @tc.desc: HandlePropertyRemoteDeviceCod
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_043, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_043 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    EXPECT_NE(remoteDeviceProperties.adapterProperties_->ParseDeviceCod(&property), INVALID_VALUE);
    remoteDeviceProperties.HandlePropertyRemoteDeviceCod(&property, device);
    HILOGI("remote_device_properties_test_043 end");
}

/**
 * @tc.name: remote_device_properties_test_044
 * @tc.desc: HandlePropertyRemoteDeviceType
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_044, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_044 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_NE(remoteDeviceProperties.adapterProperties_->ParseDeviceType(&property), INVALID_VALUE);
    remoteDeviceProperties.HandlePropertyRemoteDeviceType(&property, remoteDevice);
    HILOGI("remote_device_properties_test_044 end");
}

/**
 * @tc.name: remote_device_properties_test_045
 * @tc.desc: HandlePropertyRemoteDeviceAbsVolumKey
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_045, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_045 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDeviceProperties.adapterProperties_->ParseAbsVolumeAbility(&property),
        DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT);
    remoteDeviceProperties.HandlePropertyRemoteDeviceAbsVolumKey(&property, remoteDevice);
    HILOGI("remote_device_properties_test_045 end");
}

/**
 * @tc.name: remote_device_properties_test_046
 * @tc.desc: HandlePropertyRemoteDeviceRssi
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_046, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_046 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDeviceProperties.adapterProperties_->ParseDeviceRssi(&property), INVALID_VALUE);
    remoteDeviceProperties.HandlePropertyRemoteDeviceRssi(&property, remoteDevice);
    HILOGI("remote_device_properties_test_046 end");
}

/**
 * @tc.name: remote_device_properties_test_047
 * @tc.desc: HandlePropertyRemoteDeviceAlias
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_047, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_047 enter");
    string alias = "1";
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(alias), .val = &alias};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_NE(remoteDeviceProperties.adapterProperties_->ParseAlias(&property), INVALID_NAME);
    remoteDeviceProperties.HandlePropertyRemoteDeviceAlias(&property, remoteDevice);
    HILOGI("remote_device_properties_test_047 end");
}

/**
 * @tc.name: remote_device_properties_test_048
 * @tc.desc: HandlePropertyRemoteDeviceCustomType
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_048, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_048 enter");
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(absVolumeAbility), .val = &absVolumeAbility};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_NE(remoteDeviceProperties.adapterProperties_->ParseDeviceCustomType(&property), 0);
    remoteDeviceProperties.HandlePropertyRemoteDeviceCustomType(&property, remoteDevice);
    HILOGI("remote_device_properties_test_048 end");
}

/**
 * @tc.name: remote_device_properties_test_049
 * @tc.desc: DeviceFoundCallBack
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_049, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_049 enter");
    int numProperties = 5;
    int32_t absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    bt_property_t properties[5];
    for (int i = 0; i < numProperties; i++) {
        properties[i].type = bt_property_type_t::BT_PROPERTY_BDNAME;
        properties[i].len = sizeof(absVolumeAbility);
        properties[i].val = &absVolumeAbility;
    }
    EXPECT_FALSE(remoteDeviceProperties.IsRemoteDeviceMapEmpty());
    remoteDeviceProperties.DeviceFoundCallBack(numProperties, properties);
    HILOGI("remote_device_properties_test_049 end");
}

/*
 * @tc.number: remote_device_properties_test_050
 * @tc.name: RemovePairWhiteList
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_050, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_050 enter");
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    RawAddress device_ = RawAddress("05:11:22:33:44:66");
    remoteDeviceProperties.RemovePairWhiteList(device);
    remoteDeviceProperties.RemovePairWhiteList(device_);
    remoteDeviceProperties.RemovePairWhiteList(device);
    EXPECT_EQ(remoteDeviceProperties.whiteListMap_.size(), 0);
    HILOGI("remote_device_properties_test_050 end");
}

/*
 * @tc.number: remote_device_properties_test_051
 * @tc.name: GetDevConnStateChangeCause
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_051, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_051 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);

    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    remoteDeviceProperties.SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HID_HOST, true);
    int cause = remoteDeviceProperties.GetDevConnStateChangeCause(device, PROFILE_ID_HID_HOST, state);
    EXPECT_EQ(cause, static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_USER_DISCONNECT));
    HILOGI("remote_device_properties_test_051 end");
}

/*
 * @tc.number: remote_device_properties_test_052
 * @tc.name: GetDevConnStateChangeCause
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_052, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_052 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetDeviceClass(BluetoothDevice::PERIPHERAL_KEYBOARD);

    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    remoteDeviceProperties.SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HID_HOST, false);
    int cause = remoteDeviceProperties.GetDevConnStateChangeCause(device, PROFILE_ID_HID_HOST, state);
    EXPECT_EQ(cause, static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_KEYBOARD));
    HILOGI("remote_device_properties_test_052 end");
}

/*
 * @tc.number: remote_device_properties_test_053
 * @tc.name: GetDevConnStateChangeCause
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_053, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_053 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetDeviceClass(BluetoothDevice::PERIPHERAL_POINTING_DEVICE);

    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    remoteDeviceProperties.SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HID_HOST, false);
    int cause = remoteDeviceProperties.GetDevConnStateChangeCause(device, PROFILE_ID_HID_HOST, state);
    EXPECT_EQ(cause, static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_MOUSE));
    HILOGI("remote_device_properties_test_053 end");
}

/*
 * @tc.number: remote_device_properties_test_054
 * @tc.name: GetDevConnStateChangeCause
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_054, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_054 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetDeviceClass(BluetoothDevice::AUDIO_VIDEO_CAR_AUDIO);

    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    remoteDeviceProperties.SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HID_HOST, false);
    int cause = remoteDeviceProperties.GetDevConnStateChangeCause(device, PROFILE_ID_HID_HOST, state);
    EXPECT_EQ(cause, static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_CAR));
    HILOGI("remote_device_properties_test_054 end");
}

/*
 * @tc.number: remote_device_properties_test_055
 * @tc.name: GetDevConnStateChangeCause
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_055, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_055 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetDeviceClass(0);

    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    remoteDeviceProperties.SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HID_HOST, false);
    int cause = remoteDeviceProperties.GetDevConnStateChangeCause(device, PROFILE_ID_HID_HOST, state);
    EXPECT_EQ(cause, static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FAIL_INTERNAL));
    HILOGI("remote_device_properties_test_055 end");
}

/*
 * @tc.number: remote_device_properties_test_056
 * @tc.name: SetDevProactiveDisconnectFlag
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_056, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_056 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    bool isProDisconnect = false;
    remoteDeviceProperties.SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HID_HOST, isProDisconnect);
    bool ret = remoteDeviceProperties.IsDevProactiveDisconnect(device.GetAddress(), PROFILE_ID_HID_HOST);
    EXPECT_EQ(false, ret);
    HILOGI("remote_device_properties_test_056 end");
}

/*
 * @tc.number: remote_device_properties_test_057
 * @tc.name: SetDevProactiveDisconnectFlag
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_057, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_057 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    bool isProDisconnect = true;
    remoteDeviceProperties.SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HID_HOST, isProDisconnect);
    bool ret = remoteDeviceProperties.IsDevProactiveDisconnect(device.GetAddress(), PROFILE_ID_HID_HOST);
    EXPECT_EQ(false, ret);
    HILOGI("remote_device_properties_test_057 end");
}

/*
 * @tc.number: remote_device_properties_test_058
 * @tc.name: SetDevProactiveDisconnectFlag
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_058, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_058 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    bool isProDisconnect = true;
    remoteDeviceProperties.SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HID_HOST, isProDisconnect);
    bool ret = remoteDeviceProperties.IsDevProactiveDisconnect(device.GetAddress(), PROFILE_ID_HID_HOST);
    EXPECT_EQ(true, ret);
    HILOGI("remote_device_properties_test_058 end");
}

/*
 * @tc.number: remote_device_properties_test_059
 * @tc.name: GetDevConnStateChangeCause
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_059, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_059 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    int state = static_cast<int>(BTConnectState::CONNECTED);
    int cause = remoteDeviceProperties.GetDevConnStateChangeCause(device, PROFILE_ID_HID_HOST, state);
    EXPECT_EQ(cause, static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
    HILOGI("remote_device_properties_test_059 end");
}

/*
 * @tc.number: remote_device_properties_test_060
 * @tc.name: HandlePropertyRemoteDeviceIoCapability
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_060, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_060 enter");
    int32_t ioCapability = 0;
    bt_property_t property{.type = bt_property_type_t::HW_BT_PROPERTY_RMT_IO_CAP_KEY,
        .len = sizeof(ioCapability), .val = &ioCapability};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDeviceProperties.adapterProperties_->ParseRemoteDeviceIoCapability(&property), INVALID_VALUE);
    remoteDeviceProperties.HandlePropertyRemoteDeviceIoCapability(&property, remoteDevice);
    HILOGI("remote_device_properties_test_060 end");
}

/**
 * @tc.name: remote_device_properties_test_061
 * @tc.desc: HandlePropertyRemoteDeviceChipInfo
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_061, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_061 enter");
    bt_remote_version_t chipInfo;
    chipInfo.manufacturer = 688;
    chipInfo.sub_ver = 277;
    chipInfo.version = 11;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_REMOTE_VERSION_INFO,
        .len = sizeof(chipInfo), .val = &chipInfo};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetAddress(), "aa:11:22:33:44:55");
    remoteDeviceProperties.HandlePropertyRemoteDeviceChipInfo(&property, remoteDevice);
    HILOGI("remote_device_properties_test_061 end");
}
 
/**
 * @tc.name: remote_device_properties_test_062
 * @tc.desc: HandlePropertyRemoteDeviceMessage
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_062, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_062 enter");
    bt_remote_version_t chipInfo;
    chipInfo.manufacturer = 688;
    chipInfo.sub_ver = 277;
    chipInfo.version = 11;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_REMOTE_VERSION_INFO,
        .len = sizeof(chipInfo), .val = &chipInfo};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetAddress(), "aa:11:22:33:44:55");
    remoteDeviceProperties.HandlePropertyRemoteDeviceMessage(&property, remoteDevice);
    property.type = BT_PROPERTY_AUTO_CONNECT_SWITCH;
    remoteDeviceProperties.HandlePropertyRemoteDeviceMessage(&property, remoteDevice);
    property.type = BT_PROPERTY_ClOUD_CAP;
    remoteDeviceProperties.HandlePropertyRemoteDeviceMessage(&property, remoteDevice);
    HILOGI("remote_device_properties_test_062 end");
}

/**
 * @tc.name: remote_device_properties_test_063
 * @tc.desc: HandlePropertyRemoteDeviceChipInfo
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_063, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_063 enter");
    bt_remote_version_t chipInfo;
    chipInfo.manufacturer = 688;
    chipInfo.sub_ver = 277;
    chipInfo.version = 11;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_REMOTE_VERSION_INFO,
        .len = sizeof(chipInfo) - 1, .val = &chipInfo};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetAddress(), "aa:11:22:33:44:55");
    remoteDeviceProperties.HandlePropertyRemoteDeviceChipInfo(&property, remoteDevice);
    HILOGI("remote_device_properties_test_063 end");
}

/**
 * @tc.name: remote_device_properties_test_064
 * @tc.desc: HandlePropertyRemoteDeviceChipInfo
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_064, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_064 enter");
    bt_remote_version_t chipInfo;
    chipInfo.manufacturer = 688;
    chipInfo.sub_ver = 277;
    chipInfo.version = 11;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_REMOTE_VERSION_INFO,
        .len = sizeof(chipInfo), .val = nullptr};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetAddress(), "aa:11:22:33:44:55");
    remoteDeviceProperties.HandlePropertyRemoteDeviceChipInfo(&property, remoteDevice);
    HILOGI("remote_device_properties_test_064 end");
}

/**
 * @tc.name: remote_device_properties_test_065
 * @tc.desc: HandlePropertyRemoteDeviceMessage
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_065, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_065 enter");
    bt_remote_version_t chipInfo;
    chipInfo.manufacturer = 688;
    chipInfo.sub_ver = 277;
    chipInfo.version = 11;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_BDNAME,
        .len = sizeof(chipInfo), .val = &chipInfo};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetAddress(), "aa:11:22:33:44:55");
    remoteDeviceProperties.HandlePropertyRemoteDeviceMessage(&property, remoteDevice);
    HILOGI("remote_device_properties_test_065 end");
}

/**
 * @tc.name: remote_device_properties_test_066
 * @tc.desc: SetRemoteDevicePropertyInfo
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_066, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_066 enter");
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::string value = "123";
    EXPECT_TRUE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, bt_property_type_t::BT_PROPERTY_BDNAME,
        value));
    EXPECT_TRUE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_VENDOR_ID, value));
    value = "123ABG";
    EXPECT_TRUE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_NEW_MODEL_ID, value));
    EXPECT_FALSE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_MODEL_ID, value));
    value = "1";
    EXPECT_FALSE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_AUTO_CONNECT_SWITCH, value));

    value = "000149";
    EXPECT_TRUE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_MODEL_ID, value));

    value = "34%56";
    EXPECT_FALSE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_TIMESTAMP, value));

    EXPECT_FALSE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_FRIENDLY_NAME, value));

    value = "3456";
    EXPECT_TRUE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_TIMESTAMP, value));
    value = "3";
    EXPECT_TRUE(remoteDeviceProperties.SetRemoteDevicePropertyInfo(device, BT_PROPERTY_DEVICE_TYPE_ID, value));
    HILOGI("remote_device_properties_test_066 end");
}

/*
 * @tc.number: remote_device_properties_test_067
 * @tc.name: HandlePropertyRemoteDeviceAutoConnSwitch
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_067, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_067 enter");
    int32_t autoConnSwitch = 0;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_AUTO_CONNECT_SWITCH,
        .len = sizeof(autoConnSwitch), .val = &autoConnSwitch};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetAddress(), "aa:11:22:33:44:55");
    remoteDeviceProperties.HandlePropertyRemoteDeviceAutoConnSwitch(&property, remoteDevice);
    HILOGI("remote_device_properties_test_067 end");
}

/*
 * @tc.number: remote_device_properties_test_068
 * @tc.name: IsNeedDelayConnect
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_068, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_067 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_FALSE(remoteDeviceProperties.IsNeedDelayConnect(device));
    HILOGI("remote_device_properties_test_068 end");
}

/*
 * @tc.number: remote_device_properties_test_069
 * @tc.name: GetLastConnectionTime
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_069, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_067 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    EXPECT_EQ(remoteDeviceProperties.SetConnectionTime(device, 1), BT_ERR_INVALID_PARAM);
    remoteDeviceProperties.GetLastConnectionTime(device);
    HILOGI("remote_device_properties_test_068 end");
}

/**
 * @tc.name: remote_device_properties_test_070
 * @tc.desc: HandlePropertyRemoteDeviceConnectionTime
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_070, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_070 enter");
    bt_remote_version_t chipInfo;
    chipInfo.manufacturer = 688;
    chipInfo.sub_ver = 277;
    chipInfo.version = 11;
    bt_property_t property{.type = bt_property_type_t::BT_PROPERTY_CONNECTION_TIME,
        .len = sizeof(chipInfo), .val = &chipInfo};
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetAddress(), "aa:11:22:33:44:55");
    remoteDeviceProperties.HandlePropertyRemoteDeviceConnectionTime(&property, remoteDevice);
    HILOGI("remote_device_properties_test_070 end");
}

/**
 * @tc.name: set_cloud_prop_test_071
 * @tc.desc: SetCloudDevice
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, set_cloud_prop_test_071, TestSize.Level1)
{
    HILOGI("set_cloud_prop_test_071 enter");
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.SetCloudDevice(device, true);
    EXPECT_TRUE(remoteDeviceProperties.IsCloudDevice(device));
    HILOGI("set_cloud_prop_test_071 end");
}

/**
 * @tc.name: set_cloud_prop_test_072
 * @tc.desc: SetCloudBondState
 * @tc.type: FUNC
 */
HWTEST_F(RemoteDevicePropertiesTest, set_cloud_prop_test_072, TestSize.Level1)
{
    HILOGI("set_cloud_prop_test_072 enter");
    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.SetCloudBondState(device, 1);
    EXPECT_EQ(remoteDeviceProperties.GetCloudBondState(device), 1);
    HILOGI("set_cloud_prop_test_072 end");
}

/*
 * @tc.number: remote_device_properties_test_073
 * @tc.name: GetHwRemoteDeviceType
 * @tc.desc:
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_073, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_073 enter");
    RawAddress device = RawAddress("00:11:22:33:44:55");
    std::string name = "00:11:22:33:44:55";
    remoteDeviceProperties.UpdateRemoteHwDeviceType(device, HW_CAR_DEVICE_TYPE);
    EXPECT_EQ(remoteDeviceProperties.GetHwRemoteDeviceType(device), HW_CAR_DEVICE_TYPE);
    HILOGI("remote_device_properties_test_073 end");
}

/*
 * @tc.number: remote_device_properties_test_074
 * @tc.name: SetDeviceTypeTWSMapToHeadset
 * @tc.desc: Test hardware device type TWS (0x03) maps to DEVICE_TYPE_HEADSET
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_074, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_074 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    std::string twsValue = ToHexString(static_cast<int32_t>(HW_ECHO_PRODUCT::HEADSET_TWS));
    EXPECT_TRUE(remoteDeviceProperties.SetOtherPropertyInfo(device, bt_property_type_t::BT_PROPERTY_DEVICE_TYPE_ID, twsValue));
    int32_t customType = remoteDeviceProperties.GetCustomType(device);
    EXPECT_EQ(customType, DeviceType::DEVICE_TYPE_HEADSET);

    HILOGI("remote_device_properties_test_074 end");
}

/*
 * @tc.number: remote_device_properties_test_075
 * @tc.name: SetDeviceTypeHeadphoneMapToHeadset
 * @tc.desc: Test hardware device type Headphone (0x04) maps to DEVICE_TYPE_HEADSET
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_075, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_075 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    std::string headphoneValue = ToHexString(static_cast<int32_t>(HW_ECHO_PRODUCT::HEADSET_HEADPHONE));
    EXPECT_TRUE(remoteDeviceProperties.SetOtherPropertyInfo(device, bt_property_type_t::BT_PROPERTY_DEVICE_TYPE_ID, headphoneValue));
    int32_t customType = remoteDeviceProperties.GetCustomType(device);
    EXPECT_EQ(customType, DeviceType::DEVICE_TYPE_HEADSET);

    HILOGI("remote_device_properties_test_075 end");
}

/*
 * @tc.number: remote_device_properties_test_076
 * @tc.name: SetDeviceTypeWearableMapToHeadset
 * @tc.desc: Test hardware device type Wearable (0x05) maps to DEVICE_TYPE_HEADSET
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_076, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_076 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    std::string wearableValue = ToHexString(static_cast<int32_t>(HW_ECHO_PRODUCT::HEADSET_WERABLE));
    EXPECT_TRUE(remoteDeviceProperties.SetOtherPropertyInfo(device, bt_property_type_t::BT_PROPERTY_DEVICE_TYPE_ID, wearableValue));
    int32_t customType = remoteDeviceProperties.GetCustomType(device);
    EXPECT_EQ(customType, DeviceType::DEVICE_TYPE_HEADSET);

    HILOGI("remote_device_properties_test_076 end");
}

/*
 * @tc.number: remote_device_properties_test_077
 * @tc.name: SetDeviceTypeGlassesMapToGlasses
 * @tc.desc: Test hardware device type Glasses (0x0A) maps to DEVICE_TYPE_GLASSES
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_077, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_077 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    std::string glassesValue = ToHexString(static_cast<int32_t>(HW_ECHO_PRODUCT::GLASSES));
    EXPECT_TRUE(remoteDeviceProperties.SetOtherPropertyInfo(device, bt_property_type_t::BT_PROPERTY_DEVICE_TYPE_ID, glassesValue));
    int32_t customType = remoteDeviceProperties.GetCustomType(device);
    EXPECT_EQ(customType, DeviceType::DEVICE_TYPE_GLASSES);

    HILOGI("remote_device_properties_test_077 end");
}

/*
 * @tc.number: remote_device_properties_test_078
 * @tc.name: SetDeviceTypeCarMapToCar
 * @tc.desc: Test hardware device type Car (0x69) maps to DEVICE_TYPE_CAR
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_078, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_078 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    std::string carValue = ToHexString(static_cast<int32_t>(HW_ECHO_PRODUCT::CAR));
    EXPECT_TRUE(remoteDeviceProperties.SetOtherPropertyInfo(device, bt_property_type_t::BT_PROPERTY_DEVICE_TYPE_ID, carValue));
    int32_t customType = remoteDeviceProperties.GetCustomType(device);
    EXPECT_EQ(customType, DeviceType::DEVICE_TYPE_CAR);

    HILOGI("remote_device_properties_test_078 end");
}

/*
 * @tc.number: remote_device_properties_test_079
 * @tc.name: SetDeviceTypeUnmappedDefaultValue
 * @tc.desc: Test unmapped hardware device type keeps custom type as DEFAULT
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_079, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_079 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("aa:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    std::string unmappedValue = "999";
    EXPECT_TRUE(remoteDeviceProperties.SetOtherPropertyInfo(device, bt_property_type_t::BT_PROPERTY_DEVICE_TYPE_ID,
        unmappedValue));
    int32_t customType = remoteDeviceProperties.GetCustomType(device);
    EXPECT_EQ(customType, DeviceType::DEVICE_TYPE_DEFAULT);

    HILOGI("remote_device_properties_test_079 end");
}

/*
 * @tc.number: remote_device_properties_test_080
 * @tc.name: GetRemoteDevicePropsCallbackInner
 * @tc.desc: Test GetRemoteDevicePropsCallbackInner with all property types
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_080, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_080 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("aa:11:22:33:44:55");
    STACK::RawAddress btAddr;
    STACK::RawAddress::FromString("aa:11:22:33:44:55", btAddr);

    std::string deviceName = "TestDevice";
    int32_t deviceType = 1;
    int32_t absVolumeAbility = 1;
    uint32_t rssi = -60;
    std::string aliasName = "TestAlias";
    int32_t customType = 2;
    int32_t ioCapability = 3;
    int32_t vendorId = 0x012C;
    int32_t productId = 0x0345;
    int64_t connectionTime = 1234567890;
    int32_t cod = 0x240404;

    int numProperties = 13;
    bt_property_t* properties = static_cast<bt_property_t*>(malloc(sizeof(bt_property_t) * numProperties));
    ASSERT_NE(properties, nullptr);

    int offset = 0;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_BDNAME;
    properties[offset].len = deviceName.size();
    properties[offset].val = const_cast<char*>(deviceName.c_str());
    offset++;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_UUIDS;
    Uuid uuid = Uuid::ConvertFromString("0000110B-0000-1000-8000-00805F9B34FB");
    properties[offset].len = sizeof(uuid);
    properties[offset].val = &uuid;
    offset++;

    properties[offset].type = STACK::BT_PROPERTY_CLASS_OF_DEVICE;
    properties[offset].len = sizeof(cod);
    properties[offset].val = &cod;
    offset++;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_TYPE_OF_DEVICE;
    properties[offset].len = sizeof(deviceType);
    properties[offset].val = &deviceType;
    offset++;

    properties[offset].type = HW_BT_PROPERTY_ABS_VOLUM_KEY;
    properties[offset].len = sizeof(absVolumeAbility);
    properties[offset].val = &absVolumeAbility;
    offset++;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_REMOTE_RSSI;
    properties[offset].len = sizeof(rssi);
    properties[offset].val = &rssi;
    offset++;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_REMOTE_FRIENDLY_NAME;
    properties[offset].len = aliasName.size();
    properties[offset].val = const_cast<char*>(aliasName.c_str());
    offset++;

    properties[offset].type = HW_BT_PROPERTY_CUSTOM_TYPE;
    properties[offset].len = sizeof(customType);
    properties[offset].val = &customType;
    offset++;

    properties[offset].type = HW_BT_PROPERTY_RMT_IO_CAP_KEY;
    properties[offset].len = sizeof(ioCapability);
    properties[offset].val = &ioCapability;
    offset++;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_VENDOR_ID;
    properties[offset].len = sizeof(vendorId);
    properties[offset].val = &vendorId;
    offset++;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_PRODUCT_ID;
    properties[offset].len = sizeof(productId);
    properties[offset].val = &productId;
    offset++;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_CONNECTION_TIME;
    properties[offset].len = sizeof(connectionTime);
    properties[offset].val = &connectionTime;
    offset++;

    properties[offset].type = bt_property_type_t::BT_PROPERTY_REMOTE_VERSION_INFO;
    bt_remote_version_t chipInfo;
    chipInfo.manufacturer = 688;
    chipInfo.sub_ver = 277;
    chipInfo.version = 11;
    properties[offset].len = sizeof(chipInfo);
    properties[offset].val = &chipInfo;

    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.GetRemoteDevicePropsCallbackInner(btAddr, numProperties, properties);
    free(properties);
    HILOGI("remote_device_properties_test_080 end");
}

/*
 * @tc.number: remote_device_properties_test_081
 * @tc.name: GetRemoteDevicePropsCallBack
 * @tc.desc: Test GetRemoteDevicePropsCallBack with all branches
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_081, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_081 enter");
    RawAddress device = RawAddress("bb:11:22:33:44:55");
    STACK::RawAddress btAddr;
    STACK::RawAddress::FromString("bb:11:22:33:44:55", btAddr);

    std::string deviceName = "TestDevice2";
    int numProperties = 1;
    bt_property_t* properties = static_cast<bt_property_t*>(malloc(sizeof(bt_property_t) * numProperties));
    ASSERT_NE(properties, nullptr);

    properties[0].type = bt_property_type_t::BT_PROPERTY_BDNAME;
    properties[0].len = deviceName.size();
    properties[0].val = const_cast<char*>(deviceName.c_str());

    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);

    remoteDeviceProperties.GetRemoteDevicePropsCallBack(BT_STATUS_SUCCESS, &btAddr, numProperties, properties);
    free(properties);
    HILOGI("remote_device_properties_test_081 end");
}

/*
 * @tc.number: remote_device_properties_test_082
 * @tc.name: GetRemoteDevicePropsCallBack_NullBdAddr
 * @tc.desc: Test GetRemoteDevicePropsCallBack with null bd_addr
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_082, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_082 enter");
    std::string deviceName = "TestDevice3";
    int numProperties = 1;
    bt_property_t* properties = static_cast<bt_property_t*>(malloc(sizeof(bt_property_t) * numProperties));
    ASSERT_NE(properties, nullptr);

    properties[0].type = bt_property_type_t::BT_PROPERTY_BDNAME;
    properties[0].len = deviceName.size();
    properties[0].val = const_cast<char*>(deviceName.c_str());

    remoteDeviceProperties.GetRemoteDevicePropsCallBack(BT_STATUS_SUCCESS, nullptr, numProperties, properties);

    free(properties);
    HILOGI("remote_device_properties_test_082 end");
}

/*
 * @tc.number: remote_device_properties_test_083
 * @tc.name: GetRemoteDevicePropsCallBack_FailedStatus
 * @tc.desc: Test GetRemoteDevicePropsCallBack with failed status
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_083, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_083 enter");
    RawAddress device = RawAddress("cc:11:22:33:44:55");
    STACK::RawAddress btAddr;
    STACK::RawAddress::FromString("cc:11:22:33:44:55", btAddr);

    std::string deviceName = "TestDevice4";
    int numProperties = 1;
    bt_property_t* properties = static_cast<bt_property_t*>(malloc(sizeof(bt_property_t) * numProperties));
    ASSERT_NE(properties, nullptr);

    properties[0].type = bt_property_type_t::BT_PROPERTY_BDNAME;
    properties[0].len = deviceName.size();
    properties[0].val = const_cast<char*>(deviceName.c_str());

    remoteDeviceProperties.GetRemoteDevicePropsCallBack(BT_STATUS_FAIL, &btAddr, numProperties, properties);

    EXPECT_NE(remoteDeviceProperties.GetDeviceName(device), deviceName);
    free(properties);
    HILOGI("remote_device_properties_test_083 end");
}

/*
 * @tc.number: remote_device_properties_test_084
 * @tc.name: GetRemoteDevicePropsCallBack_AbsVolumeKey
 * @tc.desc: Test GetRemoteDevicePropsCallBack with abs volume key property
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_084, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_084 enter");
    RawAddress device = RawAddress("dd:11:22:33:44:55");
    STACK::RawAddress btAddr;
    STACK::RawAddress::FromString("dd:11:22:33:44:55", btAddr);

    int32_t absVolumeAbility = 2;
    int numProperties = 1;
    bt_property_t* properties = static_cast<bt_property_t*>(malloc(sizeof(bt_property_t) * numProperties));
    ASSERT_NE(properties, nullptr);

    properties[0].type = HW_BT_PROPERTY_ABS_VOLUM_KEY;
    properties[0].len = sizeof(absVolumeAbility);
    properties[0].val = &absVolumeAbility;

    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);

    remoteDeviceProperties.GetRemoteDevicePropsCallBack(BT_STATUS_SUCCESS, &btAddr, numProperties, properties);

    EXPECT_EQ(remoteDeviceProperties.GetDeviceAbsVolumeAbility(device), absVolumeAbility);

    free(properties);
    HILOGI("remote_device_properties_test_084 end");
}

/*
 * @tc.number: remote_device_properties_test_085
 * @tc.name: HandlePropertyRemoteDeviceCloudCap
 * @tc.desc: Test HandlePropertyRemoteDeviceCloudCap with cloud device capability
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_085, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_085 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("ee:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);

    int32_t isCloudDevice = 1;
    bt_property_t property;
    property.type = bt_property_type_t::BT_PROPERTY_ClOUD_CAP;
    property.len = sizeof(isCloudDevice);
    property.val = &isCloudDevice;

    remoteDeviceProperties.HandlePropertyRemoteDeviceCloudCap(&property, remoteDevice);

    EXPECT_TRUE(remoteDevice->IsCloudDevice());
    HILOGI("remote_device_properties_test_085 end");
}

/*
 * @tc.number: remote_device_properties_test_086
 * @tc.name: HandlePropertyRemoteDeviceCloudCap_ZeroValue
 * @tc.desc: Test HandlePropertyRemoteDeviceCloudCap with zero cloud device value
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_086, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_086 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("ff:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);

    int32_t isCloudDevice = 0;
    bt_property_t property;
    property.type = bt_property_type_t::BT_PROPERTY_ClOUD_CAP;
    property.len = sizeof(isCloudDevice);
    property.val = &isCloudDevice;

    remoteDeviceProperties.HandlePropertyRemoteDeviceCloudCap(&property, remoteDevice);

    EXPECT_FALSE(remoteDevice->IsCloudDevice());
    HILOGI("remote_device_properties_test_086 end");
}

/*
 * @tc.number: remote_device_properties_test_087
 * @tc.name: HandlePropertyRemoteDeviceCloudCap_MultipleCalls
 * @tc.desc: Test HandlePropertyRemoteDeviceCloudCap with multiple calls
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_087, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_087 enter");
    remoteDeviceProperties.remoteDevicesMap_.clear();

    RawAddress device = RawAddress("00:11:22:33:44:66");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);

    int32_t isCloudDevice = 1;
    bt_property_t property;
    property.type = bt_property_type_t::BT_PROPERTY_ClOUD_CAP;
    property.len = sizeof(isCloudDevice);
    property.val = &isCloudDevice;

    remoteDeviceProperties.HandlePropertyRemoteDeviceCloudCap(&property, remoteDevice);
    EXPECT_TRUE(remoteDevice->IsCloudDevice());

    isCloudDevice = 0;
    property.val = &isCloudDevice;
    remoteDeviceProperties.HandlePropertyRemoteDeviceCloudCap(&property, remoteDevice);

    isCloudDevice = 1;
    property.val = &isCloudDevice;
    remoteDeviceProperties.HandlePropertyRemoteDeviceCloudCap(&property, remoteDevice);
    EXPECT_TRUE(remoteDevice->IsCloudDevice());
    HILOGI("remote_device_properties_test_087 end");
}

#ifdef BLUETOOTH_SPACE_MGR_FEATURE_ENABLE
/**
 * @tc.number: remote_device_properties_test_088
 * @tc.name: GetBtPairedDevicesInEnterpriseWorkspace
 * @tc.desc: cloud device without cloud bond is hidden in enterprise workspace
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_088, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_088 enter");
    RawAddress device = RawAddress("bb:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    remoteDeviceProperties.SetCloudDevice(device, true);
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_NONE);

    auto isDeviceInList = [](const std::vector<RawAddress> &list, const RawAddress &addr) {
        for (auto &item : list) {
            if (item.GetAddress() == addr.GetAddress()) {
                return true;
            }
        }
        return false;
    };
    auto &workspaceManager = BluetoothWorkspaceManager::GetInstance();
    workspaceManager.isEnterpriseWorkspace_ = true;
    EXPECT_FALSE(isDeviceInList(remoteDeviceProperties.GetBtPairedDevices(), device));
    workspaceManager.isEnterpriseWorkspace_ = false;
    EXPECT_TRUE(isDeviceInList(remoteDeviceProperties.GetBtPairedDevices(), device));
    HILOGI("remote_device_properties_test_088 end");
}

/**
 * @tc.number: remote_device_properties_test_089
 * @tc.name: IsStartCloudPairInEnterpriseWorkspace
 * @tc.desc: cloud pair is not started in enterprise workspace
 */
HWTEST_F(RemoteDevicePropertiesTest, remote_device_properties_test_089, TestSize.Level1)
{
    HILOGI("remote_device_properties_test_089 enter");
    RawAddress device = RawAddress("bb:11:22:33:44:55");
    remoteDeviceProperties.SetCloudDevice(device, true);
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_NONE);

    auto &workspaceManager = BluetoothWorkspaceManager::GetInstance();
    workspaceManager.isEnterpriseWorkspace_ = false;
    EXPECT_TRUE(remoteDeviceProperties.IsStartCloudPair(device));
    workspaceManager.isEnterpriseWorkspace_ = true;
    EXPECT_FALSE(remoteDeviceProperties.IsStartCloudPair(device));
    workspaceManager.isEnterpriseWorkspace_ = false;
    HILOGI("remote_device_properties_test_089 end");
}
#endif

}  // namespace bluetooth
}  // namespace OHOS