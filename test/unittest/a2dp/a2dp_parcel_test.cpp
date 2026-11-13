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

#include "bluetooth_a2dp_a2dpCodecInfo.h"
#include "bluetooth_a2dp_a2dpCodecStatus.h"
#include "bluetooth_avrcp_meItem.h"
#include "bluetooth_avrcp_mpItem.h"
#include "message_parcel.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"
#include "bluetooth_host.h"


using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Bluetooth {
static BluetoothHost *host_;
constexpr int TIME = 2;
class BlueTooth2dpParcelTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("SetUpTestCase start");
        host_ = &BluetoothHost::GetDefaultHost();

        bool isBrEnable = host_->IsBrEnabled();
        HILOGI("isBrEnable = %{public}d", isBrEnable);

        if (!isBrEnable) {
            host_->EnableBle();
            host_->EnableBt();
            sleep(TIME);
        }

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
    }
    void SetUp()
    {}

    void TearDown()
    {}

public:
};

/**
 * @tc.name: A2DP_BlueTooth2dpParcelTest_00100
 * @tc.desc: A2DP_BlueTooth2dpParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, A2DP_BlueTooth2dpParcelTest_00100, TestSize.Level1)
{
    HILOGI("A2DP_BlueTooth2dpParcelTest_00100 start");
    MessageParcel data;
    BluetoothA2dpCodecStatus bluetoothA2dpCodecStatus;
    data.WriteParcelable(&bluetoothA2dpCodecStatus);
    std::shared_ptr<BluetoothA2dpCodecStatus> result(data.ReadParcelable<BluetoothA2dpCodecStatus>());
    BluetoothAvrcpMeItem bluetoothAvrcpMeItem;
    OHOS::Parcel parcel;
    bluetoothAvrcpMeItem.WriteToParcel(parcel);
    EXPECT_TRUE(bluetoothAvrcpMeItem.ReadFromParcel(parcel));
    HILOGI("A2DP_BlueTooth2dpParcelTest_00100 end");
}
 
/**
 * @tc.name: A2DP_BlueTooth2dpParcelTest_00200
 * @tc.desc: A2DP_BlueTooth2dpParcelTest_00200.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, A2DP_BlueTooth2dpParcelTest_00200, TestSize.Level1)
{
    HILOGI("A2DP_BlueTooth2dpParcelTest_00200 start");
    MessageParcel data;
    BluetoothAvrcpMeItem bluetoothAvrcpMeItem;
    data.WriteParcelable(&bluetoothAvrcpMeItem);
    std::shared_ptr<BluetoothAvrcpMeItem> result(data.ReadParcelable<BluetoothAvrcpMeItem>());
    ParcelBtUuid parcelBtUuid;
    bluetooth::Uuid uuid;
    parcelBtUuid.WriteToParcel(data, uuid);
    parcelBtUuid.ReadFromParcel(data);
    BluetoothA2dpCodecStatus bluetoothA2dpCodecStatus;
    OHOS::Parcel parcel;
    bluetoothA2dpCodecStatus.WriteToParcel(parcel);
    EXPECT_TRUE(bluetoothA2dpCodecStatus.ReadFromParcel(parcel));
    HILOGI("A2DP_BlueTooth2dpParcelTest_00200 end");
}

/**
 * @tc.name: BluetoothA2dpOffloadCodecInfo_MarshallingTest_00100
 * @tc.desc: BluetoothA2dpOffloadCodecInfo_MarshallingTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, BluetoothA2dpOffloadCodecInfo_MarshallingTest_00100, TestSize.Level1)
{
    HILOGI("BluetoothA2dpOffloadCodecInfo_MarshallingTest_00100 start");
    BluetoothA2dpOffloadCodecInfo offloadCodecInfo;
    Parcel parcel;
    bool ret = offloadCodecInfo.Marshalling(parcel);
    EXPECT_EQ(ret, true);
    BluetoothA2dpOffloadCodecInfo *info;
    info = offloadCodecInfo.Unmarshalling(parcel);
    HILOGI("BluetoothA2dpOffloadCodecInfo_MarshallingTest_00100 end");
}

/**
 * @tc.name: BluetoothA2dpOffloadCodecInfo_WriteToParcelTest_00100
 * @tc.desc: BluetoothA2dpOffloadCodecInfo_WriteToParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, BluetoothA2dpOffloadCodecInfo_WriteToParcelTest_00100, TestSize.Level1)
{
    HILOGI("BluetoothA2dpOffloadCodecInfo_WriteToParcelTest_00100 start");
    BluetoothA2dpOffloadCodecInfo offloadCodecInfo;
    Parcel parcel;
    bool ret = offloadCodecInfo.WriteToParcel(parcel);
    EXPECT_EQ(ret, true);
    HILOGI("BluetoothA2dpOffloadCodecInfo_WriteToParcelTest_00100 end");
}

/**
 * @tc.name: BluetoothA2dpOffloadCodecInfo_ReadFromParcelTest_00100
 * @tc.desc: BluetoothA2dpOffloadCodecInfo_ReadFromParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, BluetoothA2dpOffloadCodecInfo_ReadFromParcelTest_00100, TestSize.Level1)
{
    HILOGI("BluetoothA2dpOffloadCodecInfo_ReadFromParcelTest_00100 start");

    auto offloadCodecInfo = std::make_shared<BluetoothA2dpOffloadCodecInfo>();
    Parcel parcel1;
    auto result = offloadCodecInfo->ReadFromParcel(parcel1);
    EXPECT_EQ(result, false);
    MessageParcel parcel2;
    result = offloadCodecInfo->ReadFromParcel(parcel2);
    EXPECT_EQ(result, false);
    HILOGI("BluetoothA2dpOffloadCodecInfo_ReadFromParcelTest_00100 end");
}

/**
 * @tc.name: BluetoothA2dpOffloadCodecStatus_MarshallingTest_00100
 * @tc.desc: BluetoothA2dpOffloadCodecStatus_MarshallingTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, BluetoothA2dpOffloadCodecStatus_MarshallingTest_00100, TestSize.Level1)
{
    HILOGI("BluetoothA2dpOffloadCodecStatus_MarshallingTest_00100 start");

    BluetoothA2dpOffloadCodecStatus offloadCodecStatus;
    Parcel parcel;
    bool ret = offloadCodecStatus.Marshalling(parcel);
    EXPECT_EQ(ret, true);
    BluetoothA2dpOffloadCodecStatus *info;
    info = offloadCodecStatus.Unmarshalling(parcel);
    HILOGI("BluetoothA2dpOffloadCodecStatus_MarshallingTest_00100 end");
}

/**
 * @tc.name: BluetoothA2dpOffloadCodecStatus_WriteToParcelTest_00100
 * @tc.desc: BluetoothA2dpOffloadCodecStatus_WriteToParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, BluetoothA2dpOffloadCodecStatus_WriteToParcelTest_00100, TestSize.Level1)
{
    HILOGI("BluetoothA2dpOffloadCodecStatus_WriteToParcelTest_00100 start");
    BluetoothA2dpOffloadCodecStatus offloadCodecInfo;
    Parcel parcel;
    bool ret = offloadCodecInfo.WriteToParcel(parcel);
    EXPECT_EQ(ret, true);
    HILOGI("BluetoothA2dpOffloadCodecStatus_WriteToParcelTest_00100 end");
}

/**
 * @tc.name: BluetoothA2dpOffloadCodecStatus_ReadFromParcelTest_00100
 * @tc.desc: BluetoothA2dpOffloadCodecStatus_ReadFromParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, BluetoothA2dpOffloadCodecStatus_ReadFromParcelTest_00100, TestSize.Level1)
{
    HILOGI("BluetoothA2dpOffloadCodecStatus_ReadFromParcelTest_00100 start");
    auto offloadCodecStatus = std::make_shared<BluetoothA2dpOffloadCodecStatus>();
    Parcel parcel1;
    auto result = offloadCodecStatus->ReadFromParcel(parcel1);
    EXPECT_EQ(result, false);
    MessageParcel parcel2;
    result = offloadCodecStatus->ReadFromParcel(parcel2);
    EXPECT_EQ(result, false);
    HILOGI("BluetoothA2dpOffloadCodecStatus_ReadFromParcelTest_00100 end");
}

/**
 * @tc.name: bluetoothA2dpCodecInfo_WriteToParcelTest_00100
 * @tc.desc: bluetoothA2dpCodecInfo_WriteToParcelTest_00100.
 * @tc.type: FUNC
 */
HWTEST_F(BlueTooth2dpParcelTest, bluetoothA2dpCodecInfo_WriteToParcelTest_00100, TestSize.Level1)
{
    HILOGI("bluetoothA2dpCodecInfo_WriteToParcelTest_00100 start");
    auto bluetoothA2dpCodecInfo = std::make_shared<BluetoothA2dpCodecInfo>();
    Parcel parcel;
    EXPECT_TRUE(bluetoothA2dpCodecInfo->Marshalling(parcel));
    EXPECT_TRUE(bluetoothA2dpCodecInfo->Unmarshalling(parcel));
    EXPECT_TRUE(bluetoothA2dpCodecInfo->WriteToParcel(parcel));
    EXPECT_TRUE(bluetoothA2dpCodecInfo->ReadFromParcel(parcel));
    HILOGI("bluetoothA2dpCodecInfo_WriteToParcelTest_00100 end");
}

}  // namespace bluetooth
}  // namespace OHOS