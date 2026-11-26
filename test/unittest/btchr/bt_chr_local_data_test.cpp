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
#include <gtest/gtest.h>

#include "bt_chr_local_data.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class BtChrLocalDataTest : public testing::Test {
public:
    BtChrLocalDataTest() = default;
    ~BtChrLocalDataTest() = default;

    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override
    {
        BtChrLocalData::GetInstance().Stop();
    }
};

/**
 * @tc.name: GetCountryInfoWithoutStart
 * @tc.desc: return empty country info when the channel is not started
 */
HWTEST_F(BtChrLocalDataTest, GetCountryInfoWithoutStart_001, TestSize.Level1)
{
    BtChrLocalData::GetInstance().Stop();
    EXPECT_TRUE(BtChrLocalData::GetInstance().GetCountryInfo().empty());
}

/**
 * @tc.name: StartStopIdempotent
 * @tc.desc: start and stop can be called repeatedly, cache is cleared after stop
 */
HWTEST_F(BtChrLocalDataTest, StartStopIdempotent_001, TestSize.Level1)
{
    BtChrLocalData::GetInstance().Start();
    BtChrLocalData::GetInstance().Start();
    BtChrLocalData::GetInstance().Stop();
    BtChrLocalData::GetInstance().Stop();
    EXPECT_TRUE(BtChrLocalData::GetInstance().GetCountryInfo().empty());
}

/**
 * @tc.name: GetCountryInfoAfterStart
 * @tc.desc: fetching or observer registration failure does not crash
 */
HWTEST_F(BtChrLocalDataTest, GetCountryInfoAfterStart_001, TestSize.Level1)
{
    BtChrLocalData::GetInstance().Start();
    (void)BtChrLocalData::GetInstance().GetCountryInfo();
    (void)BtChrLocalData::GetInstance().GetCountryInfo(false);
    BtChrLocalData::GetInstance().RefreshCountryInfo();
    SUCCEED();
}

/**
 * @tc.name: RefreshCountryInfoAfterStop
 * @tc.desc: state change callback is ignored after stop
 */
HWTEST_F(BtChrLocalDataTest, RefreshCountryInfoAfterStop_001, TestSize.Level1)
{
    BtChrLocalData::GetInstance().Stop();
    BtChrLocalData::GetInstance().RefreshCountryInfo();
    EXPECT_TRUE(BtChrLocalData::GetInstance().GetCountryInfo().empty());
}
}  // namespace bluetooth
}  // namespace OHOS
