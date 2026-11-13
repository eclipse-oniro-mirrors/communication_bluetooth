/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "avrcp_ct_bip_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

constexpr uint16_t TEST_PSM = 0x1001;

class MockBipClientCallback : public AvrcpCtBipClient::Callback {
public:
    MockBipClientCallback() = default;
    ~MockBipClientCallback() override = default;

    void OnConnectionStateChanged(int oldState, int newState) override
    {
        lastOldState_ = oldState;
        lastNewState_ = newState;
    }

    void OnGetImagePropertiesComplete(int status, const std::string &imageHandle,
        const BipImageProperties &properties) override
    {
        lastStatus_ = status;
        lastImageHandle_ = imageHandle;
    }

    void OnGetImageComplete(int status, const std::string &imageHandle, const BipImage &image) override
    {
        lastStatus_ = status;
        lastImageHandle_ = imageHandle;
    }

    int lastOldState_ = -1;
    int lastNewState_ = -1;
    int lastStatus_ = -1;
    std::string lastImageHandle_ = "";
};

class AvrcpCtBipClientTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        callback_ = std::make_shared<MockBipClientCallback>();
        client_ = std::make_shared<AvrcpCtBipClient>("00:11:22:33:44:55", TEST_PSM, callback_);
    }
    void TearDown() override
    {
        client_ = nullptr;
        callback_ = nullptr;
    }

protected:
    std::shared_ptr<MockBipClientCallback> callback_;
    std::shared_ptr<AvrcpCtBipClient> client_;
};

/*
 * @tc.number: AvrcpCtBipClientTest_Constructor_001
 * @tc.name: Constructor
 * @tc.desc: Test client is created in disconnected state with psm
 */
HWTEST_F(AvrcpCtBipClientTest, AvrcpCtBipClientTest_Constructor_001, TestSize.Level1)
{
    EXPECT_EQ(client_->GetState(), AvrcpCtBipClient::STATE_DISCONNECTED);
    EXPECT_FALSE(client_->IsConnected());
    EXPECT_EQ(client_->GetPsm(), TEST_PSM);
}

/*
 * @tc.number: AvrcpCtBipClientTest_StateConstants_001
 * @tc.name: State constants
 * @tc.desc: Test connection state constant values
 */
HWTEST_F(AvrcpCtBipClientTest, AvrcpCtBipClientTest_StateConstants_001, TestSize.Level1)
{
    EXPECT_EQ(AvrcpCtBipClient::STATE_DISCONNECTED, 0);
    EXPECT_EQ(AvrcpCtBipClient::STATE_CONNECTING, 1);
    EXPECT_EQ(AvrcpCtBipClient::STATE_CONNECTED, 2);
    EXPECT_EQ(AvrcpCtBipClient::STATE_DISCONNECTING, 3);
}

/*
 * @tc.number: AvrcpCtBipClientTest_GetImageProperties_001
 * @tc.name: GetImageProperties not connected
 * @tc.desc: Test GetImageProperties returns false when not connected
 */
HWTEST_F(AvrcpCtBipClientTest, AvrcpCtBipClientTest_GetImageProperties_001, TestSize.Level1)
{
    EXPECT_FALSE(client_->GetImageProperties("1234567"));
    EXPECT_EQ(callback_->lastImageHandle_, "");
}

/*
 * @tc.number: AvrcpCtBipClientTest_GetImage_001
 * @tc.name: GetImage not connected
 * @tc.desc: Test GetImage returns false when not connected
 */
HWTEST_F(AvrcpCtBipClientTest, AvrcpCtBipClientTest_GetImage_001, TestSize.Level1)
{
    BipImageDescriptor descriptor;
    descriptor.format = BIP_IMAGE_FORMAT_JPEG;
    descriptor.width = 200;
    descriptor.height = 200;
    EXPECT_FALSE(client_->GetImage("1234567", descriptor));
    EXPECT_EQ(callback_->lastImageHandle_, "");
}

/*
 * @tc.number: AvrcpCtBipClientTest_Constructor_002
 * @tc.name: Constructor with null callback
 * @tc.desc: Test client can be created with null callback
 */
HWTEST_F(AvrcpCtBipClientTest, AvrcpCtBipClientTest_Constructor_002, TestSize.Level1)
{
    auto client = std::make_shared<AvrcpCtBipClient>("00:11:22:33:44:55", TEST_PSM, nullptr);
    EXPECT_EQ(client->GetState(), AvrcpCtBipClient::STATE_DISCONNECTED);
    EXPECT_FALSE(client->IsConnected());
    EXPECT_EQ(client->GetPsm(), TEST_PSM);
}

/*
 * @tc.number: AvrcpCtBipClientTest_Psm_001
 * @tc.name: Different psm values
 * @tc.desc: Test client stores different psm values
 */
HWTEST_F(AvrcpCtBipClientTest, AvrcpCtBipClientTest_Psm_001, TestSize.Level1)
{
    auto client = std::make_shared<AvrcpCtBipClient>("00:11:22:33:44:55", 0, nullptr);
    EXPECT_EQ(client->GetPsm(), 0);
}
} // namespace bluetooth
} // namespace OHOS
