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

#include <gtest/gtest.h>
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_opp_proxy.h"
#include "bluetooth_proxy_manager.h"
#include "bluetooth_profile_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
static BluetoothHost *host_;
static sptr<IRemoteObject> remoteObject_;
static int32_t g_numOne = 1;
static int32_t g_numTwo = 2;

class BluetoothOppProxyTestMock : public BluetoothOppProxy {
public:
    explicit BluetoothOppProxyTestMock(const sptr<IRemoteObject>& remote) : BluetoothOppProxy(remote)
    {}
    ~BluetoothOppProxyTestMock() override
    {}
    // sptr<IRemoteObject> Remote()
    // {
    //     return
    // }
};

static shared_ptr<BluetoothOppProxyTestMock> oppProxy_ { nullptr };

class BluetoothOppProxyTest : public testing::Test {
public:
    BluetoothOppProxyTest()
    {}
    ~BluetoothOppProxyTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
    // sptr<IBluetoothOpp> oppProxy_ = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
};

void BluetoothOppProxyTest::SetUpTestCase(void)
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

void BluetoothOppProxyTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}

void BluetoothOppProxyTest::SetUp()
{
    HILOGI("SetUp start");
    remoteObject_ = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_OPP_SERVER);
    oppProxy_ = make_shared<BluetoothOppProxyTestMock>(remoteObject_);
    HILOGI("SetUp end");
}

void BluetoothOppProxyTest::TearDown()
{
    HILOGI("TearDown start");
    oppProxy_ = nullptr;
    remoteObject_ = nullptr;
    HILOGI("TearDown start");
}

/*
 * @tc.number: BluetoothOppProxyTest_SetIncomingFileConfirmation_00100
 * @tc.name: SetIncomingFileConfirmation
 * @tc.desc: return true
 */
HWTEST_F(BluetoothOppProxyTest, BluetoothOppProxyTest_SetIncomingFileConfirmation_00100, TestSize.Level1)
{
    HILOGI("BluetoothOppProxyTest_SetIncomingFileConfirmation_00100 start");
    bool ret = oppProxy_->SetIncomingFileConfirmation(true, 1);
    EXPECT_TRUE(ret > 0);
    HILOGI("BluetoothOppProxyTest_SetIncomingFileConfirmation_00100 end");
}

/*
 * @tc.number: BluetoothOppProxyTest_CancelTransfer_00100
 * @tc.name: CancelTransfer
 * @tc.desc: return true
 */
HWTEST_F(BluetoothOppProxyTest, BluetoothOppProxyTest_CancelTransfer_00100, TestSize.Level1)
{
    HILOGI("BluetoothOppProxyTest_CancelTransfer_00100 start");
    bool ret = oppProxy_->CancelTransfer();
    EXPECT_TRUE(ret > 0);
    HILOGI("BluetoothOppProxyTest_CancelTransfer_00100 end");
}

/*
 * @tc.number: BluetoothOppProxyTest_SetLastReceivedFileUri_00100
 * @tc.name: SetLastReceivedFileUri
 * @tc.desc: return true
 */
HWTEST_F(BluetoothOppProxyTest, BluetoothOppProxyTest_SetLastReceivedFileUri_00100, TestSize.Level1)
{
    HILOGI("BluetoothOppProxyTest_SetLastReceivedFileUri_00100 start");
    bool ret = oppProxy_->SetLastReceivedFileUri("test.jpg");
    EXPECT_TRUE(ret >= 0);
    HILOGI("BluetoothOppProxyTest_SetLastReceivedFileUri_00100 end");
}

/*
 * @tc.number: BluetoothOppProxyTest_WriteParcelableStringVector_00100
 * @tc.name: WriteParcelableStringVector
 * @tc.desc: return true
 */
HWTEST_F(BluetoothOppProxyTest, BluetoothOppProxyTest_WriteParcelableStringVector_00100, TestSize.Level1)
{
    HILOGI("BluetoothOppProxyTest_WriteParcelableStringVector_00100 start");
    std::string device("00:00:00:00:00:00");
    std::vector<std::string> mimeTypes = {"mime1", "mime2"};
    HILOGI("file size = %{public}lu", mimeTypes.size());
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor());
    data.WriteString(device);
    bool ret = oppProxy_->WriteParcelableStringVector(mimeTypes, data);
    EXPECT_TRUE(ret);
    HILOGI("BluetoothOppProxyTest_WriteParcelableStringVector_00100 end");
}

/*
 * @tc.number: BluetoothOppProxyTest_WriteParcelableInt32Vector_00100
 * @tc.name: WriteParcelableInt32Vector
 * @tc.desc:
 */
HWTEST_F(BluetoothOppProxyTest, BluetoothOppProxyTest_WriteParcelableInt32Vector_00100, TestSize.Level1)
{
    HILOGI("BluetoothOppProxyTest_WriteParcelableInt32Vector_00100 start");
    std::string device("00:00:00:00:00:00");
    std::vector<int32_t> mimeTypes = {1, 2};
    HILOGI("file size = %{public}lu", mimeTypes.size());
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor());
    data.WriteString(device);
    bool ret = oppProxy_->WriteParcelableInt32Vector(mimeTypes, data);
    EXPECT_TRUE(ret);
    HILOGI("BluetoothOppProxyTest_WriteParcelableInt32Vector_00100 end");
}
} // OHOS
} // Bluetooth