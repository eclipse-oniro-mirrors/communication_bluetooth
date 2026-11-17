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

#include "bluetooth_avrcp_tg_observer_stub.h"
#include "bluetooth_avrcp_tg_server.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"
#include "bluetooth_errorcode.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;

class BluetoothAvrcpCtObserverTest : public BluetoothAvrcpTgObserverStub {
public:
    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override {}

    explicit BluetoothAvrcpCtObserverTest()
    {}
    ~BluetoothAvrcpCtObserverTest() override
    {}

private:
};

class AvrcpTgServerTest : public testing::Test {
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
    std::unique_ptr<BluetoothAvrcpTgServer> avrcpTgServer_;
    BluetoothRawAddress addr_ = BluetoothRawAddress("11:22:33:44:55:66");
};

void AvrcpTgServerTest::SetUp()
{
    ProfileServiceManager::GetInstance()->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, "AvrcpTgService");
    avrcpTgServer_ = std::make_unique<BluetoothAvrcpTgServer>();
}

void AvrcpTgServerTest::TearDown()
{
    avrcpTgServer_ = nullptr;
}

/**
 * @tc.name: avrcp_tg_server_register_001
 * @tc.desc: AvrcpTgServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgServerTest, avrcp_tg_server_register_001, TestSize.Level1)
{
    HILOGI("avrcp_tg_server_register_001 enter");
    sptr<BluetoothAvrcpCtObserverTest> observer = nullptr;
    avrcpTgServer_->RegisterObserver(observer);
    avrcpTgServer_->UnregisterObserver(observer);

    observer = new BluetoothAvrcpCtObserverTest();
    avrcpTgServer_->RegisterObserver(observer);
    avrcpTgServer_->UnregisterObserver(observer);
    EXPECT_TRUE(avrcpTgServer_ != nullptr);
    HILOGI("avrcp_tg_server_register_001 end");
}

/**
 * @tc.name: avrcp_tg_server_connect_001
 * @tc.desc: AvrcpTgServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgServerTest, avrcp_tg_server_connect_001, TestSize.Level1)
{
    HILOGI("avrcp_tg_server_connect_001 enter");
    avrcpTgServer_->Connect(addr_);
    avrcpTgServer_->SetActiveDevice(addr_);
    avrcpTgServer_->GetConnectedDevices();
    avrcpTgServer_->Disconnect(addr_);
    int32_t volumeLevel = 0;
    EXPECT_EQ(avrcpTgServer_->SetDeviceAbsoluteVolume(addr_, volumeLevel), BT_ERR_INTERNAL_ERROR);
    HILOGI("avrcp_tg_server_connect_001 end");
}

/**
 * @tc.name: avrcp_tg_server_devicestate_001
 * @tc.desc: AvrcpTgServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgServerTest, avrcp_tg_server_devicestate_001, TestSize.Level1)
{
    HILOGI("avrcp_tg_server_devicestate_001 enter");
    std::vector<int32_t> states;
    EXPECT_TRUE(avrcpTgServer_->GetDevicesByStates(states).empty());
    EXPECT_EQ(avrcpTgServer_->GetDeviceState(addr_), BT_NO_ERROR);
    HILOGI("avrcp_tg_server_devicestate_001 end");
}

/**
 * @tc.name: avrcp_tg_server_notify_001
 * @tc.desc: AvrcpTgServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgServerTest, avrcp_tg_server_notify_001, TestSize.Level1)
{
    HILOGI("avrcp_tg_server_notify_001 enter");
    avrcpTgServer_->NotifyPlaybackStatusChanged(0, 0);
    avrcpTgServer_->NotifyTrackChanged(0, 0);
    avrcpTgServer_->NotifyTrackReachedEnd(0);
    avrcpTgServer_->NotifyTrackReachedStart(0);
    avrcpTgServer_->NotifyPlaybackPosChanged(0);

    std::vector<int32_t> attributes;
    std::vector<int32_t> values;
    avrcpTgServer_->NotifyPlayerAppSettingChanged(attributes, values);

    avrcpTgServer_->NotifyNowPlayingContentChanged();
    avrcpTgServer_->NotifyAvailablePlayersChanged();
    avrcpTgServer_->NotifyAddressedPlayerChanged(0, 0);

    avrcpTgServer_->NotifyUidChanged(0);
    avrcpTgServer_->NotifyVolumeChanged(0);

    int32_t ability = 0;
    EXPECT_EQ(avrcpTgServer_->SetDeviceAbsVolumeAbility(addr_, ability), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(avrcpTgServer_->GetDeviceAbsVolumeAbility(addr_, ability), BT_ERR_INTERNAL_ERROR);
    HILOGI("avrcp_tg_server_notify_001 end");
}

/**
 * @tc.name: avrcp_tg_server_register_observer_002
 * @tc.desc: AvrcpTgServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpTgServerTest, avrcp_tg_server_register_observer_002, TestSize.Level1)
{
    HILOGI("avrcp_tg_server_register_observer_002 enter");
    std::unique_ptr<BluetoothAvrcpTgServer> avrcpTgServerTest = std::make_unique<BluetoothAvrcpTgServer>();
    for (int i = 0; i <= 1001; i++) {
        sptr<IBluetoothAvrcpTgObserver> observer;
        avrcpTgServer_->RegisterObserver(observer);
    }
    EXPECT_TRUE(avrcpTgServer_ != nullptr);
    HILOGI("avrcp_tg_server_register_observer_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS