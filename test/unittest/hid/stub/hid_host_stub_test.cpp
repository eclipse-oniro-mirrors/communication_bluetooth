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
#include "bluetooth_host.h"
#include "bluetooth_hid_host_server.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
static std::unique_ptr<BluetoothHidHostServer> hidHostServer_ {nullptr};

const int TIME = 4;

class HidHostStubTest : public testing::Test {
public:
    static void SetUpTestCase()
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
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase");
    }
    void SetUp()
    {
        hidHostServer_ = std::make_unique<BluetoothHidHostServer>();
    }
    void TearDown()
    {
        hidHostServer_ = nullptr;
    }
    void SetRandomAddr(BluetoothRawAddress &device)
    {
        string randomAddr;
        host_->GetRandomAddress(device.GetAddress(), randomAddr);
        device.address_ = randomAddr;
    }
};

/*
 * @tc.number: Hid_Stub_OnRemoteRequest_00100
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_OnRemoteRequest_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_OnRemoteRequest_00100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int ret = hidHostServer_
        ->OnRemoteRequest(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_CONNECT), data, reply, option);
    EXPECT_EQ(ret, BT_ERR_IPC_TRANS_FAILED);

    data.WriteInterfaceToken(BluetoothHidHostStub::GetDescriptor());
    hidHostServer_->OnRemoteRequest(99, data, reply, option);
    HILOGI("Hid_Stub_OnRemoteRequest_00100 end");
}

/*
 * @tc.number: Hid_Stub_ConnectInner_00100
 * @tc.name: ConnectInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_ConnectInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_ConnectInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    hidHostServer_->ConnectInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    MessageParcel dataEmpty;
    EXPECT_EQ(hidHostServer_->ConnectInner(dataEmpty, reply), BT_ERR_IPC_TRANS_FAILED);

    HILOGI("Hid_Stub_ConnectInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_DisconnectInner_00100
 * @tc.name: DisconnectInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_DisconnectInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_DisconnectInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    hidHostServer_->DisconnectInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    MessageParcel dataEmpty;
    EXPECT_EQ(hidHostServer_->DisconnectInner(dataEmpty, reply), BT_ERR_IPC_TRANS_FAILED);

    HILOGI("Hid_Stub_DisconnectInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_GetDeviceStateInner_00100
 * @tc.name: GetDeviceStateInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_GetDeviceStateInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_GetDeviceStateInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    BluetoothRawAddress address;
    data.WriteParcelable(&address);

    hidHostServer_->GetDeviceStateInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    MessageParcel dataEmpty;
    EXPECT_EQ(hidHostServer_->GetDeviceStateInner(dataEmpty, reply), BT_ERR_IPC_TRANS_FAILED);

    HILOGI("Hid_Stub_GetDeviceStateInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_GetDevicesByStatesInner_00100
 * @tc.name: GetDevicesByStatesInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_GetDevicesByStatesInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_GetDevicesByStatesInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    data.WriteInt32(300);
    EXPECT_EQ(hidHostServer_->GetDevicesByStatesInner(data, reply), BT_ERR_INVALID_PARAM);

    data.WriteInt32(1);
    hidHostServer_->GetDevicesByStatesInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    HILOGI("Hid_Stub_GetDevicesByStatesInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_RegisterObserverInner_00100
 * @tc.name: RegisterObserverInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_RegisterObserverInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_RegisterObserverInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    data.WriteRemoteObject(nullptr);
    hidHostServer_->RegisterObserverInner(data, reply);
    EXPECT_NE(hidHostServer_, nullptr);

    HILOGI("Hid_Stub_RegisterObserverInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_DeregisterObserverInner_00100
 * @tc.name: DeregisterObserverInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_DeregisterObserverInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_DeregisterObserverInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    data.WriteRemoteObject(nullptr);
    hidHostServer_->DeregisterObserverInner(data, reply);
    EXPECT_NE(hidHostServer_, nullptr);

    HILOGI("Hid_Stub_DeregisterObserverInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_HidHostVCUnplugInner_00100
 * @tc.name: HidHostVCUnplugInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_HidHostVCUnplugInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_HidHostVCUnplugInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hidHostServer_->HidHostVCUnplugInner(data, reply), NO_ERROR);

    HILOGI("Hid_Stub_HidHostVCUnplugInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_HidHostSendDataInner_00100
 * @tc.name: HidHostSendDataInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_HidHostSendDataInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_HidHostSendDataInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hidHostServer_->HidHostSendDataInner(data, reply), NO_ERROR);

    HILOGI("Hid_Stub_HidHostSendDataInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_HidHostSetReportInner_00100
 * @tc.name: HidHostSetReportInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_HidHostSetReportInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_HidHostSetReportInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hidHostServer_->HidHostSetReportInner(data, reply), NO_ERROR);

    HILOGI("Hid_Stub_HidHostSetReportInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_HidHostGetReportInner_00100
 * @tc.name: HidHostGetReportInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_HidHostGetReportInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_HidHostGetReportInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hidHostServer_->HidHostGetReportInner(data, reply), NO_ERROR);

    HILOGI("Hid_Stub_HidHostGetReportInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_HidHostSetConnectStrategyInner_00100
 * @tc.name: HidHostSetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_HidHostSetConnectStrategyInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_HidHostSetConnectStrategyInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hidHostServer_->HidHostSetConnectStrategyInner(data, reply), BT_ERR_IPC_TRANS_FAILED);

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    hidHostServer_->HidHostSetConnectStrategyInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);

    data.WriteParcelable(&randomAddr);
    hidHostServer_->HidHostSetConnectStrategyInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    HILOGI("Hid_Stub_HidHostSetConnectStrategyInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_HidHostGetConnectStrategyInner_00100
 * @tc.name: HidHostGetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_HidHostGetConnectStrategyInner_00100, TestSize.Level1)
{
    HILOGI("Hid_Stub_HidHostGetConnectStrategyInner_00100 start");

    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(hidHostServer_->HidHostGetConnectStrategyInner(data, reply), BT_ERR_IPC_TRANS_FAILED);

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    hidHostServer_->HidHostGetConnectStrategyInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);

    HILOGI("Hid_Stub_HidHostGetConnectStrategyInner_00100 end");
}

/*
 * @tc.number: Hid_Stub_HidHostGetConnectStrategyInner_00200
 * @tc.name: HidHostGetConnectStrategyInner
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Stub_HidHostGetConnectStrategyInner_00200, TestSize.Level1)
{
    HILOGI("Hid_Stub_HidHostGetConnectStrategyInner_00200 start");

    MessageParcel data;
    MessageParcel reply;

    BluetoothRawAddress realAddr("00:00:00:00:00:00");
    BluetoothRawAddress randomAddr("00:00:00:00:00:11");
    BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddr, randomAddr);

    data.WriteParcelable(&randomAddr);
    hidHostServer_->HidHostGetConnectStrategyInner(data, reply);
    EXPECT_EQ(reply.ReadInt32(), BT_ERR_INTERNAL_ERROR);
    BluetoothDeviceManager::GetInstance()->bluetoothDeviceManagerTimer->Stop();
    sleep(TIME);
    HILOGI("Hid_Stub_HidHostGetConnectStrategyInner_00200 end");
}

/*
 * @tc.number: Hid_Host_RegisterObserver_002
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(HidHostStubTest, Hid_Host_RegisterObserver_002, TestSize.Level1)
{
    HILOGI("Hid_Host_RegisterObserver_002 start");
    for(int i = 0; i <= 1001; i++) {
        sptr<IBluetoothHidHostObserver> observer;
        hidHostServer_->RegisterObserver(observer);
    }
    EXPECT_TRUE(hidHostServer_ != nullptr);
    HILOGI("Hid_Host_RegisterObserver_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS