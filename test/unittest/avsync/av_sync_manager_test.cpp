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

#include "av_sync_manager.h"
#include "av_sync_common.h"
#include "bluetooth_hw_interface.h"
#include "remote_device_properties.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
RawAddress rawAddr("00:11:22:33:44:55");
AvSyncManager avSyncManager;

extern "C" {
static bt_status_t HwIfInitMock(BthwifCallbacks *callbacks)
{
    return BT_STATUS_SUCCESS;
}

static void HwUpdateBtAudioLatencyMock(const STACK::RawAddress &addr, int32_t latency)
{
    return;
}

static bthwif_interface_t g_btHwIfTest = {
    .size = sizeof(g_btHwIfTest),
    .Init = HwIfInitMock,
    .hwUpdateBtAudioLatency = HwUpdateBtAudioLatencyMock,
};
}

class AvSyncManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/**
 * @tc.number: AvSyncManagerTest_SaveHiechoAvLatency_001
 * @tc.name: SaveHiechoAvLatency
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncManagerTest, AvSyncManagerTest_SaveHiechoAvLatency_001, TestSize.Level1)
{
    HILOGI("AvSyncManagerTest_SaveHiechoAvLatency_001 enter");
    avSyncManager.SaveHiechoAvLatency(rawAddr, 320);
    int32_t avLatency = avSyncManager.GetAvSyncLatency(rawAddr);
    EXPECT_EQ(avLatency, 320);
    HILOGI("AvSyncManagerTest_SaveHiechoAvLatency_001 end");
}

cJSON* CreateLatencyJson(int32_t latencyMs, std::string deviceName, std::string modelId, std::string newModelId)
{
    cJSON* baseListJson = cJSON_CreateArray();
    cJSON* deviceNameJson = cJSON_CreateObject();
    cJSON_AddStringToObject(deviceNameJson, "device_name", deviceName.c_str());
    cJSON_AddNumberToObject(deviceNameJson, "latency_ms", latencyMs);
    cJSON_AddItemToArray(baseListJson, deviceNameJson);
    
    cJSON* modelIdJson = cJSON_CreateObject();
    cJSON_AddStringToObject(modelIdJson, "model_id", modelId.c_str());
    cJSON_AddNumberToObject(modelIdJson, "latency_ms", latencyMs);
    cJSON_AddItemToArray(baseListJson, modelIdJson);

    cJSON* newModelIdJson = cJSON_CreateObject();
    cJSON_AddStringToObject(newModelIdJson, "new_model_id", newModelId.c_str());
    cJSON_AddNumberToObject(newModelIdJson, "latency_ms", latencyMs);
    cJSON_AddItemToArray(baseListJson, newModelIdJson);

    cJSON* root = cJSON_CreateArray();
    cJSON* baseJson = cJSON_CreateObject();
    cJSON_AddItemToObject(baseJson, "latency_base", baseListJson);
    cJSON_AddItemToArray(root, baseJson);
    return root;
}

/**
 * @tc.number: AvSyncManagerTest_ParseLatencyConfig_001
 * @tc.name: ParseLatencyConfig
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncManagerTest, AvSyncManagerTest_ParseLatencyConfig_001, TestSize.Level1)
{
    avSyncManager.isAlreadyLoad_ = true;
    avSyncManager.ReadLatencyConfig();
    avSyncManager.isAlreadyLoad_ = false;
    avSyncManager.ReadLatencyConfig();
    cJSON* root = cJSON_CreateArray();
    EXPECT_FALSE(avSyncManager.ParseLatencyConfig(root));

    cJSON_Delete(root);
    root = cJSON_CreateArray();
    EXPECT_FALSE(avSyncManager.ParseLatencyConfig(root));

    cJSON* invalidBaseJson = cJSON_CreateObject();
    cJSON_AddStringToObject(invalidBaseJson, "latency_base1", "");
    cJSON_AddItemToArray(root, invalidBaseJson);
    EXPECT_FALSE(avSyncManager.ParseLatencyConfig(root));

    cJSON* baseJson = cJSON_CreateObject();
    cJSON* latencyBaseArray = cJSON_CreateArray();
    cJSON_AddItemToObject(baseJson, "latency_base", latencyBaseArray);
    cJSON_AddItemToArray(root, baseJson);
    EXPECT_FALSE(avSyncManager.ParseLatencyConfig(root));
    cJSON_Delete(root);

    root = CreateLatencyJson(300, "FreeBuds", "290", "ZA09");
    EXPECT_TRUE(avSyncManager.ParseLatencyConfig(root));
    cJSON_Delete(root);
 
    root = CreateLatencyJson(-1, "FreeBuds", "290", "ZA09");
    EXPECT_TRUE(avSyncManager.ParseLatencyConfig(root));
    cJSON_Delete(root);

    root = CreateLatencyJson(200, "", "", "");
    EXPECT_TRUE(avSyncManager.ParseLatencyConfig(root));
    cJSON_Delete(root);
    HILOGI("AvSyncManagerTest_ParseLatencyConfig_001 end");
}

/**
 * @tc.number: AvSyncManagerTest_UpdateBtAudioLatency_001
 * @tc.name: UpdateBtAudioLatency
 * @tc.desc: FUNC
 */
HWTEST_F(AvSyncManagerTest, AvSyncManagerTest_UpdateBtAudioLatency_001, TestSize.Level1)
{
    HILOGI("AvSyncManagerTest_UpdateBtAudioLatency_001 enter");
    avSyncManager.UpdateBtAudioLatency(rawAddr);

    avSyncManager.hiechoAvLatencyMap_.Clear();
    avSyncManager.UpdateBtAudioLatency(rawAddr);

    std::string deviceName = "FreeBuds";
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btHwIfTest);
    RemoteDeviceProperties::GetInstance()->UpdateRemoteDeviceName(rawAddr, deviceName);
    avSyncManager.UpdateBtAudioLatency(rawAddr);

    avSyncManager.UpdateBtAudioLatency(rawAddr);

    avSyncManager.ClearLatencyInfo();
    int32_t setLatency = MAX_AV_LATENCY - 1;
    avSyncManager.avSyncConfigInfo_.AddLatencyBaseByDeviceName(deviceName, setLatency);
    avSyncManager.UpdateBtAudioLatency(rawAddr);

    deviceName = "FreeBuds pro";
    RemoteDeviceProperties::GetInstance()->UpdateRemoteDeviceName(rawAddr, deviceName);
    avSyncManager.avSyncConfigInfo_.AddLatencyBaseByDeviceName(deviceName, MAX_AV_LATENCY);
    avSyncManager.UpdateBtAudioLatency(rawAddr);

    avSyncManager.ClearLatencyInfo();
    avSyncManager.UpdateBtAudioLatency(rawAddr);
    int32_t avLatency = avSyncManager.GetAvSyncLatency(rawAddr);
    EXPECT_EQ(avLatency, INVALID_LATENCY);
    avSyncManager.ClearLatencyInfoByAddr(rawAddr);
    HILOGI("AvSyncManagerTest_UpdateBtAudioLatency_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS