/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "bt_def.h"
#include "bluetooth_host.h"
#include "ble_service_data.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
static Bluetooth::BluetoothHost *host_;
constexpr int TIME = 2;
class BleServiceDataTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &Bluetooth::BluetoothHost::GetDefaultHost();

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
        host_ = nullptr;
    }
    void SetUp()
    {
        pBleAdvertiserSettingsImpl = std::make_unique<BleAdvertiserSettingsImpl>();
        pBlePeripheralDevice = std::make_unique<BlePeripheralDevice>();
        pBleScanFilterImpl = std::make_unique<BleScanFilterImpl>();
        pBleAdvertiserDataImpl = std::make_unique<BleAdvertiserDataImpl>();
        pBleScanSettingsImpl = std::make_unique<BleScanSettingsImpl>();
    }

    void TearDown()
    {
        pBleAdvertiserSettingsImpl.reset();
        pBlePeripheralDevice.reset();
        pBleScanFilterImpl.reset();
        pBleAdvertiserDataImpl.reset();
        pBleScanSettingsImpl.reset();
    }

public:
    std::unique_ptr<BleAdvertiserSettingsImpl> pBleAdvertiserSettingsImpl;
    std::unique_ptr<BlePeripheralDevice> pBlePeripheralDevice;
    std::unique_ptr<BleScanFilterImpl> pBleScanFilterImpl;
    std::unique_ptr<BleAdvertiserDataImpl> pBleAdvertiserDataImpl;
    std::unique_ptr<BleScanSettingsImpl> pBleScanSettingsImpl;
};

/**
 * @tc.name: SetTxPower_00100
 * @tc.desc: Verify SetTxPower function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetTxPower_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetTxPower_00100 start");
    pBleAdvertiserSettingsImpl->SetTxPower(-1);
    EXPECT_EQ(pBleAdvertiserSettingsImpl->GetTxPower(), -1);
    HILOGI("BLE_ModuleTest_SetTxPower_00100 end");
}

/**
 * @tc.name: GetServiceUuid_00100
 * @tc.desc: Verify GetServiceUuid function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetServiceUuid_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetServiceUuid_00100 start");
    Uuid uuid = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    pBleScanFilterImpl->SetServiceUuid(uuid);
    EXPECT_EQ(pBleScanFilterImpl->GetServiceUuid().ToString(), uuid.ToString());
    HILOGI("BLE_ModuleTest_GetServiceUuid_00100 end");
}

/**
 * @tc.name: GetServiceUuidMask_00100
 * @tc.desc: Verify GetServiceUuidMask function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetServiceUuidMask_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetServiceUuidMask_00100 start");
    Uuid uuid = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    pBleScanFilterImpl->SetServiceUuidMask(uuid);
    EXPECT_EQ(pBleScanFilterImpl->GetServiceUuidMask().ToString(), uuid.ToString());
    EXPECT_TRUE(pBleScanFilterImpl->HasServiceUuidMask());
    HILOGI("BLE_ModuleTest_GetServiceUuidMask_00100 end");
}


/**
 * @tc.name: GetServiceSolicitationUuid_00100
 * @tc.desc: Verify GetServiceSolicitationUuid function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetServiceSolicitationUuid_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetServiceSolicitationUuid_00100 start");
    Uuid uuid = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    pBleScanFilterImpl->SetServiceSolicitationUuid(uuid);
    EXPECT_EQ(pBleScanFilterImpl->GetServiceSolicitationUuid().ToString(), uuid.ToString());
    HILOGI("BLE_ModuleTest_GetServiceSolicitationUuid_00100 end");
}

/**
 * @tc.name: GetServiceSolicitationUuidMask_00100
 * @tc.desc: Verify GetServiceSolicitationUuidMask function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetServiceSolicitationUuidMask_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetServiceSolicitationUuidMask_00100 start");
    Uuid uuid = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    pBleScanFilterImpl->SetServiceSolicitationUuidMask(uuid);
    EXPECT_EQ(pBleScanFilterImpl->GetServiceSolicitationUuidMask().ToString(), uuid.ToString());
    EXPECT_FALSE(pBleScanFilterImpl->HasSolicitationUuid());
    EXPECT_TRUE(pBleScanFilterImpl->HasSolicitationUuidMask());
    HILOGI("BLE_ModuleTest_GetServiceSolicitationUuidMask_00100 end");
}

/**
 * @tc.name: GetManufacturerId_00100
 * @tc.desc: Verify GetManufacturerId function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetManufacturerId_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetManufacturerId_00100 start");
    pBleScanFilterImpl->SetManufacturerId(1);
    EXPECT_EQ(pBleScanFilterImpl->GetManufacturerId(), 1);
    HILOGI("BLE_ModuleTest_GetManufacturerId_00100 end");
}

/**
 * @tc.name: GetClientId_00100
 * @tc.desc: Verify GetClientId function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetClientId_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetClientId_00100 start");
    pBleScanFilterImpl->SetClientId(1);
    EXPECT_EQ(pBleScanFilterImpl->GetClientId(), 1);
    HILOGI("BLE_ModuleTest_GetClientId_00100 end");
}

/**
 * @tc.name: GetFiltIndex_00100
 * @tc.desc: Verify GetFiltIndex function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetFiltIndex_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetFiltIndex_00100 start");
    pBleScanFilterImpl->SetFiltIndex(1);
    EXPECT_EQ(pBleScanFilterImpl->GetFiltIndex(), 1);
    HILOGI("BLE_ModuleTest_GetFiltIndex_00100 end");
}

/**
 * @tc.name: GetFilterAction_00100
 * @tc.desc: Verify GetFilterAction function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetFilterAction_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetFilterAction_00100 start");
    pBleScanFilterImpl->SetFilterAction(1);
    EXPECT_EQ(pBleScanFilterImpl->GetFilterAction(), 1);
    HILOGI("BLE_ModuleTest_GetFilterAction_00100 end");
}

/**
 * @tc.name: GetAdvIndReportFlag_00100
 * @tc.desc: Verify GetAdvIndReportFlag function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetAdvIndReportFlag_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetAdvIndReportFlag_00100 start");
    pBleScanFilterImpl->SetAdvIndReportFlag(true);
    EXPECT_TRUE(pBleScanFilterImpl->GetAdvIndReportFlag());
    HILOGI("BLE_ModuleTest_GetAdvIndReportFlag_00100 end");
}

/**
 * @tc.name: IsConnectable_00100
 * @tc.desc: Verify IsConnectable function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_IsConnectable_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_IsConnectable_00100 start");
    pBleAdvertiserSettingsImpl->SetConnectable(true);
    EXPECT_TRUE(pBleAdvertiserSettingsImpl->IsConnectable());
    HILOGI("BLE_ModuleTest_IsConnectable_00100 end");
}

/**
 * @tc.name:IsLegacyMode_00100
 * @tc.desc: Verify IsLegacyMode function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_IsLegacyMode_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_IsLegacyMode_00100 start");
    pBleAdvertiserSettingsImpl->SetLegacyMode(true);
    EXPECT_TRUE(pBleAdvertiserSettingsImpl->IsLegacyMode());
    HILOGI("BLE_ModuleTest_IsLegacyMode_00100 end");
}

/**
 * @tc.name: GetInterval_00100
 * @tc.desc: Verify GetInterval function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetInterval_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetInterval_00100 start");
    pBleAdvertiserSettingsImpl->SetInterval(1);
    EXPECT_EQ(pBleAdvertiserSettingsImpl->GetInterval(), 1);
    HILOGI("BLE_ModuleTest_GetInterval_00100 end");
}

/**
 * @tc.name: GetTxPower_00100
 * @tc.desc: Verify GetTxPower function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetTxPower_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetTxPower_00100 start");
    pBleAdvertiserSettingsImpl->SetTxPower(4);
    pBleAdvertiserSettingsImpl->SetTxPower(-1);
    pBleAdvertiserSettingsImpl->SetTxPower(0);
    pBleAdvertiserSettingsImpl->SetTxPower(1);
    pBleAdvertiserSettingsImpl->SetTxPower(2);
    pBleAdvertiserSettingsImpl->SetTxPower(3);
    EXPECT_EQ(pBleAdvertiserSettingsImpl->GetTxPower(), 3);
    HILOGI("BLE_ModuleTest_GetTxPower_00100 end");
}

/**
 * @tc.name: GetPrimaryPhy_00100
 * @tc.desc: Verify GetPrimaryPhy function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetPrimaryPhy_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetPrimaryPhy_00100 start");
    pBleAdvertiserSettingsImpl->SetPrimaryPhy(1);
    EXPECT_EQ(pBleAdvertiserSettingsImpl->GetPrimaryPhy(), 1);
    HILOGI("BLE_ModuleTest_GetPrimaryPhy_00100 end");
}

/**
 * @tc.name: GetSecondaryPhy_00100
 * @tc.desc: Verify GetSecondaryPhy function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetSecondaryPhy_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetSecondaryPhy_00100 start");
    int phy = 1;
    pBleAdvertiserSettingsImpl->SetSecondaryPhy(phy);
    EXPECT_EQ(pBleAdvertiserSettingsImpl->GetSecondaryPhy(), phy);
    HILOGI("BLE_ModuleTest_GetSecondaryPhy_00100 end");
}
 
/**
 * @tc.name: GetOwnAddr_00100
 * @tc.desc: Verify GetOwnAddr function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetOwnAddr_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetOwnAddr_00100 start");
    std::array<uint8_t, RawAddress::BT_ADDRESS_BYTE_LEN> arr = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    pBleAdvertiserSettingsImpl->SetOwnAddr(arr);
    EXPECT_EQ(pBleAdvertiserSettingsImpl->GetOwnAddr(), arr);
    HILOGI("BLE_ModuleTest_GetOwnAddr_00100 end");
}
 
/**
 * @tc.name: GetOwnAddrType_00100
 * @tc.desc: Verify GetOwnAddrType function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetOwnAddrType_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetOwnAddrType_00100 start");
    int8_t addType = 1;
    pBleAdvertiserSettingsImpl->SetOwnAddrType(addType);
    EXPECT_EQ(pBleAdvertiserSettingsImpl->GetOwnAddrType(), addType);
    HILOGI("BLE_ModuleTest_GetOwnAddrType_00100 end");
}

/**
 * @tc.name: AddServiceData_00100
 * @tc.desc: Verify AddServiceData function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_AddServiceData_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_AddServiceData_00100 start");
    Uuid uuid = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    std::string data = "testData";
    pBleAdvertiserDataImpl->AddServiceUuid(uuid);
    pBleAdvertiserDataImpl->AddServiceData(uuid, data);
    uuid = Uuid::ConvertFromString("01010000-0000-1000-8000-00805F9B34FB");
    pBleAdvertiserDataImpl->AddServiceUuid(uuid);
    pBleAdvertiserDataImpl->AddServiceData(uuid, data);
    uuid = Uuid::ConvertFromString("01010000-1000-1000-8000-00805F9B34FB");
    pBleAdvertiserDataImpl->AddServiceUuid(uuid);
    pBleAdvertiserDataImpl->AddServiceData(uuid, data);
    pBleAdvertiserDataImpl->SetAppearance(0x00);
    EXPECT_EQ(pBleAdvertiserDataImpl->AddManufacturerData(1, data), RET_NO_ERROR);
    HILOGI("BLE_ModuleTest_AddServiceData_00100 end");
}

/**
 * @tc.name: AddServiceUuids_00100
 * @tc.desc: Verify AddServiceUuids function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_AddServiceUuids_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_AddServiceUuids_00100 start");
    Uuid uuid = Uuid::ConvertFromString("0000000a-0000-1000-8000-00805F9B34FB");
    Uuid uuid1 = Uuid::ConvertFromString("00000004-0000-1000-8000-00805F9B34FB");
    Uuid uuid2 = Uuid::ConvertFromString("c2808859-1b95-458a-aa8a-442333d8d728");
    Uuid uuid3 = Uuid::ConvertFromString("c2808855-1b95-458a-aa8a-442333d8d728");
    Uuid uuid4 = Uuid::ConvertFromString("12345678-9ABC-1000-8000-00805F9B34FB");
    Uuid uuid5 = Uuid::ConvertFromString("12345678-9ABA-1000-8000-00805F9B34FB");

    std::vector<Uuid> v1;
    std::vector<Uuid> v2;
    std::vector<Uuid> v3;
    std::vector<Uuid> v4;
    v1.push_back(uuid);
    v2.push_back(uuid);
    v2.push_back(uuid1);
    v3.push_back(uuid2);
    v3.push_back(uuid3);
    v4.push_back(uuid4);
    v4.push_back(uuid5);
    pBleAdvertiserDataImpl->AddServiceUuids(v1);
    pBleAdvertiserDataImpl->AddServiceUuids(v2);
    pBleAdvertiserDataImpl->AddServiceUuids(v3);
    pBleAdvertiserDataImpl->AddServiceUuids(v4);
    EXPECT_TRUE(pBleAdvertiserDataImpl != nullptr);
    HILOGI("BLE_ModuleTest_AddServiceUuids_00100 end");
}

/**
 * @tc.name: AddCharacteristicValue
 * @tc.desc: Verify AddCharacteristicValue function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_AddCharacteristicValue_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_AddCharacteristicValue_00100 start");
    uint8_t number = 1;
    std::string data = "testData";
    pBleAdvertiserDataImpl->AddCharacteristicValue(number, data);
    pBleAdvertiserDataImpl->SetTxPowerLevel(number);
    pBleAdvertiserDataImpl->SetFlags(number);
    EXPECT_EQ(pBleAdvertiserDataImpl->GetFlags(), number);
    HILOGI("BLE_ModuleTest_AddCharacteristicValue_00100 end");
}
 
/**
 * @tc.name: SetPayload
 * @tc.desc: Verify SetPayload function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetPayload_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetPayload_00100 start");
    std::string data = "testData";
    std::string data1 = "testDatatestDatatestDatatestData";
    pBleAdvertiserDataImpl->SetDeviceName(data);
    pBleAdvertiserDataImpl->SetDeviceName(data1);
    pBleAdvertiserDataImpl->SetPayload(data);
    EXPECT_EQ(pBleAdvertiserDataImpl->GetPayload(), data);
    HILOGI("BLE_ModuleTest_SetPayload_00100 end");
}

/**
 * @tc.name: IsAclConnected_00100
 * @tc.desc: Verify IsAclConnected function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_IsAclConnected_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_IsAclConnected_00100 start");

    EXPECT_FALSE(pBlePeripheralDevice->IsAclConnected());
    int connectState = 1;
    pBlePeripheralDevice->SetAclConnectState(connectState);
    EXPECT_TRUE(pBlePeripheralDevice->IsAclConnected());

    HILOGI("BLE_ModuleTest_IsAclConnected_00100 end");
}

/**
 * @tc.name: IsAclEncrypted_00100
 * @tc.desc: Verify IsAclEncrypted function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_IsAclEncrypted_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_IsAclEncrypted_00100 start");

    EXPECT_FALSE(pBlePeripheralDevice->IsAclEncrypted());
    int connectState = 0x02;
    pBlePeripheralDevice->SetAclConnectState(connectState);
    EXPECT_TRUE(pBlePeripheralDevice->IsAclEncrypted());

    HILOGI("BLE_ModuleTest_IsAclEncrypted_00100 end");
}

/**
 * @tc.name: GetName_00100
 * @tc.desc: Verify GetName function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetName_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetName_00100 start");
    std::string data = "testData";
    pBlePeripheralDevice->SetName(data);
    EXPECT_EQ(pBlePeripheralDevice->GetName(), data);
    HILOGI("BLE_ModuleTest_GetName_00100 end");
}

/**
 * @tc.name: GetServiceData_00100
 * @tc.desc: Verify GetServiceData function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetServiceData_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetServiceData_00100 start");
    Uuid uuid = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    std::string data = "testData";
    pBlePeripheralDevice->SetServiceDataUUID(uuid, data);
    pBlePeripheralDevice->SetServiceDataUUID(uuid, data);
    std::vector<std::string> stringData = pBlePeripheralDevice->GetServiceData();
    std::vector<Uuid> uuidData = pBlePeripheralDevice->GetServiceDataUUID();
    EXPECT_EQ(stringData[0], data);
    EXPECT_EQ(pBlePeripheralDevice->GetServiceData(0), data);
    EXPECT_TRUE(pBlePeripheralDevice->IsServiceData());
    EXPECT_EQ(uuidData[0].ToString(), uuid.ToString());
    EXPECT_EQ(pBlePeripheralDevice->GetServiceDataUUID(0).ToString(), uuid.ToString());
    HILOGI("BLE_ModuleTest_GetServiceData_00100 end");
}

/**
 * @tc.name: GetServiceUUID_00100
 * @tc.desc: Verify GetServiceUUID function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetServiceUUID_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetServiceUUID_00100 start");
    Uuid uuid = Uuid::ConvertFromString("00000000-0000-1000-8000-00805F9B34FB");
    pBlePeripheralDevice->SetServiceUUID(uuid);
    pBlePeripheralDevice->SetServiceUUID(uuid);
    std::vector<Uuid> uuidData = pBlePeripheralDevice->GetServiceUUID();
    EXPECT_TRUE(pBlePeripheralDevice->IsServiceUUID());
    EXPECT_EQ(uuidData[0].ToString(), uuid.ToString());
    EXPECT_EQ(pBlePeripheralDevice->GetServiceUUID(0).ToString(), uuid.ToString());
    HILOGI("BLE_ModuleTest_GetServiceUUID_00100 end");
}

/**
 * @tc.name: GetAddressType_00100
 * @tc.desc: Verify GetAddressType function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetAddressType_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetAddressType_00100 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    pBlePeripheralDevice->SetAddress(device);
    pBlePeripheralDevice->SetAddressType(0x00);
    EXPECT_EQ(pBlePeripheralDevice->GetAddressType(), 0x00);
    HILOGI("BLE_ModuleTest_GetAddressType_00100 end");
}

/**
 * @tc.name: GetRSSI_00100
 * @tc.desc: Verify GetRSSI function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetRSSI_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetRSSI_00100 start");
    pBlePeripheralDevice->SetRSSI(0x00);
    EXPECT_EQ(pBlePeripheralDevice->GetRSSI(), 0x00);
    EXPECT_TRUE(pBlePeripheralDevice->IsRSSI());
    HILOGI("BLE_ModuleTest_GetRSSI_00100 end");
}

/**
 * @tc.name: ParseAdvertiserment_00100
 * @tc.desc: Verify ParseAdvertiserment function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_ParseAdvertiserment_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_ParseAdvertiserment_00100 start");
    uint8_t* testdata = new uint8_t(3);
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData = {testdata, 0};
    pBlePeripheralDevice->ParseAdvertiserment(blePeripheralDeviceParseAdvData);
    delete testdata;
    testdata = nullptr;
    HILOGI("BLE_ModuleTest_ParseAdvertiserment_00100 end");
}

/**
 * @tc.name: BLE_ModuleTest_ParseAdvertiserment_00200
 * @tc.desc: Verify ParseAdvertiserment function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_ParseAdvertiserment_00200, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_ParseAdvertiserment_00200 start");
    uint8_t testdata[6] = {0, 1, 2, 3, 4, 5};
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData = {testdata, 1};
    pBlePeripheralDevice->ParseAdvertiserment(blePeripheralDeviceParseAdvData);
    HILOGI("BLE_ModuleTest_ParseAdvertiserment_00200 end");
}

/**
 * @tc.name: BLE_ModuleTest_ParseAdvertiserment_00300
 * @tc.desc: Verify ParseAdvertiserment function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_ParseAdvertiserment_00300, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_ParseAdvertiserment_00300 start");
    uint8_t testdata[6] = {1, 1, 2, 3, 4, 5};
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData = {testdata, 3};
    pBlePeripheralDevice->ParseAdvertiserment(blePeripheralDeviceParseAdvData);
    HILOGI("BLE_ModuleTest_ParseAdvertiserment_00300 end");
}

/**
 * @tc.name: BuildAdvertiserData_00100
 * @tc.desc: Verify BuildAdvertiserData function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_BuildAdvertiserData_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_BuildAdvertiserData_00100 start");
    uint8_t* testdata = new uint8_t(3);
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData = {testdata, 0};
    pBlePeripheralDevice->BuildAdvertiserData(0x09, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x0A, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x19, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x01, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x02, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x04, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x07, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0xFF, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x16, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x20, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x21, blePeripheralDeviceParseAdvData);
    pBlePeripheralDevice->BuildAdvertiserData(0x22, blePeripheralDeviceParseAdvData);
    delete testdata;
    testdata = nullptr;
    HILOGI("BLE_ModuleTest_BuildAdvertiserData_00100 end");
}

/**
 * @tc.name: SetServiceDataUUID16Bits_00100
 * @tc.desc: Verify SetServiceDataUUID16Bits function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, SetServiceDataUUID16Bits_00100, TestSize.Level1)
{
    HILOGI("SetServiceDataUUID16Bits_00100 start");
    uint8_t* testdata = new uint8_t(3);
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData = {testdata, 3};
    pBlePeripheralDevice->SetServiceDataUUID16Bits(blePeripheralDeviceParseAdvData);
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData1 = {testdata, 2};
    pBlePeripheralDevice->SetServiceDataUUID16Bits(blePeripheralDeviceParseAdvData1);
    HILOGI("SetServiceDataUUID16Bits_00100 end");
}

/**
 * @tc.name: SetServiceDataUUID32Bits_00100
 * @tc.desc: Verify SetServiceDataUUID32Bits function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, SetServiceDataUUID32Bits_00100, TestSize.Level1)
{
    HILOGI("SetServiceDataUUID32Bits_00100 start");
    uint8_t* testdata = new uint8_t(3);
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData = {testdata, 5};
    pBlePeripheralDevice->SetServiceDataUUID32Bits(blePeripheralDeviceParseAdvData);
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData1 = {testdata, 4};
    pBlePeripheralDevice->SetServiceDataUUID32Bits(blePeripheralDeviceParseAdvData1);
    HILOGI("SetServiceDataUUID32Bits_00100 end");
}

/**
 * @tc.name: SetServiceDataUUID128Bits_00100
 * @tc.desc: Verify SetServiceDataUUID128Bits function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, SetServiceDataUUID128Bits_00100, TestSize.Level1)
{
    HILOGI("SetServiceDataUUID128Bits_00100 start");
    uint8_t* testdata = new uint8_t(3);
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData = {testdata, 17};
    pBlePeripheralDevice->SetServiceDataUUID128Bits(blePeripheralDeviceParseAdvData);
    BlePeripheralDeviceParseAdvData blePeripheralDeviceParseAdvData1 = {testdata, 16};
    pBlePeripheralDevice->SetServiceDataUUID128Bits(blePeripheralDeviceParseAdvData1);
    HILOGI("SetServiceDataUUID128Bits_00100 end");
}

/**
 * @tc.name: IsName_00100
 * @tc.desc: Verify IsName function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_IsName_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_IsName_00100 start");
    std::string data = "testData";
    pBlePeripheralDevice->SetName(data);
    EXPECT_TRUE(pBlePeripheralDevice->IsName());
    EXPECT_EQ(pBlePeripheralDevice->GetName(), data);
    HILOGI("BLE_ModuleTest_IsName_00100 end");
}

/**
 * @tc.name: IsBondedFromLocal_00100
 * @tc.desc: Verify IsBondedFromLocal function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_IsBondedFromLocal_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_IsBondedFromLocal_00100 start");
    pBlePeripheralDevice->SetBondedFromLocal(true);
    EXPECT_TRUE(pBlePeripheralDevice->IsBondedFromLocal());
    HILOGI("BLE_ModuleTest_IsBondedFromLocal_00100 end");
}

/**
 * @tc.name: GetConnectionHandle_00100
 * @tc.desc: Verify GetConnectionHandle function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetConnectionHandle_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetConnectionHandle_00100 start");
    pBlePeripheralDevice->SetConnectionHandle(0x00);
    EXPECT_EQ(pBlePeripheralDevice->GetConnectionHandle(), 0x00);
    HILOGI("BLE_ModuleTest_GetConnectionHandle_00100 end");
}

/**
 * @tc.name: GetDeviceType_00100
 * @tc.desc: Verify GetDeviceType function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetDeviceType_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetDeviceType_00100 start");
    pBlePeripheralDevice->SetAdFlag(0x00);
    EXPECT_EQ(pBlePeripheralDevice->GetAdFlag(), 0x00);
    EXPECT_EQ(pBlePeripheralDevice->GetDeviceType(), 0x01);
    pBlePeripheralDevice->SetAdFlag(0x04);
    EXPECT_EQ(pBlePeripheralDevice->GetDeviceType(), 0x02);
    HILOGI("BLE_ModuleTest_GetDeviceType_00100 end");
}

/**
 * @tc.name: GetPairedStatus_00100
 * @tc.desc: Verify GetPairedStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetPairedStatus_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetPairedStatus_00100 start");
    EXPECT_TRUE(pBlePeripheralDevice->SetPairedStatus(0x02));
    EXPECT_EQ(pBlePeripheralDevice->GetPairedStatus(), 0x02);
    EXPECT_TRUE(pBlePeripheralDevice->SetPairedStatus(0x02));
    EXPECT_EQ(pBlePeripheralDevice->GetPairedStatus(), 0x02);
    EXPECT_FALSE(pBlePeripheralDevice->SetPairedStatus(0x00));
    EXPECT_EQ(pBlePeripheralDevice->GetPairedStatus(), 0x02);
    EXPECT_FALSE(pBlePeripheralDevice->SetPairedStatus(0x05));
    EXPECT_EQ(pBlePeripheralDevice->GetPairedStatus(), 0x02);
    HILOGI("BLE_ModuleTest_GetPairedStatus_00100 end");
}

/**
 * @tc.name: SetManufacturerData_00100
 * @tc.desc: Verify SetManufacturerData function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetManufacturerData_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetManufacturerData_00100 start");
    pBlePeripheralDevice->SetManufacturerData("testData");
    std::map<uint16_t, std::string> actualData = pBlePeripheralDevice->GetManufacturerData();
    EXPECT_FALSE(actualData.empty());
    HILOGI("BLE_ModuleTest_SetManufacturerData_00100 end");
}

/**
 * @tc.name: SetTxPowerLevel_00100
 * @tc.desc: Verify SetTxPowerLevel function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetTxPowerLevel_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetTxPowerLevel_00100 start");
    int8_t txPower = 1;
    pBlePeripheralDevice->SetTXPower(txPower);
    EXPECT_EQ(pBlePeripheralDevice->GetTXPower(), txPower);
    EXPECT_EQ(pBlePeripheralDevice->IsTXPower(), true);
    HILOGI("BLE_ModuleTest_SetTxPowerLevel_00100 end");
}

/**
 * @tc.name: SetAdvertisingData_00100
 * @tc.desc: Verify SetAdvertisingData function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetAdvertisingData_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetAdvertisingData_00100 start");
    uint8_t advType1 = 0xFF;
    std::string advData1 = "testManufactureData";
    pBlePeripheralDevice->AddAdvertisingData(advType1, advData1);
    EXPECT_EQ(pBlePeripheralDevice->IsAdvertisingData(), true);
    auto it1 = pBlePeripheralDevice->GetAdvertisingData().find(advType1);
    if (it1 != pBlePeripheralDevice->GetAdvertisingData().end()) {
        EXPECT_EQ(it1->second, advData1);
    }
    HILOGI("BLE_ModuleTest_SetAdvertisingData_00100 end");
}

/**
 * @tc.name: GetEventType_00100
 * @tc.desc: Verify GetEventType function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_GetEventType_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_GetEventType_00100 start");
    pBlePeripheralDevice->SetEventType(0);
    EXPECT_EQ(pBlePeripheralDevice->GetEventType(), 0);
    HILOGI("BLE_ModuleTest_GetEventType_00100 end");
}

/**
 * @tc.name: SetCallbackType_00100
 * @tc.desc: Verify SetCallbackType function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetCallbackType_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetCallbackType_00100 start");
    pBleScanSettingsImpl->SetCallbackType(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetCallbackType(), 1);
    HILOGI("BLE_ModuleTest_SetCallbackType_00100 end");
}

/**
 * @tc.name: SetMatchTrackAdvType_00100
 * @tc.desc: Verify SetMatchTrackAdvType function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetMatchTrackAdvType_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetMatchTrackAdvType_00100 start");
    pBleScanSettingsImpl->SetMatchTrackAdvType(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetMatchTrackAdvType(), 1);
    HILOGI("BLE_ModuleTest_SetMatchTrackAdvType_00100 end");
}

/**
 * @tc.name: SetMatchMode
 * @tc.desc: Verify SetMatchMode function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetMatchMode_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetMatchMode_00100 start");
    FilterIdxInfo info;
    FilterIdxInfo otherInfo;
    if (info == otherInfo) {
        HILOGI("FilterIdxInfo_ModuleTest_operator_00100 info == otherInfo");
    }
    pBleScanSettingsImpl->SetMatchMode(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetMatchMode(), 1);
    HILOGI("BLE_ModuleTest_SetMatchMode_00100 end");
}

/**
 * @tc.name: SetReportDelay
 * @tc.desc: Verify SetReportDelay function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetReportDelay_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetReportDelay_00100 start");
    pBleScanSettingsImpl->SetReportDelay(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetReportDelayMillisValue(), 1);
    HILOGI("BLE_ModuleTest_SetReportDelay_00100 end");
}

/**
 * @tc.name: SetScanMode
 * @tc.desc: Verify SetScanMode function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetScanMode_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetScanMode_00100 start");
    pBleScanSettingsImpl->SetScanMode(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetScanMode(), 1);
    HILOGI("BLE_ModuleTest_SetScanMode_00100 end");
}

/**
 * @tc.name: SetLegacy
 * @tc.desc: Verify SetLegacy function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetLegacy_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetLegacy_00100 start");
    pBleScanSettingsImpl->SetLegacy(true);
    EXPECT_TRUE(pBleScanSettingsImpl->GetLegacy());
    HILOGI("BLE_ModuleTest_SetLegacy_00100 end");
}

/**
 * @tc.name: SetSensitivityMode
 * @tc.desc: Verify SetSensitivityMode function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetSensitivityMode_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetSensitivityMode_00100 start");
    pBleScanSettingsImpl->SetSensitivityMode(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetSensitivityMode(), 1);
    HILOGI("BLE_ModuleTest_SetSensitivityMode_00100 end");
}

/**
 * @tc.name: SetScanInterval
 * @tc.desc: Verify SetScanInterval function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetScanInterval_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetScanInterval_00100 start");
    pBleScanSettingsImpl->SetScanInterval(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetScanInterval(), 1);
    HILOGI("BLE_ModuleTest_SetScanInterval_00100 end");
}


/**
 * @tc.name: SetReportMode
 * @tc.desc: Verify SetReportMode function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetReportMode_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetReportMode_00100 start");
    pBleScanSettingsImpl->SetReportMode(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetReportMode(), 1);
    HILOGI("BLE_ModuleTest_SetReportMode_00100 end");
}

/**
 * @tc.name: SetPhy
 * @tc.desc: Verify SetPhy function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetPhy_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetPhy_00100 start");
    pBleScanSettingsImpl->SetPhy(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetPhy(), 1);
    HILOGI("BLE_ModuleTest_SetPhy_00100 end");
}

/**
 * @tc.name: SetScanWindow
 * @tc.desc: Verify SetScanWindow function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_SetScanWindow_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_SetScanWindow_00100 start");
    pBleScanSettingsImpl->SetScanWindow(1);
    EXPECT_EQ(pBleScanSettingsImpl->GetScanWindow(), 1);
    HILOGI("BLE_ModuleTest_SetScanWindow_00100 end");
}

/**
 * @tc.name: operator
 * @tc.desc: Verify operator function.
 * @tc.type: FUNC
 */
HWTEST_F(BleServiceDataTest, BLE_ModuleTest_operator_00100, TestSize.Level1)
{
    HILOGI("BLE_ModuleTest_operator_00100 start");
    BleScanSettingsImpl* settings1 = new BleScanSettingsImpl();
    BleScanSettingsImpl* settings2 = new BleScanSettingsImpl();
    settings1->SetLegacy(true);
    settings1->SetPhy(1);
    settings1->SetReportDelay(100);
    settings1->SetScanMode(2);
    settings1->SetCallbackType(3);
    settings1->SetMatchTrackAdvType(4);
    settings1->SetMatchMode(5);

    settings2->SetLegacy(true);
    settings2->SetPhy(1);
    settings2->SetReportDelay(100);
    settings2->SetScanMode(2);
    settings2->SetCallbackType(3);
    settings2->SetMatchTrackAdvType(4);
    settings2->SetMatchMode(5);
    EXPECT_TRUE(*settings1 == *settings2);
    HILOGI("BLE_ModuleTest_operator_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS