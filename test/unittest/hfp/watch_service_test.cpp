/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd. All rights reserved.
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
#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include "bluetooth_host.h"
#include "watch_service.h"
#include "watch_pbap_client.h"
#include "bluetooth_highpower_manager.h"
#include "hfp_hf_service.h"
#include "bt_def.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"
#include "connect_strategy_manager.h"
#include "profile_config.h"
#include "adapter_manager.h"
#include "adapter_config.h"
#include "class_creator.h"
#include "classic_adapter.h"
#include "parameters.h"
#include "bluetooth_connection_manager.h"
#include "mock/watch_service_interface_mock.h"
#include "watch_device_manager.h"
#include "bt_func_hook.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
using namespace bluetooth::watch;

namespace OHOS {
namespace bluetooth {

std::string address("11:22:33:44:55:66");
std::string address2("00:11:22:33:44:55");
const int TIME = 2;
const uint16_t UNKNOWN_VOIP_STATE = -1;
const uint16_t INVALID_VOIP_STATE = 3;
const char *g_testStateOff = "0";
const char *g_testStateOn = "1";
static BluetoothHost *host_;
static WatchService *watchService_;
static HfpHfService *hfpHfService_ {nullptr};
HfpHfStateMachine stateMachine(address);
static std::shared_ptr<bt_interface_t> btInterface;

static bool is_profile(const char *p1, const char *p2)
{
    return strlen(p1) == strlen(p2) && strncmp(p1, p2, strlen(p2)) == 0;
}

const void* GetProfileInterfaceMock(const char* profileId)
{
    HILOGI("enter");
    if (is_profile(profileId, BT_VENDOR_WATCH_ID)) {
        BtHwWatchInterface *interface = (BtHwWatchInterface *)malloc(sizeof(BtHwWatchInterface));
        if (interface != nullptr) {
            interface->init = initWatchMock;
            interface->sendTbsf = sendTbsfMock;
            interface->sendTbsr = sendTbsrMock;
            interface->sendScreenOn2Mcu = sendScreenOn2McuMock;
            interface->sendHfpState2Hisi = sendHfpState2HisiMock;
            interface->isPhoneMajorClass = IsPhoneMajorClassMock;
            interface->updateDevice = updateDeviceMock;
            interface->updateReconnectState = updateReconnectStateMock;
        }
        return (void *)interface;
    }

    if (is_profile(profileId, BT_PROFILE_HIDDEV_ID)) {
        bthd_interface_t *interface = (bthd_interface_t *)malloc(sizeof(bthd_interface_t));
        if (interface != nullptr) {
            interface->init = InitMock;
            interface->register_app = RegisterAppMock;
            interface->unregister_app = UnregisterAppMock;
            interface->connect = ConnectMock;
            interface->disconnect = DisconnectMock;
            interface->send_report = SendReportMock;
        }
        return (void *)interface;
    }

    return nullptr;
}

// 等待 pbap 客户端任务排空且状态回 IDLE（单测环境 SPP 不可用，连接必失败）
static void WaitPbapClientIdle(const std::shared_ptr<WatchPbapClient> &client)
{
    for (int i = 0; i < 300; i++) {
        bool drained = false;
        if (client->state_.load() == WatchPbapState::IDLE) {
            std::lock_guard<std::mutex> lock(client->taskMutex_);
            drained = client->taskQueue_.empty();
        }
        if (drained) {
            // 规避"任务已出队未执行"窗口：稳定确认一次最终状态
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (client->state_.load() == WatchPbapState::IDLE) {
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// 等待 pbap 客户端工作线程退出（Disconnect 任务完成）
static void WaitPbapClientStopped(const std::shared_ptr<WatchPbapClient> &client)
{
    for (int i = 0; i < 300; i++) {
        {
            std::lock_guard<std::mutex> lock(client->taskMutex_);
            if (!client->running_ && client->taskQueue_.empty()) {
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

class WatchServiceTest : public testing::Test {
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

        btInterface = std::make_shared<bt_interface_t>();
        btInterface->get_profile_interface = GetProfileInterfaceMock;
        auto manager = ProfileServiceManager::GetInstance();
        std::string profileName = "HfpHfService";
        manager->SetProfileService(ADAPTER_BREDR, profileName);
        hfpHfService_ = static_cast<HfpHfService *>(manager->GetProfileService(profileName));

        watchService_ = WatchService::GetInstance();
    }
    static void TearDownTestCase()
    {
        btInterface = nullptr;
        system::SetParameter("persist.bluetooth.watch_str_enable", "0");
        RemoveDeviceManager(address);
        watchService_->Stop();
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
public:
};

/*
 * @tc.number: watch_service_Initialize_0001
 * @tc.name: Initialize
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_Initialize_0001, TestSize.Level1)
{
    HILOGI("watch_service_Initialize_0001 start");

    watchService_->Initialize();

    watchService_->Uninitialize();

    HILOGI("watch_service_Initialize_0001 end");
}

/*
 * @tc.number: watch_service_Start_0001
 * @tc.name: Start
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_Start_0001, TestSize.Level1)
{
    HILOGI("watch_service_Start_0001 start");

    watchService_->Start();
    EXPECT_EQ(watchService_->bluetoothWatchInterface, nullptr);
    EXPECT_EQ(watchService_->bluetoothHidInterface, nullptr);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    EXPECT_NE(watchService_->bluetoothWatchInterface, nullptr);
    EXPECT_NE(watchService_->bluetoothHidInterface, nullptr);
    AddDeviceManager(address);
    watchService_->Stop();

    HILOGI("watch_service_Start_0001 end");
}

/*
 * @tc.number: watch_service_BluetoothIsOn_0001
 * @tc.name: BluetoothIsOn
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_BluetoothIsOn_0001, TestSize.Level1)
{
    HILOGI("watch_service_BluetoothIsOn_0001 start");

    SetStatus(BT_STATUS_SUCCESS);
    RawAddress device(address);
    AdapterConfig::GetInstance()->Load();
    AdapterManager::GetInstance()->CreateAdapters();
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    watchService_->BluetoothIsOn();
    watchService_->StopSaisAddedTimer();

    HILOGI("watch_service_BluetoothIsOn_0001 end");
}

/*
 * @tc.number: watch_service_ChangeStatusOfEnterStrMode_0001
 * @tc.name: ChangeStatusOfEnterStrMode
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ChangeStatusOfEnterStrMode_0001, TestSize.Level1)
{
    HILOGI("watch_service_ChangeStatusOfEnterStrMode_0001 start");

    watchService_->ChangeStatusOfEnterStrMode();

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfAudioConnected audioConnected(AUDIO_CONNECTED, stateMachine, connectedState);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->Init();
    hfpHfService_->stateMachines_[address]->current_ = &audioConnected;
    AddDeviceManager(address);
    watchService_->ChangeStatusOfEnterStrMode();
    hfpHfService_->stateMachines_[address] = nullptr;
    system::SetParameter("persist.bluetooth.watch_str_enable", "0");

    HILOGI("watch_service_ChangeStatusOfEnterStrMode_0001 end");
}

/*
 * @tc.number: watch_service_StartConnectPhone_0001
 * @tc.name: StartConnectPhone
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_StartConnectPhone_0001, TestSize.Level1)
{
    HILOGI("watch_service_StartConnectPhone_0001 start");

    RawAddress device(address);
    SetStatus(BT_STATUS_SUCCESS);
    AddDeviceManager(address);
    watchService_->StartConnectPhone();
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isSaisDevice);
    watchService_->isSaisAdded_ = true;
    watchService_->ProcessHfpStateChangeEvt(device, HFP_HF_STATE_CONNECTED);
    watchService_->StartConnectPhone();

    HILOGI("watch_service_StartConnectPhone_0001 end");
}

/*
 * @tc.number: watch_service_SetSaisAddStatus_0001
 * @tc.name: SetSaisAddStatus
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SetSaisAddStatus_0001, TestSize.Level1)
{
    HILOGI("watch_service_SetSaisAddStatus_0001 start");

    SetStatus(BT_STATUS_SUCCESS);
    watchService_->isSaisAdded_ = true;
    watchService_->SetSaisAddStatus(false);
    EXPECT_TRUE(watchService_->isSaisAdded_);

    watchService_->isSaisAdded_ = false;
    watchService_->SetSaisAddStatus(true);
    EXPECT_TRUE(watchService_->isSaisAdded_);
    HILOGI("watch_service_SetSaisAddStatus_0001 end");
}

/*
 * @tc.number: watch_service_GetBondedPhone_0001
 * @tc.name: GetBondedPhone
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_GetBondedPhone_0001, TestSize.Level1)
{
    HILOGI("watch_service_GetBondedPhone_0001 start");

    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    updatePhoneMajorClass(false);
    watchService_->GetBondedPhone();
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isSaisDevice);

    updatePhoneMajorClass(true);
    AdapterConfig::GetInstance()->Load();
    AdapterManager::GetInstance()->CreateAdapters();
    watchService_->GetBondedPhone();
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isSaisDevice);

    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    remoteDevice->SetDeviceClass(0x0200);
    watchService_->GetBondedPhone();
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isSaisDevice);

    HILOGI("watch_service_GetBondedPhone_0001 no paired device start");
    remoteDevice->SetPairedStatus(PAIR_NONE);
    watchService_->GetBondedPhone();
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isSaisDevice);

    HILOGI("watch_service_GetBondedPhone_0001 end");
}

/*
 * @tc.number: watch_service_SetLinkLossRssi_0001
 * @tc.name: SetLinkLossRssi
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SetLinkLossRssi_0001, TestSize.Level1)
{
    HILOGI("watch_service_SetLinkLossRssi_0001 start");

    RawAddress device(address);
    RemoveDeviceManager(address);
    watchService_->SetLinkLossRssi(device, -20);
    EXPECT_NE(watchService_->rssiLinkLoss_, -20);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    AddDeviceManager(address);
    watchService_->UpdateSaisDevice(device, true);
    watchService_->SetLinkLossRssi(device, -45);
    EXPECT_EQ(watchService_->rssiLinkLoss_, -45);

    HILOGI("watch_service_SetLinkLossRssi_0001 end");
}

/*
 * @tc.number: watch_service_SetSaisConnectionState_0001
 * @tc.name: SetSaisConnectionState
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SetSaisConnectionState_0001, TestSize.Level1)
{
    HILOGI("watch_service_SetSaisConnectionState_0001 start");

    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->SetSaisConnectionState(device, false);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isWatchHealthConnected_);

    AddDeviceManager(address);
    watchService_->SetSaisConnectionState(device, false);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isWatchHealthConnected_);
    watchService_->SetSaisConnectionState(device, true);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isWatchHealthConnected_);

    HILOGI("watch_service_SetSaisConnectionState_0001 end");
}

/*
 * @tc.number: watch_service_UpdateSaisDevice_0001
 * @tc.name: UpdateSaisDevice
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_UpdateSaisDevice_0001, TestSize.Level1)
{
    HILOGI("watch_service_UpdateSaisDevice_0001 start");

    bool ret = false;
    RawAddress device(address);
    RawAddress device2(address2);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->UpdateSaisDevice(device, true);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isSaisDevice);
    watchService_->UpdateSaisDevice(device2, true);
    EXPECT_FALSE(GetDeviceManager(device2.GetAddress())->isSaisDevice);

    HILOGI("watch_service_UpdateSaisDevice_0001 end");
}

/*
 * @tc.number: watch_service_UpdateHFStrategy_0001
 * @tc.name: UpdateHFStrategy
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_UpdateHFStrategy_0001, TestSize.Level1)
{
    HILOGI("watch_service_UpdateHFStrategy_0001 start");

    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->UpdateHFStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    watchService_->UpdateHFStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    EXPECT_NE(watchService_->bluetoothWatchInterface, nullptr);

    HILOGI("watch_service_UpdateHFStrategy_0001 end");
}

/*
 * @tc.number: watch_service_SetHidConnectionState_0001
 * @tc.name: SetHidConnectionState
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SetHidConnectionState_0001, TestSize.Level1)
{
    HILOGI("watch_service_SetHidConnectionState_0001 start");

    RawAddress device(address);
    AddDeviceManager(address);
    watchService_->UpdateSaisDevice(device, true);
    watchService_->SetHidConnectionState(device, BTHD_CONN_STATE_CONNECTED);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    watchService_->CheckDisconnectHid();
    watchService_->SetHidConnectionState(device, BTHD_CONN_STATE_DISCONNECTED);
    EXPECT_FALSE(watchService_->isNeedConnectHid_);
    watchService_->SetHidConnectionState(device, BTHD_CONN_STATE_CONNECTING);
    EXPECT_FALSE(watchService_->isNeedConnectHid_);

    HILOGI("watch_service_SetHidConnectionState_0001 end");
}

/*
 * @tc.number: watch_service_ProcessRemovePair_0001
 * @tc.name: ProcessRemovePair
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ProcessRemovePair_0001, TestSize.Level1)
{
    HILOGI("watch_service_ProcessRemovePair_0001 start");

    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    AddDeviceManager(address);
    watchService_->UpdateSaisDevice(device, true);
    watchService_->ProcessRemovePair();
    EXPECT_NE(watchService_->bluetoothWatchInterface, nullptr);

    HILOGI("watch_service_ProcessRemovePair_0001 end");
}

/*
 * @tc.number: watch_service_ProcessAclStateChangeEvt_0001
 * @tc.name: ProcessAclStateChangeEvt
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ProcessAclStateChangeEvt_0001, TestSize.Level1)
{
    HILOGI("watch_service_ProcessAclStateChangeEvt_0001 start");

    RawAddress device(address);
    watchService_->ProcessAclStateChangeEvt(device, false);

    HILOGI("watch_service_ProcessAclStateChangeEvt_0001 end");
}

/*
 * @tc.number: watch_service_ProcessAclStateChangeInner_0001
 * @tc.name: ProcessAclStateChangeInner
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ProcessAclStateChangeInner_0001, TestSize.Level1)
{
    HILOGI("watch_service_ProcessAclStateChangeInner_0001 start");

    RawAddress device(address);
    updatePhoneMajorClass(false);
    watchService_->ProcessAclStateChangeInner(device, true);

    updatePhoneMajorClass(true);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    watchService_->UpdateSaisDevice(device, true);
    watchService_->SetHidConnectionState(device, BTHD_CONN_STATE_CONNECTED);
    watchService_->ProcessAclStateChangeInner(device, false);
    watchService_->ProcessAclStateChangeInner(device, true);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isSaisDevice);

    RawAddress device2(address2);
    watchService_->ProcessAclStateChangeInner(device2, true);

    HILOGI("watch_service_ProcessAclStateChangeInner_0001 end");
}

/*
 * @tc.number: watch_service_ProcessPairStateChangeEvt_0001
 * @tc.name: ProcessPairStateChangeEvt
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ProcessPairStateChangeEvt_0001, TestSize.Level1)
{
    HILOGI("watch_service_ProcessPairStateChangeEvt_0001 start");

    RawAddress device(address);
    watchService_->ProcessPairStateChangeEvt(ADAPTER_BREDR, device, PAIR_NONE);

    HILOGI("watch_service_ProcessPairStateChangeEvt_0001 end");
}

/*
 * @tc.number: watch_service_ProcessPairChangeInner_0001
 * @tc.name: ProcessPairChangeInner
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ProcessPairChangeInner_0001, TestSize.Level1)
{
    HILOGI("watch_service_ProcessPairChangeInner_0001 start");

    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    RawAddress device(address);
    watchService_->ProcessPairChangeInner(ADAPTER_BREDR, device, PAIR_NONE);

    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    remoteDevice->SetDeviceClass(0x0200);
    watchService_->ProcessPairChangeInner(ADAPTER_BREDR, device, PAIR_PAIRED);

    watchService_->ProcessPairChangeInner(ADAPTER_BREDR, device, PAIR_PAIRED);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isSaisDevice);

    AddDeviceManager(address);
    RawAddress device2(address2);
    std::shared_ptr<BluetoothDevice> remoteDevice2 = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device2);
    remoteDevice2->SetPairedStatus(PAIR_PAIRED);
    remoteDevice2->SetDeviceClass(0x0200);
    watchService_->ProcessPairChangeInner(ADAPTER_BREDR, device2, PAIR_PAIRED);

    HILOGI("watch_service_ProcessPairChangeInner_0001 end");
}

/*
 * @tc.number: watch_service_ProcessHfpStateChangeEvt_0001
 * @tc.name: ProcessHfpStateChangeEvt
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ProcessHfpStateChangeEvt_0001, TestSize.Level1)
{
    HILOGI("watch_service_ProcessHfpStateChangeEvt_0001 start");

    RawAddress device(address);
    RawAddress device2(address2);
    watchService_->ProcessHfpStateChangeEvt(device2, HFP_HF_STATE_DISCONNECTING);
    EXPECT_FALSE(GetDeviceManager(device2.GetAddress()));
    watchService_->ProcessHfpStateChangeEvt(device2, HFP_HF_STATE_CONNECTED);
    EXPECT_TRUE(GetDeviceManager(device2.GetAddress()));
    watchService_->SecondaryPhoneChange(SECONDARY_PHONE_PAIR);
    watchService_->ProcessHfpStateChangeEvt(device, HFP_HF_STATE_CONNECTED);
    EXPECT_TRUE(isSecondaryPair_);
    watchService_->SecondaryPhoneChange(SECONDARY_PHONE_UNPAIR);
    RemoveDeviceManager(address2);
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_DISCONNECTED;
    watchService_->ProcessHfpStateChangeEvt(device, HFP_HF_STATE_CONNECTED);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    AddDeviceManager(address);
    watchService_->UpdateSaisDevice(device, true);
    watchService_->ProcessHfpStateChangeEvt(device, HFP_HF_STATE_CONNECTED);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isJustConnectAcl_);

    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(APPLE_IAP_UUID));
    remoteDevice->SetDeviceUuids(remoteUuids);
    BluetoothConnectionManager::GetInstance()->RecordAclConnect(
        address, LINK_TYPE_BREDR, CONNECTION_STATE_CONNECTED, "");
    watchService_->ProcessHfpStateChangeEvt(device, HFP_HF_STATE_DISCONNECTED);

    HILOGI("watch_service_ProcessHfpStateChangeEvt_0001 end");
}

/*
 * @tc.number: watch_service_OnHfpHfStateChangedHook_0001
 * @tc.name: OnHfpHfStateChangedHook
 * @tc.desc: On first HFP HF connect (no device manager), the hook creates the device
 *           manager and marks SAIS, so GetBtCallPhoneIndexHook returns primary phone
 *           index 0; under secondary phone pair mode it returns 1.
 */
HWTEST_F(WatchServiceTest, watch_service_OnHfpHfStateChangedHook_0001, TestSize.Level1)
{
    HILOGI("watch_service_OnHfpHfStateChangedHook_0001 start");

    watchService_->SecondaryPhoneChange(SECONDARY_PHONE_UNPAIR);
    RemoveDeviceManager(address);
    RawAddress device(address);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress()));
    OnHfpHfStateChangedHook(device, HFP_HF_STATE_CONNECTED);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress()) != nullptr);
    EXPECT_EQ(GetBtCallPhoneIndexHook(device), 0);
    OnHfpHfStateChangedHook(device, HFP_HF_STATE_DISCONNECTED);

    EXPECT_EQ(GetBtCallPhoneIndexHook(RawAddress(address2)), 1);

    watchService_->SecondaryPhoneChange(SECONDARY_PHONE_PAIR);
    RemoveDeviceManager(address2);
    RawAddress device2(address2);
    OnHfpHfStateChangedHook(device2, HFP_HF_STATE_CONNECTED);
    EXPECT_TRUE(GetDeviceManager(device2.GetAddress()) != nullptr);
    EXPECT_EQ(GetBtCallPhoneIndexHook(device2), 1);
    OnHfpHfStateChangedHook(device2, HFP_HF_STATE_DISCONNECTED);
    watchService_->SecondaryPhoneChange(SECONDARY_PHONE_UNPAIR);

    HILOGI("watch_service_OnHfpHfStateChangedHook_0001 end");
}

/*
 * @tc.number: watch_service_DeviceDisconnectCallBack_0001
 * @tc.name: DeviceDisconnectCallBack
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_DeviceDisconnectCallBack_0001, TestSize.Level1)
{
    HILOGI("watch_service_DeviceDisconnectCallBack_0001 start");

    RawAddress device(address);
    watchService_->DeviceDisconnectCallBack(device, IS_PHONE_CREATE_CONNECT);

    AddDeviceManager(address);
    watchService_->DeviceDisconnectCallBack(device, PIN_OR_KEY_MISSING);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isPhoneCreateConnect_);

    watchService_->DeviceDisconnectCallBack(device, IS_PHONE_CREATE_CONNECT);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isPhoneCreateConnect_);


    HILOGI("watch_service_DeviceDisconnectCallBack_0001 end");
}

/*
 * @tc.number: watch_service_IsIphone_0001
 * @tc.name: IsIphone
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsIphone_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsIphone_0001 start");

    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteDevice->SetDeviceUuids(remoteUuids);
    EXPECT_FALSE(watchService_->IsIphone(device));

    remoteUuids.push_back(Uuid::ConvertFromString(APPLE_IAP_UUID));
    remoteDevice->SetDeviceUuids(remoteUuids);
    EXPECT_TRUE(watchService_->IsIphone(device));

    HILOGI("watch_service_IsIphone_0001 end");
}

/*
 * @tc.number: watch_service_PublishEventWithIntParam_0001
 * @tc.name: PublishEventWithIntParam
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_PublishEventWithIntParam_0001, TestSize.Level1)
{
    HILOGI("watch_service_PublishEventWithIntParam_0001 start");
    bool ret = false;
    ret = watchService_->PublishEventWithIntParam(EVENT_PHONE_CONNECT_STATE_CHANGE, address,
        PARAM_KEY_PHONE_CONNECTION_STATE, 1, true);
    EXPECT_FALSE(ret);
    AddDeviceManager(address);
    ret = watchService_->PublishEventWithIntParam(EVENT_PHONE_CONNECT_STATE_CHANGE, address,
        PARAM_KEY_PHONE_CONNECTION_STATE, 1, true);
    EXPECT_TRUE(ret);

    HILOGI("watch_service_PublishEventWithIntParam_0001 end");
}

/*
 * @tc.number: watch_service_IsPhoneOrComputer_0001
 * @tc.name: IsPhoneOrComputer
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsPhoneOrComputer_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsPhoneOrComputer_0001 start");

    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    updatePhoneMajorClass(false);
    EXPECT_FALSE(watchService_->IsPhoneOrComputer(device));

    HILOGI("watch_service_IsPhoneOrComputer_0001 end");
}

/*
 * @tc.number: watch_service_SendConnectionTimtout_0001
 * @tc.name: SendConnectionTimtout
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SendConnectionTimtout_0001, TestSize.Level1)
{
    HILOGI("watch_service_SendConnectionTimtout_0001 start");

    watchService_->SendConnectionTimtout();
    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    AddDeviceManager(address);
    watchService_->UpdateSaisDevice(device, true);
    watchService_->SendConnectionTimtout();
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isSaisDevice);

    HILOGI("watch_service_SendConnectionTimtout_0001 end");
}

/*
 * @tc.number: watch_service_ChangeScanMode_0001
 * @tc.name: ChangeScanMode
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ChangeScanMode_0001, TestSize.Level1)
{
    HILOGI("watch_service_ChangeScanMode_0001 start");
    watchService_->isBluetoothDisable_ = true;
    AdapterManager::GetInstance()->GetClassicAdapter()->SetBtScanMode(SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE, 0);
    watchService_->ChangeScanMode();
    EXPECT_EQ(AdapterManager::GetInstance()->GetClassicAdapter()->GetBtScanMode(), SCAN_MODE_NONE);

    watchService_->isBluetoothDisable_ = false;
    watchService_->ChangeScanMode();

    AddDeviceManager(address);
    AdapterManager::GetInstance()->GetClassicAdapter()->SetBtScanMode(SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE, 0);
    watchService_->ChangeScanMode();
    EXPECT_EQ(AdapterManager::GetInstance()->GetClassicAdapter()->GetBtScanMode(),
        SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE);

    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    BluetoothConnectionManager::GetInstance()->RecordAclConnect(
        address, LINK_TYPE_BREDR, CONNECTION_STATE_CONNECTED, "");

    RawAddress device2(address2);
    std::shared_ptr<BluetoothDevice> remoteDevice2 = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device2);
    remoteDevice2->SetPairedStatus(PAIR_PAIRED);
    BluetoothConnectionManager::GetInstance()->RecordAclConnect(
        address2, LINK_TYPE_BREDR, CONNECTION_STATE_CONNECTED, "");
    AdapterManager::GetInstance()->GetClassicAdapter()->SetBtScanMode(BT_SCAN_MODE_CONNECTABLE, 0);
    watchService_->ChangeScanMode();
    EXPECT_EQ(AdapterManager::GetInstance()->GetClassicAdapter()->GetBtScanMode(), SCAN_MODE_NONE);

    BluetoothConnectionManager::GetInstance()->RecordAclConnect(
        address2, LINK_TYPE_BREDR, CONNECTION_STATE_DISCONNECTED, "");
    watchService_->ChangeScanMode();
    EXPECT_EQ(AdapterManager::GetInstance()->GetClassicAdapter()->GetBtScanMode(), BT_SCAN_MODE_CONNECTABLE);
    remoteDevice->SetPairedStatus(PAIR_NONE);
    remoteDevice2->SetPairedStatus(PAIR_NONE);

    HILOGI("watch_service_ChangeScanMode_0001 end");
}

/*
 * @tc.number: watch_service_ConnectBondedPhone_0001
 * @tc.name: ConnectBondedPhone
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ConnectBondedPhone_0001, TestSize.Level1)
{
    HILOGI("watch_service_ConnectBondedPhone_0001 start");

    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    AddDeviceManager(address);
    watchService_->ConnectBondedPhone();
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isConnected);

    HILOGI("watch_service_ConnectBondedPhone_0001 end");
}

/*
 * @tc.number: watch_service_ConnectHfp_0001
 * @tc.name: ConnectHfp
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ConnectHfp_0001, TestSize.Level1)
{
    HILOGI("watch_service_ConnectHfp_0001 start");

    RawAddress device(address);
    EXPECT_TRUE(watchService_->ConnectHfp(address2));
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HFP_CLIENT_CONNECTION_POLICY,
        static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    AddDeviceManager(address);
    EXPECT_TRUE(watchService_->ConnectHfp(address));

    ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HFP_CLIENT_CONNECTION_POLICY,
        static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    EXPECT_TRUE(watchService_->ConnectHfp(address));

    HILOGI("watch_service_ConnectHfp_0001 end");
}

/*
 * @tc.number: StartDisconnectHfpOnDemandTimer
 * @tc.name: StartDisconnectHfpOnDemandTimer
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_StartDisconnectHfpOnDemandTimer_0001, TestSize.Level1)
{
    HILOGI("watch_service_StartDisconnectHfpOnDemandTimer_0001 start");

    watchService_->StartDisconnectHfpOnDemandTimer(address);
    watchService_->StopDisconnectHfpOnDemandTimer();

    HILOGI("watch_service_StartDisconnectHfpOnDemandTimer_0001 end");
}

/*
 * @tc.number: watch_service_DisconnectHfpOnDemand_0001
 * @tc.name: DisconnectHfpOnDemand
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_DisconnectHfpOnDemand_0001, TestSize.Level1)
{
    HILOGI("watch_service_DisconnectHfpOnDemand_0001 start");

    RawAddress device(address);
    watchService_->DisconnectHfpOnDemand(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(APPLE_IAP_UUID));
    remoteDevice->SetDeviceUuids(remoteUuids);
    watchService_->DisconnectHfpOnDemand(address);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isOnHfCall_);

    watchService_->DisconnectHfpOnDemand(address);

    HILOGI("watch_service_DisconnectHfpOnDemand_0001 end");
}

/*
 * @tc.number: watch_service_ConnectHfpOnDemand_0001
 * @tc.name: ConnectHfpOnDemand
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ConnectHfpOnDemand_0001, TestSize.Level1)
{
    HILOGI("watch_service_ConnectHfpOnDemand_0001 start");

    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HFP_CLIENT_CONNECTION_POLICY,
        static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    watchService_->ConnectHfpOnDemand(address);

    AddDeviceManager(address);
    ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HFP_CLIENT_CONNECTION_POLICY,
        static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));

    watchService_->ConnectHfpOnDemand(address);

    HILOGI("watch_service_ConnectHfpOnDemand_0001 end");
}

/*
 * @tc.number: watch_service_SetCallID_0001
 * @tc.name: SetCallID
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SetCallID_0001, TestSize.Level1)
{
    HILOGI("watch_service_SetCallID_0001 start");

    RawAddress device(address);
    watchService_->SetCallID(address, 0);
    EXPECT_EQ(0, GetDeviceManager(device.GetAddress())->callID_);

    HILOGI("watch_service_SetCallID_0001 end");
}

/*
 * @tc.number: watch_service_IsAllowHfScoConnect_0001
 * @tc.name: IsAllowHfScoConnect
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsAllowHfScoConnect_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsAllowHfScoConnect_0001 start");

    EXPECT_FALSE(watchService_->IsAllowHfScoConnect(address));

    watchService_->HandleVoipState(VOIP_CALL_ACCEPT);
    EXPECT_FALSE(watchService_->IsAllowHfScoConnect(address));

    watchService_->SetIsHfOnCall(address, true);
    EXPECT_FALSE(watchService_->IsAllowHfScoConnect(address));

    watchService_->SetAllowHfScoConnect(address, true);
    EXPECT_TRUE(watchService_->IsAllowHfScoConnect(address));

    watchService_->SetAllowHfScoConnect(address, false);
    EXPECT_FALSE(watchService_->IsAllowHfScoConnect(address));

    HILOGI("watch_service_IsAllowHfScoConnect_0001 end");
}

/*
 * @tc.number: watch_service_IsAllowAgCall_0001
 * @tc.name: IsAllowAgCall
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsAllowAgCall_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsAllowAgCall_0001 start");

    watchService_->SetIsHfOnCall(address, true);
    EXPECT_FALSE(watchService_->IsAllowAgCall());
    watchService_->SetIsHfOnCall(address, false);
    EXPECT_TRUE(watchService_->IsAllowAgCall());

    HILOGI("watch_service_IsAllowAgCall_0001 end");
}

/*
 * @tc.number: watch_service_SetDialedCallNumber_0001
 * @tc.name: SetDialedCallNumber
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SetDialedCallNumber_0001, TestSize.Level1)
{
    HILOGI("watch_service_SetDialedCallNumber_0001 start");

    std::string dialNumber = "10086";
    watchService_->SetDialedCallNumber("10086");
    EXPECT_TRUE(watchService_->IsNeedDialCall());

    EXPECT_EQ(watchService_->GetDialedCallNumber(), dialNumber);

    watchService_->DialCallFinished();
    EXPECT_FALSE(watchService_->IsNeedDialCall());

    HILOGI("watch_service_SetDialedCallNumber_0001 end");
}

/*
 * @tc.number: watch_service_SetDialingByWatch_0001
 * @tc.name: SetDialingByWatch
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SetDialingByWatch_0001, TestSize.Level1)
{
    HILOGI("watch_service_SetDialingByWatch_0001 start");

    watchService_->SetDialingByWatch(address);
    EXPECT_TRUE(watchService_->IsDialingByWatch(address));

    HILOGI("watch_service_SetDialingByWatch_0001 end");
}

/*
 * @tc.number: watch_service_IsHfOnCall_0001
 * @tc.name: IsHfOnCall
 * @tc.desc: 设备管理器不存在返回 false；HF 通话状态决定返回值
 */
HWTEST_F(WatchServiceTest, watch_service_IsHfOnCall_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsHfOnCall_0001 start");
    RemoveDeviceManager(address);
    EXPECT_FALSE(watchService_->IsHfOnCall(address));
    AddDeviceManager(address);
    auto deviceManager = GetDeviceManager(address);
    ASSERT_NE(deviceManager, nullptr);
    deviceManager->SetIsHfOnCallLocked(false);
    deviceManager->HandleVoipStateLocked(UNKNOWN_VOIP_STATE);
    EXPECT_FALSE(watchService_->IsHfOnCall(address));
    watchService_->SetIsHfOnCall(address, true);
    EXPECT_TRUE(watchService_->IsHfOnCall(address));
    watchService_->SetIsHfOnCall(address, false);
    EXPECT_FALSE(watchService_->IsHfOnCall(address));
    HILOGI("watch_service_IsHfOnCall_0001 end");
}
 
/*
 * @tc.number: watch_service_IsHfOnCall_0002
 * @tc.name: IsHfOnCall
 * @tc.desc: VoIP 通话中（非 HF 通话）也视为通话中；VoIP 与 HF 通话任一在
 *           通话中即返回 true，全部结束返回 false
 */
HWTEST_F(WatchServiceTest, watch_service_IsHfOnCall_0002, TestSize.Level1)
{
    HILOGI("watch_service_IsHfOnCall_0002 start");
    AddDeviceManager(address);
    auto deviceManager = GetDeviceManager(address);
    ASSERT_NE(deviceManager, nullptr);
    deviceManager->SetIsHfOnCallLocked(false);
    deviceManager->HandleVoipStateLocked(UNKNOWN_VOIP_STATE);
    EXPECT_FALSE(watchService_->IsHfOnCall(address));
    deviceManager->HandleVoipStateLocked(VOIP_CALL_INCOMMING);
    EXPECT_TRUE(watchService_->IsHfOnCall(address));
    watchService_->SetIsHfOnCall(address, true);
    deviceManager->HandleVoipStateLocked(VOIP_CALL_INCOMMING);
    EXPECT_TRUE(watchService_->IsHfOnCall(address));
    watchService_->SetIsHfOnCall(address, false);
    deviceManager->HandleVoipStateLocked(UNKNOWN_VOIP_STATE);
    EXPECT_FALSE(watchService_->IsHfOnCall(address));
    HILOGI("watch_service_IsHfOnCall_0002 end");
}

/*
 * @tc.number: watch_service_ProcessCallStateChangeEvt_0001
 * @tc.name: ProcessCallStateChangeEvt
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ProcessCallStateChangeEvt_0001, TestSize.Level1)
{
    HILOGI("watch_service_ProcessCallStateChangeEvt_0001 start");

    RawAddress device(address);
    HandsFreeUnitCalls call1(address, 0, HFP_CALL_STATE_ACTIVE, "10086", false, false, true);
    watchService_->ProcessCallStateChangeEvt(address, call1);
    watchService_->SetCallID(address, 0);
    watchService_->ProcessCallStateChangeEvt(address, call1);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isRouteScoRunning_);
    watchService_->StopControlScoConnectTimer(address);

    HandsFreeUnitCalls call2(address, 1, HFP_CALL_STATE_FINISHED, "10086", false, false, true);
    watchService_->SetCallID(address, 1);
    watchService_->SetIsHfOnCall(address, true);
    watchService_->ProcessCallStateChangeEvt(address, call2);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isRouteScoRunning_);
    EXPECT_TRUE(watchService_->IsAllowHfScoConnect(address));
    watchService_->ProcessCallStateChangeEvt(address, call1);
    watchService_->StopControlScoConnectTimer(address);

    HILOGI("watch_service_ProcessCallStateChangeEvt_0001 end");
}

/*
 * @tc.number: watch_service_ProcessCallStateChangeEvt_0002
 * @tc.name: ProcessCallStateChangeEvt
 * @tc.desc: 拨号（DIALING）与接通（ACTIVE）场景也经 PBAP 查询联系人；
 *           同一通话（同号码）仅查询一次；通话全部结束后同一号码的新通话再次查询
 */
HWTEST_F(WatchServiceTest, watch_service_ProcessCallStateChangeEvt_0002, TestSize.Level1)
{
    HILOGI("watch_service_ProcessCallStateChangeEvt_0002 start");

    AddDeviceManager(address);
    auto deviceManager = GetDeviceManager(address);
    ASSERT_NE(deviceManager, nullptr);
    deviceManager->ClearContactQueriedLocked();
    watchService_->SetCallID(address, 1);

    auto client = std::make_shared<WatchPbapClient>(address);
    watchService_->watchPbapClient_ = client;
    {
        // 模拟 worker 存活：入队行为可确定性观测
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        client->running_ = true;
    }

    // scenario 1: 拨号场景 → 查询任务入队
    HandsFreeUnitCalls dialing(address, 1, HFP_CALL_STATE_DIALING, "13800138000", false, true, false);
    watchService_->ProcessCallStateChangeEvt(address, dialing);
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_EQ(static_cast<size_t>(1), client->taskQueue_.size());
    }

    // scenario 2: 同一通话 ALERTING / ACTIVE → 不重复查询
    HandsFreeUnitCalls alerting(address, 1, HFP_CALL_STATE_ALERTING, "13800138000", false, true, false);
    watchService_->ProcessCallStateChangeEvt(address, alerting);
    HandsFreeUnitCalls active(address, 1, HFP_CALL_STATE_ACTIVE, "13800138000", false, false, true);
    watchService_->ProcessCallStateChangeEvt(address, active);
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_EQ(static_cast<size_t>(1), client->taskQueue_.size());
    }

    // scenario 3: 通话全部结束 → 去重记录清空，同号码新通话再次查询
    watchService_->AllHfCallFinished(address, false);
    watchService_->SetCallID(address, 2);
    HandsFreeUnitCalls incoming(address, 2, HFP_CALL_STATE_INCOMING, "13800138000", false, false, true);
    watchService_->ProcessCallStateChangeEvt(address, incoming);
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_EQ(static_cast<size_t>(2), client->taskQueue_.size());
        client->running_ = false;
        client->taskQueue_.clear();
    }
    watchService_->watchPbapClient_ = nullptr;

    HILOGI("watch_service_ProcessCallStateChangeEvt_0002 end");
}

/*
 * @tc.number: watch_service_StartDialHook_0001
 * @tc.name: StartDialHook
 * @tc.desc: HFP 已连接时手表拨向副手机在 hook 处直接查询；同号码后续 DIALING
 *           状态事件不重复查询；拨向 sais 主手机不查询；未连接的延迟拨号分支不查询
 */
HWTEST_F(WatchServiceTest, watch_service_StartDialHook_0001, TestSize.Level1)
{
    HILOGI("watch_service_StartDialHook_0001 start");

    AddDeviceManager(address);
    auto deviceManager = GetDeviceManager(address);
    ASSERT_NE(deviceManager, nullptr);
    deviceManager->ClearContactQueriedLocked();

    auto client = std::make_shared<WatchPbapClient>(address);
    watchService_->watchPbapClient_ = client;
    {
        // 模拟 worker 存活：入队行为可确定性观测
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        client->running_ = true;
    }

    // scenario 1: HFP 已连接，拨向副手机 → hook 处查询任务入队
    EXPECT_TRUE(StartDialHook(address, HFP_HF_STATE_CONNECTED, "13800138000"));
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_EQ(static_cast<size_t>(1), client->taskQueue_.size());
    }

    // scenario 2: 同号码 DIALING 状态事件（占位 id 与真实 id 均被抑制）
    watchService_->SetCallID(address, 1);
    HandsFreeUnitCalls dialing(address, 1, HFP_CALL_STATE_DIALING, "13800138000", false, true, false);
    watchService_->ProcessCallStateChangeEvt(address, dialing);
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_EQ(static_cast<size_t>(1), client->taskQueue_.size());
    }

    // scenario 3: 拨向 sais 主手机 → 不查询
    deviceManager->SetIsSaisDeviceLocked(true);
    EXPECT_TRUE(StartDialHook(address, HFP_HF_STATE_CONNECTED, "13900139000"));
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_EQ(static_cast<size_t>(1), client->taskQueue_.size());
    }
    deviceManager->SetIsSaisDeviceLocked(false);

    // scenario 4: 未连接的延迟拨号分支（return false）→ 不查询
    EXPECT_FALSE(StartDialHook(address, HFP_HF_STATE_CONNECTING, "13700137000"));
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_EQ(static_cast<size_t>(1), client->taskQueue_.size());
    }

    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        client->running_ = false;
        client->taskQueue_.clear();
    }
    watchService_->watchPbapClient_ = nullptr;
    watchService_->DialCallFinished();
    watchService_->AllHfCallFinished(address, false);

    HILOGI("watch_service_StartDialHook_0001 end");
}

/*
 * @tc.number: watch_service_HandleVoipState_0001
 * @tc.name: HandleVoipState
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_HandleVoipState_0001, TestSize.Level1)
{
    HILOGI("watch_service_HandleVoipState_0001 start");
    RawAddress device(address);
    watchService_->HandleVoipState(UNKNOWN_VOIP_STATE);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->HandleVoipState(VOIP_CALL_INCOMMING);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isOnVoip_);

    watchService_->HandleVoipState(VOIP_CALL_ACCEPT);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isAcceptVoipByWatch);

    watchService_->HandleVoipState(UNKNOWN_VOIP_STATE);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isAcceptVoipByWatch);

    HILOGI("watch_service_HandleVoipState_0001 end");
}

/*
 * @tc.number: watch_service_SendPhoneConnectStateChange_0001
 * @tc.name: SendPhoneConnectStateChange
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SendPhoneConnectStateChange_0001, TestSize.Level1)
{
    HILOGI("watch_service_SendPhoneConnectStateChange_0001 start");

    RawAddress device(address);
    watchService_->SendPhoneConnectStateChange(device, false);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isPhoneConnected_);

    watchService_->SendPhoneConnectStateChange(device, true);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isPhoneConnected_);

    HILOGI("watch_service_SendPhoneConnectStateChange_0001 end");
}

/*
 * @tc.number: watch_service_ConnectHfpTimer_0001
 * @tc.name: ConnectHfpTimer
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ConnectHfpTimer_0001, TestSize.Level1)
{
    HILOGI("watch_service_ConnectHfpTimer_0001 start");

    RawAddress device(address);
    watchService_->StartConnectHfpTimer(address, 1000);
    watchService_->StartConnectHfpTimer(address, 2000);
    EXPECT_NE(nullptr, GetDeviceManager(device.GetAddress())->connectHfpTimer_);
    watchService_->StopConnectHfpTimer(address);

    HILOGI("watch_service_ConnectHfpTimer_0001 end");
}

/*
 * @tc.number: watch_service_ConnectHfpOnDemandTimer_0001
 * @tc.name: ConnectHfpOnDemandTimer
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ConnectHfpOnDemandTimer_0001, TestSize.Level1)
{
    HILOGI("watch_service_ConnectHfpOnDemandTimer_0001 start");

    RawAddress device(address);
    watchService_->StartConnectHfpOnDemandTimer(address);
    watchService_->StopConnectHfpOnDemandTimer(address);

    HILOGI("watch_service_ConnectHfpOnDemandTimer_0001 end");
}

/*
 * @tc.number: watch_service_ControlScoConnectTimer_0001
 * @tc.name: ControlScoConnectTimer
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ControlScoConnectTimer_0001, TestSize.Level1)
{
    HILOGI("watch_service_ControlScoConnectTimer_0001 start");

    RawAddress device(address);
    watchService_->StartControlScoConnectTimer(address);
    EXPECT_NE(nullptr, GetDeviceManager(device.GetAddress())->controlScoConnectTimer_);
    watchService_->StopControlScoConnectTimer(address);
    EXPECT_EQ(nullptr, GetDeviceManager(device.GetAddress())->controlScoConnectTimer_);

    HILOGI("watch_service_ControlScoConnectTimer_0001 end");
}

/*
 * @tc.number: watch_service_SubscribeWatchCommonEvent_0001
 * @tc.name: SubscribeWatchCommonEvent
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SubscribeWatchCommonEvent_0001, TestSize.Level1)
{
    HILOGI("watch_service_SubscribeWatchCommonEvent_0001 start");

    watchService_->SubscribeWatchCommonEvent();

    watchService_->SubscribeWatchCommonEvent();
    if (watchService_->subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(watchService_->subscriber_);
        watchService_->subscriber_ = nullptr;
    }

    HILOGI("watch_service_SubscribeWatchCommonEvent_0001 end");
}

/*
 * @tc.number: watch_service_SubscribeWatchEnterStrEvent_0001
 * @tc.name: SubscribeWatchEnterStrEvent
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SubscribeWatchEnterStrEvent_0001, TestSize.Level1)
{
    HILOGI("watch_service_SubscribeWatchEnterStrEvent_0001 start");

    watchService_->SubscribeWatchEnterStrEvent();

    watchService_->SubscribeWatchEnterStrEvent();
    if (watchService_->subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(watchService_->subscriber_);
        watchService_->subscriber_ = nullptr;
    }

    HILOGI("watch_service_SubscribeWatchEnterStrEvent_0001 end");
}

/*
 * @tc.number: watch_service_OnReceiveEvent_0001
 * @tc.name: OnReceiveEvent
 * @tc.desc: OnReceiveEvent EVENT_BLUETOOTH_VOIP_STATE
*/
HWTEST_F(WatchServiceTest, watch_service_OnReceiveEvent_0001, TestSize.Level1)
{
    HILOGI("watch_service_OnReceiveEvent_0001 start");

    EventFwk::CommonEventData eventData;
    std::string action = "EventFwk::CommonEventSupport::EVENT_BLUETOOTH_VOIP_STATE";
    AAFwk::Want want;
    want.SetAction(action);
    eventData.SetWant(want);
    OHOS::EventFwk::CommonEventSubscribeInfo eventSubscribeInfo;
    std::shared_ptr<BluetoothWatchServiceSubscriber> Subscriber =
        std::make_shared<BluetoothWatchServiceSubscriber>(eventSubscribeInfo);
    ASSERT_NE(nullptr, Subscriber);
    Subscriber->OnReceiveEvent(eventData);

    HILOGI("watch_service_OnReceiveEvent_0001 end");
}

/*
 * @tc.number: watch_service_OnReceiveEvent_0002
 * @tc.name: OnReceiveEvent
 * @tc.desc: OnReceiveEvent EVENT_BLUETOOTH_HOST_CONNECT_HFP
*/
HWTEST_F(WatchServiceTest, watch_service_OnReceiveEvent_0002, TestSize.Level1)
{
    HILOGI("watch_service_OnReceiveEvent_0002 start");

    EventFwk::CommonEventData eventData;
    std::string action = "EventFwk::CommonEventSupport::EVENT_BLUETOOTH_HOST_CONNECT_HFP";
    AAFwk::Want want;
    want.SetAction(action);
    eventData.SetWant(want);
    OHOS::EventFwk::CommonEventSubscribeInfo eventSubscribeInfo;
    std::shared_ptr<BluetoothWatchServiceSubscriber> Subscriber =
        std::make_shared<BluetoothWatchServiceSubscriber>(eventSubscribeInfo);
    ASSERT_NE(nullptr, Subscriber);

    bool ret = false;
    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    ret = AddDeviceManager(address);
    EXPECT_FALSE(ret);
    watchService_->UpdateSaisDevice(device, true);
    Subscriber->OnReceiveEvent(eventData);

    HILOGI("watch_service_OnReceiveEvent_0002 end");
}

/*
 * @tc.number: watch_service_OnReceiveEvent_0002
 * @tc.name: OnReceiveEvent
 * @tc.desc: OnReceiveEvent EVENT_BLUETOOTH_HOST_CONNECT_HFP
*/
HWTEST_F(WatchServiceTest, watch_service_BluetoothDataShareUpdate_0001, TestSize.Level1)
{
    HILOGI("watch_service_BluetoothDataShareUpdate_0001 start");
    std::string in_value = "1";
    bool ret = watchService_->BluetoothDataShareUpdate(STATUS_SETTING_MODE_URI, nullptr, in_value);
    EXPECT_FALSE(ret);
    ret = watchService_->BluetoothDataShareUpdate(STATUS_SETTING_MODE_URI, CONTROL_POINT_KEY, in_value);
    EXPECT_TRUE(ret);
    HILOGI("watch_service_BluetoothDataShareUpdate_0001 end");
}

/*
 * @tc.number: watch_service_WatchSetUnConnectable_0001
 * @tc.name: WatchSetUnConnectable
 * @tc.desc: WatchSetUnConnectable reset scan mode
*/
HWTEST_F(WatchServiceTest, watch_service_WatchSetUnConnectable_0001, TestSize.Level1)
{
    HILOGI("watch_service_WatchSetUnConnectable_0001 start");
    AdapterManager::GetInstance()->GetClassicAdapter()->SetBtScanMode(BT_SCAN_MODE_CONNECTABLE, 0);
    watchService_->WatchSetUnConnectable();
    EXPECT_EQ(AdapterManager::GetInstance()->GetClassicAdapter()->GetBtScanMode(), SCAN_MODE_NONE);
    HILOGI("watch_service_WatchSetUnConnectable_0001 end");
}

/*
 * @tc.number: watch_service_SaisAddedTimer_0001
 * @tc.name: SaisAddedTime
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SaisAddedTimer_0001, TestSize.Level1)
{
    HILOGI("watch_service_SaisAddedTimer_0001 start");

    watchService_->StartSaisAddedTimer();
    EXPECT_EQ(nullptr, watchService_->saisAddedTimer_);
    watchService_->StartSaisAddedTimer();
    EXPECT_NE(nullptr, watchService_->saisAddedTimer_);
    watchService_->StartSaisAddedTimer();
    EXPECT_NE(nullptr, watchService_->saisAddedTimer_);
    watchService_->StopSaisAddedTimer();
    EXPECT_EQ(nullptr, watchService_->saisAddedTimer_);
    watchService_->StopSaisAddedTimer();
    EXPECT_EQ(nullptr, watchService_->saisAddedTimer_);

    HILOGI("watch_service_SaisAddedTimer_0001 end");
}

/*
 * @tc.number: watch_service_SaisAddedTimeout_0001
 * @tc.name: SaisAddedTimeout
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SaisAddedTimeout_0001, TestSize.Level1)
{
    HILOGI("watch_service_SaisAddedTimeout_0001 start");
    RawAddress device(address);

    // scenario 1: no sais device, SaisAddedTimeout should not connect hfp
    RemoveDeviceManager(address);
    watchService_->SaisAddedTimeout();

    // scenario 2: sais device is connected, SaisAddedTimeout should not connect hfp
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    AddDeviceManager(address);
    auto deviceManager = GetDeviceManager(device.GetAddress());
    ASSERT_NE(nullptr, deviceManager);
    watchService_->UpdateSaisDevice(device, true);
    deviceManager->SetIsConnectedLocked(true);
    watchService_->StartConnectHfpTimer(address, 10000);
    EXPECT_NE(nullptr, deviceManager->connectHfpTimer_);
    watchService_->SaisAddedTimeout();
    EXPECT_NE(nullptr, deviceManager->connectHfpTimer_);

    // scenario 3: sais device is disconnected, SaisAddedTimeout should connect hfp and stop connect hfp timer
    deviceManager->SetIsConnectedLocked(false);
    watchService_->SaisAddedTimeout();
    EXPECT_EQ(nullptr, deviceManager->connectHfpTimer_);
    HILOGI("watch_service_SaisAddedTimeout_0001 end");
}

/*
 * @tc.number: watch_service_IsWatchStrMode_0001
 * @tc.name: IsWatchStrMode
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsWatchStrMode_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsWatchStrMode_0001 start");
    watchService_->IsWatchStrMode();
    HILOGI("watch_service_IsWatchStrMode_0001 end");
}

/*
 * @tc.number: watch_service_ChangeStatusExitStrMode_0001
 * @tc.name: ChangeStatusExitStrMode
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ChangeStatusExitStrMode_0001, TestSize.Level1)
{
    HILOGI("watch_service_QueryWatchMode_0001 start");

    system::SetParameter("persist.bluetooth.watch_half_property", "1");
    watchService_->ChangeStatusExitStrMode();
    int value = system::GetIntParameter("persist.bluetooth.watch_half_property", 0);
    EXPECT_EQ(0, value);
    system::SetParameter("persist.bluetooth.watch_half_property", "0");

    HILOGI("watch_service_ChangeStatusExitStrMode_0001 end");
}

/*
 * @tc.number: watch_service_PromptUnloadBluetoothSystemAbility_0001
 * @tc.name: PromptUnloadBluetoothSystemAbility
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_PromptUnloadBluetoothSystemAbility_0001, TestSize.Level1)
{
    HILOGI("watch_service_PromptUnloadBluetoothSystemAbility_0001 start");

    watchService_->PromptUnloadBluetoothSystemAbility();

    HILOGI("watch_service_PromptUnloadBluetoothSystemAbility_0001 end");
}

/*
 * @tc.number: watch_service_StrModeAbnormalExitHandle_0001
 * @tc.name: StrModeAbnormalExitHandle
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_StrModeAbnormalExitHandle_0001, TestSize.Level1)
{
    HILOGI("watch_service_StrModeAbnormalExitHandle_0001 start");

    int ret = watchService_->StrModeAbnormalExitHandle(ADAPTER_BLE);
    EXPECT_EQ(BT_STATUS_SUCCESS, ret);

    system::SetParameter("persist.watch_str_mode", "1");
    ret = watchService_->StrModeAbnormalExitHandle(ADAPTER_BLE);
    EXPECT_EQ(BT_STATUS_FAIL, ret);
    system::SetParameter("persist.watch_str_mode", "0");

    HILOGI("watch_service_StrModeAbnormalExitHandle_0001 end");
}

/*
 * @tc.number: watch_service_HoldRunningLock_0001
 * @tc.name: HoldRunningLock
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_HoldRunningLock_0001, TestSize.Level1)
{
    HILOGI("watch_service_HoldRunningLock_0001 start");

    const uint16_t timeoutMs = 1000;
    watchService_->HoldRunningLock(timeoutMs);
    EXPECT_NE(nullptr, watchService_->runningLock_);
    watchService_->ReleaseRunningLock();
    EXPECT_EQ(nullptr, watchService_->runningLock_);

    HILOGI("watch_service_HoldRunningLock_0001 end");
}

/*
 * @tc.number: watch_service_UnSubscribeWatchEnterStrEvent_0001
 * @tc.name: UnSubscribeWatchEnterStrEvent
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_UnSubscribeWatchEnterStrEvent_0001, TestSize.Level1)
{
    HILOGI("watch_service_UnSubscribeWatchEnterStrEvent_0001 start");

    watchService_->SubscribeWatchEnterStrEvent();
    EXPECT_NE(nullptr, watchService_->strSubscriber_);
    watchService_->UnSubscribeWatchEnterStrEvent();
    EXPECT_EQ(nullptr, watchService_->strSubscriber_);
    sleep(TIME);

    HILOGI("watch_service_UnSubscribeWatchEnterStrEvent_0001 end");
}

/**
 * @tc.name: watch_service_UpdataHighPowerMode_0001
 * @tc.desc: UpdataHighPowerMode:
 * @tc.type: FUNC
 */
HWTEST_F(WatchServiceTest, watch_service_UpdataHighPowerMode_0001, TestSize.Level1)
{
    HILOGI("watch_service_UpdataHighPowerMode_0001 enter");

    watchService_->Start();
    watchService_->lastHighPowerEnable_ = BT_HIGH_POWER_LEVEL7;
    watchService_->UpdataHighPowerMode(BT_HIGH_POWER_LEVEL7);
    EXPECT_EQ(watchService_->lastHighPowerEnable_, BT_HIGH_POWER_LEVEL7);
    watchService_->UpdataHighPowerMode(true);

    HILOGI("watch_service_UpdataHighPowerMode_0001 end");
}

/*
 * @tc.number: watch_service_NotifyBluetoothTurningOff_0001
 * @tc.name: NotifyBluetoothTurningOff
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_NotifyBluetoothTurningOff_0001, TestSize.Level1)
{
    HILOGI("watch_service_NotifyBluetoothTurningOff_0001 start");

    watchService_->Initialize();
    watchService_->NotifyBluetoothTurningOff();
    watchService_->Uninitialize();

    HILOGI("watch_service_NotifyBluetoothTurningOff_0001 end");
}

/*
 * @tc.number: watch_service_IsAllowSystemPairDialog_0001
 * @tc.name: IsAllowSystemPairDialog
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsAllowSystemPairDialog_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsAllowSystemPairDialog_0001 start");
    RawAddress device(address);
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    updatePhoneMajorClass(true);
    EXPECT_FALSE(watchService_->IsAllowSystemPairDialog(device));

    updatePhoneMajorClass(false);
    EXPECT_FALSE(watchService_->IsAllowSystemPairDialog(device));

    watchService_->SetPhoneConnHfp(address, false);
    EXPECT_FALSE(watchService_->IsAllowSystemPairDialog(device));

    sleep(TIME);

    HILOGI("watch_service_IsAllowSystemPairDialog_0001 end");
}

/*
 * @tc.number: watch_service_HidReconnectAfterUpdateMap_0001
 * @tc.name: HidReconnectAfterUpdateMap
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_HidReconnectAfterUpdateMap_0001, TestSize.Level1)
{
    HILOGI("watch_service_HidReconnectAfterUpdateMap_0001 start");

    RawAddress device(address);
    watchService_->HidReconnectAfterUpdateMap(device, true);
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_DISCONNECTED;
    EXPECT_EQ(1, watchService_->brHidReconnectCount_);
    watchService_->StopHidReconnectTimer();

    HILOGI("watch_service_HidReconnectAfterUpdateMap_0001 end");
}

/*
 * @tc.number: watch_service_HidVolumeIncrement_0001
 * @tc.name: HidVolumeIncrement
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_HidVolumeIncrement_0001, TestSize.Level1)
{
    HILOGI("watch_service_HidVolumeIncrement_0001 start");

    watchService_->HidVolumeIncrement();
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    watchService_->HidVolumeIncrement();
    EXPECT_NE(BTHD_CONN_STATE_CONNECTED, watchService_->hidConnectionState_);
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_CONNECTED;
    watchService_->HidVolumeIncrement();
    EXPECT_EQ(BTHD_CONN_STATE_CONNECTED, watchService_->hidConnectionState_);
    updateSendReportResult(BT_STATUS_SUCCESS);
    watchService_->HidVolumeIncrement();
    watchService_->Stop();

    HILOGI("watch_service_HidVolumeIncrement_0001 end");
}

/*
 * @tc.number: watch_service_HidVolumeDecrement_0001
 * @tc.name: HidVolumeDecrement
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_HidVolumeDecrement_0001, TestSize.Level1)
{
    HILOGI("watch_service_HidVolumeDecrement_0001 start");

    watchService_->HidVolumeDecrement();
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_DISCONNECTED;
    watchService_->HidVolumeDecrement();
    EXPECT_NE(BTHD_CONN_STATE_CONNECTED, watchService_->hidConnectionState_);
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_CONNECTED;
    watchService_->HidVolumeDecrement();
    EXPECT_EQ(BTHD_CONN_STATE_CONNECTED, watchService_->hidConnectionState_);
    updateSendReportResult(BT_STATUS_FAIL);
    watchService_->HidVolumeDecrement();
    watchService_->Stop();

    HILOGI("watch_service_HidVolumeDecrement_0001 end");
}

/*
 * @tc.number: watch_service_HidDoubleClick_0001
 * @tc.name: HidDoubleClick
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_HidDoubleClick_0001, TestSize.Level1)
{
    HILOGI("watch_service_HidDoubleClick_0001 start");

    watchService_->HidDoubleClick();
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_DISCONNECTED;
    watchService_->HidDoubleClick();
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_CONNECTED;
    watchService_->HidDoubleClick();
    EXPECT_EQ(BTHD_CONN_STATE_CONNECTED, watchService_->hidConnectionState_);
    watchService_->Stop();

    HILOGI("watch_service_HidDoubleClick_0001 end");
}

/*
 * @tc.number: watch_service_HidSingleClick_0001
 * @tc.name: HidSingleClick
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_HidSingleClick_0001, TestSize.Level1)
{
    HILOGI("watch_service_HidSingleClick_0001 start");

    watchService_->HidSingleClick();
    AdapterManager::GetInstance()->setBluetoothInterface(btInterface.get());
    watchService_->Start();
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_DISCONNECTED;
    watchService_->HidSingleClick();
    watchService_->hidConnectionState_ = BTHD_CONN_STATE_CONNECTED;
    updateSendReportResult(BT_STATUS_SUCCESS);
    watchService_->HidSingleClick();
    EXPECT_EQ(BTHD_CONN_STATE_CONNECTED, watchService_->hidConnectionState_);
    watchService_->Stop();

    HILOGI("watch_service_HidSingleClick_0001 end");
}

/*
 * @tc.number: watch_service_SetPhoneConnHfp_0001
 * @tc.name: SetPhoneConnHfp
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SetPhoneConnHfp_0001, TestSize.Level1)
{
    HILOGI("watch_service_SetPhoneConnHfp_0001 start");

    RawAddress device(address);
    watchService_->SetPhoneConnHfp(address, false);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isPhoneConnHfp_);
    watchService_->SetPhoneConnHfp(address, true);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isPhoneConnHfp_);

    HILOGI("watch_service_SetPhoneConnHfp_0001 end");
}

/*
 * @tc.number: watch_service_StopIphoneScoDisconnectTimer_0001
 * @tc.name: StopIphoneScoDisconnectTimer
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_StopIphoneScoDisconnectTimer_0001, TestSize.Level1)
{
    HILOGI("watch_service_StopIphoneScoDisconnectTimer_0001 start");

    RawAddress device(address);
    watchService_->StopIphoneScoDisconnectTimer(address2);
    EXPECT_EQ(watchService_->disconnectScoTimer_, nullptr);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(APPLE_IAP_UUID));
    remoteDevice->SetDeviceUuids(remoteUuids);

    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfAudioConnected audioConnected(AUDIO_CONNECTED, stateMachine, connectedState);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->Init();
    hfpHfService_->stateMachines_[address]->current_ = &audioConnected;
    watchService_->StartIphoneScoDisconnectTimer(address);
    EXPECT_NE(watchService_->disconnectScoTimer_, nullptr);
    watchService_->StopIphoneScoDisconnectTimer(address);
    EXPECT_EQ(watchService_->disconnectScoTimer_, nullptr);

    HILOGI("watch_service_StopIphoneScoDisconnectTimer_0001 end");
}

/*
 * @tc.number: watch_service_StartIphoneScoDisconnectTimer_0001
 * @tc.name: StartIphoneScoDisconnectTimer
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_StartIphoneScoDisconnectTimer_0001, TestSize.Level1)
{
    HILOGI("watch_service_StartIphoneScoDisconnectTimer_0001 start");

    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(APPLE_IAP_UUID));
    remoteDevice->SetDeviceUuids(remoteUuids);
    HfpHfConnected connectedState(CONNECTED, stateMachine);
    HfpHfAudioConnected audioConnected(AUDIO_CONNECTED, stateMachine, connectedState);
    hfpHfService_->stateMachines_[address] = std::make_unique<HfpHfStateMachine>(address);
    hfpHfService_->stateMachines_[address]->Init();
    hfpHfService_->stateMachines_[address]->current_ = &audioConnected;
    watchService_->StartIphoneScoDisconnectTimer(address);
    EXPECT_NE(watchService_->disconnectScoTimer_, nullptr);
    watchService_->StopIphoneScoDisconnectTimer(address);
    EXPECT_EQ(watchService_->disconnectScoTimer_, nullptr);

    HILOGI("watch_service_StartIphoneScoDisconnectTimer_0001 end");
}

/*
 * @tc.number: watch_service_IsValidAddress_0001
 * @tc.name: IsValidAddress
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsValidAddress_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsValidAddress_0001 start");

    EXPECT_FALSE(watchService_->IsValidAddress(""));
    std::string address = "11:22:33:44:55:66";
    EXPECT_TRUE(watchService_->IsValidAddress(address));

    std::string invalidAddress = "11:22:33:44:55:gg";
    EXPECT_FALSE(watchService_->IsValidAddress(invalidAddress));

    HILOGI("watch_service_IsValidAddress_0001 end");
}

HWTEST_F(WatchServiceTest, watch_service_HandleConnectHfpEvent_0001, TestSize.Level1)
{
    HILOGI("watch_service_HandleConnectHfpEvent_0001 start");

    EventFwk::CommonEventData data;
    AAFwk::Want want;

    want.SetParam("isVoip", 1);
    want.SetParam("needConnect", 0);
    data.SetWant(want);
    RawAddress device(address);
    watchService_->HandleConnectHfpEvent(want);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->isOnVoip_Locked());
    want.SetParam("needConnect", 1);
    data.SetWant(want);
    watchService_->HandleConnectHfpEvent(want);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->isOnVoip_Locked());

    HILOGI("watch_service_HandleConnectHfpEvent_0001 end");
}

/*
 * @tc.number: watch_service_IsEnableBluetoothAfterExitStrMode_0001
 * @tc.name: IsEnableBluetoothAfterExitStrMode
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsEnableBluetoothAfterExitStrMode_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsEnableBluetoothAfterExitStrMode_0001 start");

    system::SetParameter("persist.bluetooth.switch_enable", "1");
    EXPECT_FALSE(watchService_->IsEnableBluetoothAfterExitStrMode());

    system::SetParameter("persist.bluetooth.switch_enable", "0");
    system::SetParameter("persist.bluetooth.watch_str_enable", "1");
    EXPECT_TRUE(watchService_->IsEnableBluetoothAfterExitStrMode());

    system::SetParameter("persist.bluetooth.watch_str_enable", "0");
    EXPECT_FALSE(watchService_->IsEnableBluetoothAfterExitStrMode());

    HILOGI("watch_service_IsEnableBluetoothAfterExitStrMode_0001 end");
}

/*
 * @tc.number: watch_service_ResetScoState_0001
 * @tc.name: ResetScoState
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ResetScoState_0001, TestSize.Level1)
{
    HILOGI("watch_service_ResetScoState_0001 start");
 
    watchService_->ResetScoState();
    EXPECT_EQ(watchService_->disconnectScoTimer_, nullptr);
 
    HILOGI("watch_service_ResetScoState_0001 end");
}

/*
 * @tc.number: watch_service_ChangeStatusEnterStrMode_0001
 * @tc.name: ChangeStatusEnterStrMode
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_ChangeStatusEnterStrMode_0001, TestSize.Level1)
{
    HILOGI("watch_service_ChangeStatusEnterStrMode_0001 start");

    system::SetParameter("persist.bluetooth.switch_enable", "2");
    watchService_->ChangeStatusEnterStrMode();
    int value = system::GetIntParameter("persist.bluetooth.watch_half_property", 0);
    EXPECT_EQ(1, value);

    system::SetParameter("persist.bluetooth.switch_enable", "1");
    watchService_->ChangeStatusEnterStrMode();
    value = system::GetIntParameter("persist.bluetooth.watch_half_property", 0);
    system::SetParameter("persist.bluetooth.switch_enable", "0");
    EXPECT_EQ(0, value);

    HILOGI("watch_service_ChangeStatusEnterStrMode_0001 end");
}

/*
 * @tc.number: watch_service_IsSaisDevice_0001
 * @tc.name: IsSaisDevice
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_IsSaisDevice_0001, TestSize.Level1)
{
    HILOGI("watch_service_IsSaisDevice_0001 start");

    RawAddress device(address);
    bool ret = false;
    ret = AddDeviceManager(address);
    EXPECT_FALSE(ret);
    watchService_->UpdateSaisDevice(device, true);
    EXPECT_TRUE(watchService_->IsSaisDevice(address));
    watchService_->UpdateSaisDevice(device, false);
    EXPECT_FALSE(watchService_->IsSaisDevice(address));

    HILOGI("watch_service_IsSaisDevice_0001 end");
}

/*
 * @tc.number: watch_service_SecondaryPhoneChange_0001
 * @tc.name: SecondaryPhoneChange
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_SecondaryPhoneChange_0001, TestSize.Level1)
{
    HILOGI("watch_service_SecondaryPhoneChange_0001 start");

    watchService_->SecondaryPhoneChange(0);
    EXPECT_TRUE(watchService_->isSecondaryPair_);
    watchService_->SecondaryPhoneChange(1);
    EXPECT_FALSE(watchService_->isSecondaryPair_);
    watchService_->SecondaryPhoneChange(3);
    EXPECT_FALSE(watchService_->isSecondaryPair_);

    HILOGI("watch_service_SecondaryPhoneChange_0001 end");
}
/*
 * @tc.number: watch_service_CheckSaisDeviceAndRemoveOld_0001
 * @tc.name: CheckSaisDeviceAndRemoveOld
 * @tc.desc:
 */
HWTEST_F(WatchServiceTest, watch_service_CheckSaisDeviceAndRemoveOld_0001, TestSize.Level1)
{
    HILOGI("watch_service_CheckSaisDeviceAndRemoveOld_0001 start");
    RawAddress device(address);
    RawAddress device2(address2);
    AddDeviceManager(address);
    watchService_->SecondaryPhoneChange(0);
    watchService_->CheckSaisDeviceAndRemoveOld(device2);
    EXPECT_EQ(GetDeviceManager(address2), nullptr);
    watchService_->CheckSaisDeviceAndRemoveOld(device);
    EXPECT_FALSE(GetDeviceManager(device.GetAddress())->IsSaisDeviceLocked());
    GetDeviceManager(device.GetAddress())->SetIsSaisDeviceLocked(true);
    watchService_->CheckSaisDeviceAndRemoveOld(device);
    EXPECT_TRUE(GetDeviceManager(device.GetAddress())->IsSaisDeviceLocked());
    HILOGI("watch_service_CheckSaisDeviceAndRemoveOld_0001 end");
}

/*
 * @tc.number: watch_service_StartWatchPbap_0001
 * @tc.name: StartWatchPbap
 * @tc.desc: 已连接时复用客户端；同地址上次失败时复用客户端重连；
 *           连接设备切换时替换为按新地址创建的客户端并断开旧客户端
 */
HWTEST_F(WatchServiceTest, watch_service_StartWatchPbap_0001, TestSize.Level1)
{
    HILOGI("watch_service_StartWatchPbap_0001 start");

    // scenario 1: 客户端已连接 → 直接返回，客户端复用且不投递新连接任务
    auto client = std::make_shared<WatchPbapClient>(address);
    client->state_.store(WatchPbapState::CONNECTED);
    watchService_->watchPbapClient_ = client;
    watchService_->StartWatchPbap(address);
    EXPECT_EQ(client, watchService_->watchPbapClient_);
    EXPECT_EQ(WatchPbapState::CONNECTED, client->state_.load());
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_TRUE(client->taskQueue_.empty());
    }

    // scenario 2: 同地址上次连接失败（IDLE）→ 复用客户端重新发起连接
    client->state_.store(WatchPbapState::IDLE);
    watchService_->StartWatchPbap(address);
    EXPECT_EQ(client, watchService_->watchPbapClient_);
    EXPECT_NE(client->workerThread_, static_cast<pthread_t>(0));
    WaitPbapClientIdle(client);
    EXPECT_EQ(WatchPbapState::IDLE, client->state_.load());

    // scenario 3: 连接设备切换 → 旧客户端被替换为按新地址创建的客户端并异步断开
    watchService_->StartWatchPbap(address2);
    auto newClient = watchService_->watchPbapClient_;
    ASSERT_NE(newClient, nullptr);
    EXPECT_NE(client, newClient);
    EXPECT_EQ(address2, newClient->GetAddress());
    WaitPbapClientStopped(client);
    EXPECT_EQ(WatchPbapState::IDLE, client->state_.load());
    WaitPbapClientIdle(newClient);

    // 清理：断开新客户端
    watchService_->StopWatchPbap(address2);
    EXPECT_EQ(nullptr, watchService_->watchPbapClient_);
    WaitPbapClientStopped(newClient);

    HILOGI("watch_service_StartWatchPbap_0001 end");
}

/*
 * @tc.number: watch_service_StopWatchPbap_0001
 * @tc.name: StopWatchPbap
 * @tc.desc: 无客户端直接返回；地址不匹配保留客户端；地址匹配移除并断开
 */
HWTEST_F(WatchServiceTest, watch_service_StopWatchPbap_0001, TestSize.Level1)
{
    HILOGI("watch_service_StopWatchPbap_0001 start");

    // scenario 1: ÎÞ¿Í»§¶Ë ¡ú Ö±½Ó·µ»Ø
    watchService_->watchPbapClient_ = nullptr;
    watchService_->StopWatchPbap(address);
    EXPECT_EQ(nullptr, watchService_->watchPbapClient_);

    // scenario 2: 地址不匹配 → 客户端保留
    auto client = std::make_shared<WatchPbapClient>(address);
    watchService_->watchPbapClient_ = client;
    watchService_->StopWatchPbap(address2);
    EXPECT_EQ(client, watchService_->watchPbapClient_);

    // scenario 3: 地址匹配 → 客户端移除并断开（无工作线程时同步完成）
    watchService_->StopWatchPbap(address);
    EXPECT_EQ(nullptr, watchService_->watchPbapClient_);
    EXPECT_EQ(WatchPbapState::IDLE, client->state_.load());

    HILOGI("watch_service_StopWatchPbap_0001 end");
}

/*
 * @tc.number: watch_service_QueryWatchPbapContact_0001
 * @tc.name: QueryWatchPbapContact
 * @tc.desc: 无客户端直接返回；地址不匹配或空号码不投递查询；正常查询投递到客户端任务队列
 */
HWTEST_F(WatchServiceTest, watch_service_QueryWatchPbapContact_0001, TestSize.Level1)
{
    HILOGI("watch_service_QueryWatchPbapContact_0001 start");

    // scenario 1: 无客户端 → 直接返回，不崩溃
    watchService_->watchPbapClient_ = nullptr;
    watchService_->QueryWatchPbapContact(address, "13800138000");

    auto client = std::make_shared<WatchPbapClient>(address2);
    watchService_->watchPbapClient_ = client;
    {
        // 模拟 worker 存活：入队行为可确定性观测
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        client->running_ = true;
    }

    // scenario 2: 地址不匹配 → 不向客户端投递查询任务
    watchService_->QueryWatchPbapContact(address, "13800138000");
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_TRUE(client->taskQueue_.empty());
    }

    // scenario 3: 空号码在客户端入口被拒
    watchService_->QueryWatchPbapContact(address2, "");
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_TRUE(client->taskQueue_.empty());
    }

    // scenario 4: 正常查询 → 投递到客户端任务队列
    watchService_->QueryWatchPbapContact(address2, "13800138000");
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        EXPECT_EQ(static_cast<size_t>(1), client->taskQueue_.size());
        client->running_ = false;
        client->taskQueue_.clear();
    }
    watchService_->watchPbapClient_ = nullptr;

    HILOGI("watch_service_QueryWatchPbapContact_0001 end");
}

/*
 * @tc.number: watch_service_StopWithPbapClient_0001
 * @tc.name: Stop
 * @tc.desc: Stop 时移除并断开 pbap 客户端
 */
HWTEST_F(WatchServiceTest, watch_service_StopWithPbapClient_0001, TestSize.Level1)
{
    HILOGI("watch_service_StopWithPbapClient_0001 start");
    // Stop 会注销 classic observer，先确保已创建（对齐 BluetoothIsOn 的注册路径）
    if (watchService_->classicRemoteDeviceObserver_ == nullptr) {
        watchService_->classicRemoteDeviceObserver_ =
            std::make_unique<WatchService::ClassicRemoteDeviceObserver>(watchService_);
    }
    auto client = std::make_shared<WatchPbapClient>(address);
    watchService_->watchPbapClient_ = client;
    watchService_->Stop();
    EXPECT_EQ(nullptr, watchService_->watchPbapClient_);
    EXPECT_EQ(WatchPbapState::IDLE, client->state_.load());

    HILOGI("watch_service_StopWithPbapClient_0001 end");
}
}  // namespace bluetooth
}  // namespace OHOS
