/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
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

/*
 * Unit test for MultiA2dpManager (car_policy/multi_a2dp).
 *
 * The production implementation is included directly so that the internal
 * mapping helpers (NormalizeAddr / IsMultiA2dpDisplay), the per-display
 * device callback and the internal maps can be verified without depending on
 * the AutoZoneManager / DisplayManager services. Init()/Cleanup() retry
 * behaviour is covered only for the "not initialized" contract here; the
 * display/vendor-plugin dependent path is validated on device.
 */

#include <mutex>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "log.h"
#include "multi_a2dp_manager.cpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

namespace {
constexpr int32_t kInvalidId = -1;
constexpr int32_t kMainUserId = 100;
constexpr int32_t kCoDriverUserId = 101;
constexpr int32_t kRearUserId = 102;
constexpr int32_t kCoDriverDisplayId = 2;
constexpr int32_t kRearDisplayId = 3;

bluetooth::ZoneAudioDevice MakeDevice(const std::string &addr, int32_t status)
{
    bluetooth::ZoneAudioDevice device = {};
    size_t len = addr.size();
    if (len >= sizeof(device.deviceAddress)) {
        len = sizeof(device.deviceAddress) - 1;
    }
    (void)addr.copy(device.deviceAddress, len);
    device.status = status;
    return device;
}
}  // namespace

class MultiA2dpManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        HILOGI("MultiA2dpManagerTest::SetUpTestCase start");
    }
    static void TearDownTestCase(void)
    {
        HILOGI("MultiA2dpManagerTest::TearDownTestCase");
    }
    void SetUp() override
    {
        ClearMaps();
    }
    void TearDown() override
    {
        ClearMaps();
    }

    static void ClearMaps()
    {
        {
            std::lock_guard<std::mutex> lock(g_deviceDisplayMapMutex);
            g_addrToDisplayId.clear();
            g_displayIdToAddr.clear();
        }
        {
            std::lock_guard<std::mutex> lock(g_displayUserMapMutex);
            g_displayIdToUserId.clear();
        }
        {
            std::lock_guard<std::mutex> lock(g_displayCallbackMapMutex);
            g_deviceCallbackMap.clear();
        }
        MultiA2dpManager::UnregisterDeviceBoundCallback();
    }

    static void ConnectDevice(const std::string &addr, int32_t displayId)
    {
        BtAudioDeviceChangeCallback callback(displayId);
        bluetooth::ZoneAudioDevice device =
            MakeDevice(addr, bluetooth::ZONE_AUDIO_DEVICE_STATUS_CONNECT);
        callback.OnDeviceChange(&device, 1);
    }

    static void DisconnectDevice(const std::string &addr, int32_t displayId)
    {
        BtAudioDeviceChangeCallback callback(displayId);
        bluetooth::ZoneAudioDevice device =
            MakeDevice(addr, bluetooth::ZONE_AUDIO_DEVICE_STATUS_DISCONNECT);
        callback.OnDeviceChange(&device, 1);
    }
};

/*
 * @tc.name: NormalizeAddr_001
 * @tc.desc: MAC normalization keeps digits/colon and lowercases letters.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, NormalizeAddr_001, TestSize.Level1)
{
    EXPECT_EQ("aa:bb:cc:dd:ee:ff", NormalizeAddr("AA:BB:CC:DD:EE:FF"));
    EXPECT_EQ("aa:bb:cc:dd:ee:ff", NormalizeAddr("aa:bb:cc:dd:ee:ff"));
    EXPECT_EQ("0a:1b:2c:3d:4e:5f", NormalizeAddr("0A:1B:2C:3D:4E:5F"));
    EXPECT_EQ("", NormalizeAddr(""));
    EXPECT_EQ("aa-bb-cc", NormalizeAddr("AA-BB-CC"));
    EXPECT_EQ("zz:11", NormalizeAddr("ZZ:11"));
}

/*
 * @tc.name: IsMultiA2dpDisplay_001
 * @tc.desc: Only co-driver / central rear / rear armrest panels are supported
 *           multi A2DP displays; the check is exact and case sensitive.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, IsMultiA2dpDisplay_001, TestSize.Level1)
{
    EXPECT_TRUE(IsMultiA2dpDisplay("co-driver_panel"));
    EXPECT_TRUE(IsMultiA2dpDisplay("central_rear_panel"));
    EXPECT_TRUE(IsMultiA2dpDisplay("rear_armrest_panel"));

    EXPECT_FALSE(IsMultiA2dpDisplay("main_panel"));
    EXPECT_FALSE(IsMultiA2dpDisplay(""));
    EXPECT_FALSE(IsMultiA2dpDisplay("CO-DRIVER_PANEL"));
    EXPECT_FALSE(IsMultiA2dpDisplay("co-driver_panel "));
    EXPECT_FALSE(IsMultiA2dpDisplay("central_rear_panel_x"));
}

/*
 * @tc.name: GetDisplayIdByAddr_001
 * @tc.desc: Address lookup is normalized and misses report INVALID_USER_ID.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, GetDisplayIdByAddr_001, TestSize.Level1)
{
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetDisplayIdByAddr("11:22:33:44:55:66"));
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetDisplayIdByAddr(""));

    ConnectDevice("AA:BB:CC:DD:EE:FF", kCoDriverDisplayId);
    EXPECT_EQ(kCoDriverDisplayId, MultiA2dpManager::GetDisplayIdByAddr("AA:BB:CC:DD:EE:FF"));
    EXPECT_EQ(kCoDriverDisplayId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:ff"));
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:fe"));
}

/*
 * @tc.name: GetAddrByDisplayId_001
 * @tc.desc: Reverse lookup returns the normalized address, empty on miss.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, GetAddrByDisplayId_001, TestSize.Level1)
{
    EXPECT_EQ("", MultiA2dpManager::GetAddrByDisplayId(kCoDriverDisplayId));

    ConnectDevice("AA:BB:CC:DD:EE:0A", kCoDriverDisplayId);
    EXPECT_EQ("aa:bb:cc:dd:ee:0a", MultiA2dpManager::GetAddrByDisplayId(kCoDriverDisplayId));
    EXPECT_EQ("", MultiA2dpManager::GetAddrByDisplayId(kRearDisplayId));
}

/*
 * @tc.name: DeviceCallback_001
 * @tc.desc: Connect fills both direction maps, disconnect removes them.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, DeviceCallback_001, TestSize.Level1)
{
    ConnectDevice("AA:BB:CC:DD:EE:01", kCoDriverDisplayId);
    EXPECT_EQ(kCoDriverDisplayId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:01"));
    EXPECT_EQ("aa:bb:cc:dd:ee:01", MultiA2dpManager::GetAddrByDisplayId(kCoDriverDisplayId));

    DisconnectDevice("AA:BB:CC:DD:EE:01", kCoDriverDisplayId);
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:01"));
    EXPECT_EQ("", MultiA2dpManager::GetAddrByDisplayId(kCoDriverDisplayId));
}

/*
 * @tc.name: DeviceCallback_002
 * @tc.desc: Empty addresses are ignored; multiple devices in one notification
 *           are all recorded.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, DeviceCallback_002, TestSize.Level1)
{
    BtAudioDeviceChangeCallback callback(kCoDriverDisplayId);
    bluetooth::ZoneAudioDevice empty =
        MakeDevice("", bluetooth::ZONE_AUDIO_DEVICE_STATUS_CONNECT);
    callback.OnDeviceChange(&empty, 1);
    EXPECT_EQ("", MultiA2dpManager::GetAddrByDisplayId(kCoDriverDisplayId));

    bluetooth::ZoneAudioDevice devices[] = {
        MakeDevice("AA:BB:CC:DD:EE:02", bluetooth::ZONE_AUDIO_DEVICE_STATUS_CONNECT),
        MakeDevice("AA:BB:CC:DD:EE:03", bluetooth::ZONE_AUDIO_DEVICE_STATUS_CONNECT),
    };
    callback.OnDeviceChange(devices, 2);
    EXPECT_EQ(kCoDriverDisplayId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:02"));
    EXPECT_EQ(kCoDriverDisplayId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:03"));
    // The reverse map only keeps the latest address bound to the display.
    EXPECT_EQ("aa:bb:cc:dd:ee:03", MultiA2dpManager::GetAddrByDisplayId(kCoDriverDisplayId));
}

/*
 * @tc.name: DeviceCallback_003
 * @tc.desc: Disconnect of an unknown address is tolerated and does not clear
 *           another display's binding.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, DeviceCallback_003, TestSize.Level1)
{
    ConnectDevice("AA:BB:CC:DD:EE:04", kCoDriverDisplayId);
    ConnectDevice("AA:BB:CC:DD:EE:05", kRearDisplayId);

    // A disconnect notification only clears the reporting display's reverse
    // binding; other displays and the address map stay intact.
    DisconnectDevice("AA:BB:CC:DD:EE:99", kCoDriverDisplayId);
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:99"));
    EXPECT_EQ("", MultiA2dpManager::GetAddrByDisplayId(kCoDriverDisplayId));
    EXPECT_EQ(kRearDisplayId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:05"));
    EXPECT_EQ("aa:bb:cc:dd:ee:05", MultiA2dpManager::GetAddrByDisplayId(kRearDisplayId));
}

/*
 * @tc.name: UserMapping_001
 * @tc.desc: displayId -> userId mapping drives GetUserIdByDisplayId /
 *           GetUserIdByAddr / GetDisplayIdByUserId.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, UserMapping_001, TestSize.Level1)
{
    {
        std::lock_guard<std::mutex> lock(g_displayUserMapMutex);
        g_displayIdToUserId[kCoDriverDisplayId] = kCoDriverUserId;
        g_displayIdToUserId[kRearDisplayId] = kRearUserId;
    }
    ConnectDevice("AA:BB:CC:DD:EE:06", kCoDriverDisplayId);

    EXPECT_EQ(kCoDriverUserId, MultiA2dpManager::GetUserIdByDisplayId(kCoDriverDisplayId));
    EXPECT_EQ(kCoDriverUserId, MultiA2dpManager::GetUserIdByAddr("aa:bb:cc:dd:ee:06"));
    EXPECT_EQ(kCoDriverDisplayId, MultiA2dpManager::GetDisplayIdByUserId(kCoDriverUserId));
    EXPECT_EQ(kRearDisplayId, MultiA2dpManager::GetDisplayIdByUserId(kRearUserId));

    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetUserIdByDisplayId(9));
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetUserIdByAddr("aa:bb:cc:dd:ee:07"));
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetDisplayIdByUserId(kMainUserId));
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetDisplayIdByUserId(kInvalidId));
}

/*
 * @tc.name: UserMapping_002
 * @tc.desc: An address bound to a display without user mapping reports
 *           INVALID_USER_ID instead of leaking another display's user.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, UserMapping_002, TestSize.Level1)
{
    {
        std::lock_guard<std::mutex> lock(g_displayUserMapMutex);
        g_displayIdToUserId[kRearDisplayId] = kRearUserId;
    }
    ConnectDevice("AA:BB:CC:DD:EE:08", kCoDriverDisplayId);

    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetUserIdByAddr("aa:bb:cc:dd:ee:08"));
    EXPECT_EQ(kRearUserId, MultiA2dpManager::GetUserIdByDisplayId(kRearDisplayId));
}

/*
 * @tc.name: CleanupWithoutInit_001
 * @tc.desc: Cleanup before a successful Init must return immediately (no retry
 *           thread to wait for) and be idempotent.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, CleanupWithoutInit_001, TestSize.Level1)
{
    MultiA2dpManager::Cleanup();
    MultiA2dpManager::Cleanup();

    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:09"));
    EXPECT_EQ("", MultiA2dpManager::GetAddrByDisplayId(kCoDriverDisplayId));
    EXPECT_EQ(kInvalidId, MultiA2dpManager::GetUserIdByDisplayId(kCoDriverDisplayId));
}

/*
 * @tc.name: DeviceBoundCallback_001
 * @tc.desc: Connect notifies the registered callback with the reported address after
 *           the binding is visible; disconnect does not notify.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, DeviceBoundCallback_001, TestSize.Level1)
{
    std::vector<std::string> notified;
    int32_t displayIdSeen = kInvalidId;
    MultiA2dpManager::RegisterDeviceBoundCallback([&](const std::string &addr) {
        notified.push_back(addr);
        displayIdSeen = MultiA2dpManager::GetDisplayIdByAddr(addr);
    });

    ConnectDevice("AA:BB:CC:DD:EE:10", kRearDisplayId);
    ASSERT_EQ(1u, notified.size());
    EXPECT_EQ("AA:BB:CC:DD:EE:10", notified[0]);
    EXPECT_EQ(kRearDisplayId, displayIdSeen);

    DisconnectDevice("AA:BB:CC:DD:EE:10", kRearDisplayId);
    EXPECT_EQ(1u, notified.size());
}

/*
 * @tc.name: DeviceBoundCallback_002
 * @tc.desc: No callback or an unregistered callback is not invoked, and the mapping is
 *           still recorded.
 * @tc.type: FUNC
 */
HWTEST_F(MultiA2dpManagerTest, DeviceBoundCallback_002, TestSize.Level1)
{
    ConnectDevice("AA:BB:CC:DD:EE:11", kCoDriverDisplayId);
    EXPECT_EQ(kCoDriverDisplayId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:11"));

    int32_t count = 0;
    MultiA2dpManager::RegisterDeviceBoundCallback([&](const std::string &) { count++; });
    MultiA2dpManager::UnregisterDeviceBoundCallback();
    ConnectDevice("AA:BB:CC:DD:EE:12", kRearDisplayId);
    EXPECT_EQ(0, count);
    EXPECT_EQ(kRearDisplayId, MultiA2dpManager::GetDisplayIdByAddr("aa:bb:cc:dd:ee:12"));
}

}  // namespace Bluetooth
}  // namespace OHOS
