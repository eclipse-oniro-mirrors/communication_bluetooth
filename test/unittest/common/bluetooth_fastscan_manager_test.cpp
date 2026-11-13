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
#include "bluetooth_fastscan_manager.h"
#include "a2dp_service.h"
#include "bluetooth_host.h"
#include "adapter_manager.h"
#include "bluetooth_common_event_subscriber.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_errorcode.h"
#include "thread_util.h"
#include "parameter.h"
#include "wifi_device.h"
#include "mock/mock_wifi_device.h"

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

class BluetoothFastScanManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_LOW_PRIORITY, ThreadUtil::NOT_SWITCH_THREAD);
        host_ = &BluetoothHost::GetDefaultHost();
        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        BluetoothFastScanManager::GetInstance().Init();
        BluetoothFastScanManager::GetInstance().wifiDevice_ = wifiDeviceMock;
        if (BluetoothFastScanManager::GetInstance().btFastScanEventType_.empty()) {
            BluetoothFastScanManager::GetInstance().btFastScanEventType_ = {
                {CommonEventSupport::COMMON_EVENT_SCREEN_ON, BtFastScanEventType::SCREEN_ON},
                {CommonEventSupport::COMMON_EVENT_SCREEN_OFF, BtFastScanEventType::SCREEN_OFF},
                {CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE, BtFastScanEventType::WIFI_POWER_STATE},
                {CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE,
                    BtFastScanEventType::BLUETOOTH_HOST_STATE_UPDATE},
            };
        }
    }
    static void TearDownTestCase()
    {
        BluetoothFastScanManager::GetInstance().UnRegisterCommonEvent();
        BluetoothFastScanManager::GetInstance().DeRegisterAppObserver();
        utility::CloseAllTimer();
        wifiDeviceMock = nullptr;
    }
    void SetUp()
    {
        BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    }
};

class AppStateAwareObserverTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: FastScan_Mgr_SetFastScan_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_SetFastScan_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_SetFastScan_001 enter");
    BluetoothFastScanManager::GetInstance().isFastScanEnabled_ = true;
    BluetoothFastScanManager::GetInstance().SetFastScan(true);
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);
    HILOGI("FastScan_Mgr_SetFastScan_001 end");
}

/**
 * @tc.name: FastScan_Mgr_SetFastScan_002
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_SetFastScan_002, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_SetFastScan_002 enter");
    BluetoothFastScanManager::GetInstance().isFastScanEnabled_ = false;
    BluetoothFastScanManager::GetInstance().SetFastScan(true);
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);
    HILOGI("FastScan_Mgr_SetFastScan_002 end");
}

/**
 * @tc.name: FastScan_Mgr_UpdateFastScanMode_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_UpdateFastScanMode_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_UpdateFastScanMode_001 enter");
    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = false;
    BluetoothFastScanManager::GetInstance().UpdateFastScanMode();
    EXPECT_FALSE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothFastScanManager::GetInstance().isScreenOn_ = false;
    BluetoothFastScanManager::GetInstance().UpdateFastScanMode();
    EXPECT_FALSE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothFastScanManager::GetInstance().isScreenOn_ = true;
    BluetoothFastScanManager::GetInstance().isWifiOn_ = false;
    BluetoothFastScanManager::GetInstance().UpdateFastScanMode();
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothFastScanManager::GetInstance().isScreenOn_ = true;
    BluetoothFastScanManager::GetInstance().isWifiOn_ = true;
    BluetoothFastScanManager::GetInstance().isWhiteApp_ = false;
    BluetoothFastScanManager::GetInstance().UpdateFastScanMode();
    EXPECT_FALSE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);

    BluetoothFastScanManager::GetInstance().isFastScanEnabled_ = true;
    BluetoothFastScanManager::GetInstance().isScreenOn_ = true;
    BluetoothFastScanManager::GetInstance().isWifiOn_ = true;
    BluetoothFastScanManager::GetInstance().isWhiteApp_ = true;
    BluetoothFastScanManager::GetInstance().UpdateFastScanMode();
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);
    HILOGI("FastScan_Mgr_UpdateFastScanMode_001 end");
}

/**
 * @tc.name: FastScan_Mgr_HandleScreenStateChange_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_HandleScreenStateChange_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_HandleScreenStateChange_001 enter");
    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = false;
    EXPECT_FALSE(BluetoothFastScanManager::GetInstance().isBluetoothEnable_);

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothFastScanManager::GetInstance().isScreenOn_ = false;
    BluetoothFastScanManager::GetInstance().HandleScreenStateChange(true);
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isScreenOn_);

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothFastScanManager::GetInstance().isScreenOn_ = true;
    BluetoothFastScanManager::GetInstance().HandleScreenStateChange(true);
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isScreenOn_);

    HILOGI("FastScan_Mgr_HandleScreenStateChange_001 end");
}

/**
 * @tc.name: FastScan_Mgr_GetEventType_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_GetEventType_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_GetEventType_001 enter");
    std::string eventString = "usual.event.wifi.POWER_STATE";
    BtFastScanEventType type = BluetoothFastScanManager::GetInstance().GetEventType(eventString);
    EXPECT_TRUE(type == BtFastScanEventType::WIFI_POWER_STATE);
    HILOGI("FastScan_Mgr_GetEventType_001 end");
}

/**
 * @tc.name: FastScan_Mgr_HandleWifiStateChange_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_HandleWifiStateChange_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_HandleWifiStateChange_001 enter");
    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = false;
    EXPECT_FALSE(BluetoothFastScanManager::GetInstance().isBluetoothEnable_);

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothFastScanManager::GetInstance().isWifiOn_ = false;
    BluetoothFastScanManager::GetInstance().HandleWifiStateChange(static_cast<int>(Wifi::WifiState::ENABLED));
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isWifiOn_);

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothFastScanManager::GetInstance().isWifiOn_ = true;
    BluetoothFastScanManager::GetInstance().HandleWifiStateChange(static_cast<int>(Wifi::WifiState::ENABLED));
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isWifiOn_);
    HILOGI("FastScan_Mgr_HandleWifiStateChange_001 end");
}

/**
 * @tc.name: FastScan_Mgr_handleCommEvent_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_handleCommEvent_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_handleCommEvent_001 enter");
    BluetoothFastScanManager::GetInstance().RegisterCommonEvent();
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().subscriber_ != nullptr);
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    BluetoothFastScanCommonEventSubscriber commmonEventSubscriber(subscribeInfo);

    std::unordered_map<std::string, int> actionCode = {
        {CommonEventSupport::COMMON_EVENT_SCREEN_ON, -1},
        {CommonEventSupport::COMMON_EVENT_SCREEN_OFF, -1},
        {CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE, 0},
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

    Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    CommonEventData commonData;
    commmonEventSubscriber.OnReceiveEvent(commonData);
    HILOGI("FastScan_Mgr_handleCommEvent_001 end");
}

/**
 * @tc.name: FastScan_Mgr_RegisterObserverCommonEvent_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_RegisterObserverCommonEvent_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_RegisterObserverCommonEvent_001 enter");
    BluetoothFastScanManager::GetInstance().RegisterAppObserver();
    BluetoothFastScanManager::GetInstance().DeRegisterAppObserver();

    BluetoothFastScanManager::GetInstance().appStateObserverImp_ = nullptr;
    BluetoothFastScanManager::GetInstance().RegisterAppObserver();
    BluetoothFastScanManager::GetInstance().DeRegisterAppObserver();

    BluetoothFastScanManager::GetInstance().appStateObserverImp_ = std::make_unique<AppStateObserver>();
    BluetoothFastScanManager::GetInstance().RegisterAppObserver();

    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    BluetoothFastScanManager::GetInstance().subscriber_ =
        std::make_shared<BluetoothFastScanCommonEventSubscriber>(subscribeInfo);

    BluetoothFastScanManager::GetInstance().RegisterCommonEvent();
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().subscriber_ != nullptr);
    BluetoothFastScanManager::GetInstance().UnRegisterCommonEvent();

    BluetoothFastScanManager::GetInstance().subscriber_ == nullptr;
    BluetoothFastScanManager::GetInstance().RegisterCommonEvent();
    BluetoothFastScanManager::GetInstance().UnRegisterCommonEvent();
    BluetoothFastScanManager::GetInstance().DeRegisterAppObserver();

    HILOGI("FastScan_Mgr_RegisterObserverCommonEvent_001 end");
}

/**
 * @tc.name: FastScan_Mgr_IsA2dpPlaying_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_IsA2dpPlaying_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_IsA2dpPlaying_001 enter");
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().IsA2dpPlaying(), false);  // 0 A2DP_ROLE_SOURCE
    HILOGI("FastScan_Mgr_IsA2dpPlaying_001 end");
}

/**
 * @tc.name: FastScan_Mgr_HasA2dpConnectedDevicesForCurrentMobile_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_HasA2dpConnectedDevicesForCurrentMobile_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_HasA2dpConnectedDevicesForCurrentMobile_001 enter");
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().HasA2dpConnectedDevicesForCurrentMobile(), false);
    HILOGI("FastScan_Mgr_HasA2dpConnectedDevicesForCurrentMobile_001 end");
}

/**
 * @tc.name: FastScan_HasHfpConnectedDevicesForCurrentMobile_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_HasHfpConnectedDevicesForCurrentMobile_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_HasHfpConnectedDevicesForCurrentMobile_001 enter");
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().HasHfpConnectedDevicesForCurrentMobile(), false);
    HILOGI("FastScan_Mgr_HasHfpConnectedDevicesForCurrentMobile_001 end");
}

/**
 * @tc.name: FastScan_Mgr_RegisterA2dpServiceCb_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_RegisterA2dpServiceCb_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_RegisterA2dpServiceCb_001 enter");
    bool callbackCalled = 0;
    A2dpServiceFuncs callbacks;
    callbacks.isA2dpPlayingCb = []() { return true; };
    callbacks.setA2dpPlayingCb = [](std::function<void(int)> cb) {};
    BluetoothFastScanManager::GetInstance().RegisterA2dpServiceCb(&callbacks);
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().a2dpServiceFuncs_);
    HILOGI("FastScan_Mgr_RegisterA2dpServiceCb_001 end");
}

/**
 * @tc.name: FastScan_Mgr_HandleA2dpPlayingStateChange_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_HandleA2dpPlayingStateChange_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_HandleA2dpPlayingStateChange_001 enter");

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = false;
    BluetoothFastScanManager::GetInstance().isA2dpPlaying_ = false;
    BluetoothFastScanManager::GetInstance().HandleA2dpPlayingStateChange(2);  // 2 BTAV_AUDIO_STATE_STARTED
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().isA2dpPlaying_, false);

    BluetoothFastScanManager::GetInstance().isBluetoothEnable_ = true;
    BluetoothFastScanManager::GetInstance().HandleA2dpPlayingStateChange(2);  // 2 BTAV_AUDIO_STATE_STARTED
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().isA2dpPlaying_, true);

    BluetoothFastScanManager::GetInstance().isA2dpPlaying_ = false;
    BluetoothFastScanManager::GetInstance().HandleA2dpPlayingStateChange(2);  // 2 BTAV_AUDIO_STATE_STARTED
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().isA2dpPlaying_, true);
    HILOGI("FastScan_Mgr_HandleA2dpPlayingStateChange_001 end");
}

/**
 * @tc.name: FastScan_Mgr_GetAppCategory_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_GetAppCategory_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_GetAppCategory_001 enter");
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().GetAppCategory("123"), -1);
    HILOGI("FastScan_Mgr_GetAppCategory_001 end");
}

/**
 * @tc.name: FastScan_Mgr_GetAppTypeByBundleName_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_GetAppTypeByBundleName_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_GetAppTypeByBundleName_001 enter");
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().GetAppTypeByBundleName("123"), -1);
    HILOGI("FastScan_Mgr_GetAppTypeByBundleName_001 end");
}

/**
 * @tc.name: FastScan_Mgr_GetForegroundAppType_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_GetForegroundAppType_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_GetForegroundAppType_001 enter");
    auto apps = std::vector<OHOS::AppExecFwk::AppStateData>();
    EXPECT_EQ(BluetoothFastScanManager::GetInstance().GetForegroundAppType("123", apps), -1);
    HILOGI("FastScan_Mgr_GetForegroundAppType_001 end");
}

/**
 * @tc.name: FastScan_Mgr_SetFastScanEnabled_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothFastScanManagerTest, FastScan_Mgr_SetFastScanEnabled_001, TestSize.Level1)
{
    HILOGI("FastScan_Mgr_SetFastScanEnabled_001 enter");
    BluetoothFastScanManager::GetInstance().SetFastScanEnabled(true);
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);
    BluetoothFastScanManager::GetInstance().SetFastScanEnabled(false);
    EXPECT_FALSE(BluetoothFastScanManager::GetInstance().isFastScanEnabled_);
    HILOGI("FastScan_Mgr_SetFastScanEnabled_001 end");
}

/**
 * @tc.name: OnForegroundApplicationChanged_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AppStateAwareObserverTest, OnForegroundApplicationChanged_001, TestSize.Level0)
{
    HILOGI("OnForegroundApplicationChanged_001 enter");
    AppStateObserver::AppStateAwareObserver observer;
    AppExecFwk::AppStateData appStateData;
    appStateData.state = static_cast<int32_t>(OHOS::AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    observer.OnForegroundApplicationChanged(appStateData);
    EXPECT_TRUE(BluetoothFastScanManager::GetInstance().hfpAgServiceFuncs_ == nullptr);
    BluetoothFastScanManager::GetInstance().OnForegroundApplicationChanged(appStateData);
    HILOGI("OnForegroundApplicationChanged_001 end");
}

/**
 * @tc.name: GetForegroundAppType_001
 * @tc.desc: BluetoothFastScanManagerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AppStateAwareObserverTest, GetForegroundAppType_001, TestSize.Level0)
{
    HILOGI("GetForegroundAppType_001 enter");
    AppExecFwk::AppStateData appUnknown;
    appUnknown.bundleName = "com.example.unknown";
    AppExecFwk::AppStateData appMusic;
    appMusic.bundleName = "com.example.music";
    AppExecFwk::AppStateData appVideo;
    appVideo.bundleName = "com.example.video";
    AppExecFwk::AppStateData appGame;
    appGame.bundleName = "com.example.game";
    BluetoothFastScanManager::GetInstance().appCategoryMap_[appUnknown.bundleName] = APP_CATEGORY_UNKNOWN;
    BluetoothFastScanManager::GetInstance().appCategoryMap_[appMusic.bundleName] = APP_CATEGORY_MUSIC;
    BluetoothFastScanManager::GetInstance().appCategoryMap_[appVideo.bundleName] = APP_CATEGORY_VIDEO;
    BluetoothFastScanManager::GetInstance().appCategoryMap_[appGame.bundleName] = APP_CATEGORY_GAME;

    AppCategory ret = APP_CATEGORY_UNKNOWN;
    std::vector<OHOS::AppExecFwk::AppStateData> apps1 {};
    ret = BluetoothFastScanManager::GetInstance().GetForegroundAppType(appMusic.bundleName, apps1);
    EXPECT_EQ(ret, APP_CATEGORY_MUSIC);

    ret = BluetoothFastScanManager::GetInstance().GetForegroundAppType(appGame.bundleName, apps1);
    EXPECT_EQ(ret, APP_CATEGORY_GAME);

    std::vector<OHOS::AppExecFwk::AppStateData> apps2 {appUnknown, appMusic, appGame};
    ret = BluetoothFastScanManager::GetInstance().GetForegroundAppType(appUnknown.bundleName, apps2);
    EXPECT_EQ(ret, APP_CATEGORY_GAME);

    std::vector<OHOS::AppExecFwk::AppStateData> apps3 {appUnknown, appVideo, appMusic};
    ret = BluetoothFastScanManager::GetInstance().GetForegroundAppType(appUnknown.bundleName, apps3);
    EXPECT_EQ(ret, APP_CATEGORY_VIDEO);

    std::vector<OHOS::AppExecFwk::AppStateData> apps4 {appUnknown, appMusic};
    ret = BluetoothFastScanManager::GetInstance().GetForegroundAppType(appUnknown.bundleName, apps4);
    EXPECT_EQ(ret, APP_CATEGORY_MUSIC);
    HILOGI("GetForegroundAppType_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS