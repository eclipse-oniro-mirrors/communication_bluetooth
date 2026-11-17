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
#include <cstring>
#include "service_util.h"
#include "adapter_manager.h"
#include "ble_address_with_type.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_state_manager.h"
#include "profile_service_manager.h"
#include "hfp_ag_defines.h"
#include "pan_defines.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
ServiceUtil serviceUtil;

class BluetoothStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

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
        HILOGI("start");
        host_ = nullptr;
    }
    void SetUp()
    {
        sleep(TIME);
    }
    void TearDown()
    {}
};

/*
 * @tc.number:state_manager_test_001
 * @tc.name: state
 * @tc.desc:
 */
HWTEST_F(BluetoothStateManagerTest, state_manager_test_001, TestSize.Level1)
{
    HILOGI("state_manager_test_001 enter");
    std::string addr = "11:11:11:11:11:11";
    int state = 0;
    int connectedState = 3;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(addr, static_cast<int>(HFP_AG_STATE_CONNECTED));
    int disConnectedState = 0;
    BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(addr, state);
    EXPECT_EQ(state, connectedState);
    bool findResult  = BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(addr, state);
    EXPECT_EQ(findResult, true);
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState(addr);
    findResult  = BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(addr, state);
    EXPECT_EQ(findResult, false);
    HILOGI("state_manager_test_001 end");
}

/*
 * @tc.number:state_manager_test_002
 * @tc.name: state
 * @tc.desc:
 */
HWTEST_F(BluetoothStateManagerTest, state_manager_test_002, TestSize.Level1)
{
    HILOGI("state_manager_test_002 enter");
    int connectedState = 3;
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("11:11:11:11:11:11",
        static_cast<int>(HFP_AG_STATE_DISCONNECTED));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("22:22:22:22:22:22",
        static_cast<int>(HFP_AG_STATE_CONNECTING));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("33:33:33:33:33:33",
        static_cast<int>(HFP_AG_STATE_DISCONNECTING));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("44:44:44:44:44:44",
        static_cast<int>(HFP_AG_STATE_CONNECTED));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("55:55:55:55:55:55",
        static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTING));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("66:66:66:66:66:66",
        static_cast<int>(HFP_AG_AUDIO_STATE_DISCONNECTING));
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState("77:77:77:77:77:77",
        static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTED));

    std::list<std::string> filterAddress;
    filterAddress = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([connectedState](const std::string address, int &state) -> bool {
            return state >= connectedState;});
    EXPECT_EQ(filterAddress.size(), 4);
    HILOGI("state_manager_test_002 end");
}

/*
 * @tc.number:state_manager_test_003
 * @tc.name: state
 * @tc.desc:
 */
HWTEST_F(BluetoothStateManagerTest, state_manager_test_003, TestSize.Level1)
{
    HILOGI("state_manager_test_003 enter");
    std::string addr = "11:11:11:11:11:11";
    int state = 0;
    int connectedState = 3;
    BluetoothStateManager::GetInstance()->SetRemoteDevicePanState(addr, static_cast<int>(PAN_STATE_CONNECTED));
    int disConnectedState = 0;
    BluetoothStateManager::GetInstance()->FindRemoteDevicePanStateByAddr(addr, state);
    EXPECT_EQ(state, connectedState);
    bool findResult  = BluetoothStateManager::GetInstance()->FindRemoteDevicePanStateByAddr(addr, state);
    EXPECT_EQ(findResult, true);
    BluetoothStateManager::GetInstance()->EraseRemoteDevicePanState(addr);
    findResult  = BluetoothStateManager::GetInstance()->FindRemoteDevicePanStateByAddr(addr, state);
    EXPECT_EQ(findResult, false);
    HILOGI("state_manager_test_003 end");
}

/*
 * @tc.number:state_manager_test_004
 * @tc.name: state
 * @tc.desc:
 */
HWTEST_F(BluetoothStateManagerTest, state_manager_test_004, TestSize.Level1)
{
    HILOGI("state_manager_test_004 enter");
    int connectedState = 3;
    BluetoothStateManager::GetInstance()->SetRemoteDevicePanState("11:11:11:11:11:11",
        static_cast<int>(PAN_STATE_DISCONNECTED));
    BluetoothStateManager::GetInstance()->SetRemoteDevicePanState("22:22:22:22:22:22",
        static_cast<int>(PAN_STATE_DISCONNECTING));
    BluetoothStateManager::GetInstance()->SetRemoteDevicePanState("33:33:33:33:33:33",
        static_cast<int>(PAN_STATE_CONNECTING));
    BluetoothStateManager::GetInstance()->SetRemoteDevicePanState("44:44:44:44:44:44",
        static_cast<int>(PAN_STATE_CONNECTED));

    std::list<std::string> filterAddress;
    filterAddress = BluetoothStateManager::GetInstance()->FindRemoteDeviceByPanConnState<std::string,
        int>([connectedState](const std::string &address, int &state) -> bool {
            return state >= connectedState;});
    EXPECT_EQ(filterAddress.size(), 1);
    HILOGI("state_manager_test_004 end");
}

/*
 * @tc.number:state_manager_test_005
 * @tc.name: state
 * @tc.desc:
 */
HWTEST_F(BluetoothStateManagerTest, state_manager_test_005, TestSize.Level1)
{
    HILOGI("state_manager_test_005 enter");
    std::string addr1 = "11:22:33:33:33:33";
    std::string addr2 = "00:22:33:33:33:33";
    std::string addr3 = "11:11:11:11:11:11";
    BluetoothStateManager::GetInstance()->AddDeviceProfileConnectState(
        PROFILE_NAME_GATT_CLIENT, addr1, BTConnectState::CONNECTED);
    BTConnectState state = BluetoothStateManager::GetInstance()->GetDeviceProfileConnectState(
        PROFILE_NAME_GATT_CLIENT, addr1);
    EXPECT_EQ(state, BTConnectState::CONNECTED);

    state = BluetoothStateManager::GetInstance()->GetDeviceProfileConnectState(
        PROFILE_NAME_GATT_CLIENT, addr2);
    EXPECT_EQ(state, BTConnectState::DISCONNECTED);
    state = BluetoothStateManager::GetInstance()->GetDeviceProfileConnectState(
        PROFILE_NAME_GATT_SERVER, addr1);
    EXPECT_EQ(state, BTConnectState::DISCONNECTED);

    BluetoothStateManager::GetInstance()->RemoveDeviceProfileConnectState(
        PROFILE_NAME_GATT_CLIENT, addr1);
    state = BluetoothStateManager::GetInstance()->GetDeviceProfileConnectState(
        PROFILE_NAME_GATT_CLIENT, addr1);
    EXPECT_EQ(state, BTConnectState::DISCONNECTED);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->deviceConnectStateMap_.Size(), 1);

    BluetoothStateManager::GetInstance()->RemoveDeviceConnectState(addr1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->deviceConnectStateMap_.Size(), 0);

    BluetoothStateManager::GetInstance()->AddDeviceProfileConnectState(
        PROFILE_NAME_GATT_SERVER, addr3, BTConnectState::CONNECTED);
    state = BluetoothStateManager::GetInstance()->GetDeviceProfileConnectState(
        PROFILE_NAME_GATT_SERVER, addr3);
    EXPECT_EQ(state, BTConnectState::CONNECTED);

    BluetoothStateManager::GetInstance()->RemoveDeviceConnectState(addr3);
    state = BluetoothStateManager::GetInstance()->GetDeviceProfileConnectState(
        PROFILE_NAME_GATT_SERVER, addr3);
    EXPECT_EQ(state, BTConnectState::DISCONNECTED);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->deviceConnectStateMap_.Size(), 0);

    HILOGI("state_manager_test_005 enter");
}

/*
 * @tc.number:state_manager_test_getConnectedBleDevices_001
 * @tc.name: state
 * @tc.desc:
 */
HWTEST_F(BluetoothStateManagerTest, state_manager_test_getConnectedBleDevices_001, TestSize.Level1)
{
    HILOGI("state_manager_test_getConnectedBleDevices_001 enter");
    std::string addr1 = "11:22:33:33:33:33";
    std::string addr2 = "00:22:33:33:33:33";
    // 首次添加设备1
    BluetoothStateManager::GetInstance()->AddClientDeviceList(addr1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr1], 1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);
    // 第二次添加设备1
    BluetoothStateManager::GetInstance()->AddClientDeviceList(addr1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr1], 2);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);

    // 首次添加设备2
    BluetoothStateManager::GetInstance()->AddClientDeviceList(addr2);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr2], 1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 2);
    // 第二次添加设备2
    BluetoothStateManager::GetInstance()->AddClientDeviceList(addr2);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr2], 2);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 2);

    // 首次删除设备1，addr1引用计数减为1，list不删除
    BluetoothStateManager::GetInstance()->RemoveClientDeviceList(addr1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr1], 1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 2);
    // 第二次删除设备1，addr1引用计数减为0，list删除addr1，仅剩addr2
    BluetoothStateManager::GetInstance()->RemoveClientDeviceList(addr1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_.count(addr1), 0);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);

    // 首次删除设备2，addr2引用计数减为1，list不删除
    BluetoothStateManager::GetInstance()->RemoveClientDeviceList(addr2);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr2], 1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);
    // 第二次删除设备2，addr2引用计数减为0，list删除addr2
    BluetoothStateManager::GetInstance()->RemoveClientDeviceList(addr2);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_.count(addr2), 0);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 0);

    HILOGI("state_manager_test_getConnectedBleDevices_001 end");
}

/*
 * @tc.number:state_manager_test_getConnectedBleDevices_002
 * @tc.name: state
 * @tc.desc:
 */
HWTEST_F(BluetoothStateManagerTest, state_manager_test_getConnectedBleDevices_002, TestSize.Level1)
{
    HILOGI("state_manager_test_getConnectedBleDevices_002 enter");
    std::string addr3 = "11:11:11:11:11:11";

    // 首次添加设备3
    BluetoothStateManager::GetInstance()->AddClientDeviceList(addr3);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr3], 1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);
    // 第二次添加设备3
    BluetoothStateManager::GetInstance()->AddClientDeviceList(addr3);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr3], 2);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);
    // 第三次添加设备3
    BluetoothStateManager::GetInstance()->AddClientDeviceList(addr3);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr3], 3);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);

    // 首次删除设备3，addr3引用计数减为2，list不删除
    BluetoothStateManager::GetInstance()->RemoveClientDeviceList(addr3);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr3], 2);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);
    // 第二次删除设备3，addr3引用计数减为1，list不删除
    BluetoothStateManager::GetInstance()->RemoveClientDeviceList(addr3);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_[addr3], 1);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 1);
    // 第三次删除设备3，addr3引用计数减为0，list删除addr3
    BluetoothStateManager::GetInstance()->RemoveClientDeviceList(addr3);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_.count(addr3), 0);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 0);
    // 第四次删除设备3，list已删除addr3
    BluetoothStateManager::GetInstance()->RemoveClientDeviceList(addr3);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceCountMap_.count(addr3), 0);
    EXPECT_EQ(BluetoothStateManager::GetInstance()->clientDeviceList_.size(), 0);

    HILOGI("state_manager_test_getConnectedBleDevices_002 end");
}

}  // namespace bluetooth
}  // namespace OHOS