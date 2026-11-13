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

#include "reconnect_predicates.h"

#include <gtest/gtest.h>
#include <memory>

#include "car_connection_guard.h"
#include "car_policy_manager.h"
#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

const std::string DEV_A = "00:11:22:33:44:55";

class ReconnectPredicatesTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("ReconnectPredicatesTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("ReconnectPredicatesTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("ReconnectPredicatesTest::SetUp start");
        guard_ = std::make_unique<CarConnectionGuard>();
        incomingPred_ = std::make_unique<IncomingConnPredicate>();
        carplayPred_ = std::make_unique<CarplayMutexPredicate>(*guard_);
        iviStandbyPred_ = std::make_unique<IviStandbyPredicate>(CarPolicyManager::GetInstance());
    }
    void TearDown() override
    {
        HILOGI("ReconnectPredicatesTest::TearDown start");
        iviStandbyPred_ = nullptr;
        carplayPred_ = nullptr;
        incomingPred_ = nullptr;
        guard_ = nullptr;
    }

protected:
    std::unique_ptr<CarConnectionGuard> guard_;
    std::unique_ptr<IncomingConnPredicate> incomingPred_;
    std::unique_ptr<CarplayMutexPredicate> carplayPred_;
    std::unique_ptr<IviStandbyPredicate> iviStandbyPred_;
};

/**
 * @tc.name: Reconnect_predicates_IncomingConn_NoAdapter_001
 * @tc.desc: IncomingConnPredicate 单测环境无 Adapter 返回 false
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_IncomingConn_NoAdapter_001, TestSize.Level1)
{
    HILOGI("Reconnect_predicates_IncomingConn_NoAdapter_001 enter");
    EXPECT_FALSE(incomingPred_->ShouldTerminate(DEV_A, 0));
    HILOGI("Reconnect_predicates_IncomingConn_NoAdapter_001 end");
}

/**
 * @tc.name: Reconnect_predicates_IncomingConn_GetName_002
 * @tc.desc: IncomingConnPredicate 名称
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_IncomingConn_GetName_002, TestSize.Level0)
{
    HILOGI("Reconnect_predicates_IncomingConn_GetName_002 enter");
    EXPECT_EQ(incomingPred_->GetName(), "IncomingConnPredicate");
    HILOGI("Reconnect_predicates_IncomingConn_GetName_002 end");
}

/**
 * @tc.name: Reconnect_predicates_IncomingConn_OnTerminate_003
 * @tc.desc: IncomingConnPredicate OnTerminate 默认 no-op
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_IncomingConn_OnTerminate_003, TestSize.Level0)
{
    HILOGI("Reconnect_predicates_IncomingConn_OnTerminate_003 enter");
    incomingPred_->OnTerminate();
    SUCCEED();
    HILOGI("Reconnect_predicates_IncomingConn_OnTerminate_003 end");
}

/**
 * @tc.name: Reconnect_predicates_CarplayMutex_DefaultFalse_004
 * @tc.desc: CarplayMutexPredicate 默认返回 false（IsCarPlayMutex 未实现）
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_CarplayMutex_DefaultFalse_004, TestSize.Level1)
{
    HILOGI("Reconnect_predicates_CarplayMutex_DefaultFalse_004 enter");
    EXPECT_FALSE(carplayPred_->ShouldTerminate(DEV_A, 0));
    HILOGI("Reconnect_predicates_CarplayMutex_DefaultFalse_004 end");
}

/**
 * @tc.name: Reconnect_predicates_CarplayMutex_GetName_005
 * @tc.desc: CarplayMutexPredicate 名称
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_CarplayMutex_GetName_005, TestSize.Level0)
{
    HILOGI("Reconnect_predicates_CarplayMutex_GetName_005 enter");
    EXPECT_EQ(carplayPred_->GetName(), "CarplayMutexPredicate");
    HILOGI("Reconnect_predicates_CarplayMutex_GetName_005 end");
}

/**
 * @tc.name: Reconnect_predicates_VirtualDestructor_NoLeak_006
 * @tc.desc: IReconnectPredicate 虚析构经基类指针释放不泄漏
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_VirtualDestructor_NoLeak_006, TestSize.Level0)
{
    HILOGI("Reconnect_predicates_VirtualDestructor_NoLeak_006 enter");
    std::unique_ptr<IReconnectPredicate> pred = std::make_unique<IncomingConnPredicate>();
    pred.reset();
    SUCCEED();
    HILOGI("Reconnect_predicates_VirtualDestructor_NoLeak_006 end");
}

/**
 * @tc.name: Reconnect_predicates_IviStandby_DefaultFalse_007
 * @tc.desc: IviStandbyPredicate 单测环境（单例未 Init，非 STANDBY）不终止回连
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_IviStandby_DefaultFalse_007, TestSize.Level1)
{
    HILOGI("Reconnect_predicates_IviStandby_DefaultFalse_007 enter");
    EXPECT_FALSE(iviStandbyPred_->ShouldTerminate(DEV_A, 0));
    EXPECT_FALSE(iviStandbyPred_->ShouldTerminate(DEV_A, 3));
    HILOGI("Reconnect_predicates_IviStandby_DefaultFalse_007 end");
}

/**
 * @tc.name: Reconnect_predicates_IviStandby_GetName_008
 * @tc.desc: IviStandbyPredicate 名称
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_IviStandby_GetName_008, TestSize.Level0)
{
    HILOGI("Reconnect_predicates_IviStandby_GetName_008 enter");
    EXPECT_EQ(iviStandbyPred_->GetName(), "IviStandbyPredicate");
    HILOGI("Reconnect_predicates_IviStandby_GetName_008 end");
}

/**
 * @tc.name: Reconnect_predicates_IviStandby_OnTerminate_NoOp_009
 * @tc.desc: IviStandbyPredicate OnTerminate 默认 no-op 不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(ReconnectPredicatesTest, Reconnect_predicates_IviStandby_OnTerminate_NoOp_009, TestSize.Level0)
{
    HILOGI("Reconnect_predicates_IviStandby_OnTerminate_NoOp_009 enter");
    iviStandbyPred_->OnTerminate();
    SUCCEED();
    HILOGI("Reconnect_predicates_IviStandby_OnTerminate_NoOp_009 end");
}
}  // namespace bluetooth
}  // namespace OHOS
