/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <cstring>
#include "classic_adapter.h"
#include "power_device.h"
#include "class_creator.h"
#include "adapter_config.h"
#include "classic_config.h"
#include "bluetooth_host.h"
#include "adapter_device_config.h"
#include "gmock/gmock.h"
#include "bluetooth_access_token_mock.h"

using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;
using ::testing::Matcher;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;

class MockIAdapterDeviceConfig : public IAdapterDeviceConfig {
public:
    MOCK_METHOD0(Load, bool());
    MOCK_METHOD0(Reload, bool());
    MOCK_METHOD0(Save, bool());
    MOCK_METHOD4(GetValue, bool(const std::string &section, const std::string &subSection,
        const std::string &property, int &value));
    MOCK_METHOD4(GetValue, bool(const std::string &section, const std::string &subSection,
        const std::string &property, std::string &value));
    MOCK_METHOD4(GetValue, bool(const std::string &section, const std::string &subSection,
        const std::string &property, bool &value));
    MOCK_METHOD5(SetValue, bool(const std::string &section, const std::string &subSection,
        const std::string &property, const int &value, bool isAutoSave));
    MOCK_METHOD5(SetValue, bool(const std::string &section, const std::string &subSection,
        const std::string &property, const std::string &value, bool isAutoSave));
    MOCK_METHOD5(SetValue, bool(const std::string &section, const std::string &subSection,
        const std::string &property, const bool &value, bool isAutoSave));
    MOCK_METHOD2(GetSubSections, bool(const std::string &section, std::vector<std::string> &subSections));
    MOCK_METHOD3(RemoveSection, bool(const std::string &section, const std::string &subSection, bool isAutoSave));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, int &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, std::string &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, bool &value));
    MOCK_METHOD4(SetValue, bool(const std::string &section, const std::string &property, const int &value,
        bool isAutoSave));
    MOCK_METHOD4(SetValue, bool(const std::string &section, const std::string &property, const std::string &value,
        bool isAutoSave));
};

ClassicConfig classicConfig;
class ClassicConfigTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        BluetoothAccessTokenMock::SetNativeTokenInfo();
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBleEnable = host_->IsBleEnabled();
        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBleEnable = %{public}d, isBrEnable = %{public}d", isBleEnable, isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }
    }
    static void TearDownTestCase()
    {
        host_ = nullptr;
    }
    void SetUp()
    {}
    void TearDown()
    {
        classicConfig.config_ = nullptr;
    }
};


/**
 * @tc.name: classic_config_test_001
 * @tc.desc: ClassicConfigTest:LoadConfigFile
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_001, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_001 enter");
    EXPECT_CALL(mockIAdapterDeviceConfig, Load())
    .WillOnce(Return(false));
    classicConfig.LoadConfigFile();
    EXPECT_CALL(mockIAdapterDeviceConfig, Load())
    .WillOnce(Return(true));
    classicConfig.LoadConfigFile();
    HILOGI("classic_config_test_001 end");
}

/**
 * @tc.name: classic_config_test_002
 * @tc.desc: ClassicConfigTest:Save
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_002, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_002 enter");
    EXPECT_CALL(mockIAdapterDeviceConfig, Save())
    .WillRepeatedly(Return(false));
    classicConfig.Save();
    EXPECT_CALL(mockIAdapterDeviceConfig, Save())
    .WillRepeatedly(Return(true));
    classicConfig.Save();
    HILOGI("classic_config_test_002 end");
}


/**
 * @tc.name: classic_config_test_003
 * @tc.desc: ClassicConfigTest:SetLocalDeviceClass
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_003, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_003 enter");
    int cod = 0;
    EXPECT_CALL(mockIAdapterDeviceConfig, SetValue(SECTION_HOST, PROPERTY_CLASS_OF_DEVICE, cod, true))
    .WillOnce(Return(false));
    classicConfig.SetLocalDeviceClass(cod);
    EXPECT_CALL(mockIAdapterDeviceConfig, SetValue(SECTION_HOST, PROPERTY_CLASS_OF_DEVICE, cod, true))
    .WillOnce(Return(true));
    classicConfig.SetLocalDeviceClass(cod);
    classicConfig.GetLocalDeviceClass();
    HILOGI("classic_config_test_003 end");
}

/**
 * @tc.name: classic_config_test_004
 * @tc.desc: ClassicConfigTest:GetIoCapability
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_004, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_004 enter");
    int io = 0;
    EXPECT_CALL(mockIAdapterDeviceConfig, GetValue(SECTION_HOST, PROPERTY_IO_CAPABILITY, io))
    .WillOnce(Return(false));
    classicConfig.GetIoCapability();
    EXPECT_CALL(mockIAdapterDeviceConfig, GetValue(SECTION_HOST, PROPERTY_IO_CAPABILITY, io))
    .WillOnce(Return(true));
    classicConfig.GetIoCapability();
    HILOGI("classic_config_test_004 end");
}

/**
 * @tc.name: classic_config_test_005
 * @tc.desc: ClassicConfigTest:GetPairedAddrList
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_005, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_005 enter");
    std::vector<std::string> pairedList;
    EXPECT_CALL(mockIAdapterDeviceConfig, GetSubSections(SECTION_BREDR_PAIRED_LIST, pairedList))
    .WillOnce(Return(false));
    classicConfig.GetPairedAddrList();
    EXPECT_CALL(mockIAdapterDeviceConfig, GetSubSections(SECTION_BREDR_PAIRED_LIST, pairedList))
    .WillOnce(Return(true));
    classicConfig.GetPairedAddrList();
    HILOGI("classic_config_test_005 end");
}

/**
 * @tc.name: classic_config_test_006
 * @tc.desc: ClassicConfigTest:RemovePairedDevice
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_006, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_006 enter");
    std::string subSection = "devicename";
    EXPECT_CALL(mockIAdapterDeviceConfig, RemoveSection(SECTION_BREDR_PAIRED_LIST, subSection, true))
    .Times(2).WillOnce(Return(false));
    classicConfig.RemovePairedDevice(subSection);
    EXPECT_CALL(mockIAdapterDeviceConfig, RemoveSection(SECTION_BREDR_PAIRED_LIST, subSection, true))
    .Times(2).WillOnce(Return(true));
    classicConfig.RemovePairedDevice(subSection);
    HILOGI("classic_config_test_006 end");
}

/**
 * @tc.name: classic_config_test_007
 * @tc.desc: ClassicConfigTest:SetRemoteRandomAddr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_007, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_007 enter");
    std::string subSection = "devicename";
    std::string randomAddr = "00:11:22:33:44:55";
    EXPECT_CALL(mockIAdapterDeviceConfig, SetValue("Classic Paired Device List", subSection,
        "RandomAddr", randomAddr, true))
    .WillRepeatedly(Return(false));
    classicConfig.SetRemoteRandomAddr(subSection, randomAddr);
    EXPECT_CALL(mockIAdapterDeviceConfig, SetValue("Classic Paired Device List", subSection,
        "RandomAddr", randomAddr, true))
    .WillRepeatedly(Return(true));
    classicConfig.SetRemoteRandomAddr(subSection, randomAddr);
    HILOGI("classic_config_test_007 end");
}

/**
 * @tc.name: classic_config_test_008
 * @tc.desc: ClassicConfigTest:GetRemoteRandomAddr
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_008, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_008 enter");
    std::string subSection = "devicename";
    std::string randomAddr = "";
    EXPECT_CALL(mockIAdapterDeviceConfig, GetValue("Classic Paired Device List", subSection,
        "RandomAddr", randomAddr))
    .Times(2).WillOnce(Return(false));
    classicConfig.GetRemoteRandomAddr(subSection);
    EXPECT_CALL(mockIAdapterDeviceConfig, GetValue("Classic Paired Device List", subSection,
        "RandomAddr", randomAddr))
    .WillOnce(Return(true));
    classicConfig.GetRemoteRandomAddr(subSection);
    HILOGI("classic_config_test_008 end");
}

/**
 * @tc.name: classic_config_test_009
 * @tc.desc: ClassicConfigTest:SetRemoteRandomAddrNoSave
 * @tc.type: FUNC
 */
HWTEST_F(ClassicConfigTest, classic_config_test_009, TestSize.Level1)
{
    MockIAdapterDeviceConfig mockIAdapterDeviceConfig;
    classicConfig.config_ = &mockIAdapterDeviceConfig;

    HILOGI("classic_config_test_009 enter");
    std::string randomAddr = "00:00:00:00:00:00";
    std::string subSection = "devicename";
    EXPECT_CALL(mockIAdapterDeviceConfig, SetValue(SECTION_BREDR_PAIRED_LIST, subSection,
        PROPERTY_RANDOM_ADDR, randomAddr, false))
    .WillRepeatedly(Return(false));
    EXPECT_FALSE(classicConfig.SetRemoteRandomAddrNoSave(subSection, randomAddr));
    EXPECT_CALL(mockIAdapterDeviceConfig, SetValue(SECTION_BREDR_PAIRED_LIST, subSection,
        PROPERTY_RANDOM_ADDR, randomAddr, false))
    .WillRepeatedly(Return(true));
    EXPECT_TRUE(classicConfig.SetRemoteRandomAddrNoSave(subSection, randomAddr));
    HILOGI("classic_config_test_009 end");
}
}  // namespace bluetooth
}  // namespace OHOS