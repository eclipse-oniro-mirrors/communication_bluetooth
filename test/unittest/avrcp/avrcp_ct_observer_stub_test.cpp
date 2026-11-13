/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "bluetooth_access_token_mock.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

#include "bluetooth_pan.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_avrcp_ct_observer_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int TIME = 4;
static BluetoothHost *host_;
static std::string g_addr("00:00:00:00:00:00");

class BluetoothAvrcpCtObserverStubMock : public BluetoothAvrcpCtObserverStub {
public:
    void OnConnectionStateChanged(const RawAddress &rawAddr, int state, int cause) override
    {}

    void OnPressButton(const RawAddress &rawAddr, uint8_t button, int result) override
    {}

    void OnReleaseButton(const RawAddress &rawAddr, uint8_t button,  int result) override
    {}

    void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter,
        uint32_t numberOfItems, const std::vector<std::string> &folderNames, int result, int detail) override
    {}

    void OnGetCapabilities(const RawAddress &rawAddr, const std::vector<uint32_t> &companies,
        const std::vector<uint8_t> &events, int result) override
    {}

    void OnGetPlayerAppSettingAttributes(
        const RawAddress &rawAddr, std::vector<uint8_t> attributes, int result) override
    {}

    void OnGetPlayerAppSettingValues(const RawAddress &rawAddr,
        int attribute, const std::vector<uint8_t> &values, int result) override
    {}

    void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, int result) override
    {}

    void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> attributes,
        const std::vector<uint8_t> &values, int result) override
    {}

    void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr,
        const std::vector<uint8_t> attribtues, const std::vector<std::string> &attributeName, int result) override
    {}

    void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr,
        const std::vector<uint8_t> &values, const std::vector<std::string> &valueName, int result) override
    {}

    void OnGetElementAttributes(const RawAddress &rawAddr,
        const std::vector<uint32_t> &attribtues, const std::vector<std::string> &valueName, int result) override
    {}

    void OnGetPlayStatus(const RawAddress &rawAddr,
        uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result) override
    {}

    void OnPlayItem(const RawAddress &rawAddr, int status, int result) override
    {}

    void OnGetTotalNumberOfItems(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
        int result, int detail) override
    {}

    void OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attribtues,
        const std::vector<std::string> &values, int result, int detail) override
    {}

    void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, int result) override
    {}

    void OnPlaybackStatusChanged(const RawAddress &rawAddr, uint8_t playStatus, int result) override
    {}

    void OnTrackChanged(const RawAddress &rawAddr, uint64_t uid, int result) override
    {}

    void OnTrackReachedEnd(const RawAddress &rawAddr, int result) override
    {}

    void OnTrackReachedStart(const RawAddress &rawAddr, int result) override
    {}

    void OnPlaybackPosChanged(const RawAddress &rawAddr, uint32_t playbackPos, int result) override
    {}

    void OnPlayerAppSettingChanged(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result) override
    {}

    void OnNowPlayingContentChanged(const RawAddress &rawAddr, int result) override
    {}

    void OnAvailablePlayersChanged(const RawAddress &rawAddr, int result) override
    {}

    void OnAddressedPlayerChanged(
        const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, int result) override
    {}

    void OnUidChanged(const RawAddress &rawAddr, uint16_t uidCounter, int result) override
    {}

    void OnVolumeChanged(const RawAddress &rawAddr, uint8_t volume, int result) override
    {}

    void OnGetMediaPlayers(const RawAddress &rawAddr, uint16_t uidCounter,
        std::vector<BluetoothAvrcpMpItem> &items, int result, int detail) override
    {}

    void OnGetFolderItems(const RawAddress &rawAddr, uint16_t uidCounter, std::vector<BluetoothAvrcpMeItem> &items,
           int result, int detail) override
    {}

    explicit BluetoothAvrcpCtObserverStubMock()
    {}
    ~BluetoothAvrcpCtObserverStubMock() override
    {}

private:
};

class BluetoothAvrcpCtObserverStubTest : public testing::Test {
public:
    BluetoothAvrcpCtObserverStubTest()
    {}
    ~BluetoothAvrcpCtObserverStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void BluetoothAvrcpCtObserverStubTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    BluetoothAccessTokenMock::SetNativeTokenInfo();
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
void BluetoothAvrcpCtObserverStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    host_ = nullptr;
    HILOGI("TearDownTestCase end");
}
void BluetoothAvrcpCtObserverStubTest::SetUp()
{}
void BluetoothAvrcpCtObserverStubTest::TearDown()
{}


/*
 * @tc.number: AvrcpCtObserverStubTest_OnRemoteRequest_001
 * @tc.name: OnRemoteRequest
 * @tc.desc: return ERR_INVALID_STATE
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnRemoteRequest_001 start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_CONNECTION_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
    HILOGI("AvrcpCtObserverStubTest_OnRemoteRequest_001 end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnRemoteRequest_002
 * @tc.name: OnRemoteRequest
 * @tc.desc: return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnRemoteRequest_002 start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_OBSERVER_BUTT),
        data, reply, option);
    EXPECT_EQ(ret, 305);
    HILOGI("AvrcpCtObserverStubTest_OnRemoteRequest_002 end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnConnectionStateChangedInner
 * @tc.name: OnConnectionStateChangedInner
 * @tc.desc: Check OnConnectionStateChanged interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnConnectionStateChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnConnectionStateChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_CONNECTION_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnConnectionStateChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnPressButtonInner
 * @tc.name: OnPressButtonInner
 * @tc.desc: Check OnPressButtonInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnPressButtonInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnPressButtonInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PRESS_BUTTON),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnPressButtonInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnReleaseButtonInner
 * @tc.name: OnReleaseButtonInner
 * @tc.desc: Check OnReleaseButtonInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnReleaseButtonInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnReleaseButtonInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_RELEASE_BUTTON),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnReleaseButtonInner end");
}

const int32_t AVRCP_INT32_NUM_ONE = 0x01;
const int32_t AVRCP_INT32_NUM_TWO = 0x02;
const uint32_t AVRCP_UINT32_NUM_ONE = 0x01;
const uint32_t AVRCP_UINT32_NUM_TWO = 0x02;
const uint32_t AVRCP_NUM_OF_ITEMS = 0x0A;
const uint32_t AVRCP_FOLDER_DEPTH_MAX = 0xFF;

/*
 * @tc.number: AvrcpCtObserverStubTest_OnSetBrowsedPlayerInner_001
 * @tc.name: OnSetBrowsedPlayerInner
 * @tc.desc: NO_ERROR
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnSetBrowsedPlayerInner_001, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnSetBrowsedPlayerInner_001 start");

    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor); // 通过remort校验
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteUint32(AVRCP_NUM_OF_ITEMS);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX);
    for (int i = 0; i < AVRCP_FOLDER_DEPTH_MAX; i++) {
        data.WriteString("Avrcp" + std::to_string(i));
    }
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_SET_BROWSED_PLAYER),
        data, reply, option);

    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnSetBrowsedPlayerInner_001 end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnSetBrowsedPlayerInner_002
 * @tc.name: OnSetBrowsedPlayerInner
 * @tc.desc: BT_ERR_INVALID_PARAM
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnSetBrowsedPlayerInner_002, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnSetBrowsedPlayerInner_002 start");

    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor); // 通过remort校验
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteUint32(AVRCP_NUM_OF_ITEMS);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    for (int i = 0; i < AVRCP_FOLDER_DEPTH_MAX + 1; i++) {
        data.WriteString("Avrcp" + std::to_string(i));
    }
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_SET_BROWSED_PLAYER),
        data, reply, option);

    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnSetBrowsedPlayerInner_002 end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetCapabilitiesInner
 * @tc.name: OnGetCapabilitiesInner
 * @tc.desc: NO_ERROR
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetCapabilitiesInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetCapabilitiesInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_CAPABILITIES),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试companiesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_CAPABILITIES),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    // 测试eventsSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_CAPABILITIES),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnGetCapabilitiesInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetPlayerAppSettingAttributesInner
 * @tc.name: OnGetPlayerAppSettingAttributesInner
 * @tc.desc: Check OnGetPlayerAppSettingAttributesInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetPlayerAppSettingAttributesInner,
    TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingAttributesInner start");
    // 测试正常情况
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTES),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试attributesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTES),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingAttributesInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetPlayerAppSettingValuesInner
 * @tc.name: OnGetPlayerAppSettingValuesInner
 * @tc.desc: Check OnGetPlayerAppSettingValuesInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetPlayerAppSettingValuesInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingValuesInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试valuesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingValuesInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetPlayerAppSettingCurrentValueInner
 * @tc.name: OnGetPlayerAppSettingCurrentValueInner
 * @tc.desc: Check OnGetPlayerAppSettingCurrentValueInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetPlayerAppSettingCurrentValueInner,
    TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingCurrentValueInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_CURRENT_VALUE),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试attrbutesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_CURRENT_VALUE),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    // 测试valuesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_CURRENT_VALUE),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingCurrentValueInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnSetPlayerAppSettingCurrentValueInner
 * @tc.name: OnSetPlayerAppSettingCurrentValueInner
 * @tc.desc: Check OnSetPlayerAppSettingCurrentValueInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnSetPlayerAppSettingCurrentValueInner,
    TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnSetPlayerAppSettingCurrentValueInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_SET_PLAYER_APP_SETTING_CURRENT_VALUE),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnSetPlayerAppSettingCurrentValueInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetPlayerAppSettingAttributeTextInner
 * @tc.name: OnGetPlayerAppSettingAttributeTextInner
 * @tc.desc: Check OnGetPlayerAppSettingAttributeTextInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetPlayerAppSettingAttributeTextInner,
    TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingAttributeTextInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX);
    for (int i = 0; i < AVRCP_FOLDER_DEPTH_MAX; i++) {
        data.WriteString("Avrcp" + std::to_string(i));
    }
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试attrbutesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    // 测试valuesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingAttributeTextInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetPlayerAppSettingValueTextInner
 * @tc.name: OnGetPlayerAppSettingValueTextInner
 * @tc.desc: Check OnGetPlayerAppSettingValueTextInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetPlayerAppSettingValueTextInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingValueTextInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX);
    for (int i = 0; i < AVRCP_FOLDER_DEPTH_MAX; i++) {
        data.WriteString("Avrcp" + std::to_string(i));
    }
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_VALUE_TEXT),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试valuesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_VALUE_TEXT),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    // 测试valueNamesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_VALUE_TEXT),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayerAppSettingValueTextInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetElementAttributesInner
 * @tc.name: OnGetElementAttributesInner
 * @tc.desc: Check OnGetElementAttributesInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetElementAttributesInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetElementAttributesInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX);
    for (int i = 0; i < AVRCP_FOLDER_DEPTH_MAX; i++) {
        data.WriteString("Avrcp" + std::to_string(i));
    }
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_ELEMENT_ATTRIBUTRES),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试valuesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_ELEMENT_ATTRIBUTRES),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    // 测试valueNamesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_ELEMENT_ATTRIBUTRES),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnGetElementAttributesInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetPlayStatusInner
 * @tc.name: OnGetPlayStatusInner
 * @tc.desc: Check OnGetPlayStatusInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetPlayStatusInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayStatusInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAY_STATUS),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnGetPlayStatusInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnPlayItemInner
 * @tc.name: OnPlayItemInner
 * @tc.desc: Check OnPlayItemInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnPlayItemInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnPlayItemInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAY_ITEM),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnPlayItemInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetTotalNumberOfItemsInner
 * @tc.name: OnGetTotalNumberOfItemsInner
 * @tc.desc: Check OnGetTotalNumberOfItemsInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetTotalNumberOfItemsInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetTotalNumberOfItemsInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_TOTAL_NUMBER_OF_ITEMS),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnGetTotalNumberOfItemsInner end");
}

/*
 * @tc.number: AvrcpController051
 * @tc.name: OnGetItemAttributesInner
 * @tc.desc: Check OnGetItemAttributesInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetItemAttributesInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetItemAttributesInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX);
    for (int i = 0; i < AVRCP_FOLDER_DEPTH_MAX; i++) {
        data.WriteString("Avrcp" + std::to_string(i));
    }
    data.WriteInt32(0);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_ITEM_ATTRIBUTES),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试attribtuesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_ITEM_ATTRIBUTES),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    // 测试valuesSize大于最大值的情况
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteUint32(AVRCP_UINT32_NUM_ONE);
    data.WriteUint32(AVRCP_UINT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_ITEM_ATTRIBUTES),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnGetItemAttributesInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnSetAbsoluteVolumeInner
 * @tc.name: OnSetAbsoluteVolumeInner
 * @tc.desc: Check OnSetAbsoluteVolumeInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnSetAbsoluteVolumeInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnSetAbsoluteVolumeInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_SET_ABSOLUTE_VOLUME),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnSetAbsoluteVolumeInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnPlaybackStatusChangedInner
 * @tc.name: OnPlaybackStatusChangedInner
 * @tc.desc: Check OnPlaybackStatusChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnPlaybackStatusChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnPlaybackStatusChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAYBACK_STATUS_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnPlaybackStatusChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnTrackChangedInner
 * @tc.name: OnTrackChangedInner
 * @tc.desc: Check OnTrackChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnTrackChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnTrackChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_TRACK_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnTrackChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnTrackReachedEndInner
 * @tc.name: OnTrackReachedEndInner
 * @tc.desc: Check OnTrackReachedEndInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnTrackReachedEndInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnTrackReachedEndInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_TRACK_REACHED_END),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnTrackReachedEndInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnTrackReachedStartInner
 * @tc.name: OnTrackReachedStartInner
 * @tc.desc: Check OnTrackReachedStartInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnTrackReachedStartInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnTrackReachedStartInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_TRACK_REACHED_START),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnTrackReachedStartInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnPlaybackPosChangedInner
 * @tc.name: OnPlaybackPosChangedInner
 * @tc.desc: Check OnPlaybackPosChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnPlaybackPosChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnPlaybackPosChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAYBACK_POS_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnPlaybackPosChangedInner end");
}

/*
 * @tc.number: AvrcpController058
 * @tc.name: OnPlayerAppSettingChangedInner
 * @tc.desc: Check OnPlayerAppSettingChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnPlayerAppSettingChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnPlayerAppSettingChangedInner start");
    // 测试正常情况
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAY_APP_SETTING_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    // 测试attribtuesSize大于最大值
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAY_APP_SETTING_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    // 测试valuesSize大于最大值
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_TWO);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAY_APP_SETTING_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpCtObserverStubTest_OnPlayerAppSettingChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnNowPlayingContentChangedInner
 * @tc.name: OnNowPlayingContentChangedInner
 * @tc.desc: Check OnNowPlayingContentChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnNowPlayingContentChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnNowPlayingContentChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_NOW_PLAYING_CONTENT_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnNowPlayingContentChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnAvailablePlayersChangedInner
 * @tc.name: OnAvailablePlayersChangedInner
 * @tc.desc: Check OnAvailablePlayersChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnAvailablePlayersChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnAvailablePlayersChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_AVAILABLE_PLAYER_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnAvailablePlayersChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnAddressedPlayerChangedInner
 * @tc.name: OnAddressedPlayerChangedInner
 * @tc.desc: Check OnAvailablePlayersChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnAddressedPlayerChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnAddressedPlayerChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_ADDRESSED_PLAYER_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnAddressedPlayerChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnUidChangedInner
 * @tc.name: OnUidChangedInner
 * @tc.desc: Check OnAvailablePlayersChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnUidChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnUidChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_UID_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnUidChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnVolumeChangedInner
 * @tc.name: OnVolumeChangedInner
 * @tc.desc: Check OnVolumeChangedInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnVolumeChangedInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnVolumeChangedInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_VOLUME_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    HILOGI("AvrcpCtObserverStubTest_OnVolumeChangedInner end");
}

/*
 * @tc.number: AvrcpCtObserverStubTest_OnGetMediaPlayersInner
 * @tc.name: OnGetMediaPlayersInner
 * @tc.desc: Check OnGetMediaPlayersInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetMediaPlayersInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetMediaPlayersInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    BluetoothAvrcpMpItem item;
    data.WriteParcelable(&item);
    data.WriteInt32(0);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_MEDIA_PLAYERS),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试itemsSize超过最大值
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_FOLDER_DEPTH_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_MEDIA_PLAYERS),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);

    // 测试item为null
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteParcelable(nullptr);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_MEDIA_PLAYERS),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);
    HILOGI("AvrcpCtObserverStubTest_OnGetMediaPlayersInner end");
}

const uint32_t AVRCP_FOLDER_ITEMS_NUM_MAX = 0xFFFF;

/*
 * @tc.number: AvrcpController065
 * @tc.name: OnGetFolderItemsInner
 * @tc.desc: Check OnGetFolderItemsInner interface
*/
HWTEST_F(BluetoothAvrcpCtObserverStubTest, AvrcpCtObserverStubTest_OnGetFolderItemsInner, TestSize.Level1)
{
    HILOGI("AvrcpCtObserverStubTest_OnGetFolderItemsInner start");
    BluetoothAvrcpCtObserverStubMock bluetoothAvrcpCtObserverStubTest;
    // 测试正常情况
    MessageParcel data;
    std::u16string descriptor = BluetoothAvrcpCtObserverStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    BluetoothAvrcpMpItem item;
    data.WriteParcelable(&item);
    data.WriteInt32(0);
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;

    ErrCode ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_FOLDER_ITEMS),
        data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    // 测试itemsSize超过最大值
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_FOLDER_ITEMS_NUM_MAX + 1);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_FOLDER_ITEMS),
        data, reply, option);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    // 测试item为null
    data.WriteInterfaceToken(descriptor);
    data.WriteString(g_addr);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteInt32(AVRCP_INT32_NUM_ONE);
    data.WriteParcelable(nullptr);
    ret = bluetoothAvrcpCtObserverStubTest.OnRemoteRequest(
        static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_FOLDER_ITEMS),
        data, reply, option);
    EXPECT_EQ(ret, TRANSACTION_ERR);
    HILOGI("AvrcpCtObserverStubTest_OnGetFolderItemsInner end");
}
} // Bluetooth
} // OHOS