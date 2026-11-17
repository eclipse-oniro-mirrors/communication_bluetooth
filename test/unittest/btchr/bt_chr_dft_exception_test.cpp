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
#include "bt_chr_dft_exception.h"
#include "bt_chr_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
static BluetoothHost *host_;
class BtChrDftExceptionTest : public testing::Test {
public:
    BtChrDftExceptionTest() = default;
    ~BtChrDftExceptionTest() = default;

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
 * @tc.name: Test_BtChrDftEventWriteInt_001
 * @tc.desc: Verify the BtChrDftEventWriteInt function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftExceptionTest, Test_BtChrDftEventWriteInt_001, TestSize.Level1)
{
    HILOGI("Test_BtChrDftEventWriteInt_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    BtChrDftEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "LINKTYPE", 1);
    BtChrDftEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "A2DPSTATUS", 0);
    BtChrDftEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "HFPSTATUS", 2);
    BtChrDftEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "DISCONNECTREASON", 22);
    BtChrDftEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "ISPASSIVE", 1);
    BtChrDftEventWriteInt(CHR_BT_EXCEPTION_NEW, peerAddr, CHR_ERRCODE, 12);
    BtChrDftEventWriteInt(CHR_UNION_HISI_COMMON_INFO, peerAddr, "SIGANL_STRENGTH", -33);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrDftEventWriteInt_001 end");
}

/**
 * @tc.name: Test_BtChrDftEventWriteTime_001
 * @tc.desc: Verify the BtChrDftEventWriteTime function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftExceptionTest, Test_BtChrDftEventWriteTime_001, TestSize.Level1)
{
    HILOGI("Test_BtChrDftEventWriteTime_001 enter");
    std::string peerAddr = "74:0a:12:d4:56:d5";
    BtChrDftEventWriteTime(CHR_USER_DISCONNECT, peerAddr, "ACLCONNECTIONTIME");
    BtChrDftEventWriteTime(CHR_USER_DISCONNECT, peerAddr, "ACLCONNECTEDTIME");
    BtChrDftEventWriteTime("BT_USER_OTHER_EVENT", peerAddr, "ACLCONNECTEDTIME");
    BtChrDftEventWriteInt(CHR_USER_DISCONNECT, peerAddr, "DISCONNECTREASON", 19);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrDftEventWriteTime_001 end");
}

/**
 * @tc.name: Test_BtChrBtExcpEvent_001
 * @tc.desc: Verify the BtChrBtExcpEvent function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftExceptionTest, Test_BtChrBtExcpEvent_001, TestSize.Level1)
{
    HILOGI("Test_BtChrBtExcpEvent_001 enter");
    BtChrBtExcpEvent("74:0a:12:d4:56:d5", BTOPT_RESOURCE_BLE_NO_FREE, CHR_SUB_ERRCODE_CASE1);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrBtExcpEvent_001 end");
}

/**
 * @tc.name: Test_BtChrSocketExcpEvent_001
 * @tc.desc: Verify the BtChrSocketExcpEvent function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftExceptionTest, Test_BtChrSocketExcpEvent_001, TestSize.Level1)
{
    HILOGI("Test_BtChrSocketExcpEvent_001 enter");
    std::string uuid = "001-002";
    std::string pkgName = "com.example.settings";
    BtChrAddSocketCallingRecord("74:0a:12:d4:56:d5", uuid, "");
    std::string packageName = BtChrSocketExcpEvent("74:0a:12:d4:56:d5", uuid, CLIENT_CONNECT_PENDING, 1);
    EXPECT_STREQ(packageName.c_str(), "unknow");

    BtChrAddSocketCallingRecord("74:0a:12:d4:56:d6", uuid, pkgName);
    packageName = BtChrSocketExcpEvent("74:0a:12:d4:56:d6", uuid, CLIENT_CONNECT_SUCCESS, 0);
    EXPECT_STREQ(packageName.c_str(), pkgName.c_str());

    packageName = BtChrSocketExcpEvent("74:0a:12:d4:56:d7", uuid, CLIENT_CONNECT_START_FAIL, 0);
    EXPECT_STREQ(packageName.c_str(), "unknow");
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrSocketExcpEvent_001 end");
}

/**
 * @tc.name: Test_BtChrRemovePairHandle_001
 * @tc.desc: Verify the BtChrRemovePairHandle function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftExceptionTest, Test_BtChrRemovePairHandle_001, TestSize.Level1)
{
    HILOGI("Test_BtChrRemovePairHandle_001 enter");
    BtChrRemovePairHandle("74:0a:12:d4:56:d5", 1);
    BtChrRemovePairHandle("74:0a:12:d4:56:d9", 0);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrRemovePairHandle_001 end");
}

/**
 * @tc.name: Test_BtChrSetAudioVolume_001
 * @tc.desc: Verify the BtChrSetAudioVolume function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftExceptionTest, Test_BtChrSetAudioVolume_001, TestSize.Level1)
{
    HILOGI("Test_BtChrSetAudioVolume_001 enter");
    BtChrSetAudioVolume("74:0a:12:d4:56:d5", 0);
    BtChrSetAudioVolume("74:0a:12:d4:56:d9", 5);
    BtChrSetAudioVolume("74:0a:12:d4:56:d9", -1);
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrSetAudioVolume_001 end");
}

/**
 * @tc.name: Test_BtChrResetExcpEvent_001
 * @tc.desc: Verify the BtChrResetExcpEvent function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftExceptionTest, Test_BtChrResetExcpEvent_001, TestSize.Level1)
{
    HILOGI("Test_BtChrResetExcpEvent_001 enter");
    BtChrResetExcpEvent("STATE_TURN_OFF");
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrResetExcpEvent_001 end");
}

/**
 * @tc.name: Test_BtChrBleConnData_001
 * @tc.desc: Verify the BtChrBleConnData function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrDftExceptionTest, Test_BtChrBleConnData_001, TestSize.Level1)
{
    HILOGI("Test_BtChrBleConnData_001 enter");
    BtChrBleConnData("74:0a:12:d4:56:d5", 0, "blutooth");
    BtChrBleConnData("74:0a:12:d4:56:d5", 1, "blutooth");
    BtChrBleConnData("74:0a:12:d4:56:d5", 2, "blutooth");
    EXPECT_NE(GetCurrentTime(), "");
    HILOGI("Test_BtChrBleConnData_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS