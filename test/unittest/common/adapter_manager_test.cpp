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
constexpr int TIME_1S = 1;
constexpr int DELAY_MS = 200;
AdapterManager *g_adapterManager = nullptr;
STACK::RawAddress addr0;
STACK::RawAddress addr1({0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

class ClassicAdapterStateObserverTest : public IAdapterStateObserver {
public:
    void OnStateChange(const BTTransport transport, const BTStateID state) override
    {}
    void OnBluetoothStateChanged(int state) override
    {}
    void OnRefusePolicyChanged(const int32_t pid, const int64_t prohibitedSecondsTime) override
    {}
};

class ClassicSystemStateObserverTest : public ISystemStateObserver {
public:
    void OnSystemStateChange(const BTSystemState state) override
    {}
};

static void *BleAdapterCreate(void)
{
    return new BleAdapter();
}

static void *ClassicAdapterCreate(void)
{
    return new ClassicAdapter();
}

class AdapterManagerTest : public testing::Test {
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
        g_adapterManager = new AdapterManager();
        ThreadUtil::GetInstance().ChangeThreadState(
            ThreadId::THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::DISABLED);
        ThreadUtil::GetInstance().ChangeThreadState(ThreadId::THREAD_ID_CLASSIC, ThreadUtil::ThreadState::DISABLED);
        ThreadUtil::GetInstance().ChangeThreadState(ThreadId::THREAD_ID_BLE, ThreadUtil::ThreadState::DISABLED);
        g_adapterManager->InitializeAfterAllDependencyOn();
    }
    void TearDown()
    {
        sleep(TIME_1S);
        delete g_adapterManager;
        g_adapterManager = nullptr;
    }
};

/*
 * @tc.number:AdapterManager_UnitTest_AdapterStateChangedCb_001
 * @tc.name: AdapterStateChangedCb
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_AdapterStateChangedCb_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_AdapterStateChangedCb_001 enter");
    bt_state_t state = BT_STATE_OFF;
    g_adapterManager->AdapterStateChangedCb(state);
    EXPECT_EQ(g_adapterManager->GetState(BTTransport::ADAPTER_BREDR), BTStateID::STATE_TURN_OFF);
    sleep(TIME);
    state = BT_STATE_ON;
    g_adapterManager->AdapterStateChangedCb(state);
    HILOGI("AdapterManager_UnitTest_AdapterStateChangedCb_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_DeviceFoundCb_001
 * @tc.name: DeviceFoundCb
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_DeviceFoundCb_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_DeviceFoundCb_001 enter");
    int numProperties = 1;
    bt_property_t properties[1];
    char name[] = "test";
    properties[0].type = static_cast<bt_property_type_t>(BT_PROPERTY_BDNAME);
    properties[0].val = name;
    properties[0].len = strlen(name);

    g_adapterManager->DeviceFoundCb(numProperties, properties);
    EXPECT_EQ(g_adapterManager->GetState(BTTransport::ADAPTER_BREDR), BTStateID::STATE_TURN_OFF);
    HILOGI("AdapterManager_UnitTest_DeviceFoundCb_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_StateChangedCb_001
 * @tc.name: StateChangedCb
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_StateChangedCb_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_StateChangedCb_001 enter");
    bt_discovery_state_t discoveryState = BT_DISCOVERY_STOPPED;
    g_adapterManager->DiscoveryStateChangedCb(discoveryState);
    bt_status_t status = BT_STATUS_BUSY;
    bt_bond_state_t bondState = BT_BOND_STATE_BONDED;
    g_adapterManager->BondStateChangedCb(status, &addr0, bondState);
    bt_bdname_t *bd_name = (bt_bdname_t *)malloc(sizeof(bt_bdname_t));
    uint32_t cod = 12;
    bt_ssp_variant_t pairingVariant = BT_SSP_VARIANT_CONSENT;
    uint32_t passKey = 123;
    g_adapterManager->SspRequestCb(&addr0, bd_name, cod, pairingVariant, passKey);

    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    EXPECT_TRUE(AdapterManager::GetInstance()->GetClassicAdapter() != nullptr);
    g_adapterManager->DiscoveryStateChangedCb(discoveryState);
    g_adapterManager->DiscoveryStateChangedCb(static_cast<bt_discovery_state_t>(2));
    g_adapterManager->BondStateChangedCb(status, &addr0, bondState);
    g_adapterManager->SspRequestCb(&addr0, bd_name, cod, pairingVariant, passKey);
    free(bd_name);
    HILOGI("AdapterManager_UnitTest_StateChangedCb_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_StateChangedCb_002
 * @tc.name: AclStateChangedCb
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_StateChangedCb_002, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_StateChangedCb_002 enter");
    bt_status_t status = BT_STATUS_BUSY;
    bt_acl_state_t state = BT_ACL_STATE_CONNECTED;
    bt_hci_error_code_t hci_reason = 0;
    g_adapterManager->AclStateChangedCb(status, &addr0, state, hci_reason, 0);

    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    EXPECT_TRUE(AdapterManager::GetInstance()->GetClassicAdapter() != nullptr);
    g_adapterManager->AclStateChangedCb(status, &addr0, state, hci_reason, 0);
    g_adapterManager->AclStateChangedCb(status, &addr1, state, hci_reason, 2);
    status = BT_STATUS_SUCCESS;
    g_adapterManager->AclStateChangedCb(status, &addr1, state, hci_reason, 1);
    g_adapterManager->AclStateChangedCb(status, &addr1, state, hci_reason, 2);
    state = BT_ACL_STATE_DISCONNECTED;
    g_adapterManager->AclStateChangedCb(status, &addr1, state, hci_reason, 1);
    g_adapterManager->AclStateChangedCb(status, &addr1, state, hci_reason, 2);
    HILOGI("AdapterManager_UnitTest_StateChangedCb_002 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_StateChangedCb_003
 * @tc.name: RemoteDevicePropertiesCb
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_StateChangedCb_003, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_StateChangedCb_003 enter");
    bt_status_t status = BT_STATUS_BUSY;
    int numProperties = 1;
    bt_property_t *properties = (bt_property_t *)malloc(sizeof(bt_property_t));
    g_adapterManager->RemoteDevicePropertiesCb(status, &addr0, numProperties, properties);
    uint8_t eventId = SPP_CLIENT_CONNECT_DONE;
    std::shared_ptr<bt_sensing_info_t> info = std::make_shared<bt_sensing_info_t>();
    g_adapterManager->SensingStateChangedCb(eventId, info.get());
    EXPECT_EQ(g_adapterManager->GetState(BTTransport::ADAPTER_BREDR), BTStateID::STATE_TURN_OFF);
    free(properties);
    HILOGI("AdapterManager_UnitTest_StateChangedCb_003 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_StateChangedCb_004
 * @tc.name: PinRequestCb
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_StateChangedCb_004, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_StateChangedCb_004 enter");
    bt_bdname_t *bdName = (bt_bdname_t *)malloc(sizeof(bt_bdname_t));
    uint32_t cod = 1;
    bool min16Digit = true;
    g_adapterManager->PinRequestCb(&addr0, bdName, cod, min16Digit);

    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    EXPECT_TRUE(AdapterManager::GetInstance()->GetClassicAdapter() != nullptr);
    g_adapterManager->PinRequestCb(&addr0, bdName, cod, min16Digit);
    free(bdName);
    HILOGI("AdapterManager_UnitTest_StateChangedCb_004 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_ReleaseWakeLock_001
 * @tc.name: ReleaseWakeLock
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_ReleaseWakeLock_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_ReleaseWakeLock_001 enter");
    char *lockName = new char[10];
    EXPECT_EQ(g_adapterManager->ReleaseWakeLock(lockName), BT_STATUS_SUCCESS);
    free(lockName);
    HILOGI("AdapterManager_UnitTest_ReleaseWakeLock_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_StackInit_001
 * @tc.name: StackInit
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_StackInit_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_StackInit_001 enter");
    EXPECT_EQ(g_adapterManager->StackInit(), BT_STATUS_SUCCESS);
    HILOGI("AdapterManager_UnitTest_StackInit_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_EnableBluetoothFromOffToRestrictMode_001
 * @tc.name: EnableBluetoothFromOffToRestrictMode
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_EnableBluetoothFromOffToRestrictMode_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_EnableBluetoothFromOffToRestrictMode_001 enter");
    const char *callingName = "test";
    int ret = g_adapterManager->EnableBluetoothFromOffToRestrictMode(callingName);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("AdapterManager_UnitTest_EnableBluetoothFromOffToRestrictMode_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_Start_001
 * @tc.name: Start
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_Start_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_Start_001 enter");
    EXPECT_FALSE(g_adapterManager->Start());
    g_adapterManager->Stop();
    HILOGI("AdapterManager_UnitTest_Start_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_AdapterStop_001
 * @tc.name: AdapterStop
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_AdapterStop_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_AdapterStop_001 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    g_adapterManager->CreateAdapters();
    EXPECT_TRUE(g_adapterManager->AdapterStop());
    HILOGI("AdapterManager_UnitTest_AdapterStop_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_NotifyDialogResult_001
 * @tc.name: NotifyDialogResult
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_NotifyDialogResult_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_NotifyDialogResult_001 enter");
    uint32_t dialogType = 0;
    bool dialogResult = true;
    EXPECT_EQ(g_adapterManager->NotifyDialogResult(dialogType, dialogResult), BT_NO_ERROR);
    HILOGI("AdapterManager_UnitTest_NotifyDialogResult_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_EnableInner_001
 * @tc.name: EnableInner
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_EnableInner_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_EnableInner_001 enter");
    std::string callingName = "com.test.callingName";
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_EQ(g_adapterManager->EnableInner(callingName, transport, true, true), BT_NO_ERROR);
    sleep(TIME);
    transport = ADAPTER_BLE;
    EXPECT_EQ(g_adapterManager->EnableInner(callingName, transport, true, true), BT_NO_ERROR);
    HILOGI("AdapterManager_UnitTest_EnableInner_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_Enable_001
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_Enable_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_Enable_001 enter");
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_EQ(g_adapterManager->Enable(transport), BT_ERR_INTERNAL_ERROR);
    sleep(TIME);
    transport = ADAPTER_BLE;
    EXPECT_EQ(g_adapterManager->Enable(transport), BT_ERR_INTERNAL_ERROR);
    HILOGI("AdapterManager_UnitTest_Enable_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_DisableInner_001
 * @tc.name: DisableInner
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_DisableInner_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_DisableInner_001 enter");
    std::string callingName = "com.test.callingName";
    BTTransport transport = ADAPTER_BREDR;
    EXPECT_EQ(g_adapterManager->DisableInner(transport, true, callingName), BT_NO_ERROR);
    sleep(TIME);
    transport = ADAPTER_BLE;
    EXPECT_EQ(g_adapterManager->DisableInner(transport, true, callingName), BT_NO_ERROR);
    HILOGI("AdapterManager_UnitTest_DisableInner_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_Disable_001
 * @tc.name: Disable
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_Disable_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_Disable_001 enter");
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    g_adapterManager->Disable(transport);
    sleep(TIME);
    transport = BTTransport::ADAPTER_BLE;
    g_adapterManager->Disable(transport);
    sleep(TIME);
    g_adapterManager->Enable(transport);
    sleep(TIME);
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    transport = BTTransport::ADAPTER_BREDR;
    g_adapterManager->Disable(transport);
    EXPECT_TRUE(connectionManager->bleLinks_.Empty());
    EXPECT_TRUE(connectionManager->brLinks_.Empty());
    HILOGI("AdapterManager_UnitTest_Disable_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_Disable_002
 * @tc.name: Disable
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_Disable_002, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_Disable_002 enter");
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    bool isAsync = true;
    std::string callingName = "com.test.callingName";
    bool isUserTriggered = true;
    system::SetParameter("persist.edm.force_enable_bluetooth", "true");
    EXPECT_TRUE(system::GetBoolParameter("persist.edm.force_enable_bluetooth", false));
    int ret = g_adapterManager->Disable(transport, isAsync, callingName, isUserTriggered);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    system::SetParameter("persist.edm.force_enable_bluetooth", "false");
    HILOGI("AdapterManager_UnitTest_Disable_002 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_EnableBluetoothToRestrictMode_001
 * @tc.name: EnableBluetoothToRestrictMode
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_EnableBluetoothToRestrictMode_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_EnableBluetoothToRestrictMode_001 enter");
    std::string callingName = "com.test.callingName";
    bool isUserTriggered = true;
    system::SetParameter("persist.edm.force_enable_bluetooth", "true");
    EXPECT_TRUE(system::GetBoolParameter("persist.edm.force_enable_bluetooth", false));
    int ret = g_adapterManager->EnableBluetoothToRestrictMode(callingName, isUserTriggered);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    system::SetParameter("persist.edm.force_enable_bluetooth", "false");
    HILOGI("AdapterManager_UnitTest_EnableBluetoothToRestrictMode_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_FactoryReset_001
 * @tc.name: FactoryReset
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_FactoryReset_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_FactoryReset_001 enter");
    EXPECT_FALSE(g_adapterManager->FactoryReset());
    HILOGI("AdapterManager_UnitTest_FactoryReset_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_ClearAllStorage_001
 * @tc.name: ClearAllStorage
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_ClearAllStorage_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_ClearAllStorage_001 enter");
    EXPECT_TRUE(g_adapterManager->ClearAllStorage());
    HILOGI("AdapterManager_UnitTest_ClearAllStorage_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_GetState_001
 * @tc.name: GetState
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_GetState_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_GetState_001 enter");
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    EXPECT_EQ(g_adapterManager->GetState(transport), BTStateID::STATE_TURN_OFF);

    transport = BTTransport::ADAPTER_BLE;
    EXPECT_EQ(g_adapterManager->GetState(transport), BTStateID::STATE_TURN_OFF);
    HILOGI("AdapterManager_UnitTest_GetState_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_DeregisterStateObserver_001
 * @tc.name: DeregisterStateObserver
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_DeregisterStateObserver_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_DeregisterStateObserver_001 enter");
    ClassicAdapterStateObserverTest observer;
    EXPECT_FALSE(g_adapterManager->DeregisterStateObserver(observer));
    HILOGI("AdapterManager_UnitTest_DeregisterStateObserver_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_DeregisterSystemStateObserver_001
 * @tc.name: DeregisterSystemStateObserver
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_DeregisterSystemStateObserver_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_DeregisterSystemStateObserver_001 enter");
    ClassicSystemStateObserverTest observer;
    EXPECT_FALSE(g_adapterManager->DeregisterSystemStateObserver(observer));
    HILOGI("AdapterManager_UnitTest_DeregisterSystemStateObserver_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_GetAdapter_001
 * @tc.name: GetAdapter
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_GetAdapter_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_GetAdapter_001 enter");
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    g_adapterManager->CreateAdapters();
    EXPECT_TRUE(g_adapterManager->GetClassicAdapterInterface() != nullptr);
    EXPECT_TRUE(g_adapterManager->GetBleAdapterInterface() != nullptr);
    HILOGI("AdapterManager_UnitTest_GetAdapter_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_OnSysStateChange_001
 * @tc.name: OnSysStateChange
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_OnSysStateChange_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_OnSysStateChange_001 enter");
    std::string state = "Started";
    g_adapterManager->OnSysStateChange(state);
    EXPECT_TRUE(g_adapterManager->IsBluetoothSysStateStarted());
    HILOGI("AdapterManager_UnitTest_OnSysStateChange_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_OnSysStateExit_001
 * @tc.name: OnSysStateExit
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_OnSysStateExit_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_OnSysStateExit_001 enter");
    std::string state = "FactoryResetting";
    g_adapterManager->OnSysStateExit(state);
    sleep(TIME);
    std::string state1 = "Stopping";
    g_adapterManager->OnSysStateExit(state1);
    sleep(TIME);
    std::string state2 = "Started";
    g_adapterManager->OnSysStateExit(state2);
    EXPECT_FALSE(g_adapterManager->IsBluetoothSysStateStarted());
    HILOGI("AdapterManager_UnitTest_OnSysStateExit_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_OnAdapterStateChange_001
 * @tc.name: OnAdapterStateChange
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_OnAdapterStateChange_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_OnAdapterStateChange_001 enter");
    BTTransport transport = BTTransport::ADAPTER_BLE;
    BTStateID state = STATE_TURN_ON;
    g_adapterManager->OnAdapterStateChange(transport, state);
    EXPECT_FALSE(g_adapterManager->IsBluetoothSysStateStarted());
    HILOGI("AdapterManager_UnitTest_OnAdapterStateChange_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_IsBluetoothSwitchEnableFromSystemParameter_001
 * @tc.name: IsBluetoothSwitchEnableFromSystemParameter
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_IsBluetoothSwitchEnableFromSystemParameter_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_IsBluetoothSwitchEnableFromSystemParameter_001 enter");
    EXPECT_FALSE(g_adapterManager->IsBluetoothSwitchEnableFromSystemParameter());
    HILOGI("AdapterManager_UnitTest_IsBluetoothSwitchEnableFromSystemParameter_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_UnLoadBluetoothSystemAbility_001
 * @tc.name: UnLoadBluetoothSystemAbility
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_UnLoadBluetoothSystemAbility_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_UnLoadBluetoothSystemAbility_001 enter");
    BTTransport transport = BTTransport::ADAPTER_BLE;
    BTStateID state = STATE_TURN_ON;
    g_adapterManager->UnLoadBluetoothSystemAbility(transport, state);
    state = STATE_TURN_OFF;
    g_adapterManager->UnLoadBluetoothSystemAbility(transport, state);
    EXPECT_FALSE(g_adapterManager->IsBluetoothSysStateStarted());
    HILOGI("AdapterManager_UnitTest_UnLoadBluetoothSystemAbility_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_UpdateBluetoothSwitchStatus_001
 * @tc.name: UpdateBluetoothSwitchStatus
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_UpdateBluetoothSwitchStatus_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_UpdateBluetoothSwitchStatus_001 enter");
    BTTransport transport = ADAPTER_BLE;
    BTStateID state = BTStateID::STATE_TURN_OFF;
    g_adapterManager->UpdateBluetoothSwitchStatus(transport, state);
    state = BTStateID::STATE_TURN_ON;
    g_adapterManager->UpdateBluetoothSwitchStatus(transport, state);
    EXPECT_FALSE(g_adapterManager->IsBluetoothSysStateStarted());
    HILOGI("AdapterManager_UnitTest_UpdateBluetoothSwitchStatus_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_RemoveDeviceProfileConfig_001
 * @tc.name: RemoveDeviceProfileConfig
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_RemoveDeviceProfileConfig_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_RemoveDeviceProfileConfig_001 enter");
    std::vector<RawAddress> devices;
    devices.push_back(RawAddress("00:00:00:00:00:01"));
    devices.push_back(RawAddress("00:00:00:00:00:02"));
    g_adapterManager->RemoveDeviceProfileConfig(devices);
    EXPECT_FALSE(g_adapterManager->IsBluetoothSysStateStarted());
    HILOGI("AdapterManager_UnitTest_RemoveDeviceProfileConfig_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_GetMaxNumConnectedAudioDevices_001
 * @tc.name: GetMaxNumConnectedAudioDevices
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_GetMaxNumConnectedAudioDevices_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_GetMaxNumConnectedAudioDevices_001 enter");
    EXPECT_EQ(g_adapterManager->GetMaxNumConnectedAudioDevices(), 6);
    HILOGI("AdapterManager_UnitTest_GetMaxNumConnectedAudioDevices_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_PhonebookPermission_001
 * @tc.name: PhonebookPermission
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_PhonebookPermission_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_PhonebookPermission_001 enter");
    std::string address = "00:11:22:33:44:55:66";
    BTPermissionType permission = BTPermissionType::ACCESS_UNKNOWN;
    g_adapterManager->SetPhonebookPermission(address, permission);
    EXPECT_EQ(g_adapterManager->GetPhonebookPermission(address), BTPermissionType::ACCESS_UNKNOWN);
    permission = BTPermissionType::ACCESS_ALLOWED;
    EXPECT_TRUE(g_adapterManager->SetPhonebookPermission(address, permission));
    EXPECT_EQ(g_adapterManager->GetPhonebookPermission(address), BTPermissionType::ACCESS_ALLOWED);
    permission = BTPermissionType::ACCESS_FORBIDDEN;
    EXPECT_TRUE(g_adapterManager->SetPhonebookPermission(address, permission));
    EXPECT_EQ(g_adapterManager->GetPhonebookPermission(address), BTPermissionType::ACCESS_FORBIDDEN);
    EXPECT_FALSE(g_adapterManager->SetPhonebookPermission(address, static_cast<BTPermissionType>(3)));
    HILOGI("AdapterManager_UnitTest_PhonebookPermission_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_GetPowerMode_001
 * @tc.name: GetPowerMode
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_GetPowerMode_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_GetPowerMode_001 enter");
    std::string address = "11:22:33:44:55:66";
    EXPECT_EQ(g_adapterManager->GetPowerMode(address), 0);
    HILOGI("AdapterManager_UnitTest_GetPowerMode_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_UnloadBluetoothSaTimer_001
 * @tc.name: UnloadBluetoothSaTimer
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_UnloadBluetoothSaTimer_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_UnloadBluetoothSaTimer_001 enter");
    g_adapterManager->StartUnloadBluetoothSaTimer();
    EXPECT_TRUE(g_adapterManager->IsUnloadBluetoothSaTimerStarted());
    g_adapterManager->UnloadBluetoothSaTimerCallback();
    HILOGI("AdapterManager_UnitTest_UnloadBluetoothSaTimer_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_UpdateBluetoothState_001
 * @tc.name: UpdateBluetoothState
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_UpdateBluetoothState_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_UpdateBluetoothState_001 enter");
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    BTStateID state = STATE_TURN_OFF;
    EXPECT_FALSE(g_adapterManager->UpdateBluetoothState(transport, state));
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    g_adapterManager->CreateAdapters();
    EXPECT_FALSE(g_adapterManager->UpdateBluetoothState(transport, state));
    transport = BTTransport::ADAPTER_BLE;
    EXPECT_FALSE(g_adapterManager->UpdateBluetoothState(transport, state));
    HILOGI("AdapterManager_UnitTest_UpdateBluetoothState_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_EnableHisiPcm_001
 * @tc.name: EnableHisiPcm
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_EnableHisiPcm_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_EnableHisiPcm_001 enter");
    bool enable = true;
    g_adapterManager->EnableHisiPcm(enable);

    bool disable = false;
    g_adapterManager->EnableHisiPcm(disable);
    EXPECT_FALSE(g_adapterManager->IsBluetoothSysStateStarted());
    HILOGI("AdapterManager_UnitTest_EnableHisiPcm_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_PostAutoConnectTask_001
 * @tc.name: PostAutoConnectTask
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_PostAutoConnectTask_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_PostAutoConnectTask_001 enter");
#ifdef BLUETOOTH_WATCH_ENABLE
    g_adapterManager->PostAutoConnectTask(DELAY_MS);
#endif
    g_adapterManager->PostAutoConnectTask(DELAY_MS);
    EXPECT_FALSE(g_adapterManager->IsBluetoothSysStateStarted());
    HILOGI("AdapterManager_UnitTest_PostAutoConnectTask_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_GetRestrictedState_001
 * @tc.name: GetRestrictedState
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_GetRestrictedState_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_GetRestrictedState_001 enter");
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    EXPECT_EQ(g_adapterManager->GetRestrictedState(transport), BTStateID::STATE_TURN_OFF);

    transport = BTTransport::ADAPTER_BLE;
    EXPECT_EQ(g_adapterManager->GetRestrictedState(transport), BTStateID::STATE_TURN_OFF);
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    g_adapterManager->CreateAdapters();
    g_adapterManager->UpdateBluetoothState(transport, BTStateID::STATE_TURNING_ON);
    EXPECT_EQ(g_adapterManager->GetRestrictedState(transport), BTStateID::STATE_TURN_OFF);
    g_adapterManager->UpdateBluetoothState(transport, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(g_adapterManager->GetRestrictedState(transport), BTStateID::STATE_TURN_ON);
    HILOGI("AdapterManager_UnitTest_GetRestrictedState_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_IsThirdpartyNeedBluetoothSwitchDialog_001
 * @tc.name: IsThirdpartyNeedBluetoothSwitchDialog
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_IsThirdpartyNeedBluetoothSwitchDialog_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_IsThirdpartyNeedBluetoothSwitchDialog_001 enter");
    g_adapterManager->SetBluetoothRestrictedFlag(true);
    EXPECT_FALSE(g_adapterManager->IsThirdpartyNeedBluetoothSwitchDialog());
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    g_adapterManager->CreateAdapters();
    g_adapterManager->UpdateBluetoothState(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    EXPECT_FALSE(g_adapterManager->IsThirdpartyNeedBluetoothSwitchDialog());
    HILOGI("AdapterManager_UnitTest_IsThirdpartyNeedBluetoothSwitchDialog_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_EnablebluetoothFromRestricted_001
 * @tc.name: EnablebluetoothFromRestricted
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_EnablebluetoothFromRestricted_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_EnablebluetoothFromRestricted_001 enter");
    EXPECT_EQ(g_adapterManager->EnablebluetoothFromRestricted(), BT_NO_ERROR);
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    g_adapterManager->CreateAdapters();
    g_adapterManager->UpdateBluetoothState(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(g_adapterManager->EnablebluetoothFromRestricted(), BT_NO_ERROR);
    HILOGI("AdapterManager_UnitTest_EnablebluetoothFromRestricted_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_SatelliteControl_001
 * @tc.name: SatelliteControl
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_SatelliteControl_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_SatelliteControl_001 enter");
    int state = 0;
    EXPECT_EQ(g_adapterManager->SatelliteControl(0, state), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(g_adapterManager->SatelliteControl(1, state), BT_NO_ERROR);

    state = 1;
    EXPECT_EQ(g_adapterManager->SatelliteControl(0, state), BT_ERR_INTERNAL_ERROR);
    EXPECT_EQ(g_adapterManager->SatelliteControl(1, state), BT_NO_ERROR);
    HILOGI("AdapterManager_UnitTest_SatelliteControl_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_IsBluetoothRestricted_001
 * @tc.name: IsBluetoothRestricted
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_IsBluetoothRestricted_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_IsBluetoothRestricted_001 enter");
    g_adapterManager->SetBluetoothRestrictedFlag(false);
    EXPECT_FALSE(g_adapterManager->IsBluetoothRestricted());
    g_adapterManager->SetBluetoothRestrictedFlag(true);
    EXPECT_TRUE(g_adapterManager->IsBluetoothRestricted());
    HILOGI("AdapterManager_UnitTest_IsBluetoothRestricted_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_IsAdpaterNullptrWhenEnable_001
 * @tc.name: IsAdpaterNullptrWhenEnable
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_IsAdpaterNullptrWhenEnable_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_IsAdpaterNullptrWhenEnable_001 enter");
    EXPECT_TRUE(g_adapterManager->IsAdpaterNullptrWhenEnable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(g_adapterManager->IsAdpaterNullptrWhenEnable(BTTransport::ADAPTER_BLE));
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("BleAdapter", BleAdapterCreate);
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    g_adapterManager->CreateAdapters();
    EXPECT_FALSE(g_adapterManager->IsAdpaterNullptrWhenEnable(BTTransport::ADAPTER_BREDR));
    HILOGI("AdapterManager_UnitTest_IsAdpaterNullptrWhenEnable_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_SetVirtualAutoConnectType_001
 * @tc.name:SetVirtualAutoConnectType
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_SetVirtualAutoConnectType_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_SetVirtualAutoConnectType_001 enter");
    std::string addr("11:22:33:44:55:66");
    int connType = 0;
    int businessType = 4;
    EXPECT_EQ(g_adapterManager->SetVirtualAutoConnectType(addr, connType, businessType), BT_ERR_INVALID_PARAM);
    EXPECT_FALSE(g_adapterManager->IsSupportVirtualAutoConnect(addr));
    connType = 1;
    EXPECT_EQ(g_adapterManager->SetVirtualAutoConnectType(addr, connType, businessType), BT_NO_ERROR);
    businessType = CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG;
    EXPECT_EQ(g_adapterManager->SetVirtualAutoConnectType(addr, connType, businessType), BT_NO_ERROR);
    EXPECT_FALSE(g_adapterManager->IsSupportVirtualAutoConnect(addr));
    HILOGI("AdapterManager_UnitTest_SetVirtualAutoConnectType_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_SetGetQuietMode_001
 * @tc.name: SetGetQuietMode
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_SetGetQuietMode_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_SetGetQuietMode_001 enter");
    g_adapterManager->SetQuietMode(true);
    EXPECT_TRUE(g_adapterManager->GetQuietMode());

    g_adapterManager->SetQuietMode(false);
    EXPECT_FALSE(g_adapterManager->GetQuietMode());
    HILOGI("AdapterManager_UnitTest_SetGetQuietMode_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_SaveConnectionTime_001
 * @tc.name:SaveConnectionTime
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_SaveConnectionTime_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_SaveConnectionTime_001 enter");
    std::string addr = "11:22:33:44:55:66";
	RawAddress device(addr);
    g_adapterManager->SaveConnectionTime(device);
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    g_adapterManager->SaveConnectionTime(device);
    EXPECT_FALSE(g_adapterManager->IsSupportVirtualAutoConnect(addr));
    HILOGI("AdapterManager_UnitTest_SaveConnectionTime_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_CloudBondingState_001
 * @tc.name:CloudBondingState
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_CloudBondingState_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_CloudBondingState_001 enter");
    RawAddress device = RawAddress("66:11:22:33:44:55");
    g_adapterManager->ResetCloudBondingState(device);
    int connectionState = CONNECTION_STATE_CONNECTED;
    g_adapterManager->HandleCloudBondWhenAclStateChange(device, connectionState);
    connectionState = CONNECTION_STATE_DISCONNECTED;
    g_adapterManager->HandleCloudBondWhenAclStateChange(device, connectionState);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    RemoteDeviceProperties::GetInstance()->SetCloudDevice(device, true);
    EXPECT_FALSE(g_adapterManager->StartPairIfInCloudCreateBondState(device));
    RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_BONDING);
    g_adapterManager->ResetCloudBondingState(device);
    RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_CREATE_BOND);
    EXPECT_TRUE(g_adapterManager->StartPairIfInCloudCreateBondState(device));
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    EXPECT_FALSE(g_adapterManager->StartPairIfInCloudCreateBondState(device));
    g_adapterManager->HandleCloudBondWhenAclStateChange(device, connectionState);
    RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_BONDING);
    g_adapterManager->ResetCloudBondingState(device);
    HILOGI("AdapterManager_UnitTest_CloudBondingState_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_ResetCloudBondingState_001
 * @tc.name:ResetCloudBondingState
 * @tc.desc: Test ResetCloudBondingState when cloud bond state is BONDING, should reset to NONE
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_ResetCloudBondingState_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_ResetCloudBondingState_001 enter");
    RawAddress device = RawAddress("88:11:22:33:44:55");
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    RemoteDeviceProperties::GetInstance()->SetCloudDevice(device, true);
    RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_BONDING);
    EXPECT_EQ(RemoteDeviceProperties::GetInstance()->GetCloudBondState(device), CLOUD_BOND_BONDING);
    g_adapterManager->ResetCloudBondingState(device);
    EXPECT_EQ(RemoteDeviceProperties::GetInstance()->GetCloudBondState(device), CLOUD_BOND_NONE);
    HILOGI("AdapterManager_UnitTest_ResetCloudBondingState_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_ResetCloudBondingState_002
 * @tc.name:ResetCloudBondingState
 * @tc.desc: Test ResetCloudBondingState when cloud bond state is not BONDING, should not change state
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_ResetCloudBondingState_002, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_ResetCloudBondingState_002 enter");
    RawAddress device = RawAddress("88:11:22:33:44:66");
    RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    RemoteDeviceProperties::GetInstance()->SetCloudDevice(device, true);
    RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_NONE);
    g_adapterManager->ResetCloudBondingState(device);
    EXPECT_EQ(RemoteDeviceProperties::GetInstance()->GetCloudBondState(device), CLOUD_BOND_NONE);
    RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_CREATE_BOND);
    g_adapterManager->ResetCloudBondingState(device);
    EXPECT_EQ(RemoteDeviceProperties::GetInstance()->GetCloudBondState(device), CLOUD_BOND_CREATE_BOND);
    RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_BONDED);
    g_adapterManager->ResetCloudBondingState(device);
    EXPECT_EQ(RemoteDeviceProperties::GetInstance()->GetCloudBondState(device), CLOUD_BOND_BONDED);
    HILOGI("AdapterManager_UnitTest_ResetCloudBondingState_002 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_HandleAclStateChanged_001
 * @tc.name:HandleAclStateChanged
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_HandleAclStateChanged_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_HandleAclStateChanged_001 enter");
    RawAddress device("11:22:33:44:55:66");
    std::shared_ptr<BluetoothDevice> remoteDev =
        RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(device);
    bt_acl_state_t state = BT_ACL_STATE_CONNECTED;
    EXPECT_EQ(g_adapterManager->HandleAclStateChanged(remoteDev, device, &addr1, state), CONNECTION_STATE_CONNECTED);
    HILOGI("AdapterManager_UnitTest_HandleAclStateChanged_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_GetAdapterConnectState_001
 * @tc.name:GetAdapterConnectState
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_GetAdapterConnectState_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_GetAdapterConnectState_001 enter");
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::DISABLED);
    g_adapterManager->Initialize();
    EXPECT_EQ(g_adapterManager->GetAdapterConnectState(), BTConnectState::DISCONNECTED);
    HILOGI("AdapterManager_UnitTest_GetAdapterConnectState_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_ExecuteTaskWhenBluetoothOn_001
 * @tc.name: ExecuteTaskWhenBluetoothOn
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_ExecuteTaskWhenBluetoothOn_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_ExecuteTaskWhenBluetoothOn_001 enter");
    g_adapterManager->SetBluetoothRestrictedFlag(false);
    EXPECT_FALSE(g_adapterManager->IsBluetoothRestricted());
    g_adapterManager->ExecuteTaskWhenBluetoothOn();
    g_adapterManager->SetBluetoothRestrictedFlag(true);
    EXPECT_TRUE(g_adapterManager->IsBluetoothRestricted());
    g_adapterManager->ExecuteTaskWhenBluetoothOn();
    AdapterConfig::GetInstance()->Load();
    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    g_adapterManager->CreateAdapters();
    std::shared_ptr<IAdapterClassic> classicAdapter = g_adapterManager->GetClassicAdapterInterface();
    g_adapterManager->ExecuteTaskWhenBluetoothOn();
    RawAddress device("11:22:33:44:55:66");
    RemoteDeviceProperties* remoteDeviceProperties = RemoteDeviceProperties::GetInstance();
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties->FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    EXPECT_TRUE(classicAdapter->GetPairedDevices().size() > 0);
    g_adapterManager->ExecuteTaskWhenBluetoothOn();
    HILOGI("AdapterManager_UnitTest_ExecuteTaskWhenBluetoothOn_001 end");
}

/*
 * @tc.number:AdapterManager_UnitTest_GetTmpAdvName_001
 * @tc.name: GetTmpAdvName
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, AdapterManager_UnitTest_GetTmpAdvName_001, TestSize.Level1)
{
    HILOGI("AdapterManager_UnitTest_GetTmpAdvName_001 enter");
    uint32_t tokenId = 123456;
    std::string name = "test";
    EXPECT_EQ(g_adapterManager->SetTmpAdvName(tokenId, name), BT_NO_ERROR);
    std::string temp = g_adapterManager->GetTmpAdvName(tokenId);
    EXPECT_EQ(temp, name);
    g_adapterManager->EraseTmpAdvName(tokenId);
    temp = g_adapterManager->GetTmpAdvName(tokenId);
    EXPECT_EQ(temp, "");
    HILOGI("AdapterManager_UnitTest_GetTmpAdvName_001 end");
}

/*
 * @tc.number:GenerateLocalOobDataCb_001
 * @tc.name: GenerateLocalOobDataCb
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, GenerateLocalOobDataCb_001, TestSize.Level1)
{
    HILOGI("GenerateLocalOobDataCb_001 enter");
    tBT_TRANSPORT transport = BT_TRANSPORT_BR_EDR;
    bt_oob_data_t oobData;
    g_adapterManager->GenerateLocalOobDataCb(transport, oobData);
    EXPECT_FALSE(oobData.is_valid);
    HILOGI("GenerateLocalOobDataCb_001 end");
}

/*
 * @tc.number:GenerateLocalOobDataCb_002
 * @tc.name: GenerateLocalOobDataCb
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, GenerateLocalOobDataCb_002, TestSize.Level1)
{
    HILOGI("GenerateLocalOobDataCb_002 enter");
    tBT_TRANSPORT transport = BT_TRANSPORT_BR_EDR;
    bt_oob_data_t oobData;
    oobData.is_valid = true;
    g_adapterManager->GenerateLocalOobDataCb(transport, oobData);
    EXPECT_TRUE(oobData.is_valid);
    HILOGI("GenerateLocalOobDataCb_002 end");
}

/*
 * @tc.number:BuildBluetoothOobData_001
 * @tc.name: BuildBluetoothOobData
 * @tc.desc:
 */
HWTEST_F(AdapterManagerTest, BuildBluetoothOobData_001, TestSize.Level1)
{
    HILOGI("BuildBluetoothOobData_001 enter");
    std::vector<uint8_t> addressWithType = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01};
    // addressWithType中前6位为小端序的地址，转为大端序
    std::vector<uint8_t> addressWithTypeReversed = {0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x01};
    std::vector<uint8_t> confirmHash = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10};
    std::vector<uint8_t> randomHash = {0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
        0x0F, 0x10, 0x11};
    bt_oob_data_t inData;
    inData.is_valid = true;
    std::copy(addressWithType.begin(), addressWithType.end(), inData.address);
    std::copy(confirmHash.begin(), confirmHash.end(), inData.c);
    std::copy(randomHash.begin(), randomHash.end(), inData.r);
    inData.device_name[0] = 't';
    inData.device_name[1] = 'e';
    inData.device_name[2] = 's';
    inData.device_name[3] = 't';
    inData.device_name[4] = '\0';
    inData.le_device_role = 0x01;
    BluetoothOobData outData = g_adapterManager->BuildBluetoothOobData(inData);

    EXPECT_TRUE(outData.HasOobData());
    EXPECT_EQ(outData.GetRandomizerHash(), randomHash);
    EXPECT_EQ(outData.GetConfirmationHash(), confirmHash);
    EXPECT_EQ(outData.GetAddressWithType(), addressWithTypeReversed);
    EXPECT_EQ(outData.GetDeviceRole(), LeDeviceRole::DEVICE_ROLE_CENTRAL_ONLY);
    EXPECT_EQ(outData.GetDeviceName(), "test");
    HILOGI("BuildBluetoothOobData_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS