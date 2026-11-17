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
#include "json_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
class JsonUtilTest : public testing::Test {
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
 * @tc.number: JsonUtilTest_GetInt32_001
 * @tc.name: GetInt32
 * @tc.desc: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest_GetInt32_001, TestSize.Level0)
{
    HILOGI("JsonUtilTest_GetInt32_001 enter");
    int latency = 300;
    cJSON* jsonValue = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonValue, "latency_ms", latency);
    EXPECT_EQ(JsonUtil::GetInt32(jsonValue, "latency_ms", 0), latency);
    EXPECT_EQ(JsonUtil::GetInt32(jsonValue, "latency", 0), 0);

    std::string latency_str = "400";
    cJSON_AddStringToObject(jsonValue, "latency_ms_str", latency_str.c_str());
    EXPECT_EQ(JsonUtil::GetInt32(jsonValue, "latency_ms_str", 0), 0);
    cJSON_Delete(jsonValue);
    HILOGI("JsonUtilTest_GetInt32_001 end");
}

/**
 * @tc.number: JsonUtilTest_GetString_001
 * @tc.name: GetString
 * @tc.desc: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest_GetString_001, TestSize.Level0)
{
    HILOGI("JsonUtilTest_GetString_001 enter");
    std::string deviceName = "FreeBuds";
    cJSON* jsonValue = cJSON_CreateObject();
    cJSON_AddStringToObject(jsonValue, "device_name", deviceName.c_str());
    EXPECT_EQ(JsonUtil::GetString(jsonValue, "device_name", ""), deviceName);
    EXPECT_EQ(JsonUtil::GetString(jsonValue, "device_", ""), "");

    int name = 200;
    cJSON_AddNumberToObject(jsonValue, "device_name_num", name);
    EXPECT_EQ(JsonUtil::GetString(jsonValue, "device_name_num", ""), "");
    cJSON_Delete(jsonValue);
    HILOGI("JsonUtilTest_GetString_001 end");
}


/**
 * @tc.number: JsonUtilTest_GetSimpleFormatString_001
 * @tc.name: GetSimpleFormatString
 * @tc.desc: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest_GetSimpleFormatString_001, TestSize.Level0)
{
    HILOGI("JsonUtilTest_GetSimpleFormatString_001 enter");
    std::string deviceName = "FreeBuds";
    cJSON* jsonValue = cJSON_CreateObject();
    cJSON_AddStringToObject(jsonValue, "device_name", deviceName.c_str());
    cJSON_AddNumberToObject(jsonValue, "latency_ms", 300);
    EXPECT_EQ(JsonUtil::GetSimpleFormatString(jsonValue), "{\"device_name\":\"FreeBuds\",\"latency_ms\":300}");
    cJSON_Delete(jsonValue);
    HILOGI("JsonUtilTest_GetSimpleFormatString_001 end");
}

/**
 * @tc.number: JsonUtilTest_ReadJsonFile_001
 * @tc.name: ReadJsonFile
 * @tc.desc: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest_ReadJsonFile_001, TestSize.Level0)
{
    HILOGI("JsonUtilTest_ReadJsonFile_001 enter");
    std::string filePath = "/system/etc/bluetooth/bt_av_latency_config.json";
    cJSON* jsonValue = JsonUtil::ReadJsonFile(filePath.c_str());
    EXPECT_NE(jsonValue, nullptr);

    jsonValue = JsonUtil::ReadJsonFile("data/test.json");
    EXPECT_EQ(jsonValue, nullptr);
    cJSON_Delete(jsonValue);
    HILOGI("JsonUtilTest_ReadJsonFile_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS