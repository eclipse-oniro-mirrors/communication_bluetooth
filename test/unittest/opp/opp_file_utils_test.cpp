/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "bluetooth_opp.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "opp_obex_client.h"
#include "opp_def.h"
#include "obex_def.h"
#include "opp_service_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
using namespace std;
class OppFileUtilsTest : public testing::Test {
public:
    OppFileUtilsTest();
    ~OppFileUtilsTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
};

OppFileUtilsTest::OppFileUtilsTest()
{
}

OppFileUtilsTest::~OppFileUtilsTest()
{
}

void OppFileUtilsTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    HILOGI("SetUpTestCase end");
}

void OppFileUtilsTest::TearDownTestCase(void)
{
}

void OppFileUtilsTest::SetUp()
{
}

void OppFileUtilsTest::TearDown()
{
}

/*
 * @tc.number: OPP_ModuleTest_StartUIAbilityByCall_00100
 * @tc.name: StartUIAbilityByCall
 * @tc.desc:
 */
HWTEST_F(OppFileUtilsTest, OPP_ModuleTest_StartUIAbilityByCall_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_StartUIAbilityByCall_00100 start");
    AAFwk::Want abilityWant;
    std::string bundleName = "com.ohos.bluetooth";
    std::string abilityName = "BluetoothReceiveUIAbility";
    abilityWant.SetElementName(bundleName, abilityName);
 
    AAFwk::Want eventWant;
    std::string eventAction = "usual.event.bluetooth.OPP.RECEIVE";
    std::string key = "fileName";
    std::string value = "test.jpg";
    eventWant.SetAction(eventAction);
    eventWant.SetParam(key, value);

    OppAbilityManagerUtils::SubscribOppReceiveEvent();
    OppAbilityManagerUtils::StartUIAbilityByCall(abilityWant, eventWant);
    OppAbilityManagerUtils::WakeUpOppSendUIAbility(key);
    OppAbilityManagerUtils::SubscribTransactionChrEvent();

    auto oppContinuousTaskFunc = [](const std::string &action) -> void {
        HILOGI("ContinuousTaskEvent receive event. %{public}s", action.c_str());
    };
    OppAbilityManagerUtils::SetOppContinuousTaskFunc(oppContinuousTaskFunc);

    auto aclDisConnEventFunc = [](const std::string &addr) -> void {
        HILOGI("ContinuousTaskEvent aclDisConnEventFunc");
    };
    OppAbilityManagerUtils::SetAclDisConnEventFunc(aclDisConnEventFunc);

    EXPECT_EQ(bundleName, "com.ohos.bluetooth");
    HILOGI("OPP_ModuleTest_StartUIAbilityByCall_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_StartUIExtentionAbility_00100
 * @tc.name: StartUIExtentionAbility
 * @tc.desc:
 */
HWTEST_F(OppFileUtilsTest, OPP_ModuleTest_StartUIExtentionAbility_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_StartUIExtentionAbility_00100 start");
    std::string bundleName = "com.ohos.bluetooth";
    std::string abilityName = "BluetoothReciveUIAbility";
    std::map<std::string, std::string> params;
    params.emplace("fileUriType", "files");
    params.emplace("bundleName", bundleName);
    params.emplace("abilityName", abilityName);
    params.emplace("focusState", "0");
    OppAbilityManagerUtils::StartUIExtentionAbility(params);
    EXPECT_EQ(bundleName, "com.ohos.bluetooth");
    HILOGI("OPP_ModuleTest_StartUIExtentionAbility_00100 end");
}

/*
 * @tc.number: OPP_ModuleTest_OnReceiveEvent_00100
 * @tc.name: OnReceiveEvent
 * @tc.desc:
 */
HWTEST_F(OppFileUtilsTest, OPP_ModuleTest_OnReceiveEvent_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_OnReceiveEvent_00100 start");
    EventFwk::CommonEventData eventData;
    std::string action = "ohos.event.notification.BT.TAP_ACCEPT";
    AAFwk::Want want;
    want.SetAction(action);
    eventData.SetWant(want);
    OHOS::EventFwk::CommonEventSubscribeInfo eventSubscribeInfo;
    std::shared_ptr<OppEventSubscriber> subscriber = std::make_shared<OppEventSubscriber>(eventSubscribeInfo);
    ASSERT_NE(nullptr, subscriber);
    subscriber->OnReceiveEvent(eventData);
    HILOGI("OPP_ModuleTest_OnReceiveEvent_00100 end");
}


/*
 * @tc.number: OPP_ModuleTest_GetFileNameFromFilePath_00100
 * @tc.name: GetFileNameFromFilePath
 * @tc.desc:
 */
HWTEST_F(OppFileUtilsTest, OPP_ModuleTest_GetFileNameFromFilePath_00100, TestSize.Level1)
{
    HILOGI("OPP_ModuleTest_GetFileNameFromFilePath_00100 start");
    EventFwk::CommonEventData eventData;
    std::string filePath = "6.0abc.txt";
    std::string fileName = "";
    fileName = GetFileNameFromFilePath(filePath, true);
    EXPECT_TRUE(fileName.find(".txt") != std::string::npos);
    filePath = "6.0abc.txt.dlp";
    fileName = GetFileNameFromFilePath(filePath, true);
    EXPECT_TRUE(fileName.find(".txt.dlp") != std::string::npos);
    filePath = "6.0abc.txt.pdf";
    fileName = GetFileNameFromFilePath(filePath, true);
    EXPECT_TRUE(fileName.find(".txt.pdf") == std::string::npos);
    filePath = "abc.dlp";
    fileName = GetFileNameFromFilePath(filePath, true);
    EXPECT_TRUE(fileName.find(".dlp") != std::string::npos);

    filePath = "";
    for (size_t index = 0; index < MAX_FILE_NAME_LENGTH; index++) {
        filePath += "1";
    }
    filePath += ".txt";
    fileName = GetFileNameFromFilePath(filePath, true);
    EXPECT_EQ(fileName.length(), MAX_FILE_NAME_LENGTH);
    HILOGI("OPP_ModuleTest_GetFileNameFromFilePath_00100 end");
}
}  // namespace Bluetooth
}  // namespace OHOS