/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#include "bluetooth_oob_observer_proxy.h"
#include "bluetooth_profile_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

sptr<IRemoteObject> hostObject;
BluetoothHost *host_;
BluetoothOobObserverProxy oobObserverProxy(hostObject);
constexpr int TIME = 2;

class OobObserverProxyTest : public testing::Test {
public:
    OobObserverProxyTest()
    {}
    ~OobObserverProxyTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void OobObserverProxyTest::SetUpTestCase(void)
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

void OobObserverProxyTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    sleep(TIME);
    host_ = nullptr;
    hostObject = nullptr;
}

void OobObserverProxyTest::SetUp()
{
}

void OobObserverProxyTest::TearDown()
{
}

/**
 * @tc.number: Oob_Observer_Proxy_ModuleTest_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(OobObserverProxyTest, Oob_Observer_Proxy_ModuleTest_00100, TestSize.Level1)
{
    HILOGI("Oob_Observer_Proxy_ModuleTest_00100 start");
    int32_t ret = BT_NO_ERROR;
    BluetoothOobData oobData;
    oobObserverProxy.OnGenerateLocalOobData(ret, oobData);
    uint32_t code = 2;
    MessageOption flags;
    MessageParcel data;
    MessageParcel reply;
    ErrCode err = oobObserverProxy.InnerTransact(code, flags, data, reply);
    EXPECT_EQ(err, 32);
    HILOGI("Oob_Observer_Proxy_ModuleTest_00100 end");
}

}  // namespace Bluetooth
}  // namespace OHOS