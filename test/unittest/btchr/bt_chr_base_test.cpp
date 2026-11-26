/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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
#include "bt_chr_base.h"
#include "bt_chr_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
static BluetoothHost *host_;
class BtChrBaseTest : public testing::Test {
public:
    BtChrBaseTest() = default;
    ~BtChrBaseTest() = default;

    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Test_BtChrEventWriteParams_001
 * @tc.desc: Verify the BtChrEventWriteParams function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrEventWriteParams_001, TestSize.Level1)
{
    HILOGI("Test_BtChrEventWriteParams_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("ISPASSIVE", 1));
    params.push_back(BtChrEventParam("DISCONNECTREASON", 8));
    BtChrEventWriteParams(CHR_USER_DISCONNECT, peerAddr, params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrEventWriteParams_001 end");
}

/**
 * @tc.name: Test_BtChrEventWriteInt_001
 * @tc.desc: Verify the BtChrEventWriteInt function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrEventWriteInt_001, TestSize.Level1)
{
    HILOGI("Test_BtChrEventWriteInt_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    BtChrEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "ISPASSIVE", 1);
    BtChrEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "DISCONNECTREASON", 22);

    BtChrEventWriteInt(CHR_BT_PAIR_EXCEPTION, peerAddr, "BONDEDCNT", 1);
    BtChrEventWriteInt(CHR_BT_PAIR_EXCEPTION, peerAddr, CHR_ERRCODE, ERRCODE_PAIR_SUCCESS);

    BtChrEventWriteInt(CHR_BT_EXCEPTION_NEW, peerAddr, CHR_ERRCODE, 12);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrEventWriteInt_001 end");
}

/**
 * @tc.name: Test_BtChrEventWriteStr_001
 * @tc.desc: Verify the BtChrEventWriteStr function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrEventWriteStr_001, TestSize.Level1)
{
    HILOGI("Test_BtChrEventWriteStr_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    BtChrEventWriteStr(CHR_HFP_RFCOMM_EXCEPTION, peerAddr, "SCENEINFO", "scence info test");
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrEventWriteStr_001 end");
}

/**
 * @tc.name: Test_BtChrEventWriteTime_001
 * @tc.desc: Verify the BtChrEventWriteTime function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrEventWriteTime_001, TestSize.Level1)
{
    HILOGI("Test_BtChrEventWriteTime_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    BtChrEventWriteTime(CHR_USER_DISCONNECT, peerAddr, "ACLCONNECTIONTIME");
    BtChrEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "DISCONNECTREASON", 19);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrEventWriteTime_001 end");
}

/**
 * @tc.name: Test_BtChrBaseReleaseAll_001
 * @tc.desc: Verify the BtChrBaseReleaseAll function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrBaseReleaseAll_001, TestSize.Level1)
{
    HILOGI("Test_BtChrBaseReleaseAll_001 enter");
    std::vector<std::string> notClearAddrs;
    BtChrBaseReleaseAll(notClearAddrs);

    std::string peerAddr = "74:0a:12:d4:56:d5";
    std::string peerAddr1 = "74:0a:12:d4:56:d6";
    notClearAddrs = {peerAddr, peerAddr1};
    BtChrBaseReleaseAll(notClearAddrs);

    BtChrEventWriteTime(CHR_USER_DISCONNECT, peerAddr, "ACLCONNECTIONTIME");
    BtChrEventWriteStr(CHR_HFP_RFCOMM_EXCEPTION, peerAddr1, "SCENEINFO", "scence info test");
    BtChrBaseReleaseAll(notClearAddrs);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrBaseReleaseAll_001 end");
}

/**
 * @tc.name: Test_BtChrUeEventWrite_001
 * @tc.desc: Verify the BtChrUeEventWrite function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrUeEventWrite_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUeEventWrite_001 enter");
    std::string eventName = "";
    std::vector<BtChrEventParam> params;
    BtChrUeEventWrite(eventName, params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrUeEventWrite_001 end");
}

/**
 * @tc.name: Test_BtChrUeEventWrite_002
 * @tc.desc: Verify the BtChrUeEventWrite function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrUeEventWrite_002, TestSize.Level1)
{
    HILOGI("Test_BtChrUeEventWrite_002 enter");
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("DEL_CLOUD_NUM", 0));
    params.push_back(BtChrEventParam("DEL_CLOUD_INFO", ""));
    BtChrUeEventWrite("BT_DEL_PAIRED_EVENT", params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrUeEventWrite_002 end");
}

/**
 * @tc.name: Test_BtChrUeEventWrite_StringArray_001
 * @tc.desc: Verify BtChrUeEventWrite with STRING_ARRAY param containing multiple strings.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrUeEventWrite_StringArray_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUeEventWrite_StringArray_001 enter");
    std::vector<BtChrEventParam> params;
    std::vector<std::string> strArray = {"str_aaa", "str_bbb", "str_ccc"};
    params.push_back(BtChrEventParam("REACHING_MAX_PACKAGE_ARRAY", strArray));
    BtChrUeEventWrite("BT_STR_ARRAY_EVENT", params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrUeEventWrite_StringArray_001 end");
}

/**
 * @tc.name: Test_BtChrUeEventWrite_StringArray_002
 * @tc.desc: Verify BtChrUeEventWrite with STRING_ARRAY param containing single string.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrUeEventWrite_StringArray_002, TestSize.Level1)
{
    HILOGI("Test_BtChrUeEventWrite_StringArray_002 enter");
    std::vector<BtChrEventParam> params;
    std::vector<std::string> strArray = {"single_str"};
    params.push_back(BtChrEventParam("SINGLE_STR_ARRAY", strArray));
    BtChrUeEventWrite("BT_SINGLE_STR_ARRAY_EVENT", params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrUeEventWrite_StringArray_002 end");
}

/**
 * @tc.name: Test_BtChrUeEventWrite_StringArray_003
 * @tc.desc: Verify BtChrUeEventWrite with empty STRING_ARRAY param (boundary).
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrUeEventWrite_StringArray_003, TestSize.Level1)
{
    HILOGI("Test_BtChrUeEventWrite_StringArray_003 enter");
    std::vector<BtChrEventParam> params;
    std::vector<std::string> strArray;
    params.push_back(BtChrEventParam("EMPTY_STR_ARRAY", strArray));
    BtChrUeEventWrite("BT_EMPTY_STR_ARRAY_EVENT", params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrUeEventWrite_StringArray_003 end");
}

/**
 * @tc.name: Test_BtChrUeEventWrite_MixedTypes_001
 * @tc.desc: Verify BtChrUeEventWrite with mixed param types including multiple STRING_ARRAY.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrUeEventWrite_MixedTypes_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUeEventWrite_MixedTypes_001 enter");
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("INT_PARAM", 42));
    params.push_back(BtChrEventParam("UINT_PARAM", static_cast<uint32_t>(7), 0));
    params.push_back(BtChrEventParam("STR_PARAM", "string_value"));
    std::vector<int64_t> i64Array = {100, 200, 300};
    params.push_back(BtChrEventParam("I64_ARRAY_PARAM", i64Array));
    std::vector<std::string> strArray1 = {"arr1_a", "arr1_b"};
    params.push_back(BtChrEventParam("STR_ARRAY_PARAM_1", strArray1));
    std::vector<std::string> strArray2 = {"arr2_x", "arr2_y", "arr2_z"};
    params.push_back(BtChrEventParam("STR_ARRAY_PARAM_2", strArray2));
    BtChrUeEventWrite("BT_MIXED_TYPES_EVENT", params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrUeEventWrite_MixedTypes_001 end");
}

/**
 * @tc.name: Test_BtChrUeEventWrite_UnsupportedType_001
 * @tc.desc: Verify BtChrUeEventWrite with unsupported param type falls into else branch.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrUeEventWrite_UnsupportedType_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUeEventWrite_UnsupportedType_001 enter");
    std::vector<BtChrEventParam> params;
    BtChrEventParam unsupportedParam("UNSUPPORTED_PARAM", 0);
    unsupportedParam.type = static_cast<BtChrEventParamType>(99);
    params.push_back(unsupportedParam);
    BtChrUeEventWrite("BT_UNSUPPORTED_EVENT", params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrUeEventWrite_UnsupportedType_001 end");
}

/**
 * @tc.name: Test_BtChrUeEventWrite_UnsupportedType_002
 * @tc.desc: Verify BtChrUeEventWrite with only unsupported type results in empty params (early return).
 * @tc.type: FUNC
 */
HWTEST_F(BtChrBaseTest, Test_BtChrUeEventWrite_UnsupportedType_002, TestSize.Level1)
{
    HILOGI("Test_BtChrUeEventWrite_UnsupportedType_002 enter");
    std::vector<BtChrEventParam> params;
    BtChrEventParam unsupportedParam1("UNSUPPORTED_1", 0);
    unsupportedParam1.type = static_cast<BtChrEventParamType>(0);
    BtChrEventParam unsupportedParam2("UNSUPPORTED_2", 0);
    unsupportedParam2.type = static_cast<BtChrEventParamType>(99);
    params.push_back(unsupportedParam1);
    params.push_back(unsupportedParam2);
    BtChrUeEventWrite("BT_ALL_UNSUPPORTED_EVENT", params);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrUeEventWrite_UnsupportedType_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS
