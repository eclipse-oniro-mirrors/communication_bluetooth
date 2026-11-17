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

#include "bluetooth_hid_device_sdp_parcel.h"
#include "bluetooth_hid_device_qos_parcel.h"
#include "hid_device_data.h"
#include "message_parcel.h"
#include "bluetooth_log.h"
#include "bluetooth_host.h"


using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
class BlueToothHidDeviceParcelTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
    }
    void SetUp()
    {}

    void TearDown()
    {}

public:
};

/**
 * @tc.name: BlueToothHidDeviceSdpParcelTest_00100
 * @tc.desc: BlueToothHidDeviceSdpParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothHidDeviceParcelTest, BlueToothHidDeviceSdpParcelTest_00100, TestSize.Level1)
{
    HILOGI("BlueToothHidDeviceSdpParcelTest_00100 start");
    MessageParcel data;
    BluetoothHidDeviceSdp sdpSetting;
    sdpSetting.name_ = "test";
    sdpSetting.description_ = "test";
    sdpSetting.provider_ = "test";
    sdpSetting.subclass_ = static_cast<int>(Subclass::SUBCLASS_MOUSE);
    sdpSetting.sdpSettings_ = {0x01, 0x02};
    bool result = sdpSetting.WriteToParcel(data);
    EXPECT_TRUE(result);
    BluetoothHidDeviceSdp sdp;
    sdp = sdpSetting;
    result = sdp.ReadFromParcel(data);
    EXPECT_TRUE(result);
    HILOGI("BlueToothHidDeviceSdpParcelTest_00100 end");
}

/**
 * @tc.name: BlueToothHidDeviceQosParcelTest_00100
 * @tc.desc: BlueToothHidDeviceQosParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothHidDeviceParcelTest, BlueToothHidDeviceQosParcelTest_00100, TestSize.Level1)
{
    HILOGI("BlueToothHidDeviceQosParcelTest_00100 start");
    MessageParcel data;
    BluetoothHidDeviceQos Qos;
    Qos.serviceType_ = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    Qos.tokenRate_ = DEFAULT_TOKEN_RATE;
    Qos.tokenBucketSize_ = DEFAULT_TOKEN_BUCKET_SIZE;
    Qos.peakBandwidth_ = DEFAULT_PEAK_BANDWIDTH;
    Qos.latency_ = DEFAULT_LATENCY;
    Qos.delayVariation_ = DEFAULT_DELAY_VARIATION;
    bool result = Qos.WriteToParcel(data);
    EXPECT_TRUE(result);
    BluetoothHidDeviceQos qosSetting;
    qosSetting = Qos;
    result = qosSetting.ReadFromParcel(data);
    EXPECT_TRUE(result);
    HILOGI("BlueToothHidDeviceQosParcelTest_00100 end");
}


/**
 * @tc.name: BlueToothHidDeviceSdpMarshallingTest_00100
 * @tc.desc: BlueToothHidDeviceSdpMarshallingTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothHidDeviceParcelTest, BlueToothHidDeviceSdpMarshallingTest_00100, TestSize.Level1)
{
    HILOGI("BlueToothHidDeviceSdpMarshallingTest_00100 start");
    MessageParcel data;
    bluetooth::SdpSetting sdpSetting;
    sdpSetting.name_ = "test";
    sdpSetting.description_ = "test";
    sdpSetting.provider_ = "test";
    sdpSetting.subclass_ = static_cast<int>(Subclass::SUBCLASS_MOUSE);
    sdpSetting.sdpSettings_ = {0x01, 0x02};

    BluetoothHidDeviceSdp bluetoothSdp;
    bluetoothSdp = sdpSetting;
    bool ret = bluetoothSdp.Marshalling(data);
    EXPECT_TRUE(ret);
    auto sdpPtr = BluetoothHidDeviceSdp::Unmarshalling(data);
    EXPECT_TRUE(sdpPtr != nullptr);
    HILOGI("BlueToothHidDeviceSdpMarshallingTest_00100 end");
}

/**
 * @tc.name: BlueToothHidDeviceQosMarshallingTest_00100
 * @tc.desc: BlueToothHidDeviceQosMarshallingTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueToothHidDeviceParcelTest, BlueToothHidDeviceQosMarshallingTest_00100, TestSize.Level1)
{
    HILOGI("BlueToothHidDeviceQosMarshallingTest_00100 start");
    MessageParcel data;
    bluetooth::QosSetting Qos;
    Qos.serviceType_ = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    Qos.tokenRate_ = DEFAULT_TOKEN_RATE;
    Qos.tokenBucketSize_ = DEFAULT_TOKEN_BUCKET_SIZE;
    Qos.peakBandwidth_ = DEFAULT_PEAK_BANDWIDTH;
    Qos.latency_ = DEFAULT_LATENCY;
    Qos.delayVariation_ = DEFAULT_DELAY_VARIATION;

    BluetoothHidDeviceQos bluetoothQos;
    bluetoothQos = Qos;
    bool ret = bluetoothQos.Marshalling(data);
    EXPECT_TRUE(ret);
    auto qosPtr = BluetoothHidDeviceQos::Unmarshalling(data);
    EXPECT_TRUE(qosPtr != nullptr);
    HILOGI("BlueToothHidDeviceQosMarshallingTest_00100 end");
}

}  // namespace bluetooth
}  // namespace OHOS