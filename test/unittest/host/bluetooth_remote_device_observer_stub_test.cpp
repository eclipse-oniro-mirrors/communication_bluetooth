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
#include <cstring>
#include "bluetooth_host.h"
#include "bluetooth_remote_device_observer_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

class BluetoothRemoteDeviceObserverStubMock : public BluetoothRemoteDeviceObserverstub {
public:
    void OnAclStateChanged(const BluetoothRawAddress &device, int32_t state, uint32_t reason) {}
    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device,
        int32_t status, int32_t cause, const std::string &causeMessage) {}
    void OnRemoteUuidChanged(const BluetoothRawAddress &device, const std::vector<bluetooth::Uuid> uuids) {}
    void OnRemoteNameChanged(const BluetoothRawAddress &device, const std::string deviceName) {}
    void OnRemoteAliasChanged(const BluetoothRawAddress &device, const std::string alias) {}
    void OnRemoteCodChanged(const BluetoothRawAddress &device, int32_t cod) {}
    void OnRemoteBatteryChanged(const BluetoothRawAddress &device, const BluetoothBatteryInfo &batteryInfo) {}
    void OnRemoteDeviceCommonInfoReport(const BluetoothRawAddress &device, const std::vector<uint8_t> &value) {}
};

static BluetoothHost *host_;
static std::unique_ptr<BluetoothRemoteDeviceObserverStubMock> observerStub_ {nullptr};

const int TIME = 4;

class BluetoothRemoteDeviceObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase()
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

        HILOGI("SetUpTestCase end");
    }
    static void TearDownTestCase()
    {
        HILOGI("TearDownTestCase");
    }
    void SetUp()
    {
        observerStub_ = std::make_unique<BluetoothRemoteDeviceObserverStubMock>();
    }
    void TearDown()
    {
        observerStub_ = nullptr;
    }
};

/*
 * @tc.number: Remote_Device_Observer_OnAclStateChangedInner_00100
 * @tc.name: OnAclStateChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnAclStateChangedInner_00100, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnAclStateChangedInner_00100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    data.WriteInterfaceToken(BluetoothRemoteDeviceObserverstub::GetDescriptor());
    observerStub_->OnRemoteRequest(99, data, reply, option);

    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnAclStateChangedInner(observerStub_.get(),
        data, reply), TRANSACTION_ERR);

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnAclStateChangedInner(observerStub_.get(),
        data, reply), NO_ERROR);

    HILOGI("Remote_Device_Observer_OnAclStateChangedInner_00100 end");
}

/*
 * @tc.number: Remote_Device_Observer_OnRemoteNameUuidChangedInner_01
 * @tc.name: OnRemoteNameUuidChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnRemoteNameUuidChangedInner_01, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnRemoteNameUuidChangedInner_01 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteNameUuidChangedInner(observerStub_.get(),
        data, reply), TRANSACTION_ERR);

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    data.WriteUint16(1025);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteNameUuidChangedInner(observerStub_.get(),
        data, reply), INVALID_DATA);

    HILOGI("Remote_Device_Observer_OnRemoteNameUuidChangedInner_01 end");
}

/*
 * @tc.number: Remote_Device_Observer_OnRemoteNameUuidChangedInner_02
 * @tc.name: OnRemoteNameUuidChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnRemoteNameUuidChangedInner_02, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnRemoteNameUuidChangedInner_02 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    data.WriteUint16(1);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteNameUuidChangedInner(observerStub_.get(),
        data, reply), TRANSACTION_ERR);

    HILOGI("Remote_Device_Observer_OnRemoteNameUuidChangedInner_02 end");
}

/*
 * @tc.number: Remote_Device_Observer_OnRemoteNameUuidChangedInner_03
 * @tc.name: OnRemoteNameUuidChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnRemoteNameUuidChangedInner_03, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnRemoteNameUuidChangedInner_03 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    data.WriteUint16(1);
    BluetoothUuid btUuid;
    data.WriteParcelable(&btUuid);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteNameUuidChangedInner(observerStub_.get(),
        data, reply), NO_ERROR);

    HILOGI("Remote_Device_Observer_OnRemoteNameUuidChangedInner_03 end");
}

/*
 * @tc.number: Remote_Device_Observer_OnRemoteNameChangedInner_01
 * @tc.name: OnRemoteNameChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnRemoteNameChangedInner_01, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnRemoteNameChangedInner_01 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteNameChangedInner(observerStub_.get(),
        data, reply), NO_ERROR);

    HILOGI("Remote_Device_Observer_OnRemoteNameChangedInner_01 end");
}

/*
 * @tc.number: Remote_Device_Observer_OnRemoteAliasChangedInner_01
 * @tc.name: OnRemoteAliasChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnRemoteAliasChangedInner_01, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnRemoteAliasChangedInner_01 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteAliasChangedInner(observerStub_.get(),
        data, reply), TRANSACTION_ERR);

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteAliasChangedInner(observerStub_.get(),
        data, reply), NO_ERROR);

    HILOGI("Remote_Device_Observer_OnRemoteAliasChangedInner_01 end");
}

/*
 * @tc.number: Remote_Device_Observer_OnRemoteCodChangedInner_01
 * @tc.name: OnRemoteCodChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnRemoteCodChangedInner_01, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnRemoteCodChangedInner_01 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteCodChangedInner(observerStub_.get(),
        data, reply), TRANSACTION_ERR);

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteCodChangedInner(observerStub_.get(),
        data, reply), NO_ERROR);

    HILOGI("Remote_Device_Observer_OnRemoteCodChangedInner_01 end");
}

/*
 * @tc.number: Remote_Device_Observer_OnRemoteBatteryChangedInner_01
 * @tc.name: OnRemoteBatteryChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnRemoteBatteryChangedInner_01, TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnRemoteBatteryChangedInner_01 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteBatteryChangedInner(observerStub_.get(),
        data, reply), BT_ERR_INTERNAL_ERROR);

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    BluetoothBatteryInfo batteryInfo;
    data.WriteParcelable(&batteryInfo);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteBatteryChangedInner(observerStub_.get(),
        data, reply), NO_ERROR);
    HILOGI("Remote_Device_Observer_OnRemoteBatteryChangedInner_01 end");
}

/*
 * @tc.number: Remote_Device_Observer_OnRemoteDeviceCommonInfoReportInner_01
 * @tc.name: OnRemoteDeviceCommonInfoReportInner
 * @tc.desc:
 */
HWTEST_F(BluetoothRemoteDeviceObserverStubTest, Remote_Device_Observer_OnRemoteDeviceCommonInfoReportInner_01,
    TestSize.Level1)
{
    HILOGI("Remote_Device_Observer_OnRemoteDeviceCommonInfoReportInner_01 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteDeviceCommonInfoReportInner(observerStub_.get(),
        data, reply), BT_ERR_INTERNAL_ERROR);

    BluetoothRawAddress address;
    data.WriteParcelable(&address);
    std::vector<uint8_t> value = { 0 };
    data.WriteUInt8Vector(value);
    EXPECT_EQ(BluetoothRemoteDeviceObserverstub::OnRemoteDeviceCommonInfoReportInner(observerStub_.get(),
        data, reply), NO_ERROR);
    HILOGI("Remote_Device_Observer_OnRemoteDeviceCommonInfoReportInner_01 end");
}

}  // namespace bluetooth
}  // namespace OHOS