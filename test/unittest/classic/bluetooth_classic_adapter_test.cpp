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
#include "classic_adapter.h"
#include "power_device.h"
#include "class_creator.h"
#include "adapter_config.h"
#include "ble_adapter.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "interface_adapter_classic.h"
#include "profile_service_manager.h"
#include "hfp_ag_service.h"
#include "a2dp_service.h"
#include "a2dp_src/a2dp_src_service.h"
#include "hid_host_service.h"
#include "connect_strategy_manager.h"
#include "profile_config.h"
#include "bluetooth_remote_device_info.h"
#include "bluetooth_hw_interface.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_access_token_mock.h"
#include "directory_ex.h"

using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
ClassicAdapter *classicAdapter = nullptr;
RemoteDeviceProperties remoteDeviceProperties;
AdapterProperties adapterProperties;
const int BT_SCAN_DURATION_MS = 120000;

void *HfpAgServiceCreate(void)
{
    return new HfpAgService();
}

void *A2dpSrcServiceCreate(void)
{
    return new A2dpSrcService();
}

extern "C" {
static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

static bool HwGetDeviceInfoMock(const STACK::RawAddress &addr, bt_property_t &prop)
{
    return true;
}

static void HwBtsndHcicDisconnectMock(const STACK::RawAddress &bdAddr)
{
    HILOGI("enter");
}

static bool IsIncomingConnectionMock(const STACK::RawAddress &bdAddr)
{
    HILOGI("enter");
    return true;
}

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwGetDeviceInfo = HwGetDeviceInfoMock,
    .hwBtsndHcicDisconnect = HwBtsndHcicDisconnectMock,
    .isIncomingConnection = IsIncomingConnectionMock,
};
}

class BluetoothClassicAdapterTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        BluetoothAccessTokenMock::SetNativeTokenInfo();
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        HILOGI("isBleEnable = %{public}d", isBleEnable);

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
public:
};

/**
 * @tc.name: classic_adapter_test_004
 * @tc.desc: BluetoothClassicAdapterTest:SetBtScanMode
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_004, TestSize.Level1)
{
    HILOGI("classic_adapter_test_004 enter");
    int modeFalse = 0x06;
    int durationFalse = -1;
    EXPECT_FALSE(classicAdapter->SetBtScanMode(modeFalse, durationFalse));
    HILOGI("classic_adapter_test_004 end");
}

/**
 * @tc.name: classic_adapter_test_005
 * @tc.desc: BluetoothClassicAdapterTest:SetBtScanMode
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_005, TestSize.Level1)
{
    HILOGI("classic_adapter_test_005 enter");
    int mode = SCAN_MODE_GENERAL_DISCOVERABLE;
    int durationFalse = -1;
    EXPECT_FALSE(classicAdapter->SetBtScanMode(mode, durationFalse));
    HILOGI("classic_adapter_test_005 end");
}

/**
 * @tc.name: classic_adapter_test_006
 * @tc.desc: BluetoothClassicAdapterTest:SetBtScanMode
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_006, TestSize.Level1)
{
    HILOGI("classic_adapter_test_006 enter");
    int mode = SCAN_MODE_GENERAL_DISCOVERABLE;
    int duration = 1000;
    EXPECT_FALSE(classicAdapter->SetBtScanMode(mode, duration));
    HILOGI("classic_adapter_test_006 end");
}

/**
 * @tc.name: classic_adapter_test_007
 * @tc.desc: BluetoothClassicAdapterTest:SetBtScanMode
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_007, TestSize.Level1)
{
    HILOGI("classic_adapter_test_007 enter");
    int mode = SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE;
    int duration = 1000;
    EXPECT_FALSE(classicAdapter->SetBtScanMode(mode, duration));
    HILOGI("classic_adapter_test_007 end");
}

/**
 * @tc.name: classic_adapter_test_008
 * @tc.desc: BluetoothClassicAdapterTest:SetBtScanMode
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_008, TestSize.Level1)
{
    HILOGI("classic_adapter_test_008 enter");
    int mode = SCAN_MODE_NONE;
    int duration = 1000;
    EXPECT_FALSE(classicAdapter->SetBtScanMode(mode, duration));
    HILOGI("classic_adapter_test_008 end");
}

/**
 * @tc.name: classic_adapter_test_023
 * @tc.desc: BluetoothClassicAdapterTest:SspRequestInner
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_023, TestSize.Level1)
{
    HILOGI("classic_adapter_test_023 enter");
    STACK::RawAddress remote_bd_addr;
    uint32_t cod = 0;
    classicAdapter->SspRequest(nullptr, nullptr, cod, BT_SSP_VARIANT_PASSKEY_ENTRY, 0);
    classicAdapter->SspRequest(&remote_bd_addr, nullptr, cod, BT_SSP_VARIANT_PASSKEY_ENTRY, 0);
    classicAdapter->SspRequestInner(remote_bd_addr, nullptr, cod, BT_SSP_VARIANT_PASSKEY_CONFIRMATION, 0);
    classicAdapter->SspRequestInner(remote_bd_addr, nullptr, cod, BT_SSP_VARIANT_CONSENT, 0);
    classicAdapter->SspRequestInner(remote_bd_addr, nullptr, cod, BT_SSP_VARIANT_PASSKEY_NOTIFICATION, 0);
    classicAdapter->SspRequestInner(remote_bd_addr, nullptr, cod, BT_SSP_VARIANT_PASSKEY_ENTRY, 0);
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    remote_bd_addr = ServiceUtil::AddrToStack(realAddr1);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(realAddr1);
    remoteDevice->SetBondedFromLocal(false);
    classicAdapter->SspRequestInner(remote_bd_addr, nullptr, cod, BT_SSP_VARIANT_PASSKEY_CONFIRMATION, 0);
    remoteDevice->SetBondedFromLocal(true);
    classicAdapter->SetPairingConfirmationIfNeed("00:11:22:33:44:66");
    classicAdapter->SspRequestInner(remote_bd_addr, nullptr, cod, BT_SSP_VARIANT_PASSKEY_CONFIRMATION, 0);
    EXPECT_TRUE(classicAdapter != nullptr);
    HILOGI("classic_adapter_test_023 end");
}

/**
 * @tc.name: classic_adapter_test_024
 * @tc.desc: BluetoothClassicAdapterTest:PinRequest
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_024, TestSize.Level1)
{
    HILOGI("classic_adapter_test_024 enter");
    STACK::RawAddress remote_bd_addr;
    uint32_t cod = 0;
    classicAdapter->PinRequest(&remote_bd_addr, nullptr, cod, true);
    classicAdapter->PinRequest(nullptr, nullptr, cod, true);
    RawAddress device = ServiceUtil::AddrFromStack(remote_bd_addr);
    EXPECT_FALSE(classicAdapter->StartCrediblePair(BTTransport::ADAPTER_BREDR, device));
    classicAdapter->PinRequest(&remote_bd_addr, nullptr, cod, true);
    HILOGI("classic_adapter_test_024 end");
}

/**
 * @tc.name: classic_adapter_test_025
 * @tc.desc: BluetoothClassicAdapterTest:PinRequestInner
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_025, TestSize.Level1)
{
    HILOGI("classic_adapter_test_025 enter");
    STACK::RawAddress remote_bd_addr;
    uint32_t cod = 0;
    classicAdapter->PinRequestInner(remote_bd_addr, nullptr, cod, true);
    RawAddress device = RawAddress("00:11:22:33:44:66");
    remote_bd_addr = ServiceUtil::AddrToStack(device);
    classicAdapter->PinRequestInner(remote_bd_addr, nullptr, cod, true);
    classicAdapter->SendPairConfirmed(device, 0, 0);
    EXPECT_FALSE(remoteDeviceProperties.CheckPairWhiteList(device));
    HILOGI("classic_adapter_test_025 end");
}

/**
 * @tc.name: classic_adapter_test_SendPairConfirmed_002
 * @tc.desc: BluetoothClassicAdapterTest:SendPairConfirmed
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_SendPairConfirmed_002, TestSize.Level1)
{
    HILOGI("classic_adapter_test_SendPairConfirmed_002 enter");
    RawAddress device = RawAddress("00:11:22:33:44:56");
    remoteDeviceProperties.AddPairWhiteList(device);
    classicAdapter->SendPairConfirmed(device, 0, 0);
    EXPECT_FALSE(remoteDeviceProperties.CheckPairWhiteList(device));
    HILOGI("classic_adapter_test_SendPairConfirmed_002 end");
}

/**
 * @tc.name: classic_adapter_test_027
 * @tc.desc: BluetoothClassicAdapterTest:SaveRemoteDeviceUuids
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_027, TestSize.Level1)
{
    HILOGI("classic_adapter_test_027 enter");
    std::shared_ptr<BluetoothDevice> remote = std::make_shared<BluetoothDevice>();
    std::vector<Uuid> v;
    classicAdapter->SaveRemoteDeviceUuids(remote, v);
    Uuid uuid = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    v.push_back(uuid);
    classicAdapter->SaveRemoteDeviceUuids(remote, v);
    std::vector<Uuid> deviceUuids = remote->GetDeviceUuids();
    EXPECT_EQ(deviceUuids[0], uuid);
    remote->SetDeviceType(DEVICE_TYPE_DUAL_MONO);
    classicAdapter->SaveRemoteDeviceUuids(remote, v);
    HILOGI("classic_adapter_test_027 end");
}

/**
 * @tc.name: classic_adapter_test_028
 * @tc.desc: BluetoothClassicAdapterTest:IsUuidsEqual
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_028, TestSize.Level1)
{
    HILOGI("classic_adapter_test_028 enter");
    Uuid uuid1 = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    Uuid uuid2 = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B58FB");
    std::vector<Uuid> v1;
    std::vector<Uuid> v2;
    std::vector<Uuid> v3;
    v1.push_back(uuid1);
    v1.push_back(uuid2);
    v2.push_back(uuid2);
    EXPECT_FALSE(classicAdapter->IsUuidsEqual(v1, v2));
    EXPECT_FALSE(classicAdapter->IsUuidsEqual(v2, v1));
    v3.push_back(uuid1);
    v3.push_back(uuid1);
    EXPECT_FALSE(classicAdapter->IsUuidsEqual(v1, v3));
    v2.push_back(uuid1);
    EXPECT_TRUE(classicAdapter->IsUuidsEqual(v1, v2));
    HILOGI("classic_adapter_test_028 end");
}

/**
 * @tc.name: classic_adapter_test_029
 * @tc.desc: BluetoothClassicAdapterTest:SetFastScan
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_029, TestSize.Level1)
{
    HILOGI("classic_adapter_test_029 enter");
    EXPECT_TRUE(classicAdapter->SetFastScan(true));
    HILOGI("classic_adapter_test_029 end");
}

/**
 * @tc.name: classic_adapter_test_030
 * @tc.desc: BluetoothClassicAdapterTest:SetPairingConfirmationIfNeed
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_030, TestSize.Level1)
{
    HILOGI("classic_adapter_test_030 enter");
    RawAddress addr("00:00:00:00:00:00");
    EXPECT_FALSE(classicAdapter->StartCrediblePair(BTTransport::ADAPTER_BREDR, addr));
    EXPECT_FALSE(classicAdapter->StartCrediblePair(BTTransport::ADAPTER_BLE, addr));
    sleep(2);
    EXPECT_FALSE(classicAdapter->SetPairingConfirmationIfNeed("00:00:00:00:00:00"));
    HILOGI("classic_adapter_test_030 end");
}

/**
 * @tc.name: classic_adapter_test_031
 * @tc.desc: BluetoothClassicAdapterTest:SetPairingConfirmationIfNeed
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_031, TestSize.Level1)
{
    HILOGI("classic_adapter_test_031 enter");
    RawAddress addr("00:00:00:00:00:01");
    EXPECT_FALSE(classicAdapter->StartCrediblePair(BTTransport::ADAPTER_BREDR, addr));
    EXPECT_FALSE(classicAdapter->StartCrediblePair(BTTransport::ADAPTER_BLE, addr));
    sleep(2);
    EXPECT_FALSE(classicAdapter->SetPairingConfirmationIfNeed("00:00:00:00:00:02"));
    HILOGI("classic_adapter_test_031 end");
}

/**
 * @tc.name: classic_adapter_test_032
 * @tc.desc: BluetoothClassicAdapterTest:ConnectAnyAllowedProfiles
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_032, TestSize.Level1)
{
    HILOGI("classic_adapter_test_032");
    RawAddress device = RawAddress("11:11:22:33:44:55");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    EXPECT_EQ(remoteDevice->GetPairedStatus(), PAIR_PAIRED);

    IProfileConfig *config = ProfileConfig::GetInstance();
    int stratege = 1; // 1 mean connection_allowed
    config->Load();
    config->SetValue(remoteDevice->GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HFP_CONNECTION_POLICY, stratege);
    config->SetValue(remoteDevice->GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_A2DP_CONNECTION_POLICY, stratege);
    config->SetValue(remoteDevice->GetAddress(), SECTION_CONNECTION_POLICIES,
        PROPERTY_HID_CONNECTION_POLICY, stratege);

    ConnectStrategyManager::GetInstance()->remoteDeviceProperties_ = &remoteDeviceProperties;
    if (classicAdapter->agService_) {
        classicAdapter->agService_->SetConnectStrategy(device, stratege);
    }
    if (classicAdapter->a2dpSrcService_) {
        classicAdapter->a2dpSrcService_->SetConnectStrategy(device, stratege);
    }
    if (classicAdapter->hidHostService_) {
        classicAdapter->hidHostService_->SetConnectStrategy(device, stratege);
    }

    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_A2DP_SINK));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->ConnectAllowedProfiles(device);

    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_HF));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->ConnectAllowedProfiles(device);

    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HID_HOST));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->ConnectAllowedProfiles(device);
    classicAdapter->ConnectAnyAllowedProfiles(device);
    HILOGI("classic_adapter_test_032 end");
}

/**
 * @tc.name: classic_adapter_test_033
 * @tc.desc: BluetoothClassicAdapterTest:ConnectAllowedProfiles
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_033, TestSize.Level1)
{
    HILOGI("classic_adapter_test_033 enter");
    RawAddress remoteAddr = RawAddress("99:11:22:33:44:55");
    classicAdapter->ConnectAllowedProfiles(remoteAddr);

    RawAddress device = RawAddress("88:11:22:33:44:55");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_EQ(remoteDevice->GetPairedStatus(), PAIR_NONE);

    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_A2DP_SINK));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->ConnectAllowedProfiles(device);

    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_HF));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->ConnectAllowedProfiles(device);

    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HID_HOST));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->ConnectAllowedProfiles(device);
    HILOGI("classic_adapter_test_033 end");
}

/**
 * @tc.name: classic_adapter_test_034
 * @tc.desc: BluetoothClassicAdapterTest:DisconnectAllowedProfiles
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_034, TestSize.Level1)
{
    HILOGI("classic_adapter_test_034 enter");
    RawAddress remoteAddr = RawAddress("00:11:22:33:44:55");
    classicAdapter->DisconnectAllowedProfiles(remoteAddr);
    int32_t ret = classicAdapter->SetDeviceCustomType(remoteAddr, 1);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    classicAdapter->DisconnectAllowedProfiles(remoteAddr);
    ret = classicAdapter->SetDeviceCustomType(remoteAddr, 0);
    EXPECT_EQ(ret, BT_ERR_INVALID_PARAM);
    classicAdapter->DisconnectAllowedProfiles(remoteAddr);
    classicAdapter->DisconnectAllowedProfilesNotSetVirtualConnSwitch(remoteAddr);
    HILOGI("classic_adapter_test_034 end");
}

/**
 * @tc.name: classic_adapter_test_035
 * @tc.desc: BluetoothClassicAdapterTest:ConnectAnyAllowedProfiles
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_035, TestSize.Level1)
{
    HILOGI("classic_adapter_test_035 enter");
    RawAddress remoteAddr = RawAddress("00:11:22:33:44:55");
    classicAdapter->ConnectAnyAllowedProfiles(remoteAddr);
    EXPECT_FALSE(classicAdapter->IsAnyProfilesAllowed(remoteAddr));
    HILOGI("classic_adapter_test_035 end");
}

/**
 * @tc.name: classic_adapter_test_037
 * @tc.desc: BluetoothClassicAdapterTest:IsAnyProfilesAllowed
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_037, TestSize.Level1)
{
    HILOGI("classic_adapter_test_037 enter");
    std::vector<Uuid> localUuids;
    std::vector<Uuid> remoteUuids;
    EXPECT_FALSE(classicAdapter->IsProfileSupported(PROFILE_ID_HFP_AG, localUuids, remoteUuids));
    localUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_AG));
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_HF));
    EXPECT_TRUE(classicAdapter->IsProfileSupported(PROFILE_ID_HFP_AG, localUuids, remoteUuids));
    localUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HSP_AG));
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HSP_HS));
    EXPECT_TRUE(classicAdapter->IsProfileSupported(PROFILE_ID_HFP_AG, localUuids, remoteUuids));
    remoteUuids.push_back(Uuid::ConvertFromString(ADV_AUDIO_DIST));
    EXPECT_TRUE(classicAdapter->IsProfileSupported(PROFILE_ID_A2DP_SRC, localUuids, remoteUuids));
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_A2DP_SINK));
    EXPECT_TRUE(classicAdapter->IsProfileSupported(PROFILE_ID_A2DP_SRC, localUuids, remoteUuids));
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HID_HOST));
    EXPECT_TRUE(classicAdapter->IsProfileSupported(PROFILE_ID_HID_HOST, localUuids, remoteUuids));
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HOGP));
    EXPECT_TRUE(classicAdapter->IsProfileSupported(PROFILE_ID_HID_HOST, localUuids, remoteUuids));
    HILOGI("classic_adapter_test_037 end");
}

/**
 * @tc.name: classic_adapter_test_038
 * @tc.desc: BluetoothClassicAdapterTest:NeedWaitForSdpComplete
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_038, TestSize.Level1)
{
    HILOGI("classic_adapter_test_038 enter");
    RawAddress device = RawAddress("00:11:22:33:44:66");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    EXPECT_TRUE(classicAdapter->NeedWaitForSdpComplete(remoteDevice));
    remoteDevice->SetDeviceType(DEVICE_TYPE_BREDR);
    EXPECT_TRUE(classicAdapter->NeedWaitForSdpComplete(remoteDevice));
    remoteDevice->SetDeviceType(DEVICE_TYPE_DUAL_MONO);
    EXPECT_TRUE(classicAdapter->NeedWaitForSdpComplete(remoteDevice));
    remoteDevice->SetDeviceType(DEVICE_TYPE_LE);
    EXPECT_FALSE(classicAdapter->NeedWaitForSdpComplete(remoteDevice));
    EXPECT_EQ(classicAdapter->GetDeviceType(device), DEVICE_TYPE_LE);
    remoteDevice->SetDeviceType(DEVICE_TYPE_UNKNOWN);
    EXPECT_FALSE(classicAdapter->NeedWaitForSdpComplete(remoteDevice));
    EXPECT_EQ(classicAdapter->GetDeviceType(device), DEVICE_TYPE_UNKNOWN);
    HILOGI("classic_adapter_test_038 end");
}

/**
 * @tc.name: classic_adapter_test_043
 * @tc.desc: BluetoothClassicAdapterTest:SetLocalName
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_043, TestSize.Level1)
{
    HILOGI("classic_adapter_test_043 enter");
    classicAdapter->GetLocalAddress();
    classicAdapter->GetLocalName();
    std::string name = "";
    classicAdapter->SetLocalName(name);
    EXPECT_EQ(classicAdapter->GetLocalName(), "");
    HILOGI("classic_adapter_test_043 end");
}

/**
 * @tc.name: classic_adapter_test_045
 * @tc.desc: BluetoothClassicAdapterTest:SetLocalDeviceClass
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_045, TestSize.Level1)
{
    HILOGI("classic_adapter_test_045 enter");
    classicAdapter->GetLocalDeviceClass();
    uint32_t cod = 0;
    classicAdapter->SetLocalDeviceClass(cod);
    EXPECT_EQ(classicAdapter->GetLocalDeviceClass(), 2752772);
    HILOGI("classic_adapter_test_045 end");
}

/**
 * @tc.name: classic_adapter_test_047
 * @tc.desc: BluetoothClassicAdapterTest:GetBondableMode
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_047, TestSize.Level1)
{
    HILOGI("classic_adapter_test_047 enter");
    EXPECT_EQ(classicAdapter->GetBondableMode(), 0);
    HILOGI("classic_adapter_test_047 end");
}

/**
 * @tc.name: classic_adapter_test_048
 * @tc.desc: BluetoothClassicAdapterTest:SetBondableMode
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_048, TestSize.Level1)
{
    HILOGI("classic_adapter_test_048 enter");
    int mode = 0;
    EXPECT_TRUE(classicAdapter->SetBondableMode(mode));
    classicAdapter->GetBtDiscoveryEndMillis();
    HILOGI("classic_adapter_test_048 end");
}

/**
 * @tc.name: classic_adapter_test_051
 * @tc.desc: BluetoothClassicAdapterTest:IsAclConnected
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_051, TestSize.Level1)
{
    HILOGI("classic_adapter_test_051 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    EXPECT_FALSE(classicAdapter->IsAclConnected(realAddr1));
    RawAddress realAddr2 = RawAddress("00:11:22:33:44:78");
    EXPECT_FALSE(classicAdapter->IsAclConnected(realAddr2));
    HILOGI("classic_adapter_test_051 end");
}

/**
 * @tc.name: classic_adapter_test_052
 * @tc.desc: BluetoothClassicAdapterTest:IsAclEncrypted
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_052, TestSize.Level1)
{
    HILOGI("classic_adapter_test_052 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    EXPECT_FALSE(classicAdapter->IsAclEncrypted(realAddr1));
    classicAdapter->GetDeviceName(realAddr1);
    classicAdapter->SendRemoteCodChanged(realAddr1, 0);
    classicAdapter->SendRemoteNameChanged(realAddr1, "");
    classicAdapter->SendScanModeChanged(0);
    HILOGI("classic_adapter_test_052 end");
}

/**
 * @tc.name: classic_adapter_test_054
 * @tc.desc: BluetoothClassicAdapterTest:CancelPairing
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_054, TestSize.Level1)
{
    HILOGI("classic_adapter_test_054 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    EXPECT_FALSE(classicAdapter->CancelPairing(realAddr1));
    HILOGI("classic_adapter_test_054 end");
}

/**
 * @tc.name: classic_adapter_test_055
 * @tc.desc: BluetoothClassicAdapterTest:GetPairState
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_055, TestSize.Level1)
{
    HILOGI("classic_adapter_test_055 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    EXPECT_EQ(classicAdapter->GetPairState(realAddr1), 1);
    HILOGI("classic_adapter_test_055 end");
}

/**
 * @tc.name: classic_adapter_test_056
 * @tc.desc: BluetoothClassicAdapterTest:PairRequestReply
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_056, TestSize.Level1)
{
    HILOGI("classic_adapter_test_056 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    EXPECT_FALSE(classicAdapter->PairRequestReply(realAddr1, true));
    HILOGI("classic_adapter_test_056 end");
}

/**
 * @tc.name: classic_adapter_test_057
 * @tc.desc: BluetoothClassicAdapterTest:SetDevicePin
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_057, TestSize.Level1)
{
    HILOGI("classic_adapter_test_057 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    std::string pinCode = "123456";
    EXPECT_FALSE(classicAdapter->SetDevicePin(realAddr1, pinCode));
    HILOGI("classic_adapter_test_057 end");
}

/**
 * @tc.name: classic_adapter_test_058
 * @tc.desc: BluetoothClassicAdapterTest:SetDevicePin
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_058, TestSize.Level1)
{
    HILOGI("classic_adapter_test_058 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    std::string pinCode = "123456";
    EXPECT_FALSE(classicAdapter->SetDevicePin(realAddr1, pinCode));
    HILOGI("classic_adapter_test_058 end");
}

/**
 * @tc.name: classic_adapter_test_060
 * @tc.desc: BluetoothClassicAdapterTest:GetDeviceClass
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_060, TestSize.Level1)
{
    HILOGI("classic_adapter_test_060 enter");
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:66");
    EXPECT_EQ(classicAdapter->GetDeviceClass(realAddr1), 0);

    RawAddress device = RawAddress("00:11:22:33:44:66");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetDeviceClass(0x400);
    EXPECT_NE(classicAdapter->GetDeviceClass(device), 0);

    remoteDevice->SetHwTalkBandBandMode(TalkBandMode::VALUE_OF_TALKBAND_BAND_MODE);
    EXPECT_EQ(classicAdapter->GetDeviceClass(device), 0x280704);
    remoteDevice->SetHwTalkBandBandMode(TalkBandMode::VALUE_OF_TALKBAND_HEADSET_MODE);
    EXPECT_EQ(classicAdapter->GetDeviceClass(device), 0x240418);
    HILOGI("classic_adapter_test_060 end");
}

/**
 * @tc.name: classic_adapter_test_061
 * @tc.desc: BluetoothClassicAdapterTest:GetAliasName
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_061, TestSize.Level1)
{
    HILOGI("classic_adapter_test_061 enter");
    RawAddress device = RawAddress("00:11:22:33:44:66");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    std::string name = "asd";
    EXPECT_TRUE(classicAdapter->SetAliasName(device, name));
    EXPECT_EQ(classicAdapter->GetAliasName(device), name);
    HILOGI("classic_adapter_test_061 end");
}

/**
 * @tc.name: classic_adapter_test_068
 * @tc.desc: BluetoothClassicAdapterTest:DiscoveryStateChangedInner
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_068, TestSize.Level1)
{
    HILOGI("classic_adapter_test_068 enter");
    classicAdapter->DiscoveryStateChangedInner(BT_DISCOVERY_STOPPED);
    classicAdapter->DiscoveryStateChangedInner(BT_DISCOVERY_STARTED);
    int BT_DISCOVERY_INVALID = 2;
    classicAdapter->DiscoveryStateChangedInner(static_cast<bt_discovery_state_t>(BT_DISCOVERY_INVALID));
    classicAdapter->SendDiscoveryStateChanged(DISCOVERY_STOPED);
    EXPECT_TRUE(classicAdapter->IsBtDiscovering());
    HILOGI("classic_adapter_test_068 end");
}

/**
 * @tc.name: classic_adapter_test_069
 * @tc.desc: BluetoothClassicAdapterTest:DiscoveryStateChangedInner
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_069, TestSize.Level1)
{
    HILOGI("classic_adapter_test_069 enter");
    EXPECT_EQ(classicAdapter->FactoryReset(), 0);
    HILOGI("classic_adapter_test_069 end");
}

/**
 * @tc.name: classic_adapter_test_070
 * @tc.desc: BluetoothClassicAdapterTest:GetDeviceCustomType/SetDeviceCustomType
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_070, TestSize.Level1)
{
    HILOGI("classic_adapter_test_070 enter");
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    RawAddress device = RawAddress("00:11:22:33:44:66");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    BluetoothRemoteDeviceInfo deviceInfo;
    deviceInfo.customType_ = -1;

    EXPECT_EQ(classicAdapter->GetRemoteDeviceInfo(device, deviceInfo), BT_NO_ERROR);
    EXPECT_EQ(deviceInfo.modelId_.substr(0, 6), "_00_00");
    EXPECT_NE(classicAdapter->SetDeviceCustomType(device, deviceInfo.customType_), BT_NO_ERROR);

    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_HF));
    remoteDevice->SetDeviceUuids(remoteUuids);
    deviceInfo.customType_ = 1;
    EXPECT_EQ(classicAdapter->GetRemoteDeviceInfo(device, deviceInfo), BT_NO_ERROR);
    EXPECT_EQ(classicAdapter->SetDeviceCustomType(device, 2), BT_NO_ERROR);
    EXPECT_EQ(classicAdapter->GetRemoteDeviceInfo(device, deviceInfo), BT_NO_ERROR);
    HILOGI("classic_adapter_test_070 end");
}

/**
 * @tc.name: classic_adapter_test_071
 * @tc.desc: BluetoothClassicAdapterTest:GetRemoteDeviceInfo
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_071, TestSize.Level1)
{
    HILOGI("classic_adapter_test_071 enter");
    RawAddress device = RawAddress("00:11:22:33:44:66");
    BluetoothRemoteDeviceInfo deviceInfo;
    EXPECT_EQ(classicAdapter->GetRemoteDeviceInfo(device, deviceInfo), BT_NO_ERROR);
    HILOGI("classic_adapter_test_071 end");
}

/**
 * @tc.name: classic_adapter_test_072
 * @tc.desc: BluetoothClassicAdapterTest:IsNeededReturnCodForAudio
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_072, TestSize.Level1)
{
    HILOGI("classic_adapter_test_072 enter");
    static int32_t uidAudio = 1041;
    RawAddress device = RawAddress("00:11:22:33:44:66");
    int customType = 0;
    EXPECT_FALSE(classicAdapter->IsNeededReturnCodForAudio(0, customType));
    customType = 2;
    EXPECT_TRUE(classicAdapter->IsNeededReturnCodForAudio(uidAudio, customType));
    customType = 4;
    EXPECT_TRUE(classicAdapter->IsNeededReturnCodForAudio(uidAudio, customType));
    HILOGI("classic_adapter_test_072 end");
}

/**
 * @tc.name: classic_adapter_test_074
 * @tc.desc: BluetoothClassicAdapterTest:GetRemoteDevicePropertyInfo
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_074, TestSize.Level1)
{
    HILOGI("classic_adapter_test_074 enter");
    bthwif_interface_t *gbtHwIfInterFaceTest = nullptr;
    BluetoothHwInterface::GetInstance()->SetBtHwInterface(gbtHwIfInterFaceTest);
    RawAddress device = RawAddress("00:11:22:33:44:66");
    std::string modelId = "";
    EXPECT_FALSE(classicAdapter->GetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_MODEL_ID, modelId));

    std::string newModelId = "";
    EXPECT_FALSE(classicAdapter->GetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_NEW_MODEL_ID, newModelId));

    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    EXPECT_TRUE(classicAdapter->GetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_MODEL_ID, modelId));
    EXPECT_TRUE(classicAdapter->GetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_NEW_MODEL_ID, newModelId));
    HILOGI("classic_adapter_test_074 end");
}

/**
 * @tc.name: classic_adapter_test_076
 * @tc.desc: BluetoothClassicAdapterTest:GetPeripheralCod
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_076, TestSize.Level1)
{
    HILOGI("classic_adapter_test_076 enter");
    RawAddress device = RawAddress("66:11:22:33:44:55");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    int32_t cod = 0;

    remoteDevice->SetVendorId(0);
    classicAdapter->GetPeripheralCod(device, cod);

    remoteDevice->SetVendorId(VENDOR_ID_OF_HW_DEVICE);

    remoteDevice->SetProductId(4241);
    classicAdapter->GetPeripheralCod(device, cod);
    EXPECT_EQ(cod, CLASS_OF_DEVICE_HID_PENCIL);

    remoteDevice->SetProductId(4253);
    classicAdapter->GetPeripheralCod(device, cod);
    EXPECT_EQ(cod, CLASS_OF_DEVICE_HID_KEYBOARD);

    remoteDevice->SetProductId(4284);
    classicAdapter->GetPeripheralCod(device, cod);
    EXPECT_EQ(cod, CLASS_OF_DEVICE_HID_POINTING);

    HILOGI("classic_adapter_test_076 end");
}

/**
 * @tc.name: classic_adapter_test_077
 * @tc.desc: BluetoothClassicAdapterTest:IsNeedDelayConnect
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_077, TestSize.Level1)
{
    HILOGI("classic_adapter_test_077 enter");
    RawAddress device = RawAddress("66:11:22:33:44:55");
    classicAdapter->DisconnectPairedDeviceProfiles();
    EXPECT_FALSE(classicAdapter->IsNeedDelayConnect(device));
    HILOGI("classic_adapter_test_077 end");
}

/**
 * @tc.name: classic_adapter_test_078
 * @tc.desc: BluetoothClassicAdapterTest:IsIncomingConnection
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_078, TestSize.Level1)
{
    HILOGI("classic_adapter_test_078 enter");
    BluetoothHwInterface::GetInstance()->SetBtHwInterface(&g_btHwIfTest);
    RawAddress device = RawAddress("66:11:22:33:44:55");
    EXPECT_TRUE(classicAdapter->IsIncomingConnection(device));
    HILOGI("classic_adapter_test_078 end");
}

/**
 * @tc.name: classic_adapter_test_079
 * @tc.desc: BluetoothClassicAdapterTest:GetDeviceTransport
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_079, TestSize.Level1)
{
    HILOGI("classic_adapter_test_079 enter");
    RawAddress device = RawAddress("66:11:22:33:44:55");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    int32_t transport = TRANSPORT_UNKNOWN;
    EXPECT_EQ(classicAdapter->GetDeviceTransport(device, transport), BT_NO_ERROR);
    remoteDevice->SetDeviceType(DEVICE_TYPE_LE);
    EXPECT_EQ(classicAdapter->GetDeviceTransport(device, transport), BT_NO_ERROR);
    remoteDevice->SetDeviceType(DEVICE_TYPE_DUAL_MONO);
    EXPECT_EQ(classicAdapter->GetDeviceTransport(device, transport), BT_NO_ERROR);
    remoteDevice->SetDeviceType(DEVICE_TYPE_UNKNOWN);
    EXPECT_EQ(classicAdapter->GetDeviceTransport(device, transport), BT_NO_ERROR);
    HILOGI("classic_adapter_test_079 end");
}

/**
 * @tc.name: classic_adapter_test_081
 * @tc.desc: BluetoothClassicAdapterTest:CheckDeviceUuidsAndSendChr
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_081, TestSize.Level1)
{
    HILOGI("classic_adapter_test_081 enter");
    classicAdapter->CheckDeviceUuidsAndSendChr(nullptr);

    RawAddress device = RawAddress("66:11:22:33:44:55");
    std::shared_ptr<BluetoothDevice> remoteDevice =
        std::make_shared<BluetoothDevice>(device.GetAddress());
    classicAdapter->CheckDeviceUuidsAndSendChr(remoteDevice);

    std::vector<Uuid> remoteUuids;
    remoteUuids.push_back(Uuid::ConvertFromString(EMPTY_UUID));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->CheckDeviceUuidsAndSendChr(remoteDevice);

    remoteUuids.pop_back();
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_A2DP_SINK));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->CheckDeviceUuidsAndSendChr(remoteDevice);

    remoteUuids.pop_back();
    remoteUuids.push_back(Uuid::ConvertFromString(EMPTY_UUID));
    remoteUuids.push_back(Uuid::ConvertFromString(BLUETOOTH_UUID_A2DP_SINK));
    remoteDevice->SetDeviceUuids(remoteUuids);
    classicAdapter->CheckDeviceUuidsAndSendChr(remoteDevice);
    EXPECT_TRUE(classicAdapter != nullptr);
    HILOGI("classic_adapter_test_081 end");
}

/**
 * @tc.name: classic_adapter_test_083
 * @tc.desc: BluetoothClassicAdapterTest:RemovePairInner
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_083, TestSize.Level1)
{
    HILOGI("classic_adapter_test_083 enter");
    classicAdapter->ResumeBleStackDevice();
    RawAddress realAddr1 = RawAddress("00:11:22:33:44:55");
    EXPECT_FALSE(classicAdapter->RemovePairInner(BTTransport::ADAPTER_BREDR, realAddr1));
    EXPECT_FALSE(classicAdapter->RemovePairInner(BTTransport::ADAPTER_BLE, realAddr1));
    HILOGI("classic_adapter_test_083 end");
}

/**
 * @tc.name: classic_adapter_test_085
 * @tc.desc: BluetoothClassicAdapterTest:StartBtScanModeTimer
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_085, TestSize.Level1)
{
    HILOGI("classic_adapter_test_085 enter");
    int mode = SCAN_MODE_GENERAL_DISCOVERABLE;
    int duration = 0;
    classicAdapter->StartBtScanModeTimer(mode, duration);
    mode = SCAN_MODE_GENERAL_DISCOVERABLE;
    duration = 1;
    classicAdapter->StartBtScanModeTimer(mode, duration);
    mode = SCAN_MODE_CONNECTABLE;
    duration = 0;
    classicAdapter->StartBtScanModeTimer(mode, duration);
    mode = SCAN_MODE_CONNECTABLE;
    duration = 1;
    classicAdapter->StartBtScanModeTimer(mode, duration);
    classicAdapter->StopBtScanModeTimer();
    int level = 0;
    EXPECT_TRUE(classicAdapter->SetFastScanLevel(level));
    HILOGI("classic_adapter_test_085 end");
}

/**
 * @tc.name: classic_adapter_test_087
 * @tc.desc: BluetoothClassicAdapterTest:GetRemoteDeviceBatteryInfo
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_087, TestSize.Level1)
{
    HILOGI("classic_adapter_test_087 enter");
    RawAddress device = RawAddress("00:11:22:33:44:56");
    BatteryInfo batteryInfo;
    int ret = classicAdapter->GetRemoteDeviceBatteryInfo(device, batteryInfo);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("classic_adapter_test_087 end");
}

/**
 * @tc.name: classic_adapter_test_088
 * @tc.desc: BluetoothClassicAdapterTest:CovertUnbondCause
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_088, TestSize.Level1)
{
    HILOGI("classic_adapter_test_088 enter");
    bt_status_t status = BT_STATUS_RMT_DEV_DOWN;
    int unbondCause = UNBOND_CAUSE_REMOTE_DEVICE_DOWN;
    std::string causeMessage = UNBOND_MSG_UNKNOWN;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_AUTH_FAILURE;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_AUTH_REJECTED;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_SUCCESS;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_BUSY;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_NOT_READY;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_UNSUPPORTED;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_PARM_INVALID;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_JNI_ENVIRONMENT_ERROR;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    status = BT_STATUS_WAKELOCK_ERROR;
    classicAdapter->CovertUnbondCause(status, unbondCause, causeMessage);
    EXPECT_EQ(unbondCause, UNBOND_CAUSE_INTERNAL_ERROR);
    HILOGI("classic_adapter_test_088 end");
}

/**
 * @tc.name: classic_adapter_test_090
 * @tc.desc: BluetoothClassicAdapterTest:ControlDeviceAction
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_090, TestSize.Level1)
{
    HILOGI("classic_adapter_test_090 enter");
    std::string addr1 = "";
    uint32_t controlType = 0;
    uint32_t controlTypeVal = 1;
    uint32_t controlObject = 2;
    int ret = classicAdapter->ControlDeviceAction(addr1, controlType, controlTypeVal, controlObject);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("classic_adapter_test_090 end");
}

/**
 * @tc.name: classic_adapter_test_091
 * @tc.desc: BluetoothClassicAdapterTest:DeleteDeviceInfoFiles
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_091, TestSize.Level1)
{
    HILOGI("classic_adapter_test_091 enter");
    classicAdapter->DeleteDeviceInfoFiles();
    std::string filePath = "/data/service/el1/public/bluetooth";
    std::vector<std::string> files;
    GetDirFiles(filePath, files);
    EXPECT_TRUE(classicAdapter != nullptr);
    HILOGI("classic_adapter_test_091 end");
}

/**
 * @tc.name: classic_adapter_test_092
 * @tc.desc: BluetoothClassicAdapterTest:SetConnectionTime
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_092, TestSize.Level1)
{
    HILOGI("classic_adapter_test_092 enter");
    RawAddress device = RawAddress("00:11:22:33:44:56");
    remoteDeviceProperties.FindRemoteDevice(device);
    classicAdapter->SetConnectionTime(device, 1);
    int64_t connectTime = 0;
    classicAdapter->GetLastConnectionTime(device, connectTime);
    EXPECT_EQ(connectTime, 1);
    HILOGI("classic_adapter_test_092 end");
}

/**
 * @tc.name: classic_adapter_test_093
 * @tc.desc: BluetoothClassicAdapterTest:IsDisconnectAclBeforeBond
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_093, TestSize.Level1)
{
    HILOGI("classic_adapter_test_093 enter");
    std::string device = "00:11:22:33:44:56";
    classicAdapter->AddDisconnectAclBeforeBondDevice(device);
    EXPECT_TRUE(classicAdapter->IsDisconnectAclBeforeBond(device));
    classicAdapter->DelDisconnectAclBeforeBondDevice(device);
    HILOGI("classic_adapter_test_093 end");
}

/**
 * @tc.name: classic_adapter_test_096
 * @tc.desc: BluetoothClassicAdapterTest:SetCloudBondState
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_096, TestSize.Level1)
{
    HILOGI("classic_adapter_test_096 enter");
    RawAddress device = RawAddress("00:11:22:33:44:66");
    classicAdapter->SetCloudBondState(device, 1);
    int32_t cloudState = classicAdapter->GetCloudBondState(device);
    EXPECT_EQ(cloudState, 1);
    HILOGI("classic_adapter_test_096 end");
}

/**
 * @tc.name: classic_adapter_test_097
 * @tc.desc: BluetoothClassicAdapterTest:IsCloudDevice
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_097, TestSize.Level1)
{
    HILOGI("classic_adapter_test_097 enter");
    RawAddress device = RawAddress("00:11:22:33:44:66");
    int32_t ret = classicAdapter->IsCloudDevice(device);
    EXPECT_EQ(ret, BT_NO_ERROR);
    HILOGI("classic_adapter_test_097 end");
}

/**
 * @tc.name: classic_adapter_test_098
 * @tc.desc: BluetoothClassicAdapterTest:ClearCloudDevice
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_098, TestSize.Level1)
{
    HILOGI("classic_adapter_test_098 enter");
    RawAddress device = RawAddress("11:11:22:33:44:55");
    remoteDeviceProperties.FindRemoteDevice(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    classicAdapter->ClearCloudDevice(remoteDevice);
    EXPECT_FALSE(classicAdapter->IsStartCloudPair(device));
    HILOGI("classic_adapter_test_098 end");
    sleep(TIME);
}

HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_099, TestSize.Level1)
{
    HILOGI("classic_adapter_test_099 enter");
    RawAddress device = RawAddress("55:66:11:22:33:44");
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties.FindRemoteDevice(device);
    remoteDevice->SetCloudDevice(true);
    remoteDevice->SetBondedFromLocal(true);

    bt_status_t status = BT_STATUS_FAIL;
    STACK::RawAddress bd_addr = ServiceUtil::AddrToStack(device);
    bt_bond_state_t state = BT_BOND_STATE_NONE;

    classicAdapter->BondStateChangedInner(status, bd_addr, state);
    EXPECT_TRUE(remoteDevice->IsBondedFromLocal() == false);
    HILOGI("classic_adapter_test_099 end");
}

/**
 * @tc.name: classic_adapter_test_100
 * @tc.desc: AddPendingConnectDevice、RemovePendingConnectDevice
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_100, TestSize.Level1)
{
    HILOGI("classic_adapter_test_100 enter");
    RawAddress device1 = RawAddress("11:11:22:33:44:55");
    RawAddress device2 = RawAddress("22:22:33:44:55:66");
    int profileId1 = PROFILE_ID_HID_HOST;
    int profileId2 = PROFILE_ID_BAS;
    classicAdapter->AddPendingConnectDevice(device1, profileId1);
    EXPECT_EQ(classicAdapter->deviceProfileAddressMap_.Size(), 1);
    classicAdapter->AddPendingConnectDevice(device2, profileId2);
    EXPECT_EQ(classicAdapter->deviceProfileAddressMap_.Size(), 2);
    classicAdapter->RemovePendingConnectDevice(device1);
    EXPECT_EQ(classicAdapter->deviceProfileAddressMap_.Size(), 1);
    classicAdapter->RemovePendingConnectDevice(device2);
    EXPECT_EQ(classicAdapter->deviceProfileAddressMap_.Size(), 0);
    classicAdapter->RemovePendingConnectDevice(device1);
    EXPECT_EQ(classicAdapter->deviceProfileAddressMap_.Size(), 0);
    HILOGI("classic_adapter_test_100 end");
}

/**
 * @tc.name: classic_adapter_test_101
 * @tc.desc: IsDeviceReadyConnect
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_101, TestSize.Level1)
{
    HILOGI("classic_adapter_test_101 enter");
    RawAddress device = RawAddress("11:11:22:33:44:55");
    int profileId = PROFILE_ID_HID_HOST;
    classicAdapter->IsDeviceReadyConnect(device);

    classicAdapter->AddPendingConnectDevice(device, profileId);
    EXPECT_EQ(classicAdapter->deviceProfileAddressMap_.Size(), 1);
    classicAdapter->IsDeviceReadyConnect(device);
    EXPECT_TRUE(classicAdapter->hidHostService_ != nullptr);
    EXPECT_EQ(classicAdapter->deviceProfileAddressMap_.Size(), 0);

    sleep(TIME);
    HILOGI("classic_adapter_test_101 end");
}

/**
 * @tc.name: classic_adapter_test_102
 * @tc.desc: BluetoothClassicAdapterTest:SendPairStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_102, TestSize.Level1)
{
    HILOGI("classic_adapter_test_102 enter");
    RawAddress device = RawAddress("11:11:22:33:44:55");
    EXPECT_TRUE(classicAdapter!= nullptr);
    classicAdapter->SendPairStatusChanged(BTTransport::ADAPTER_BREDR, device, PAIR_PAIRED, 0, BOND_MSG_NO_ERROR);
    classicAdapter->SetDeviceCustomType(device, 2);
    classicAdapter->SendPairStatusChanged(BTTransport::ADAPTER_BREDR, device, PAIR_PAIRED, 0, BOND_MSG_NO_ERROR);
    remoteDeviceProperties.UpdateRemoteDeviceCod(device, 0x418);
    classicAdapter->SendPairStatusChanged(BTTransport::ADAPTER_BREDR, device, PAIR_PAIRED, 0, BOND_MSG_NO_ERROR);
    classicAdapter->SetDeviceCustomType(device, 0);
    classicAdapter->SendPairStatusChanged(BTTransport::ADAPTER_BREDR, device, PAIR_PAIRED, 0, BOND_MSG_NO_ERROR);
    sleep(TIME);
    HILOGI("classic_adapter_test_102 end");
}

/**
 * @tc.name: classic_adapter_test_103
 * @tc.desc: BluetoothClassicAdapterTest:NotifyBondStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothClassicAdapterTest, classic_adapter_test_103, TestSize.Level1)
{
    HILOGI("classic_adapter_test_103 enter");
    RawAddress device = RawAddress("11:11:22:33:44:55");
    bt_status_t status = BT_STATUS_SUCCESS;
    STACK::RawAddress bd_addr = ServiceUtil::AddrToStack(device);
    bt_bond_state_t state = BT_BOND_STATE_NONE;
    EXPECT_TRUE(classicAdapter != nullptr);
    if (classicAdapter != nullptr) {
        classicAdapter->NotifyBondStateChanged(status, bd_addr, state);
    }
    // wait for the async DeleteDevice task posted to the a2dp ffrt queue to finish,
    // otherwise TearDownTestCase deletes a2dpSrcService_ while the task is running and crashes
    sleep(TIME);
    HILOGI("classic_adapter_test_103 end");
}
}  // namespace bluetooth
}  // namespace OHOS
