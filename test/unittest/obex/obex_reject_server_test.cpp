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
#include <gmock/gmock.h>
#include <cstring>
#include "bluetooth_log.h"
#include "obex_def.h"
#include "obex_header.h"
#include "obex_reject_server.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
ObexRejectServer rejectServer;

class ObexRejectServerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {
    }
};

/*
 * @tc.number: ObexRejectServer_OnGet_001
 * @tc.name: OnGet
 * @tc.desc:
 */
HWTEST_F(ObexRejectServerTest, ObexRejectServer_OnGet_001, TestSize.Level1)
{
    HILOGI("ObexRejectServer_OnGet_001 start");
    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> output;
    bool sendBodyHeader;
    int res = rejectServer.OnGet(request, reply, output, sendBodyHeader);
    EXPECT_EQ(res, OBEX_HTTP_UNAVAILABLE);
    HILOGI("ObexRejectServer_OnGet_001 end");
}

/*
 * @tc.number: ObexRejectServer_OnPut_001
 * @tc.name: OnPut
 * @tc.desc:
 */
HWTEST_F(ObexRejectServerTest, ObexRejectServer_OnPut_001, TestSize.Level1)
{
    HILOGI("ObexRejectServer_OnPut_001 start");
    ObexHeader request;
    ObexHeader reply;
    std::vector<uint8_t> output;
    bool requestFinished;
    int res = rejectServer.OnPut(request, reply, output, requestFinished);
    EXPECT_EQ(res, OBEX_HTTP_UNAVAILABLE);
    HILOGI("ObexRejectServer_OnPut_001 end");
}

/*
 * @tc.number: ObexRejectServer_OnConnect_001
 * @tc.name: OnConnect
 * @tc.desc:
 */
HWTEST_F(ObexRejectServerTest, ObexRejectServer_OnConnect_001, TestSize.Level1)
{
    HILOGI("ObexRejectServer_OnConnect_001 start");
    ObexHeader request;
    ObexHeader reply;
    int res = rejectServer.OnConnect(request, reply);
    EXPECT_EQ(res, OBEX_HTTP_UNAVAILABLE);
    HILOGI("ObexRejectServer_OnConnect_001 end");
}

/*
 * @tc.number: ObexRejectServer_OnDisconnect_001
 * @tc.name: OnDisconnect
 * @tc.desc:
 */
HWTEST_F(ObexRejectServerTest, ObexRejectServer_OnDisconnect_001, TestSize.Level1)
{
    HILOGI("ObexRejectServer_OnDisconnect_001 start");
    ObexHeader request;
    ObexHeader reply;
    int res = rejectServer.OnConnect(request, reply);
    EXPECT_EQ(res, OBEX_HTTP_UNAVAILABLE);
    HILOGI("ObexRejectServer_OnDisconnect_001 end");
}

/*
 * @tc.number: ObexRejectServer_OnSetPath_001
 * @tc.name: OnSetPath
 * @tc.desc:
 */
HWTEST_F(ObexRejectServerTest, ObexRejectServer_OnSetPath_001, TestSize.Level1)
{
    HILOGI("ObexRejectServer_OnSetPath_001 start");
    ObexHeader request;
    ObexHeader reply;
    int res = rejectServer.OnSetPath(request, reply, false, false);
    EXPECT_EQ(res, OBEX_HTTP_UNAVAILABLE);
    HILOGI("ObexRejectServer_OnSetPath_001 end");
}

/*
 * @tc.number: ObexRejectServer_OnDelete_001
 * @tc.name: OnDelete
 * @tc.desc:
 */
HWTEST_F(ObexRejectServerTest, ObexRejectServer_OnDelete_001, TestSize.Level1)
{
    HILOGI("ObexRejectServer_OnDelete_001 start");
    ObexHeader request;
    ObexHeader reply;
    int res = rejectServer.OnDelete(request, reply);
    EXPECT_EQ(res, OBEX_HTTP_UNAVAILABLE);
    HILOGI("ObexRejectServer_OnDelete_001 end");
}

/*
 * @tc.number: ObexRejectServer_OnAbort_001
 * @tc.name: OnAbort
 * @tc.desc:
 */
HWTEST_F(ObexRejectServerTest, ObexRejectServer_OnAbort_001, TestSize.Level1)
{
    HILOGI("ObexRejectServer_OnAbort_001 start");
    ObexHeader request;
    ObexHeader reply;
    int res = rejectServer.OnAbort(request, reply);
    EXPECT_EQ(res, OBEX_HTTP_UNAVAILABLE);
    HILOGI("ObexRejectServer_OnAbort_001 end");
}

/*
 * @tc.number: ObexRejectServer_GetConnectionId_001
 * @tc.name: GetConnectionId
 * @tc.desc:
 */
HWTEST_F(ObexRejectServerTest, ObexRejectServer_GetConnectionId_001, TestSize.Level1)
{
    HILOGI("ObexRejectServer_GetConnectionId_001 start");
    rejectServer.SetConnectionId(0);
    int res = rejectServer.GetConnectionId();
    EXPECT_EQ(res, OBEX_HTTP_UNAVAILABLE);
    rejectServer.OnClose(true);
    HILOGI("ObexRejectServer_GetConnectionId_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS