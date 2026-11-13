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
#include "adapter_device_config.h"
#include "hid_device_service_impl.h"
#include "bluetooth_host.h"
#include "bluetooth_raw_address.h"
#include "base_def.h"
#include "bluetooth_errorcode.h"
#include "class_creator.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;
using namespace bluetooth;

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
const int TIME = 4;

class HidDeviceServiceImplTest : public testing::Test {
public:
    HidDeviceServiceImplTest();
    ~HidDeviceServiceImplTest();

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
    }
    static void TearDownTestCase()
    {}

    void SetUp()
    {
    }

    void TearDown()
    {}

private:
    HidDeviceServiceImpl *hidDeviceServiceImpl = nullptr;
};

HidDeviceServiceImplTest::HidDeviceServiceImplTest()
{
    hidDeviceServiceImpl = new HidDeviceServiceImpl();
}

HidDeviceServiceImplTest::~HidDeviceServiceImplTest()
{
    delete hidDeviceServiceImpl;
}

/*
 * @tc.number: HId_Device_Service_Impl_SetAppState_00100
 * @tc.name: SetAppState
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_SetAppState_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_SetAppState_00100 start");
    hidDeviceServiceImpl->SetAppState(true);
    EXPECT_EQ(hidDeviceServiceImpl->GetAppState(), true);
    hidDeviceServiceImpl->SetAppState(false);
    EXPECT_EQ(hidDeviceServiceImpl->GetAppState(), false);
    HILOGI("HId_Device_Service_Impl_SetAppState_00100 end");
}

/*
 * @tc.number: HId_Device_Service_Impl_IsAppForeground_00100
 * @tc.name: IsAppForeground
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_IsAppForeground_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_IsAppForeground_00100 start");
    std::string bundleName = "invalidBundleName";
    EXPECT_EQ(hidDeviceServiceImpl->IsAppForeground(bundleName), false);
    HILOGI("HId_Device_Service_Impl_IsAppForeground_00100 end");
}

/*
 * @tc.number: HId_Device_Service_Impl_Connect_00100
 * @tc.name: Connect
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_Connect_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_Connect_00100 start");
    RawAddress device("11:22:33:44:55:66");
    EXPECT_EQ(hidDeviceServiceImpl->Connect(device), BT_ERR_INTERNAL_ERROR);
    HILOGI("HId_Device_Service_Impl_Connect_00100 end");
}

/*
 * @tc.number: HId_Device_Service_Impl_Disconnect_00100
 * @tc.name: Disconnect
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_Disconnect_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_Disconnect_00100 start");
    RawAddress device("11:22:33:44:55:66");
    EXPECT_EQ(hidDeviceServiceImpl->Disconnect(device), BT_ERR_INTERNAL_ERROR);
    HILOGI("HId_Device_Service_Impl_Disconnect_00100 end");
}

/*
 * @tc.number: HId_Device_Service_Impl_RegisterHidDevice_00100
 * @tc.name: RegisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_RegisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_RegisterHidDevice_00100 start");
    RawAddress device("11:22:33:44:55:66");
    BluetoothHidDeviceSdp sdpSetting;
    sdpSetting.name_ = "test";
    sdpSetting.description_ = "test";
    sdpSetting.provider_ = "test";
    sdpSetting.subclass_ = static_cast<int>(Subclass::SUBCLASS_MOUSE);
    sdpSetting.sdpSettings_ = {};
    BluetoothHidDeviceQos inQos;
    inQos.serviceType_ = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    inQos.tokenRate_ = DEFAULT_TOKEN_RATE;
    inQos.tokenBucketSize_ = DEFAULT_TOKEN_BUCKET_SIZE;
    inQos.peakBandwidth_ = DEFAULT_PEAK_BANDWIDTH;
    inQos.latency_ = DEFAULT_LATENCY;
    inQos.delayVariation_ = DEFAULT_DELAY_VARIATION;
    BluetoothHidDeviceQos outQos;
    outQos.serviceType_ = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    outQos.tokenRate_ = DEFAULT_TOKEN_RATE;
    outQos.tokenBucketSize_ = DEFAULT_TOKEN_BUCKET_SIZE;
    outQos.peakBandwidth_ = DEFAULT_PEAK_BANDWIDTH;
    outQos.latency_ = DEFAULT_LATENCY;
    outQos.delayVariation_ = DEFAULT_DELAY_VARIATION;
    std::string bundleName = "invalid bundle name";

    EXPECT_EQ(hidDeviceServiceImpl->RegisterHidDevice(sdpSetting, inQos, outQos, bundleName),
        BT_ERR_HID_APPLICATION_NOT_IN_FOREGROUND);
    HILOGI("HId_Device_Service_Impl_RegisterHidDevice_00100 end");
}

/*
 * @tc.number: HId_Device_Service_Impl_UnregisterHidDevice_00100
 * @tc.name: UnregisterHidDevice
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_UnregisterHidDevice_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_UnregisterHidDevice_00100 start");
    EXPECT_EQ(hidDeviceServiceImpl->UnregisterHidDevice(), BT_ERR_INTERNAL_ERROR);
    HILOGI("HId_Device_Service_Impl_UnregisterHidDevice_00100 end");
}

/*
 * @tc.number: HId_Device_Service_Impl_SendReport_00100
 * @tc.name: SendReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_SendReport_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_SendReport_00100 start");
    int id = 0;
    std::vector<uint8_t> data = {};
    EXPECT_EQ(hidDeviceServiceImpl->SendReport(id, data), BT_ERR_INTERNAL_ERROR);
    HILOGI("HId_Device_Service_Impl_SendReport_00100 end");
}

/*
 * @tc.number: HId_Device_Service_Impl_ReplyReport_00100
 * @tc.name: ReplyReport
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_ReplyReport_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_ReplyReport_00100 start");
    ReportType type = REPORT_TYPE_INPUT;
    int id = 0;
    std::vector<uint8_t> data = {};
    EXPECT_EQ(hidDeviceServiceImpl->ReplyReport(type, id, data), BT_ERR_INTERNAL_ERROR);
    HILOGI("HId_Device_Service_Impl_ReplyReport_00100 end");
}

/*
 * @tc.number: HId_Device_Service_Impl_ReportError_00100
 * @tc.name: ReportError
 * @tc.desc:
 */
HWTEST_F(HidDeviceServiceImplTest, HId_Device_Service_Impl_ReportError_00100, TestSize.Level1)
{
    HILOGI("HId_Device_Service_Impl_ReportError_00100 start");
    ErrorReason type = RSP_SUCCESS;
    EXPECT_EQ(hidDeviceServiceImpl->ReportError(type), BT_ERR_INTERNAL_ERROR);
    HILOGI("HId_Device_Service_Impl_ReportError_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS