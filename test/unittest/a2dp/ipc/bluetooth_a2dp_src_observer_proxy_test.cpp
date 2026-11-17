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
#include "bluetooth_a2dp_src_observer_proxy.h"
#include "bluetooth_a2dp_src_stub.h"
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
RawAddress addr1("11:22:33:44:55:66");
Parcel parcel;
BluetoothA2dpCodecStatus *codecData = BluetoothA2dpCodecStatus::Unmarshalling(parcel);

class BluetoothA2dpSrcObserverStubTest : public BluetoothA2dpSrcStub {
public:
    int Connect(const RawAddress &device) override
    {
        return 0;
    }
    int Disconnect(const RawAddress &device) override
    {
        return 0;
    }
    void RegisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer) override {}
    void DeregisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer) override {}
    int GetDevicesByStates(const std::vector<int32_t> &states, std::vector<RawAddress> &rawAddrs) override
    {
        return 0;
    }
    int GetDeviceState(const RawAddress &device, int &state) override
    {
        return 0;
    }
    int GetPlayingState(const RawAddress &device, int &state) override
    {
        return 0;
    }
    int SetConnectStrategy(const RawAddress &device, int32_t strategy) override
    {
        return 0;
    }
    int GetConnectStrategy(const RawAddress &device, int &strategy) override
    {
        return 0;
    }
    int SetActiveSinkDevice(const RawAddress &device) override
    {
        return 0;
    }
    RawAddress GetActiveSinkDevice() override
    {
        return addr1;
    }
    BluetoothA2dpCodecStatus GetCodecStatus(const RawAddress &device) override
    {
        return *codecData;
    }

    int GetCodecPreference(const RawAddress &device, BluetoothA2dpCodecInfo &info) override
    {
        return 0;
    }

    int SetCodecPreference(const RawAddress &device, const BluetoothA2dpCodecInfo &info) override
    {
        return 0;
    }
    void SwitchOptionalCodecs(const RawAddress &device, bool isEnable) override {}
    int GetOptionalCodecsSupportState(const RawAddress &device) override
    {
        return 0;
    }
    int StartPlaying(const RawAddress &device) override
    {
        return 0;
    }
    int SuspendPlaying(const RawAddress &device) override
    {
        return 0;
    }
    int StopPlaying(const RawAddress &device) override
    {
        return 0;
    }
    int WriteFrame(const uint8_t *data, uint32_t size) override
    {
        return 0;
    }
    int GetRenderPosition(const RawAddress &device, uint32_t &delayValue, uint64_t &sendDataSize,
                          uint32_t &timeStamp) override
    {
        return 0;
    }

    int OffloadStartPlaying(const RawAddress &device, const std::vector<int32_t> &sessionsId) override
    {
        return 0;
    }

    int OffloadStopPlaying(const RawAddress &device, const std::vector<int32_t> &sessionsId) override
    {
        return 0;
    }

    int A2dpOffloadSessionPathRequest(const RawAddress &device,
        const std::vector<BluetoothA2dpStreamInfo> &info) override
    {
        return 0;
    }

    BluetoothA2dpOffloadCodecStatus GetOffloadCodecStatus(const RawAddress &device) override
    {
        BluetoothA2dpOffloadCodecStatus ret;
        return ret;
    }

    int EnableAutoPlay(const RawAddress &device) override
    {
        return 0;
    }
    
    int DisableAutoPlay(const RawAddress &device, const int duration)
    {
        return 0;
    }

    int GetAutoPlayDisabledDuration(const RawAddress &device, int &duration)
    {
        return 0;
    }

    void GetVirtualDeviceList(std::vector<std::string> &devices) override
    {}

    void UpdateVirtualDevice(int32_t action, const std::string &address) override
    {}

    explicit BluetoothA2dpSrcObserverStubTest() {}
    ~BluetoothA2dpSrcObserverStubTest() override {}

private:
};

class BluetoothA2dpSrcObserverProxyTest : public testing::Test {
public:
    BluetoothA2dpSrcObserverProxyTest() {}
    ~BluetoothA2dpSrcObserverProxyTest() {}

    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
    sptr<BluetoothA2dpSrcObserverStubTest> bluetoothA2dpSrcObserverStubTest = new BluetoothA2dpSrcObserverStubTest();
    BluetoothA2dpSrcObserverProxy *proxy_ = new BluetoothA2dpSrcObserverProxy(bluetoothA2dpSrcObserverStubTest);
};

void BluetoothA2dpSrcObserverProxyTest::SetUpTestSuite(void)
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

void BluetoothA2dpSrcObserverProxyTest::TearDownTestSuite(void) {
    utility::CloseAllTimer();
}

void BluetoothA2dpSrcObserverProxyTest::SetUp() {}

void BluetoothA2dpSrcObserverProxyTest::TearDown() {}

/*
 * @tc.number: BluetoothA2dpSrcObserverProxy001
 * @tc.name: OnConnectionStateChanged
 * @tc.desc: OnConnectionStateChanged
 */
HWTEST_F(BluetoothA2dpSrcObserverProxyTest, bluetooth_a2dp_src_observer_proxy_OnConnectionStateChanged_001,
    TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_observer_proxy_OnConnectionStateChanged_001 start");
    int state = 0;
    int cause = 0;
    // RawAddress addr;
    proxy_->OnConnectionStateChanged(addr1, state, cause);
    HILOGI("bluetooth_a2dp_src_observer_proxy_OnConnectionStateChanged_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcObserverProxy002
 * @tc.name: OnPlayingStatusChanged
 * @tc.desc: OnPlayingStatusChanged
 */
HWTEST_F(BluetoothA2dpSrcObserverProxyTest, bluetooth_a2dp_src_observer_proxy_OnPlayingStatusChanged_001,
    TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_observer_proxy_OnPlayingStatusChanged_001 start");
    int state = 0;
    // RawAddress addr;
    proxy_->OnPlayingStatusChanged(addr1, state, state);
    HILOGI("bluetooth_a2dp_src_observer_proxy_OnPlayingStatusChanged_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcObserverProxy003
 * @tc.name: OnConfigurationChanged
 * @tc.desc: OnConfigurationChanged
 */
HWTEST_F(BluetoothA2dpSrcObserverProxyTest, bluetooth_a2dp_src_observer_proxy_OnConfigurationChanged_001,
    TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_observer_proxy_OnConfigurationChanged_001 start");
    int state = 0;
    BluetoothA2dpCodecInfo info;
    proxy_->OnConfigurationChanged(addr1, info, state);
    HILOGI("bluetooth_a2dp_src_observer_proxy_OnConfigurationChanged_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcObserverProxy004
 * @tc.name: InnerTransact
 * @tc.desc: InnerTransact
 */
HWTEST_F(BluetoothA2dpSrcObserverProxyTest, bluetooth_a2dp_src_observer_proxy_InnerTransact_001, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_observer_proxy_InnerTransact_001 start");
    int state = BT_A2DP_SRC_OBSERVER_CONNECTION_STATE_CHANGED;
    MessageOption flags;
    MessageParcel data;
    MessageParcel reply;
    int ret = proxy_->InnerTransact(state, flags, data, reply);
    EXPECT_EQ(TRANSACTION_ERR, ret);
    state = BT_A2DP_SRC_OBSERVER_PLAYING_STATUS_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    state = BT_A2DP_SRC_OBSERVER_CONFIGURATION_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    state = BT_A2DP_SRC_OBSERVER_MEDIASTACK_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    state = BT_A2DP_SRC_OBSERVER_VIRTUALDEVICE_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    state = BT_A2DP_SRC_OBSERVER_CAPTURE_CONNECTION_STATE_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    HILOGI("bluetooth_a2dp_src_observer_proxy_InnerTransact_001 end");
}

/*
 * @tc.number: BluetoothA2dpSrcObserverProxy005
 * @tc.name: InnerTransact
 * @tc.desc: InnerTransact
 */
HWTEST_F(BluetoothA2dpSrcObserverProxyTest, bluetooth_a2dp_src_observer_proxy_InnerTransact_002, TestSize.Level1)
{
    HILOGI("bluetooth_a2dp_src_observer_proxy_InnerTransact_002 start");
    int state = BT_A2DP_SRC_OBSERVER_CONNECTION_STATE_CHANGED;
    MessageOption flags;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(BluetoothA2dpSrcStub::GetDescriptor());
    data.WriteString("11:22:33:44:55:66");
    int ret = proxy_->InnerTransact(state, flags, data, reply);
    EXPECT_EQ(ERR_OK, ret);
    state = BT_A2DP_SRC_OBSERVER_PLAYING_STATUS_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    state = BT_A2DP_SRC_OBSERVER_CONFIGURATION_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    state = BT_A2DP_SRC_OBSERVER_MEDIASTACK_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    state = BT_A2DP_SRC_OBSERVER_VIRTUALDEVICE_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    state = BT_A2DP_SRC_OBSERVER_CAPTURE_CONNECTION_STATE_CHANGED;
    proxy_->InnerTransact(state, flags, data, reply);
    HILOGI("bluetooth_a2dp_src_observer_proxy_InnerTransact_002 end");
}

/*
 * @tc.number: BluetoothA2dpSrcObserverProxy006
 * @tc.name: OnVirtualDeviceChanged
 * @tc.desc: OnVirtualDeviceChanged
 */
HWTEST_F(BluetoothA2dpSrcObserverProxyTest, BluetoothA2dpSrcObserverProxy007, TestSize.Level1)
{
    HILOGI("BluetoothA2dpSrcObserverProxy007 start");
    int action = 0;
    proxy_->OnMediaStackChanged(addr1, action);
    std::string address = "11:22:33:44:55:66";
    proxy_->OnVirtualDeviceChanged(action, address);
    int state = 1;
    BluetoothA2dpCodecInfo info;
    proxy_->OnCaptureConnectionStateChanged(addr1, state, info);
    OHOS::Parcel parcel;
    info.WriteToParcel(parcel);
    EXPECT_TRUE(proxy_ != nullptr);
    HILOGI("BluetoothA2dpSrcObserverProxy007 end");
}
} // namespace Bluetooth
} // namespace OHOS
