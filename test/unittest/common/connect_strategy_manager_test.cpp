/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "adapter_manager.h"
#include "bluetooth_access_token_mock.h"
#include "bluetooth_airplane_manager.h"
#include "bluetooth_connection_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "connect_strategy_manager.h"
#include "profile_service_manager.h"
#include "thread_util.h"
#include "class_creator.h"
#include "profile_config.h"
#include "gavdp/a2dp_service.h"
#include "a2dp_src/a2dp_src_service.h"
#include "profile_service_manager.h"
#include "hfp_ag/hfp_ag_service.h"
#include "syspara/parameters.h"
#include "adapter_config.h"
#include "btcommon/timer_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
ConnectStrategyManager* strategyManager;

static void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

static void *HfpAgServiceCreate(void)
{
    return new HfpAgService();
}

class ConnectStrategyManagerTest : public testing::Test {
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
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
        utility::CloseAllTimer();
    }
    void SetUp()
    {
        strategyManager = ConnectStrategyManager::GetInstance();
        ThreadUtil::GetInstance().ChangeThreadState(
            ThreadId::THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::DISABLED);
        ThreadUtil::GetInstance().ChangeThreadState(ThreadId::THREAD_ID_CLASSIC, ThreadUtil::ThreadState::DISABLED);
        ThreadUtil::GetInstance().ChangeThreadState(ThreadId::THREAD_ID_BLE, ThreadUtil::ThreadState::DISABLED);
    }
    void TearDown()
    {
        sleep(TIME);
    }
};

static void *ClassicAdapterCreate(void)
{
    return new ClassicAdapter();
}

/*
 * @tc.number:StrategyManager_UnitTest_QueryAutoConnectDevice_001
 * @tc.name:QueryAutoConnectDevice
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_QueryAutoConnectDevice_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_QueryAutoConnectDevice_001 enter");
    std::string addr = "22:33:44:55:66:11";
    strategyManager->QueryAutoConnectDevice(addr);
    EXPECT_TRUE(strategyManager->SaveLastConnectedDevice(addr));
    EXPECT_TRUE(strategyManager->QueryLastConnectedDevice(addr));
    EXPECT_TRUE(strategyManager->QueryAutoConnectDevice(addr));
    HILOGI("StrategyManager_UnitTest_QueryAutoConnectDevice_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_AutoConnect_001
 * @tc.name:AutoConnect
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_AutoConnect_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_AutoConnect_001 enter");
    std::string addr = "22:33:44:55:66:11";
    RawAddress device(addr);
    AdapterManager::GetInstance()->SetQuietMode(false);
    EXPECT_FALSE(AdapterManager::GetInstance()->GetQuietMode());
    strategyManager->AutoConnect();
    AdapterManager::GetInstance()->SetQuietMode(true);
    EXPECT_TRUE(AdapterManager::GetInstance()->GetQuietMode());
    strategyManager->AutoConnect();
    EXPECT_TRUE(strategyManager->QueryAutoConnectDevice(addr));
    strategyManager->AutoConnect();
    HILOGI("StrategyManager_UnitTest_AutoConnect_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_AutoConnectProfile_001
 * @tc.name:AutoConnectProfile
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_AutoConnectProfile_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_AutoConnectProfile_001 enter");
    std::string addr = "22:33:44:55:66:11";
    strategyManager->AutoConnectProfile(addr);
    ProfileConfig::GetInstance()->Load();
    ClassFactory::RegisterClass(PROFILE_NAME_A2DP_SRC, A2dpSrcServiceCreate);
    ClassFactory::RegisterClass(PROFILE_NAME_HFP_AG, HfpAgServiceCreate);
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, PROFILE_NAME_A2DP_SRC);
    strategyManager->AutoConnectProfile(addr);
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, PROFILE_NAME_HFP_AG);
    strategyManager->AutoConnectProfile(addr);
    RawAddress device(addr);
    EXPECT_FALSE(strategyManager->IsProfileNeedDelayConnect(device));
    HILOGI("StrategyManager_UnitTest_AutoConnectProfile_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_IsAllProfilesDisconnect_001
 * @tc.name: IsAllProfilesDisconnect
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_IsAllProfilesDisconnect_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_IsAllProfilesDisconnect_001 enter");
    std::string device = "11:22:33:44:55:66";
    EXPECT_FALSE(strategyManager->IsAllProfilesDisconnect(device));
    HILOGI("StrategyManager_UnitTest_IsAllProfilesDisconnect_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_IsProfileConnectFail_001
 * @tc.name:IsProfileConnectFail
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_IsProfileConnectFail_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_IsProfileConnectFail_001 enter");
    int profileStatus = 0;
    bool profileAllowed = false;
    EXPECT_FALSE(strategyManager->IsProfileConnectFail(profileStatus, profileAllowed));

    profileStatus = 0;
    profileAllowed = true;
    EXPECT_FALSE(strategyManager->IsProfileConnectFail(profileStatus, profileAllowed));

    profileStatus = 2900004;
    profileAllowed = true;
    EXPECT_FALSE(strategyManager->IsProfileConnectFail(profileStatus, profileAllowed));

    profileStatus = 1;
    profileAllowed = true;
    EXPECT_TRUE(strategyManager->IsProfileConnectFail(profileStatus, profileAllowed));
    HILOGI("StrategyManager_UnitTest_IsProfileConnectFail_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_InitConnectStrategy_001
 * @tc.name:InitConnectStrategy
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_InitConnectStrategy_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_InitConnectStrategy_001 enter");
    std::string addr;
    strategyManager->InitConnectStrategy(addr);

    addr = "11:22:33:44:55:66";
    RawAddress device(addr);
    ProfileConfig::GetInstance()->Load();
    ClassFactory::RegisterClass(PROFILE_NAME_A2DP_SRC, A2dpSrcServiceCreate);
    ClassFactory::RegisterClass(PROFILE_NAME_HFP_AG, HfpAgServiceCreate);
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, PROFILE_NAME_A2DP_SRC);
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, PROFILE_NAME_HFP_AG);
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(device);
    Uuid uuid_1 = Uuid::ConvertFromString("0000110B-0000-1000-8000-00805F9B34FB");
    Uuid uuid_2 = Uuid::ConvertFromString("0000110D-0000-1000-8000-00805F9B34FB");
    std::vector<Uuid> uuidData {};
    uuidData.push_back(uuid_1);
    uuidData.push_back(uuid_2);
    remoteDevice->SetDeviceUuids(uuidData);
    strategyManager->InitConnectStrategy(addr);
    uuidData.clear();

    uuid_1 = Uuid::ConvertFromString("0000111E-0000-1000-8000-00805F9B34FB");
    uuid_2 = Uuid::ConvertFromString("00001108-0000-1000-8000-00805F9B34FB");
    uuidData.push_back(uuid_1);
    uuidData.push_back(uuid_2);
    remoteDevice->SetDeviceUuids(uuidData);
    strategyManager->InitConnectStrategy(addr);
    uuidData.clear();

    uuid_1 = Uuid::ConvertFromString("00001124-0000-1000-8000-00805F9B34FB");
    uuid_2 = Uuid::ConvertFromString("00001812-0000-1000-8000-00805F9B34FB");
    uuidData.push_back(uuid_1);
    uuidData.push_back(uuid_2);
    remoteDevice->SetDeviceUuids(uuidData);
    strategyManager->InitConnectStrategy(addr);

    std::string profileName = "a2dpService";
    EXPECT_FALSE(strategyManager->StrategyConnectProfile(addr, profileName));
    profileName = "hfpService";
    EXPECT_FALSE(strategyManager->StrategyConnectProfile(addr, profileName));
    profileName = "invalidService";
    EXPECT_FALSE(strategyManager->StrategyConnectProfile(addr, profileName));
    HILOGI("StrategyManager_UnitTest_InitConnectStrategy_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_SaveConnectStrategy_001
 * @tc.name:SaveConnectStrategy
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_SaveConnectStrategy_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_SaveConnectStrategy_001 enter");
    std::string addr;
    RawAddress device_1(addr);
    std::string profile = "A2dpConnectionPolicy";
    int strategy = 0;
    EXPECT_FALSE(strategyManager->SaveConnectStrategy(device_1, profile, strategy));

    addr = "11:22:33:44:55:66";
    RawAddress device_2(addr);
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device_2);
    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(device_2);
    EXPECT_FALSE(strategyManager->SaveConnectStrategy(device_2, profile, strategy));
    strategy = 1;
    EXPECT_TRUE(strategyManager->SaveConnectStrategy(device_2, profile, strategy));
    remoteDevice->SetPairedStatus(2);
    EXPECT_TRUE(strategyManager->SaveConnectStrategy(device_2, profile, strategy));
    strategy = 0;
    EXPECT_FALSE(strategyManager->SaveConnectStrategy(device_2, profile, strategy));
    strategyManager->StrategyHandleProfileWaitConnect(addr, profile);
    strategyManager->ProfileConnectionStateChange(addr, profile);

    addr = "12:23:34:45:56:67";
    RawAddress device_3(addr);
    profile = "HfpConnectionPolicy";
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device_3);
    std::shared_ptr<BluetoothDevice> remoteDevice_3 =
        RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(device_3);
    remoteDevice_3->SetPairedStatus(2);
    strategy = 1;
    EXPECT_TRUE(strategyManager->SaveConnectStrategy(device_3, profile, strategy));

    strategy = 2;
    EXPECT_TRUE(strategyManager->SaveConnectStrategy(device_3, profile, strategy));

    strategy = 3;
    EXPECT_FALSE(strategyManager->SaveConnectStrategy(device_3, profile, strategy));

    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    profile = "hfpService";
    strategyManager->StrategyHandleProfileWaitConnect(addr, profile);
    strategyManager->ProfileConnectionStateChange(addr, profile);
    EXPECT_TRUE(AdapterManager::GetInstance()->GetClassicAdapter() != nullptr);
    HILOGI("StrategyManager_UnitTest_SaveConnectStrategy_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_SaveLastActiveDevice_001
 * @tc.name:SaveLastActiveDevice
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_SaveLastActiveDevice_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_SaveLastActiveDevice_001 enter");
    std::string addr("11:22:33:44:55:66");
    EXPECT_TRUE(strategyManager->SaveLastActiveDevice(addr));
    EXPECT_TRUE(strategyManager->QueryLastActiveDevice(addr));
    HILOGI("StrategyManager_UnitTest_SaveLastActiveDevice_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_UpdateAutoConnectDeivce_001
 * @tc.name:UpdateAutoConnectDeivce
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_UpdateAutoConnectDeivce_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_UpdateAutoConnectDeivce_001 enter");
    RemoteDeviceProperties* remoteDeviceProperties = RemoteDeviceProperties::GetInstance();
    remoteDeviceProperties->FreeMemory();
    std::string addr("11:22:33:44:55:66");
    strategyManager->UpdateAutoConnectDeivce(addr);

    RawAddress device(addr);
    remoteDeviceProperties->FindRemoteDevice(device);
    strategyManager->UpdateAutoConnectDeivce(addr);
    std::string address("11:22:33:44:55:77");
    RawAddress device2(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties->FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice2 = remoteDeviceProperties->FindRemoteDevice(device2);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    AdapterManager::GetInstance()->CreateAdapters();
    std::shared_ptr<IAdapterClassic> classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    EXPECT_TRUE(classicAdapter->GetPairedDevices().size() > 0);
    strategyManager->UpdateAutoConnectDeivce(addr);
    strategyManager->SaveLastActiveDevice(addr);
    strategyManager->UpdateAutoConnectDeivce(addr);
    ProfileConfig::GetInstance()->Load();
    ClassFactory::RegisterClass(PROFILE_NAME_A2DP_SRC, A2dpSrcServiceCreate);
    ClassFactory::RegisterClass(PROFILE_NAME_HFP_AG, HfpAgServiceCreate);
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, PROFILE_NAME_A2DP_SRC);
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, PROFILE_NAME_HFP_AG);
    HfpAgService *hfpAgService = HfpAgService::GetService();
    hfpAgService->UpdateActiveDevice(address);
    strategyManager->UpdateAutoConnectDeivce(addr);
    strategyManager->SaveLastConnectedDevice(addr);
    strategyManager->UpdateAutoConnectDeivce(addr);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    std::shared_ptr<A2dpDeviceInfo> info = std::make_shared<A2dpDeviceInfo>(device);
    a2dpService->AddDeviceToList(address, info);
    strategyManager->UpdateAutoConnectDeivce(addr);
    Uuid uuid_1 = Uuid::ConvertFromString("0000110B-0000-1000-8000-00805F9B34FB");
    Uuid uuid_2 = Uuid::ConvertFromString("0000110D-0000-1000-8000-00805F9B34FB");
    std::vector<Uuid> uuidData {};
    uuidData.push_back(uuid_1);
    uuidData.push_back(uuid_2);
    remoteDevice2->SetDeviceUuids(uuidData);
    strategyManager->UpdateAutoConnectDeivce(addr);
    uuidData.clear();
    uuid_1 = Uuid::ConvertFromString("0000111E-0000-1000-8000-00805F9B34FB");
    uuid_2 = Uuid::ConvertFromString("00001108-0000-1000-8000-00805F9B34FB");
    uuidData.push_back(uuid_1);
    uuidData.push_back(uuid_2);
    remoteDevice2->SetDeviceUuids(uuidData);
    strategyManager->UpdateAutoConnectDeivce(addr);
    uuidData.clear();

    uuid_1 = Uuid::ConvertFromString("00001124-0000-1000-8000-00805F9B34FB");
    uuid_2 = Uuid::ConvertFromString("00001812-0000-1000-8000-00805F9B34FB");
    uuidData.push_back(uuid_1);
    uuidData.push_back(uuid_2);
    remoteDevice2->SetDeviceUuids(uuidData);
    strategyManager->UpdateAutoConnectDeivce(addr);
    uuidData.clear();

    uuid_1 = Uuid::ConvertFromString("0000FDF0-0000-1000-8000-00805F9B34FB");
    uuidData.push_back(uuid_1);
    remoteDevice2->SetDeviceUuids(uuidData);
    strategyManager->UpdateAutoConnectDeivce(addr);
    EXPECT_TRUE(strategyManager->IsAutoConnectingAllowed(device));
    strategyManager->EraseDevicePreviouslyConnected(addr);
    HILOGI("StrategyManager_UnitTest_UpdateAutoConnectDeivce_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_PublishAutoConnDevice_001
 * @tc.name:PublishAutoConnDevice
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_PublishAutoConnDevice_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_PublishAutoConnDevice_001 enter");
    EXPECT_TRUE(strategyManager->PublishAutoConnDevice(strategyManager->QueryLastActiveDevice, "lastActiveDevice"));
    EXPECT_TRUE(strategyManager->PublishAutoConnDevice(strategyManager->QueryLastConnectedDevice,
        "lastConnectedDevice"));
    HILOGI("StrategyManager_UnitTest_PublishAutoConnDevice_001 end");
}

/*
 * @tc.number:StrategyManager_UnitTest_ReportAutoConnectResultChr_001
 * @tc.name:ReportAutoConnectResultChr
 * @tc.desc:
 */
HWTEST_F(ConnectStrategyManagerTest, StrategyManager_UnitTest_ReportAutoConnectResultChr_001, TestSize.Level1)
{
    HILOGI("StrategyManager_UnitTest_ReportAutoConnectResultChr_001 enter");
    strategyManager->ReportAutoConnectResultChr("00:00:00:00:00:00", true, true, true);
    strategyManager->ReportAutoConnectResultChr("00:00:00:00:00:00", true, true, false);
    strategyManager->ReportAutoConnectResultChr("00:00:00:00:00:00", true, false, true);
    strategyManager->ReportAutoConnectResultChr("00:00:00:00:00:00", false, true, true);
    strategyManager->ReportAutoConnectResultChr("00:00:00:00:00:00", true, false, false);
    strategyManager->ReportAutoConnectResultChr("00:00:00:00:00:00", false, false, true);
    strategyManager->ReportAutoConnectResultChr("00:00:00:00:00:00", false, true, false);
    strategyManager->ReportAutoConnectResultChr("00:00:00:00:00:00", false, false, false);
    EXPECT_FALSE(strategyManager->IsProfileConnectFail(0, false));
    HILOGI("StrategyManager_UnitTest_ReportAutoConnectResultChr_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS