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

#include "pairable_adv_controller.h"

#include <gtest/gtest.h>
#include <memory>

#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

// 单测环境无 BleAdapter，广播控制器为 no-op：仅验证各入口不崩溃。
// handle 生命周期（首次申请/Enable 复用/析构释放）需引入 BleAdapter 桩后补充。
class PairableAdvControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("PairableAdvControllerTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("PairableAdvControllerTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("PairableAdvControllerTest::SetUp start");
        adv_ = std::make_unique<PairableAdvController>();
    }
    void TearDown() override
    {
        HILOGI("PairableAdvControllerTest::TearDown start");
        adv_ = nullptr;
    }

protected:
    std::unique_ptr<PairableAdvController> adv_;
};

/**
 * @tc.name: Pairable_adv_controller_StopAdv_NoCrash_001
 * @tc.desc: 单测环境无 BleAdapter，StopAdv no-op 不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(PairableAdvControllerTest, Pairable_adv_controller_StopAdv_NoCrash_001, TestSize.Level0)
{
    HILOGI("Pairable_adv_controller_StopAdv_NoCrash_001 enter");
    adv_->StopAdv();
    SUCCEED();
    HILOGI("Pairable_adv_controller_StopAdv_NoCrash_001 end");
}

/**
 * @tc.name: Pairable_adv_controller_StartAdv_NoCrash_002
 * @tc.desc: 单测环境无 BleAdapter，StartAdv no-op 不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(PairableAdvControllerTest, Pairable_adv_controller_StartAdv_NoCrash_002, TestSize.Level0)
{
    HILOGI("Pairable_adv_controller_StartAdv_NoCrash_002 enter");
    adv_->StartAdv();
    SUCCEED();
    HILOGI("Pairable_adv_controller_StartAdv_NoCrash_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS
