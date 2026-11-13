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
#include "ble_address_with_type.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
ServiceUtil serviceUtil;

class ServiceUtilTest : public testing::Test {
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
    }
    static void TearDownTestCase()
    {
        HILOGI("start");
        host_ = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number:service_util_test_001
 * @tc.name: AddrTypeFromStack
 * @tc.desc:
 */
HWTEST_F(ServiceUtilTest, service_util_test_001, TestSize.Level1)
{
    HILOGI("service_util_test_001 enter");
    uint8_t type = BLE_ADDR_PUBLIC;
    EXPECT_EQ(serviceUtil.AddrTypeFromStack(type), BLE_ADDR_TYPE_PUBLIC);
    type = BLE_ADDR_RANDOM;
    EXPECT_EQ(serviceUtil.AddrTypeFromStack(type), BLE_ADDR_TYPE_RANDOM);
    type = BLE_ADDR_ANONYMOUS;
    EXPECT_EQ(serviceUtil.AddrTypeFromStack(type), BLE_ADDR_TYPE_ANONYMOUS);
    type = 0x10;
    EXPECT_EQ(serviceUtil.AddrTypeFromStack(type), -1);
    HILOGI("service_util_test_001 end");
}

/*
 * @tc.number:service_util_test_001
 * @tc.name: TransportToBlueroid
 * @tc.desc:
 */
HWTEST_F(ServiceUtilTest, service_util_test_002, TestSize.Level1)
{
    HILOGI("service_util_test_002 enter");
    uint8_t transport = BT_TRANSPORT_BREDR;
    EXPECT_EQ(serviceUtil.TransportToBlueroid(transport), BT_STATUS_FAIL);
    transport = BT_TRANSPORT_BLE;
    EXPECT_EQ(serviceUtil.TransportToBlueroid(transport), BT_STATUS_NOT_READY);
    transport = 0x10;
    EXPECT_EQ(serviceUtil.TransportToBlueroid(transport), BT_STATUS_SUCCESS);
    HILOGI("service_util_test_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS