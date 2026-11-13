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
#include <cstring>
#include "bluetooth_access_token_mock.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "profile_service_manager.h"
#include "bluetooth_hw_interface.h"
#include "hw_bt_hwif.h"
#include "ipc_skeleton.h"
#include "bluetooth_hw_interface.cpp"

using namespace testing;
using namespace testing::ext;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

extern "C" {
static bt_status_t g_state;
static const BthwifCallbacks* g_btAdapterhwifCallbacks = nullptr;
static bool g_readOffloadCodecRet = false;

static void SetStatus(bt_status_t state)
{
    HILOGI("state = %{public}d", state);
    g_state = state;
}
static bt_status_t BtAdapterhwifInterfaceInitMock(BthwifCallbacks* callbacks)
{
    HILOGI("state = %{public}d", g_state);
    g_btAdapterhwifCallbacks = callbacks;
    return g_state;
}
static void SetReadOffloadCodecResult(bool ret)
{
    g_readOffloadCodecRet = ret;
}
static void ResetSensorhubMock(bool enableSensorhub)
{
}
static void SetBurstParamMock(int duration, int maxExtAdvEvents, int burstWindow, int burstInterval, int advId)
{
}
static void SetScanReportChannelSensorHubMock(uint8_t filterIdx, uint8_t isToAp)
{
}
static void SendParamsToSensorhubMock(const char *params, char type, int length)
{
}
static bool GetSensorhubCapMock()
{
    return true;
}
static uint8_t GetSensorhubAdvHandleMock()
{
    return 1;
}
static void DeleteSensorhubFilterMock(char filterID)
{
}
static void SendHiechoDataMock(uint8_t type, uint8_t *source, uint16_t len, uint8_t *args)
{
}
static void HwConnAttrSetMock(uint8_t transport, const RawAddress &addr, const HwConnAttr *attr)
{
}

bool GetA2dpOffloadCodecConfigMock(const RawAddress &addr, A2dpOffloadConfigCallback &config)
{
    config.codec_type = 0x80;
    config.mtu = 0x373;
    config.l2c_rcid = 0x60;
    return g_readOffloadCodecRet;
}

uint8_t HwGetLocalDeviceEchoTypeMock()
{
    return 0x01;
}

static const bthwif_interface_t g_btAdapterhwifInterface = {
    .Init = BtAdapterhwifInterfaceInitMock,
    .resetSensorhub = ResetSensorhubMock,
    .setBurstParam = SetBurstParamMock,
    .setScanReportChannelSensorHub = SetScanReportChannelSensorHubMock,
    .sendParamsToSensorhub = SendParamsToSensorhubMock,
    .getSensorhubCap = GetSensorhubCapMock,
    .getSensorhubAdvHandle = GetSensorhubAdvHandleMock,
    .deleteSensorhubFilter = DeleteSensorhubFilterMock,
    .sendHiechoData = SendHiechoDataMock,
    .hwConnAttrSet = HwConnAttrSetMock,
    .getA2dpOffloadCodecConfig = GetA2dpOffloadCodecConfigMock,
    .hwGetLocalDeviceEchoType = HwGetLocalDeviceEchoTypeMock,
};
}

namespace OHOS {
namespace bluetooth {
const int TIME = 2;
const int TIME_1 = 1;
static Bluetooth::BluetoothHost *host_;

class BluetoothHwInterfaceTest : public testing::Test {
public:
    BluetoothHwInterfaceTest()
    {}
    ~BluetoothHwInterfaceTest()
    {}
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
}; // Bluetoothhwinterfacetest

void BluetoothHwInterfaceTest::SetUpTestSuite(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &Bluetooth::BluetoothHost::GetDefaultHost();

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
void BluetoothHwInterfaceTest::TearDownTestSuite(void)
{
    host_ = nullptr;
}
void BluetoothHwInterfaceTest::SetUp()
{}
void BluetoothHwInterfaceTest::TearDown()
{}

/**
 * @tc.name: BluetoothHwInterfaceTest_InitBtHwInterface_001
 * @tc.desc: InitBtHwInterface
 * @tc.type: FUNC bthwif is nullptr
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_InitBtHwInterface_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_InitBtHwInterface_001 start");
    EXPECT_FALSE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(nullptr));
    HILOGI("BluetoothHwInterfaceTest_InitBtHwInterface_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_InitBtHwInterface_002
 * @tc.desc: InitBtHwInterface
 * @tc.type: FUNC status != BT_STATUS_SUCCESS && status != BT_STATUS_DONE
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_InitBtHwInterface_002, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_InitBtHwInterface_002 start");
    SetStatus(BT_STATUS_FAIL);
    EXPECT_FALSE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    HILOGI("BluetoothHwInterfaceTest_InitBtHwInterface_002 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_InitBtHwInterface_003
 * @tc.desc: InitBtHwInterface
 * @tc.type: FUNC status == BT_STATUS_DONE
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_InitBtHwInterface_003, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_InitBtHwInterface_003 start");
    SetStatus(BT_STATUS_DONE);
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    HILOGI("BluetoothHwInterfaceTest_InitBtHwInterface_003 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_001
 * @tc.desc: HwHiechoIndCallCb
 * @tc.type: FUNC payload is null
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_001 start");
    SetStatus(BT_STATUS_SUCCESS);
    uint8_t echoType = HIECHO_REQ;
    uint8_t *payload = nullptr;
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t *args = nullptr;
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_002
 * @tc.desc: HwHiechoIndCallCb
 * @tc.type: FUNC args is null
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_002, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_002 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[10];
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t *args = nullptr;
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_002 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_003
 * @tc.desc: HwHiechoIndCallCb
 * @tc.type: payload != nullptr && args != nullptr
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_003, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_003 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[2] = {0x01, 0x02};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_003 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_004
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: argsLen != HIECHO_ARGS_LEN
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_004, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_004 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[2] = {0x01, 0x02};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = 0;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_004 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_005
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: len <= HIECHO_SEND_HEADER_LEN
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_005, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_005 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[2] = {0x01, 0x02};
    uint16_t len = HIECHO_SEND_HEADER_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_005 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_006
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: dataLength > (len - HIECHO_SEND_HEADER_LEN)
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_006, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_006 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[2] = {0x05, 0x06};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_006 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_007
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: DTS_CAP
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_007, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_007 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[2] = {0x05, 0x01};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    sleep(1);
    echoType = HIECHO_RSP;
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_007 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_008
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: AUDIO_CAP_QUERY
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_008, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_008 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[AUDIO_CAP_QUERY_LEN] = {0x2D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    sleep(1);

    len = AUDIO_CAP_QUERY_LEN;
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_008 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_009
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: WEAR_STATUS
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_009, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_009 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[2] = {0x06, 0x01};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);

    sleep(1);
    uint8_t payload2[WEAR_STATUS_LEN] = {0x06, 0x03, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload2, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_009 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_010
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: DTS_REPORT
 */
const uint16_t DTS_HEADER_LEN_4 = 4;
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_010, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_010 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[2] = {0x07, 0x01};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);

    sleep(1);
    echoType = HIECHO_RSP;
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);

    sleep(1);
    uint8_t payload2[DTS_HEADER_LEN_4] = {0x07, 0x04, 0x00, 0x00};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload2, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_010 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_KeepBleScanInConn_001
 * @tc.desc: KeepBleScanInConn
 * @tc.type: no need to keep ble scan
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_KeepBleScanInConn_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_KeepBleScanInConn_001 start");
    std::string callingName("softbus_server");
    std::string addr("00:00:00:00:00:00");
    STACK::RawAddress rawAddr;
    STACK::RawAddress::FromString(addr, rawAddr);
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    BluetoothHwInterface::GetInstance()->KeepBleScanInConn(callingName,
        OHOS::IPCSkeleton::GetCallingUid(), BT_TRANSPORT_LE, rawAddr);
    HILOGI("BluetoothHwInterfaceTest_KeepBleScanInConn_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_KeepBleScanInConn_002
 * @tc.desc: KeepBleScanInConn
 * @tc.type: no need to keep ble scan
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_KeepBleScanInConn_002, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_KeepBleScanInConn_002 start");
    std::string callingName("softbus_server");
    std::string addr("00:00:00:00:00:00");
    STACK::RawAddress rawAddr;
    STACK::RawAddress::FromString(addr, rawAddr);
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    BluetoothHwInterface::GetInstance()->KeepBleScanInConn(callingName,
        1024, BT_TRANSPORT_LE, rawAddr);
    HILOGI("BluetoothHwInterfaceTest_KeepBleScanInConn_002 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_KeepBleScanInConn_003
 * @tc.desc: KeepBleScanInConn
 * @tc.type: no need to keep ble scan
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_KeepBleScanInConn_003, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_KeepBleScanInConn_003 start");
    std::string callingName("softbus_server");
    std::string addr("00:00:00:00:00:00");
    STACK::RawAddress rawAddr;
    STACK::RawAddress::FromString(addr, rawAddr);
    BluetoothHwInterface::GetInstance()->SetBtHwInterface(nullptr);
    EXPECT_EQ(BluetoothHwInterface::GetInstance()->bthwInterface_, nullptr);
    BluetoothHwInterface::GetInstance()->KeepBleScanInConn(callingName,
        1024, BT_TRANSPORT_LE, rawAddr);
    HILOGI("BluetoothHwInterfaceTest_KeepBleScanInConn_003 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_011
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: QUERY_BUSINESS
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_011, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_011 start");
    uint8_t echoType = HIECHO_RSP;
    uint8_t payload[5] = {0x10, 0x02, 0x00, 0x02, 0x01};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);

    sleep(1);
    echoType = HIECHO_REQ;
    uint8_t payload2[5] = {0x10, 0x02, 0x00, 0x02, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);

    sleep(1);
    echoType = HIECHO_RSP;
    uint8_t payload3[5] = {0x10, 0x01, 0x00, 0x02, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_011 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_012
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: AUTO_CONN_SWITCH
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_012, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_012 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[4] = {0x2E, 0x01, 0x00, 0x01};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);

    sleep(1);
    echoType = HIECHO_RSP;
    uint8_t payload2[4] = {0x2E, 0x01, 0x00, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);

    sleep(1);
    echoType = HIECHO_REQ;
    uint8_t payload3[4] = {0x2E, 0x02, 0x00, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_012 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_013
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: DEVICE_DESCRIP
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_013, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_013 start");
    uint8_t echoType = HIECHO_RSP;
    uint8_t payload[4] = {0x23, 0x15, 0x00, 0x01};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
 
    sleep(1);
    echoType = HIECHO_REQ;
    uint8_t payload2[4] = {0x23, 0x15, 0x00, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
 
    sleep(1);
    echoType = HIECHO_RSP;
    uint8_t payload3[4] = {0x23, 0x02, 0x00, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_013 end");
}
 
/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_014
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: DEVICE_CONN
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_014, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_014 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[5] = {0x24, 0x02, 0x00, 0x01, 0x00};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
 
    sleep(1);
    echoType = HIECHO_RSP;
    uint8_t payload2[5] = {0x24, 0x02, 0x00, 0x01, 0x00};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
 
    sleep(1);
    echoType = HIECHO_REQ;
    uint8_t payload3[5] = {0x24, 0x01, 0x00, 0x01, 0x00};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_014 end");
}
 
/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoIndCallBack_015
 * @tc.desc: HwHiechoIndCallBack
 * @tc.type: DEVICE_STATE
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoIndCallBack_015, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_015 start");
    uint8_t echoType = HIECHO_REQ;
    uint8_t payload[5] = {0x26, 0x02, 0x00, 0x01, 0x01};
    uint16_t len = HIECHO_ARGS_LEN;
    uint8_t args[HIECHO_ARGS_LEN] = {0x01, 0x02};
    uint16_t argsLen = HIECHO_ARGS_LEN;
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
 
    sleep(1);
    echoType = HIECHO_RSP;
    uint8_t payload2[5] = {0x26, 0x02, 0x00, 0x01, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
 
    sleep(1);
    echoType = HIECHO_REQ;
    uint8_t payload3[5] = {0x26, 0x01, 0x00, 0x01, 0x01};
    g_btAdapterhwifCallbacks->hwHiechoIndCb(echoType, payload, len, args, argsLen);
    HILOGI("BluetoothHwInterfaceTest_HwHiechoIndCallBack_015 end");
}
 
/**
 * @tc.name: BluetoothHwInterfaceTest_UpdateA2dpOffloadCodecStatus_013
 * @tc.desc: UpdateA2dpOffloadCodecStatus
 * @tc.type: Rawaddress
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_UpdateA2dpOffloadCodecStatus_013, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_UpdateA2dpOffloadCodecStatus_013 start");
    RawAddress device = RawAddress("11:22:33:44:55:66");
    EXPECT_TRUE(BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface));
    SetReadOffloadCodecResult(false);
    BluetoothHwInterface::GetInstance()->UpdateA2dpOffloadCodecStatus(device);
    SetReadOffloadCodecResult(true);
    BluetoothHwInterface::GetInstance()->UpdateA2dpOffloadCodecStatus(device);
    HILOGI("BluetoothHwInterfaceTest_UpdateA2dpOffloadCodecStatus_013 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_hwProfileStateCb_001
 * @tc.desc: hwProfileStateCb
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_hwProfileStateCb_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_hwProfileStateCb_001 start");
    std::string addr("00:00:00:00:00:00");
    STACK::RawAddress rawAddr;
    STACK::RawAddress::FromString(addr, rawAddr);
    BluetoothAudioManager &bluetoothAudioManager = BluetoothAudioManager::GetInstance();
    uint8_t preA2dpState = static_cast<uint8_t>(A2DP_STATUS_ENABLE);
    uint8_t preHfpState = static_cast<uint8_t>(HFP_STATUS_ENABLE);
    bluetoothAudioManager.GetProfileStatus(addr, preA2dpState, preHfpState);
    EXPECT_EQ(preA2dpState, static_cast<uint8_t>(A2DP_STATUS_ENABLE));
    EXPECT_EQ(preHfpState, static_cast<uint8_t>(HFP_STATUS_ENABLE));
    
    g_btAdapterhwifCallbacks->hwProfileStateCb(&rawAddr, 1, 1, "xxxphone", 0);
    bluetoothAudioManager.GetProfileStatus(addr, preA2dpState, preHfpState);
    EXPECT_EQ(preA2dpState, static_cast<uint8_t>(A2DP_STATUS_ENABLE));
    EXPECT_EQ(preHfpState, static_cast<uint8_t>(HFP_STATUS_ENABLE));

    g_btAdapterhwifCallbacks->hwProfileStateCb(&rawAddr, 2, 2, "xxxphone", 0);
    bluetoothAudioManager.GetProfileStatus(addr, preA2dpState, preHfpState);
    EXPECT_EQ(preA2dpState, static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE));
    EXPECT_EQ(preHfpState, static_cast<uint8_t>(HFP_STATUS_DISABLE));

    g_btAdapterhwifCallbacks->hwProfileStateCb(&rawAddr, 2, 0, "xxxphone", 0);
    bluetoothAudioManager.GetProfileStatus(addr, preA2dpState, preHfpState);
    EXPECT_EQ(preA2dpState, static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE));
    EXPECT_EQ(preHfpState, static_cast<uint8_t>(HFP_STATUS_DISABLE));

    g_btAdapterhwifCallbacks->hwProfileStateCb(&rawAddr, 0, 2, "xxxphone", 0);
    bluetoothAudioManager.GetProfileStatus(addr, preA2dpState, preHfpState);
    EXPECT_EQ(preA2dpState, static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE));
    EXPECT_EQ(preHfpState, static_cast<uint8_t>(HFP_STATUS_DISABLE));

    g_btAdapterhwifCallbacks->hwProfileStateCb(&rawAddr, 3, 2, "xxxphone", 0);
    bluetoothAudioManager.GetProfileStatus(addr, preA2dpState, preHfpState);
    EXPECT_EQ(preA2dpState, static_cast<uint8_t>(A2DP_DISABLE_SEIZE));
    EXPECT_EQ(preHfpState, static_cast<uint8_t>(HFP_STATUS_DISABLE));

    g_btAdapterhwifCallbacks->hwProfileStateCb(&rawAddr, 1, 1, "", 0);
    g_btAdapterhwifCallbacks->hwProfileStateCb(&rawAddr, 1, 1, "", 0);
    bluetoothAudioManager.GetProfileStatus(addr, preA2dpState, preHfpState);
    EXPECT_EQ(preA2dpState, static_cast<uint8_t>(A2DP_STATUS_ENABLE));
    EXPECT_EQ(preHfpState, static_cast<uint8_t>(HFP_STATUS_ENABLE));
    HILOGI("BluetoothHwInterfaceTest_hwProfileStateCb_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_IsOtherA2dpDeviceInPlayingMusic_001
 * @tc.desc: IsOtherA2dpDeviceInPlayingMusic
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_IsOtherA2dpDeviceInPlayingMusic_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_IsOtherA2dpDeviceInPlayingMusic_001 start");
    CapsuleParam capsuleParam;
    capsuleParam.isAudioOutputToA2dp = true;
    capsuleParam.isMusicActive = true;
    capsuleParam.isA2dpActiveDeviceEqual = false;
    EXPECT_TRUE(IsOtherA2dpDeviceInPlayingMusic(capsuleParam));

    capsuleParam.isAudioOutputToA2dp = false;
    capsuleParam.isMusicActive = true;
    capsuleParam.isA2dpActiveDeviceEqual = false;
    EXPECT_FALSE(IsOtherA2dpDeviceInPlayingMusic(capsuleParam));

    capsuleParam.isAudioOutputToA2dp = true;
    capsuleParam.isMusicActive = true;
    capsuleParam.isA2dpActiveDeviceEqual = true;
    EXPECT_FALSE(IsOtherA2dpDeviceInPlayingMusic(capsuleParam));
    HILOGI("BluetoothHwInterfaceTest_IsOtherA2dpDeviceInPlayingMusic_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_IsOtherScoDeviceInCalling_001
 * @tc.desc: IsOtherScoDeviceInCalling
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_IsOtherScoDeviceInCalling_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_IsOtherScoDeviceInCalling_001 start");
    CapsuleParam capsuleParam;
    capsuleParam.isAudioOutputToSco = true;
    capsuleParam.isInActualcall = true;
    capsuleParam.isHfpActiveDeviceEqual = false;
    EXPECT_TRUE(IsOtherScoDeviceInCalling(capsuleParam));

    capsuleParam.isAudioOutputToSco = false;
    capsuleParam.isInActualcall = true;
    capsuleParam.isHfpActiveDeviceEqual = false;
    EXPECT_FALSE(IsOtherScoDeviceInCalling(capsuleParam));

    capsuleParam.isAudioOutputToSco = true;
    capsuleParam.isInActualcall = true;
    capsuleParam.isHfpActiveDeviceEqual = true;
    EXPECT_FALSE(IsOtherScoDeviceInCalling(capsuleParam));
    HILOGI("BluetoothHwInterfaceTest_IsOtherScoDeviceInCalling_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_ReportCapsuleInfo_001
 * @tc.desc: ReportCapsuleInfo
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_ReportCapsuleInfo_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_ReportCapsuleInfo_001 start");
    RawAddress addr("00:00:00:00:00:11");
    uint8_t profileState = 1;
    std::string targetBtDevice = "xxx phone";
    ReportCapsuleInfo(addr, profileState, targetBtDevice);

    profileState = 2;
    targetBtDevice = "";
    ReportCapsuleInfo(addr, profileState, targetBtDevice);
    EXPECT_TRUE(HwIsSupportMlinkDevice());
    HILOGI("BluetoothHwInterfaceTest_ReportCapsuleInfo_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_NotifyAudioManagerShowEnableCapsule_001
 * @tc.desc: NotifyAudioManagerShowEnableCapsule
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_NotifyAudioManagerShowEnableCapsule_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_NotifyAudioManagerShowEnableCapsule_001 start");
    RawAddress addr("00:00:00:00:00:12");
    CapsuleParam capsuleParam;
    capsuleParam.device = addr;
    capsuleParam.isInActualcall = true;
    capsuleParam.isHfpActiveDeviceEqual = true;
    capsuleParam.isAudioOutputToSco = true;

    capsuleParam.isA2dpActiveDeviceEqual = false;
    capsuleParam.isMusicActive = false;
    capsuleParam.isA2dpPlaying = false;
    NotifyAudioManagerShowEnableCapsule(capsuleParam);

    capsuleParam.isInActualcall = false;
    capsuleParam.isHfpActiveDeviceEqual = true;
    capsuleParam.isAudioOutputToSco = true;

    capsuleParam.isA2dpActiveDeviceEqual = true;
    capsuleParam.isMusicActive = true;
    capsuleParam.isA2dpPlaying = true;
    NotifyAudioManagerShowEnableCapsule(capsuleParam);

    capsuleParam.isInActualcall = true;
    capsuleParam.isHfpActiveDeviceEqual = false;
    capsuleParam.isAudioOutputToSco = true;

    capsuleParam.isA2dpActiveDeviceEqual = false;
    capsuleParam.isMusicActive = true;
    capsuleParam.isA2dpPlaying = true;
    NotifyAudioManagerShowEnableCapsule(capsuleParam);
    EXPECT_TRUE(HwIsSupportMlinkDevice());
    HILOGI("BluetoothHwInterfaceTest_NotifyAudioManagerShowEnableCapsule_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_NotifyAudioManagerShowDisableCapsule_001
 * @tc.desc: NotifyAudioManagerShowDisableCapsule
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_NotifyAudioManagerShowDisableCapsule_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_NotifyAudioManagerShowDisableCapsule_001 start");
    std::string device{"00:00:00:00:00:13"};
    RawAddress addr(device);
    CapsuleParam capsuleParam;
    capsuleParam.device = addr;
    capsuleParam.targetBtDevice = "";
    capsuleParam.isA2dpPlaying = true;
    capsuleParam.isMusicActive = true;
    capsuleParam.isAudioOutputToA2dp = true;
    capsuleParam.isAudioConnected = false;
    NotifyAudioManagerShowDisableCapsule(capsuleParam);

    capsuleParam.targetBtDevice = "xxx phone";
    capsuleParam.isA2dpPlaying = false;
    capsuleParam.isMusicActive = false;
    capsuleParam.isAudioOutputToA2dp = false;
    capsuleParam.isAudioConnected = true;
    NotifyAudioManagerShowDisableCapsule(capsuleParam);

    capsuleParam.targetBtDevice = "xxx phone";
    capsuleParam.isA2dpPlaying = false;
    capsuleParam.isMusicActive = false;
    capsuleParam.isAudioOutputToA2dp = false;
    capsuleParam.isAudioConnected = false;

    RefusePlayHelper::GetInstance()->SetLastHangUpTime(device);
    NotifyAudioManagerShowDisableCapsule(capsuleParam);

    sleep(2);
    NotifyAudioManagerShowDisableCapsule(capsuleParam);
    EXPECT_TRUE(HwIsSupportMlinkDevice());
    HILOGI("BluetoothHwInterfaceTest_NotifyAudioManagerShowDisableCapsule_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_BoolToStr_001
 * @tc.desc: BoolToStr
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_BoolToStr_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_BoolToStr_001 start");
    EXPECT_EQ(BoolToStr(true), BOOL_TRUE_VALUE);
    EXPECT_EQ(BoolToStr(false), BOOL_FALSE_VALUE);
    HILOGI("BluetoothHwInterfaceTest_BoolToStr_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_PrintCapsuleParam_001
 * @tc.desc: PrintCapsuleParam
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_PrintCapsuleParam_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_PrintCapsuleParam_001 start");
    CapsuleParam capsuleParam;
    capsuleParam.isAudioOutputToA2dp = false;
    capsuleParam.isAudioOutputToSco = true;
    capsuleParam.isMusicActive = false;
    capsuleParam.isA2dpPlaying = false;
    capsuleParam.isInActualcall = true;
    capsuleParam.isAudioConnected = true;
    capsuleParam.isA2dpActiveDeviceEqual = false;
    capsuleParam.isHfpActiveDeviceEqual = true;
    PrintCapsuleParam(capsuleParam);
    EXPECT_TRUE(HwIsSupportMlinkDevice());
    HILOGI("BluetoothHwInterfaceTest_PrintCapsuleParam_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_NotifyAudioManagerShowCapsule_001
 * @tc.desc: NotifyAudioManagerShowCapsule
 * @tc.type:
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_NotifyAudioManagerShowCapsule_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_NotifyAudioManagerShowCapsule_001 start");
    std::string device{"00:00:00:00:00:13"};
    RawAddress addr(device);
    uint8_t a2dpState = static_cast<uint8_t>(A2DP_STATUS_ENABLE);
    uint8_t hfpState = static_cast<uint8_t>(HFP_STATUS_ENABLE);
    std::string targetBtDevice = "";
    NotifyAudioManagerShowCapsule(addr, a2dpState, hfpState, targetBtDevice, true);

    a2dpState = static_cast<uint8_t>(A2DP_STATUS_ENABLE);
    hfpState = static_cast<uint8_t>(HFP_STATUS_ENABLE);
    NotifyAudioManagerShowCapsule(addr, a2dpState, hfpState, targetBtDevice, false);

    a2dpState = static_cast<uint8_t>(A2DP_DISABLE_SEIZE);
    hfpState = static_cast<uint8_t>(HFP_STATUS_DISABLE);
    NotifyAudioManagerShowCapsule(addr, a2dpState, hfpState, targetBtDevice, true);

    a2dpState = static_cast<uint8_t>(A2DP_DISABLE_SEIZE);
    hfpState = static_cast<uint8_t>(HFP_STATUS_DISABLE);
    NotifyAudioManagerShowCapsule(addr, a2dpState, hfpState, targetBtDevice, false);

    a2dpState = static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE);
    hfpState = static_cast<uint8_t>(HFP_STATUS_DISABLE);
    NotifyAudioManagerShowCapsule(addr, a2dpState, hfpState, targetBtDevice, true);
    EXPECT_TRUE(HwIsSupportMlinkDevice());

    a2dpState = static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE);
    hfpState = static_cast<uint8_t>(HFP_STATUS_DISABLE);
    NotifyAudioManagerShowCapsule(addr, a2dpState, hfpState, targetBtDevice, false);
    EXPECT_TRUE(HwIsSupportMlinkDevice());
    HILOGI("BluetoothHwInterfaceTest_NotifyAudioManagerShowCapsule_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoDeviceCtrl_001
 * @tc.desc: HwHiechoDeviceCtrl
 * @tc.type: 
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoDeviceCtrl_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoDeviceCtrl_001 start");
    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface);
    uint8_t remoteAddr[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    uint32_t controlTypeVal = 1;
    uint32_t controlObject = 2;
    uint32_t controlType = DEVICE_CTRL_CMD_PLAY;
    BluetoothHwInterface::GetInstance()->HwHiechoDeviceCtrl(remoteAddr, controlType, controlTypeVal, controlObject);
    controlType = DEVICE_CTRL_CMD_VIBRATE;
    BluetoothHwInterface::GetInstance()->HwHiechoDeviceCtrl(remoteAddr, controlType, controlTypeVal, controlObject);
    controlType = DEVICE_CTRL_CMD_FLASH;
    BluetoothHwInterface::GetInstance()->HwHiechoDeviceCtrl(remoteAddr, controlType, controlTypeVal, controlObject);
    controlType = DEVICE_CTRL_CMD_LOCK;
    BluetoothHwInterface::GetInstance()->HwHiechoDeviceCtrl(remoteAddr, controlType, controlTypeVal, controlObject);
    controlType = DEVICE_CTRL_CMD_ERASE;
    BluetoothHwInterface::GetInstance()->HwHiechoDeviceCtrl(remoteAddr, controlType, controlTypeVal, controlObject);
    EXPECT_TRUE(HwIsSupportMlinkDevice());
    HILOGI("BluetoothHwInterfaceTest_HwHiechoDeviceCtrl_001 end");
}

/**
 * @tc.name: BluetoothHwInterfaceTest_HwHiechoSendHwAccountHashToTwsCb_001
 * @tc.desc: HwHiechoSendHwAccountHashToTwsCb
 * @tc.type: 
 */
HWTEST_F(BluetoothHwInterfaceTest, BluetoothHwInterfaceTest_HwHiechoSendHwAccountHashToTwsCb_001, TestSize.Level1)
{
    HILOGI("BluetoothHwInterfaceTest_HwHiechoSendHwAccountHashToTwsCb_001 start");
    const uint8_t DUMMY_ADDRESS[6] { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
    STACK::RawAddress device = STACK::RawAddress(DUMMY_ADDRESS);
    EXPECT_TRUE(device != STACK::RawAddress::kEmpty);

    BluetoothHwInterface::GetInstance()->InitBtHwInterface(&g_btAdapterhwifInterface);
    HwHiechoSendHwAccountHashToTwsCb(device);
    EXPECT_TRUE(HwIsSupportMlinkDevice());
    HILOGI("BluetoothHwInterfaceTest_HwHiechoSendHwAccountHashToTwsCb_001 end");
}

} // bluetooth
} // OHOS
