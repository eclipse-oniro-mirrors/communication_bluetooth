/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "bluetooth_errorcode.h"
#include "bluetooth_hfp_ag_observer_proxy.h"
#include "bluetooth_hfp_ag_stub.h"
#include "bluetooth_log.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"
#include "raw_address.h"
#include "btcommon/timer_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;
RawAddress address("11:22:33:44:55:66");

class BluetoothHfpAgObserverStubTest : public BluetoothHfpAgStub {
public:
    int32_t GetConnectDevices(std::vector<BluetoothRawAddress> &devices) override
    {
        return 0;
    }
    int GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRawAddress> &devices) override
    {
        return 0;
    }
    int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) override
    {
        return 0;
    }
    int32_t Connect(const BluetoothRawAddress &device) override
    {
        return 0;
    }
    int32_t Disconnect(const BluetoothRawAddress &device) override
    {
        return 0;
    }
    int GetScoState(const BluetoothRawAddress &device) override
    {
        return 0;
    }
    bool ConnectSco() override
    {
        return true;
    }
    bool DisconnectSco() override
    {
        return true;
    }
    int32_t ConnectSco(uint8_t callType) override
    {
        return 0;
    }
    int32_t DisconnectSco(uint8_t callType) override
    {
        return 0;
    }
    void PhoneStateChanged(BluetoothPhoneState &phoneState) override
    {}
    void ClccResponse(int index, int direction, int status, int mode, bool mpty, const std::string &number,
        int type) override
    {}
    bool OpenVoiceRecognition(const BluetoothRawAddress &device) override
    {
        return true;
    }
    bool CloseVoiceRecognition(const BluetoothRawAddress &device) override
    {
        return true;
    }
    int32_t IsAudioConnected(bool &isAudioOn) override
    {
        return 0;
    }
    bool SetActiveDevice(const BluetoothRawAddress &device) override
    {
        return true;
    }
    std::string GetActiveDevice() override
    {
        return nullptr;
    }
    bool IntoMock(const BluetoothRawAddress &device, int state) override
    {
        return true;
    }
    bool SendNoCarrier(const BluetoothRawAddress &device) override
    {
        return true;
    }
    void RegisterObserver(const sptr<IBluetoothHfpAgObserver> &observer) override {}
    void DeregisterObserver(const sptr<IBluetoothHfpAgObserver> &observer) override {}
    int SetConnectStrategy(const BluetoothRawAddress &device, int strategy) override
    {
        return 0;
    }
    int GetConnectStrategy(const BluetoothRawAddress &device, int &strategy) override
    {
        return 0;
    }

    int IsInbandRingingEnabled(bool &isEnabled) override
    {
        return 0;
    }

    void CallDetailsChanged(int callId, int callState) override
    {}

    void EnableBtCallLog(bool state) override
    {}

    int IsHfpFeatureSupported(const BluetoothRawAddress &device, bool &isSupported, int type) override
    {
        return 0;
    }

    void GetVirtualDeviceList(std::vector<std::string> &devices) override
    {}

    void UpdateVirtualDevice(int32_t action, const std::string &address) override
    {}

    int GetCurrentCallType(int &callType) override
    {
        return 0;
    }
    explicit BluetoothHfpAgObserverStubTest() {}
    ~BluetoothHfpAgObserverStubTest() override {}

private:
};

class BluetoothHfpAgObserverProxyTest : public testing::Test {
public:
    BluetoothHfpAgObserverProxyTest() {}
    ~BluetoothHfpAgObserverProxyTest() {}

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    sptr<BluetoothHfpAgObserverStubTest> bluetoothHfpAgObserverStubTest = new BluetoothHfpAgObserverStubTest();
    BluetoothHfpAgObserverProxy *proxy_ = new BluetoothHfpAgObserverProxy(bluetoothHfpAgObserverStubTest);
};

void BluetoothHfpAgObserverProxyTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBrEnable = %{public}d", isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}

void BluetoothHfpAgObserverProxyTest::TearDownTestSuite(void) {
    utility::CloseAllTimer();
}

void BluetoothHfpAgObserverProxyTest::SetUp() {}

void BluetoothHfpAgObserverProxyTest::TearDown() {}

/*
 * @tc.number: BluetoothHfpAgObserverProxy001
 * @tc.name: OnConnectionStateChanged
 * @tc.desc: OnConnectionStateChanged
 */
HWTEST_F(BluetoothHfpAgObserverProxyTest, bluetooth_hfp_Ag_observer_proxy_OnConnectionStateChanged_001, TestSize.Level1)
{
    HILOGI("bluetooth_hfp_Ag_observer_proxy_OnConnectionStateChanged_001 start");
    int state = 0;
    int cause = 0;
    proxy_->OnScoStateChanged(address, state);
    proxy_->OnConnectionStateChanged(address, state, cause);
    proxy_->OnActiveDeviceChanged(address);
    proxy_->OnHfEnhancedDriverSafetyChanged(address, state);
    EXPECT_TRUE(proxy_ != nullptr);
    HILOGI("bluetooth_hfp_Ag_observer_proxy_OnConnectionStateChanged_001 end");
}
} // namespace Bluetooth
} // namespace OHOS
