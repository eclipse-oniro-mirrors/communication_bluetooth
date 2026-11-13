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
#include "message_parcel.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_hfp_hf_call.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"
#include "bluetooth_host.h"


using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
class BluetoothHfpParcelTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

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
        HILOGI("TearDownTestCase start");
        host_ = nullptr;
        HILOGI("TearDownTestCase end");
    }
    void SetUp()
    {}

    void TearDown()
    {}

public:
};
/**
 * @tc.name: BluetoothHfpParcelTest_00100
 * @tc.desc: BluetoothHfpParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BluetoothHfpParcelTest, BluetoothHfpParcelTest_00100, TestSize.Level1)
{
    HILOGI("BluetoothHfpParcelTest_00100 start");
    MessageParcel data;
    BluetoothHfpHfCall bluetoothHfpHfCall;
    data.WriteParcelable(&bluetoothHfpHfCall);
    EXPECT_TRUE(bluetoothHfpHfCall.Marshalling(data));
    EXPECT_FALSE(bluetoothHfpHfCall.Unmarshalling(data));
    EXPECT_TRUE(bluetoothHfpHfCall.WriteToParcel(data));
    EXPECT_FALSE(bluetoothHfpHfCall.ReadFromParcel(data));
    std::shared_ptr<BluetoothHfpHfCall> result(data.ReadParcelable<BluetoothHfpHfCall>());
    HILOGI("BluetoothHfpParcelTest_00100 end");
}

}  // namespace bluetooth
}  // namespace OHOS