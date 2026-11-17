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
#include "bluetooth_highpower_manager.h"
#include "bluetooth_host.h"
#include "adapter_manager.h"
#include "bluetooth_common_event_subscriber.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_errorcode.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "thread_util.h"
#include "parameter.h"
#include "wifi_device.h"
#include "mock/mock_wifi_device.h"
#include "mock/mock_wifi_hotspot.h"

using namespace OHOS::EventFwk;
using namespace OHOS::Bluetooth;
using namespace OHOS::Wifi;
using namespace testing;
using namespace testing::ext;
using ::testing::_;
using ::testing::Invoke;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
static BluetoothHost *host_;
static std::shared_ptr<OHOS::Wifi::MockWifiDevice> wifiDeviceMock = std::make_shared<OHOS::Wifi::MockWifiDevice>();
static std::shared_ptr<OHOS::Wifi::MockWifiHotSpot> wifiHotSpotMock = std::make_shared<OHOS::Wifi::MockWifiHotSpot>();

class BluetoothHighPowerManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        host_ = &BluetoothHost::GetDefaultHost();
        BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        auto manager = ProfileServiceManager::GetInstance();
        std::string hfpAgProfileName = "HfpAgService";
        manager->SetProfileService(ADAPTER_BREDR, hfpAgProfileName);
        std::string a2dpSrcProfileName = "A2dpSrcService";
        manager->SetProfileService(ADAPTER_BREDR, a2dpSrcProfileName);
        BluetoothHighPowerManager::GetInstance().Init();
        BluetoothHighPowerManager::GetInstance().wifiDevice_ = wifiDeviceMock;
        BluetoothHighPowerManager::GetInstance().wifiHotSpot_ = wifiHotSpotMock;
        if (BluetoothHighPowerManager::GetInstance().btHighPowerEventType_.empty()) {
            BluetoothHighPowerManager::GetInstance().btHighPowerEventType_ = {
                {CommonEventSupport::COMMON_EVENT_SCREEN_ON, BtHighPowerEventType::SCREEN_ON},
                {CommonEventSupport::COMMON_EVENT_SCREEN_OFF, BtHighPowerEventType::SCREEN_OFF},
                {CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE, BtHighPowerEventType::WIFI_POWER_STATE},
                {CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE, BtHighPowerEventType::WIFI_CONN_STATE},
                {CommonEventSupport::COMMON_EVENT_WIFI_HOTSPOT_STATE, BtHighPowerEventType::WIFI_HOTSPOT_STATE},
                {CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN, BtHighPowerEventType::WIFI_AP_STA_JOIN},
                {CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE, BtHighPowerEventType::WIFI_AP_STA_LEAVE},
                {CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE,
                    BtHighPowerEventType::BLUETOOTH_HOST_STATE_UPDATE},
            };
        }
    }
    static void TearDownTestCase()
    {
        wifiDeviceMock = nullptr;
        wifiHotSpotMock = nullptr;
    }
    void SetUp()
    {
        BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
        BluetoothHighPowerManager::GetInstance().isBluetoothEnable_ = true;
    }
    void TearDown()
    {
        BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    }
};

/**
 * @tc.name: HighPower_Mgr_GetWifiStaPowerState_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetWifiStaPowerState_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetWifiStaPowerState_001 enter");
    EXPECT_CALL(*wifiDeviceMock, GetWifiState(_)).Times(1)
        .WillOnce(Invoke([](int &state) -> OHOS::Wifi::ErrCode {
            state = static_cast<int>(Wifi::WifiState::ENABLED);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().GetWifiStaPowerState());
    HILOGI("HighPower_Mgr_GetWifiStaPowerState_001 end");
}

/**
 * @tc.name: HighPower_Mgr_GetWifiStaPowerState_002
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetWifiStaPowerState_002, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetWifiStaPowerState_002 enter");
    EXPECT_CALL(*wifiDeviceMock, GetWifiState(_)).Times(1)
        .WillOnce(Invoke([](int &state) -> OHOS::Wifi::ErrCode {
            state = static_cast<int>(Wifi::WifiState::DISABLED);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().GetWifiStaPowerState());
    HILOGI("HighPower_Mgr_GetWifiStaPowerState_002 end");
}

/**
 * @tc.name: HighPower_Mgr_GetWifiStaPowerState_003
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetWifiStaPowerState_003, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetWifiStaPowerState_003 enter");
    EXPECT_CALL(*wifiDeviceMock, GetWifiState(_)).Times(1)
        .WillOnce(Invoke([](int &state) -> OHOS::Wifi::ErrCode {
            state = static_cast<int>(Wifi::WifiState::ENABLED);
            return OHOS::Wifi::ErrCode::WIFI_OPT_FAILED;
    }));
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().GetWifiStaPowerState());
    HILOGI("HighPower_Mgr_GetWifiStaPowerState_003 end");
}
/**
 * @tc.name: HighPower_Mgr_GetWifiHotSpotPowerState_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetWifiHotSpotPowerState_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetWifiHotSpotPowerState_001 enter");
    EXPECT_CALL(*wifiHotSpotMock, GetHotspotState(_)).Times(1)
        .WillOnce(Invoke([](int &state) -> OHOS::Wifi::ErrCode {
            state = static_cast<int>(Wifi::ApState::AP_STATE_STARTED);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().GetWifiHotSpotPowerState());
    HILOGI("HighPower_Mgr_GetWifiHotSpotPowerState_001 end");
}

/**
 * @tc.name: HighPower_Mgr_GetWifiHotSpotPowerState_002
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetWifiHotSpotPowerState_002, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetWifiHotSpotPowerState_002 enter");
    EXPECT_CALL(*wifiHotSpotMock, GetHotspotState(_)).Times(1)
        .WillOnce(Invoke([](int &state) -> OHOS::Wifi::ErrCode {
            state = static_cast<int>(Wifi::ApState::AP_STATE_CLOSED);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().GetWifiHotSpotPowerState());
    HILOGI("HighPower_Mgr_GetWifiHotSpotPowerState_002 end");
}

/**
 * @tc.name: HighPower_Mgr_GetWifiHotSpotPowerState_003
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetWifiHotSpotPowerState_003, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetWifiHotSpotPowerState_003 enter");
    EXPECT_CALL(*wifiHotSpotMock, GetHotspotState(_)).Times(1)
        .WillOnce(Invoke([](int &state) -> OHOS::Wifi::ErrCode {
            state = static_cast<int>(Wifi::ApState::AP_STATE_STARTED);
            return OHOS::Wifi::ErrCode::WIFI_OPT_FAILED;
        }));
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().GetWifiHotSpotPowerState());
    HILOGI("HighPower_Mgr_GetWifiHotSpotPowerState_003 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateWifiState_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateWifiState_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateWifiState_001 enter");
    BluetoothHighPowerManager::GetInstance().isWifiEnable_ = true;
    EXPECT_CALL(*wifiDeviceMock, GetLinkedInfo(_)).Times(1)
        .WillOnce(Invoke([](Wifi::WifiLinkedInfo &info) -> OHOS::Wifi::ErrCode {
            info.connState = Wifi::ConnState::CONNECTED;
            info.frequency = 5160;
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    BluetoothHighPowerManager::GetInstance().UpdateWifiState();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().wifiSta5GhzActive_);
    HILOGI("HighPower_Mgr_UpdateWifiState_001 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateWifiState_002
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateWifiState_002, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateWifiState_002 enter");
    BluetoothHighPowerManager::GetInstance().isWifiEnable_ = true;
    EXPECT_CALL(*wifiDeviceMock, GetLinkedInfo(_)).Times(1)
        .WillOnce(Invoke([](Wifi::WifiLinkedInfo &info) -> OHOS::Wifi::ErrCode {
            info.connState = Wifi::ConnState::CONNECTED;
            info.frequency = 2412;
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    BluetoothHighPowerManager::GetInstance().UpdateWifiState();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().wifiSta2GhzActive_);
    HILOGI("HighPower_Mgr_UpdateWifiState_002 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateWifiState_003
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateWifiState_003, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateWifiState_003 enter");
    BluetoothHighPowerManager::GetInstance().isWifiEnable_ = true;
    BluetoothHighPowerManager::GetInstance().InitWifiState();
    EXPECT_CALL(*wifiDeviceMock, GetLinkedInfo(_)).Times(1)
        .WillOnce(Invoke([](Wifi::WifiLinkedInfo &info) -> OHOS::Wifi::ErrCode {
            return OHOS::Wifi::ErrCode::WIFI_OPT_FAILED;
        }));
    BluetoothHighPowerManager::GetInstance().UpdateWifiState();
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().wifiSta2GhzActive_);
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().wifiSta5GhzActive_);
    HILOGI("HighPower_Mgr_UpdateWifiState_003 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateWifiState_004
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateWifiState_004, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateWifiState_004 enter");
    BluetoothHighPowerManager::GetInstance().isWifiEnable_ = false;
    BluetoothHighPowerManager::GetInstance().UpdateWifiState();
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().wifiSta2GhzActive_);
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().wifiSta5GhzActive_);
    HILOGI("HighPower_Mgr_UpdateWifiState_004 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateSoftApState_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateSoftApState_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateSoftApState_001 enter");
    BluetoothHighPowerManager::GetInstance().isHotSpotEnable_ = true;

    EXPECT_CALL(*wifiHotSpotMock, GetStationList(_)).Times(1)
        .WillOnce(Invoke([](std::vector<StationInfo> &result) -> OHOS::Wifi::ErrCode {
            StationInfo staClient = {"test_deviceName", "AA:BB:CC:DD:EE:FF", 1, "127.0.0.2"};
            result.push_back(staClient);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_CALL(*wifiHotSpotMock, GetHotspotConfig(_)).Times(1)
        .WillOnce(Invoke([](HotspotConfig &config) -> OHOS::Wifi::ErrCode {
            config.SetBand(Wifi::BandType::BAND_2GHZ);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    BluetoothHighPowerManager::GetInstance().UpdateSoftApState();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().softAp2GhzActive_);
    HILOGI("HighPower_Mgr_UpdateSoftApState_001 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateSoftApState_002
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateSoftApState_002, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateSoftApState_002 enter");
    BluetoothHighPowerManager::GetInstance().isHotSpotEnable_ = true;

    EXPECT_CALL(*wifiHotSpotMock, GetStationList(_)).Times(1)
        .WillOnce(Invoke([](std::vector<StationInfo> &result) -> OHOS::Wifi::ErrCode {
            StationInfo staClient = {"test_deviceName", "AA:BB:CC:DD:EE:FF", 1, "127.0.0.2"};
            result.push_back(staClient);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_CALL(*wifiHotSpotMock, GetHotspotConfig(_)).Times(1)
        .WillOnce(Invoke([](HotspotConfig &config) -> OHOS::Wifi::ErrCode {
            config.SetBand(Wifi::BandType::BAND_5GHZ);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    BluetoothHighPowerManager::GetInstance().UpdateSoftApState();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().softAp5GhzActive_);
    HILOGI("HighPower_Mgr_UpdateSoftApState_002 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateSoftApState_003
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateSoftApState_003, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateSoftApState_003 enter");
    BluetoothHighPowerManager::GetInstance().isHotSpotEnable_ = true;

    EXPECT_CALL(*wifiHotSpotMock, GetStationList(_)).Times(1)
        .WillOnce(Invoke([](std::vector<StationInfo> &result) -> OHOS::Wifi::ErrCode {
            StationInfo staClient = {"test_deviceName", "AA:BB:CC:DD:EE:FF", 1, "127.0.0.2"};
            result.push_back(staClient);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_CALL(*wifiHotSpotMock, GetHotspotConfig(_)).Times(1)
        .WillOnce(Invoke([](HotspotConfig &config) -> OHOS::Wifi::ErrCode {
            return OHOS::Wifi::ErrCode::WIFI_OPT_FAILED;
        }));
    BluetoothHighPowerManager::GetInstance().UpdateSoftApState();
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().softAp5GhzActive_);
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().softAp2GhzActive_);
    HILOGI("HighPower_Mgr_UpdateSoftApState_003 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateSoftApState_004
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateSoftApState_004, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateSoftApState_004 enter");
    BluetoothHighPowerManager::GetInstance().isHotSpotEnable_ = true;

    EXPECT_CALL(*wifiHotSpotMock, GetStationList(_)).Times(1)
    .WillOnce(Invoke([](std::vector<StationInfo> &result) -> OHOS::Wifi::ErrCode {
        return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
    }));
    BluetoothHighPowerManager::GetInstance().UpdateSoftApState();
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().softAp5GhzActive_);
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().softAp2GhzActive_);
    HILOGI("HighPower_Mgr_UpdateSoftApState_004 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateSoftApState_005
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateSoftApState_005, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateSoftApState_005 enter");
    BluetoothHighPowerManager::GetInstance().isHotSpotEnable_ = false;
    BluetoothHighPowerManager::GetInstance().UpdateSoftApState();
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().softAp5GhzActive_);
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().softAp2GhzActive_);
    HILOGI("HighPower_Mgr_UpdateSoftApState_005 end");
}

/**
 * @tc.name: HighPower_Mgr_SetHighPowerModeEnable_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_SetHighPowerModeEnable_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_SetHighPowerModeEnable_001 enter");
    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_LEVEL7;
    BluetoothHighPowerManager::GetInstance().SetHighPowerModeEnable(BT_HIGH_POWER_LEVEL7);
    EXPECT_EQ(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_, BT_HIGH_POWER_LEVEL7);
    HILOGI("HighPower_Mgr_SetHighPowerModeEnable_001 end");
}

/**
 * @tc.name: HighPower_Mgr_SetHighPowerModeEnable_002
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_SetHighPowerModeEnable_002, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_SetHighPowerModeEnable_002 enter");
    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().SetHighPowerModeEnable(BT_HIGH_POWER_LEVEL7);
    EXPECT_EQ(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_, BT_HIGH_POWER_LEVEL7);
    HILOGI("HighPower_Mgr_SetHighPowerModeEnable_002 end");
}

/**
 * @tc.name: HighPower_Mgr_HandleBluetoothStateChange_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_HandleBluetoothStateChange_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_HandleBluetoothStateChange_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().isBluetoothEnable_ = false;
    BluetoothHighPowerManager::GetInstance().HandleBluetoothStateChange(bluetooth::BTStateID::STATE_TURN_ON);
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().isBluetoothEnable_);
    HILOGI("HighPower_Mgr_HandleBluetoothStateChange_001 end");
}

/**
 * @tc.name: HighPower_Mgr_HandleBluetoothStateChange_002
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_HandleBluetoothStateChange_002, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_HandleBluetoothStateChange_002 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().HandleBluetoothStateChange(bluetooth::BTStateID::STATE_TURN_OFF);
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().isBluetoothEnable_);
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().currentBtScenario_ == BluetoothScenario::BT_INACTIVE);
    HILOGI("HighPower_Mgr_HandleBluetoothStateChange_002 end");
}

/**
 * @tc.name: HighPower_Mgr_HandleBluetoothStateChange_003
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_HandleBluetoothStateChange_003, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_HandleBluetoothStateChange_003 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().HandleBluetoothStateChange(bluetooth::BTStateID::STATE_TURN_ON);
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().isBluetoothEnable_);
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().currentBtScenario_ == BluetoothScenario::BT_INACTIVE);
    HILOGI("HighPower_Mgr_HandleBluetoothStateChange_003 end");
}

/**
 * @tc.name: UpdateHighPowerModeOnBtWorking
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateHighPowerModeOnBtWorking_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateHighPowerModeOnBtPlaying_001 enter");
    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_2GHZ_ACTIVE;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerModeOnBtWorking();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);

    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_INACTIVE;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerModeOnBtWorking();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);

    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_5GZ_ACTIVE;
    BluetoothHighPowerManager::GetInstance().isScreenOnNow_ = false;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerModeOnBtWorking();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);


    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_5GZ_ACTIVE;
    BluetoothHighPowerManager::GetInstance().isScreenOnNow_ = true;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerModeOnBtWorking();
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);

    HILOGI("HighPower_Mgr_UpdateHighPowerModeOnBtPlaying_001 end");
}

/**
 * @tc.name: HighPower_Mgr_GetWifiScenario_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetWifiScenario_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetWifiScenario_001 enter");
    BluetoothHighPowerManager::GetInstance().InitSoftApState();
    BluetoothHighPowerManager::GetInstance().InitWifiState();
    BluetoothHighPowerManager::GetInstance().wifiSta2GhzActive_ = true;
    WifiScenario res = BluetoothHighPowerManager::GetInstance().GetWifiScenario();
    EXPECT_TRUE(res == WifiScenario::WIFI_2GHZ_ACTIVE);

    BluetoothHighPowerManager::GetInstance().InitSoftApState();
    BluetoothHighPowerManager::GetInstance().InitWifiState();
    BluetoothHighPowerManager::GetInstance().softAp5GhzActive_ = true;
    res = BluetoothHighPowerManager::GetInstance().GetWifiScenario();
    EXPECT_TRUE(res == WifiScenario::WIFI_5GZ_ACTIVE);

    BluetoothHighPowerManager::GetInstance().InitSoftApState();
    BluetoothHighPowerManager::GetInstance().InitWifiState();
    res = BluetoothHighPowerManager::GetInstance().GetWifiScenario();
    EXPECT_TRUE(res == WifiScenario::WIFI_INACTIVE);

    HILOGI("HighPower_Mgr_GetWifiScenario_001 end");
}

/**
 * @tc.name: HighPower_Mgr_UpdateHighPowerMode_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_UpdateHighPowerMode_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_UpdateHighPowerMode_001 enter");
    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_LEVEL7;
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerMode();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);

    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_LEVEL7;
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_INACTIVE;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerMode();
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);

    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_PLAYING;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_2GHZ_ACTIVE;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerMode();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);

    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_CELLULAR_CALLING;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerMode();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);

    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_VOIP_CALLING;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_INACTIVE;
    BluetoothHighPowerManager::GetInstance().UpdateHighPowerMode();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);
    HILOGI("HighPower_Mgr_UpdateHighPowerMode_001 end");
}

/**
 * @tc.name: HighPower_Mgr_HandleScreenStateChange_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_HandleScreenStateChange_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_HandleScreenStateChange_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().isScreenOnNow_ = false;
    BluetoothHighPowerManager::GetInstance().HandleScreenStateChange(true);
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().isScreenOnNow_);

    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothHighPowerManager::GetInstance().isScreenOnNow_ = false;
    BluetoothHighPowerManager::GetInstance().HandleScreenStateChange(true);
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().isScreenOnNow_);

    HILOGI("HighPower_Mgr_HandleScreenStateChange_001 end");
}

/**
 * @tc.name: HighPower_Mgr_GetEventType_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetEventType_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetEventType_001 enter");
    std::string eventString = "usual.event.wifi.POWER_STATE";
    BtHighPowerEventType type = BluetoothHighPowerManager::GetInstance().GetEventType(eventString);
    EXPECT_TRUE(type == BtHighPowerEventType::WIFI_POWER_STATE);
    HILOGI("HighPower_Mgr_GetEventType_001 end");
}

/**
 * @tc.name: HighPower_Mgr_HandleWifiStateChange_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_HandleWifiStateChange_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_HandleWifiStateChange_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().isWifiEnable_ = false;
    BluetoothHighPowerManager::GetInstance().HandleWifiStateChange(static_cast<int>(Wifi::WifiState::ENABLED));
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().isWifiEnable_);

    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().isWifiEnable_ = false;
    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_CELLULAR_CALLING;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_INACTIVE;
    EXPECT_CALL(*wifiDeviceMock, GetLinkedInfo(_)).Times(1)
    .WillOnce(Invoke([](Wifi::WifiLinkedInfo &info) -> OHOS::Wifi::ErrCode {
        info.connState = Wifi::ConnState::CONNECTED;
        info.frequency = 2412;
        return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
    }));
    BluetoothHighPowerManager::GetInstance().HandleWifiStateChange(static_cast<int>(Wifi::WifiState::ENABLED));
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().currentWifiScenario_ == WifiScenario::WIFI_2GHZ_ACTIVE);
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);
    HILOGI("HighPower_Mgr_HandleWifiStateChange_001 end");
}

/**
 * @tc.name: HighPower_Mgr_HandleWifiConnectStateChange_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_HandleWifiConnectStateChange_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_HandleWifiConnectStateChange_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().isWifiConnected_ = false;
    BluetoothHighPowerManager::GetInstance()
        .HandleWifiConnectStateChange(static_cast<int>(OHOS::Wifi::ConnState::CONNECTED));
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().isWifiConnected_);

    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().isWifiConnected_ = false;
    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_CELLULAR_CALLING;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_INACTIVE;
    EXPECT_CALL(*wifiDeviceMock, GetLinkedInfo(_)).Times(1)
    .WillOnce(Invoke([](Wifi::WifiLinkedInfo &info) -> OHOS::Wifi::ErrCode {
        info.connState = Wifi::ConnState::CONNECTED;
        info.frequency = 2412;
        return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
    }));
    BluetoothHighPowerManager::GetInstance().HandleWifiConnectStateChange(static_cast<int>(Wifi::WifiState::ENABLED));
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().currentWifiScenario_ == WifiScenario::WIFI_2GHZ_ACTIVE);
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);
    HILOGI("HighPower_Mgr_HandleWifiConnectStateChange_001 end");
}

/**
 * @tc.name: HighPower_Mgr_HandleHotSpotStateChange_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_HandleHotSpotStateChange_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_HandleHotSpotStateChange_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().isHotSpotEnable_ = false;
    BluetoothHighPowerManager::GetInstance().HandleHotSpotStateChange(
        static_cast<int>(Wifi::ApState::AP_STATE_STARTED));
    EXPECT_FALSE(BluetoothHighPowerManager::GetInstance().isHotSpotEnable_);

    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().isHotSpotEnable_ = false;
    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_CELLULAR_CALLING;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_INACTIVE;
    EXPECT_CALL(*wifiHotSpotMock, GetStationList(_)).Times(1)
        .WillOnce(Invoke([](std::vector<StationInfo> &result) -> OHOS::Wifi::ErrCode {
            StationInfo staClient = {"test_deviceName", "AA:BB:CC:DD:EE:FF", 1, "127.0.0.2"};
            result.push_back(staClient);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_CALL(*wifiHotSpotMock, GetHotspotConfig(_)).Times(1)
        .WillOnce(Invoke([](HotspotConfig &config) -> OHOS::Wifi::ErrCode {
            config.SetBand(Wifi::BandType::BAND_2GHZ);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    BluetoothHighPowerManager::GetInstance()
        .HandleHotSpotStateChange(static_cast<int>(Wifi::ApState::AP_STATE_STARTED));
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().currentWifiScenario_ == WifiScenario::WIFI_2GHZ_ACTIVE);
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);
    HILOGI("HighPower_Mgr_HandleHotSpotStateChange_001 end");
}

/**
 * @tc.name: HighPower_Mgr_HandleHotSpotConnectStateChange_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_HandleHotSpotConnectStateChange_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_HandleHotSpotConnectStateChange_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().HandleHotSpotConnectStateChange();

    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothHighPowerManager::GetInstance().isHotSpotEnable_ = true;
    BluetoothHighPowerManager::GetInstance().lastHighPowerMode_ = BT_HIGH_POWER_DISABLE;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_CELLULAR_CALLING;
    BluetoothHighPowerManager::GetInstance().currentWifiScenario_ = WifiScenario::WIFI_INACTIVE;
    EXPECT_CALL(*wifiHotSpotMock, GetStationList(_)).Times(1)
        .WillOnce(Invoke([](std::vector<StationInfo> &result) -> OHOS::Wifi::ErrCode {
            StationInfo staClient = {"test_deviceName", "AA:BB:CC:DD:EE:FF", 1, "127.0.0.2"};
            result.push_back(staClient);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    EXPECT_CALL(*wifiHotSpotMock, GetHotspotConfig(_)).Times(1)
        .WillOnce(Invoke([](HotspotConfig &config) -> OHOS::Wifi::ErrCode {
            config.SetBand(Wifi::BandType::BAND_2GHZ);
            return OHOS::Wifi::ErrCode::WIFI_OPT_SUCCESS;
        }));
    BluetoothHighPowerManager::GetInstance().HandleHotSpotConnectStateChange();
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().currentWifiScenario_ == WifiScenario::WIFI_2GHZ_ACTIVE);
    EXPECT_TRUE(BluetoothHighPowerManager::GetInstance().lastHighPowerMode_);
    HILOGI("HighPower_Mgr_HandleHotSpotConnectStateChange_001 end");
}

/**
 * @tc.name: HighPower_Mgr_OnScoStateChanged_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_OnScoStateChanged_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_OnScoStateChanged_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().isBluetoothEnable_ = true;
    RawAddress device({0x11, 0x22, 0x33, 0x44, 0x55, 0x66});
    int state = 6;
    int reason = 0;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_CELLULAR_CALLING;
    BluetoothHighPowerManager::GetInstance().OnScoStateChanged(device, state, reason);

    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().OnScoStateChanged(device, state, reason);
    if (BluetoothHighPowerManager::GetInstance().hfpObserverImp_ == nullptr) {
        BluetoothHighPowerManager::GetInstance().hfpObserverImp_ = std::make_unique<HfpAgObserver>();
    }

    BluetoothHighPowerManager::GetInstance().hfpObserverImp_->OnScoStateChanged(device, state, reason);
    HILOGI("HighPower_Mgr_OnScoStateChanged_001 end");
}

/**
 * @tc.name: HighPower_Mgr_OnPlayingStatusChaned_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_OnPlayingStatusChaned_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_OnPlayingStatusChaned_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().isBluetoothEnable_ = true;
    RawAddress device({0x11, 0x22, 0x33, 0x44, 0x55, 0x66});
    int state = 3;
    int error = 0;
    BluetoothHighPowerManager::GetInstance().hfpAudioOn_ = false;
    BluetoothHighPowerManager::GetInstance().currentBtScenario_ = BluetoothScenario::BT_CELLULAR_CALLING;
    BluetoothHighPowerManager::GetInstance().OnPlayingStatusChaned(device, state, error);

    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().OnPlayingStatusChaned(device, state, error);
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().isBluetoothEnable_ = false;
    BluetoothHighPowerManager::GetInstance().OnPlayingStatusChaned(device, state, error);
    if (BluetoothHighPowerManager::GetInstance().a2dpObserverImp_ == nullptr) {
        BluetoothHighPowerManager::GetInstance().a2dpObserverImp_ = std::make_unique<A2dpObserver>();
    }
    BluetoothHighPowerManager::GetInstance().a2dpObserverImp_->OnPlayingStatusChaned(device, state, error);

    HILOGI("HighPower_Mgr_OnPlayingStatusChaned_001 end");
}

/**
 * @tc.name: HighPower_Mgr_OnPlayingStatusChaned_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_RegisterObserver_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_RegisterObserver_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = true;
    BluetoothHighPowerManager::GetInstance().RegisterA2dpObserver();
    BluetoothHighPowerManager::GetInstance().DeRegisterA2dpObserver();
    BluetoothHighPowerManager::GetInstance().RegisterHfpAgObserver();
    BluetoothHighPowerManager::GetInstance().DeRegisterHfpAgObserver();

    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().a2dpObserverImp_ = nullptr;
    BluetoothHighPowerManager::GetInstance().DeRegisterA2dpObserver();
    BluetoothHighPowerManager::GetInstance().a2dpObserverImp_ = std::make_unique<A2dpObserver>();
    BluetoothHighPowerManager::GetInstance().RegisterA2dpObserver();
    BluetoothHighPowerManager::GetInstance().DeRegisterA2dpObserver();

    BluetoothHighPowerManager::GetInstance().hfpObserverImp_ = nullptr;
    BluetoothHighPowerManager::GetInstance().DeRegisterHfpAgObserver();
    BluetoothHighPowerManager::GetInstance().hfpObserverImp_  = std::make_unique<HfpAgObserver>();
    BluetoothHighPowerManager::GetInstance().RegisterHfpAgObserver();
    BluetoothHighPowerManager::GetInstance().DeRegisterHfpAgObserver();
    BluetoothHighPowerManager::GetInstance().hfpObserverImp_ = nullptr;
    BluetoothHighPowerManager::GetInstance().a2dpObserverImp_ = nullptr;

    HILOGI("HighPower_Mgr_RegisterObserver_001 end");
}

/**
 * @tc.name: HighPower_Mgr_IsSupportHighPowerV1_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_IsSupportHighPowerV1_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_IsSupportHighPowerV1_001 enter");
    BluetoothHighPowerManager::GetInstance().IsSupportHighPowerV1();
    HILOGI("HighPower_Mgr_IsSupportHighPowerV1_001 end");
}

/**
 * @tc.name: HighPower_Mgr_GetSupportedHighPowerLevel_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetSupportedHighPowerLevel_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetSupportedHighPowerLevel_001 enter");
    uint8_t level = BluetoothHighPowerManager::GetInstance().GetSupportedHighPowerLevel();
    EXPECT_NE(level, BT_HIGH_POWER_DISABLE);
    HILOGI("HighPower_Mgr_GetSupportedHighPowerLevel_001 end");
}

/**
 * @tc.name: HighPower_Mgr_GetScenario_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_GetScenario_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_GetScenario_001 enter");
    BluetoothHighPowerManager::GetInstance().hfpAudioOn_ = true;
    BluetoothScenario scenario = BluetoothHighPowerManager::GetInstance().GetBtScenario();
    EXPECT_TRUE(scenario == BluetoothScenario::BT_CELLULAR_CALLING);
    BluetoothHighPowerManager::GetInstance().hfpAudioOn_ = false;
    BluetoothHighPowerManager::GetInstance().a2dpPlaying_ = true;
    scenario = BluetoothHighPowerManager::GetInstance().GetBtScenario();
    EXPECT_TRUE(scenario == BluetoothScenario::BT_PLAYING);
    BluetoothHighPowerManager::GetInstance().hfpAudioOn_ = false;
    BluetoothHighPowerManager::GetInstance().a2dpPlaying_ = false;
    scenario = BluetoothHighPowerManager::GetInstance().GetBtScenario();
    EXPECT_TRUE(scenario == BluetoothScenario::BT_INACTIVE);
    HILOGI("HighPower_Mgr_GetScenario_001 end");
}

/**
 * @tc.name: HighPower_Mgr_handleCommEvent_001
 * @tc.desc: BluetoothHighPowerManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHighPowerManagerTest, HighPower_Mgr_handleCommEvent_001, TestSize.Level1)
{
    HILOGI("HighPower_Mgr_handleCommEvent_001 enter");
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    BluetoothHighPowerManager::GetInstance().RegisterCommonEvent();
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_HOTSPOT_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    BluetoothHighPowerCommonEventSubscriber commmonEventSubscriber(subscribeInfo);

    std::unordered_map<std::string, int> actionCode = {
        {CommonEventSupport::COMMON_EVENT_SCREEN_ON, -1},
        {CommonEventSupport::COMMON_EVENT_SCREEN_OFF, -1},
        {CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE, 0},
        {CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE, 0},
        {CommonEventSupport::COMMON_EVENT_WIFI_HOTSPOT_STATE, 0},
        {CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN, -1},
        {CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE, -1},
        {CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE, 0},
    };
    for (const auto &iter : actionCode) {
        Want want;
        want.SetAction(iter.first);
        CommonEventData commonData;
        commonData.SetWant(want);
        if (iter.second != -1) {
            commonData.SetCode(iter.second);
        }
        commmonEventSubscriber.OnReceiveEvent(commonData);
    }
    BluetoothHighPowerManager::GetInstance().isDisabledByCCM_ = false;
    Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    CommonEventData commonData;
    commmonEventSubscriber.OnReceiveEvent(commonData);
    HILOGI("HighPower_Mgr_handleCommEvent_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS