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
#include "compat.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
Compat compat;

class CompatTest : public testing::Test {
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
        host_ = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: CompatTest_UnitTest_CompatCheck_001
 * @tc.name: CompatCheck
 * @tc.desc: could not found the type, return false
*/
HWTEST_F(CompatTest, CompatTest_UnitTest_CompatCheck_001, TestSize.Level1)
{
    HILOGI("CompatTest_UnitTest_CompatCheck_001 start");
    std::string value = "compat_test_001";
    bool ret = compat.CompatCheck(static_cast<CompatType>(0xFF), value);
    EXPECT_TRUE(ret == false);
    HILOGI("CompatTest_UnitTest_CompatCheck_001 end");
}

/*
 * @tc.number: CompatTest_UnitTest_CompatCheck_002
 * @tc.name: CompatCheck
 * @tc.desc: could found the type and the value, return true
*/
HWTEST_F(CompatTest, CompatTest_UnitTest_CompatCheck_002, TestSize.Level1)
{
    HILOGI("CompatTest_UnitTest_CompatCheck_002 start");
    std::string value = "BMW";
    bool ret = compat.CompatCheck(CompatType::COMPAT_REJECT_AUTO_PAIRING, value);
    EXPECT_TRUE(ret == true);
    HILOGI("CompatTest_UnitTest_CompatCheck_002 end");
}

/*
 * @tc.number: CompatTest_UnitTest_CompatCheck_003
 * @tc.name: CompatCheck
 * @tc.desc: could found the type, not found the value, return false
*/
HWTEST_F(CompatTest, CompatTest_UnitTest_CompatCheck_003, TestSize.Level1)
{
    HILOGI("CompatTest_UnitTest_CompatCheck_003 start");
    std::string value = "VENDOR";
    bool ret = compat.CompatCheck(CompatType::COMPAT_REJECT_AUTO_PAIRING, value);
    EXPECT_TRUE(ret == false);
    HILOGI("CompatTest_UnitTest_CompatCheck_003 end");
}
}  // namespace bluetooth
}  // namespace OHOS