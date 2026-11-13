/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_pan.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_pan_proxy.h"
#include "bluetooth_proxy_manager.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
static BluetoothHost *host_;
static sptr<IRemoteObject> remoteObject_;

class BluetoothPanProxyTestMock : public BluetoothPanProxy {
public:
    explicit BluetoothPanProxyTestMock(const sptr<IRemoteObject>& remote) : BluetoothPanProxy(remote)
    {}
    ~BluetoothPanProxyTestMock() override
    {}
};

static shared_ptr<BluetoothPanProxyTestMock> panProxy_ { nullptr };

class BluetoothPanProxyTest : public testing::Test {
public:
    BluetoothPanProxyTest()
    {}
    ~BluetoothPanProxyTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BluetoothPanProxyTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
    host_ = &BluetoothHost::GetDefaultHost();

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

void BluetoothPanProxyTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void BluetoothPanProxyTest::SetUp()
{
    HILOGI("SetUp start");
    remoteObject_ = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_OPP_SERVER);
    panProxy_ = make_shared<BluetoothPanProxyTestMock>(remoteObject_);
    HILOGI("SetUp end");
}

void BluetoothPanProxyTest::TearDown()
{
    HILOGI("TearDown start");
    panProxy_ = nullptr;
    remoteObject_ = nullptr;
    HILOGI("TearDown start");
}

/*
 * @tc.number: BluetoothPanProxyTest_RegisterObserver_00100
 * @tc.name: WriteParcelableInt32Vector
 * @tc.desc: return true
 */
HWTEST_F(BluetoothPanProxyTest, BluetoothPanProxyTest_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("BluetoothPanProxyTest_RegisterObserver_00100 start");
    std::string device("00:00:00:00:00:00");
    std::vector<int32_t> mimeTypes = {1, 2};

    MessageParcel data;
    data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor());
    data.WriteString(device);
    bool ret = panProxy_->WriteParcelableInt32Vector(mimeTypes, data);
    EXPECT_TRUE(ret);
    HILOGI("BluetoothPanProxyTest_RegisterObserver_00100 end");
}
} // Bluetooth
} // OHOS