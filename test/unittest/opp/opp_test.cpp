/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "bluetooth_errorcode.h"
#include "bluetooth_opp.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_log.h"
#include "napi_bluetooth_utils.h"
#include "bluetooth_opp_transfer_information.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace std;

class OppObserverTest : public OppObserver {
public:
    OppObserverTest() = default;
    virtual ~OppObserverTest() = default;
    virtual void OnReceiveIncomingFileChanged(const BluetoothOppTransferInformation &transferInformation) {}
    virtual void OnTransferStateChanged(const BluetoothOppTransferInformation &transferInformation) {}

private:
};

constexpr int TIME = 2;
static Bluetooth::Opp  *profile_;
static std::shared_ptr<OppObserverTest> oppObserverTest = std::make_shared<OppObserverTest>();
static BluetoothHost *host_;

class OppTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
        std::vector<std::string> bluetoothRemoteDeviceAddr);
    bool CompareTransferInformation(const BluetoothOppTransferInformation &information,
        const BluetoothOppTransferInformation &transferInformation);

private:
};

void OppTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBleEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    profile_ = Opp::GetProfile();

    HILOGI("SetUpTestCase end");
}

void OppTest::TearDownTestCase(void)
{
    host_ = nullptr;
}

void OppTest::SetUp()
{
}

void OppTest::TearDown()
{
}

bool OppTest::CompareDevice(std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState,
    std::vector<std::string> bluetoothRemoteDeviceAddr)
{
    if (bluetoothRemoteDeviceByState.size() != bluetoothRemoteDeviceAddr.size()) {
        return false;
    }
    for (int i = 0; i < bluetoothRemoteDeviceByState.size(); i++) {
        if (bluetoothRemoteDeviceByState[i].GetDeviceAddr().compare(bluetoothRemoteDeviceAddr[i]) != 0) {
            return false;
        }
    }
    return true;
}

bool OppTest::CompareTransferInformation(const BluetoothOppTransferInformation &information,
    const BluetoothOppTransferInformation &transferInformation)
{
    if (&information == &transferInformation) {
        return true;
    }
    if ((information.GetFileName() != transferInformation.GetFileName()) ||
        (information.GetFilePath() != transferInformation.GetFilePath()) ||
        (information.GetMimeType() != transferInformation.GetMimeType()) ||
        (information.GetDeviceName() != transferInformation.GetDeviceName()) ||
        (information.GetDeviceAddress() != transferInformation.GetDeviceAddress()) ||
        (information.GetTimeStamp() != transferInformation.GetTimeStamp()) ||
        (information.GetCurrentBytes() != transferInformation.GetCurrentBytes()) ||
        (information.GetTotalBytes() != transferInformation.GetTotalBytes()) ||
        (information.GetResult() != transferInformation.GetResult()) ||
        (information.GetDirection() != transferInformation.GetDirection()) ||
        (information.GetStatus() != transferInformation.GetStatus()) ||
        (information.GetId() != transferInformation.GetId())) {
        return false;
    }
    return true;
}

/*
 * @tc.number: OPP_ModuleTest_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_GetDevicesByStates_00100 start");

    HILOGI("GetDevicesByStates function test");

    profile_ = Opp::GetProfile();
    std::vector<int> states;
    std::vector<std::string> bluetoothRemoteDeviceAddr;
    std::vector<BluetoothRemoteDevice> bluetoothRemoteDeviceByState;
    profile_->GetDevicesByStates(states, bluetoothRemoteDeviceByState);
    EXPECT_TRUE(CompareDevice(bluetoothRemoteDeviceByState, bluetoothRemoteDeviceAddr));

    HILOGI("OPP_ModuleTest_GetDevicesByStates_00100 end");
}


/*
 * @tc.number: OPP_ModuleTest_GetDeviceState_00100
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_GetDeviceState_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_GetDeviceState_00100 start");

    HILOGI("GetDeviceState function test");

    BluetoothRemoteDevice device;
    profile_ = Opp::GetProfile();
    int32_t result = false;
    EXPECT_TRUE(profile_->GetDeviceState(device, result) > 0);

    HILOGI("OPP_ModuleTest_GetDeviceState_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_SetLastReceivedFileUri_00100
 * @tc.name: SetLastReceivedFileUri
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_SetLastReceivedFileUri_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_SetLastReceivedFileUri_00100 start");
    std::string device;
    profile_ = Opp::GetProfile();
    EXPECT_TRUE(profile_->SetLastReceivedFileUri("test.jpg") > 0);
    HILOGI("OPP_ModuleTest_SetLastReceivedFileUri_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_SendFile_00100
 * @tc.name: SendFile
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_SendFile_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_SendFile_00100 start");
    std::string device;
    profile_ = Opp::GetProfile();
    bool result = false;
    std::vector<BluetoothOppFileHolder> fileHolders;
    BluetoothOppFileHolder fileHolder("test.jpg", 100, 20);
    fileHolders.push_back(fileHolder);
    EXPECT_TRUE(profile_->SendFile(device, fileHolders, result) > 0);
    HILOGI("OPP_ModuleTest_SendFile_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_SetIncomingFileConfirmation_00100
 * @tc.name: SetIncomingFileConfirmation
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_SetIncomingFileConfirmation_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_SetIncomingFileConfirmation_00100 start");
    bool accept = true;
    profile_ = Opp::GetProfile();
    EXPECT_TRUE(profile_->SetIncomingFileConfirmation(accept, 1) > 0);
    HILOGI("OPP_ModuleTest_SetIncomingFileConfirmation_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_GetCurrentTransferInformation_00100
 * @tc.name: GetCurrentTransferInformation
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_GetCurrentTransferInformation_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_GetCurrentTransferInformation_00100 start");
    profile_ = Opp::GetProfile();
    BluetoothOppTransferInformation Infermation;
    BluetoothOppTransferInformation transferInfermation;
    profile_->GetCurrentTransferInformation(transferInfermation);
    EXPECT_TRUE(CompareTransferInformation(Infermation, transferInfermation));
    HILOGI("OPP_ModuleTest_GetCurrentTransferInformation_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_CancelTransfer_00100
 * @tc.name: CancelTransfer
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_CancelTransfer_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_CancelTransfer_00100 start");

    profile_ = Opp::GetProfile();
    bool result = false;
    EXPECT_TRUE(profile_->CancelTransfer(result) > 0);
    HILOGI("OPP_ModuleTest_CancelTransfer_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_RegisterObserver_00100
 * @tc.name: RegisterObserver
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_RegisterObserver_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_RegisterObserver_00100 start");
    profile_ = Opp::GetProfile();
    profile_->RegisterObserver(oppObserverTest);
    EXPECT_TRUE(oppObserverTest != nullptr);
    HILOGI("OPP_ModuleTest_RegisterObserver_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_DeregisterObserver_00100
 * @tc.name: DeregisterObserver
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_DeregisterObserver_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_DeregisterObserver_00100 start");
    profile_ = Opp::GetProfile();
    profile_->DeregisterObserver(oppObserverTest);
    EXPECT_TRUE(oppObserverTest != nullptr);
    HILOGI("OPP_ModuleTest_DeregisterObserver_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_OnReceiveIncomingFileChanged_00100
 * @tc.name: OnReceiveIncomingFileChanged
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_OnReceiveIncomingFileChanged_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_OnReceiveIncomingFileChanged_00100 start");

    HILOGI("OnReceiveIncomingFileChanged function test");

    BluetoothOppTransferInformation trasnferInformation;
    oppObserverTest->OnReceiveIncomingFileChanged(trasnferInformation);

    HILOGI("OPP_ModuleTest_OnReceiveIncomingFileChanged_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_OnTransferStateChanged_00100
 * @tc.name: OnTransferStateChanged
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_OnTransferStateChanged_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_OnTransferStateChanged_00100 start");

    HILOGI("OnTransferStateChanged function test");

    BluetoothOppTransferInformation trasnferInformation;
    oppObserverTest->OnTransferStateChanged(trasnferInformation);

    HILOGI("OPP_ModuleTest_OnTransferStateChanged_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_Marshalling_00100
 * @tc.name: Marshalling
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_Marshalling_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_Marshalling_00100 start");
    BluetoothIOppTransferInformation bluetoothIOppTransferInformation;
    OHOS::Parcel parcel;
    bluetoothIOppTransferInformation.Marshalling(parcel);
    HILOGI("OPP_ModuleTest_Marshalling_00100 end");
}
 
/*
 * @tc.number: OPP_ModuleTest_Unmarshalling_00100
 * @tc.name: Unmarshalling
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_Unmarshalling_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_Unmarshalling_00100 start");
    BluetoothIOppTransferInformation bluetoothIOppTransferInformation;
    OHOS::Parcel parcel;
    bluetoothIOppTransferInformation.Unmarshalling(parcel);
    HILOGI("OPP_ModuleTest_Unmarshalling_00100 end");
}
 
/*
 * @tc.number: OPP_ModuleTest_WriteToParcel_00100
 * @tc.name: WriteToParcel
 * @tc.desc:
 */
HWTEST_F(OppTest, OPP_ModuleTest_WriteToParcel_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_WriteToParcel_00100 start");
    BluetoothIOppTransferInformation bluetoothIOppTransferInformation;
    OHOS::Parcel parcel;
    bluetoothIOppTransferInformation.WriteToParcel(parcel);
    bluetoothIOppTransferInformation.ReadFromParcel(parcel);
    HILOGI("OPP_ModuleTest_WriteToParcel_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS