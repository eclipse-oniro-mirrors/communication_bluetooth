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

#include "bluetooth_avrcp_ct_observer_stub.h"
#include "bluetooth_avrcp_ct_server.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"
#include "avrcp_ct/avrcp_ct_service.h"
#include "class_creator.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
const int TIME = 2;
static Bluetooth::BluetoothHost *host_;

class BluetoothAvrcpCtObserverTest : public BluetoothAvrcpCtObserverStub {
public:
    void OnConnectionStateChanged(const RawAddress &rawAddr, int state, int cause) override {}
    void OnPressButton(const RawAddress &rawAddr, uint8_t button, int result) override {}
    void OnReleaseButton(const RawAddress &rawAddr, uint8_t button,  int result) override {}
    void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter,
        uint32_t numberOfItems, const std::vector<std::string> &folderNames, int result, int detail) override {}
    void OnGetCapabilities(const RawAddress &rawAddr, const std::vector<uint32_t> &companies,
        const std::vector<uint8_t> &events, int result) override {}
    void OnGetPlayerAppSettingAttributes(
        const RawAddress &rawAddr, std::vector<uint8_t> attributes, int result) override {}
    void OnGetPlayerAppSettingValues(const RawAddress &rawAddr,
        int attribute, const std::vector<uint8_t> &values, int result) override {}
    void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> attributes,
        const std::vector<uint8_t> &values, int result) override {}
    void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, int result) override {}
    void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr,
        const std::vector<uint8_t> attribtues, const std::vector<std::string> &attributeName, int result) override {}
    void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr,
        const std::vector<uint8_t> &values, const std::vector<std::string> &valueName, int result) override {}
    void OnGetElementAttributes(const RawAddress &rawAddr,
        const std::vector<uint32_t> &attribtues, const std::vector<std::string> &valueName, int result) override {}
    void OnGetPlayStatus(const RawAddress &rawAddr,
        uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result) override {}
    void OnPlayItem(const RawAddress &rawAddr, int status, int result) override {}
    void OnGetTotalNumberOfItems(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
        int result, int detail) override {}
    void OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attribtues,
        const std::vector<std::string> &values, int result, int detail) override {}
    void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, int result) override {}
    void OnPlaybackStatusChanged(const RawAddress &rawAddr, uint8_t playStatus, int result) override {}
    void OnTrackChanged(const RawAddress &rawAddr, uint64_t uid, int result) override {}
    void OnTrackReachedEnd(const RawAddress &rawAddr, int result) override {}
    void OnTrackReachedStart(const RawAddress &rawAddr, int result) override {}
    void OnPlaybackPosChanged(const RawAddress &rawAddr, uint32_t playbackPos, int result) override {}
    void OnPlayerAppSettingChanged(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result) override {}
    void OnNowPlayingContentChanged(const RawAddress &rawAddr, int result) override {}
    void OnAvailablePlayersChanged(const RawAddress &rawAddr, int result) override {}
    void OnAddressedPlayerChanged(
        const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, int result) override {}
    void OnUidChanged(const RawAddress &rawAddr, uint16_t uidCounter, int result) override {}
    void OnVolumeChanged(const RawAddress &rawAddr, uint8_t volume, int result) override {}
    void OnGetMediaPlayers(const RawAddress &rawAddr, uint16_t uidCounter,
        std::vector<BluetoothAvrcpMpItem> &items, int result, int detail) override {}
    void OnGetFolderItems(const RawAddress &rawAddr, uint16_t uidCounter, std::vector<BluetoothAvrcpMeItem> &items,
           int result, int detail) override {}

    explicit BluetoothAvrcpCtObserverTest()
    {}
    ~BluetoothAvrcpCtObserverTest() override
    {}

private:
};

void *AvrcpCtServiceCreate(void)
{
    return new AvrcpCtService();
}

class AvrcpCtServerTest : public testing::Test {
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

        ClassFactory::RegisterClass("AvrcpCtService", AvrcpCtServiceCreate);

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
    }
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<BluetoothAvrcpCtServer> avrcpCtServer_;
    RawAddress device_ = RawAddress("11:22:33:44:55:66");
};

void AvrcpCtServerTest::SetUp()
{
    // new AvrcpCtService
    ProfileServiceManager::GetInstance()->SetProfileService(bluetooth::BTTransport::ADAPTER_BREDR, "AvrcpCtService");
    avrcpCtServer_ = std::make_unique<BluetoothAvrcpCtServer>();
}

void AvrcpCtServerTest::TearDown()
{
    // delete AvrcpCtService
    auto manager = ProfileServiceManager::GetInstance();
    AvrcpCtService *avrcpCtService = static_cast<AvrcpCtService *>(manager->GetProfileService("AvrcpCtService"));
    delete avrcpCtService;
    avrcpCtServer_ = nullptr;
}

/**
 * @tc.name: avrcp_ct_server_register_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_register_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_register_001 enter");
    sptr<BluetoothAvrcpCtObserverTest> observer = nullptr;
    avrcpCtServer_->RegisterObserver(observer);
    avrcpCtServer_->UnregisterObserver(observer);

    observer = new BluetoothAvrcpCtObserverTest();
    avrcpCtServer_->RegisterObserver(observer);
    avrcpCtServer_->UnregisterObserver(observer);
    EXPECT_TRUE(avrcpCtServer_ != nullptr);
    HILOGI("avrcp_ct_server_register_001 end");
}

/**
 * @tc.name: avrcp_ct_server_getdevicestate_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_getdevicestate_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_getdevicestate_001 enter");
    std::vector<int32_t> states;
    EXPECT_TRUE(avrcpCtServer_->GetDevicesByStates(states).empty());

    EXPECT_EQ(avrcpCtServer_->GetDeviceState(device_), 0);
    HILOGI("avrcp_ct_server_getdevicestate_001 end");
}

/**
 * @tc.name: avrcp_ct_server_connect_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_connect_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_connect_001 enter");
    EXPECT_EQ(avrcpCtServer_->Connect(device_), BT_NO_ERROR);
    EXPECT_EQ(avrcpCtServer_->Disconnect(device_), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_connect_001 end");
}

/**
 * @tc.name: avrcp_ct_server_button_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_button_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_button_001 enter");
    EXPECT_EQ(avrcpCtServer_->PressButton(device_, 0), BT_NO_ERROR);
    EXPECT_EQ(avrcpCtServer_->ReleaseButton(device_, 0), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_button_001 end");
}

/**
 * @tc.name: avrcp_ct_server_unitinfo_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_unitinfo_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_unitinfo_001 enter");
    EXPECT_EQ(avrcpCtServer_->GetUnitInfo(device_), BT_NO_ERROR);
    EXPECT_EQ(avrcpCtServer_->GetSubUnitInfo(device_), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_unitinfo_001 end");
}

/**
 * @tc.name: avrcp_ct_server_supportedcompanies_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_supportedcompanies_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_supportedcompanies_001 enter");
    EXPECT_EQ(avrcpCtServer_->GetSupportedCompanies(device_), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_supportedcompanies_001 end");
}

/**
 * @tc.name: avrcp_ct_server_supportedevents_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_supportedevents_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_supportedevents_001 enter");
    EXPECT_EQ(avrcpCtServer_->GetSupportedEvents(device_), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_supportedevents_001 end");
}

/**
 * @tc.name: avrcp_ct_server_getplayerappsettingattributes_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_getplayerappsettingattributes_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_getplayerappsettingattributes_001 enter");
    EXPECT_EQ(avrcpCtServer_->GetPlayerAppSettingAttributes(device_), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_getplayerappsettingattributes_001 end");
}

/**
 * @tc.name: avrcp_ct_server_playerappsettingvalues_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_playerappsettingvalues_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_playerappsettingvalues_001 enter");
    EXPECT_EQ(avrcpCtServer_->GetPlayerAppSettingValues(device_, 0), BT_NO_ERROR);

    std::vector<int32_t> values;
    EXPECT_EQ(avrcpCtServer_->SetPlayerAppSettingCurrentValue(device_, attributes, values), BT_NO_ERROR);

    std::vector<int32_t> attributes;
    EXPECT_EQ(avrcpCtServer_->GetPlayerAppSettingCurrentValue(device_, attributes), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_playerappsettingvalues_001 end");
}

/**
 * @tc.name: avrcp_ct_server_playerappsettingattributetext_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_playerappsettingattributetext_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_playerappsettingattributetext_001 enter");
    std::vector<int32_t> attributes;
    EXPECT_EQ(avrcpCtServer_->GetPlayerAppSettingAttributeText(device_, attributes), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_playerappsettingattributetext_001 end");
}

/**
 * @tc.name: avrcp_ct_server_playerappsettingvaluetext_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_playerappsettingvaluetext_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_playerappsettingvaluetext_001 enter");
    std::vector<int32_t> values;
    EXPECT_EQ(avrcpCtServer_->GetPlayerAppSettingValueText(device_, 0, values), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_playerappsettingvaluetext_001 end");
}

/**
 * @tc.name: avrcp_ct_server_getelementattributes_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_getelementattributes_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_getelementattributes_001 enter");
    std::vector<int32_t> attributes;
    EXPECT_EQ(avrcpCtServer_->GetElementAttributes(device_, attributes), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_getelementattributes_001 end");
}

/**
 * @tc.name: avrcp_ct_server_getplaystatus_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_getplaystatus_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_getplaystatus_001 enter");
    EXPECT_EQ(avrcpCtServer_->GetPlayStatus(device_), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_getplaystatus_001 end");
}

/**
 * @tc.name: avrcp_ct_server_playitem_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_playitem_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_playitem_001 enter");
    EXPECT_EQ(avrcpCtServer_->PlayItem(device_, 0, 0, 0), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_playitem_001 end");
}

/**
 * @tc.name: avrcp_ct_server_getfolderitems_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_getfolderitems_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_getfolderitems_001 enter");
    std::vector<int32_t> attributes;
    int32_t startItem = 0;
    int32_t endItem = 100;
    EXPECT_EQ(avrcpCtServer_->GetFolderItems(device_, startItem, endItem, attributes), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_getfolderitems_001 end");
}

/**
 * @tc.name: avrcp_ct_server_gettotalnumberofitems_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_gettotalnumberofitems_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_gettotalnumberofitems_001 enter");
    EXPECT_EQ(avrcpCtServer_->GetTotalNumberOfItems(device_, 0), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_gettotalnumberofitems_001 end");
}

/**
 * @tc.name: avrcp_ct_server_setabsolutevolume_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_setabsolutevolume_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_setabsolutevolume_001 enter");
    EXPECT_EQ(avrcpCtServer_->SetAbsoluteVolume(device_, 0), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_setabsolutevolume_001 end");
}

/**
 * @tc.name: avrcp_ct_server_notification_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_notification_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_notification_001 enter");
    std::vector<int32_t> events;
    EXPECT_EQ(avrcpCtServer_->EnableNotification(device_, events, 0), BT_NO_ERROR);
    EXPECT_EQ(avrcpCtServer_->DisableNotification(device_, events), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_notification_001 end");
}

/**
 * @tc.name: avrcp_ct_server_getitemattributes_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_getitemattributes_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_getitemattributes_001 enter");
    std::vector<int32_t> attributes;
    EXPECT_EQ(avrcpCtServer_->GetItemAttributes(device_, 0, 0, attributes), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_getitemattributes_001 end");
}

/**
 * @tc.name: avrcp_ct_server_setbrowsedplayer_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_setbrowsedplayer_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_setbrowsedplayer_001 enter");
    EXPECT_EQ(avrcpCtServer_->SetBrowsedPlayer(device_, 0), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_setbrowsedplayer_001 end");
}

/**
 * @tc.name: avrcp_ct_server_getmeidaplayerlist_001
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_getmeidaplayerlist_001, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_getmeidaplayerlist_001 enter");
    int32_t startItem = 0;
    int32_t endItem = 100;
    EXPECT_EQ(avrcpCtServer_->GetMeidaPlayerList(device_, startItem, endItem), BT_NO_ERROR);
    HILOGI("avrcp_ct_server_getmeidaplayerlist_001 end");
}

/**
 * @tc.name: avrcp_ct_server_register_observer_002
 * @tc.desc: AvrcpCtServerTest:
 * @tc.type: FUNC
 */
HWTEST_F(AvrcpCtServerTest, avrcp_ct_server_register_observer_002, TestSize.Level1)
{
    HILOGI("avrcp_ct_server_register_observer_002 enter");
    std::unique_ptr<BluetoothAvrcpCtObserverTest> avrcpCtServerTest = std::make_unique<BluetoothAvrcpCtObserverTest>();
    for (int i = 0; i <= 1001; i++) {
        sptr<IBluetoothAvrcpCtObserver> observer;
        avrcpCtServer_->RegisterObserver(observer);
    }
    EXPECT_TRUE(avrcpCtServer_ != nullptr);
    HILOGI("avrcp_ct_server_register_observer_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS