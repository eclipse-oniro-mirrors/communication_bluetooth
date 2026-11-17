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

#include "remote_device_properties.h"

#include <gtest/gtest.h>

#include "bluetooth_dialog.h"
#include "dialog_pair.h"
#include "permission_manager.h"
#include "raw_address.h"
#include "want.h"

using namespace testing;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
namespace {
const std::string DEV_DRIVER = "00:11:22:33:44:55";
const std::string DEV_PASSENGER = "AA:BB:CC:DD:EE:FF";
// main user id and secondary user id on multi-user products
constexpr int32_t DRIVER_USER_ID = 100;
constexpr int32_t PASSENGER_USER_ID = 102;
constexpr int PAIR_REQ_TYPE = 4;
constexpr int PAIR_PASSKEY = 0;

RemoteDeviceProperties &GetProps()
{
    return *RemoteDeviceProperties::GetInstance();
}
}  // namespace

// TC: SF_PAIR_UID_001  Set/Get 记录与读取往返（Get 为纯读取，不改变记录）
TEST(PairCallerUserIdTest, SetAndGet_Roundtrip)
{
    RawAddress device(DEV_PASSENGER);
    GetProps().RemovePairCallerUserId(device);
    GetProps().SetPairCallerUserId(device, PASSENGER_USER_ID);
    EXPECT_EQ(PASSENGER_USER_ID, GetProps().GetPairCallerUserId(device));
    EXPECT_EQ(PASSENGER_USER_ID, GetProps().GetPairCallerUserId(device));
    GetProps().RemovePairCallerUserId(device);
    EXPECT_EQ(INVALID_USER_ID, GetProps().GetPairCallerUserId(device));
}

// TC: SF_PAIR_UID_002  无记录时 Get 返回 INVALID_USER_ID（默认屏兜底）
TEST(PairCallerUserIdTest, Get_NoRecord_ReturnsInvalid)
{
    RawAddress device(DEV_PASSENGER);
    GetProps().RemovePairCallerUserId(device);
    EXPECT_EQ(INVALID_USER_ID, GetProps().GetPairCallerUserId(device));
}

// TC: SF_PAIR_UID_003  同一设备再次发起配对覆盖记录（弹窗跟随最新发起方）
TEST(PairCallerUserIdTest, Set_Twice_Overwrites)
{
    RawAddress device(DEV_PASSENGER);
    GetProps().SetPairCallerUserId(device, PASSENGER_USER_ID);
    GetProps().SetPairCallerUserId(device, DRIVER_USER_ID);
    EXPECT_EQ(DRIVER_USER_ID, GetProps().GetPairCallerUserId(device));
}

// TC: SF_PAIR_UID_004  Remove 清理记录
TEST(PairCallerUserIdTest, Remove_Record_Cleared)
{
    RawAddress device(DEV_PASSENGER);
    GetProps().SetPairCallerUserId(device, PASSENGER_USER_ID);
    GetProps().RemovePairCallerUserId(device);
    EXPECT_EQ(INVALID_USER_ID, GetProps().GetPairCallerUserId(device));
}

// TC: SF_PAIR_UID_005  多设备记录互不干扰（不同用户同时配对不同设备）
TEST(PairCallerUserIdTest, Set_MultipleDevices_Independent)
{
    RawAddress driverDevice(DEV_DRIVER);
    RawAddress passengerDevice(DEV_PASSENGER);
    GetProps().SetPairCallerUserId(driverDevice, DRIVER_USER_ID);
    GetProps().SetPairCallerUserId(passengerDevice, PASSENGER_USER_ID);
    EXPECT_EQ(DRIVER_USER_ID, GetProps().GetPairCallerUserId(driverDevice));
    EXPECT_EQ(PASSENGER_USER_ID, GetProps().GetPairCallerUserId(passengerDevice));
}

// TC: SF_PAIR_UID_006  无效 userId 不写入也不覆盖旧记录（守卫在 Set 入口，
// 保护 ACL rebond/栈内发起不沿用陈旧路由，弹窗走默认屏兜底）
TEST(PairCallerUserIdTest, Set_InvalidUserId_Ignored_RecordPreserved)
{
    RawAddress device(DEV_PASSENGER);
    GetProps().RemovePairCallerUserId(device);
    GetProps().SetPairCallerUserId(device, PASSENGER_USER_ID);
    GetProps().SetPairCallerUserId(device, INVALID_USER_ID);
    EXPECT_EQ(PASSENGER_USER_ID, GetProps().GetPairCallerUserId(device));
}

// TC: SF_PAIR_UID_007  镜像 StartPair 调用模式：单测进程为 native 调用方，
// GetCallingUserId 返回 INVALID_USER_ID，Set 吸收守卫后不产生记录
TEST(PairCallerUserIdTest, Set_FromNativeCaller_NotRecorded)
{
    RawAddress device(DEV_PASSENGER);
    GetProps().RemovePairCallerUserId(device);
    GetProps().SetPairCallerUserId(device, PermissionManager::GetCallingUserId());
    EXPECT_EQ(INVALID_USER_ID, GetProps().GetPairCallerUserId(device));
}

// TC: SF_PAIR_UID_008  已记录发起方 userId 时请求配对弹窗不消费该记录
// （一次配对会话可能多次请求弹窗：TWS 融合重试/配对失败重试由栈内发起，不经过
// StartPair 重新 Set，记录须保留供后续弹窗路由；记录由下一次 StartPair 覆盖、
// 手动解绑时清除。单测环境无 AMS/sceneboard 路由效果，由整机 T1/T2 用例验证）
TEST(PairCallerUserIdTest, RequestPairDialog_WithUserId_PreservesRecord)
{
    RawAddress device(DEV_PASSENGER);
    GetProps().RemovePairCallerUserId(device);
    GetProps().SetPairCallerUserId(device, PASSENGER_USER_ID);
    DialogPair::RequestBluetoothPairDialog(device, PAIR_REQ_TYPE, PAIR_PASSKEY);
    EXPECT_EQ(PASSENGER_USER_ID, GetProps().GetPairCallerUserId(device));
    GetProps().RemovePairCallerUserId(device);
}

// TC: SF_PAIR_UID_009  无记录时请求配对弹窗走默认路由（-1），仍无记录
TEST(PairCallerUserIdTest, RequestPairDialog_NoRecord_StaysInvalid)
{
    RawAddress device(DEV_PASSENGER);
    GetProps().RemovePairCallerUserId(device);
    DialogPair::RequestBluetoothPairDialog(device, PAIR_REQ_TYPE, PAIR_PASSKEY);
    EXPECT_EQ(INVALID_USER_ID, GetProps().GetPairCallerUserId(device));
}

// TC: SF_PAIR_UID_010  不传 userId（默认参数 DEFAULT_VALUE(-1)）与显式传 -1 调用等价
// （覆盖既有调用方路径：PBAP/MAP/AutoPlay 授权弹窗等不传 userId，走默认参数）
TEST(PairCallerUserIdTest, DialogConnectExtensionAbility_DefaultParam_MatchesExplicitInvalid)
{
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    bool retWithDefault = BluetoothDialog::DialogConnectExtensionAbility(
        want, "cmdStr", "bundleName", "abilityName");
    bool retWithExplicitInvalid = BluetoothDialog::DialogConnectExtensionAbility(
        want, "cmdStr", "bundleName", "abilityName", -1);
    EXPECT_EQ(retWithExplicitInvalid, retWithDefault);
}

}  // namespace bluetooth
}  // namespace OHOS
