/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include <mutex>
#include <thread>

#include "bluetooth_access_token_mock.h"
#include "bluetooth_def.h"
#include "bluetooth_device_class.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_log.h"
#include "bluetooth_proxy_manager.h"
#include "bluetooth_profile_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

BluetoothHost *bluetoothHost_;
sptr<IRemoteObject> remoteObject;
int g_BluetoothState = -1;
constexpr int TIME = 2;
constexpr int TIME_10 = 10;

class BluetoothRemoteDeviceTest : public testing::Test {
public:
    BluetoothRemoteDeviceTest()
    {}
    ~BluetoothRemoteDeviceTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BluetoothRemoteDeviceTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
    bluetoothHost_ = &BluetoothHost::GetDefaultHost();
    remoteObject = BluetoothProfileManager::GetInstance().GetProfileRemote("HostObject");

    bool isBleEnable = bluetoothHost_->IsBleEnabled();
    bool isBrEnable = bluetoothHost_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        bluetoothHost_->EnableBle();
        bluetoothHost_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}

void BluetoothRemoteDeviceTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    bluetoothHost_ = nullptr;
}

void BluetoothRemoteDeviceTest::SetUp()
{
}

void BluetoothRemoteDeviceTest::TearDown()
{
}

/**
 * @tc.number: Remote_Device_ModuleTest_IS_BT_ENABLED_00100
 * @tc.name: IS_BT_ENABLED
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IS_BT_ENABLED_00100, TestSize.Level1)
{
    HILOGI("Host_ModuleTest_GetBtState_00100 start");

    EXPECT_EQ(bluetoothHost_->GetBtState(), BTStateID::STATE_TURN_ON);
    if (!IS_BT_ENABLED()) {
        bluetoothHost_->EnableBle();
        bluetoothHost_->EnableBt();
        sleep(TIME);
    }

    HILOGI("Host_ModuleTest_GetBtState_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetPhonebookPermission_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetPhonebookPermission_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetPhonebookPermission_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_TRUE(device_.SetPhonebookPermission(static_cast<int>(BTPermissionType::ACCESS_ALLOWED)));
    EXPECT_EQ(device_.GetPhonebookPermission(), static_cast<int>(BTPermissionType::ACCESS_ALLOWED));

    HILOGI("Remote_Device_ModuleTest_GetPhonebookPermission_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetPhonebookPermission_00200
 * @tc.name:
 * @tc.desc: transport_:BT_TRANSPORT_BLE
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetPhonebookPermission_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetPhonebookPermission_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BLE);
    EXPECT_TRUE(device_.SetPhonebookPermission(static_cast<int>(BTPermissionType::ACCESS_ALLOWED)));
    EXPECT_EQ(device_.GetPhonebookPermission(), static_cast<int>(BTPermissionType::ACCESS_ALLOWED));

    HILOGI("Remote_Device_ModuleTest_GetPhonebookPermission_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetPhonebookPermission_00300
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetPhonebookPermission_00300, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetPhonebookPermission_00300 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_TRUE(device_.SetPhonebookPermission(static_cast<int>(BTPermissionType::ACCESS_ALLOWED)));
    EXPECT_EQ(device_.GetPhonebookPermission(), static_cast<int>(BTPermissionType::ACCESS_ALLOWED));

    HILOGI("Remote_Device_ModuleTest_GetPhonebookPermission_00300 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetPhonebookPermission_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetPhonebookPermission_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetPhonebookPermission_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_TRUE(device_.SetPhonebookPermission(static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)));
    EXPECT_EQ(device_.GetPhonebookPermission(), static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN));

    HILOGI("Remote_Device_ModuleTest_SetPhonebookPermission_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetPhonebookPermission_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetPhonebookPermission_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetPhonebookPermission_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_TRUE(device_.SetPhonebookPermission(static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)));
    EXPECT_EQ(device_.GetPhonebookPermission(), static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN));

    HILOGI("Remote_Device_ModuleTest_SetPhonebookPermission_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetMessagePermission_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetMessagePermission_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetPhonebookPermission_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_TRUE(device_.SetMessagePermission(static_cast<int>(BTPermissionType::ACCESS_ALLOWED)));
    EXPECT_EQ(device_.GetMessagePermission(), static_cast<int>(BTPermissionType::ACCESS_ALLOWED));

    HILOGI("Remote_Device_ModuleTest_GetMessagePermission_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetMessagePermission_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetMessagePermission_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetMessagePermission_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_TRUE(device_.SetMessagePermission(static_cast<int>(BTPermissionType::ACCESS_ALLOWED)));
    EXPECT_EQ(device_.GetMessagePermission(), static_cast<int>(BTPermissionType::ACCESS_ALLOWED));

    HILOGI("Remote_Device_ModuleTest_GetMessagePermission_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetMessagePermission_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetMessagePermission_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetMessagePermission_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_TRUE(device_.SetMessagePermission(static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)));
    EXPECT_EQ(device_.GetMessagePermission(), static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN));

    HILOGI("Remote_Device_ModuleTest_SetMessagePermission_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetMessagePermission_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetMessagePermission_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetMessagePermission_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_TRUE(device_.SetMessagePermission(static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)));
    EXPECT_EQ(device_.GetMessagePermission(), static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN));

    HILOGI("Remote_Device_ModuleTest_SetMessagePermission_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetPowerMode_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetPowerMode_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetPowerMode_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_GE(device_.GetPowerMode(), 0);

    HILOGI("Remote_Device_ModuleTest_GetPowerMode_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetPowerMode_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetPowerMode_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetPowerMode_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_GE(device_.GetPowerMode(), 0);

    HILOGI("Remote_Device_ModuleTest_GetPowerMode_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceAddr_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceAddr_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceAddr_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device_.GetDeviceAddr(), "00:00:00:00:00:00");

    HILOGI("Remote_Device_ModuleTest_GetDeviceAddr_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceName_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceName_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_TRUE(device_.GetDeviceName().empty());

    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceName_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceName_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_TRUE(device_.GetDeviceName().empty());

    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceName_00300
 * @tc.name: GetDeviceName(std::string &name)
 * @tc.desc: GetDeviceName(std::string) function test
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceName_00300, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00300 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    std::string name = INVALID_NAME;
    EXPECT_EQ(device_.GetDeviceName(), name);

    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00300 end name:%{public}s", name.c_str());
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceName_00400
 * @tc.name: GetDeviceName(std::string &name)
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceName_00400, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00400 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    std::string name = INVALID_NAME;
    EXPECT_FALSE(device_.GetDeviceName(name));

    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00400 end name:%{public}s", name.c_str());
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceName_00500
 * @tc.name: GetDeviceName(std::string &name)
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceName_00500, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00500 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    std::string name = INVALID_NAME;
    EXPECT_EQ(device_.GetDeviceName(name), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00500 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceAlias_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceAlias_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceAlias_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device_.GetDeviceAlias(), INVALID_NAME);

    HILOGI("Remote_Device_ModuleTest_GetDeviceAlias_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceAlias_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceAlias_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceAlias_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_EQ(device_.GetDeviceAlias(), INVALID_NAME);

    HILOGI("Remote_Device_ModuleTest_GetDeviceAlias_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDeviceAlias_00100
 * @tc.name:
 * @tc.desc: operation fail because could not find addr("00:00:00:00:00:00") in remoteDevicesMap_
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDeviceAlias_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDeviceAlias_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_NE(device_.SetDeviceAlias("deviceName"), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDeviceAlias_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDeviceAlias_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDeviceAlias_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDeviceAlias_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_NE(device_.SetDeviceAlias("deviceName"), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDeviceAlias_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceTransport_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceTransport_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceTransport_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int32_t transport = 3;
    EXPECT_EQ(device_.GetDeviceTransport(transport), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceTransport_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceCustomType_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceCustomType_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceCustomType_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int32_t customType = -1;
    EXPECT_EQ(device_.GetDeviceCustomType(customType), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceCustomType_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceCustomType_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceCustomType_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceCustomType_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    int32_t customType = -1;
    EXPECT_EQ(device_.GetDeviceCustomType(customType), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceCustomType_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceDeviceOfType_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceDeviceOfType_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceDeviceOfType_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    uint8_t deviceOfType = 0xFF;
    EXPECT_EQ(device_.GetDeviceDeviceOfType(deviceOfType), BT_NO_ERROR);
    EXPECT_EQ(deviceOfType, 0);

    HILOGI("Remote_Device_ModuleTest_GetDeviceDeviceOfType_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceDeviceOfType_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceDeviceOfType_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceDeviceOfType_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    uint8_t deviceOfType = 0xFF;
    EXPECT_EQ(device_.GetDeviceDeviceOfType(deviceOfType), BT_NO_ERROR);
    EXPECT_EQ(deviceOfType, 0);

    HILOGI("Remote_Device_ModuleTest_GetDeviceDeviceOfType_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDeviceCustomType_00100
 * @tc.name:
 * @tc.desc: operation fail because could not find addr("00:00:00:00:00:00") in remoteDevicesMap_
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDeviceCustomType_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDeviceCustomType_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_NE(device_.SetDeviceCustomType(0), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDeviceCustomType_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDeviceCustomType_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDeviceCustomType_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDeviceCustomType_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_NE(device_.SetDeviceCustomType(0), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDeviceCustomType_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceType_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceType_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceType_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device_.GetDeviceType(), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceType_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceType_00200
 * @tc.name: GetDeviceType()
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceType_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceType_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_EQ(device_.GetDeviceType(), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceType_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetRemoteDeviceBatteryInfo_00100
 * @tc.name:
 * @tc.desc: "00:00:00:00:00:00" is not found in remoteDevicesMap_, just return 0
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetRemoteDeviceBatteryInfo_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetRemoteDeviceBatteryInfo_00100 start");

    DeviceBatteryInfo batteryInfo;
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device_.GetRemoteDeviceBatteryInfo(batteryInfo), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetRemoteDeviceBatteryInfo_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceBatteryLevel_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetRemoteDeviceBatteryInfo_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetRemoteDeviceBatteryInfo_00200 start");

    DeviceBatteryInfo batteryInfo;
    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_EQ(device_.GetRemoteDeviceBatteryInfo(batteryInfo), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetRemoteDeviceBatteryInfo_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetPairState_00100
 * @tc.name:
 * @tc.desc: "00:00:00:00:00:00" is not found in remoteDevicesMap_, just return PAIR_NONE
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetPairState_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetPairState_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int pairState;
    EXPECT_EQ(device_.GetPairState(pairState), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetPairState_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetPairState_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetPairState_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetPairState_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    int pairState;
    EXPECT_EQ(device_.GetPairState(pairState), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetPairState_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_StartPair_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_StartPair_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_StartPair_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device_.StartPair(), BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_StartPair_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_StartPair_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_StartPair_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_StartPair_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_EQ(device_.StartPair(), BT_ERR_INTERNAL_ERROR);
    HILOGI("Remote_Device_ModuleTest_StartPair_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_CancelPairing_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_CancelPairing_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_CancelPairing_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    std::this_thread::sleep_for(std::chrono::seconds(6));
    EXPECT_EQ(device_.CancelPairing(), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_CancelPairing_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_CancelPairing_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_CancelPairing_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_CancelPairing_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    std::this_thread::sleep_for(std::chrono::seconds(6));
    EXPECT_EQ(device_.CancelPairing(), BT_ERR_DEVICE_NOT_PAIRING);

    HILOGI("Remote_Device_ModuleTest_CancelPairing_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_IsBondedFromLocal_00100
 * @tc.name:
 * @tc.desc: return bondedFromLocal_, default value is 0
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IsBondedFromLocal_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_IsBondedFromLocal_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(device_.IsBondedFromLocal());

    HILOGI("Remote_Device_ModuleTest_IsBondedFromLocal_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_IsBondedFromLocal_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IsBondedFromLocal_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_IsBondedFromLocal_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_FALSE(device_.IsBondedFromLocal());

    HILOGI("Remote_Device_ModuleTest_IsBondedFromLocal_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_IsAclConnected_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IsAclConnected_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_IsAclConnected_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(device_.IsAclConnected());

    HILOGI("Remote_Device_ModuleTest_IsAclConnected_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_IsAclConnected_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IsAclConnected_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_IsAclConnected_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_FALSE(device_.IsAclConnected());

    HILOGI("Remote_Device_ModuleTest_IsAclConnected_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_IsAclEncrypted_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IsAclEncrypted_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_IsAclEncrypted_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(device_.IsAclEncrypted());

    HILOGI("Remote_Device_ModuleTest_IsAclEncrypted_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_IsAclEncrypted_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IsAclEncrypted_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_IsAclEncrypted_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_FALSE(device_.IsAclEncrypted());

    HILOGI("Remote_Device_ModuleTest_IsAclEncrypted_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceClass_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceClass_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceClass_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int cod = 0;
    (void)device_.GetDeviceClass(cod);
    BluetoothDeviceClass getLocalcod = BluetoothDeviceClass(cod);
    EXPECT_EQ(getLocalcod.GetMajorClass(), 0);

    HILOGI("Remote_Device_ModuleTest_GetDeviceClass_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceClass_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceClass_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceClass_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    int cod = 0;
    (void)device_.GetDeviceClass(cod);
    BluetoothDeviceClass getLocalcod = BluetoothDeviceClass(cod);
    EXPECT_EQ(getLocalcod.GetMajorClass(), 0);

    HILOGI("Remote_Device_ModuleTest_GetDeviceClass_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceClass_00300
 * @tc.name:
 * @tc.desc: funtion test
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceClass_00300, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceClass_00300 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int cod = 0;
    (void)device_.GetDeviceClass(cod);
    BluetoothDeviceClass getLocalcod = BluetoothDeviceClass(cod);
    EXPECT_EQ(getLocalcod.GetClassOfDevice(), 0);
    HILOGI("Remote_Device_ModuleTest_GetDeviceClass_00300 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceUuids_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceUuids_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceUuids_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    std::vector<std::string> uuids{};
    EXPECT_EQ(device_.GetDeviceUuids(uuids), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceUuids_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceUuids_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceUuids_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceUuids_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    std::vector<std::string> uuids{};
    EXPECT_EQ(device_.GetDeviceUuids(uuids), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceUuids_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDevicePin_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDevicePin_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDevicePin_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(device_.SetDevicePin("000000") == BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDevicePin_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDevicePin_00101
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDevicePin_00101, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDevicePin_00101 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(device_.SetDevicePin("000000000000000000000000") == BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDevicePin_00101 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDevicePin_00102
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDevicePin_00102, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDevicePin_00102 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_FALSE(device_.SetDevicePin("000000") == BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDevicePin_00102 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDevicePairingConfirmation_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDevicePairingConfirmation_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDevicePairingConfirmation_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_NE(device_.SetDevicePairingConfirmation(true), BT_NO_ERROR);
    EXPECT_NE(device_.SetDevicePairingConfirmation(true), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDevicePairingConfirmation_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDevicePairingConfirmation_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDevicePairingConfirmation_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDevicePairingConfirmation_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_NE(device_.SetDevicePairingConfirmation(true), BT_NO_ERROR);
    EXPECT_NE(device_.SetDevicePairingConfirmation(true), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_SetDevicePairingConfirmation_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDevicePasskey_00100
 * @tc.name:
 * @tc.desc: SetDevicePasskey return false for BR
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDevicePasskey_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDevicePasskey_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(device_.SetDevicePasskey(123456, true));
    EXPECT_FALSE(device_.SetDevicePasskey(123456, false));

    HILOGI("Remote_Device_ModuleTest_SetDevicePasskey_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_SetDevicePasskey_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetDevicePasskey_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetDevicePasskey_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_FALSE(device_.SetDevicePasskey(123456, true));
    EXPECT_FALSE(device_.SetDevicePasskey(123456, false));

    HILOGI("Remote_Device_ModuleTest_SetDevicePasskey_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_PairRequestReply_00100
 * @tc.name:
 * @tc.desc:  PairRequestReply return false for BR type;
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_PairRequestReply_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_PairRequestReply_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(device_.PairRequestReply(true));
    EXPECT_FALSE(device_.PairRequestReply(false));

    HILOGI("Remote_Device_ModuleTest_PairRequestReply_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_PairRequestReply_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_PairRequestReply_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_PairRequestReply_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_FALSE(device_.PairRequestReply(true));
    EXPECT_FALSE(device_.PairRequestReply(false));

    HILOGI("Remote_Device_ModuleTest_PairRequestReply_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetTransportType_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetTransportType_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetTransportType_00100 start");

    BluetoothRemoteDevice device1_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device1_.GetTransportType(), BT_TRANSPORT_BREDR);

    BluetoothRemoteDevice device2_("00:00:00:00:00:00", BT_TRANSPORT_BLE);
    EXPECT_EQ(device2_.GetTransportType(), BT_TRANSPORT_BLE);

    BluetoothRemoteDevice device3_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_EQ(device3_.GetTransportType(), INVALID_TYPE);

    HILOGI("Remote_Device_ModuleTest_GetTransportType_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_ReadRemoteRssiValue_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_ReadRemoteRssiValue_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_ReadRemoteRssiValue_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_TRUE(device_.ReadRemoteRssiValue());

    HILOGI("Remote_Device_ModuleTest_ReadRemoteRssiValue_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_ReadRemoteRssiValue_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_ReadRemoteRssiValue_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_ReadRemoteRssiValue_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    EXPECT_TRUE(device_.ReadRemoteRssiValue());

    HILOGI("Remote_Device_ModuleTest_ReadRemoteRssiValue_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_IsValidBluetoothRemoteDevice_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IsValidBluetoothRemoteDevice_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_IsValidBluetoothRemoteDevice_00100 start");

    BluetoothRemoteDevice device0_("00", BT_TRANSPORT_BREDR);
    EXPECT_FALSE(device0_.IsValidBluetoothRemoteDevice());
    BluetoothRemoteDevice device1_("00:00:00:00:00:00", 3);
    device1_.IsValidBluetoothRemoteDevice();
    HILOGI("Remote_Device_ModuleTest_IsValidBluetoothRemoteDevice_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_PairCredibleDevice_00100
 * @tc.name: StartCrediblePair
 * @tc.desc: Check BT Status
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_PairCredibleDevice_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device_.StartCrediblePair(), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_PairCredibleDevice_00101
 * @tc.name: StartCrediblePair
 * @tc.desc: Check Is Valid Device
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_PairCredibleDevice_00101, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00101 start");

    BluetoothRemoteDevice device_("0:00:00:00:00:0055", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device_.StartCrediblePair(), BT_ERR_ADDRESS_OR_TRANSPORT_ERROR);

    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00101 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_PairCredibleDevice_00103
 * @tc.name: StartCrediblePair
 * @tc.desc: Ble device StartCrediblePair
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_PairCredibleDevice_00103, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00103 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:01", BT_TRANSPORT_BLE);
    EXPECT_EQ(device_.StartCrediblePair(), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00103 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_PairCredibleDevice_00104
 * @tc.name: StartCrediblePair
 * @tc.desc: Check Valid Transport
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_PairCredibleDevice_00104, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00104 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:02", 6);
    device_.StartCrediblePair();
    EXPECT_NE(device_.address_, "00");

    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00104 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_PairCredibleDevice_00105
 * @tc.name: StartCrediblePair
 * @tc.desc: Check AddPairWhiteList
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_PairCredibleDevice_00105, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00105 start");

    BluetoothRemoteDevice device1("00:00:00:00:00:03", BT_TRANSPORT_BLE);
    EXPECT_NE(device1.StartCrediblePair(), BT_NO_ERROR);
    sleep(TIME);
    EXPECT_NE(device1.StartCrediblePair(), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00105 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_PairCredibleDevice_00106
 * @tc.name: StartCrediblePair
 * @tc.desc: Check WhiltListCallTimeOut
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_PairCredibleDevice_00106, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00106 start");

    sleep(TIME);
    BluetoothRemoteDevice device1("00:00:00:00:00:04", BT_TRANSPORT_BLE);
    EXPECT_NE(device1.StartCrediblePair(), BT_NO_ERROR);
    sleep(TIME_10);

    HILOGI("Remote_Device_ModuleTest_PairCredibleDevice_00106 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceProductType_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceProductType_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceProductType_00100 start");

    BluetoothRemoteDevice device_("abcdabcd", BT_TRANSPORT_BREDR);
    int cod = 0;
    int majorClass = 0;
    int majorMinorClass = 0;
    int ret = device_.GetDeviceProductType(cod, majorClass, majorMinorClass);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    HILOGI("Remote_Device_ModuleTest_GetDeviceProductType_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceProductType_00200
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceProductType_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceProductType_00200 start");

    BluetoothRemoteDevice device_("11:11:11:11:11:11", BT_TRANSPORT_BREDR);
    int cod = 0;
    int majorClass = 0;
    int majorMinorClass = 0;
    EXPECT_EQ(device_.GetDeviceProductType(cod, majorClass, majorMinorClass), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceProductType_00200 end");
}

/**
 * @tc.number: GetDeviceProductId_00100
 * @tc.name: GetDeviceProductId
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceProductId_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceProductId_00100 start");
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    std::string productId;
    EXPECT_EQ(device_.GetDeviceProductId(productId), BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_GetDeviceProductId_00100 end");
}

/**
 * @tc.number: SetVirtualAutoConnectType_00100
 * @tc.name: SetVirtualAutoConnectType
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_SetVirtualAutoConnectType_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetVirtualAutoConnectType_00100 start");
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int connType = 0;
    int businessType = 0;
    EXPECT_EQ(device_.SetVirtualAutoConnectType(connType, businessType), BT_ERR_INTERNAL_ERROR);
    HILOGI("Remote_Device_ModuleTest_SetVirtualAutoConnectType_00100 end");
}

/**
 * @tc.number: IsSupportVirtualAutoConnect_00100
 * @tc.name: IsSupportVirtualAutoConnect
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_IsSupportVirtualAutoConnect_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_IsSupportVirtualAutoConnect_00100 start");
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    bool outSupport = true;
    EXPECT_EQ(device_.IsSupportVirtualAutoConnect(outSupport), BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_IsSupportVirtualAutoConnect_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceVendorId_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceVendorId_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceVendorId_00100 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    uint16_t vendorId = 0;
    EXPECT_EQ(device_.GetDeviceVendorId(vendorId), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceVendorId_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceVendorId_00200
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceVendorId_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceVendorId_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BLE);
    uint16_t vendorId = 0;
    EXPECT_EQ(device_.GetDeviceVendorId(vendorId), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceVendorId_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceVendorId_00300
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceVendorId_00300, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceVendorId_00300 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    uint16_t vendorId = 0;
    EXPECT_EQ(device_.GetDeviceVendorId(vendorId), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceVendorId_00300 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceVendorId_00200
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceProductId_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceVendorId_00200 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    uint16_t productId = 0;
    EXPECT_EQ(device_.GetDeviceProductId(productId), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceVendorId_00200 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceProductId_00300
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceProductId_00300, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceProductId_00300 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BLE);
    uint16_t productId = 0;
    EXPECT_EQ(device_.GetDeviceProductId(productId), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceProductId_00300 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceProductId_00400
 * @tc.name:
 * @tc.desc: IsValidBluetoothRemoteDevice():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceProductId_00400, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceProductId_00400 start");

    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    uint16_t productId = 0;
    EXPECT_EQ(device_.GetDeviceProductId(productId), BT_NO_ERROR);

    HILOGI("Remote_Device_ModuleTest_GetDeviceProductId_00400 end");
}

/**
 * @tc.number: GetDeviceManufacturerData_00100
 * @tc.name: GetDeviceManufacturerData
 * @tc.desc: Normal case - Get manufacture data with BT_TRANSPORT_BREDR
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceManufacturerData_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceManufacturerData_00100 start");
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    std::vector<uint8_t> manufacturerData;
    int32_t ret = device_.GetDeviceManufacturerData(manufacturerData);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_GetDeviceManufacturerData_00100 end");
}

/**
 * @tc.number: GetDeviceManufacturerData_00200
 * @tc.name: GetDeviceManufacturerData
 * @tc.desc: Get manufacture data with BT_TRANSPORT_BLE
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceManufacturerData_00200, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceManufacturerData_00200 start");
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BLE);
    std::vector<uint8_t> manufacturerData;
    int32_t ret = device_.GetDeviceManufacturerData(manufacturerData);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_GetDeviceManufacturerData_00200 end");
}

/**
 * @tc.number: GetDeviceManufacturerData_00300
 * @tc.name: GetDeviceManufacturerData
 * @tc.desc: Invalid device type - INVALID_TYPE
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceManufacturerData_00300, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceManufacturerData_00300 start");
    BluetoothRemoteDevice device_("00:00:00:00:00:00", INVALID_TYPE);
    std::vector<uint8_t> manufacturerData;
    int32_t ret = device_.GetDeviceManufacturerData(manufacturerData);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_GetDeviceManufacturerData_00300 end");
}

/**
 * @tc.number: GetDeviceManufacturerData_00400
 * @tc.name: GetDeviceManufacturerData
 * @tc.desc: Test that empty vector is returned when info is null
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceManufacturerData_00400, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceManufacturerData_00400 start");
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    std::vector<uint8_t> manufacturerData;
    int32_t ret = device_.GetDeviceManufacturerData(manufacturerData);
    EXPECT_EQ(ret, BT_NO_ERROR);
    EXPECT_TRUE(manufacturerData.empty());
    HILOGI("Remote_Device_ModuleTest_GetDeviceManufacturerData_00400 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_ControlDeviceAction_00100
 * @tc.name: ControlDeviceAction
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_ControlDeviceAction_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_ControlDeviceAction_00100 start");
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    uint32_t controlType = 0;
    uint32_t controlTypeVal = 1;
    uint32_t controlObject = 2;
    EXPECT_NE(device_.ControlDeviceAction(controlType, controlTypeVal, controlObject), BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_ControlDeviceAction_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetLastConnectionTime_00100
 * @tc.name: GetLastConnectionTime
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetLastConnectionTime_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetLastConnectionTime_00100 start");
    BluetoothRemoteDevice device_("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int64_t connectionTime = 0;
    EXPECT_EQ(device_.GetLastConnectionTime(connectionTime), BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_GetLastConnectionTime_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetCloudBondState_00100
 * @tc.name: GetCloudBondState
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetCloudBondState_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetCloudBondState_00100 start");
    BluetoothRemoteDevice device_("11:22:33:44:55:66", BT_TRANSPORT_BREDR);
    int32_t cloudBondState = 0;
    EXPECT_EQ(device_.GetCloudBondState(cloudBondState), BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_GetCloudBondState_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetDeviceName_00600
 * @tc.name:
 * @tc.desc: IS_BT_ENABLED():false
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetDeviceName_00600, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00600 start");
    g_BluetoothState = bluetoothHost_->GetBtState();
    if ((g_BluetoothState != BTStateID::STATE_TURNING_OFF) && (g_BluetoothState != BTStateID::STATE_TURN_OFF)) {
        bluetoothHost_->DisableBt();
        bluetoothHost_->DisableBle();
        sleep(TIME);
    }
    BluetoothRemoteDevice device_("00:00:00:00:00:00", BT_TRANSPORT_BREDR);
    int cod = 0;
    int majorClass = 0;
    int majorMinorClass = 0;
    EXPECT_EQ(device_.GetDeviceProductType(cod, majorClass, majorMinorClass), BT_ERR_INVALID_STATE);
    EXPECT_TRUE(device_.GetDeviceName().empty());
    std::string name = INVALID_NAME;
    EXPECT_EQ(device_.GetDeviceName(name), BT_ERR_INVALID_STATE);
    EXPECT_EQ(device_.StartPair(), BT_ERR_INVALID_STATE);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(device_.CancelPairing(), BT_ERR_INVALID_STATE);
    (void)device_.GetDeviceClass(cod);
    BluetoothDeviceClass getLocalcod = BluetoothDeviceClass(cod);
    EXPECT_EQ(getLocalcod.GetMajorClass(), 0);
    EXPECT_EQ(device_.SetDevicePairingConfirmation(true), BT_ERR_INVALID_STATE);
    EXPECT_EQ(device_.SetDevicePairingConfirmation(false), BT_ERR_INVALID_STATE);
    EXPECT_EQ(device_.StartCrediblePair(), BT_ERR_INVALID_STATE);
    bluetoothHost_->EnableBle();
    bluetoothHost_->EnableBt();
    sleep(TIME);

    HILOGI("Remote_Device_ModuleTest_GetDeviceName_00600 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_StartRemoteSdpSearch_00100
 * @tc.name: StartRemoteSdpSearch
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_StartRemoteSdpSearch_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_StartRemoteSdpSearch_00100 start");
    BluetoothRemoteDevice device_("22:33:44:55:66:99", BT_TRANSPORT_BREDR);
    std::string uuid = "00001111-0000-1000-8000-00805f9b34fb";
    EXPECT_EQ(device_.StartRemoteSdpSearch(uuid), true);
    HILOGI("Remote_Device_ModuleTest_StartRemoteSdpSearch_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_GetRemoteServices_00100
 * @tc.name: GetRemoteServices
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_GetRemoteServices_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_StartRemoteSdpSearch_00100 start");
    BluetoothRemoteDevice device_("22:33:44:55:66:99", BT_TRANSPORT_BREDR);
    EXPECT_EQ(device_.GetRemoteServices(), true);
    HILOGI("Remote_Device_ModuleTest_StartRemoteSdpSearch_00100 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_StartPairOutOfBand_001
 * @tc.name: StartPairOutOfBand
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_StartPairOutOfBand_001, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_StartPairOutOfBand_001 start");
    uint8_t addressType = AddressType::REAL_ADDRESS;
    std::string remoteAddr = "11:22:33:44:55:66";
    int32_t transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice device_(addressType, remoteAddr, transport);
    OobData oobData;
    EXPECT_EQ(device_.StartPairOutOfBand(oobData), BT_ERR_INTERNAL_ERROR);
    HILOGI("Remote_Device_ModuleTest_StartPairOutOfBand_001 end");
}

/**
 * @tc.number: Remote_Device_ModuleTest_StartPairOutOfBand_002
 * @tc.name: StartPairOutOfBand
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, Remote_Device_ModuleTest_StartPairOutOfBand_002, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_StartPairOutOfBand_002 start");
    uint8_t addressType = AddressType::REAL_ADDRESS;
    std::string remoteAddr = "11:22:33:44:55:66";
    AddressInfo addressInfo;
    addressInfo.SetAddress(remoteAddr);
    addressInfo.SetAddressType(addressType);
    int32_t transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice device_(addressInfo, transport);
    OobData oobData;
    std::vector<uint8_t> confirmHash = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10};
    std::vector<uint8_t> addressWithType = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01};
    oobData.SetConfirmationHash(confirmHash);
    oobData.SetAddressWithType(addressWithType);
    EXPECT_EQ(device_.StartPairOutOfBand(oobData), BT_NO_ERROR);
    HILOGI("Remote_Device_ModuleTest_StartPairOutOfBand_002 end");
}

/**
 * @tc.number: OobData_001
 * @tc.name: OobData
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, OobData_001, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_OobData_001 start");
    OobData oobData;
    std::vector<uint8_t> confirmHash = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10};
    std::vector<uint8_t> randomHash = {0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
        0x0F, 0x10, 0x11};
    std::vector<uint8_t> addressWithType = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01};
    oobData.SetConfirmationHash(confirmHash);
    oobData.SetRandomizerHash(randomHash);
    oobData.SetAddressWithType(addressWithType);
    oobData.SetOobDataType(OobDataType::P192);
    oobData.SetDeviceRole(LeDeviceRole::DEVICE_ROLE_BOTH_PREFER_CENTRAL);
    oobData.SetDeviceName("testName");
    EXPECT_TRUE(oobData.HasOobData());
    EXPECT_EQ(oobData.GetRandomizerHash(), randomHash);
    EXPECT_EQ(oobData.GetConfirmationHash(), confirmHash);
    EXPECT_EQ(oobData.GetAddressWithType(), addressWithType);
    EXPECT_EQ(oobData.GetOobDataType(), OobDataType::P192);
    EXPECT_EQ(oobData.GetDeviceRole(), LeDeviceRole::DEVICE_ROLE_BOTH_PREFER_CENTRAL);
    EXPECT_EQ(oobData.GetDeviceName(), "testName");
    HILOGI("Remote_Device_ModuleTest_OobData_001 end");
}

/**
 * @tc.number: SetRemoteDeviceBatteryInfo_001
 * @tc.name: SetRemoteDeviceBatteryInfo
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceTest, SetRemoteDeviceBatteryInfo_001, TestSize.Level1)
{
    HILOGI("Remote_Device_ModuleTest_SetRemoteDeviceBatteryInfo_001 start");
    BluetoothRemoteDevice device_("22:33:44:55:66:99", BT_TRANSPORT_BREDR);
    DeviceBatteryInfo info;
    info.batteryLevel_ = 50;
    info.leftEarBatteryLevel_ = 50;
    info.leftEarChargeState_ = DeviceChargeState::DEVICE_NORMAL_CHARGE_IN_CHARGING;
    info.rightEarBatteryLevel_ = 50;
    info.rightEarChargeState_ = DeviceChargeState::DEVICE_NORMAL_CHARGE_IN_CHARGING;
    info.boxBatteryLevel_ = 50;
    info.boxChargeState_ = DeviceChargeState::DEVICE_NORMAL_CHARGE_IN_CHARGING;
    EXPECT_EQ(device_.SetRemoteDeviceBatteryInfo(info), BT_ERR_PERMISSION_FAILED); // This API is only for audio_server
    HILOGI("Remote_Device_ModuleTest_SetRemoteDeviceBatteryInfo_001 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
