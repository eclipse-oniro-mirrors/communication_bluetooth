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
#include "bluetooth_device_battery_manager.h"
#include "bluetooth_state_manager.h"
#include "hfp_ag_defines.h"
#include "raw_address.h"
#include "remote_device_properties.h"
#include "thread_util.h"

#include "profile_service_manager.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class BluetoothDeviceBatteryManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ThreadUtil::GetInstance().ChangeThreadState(
            THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::NOT_SWITCH_THREAD);
        auto manager = ProfileServiceManager::GetInstance();
        std::string profileName = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, profileName);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(addr1_));
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(addr2_));
    }
    void TearDown()
    {
        BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_ = "";
        BluetoothDeviceBatteryManager::GetInstance()->a2dpActiveDevice_ = "";
        BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_ = "";
        BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_ = -1;
        BluetoothDeviceBatteryManager::GetInstance()->connectedDevices_.Clear();
        RemoteDeviceProperties::GetInstance()->FreeMemory();
    }
public:
    std::string addr1_ = "00:11:22:33:44:55";
    std::string addr2_ = "11:22:33:33:22:11";
    std::string addr3_ = "01:11:22:33:44:55";
};

/**
 * @tc.name: bluetooth_device_battery_manager_test_001
 * @tc.desc: : ProcessBatteryCommond
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_001, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_001 enter");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+XEVENT"), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+AAA=1,1,8"), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_,
        "+IPHONEACCEV=1,1,8"), true);
    HILOGI("bluetooth_device_battery_manager_test_001 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_002
 * @tc.desc: : ProcessHwBatteryCommond
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_002, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_002 enter");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY", BATTERY_OBTAIN_TYPE_AT), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+AAA=1,1,8", BATTERY_OBTAIN_TYPE_AT), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,100,3,1,4,100,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_AT), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=6,2,100,3,1,4,100,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_AT), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+CLOSEVENDORBATTERY=2", BATTERY_OBTAIN_TYPE_AT), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+CLOSEVENDORBATTERY=3", BATTERY_OBTAIN_TYPE_AT), true);
    HILOGI("bluetooth_device_battery_manager_test_002 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_003
 * @tc.desc: : ProcessIphoneAccevBatteryEvent
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_003, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_003 enter");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessIphoneAccevBatteryEvent(addr1_, ""), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessIphoneAccevBatteryEvent(addr1_, "a"), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessIphoneAccevBatteryEvent(addr1_, "1,1"), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessIphoneAccevBatteryEvent(addr1_, "1,a,b"), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessIphoneAccevBatteryEvent(addr1_, "1,2,b"), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessIphoneAccevBatteryEvent(addr1_, "1,1,b"), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessIphoneAccevBatteryEvent(addr1_, "1,1,10"), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessIphoneAccevBatteryEvent(addr1_, "1,1,9"), true);
    HILOGI("bluetooth_device_battery_manager_test_003 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_004
 * @tc.desc: : ProcessHwBatteryInfo
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_004, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_004 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryInfo(addr1_, "", BATTERY_OBTAIN_TYPE_AT);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryInfo(addr1_, "6,2,100,3,1,4,100,5,1,6,100,7,0",
        BATTERY_OBTAIN_TYPE_AT);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_004 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_005
 * @tc.desc: : ParseBatteryInfo
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_005, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_005 enter");
    BatteryInfo info;
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ParseBatteryInfo("7,1,100,2,100,3,1,4,100,5,1,6,100,7,0",
        info), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ParseBatteryInfo("", info), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ParseBatteryInfo("a", info), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ParseBatteryInfo("1", info), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ParseBatteryInfo("1,a,b", info), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ParseBatteryInfo("1,1,b", info), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ParseBatteryInfo("1,1,101", info), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ParseBatteryInfo("1,1,100", info), true);
    HILOGI("bluetooth_device_battery_manager_test_005 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_006
 * @tc.desc: : CheckBatteryInfoValid
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_006, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_006 enter");
    BatteryInfo info;
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_NORMAL_DEVICE_BATTERY_LEVEL, 1), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_LEFT_EAR_BATTERY_LEVEL, 1), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_RIGHT_EAR_BATTERY_LEVEL, 1), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_BOX_BATTERY_LEVEL, 1), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_NORMAL_DEVICE_BATTERY_LEVEL, -1), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_NORMAL_DEVICE_BATTERY_LEVEL, 101), false);

    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_LEFT_EAR_CHARGE_STATE, 1), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_RIGHT_EAR_CHARGE_STATE, 1), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_BOX_CHARGE_STATE, 1), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_LEFT_EAR_CHARGE_STATE, -1), false);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->CheckBatteryInfoValid(
        KEY_LEFT_EAR_CHARGE_STATE, 4), false);
    HILOGI("bluetooth_device_battery_manager_test_006 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_007
 * @tc.desc: : UpdateBatteryInfoAndReport
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_007, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_007 enter");
    BatteryInfo info;
    BluetoothDeviceBatteryManager::GetInstance()->UpdateBatteryInfoAndReport(addr1_, BATTERY_OBTAIN_TYPE_AT, info);
    BluetoothDeviceBatteryManager::GetInstance()->UpdateBatteryInfoAndReport(addr1_, BATTERY_OBTAIN_TYPE_ECHO, info);
    BluetoothDeviceBatteryManager::GetInstance()->UpdateBatteryInfoAndReport(addr1_, 3, info);
    BluetoothDeviceBatteryManager::GetInstance()->UpdateBatteryInfoAndReport("", BATTERY_OBTAIN_TYPE_AT, info);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_007 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_008
 * @tc.desc: : ProcessActiveDeviceChanged
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_008, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_008 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("abc", addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_, "");

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("HfpAgService", addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("A2dpSrcService", addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->a2dpActiveDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("A2dpSrcService", "");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->a2dpActiveDevice_, "");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_008 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_009
 * @tc.desc: : ProcessAclStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_009, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_009 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, 3);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->connectedDevices_.Size(), 1);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->connectedDevices_.Size(), 0);
    HILOGI("bluetooth_device_battery_manager_test_009 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_010
 * @tc.desc: : ProcessHfpDisconnected
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_010, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_010 enter");
    BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_ = addr1_;
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected("");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_010 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_011
 * @tc.desc: :acl connect addr2(addr2)-->acl connect addr1(addr1)-->hfp active addr2(addr2)
                -->hfp disconnect addr2(addr1)-->acl disconnect addr1(addr2)->-->acl disconnect addr12()
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_011, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_011 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("HfpAgService", addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+VENDORBATTERY=6,2,100,3,1,4,100,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_011 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_012
 * @tc.desc: :acl connect addr1(addr1)-->acl connect addr2(addr2)-->a2dp active addr1(addr1)
                -->hfp active addr2(addr2)-->hfp de active(addr1)-->a2dp de active(addr2)
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_012, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_012 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,100,3,1,4,100,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+VENDORBATTERY=6,2,100,3,1,4,100,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("A2dpSrcService", addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("HfpAgService", addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("HfpAgService", "");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("A2dpSrcService", "");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_012 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_013
 * @tc.desc: :acl connect addr1(addr1)-->acl connect addr2(addr2)
            -->addr2 IPHONEACCECV(90)-->addr2 VENDORBATTERY (100)-->addr2 UPDATEVENDORBATTERY(85)
            -->addr2 UPDATEVENDORBATTERY(75)-->addr2 UPDATEVENDORBATTERY(90)-->HFP disconnect addr2(-1)
            -->addr1 UPDATEVENDORBATTERY(-1)-->acl disconnect addr2(addr1, 95)
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_013, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_013 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr2_, "+IPHONEACCEV=1,1,8");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 90);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+VENDORBATTERY=6,2,100,3,1,4,100,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+UPDATEVENDORBATTERY=6,2,100,3,1,4,85,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 85);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+UPDATEVENDORBATTERY=6,2,80,3,1,4,75,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 75);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+CLOSEVENDORBATTERY=2", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 75);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr2_);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+UPDATEVENDORBATTERY=3,3,1,5,1,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+UPDATEVENDORBATTERY=3,3,1,5,1,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=6,2,100,3,1,4,95,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 95);

    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 95);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_DISCONNECTED);
    HILOGI("bluetooth_device_battery_manager_test_013 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_014
 * @tc.desc: :acl connect addr1(addr1)-->addr1 IPHONEACCECV(90)-->addr1 VENDORBATTERY (80)-->hfp disconnect(-1)
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_014, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_014 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,8");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 90);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=1,1,90", BATTERY_OBTAIN_TYPE_AT);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 90);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=1,1,80", BATTERY_OBTAIN_TYPE_AT);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 80);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr1_);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_014 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_015
 * @tc.desc: : ProcessHfpDisconnected
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_015, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_015 enter");
    BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_ = addr2_;
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected("");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->hfpActiveDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_015 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_016
 * @tc.desc: : SetBatteryInfo
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_016, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_016 enter");
    BatteryInfo info;
    int32_t battery = 0;
    BluetoothDeviceBatteryManager::GetInstance()->SetBatteryInfo(KEY_EXCEPTION, battery, info);
    EXPECT_EQ(info.boxChargeState_, battery);
    HILOGI("bluetooth_device_battery_manager_test_016 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_017
 * @tc.desc: : addr1 connect, report battery 100, addr2 connect, close addr2 box, current device is addr1,
 *             device battery is addr1
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_017, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_017 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,100,3,1,4,100,5,1,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr2_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr1_, CONNECTION_STATE_DISCONNECTED);
    HILOGI("bluetooth_device_battery_manager_test_017 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_018
 * @tc.desc: addr1 connect -> report addr1 IPHONEACCEV 100 -> addr2 connect -> report addr2 IPHONEACCEV 90 ->
             close addr1 box -> current device is addr2 -> report addr2 IPHONEACCEV 80 -> report addr2 IPHONEACCEV 100
 *           device battery is addr1
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_018, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_018 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,9");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr2_, "+IPHONEACCEV=1,1,8");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 90);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr2_, "+IPHONEACCEV=1,1,7");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 80);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr2_, "+IPHONEACCEV=1,1,9");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr2_, CONNECTION_STATE_DISCONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr1_, CONNECTION_STATE_DISCONNECTED);
    HILOGI("bluetooth_device_battery_manager_test_018 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_019
 * @tc.desc: addr1 connect -> report addr1 IPHONEACCEV 100 -> report addr1 ehco 90 ->
             report addr1 IPHONEACCEV 80 -> report addr1 IPHONEACCEV 100 -> report addr1 ehco 70
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_019, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_019 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,9");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,90,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 90);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,7");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 80);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,9");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,70,3,0,4,90,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 70);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr1_, CONNECTION_STATE_DISCONNECTED);
    HILOGI("bluetooth_device_battery_manager_test_019 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_020
 * @tc.desc: addr1 connect -> report addr1 VENDORBATTERY 100 -> report addr1 close ->
 *           report addr1 UPDATEVENDORBATTERY 100 -> report addr1 VENDORBATTERY 100 ->
 *           report addr1 disconnect
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_020, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_020 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,100,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=6,2,100,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_AT), true);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");

    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,100,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_AT), true);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_020 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_021
 * @tc.desc: addr1 connect -> addr2 connect -> report addr2 VENDORBATTERY 100 ->
 *           report addr1 VENDORBATTERY 90 -> report addr2 UPDATEVENDORBATTERY 80 ->
 *           report addr1 close -> report addr1 VENDORBATTERY 100 -> report addr2 close
 *           report addr2 VENDORBATTERY 100 -> addr2 disconnect -> addr1 disconnect
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_021, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_021 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+VENDORBATTERY=6,2,100,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,90,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=6,2,80,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,100,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_AT), true);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr2_, "+VENDORBATTERY=6,2,100,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_AT), true);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_021 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_022
 * @tc.desc: addr1 connect -> report addr2 VENDORBATTERY 100 -> addr2 connect
 *           active addr2 a2dp-> report addr1 UPDATEVENDORBATTERY 80 -> addr1 disconnect -> addr2 disconnect
 *           验证连接耳机后，连接车机并且车机为激活设备，耳机断连后不应该显示电量
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_022, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_022 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,100,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("A2dpSrcService", addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->a2dpActiveDevice_, addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=6,2,80,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 80);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, -1);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_022 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_023
 * @tc.desc: SetDeviceSupportBattery
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_023, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_023 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->SetDeviceSupportBattery(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    HILOGI("bluetooth_device_battery_manager_test_023 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_024
 * @tc.desc: ProcessFindDeviceCommond
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_024, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_024 enter");
    std::vector<uint8_t> originalVec;
    BluetoothDeviceBatteryManager::GetInstance()->ProcessFindDeviceCommond(addr1_, originalVec);
    originalVec = {0x11};
    BluetoothDeviceBatteryManager::GetInstance()->ProcessFindDeviceCommond(addr1_, originalVec);
    HILOGI("bluetooth_device_battery_manager_test_024 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_025
 * @tc.desc: DeviceConnUpdate
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_025, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_025 enter");
    uint8_t earphonePropVal = 0x01;
    uint8_t connStateVal = 0xFF;
    BluetoothDeviceBatteryManager::GetInstance()->DeviceConnUpdate(addr1_, earphonePropVal, connStateVal);
    connStateVal = 0x00;
    BluetoothDeviceBatteryManager::GetInstance()->DeviceConnUpdate(addr1_, earphonePropVal, connStateVal);
    HILOGI("bluetooth_device_battery_manager_test_025 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_026
 * @tc.desc: DeviceControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_026, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_026 enter");
    uint8_t leftVal = 0x00;
    uint8_t rightVal = 0x00;
    BluetoothDeviceBatteryManager::GetInstance()->DeviceControlStatus(addr1_, leftVal, rightVal);
    HILOGI("bluetooth_device_battery_manager_test_026 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_027
 * @tc.desc: addr1 connect -> report addr1 VENDORBATTERY 99 -> report addr1 IPHONEACCEV 90 ->
             report addr1 IPHONEACCEV 70 -> report addr1 UPDATEVENDORBATTERY 60 -> report addr1 ehco 70 1
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_027, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_027 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,9");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,99,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 99);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,8");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 90);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,7");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 80);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,6");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 70);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=6,2,60,3,0,4,78,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 60);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,7");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 80);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=6,2,70,3,0,4,78,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 70);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+UPDATEVENDORBATTERY=6,2,85,3,1,4,86,5,3,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 85);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr1_, CONNECTION_STATE_DISCONNECTED);
    HILOGI("bluetooth_device_battery_manager_test_027 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_028
 * @tc.desc: addr1 connect -> addr2 connect -> report addr1 IPHONEACCEV 90
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_028, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_028 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, "");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,9");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr1_, CONNECTION_STATE_DISCONNECTED);
    HILOGI("bluetooth_device_battery_manager_test_028 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_029
 * @tc.desc: addr connect -> report addr VENDORBATTERY 99 -> addr1 connect ->
             addr2 connect -> report addr1 IPHONEACCEV 100
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_029, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_029 enter");
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,99,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 99);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr2_, CONNECTION_STATE_CONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr2_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr3_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr2_, "+IPHONEACCEV=1,1,9");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr2_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr1_, CONNECTION_STATE_DISCONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr2_, CONNECTION_STATE_DISCONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr3_, CONNECTION_STATE_DISCONNECTED);
    HILOGI("bluetooth_device_battery_manager_test_029 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_030
 * @tc.desc: 1.addr connect -> active a2dp -> active hfp -> report addr VENDORBATTERY 100
             2.addr connect -> active a2dp -> active hfp -> report addr IPHONEACCEV 100
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_030, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_030 enter");
    // 验证设备先被激活a2dp和hfp后才收到耳机上报电量的场景
    // 1.addr connect -> active a2dp -> active hfp -> report addr VENDORBATTERY 100
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("A2dpSrcService", addr1_);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("HfpAgService", addr1_);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(
        addr1_, "+VENDORBATTERY=6,2,100,3,0,4,100,5,0,6,100,7,0", BATTERY_OBTAIN_TYPE_ECHO);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr1_, CONNECTION_STATE_DISCONNECTED);

    // 2.addr connect -> active a2dp -> active hfp -> report addr IPHONEACCEV 100
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(
        addr1_, CONNECTION_STATE_CONNECTED);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("A2dpSrcService", addr1_);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        addr1_, static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(addr1_);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged("HfpAgService", addr1_);
    BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(addr1_, "+IPHONEACCEV=1,1,9");
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->currentBatteryDevice_, addr1_);
    EXPECT_EQ(BluetoothDeviceBatteryManager::GetInstance()->reportBatteryLevel_, 100);

    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(addr1_, CONNECTION_STATE_DISCONNECTED);
    HILOGI("bluetooth_device_battery_manager_test_030 end");
}

/**
 * @tc.name: bluetooth_device_battery_manager_test_031
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothDeviceBatteryManagerTest, bluetooth_device_battery_manager_test_031, TestSize.Level1)
{
    HILOGI("bluetooth_device_battery_manager_test_031 enter");
    RawAddress device("11:22:33:44:55:66");
    BatteryInfo info;
    info.batteryLevel_ = 50;
    info.leftEarBatteryLevel_ = 50;
    info.leftEarChargeState_ = 1;
    info.rightEarBatteryLevel_ = 50;
    info.rightEarChargeState_ = 1;
    info.boxBatteryLevel_ = 50;
    info.boxChargeState_ = 1;
    BluetoothDeviceBatteryManager::GetInstance()->SetRemoteDeviceBatteryInfo(device, info); // 未连接的设备会设置失败
    BatteryDetailInfo outInfo;
    RemoteDeviceProperties::GetInstance()->GetDeviceBatteryInfo("11:22:33:44:55:66", outInfo);
    EXPECT_EQ(outInfo.batteryInfo_.batteryLevel_, -1); // 默认值
    HILOGI("bluetooth_device_battery_manager_test_031 end");
}
}  // namespace bluetooth
}  // namespace OHOS