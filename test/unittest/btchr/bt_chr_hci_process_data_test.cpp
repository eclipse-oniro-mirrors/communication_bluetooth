/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd. All rights reserved.
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
#define private public
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <sstream>
#include <random>
#include "thread_util.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bt_chr_hci_process_data.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

constexpr int SINGLE_STR_LENGTH = 3;
constexpr int TOTLE_STR_LENGTH = 1023;
constexpr int DIGITS_NUM = 10; // for 0~9
constexpr int CHAR_NUM = 26; // for a~z

class BtChrHciProcessDataTest : public testing::Test {
public:
    BtChrHciProcessDataTest() = default;
    ~BtChrHciProcessDataTest() = default;

    static void SetUpTestCase() {
        ThreadUtil::GetInstance().ChangeThreadState(
            THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::NOT_SWITCH_THREAD);
    }
    static void TearDownTestCase() {}
    void SetUp() {};
    void TearDown() {};
};

std::string StrRand(uint32_t length)
{
    std::random_device rd;
    std::default_random_engine random(rd());

    std::string res {};
    for (int i = 0; i < length; i++) {
        uint32_t val = random() % (DIGITS_NUM + CHAR_NUM + CHAR_NUM); // 26 + 26 + 10
        if (val < DIGITS_NUM) {
            res += static_cast<char>('0' + val);
        } else if (val < (CHAR_NUM + DIGITS_NUM)) {
            res += static_cast<char>('a' + (val - DIGITS_NUM));
        } else {
            res += static_cast<char>('A' + (val - CHAR_NUM - DIGITS_NUM));
        }
    }

    return res;
}

std::string AddrRand()
{
    std::random_device rd;
    std::default_random_engine random(rd());
    static std::vector<std::string> randAddr = {
        "11:d8:45:5c:9d:aa",
        "22:d8:45:5c:9d:bb",
        "33:d8:45:5c:9d:cc",
        "55:d8:45:5c:9d:dd",
        "66:d8:45:5c:9d:ee",
        "88:d8:45:5c:9d:ff"
    };
    uint32_t val = random() % sizeof(randAddr);
    return randAddr[val];
}

void SaveImportantHciProcessDataTest(const std::string &peerAddr)
{
    HILOGI("SaveImportantHciProcessDataTest running thread...:addr: %{public}s", peerAddr.c_str());
    for (int j = 0; j < TOTLE_STR_LENGTH; j++) {
        std::string code = StrRand(SINGLE_STR_LENGTH);
        BtChrHciProcessData::GetInstance().SaveImportantHciProcessData(peerAddr, code);
    }
    HILOGI("SaveImportantHciProcessDataTest end thread...:addr: %{public}s", peerAddr.c_str());
}

void GetImportantHciProcessDataTest()
{
    std::string expCode {};
    const std::string peerAddr = AddrRand();
    SaveImportantHciProcessDataTest(peerAddr);
    BtChrHciProcessData::GetInstance().GetImportantHciProcessData(peerAddr);
}

/**
 * @tc.name: Test_SaveImportantHciProcessData_001
 * @tc.desc: Verify the SaveImportantHciProcessData function.
 * @tc.type: FUNC
 */
HWMTEST_F(BtChrHciProcessDataTest, Test_SaveImportantHciProcessData_001, TestSize.Level1, 10)
{
    HILOGI("Test_SaveImportantHciProcessData_001 enter");
    string expCode {};
    const std::string peerAddr = AddrRand();
    SaveImportantHciProcessDataTest(peerAddr);
    HILOGI("Test_SaveImportantHciProcessData_001 waiting......");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    HILOGI("Test_SaveImportantHciProcessData_001 end");
}

/**
 * @tc.name: Test_GetImportantHciProcessData_001
 * @tc.desc: Verify the GetImportantHciProcessData function.
 * @tc.type: FUNC
 */
HWMTEST_F(BtChrHciProcessDataTest, Test_GetImportantHciProcessData_001, TestSize.Level1, 10)
{
    HILOGI("Test_GetImportantHciProcessData_001 enter");
    GetImportantHciProcessDataTest();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    HILOGI("Test_GetImportantHciProcessData_001 end");
}

/**
 * @tc.name: Test_GetImportantHciProcessData_002
 * @tc.desc: Verify the GetImportantHciProcessData function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrHciProcessDataTest, Test_GetImportantHciProcessData_002, TestSize.Level1)
{
    HILOGI("Test_GetImportantHciProcessData_002 enter");
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::DISABLED);
    std::string expCode {};
    const std::string peerAddr = "88:d8:45:5c:9d:aa";
    for (int i = 0; i < 350; i++) {
        std::string code = StrRand(SINGLE_STR_LENGTH);
        BtChrHciProcessData::GetInstance().SaveImportantHciProcessData(peerAddr, code);
        std::string ret = BtChrHciProcessData::GetInstance().GetImportantHciProcessData(peerAddr);
        expCode += code;
        if (expCode.size() > TOTLE_STR_LENGTH) {
            expCode = expCode.substr(expCode.size() - TOTLE_STR_LENGTH);
        }
        EXPECT_EQ(ret, expCode);
    }
    HILOGI("Test_GetImportantHciProcessData_002 end");
}
}  // namespace bluetooth
}  // namespace OHOS