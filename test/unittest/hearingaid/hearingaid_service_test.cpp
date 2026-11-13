/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "hearing_aid_service_test"
#endif

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "hearing_aid_service_impl.h"
#include "service_util.h"
#include "btcommon/timer_manager.h"
#include "profile_service_manager.h"
#include "hearing_aid_service.h"
#include "hearing_aid_service_impl_interface.h"

using namespace testing;
using namespace testing::ext;

class TestCallback : public ::bluetooth::hearing_aid::HearingAidCallbacks {
public:
    TestCallback() {};
    TestCallback(std::function<void(int)> stackMock):stackMock_(stackMock) {};
    std::function<void(int)> stackMock_;

    void OnConnectionState(::bluetooth::hearing_aid::ConnectionState state, const RawAddress &address) {};
    void OnDeviceAvailable(uint8_t capabilities, uint64_t hiSyncId, const RawAddress &address) {};
};

class HearingAidTestInterface : public ::bluetooth::hearing_aid::HearingAidInterface {
public:
    ~HearingAidTestInterface()
    {
        HILOGI("hearing aid test interface destoryed");
    };

    HearingAidTestInterface() {};
    ::bluetooth::hearing_aid::HearingAidCallbacks *callbacks_;

    /** Register the Hearing Aid callbacks */
    void Init(::bluetooth::hearing_aid::HearingAidCallbacks *callbacks)
    {
        HILOGI("hearing aid test interface destoryed");
        callbacks_ = callbacks;
    };

    /** Connect to Hearing Aid */
    void Connect(const RawAddress &address)
    {
        HILOGI("hearing aid test interface connect %{public}s", address.ToString().c_str());
    };

    /** Disconnect from Hearing Aid */
    void Disconnect(const RawAddress &address)
    {
        HILOGI("hearing aid test interface disconnect %{public}s", address.ToString().c_str());
    };

    /** Add a hearing aid device to acceptlist */
    void AddToAcceptlist(const RawAddress &address)
    {
        HILOGI("hearing aid test interface addToAcceptlist %{public}s", address.ToString().c_str());
    };

    /** Set the volume */
    void SetVolume(int8_t volume)
    {
        HILOGI("hearing aid test interface SetVolume %{public}d", volume);
    }

    /** Set the volume by device address */
    MOCK_METHOD(void, SetVolumeByAddr, (const RawAddress &address, int volume));

    void SetVolumeByAddr(const RawAddress &address, int8_t volume)
    {
        HILOGI("hearing aid test interface SetVolumeByAddr %{public}s", address.ToString().c_str());
    };

    /** Start hearing aid audio session by deivice address */
    void StartSendingAudio(const RawAddress &address)
    {
        HILOGI("hearing aid test interface startSendingAudio %{public}s", address.ToString().c_str());
    };

    /** Closes the interface. */
    void Cleanup(void)
    {
        HILOGI("hearing aid test interface cleanup");
    };

    /* Called when Hearing Aid is unbonded. */
    void RemoveDevice(const RawAddress &address)
    {
        HILOGI("hearing aid test interface removeDevice %{public}s", address.ToString().c_str());
    };
};

namespace OHOS {
namespace bluetooth {
using namespace std;

constexpr int TIME = 4;

static HearingAidServiceData *g_data = nullptr;
HearingAidService *hearingAidService_ = nullptr;

class HearingAidServiceImplTestInterface : public HearingAidServiceImplInterface {
public:
    HearingAidServiceImplTestInterface()
    {};
    virtual ~HearingAidServiceImplTestInterface()
    {};

    virtual void SetGetDeviceNameFunc(std::function<std::string(const RawAddress &device)> func)
    {};

    virtual void SetGetAllowConnectFunc(std::function<bool(const RawAddress &device)> func)
    {};

    virtual void SetServiceDataFunc(std::function<HearingAidServiceData&()> func)
    {};

    virtual void SetServicePosteventFunc(std::function<void(const HearingAidMessage &event)> func)
    {};
    virtual void SetGetStorageVolumeFunc(std::function<bool(const std::string &addr, int &volume)> func)
    {};

    virtual int SetActiveDevice(const RawAddress &device, HearingAidServiceData &data)
    {
        return 0;
    };

    virtual void ProcessEvent(const HearingAidMessage &event, HearingAidServiceData &data)
    {};
};

class HearingaidServiceTest : public testing::Test {
public:
    HearingaidServiceTest();
    ~HearingaidServiceTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
};

HearingaidServiceTest::HearingaidServiceTest()
{
}

HearingaidServiceTest::~HearingaidServiceTest()
{
}

void HearingaidServiceTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    HILOGI("SetUpTestCase end");
}

void HearingaidServiceTest::TearDownTestCase(void)
{
    HILOGI("TearDownTestCase start");
    utility::CloseAllTimer();
    HILOGI("TearDownTestCase end");
}

void HearingaidServiceTest::SetUp()
{
    auto manager = ProfileServiceManager::GetInstance();
    manager->SetProfileService(BTTransport::ADAPTER_BREDR, PROFILE_NAME_HEARING_AID);
    hearingAidService_ = static_cast<HearingAidService *>(manager->GetProfileService(PROFILE_NAME_HEARING_AID));
    std::shared_ptr<HearingAidServiceImplTestInterface> hearingAidServiceImpl =
        std::make_shared<HearingAidServiceImplTestInterface>();
    hearingAidService_->serviceImpl_ = std::shared_ptr<HearingAidServiceImplInterface>(hearingAidServiceImpl);
    g_data = new HearingAidServiceData;
    HearingAidTestInterface *interface = new HearingAidTestInterface();
    TestCallback *callback = new TestCallback();
    interface->Init(callback);
    g_data->bluetoothHearingAidInterface_ = interface;
    HearingAidServiceImpl *serviceImpl = new HearingAidServiceImpl();
    HearingAidServiceImpl::SetServiceImpl(serviceImpl);

    HearingAidServiceImpl::GetServiceImpl()->SetServicePosteventFunc([&](const HearingAidMessage &event) {
        HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event, *g_data);
    });
    HearingAidServiceImpl::GetServiceImpl()->SetGetDeviceNameFunc(
        [&](const RawAddress audioInformAddress)->std::string {
        return std::string("testDevice");
    });
    HearingAidServiceImpl::GetServiceImpl()->SetGetStorageVolumeFunc([&](const std::string &addr, int &volume) -> bool {
        volume = -125;
        return true;
    });
    HearingAidServiceImpl::GetServiceImpl()->SetGetAllowConnectFunc(
        [&](const RawAddress &device) -> bool { return true; });
    HearingAidServiceImpl::GetServiceImpl()->SetServiceDataFunc([&]() -> HearingAidServiceData& { return *g_data; });
}

void HearingaidServiceTest::TearDown()
{
    delete hearingAidService_;
    hearingAidService_ = nullptr;
    delete(g_data->bluetoothHearingAidInterface_);
    delete(g_data);
    sleep(1);
}
/*
 * @tc.number: Hearingaid_ModuleTest_Connect_00001
 * @tc.name: Connect
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_Connect_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Connect_00001 start");
    std::string validAddress = "00:11:22:33:44:55";

    // 下发connect
    HearingAidMessage event1(HEARING_AID_CONNECT_EVT, validAddress);
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event1, *g_data);
    // 第一步处理完成时进入connecting
    int ret = g_data->stateCache_.GetDeviceStateCache(validAddress);
    EXPECT_EQ(ret, HEARING_AID_STATE_CONNECTING);
    // 模拟device avliable
    RawAddress rawAddr(validAddress);
    STACK::RawAddress addr = ServiceUtil::AddrToStack(rawAddr);
    g_data->mDeviceCapabilitiesMap_.EnsureInsert(rawAddr.GetAddress(), 1);
    g_data->mDeviceHiSyncIdMap_.EnsureInsert(rawAddr.GetAddress(), 1);
    // 回调connected
    EXPECT_CALL(
        *static_cast<HearingAidTestInterface *>(g_data->bluetoothHearingAidInterface_), SetVolumeByAddr(addr, -125));
    HearingAidMessage event2(
        HEARING_AID_STACK_STATE_EVT, validAddress, static_cast<int>(BTHEARING_AID_STATE_CONNECTED));
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event2, *g_data);
    // 获取connected
    ret = g_data->stateCache_.GetDeviceStateCache(validAddress);
    EXPECT_EQ(ret, HEARING_AID_STATE_CONNECTED);
    HILOGI("Hearingaid_ModuleTest_Connect_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_Connect_00002
 * @tc.name: Connect
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_Connect_00002, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Connect_00002 start");
    std::string validAddress = "00:11:22:33:44:55";

    // 下发connect
    HearingAidMessage event1(HEARING_AID_CONNECT_EVT, validAddress);
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event1, *g_data);
    // 第一步处理完成时进入connecting
    int ret = g_data->stateCache_.GetDeviceStateCache(validAddress);
    EXPECT_EQ(ret, HEARING_AID_STATE_CONNECTING);
    // 触发timeout
    HearingAidMessage event2(HEARING_AID_CONNECT_TIMEOUT_EVT, validAddress);
    HearingAidServiceImpl::GetServiceImpl()->PostEventToService(event2);
    // 进入disconnecting
    ret = g_data->stateCache_.GetDeviceStateCache(validAddress);
    EXPECT_EQ(ret, HEARING_AID_STATE_DISCONNECTING);
    HILOGI("Hearingaid_ModuleTest_Connect_00002 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_Connect_00003
 * @tc.name: Connect
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_Connect_00003, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Connect_00003 start");
    std::string validAddress = "00:11:22:33:44:55";

    // 下发connect
    HearingAidMessage event1(HEARING_AID_CONNECT_EVT, validAddress);
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event1, *g_data);
    // 第一步处理完成时进入connecting
    int ret = g_data->stateCache_.GetDeviceStateCache(validAddress);
    EXPECT_EQ(ret, HEARING_AID_STATE_CONNECTING);
    // 等31s超时触发timeout
    sleep(31);
    // 进入disconnecting
    ret = g_data->stateCache_.GetDeviceStateCache(validAddress);
    EXPECT_EQ(ret, HEARING_AID_STATE_DISCONNECTING);
    // 手动移除timer 不然ffrt线程会crash
    auto it = g_data->stateMachines_.find(validAddress);
    EXPECT_EQ(it != g_data->stateMachines_.end() && it->second != nullptr && !it->second->IsRemoving(), true);
    if (it != g_data->stateMachines_.end() && it->second != nullptr && !it->second->IsRemoving()) {
        it->second->StopConnectionTimer();
    }
    HILOGI("Hearingaid_ModuleTest_Connect_00003 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_Connect_00004
 * @tc.name: Connect
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_Connect_00004, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Connect_00004 start");
    std::string validAddress = "00:11:22:33:44:55";
    RawAddress device(validAddress);
    EXPECT_EQ(hearingAidService_->Connect(device), HEARING_AID_FAILURE);
    HILOGI("Hearingaid_ModuleTest_Connect_00004 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_Connect_00005
 * @tc.name: Connect
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_Connect_00005, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_Connect_00005 start");
    std::string validAddress = "11:22:33:44:55:66";

    // 下发connect
    HearingAidMessage event1(HEARING_AID_CONNECT_EVT, validAddress);
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event1, *g_data);
    // 第一步处理完成时进入connecting
    int ret = g_data->stateCache_.GetDeviceStateCache(validAddress);
    EXPECT_EQ(ret, HEARING_AID_STATE_CONNECTING);
    HearingAidServiceImpl::GetServiceImpl()->SetGetStorageVolumeFunc([&](const std::string &addr, int &volume) -> bool {
        return false;
    });
    // 模拟device avliable
    RawAddress rawAddr(validAddress);
    STACK::RawAddress addr = ServiceUtil::AddrToStack(rawAddr);
    g_data->mDeviceCapabilitiesMap_.EnsureInsert(rawAddr.GetAddress(), 1);
    g_data->mDeviceHiSyncIdMap_.EnsureInsert(rawAddr.GetAddress(), 1);
    // 回调connected
    EXPECT_CALL(
        *static_cast<HearingAidTestInterface *>(g_data->bluetoothHearingAidInterface_), SetVolumeByAddr(addr, -64));
    HearingAidMessage event2(
        HEARING_AID_STACK_STATE_EVT, validAddress, static_cast<int>(BTHEARING_AID_STATE_CONNECTED));
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event2, *g_data);
    // 获取connected
    ret = g_data->stateCache_.GetDeviceStateCache(validAddress);
    EXPECT_EQ(ret, HEARING_AID_STATE_CONNECTED);
    HILOGI("Hearingaid_ModuleTest_Connect_00005 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_SetVolume_00001
 * @tc.name: SetVloume
 * @tc.desc: Normal input with volume
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_SetVolume_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetVolume_00001 start");
    std::string validAddress = "00:11:22:33:44:55";

    // 模拟device avliable
    RawAddress rawAddr(validAddress);
    STACK::RawAddress addr = ServiceUtil::AddrToStack(rawAddr);
    g_data->mDeviceCapabilitiesMap_.EnsureInsert(rawAddr.GetAddress(), 1);
    g_data->mDeviceHiSyncIdMap_.EnsureInsert(rawAddr.GetAddress(), 1);
    // 回调connected
    HearingAidMessage event1(
        HEARING_AID_STACK_STATE_EVT, validAddress, static_cast<int>(BTHEARING_AID_STATE_CONNECTED));
    EXPECT_CALL(
        *static_cast<HearingAidTestInterface *>(g_data->bluetoothHearingAidInterface_), SetVolumeByAddr(addr, -125));
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event1, *g_data);

    HILOGI("Hearingaid_ModuleTest_SetVolume_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetConnectDevices_00001
 * @tc.name: GetConnectDevices
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_GetConnectDevices_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetConnectDevices_00001 start");
    EXPECT_EQ(hearingAidService_->GetConnectDevices().size(), 0);
    HILOGI("Hearingaid_ModuleTest_GetConnectDevices_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetConnectState_00001
 * @tc.name: GetConnectState
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_GetConnectState_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetConnectState_00001 start");
    EXPECT_EQ(hearingAidService_->GetConnectState(), 0);
    HILOGI("Hearingaid_ModuleTest_GetConnectState_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetDevicesByStates_00001
 * @tc.name: GetDevicesByStates
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_GetDevicesByStates_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetDevicesByStates_00001 start");
    std::vector<int> states = {0, 1, 2};
    EXPECT_EQ(hearingAidService_->GetDevicesByStates(states).size(), 0);
    HILOGI("Hearingaid_ModuleTest_GetDevicesByStates_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetDeviceState_00001
 * @tc.name: GetDeviceState
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_GetDeviceState_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetDeviceState_00001 start");
    std::string validAddress = "00:11:22:33:44:55";
    RawAddress device(validAddress);
    EXPECT_NE(hearingAidService_->GetDeviceState(device), HEARING_AID_STATE_CONNECTED);
    HILOGI("Hearingaid_ModuleTest_GetDeviceState_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_SetActiveDevice_00001
 * @tc.name: SetActiveDevice
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_SetActiveDevice_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetActiveDevice_00001 start");
    std::string validAddress = "00:11:22:33:44:55";

    // 模拟device avliable
    RawAddress rawAddr(validAddress);
    g_data->mDeviceCapabilitiesMap_.EnsureInsert(rawAddr.GetAddress(), 1);
    g_data->mDeviceHiSyncIdMap_.EnsureInsert(rawAddr.GetAddress(), 1);

    // 设置active device
    HearingAidMessage event1(HEARING_AID_SET_ACTIVE_DEVICE, validAddress);
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event1, *g_data);

    EXPECT_EQ(g_data->mActiveDeviceHiSyncId_ == 1, true);
    std::string name;
    hearingAidService_->GetActiveDevice(rawAddr, name);
    HILOGI("Hearingaid_ModuleTest_SetActiveDevice_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_SetActiveDevice_00002
 * @tc.name: SetActiveDevice
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_SetActiveDevice_00002, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetActiveDevice_00002 start");
    std::string leftAddress = "00:11:22:33:44:55";
    std::string rightAddress = "00:11:22:33:44:66";

    // 模拟device avliable
    RawAddress rawAddr(leftAddress);
    g_data->mDeviceCapabilitiesMap_.EnsureInsert(rawAddr.GetAddress(), 1);
    g_data->mDeviceHiSyncIdMap_.EnsureInsert(rawAddr.GetAddress(), 1);

    RawAddress rawAddr1(rightAddress);
    g_data->mDeviceCapabilitiesMap_.EnsureInsert(rawAddr1.GetAddress(), 2);
    g_data->mDeviceHiSyncIdMap_.EnsureInsert(rawAddr1.GetAddress(), 1);

    // 设置active device
    HearingAidMessage event1(HEARING_AID_SET_ACTIVE_DEVICE, leftAddress);
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event1, *g_data);

    EXPECT_EQ(g_data->mActiveDeviceHiSyncId_ == 1, true);

    // 设置active device
    HearingAidMessage event2(HEARING_AID_SET_ACTIVE_DEVICE, rightAddress);
    HearingAidServiceImpl::GetServiceImpl()->ProcessEvent(event2, *g_data);

    EXPECT_EQ(g_data->mActiveDeviceHiSyncId_ == 1, true);
    HILOGI("Hearingaid_ModuleTest_SetActiveDevice_00002 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetVolume_00001
 * @tc.name: GetVolume
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_GetVolume_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetVolume_00001 start");
    std::string validAddress = "00:11:22:33:44:55";
    int volume = 0;
    EXPECT_FALSE(hearingAidService_->GetVolume(validAddress, volume));
    HILOGI("Hearingaid_ModuleTest_GetVolume_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetHearingAidDeviceInfo_00001
 * @tc.name: GetHearingAidDeviceInfo
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_GetHearingAidDeviceInfo_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetHearingAidDeviceInfo_00001 start");
    std::string validAddress = "00:11:22:33:44:55";
    RawAddress device(validAddress);
    int side = 1;
    int mode = 1;
    int64_t hiSyncId = 1;
    EXPECT_EQ(hearingAidService_->GetHearingAidDeviceInfo(device, side, mode, hiSyncId), HEARING_AID_FAILURE);
    HILOGI("Hearingaid_ModuleTest_GetHearingAidDeviceInfo_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_SetConnectStrategy_00001
 * @tc.name: SetConnectStrategy
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_SetConnectStrategy_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_SetConnectStrategy_00001 start");
    std::string validAddress = "00:11:22:33:44:55";
    RawAddress device(validAddress);
    int strategy = 0;
    EXPECT_EQ(hearingAidService_->SetConnectStrategy(device, strategy), Bluetooth::BT_ERR_INTERNAL_ERROR);
    strategy = 1;
    EXPECT_EQ(hearingAidService_->SetConnectStrategy(device, strategy), Bluetooth::BT_ERR_INTERNAL_ERROR);
    strategy = 2;
    EXPECT_EQ(hearingAidService_->SetConnectStrategy(device, strategy), Bluetooth::BT_ERR_INTERNAL_ERROR);
    hearingAidService_->GetConnectStrategy(device);
    HILOGI("Hearingaid_ModuleTest_SetConnectStrategy_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_AllowConnect_00001
 * @tc.name: AllowConnect
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_AllowConnect_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_AllowConnect_00001 start");
    std::string validAddress = "00:11:22:33:44:55";
    RawAddress device(validAddress);
    EXPECT_FALSE(hearingAidService_->AllowConnect(device));
    HILOGI("Hearingaid_ModuleTest_AllowConnect_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_ConnectAllDevice_00001
 * @tc.name: ConnectAllDevice
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_ConnectAllDevice_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_ConnectAllDevice_00001 start");
    std::string validAddress = "00:11:22:33:44:55";
    EXPECT_EQ(hearingAidService_->ConnectAllDevice(validAddress), HEARING_AID_FAILURE);
    HILOGI("Hearingaid_ModuleTest_ConnectAllDevice_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_IsLeftDevice_00001
 * @tc.name: IsLeftDevice
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_IsLeftDevice_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_IsLeftDevice_00001 start");
    std::string validAddress = "00:11:22:33:44:55";
    bool isLeft = false;
    EXPECT_FALSE(hearingAidService_->IsLeftDevice(isLeft, validAddress));
    HILOGI("Hearingaid_ModuleTest_IsLeftDevice_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_ShutDownDone_00001
 * @tc.name: ShutDownDone
 * @tc.desc:
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_ShutDownDone_00001, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_ShutDownDone_00001 start");
    hearingAidService_->StartUp();
    hearingAidService_->StartStackProfile();
    hearingAidService_->ShutDownDone(true);
    EXPECT_FALSE(hearingAidService_->serviceData_.isInit_);
    HILOGI("Hearingaid_ModuleTest_ShutDownDone_00001 end");
}

/*
 * @tc.number: Hearingaid_ModuleTest_GetVolume_00002
 * @tc.name: GetVolume
 * @tc.desc: Normal input with valid address
 */
HWTEST_F(HearingaidServiceTest, Hearingaid_ModuleTest_GetVolume_00002, TestSize.Level1)
{
    HILOGI("Hearingaid_ModuleTest_GetVolume_00002 start");
    std::string validAddress = "AD:13:42:B3:4C:E5";
    int volume = 0;
    EXPECT_FALSE(hearingAidService_->GetVolume(validAddress, volume));
    HILOGI("Hearingaid_ModuleTest_GetVolume_00002 end");
}
}  // namespace Bluetooth
}  // namespace OHOS