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

#include <gtest/gtest.h>
#include "bluetooth_avrcp_tg_stub.h"
#include "bluetooth_avrcp_tg_server.h"
#include "class_creator.h"
#include "avrcp_tg_service.h"
#include "bluetooth_log.h"
#include "bluetooth_host.h"
#include "bluetooth_errorcode.h"


using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
void *AvrcpTgServiceCreate(void)
{
    return new AvrcpTgService();
}

static BluetoothHost *host_;
const int TIME = 2;

class AvrcpTgStubTest : public testing::Test {
public:
    AvrcpTgStubTest()
    {}
    ~AvrcpTgStubTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::unique_ptr<BluetoothAvrcpTgServer> avrcpTgServer {nullptr};
};

void AvrcpTgStubTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();
    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBleEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }
    ClassFactory::RegisterClass("AvrcpTgService", AvrcpTgServiceCreate);
    HILOGI("SetUpTestCase end");
}

void AvrcpTgStubTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase");
}

void AvrcpTgStubTest::SetUp()
{
    avrcpTgServer = std::make_unique<BluetoothAvrcpTgServer>();
}

void AvrcpTgStubTest::TearDown()
{
    avrcpTgServer = nullptr;
}


/*
 * @tc.number: AvrcpTgStubTest_OnRemoteRequest001
 * @tc.name: AvrcpTgStubTest_OnRemoteRequest001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_OnRemoteRequest001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_OnRemoteRequest001 start");
    uint32_t code = 23;
    MessageParcel data;
    data.WriteInterfaceToken(BluetoothAvrcpTgServer::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(avrcpTgServer->OnRemoteRequest(code, data, reply, option), 305);
    HILOGI("AvrcpTgStubTest_OnRemoteRequest001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_SetActiveDeviceInner001
 * @tc.name: AvrcpTgStubTest_SetActiveDeviceInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_SetActiveDeviceInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_SetActiveDeviceInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress bluetoothRawAddress;
    data1.WriteParcelable(&bluetoothRawAddress);
    EXPECT_EQ(avrcpTgServer->SetActiveDeviceInner(data, reply), 1);
    EXPECT_EQ(avrcpTgServer->SetActiveDeviceInner(data1, reply), BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpTgStubTest_SetActiveDeviceInner001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_ConnectInner001
 * @tc.name: AvrcpTgStubTest_ConnectInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_ConnectInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_ConnectInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress bluetoothRawAddress;
    data1.WriteParcelable(&bluetoothRawAddress);
    EXPECT_EQ(avrcpTgServer->ConnectInner(data, reply), 1);
    EXPECT_EQ(avrcpTgServer->ConnectInner(data1, reply), BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpTgStubTest_ConnectInner001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_DisconnectInner001
 * @tc.name: AvrcpTgStubTest_DisconnectInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_DisconnectInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_DisconnectInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress bluetoothRawAddress;
    data1.WriteParcelable(&bluetoothRawAddress);
    EXPECT_EQ(avrcpTgServer->DisconnectInner(data, reply), 1);
    EXPECT_EQ(avrcpTgServer->DisconnectInner(data1, reply), BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpTgStubTest_DisconnectInner001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_GetConnectedDevicesInner001
 * @tc.name: AvrcpTgStubTest_GetConnectedDevicesInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_GetConnectedDevicesInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_GetConnectedDevicesInner001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(avrcpTgServer->GetConnectedDevicesInner(data, reply), BT_NO_ERROR);
    HILOGI("AvrcpTgStubTest_GetConnectedDevicesInner001 start");
}


/*
 * @tc.number: AvrcpTgStubTest_GetDevicesByStatesInner001
 * @tc.name: AvrcpTgStubTest_GetDevicesByStatesInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_GetDevicesByStatesInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_GetDevicesByStatesInner001 start");
    MessageParcel data;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    data1.WriteInt32(0x01);
    MessageParcel reply;
    EXPECT_EQ(avrcpTgServer->GetDevicesByStatesInner(data, reply), BT_NO_ERROR);
    EXPECT_EQ(avrcpTgServer->GetDevicesByStatesInner(data1, reply), BT_NO_ERROR);
    HILOGI("AvrcpTgStubTest_GetDevicesByStatesInner001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_GetDeviceStateInner001
 * @tc.name: AvrcpTgStubTest_GetDeviceStateInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_GetDeviceStateInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_GetDeviceStateInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress bluetoothRawAddress;
    data1.WriteParcelable(&bluetoothRawAddress);
    EXPECT_EQ(avrcpTgServer->GetDeviceStateInner(data, reply), 1);
    EXPECT_EQ(avrcpTgServer->GetDeviceStateInner(data1, reply), BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpTgStubTest_GetDeviceStateInner001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_NotifyPlayerAppSettingChangedInner001
 * @tc.name: AvrcpTgStubTest_NotifyPlayerAppSettingChangedInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_NotifyPlayerAppSettingChangedInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_NotifyPlayerAppSettingChangedInner001 start");
    MessageParcel data;
    MessageParcel data1;
    data1.WriteInt32(0x01);
    data1.WriteInt32(0x01);
    data1.WriteInt32(0x01);
    data1.WriteInt32(0x01);
    MessageParcel reply;
    EXPECT_EQ(avrcpTgServer->NotifyPlayerAppSettingChangedInner(data, reply), BT_NO_ERROR);
    EXPECT_EQ(avrcpTgServer->NotifyPlayerAppSettingChangedInner(data1, reply), BT_NO_ERROR);
    HILOGI("AvrcpTgStubTest_NotifyPlayerAppSettingChangedInner001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_SetDeviceAbsoluteVolumeInner001
 * @tc.name: AvrcpTgStubTest_SetDeviceAbsoluteVolumeInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_SetDeviceAbsoluteVolumeInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_SetDeviceAbsoluteVolumeInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress bluetoothRawAddress;
    data1.WriteParcelable(&bluetoothRawAddress);
    data1.WriteInt32(0x01);
    EXPECT_EQ(avrcpTgServer->SetDeviceAbsoluteVolumeInner(data, reply), BT_ERR_IPC_TRANS_FAILED);
    EXPECT_EQ(avrcpTgServer->SetDeviceAbsoluteVolumeInner(data1, reply), BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpTgStubTest_SetDeviceAbsoluteVolumeInner001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_GetDeviceAbsVolumeAbilityInner001
 * @tc.name: AvrcpTgStubTest_GetDeviceAbsVolumeAbilityInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_GetDeviceAbsVolumeAbilityInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_GetDeviceAbsVolumeAbilityInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress bluetoothRawAddress;
    data1.WriteParcelable(&bluetoothRawAddress);
    EXPECT_EQ(avrcpTgServer->GetDeviceAbsVolumeAbilityInner(data, reply), BT_ERR_IPC_TRANS_FAILED);
    EXPECT_EQ(avrcpTgServer->GetDeviceAbsVolumeAbilityInner(data1, reply), BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpTgStubTest_GetDeviceAbsVolumeAbilityInner001 start");
}

/*
 * @tc.number: AvrcpTgStubTest_SetDeviceAbsVolumeAbilityInner001
 * @tc.name: AvrcpTgStubTest_SetDeviceAbsVolumeAbilityInner001
*/
HWTEST_F(AvrcpTgStubTest, AvrcpTgStubTest_SetDeviceAbsVolumeAbilityInner001, TestSize.Level1)
{
    HILOGI("AvrcpTgStubTest_SetDeviceAbsVolumeAbilityInner001 start");
    MessageParcel data;
    MessageParcel reply;
    MessageParcel data1;
    BluetoothRawAddress bluetoothRawAddress;
    data1.WriteParcelable(&bluetoothRawAddress);
    data1.WriteInt32(0x01);
    EXPECT_EQ(avrcpTgServer->SetDeviceAbsVolumeAbilityInner(data, reply), BT_ERR_IPC_TRANS_FAILED);
    EXPECT_EQ(avrcpTgServer->SetDeviceAbsVolumeAbilityInner(data1, reply), BT_ERR_INVALID_PARAM);
    HILOGI("AvrcpTgStubTest_SetDeviceAbsVolumeAbilityInner001 start");
}

}
}