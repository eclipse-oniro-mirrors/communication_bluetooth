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
#include <cstring>
#include "adapter_device_config.h"
#include "opp_service_impl.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"
#include "class_creator.h"


using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
using namespace std;

static BluetoothHost *host_;
const int TIME = 4;
static string filePath = "/data/service/el1/public/bluetooth/bluetooth_audio_manager_config.xml";

class OppServiceImplTest : public testing::Test {
public:
    OppServiceImplTest();
    ~OppServiceImplTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
    std::string addr_ = "11:22:33:44:55:66";
    std::unique_ptr<OppServiceImpl> OppServiceImplTest_;
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
    OppSdpFoundRecord oppRecord_ = {0, 0, 0, 0, "", "", "", false};
};

OppServiceImplTest::OppServiceImplTest()
{
    OppServiceImplTest_ = std::make_unique<OppServiceImpl>();
    socketDevice_ = std::make_shared<ObexSocketDevice>(-1, 0, 0, SocketType::TYPE_RFCOMM, addr_);
}

OppServiceImplTest::~OppServiceImplTest()
{
    if (OppServiceImplTest_) {
        OppServiceImplTest_->stateMachines_.clear();
    }
    OppServiceImplTest_.reset();
    socketDevice_ = nullptr;
}

void OppServiceImplTest::SetUpTestCase(void)
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
}

void OppServiceImplTest::TearDownTestCase(void)
{
}

void OppServiceImplTest::SetUp()
{
}

void OppServiceImplTest::TearDown()
{
}

/*
 * @tc.number: Opp_Service_Impl_SendFile_00100
 * @tc.name: SendFile
 * @tc.desc:
 */
HWTEST_F(OppServiceImplTest, Opp_Service_Impl_SendFile_00100, TestSize.Level1)
{
    HILOGI("Opp_Service_Impl_SendFile_00100 start");
    const RawAddress device = RawAddress(addr_);
    std::vector<std::string> filePaths;
    filePaths.push_back(filePath);
    std::vector<std::string> mimeTypes;
    mimeTypes.push_back("jpg");
    OppServiceImplTest_->StartNextTransfer();
    std::vector<IOppTransferFileHolder> fileHolderList;
    for (int i = 0; i <= 2; i++) {
        fileHolderList.push_back(IOppTransferFileHolder("test.jpg", 100, 100));
    }
    EXPECT_EQ(OppServiceImplTest_->SendFile(device, fileHolderList), RET_NO_SUPPORT);
    HILOGI("Opp_Service_Impl_SendFile_00100 end");
}

/*
 * @tc.number: Opp_Service_Impl_SetIncomingFileConfirmation_00200
 * @tc.name: SetIncomingFileConfirmation
 * @tc.desc:
 */
HWTEST_F(OppServiceImplTest, Opp_Service_Impl_SetIncomingFileConfirmation_00200, TestSize.Level1)
{
    HILOGI("Opp_Service_Impl_SetIncomingFileConfirmation_00200 start");
    OppServiceImplTest_->GetCurrentTransferInformation();
    int ret = OppServiceImplTest_->SetIncomingFileConfirmation(true, 10);
    EXPECT_EQ(ret, RET_BAD_STATUS);
    HILOGI("Opp_Service_Impl_SetIncomingFileConfirmation_00200 end");
}

/*
 * @tc.number: Opp_Service_Impl_GetDevicesByStates_00500
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(OppServiceImplTest, Opp_Service_Impl_GetDevicesByStates_00500, TestSize.Level1)
{
    HILOGI("Opp_Service_Impl_GetDevicesByStates_00500 start");
    std::vector<int> states = {0};
    OppServiceImplTest_->stateMachines_.clear();
    OppServiceImplTest_->stateMachines_[addr_] = nullptr;
    std::vector<RawAddress> devices = OppServiceImplTest_->GetDevicesByStates(states);
    EXPECT_EQ(devices.size(), 0);
    HILOGI("Opp_Service_Impl_GetDevicesByStates_00500 end");
}

/*
 * @tc.number: Opp_Service_Impl_CancelTransfer_00100
 * @tc.name: CancelTransfer
 * @tc.desc:
 */
HWTEST_F(OppServiceImplTest, Opp_Service_Impl_CancelTransfer_00100, TestSize.Level1)
{
    HILOGI("Opp_Service_Impl_CancelTransfer_00100 start");
    OppServiceImplTest_->OnObexDisconnected(addr_);
    OppServiceImplTest_->OnObexConnected(addr_);
    int ret = OppServiceImplTest_->CancelTransfer(addr_);
    EXPECT_EQ(ret, RET_BAD_STATUS);
    HILOGI("Opp_Service_Impl_CancelTransfer_00100 end");
}

/*
 * @tc.number: Opp_Service_Impl_SetLastReceivedFileUri_00100
 * @tc.name: SetLastReceivedFileUri
 * @tc.desc:
 */
HWTEST_F(OppServiceImplTest, Opp_Service_Impl_SetLastReceivedFileUri_00100, TestSize.Level1)
{
    HILOGI("Opp_Service_Impl_SetLastReceivedFileUri_00100 start");
    int ret = OppServiceImplTest_->SetLastReceivedFileUri("removeFile: test");
    EXPECT_EQ(ret, RET_NO_ERROR);
    HILOGI("Opp_Service_Impl_SetLastReceivedFileUri_00100 end");
}

/*
 * @tc.number: Opp_Service_Impl_OnReceiveIncomingConnect_00100
 * @tc.name: OnReceiveIncomingConnect
 * @tc.desc:
 */
HWTEST_F(OppServiceImplTest, Opp_Service_Impl_OnReceiveIncomingConnect_00100, TestSize.Level1)
{
    HILOGI("Opp_Service_Impl_OnReceiveIncomingConnect_00100 start");
    OppServiceImplTest_->SdpFoundCallback(oppRecord_);
    std::vector<IOppTransferFileHolder> fileHolderList;
    for (int i = 0; i <= 2; i++) {
        fileHolderList.push_back(IOppTransferFileHolder("test.jpg", 100, 100));
    }
    const RawAddress device = RawAddress(addr_);
    OppServiceImplTest_->SendFile(device, fileHolderList);
    int ret = OppServiceImplTest_->OnReceiveIncomingConnect(1, 1, addr_, nullptr);
    EXPECT_EQ(ret, ResponseCodes::OBEX_HTTP_UNAVAILABLE);
    HILOGI("Opp_Service_Impl_OnReceiveIncomingConnect_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS
