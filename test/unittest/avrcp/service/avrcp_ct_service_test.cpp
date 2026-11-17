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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "avrcp_ct_define.h"
#include "avrcp_ct_service.h"
#include "avrcp_ct_state_machine.h"
#include "interface_profile_avrcp_ct.h"

namespace bluetooth {
class AvrcpCtServiceTest : public testing::Test {
public:
    AvrcpCtServiceTest()
    {
        service_ = nullptr;
    }
    ~AvrcpCtServiceTest()
    {
        if (service_ != nullptr) {
            delete service_;
            service_ = nullptr;
        }
    }

protected:
    void SetUp() override
    {
        service_ = new AvrcpCtService();
    }

    void TearDown() override
    {
        if (service_ != nullptr) {
            delete service_;
            service_ = nullptr;
        }
    }

    AvrcpCtService* service_;
};

HWTEST_F(AvrcpCtServiceTest, Enable001, TestSize.Level1)
{
    int ret = service_->Enable();
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, Disable001, TestSize.Level1)
{
    int ret = service_->Disable();
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, GetConnectState001, TestSize.Level1)
{
    AvrcpCtConnectState state = service_->GetConnectState();
    EXPECT_EQ(state, AvrcpCtConnectState::DISCONNECTED);
}

HWTEST_F(AvrcpCtServiceTest, GetDeviceConnectState001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    AvrcpCtConnectState state = service_->GetDeviceConnectState(deviceAddr);
    EXPECT_EQ(state, AvrcpCtConnectState::DISCONNECTED);
}

HWTEST_F(AvrcpCtServiceTest, Connect001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    int ret = service_->Connect(deviceAddr);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, Disconnect001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    int ret = service_->Disconnect(deviceAddr);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, SendPlayerAppSettingCmd001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    uint8_t cmdType = 0;
    std::vector<uint8_t> attrs = {0};
    std::vector<uint8_t> values = {0};
    int ret = service_->SendPlayerAppSettingCmd(deviceAddr, cmdType, attrs, values);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, SendPlayBackCmd001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    uint8_t cmdType = 0;
    uint8_t playStatus = 0;
    int ret = service_->SendPlayBackCmd(deviceAddr, cmdType, playStatus);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, SendVolumeCmd001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    uint8_t cmdType = 0;
    uint8_t volume = 0;
    int ret = service_->SendVolumeCmd(deviceAddr, cmdType, volume);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, SendMidItemCmd001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    uint8_t cmdType = 0;
    std::vector<uint8_t> attrs = {0};
    int ret = service_->SendMidItemCmd(deviceAddr, cmdType, attrs);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, SendPlayBackStatusCmd001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    uint8_t cmdType = 0;
    int ret = service_->SendPlayBackStatusCmd(deviceAddr, cmdType);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, SendNowPlayingContentCmd001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    uint8_t cmdType = 0;
    int ret = service_->SendNowPlayingContentCmd(deviceAddr, cmdType);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, GetDeviceAddrByRole001, TestSize.Level1)
{
    std::string deviceAddr = service_->GetDeviceAddrByRole(BtRole::ROLE_BREDR_SECONDARY);
    EXPECT_EQ(deviceAddr, "");
}

HWTEST_F(AvrcpCtServiceTest, IsActiveDevice001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    bool isActive = service_->IsActiveDevice(deviceAddr);
    EXPECT_FALSE(isActive);
}

HWTEST_F(AvrcpCtServiceTest, SetActiveDevice001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    int ret = service_->SetActiveDevice(deviceAddr);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, RemoveActiveDevice001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    int ret = service_->RemoveActiveDevice(deviceAddr);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

HWTEST_F(AvrcpCtServiceTest, GetCurrentPlayingDevice001, TestSize.Level1)
{
    std::string deviceAddr = service_->GetCurrentPlayingDevice();
    EXPECT_EQ(deviceAddr, "");
}

HWTEST_F(AvrcpCtServiceTest, IsSessionConnected001, TestSize.Level1)
{
    std::string deviceAddr = "00:00:00:00:00:00";
    bool isConnected = service_->IsSessionConnected(deviceAddr);
    EXPECT_FALSE(isConnected);
}
} // namespace bluetooth