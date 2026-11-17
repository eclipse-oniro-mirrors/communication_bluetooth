/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "bluetooth_common_event_subscriber.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
static const char *const BLUETOOTH_WHITELIST_CHANGED_EVENT = "com.ohos.edm.bluetoothdeviceschanged";
static const char *const BLUETOOTH_DENYLIST_CHANGED_EVENT = "com.ohos.edm.disallowedbluetoothdeviceschanged";
static const char *const COMMON_EVENT_DATASHARE_READY = "usual.event.DATA_SHARE_READY";

class BluetoothCommonEventSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();
        sleep(TIME);

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
    {}
};

/*
 * @tc.number: BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_002
 * @tc.name: OnReceiveEvent
 * @tc.desc: OnReceiveEvent COMMON_EVENT_DATASHARE_READY
*/
HWTEST_F(
    BluetoothCommonEventSubscriberTest, BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_002, TestSize.Level1)
{
    HILOGI("BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEventm_002 start");
    EventFwk::CommonEventData eventData;
    std::string action = "EventFwk::CommonEventSupport::COMMON_EVENT_DATASHARE_READY";
    AAFwk::Want want;
    want.SetAction(action);
    eventData.SetWant(want);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo;
    std::shared_ptr<BluetoothCommonEventSubscriber> Subscriber =
        std::make_shared<BluetoothCommonEventSubscriber>(subscriberInfo);
    ASSERT_NE(nullptr, Subscriber);
    Subscriber->OnReceiveEvent(eventData);
    HILOGI("BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEventm_002 end");
}

/*
 * @tc.number: BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_003
 * @tc.name: OnReceiveEvent
 * @tc.desc: OnReceiveEvent BLUETOOTH_WHITELIST_CHANGED_EVENT
*/
HWTEST_F(
    BluetoothCommonEventSubscriberTest, BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_003, TestSize.Level1)
{
    HILOGI("BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEventm_003 start");
    EventFwk::CommonEventData eventData;
    std::string action = "EventFwk::CommonEventSupport::BLUETOOTH_WHITELIST_CHANGED_EVENT";
    AAFwk::Want want;
    want.SetAction(action);
    eventData.SetWant(want);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo;
    std::shared_ptr<BluetoothCommonEventSubscriber> Subscriber =
        std::make_shared<BluetoothCommonEventSubscriber>(subscriberInfo);
    ASSERT_NE(nullptr, Subscriber);
    Subscriber->OnReceiveEvent(eventData);
    HILOGI("BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEventm_003 end");
}

/*
 * @tc.number: BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_004
 * @tc.name: OnReceiveEvent
 * @tc.desc: OnReceiveEvent COMMON_EVENT_POWER_MANAGER_STATE_CHANGED
*/
HWTEST_F(
    BluetoothCommonEventSubscriberTest, BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_004, TestSize.Level1)
{
    HILOGI("BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEventm_004 start");
    EventFwk::CommonEventData eventData;
    std::string action = "EventFwk::CommonEventSupport::COMMON_EVENT_POWER_MANAGER_STATE_CHANGED";
    AAFwk::Want want;
    want.SetAction(action);
    eventData.SetWant(want);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo;
    std::shared_ptr<BluetoothCommonEventSubscriber> Subscriber =
        std::make_shared<BluetoothCommonEventSubscriber>(subscriberInfo);
    ASSERT_NE(nullptr, Subscriber);
    Subscriber->OnReceiveEvent(eventData);
    HILOGI("BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEventm_004 end");
}

/*
 * @tc.number: BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_005
 * @tc.name: OnReceiveEvent
 * @tc.desc: OnReceiveEvent BLUETOOTH_DENYLIST_CHANGED_EVENT
*/
HWTEST_F(
    BluetoothCommonEventSubscriberTest, BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_005, TestSize.Level1)
{
    HILOGI("BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_005 start");
    EventFwk::CommonEventData eventData;
    std::string action = "EventFwk::CommonEventSupport::BLUETOOTH_DENYLIST_CHANGED_EVENT";
    AAFwk::Want want;
    want.SetAction(action);
    eventData.SetWant(want);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo;
    std::shared_ptr<BluetoothCommonEventSubscriber> Subscriber =
        std::make_shared<BluetoothCommonEventSubscriber>(subscriberInfo);
    ASSERT_NE(nullptr, Subscriber);
    Subscriber->OnReceiveEvent(eventData);
    HILOGI("BluetoothCommonEventSubscriberTest_UnitTest_OnReceiveEvent_005 end");
}
}  // namespace bluetooth
}  // namespace OHOS