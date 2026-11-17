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
#include "bluetooth_host_observer_proxy.h"
#include "bluetooth_profile_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

sptr<IRemoteObject> hostObject;
BluetoothHost *host_;
BluetoothHostObserverProxy hostObserverProxy(hostObject);
constexpr int TIME = 2;

class HostObserverProxyTest : public testing::Test {
public:
    HostObserverProxyTest()
    {}
    ~HostObserverProxyTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HostObserverProxyTest::SetUpTestCase(void)
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

void HostObserverProxyTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    sleep(TIME);
    host_ = nullptr;
    hostObject = nullptr;
}

void HostObserverProxyTest::SetUp()
{
}

void HostObserverProxyTest::TearDown()
{
}

/**
 * @tc.number: Host_Observer_Proxy_ModuleTest_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(HostObserverProxyTest, Host_Observer_Proxy_ModuleTest_00100, TestSize.Level1)
{
    HILOGI("Host_Observer_Proxy_ModuleTest_00100 start");
    int32_t transport = 1;
    int32_t status = 0;
    hostObserverProxy.OnStateChanged(transport, status);
    hostObserverProxy.OnDiscoveryStateChanged(status);
    string addr = "66:55:44:33:22:11";
    BluetoothRawAddress device(addr);
    hostObserverProxy.OnPairRequested(transport, device);
    int reqType = 1;
    int number = 1;
    hostObserverProxy.OnPairConfirmed(transport, device, reqType, number);
    int scanmode = 0;
    hostObserverProxy.OnScanModeChanged(scanmode);
    string name = "BluetoothDevice";
    hostObserverProxy.OnDeviceNameChanged(name);
    hostObserverProxy.OnDeviceAddrChanged(addr);
    uint32_t code = 2;
    MessageOption flags;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = hostObserverProxy.InnerTransact(code, flags, data, reply);
    EXPECT_EQ(ret, 32);

    HILOGI("Host_Observer_Proxy_ModuleTest_00100 end");
}

}  // namespace Bluetooth
}  // namespace OHOS