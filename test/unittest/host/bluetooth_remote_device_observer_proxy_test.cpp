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
#include "bluetooth_remote_device_observer_proxy.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth.h"
#include "adapter_properties.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {

sptr<IRemoteObject> hostObject;
BluetoothHost *host_;
BluetoothRemoteDeviceObserverproxy remoteDeviceObserverProxy(hostObject);
constexpr int TIME = 2;
std::string address("11:22:33:44:55:66");
AdapterProperties adapterProperties;

class RemoteDeviceObserverProxyTest : public testing::Test {
public:
    RemoteDeviceObserverProxyTest()
    {}
    ~RemoteDeviceObserverProxyTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RemoteDeviceObserverProxyTest::SetUpTestCase(void)
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

void RemoteDeviceObserverProxyTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    sleep(TIME);
    host_ = nullptr;
    hostObject = nullptr;
}

void RemoteDeviceObserverProxyTest::SetUp()
{
}

void RemoteDeviceObserverProxyTest::TearDown()
{
}

/**
 * @tc.number: Remote_Device_Observer_Proxy_ModuleTest_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(RemoteDeviceObserverProxyTest, Remote_Device_Observer_Proxy_ModuleTest_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_Proxy_ModuleTest_00100 start");

    int32_t state = 0;
    uint32_t reason = 0;
    BluetoothRawAddress device(address);
    remoteDeviceObserverProxy.OnAclStateChanged(device, state, reason);

    int32_t transport = 1;
    int32_t status = 0;
    int cause = 0;
    remoteDeviceObserverProxy.OnPairStatusChanged(transport, device, status, cause, "");

    std::vector<bluetooth::Uuid> uuids;
    remoteDeviceObserverProxy.OnRemoteUuidChanged(device, uuids);

    bt_property_t* properties = (bt_property_t*)malloc(sizeof(bt_property_t));
    int x = 10;
    properties->val = static_cast<void*>(&x);
    properties->len = sizeof(bluetooth::Uuid) + 1;
    uuids = adapterProperties.ParseDeviceUuid(properties);
    remoteDeviceObserverProxy.OnRemoteUuidChanged(device, uuids);

    std::string deviceName = "BluetoothRemote";
    remoteDeviceObserverProxy.OnRemoteNameChanged(device, deviceName);

    std::string alias = "RemoteBtDevice";
    remoteDeviceObserverProxy.OnRemoteAliasChanged(device, alias);

    int32_t cod = 0;
    remoteDeviceObserverProxy.OnRemoteCodChanged(device, cod);

    BluetoothBatteryInfo batteryInfo;
    remoteDeviceObserverProxy.OnRemoteBatteryChanged(device, batteryInfo);

    std::vector<uint8_t> value = {1, 2, 3};
    remoteDeviceObserverProxy.OnRemoteDeviceCommonInfoReport(device, value);

    uint32_t code = 2;
    MessageOption flags;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = remoteDeviceObserverProxy.InnerTransact(code, flags, data, reply);
    EXPECT_EQ(ret, 7);

    HILOGI("Remote_Device_Observer_Proxy_ModuleTest_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS