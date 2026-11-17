/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "uuid.h"
#include "vector"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 2;
class GattClientCallbackTest : public GattClientCallback {
public:
    GattClientCallbackTest()
    {}
    ~GattClientCallbackTest()
    {}

    void OnConnectionStateChanged(int connectionState, int ret)
    {
        HILOGI("GattClientCallbackTest::OnConnectionStateChanged called");
    }

    void OnCharacteristicChanged(const GattCharacteristic &characteristic)
    {
        HILOGI("GattClientCallbackTest::OnCharacteristicChanged called");
    }

    void OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
    {
        HILOGI("GattClientCallbackTest::OnCharacteristicReadResult called");
    }

    void OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
    {
        HILOGI("GattClientCallbackTest::OnCharacteristicWriteResult called");
    }

    void OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
    {
        HILOGI("GattClientCallbackTest::OnDescriptorReadResult called");
    }

    void OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
    {
        HILOGI("GattClientCallbackTest::OnDescriptorWriteResult called");
    }

    void OnMtuUpdate(int mtu, int ret)
    {
        HILOGI("GattClientCallbackTest::OnMtuUpdate called");
    }

    void OnServicesDiscovered(int status)
    {
        HILOGI("GattClientCallbackTest::OnServicesDiscovered called");
    }

    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status)
    {
        HILOGI("GattClientCallbackTest::OnConnectionParameterChanged called");
    }

    void OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status)
    {
        HILOGI("GattClientCallbackTest::OnSetNotifyCharacteristic called");
    }
};
static BluetoothHost *host_;
class GattManagerTest : public testing::Test {
public:
    GattManagerTest()
    {}
    ~GattManagerTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetRandomAddr(BluetoothRemoteDevice &device);
};

void GattManagerTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
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
void GattManagerTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
    host_ = nullptr;
}
void GattManagerTest::SetUp()
{}

void GattManagerTest::TearDown()
{}

void GattManagerTest::SetRandomAddr(BluetoothRemoteDevice &device)
{
    string randomAddr;
    host_->GetRandomAddress(device.GetDeviceAddr(), randomAddr);
    device.address_ = randomAddr;
}

/*
 * @tc.number: GattManager001
 * @tc.name: GetDevicesByStates
*/
HWTEST_F(GattManagerTest, GattManager_UnitTest_GetDevicesByStates, TestSize.Level1)
{
    HILOGI("GattManager::GattManager ends");
    BluetoothRemoteDevice device;
    SetRandomAddr(device);
    GattClient client(device);
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);

    std::array<int, GattManager::GATT_CONNECTION_STATE_NUM> states = {1, 2, 3};

    GattManager test;
    test.GetDevicesByStates(states);

    HILOGI("GattManager::GetDevicesByState ends");
}

/*
 * @tc.number: GattManager002
 * @tc.name: GetConnectedDevices
*/
HWTEST_F(GattManagerTest, GattManager_UnitTest_GetConnectedDevices, TestSize.Level1)
{
    HILOGI("GattManager::GattManager ends");
    BluetoothRemoteDevice device;
    GattClient client(device);
    std::shared_ptr<GattClientCallbackTest> callback_ = std::make_shared<GattClientCallbackTest>();
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);

    GattManager test;

    EXPECT_EQ((int)test.GetConnectedDevices().size(), 0);

    HILOGI("GattManager::GetConnectedDevices ends");
}

}  // namespace Bluetooth
}  // namespace OHOS
