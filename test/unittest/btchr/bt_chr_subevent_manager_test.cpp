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
#include "bt_chr_subevent_manager.h"
#include "bt_chr_dft_exception.h"
#include "cJSON.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
constexpr int MAX_DEVICE_INFO_NUM = 100;
static BluetoothHost *host_;
class BtChrSubeventManagerTest : public testing::Test {
public:
    BtChrSubeventManagerTest() = default;
    ~BtChrSubeventManagerTest() = default;

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
 * @tc.name: Test_GetDeviceInfoFromMap_001
 * @tc.desc: Verify the GetDeviceInfoFromMap function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_GetDeviceInfoFromMap_001, TestSize.Level1)
{
    HILOGI("Test_GetDeviceInfoFromMap_001 enter");
    std::string deviceName = "DeviceName";
    BtChrSetLocalDeviceName(const_cast<char*>(deviceName.c_str()));
    std::string addr = "11:22:33:44:55:60";
    BtChrDeviceInfo deviceInfo;
    EXPECT_FALSE(GetDeviceInfoFromMap(addr, deviceInfo));
    BtChrUpdateLinkType(addr, 1);
    EXPECT_TRUE(GetDeviceInfoFromMap(addr, deviceInfo));
    EXPECT_TRUE(deviceInfo.linkType == 1);
    HILOGI("Test_GetDeviceInfoFromMap_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateLinkType_001
 * @tc.desc: Verify the BtChrUpdateLinkType function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateLinkType_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateLinkType_001 enter");
    std::string addr = "11:22:33:44:55:61";
    BtChrUpdateLinkType(addr, 0);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateLinkType_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
    cJSON* likeType = cJSON_GetObjectItemCaseSensitive(deviceInfo, "LIKETYPE");
        EXPECT_TRUE(likeType != nullptr && cJSON_IsNumber(likeType) && likeType->valueint == 0);
    }

    BtChrUpdateLinkType(addr, 1);
    btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON_Delete(jsonVal);
    jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateLinkType_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* likeType = cJSON_GetObjectItemCaseSensitive(deviceInfo, "LIKETYPE");
        EXPECT_TRUE(likeType != nullptr && cJSON_IsNumber(likeType) && likeType->valueint == 1);
    }
    cJSON_Delete(jsonVal);
    HILOGI("Test_BtChrUpdateLinkType_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateA2dpStatus_001
 * @tc.desc: Verify the BtChrUpdateA2dpStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateA2dpStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateA2dpStatus_001 enter");
    std::string addr = "11:22:33:44:55:62";
    BtChrUpdateA2dpStatus(addr, 0);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateLinkType_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* a2dpStatus = cJSON_GetObjectItemCaseSensitive(deviceInfo, "A2DPSTATUS");
        EXPECT_TRUE(a2dpStatus != nullptr && cJSON_IsNumber(a2dpStatus) && a2dpStatus->valueint == 0);
    }

    BtChrUpdateLinkType(addr, 1);
    BtChrUpdateA2dpStatus(addr, 1);
    btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON_Delete(jsonVal);
    jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateA2dpStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* a2dpStatus = cJSON_GetObjectItemCaseSensitive(deviceInfo, "A2DPSTATUS");
        EXPECT_TRUE(a2dpStatus != nullptr && cJSON_IsNumber(a2dpStatus) && a2dpStatus->valueint == 1);
    }
    cJSON_Delete(jsonVal);
    HILOGI("Test_BtChrUpdateA2dpStatus_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateHfpStatus_001
 * @tc.desc: Verify the BtChrUpdateHfpStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateHfpStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateHfpStatus_001 enter");
    std::string addr = "11:22:33:44:55:63";
    BtChrUpdateHfpStatus(addr, 0);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateLinkType_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* hfpStatus = cJSON_GetObjectItemCaseSensitive(deviceInfo, "HFPSTATUS");
        EXPECT_TRUE(hfpStatus != nullptr && cJSON_IsNumber(hfpStatus) && hfpStatus->valueint == 0);
    }

    BtChrUpdateLinkType(addr, 1);
    BtChrUpdateHfpStatus(addr, 1);
    btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON_Delete(jsonVal);
    jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateA2dpStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* hfpStatus = cJSON_GetObjectItemCaseSensitive(deviceInfo, "HFPSTATUS");
        EXPECT_TRUE(hfpStatus != nullptr && cJSON_IsNumber(hfpStatus) && hfpStatus->valueint == 1);
    }
    cJSON_Delete(jsonVal);
    HILOGI("Test_BtChrUpdateHfpStatus_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateHidStatus_001
 * @tc.desc: Verify the BtChrUpdateHidStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateHidStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateHidStatus_001 enter");
    std::string addr = "11:22:33:44:55:64";
    BtChrUpdateHidStatus(addr, 0);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateHidStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* hidStatus = cJSON_GetObjectItemCaseSensitive(deviceInfo, "HIDSTATUS");
        EXPECT_TRUE(hidStatus != nullptr && cJSON_IsNumber(hidStatus) && hidStatus->valueint == 0);
    }

    BtChrUpdateLinkType(addr, 1);
    BtChrUpdateHidStatus(addr, 1);
    btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON_Delete(jsonVal);
    jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateHidStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* hidStatus = cJSON_GetObjectItemCaseSensitive(deviceInfo, "HIDSTATUS");
        EXPECT_TRUE(hidStatus != nullptr && cJSON_IsNumber(hidStatus) && hidStatus->valueint == 1);
    }
    cJSON_Delete(jsonVal);
    HILOGI("Test_BtChrUpdateHidStatus_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateSupportA2dpStatus_001
 * @tc.desc: Verify the BtChrUpdateSupportA2dpStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateSupportA2dpStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateSupportA2dpStatus_001 enter");
    std::string addr = "11:22:33:44:55:65";
    BtChrUpdateSupportA2dpStatus(addr, 1);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateSupportA2dpStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* needA2dp = cJSON_GetObjectItemCaseSensitive(deviceInfo, "NEEDA2DP");
        EXPECT_TRUE(needA2dp != nullptr && cJSON_IsNumber(needA2dp) && needA2dp->valueint == 1);
    }

    BtChrUpdateLinkType(addr, 1);
    BtChrUpdateSupportA2dpStatus(addr, 0);
    btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON_Delete(jsonVal);
    jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateSupportA2dpStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* needA2dp = cJSON_GetObjectItemCaseSensitive(deviceInfo, "NEEDA2DP");
        EXPECT_TRUE(needA2dp != nullptr && cJSON_IsNumber(needA2dp) && needA2dp->valueint == 0);
    }
    cJSON_Delete(jsonVal);
    HILOGI("Test_BtChrUpdateSupportA2dpStatus_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateSupportHfpStatus_001
 * @tc.desc: Verify the BtChrUpdateSupportHfpStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateSupportHfpStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateSupportHfpStatus_001 enter");
    std::string addr = "11:22:33:44:55:66";
    BtChrUpdateSupportHfpStatus(addr, 1);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateSupportHfpStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* needHfp = cJSON_GetObjectItemCaseSensitive(deviceInfo, "NEEDHFP");
        EXPECT_TRUE(needHfp != nullptr && cJSON_IsNumber(needHfp) && needHfp->valueint == 1);
    }

    BtChrUpdateLinkType(addr, 1);
    BtChrUpdateSupportHfpStatus(addr, 0);
    btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON_Delete(jsonVal);
    jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateSupportHfpStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* needHfp = cJSON_GetObjectItemCaseSensitive(deviceInfo, "NEEDHFP");
        EXPECT_TRUE(needHfp != nullptr && cJSON_IsNumber(needHfp) && needHfp->valueint == 0);
    }
    cJSON_Delete(jsonVal);
    HILOGI("Test_BtChrUpdateSupportHfpStatus_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateSupportHidStatus_001
 * @tc.desc: Verify the BtChrUpdateSupportHidStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateSupportHidStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateSupportHidStatus_001 enter");
    std::string addr = "11:22:33:44:55:67";
    BtChrUpdateSupportHidStatus(addr, 1);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateSupportHidStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* needHid = cJSON_GetObjectItemCaseSensitive(deviceInfo, "NEEDHID");
        EXPECT_TRUE(needHid != nullptr && cJSON_IsNumber(needHid) && needHid->valueint == 1);
    }

    BtChrUpdateLinkType(addr, 1);
    BtChrUpdateSupportHidStatus(addr, 0);
    btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON_Delete(jsonVal);
    jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateSupportHidStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* needHid = cJSON_GetObjectItemCaseSensitive(deviceInfo, "NEEDHID");
        EXPECT_TRUE(needHid != nullptr && cJSON_IsNumber(needHid) && needHid->valueint == 0);
    }
    cJSON_Delete(jsonVal);
    HILOGI("Test_BtChrUpdateSupportHidStatus_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateDeviceInfo_001
 * @tc.desc: Verify the BtChrUpdateDeviceInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateDeviceInfo_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateDeviceInfo_001 enter");
    std::string addr = "11:22:33:44:55:65";
    std::string deviceName = "DeviceNameTest";
    BtChrUpdateDeviceInfo(addr, deviceName, 10, 0x102);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateLinkType_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* deviceNameJson = cJSON_GetObjectItemCaseSensitive(deviceInfo, "DEVICENAME");
        EXPECT_TRUE(deviceNameJson != nullptr && cJSON_IsString(deviceNameJson) &&
                    std::string(deviceNameJson->valuestring) == deviceName);
        cJSON* btDeviceRssi = cJSON_GetObjectItemCaseSensitive(deviceInfo, "BTDEVICERSSI");
        EXPECT_TRUE(btDeviceRssi != nullptr && cJSON_IsNumber(btDeviceRssi) &&
                    btDeviceRssi->valueint == 10);
    }

    std::string deviceName1 = "DeviceNameTest1";
    BtChrUpdateDeviceInfo(addr, deviceName1, 15, 0x102);
    btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON_Delete(jsonVal);
    jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    if (jsonVal) {
        HILOGI("Test_BtChrUpdateA2dpStatus_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
        deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
        if (cJSON_IsObject(deviceInfo)) {
            cJSON* deviceNameJson = cJSON_GetObjectItemCaseSensitive(deviceInfo, "DEVICENAME");
            EXPECT_TRUE(deviceNameJson != nullptr && cJSON_IsString(deviceNameJson) &&
                        std::string(deviceNameJson->valuestring) == deviceName1);
            cJSON* btDeviceRssi = cJSON_GetObjectItemCaseSensitive(deviceInfo, "BTDEVICERSSI");
            EXPECT_TRUE(btDeviceRssi != nullptr && cJSON_IsNumber(btDeviceRssi) &&
                        btDeviceRssi->valueint == 15);
        }
        cJSON_Delete(jsonVal);
    }
    HILOGI("Test_BtChrUpdateDeviceInfo_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateChipInfo_001
 * @tc.desc: Verify the BtChrUpdateChipInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateChipInfo_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateChipInfo_001 enter");
    std::string addr = "11:22:33:44:55:65";
    int btVersion = 11;
    int btSubVersion = 277;
    int chipCompanyId = 688;
    BtChrUpdateChipInfo(addr, btVersion, btSubVersion, chipCompanyId);
    std::string btCommonJonStr = BtChrGetBtCommonJsonStr(addr);
    cJSON* jsonVal = cJSON_Parse(btCommonJonStr.c_str());
    EXPECT_NE(jsonVal, nullptr);
    HILOGI("Test_BtChrUpdateLinkType_001 btCommonJonStr: %{public}s", btCommonJonStr.c_str());
    cJSON* deviceInfo = cJSON_GetObjectItemCaseSensitive(jsonVal, "DEVICEINFO");
    if (cJSON_IsObject(deviceInfo)) {
        cJSON* btVersionJson = cJSON_GetObjectItemCaseSensitive(deviceInfo, "BTVERSION");
        EXPECT_TRUE(btVersionJson != nullptr && cJSON_IsNumber(btVersionJson) &&
                    btVersionJson->valueint == btVersion);
        cJSON* btSubVersionJson = cJSON_GetObjectItemCaseSensitive(deviceInfo, "BTSUBVERSION");
        EXPECT_TRUE(btSubVersionJson != nullptr && cJSON_IsNumber(btSubVersionJson) &&
                    btSubVersionJson->valueint == btSubVersion);
        cJSON* chipCompanyIdJson = cJSON_GetObjectItemCaseSensitive(deviceInfo, "CHIPCOMPANYID");
        EXPECT_TRUE(chipCompanyIdJson != nullptr && cJSON_IsNumber(chipCompanyIdJson) &&
                    chipCompanyIdJson->valueint == chipCompanyId);
    }
    cJSON_Delete(jsonVal);
    HILOGI("Test_BtChrUpdateChipInfo_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateChipInfo_002
 * @tc.desc: Verify the BtChrUpdateChipInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateChipInfo_002, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateChipInfo_002 enter");
    std::string addr = "12345";
    int btVersion = 11;
    int btSubVersion = 277;
    int chipCompanyId = 688;
    for (int i = 0; i < MAX_DEVICE_INFO_NUM; i++) {
        string str = std::to_string(i);
        BtChrUpdateChipInfo(str, 0, 0, 0);
    }
    BtChrUpdateChipInfo(addr, btVersion, btSubVersion, chipCompanyId);
    BtChrDeviceInfo deviceInfo;
    EXPECT_EQ(GetDeviceInfoFromMap(addr, deviceInfo), 0);
    HILOGI("Test_BtChrUpdateChipInfo_002 end");
}

/**
 * @tc.name: Test_BtChrAddConnectedDevice_001
 * @tc.desc: Verify the BtChrAddConnectedDevice function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddConnectedDevice_001, TestSize.Level1)
{
    HILOGI("Test_BtChrAddConnectedDevice_001 enter");
    std::string addr = "11:22:33:44:55:66";
    for (int i = 10; i < 22; i++) {
        BtChrAddConnectedDevice(addr.append(std::to_string(i)));
    }
    BtChrDeviceInfo deviceInfo;
    EXPECT_FALSE(GetDeviceInfoFromMap(addr, deviceInfo));
    std::string connectedDeviceJonStr = BtChrGetConnectedDeviceJsonStr();
    HILOGI("Test_BtChrAddConnectedDevice_001 end");
}

/**
 * @tc.name: Test_BtChrDelConnectedDevice_001
 * @tc.desc: Verify the BtChrDelConnectedDevice function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrDelConnectedDevice_001, TestSize.Level1)
{
    HILOGI("Test_BtChrDelConnectedDevice_001 enter");
    std::string addr = "11:22:33:44:55:66";
    for (int i = 10; i < 22; i++) {
        BtChrDelConnectedDevice(addr.append(std::to_string(i)));
    }
    BtChrDeviceInfo deviceInfo;
    EXPECT_FALSE(GetDeviceInfoFromMap(addr, deviceInfo));
    HILOGI("Test_BtChrDelConnectedDevice_001 end");
}

/**
 * @tc.name: Test_BtChrAddAuthResultInfo_001
 * @tc.desc: Verify the BtChrAddAuthResultInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddAuthResultInfo_001, TestSize.Level1)
{
    HILOGI("Test_BtChrAddAuthResultInfo_001 enter");
    std::string addr = "11:22:33:44:55:66";
    BtChrAddAuthResultInfo(addr, 1);
    BtChrDeviceInfo deviceInfo;
    EXPECT_TRUE(GetDeviceInfoFromMap(addr, deviceInfo));
    HILOGI("Test_BtChrAddAuthResultInfo_001 end");
}

/**
 * @tc.name: Test_BtChrAddConnSceneInfo_001
 * @tc.desc: Verify the BtChrAddConnSceneInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddConnSceneInfo_001, TestSize.Level1)
{
    HILOGI("Test_BtChrAddConnSceneInfo_001 enter");
    std::string addr = "11:22:33:44:55:66";
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_PAIR, "remote_device", 0);
    std::string btConnSceneJonStr = BtChrGetConnSceneJsonStr(addr);
    BtChrDeviceInfo deviceInfo;
    EXPECT_TRUE(GetDeviceInfoFromMap(addr, deviceInfo));
    HILOGI("Test_BtChrAddConnSceneInfo_001 end");
}

/**
 * @tc.name: Test_BtChrAddConnSceneInfo_002
 * @tc.desc: Verify the BtChrAddConnSceneInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddConnSceneInfo_002, TestSize.Level1)
{
    HILOGI("Test_BtChrAddConnSceneInfo_002 enter");
    std::string addr = "11:22:33:44:55:66";
    std::string pairApp = "remote_device";
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_PAIR, pairApp, 0);
    std::string btConnSceneJonStr = BtChrGetConnSceneJsonStr(addr);
    EXPECT_TRUE(btConnSceneJonStr.find(pairApp) != std::string::npos);
    HILOGI("Test_BtChrAddConnSceneInfo_002 end");
}

/**
 * @tc.name: Test_BtChrAddCallingPackageName_001
 * @tc.desc: Verify the BtChrAddCallingPackageName function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddCallingPackageName_001, TestSize.Level1)
{
    HILOGI("Test_BtChrAddCallingPackageName_001 enter");
    std::string addr = "11:22:33:44:66:77";
    std::string pairApp = "broker";
    std::string packageName = "test_bluetooth";
    std::string exceptPackageName = "broker:test_bluetooth";
    BtChrAddCallingPackageName(addr, packageName);
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_PAIR, pairApp, 0);
    std::string btConnSceneJonStr = BtChrGetConnSceneJsonStr(addr);
    EXPECT_TRUE(btConnSceneJonStr.find(exceptPackageName) != std::string::npos);

    std::string addr1 = "11:22:33:44:66:88";
    BtChrAddConnSceneInfo(addr1, PAIR_TYPE_USER_PAIR, pairApp, 0);
    BtChrAddCallingPackageName(addr1, packageName);
    btConnSceneJonStr = BtChrGetConnSceneJsonStr(addr1);
    EXPECT_TRUE(btConnSceneJonStr.find(exceptPackageName) != std::string::npos);
    HILOGI("Test_BtChrAddCallingPackageName_001 end");
}

std::string GetPkgNameFromJson(std::string& jonStr)
{
    cJSON* jsonVal = cJSON_Parse(jonStr.c_str());
    if (jsonVal == nullptr) {
        return "";
    }
    cJSON* pairAppJson = cJSON_GetObjectItemCaseSensitive(jsonVal, "PAIRAPP");
    if (pairAppJson == nullptr) {
        return "";
    }
    return pairAppJson->valuestring;
}

/**
 * @tc.name: Test_BtChrAddCallingPackageName_002
 * @tc.desc: Verify the BtChrAddCallingPackageName function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddCallingPackageName_002, TestSize.Level1)
{
    HILOGI("Test_BtChrAddCallingPackageName_002 enter"); // scene1
    std::string addr = "22:00:33:44:66:77";
    std::string pairApp = "broker";
    std::string packageName = "test_bluetooth";
    std::string exceptPackageName = "broker:test_bluetooth";
    BtChrAddCallingPackageName(addr, packageName);
    BtChrAddCallingPackageName(addr, packageName);
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_PAIR, pairApp, 0);
    std::string btConnSceneJonStr = BtChrGetConnSceneJsonStr(addr);
    EXPECT_TRUE(btConnSceneJonStr.find(exceptPackageName) != std::string::npos);

    std::string pkgName = GetPkgNameFromJson(btConnSceneJonStr);
    HILOGI("Test_BtChrAddCallingPackageName pkgName: %{public}s", pkgName.c_str());
    EXPECT_EQ(pkgName, exceptPackageName);
    HILOGI("Test_BtChrAddCallingPackageName_002 end");
}

/**
 * @tc.name: Test_BtChrAddCallingPackageName_003
 * @tc.desc: Verify the BtChrAddCallingPackageName function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddCallingPackageName_003, TestSize.Level1)
{
    HILOGI("Test_BtChrAddCallingPackageName_003 enter");
    std::string addr = "22:11:33:44:66:77";
    std::string pairApp = "broker";
    std::string packageName = "test_bluetooth";
    std::string exceptPackageName = "broker:test_bluetooth";
    BtChrAddCallingPackageName(addr, packageName);
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_PAIR, pairApp, 0);
    BtChrAddCallingPackageName(addr, packageName);
    std::string btConnSceneJonStr = BtChrGetConnSceneJsonStr(addr);
    EXPECT_TRUE(btConnSceneJonStr.find(exceptPackageName) != std::string::npos);

    std::string pkgName = GetPkgNameFromJson(btConnSceneJonStr);
    HILOGI("Test_BtChrAddCallingPackageName pkgName: %{public}s", pkgName.c_str());
    EXPECT_EQ(pkgName, exceptPackageName);
    HILOGI("Test_BtChrAddCallingPackageName_003 end");
}

/**
 * @tc.name: Test_BtChrAddCallingPackageName_004
 * @tc.desc: Verify the BtChrAddCallingPackageName function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddCallingPackageName_004, TestSize.Level1)
{
    HILOGI("Test_BtChrAddCallingPackageName_004 enter");
    std::string addr = "22:22:33:44:66:77";
    std::string pairApp = "broker";
    std::string packageName = "test_bluetooth";
    std::string exceptPackageName = "broker:test_bluetooth";
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_PAIR, pairApp, 0);
    BtChrAddCallingPackageName(addr, packageName);
    BtChrAddCallingPackageName(addr, packageName);
    std::string btConnSceneJonStr = BtChrGetConnSceneJsonStr(addr);
    EXPECT_TRUE(btConnSceneJonStr.find(exceptPackageName) != std::string::npos);

    std::string pkgName = GetPkgNameFromJson(btConnSceneJonStr);
    HILOGI("Test_BtChrAddCallingPackageName pkgName: %{public}s", pkgName.c_str());
    EXPECT_EQ(pkgName, exceptPackageName);
    HILOGI("Test_BtChrAddCallingPackageName_004 end");
}

/**
 * @tc.name: Test_BtChrAddCallingPackageName_005
 * @tc.desc: Verify the BtChrAddCallingPackageName function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrAddCallingPackageName_005, TestSize.Level1)
{
    HILOGI("Test_BtChrAddCallingPackageName_005 enter");
    std::string addr = "22:33:33:44:66:88";
    std::string pairApp = "bluetooth_service";
    std::string pairApp1 = "bluetooth_service_test";
    std::string packageName = "test_bluetooth";
    std::string exceptPackageName = "bluetooth_service";
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_PAIR, pairApp, 0);
    BtChrAddCallingPackageName(addr, packageName);
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_PAIR, pairApp1, 0);
    BtChrAddCallingPackageName(addr, packageName);
    std::string btConnSceneJonStr = BtChrGetConnSceneJsonStr(addr);
    EXPECT_TRUE(btConnSceneJonStr.find(exceptPackageName) != std::string::npos);
    
    std::string pkgName = GetPkgNameFromJson(btConnSceneJonStr);
    HILOGI("Test_BtChrAddCallingPackageName pkgName: %{public}s", pkgName.c_str());
    EXPECT_EQ(pkgName, exceptPackageName);
    HILOGI("Test_BtChrAddCallingPackageName_005 end");
}

/**
 * @tc.name: Test_BtChrUpdateGoodChannelNum_001
 * @tc.desc: Verify the BtChrUpdateGoodChannelNum function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateGoodChannelNum_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateGoodChannelNum_001 enter");
    std::string addr = "11:22:33:44:55:66";
    std::string eventKey = CHR_USER_DISCONNECT + addr;
    std::string emptyChipInfo = "";
    BtChrUpdateGoodChannelNum(addr, 14);
    std::string btChipInfoJonStr = BtChrGetChipInfoJsonStr(addr, eventKey);
    EXPECT_TRUE(btChipInfoJonStr != emptyChipInfo);
    HILOGI("Test_BtChrUpdateGoodChannelNum_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateGoodChannelNum_002
 * @tc.desc: Verify the BtChrUpdateGoodChannelNum function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateGoodChannelNum_002, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateGoodChannelNum_002 enter");
    std::string addr = "11:22:33:44:55:66";
    std::string eventKey = CHR_A2DP_AUDIO_EXCEPTION + addr;
    std::string emptyChipInfo = "";
    BtChrUpdateGoodChannelNum(addr, 14);
    std::string btChipInfoJonStr = BtChrGetChipInfoJsonStr(addr, eventKey);
    EXPECT_TRUE(btChipInfoJonStr != emptyChipInfo);
    BtChrDelAudioChipInfo(addr);
    BtChrClearAudioAntSwitchState();
    btChipInfoJonStr = BtChrGetChipInfoJsonStr(addr, eventKey);
    EXPECT_TRUE(btChipInfoJonStr == emptyChipInfo);
    HILOGI("Test_BtChrUpdateGoodChannelNum_002 end");
}

/**
 * @tc.name: Test_BtChrUpdateAntennaStatus_001
 * @tc.desc: Verify the BtChrUpdateAntennaStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateAntennaStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateAntennaStatus_001 enter");
    std::string addr = "11:22:33:44:55:66";
    std::string eventKey = CHR_A2DP_AUDIO_EXCEPTION + addr;
    std::string emptyChipInfo = "";
    std::string antennaStatus = "{timestampMs:1751515284770,policy:1}";
    BtChrUpdateAntennaStatus(addr, antennaStatus);
    std::string btChipInfoJonStr = BtChrGetChipInfoJsonStr(addr, eventKey);
    EXPECT_TRUE(btChipInfoJonStr != emptyChipInfo);
    BtChrDelAudioChipInfo(addr);
    BtChrClearAudioAntSwitchState();
    btChipInfoJonStr = BtChrGetChipInfoJsonStr(addr, eventKey);
    EXPECT_TRUE(btChipInfoJonStr == emptyChipInfo);
    HILOGI("Test_BtChrUpdateAntennaStatus_001 end");
}


/**
 * @tc.name: Test_BtChrUpdateAntennaStatus_002
 * @tc.desc: Verify the Test_BtChrUpdateAntennaStatus_002 function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateAntennaStatus_002, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateAntennaStatus_002 enter");
    std::string peerAddr = "11:22:33:44:55:66";
    std::string eventKey = CHR_USER_DISCONNECT + peerAddr;
    std::string emptyChipInfo = "";
    std::string antennaStatus = "{timestampMs:1751515284770,policy:1}";
    BtChrUpdateAntennaStatus(peerAddr, antennaStatus);
    BtChrEventWriteStr(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, "EARBUD_INFO", "scence info test");
    BtChrEventWriteInt(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, CHR_SUB_ERRCODE, CHR_SUB_ERRCODE_CASE1);
    BtChrEventWriteInt(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, CHR_ERRCODE, CHR_A2DP_EARBUD_EXCEPTION);
    std::string btChipInfoJonStr = BtChrGetChipInfoJsonStr(peerAddr, eventKey);
    EXPECT_TRUE(btChipInfoJonStr != emptyChipInfo);
    HILOGI("Test_BtChrUpdateAntennaStatus_002 end");
}

/**
 * @tc.name: Test_BtChrUpdateAntennaSwitchStatus_001
 * @tc.desc: Verify the BtChrUpdateAntennaSwitchStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateAntennaSwitchStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateAntennaSwitchStatus_001 enter");
    std::string peerAddr = "11:22:33:44:55:66";
    std::string eventKey = CHR_USER_DISCONNECT + peerAddr;
    std::string emptyChipInfo = "";
    const uint32_t antNum = 3;
    uint8_t antGroupIdx[antNum] = {0, 1, 0xFF}, configState[antNum] = {1, 1, 0xFF};
    BtChrUpdateAntennaSwitchStatus(antGroupIdx, configState, antNum);
    std::string btChipInfoJonStr = BtChrGetChipInfoJsonStr(peerAddr, eventKey);
    EXPECT_TRUE(btChipInfoJonStr != emptyChipInfo);
    cJSON* root = cJSON_Parse(btChipInfoJonStr.c_str());
    EXPECT_TRUE(root != nullptr);
    cJSON* antSwitchJson = cJSON_GetObjectItem(root, "ANTENNASWITCHSTATE");
    EXPECT_TRUE(antSwitchJson != nullptr);
    char* antSwitchStr = cJSON_PrintUnformatted(antSwitchJson);
    std::string antSwitch(antSwitchStr);
    EXPECT_TRUE(!antSwitch.empty());
    HILOGI("Test_BtChrUpdateAntennaSwitchStatus_001 end");
}

/**
 * @tc.name: Test_BtChrUpdateAudioAntennaSwitchStatus_001
 * @tc.desc: Verify the BtChrUpdateAudioAntennaSwitchStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(BtChrSubeventManagerTest, Test_BtChrUpdateAudioAntennaSwitchStatus_001, TestSize.Level1)
{
    HILOGI("Test_BtChrUpdateAudioAntennaSwitchStatus_001 enter");
    std::string peerAddr = "11:22:33:44:55:66";
    std::string eventKey = CHR_A2DP_AUDIO_EXCEPTION + peerAddr;
    std::string emptyChipInfo = "";
    const uint32_t antNum = 3;
    uint16_t antGroupIdx[antNum] = {0, 1, 0xFF}, configState[antNum] = {1, 1, 0xFF};
    BtChrUpdateAudioAntennaSwitchStatus(antGroupIdx, configState, antNum);
    std::string btChipInfoJonStr = BtChrGetChipInfoJsonStr(peerAddr, eventKey);
    EXPECT_TRUE(btChipInfoJonStr != emptyChipInfo);
    cJSON* root = cJSON_Parse(btChipInfoJonStr.c_str());
    EXPECT_TRUE(root != nullptr);
    cJSON* antSwitchJson = cJSON_GetObjectItem(root, "ANTENNASWITCHSTATE");
    EXPECT_TRUE(antSwitchJson != nullptr);
    char* antSwitchStr = cJSON_PrintUnformatted(antSwitchJson);
    std::string antSwitch(antSwitchStr);
    EXPECT_TRUE(!antSwitch.empty());
    HILOGI("Test_BtChrUpdateAudioAntennaSwitchStatus_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS
