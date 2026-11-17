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
#include <vector>
#include <string>
#include <cstring>
#include "classic_adapter.h"
#include "power_device.h"
#include "class_creator.h"
#include "adapter_config.h"
#include "ble_adapter.h"
#include "bluetooth_host.h"
#include "hw_bt_hwif.h"
#include "adapter_manager.h"
#include "classic_sync_paired_device.h"
#include "bluetooth_access_token_mock.h"

using namespace OHOS::Bluetooth;
using namespace testing;
using namespace testing::ext;

extern "C" {
static bt_status_t g_state = BT_STATUS_SUCCESS;
static const BthwifCallbacks* g_btAdapterhwifCallbacks = nullptr;
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
static bool NeedSyncPairedInfoMock(std::vector<std::string> &pairedInfo)
{
    return true;
}
static const bthwif_interface_t g_btAdapterhwifInterface = {
    .Init = BtAdapterhwifInterfaceInitMock,
    .needSyncPairedInfo = NeedSyncPairedInfoMock,
};
}

namespace OHOS {
namespace bluetooth {

static BluetoothHost *host_;
constexpr int TIME = 2;

class ClassicSyncTest : public testing::Test {
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

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase start");
        host_ = nullptr;
        HILOGI("TearDownTestCase end");
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

/**
 * @tc.name: classic_sync_test_001
 * @tc.desc: BluetoothSyncPairedDev::SyncPairedDev
 * @tc.type: btIf == nullptr
 */
HWTEST_F(ClassicSyncTest, classic_sync_test_001, TestSize.Level1)
{
    HILOGI("classic_sync_test_001 enter");
    std::vector<std::string> addressVec;
    bool needSync = BluetoothSyncPairedDev::GetInstance().SyncPairedDev(addressVec);
    EXPECT_FALSE(needSync);
    HILOGI("classic_sync_test_001 end");
}

static const void* GetProfileInterfaceMock1(const char* profileId)
{
    HILOGI("enter");
    return nullptr;
}

/**
 * @tc.name: classic_sync_test_002
 * @tc.desc: BluetoothSyncPairedDev::SyncPairedDev
 * @tc.type: bthwif == nullptr
 */
HWTEST_F(ClassicSyncTest, classic_sync_test_002, TestSize.Level1)
{
    HILOGI("classic_sync_test_002 enter");
    bt_interface_t btInterface = {
        .get_profile_interface = GetProfileInterfaceMock1,
    };
    AdapterManager::GetInstance()->setBluetoothInterface(&btInterface);
    std::vector<std::string> addressVec;
    bool needSync = BluetoothSyncPairedDev::GetInstance().SyncPairedDev(addressVec);
    EXPECT_FALSE(needSync);
    HILOGI("classic_sync_test_002 end");
}

static const void* GetProfileInterfaceMock2(const char* profileId)
{
    HILOGI("enter");
    return (void *)(&g_btAdapterhwifInterface);
}

/**
 * @tc.name: classic_sync_test_003
 * @tc.desc: BluetoothSyncPairedDev::SyncPairedDev
 * @tc.type: bthwif == nullptr
 */
HWTEST_F(ClassicSyncTest, classic_sync_test_003, TestSize.Level1)
{
    HILOGI("classic_sync_test_003 enter");
    bt_interface_t btInterface = {
        .get_profile_interface = GetProfileInterfaceMock2,
    };
    AdapterManager::GetInstance()->setBluetoothInterface(&btInterface);
    std::vector<std::string> addressVec;
    bool needSync = BluetoothSyncPairedDev::GetInstance().SyncPairedDev(addressVec);
    EXPECT_TRUE(needSync);
    HILOGI("classic_sync_test_003 end");
}
}
}