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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "bluetooth_a2dp_snk.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_a2dp_sink_proxy.h"
#include "bluetooth_a2dp_sink_observer_stub.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"
#include "raw_address.h"
#include "bluetooth_profile_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace std;

constexpr int TIME = 4;

class A2dpSinkObserverCommon : public A2dpSinkObserver {
public:
    A2dpSinkObserverCommon() = default;
    virtual ~A2dpSinkObserverCommon() = default;
    
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause){}

private:
};

class BluetoothA2dpSinkObserverStubTest : public BluetoothA2dpSinkObserverStub {
public:
    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {}

    explicit BluetoothA2dpSinkObserverStubTest()
    {}
    ~BluetoothA2dpSinkObserverStubTest() override
    {}

private:
};

static std::shared_ptr<A2dpSinkObserverCommon> observer_ = std::make_shared<A2dpSinkObserverCommon>();
static A2dpSink *profile_;
static BluetoothHost *host_;


class A2dpSinkTest : public testing::Test {
public:
    A2dpSinkTest()
    {}
    ~A2dpSinkTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<IBluetoothA2dpSink> proxy_ = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);

};


void A2dpSinkTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBleEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    profile_ = A2dpSink::GetProfile();

    HILOGI("SetUpTestCase end");
}

void A2dpSinkTest::TearDownTestCase(void)
{
    host_ = nullptr;
}

void A2dpSinkTest::SetUp()
{
}

void A2dpSinkTest::TearDown(void)
{
}

/*
 * @tc.number: A2dpSink001
 * @tc.name: GetProfile
 * @tc.desc: Get a2dp sink instance
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetProfile, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetProfile start");

    profile_ = A2dpSink::GetProfile();
    EXPECT_NE(nullptr, profile_);
    HILOGI("A2dpSink_UnitTest_GetProfile end");
}

/*
 * @tc.number: A2dpSink002
 * @tc.name: GetPlayingState
 * @tc.desc: Get device playing state by address when peer device is on connected
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetPlayingState, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetPlayingState start");

    profile_ = A2dpSink::GetProfile();
    BluetoothRemoteDevice device;
    int state = profile_->GetPlayingState(device);
    EXPECT_EQ(state, RET_BAD_STATUS);
    HILOGI("A2dpSink_UnitTest_GetPlayingState end");
}

/*
 * @tc.number: A2dpSink003
 * @tc.name: Connect
 * @tc.desc: Connect to the peer bluetooth device
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_Connect, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_Connect start");

    profile_ = A2dpSink::GetProfile();
    BluetoothRemoteDevice device;
    bool isOK = profile_->Connect(device);
    EXPECT_FALSE(isOK);
    HILOGI("A2dpSink_UnitTest_Connect end");
}

/*
 * @tc.number: A2dpSink004
 * @tc.name: Disconnect
 * @tc.desc: Disconnect with the peer bluetooth service
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_Disconnect, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_Disconnect start");

    profile_ = A2dpSink::GetProfile();
    BluetoothRemoteDevice device;
    bool isOK = profile_->Disconnect(device);
    EXPECT_FALSE(isOK);
    HILOGI("A2dpSink_UnitTest_Disconnect end");
}

/*
 * @tc.number: A2dpSink005
 * @tc.name: RegisterObserver
 * @tc.desc: Register callback function of framework
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_RegisterObserver, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_RegisterObserver start");

    profile_ = A2dpSink::GetProfile();
    profile_->RegisterObserver(observer_);
    HILOGI("A2dpSink_UnitTest_RegisterObserver end");
}

/*
 * @tc.number: A2dpSink006
 * @tc.name: DeregisterObserver
 * @tc.desc: Deregister callback function of framework
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_DeregisterObserver, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_DeregisterObserver start");

    profile_ = A2dpSink::GetProfile();
    profile_->DeregisterObserver(observer_);
    HILOGI("A2dpSink_UnitTest_DeregisterObserver end");
}

/*
 * @tc.number: HandsFreeUnit007
 * @tc.name: GetDeviceState
 * @tc.desc: Get device connection state by address
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetDeviceState, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetDeviceState start");

    profile_ = A2dpSink::GetProfile();
    BluetoothRemoteDevice device;
    EXPECT_EQ(profile_->GetDeviceState(device), RET_BAD_STATUS);

    HILOGI("A2dpSink_UnitTest_GetDeviceState end");
}

/*
 * @tc.number: HandsFreeUnit008
 * @tc.name: GetDevicesByStates
 * @tc.desc: Get devices by connection states
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetDevicesByStates, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetDevicesByStates start");

    profile_ = A2dpSink::GetProfile();
    BluetoothRemoteDevice device;
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    vector<BluetoothRemoteDevice> devices = profile_->GetDevicesByStates(states);
    EXPECT_NE(nullptr, &devices);
    HILOGI("A2dpSink_UnitTest_GetDevicesByStates end");
}

/*
 * @tc.number: HandsFreeUnit009
 * @tc.name: GetConnectStrategy
 * @tc.desc: Get connect strategy by address
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetConnectStrategy, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetConnectStrategy start");

    profile_ = A2dpSink::GetProfile();
    BluetoothRemoteDevice device;
    int ret = profile_->GetConnectStrategy(device);
    EXPECT_EQ(ret, -3);

    HILOGI("A2dpSink_UnitTest_GetConnectStrategy end");
}

/*
 * @tc.number: HandsFreeUnit010
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_SetConnectStrategy, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_SetConnectStrategy start");

    profile_ = A2dpSink::GetProfile();
    BluetoothRemoteDevice device;
    bool isOK = profile_->SetConnectStrategy(device,2);
    EXPECT_EQ(isOK, false);

    HILOGI("A2dpSink_UnitTest_SetConnectStrategy end");
}

/*
 * @tc.number: A2dpSink011
 * @tc.name: Connect
 * @tc.desc: Check connect interface
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_Connect001, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_Connect001 start");
 
    BluetoothRemoteDevice device;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        int ret = proxy_->Connect(RawAddress(device.GetDeviceAddr()));
        EXPECT_EQ(ret, NO_ERROR);
    }
    BluetoothA2dpSinkObserverStubTest bluetoothA2dpSinkObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    bluetoothA2dpSinkObserverStubTest.OnConnectionStateChangedInner(data, reply);
    HILOGI("A2dpSink_UnitTest_Connect001 end");
}

/*
 * @tc.number: A2dpSink012
 * @tc.name: Disconnect
 * @tc.desc: Check Disconnect interface
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_Disconnect001, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_Connect001 start");
 
    BluetoothRemoteDevice device;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        int ret = proxy_->Disconnect(RawAddress(device.GetDeviceAddr()));
        EXPECT_EQ(ret, NO_ERROR);
    }
    HILOGI("A2dpSink_UnitTest_Connect001 end");
}

/*
 * @tc.number: A2dpSink013
 * @tc.name: GetDevicesByStates
 * @tc.desc: Check GetDevicesByStates interface
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetDevicesByStates001, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetDevicesByStates001 start");
 
    vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        std::vector<RawAddress> rawAdds = proxy_->GetDevicesByStates(states);
        EXPECT_FALSE(rawAdds.empty());
    }
    HILOGI("A2dpSink_UnitTest_GetDevicesByStates001 end");
}

/*
 * @tc.number: A2dpSink014
 * @tc.name: GetDeviceState
 * @tc.desc: Check GetDeviceState interface
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetDeviceState001, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetDeviceState001 start");
 
    BluetoothRemoteDevice device;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        int32_t ret = proxy_->GetDeviceState(RawAddress(device.GetDeviceAddr()));
        EXPECT_EQ(ret, NO_ERROR);
    }
    HILOGI("A2dpSink_UnitTest_GetDeviceState001 end");
}

/*
 * @tc.number: A2dpSink015
 * @tc.name: GetPlayingState
 * @tc.desc: Check GetPlayingState interface
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetPlayingState001, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetPlayingState001 start");
    BluetoothRemoteDevice device;
    int state = RET_BAD_STATUS;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        int32_t ret = proxy_->GetPlayingState(RawAddress(device.GetDeviceAddr()), state);
        EXPECT_EQ(ret, NO_ERROR);
    }
    HILOGI("A2dpSink_UnitTest_GetPlayingState001 end");
}

/*
 * @tc.number: A2dpSink016
 * @tc.name: SetConnectStrategy
 * @tc.desc: Set connect strategy by address
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_SetConnectStrategy001, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_SetConnectStrategy001 start");

    BluetoothRemoteDevice device;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        int32_t ret = proxy_->SetConnectStrategy(RawAddress(device.GetDeviceAddr()), 1);
        EXPECT_EQ(ret, NO_ERROR);
    }
    
    HILOGI("A2dpSink_UnitTest_SetConnectStrategy001 end");
}

/*
 * @tc.number: A2dpSink016
 * @tc.name: GetConnectStrategy
 * @tc.desc: Check GetConnectStrategy interface
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_GetConnectStrategy001, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_GetConnectStrategy001 start");
    BluetoothRemoteDevice device;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        int32_t ret = proxy_->GetConnectStrategy(RawAddress(device.GetDeviceAddr()));
        EXPECT_EQ(ret, NO_ERROR);
    }

    HILOGI("A2dpSink_UnitTest_GetConnectStrategy001 end");
}

/*
 * @tc.number: A2dpSink016
 * @tc.name: SendDelay
 * @tc.desc: Check SendDelay interface
*/
HWTEST_F(A2dpSinkTest, A2dpSink_UnitTest_SendDelay001, TestSize.Level1)
{
    HILOGI("A2dpSink_UnitTest_SendDelay001 start");
    BluetoothRemoteDevice device;
    int32_t delayValue = 0;
    if (proxy_ == nullptr) {
        HILOGE("proxy is nullptr");
    } else {
        int32_t ret = proxy_->SendDelay(RawAddress(device.GetDeviceAddr()), delayValue);
        EXPECT_EQ(ret, NO_ERROR);
    }
    HILOGI("A2dpSink_UnitTest_SendDelay001 start");
}
}  // namespace Bluetooth
}  // namespace OHOS