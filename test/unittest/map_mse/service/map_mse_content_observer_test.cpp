
/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <vector>
#include <cstdint>
#include <optional>
#include "map_mse_content_observer.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "../../map_mse/service/data_share_helper_impl_test.h"
#include "map_mse_service_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
using Message = MapMseContentObserver::Message;

constexpr int32_t TIME = 4;
static BluetoothHost *host_;

class MapMseContentObserverTest : public testing::Test {
public:
    MapMseContentObserverTest() {}
    ~MapMseContentObserverTest() {}
    MapMnsSdpFoundRecord record;
    MapMseServiceImpl mapMseServiceImpl;
    std::shared_ptr<MapMseMnsClient> mnsClient = nullptr;
    std::shared_ptr<MapMseContentObserver> mapMseContentObserver = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MapMseContentObserverTest::SetUpTestCase(void)
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
}

void MapMseContentObserverTest::TearDownTestCase(void)
{
    utility::CloseAllTimer();
}

void MapMseContentObserverTest::SetUp()
{
    mnsClient = std::make_shared<MapMseMnsClient>(record, &mapMseServiceImpl);
    auto device = std::make_shared<ObexSocketDevice>(-1, 0, 0, SocketType::TYPE_RFCOMM, "00:00:00:00:00:00");
    std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(device);
    mnsClient->clientSession_ = std::make_shared<ObexClientSession>(transport, nullptr);
    mapMseContentObserver = std::make_shared<MapMseContentObserver>(mnsClient);
    mapMseContentObserver->smsDataShareHelper_ = std::make_shared<DataShareHelperImplTest>();
}

void MapMseContentObserverTest::TearDown() {
    if (mapMseContentObserver) {
        mapMseContentObserver->UnSubscribeSmsEvent();
    }
    mapMseContentObserver = nullptr;
    mnsClient = nullptr;
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_UnitTest_SetNotificationRegistration_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_UnitTest_SetNotificationRegistration_001 start");
    int32_t masId = 0;
    std::shared_ptr<MapMseAppParams> params = std::make_shared<MapMseAppParams>();
    int32_t res = mapMseContentObserver->SetNotificationRegistration(masId, params);
    EXPECT_EQ(res, 0xC0);
    params->notificationStatus_ = 0;
    res = mapMseContentObserver->SetNotificationRegistration(masId, params);
    EXPECT_EQ(res, 0xA0);
    HILOGI("MapMseContentObserverTest_UnitTest_SetNotificationRegistration_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_UnitTest_SetMessageStatus_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_UnitTest_SetMessageStatus_001 start");
    std::string handle = "test";
    int32_t masId = 0;
    std::shared_ptr<MapMseAppParams> params = std::make_shared<MapMseAppParams>();
    int32_t res = mapMseContentObserver->SetMessageStatus(handle, params);
    EXPECT_EQ(res, 0xCC);
    params->statusIndicator_ = 0;
    res = mapMseContentObserver->SetNotificationRegistration(masId, params);
    EXPECT_EQ(res, 0xC0);
    params->statusValue_ = 0;
    res = mapMseContentObserver->SetNotificationRegistration(masId, params);
    EXPECT_EQ(res, 0xC0);
    params->statusIndicator_ = 1;
    res = mapMseContentObserver->SetNotificationRegistration(masId, params);
    EXPECT_EQ(res, 0xC0);
    params->statusIndicator_ = 2;
    res = mapMseContentObserver->SetNotificationRegistration(masId, params);
    EXPECT_EQ(res, 0xC0);
    HILOGI("MapMseContentObserverTest_UnitTest_SetMessageStatus_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_UnitTest_SetMessageStatusRead_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_UnitTest_SetMessageStatusRead_001 start");
    int64_t handle = 1;
    int32_t statusValue = 1;
    bool res = mapMseContentObserver->SetMessageStatusRead(handle, statusValue);
    EXPECT_TRUE(res);
    mapMseContentObserver->smsDataShareHelper_ = nullptr;
    res = mapMseContentObserver->SetMessageStatusRead(handle, statusValue);
    EXPECT_FALSE(res);
    HILOGI("MapMseContentObserverTest_UnitTest_SetMessageStatusRead_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_UnitTest_HandleSmsMessageListChanged_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_UnitTest_HandleSmsMessageListChanged_001 start");
    mapMseContentObserver->HandleSmsMessageListChanged();
    HILOGI("MapMseContentObserverTest_UnitTest_HandleSmsMessageListChanged_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_UnitTest_UnSubscribeSmsEvent_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_UnitTest_UnSubscribeSmsEvent_001 start");
    mapMseContentObserver->UnSubscribeSmsEvent();
    HILOGI("MapMseContentObserverTest_UnitTest_UnSubscribeSmsEvent_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_HandleListChangeForNewMessage_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_HandleListChangeForNewMessage_001 start");
    std::map<int64_t, Message> currMessageList;
    int64_t handle = 12;
    int64_t msgState = 1; // Sending
    bool sent = false;
    bool read = true;
    Message msg{handle, msgState, sent, read};
    mapMseContentObserver->HandleListChangeForNewMessage(currMessageList, msg);

    msg.sent = true;
    mapMseContentObserver->smsMessageSentHandle_ = handle;
    mapMseContentObserver->HandleListChangeForNewMessage(currMessageList, msg);

    msg.msgState = 0;
    mapMseContentObserver->HandleListChangeForNewMessage(currMessageList, msg);

    mapMseContentObserver->smsMessageSentHandle_ = 0;
    msg.msgState = 1;
    mapMseContentObserver->HandleListChangeForNewMessage(currMessageList, msg);

    msg.msgState = 0;
    mapMseContentObserver->HandleListChangeForNewMessage(currMessageList, msg);
    HILOGI("MapMseContentObserverTest_HandleListChangeForNewMessage_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_HandleListChangeForExistMessage_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_HandleListChangeForExistMessage_001 start");
    std::map<int64_t, Message> currMessageList;
    int64_t handle = 12;
    int64_t msgState = 1; // Sending
    bool sent = false;
    bool read = true;
    Message msg{handle, msgState, sent, read};
    mapMseContentObserver->smsMessageCollection_[msg.handle] = msg;

    msg.msgState = 0;
    mapMseContentObserver->HandleListChangeForExistMessage(currMessageList, msg);

    msg.msgState = 2;
    mapMseContentObserver->HandleListChangeForExistMessage(currMessageList, msg);

    msg.msgState = 1;
    mapMseContentObserver->HandleListChangeForExistMessage(currMessageList, msg);

    msg.readFlag = false;
    mapMseContentObserver->HandleListChangeForExistMessage(currMessageList, msg);
    HILOGI("MapMseContentObserverTest_HandleListChangeForExistMessage_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_HandleListChangeForExistMessage_002, TestSize.Level1)
{
    std::map<int64_t, Message> currMessageList;
    Message msg{12, 1, false, true, true};  // soft deleted
    mapMseContentObserver->HandleListChangeForExistMessage(currMessageList, msg);
    EXPECT_TRUE(currMessageList.empty());
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_GetSendMessageHandle_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_GetSendMessageHandle_001 start");
    int res = mapMseContentObserver->GetSendMessageHandle();
    std::shared_ptr<MapMseContentObserver::MapDataShareObserver> mapDataShareObserver =
        std::make_shared<MapMseContentObserver::MapDataShareObserver>(*mapMseContentObserver);
    DataShareObserver::ChangeInfo *changeInfo = new DataShareObserver::ChangeInfo();
    mapDataShareObserver->OnChange(*changeInfo);
    EXPECT_EQ(res, -1);
    HILOGI("MapMseContentObserverTest_GetSendMessageHandle_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_HandleListChangeForDeleteMessage_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_HandleListChangeForDeleteMessage_001 start");
    // newMsgList.size() == smsMessageCollection_.size()
    std::map<int64_t, Message> currMessageList;
    mapMseContentObserver->smsMessageCollection_.clear();
    mapMseContentObserver->HandleListChangeForDeleteMessage(currMessageList);

    // newMsgList.count(msg.first)
    int64_t handle = 12;
    int64_t msgState = 0;
    bool sent = false;
    bool read = true;
    Message msg{handle, msgState, sent, read};
    currMessageList[msg.handle] = msg;
    mapMseContentObserver->smsMessageCollection_[msg.handle] = msg;
    mapMseContentObserver->HandleListChangeForDeleteMessage(currMessageList);

    // !newMsgList.count(msg.first)
    currMessageList.clear();
    mapMseContentObserver->HandleListChangeForDeleteMessage(currMessageList);
    HILOGI("MapMseContentObserverTest_HandleListChangeForDeleteMessage_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_GetMessageEventFolder_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_GetMessageEventFolder_001 start");
    const std::string MAP_EVENT_FOLDER_INBOX = "telecom/msg/inbox";
    const std::string MAP_EVENT_FOLDER_OUTBOX = "telecom/msg/outbox";
    const std::string MAP_EVENT_FOLDER_SENT = "telecom/msg/sent";
    const std::string MAP_EVENT_FOLDER_DRAFT = "telecom/msg/draft";
    int64_t handle = 12;
    int64_t msgState = 0;
    bool sent = false;
    bool read = true;
    Message msg{handle, msgState, sent, read};
    std::string folder = mapMseContentObserver->GetMessageEventFolder(msg);
    EXPECT_EQ(folder, MAP_EVENT_FOLDER_INBOX);

    msg.sent = true;
    msg.msgState = 3;
    folder = mapMseContentObserver->GetMessageEventFolder(msg);
    EXPECT_EQ(folder, MAP_EVENT_FOLDER_DRAFT);

    msg.msgState = 0;
    folder = mapMseContentObserver->GetMessageEventFolder(msg);
    EXPECT_EQ(folder, MAP_EVENT_FOLDER_SENT);
    HILOGI("MapMseContentObserverTest_GetMessageEventFolder_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_SendEvent_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_SendEvent_001 start");
    MapMseContentObserver::MapEvent event;
    mnsClient->connected_ = false;
    mapMseContentObserver->SendEvent(event);

    mnsClient->connected_ = true;
    mapMseContentObserver->SendEvent(event);
    HILOGI("MapMseContentObserverTest_SendEvent_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_HandleSmsSendResult_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_HandleSmsSendResult_001 start");
    mapMseContentObserver->HandleSmsSendResult(ISendShortMessageCallback::SEND_SMS_SUCCESS);

    mapMseContentObserver->HandleSmsSendResult(ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
    HILOGI("MapMseContentObserverTest_HandleSmsSendResult_001 end");
}

HWTEST_F(MapMseContentObserverTest, MapMseContentObserverTest_PublishRefreshSmsListEvent_001, TestSize.Level1)
{
    HILOGI("MapMseContentObserverTest_PublishRefreshSmsListEvent_001 start");
    bool res = mapMseContentObserver->PublishRefreshSmsListEvent();
    EXPECT_TRUE(res);
    HILOGI("MapMseContentObserverTest_PublishRefreshSmsListEvent_001 end");
}

}  // namespace bluetooth
}  // namespace OHOS