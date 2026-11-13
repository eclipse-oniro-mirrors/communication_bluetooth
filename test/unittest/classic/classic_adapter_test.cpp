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
#include <gmock/gmock.h>
#include <cstring>
#include <memory>
#include "classic_adapter.h"
#include "power_device.h"
#include "class_creator.h"
#include "adapter_config.h"
#include "ble_adapter.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "interface_adapter_classic.h"
#include "interface_adapter_manager.h"
#include "profile_service_manager.h"
#include "hfp_ag_service.h"
#include "a2dp_service.h"
#include "a2dp_src/a2dp_src_service.h"
#include "hid_host_service.h"
#include "connect_strategy_manager.h"
#include "profile_config.h"
#include "profile_info.h"
#include "bluetooth_remote_device_info.h"
#include "bluetooth_hw_interface.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_access_token_mock.h"
#include "directory_ex.h"
#include "interface_profile_hearing_aid.h"
#include "interface_profile_a2dp_src.h"
#include "context.h"

using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;
using ::testing::Invoke;

namespace OHOS {
namespace bluetooth {

class MockIProfileHfpAg : public IProfileHfpAg {
public:
    MOCK_METHOD(int, Connect, (const RawAddress &device), (override));
    MOCK_METHOD(int, Disconnect, (const RawAddress &device), (override));
    MOCK_METHOD(std::list<RawAddress>, GetConnectDevices, (), (override));
    MOCK_METHOD(int, GetConnectState, (), (override));
    MOCK_METHOD(int, GetMaxConnectNum, (), (override));
    MOCK_METHOD(int, GetDeviceState, (const RawAddress &device), (override));
    MOCK_METHOD(std::vector<RawAddress>, GetDevicesByStates, (std::vector<int> states), (override));
    MOCK_METHOD(int, SetConnectStrategy, (const RawAddress &device, int strategy), (override));
    MOCK_METHOD(int, GetConnectStrategy, (const RawAddress &device), (override));
    MOCK_METHOD(utility::Context *, GetContext, (), (override));
    MOCK_METHOD(int, GetScoState, (const RawAddress &device), (override));
    MOCK_METHOD(int32_t, ConnectSco, (uint8_t callType), (override));
    MOCK_METHOD(int32_t, DisconnectSco, (uint8_t callType), (override));
    MOCK_METHOD(bool, ConnectSco, (), (override));
    MOCK_METHOD(bool, DisconnectSco, (), (override));
    MOCK_METHOD(void, PhoneStateChanged, (Bluetooth::BluetoothPhoneState &phoneState, bool isVirtualCall), (override));
    MOCK_METHOD(void, ClccResponse, (int index, int direction, int status, int mode, bool mpty, const std::string &number, int type), (override));
    MOCK_METHOD(bool, OpenVoiceRecognition, (const RawAddress &device), (override));
    MOCK_METHOD(bool, CloseVoiceRecognition, (const RawAddress &device), (override));
    MOCK_METHOD(bool, SetActiveDevice, (const RawAddress &device), (override));
    MOCK_METHOD(bool, IntoMock, (int state), (override));
    MOCK_METHOD(bool, SendNoCarrier, (const RawAddress &device), (override));
    MOCK_METHOD(std::string, GetActiveDevice, (), (override));
    MOCK_METHOD(bool, IsAudioConnected, (), (override));
    MOCK_METHOD(void, RegisterObserver, (HfpAgServiceObserver &observer), (override));
    MOCK_METHOD(void, DeregisterObserver, (HfpAgServiceObserver &observer), (override));
    MOCK_METHOD(void, NotifyAudioVolumeEvent, (int32_t streamType, int32_t volume), (override));
    MOCK_METHOD(bool, IsInbandRingingEnabled, (), (override));
    MOCK_METHOD(void, NotifyPhonebookAuthResult, (const RawAddress &device, int32_t accessAuthorization, int32_t pbapLoadFlag), (override));
    MOCK_METHOD(void, CallDetailsChanged, (int callId, int callState), (override));
    MOCK_METHOD(void, EnableBtCallLog, (bool state), (override));
    MOCK_METHOD(int, IsVgsSupported, (const RawAddress &device, bool &isSupported), (override));
    MOCK_METHOD(void, UpdateVirtualDevice, (int32_t action, const std::string &address), (override));
    MOCK_METHOD(void, GetVirtualDeviceList, (std::vector<std::string> &devices), (override));
    MOCK_METHOD(int, GetCurrentCallType, (int &callType), (override));
    MOCK_METHOD(int, IsVoiceRecognitionSupported, (const RawAddress &device, bool &isSupported), (override));
};

class MockIProfileA2dpSrc : public IProfileA2dpSrc {
public:
    MOCK_METHOD(int, Connect, (const RawAddress &device), (override));
    MOCK_METHOD(int, Disconnect, (const RawAddress &device), (override));
    MOCK_METHOD(std::list<RawAddress>, GetConnectDevices, (), (override));
    MOCK_METHOD(int, GetConnectState, (), (override));
    MOCK_METHOD(int, GetMaxConnectNum, (), (override));
    MOCK_METHOD(int, SetConnectStrategy, (const RawAddress &device, int strategy), (override));
    MOCK_METHOD(int, GetConnectStrategy, (const RawAddress &device), (const override));
    MOCK_METHOD(std::vector<RawAddress>, GetDevicesByStates, (std::vector<int> &states), (const override));
    MOCK_METHOD(int, GetDeviceState, (const RawAddress &device), (const override));
    MOCK_METHOD(int, GetPlayingState, (const RawAddress &device, int &state), (const override));
    MOCK_METHOD(void, RegisterObserver, (IA2dpObserver *observer), (override));
    MOCK_METHOD(utility::Context *, GetContext, (), (override));
    MOCK_METHOD(int, SetActiveSinkDevice, (const RawAddress &device), (override));
    MOCK_METHOD(const RawAddress &, GetActiveSinkDevice, (), (const override));
    MOCK_METHOD(int, SendDelay, (const RawAddress &device, uint16_t delayValue), (override));
    MOCK_METHOD(A2dpSrcCodecStatus, GetCodecStatus, ((const RawAddress &device)), (const override));
    MOCK_METHOD(int, GetCodecPreference, (const RawAddress &device, A2dpSrcCodecInfo &info), (override));
    MOCK_METHOD(int, SetCodecPreference, (const RawAddress &device, const A2dpSrcCodecInfo &info), (override));
    MOCK_METHOD(void, SwitchOptionalCodecs, (const RawAddress &device, bool isEnable), (override));
    MOCK_METHOD(int, GetOptionalCodecsSupportState, (const RawAddress &device), (const override));
    MOCK_METHOD(int, StartPlaying, (const RawAddress &device), (override));
    MOCK_METHOD(int, SuspendPlaying, (const RawAddress &device), (override));
    MOCK_METHOD(int, StopPlaying, (const RawAddress &device), (override));
    MOCK_METHOD(void, DeregisterObserver, (IA2dpObserver *observer), (override));
    MOCK_METHOD(int, WriteFrame, (const uint8_t *data, uint32_t size), (override));
    MOCK_METHOD(int, GetRenderPosition, (const RawAddress &device, uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp), (override));
    MOCK_METHOD(void, NotifyMediaStackChanged, (int action, const RawAddress &device), (override));
    MOCK_METHOD(bool, IsActiveDeviceInPlayToPauseDenyList, (), (override));
    MOCK_METHOD(int, OffloadStartPlaying, (const RawAddress &device, const std::vector<int> &sessionsId), (override));
    MOCK_METHOD(int, OffloadStopPlaying, (const RawAddress &device, const std::vector<int> &sessionsId), (override));
    MOCK_METHOD(int, A2dpOffloadSessionPathRequest, (const RawAddress &device, const std::vector<struct A2dpSrcStreamInfo> &info), (override));
    MOCK_METHOD(A2dpSrcOffloadCodecStatus, GetA2dpOffloadCodecStatus, (const RawAddress &device), (override));
    MOCK_METHOD(void, ForceStopOffloadPlaying, (const RawAddress &device), (override));
    MOCK_METHOD(int, EnableAutoPlay, (const RawAddress &device), (override));
    MOCK_METHOD(int, DisableAutoPlay, (const RawAddress &device, const int duration), (override));
    MOCK_METHOD(int, GetAutoPlayDisabledDuration, (const RawAddress &device, int &duration), (override));
    MOCK_METHOD(void, OnRenderStateChange, (bool isRenderActive), (override));
    MOCK_METHOD(void, UpdateVirtualDevice, (int32_t action, const std::string &address), (override));
    MOCK_METHOD(void, GetVirtualDeviceList, (std::vector<std::string> &devices), (override));
    MOCK_METHOD(void, OnRenderSceneChange, (bool anyRenderRunning, bool gameRunning, bool musicRunning), (override));
    MOCK_METHOD(void, HwSetActiveMode, (), (override));
    MOCK_METHOD(void, SetNavigationMode, (bool navigationRunning), (override));
    MOCK_METHOD(int, GetMusicState, (), (override));
    MOCK_METHOD(int, GetGameState, (), (override));
};

class MockIProfileHidHost : public IProfileHidHost {
public:
    MOCK_METHOD(int, Connect, (const RawAddress &device), (override));
    MOCK_METHOD(int, Disconnect, (const RawAddress &device), (override));
    MOCK_METHOD(std::list<RawAddress>, GetConnectDevices, (), (override));
    MOCK_METHOD(int, GetConnectState, (), (override));
    MOCK_METHOD(int, GetMaxConnectNum, (), (override));
    MOCK_METHOD(int, SetConnectStrategy, (const RawAddress &device, int strategy), (override));
    MOCK_METHOD(int, GetConnectStrategy, (const RawAddress &device), (override));
    MOCK_METHOD(std::vector<RawAddress>, GetDevicesByStates, (std::vector<int> states), (override));
    MOCK_METHOD(int, GetDeviceState, (const RawAddress &device), (override));
    MOCK_METHOD(utility::Context *, GetContext, (), (override));
    MOCK_METHOD(void, RegisterObserver, (IHidHostObserver &observer), (override));
    MOCK_METHOD(void, DeregisterObserver, (IHidHostObserver &observer), (override));
    MOCK_METHOD(int, HidHostVCUnplug, (std::string device, uint8_t id, uint16_t size, uint8_t type), (override));
    MOCK_METHOD(int, HidHostSendData, (std::string device, uint8_t id, uint16_t size, uint8_t type), (override));
    MOCK_METHOD(int, HidHostSetReport, (std::string device, uint8_t type, uint16_t size, const uint8_t* report), (override));
    MOCK_METHOD(int, HidHostGetReport, (std::string device, uint8_t id, uint16_t size, const uint8_t type), (override));
    MOCK_METHOD(void, ReStartStackHidProfile, (), (override));
};

class MockIProfileHearingAid : public IProfileHearingAid {
public:
    MOCK_METHOD(int, Connect, (const RawAddress &device), (override));
    MOCK_METHOD(int, Disconnect, (const RawAddress &device), (override));
    MOCK_METHOD(std::list<RawAddress>, GetConnectDevices, (), (override));
    MOCK_METHOD(int, GetConnectState, (), (override));
    MOCK_METHOD(int, GetMaxConnectNum, (), (override));
    MOCK_METHOD(int, SetConnectStrategy, (const RawAddress &device, int strategy), (override));
    MOCK_METHOD(int, GetConnectStrategy, (const RawAddress &device), (override));
    MOCK_METHOD(std::vector<RawAddress>, GetDevicesByStates, (std::vector<int> &states), (override));
    MOCK_METHOD(int, GetDeviceState, (const RawAddress &device), (override));
    MOCK_METHOD(utility::Context *, GetContext, (), (override));
    MOCK_METHOD(int, SetActiveDevice, (const RawAddress &device), (override));
    MOCK_METHOD(void, RegisterObserver, (IHearingAidObserver &observer), (override));
    MOCK_METHOD(void, DeregisterObserver, (IHearingAidObserver &observer), (override));
    MOCK_METHOD(int, SetVolumeByAddr, (const RawAddress &device, int volume), (override));
    MOCK_METHOD(void, GetActiveDevice, (RawAddress &addr, std::string &name), (override));
    MOCK_METHOD(int, GetHearingAidDeviceInfo, (const RawAddress &device, int& side, int mode, int64_t& hiSyncId), (override));
    MOCK_METHOD(bool, IsLe2MPhySupported, (), (override));
    MOCK_METHOD(void, StartStackProfile, (), (override));
};

static BluetoothHost *host_;
constexpr int TIME = 2;
ClassicAdapter *classicAdapter = nullptr;
RemoteDeviceProperties remoteDeviceProperties;
AdapterProperties adapterProperties;

void *HfpAgServiceCreate(void)
{
    return new HfpAgService();
}

void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

class AdapterStateObserverTest : public IAdapterClassicObserver {
public:
    void OnDiscoveryStateChanged(const int32_t status) override
    {
        HILOGI("into OnDiscoveryStateChanged");
    }

    void OnDiscoveryResult(const RawAddress &device, int rssi,
        const std::string deviceName, int deviceClass, int transport) override
    {
        HILOGI("into OnDiscoveryResult");
    }

    void OnPairRequested(const BTTransport transport, const RawAddress &device) override
    {
        HILOGI("into OnPairRequested");
    }

    void OnPairConfirmed(
        const BTTransport transport, const RawAddress &device, int32_t reqType, int32_t number) override
    {
        HILOGI("into OnPairConfirmed");
    }

    void OnScanModeChanged(int32_t mode) override
    {
        HILOGI("into OnScanModeChanged");
    }

    void OnDeviceNameChanged(const std::string &deviceName) override
    {
        HILOGI("into OnDeviceNameChanged");
    }

    void OnDeviceAddrChanged(const std::string &address) override
    {
        HILOGI("into OnDeviceAddrChanged");
    }

    void OnGenerateLocalOobData(int32_t ret, const Bluetooth::BluetoothOobData &oobData) override
    {
        HILOGI("into OnGenerateLocalOobData");
    }
};

class ClassicRemoteDeviceObserverTest : public IClassicRemoteDeviceObserver {
public:
    void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) override{}
    void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, int status, int cause,
        const std::string &causeMessage) override{}
    void OnRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) override{}
    void OnRemoteNameChanged(const RawAddress &device, const std::string &deviceName) override{}
    void OnRemoteAliasChanged(const RawAddress &device, const std::string &alias) override{}
    void OnRemoteCodChanged(const RawAddress &device, int cod) override{}
    void OnRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo) override {}
    void OnRemoteDeviceCommonInfoReport(const RawAddress &device, const std::vector<uint8_t> &value) override {}
};

class ClassicAdapterTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        BluetoothAccessTokenMock::SetNativeTokenInfo();
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBleEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        // new classicAdapter
        classicAdapter = ClassCreator<ClassicAdapter>::NewInstance(ADAPTER_NAME_CLASSIC);
        classicAdapter->remoteDeviceProperties_ = &remoteDeviceProperties;
        classicAdapter->adapterProperties_ = &adapterProperties;
        std::vector<Uuid> localUuids;
        localUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_AG));
        classicAdapter->adapterProperties_->SetServiceUuids(localUuids);
        ClassFactory::RegisterClass("HfpAgService", HfpAgServiceCreate);
        ClassFactory::RegisterClass("A2dpSrcService", A2dpSrcServiceCreate);
        auto manager = ProfileServiceManager::GetInstance();
        manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_HFP_AG);
        manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_A2DP_SRC);
        manager->SetProfileService(ADAPTER_BREDR, PROFILE_NAME_HID_HOST);

        classicAdapter->agService_ = HfpAgService::GetService();
        classicAdapter->a2dpSrcService_ = GetServiceInstance(A2DP_ROLE_SOURCE);
        classicAdapter->hidHostService_ = HidHostService::GetService();
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        if (classicAdapter->agService_ != nullptr) {
            delete classicAdapter->agService_;
        }
        if (classicAdapter->a2dpSrcService_ != nullptr) {
            delete classicAdapter->a2dpSrcService_;
        }
        if (classicAdapter->hidHostService_ != nullptr) {
            delete classicAdapter->hidHostService_;
        }
        // delete classicAdapter
        delete classicAdapter;
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp()
    {}
    void TearDown()
    {}
    MockIProfileHfpAg *mockHfpAg;
    MockIProfileA2dpSrc *mockA2dpSrc;
    MockIProfileHidHost *mockHidHost;
    MockIProfileHearingAid *mockHearingAid;
public:
};

/**
 * @tc.name: classic_adapter_test_001
 * @tc.desc: ClassicAdapterTest:ShutDown
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_001, TestSize.Level1)
{
    HILOGI("classic_adapter_test_001 enter");
    classicAdapter->ProcessPostEnable();
    classicAdapter->ShutDown();
    classicAdapter->ProcessPostEnable();
    EXPECT_NE(classicAdapter->StartBtDiscovery(), BT_NO_ERROR);
    classicAdapter->ShutDown();
    HILOGI("classic_adapter_test_001 end");
}

/**
 * @tc.name: classic_adapter_test_002
 * @tc.desc: ClassicAdapterTest:DeregisterClassicAd00:11:22:33:44:66apterObserver
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_002, TestSize.Level1)
{
    HILOGI("classic_adapter_test_002 enter");
    AdapterStateObserverTest observer;
    EXPECT_FALSE(classicAdapter->DeregisterClassicAdapterObserver(observer));
    HILOGI("classic_adapter_test_002 end");
}

/**
 * @tc.name: classic_adapter_test_003
 * @tc.desc: ClassicAdapterTest:LoadPairedDeviceInfo
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_003, TestSize.Level1)
{
    HILOGI("classic_adapter_test_003 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    Bluetooth::BluetoothOobData dummyOobData;
    EXPECT_FALSE(classicAdapter->StartPair(BTTransport::ADAPTER_BREDR, realAddr1, dummyOobData));
    bt_status_t status = BT_STATUS_SUCCESS;
    STACK::RawAddress bd_addr;
    bt_bond_state_t state = BT_BOND_STATE_BONDED;
    classicAdapter->BondStateChangedInner(status, bd_addr, state);
    bd_addr = ServiceUtil::AddrToStack(realAddr1);
    classicAdapter->BondStateChangedInner(status, bd_addr, state);
    classicAdapter->LoadPairedDeviceInfo();
    HILOGI("classic_adapter_test_003 end");
}

/**
 * @tc.name: classic_adapter_test_004
 * @tc.desc: ClassicAdapterTest:StartBtDiscovery
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_004, TestSize.Level1)
{
    HILOGI("classic_adapter_test_004 enter");
    EXPECT_NE(classicAdapter->StartBtDiscovery(), BT_NO_ERROR);
    HILOGI("classic_adapter_test_004 end");
}

/**
 * @tc.name: classic_adapter_test_005
 * @tc.desc: ClassicAdapterTest:CancelBtDiscovery
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_005, TestSize.Level1)
{
    HILOGI("classic_adapter_test_005 enter");
    classicAdapter->CancelBtDiscovery();
    RawAddress device = RawAddress("00:11:22:33:44:55");
    classicAdapter->CancelDiscoveryIfInCloudBonding(device);
    EXPECT_EQ(classicAdapter->CancelBtDiscovery(), false);
    HILOGI("classic_adapter_test_005 end");
}

/**
 * @tc.name: classic_adapter_test_006
 * @tc.desc: ClassicAdapterTest:GetDeviceUuids
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_006, TestSize.Level1)
{
    HILOGI("classic_adapter_test_006 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:55");
    std::vector<std::string> uuids = classicAdapter->GetDeviceUuids(realAddr1);
    EXPECT_TRUE(uuids.empty());
    HILOGI("classic_adapter_test_006 end");
}

/**
 * @tc.name: classic_adapter_test_007
 * @tc.desc: ClassicAdapterTest:GetLocalProfileUuids
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_007, TestSize.Level1)
{
    HILOGI("classic_adapter_test_007 enter");
    std::vector<std::string> uuids = classicAdapter->GetLocalProfileUuids();
    EXPECT_FALSE(uuids.empty());
    HILOGI("classic_adapter_test_007 end");
}

/**
 * @tc.name: classic_adapter_test_008
 * @tc.desc: ClassicAdapterTest:GetPairedDevices
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_008, TestSize.Level1)
{
    HILOGI("classic_adapter_test_008 enter");
    std::vector<RawAddress> pairedDevices = classicAdapter->GetPairedDevices();
    EXPECT_TRUE(pairedDevices.empty());
    HILOGI("classic_adapter_test_008 end");
}

/**
 * @tc.name: classic_adapter_test_009
 * @tc.desc: ClassicAdapterTest:StartPair
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_009, TestSize.Level1)
{
    HILOGI("classic_adapter_test_009 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:55");
    Bluetooth::BluetoothOobData dummyOobData;
    EXPECT_FALSE(classicAdapter->StartPair(BTTransport::ADAPTER_BREDR, realAddr1, dummyOobData));
    EXPECT_FALSE(classicAdapter->StartPair(BTTransport::ADAPTER_BLE, realAddr1, dummyOobData));
    HILOGI("classic_adapter_test_009 end");
}

/**
 * @tc.name: classic_adapter_test_010
 * @tc.desc: ClassicAdapterTest:RemovePair
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_010, TestSize.Level1)
{
    HILOGI("classic_adapter_test_010 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:55");
    EXPECT_FALSE(classicAdapter->RemovePair(BTTransport::ADAPTER_BREDR, realAddr1));
    EXPECT_FALSE(classicAdapter->RemovePair(BTTransport::ADAPTER_BLE, realAddr1));
    HILOGI("classic_adapter_test_010 end");
}

/**
 * @tc.name: classic_adapter_test_011
 * @tc.desc: ClassicAdapterTest:RemoveAllPairs
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_011, TestSize.Level1)
{
    HILOGI("classic_adapter_test_011 enter");
    EXPECT_TRUE(classicAdapter->RemoveAllPairs());
    HILOGI("classic_adapter_test_011 end");
}

/**
 * @tc.name: classic_adapter_test_012
 * @tc.desc: ClassicAdapterTest:UserConfirmAutoReply
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_012, TestSize.Level1)
{
    HILOGI("classic_adapter_test_012 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:55");
    int type = 0;
    bool accepty = true;
    Bluetooth::BluetoothOobData dummyOobData;
    EXPECT_FALSE(classicAdapter->StartPair(BTTransport::ADAPTER_BREDR, realAddr1, dummyOobData));
    EXPECT_FALSE(classicAdapter->StartPair(BTTransport::ADAPTER_BLE, realAddr1, dummyOobData));
    classicAdapter->UserConfirmAutoReply(realAddr1, type, accepty);
    HILOGI("classic_adapter_test_012 end first switch");
    type = 4;
    classicAdapter->UserConfirmAutoReply(realAddr1, type, accepty);
    HILOGI("classic_adapter_test_012 end second switch");
    type = 1;
    classicAdapter->UserConfirmAutoReply(realAddr1, type, accepty);
    HILOGI("classic_adapter_test_012 end third switch");
    type = 2;
    classicAdapter->UserConfirmAutoReply(realAddr1, type, accepty);
    HILOGI("classic_adapter_test_012 end third switch");
    type = 5;
    classicAdapter->UserConfirmAutoReply(realAddr1, type, accepty);
    HILOGI("classic_adapter_test_012 end defaut switch");
}

/**
 * @tc.name: classic_adapter_test_013
 * @tc.desc: ClassicAdapterTest:DeregisterRemoteDeviceObserver
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_013, TestSize.Level1)
{
    HILOGI("classic_adapter_test_013 enter");
    ClassicRemoteDeviceObserverTest observer;
    EXPECT_FALSE(classicAdapter->DeregisterRemoteDeviceObserver(observer));
    HILOGI("classic_adapter_test_013 end");
}

/**
 * @tc.name: classic_adapter_test_014
 * @tc.desc: ClassicAdapterTest:CreateAclConnect
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_014, TestSize.Level1)
{
    HILOGI("classic_adapter_test_014 enter");
    std::string address = "11:22:33:44:55:66";
    classicAdapter->CreateAclConnect(address);
    EXPECT_FALSE(classicAdapter->GetRemoteServices(address));
    RawAddress device = RawAddress(address);
    classicAdapter->SendAclStateChanged(device, CONNECTION_STATE_DISCONNECTED, 0, PAIR_PAIRED);
    classicAdapter->SendAclStateChanged(device, CONNECTION_STATE_CONNECTED, 0, PAIR_PAIRED);
    HILOGI("classic_adapter_test_014 end");
}

/**
 * @tc.name: classic_adapter_test_015
 * @tc.desc: ClassicAdapterTest:CloudDevice
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_015, TestSize.Level1)
{
    HILOGI("classic_adapter_test_015 enter");
    RawAddress device = RawAddress("66:11:22:33:44:55");
    STACK::RawAddress bdAddr = ServiceUtil::AddrToStack(device);
    remoteDeviceProperties.FindRemoteDevice(device);
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_bond_state_t state = BT_BOND_STATE_BONDED;
    std::shared_ptr<BluetoothDevice> remoteDevice;
    classicAdapter->HandleBondStateBondNone(status, bdAddr, remoteDevice, device, state);
    EXPECT_FALSE(classicAdapter->CheckBondStateAndReturn(remoteDevice, UNBOND_CAUSE_INTERNAL_ERROR));
    EXPECT_FALSE(classicAdapter->IsInCloudBondingState(remoteDevice));
    remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    classicAdapter->HandleBondStateBondNone(status, bdAddr, remoteDevice, device, state);
    EXPECT_FALSE(classicAdapter->IsInCloudBondingState(remoteDevice));
    remoteDeviceProperties.SetCloudDevice(device, true);
    EXPECT_TRUE(classicAdapter->IsInCloudBondingState(remoteDevice));
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_REMOVING);
    EXPECT_TRUE(classicAdapter->StartCloudPair(device));
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_CREATE_BOND);
    EXPECT_TRUE(classicAdapter->StartCloudPair(device));
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_BONDING);
    EXPECT_TRUE(classicAdapter->StartCloudPair(device));
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_BONDED);
    EXPECT_TRUE(classicAdapter->IsInCloudBondingState(remoteDevice));
    EXPECT_TRUE(classicAdapter->CheckBondStateAndReturn(remoteDevice, UNBOND_CAUSE_INTERNAL_ERROR));
    EXPECT_TRUE(classicAdapter->IsInCloudBondingState(remoteDevice));
    EXPECT_FALSE(classicAdapter->StartCloudPair(device));
    EXPECT_FALSE(classicAdapter->CheckBondStateAndReturn(remoteDevice, UNBOND_CAUSE_USER_REMOVED));
    classicAdapter->HandleBondStateBonding(remoteDevice, device);
    EXPECT_EQ(OHOS::bluetooth::IAdapterManager::GetInstance()->
        GetPhonebookPermission("66:11:22:33:44:55"), BTPermissionType::ACCESS_UNKNOWN);
    EXPECT_EQ(OHOS::bluetooth::IAdapterManager::GetInstance()->
        GetMessagePermission("66:11:22:33:44:55"), BTPermissionType::ACCESS_UNKNOWN);    
    HILOGI("classic_adapter_test_015 end");
}

/**
 * @tc.name: classic_adapter_test_016
 * @tc.desc: ClassicAdapterTest:GenerateLocalOobDataCb
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_016, TestSize.Level1)
{
    HILOGI("classic_adapter_test_016 enter");
    int32_t status = BT_STATUS_SUCCESS;
    Bluetooth::BluetoothOobData oobData;
    classicAdapter->GenerateLocalOobDataCb(status, oobData);
    EXPECT_FALSE(oobData.HasAddressWithType());
    HILOGI("classic_adapter_test_016 end");
}

/**
 * @tc.name: classic_adapter_test_017
 * @tc.desc: ClassicAdapterTest:SetRemoteDeviceBatteryInfo
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_017, TestSize.Level1)
{
    HILOGI("classic_adapter_test_017 enter");
    RawAddress address("11:22:33:44:55:66");
    BatteryInfo batteryInfo;
    EXPECT_EQ(classicAdapter->SetRemoteDeviceBatteryInfo(address, batteryInfo), BT_NO_ERROR);
    HILOGI("classic_adapter_test_017 end");
}

/**
 * @tc.name: classic_adapter_test_018
 * @tc.desc: ClassicAdapterTest:SatisfyDisconnectAclCondition isSystemHap is true
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_018, TestSize.Level1)
{
    HILOGI("classic_adapter_test_018 enter");
    RawAddress address("11:22:33:44:55:66");
    EXPECT_TRUE(classicAdapter->SatisfyDisconnectAclCondition(address, true));
    HILOGI("classic_adapter_test_018 end");
}

/**
 * @tc.name: classic_adapter_test_019
 * @tc.desc: ClassicAdapterTest:SatisfyDisconnectAclCondition HID Host isConnected
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_019, TestSize.Level1)
{
    HILOGI("classic_adapter_test_019 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();

    {
        EXPECT_CALL(*mockHidHost, GetDeviceState(_))
            .WillOnce(Return(static_cast<int>(BTConnectState::CONNECTED)));
    }

    bool isSatisfyDisconnectAclCondition = classicAdapter->SatisfyDisconnectAclCondition(device, false);
    EXPECT_FALSE(isSatisfyDisconnectAclCondition);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_019 end");
}

/**
 * @tc.name: classic_adapter_test_020
 * @tc.desc: ClassicAdapterTest:SatisfyDisconnectAclCondition Hearing Aid isConnected
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_020, TestSize.Level1)
{
    HILOGI("classic_adapter_test_020 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();

    {
        EXPECT_CALL(*mockHearingAid, GetDeviceState(_))
            .WillOnce(Return(static_cast<int>(BTConnectState::CONNECTED)));
    }

    bool isSatisfyDisconnectAclCondition = classicAdapter->SatisfyDisconnectAclCondition(device, false);
    EXPECT_FALSE(isSatisfyDisconnectAclCondition);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_020 end");
}

/**
 * @tc.name: classic_adapter_test_021
 * @tc.desc: ClassicAdapterTest:SatisfyDisconnectAclCondition a2dp and hfp isConnected
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_021, TestSize.Level1)
{
    HILOGI("classic_adapter_test_021 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();

    {
        EXPECT_CALL(*mockHfpAg, GetDeviceState(_))
            .WillOnce(Return(static_cast<int>(BTConnectState::CONNECTED)));
        EXPECT_CALL(*mockA2dpSrc, GetDeviceState(_))
            .WillOnce(Return(static_cast<int>(BTConnectState::CONNECTED)));
    }

    bool isSatisfyDisconnectAclCondition = classicAdapter->SatisfyDisconnectAclCondition(device, false);
    EXPECT_TRUE(isSatisfyDisconnectAclCondition);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_021 end");
}

/**
 * @tc.name: classic_adapter_test_022
 * @tc.desc: ClassicAdapterTest:ConnectAnyAllowedProfile only connect hfp ag
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_022, TestSize.Level1)
{
    HILOGI("classic_adapter_test_022 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");

    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_HF));
    remoteDevice->SetDeviceUuids(remoteUuids);
    bool isHfpAgConnectCalled = false;

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();

    {
        EXPECT_CALL(*mockHfpAg, GetConnectStrategy(_))
            .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)));
        EXPECT_CALL(*mockHfpAg, Connect(_))
            .WillOnce(Invoke([&isHfpAgConnectCalled](const RawAddress &) {
                isHfpAgConnectCalled = true;
                return 0;
            }));
    }

    classicAdapter->ConnectAnyAllowedProfiles(device);
    EXPECT_TRUE(isHfpAgConnectCalled);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_022 end");
}

/**
 * @tc.name: classic_adapter_test_023
 * @tc.desc: ClassicAdapterTest:ConnectAnyAllowedProfile only connect a2dp src
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_023, TestSize.Level1)
{
    HILOGI("classic_adapter_test_023 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");

    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_A2DP_SINK));
    remoteDevice->SetDeviceUuids(remoteUuids);

    bool isA2dpSrcConnectCalled = false;
    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();

    {
        EXPECT_CALL(*mockA2dpSrc, GetConnectStrategy(_))
            .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)));
        EXPECT_CALL(*mockA2dpSrc, Connect(_))
            .WillOnce(Invoke([&isA2dpSrcConnectCalled](const RawAddress &) {
                isA2dpSrcConnectCalled = true;
                return 0;
            }));
    }

    classicAdapter->ConnectAnyAllowedProfiles(device);
    EXPECT_TRUE(isA2dpSrcConnectCalled);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_023 end");
}

/**
 * @tc.name: classic_adapter_test_024
 * @tc.desc: ClassicAdapterTest:ConnectAnyAllowedProfile only connect HID Host
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_024, TestSize.Level1)
{
    HILOGI("classic_adapter_test_024 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");

    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HID_HOST));
    remoteDevice->SetDeviceUuids(remoteUuids);

    bool isHIDHostConnectCalled = false;

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();

    {
        EXPECT_CALL(*mockHidHost, GetConnectStrategy(_))
            .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)));
        EXPECT_CALL(*mockHidHost, Connect(_))
            .WillOnce(Invoke([&isHIDHostConnectCalled](const RawAddress &) {
                isHIDHostConnectCalled = true;
                return 0;
            }));
    }

    classicAdapter->ConnectAnyAllowedProfiles(device, false);
    EXPECT_FALSE(isHIDHostConnectCalled);
    classicAdapter->ConnectAnyAllowedProfiles(device, true);
    EXPECT_TRUE(isHIDHostConnectCalled);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_024 end");
}

/**
 * @tc.name: classic_adapter_test_025
 * @tc.desc: ClassicAdapterTest:ConnectAnyAllowedProfile only connect hearing aid
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_025, TestSize.Level1)
{
    HILOGI("classic_adapter_test_025 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");

    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HEARING_AID));
    remoteDevice->SetDeviceUuids(remoteUuids);

    bool isHearingAidConnectCalled = false;

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();

    {
        EXPECT_CALL(*mockHearingAid, GetConnectStrategy(_))
            .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)));
        EXPECT_CALL(*mockHearingAid, Connect(_))
            .WillOnce(Invoke([&isHearingAidConnectCalled](const RawAddress &) {
                isHearingAidConnectCalled = true;
                return 0;
            }));
    }

    classicAdapter->ConnectAnyAllowedProfiles(device, false);
    EXPECT_FALSE(isHearingAidConnectCalled);
    classicAdapter->ConnectAnyAllowedProfiles(device, true);
    EXPECT_TRUE(isHearingAidConnectCalled);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_025 end");
}

/**
 * @tc.name: classic_adapter_test_026
 * @tc.desc: ClassicAdapterTest:DisconnectProfiles only connect a2dp src
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_026, TestSize.Level1)
{
    HILOGI("classic_adapter_test_026 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");
    bool isA2dpSrcConnectCalled = false;

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();
    {
        EXPECT_CALL(*mockA2dpSrc, GetDeviceState(_))
            .WillOnce(Return(static_cast<int>(BTConnectState::CONNECTED)));
        EXPECT_CALL(*mockA2dpSrc, Disconnect(_))
            .WillOnce(Invoke([&isA2dpSrcConnectCalled](const RawAddress &) {
                isA2dpSrcConnectCalled = true;
                return 0;
            }));
    }

    classicAdapter->DisconnectProfiles(device);
    EXPECT_TRUE(isA2dpSrcConnectCalled);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_026 end");
}

/**
 * @tc.name: classic_adapter_test_027
 * @tc.desc: ClassicAdapterTest:DisconnectProfiles only connect hfp ag
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_027, TestSize.Level1)
{
    HILOGI("classic_adapter_test_027 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");
    bool isHfpAgConnectCalled = false;

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();
    {
        EXPECT_CALL(*mockHfpAg, GetDeviceState(_))
            .WillOnce(Return(static_cast<int>(BTConnectState::CONNECTED)));
        EXPECT_CALL(*mockHfpAg, Disconnect(_))
            .WillOnce(Invoke([&isHfpAgConnectCalled](const RawAddress &) {
                isHfpAgConnectCalled = true;
                return 0;
            }));
    }

    classicAdapter->DisconnectProfiles(device);
    EXPECT_TRUE(isHfpAgConnectCalled);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_027 end");
}

/**
 * @tc.name: classic_adapter_test_028
 * @tc.desc: ClassicAdapterTest:DisconnectProfiles only connect HID Host
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_028, TestSize.Level1)
{
    HILOGI("classic_adapter_test_028 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");
    bool isHIDHostConnectCalled = false;

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();
    {
        EXPECT_CALL(*mockHidHost, GetDeviceState(_))
            .WillRepeatedly(Return(static_cast<int>(BTConnectState::CONNECTED)));
        EXPECT_CALL(*mockHidHost, Disconnect(_))
            .WillRepeatedly(Invoke([&isHIDHostConnectCalled](const RawAddress &) {
                isHIDHostConnectCalled = true;
                return 0;
            }));
    }

    classicAdapter->DisconnectProfiles(device, false);
    EXPECT_FALSE(isHIDHostConnectCalled);
    classicAdapter->DisconnectProfiles(device, true);
    EXPECT_TRUE(isHIDHostConnectCalled);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_028 end");
}

/**
 * @tc.name: classic_adapter_test_029
 * @tc.desc: ClassicAdapterTest:DisconnectProfiles only connect hearing aid
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_029, TestSize.Level1)
{
    HILOGI("classic_adapter_test_029 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();

    RawAddress device("11:22:33:44:55:66");
    bool isHearingAidConnectCalled = false;

    classicAdapter->agService_ = mockHfpAg.get();
    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();
    classicAdapter->hidHostService_ = mockHidHost.get();
    classicAdapter->hearingAidService_ = mockHearingAid.get();
    {
        EXPECT_CALL(*mockHearingAid, GetDeviceState(_))
            .WillRepeatedly(Return(static_cast<int>(BTConnectState::CONNECTED)));
        EXPECT_CALL(*mockHearingAid, Disconnect(_))
            .WillRepeatedly(Invoke([&isHearingAidConnectCalled](const RawAddress &) {
                isHearingAidConnectCalled = true;
                return 0;
            }));
    }

    classicAdapter->DisconnectProfiles(device, false);
    EXPECT_FALSE(isHearingAidConnectCalled);
    classicAdapter->DisconnectProfiles(device, true);
    EXPECT_TRUE(isHearingAidConnectCalled);

    classicAdapter->agService_ = nullptr;
    classicAdapter->a2dpSrcService_ = nullptr;
    classicAdapter->hidHostService_ = nullptr;
    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_029 end");
}

/**
 * @tc.name: classic_adapter_test_GetProfileSupportState_001
 * @tc.desc: ClassicAdapterTest:GetProfileSupportState returns false for invalid profileId
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_GetProfileSupportState_001, TestSize.Level1)
{
    HILOGI("classic_adapter_test_GetProfileSupportState_001 enter");
    RawAddress device("11:22:33:44:55:66");
    constexpr uint32_t INVALID_PROFILE_ID = 0xFFFFFFFF;
    bool result = classicAdapter->GetProfileSupportState(INVALID_PROFILE_ID, device, true);
    EXPECT_FALSE(result);
    HILOGI("classic_adapter_test_GetProfileSupportState_001 end");
}

/**
 * @tc.name: classic_adapter_test_GetProfileSupportState_002
 * @tc.desc: ClassicAdapterTest:GetProfileSupportState returns false when isConnectMoreProfileEnabled is false
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_GetProfileSupportState_002, TestSize.Level1)
{
    HILOGI("classic_adapter_test_GetProfileSupportState_002 enter");
    RawAddress device("11:22:33:44:55:66");
    bool result = classicAdapter->GetProfileSupportState(PROFILE_ID_HID_HOST, device, true);
    EXPECT_FALSE(result);
    HILOGI("classic_adapter_test_GetProfileSupportState_002 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_001
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns false for unknown profileId
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_001, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_001 enter");
    RawAddress device("11:22:33:44:55:66");
    constexpr uint32_t INVALID_PROFILE_ID = 0xFFFFFFFF;
    bool result = classicAdapter->IsProfileAllowed(INVALID_PROFILE_ID, device);
    EXPECT_FALSE(result);
    HILOGI("classic_adapter_test_IsProfileAllowed_001 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_002
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns true for HFP_AG when strategy is CONNECTION_ALLOWED
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_002, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_002 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    RawAddress device("11:22:33:44:55:66");

    classicAdapter->agService_ = mockHfpAg.get();

    EXPECT_CALL(*mockHfpAg, GetConnectStrategy(_))
        .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)));

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_HFP_AG, device);
    EXPECT_TRUE(result);

    classicAdapter->agService_ = nullptr;
    HILOGI("classic_adapter_test_IsProfileAllowed_002 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_003
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns false for HFP_AG when strategy is not CONNECTION_ALLOWED
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_003, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_003 enter");
    auto mockHfpAg = std::make_unique<MockIProfileHfpAg>();
    RawAddress device("11:22:33:44:55:66");

    classicAdapter->agService_ = mockHfpAg.get();

    EXPECT_CALL(*mockHfpAg, GetConnectStrategy(_))
        .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)));

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_HFP_AG, device);
    EXPECT_FALSE(result);

    classicAdapter->agService_ = nullptr;
    HILOGI("classic_adapter_test_IsProfileAllowed_003 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_004
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns false for HFP_AG when service is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_004, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_004 enter");
    RawAddress device("11:22:33:44:55:66");
    classicAdapter->agService_ = nullptr;

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_HFP_AG, device);
    EXPECT_FALSE(result);

    HILOGI("classic_adapter_test_IsProfileAllowed_004 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_005
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns true for A2DP_SRC when strategy is CONNECTION_ALLOWED
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_005, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_005 enter");
    auto mockA2dpSrc = std::make_unique<MockIProfileA2dpSrc>();
    RawAddress device("11:22:33:44:55:66");

    classicAdapter->a2dpSrcService_ = mockA2dpSrc.get();

    EXPECT_CALL(*mockA2dpSrc, GetConnectStrategy(_))
        .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)));

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_A2DP_SRC, device);
    EXPECT_TRUE(result);

    classicAdapter->a2dpSrcService_ = nullptr;
    HILOGI("classic_adapter_test_IsProfileAllowed_005 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_006
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns false for A2DP_SRC when service is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_006, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_006 enter");
    RawAddress device("11:22:33:44:55:66");
    classicAdapter->a2dpSrcService_ = nullptr;

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_A2DP_SRC, device);
    EXPECT_FALSE(result);

    HILOGI("classic_adapter_test_IsProfileAllowed_006 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_007
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns true for HID_HOST when strategy is CONNECTION_ALLOWED
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_007, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_007 enter");
    auto mockHidHost = std::make_unique<MockIProfileHidHost>();
    RawAddress device("11:22:33:44:55:66");

    classicAdapter->hidHostService_ = mockHidHost.get();

    EXPECT_CALL(*mockHidHost, GetConnectStrategy(_))
        .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)));

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_HID_HOST, device);
    EXPECT_TRUE(result);

    classicAdapter->hidHostService_ = nullptr;
    HILOGI("classic_adapter_test_IsProfileAllowed_007 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_008
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns false for HID_HOST when service is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_008, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_008 enter");
    RawAddress device("11:22:33:44:55:66");
    classicAdapter->hidHostService_ = nullptr;

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_HID_HOST, device);
    EXPECT_FALSE(result);

    HILOGI("classic_adapter_test_IsProfileAllowed_008 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_009
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns true for HEARING_AID when strategy is CONNECTION_ALLOWED
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_009, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_009 enter");
    auto mockHearingAid = std::make_unique<MockIProfileHearingAid>();
    RawAddress device("11:22:33:44:55:66");

    classicAdapter->hearingAidService_ = mockHearingAid.get();

    EXPECT_CALL(*mockHearingAid, GetConnectStrategy(_))
        .WillOnce(Return(static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)));

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_HEARING_AID, device);
    EXPECT_TRUE(result);

    classicAdapter->hearingAidService_ = nullptr;
    HILOGI("classic_adapter_test_IsProfileAllowed_009 end");
}

/**
 * @tc.name: classic_adapter_test_IsProfileAllowed_010
 * @tc.desc: ClassicAdapterTest:IsProfileAllowed returns false for HEARING_AID when service is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_IsProfileAllowed_010, TestSize.Level1)
{
    HILOGI("classic_adapter_test_IsProfileAllowed_010 enter");
    RawAddress device("11:22:33:44:55:66");
    classicAdapter->hearingAidService_ = nullptr;

    bool result = classicAdapter->IsProfileAllowed(PROFILE_ID_HEARING_AID, device);
    EXPECT_FALSE(result);

    HILOGI("classic_adapter_test_IsProfileAllowed_010 end");
}
/**
 * @tc.name: classic_adapter_test_BondStateChangedInner_CloudBondState_001
 * @tc.desc: ClassicAdapterTest:BondStateChangedInner cloud device with CLOUD_BOND_NONE not record chr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_BondStateChangedInner_CloudBondState_001, TestSize.Level1)
{
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudBondState_001 enter");
    RawAddress device = RawAddress("77:11:22:33:44:55");
    STACK::RawAddress bdAddr = ServiceUtil::AddrToStack(device);
    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.SetCloudDevice(device, true);
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_NONE);
    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_bond_state_t state = BT_BOND_STATE_BONDED;
    classicAdapter->BondStateChangedInner(status, bdAddr, state);
    EXPECT_EQ(remoteDevice->GetCloudBondState(), CLOUD_BOND_BONDED);
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudBondState_001 end");
}

/**
 * @tc.name: classic_adapter_test_BondStateChangedInner_CloudBondState_002
 * @tc.desc: ClassicAdapterTest:BondStateChangedInner cloud device with cloudPairInProgress record chr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_BondStateChangedInner_CloudBondState_002, TestSize.Level1)
{
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudBondState_002 enter");
    RawAddress device = RawAddress("77:11:22:33:44:66");
    STACK::RawAddress bdAddr = ServiceUtil::AddrToStack(device);
    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.SetCloudDevice(device, true);
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_BONDING);
    remoteDevice->SetCloudPairInProgress(true);
    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_bond_state_t state = BT_BOND_STATE_BONDED;
    classicAdapter->BondStateChangedInner(status, bdAddr, state);
    EXPECT_EQ(remoteDevice->GetCloudBondState(), CLOUD_BOND_BONDED);
    EXPECT_FALSE(remoteDevice->IsCloudPairInProgress());
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudBondState_002 end");
}

/**
 * @tc.name: classic_adapter_test_BondStateChangedInner_CloudBondState_003
 * @tc.desc: ClassicAdapterTest:BondStateChangedInner non-cloud device does not enter cloud branch
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_BondStateChangedInner_CloudBondState_003, TestSize.Level1)
{
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudBondState_003 enter");
    RawAddress device = RawAddress("77:11:22:33:44:77");
    STACK::RawAddress bdAddr = ServiceUtil::AddrToStack(device);
    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.SetCloudDevice(device, false);
    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_bond_state_t state = BT_BOND_STATE_BONDED;
    classicAdapter->BondStateChangedInner(status, bdAddr, state);
    EXPECT_EQ(remoteDevice->GetCloudBondState(), CLOUD_BOND_NONE);
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudBondState_003 end");
}

/**
 * @tc.name: classic_adapter_test_BondStateChangedInner_CloudPairInProgress_001
 * @tc.desc: ClassicAdapterTest:already cloud paired device startPair should not record cloud pair success chr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_BondStateChangedInner_CloudPairInProgress_001, TestSize.Level1)
{
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudPairInProgress_001 enter");
    RawAddress device = RawAddress("77:11:22:33:44:88");
    STACK::RawAddress bdAddr = ServiceUtil::AddrToStack(device);
    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.SetCloudDevice(device, true);
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_BONDED);
    remoteDevice->SetCloudPairInProgress(false);
    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_bond_state_t state = BT_BOND_STATE_BONDED;
    classicAdapter->BondStateChangedInner(status, bdAddr, state);
    EXPECT_EQ(remoteDevice->GetCloudBondState(), CLOUD_BOND_BONDED);
    EXPECT_FALSE(remoteDevice->IsCloudPairInProgress());
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudPairInProgress_001 end");
}

/**
 * @tc.name: classic_adapter_test_BondStateChangedInner_CloudPairInProgress_002
 * @tc.desc: ClassicAdapterTest:cloud pair in progress device BondStateChangedInner should record chr and reset flag
 * @tc.type: FUNC
 */
HWTEST_F(ClassicAdapterTest, classic_adapter_test_BondStateChangedInner_CloudPairInProgress_002, TestSize.Level1)
{
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudPairInProgress_002 enter");
    RawAddress device = RawAddress("77:11:22:33:44:99");
    STACK::RawAddress bdAddr = ServiceUtil::AddrToStack(device);
    remoteDeviceProperties.FindRemoteDevice(device);
    auto remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDeviceProperties.SetCloudDevice(device, true);
    remoteDeviceProperties.SetCloudBondState(device, CLOUD_BOND_BONDED);
    remoteDevice->SetCloudPairInProgress(true);
    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_bond_state_t state = BT_BOND_STATE_BONDED;
    classicAdapter->BondStateChangedInner(status, bdAddr, state);
    EXPECT_EQ(remoteDevice->GetCloudBondState(), CLOUD_BOND_BONDED);
    EXPECT_FALSE(remoteDevice->IsCloudPairInProgress());
    HILOGI("classic_adapter_test_BondStateChangedInner_CloudPairInProgress_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS