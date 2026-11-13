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
#include "pbap_pse_service.h"
#include "class_creator.h"
#include "profile_service_manager.h"
#include "thread_util.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
constexpr int TIME = 2;
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
std::shared_ptr<IObexSocketObserver> socketObserver = std::make_shared<PbapPseSocketObserver>();
std::shared_ptr<ObexServerSocket> pbapObexServerSocket =
    std::make_shared<ObexServerSocket>(PBAP_PSE_SERVICE_NAME, PBAP_RFCOMM_CHANNEL, -1, EMPTY_UUID, socketObserver);
std::shared_ptr<ObexServerSocket> mapObexServerSocket =
    std::make_shared<ObexServerSocket>("OBEX Message Access", 26, 0x1029, EMPTY_UUID, socketObserver);

void *PbapPseServiceCreate(void)
{
    return new PbapPseService();
}

class ObexServerSocketTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ClassFactory::RegisterClass("PbapPseService", PbapPseServiceCreate);
    }
    static void TearDownTestCase()
    {
        sleep(TIME);
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * @tc.number: ObexServerSocket_001
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ObexServerSocketTest, ObexServerSocket_001, TestSize.Level1)
{
    HILOGI("ObexServerSocket_001 start11");
    pbapObexServerSocket->Startup(3);
    EXPECT_FALSE(pbapObexServerSocket->CreateSocket(3));
    
    ProfileServiceManager::GetInstance()->SetProfileService(ADAPTER_BREDR, "SocketService");
    ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_PBAP, ThreadUtil::ThreadState::DISABLED);
    pbapObexServerSocket->Startup(3);
    EXPECT_FALSE(pbapObexServerSocket->CreateSocket(3));
    pbapObexServerSocket->StartAccept();
    pbapObexServerSocket->Shutdown();

    mapObexServerSocket->Startup(3);
    EXPECT_FALSE(mapObexServerSocket->CreateSocket(3));
    mapObexServerSocket->StartAccept();
    mapObexServerSocket->Shutdown();
    HILOGI("ObexServerSocket_001 end");
}
}  // namespace bluetooth
}  // namespace OHOS