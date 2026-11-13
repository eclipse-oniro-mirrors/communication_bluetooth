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

#include "car_device_type_resolver.h"

#include <gtest/gtest.h>
#include <memory>

#include "log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace bluetooth {

const std::string DEV_A = "00:11:22:33:44:55";

class CarDeviceTypeResolverTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("CarDeviceTypeResolverTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("CarDeviceTypeResolverTest::TearDownTestCase");
    }
    void SetUp() override
    {
        HILOGI("CarDeviceTypeResolverTest::SetUp start");
        resolver_ = std::make_unique<CarDeviceTypeResolver>();
    }
    void TearDown() override
    {
        HILOGI("CarDeviceTypeResolverTest::TearDown start");
        resolver_ = nullptr;
    }

protected:
    std::unique_ptr<CarDeviceTypeResolver> resolver_;
};

/**
 * @tc.name: Car_device_type_resolver_IsHwPhone_NoAdapter_001
 * @tc.desc: 单测环境无 ClassicAdapter，IsHwPhone fail-safe 返回 false
 * @tc.type: FUNC
 */
HWTEST_F(CarDeviceTypeResolverTest, Car_device_type_resolver_IsHwPhone_NoAdapter_001, TestSize.Level1)
{
    HILOGI("Car_device_type_resolver_IsHwPhone_NoAdapter_001 enter");
    EXPECT_FALSE(resolver_->IsHwPhone(DEV_A));
    HILOGI("Car_device_type_resolver_IsHwPhone_NoAdapter_001 end");
}

/**
 * @tc.name: Car_device_type_resolver_IsHwCarDevice_NoAdapter_002
 * @tc.desc: 单测环境无 ClassicAdapter，IsHwCarDevice fail-safe 返回 false
 * @tc.type: FUNC
 */
HWTEST_F(CarDeviceTypeResolverTest, Car_device_type_resolver_IsHwCarDevice_NoAdapter_002, TestSize.Level1)
{
    HILOGI("Car_device_type_resolver_IsHwCarDevice_NoAdapter_002 enter");
    EXPECT_FALSE(resolver_->IsHwCarDevice(DEV_A));
    HILOGI("Car_device_type_resolver_IsHwCarDevice_NoAdapter_002 end");
}

/**
 * @tc.name: Car_device_type_resolver_EmptyAddr_NoCrash_003
 * @tc.desc: 空地址 fail-safe 返回 false 不崩溃
 * @tc.type: FUNC
 */
HWTEST_F(CarDeviceTypeResolverTest, Car_device_type_resolver_EmptyAddr_NoCrash_003, TestSize.Level0)
{
    HILOGI("Car_device_type_resolver_EmptyAddr_NoCrash_003 enter");
    EXPECT_FALSE(resolver_->IsHwPhone(""));
    EXPECT_FALSE(resolver_->IsHwCarDevice(""));
    HILOGI("Car_device_type_resolver_EmptyAddr_NoCrash_003 end");
}
}  // namespace bluetooth
}  // namespace OHOS
