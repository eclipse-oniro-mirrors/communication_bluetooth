/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#include "bluetooth_resource_manager_observer_proxy.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_access_token_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
sptr<IRemoteObject> hostObject;
BluetoothHost *host_;
BluetoothResourceManagerObserverproxy resourceManagerObserverProxy(hostObject);
constexpr int TIME = 2;

class ResourceManagerObserverProxyTest : public testing::Test {
public:
    ResourceManagerObserverProxyTest()
    {}
    ~ResourceManagerObserverProxyTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ResourceManagerObserverProxyTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");

    BluetoothAccessTokenMock::SetNativeTokenInfo();
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

void ResourceManagerObserverProxyTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    sleep(TIME);
    host_ = nullptr;
    hostObject = nullptr;
}

void ResourceManagerObserverProxyTest::SetUp()
{
}

void ResourceManagerObserverProxyTest::TearDown()
{
}

/**
 * @tc.number: Bluetooth_Resource_Manager_Observer_Proxy_ModuleTest_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ResourceManagerObserverProxyTest,
    Bluetooth_Resource_Manager_Observer_Proxy_ModuleTest_00100, TestSize.Level1)
{
    HILOGI("Bluetooth_Resource_Manager_Observer_Proxy_ModuleTest_00100 start");

    uint8_t eventId = SPP_CLIENT_CONNECT_DONE;
    bluetooth::SensingInfo sensingInfo;
    BluetoothSensingInfo info(sensingInfo);
    resourceManagerObserverProxy.OnSensingStateChanged(eventId, info);
    eventId = BLE_FASTEST_CONN_DECISION;
    uint32_t result = CONNECTION_ACCEPT;
    resourceManagerObserverProxy.OnBluetoothResourceDecision(eventId, info, result);
    uint32_t code = 2;
    MessageOption flags;
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = resourceManagerObserverProxy.InnerTransact(code, flags, data, reply);
    EXPECT_EQ(ret, 7);

    HILOGI("Bluetooth_Resource_Manager_Observer_Proxy_ModuleTest_00100 end");
}

}  // namespace Bluetooth
}  // namespace OHOS