/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "bluetooth_log.h"
#include "bluetooth_a2dp_sink_server.h"
#include "bluetooth_host.h"
#include "bluetooth_a2dp_sink_observer_stub.h"
#include "profile_service_manager.h"


using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;

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

class A2dpSinkServerTest : public testing::Test {
public:
    static void SetUpTestCase()
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
    static void TearDownTestCase()
    {
    }
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<BluetoothA2dpSinkServer> a2dpSinkServer_;
};

void A2dpSinkServerTest::SetUp()
{
    a2dpSinkServer_ = std::make_unique<BluetoothA2dpSinkServer>();
}

void A2dpSinkServerTest::TearDown()
{
    a2dpSinkServer_ = nullptr;
}

/**
 * @tc.name: a2dp_sink_server_register_001
 * @tc.desc: A2dpSinkServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSinkServerTest, a2dp_sink_server_register_001, TestSize.Level1)
{
    HILOGI("a2dp_sink_server_register_001 enter");
    sptr<BluetoothA2dpSinkObserverStubTest> observer = nullptr;
    a2dpSinkServer_->RegisterObserver(observer);
    a2dpSinkServer_->DeregisterObserver(observer);

    observer = new BluetoothA2dpSinkObserverStubTest();
    a2dpSinkServer_->RegisterObserver(observer);
    a2dpSinkServer_->DeregisterObserver(observer);
    EXPECT_TRUE(a2dpSinkServer_ != NULL);
    HILOGI("a2dp_sink_server_register_001 end");
}

/**
 * @tc.name: a2dp_sink_server_null_service_001
 * @tc.desc: call all interfaces when a2dpSnkService_ is not created, should not crash
 * @tc.type: FUNC
 */
HWTEST_F(A2dpSinkServerTest, a2dp_sink_server_null_service_001, TestSize.Level1)
{
    HILOGI("a2dp_sink_server_null_service_001 enter");
    BluetoothRawAddress device("11:22:33:44:55:66");

    a2dpSinkServer_->Connect(device);
    a2dpSinkServer_->Disconnect(device);
    a2dpSinkServer_->GetDeviceState(device);

    std::vector<int32_t> states;
    EXPECT_TRUE(a2dpSinkServer_->GetDevicesByStates(states).empty());

    int state = 0;
    a2dpSinkServer_->GetPlayingState(device, state);
    a2dpSinkServer_->SetConnectStrategy(device, 0);
    a2dpSinkServer_->GetConnectStrategy(device);
    HILOGI("a2dp_sink_server_null_service_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS