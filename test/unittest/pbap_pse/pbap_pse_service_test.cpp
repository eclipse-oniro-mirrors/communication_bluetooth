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
#include "pbap_pse_service.h"
#include "pbap_pse_message.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"
#include "class_creator.h"
#include "classic_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
const int TIME = 4;
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
void *PbapPseServiceCreate(void)
{
    return new PbapPseService();
}

class PbapPseServiceTest : public testing::Test {
public:
    PbapPseServiceTest();
    ~PbapPseServiceTest();

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
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }

private:
    std::string addr_ = "11:22:33:44:55:66";
    std::unique_ptr<PbapPseService> pbapPseServiceTest_ = nullptr;
};

PbapPseServiceTest::PbapPseServiceTest()
{
    pbapPseServiceTest_ = std::make_unique<PbapPseService>();
}

PbapPseServiceTest::~PbapPseServiceTest()
{
    sleep(1);
    pbapPseServiceTest_ = nullptr;
}

/*
 * @tc.number: PbapPse_Service_SetConnectionStrategy_00100
 * @tc.name: SetConnectionStrategy
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_SetConnectionStrategy_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_SetConnectionStrategy_00100 start");
    EXPECT_EQ(pbapPseServiceTest_->SetConnectionStrategy(RawAddress(addr_), 0), BT_ERR_INTERNAL_ERROR);
    HILOGI("PbapPse_Service_SetConnectionStrategy_00100 end");
}

/*
 * @tc.number: PbapPse_Service_ShareType_00100
 * @tc.name: ShareType
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_ShareType_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_ShareType_00100 start");
    EXPECT_EQ(pbapPseServiceTest_->SetShareType(RawAddress(addr_), 0), BT_ERR_INTERNAL_ERROR);

    int32_t shareType = 0;
    int32_t ret = pbapPseServiceTest_->GetShareType(RawAddress(addr_), shareType);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("PbapPse_Service_ShareType_00100 end");
}

/*
 * @tc.number: PbapPse_Service_PhoneBookAccessAuthorization_00100
 * @tc.name: PhoneBookAccessAuthorization
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_PhoneBookAccessAuthorization_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_PhoneBookAccessAuthorization_00100 start");
    EXPECT_EQ(pbapPseServiceTest_->SetPhoneBookAccessAuthorization(RawAddress(addr_), 0), BT_ERR_INTERNAL_ERROR);

    int32_t accessAuthorization = 0;
    int32_t ret = pbapPseServiceTest_->GetPhoneBookAccessAuthorization(RawAddress(addr_), accessAuthorization);
    EXPECT_EQ(ret, BT_ERR_INTERNAL_ERROR);
    HILOGI("PbapPse_Service_PhoneBookAccessAuthorization_00100 end");
}

/*
 * @tc.number: PbapPse_Service_StopSocketListener_00100
 * @tc.name: StopSocketListener
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_StopSocketListener_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_StopSocketListener_00100 start");
    pbapPseServiceTest_->obexServerSocket_ = nullptr;
    pbapPseServiceTest_->StartSocketListener();
    pbapPseServiceTest_->StopSocketListener();

    std::shared_ptr<IObexSocketObserver> socketObserver = std::make_shared<PbapPseSocketObserver>();
    pbapPseServiceTest_->obexServerSocket_ = std::make_shared<ObexServerSocket>(PBAP_PSE_SERVICE_NAME,
        PBAP_RFCOMM_CHANNEL, -1, EMPTY_UUID, socketObserver);
    pbapPseServiceTest_->StartSocketListener();
    pbapPseServiceTest_->StopSocketListener();
    EXPECT_EQ(pbapPseServiceTest_->obexServerSocket_, nullptr);
    pbapPseServiceTest_->NotifyStateChanged(RawAddress(addr_), PBAP_PSE_STATE_CONNECTED, 0);
    pbapPseServiceTest_->NotifyStateChanged(RawAddress(addr_), 3, 0);
    HILOGI("PbapPse_Service_StopSocketListener_00100 end");
}

/*
 * @tc.number: PbapPse_Service_CreateSdpRecord_00100
 * @tc.name: CreateSdpRecord
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_CreateSdpRecord_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_CreateSdpRecord_00100 start");
    pbapPseServiceTest_->mSdpHandle = 0;
    pbapPseServiceTest_->CreateSdpRecord();
    pbapPseServiceTest_->CleanSdpRecord();

    pbapPseServiceTest_->mSdpHandle = -1;
    pbapPseServiceTest_->CreateSdpRecord();
    pbapPseServiceTest_->CleanSdpRecord();
    EXPECT_EQ(pbapPseServiceTest_->mSdpHandle, -1);
    HILOGI("PbapPse_Service_CreateSdpRecord_00100 end");
}

/*
 * @tc.number: PbapPse_Service_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_Disconnect_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_Disconnect_00100 start");
    const RawAddress device = RawAddress(addr_);
    EXPECT_EQ(pbapPseServiceTest_->Disconnect(device), BT_ERR_INTERNAL_ERROR);
    HILOGI("PbapPse_Service_Disconnect_00100 end");
}

/*
 * @tc.number: PbapPse_Service_GetConnectState_00100
 * @tc.name: GetConnectState
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_GetConnectState_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_GetConnectState_00100 start");
    EXPECT_EQ(pbapPseServiceTest_->GetConnectState(), PROFILE_STATE_DISCONNECTED);
    HILOGI("PbapPse_Service_GetConnectState_00100 end");
}

/*
 * @tc.number: PbapPse_Service_GetDeviceState_00101
 * @tc.name: GetDeviceState
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_GetDeviceState_00101, TestSize.Level1)
{
    HILOGI("PbapPse_Service_GetDeviceState_00101 start");
    const RawAddress device = RawAddress(addr_);
    EXPECT_EQ(pbapPseServiceTest_->GetDeviceState(device), static_cast<int>(BTConnectState::DISCONNECTED));
    HILOGI("PbapPse_Service_GetDeviceState_00101 end");
}

/*
 * @tc.number: PbapPse_Service_GetDevicesByStates_00100
 * @tc.name: GetDevicesByStates
 * @tc.desc:
 */
HWTEST_F(PbapPseServiceTest, PbapPse_Service_GetDevicesByStates_00100, TestSize.Level1)
{
    HILOGI("PbapPse_Service_GetDevicesByStates_00100 start");
    std::vector<int> states = {0};
    std::vector<RawAddress> devices = pbapPseServiceTest_->GetDevicesByStates(states);
    EXPECT_EQ(devices.size(), 0);
    HILOGI("PbapPse_Service_GetDevicesByStates_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS