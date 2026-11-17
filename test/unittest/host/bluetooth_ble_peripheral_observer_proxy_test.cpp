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

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_ble_peripheral_observer_proxy.h"
#include "bluetooth_profile_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

sptr<IRemoteObject> hostObject;
BluetoothHost *host_;
BluetoothBlePeripheralObserverProxy bleObserverProxy(hostObject);
constexpr int TIME = 2;
std::string address("11:22:33:44:55:66");

class BleObserverProxyTest : public testing::Test {
public:
    BleObserverProxyTest()
    {}
    ~BleObserverProxyTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BleObserverProxyTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");

    host_ = &BluetoothHost::GetDefaultHost();
    hostObject = BluetoothProfileManager::GetInstance().GetProfileRemote("HostObject");
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

void BleObserverProxyTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    sleep(TIME);
    host_ = nullptr;
    hostObject = nullptr;
}

void BleObserverProxyTest::SetUp()
{
}

void BleObserverProxyTest::TearDown()
{
}

/**
 * @tc.number: Ble_Peripheral_Observer_Proxy_ModuleTest_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleObserverProxyTest, Ble_Peripheral_Observer_Proxy_ModuleTest_00100, TestSize.Level1)
{
    HILOGI("Ble_Peripheral_Observer_Proxy_ModuleTest_00100 start");
    int rssi = -30;
    int status = 0;
    BluetoothRawAddress device(address);
    bleObserverProxy.OnReadRemoteRssiEvent(device, rssi, status);
    int32_t transport = 1;
    int cause = 0;
    bleObserverProxy.OnPairStatusChanged(transport, device, status, cause, "");
    unsigned int reason = 0;
    int state = 0;
    bleObserverProxy.OnAclStateChanged(device, state, reason);
    uint32_t code = 2;
    MessageOption flags;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = bleObserverProxy.InnerTransact(code, flags, data, reply);
    EXPECT_EQ(ret, 7);

    HILOGI("Ble_Peripheral_Observer_Proxy_ModuleTest_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS