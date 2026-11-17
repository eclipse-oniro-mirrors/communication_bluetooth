/*
 * Copyright (c) 2012 Huawei Device Co., Ltd. All rights reserved.
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
#include "bluetooth_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

static BluetoothHost *host_;
constexpr int TIME = 2;
std::string deviceName_ {};
std::string aliasName_ {};
const int PAIR_TDDTEST1 = 0x00;
const int PAIR_TDDTEST2 = 0x02;
const int PAIR_TDDTEST3 = 0x05;

class BtDeviceTest : public testing::Test {
public:
    BtDeviceTest() {}
    ~BtDeviceTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    std::unique_ptr<bluetooth::BluetoothDevice> pBluetoothDevice;
};

void BtDeviceTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    HILOGI("isBleEnable = %{public}d", isBleEnable);

    if (!isBleEnable) {
        host_->EnableBle();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}
void BtDeviceTest::TearDownTestCase(void)
{
    host_ = nullptr;
    utility::CloseAllTimer();
}
void BtDeviceTest::SetUp()
{
    pBluetoothDevice = std::make_unique<bluetooth::BluetoothDevice>();
}
void BtDeviceTest::TearDown()
{
    pBluetoothDevice.reset();
}
/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetRemoteName_00100
 * @tc.name: SetRemoteName/GetRemoteName
 * @tc.desc: name.length() >= MAX_REMOTE_BT_NAME_LEN
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetRemoteName_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetRemoteName_00100 start");

    std::string name = "Device_Unit_Test_Set_Get_Remote_Name_Repeat_1_Device_Unit_Test_Set_Get_Remote_Name_Repeat_2_" \
            "Device_Unit_Test_Set_Get_Remote_Name_Repeat_3_Device_Unit_Test_Set_Get_Remote_Name_Repeat_4_" \
            "Device_Unit_Test_Set_Get_Remote_Name_Repeat_5_Device_Unit_Test_Set_Get_Remote_Name_Repeat_6";
    pBluetoothDevice->SetRemoteName(name);
    std::string getName = pBluetoothDevice->GetRemoteName(true);
    EXPECT_EQ(getName, name.substr(0, bluetooth::MAX_REMOTE_BT_NAME_LEN));

    HILOGI("BTDEVICE_ModuleTest_SetGetRemoteName_00100 end getName:%{public}s", getName.c_str());
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetRemoteName_00200
 * @tc.name: SetRemoteName/GetRemoteName
 * @tc.desc: name : "Device_Unit_Test"
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetRemoteName_00200, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetRemoteName_00200 start");

    std::string name = "Device_Unit_Test";
    pBluetoothDevice->SetRemoteName(name);
    EXPECT_EQ(pBluetoothDevice->GetRemoteName(true), name);
    EXPECT_EQ(pBluetoothDevice->GetRemoteName(false), name);
    HILOGI("BTDEVICE_ModuleTest_SetGetRemoteName_00200 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetRemoteName_00300
 * @tc.name: SetAliasName/GetRemoteName
 * @tc.desc: name : "Device_Alias_Unit_Test"
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetRemoteName_00300, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetRemoteName_00300 start");

    std::string name = "Device_Alias_Unit_Test";
    pBluetoothDevice->SetAliasName(name);
    EXPECT_EQ(pBluetoothDevice->GetRemoteName(true), name);
    EXPECT_NE(pBluetoothDevice->GetRemoteName(false), name);
    HILOGI("BTDEVICE_ModuleTest_SetGetRemoteName_00100 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetAliasName_00100
 * @tc.name: SetAliasName/GetAliasName
 * @tc.desc: name : null
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetAliasName_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetAliasName_00100 start");

    std::string name = "";
    pBluetoothDevice->SetAliasName(name);
    EXPECT_NE(pBluetoothDevice, nullptr);
    HILOGI("BTDEVICE_ModuleTest_SetGetAliasName_00100 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetAliasName_00200
 * @tc.name: SetAliasName/GetAliasName
 * @tc.desc: name.length() >= MAX_REMOTE_BT_NAME_LEN
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetAliasName_00200, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetAliasName_00200 start");
    std::string name = "Device_Unit_Test_Set_Get_Remote_Name_Repeat_1_Device_Unit_Test_Set_Get_Remote_Name_Repeat_2_" \
            "Device_Unit_Test_Set_Get_Remote_Name_Repeat_3_Device_Unit_Test_Set_Get_Remote_Name_Repeat_4_" \
            "Device_Unit_Test_Set_Get_Remote_Name_Repeat_5_Device_Unit_Test_Set_Get_Remote_Name_Repeat_6";
    EXPECT_TRUE(pBluetoothDevice->SetAliasName(name));
    std::string getName = pBluetoothDevice->GetAliasName();
    EXPECT_EQ(getName, name.substr(0, bluetooth::MAX_REMOTE_BT_NAME_LEN));

    HILOGI("BTDEVICE_ModuleTest_SetGetAliasName_00200 end getName:%{public}s", getName.c_str());
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetAliasName_00300
 * @tc.name: SetAliasName/GetAliasName
 * @tc.desc: name : "Device_Unit_Test"
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetAliasName_00300, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetAliasName_00300 start");

    std::string name = "Device_Unit_Test";
    EXPECT_TRUE(pBluetoothDevice->SetAliasName(name));
    std::string getName = pBluetoothDevice->GetAliasName();
    EXPECT_EQ(getName, name);

    HILOGI("BTDEVICE_ModuleTest_SetGetAliasName_00300 end getName:%{public}s", getName.c_str());
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetAliasName_00400
 * @tc.name: SetAliasName/GetAliasName
 * @tc.desc: GetAliasName() : name : ""
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetAliasName_00400, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetAliasName_00400 start");

    std::string aliasName = "";
    std::string deviceName = "Device_Unit_Test";

    pBluetoothDevice->SetRemoteName(deviceName);

    pBluetoothDevice->SetAliasName(aliasName);
    pBluetoothDevice->GetAliasName();
    EXPECT_NE(pBluetoothDevice, nullptr);

    HILOGI("BTDEVICE_ModuleTest_SetGetAliasName_00400 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetDeviceClass_00100
 * @tc.name: SetDeviceClass/GetDeviceClass
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetDeviceClass_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetDeviceClass_00100 start");

    int codSet = 0x0200;
    pBluetoothDevice->SetDeviceClass(codSet);
    int codGet = pBluetoothDevice->GetDeviceClass();
    EXPECT_EQ(codGet, codSet);

    HILOGI("BTDEVICE_ModuleTest_SetGetDeviceClass_00100 end codGet:%{public}d", codGet);
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetPairedStatus_00100
 * @tc.name: SetPairedStatus/GetPairedStatus
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetPairedStatus_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetPairedStatus_00200 start");

    EXPECT_FALSE(pBluetoothDevice->SetPairedStatus(PAIR_TDDTEST1));
    EXPECT_FALSE(pBluetoothDevice->SetPairedStatus(PAIR_TDDTEST3));
    EXPECT_TRUE(pBluetoothDevice->SetPairedStatus(PAIR_TDDTEST2));
    EXPECT_EQ(pBluetoothDevice->GetPairedStatus(), PAIR_TDDTEST2);
    HILOGI("BTDEVICE_ModuleTest_SetPairedStatus_00200 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetCustomType_00100
 * @tc.name: SetCustomType/GetCustomType
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetCustomType_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetCustomType_00100 start");

    int customTypeSet = 2;
    pBluetoothDevice->SetCustomType(customTypeSet);
    int customGet = pBluetoothDevice->GetCustomType();
    EXPECT_EQ(customGet, customTypeSet);

    HILOGI("BTDEVICE_ModuleTest_SetGetCustomType_00100 end customGet:%{public}d", customGet);
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetGetAutoConnSwitch_00100
 * @tc.name: SetAutoConnSwitch/GetAutoConnSwitch
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetGetAutoConnSwitch_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetGetAutoConnSwitch_00100 start");

    int autoConnSwitchSet = 9;
    pBluetoothDevice->SetAutoConnSwitch(autoConnSwitchSet);
    int autoConnSwitchGet = pBluetoothDevice->GetAutoConnSwitch();
    EXPECT_EQ(autoConnSwitchGet, autoConnSwitchSet);

    HILOGI("BTDEVICE_ModuleTest_SetGetAutoConnSwitch_00100 end autoConnSwitchGet:%{public}d", autoConnSwitchGet);
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetNeedDelayConnectFlag_00100
 * @tc.name: SetNeedDelayConnectFlag/IsNeedDelayConnect
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetNeedDelayConnectFlag_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetNeedDelayConnectFlag_00100 start");

    pBluetoothDevice->SetNeedDelayConnectFlag(false);
    bool flag = pBluetoothDevice->IsNeedDelayConnect();
    EXPECT_EQ(flag, false);

    HILOGI("BTDEVICE_ModuleTest_SetNeedDelayConnectFlag_00100 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetBondedFromLocal_00100
 * @tc.name: SetBondedFromLocal
 * @tc.desc: name : null
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetBondedFromLocal_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetBondedFromLocal_00100 start");

    pBluetoothDevice->SetBondedFromLocal(true);
    EXPECT_NE(pBluetoothDevice, nullptr);
    HILOGI("BTDEVICE_ModuleTest_SetBondedFromLocal_00100 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_DialogControl_00100
 * @tc.name: DialogControl
 * @tc.desc: name : null
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_DialogControl_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_DialogControl_00100 start");

    pBluetoothDevice->DialogControlSetActivePair(true);
    bool dialogControlActivePair = pBluetoothDevice->DialogControlIsActivePair();
    EXPECT_EQ(dialogControlActivePair, true);
    pBluetoothDevice->DialogControlAddConnectingProfileCount();
    pBluetoothDevice->DialogControlConvertCause(1);
    pBluetoothDevice->DialogControlRemoveConnectingProfileCount(0);
    pBluetoothDevice->DialogControlClearConnectingProfileCount();
    dialogControlActivePair = pBluetoothDevice->DialogControlIsActivePair();
    EXPECT_EQ(dialogControlActivePair, false);
    HILOGI("BTDEVICE_ModuleTest_DialogControl_00100 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_SetHwTalkBandBandMode_00100
 * @tc.name: SetHwTalkBandBandMode
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_SetHwTalkBandBandMode_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_SetHwTalkBandBandMode_00100 start");

    pBluetoothDevice->SetHwTalkBandBandMode(0);
    EXPECT_TRUE(pBluetoothDevice->isBandMode());
    EXPECT_TRUE(pBluetoothDevice->isBandDevice());

    HILOGI("BTDEVICE_ModuleTest_SetHwTalkBandBandMode_00100 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_GetVersion_00100
 * @tc.name: GetVersion
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_GetVersion_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_GetVersion_00100 start");

    EXPECT_EQ(pBluetoothDevice->GetVersion(), 0);
    pBluetoothDevice->SetVersion(1);
    EXPECT_EQ(pBluetoothDevice->GetVersion(), 1);
    HILOGI("BTDEVICE_ModuleTest_GetVersion_00100 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_GetLastConnectionTime_00100
 * @tc.name: GetLastConnectionTime
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_GetLastConnectionTime_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_GetLastConnectionTime_00100 start");

    pBluetoothDevice->SetConnectionTime(1);
    EXPECT_EQ(pBluetoothDevice->GetLastConnectionTime(), 1);

    HILOGI("BTDEVICE_ModuleTest_GetLastConnectionTime_00100 end");
}

/*
 * @tc.number: BTDEVICE_ModuleTest_IsCloudDevice_00100
 * @tc.name: IsCloudDevice
 * @tc.desc: function test
 */
HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_IsCloudDevice_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_IsCloudDevice_00100 start");

    pBluetoothDevice->SetCloudDevice(true);
    EXPECT_TRUE(pBluetoothDevice->IsCloudDevice());

    HILOGI("BTDEVICE_ModuleTest_IsCloudDevice_00100 end");
}

HWTEST_F(BtDeviceTest, BTDEVICE_ModuleTest_IsConnectFromLocalChangeCause_00100, TestSize.Level1)
{
    HILOGI("BTDEVICE_ModuleTest_IsConnectFromLocalChangeCause_00100 start");

    int cause = 2;
    int localConnectCause = 1001;
    pBluetoothDevice->SetIsConnectFromLocal(true);
    int causeGet = pBluetoothDevice->IsConnectFromLocalChangeCause(cause);
    EXPECT_EQ(causeGet, localConnectCause);

    pBluetoothDevice->SetIsConnectFromLocal(false);
    causeGet = pBluetoothDevice->IsConnectFromLocalChangeCause(cause);
    EXPECT_EQ(causeGet, cause);

    HILOGI("BTDEVICE_ModuleTest_IsConnectFromLocalChangeCause_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS