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

#include "car_connection_guard.h"

#include <gtest/gtest.h>
#include <memory>

#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

const std::string DEV_A = "00:11:22:33:44:55";
const std::string DEV_B = "AA:BB:CC:DD:EE:FF";

class CarConnectionGuardTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("CarConnectionGuardTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("CarConnectionGuardTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("CarConnectionGuardTest::SetUp start");
        guard_ = std::make_unique<CarConnectionGuard>();
    }
    void TearDown() override
    {
        HILOGI("CarConnectionGuardTest::TearDown start");
        guard_ = nullptr;
    }

protected:
    std::unique_ptr<CarConnectionGuard> guard_;
};

/**
 * @tc.name: Car_connection_guard_IsAllowConnA2dp_DefaultUnknown_001
 * @tc.desc: 默认 FA/BtMusic UNKNOWN 时 fail-open 允许 A2DP
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_IsAllowConnA2dp_DefaultUnknown_001, TestSize.Level1)
{
    HILOGI("Car_connection_guard_IsAllowConnA2dp_DefaultUnknown_001 enter");
    EXPECT_TRUE(guard_->IsAllowConnA2dp());
    HILOGI("Car_connection_guard_IsAllowConnA2dp_DefaultUnknown_001 end");
}

/**
 * @tc.name: Car_connection_guard_IsAllowConnA2dp_FaBtOn_002
 * @tc.desc: FA=BT_ON 时允许 A2DP
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_IsAllowConnA2dp_FaBtOn_002, TestSize.Level1)
{
    HILOGI("Car_connection_guard_IsAllowConnA2dp_FaBtOn_002 enter");
    guard_->SetFaStatus(CarConnectionGuard::FA_STATUS_BT_ON);
    EXPECT_TRUE(guard_->IsAllowConnA2dp());
    HILOGI("Car_connection_guard_IsAllowConnA2dp_FaBtOn_002 end");
}

/**
 * @tc.name: Car_connection_guard_IsAllowConnA2dp_FaOffBtMusicOff_003
 * @tc.desc: FA=OFF 且 BtMusic=OFF 时不允许 A2DP
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_IsAllowConnA2dp_FaOffBtMusicOff_003, TestSize.Level1)
{
    HILOGI("Car_connection_guard_IsAllowConnA2dp_FaOffBtMusicOff_003 enter");
    guard_->SetFaStatus(CarConnectionGuard::FA_STATUS_OFF);
    guard_->SetBtMusicStatus(CarConnectionGuard::BT_MUSIC_STATUS_OFF);
    EXPECT_FALSE(guard_->IsAllowConnA2dp());
    HILOGI("Car_connection_guard_IsAllowConnA2dp_FaOffBtMusicOff_003 end");
}

/**
 * @tc.name: Car_connection_guard_IsAllowConnA2dp_BtMusicOn_004
 * @tc.desc: BtMusic=ON 时允许 A2DP（即使 FA=OFF）
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_IsAllowConnA2dp_BtMusicOn_004, TestSize.Level1)
{
    HILOGI("Car_connection_guard_IsAllowConnA2dp_BtMusicOn_004 enter");
    guard_->SetFaStatus(CarConnectionGuard::FA_STATUS_OFF);
    guard_->SetBtMusicStatus(CarConnectionGuard::BT_MUSIC_STATUS_ON);
    EXPECT_TRUE(guard_->IsAllowConnA2dp());
    HILOGI("Car_connection_guard_IsAllowConnA2dp_BtMusicOn_004 end");
}

/**
 * @tc.name: Car_connection_guard_SetFaStatus_LeaveBtMusic_005
 * @tc.desc: FA 切离 BT_ON 时清除 BtMusic=ON
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_SetFaStatus_LeaveBtMusic_005, TestSize.Level1)
{
    HILOGI("Car_connection_guard_SetFaStatus_LeaveBtMusic_005 enter");
    guard_->SetBtMusicStatus(CarConnectionGuard::BT_MUSIC_STATUS_ON);
    guard_->SetFaStatus(CarConnectionGuard::FA_STATUS_OFF);
    EXPECT_EQ(guard_->GetBtMusicStatus(), CarConnectionGuard::BT_MUSIC_STATUS_OFF);
    HILOGI("Car_connection_guard_SetFaStatus_LeaveBtMusic_005 end");
}

/**
 * @tc.name: Car_connection_guard_SetFaStatus_Unknown_006
 * @tc.desc: FA=UNKNOWN 时不清除 BtMusic
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_SetFaStatus_Unknown_006, TestSize.Level1)
{
    HILOGI("Car_connection_guard_SetFaStatus_Unknown_006 enter");
    guard_->SetBtMusicStatus(CarConnectionGuard::BT_MUSIC_STATUS_ON);
    guard_->SetFaStatus(CarConnectionGuard::STATUS_UNKNOWN);
    EXPECT_EQ(guard_->GetBtMusicStatus(), CarConnectionGuard::BT_MUSIC_STATUS_ON);
    HILOGI("Car_connection_guard_SetFaStatus_Unknown_006 end");
}

/**
 * @tc.name: Car_connection_guard_IsConnectionAllowedBySinkLimit_NoDevice_007
 * @tc.desc: 无设备连接时 SinkLimit 允许
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_IsConnectionAllowedBySinkLimit_NoDevice_007, TestSize.Level1)
{
    HILOGI("Car_connection_guard_IsConnectionAllowedBySinkLimit_NoDevice_007 enter");
    EXPECT_TRUE(guard_->IsConnectionAllowedBySinkLimit(DEV_A));
    HILOGI("Car_connection_guard_IsConnectionAllowedBySinkLimit_NoDevice_007 end");
}

/**
 * @tc.name: Car_connection_guard_IsCarPlayMutex_Default_008
 * @tc.desc: IsCarPlayMutex 默认返回 false（暂未实现）
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_IsCarPlayMutex_Default_008, TestSize.Level1)
{
    HILOGI("Car_connection_guard_IsCarPlayMutex_Default_008 enter");
    EXPECT_FALSE(guard_->IsCarPlayMutex());
    HILOGI("Car_connection_guard_IsCarPlayMutex_Default_008 end");
}

/**
 * @tc.name: Car_connection_guard_Check_A2dpGateDenied_009
 * @tc.desc: Check A2DP 门控拒绝时返回 A2DP_GATE
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_Check_A2dpGateDenied_009, TestSize.Level1)
{
    HILOGI("Car_connection_guard_Check_A2dpGateDenied_009 enter");
    guard_->SetFaStatus(CarConnectionGuard::FA_STATUS_OFF);
    guard_->SetBtMusicStatus(CarConnectionGuard::BT_MUSIC_STATUS_OFF);
    auto result = guard_->Check(DEV_A, ProfileType::A2DP_SINK);
    EXPECT_FALSE(result.allowed);
    EXPECT_EQ(result.reason, DenyReason::A2DP_GATE);
    HILOGI("Car_connection_guard_Check_A2dpGateDenied_009 end");
}

/**
 * @tc.name: Car_connection_guard_Check_HfpNotAffectedByA2dpGate_010
 * @tc.desc: Check HFP 不受 A2DP 门控影响
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_Check_HfpNotAffectedByA2dpGate_010, TestSize.Level1)
{
    HILOGI("Car_connection_guard_Check_HfpNotAffectedByA2dpGate_010 enter");
    guard_->SetFaStatus(CarConnectionGuard::FA_STATUS_OFF);
    guard_->SetBtMusicStatus(CarConnectionGuard::BT_MUSIC_STATUS_OFF);
    auto result = guard_->Check(DEV_A, ProfileType::HFP_HF);
    EXPECT_TRUE(result.allowed);
    HILOGI("Car_connection_guard_Check_HfpNotAffectedByA2dpGate_010 end");
}

/**
 * @tc.name: Car_connection_guard_GetA2dpConnectedAddrs_NoService_011
 * @tc.desc: 单测环境无 A2DP service 时返回空列表
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_GetA2dpConnectedAddrs_NoService_011, TestSize.Level1)
{
    HILOGI("Car_connection_guard_GetA2dpConnectedAddrs_NoService_011 enter");
    EXPECT_TRUE(guard_->GetA2dpConnectedAddrs().empty());
    HILOGI("Car_connection_guard_GetA2dpConnectedAddrs_NoService_011 end");
}

/**
 * @tc.name: Car_connection_guard_GetHfpConnectedAddrs_NoService_012
 * @tc.desc: 单测环境无 HFP service 时返回空列表
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_GetHfpConnectedAddrs_NoService_012, TestSize.Level1)
{
    HILOGI("Car_connection_guard_GetHfpConnectedAddrs_NoService_012 enter");
    EXPECT_TRUE(guard_->GetHfpConnectedAddrs().empty());
    HILOGI("Car_connection_guard_GetHfpConnectedAddrs_NoService_012 end");
}

/**
 * @tc.name: Car_connection_guard_DisconnectOthersForSinkLimit_NoCrash_013
 * @tc.desc: SinkLimit 抢占断开在单测环境（无 service）不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_DisconnectOthersForSinkLimit_NoCrash_013, TestSize.Level0)
{
    HILOGI("Car_connection_guard_DisconnectOthersForSinkLimit_NoCrash_013 enter");
    guard_->DisconnectOthersForSinkLimit(DEV_A);
    SUCCEED();
    HILOGI("Car_connection_guard_DisconnectOthersForSinkLimit_NoCrash_013 end");
}

/**
 * @tc.name: Car_connection_guard_DenyReasonName_014
 * @tc.desc: DenyReason 枚举转字符串正确
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_DenyReasonName_014, TestSize.Level1)
{
    HILOGI("Car_connection_guard_DenyReasonName_014 enter");
    EXPECT_STREQ(DenyReasonName(DenyReason::SINK_LIMIT), "SINK_LIMIT");
    EXPECT_STREQ(DenyReasonName(DenyReason::A2DP_GATE), "A2DP_GATE");
    EXPECT_STREQ(DenyReasonName(DenyReason::CARPLAY_MUTEX), "CARPLAY_MUTEX");
    EXPECT_STREQ(DenyReasonName(DenyReason::NONE), "NONE");
    HILOGI("Car_connection_guard_DenyReasonName_014 end");
}

/**
 * @tc.name: Car_connection_guard_CheckResult_Factories_015
 * @tc.desc: CheckResult::Allow/Deny 工厂方法构造正确
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_CheckResult_Factories_015, TestSize.Level1)
{
    HILOGI("Car_connection_guard_CheckResult_Factories_015 enter");
    auto allow = CheckResult::Allow();
    EXPECT_TRUE(allow.allowed);
    EXPECT_EQ(allow.reason, DenyReason::NONE);

    auto deny = CheckResult::Deny(DenyReason::SINK_LIMIT);
    EXPECT_FALSE(deny.allowed);
    EXPECT_EQ(deny.reason, DenyReason::SINK_LIMIT);
    HILOGI("Car_connection_guard_CheckResult_Factories_015 end");
}

/**
 * @tc.name: Car_connection_guard_EmptyAddr_NoCrash_016
 * @tc.desc: 空地址调用各入口不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarConnectionGuardTest, Car_connection_guard_EmptyAddr_NoCrash_016, TestSize.Level0)
{
    HILOGI("Car_connection_guard_EmptyAddr_NoCrash_016 enter");
    guard_->Check("", ProfileType::HFP_HF);
    guard_->IsConnectionAllowedBySinkLimit("");
    guard_->DisconnectOthersForSinkLimit("");
    SUCCEED();
    HILOGI("Car_connection_guard_EmptyAddr_NoCrash_016 end");
}
}  // namespace bluetooth
}  // namespace OHOS
