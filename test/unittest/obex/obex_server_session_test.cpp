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
#include "bluetooth_log.h"
#include "obex_server_session.h"
#include "pbap_pse_obex_server.h"
#include "pbap_pse_service.h"
#include "class_creator.h"

#include "bluetooth_opp_transfer_information.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
std::string address("11:22:33:44:55:66");
std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
            51, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_RFCOMM, address);
std::shared_ptr<ObexServerTransport> obexTransport = std::make_shared<ObexServerTransport>(socketDevice);
std::shared_ptr<ObexServerSession> obexServerSession = std::make_shared<ObexServerSession>(obexTransport, nullptr);

void *PbapPseServiceCreate(void)
{
    return new PbapPseService();
}

class ObexServerSessionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
    }
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: ObexServerSession_Start_001
 * @tc.name: Start
 * @tc.desc:
 */
HWTEST_F(ObexServerSessionTest, ObexServerSession_Start_001, TestSize.Level1)
{
    HILOGI("ObexServerSession_Start_001 start");
    obexServerSession->Start();
    sleep(1);
    EXPECT_TRUE(obexServerSession->start_);
    obexServerSession->Start();
    HILOGI("ObexServerSession_Start_001 end");
}

/*
 * @tc.number: ObexServerSession_UpdateHandler_001
 * @tc.name: Run
 * @tc.desc:
 */
HWTEST_F(ObexServerSessionTest, ObexServerSession_UpdateHandler_001, TestSize.Level1)
{
    HILOGI("ObexServerSession_UpdateHandler_001 start");
    std::shared_ptr<ObexRequestHandler> handler;
    obexServerSession->UpdateHandler(handler);
    EXPECT_TRUE(obexServerSession->requestHandler_ == nullptr);
    HILOGI("ObexServerSession_UpdateHandler_001 end");
}

/*
 * @tc.number: OPP_ModuleTest_Marshalling_00100
 * @tc.name: Marshalling
 * @tc.desc:
 */
HWTEST_F(ObexServerSessionTest, OPP_ModuleTest_Marshalling_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_Marshalling_00100 start");
    Bluetooth::BluetoothIOppTransferInformation bluetoothIOppTransferInformation;
    OHOS::Parcel parcel;
    EXPECT_TRUE(bluetoothIOppTransferInformation.Marshalling(parcel));
    EXPECT_TRUE(bluetoothIOppTransferInformation.Unmarshalling(parcel));
    HILOGI("OPP_ModuleTest_Marshalling_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_WriteToParcel_00100
 * @tc.name: WriteToParcel
 * @tc.desc:
 */
HWTEST_F(ObexServerSessionTest, OPP_ModuleTest_WriteToParcel_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_WriteToParcel_00100 start");
    Bluetooth::BluetoothIOppTransferInformation bluetoothIOppTransferInformation;
    OHOS::Parcel parcel;
    EXPECT_TRUE(bluetoothIOppTransferInformation.WriteToParcel(parcel));
    EXPECT_TRUE(bluetoothIOppTransferInformation.ReadFromParcel(parcel));
    HILOGI("OPP_ModuleTest_WriteToParcel_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_Marshalling_00200
 * @tc.name: Marshalling
 * @tc.desc:
 */
HWTEST_F(ObexServerSessionTest, OPP_ModuleTest_Marshalling_00200, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_Marshalling_00100 start");
    Bluetooth::BluetoothIOppTransferFileHolder bluetoothIOppTransferFileHolder;
    OHOS::Parcel parcel;
    EXPECT_TRUE(bluetoothIOppTransferFileHolder.Marshalling(parcel));
    EXPECT_TRUE(bluetoothIOppTransferFileHolder.Unmarshalling(parcel));
    HILOGI("OPP_ModuleTest_Marshalling_00200 end");
}

/*
 * @tc.number: OPP_ModuleTest_WriteToParcel_00200
 * @tc.name: WriteToParcel
 * @tc.desc:
 */
HWTEST_F(ObexServerSessionTest, OPP_ModuleTest_WriteToParcel_00200, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_WriteToParcel_00200 start");
    Bluetooth::BluetoothIOppTransferFileHolder bluetoothIOppTransferFileHolder;
    OHOS::Parcel parcel;
    EXPECT_TRUE(bluetoothIOppTransferFileHolder.WriteToParcel(parcel));
    EXPECT_TRUE(bluetoothIOppTransferFileHolder.ReadFromParcel(parcel));
    HILOGI("OPP_ModuleTest_WriteToParcel_00200 end");
}

/*
 * @tc.number: ObexServerSession_Stop_001
 * @tc.name: Run
 * @tc.desc:
 */
HWTEST_F(ObexServerSessionTest, ObexServerSession_Stop_001, TestSize.Level1)
{
    HILOGI("ObexServerSession_Stop_001 start");
    obexServerSession->Stop();
    sleep(1);
    EXPECT_FALSE(obexServerSession->start_);
    obexServerSession->Stop();
    HILOGI("ObexServerSession_Stop_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS