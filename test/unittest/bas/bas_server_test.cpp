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
#include <memory>
#include <string>
#include "bluetooth_bas_server.h"
#include "bluetooth_device_battery_observer_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
class BasServerTestObserver : public BluetoothDeviceBatteryObserverStub {
public:
    BasServerTestObserver() = default;
    ~BasServerTestObserver() = default;
    void OnGetBatteryLevelEvent(const BluetoothRawAddress &device, int32_t batteryLevel) {}
    void OnBatteryLevelChanged(const BluetoothRawAddress &device, int32_t batteryLevel) {}
};

class BasServerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("BasServerTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("BasServerTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("BasServerTest::SetUp start");
        std::string profileName = "BasService";
        ProfileServiceManager::GetInstance()->SetProfileService(ADAPTER_BREDR, profileName);
        basServer_ = std::make_unique<BluetoothBasServer>();
    }
    void TearDown() override
    {
        HILOGI("BasServerTest::TearDown start");
        basServer_ = nullptr;
    }

protected:
    std::unique_ptr<BluetoothBasServer> basServer_;
};

/*
 * @tc.number: Bas_Stub_OnRemoteRequest_00100
 * @tc.name: OnRemoteRequest
 * @tc.desc: Test OnRemoteRequest with invalid interface token
 */
HWTEST_F(BasServerTest, Bas_Stub_OnRemoteRequest_00100, TestSize.Level1)
{
    HILOGI("Bas_Stub_OnRemoteRequest_00100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {MessageOption::TF_SYNC};

    int ret = basServer_->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);

    HILOGI("Bas_Stub_OnRemoteRequest_00100 end");
}

/*
 * @tc.number: Bas_Stub_RegisterDeviceBatteryObserver_00100
 * @tc.name: RegisterDeviceBatteryObserver
 * @tc.desc: Test RegisterDeviceBatteryObserver
 */
HWTEST_F(BasServerTest, Bas_Stub_RegisterDeviceBatteryObserver_00100, TestSize.Level1)
{
    HILOGI("Bas_Stub_RegisterDeviceBatteryObserver_00100 start");

    sptr<IBluetoothDeviceBatteryObserver> observer = nullptr;
    int32_t ret = basServer_->RegisterDeviceBatteryObserver(observer);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    observer = new BasServerTestObserver();
    ret = basServer_->RegisterDeviceBatteryObserver(observer);
    EXPECT_EQ(ret, BT_NO_ERROR);

    HILOGI("Bas_Stub_RegisterDeviceBatteryObserver_00100 end");
}

/*
 * @tc.number: Bas_Stub_DeregisterDeviceBatteryObserver_00100
 * @tc.name: DeregisterDeviceBatteryObserver
 * @tc.desc: Test DeregisterDeviceBatteryObserver
 */
HWTEST_F(BasServerTest, Bas_Stub_DeregisterDeviceBatteryObserver_00100, TestSize.Level1)
{
    HILOGI("Bas_Stub_DeregisterDeviceBatteryObserver_00100 start");

    sptr<IBluetoothDeviceBatteryObserver> observer = nullptr;
    int32_t ret = basServer_->DeregisterDeviceBatteryObserver(observer);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    observer = new BasServerTestObserver();
    ret = basServer_->DeregisterDeviceBatteryObserver(observer);
    EXPECT_EQ(ret, BT_NO_ERROR);

    HILOGI("Bas_Stub_DeregisterDeviceBatteryObserver_00100 end");
}

/*
 * @tc.number: Bas_Stub_GetBatteryLevel_00100
 * @tc.name: GetBatteryLevel
 * @tc.desc: Test GetBatteryLevel with invalid address
 */
HWTEST_F(BasServerTest, Bas_Stub_GetBatteryLevel_00100, TestSize.Level1)
{
    HILOGI("Bas_Stub_GetBatteryLevel_00100 start");

    std::string address = "";
    int32_t ret = basServer_->GetBatteryLevel(address);
    EXPECT_NE(ret, BT_NO_ERROR);

    HILOGI("Bas_Stub_GetBatteryLevel_00100 end");
}

/*
 * @tc.number: Bas_Stub_GetBatteryLevel_00200
 * @tc.name: GetBatteryLevel
 * @tc.desc: Test GetBatteryLevel with non-existent address
 */
HWTEST_F(BasServerTest, Bas_Stub_GetBatteryLevel_00200, TestSize.Level1)
{
    HILOGI("Bas_Stub_GetBatteryLevel_00200 start");

    std::string address = "00:00:00:00:00:00";
    int32_t ret = basServer_->GetBatteryLevel(address);
    EXPECT_NE(ret, BT_NO_ERROR);

    HILOGI("Bas_Stub_GetBatteryLevel_00200 end");
}

/*
 * @tc.number: Bas_Stub_GetConnectedDeviceBatteryInfos_00100
 * @tc.name: GetConnectedDeviceBatteryInfos
 * @tc.desc: Test GetConnectedDeviceBatteryInfos
 */
HWTEST_F(BasServerTest, Bas_Stub_GetConnectedDeviceBatteryInfos_00100, TestSize.Level1)
{
    HILOGI("Bas_Stub_GetConnectedDeviceBatteryInfos_00100 start");

    std::map<std::string, int32_t> batteryInfos;
    int32_t ret = basServer_->GetConnectedDeviceBatteryInfos(batteryInfos);
    EXPECT_NE(ret, BT_NO_ERROR);

    HILOGI("Bas_Stub_GetConnectedDeviceBatteryInfos_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS