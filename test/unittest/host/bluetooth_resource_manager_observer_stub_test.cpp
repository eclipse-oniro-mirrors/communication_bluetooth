/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#include "bluetooth_resource_manager_observer_stub.h"
#include "bluetooth_sensing_info.h"
#include "bluetooth_log.h"
#include "ipc_types.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
class BluetoothResourceManagerObserverStubMock : public BluetoothResourceManagerObserverStub {
public:
    void OnSensingStateChanged(uint8_t eventId, const BluetoothSensingInfo &info) {}
    void OnBluetoothResourceDecision(uint8_t eventId, const BluetoothSensingInfo &info, uint32_t &result) {}
};

static BluetoothHost *host_;
static std::unique_ptr<BluetoothResourceManagerObserverStubMock> observerStub_ {nullptr};

const int TIME = 4;

class BluetoothResourceManagerObserverStubTest : public testing::Test {
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
        observerStub_ = std::make_unique<BluetoothResourceManagerObserverStubMock>();
    }
    void TearDown()
    {
        observerStub_ = nullptr;
    }
};

/*
 * @tc.number: Resourse_Manager_Observer_Stub_OnRemoteRequest_00100
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceManagerObserverStubTest, Resourse_Manager_Observer_OnRemoteRequest_00100, TestSize.Level1)
{
    HILOGI("Resourse_Manager_Observer_OnRemoteRequest_00100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    data.WriteInterfaceToken(BluetoothResourceManagerObserverStub::GetDescriptor());
    int ret = observerStub_->OnRemoteRequest(99, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
    HILOGI("Resourse_Manager_Observer_OnRemoteRequest_00100 end");
}

/*
 * @tc.number: Resourse_Manager_Observer_Stub_OnRemoteRequest_00101
 * @tc.name: OnRemoteRequest
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceManagerObserverStubTest, Resourse_Manager_Observer_Stub_OnRemoteRequest_00101,
    TestSize.Level1)
{
    HILOGI("Resourse_Manager_Observer_Stub_OnRemoteRequest_00101 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };
    int ret = observerStub_->OnRemoteRequest(
        BluetoothResourceManagerObserverInterfaceCode::BT_RESOURCE_MANAGER_OBSERVER_BUTT,
        data, reply, option);
    EXPECT_EQ(ret, 2900003);
    HILOGI("Resourse_Manager_Observer_Stub_OnRemoteRequest_00101 end");
}

/*
 * @tc.number: Resourse_Manager_Observer_OnSensingStateChangedInner_01
 * @tc.name: OnSensingStateChangedInner
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceManagerObserverStubTest,
    Resourse_Manager_Observer_OnSensingStateChangedInner_01, TestSize.Level1)
{
    HILOGI("Resourse_Manager_Observer_OnSensingStateChangedInner_01 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    uint8_t eventId = SPP_CLIENT_CONNECT_DONE;
    data.WriteUint8(eventId);
    data.WriteParcelable(nullptr);
    EXPECT_EQ(BluetoothResourceManagerObserverStub::OnSensingStateChangedInner(observerStub_.get(),
        data, reply), TRANSACTION_ERR);

    bluetooth::SensingInfo sensingInfo;
    BluetoothSensingInfo info(sensingInfo);
    data.WriteUint8(eventId);
    data.WriteParcelable(&info);

    EXPECT_EQ(BluetoothResourceManagerObserverStub::OnSensingStateChangedInner(observerStub_.get(),
        data, reply), NO_ERROR);

    HILOGI("Resourse_Manager_Observer_OnSensingStateChangedInner_01 end");
}

/*
 * @tc.number: Resourse_Manager_Observer_OnBluetoothResourceDecisionInner_01
 * @tc.name: OnBluetoothResourceDecisionInner
 * @tc.desc:
 */
HWTEST_F(BluetoothResourceManagerObserverStubTest,
    Resourse_Manager_Observer_OnBluetoothResourceDecisionInner_01, TestSize.Level1)
{
    HILOGI("Resourse_Manager_Observer_OnBluetoothResourceDecisionInner_01 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    uint8_t eventId = BLE_FASTEST_CONN_DECISION;
    bluetooth::SensingInfo sensingInfo;
    BluetoothSensingInfo info(sensingInfo);
    data.WriteUint8(eventId);
    data.WriteParcelable(&info);

    EXPECT_EQ(BluetoothResourceManagerObserverStub::OnBluetoothResourceDecisionInner(observerStub_.get(),
        data, reply), NO_ERROR);

    data.WriteUint8(eventId);
    data.WriteParcelable(nullptr);
    EXPECT_EQ(BluetoothResourceManagerObserverStub::OnBluetoothResourceDecisionInner(observerStub_.get(),
        data, reply), TRANSACTION_ERR);
    HILOGI("Resourse_Manager_Observer_OnBluetoothResourceDecisionInner_01 end");
}
}
}