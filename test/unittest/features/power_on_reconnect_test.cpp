/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd. All rights reserved.
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
#include <gmock/gmock.h>
#include <cstring>
#include "bluetooth_host.h"
#include "power_on_reconnect.h"
#include "bt_def.h"
#include "bluetooth_log.h"
#include "connect_strategy_manager.h"
#include "adapter_manager.h"
#include "classic_adapter.h"
#include "bluetooth_connection_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

std::string address("11:22:33:44:55:66");

const int TIME = 2;
static BluetoothHost *host_;
static PowerOnReconnect *PowerOnReconnect_ {nullptr};


class PowerOnReconnectTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        bool isBrEnable = host_->IsBrEnabled();

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        PowerOnReconnect_ = PowerOnReconnect::GetInstance();
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
public:
};

/*
 * @tc.number: power_on_reconnect_Initialize_0001
 * @tc.name: Initialize
 * @tc.desc:
 */
HWTEST_F(PowerOnReconnectTest, power_on_reconnect_Initialize_0001, TestSize.Level1)
{
    HILOGI("power_on_reconnect_Initialize_0001 start");

    PowerOnReconnect_->Initialize();
    EXPECT_NE(PowerOnReconnect_->subscriber_, nullptr);

    PowerOnReconnect_->Uninitialize();
    EXPECT_EQ(PowerOnReconnect_->subscriber_, nullptr);

    HILOGI("power_on_reconnect_Initialize_0001 end");
}

/*
 * @tc.number: power_on_reconnect_DisconnectPeripheralsInStr_0001
 * @tc.name: DisconnectPeripheralsInStr
 * @tc.desc:
 */
HWTEST_F(PowerOnReconnectTest, power_on_reconnect_DisconnectPeripheralsInStr_0001, TestSize.Level1)
{
    HILOGI("power_on_reconnect_DisconnectPeripheralsInStr_0001 start");

    RawAddress device(address);
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    BluetoothConnectionManager::GetInstance()->RecordAclConnect(
        address, LINK_TYPE_BREDR, CONNECTION_STATE_CONNECTED, "");
    PowerOnReconnect_->DisconnectPeripheralsInStr();

    sleep(TIME);

    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    EXPECT_FALSE(classicAdapter->IsAclConnected(device));

    HILOGI("power_on_reconnect_DisconnectPeripheralsInStr_0001 end");
}

/*
 * @tc.number: power_on_reconnect_StartAutoConnect_0001
 * @tc.name: StartAutoConnectInStr
 * @tc.desc:
 */
HWTEST_F(PowerOnReconnectTest, power_on_reconnect_StartAutoConnect_0001, TestSize.Level1)
{
    HILOGI("power_on_reconnect_StartAutoConnect_0001 start");

    PowerOnReconnect_->isForceSleep_.store(true);
    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    BluetoothConnectionManager::GetInstance()->RecordAclConnect(
        address, LINK_TYPE_BREDR, CONNECTION_STATE_DISCONNECTED, "");

    PowerOnReconnect_->StartAutoConnect();

    sleep(TIME);

    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    EXPECT_TRUE(classicAdapter->IsAclConnected(device));

    HILOGI("power_on_reconnect_StartAutoConnect_0001 end");
}

}  // namespace bluetooth
}  // namespace OHOS
