/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
 
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_profile_manager.h"

#include "event_handler.h"  // libeventhandler
#include "event_runner.h"  // libeventhandler
#include <future>
#include <memory>
#include <thread>
using OHOS::AppExecFwk::EventRunner;
using OHOS::AppExecFwk::EventHandler;
 
#include "ffrt_inner.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
 
class BluetoothProfileManagerTest : public testing::Test {
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
 * @tc.number: BluetoothProfileManagerTest_UnitTest_GetValidId_001
 * @tc.name: GetValidId
 * @tc.desc: GetValidId
*/
HWTEST_F(BluetoothProfileManagerTest, BluetoothProfileManagerTest_UnitTest_GetValidId_001, TestSize.Level1)
{
    HILOGI("BluetoothProfileManagerTest_UnitTest_GetValidId_001 start");
    BluetoothProfileManager::GetInstance().registerValidId_ = INT32_MAX - 1;
    int32_t ret = BluetoothProfileManager::GetInstance().GetValidId();
    EXPECT_EQ(ret, INT32_MAX);
    HILOGI("BluetoothProfileManagerTest_UnitTest_GetValidId_001 end");
}

/*
 * @tc.number: BluetoothProfileManagerTest_UnitTest_GetHostRemote_001
 * @tc.name: GetHostRemote
 * @tc.desc: GetHostRemote
*/
HWTEST_F(BluetoothProfileManagerTest, BluetoothProfileManagerTest_UnitTest_GetHostRemote_001, TestSize.Level1)
{
    HILOGI("BluetoothProfileManagerTest_UnitTest_GetHostRemote_001 start");
    BluetoothProfileManager::GetInstance().isNeedCheckBluetoothServiceOn_ = false;
    sptr<IRemoteObject> object = BluetoothProfileManager::GetInstance().GetHostRemote();
    EXPECT_TRUE(object != nullptr);
    BluetoothProfileManager::GetInstance().isNeedCheckBluetoothServiceOn_ = true;
    object = BluetoothProfileManager::GetInstance().GetHostRemote();
    EXPECT_TRUE(object != nullptr);
    HILOGI("BluetoothProfileManagerTest_UnitTest_GetHostRemote_001 end");
}

/*
 * @tc.number: BluetoothProfileManagerTest_UnitTest_CheckFreeze_001
 * @tc.name: CheckFreeze
 * @tc.desc: CheckFreeze
*/
HWTEST_F(BluetoothProfileManagerTest, BluetoothProfileManagerTest_UnitTest_CheckFreeze_001, TestSize.Level1)
{
    HILOGI("BluetoothProfileManagerTest_UnitTest_CheckFreeze_001 start");
    EXPECT_TRUE(BluetoothProfileManager::GetInstance().IsBluetoothServiceOn());
    ffrt::queue queue("test");
    int flag = 0;
    auto testFuc = [&flag, &queue]() {
        BluetoothProfileManager::GetInstance().GetHostRemote();
        flag++;
    };
 
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create("test");
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>(eventRunner);
    std::shared_ptr<Bluetooth::BluetoothProfileManager::BluetoothSystemAbility>
        statusChange = std::make_shared<Bluetooth::BluetoothProfileManager::BluetoothSystemAbility>();
    auto testFuc1 = [&statusChange, &flag]() {
        statusChange->OnAddSystemAbility(1130, "");
        flag++;
    };
 
    for (int i = 0; i < 10000; i++) {
        queue.submit(testFuc);
        eventHandler->PostTask(testFuc1);
    }
    HILOGI("BluetoothProfileManagerTest_UnitTest_CheckFreeze_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS