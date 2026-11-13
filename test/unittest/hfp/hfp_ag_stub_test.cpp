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
#include <cstring>

#include "bluetooth_access_token_mock.h"
#include "bluetooth_host.h"
#include "bluetooth_hfp_ag_server.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
static std::unique_ptr<BluetoothHfpAgServer> hfpAgServer_ {nullptr};

const int TIME = 4;

class HfpAgStubTest : public testing::Test {
public:
    static void SetUpTestCase()
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
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase start");
        sleep(TIME);
        host_ = nullptr;
        utility::CloseAllTimer();
        HILOGI("TearDownTestCase end");
    }
    void SetUp()
    {
        hfpAgServer_ = std::make_unique<BluetoothHfpAgServer>();
    }
    void TearDown()
    {
        hfpAgServer_ = nullptr;
    }
};

/*
 * @tc.number: Hfp_Ag_Stub_OnRemoteRequest_00100
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_OnRemoteRequest_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_OnRemoteRequest_00100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    EXPECT_EQ(hfpAgServer_->OnRemoteRequest(static_cast<uint32_t>(BluetoothHfpAgInterfaceCode::BT_HFP_AG_CONNECT),
        data, reply, option), BT_ERR_IPC_TRANS_FAILED);

    data.WriteInterfaceToken(BluetoothHfpAgStub::GetDescriptor());
    EXPECT_EQ(hfpAgServer_->OnRemoteRequest(99, data, reply, option), 305);
    HILOGI("Hfp_Ag_Stub_OnRemoteRequest_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_OnRemoteRequest_00200
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_OnRemoteRequest_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_OnRemoteRequest_00200 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    data.WriteInterfaceToken(BluetoothHfpAgStub::GetDescriptor());
    hfpAgServer_->OnRemoteRequest(static_cast<uint32_t>(BluetoothHfpAgInterfaceCode::BT_HFP_AG_GET_CONNECT_DEVICES),
        data, reply, option);

    EXPECT_EQ(reply.ReadInt32(), 0);
    HILOGI("Hfp_Ag_Stub_OnRemoteRequest_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetDevicesByStatesInner_00100
 * @tc.name: GetDevicesByStatesInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetDevicesByStatesInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetDevicesByStatesInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    std::vector<int> states;
    data.WriteInt32Vector(states);

    hfpAgServer_->GetDevicesByStatesInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), 0);

    HILOGI("Hfp_Ag_Stub_GetDevicesByStatesInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetDeviceStateInner_00100
 * @tc.name: GetDeviceStateInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetDeviceStateInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetDeviceStateInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(hfpAgServer_->GetDeviceStateInner(data, reply), BT_ERR_IPC_TRANS_FAILED);

    BluetoothRawAddress address("11:22:33:44:55:66");
    data.WriteParcelable(&address);
    hfpAgServer_->GetDeviceStateInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    HILOGI("Hfp_Ag_Stub_GetDeviceStateInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetDeviceStateInner_00200
 * @tc.name: GetDeviceStateInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetDeviceStateInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetDeviceStateInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->GetDeviceStateInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_GetDeviceStateInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_ConnectInner_00100
 * @tc.name: ConnectInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_ConnectInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_ConnectInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(hfpAgServer_->ConnectInner(data, reply), BT_ERR_IPC_TRANS_FAILED);

    BluetoothRawAddress address("11:22:33:44:55:66");
    data.WriteParcelable(&address);
    hfpAgServer_->ConnectInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    HILOGI("Hfp_Ag_Stub_ConnectInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_ConnectInner_00200
 * @tc.name: ConnectInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_ConnectInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_ConnectInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->ConnectInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_ConnectInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_DisconnectInner_00100
 * @tc.name: DisconnectInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_DisconnectInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_DisconnectInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(hfpAgServer_->DisconnectInner(data, reply), BT_ERR_IPC_TRANS_FAILED);

    BluetoothRawAddress address("11:22:33:44:55:66");
    data.WriteParcelable(&address);
    hfpAgServer_->DisconnectInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    HILOGI("Hfp_Ag_Stub_DisconnectInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_DisconnectInner_00200
 * @tc.name: DisconnectInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_DisconnectInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_DisconnectInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->DisconnectInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_DisconnectInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetScoStateInner_00100
 * @tc.name: GetScoStateInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetScoStateInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetScoStateInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(hfpAgServer_->GetScoStateInner(data, reply), TRANSACTION_ERR);

    BluetoothRawAddress address("11:22:33:44:55:66");
    data.WriteParcelable(&address);
    hfpAgServer_->GetScoStateInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_FAILURE);

    HILOGI("Hfp_Ag_Stub_GetScoStateInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetScoStateInner_00200
 * @tc.name: GetScoStateInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetScoStateInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetScoStateInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->GetScoStateInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_FAILURE);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_GetScoStateInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_ConnectScoInner_00100
 * @tc.name: ConnectScoInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_ConnectScoInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_ConnectScoInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    hfpAgServer_->ConnectScoInner(data, reply);
    EXPECT_FALSE(reply.ReadBool());

    HILOGI("Hfp_Ag_Stub_ConnectScoInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_DisconnectScoInner_00100
 * @tc.name: DisconnectScoInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_DisconnectScoInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_DisconnectScoInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    hfpAgServer_->DisconnectScoInner(data, reply);
    EXPECT_FALSE(reply.ReadBool());

    HILOGI("Hfp_Ag_Stub_DisconnectScoInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_PhoneStateChangedInner_00100
 * @tc.name: PhoneStateChangedInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_PhoneStateChangedInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_PhoneStateChangedInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(1);
    phoneState.SetHeldNum(2);
    phoneState.SetCallState(3);
    phoneState.SetNumber("123");
    phoneState.SetCallType(4);
    phoneState.SetName("name");
    data.WriteParcelable(&phoneState);
    EXPECT_EQ(hfpAgServer_->PhoneStateChangedInner(data, reply), NO_ERROR);

    HILOGI("Hfp_Ag_Stub_PhoneStateChangedInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_ClccResponseInner_00100
 * @tc.name: ClccResponseInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_ClccResponseInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_ClccResponseInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    data.WriteInt32(1);
    data.WriteInt32(2);
    data.WriteInt32(3);
    data.WriteInt32(4);
    data.WriteBool(true);
    data.WriteString("123");
    data.WriteInt32(5);
    EXPECT_EQ(hfpAgServer_->PhoneStateChangedInner(data, reply), BT_ERR_IPC_TRANS_FAILED);

    HILOGI("Hfp_Ag_Stub_ClccResponseInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_OpenVoiceRecognitionInner_00100
 * @tc.name: OpenVoiceRecognitionInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_OpenVoiceRecognitionInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_OpenVoiceRecognitionInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hfpAgServer_->OpenVoiceRecognitionInner(data, reply), TRANSACTION_ERR);
    HILOGI("Hfp_Ag_Stub_OpenVoiceRecognitionInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_OpenVoiceRecognitionInner_00200
 * @tc.name: OpenVoiceRecognitionInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_OpenVoiceRecognitionInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_OpenVoiceRecognitionInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    EXPECT_EQ(hfpAgServer_->OpenVoiceRecognitionInner(data, reply), BT_ERR_INVALID_PARAM);

    HILOGI("Hfp_Ag_Stub_OpenVoiceRecognitionInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_OpenVoiceRecognitionInner_00300
 * @tc.name: OpenVoiceRecognitionInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_OpenVoiceRecognitionInner_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_OpenVoiceRecognitionInner_00300 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->OpenVoiceRecognitionInner(data, reply);
    EXPECT_FALSE(reply.ReadInt32());
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_OpenVoiceRecognitionInner_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_CloseVoiceRecognitionInner_00100
 * @tc.name: CloseVoiceRecognitionInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_CloseVoiceRecognitionInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_CloseVoiceRecognitionInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hfpAgServer_->CloseVoiceRecognitionInner(data, reply), TRANSACTION_ERR);
    HILOGI("Hfp_Ag_Stub_CloseVoiceRecognitionInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_CloseVoiceRecognitionInner_00200
 * @tc.name: CloseVoiceRecognitionInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_CloseVoiceRecognitionInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_CloseVoiceRecognitionInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    EXPECT_EQ(hfpAgServer_->CloseVoiceRecognitionInner(data, reply), BT_ERR_INVALID_PARAM);

    HILOGI("Hfp_Ag_Stub_CloseVoiceRecognitionInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_CloseVoiceRecognitionInner_00300
 * @tc.name: CloseVoiceRecognitionInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_CloseVoiceRecognitionInner_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_CloseVoiceRecognitionInner_00300 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->CloseVoiceRecognitionInner(data, reply);
    EXPECT_FALSE(reply.ReadInt32());
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_CloseVoiceRecognitionInner_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_SetActiveDeviceInner_00100
 * @tc.name: SetActiveDeviceInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_SetActiveDeviceInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_SetActiveDeviceInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hfpAgServer_->SetActiveDeviceInner(data, reply), TRANSACTION_ERR);
    HILOGI("Hfp_Ag_Stub_SetActiveDeviceInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_SetActiveDeviceInner_00200
 * @tc.name: SetActiveDeviceInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_SetActiveDeviceInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_SetActiveDeviceInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    EXPECT_EQ(hfpAgServer_->SetActiveDeviceInner(data, reply), NO_ERROR);

    HILOGI("Hfp_Ag_Stub_SetActiveDeviceInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_SetActiveDeviceInner_00300
 * @tc.name: SetActiveDeviceInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_SetActiveDeviceInner_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_SetActiveDeviceInner_00300 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->SetActiveDeviceInner(data, reply);
    EXPECT_FALSE(reply.ReadInt32());
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_SetActiveDeviceInner_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetActiveDeviceInner_00100
 * @tc.name: GetActiveDeviceInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetActiveDeviceInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetActiveDeviceInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    hfpAgServer_->GetActiveDeviceInner(data, reply);
    EXPECT_EQ(reply.ReadString(), "");

    HILOGI("Hfp_Ag_Stub_GetActiveDeviceInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_SetConnectStrategyInner_00100
 * @tc.name: SetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_SetConnectStrategyInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_SetConnectStrategyInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hfpAgServer_->SetConnectStrategyInner(data, reply), BT_ERR_IPC_TRANS_FAILED);
    HILOGI("Hfp_Ag_Stub_SetConnectStrategyInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_SetConnectStrategyInner_00200
 * @tc.name: SetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_SetConnectStrategyInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_SetConnectStrategyInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress address("11:22:33:44:55:66");
    data.WriteParcelable(&address);
    hfpAgServer_->SetConnectStrategyInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hfp_Ag_Stub_SetConnectStrategyInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_SetConnectStrategyInner_00300
 * @tc.name: SetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_SetConnectStrategyInner_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_SetConnectStrategyInner_00300 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->SetConnectStrategyInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_SetConnectStrategyInner_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetConnectStrategyInner_00100
 * @tc.name: GetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetConnectStrategyInner_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetConnectStrategyInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hfpAgServer_->GetConnectStrategyInner(data, reply), BT_ERR_IPC_TRANS_FAILED);
    HILOGI("Hfp_Ag_Stub_GetConnectStrategyInner_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetConnectStrategyInner_00200
 * @tc.name: GetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetConnectStrategyInner_00200, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetConnectStrategyInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress address("11:22:33:44:55:66");
    data.WriteParcelable(&address);
    hfpAgServer_->GetConnectStrategyInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hfp_Ag_Stub_GetConnectStrategyInner_00200 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetConnectStrategyInner_00300
 * @tc.name: GetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetConnectStrategyInner_00300, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetConnectStrategyInner_00300 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->GetConnectStrategyInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hfp_Ag_Stub_GetConnectStrategyInner_00300 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_ConnectScoInnerEx_00100
 * @tc.name: ConnectScoInnerEx
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_ConnectScoInnerEx_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_ConnectScoInnerEx_00100 start");

    MessageParcel data;
    MessageParcel reply;

    uint8_t callType = static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL);
    data.WriteUint8(callType);

    hfpAgServer_->ConnectScoInnerEx(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    HILOGI("Hfp_Ag_Stub_ConnectScoInnerEx_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_DisconnectScoInnerEx_00100
 * @tc.name: DisconnectScoInnerEx
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_DisconnectScoInnerEx_00100, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_DisconnectScoInnerEx_00100 start");

    MessageParcel data;
    MessageParcel reply;

    uint8_t callType = static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL);
    data.WriteUint8(callType);

    hfpAgServer_->DisconnectScoInnerEx(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    HILOGI("Hfp_Ag_Stub_DisconnectScoInnerEx_00100 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_CallDetailsChangedInner_001
 * @tc.name: CallDetailsChangedInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_CallDetailsChangedInner_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_CallDetailsChangedInner_001 start");

    MessageParcel data;
    MessageParcel reply;

    int callId = 1;
    int callState = 1;
    data.WriteUint8(callId);
    data.WriteUint8(callState);

    hfpAgServer_->CallDetailsChangedInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_NO_ERROR);

    HILOGI("Hfp_Ag_Stub_CallDetailsChangedInner_001 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_IsHfpFeatureSupportedInner_001
 * @tc.name: IsHfpFeatureSupportedInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_IsHfpFeatureSupportedInner_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_IsHfpFeatureSupportedInner_001 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);
    data.WriteParcelable(&randomAddr);

    hfpAgServer_->IsHfpFeatureSupportedInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();

    HILOGI("Hfp_Ag_Stub_IsVgsSupportedInner_001 end, result:%{public}d", reply.ReadInt32());
}

/*
 * @tc.number: Hfp_Ag_Stub_GetVirtualDeviceListInner_001
 * @tc.name: GetVirtualDeviceListInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetVirtualDeviceListInner_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetVirtualDeviceListInner_001 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hfpAgServer_->GetVirtualDeviceListInner(data, reply), BT_NO_ERROR);
    HILOGI("Hfp_Ag_Stub_GetVirtualDeviceListInner_001 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_UpdateVirtualDeviceInner_001
 * @tc.name: UpdateVirtualDeviceInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_UpdateVirtualDeviceInner_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_UpdateVirtualDeviceInner_001 start");

    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(bluetooth::VIRTUAL_DEVICE_ADD);
    data.WriteString("11:22:33:44:55:66");

    EXPECT_EQ(hfpAgServer_->UpdateVirtualDeviceInner(data, reply), BT_NO_ERROR);
    HILOGI("Hfp_Ag_Stub_UpdateVirtualDeviceInner_001 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_GetCurrentCallTypeInner_001
 * @tc.name: GetCurrentCallTypeInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_GetCurrentCallTypeInner_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_GetCurrentCallTypeInner_001 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hfpAgServer_->GetCurrentCallTypeInner(data, reply), BT_NO_ERROR);
    HILOGI("Hfp_Ag_Stub_GetCurrentCallTypeInner_001 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_ClccResponseInner_001
 * @tc.name: ClccResponseInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_ClccResponseInner_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_ClccResponseInner_001 start");

    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteBool(true);
    data.WriteString("1");
    data.WriteInt32(1);

    EXPECT_EQ(hfpAgServer_->ClccResponseInner(data, reply), BT_NO_ERROR);
    HILOGI("Hfp_Ag_Stub_ClccResponseInner_001 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_EnableBtCallLogInner_001
 * @tc.name: EnableBtCallLogInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_EnableBtCallLogInner_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_EnableBtCallLogInner_001 start");

    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(true);
    EXPECT_EQ(hfpAgServer_->EnableBtCallLogInner(data, reply), BT_NO_ERROR);

    HILOGI("Hfp_Ag_Stub_IsVgsSupportedInner_001 end, result:%{public}d", reply.ReadInt32());
}

/*
 * @tc.number: Hfp_Ag_Stub_IsAudioOnInner_001
 * @tc.name: IsAudioOnInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_IsAudioOnInner_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_IsAudioOnInner_001 start");
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(hfpAgServer_->IsAudioConnectedInner(data, reply), BT_NO_ERROR);
    HILOGI("Hfp_Ag_Stub_IsAudioOnInner_001 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_Phone_State_WriteToParcel_001
 * @tc.name: EnableBtCallLogInner
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_Phone_State_WriteToParcel_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_Phone_State_WriteToParcel_001 start");
    BluetoothPhoneState phoneState;
    OHOS::Parcel parcel;
    EXPECT_TRUE(phoneState.WriteToParcel(parcel));
    HILOGI("Hfp_Ag_Stub_Phone_State_WriteToParcel_001 end");
}

/*
 * @tc.number: Hfp_Ag_Stub_Raw_Address_WriteToParcel_001
 * @tc.name: WriteToParcel
 * @tc.desc:
 */
HWTEST_F(HfpAgStubTest, Hfp_Ag_Stub_Raw_Address_WriteToParcel_001, TestSize.Level1)
{
    HILOGI("Hfp_Ag_Stub_Raw_Address_WriteToParcel_001 start");
    BluetoothRawAddress address;
    OHOS::Parcel parcel;
    EXPECT_TRUE(address.WriteToParcel(parcel));
    HILOGI("Hfp_Ag_Stub_Raw_Address_WriteToParcel_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS