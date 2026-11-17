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
#include "service_util.h"
#include "power_device.h"
#include "adapter_device_info.h"
#include "class_creator.h"
#include "adapter_manager.h"
#include "ble_adapter.h"
#include "bluetooth_host.h"
#include "ble_address_with_type.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
AdapterDeviceInfo adapterDeviceInfo;

class AdapterDeviceInfoTest : public testing::Test {
public:
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
    {
        host_ = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number:adapter_device_info_test_001
 * @tc.name: Load
 * @tc.desc:
 */
HWTEST_F(AdapterDeviceInfoTest, adapter_device_info_test_001, TestSize.Level1)
{
    HILOGI("adapter_device_info_test_001 enter");
    EXPECT_FALSE(adapterDeviceInfo.Load());
    HILOGI("adapter_device_info_test_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS