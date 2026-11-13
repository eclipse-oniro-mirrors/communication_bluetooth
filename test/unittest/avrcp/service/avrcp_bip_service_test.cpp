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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "avrcp_bip_service.h"
#include "bluetooth_host.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
static BluetoothHost *host_;

class BipServiceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

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
    {}
    void SetUp() {}
    void TearDown() {}

protected:
    static std::string testAddress_;
};

/*
 * @tc.number: BipService_RemoveStateMachine_001
 * @tc.name: RemoveStateMachine
 * @tc.desc:
 */
HWTEST_F(BipServiceTest, BipService_RemoveStateMachine_001, TestSize.Level1)
{
    HILOGI("BipService_RemoveStateMachine_001 start");
    auto bipService = std::make_shared<BipService>();
    std::string device("00:00:00:00:00:00");

    bipService->RemoveStateMachine(device);
    std::shared_ptr<BipStateMachine> stateMachine = nullptr;
    EXPECT_FALSE(bipService->stateMachines_.Find(device, stateMachine));
    HILOGI("BipService_RemoveStateMachine_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS