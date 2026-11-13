/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "bluetooth_kia_refuse_policy_manager.h"
#include "control_intercept_plugin.h"
#include "ipc_skeleton.h"
#include "log.h"
#include "syspara/parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
constexpr const char SYS_PARAM_SERVICE_FORCE_ENABLE[] = "const.pc_security.fileguard_force_enable";
class BluetoothKiaRefusePolicyManagerTest : public testing::Test {
public:
    BluetoothKiaRefusePolicyManagerTest() {}
    ~BluetoothKiaRefusePolicyManagerTest() {}
 
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp(void);
    void TearDown(void);
};

void BluetoothKiaRefusePolicyManagerTest::SetUpTestSuite(void)
{
    system::SetParameter(SYS_PARAM_SERVICE_FORCE_ENABLE, "true");
}
void BluetoothKiaRefusePolicyManagerTest::TearDownTestSuite(void)
{
}
void BluetoothKiaRefusePolicyManagerTest::SetUp(void)
{}
void BluetoothKiaRefusePolicyManagerTest::TearDown(void)
{}

static int64_t GetBootTime()
{
    constexpr int64_t msPerSecond = 1000;
    constexpr int64_t nsPerMs = 1000000;
    struct timespec times;
    if (clock_gettime(CLOCK_MONOTONIC, &times) < 0) {
        HILOGE("Failed clock_gettime:%{public}s, ShouldRefuseConnect:false", strerror(errno));
        return INT64_MAX;
    }
    int64_t bootTime = ((times.tv_sec * msPerSecond) + (times.tv_nsec / nsPerMs));
    HILOGI("bootTime:%{public}lu", bootTime);
    return bootTime;
}

/*
 * @tc.number:bluetooth_kia_refuse_policy_manager_test_001
 * @tc.name: state
 * @tc.desc:not add policy
 */
HWTEST_F(BluetoothKiaRefusePolicyManagerTest, bluetooth_kia_refuse_policy_manager_test_001, TestSize.Level0)
{
    HILOGI("bluetooth_kia_refuse_policy_manager_test_001 enter");
    const int32_t pidA = 1111;
    const int32_t pidB = 2222;
    EXPECT_FALSE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_GATT, pidA));
    EXPECT_FALSE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_GATT, pidB));
    EXPECT_FALSE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_SPP, pidA));
    HILOGI("bluetooth_kia_refuse_policy_manager_test_001 end");
}

/*
 * @tc.number:bluetooth_kia_refuse_policy_manager_test_002
 * @tc.name: state
 * @tc.desc:add(REFUSE_PROTOCOL_TYPE_GATT, pidA, GetBootTime() + 10)
 */
HWTEST_F(BluetoothKiaRefusePolicyManagerTest, bluetooth_kia_refuse_policy_manager_test_002, TestSize.Level0)
{
    HILOGI("bluetooth_kia_refuse_policy_manager_test_002 enter");
    const int32_t pidA = 1111;
    const int32_t pidB = 2222;
    const int64_t prohibitedSecondsTime = GetBootTime() + 10;
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().UpdateRefusePolicy(REFUSE_PROTOCOL_TYPE_GATT,
        pidA, prohibitedSecondsTime) == 0);
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_GATT, pidA));
    EXPECT_FALSE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_GATT, pidB));
    sleep(10);
    EXPECT_FALSE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_GATT, pidA));
    HILOGI("bluetooth_kia_refuse_policy_manager_test_002 end");
}

/*
 * @tc.number:bluetooth_kia_refuse_policy_manager_test_003
 * @tc.name: state
 * @tc.desc:add(REFUSE_PROTOCOL_TYPE_GATT, pidA, GetBootTime() + 10)
 * add(REFUSE_PROTOCOL_TYPE_SPP, pidB, GetBootTime() + 10)
 */
HWTEST_F(BluetoothKiaRefusePolicyManagerTest, bluetooth_kia_refuse_policy_manager_test_003, TestSize.Level0)
{
    HILOGI("bluetooth_kia_refuse_policy_manager_test_003 enter");
    const int32_t pidA = 3333;
    const int32_t pidB = 4444;
    const int64_t prohibitedSecondsTimeA= GetBootTime() + 5000; // 5s
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().UpdateRefusePolicy(REFUSE_PROTOCOL_TYPE_GATT,
        pidA, prohibitedSecondsTimeA) == 0);
    const int64_t prohibitedSecondsTimeB= GetBootTime() + 10000; // 10s
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_GATT, pidA));
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().UpdateRefusePolicy(REFUSE_PROTOCOL_TYPE_SPP,
        pidB, prohibitedSecondsTimeB) == 0);
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_SPP, pidB));
    sleep(5);
    EXPECT_FALSE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_GATT, pidA));
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_SPP, pidB));
    sleep(5);
    EXPECT_FALSE(BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_SPP, pidB));
    HILOGI("bluetooth_kia_refuse_policy_manager_test_003 end");
}

/*
 * @tc.number:bluetooth_kia_refuse_policy_manager_test_004
 * @tc.name: state
 * @tc.desc:测试插件
 */
HWTEST_F(BluetoothKiaRefusePolicyManagerTest, bluetooth_kia_refuse_policy_manager_test_004, TestSize.Level0)
{
    HILOGI("bluetooth_kia_refuse_policy_manager_test_004 enter");
    int32_t pid = IPCSkeleton::GetCallingPid();
    const int64_t prohibitedSecondsTime= GetBootTime() + 5000; // 5s
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().UpdateRefusePolicy(REFUSE_PROTOCOL_TYPE_GATT,
        pid, prohibitedSecondsTime) == 0);
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().UpdateRefusePolicy(REFUSE_PROTOCOL_TYPE_SPP,
        pid, prohibitedSecondsTime) == 0);
    EXPECT_TRUE(BluetoothRefusePolicyManager::GetInstance().UpdateRefusePolicy(REFUSE_PROTOCOL_TYPE_OPP,
        pid, prohibitedSecondsTime) == 0);

    const std::vector<std::string> filePaths = {"/data/test1.txt", "/data/test2.txt"};
    ControlInterceptMessage msg {
        .addr = "12:34:56:78:90:AB",
        .isSystemHap = true,
        .isNativeCaller = true,
        .pid = pid,
    };
    EXPECT_FALSE(ControlInterceptIsAllowedFastestGattConn(msg));
    EXPECT_FALSE(ControlInterceptIsAllowedGattConn(msg));
    EXPECT_FALSE(ControlInterceptIsAllowedSocketConn(msg));

    msg.filePaths = {};
    EXPECT_FALSE(ControlInterceptIsAllowedGattWrite(msg));

    msg.isSystemHap = false;
    msg.isNativeCaller = false;
    msg.filePaths = filePaths;
    EXPECT_FALSE(ControlInterceptIsAllowedOppWrite(msg));

    msg.filePaths = {};
    EXPECT_TRUE(ControlInterceptIsAllowedPairConn(msg));

    sleep(5);
    msg.isSystemHap = true;
    msg.isNativeCaller = true;
    EXPECT_TRUE(ControlInterceptIsAllowedFastestGattConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedGattConn(msg));
    EXPECT_TRUE(ControlInterceptIsAllowedSocketConn(msg));

    msg.filePaths = {};
    EXPECT_TRUE(ControlInterceptIsAllowedGattWrite(msg));

    msg.filePaths = filePaths;
    EXPECT_TRUE(ControlInterceptIsAllowedOppWrite(msg));

    EXPECT_TRUE(ControlInterceptIsAllowedPairConn(msg));
    HILOGI("bluetooth_kia_refuse_policy_manager_test_004 end");
}
}  // namespace bluetooth
}  // namespace OHOS