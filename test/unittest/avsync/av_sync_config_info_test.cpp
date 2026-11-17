/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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

#include "bluetooth_log.h"
#include "av_sync_common.h"
#include "av_sync_config_info.h"
#include "adapter_manager.h"
#include "classic_adapter.h"
#include "class_creator.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
AvSyncConfigInfo avSyncConfigInfo;

static void *ClassicAdapterCreate(void)
{
    return new ClassicAdapter();
}

class AvSyncConfigInfoTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {
        avSyncConfigInfo.ClearLatencyInfo();
    }
};

/**
 * @tc.number: AvSyncConfigInfoTest_AddLatencyBaseByDeviceName_001
 * @tc.name: AddLatencyBaseByDeviceName
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncConfigInfoTest, AvSyncConfigInfoTest_AddLatencyBaseByDeviceName_001, TestSize.Level1)
{
    HILOGI("AvSyncConfigInfoTest_AddLatencyBaseByDeviceName_001 enter");
    avSyncConfigInfo.AddLatencyBaseByDeviceName("FreeBuds", 300);
    int32_t avLatency = avSyncConfigInfo.GetLatencyBaseByDeviceName("FreeBuds");
    EXPECT_EQ(avLatency, 300);
    avSyncConfigInfo.AddLatencyBaseByDeviceName("", 300);
    avSyncConfigInfo.AddLatencyBaseByDeviceName("FreeBuds", INVALID_LATENCY);
    avLatency = avSyncConfigInfo.GetLatencyBaseByDeviceName("FreeBuds");
    EXPECT_EQ(avLatency, 300);
    HILOGI("AvSyncConfigInfoTest_AddLatencyBaseByDeviceName_001 end");
}

/**
 * @tc.number: AvSyncConfigInfoTest_AddLatencyBaseByModelId_001
 * @tc.name: AddLatencyBaseByModelId
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncConfigInfoTest, AvSyncConfigInfoTest_AddLatencyBaseByModelId_001, TestSize.Level1)
{
    HILOGI("AvSyncConfigInfoTest_AddLatencyBaseByModelId_001 enter");
    avSyncConfigInfo.AddLatencyBaseByModelId("290", 200);
    int32_t avLatency = avSyncConfigInfo.GetLatencyBaseByModelId("290");
    EXPECT_EQ(avLatency, 200);
    avSyncConfigInfo.AddLatencyBaseByModelId("", 300);
    avSyncConfigInfo.AddLatencyBaseByModelId("290", INVALID_LATENCY);
    HILOGI("AvSyncConfigInfoTest_AddLatencyBaseByModelId_001 end");
}

/**
 * @tc.number: AvSyncConfigInfoTest_AddLatencyBaseByNewModelId_001
 * @tc.name: AddLatencyBaseByNewModelId
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncConfigInfoTest, AvSyncConfigInfoTest_AddLatencyBaseByNewModelId_001, TestSize.Level1)
{
    HILOGI("AvSyncConfigInfoTest_AddLatencyBaseByNewModelId_001 enter");
    avSyncConfigInfo.AddLatencyBaseByNewModelId("ZC02", 400);
    int32_t avLatency = avSyncConfigInfo.GetLatencyBaseByNewModelId("ZC02");
    EXPECT_EQ(avLatency, 400);
    avSyncConfigInfo.AddLatencyBaseByNewModelId("", 400);
    avSyncConfigInfo.AddLatencyBaseByNewModelId("ZC02", INVALID_LATENCY);
    HILOGI("AvSyncConfigInfoTest_AddLatencyBaseByNewModelId_001 end");
}

/**
 * @tc.number: AvSyncConfigInfoTest_GetLatencyBase_001
 * @tc.name: GetLatencyBase
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncConfigInfoTest, AvSyncConfigInfoTest_GetLatencyBase_001, TestSize.Level1)
{
    HILOGI("AvSyncConfigInfoTest_GetLatencyBase_001 enter");
    RawAddress rawAddr("00:11:22:33:44:55");
    int32_t latency = avSyncConfigInfo.GetLatencyBase(rawAddr);
    EXPECT_EQ(latency, INVALID_LATENCY);

    ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
    AdapterManager::GetInstance()->CreateAdapters();
    latency = avSyncConfigInfo.GetLatencyBase(rawAddr);
    EXPECT_EQ(latency, INVALID_LATENCY);
    HILOGI("AvSyncConfigInfoTest_GetLatencyBase_001 end");
}

/**
 * @tc.number: AvSyncConfigInfoTest_GetLatencyBaseByDeviceName_001
 * @tc.name: GetLatencyBaseByDeviceName
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncConfigInfoTest, AvSyncConfigInfoTest_GetLatencyBaseByDeviceName_001, TestSize.Level1)
{
    HILOGI("AvSyncConfigInfoTest_GetLatencyBaseByDeviceName_001 enter");
    int32_t latency = avSyncConfigInfo.GetLatencyBaseByDeviceName("");
    EXPECT_EQ(latency, INVALID_LATENCY);

    std::string deviceName = "FreeBuds";
    latency = avSyncConfigInfo.GetLatencyBaseByDeviceName(deviceName);
    EXPECT_EQ(latency, INVALID_LATENCY);

    int32_t avLatency = 300;
    avSyncConfigInfo.AddLatencyBaseByDeviceName(deviceName, avLatency);
    latency = avSyncConfigInfo.GetLatencyBaseByDeviceName(deviceName);
    EXPECT_EQ(latency, avLatency);
    HILOGI("AvSyncConfigInfoTest_GetLatencyBaseByDeviceName_001 end");
}

/**
 * @tc.number: AvSyncConfigInfoTest_GetLatencyBaseByModelId_001
 * @tc.name: GetLatencyBaseByModelId
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncConfigInfoTest, AvSyncConfigInfoTest_GetLatencyBaseByModelId_001, TestSize.Level1)
{
    HILOGI("AvSyncConfigInfoTest_GetLatencyBaseByModelId_001 enter");
    int32_t latency = avSyncConfigInfo.GetLatencyBaseByModelId("");
    EXPECT_EQ(latency, INVALID_LATENCY);

    std::string modelId = "290";
    latency = avSyncConfigInfo.GetLatencyBaseByModelId(modelId);
    EXPECT_EQ(latency, INVALID_LATENCY);

    int32_t avLatency = 300;
    avSyncConfigInfo.AddLatencyBaseByModelId(modelId, avLatency);
    latency = avSyncConfigInfo.GetLatencyBaseByModelId(modelId);
    EXPECT_EQ(latency, avLatency);
    HILOGI("AvSyncConfigInfoTest_GetLatencyBaseByModelId_001 end");
}

/**
 * @tc.number: AvSyncConfigInfoTest_GetLatencyBaseByNewModelId_001
 * @tc.name: GetLatencyBaseByNewModelId
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncConfigInfoTest, AvSyncConfigInfoTest_GetLatencyBaseByNewModelId_001, TestSize.Level1)
{
    HILOGI("AvSyncConfigInfoTest_GetLatencyBaseByNewModelId_001 enter");
    int32_t latency = avSyncConfigInfo.GetLatencyBaseByNewModelId("");
    EXPECT_EQ(latency, INVALID_LATENCY);

    std::string newModelId = "ZA09";
    latency = avSyncConfigInfo.GetLatencyBaseByNewModelId(newModelId);
    EXPECT_EQ(latency, INVALID_LATENCY);

    int32_t avLatency = 300;
    avSyncConfigInfo.AddLatencyBaseByNewModelId(newModelId, avLatency);
    latency = avSyncConfigInfo.GetLatencyBaseByNewModelId(newModelId);
    EXPECT_EQ(latency, avLatency);
    HILOGI("AvSyncConfigInfoTest_GetLatencyBaseByNewModelId_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS