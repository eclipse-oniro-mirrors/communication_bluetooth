/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "sdp_adapter.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "adapter_manager.h"
#include "btif_interface_mock.h"
#include "sdp_found_event_publish.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 4;
const int32_t SDP_MAP_MSG_TYPE_SMS_GSM = 0x02;
const int32_t SDP_MAP_MSG_TYPE_SMS_CDMA = 0x04;
const int32_t SDP_MAP_MSG_TYPE_MMS = 0x08;
const int32_t MAP_RFCOMM_CHANNEL = 26;
const int32_t MAP_L2CAP_PSM = 0x1029;
const int32_t SDP_MAP_MAS_VERSION_1_2 = 0x0102;
const int32_t SDP_MAP_MAS_FEATURES_1_2 = 0x0000007F;
const int32_t SDP_MAP_MAS_VERSION_1_4 = 0x0104;
const int32_t SDP_MAP_MAS_FEATURES_1_4 = 0x000603FF;
static const std::string OPP_SERVICE_NAME = "OBEX Object Push";
const std::string MNS_UUID = "00001133-0000-1000-8000-00805F9B34FB";
const std::string MAP_MAS_UUID = "00001132-0000-1000-8000-00805f9b34fb";
const std::string PBAP_PSE_UUID = "0000112f-0000-1000-8000-00805f9b34fb";
const std::string SAP_UUID = "0000112d-0000-1000-8000-00805f9b34fb";
const std::string DIP_UUID = "00001200-0000-1000-8000-00805f9b34fb";
const std::string OBEX_OBJECT_PUSH_UUID = "00001105-0000-1000-8000-00805f9b34fb";
const std::string UNKNOWN_UUID = "00001111-0000-1000-8000-00805f9b34fb";
constexpr int32_t SDP_OPP_SERVER_VERSION = 0x0102;
constexpr int32_t OPP_RFCOMM_CHANNEL = 12;
constexpr int32_t OPP_L2CAP_PSM = 0x1023;

static BluetoothHost *host_;
static bt_interface_t *btInterface = nullptr;
class SdpAdapterTest : public testing::Test {
public:
    SdpAdapterTest();
    ~SdpAdapterTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

SdpAdapterTest::SdpAdapterTest()
{
    HILOGI("SdpAdapterTest CONSTRUCTOR");
}

SdpAdapterTest::~SdpAdapterTest()
{
    HILOGI("###### ~SdpAdapterTest ######");
}

void SdpAdapterTest::SetUpTestCase(void)
{
    HILOGI("SdpAdapterTest SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();
    btInterface = (bt_interface_t *)malloc(sizeof(bt_interface_t));
    btInterface->get_profile_interface = bluetoothMock::GetSdpProfileInterfaceMock;

    bool isBleEnable = host_->IsBleEnabled();
    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("SdpAdapterTest isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    AdapterManager::GetInstance()->setBluetoothInterface(btInterface);
    HILOGI("SdpAdapterTest SetUpTestCase end");
}

void SdpAdapterTest::TearDownTestCase(void)
{
    HILOGI("SdpAdapterTest TearDownTestCase start");
    sleep(TIME);
    host_ = nullptr;
    delete(btInterface);
    btInterface = nullptr;
    HILOGI("SdpAdapterTest TearDownTestCase end");
}

void SdpAdapterTest::SetUp()
{
}

void SdpAdapterTest::TearDown()
{
}

/**
 * @tc.number:Sdp_Adapter_Unit_Test_CreateMapMasSdpRecord
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(SdpAdapterTest, Sdp_Adapter_Unit_Test_CreateMapMasSdpRecord, TestSize.Level1)
{
    HILOGI("SdpAdapterTest::CreateMapMasSdpRecord starts");

    int32_t msgTypes = SDP_MAP_MSG_TYPE_SMS_GSM | SDP_MAP_MSG_TYPE_SMS_CDMA | SDP_MAP_MSG_TYPE_MMS;
    MapMasSdpCreateParam param{"SMS/MMS",
        1000,
        MAP_RFCOMM_CHANNEL,
        MAP_L2CAP_PSM,
        SDP_MAP_MAS_VERSION_1_4,
        msgTypes,
        SDP_MAP_MAS_FEATURES_1_4};
    EXPECT_EQ(SdpAdapter::GetInstance().CreateMapMasSdpRecord(param), -1);

    HILOGI("SdpAdapterTest::CreateMapMasSdpRecord ends");
}

/**
 * @tc.number:Sdp_Adapter_Unit_Test_CreateOppSdpRecord
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(SdpAdapterTest, Sdp_Adapter_Unit_Test_CreateOppSdpRecord, TestSize.Level1)
{
    HILOGI("SdpAdapterTest::CreateOppSdpRecord starts");

    char name[] = "SMS/MMS";
    OppSdpCreateParam param{
        name,
        OPP_RFCOMM_CHANNEL,
        OPP_L2CAP_PSM,
        SDP_OPP_SERVER_VERSION};
    EXPECT_EQ(SdpAdapter::GetInstance().CreateOppSdpRecord(param), -1);

    HILOGI("SdpAdapterTest::CreateOppSdpRecord ends");
}

/**
 * @tc.number:Sdp_Adapter_Unit_Test_RemoveSdpRecord
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(SdpAdapterTest, Sdp_Adapter_Unit_Test_RemoveSdpRecord, TestSize.Level1)
{
    HILOGI("SdpAdapterTest::RemoveSdpRecord starts");

    int32_t handle = 100;
    EXPECT_TRUE(SdpAdapter::GetInstance().RemoveSdpRecord(handle));

    HILOGI("SdpAdapterTest::RemoveSdpRecord ends");
}

/**
 * @tc.number:Sdp_Adapter_Unit_Test_StartRemoteSdpSearch
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(SdpAdapterTest, Sdp_Adapter_Unit_Test_StartRemoteSdpSearch, TestSize.Level1)
{
    HILOGI("SdpAdapterTest::StartRemoteSdpSearch starts");

    EXPECT_TRUE(SdpAdapter::GetInstance().StartRemoteSdpSearch("11:22:33:44:55:66", MNS_UUID));

    HILOGI("SdpAdapterTest::StartRemoteSdpSearch ends");
}

/**
 * @tc.number:Sdp_Adapter_Unit_Test_PublishSdpFoundEvent
 * @tc.name: Enable
 * @tc.desc:
 */
HWTEST_F(SdpAdapterTest, Sdp_Adapter_Unit_Test_PublishSdpFoundEvent, TestSize.Level1)
{
    bool ret = SdpFoundEventPublishHelper::PublishSdpFoundEvent(0, "11:22:33:44:55:66",
        UNKNOWN_UUID, false, nullptr);
    EXPECT_FALSE(ret);
    bluetooth_sdp_record record{};
    bool ret1 = SdpFoundEventPublishHelper::PublishSdpFoundEvent(0, "11:22:33:44:55:66",
        MNS_UUID, false, &record);
    bool ret2 = SdpFoundEventPublishHelper::PublishSdpFoundEvent(0, "11:22:33:44:55:66",
        MAP_MAS_UUID, false, &record);
    bool ret3 = SdpFoundEventPublishHelper::PublishSdpFoundEvent(0, "11:22:33:44:55:66",
        PBAP_PSE_UUID, false, &record);
    bool ret4 = SdpFoundEventPublishHelper::PublishSdpFoundEvent(0, "11:22:33:44:55:66",
        SAP_UUID, false, &record);
    bool ret5 = SdpFoundEventPublishHelper::PublishSdpFoundEvent(0, "11:22:33:44:55:66",
        DIP_UUID, false, &record);
    bool ret6 = SdpFoundEventPublishHelper::PublishSdpFoundEvent(0, "11:22:33:44:55:66",
        OBEX_OBJECT_PUSH_UUID, false, &record);
    bool ret7 = SdpFoundEventPublishHelper::PublishSdpFoundEvent(0, "11:22:33:44:55:66",
        UNKNOWN_UUID, false, &record);
    EXPECT_TRUE(ret1 && ret2 && ret3 && ret4 && ret5 && ret6 && ret7);
    HILOGI("SdpAdapterTest::PublishSdpFoundEvent ends");
}
}  // namespace bluetooth
}  // namespace OHOS