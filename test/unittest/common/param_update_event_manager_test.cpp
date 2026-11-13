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
#include "param_update_event_manager.h"
#include "log.h"
#include "bt_def.h"
#include "thread_util.h"
#include "adapter_manager.h"
#include "sys_state_machine.h"
#include "hw_bt_hwif.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

std::shared_ptr<ParamUpdateEventManager> g_paramUpdateEventManager = nullptr;

class ParamUpdateEventManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {
        g_paramUpdateEventManager = std::make_shared<ParamUpdateEventManager>();
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_ADAPTER_MANAGER, ThreadUtil::ThreadState::DISABLED);
        bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    }
    void TearDown()
    {
        g_paramUpdateEventManager = nullptr;
        AdapterManager::GetInstance()->setBluetoothInterface(bluetoothInterface);
    }
private:
    bt_interface_t* bluetoothInterface = nullptr;
};

/*
 * @tc.number:param_update_event_manager_test_001
 * @tc.name: SubscriberEvent
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_001, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_001 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    g_paramUpdateEventManager->SubscriberEvent();
    HILOGI("param_update_event_manager_test_001 end");
}

/*
 * @tc.number:param_update_event_manager_test_002
 * @tc.name: SubscriberEvent
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_002, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_002 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    g_paramUpdateEventManager->SubscriberEvent();
    g_paramUpdateEventManager->SubscriberEvent();
    HILOGI("param_update_event_manager_test_002 end");
}

/*
 * @tc.number:param_update_event_manager_test_003
 * @tc.name: UnSubscriberEvent
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_003, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_003 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    g_paramUpdateEventManager->UnSubscriberEvent();
    HILOGI("param_update_event_manager_test_003 end");
}

/*
 * @tc.number:param_update_event_manager_test_004
 * @tc.name: UnSubscriberEvent
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_004, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_004 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    // libcesfwk_core.z.so cficheck 导致crash 暂时屏蔽SubscriberEvent();
    // libcesfwk_core.z.so cficheck 导致crash 暂时屏蔽UnSubscriberEvent();
    HILOGI("param_update_event_manager_test_004 end");
}

/*
 * @tc.number:param_update_event_manager_test_005
 * @tc.name: OnReceiveEvent
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_005, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_005 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    const AAFwk::Want want;
    g_paramUpdateEventManager->OnReceiveEvent(want);
    HILOGI("param_update_event_manager_test_005 end");
}

/*
 * @tc.number:param_update_event_manager_test_006
 * @tc.name: HandleParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_006, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_006 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    AdapterManager::GetInstance()->CreateAdapters();
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    BTStateID state = STATE_TURN_OFF;
    AdapterManager::GetInstance()->UpdateBluetoothState(transport, state);
    transport = BTTransport::ADAPTER_BLE;
    AdapterManager::GetInstance()->UpdateBluetoothState(transport, state);
    g_paramUpdateEventManager->HandleParamUpdate();
    HILOGI("param_update_event_manager_test_006 end");
}

/*
 * @tc.number:param_update_event_manager_test_007
 * @tc.name: HandleParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_007, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_007 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    AdapterManager::GetInstance()->CreateAdapters();
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    BTStateID state = STATE_TURN_ON;
    AdapterManager::GetInstance()->UpdateBluetoothState(transport, state);
    transport = BTTransport::ADAPTER_BLE;
    AdapterManager::GetInstance()->UpdateBluetoothState(transport, state);
    g_paramUpdateEventManager->HandleParamUpdate();
    HILOGI("param_update_event_manager_test_007 end");
}

/*
 * @tc.number:param_update_event_manager_test_008
 * @tc.name: HandleParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_008, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_008 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    AdapterManager::GetInstance()->CreateAdapters();
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    BTStateID state = STATE_TURN_ON;
    AdapterManager::GetInstance()->UpdateBluetoothState(transport, state);
    transport = BTTransport::ADAPTER_BLE;
    AdapterManager::GetInstance()->UpdateBluetoothState(transport, state);
    AdapterManager::GetInstance()->OnSysStateChange(SYS_STATE_STARTED);

    g_paramUpdateEventManager->HandleParamUpdate();
    HILOGI("param_update_event_manager_test_008 end");
}

/*
 * @tc.number:param_update_event_manager_test_009
 * @tc.name: HandleParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_009, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_009 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    AdapterManager::GetInstance()->CreateAdapters();
    BTTransport transport = BTTransport::ADAPTER_BREDR;
    BTStateID state = STATE_TURN_ON;
    AdapterManager::GetInstance()->UpdateBluetoothState(transport, state);
    transport = BTTransport::ADAPTER_BLE;
    AdapterManager::GetInstance()->UpdateBluetoothState(transport, state);
    AdapterManager::GetInstance()->OnSysStateChange(SYS_STATE_STARTED);

    g_paramUpdateEventManager->HandleParamUpdate();
    g_paramUpdateEventManager->HandleParamUpdate();
    HILOGI("param_update_event_manager_test_009 end");
}

/*
 * @tc.number:param_update_event_manager_test_010
 * @tc.name: DoBtParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_010, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_010 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    AdapterManager::GetInstance()->setBluetoothInterface(nullptr);
    g_paramUpdateEventManager->DoBtParamUpdate();
    HILOGI("param_update_event_manager_test_010 end");
}

/*
 * @tc.number:param_update_event_manager_test_011
 * @tc.name: DoBtParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_011, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_011 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    bt_interface_t btInterface = {
        .get_profile_interface = nullptr,
    };
    AdapterManager::GetInstance()->setBluetoothInterface(&btInterface);

    g_paramUpdateEventManager->DoBtParamUpdate();
    HILOGI("param_update_event_manager_test_011 end");
}

static const void* GetProfileInterfaceMock1(const char* profileId)
{
    HILOGI("enter");
    return nullptr;
}

/*
 * @tc.number:param_update_event_manager_test_012
 * @tc.name: DoBtParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_012, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_012 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    bt_interface_t btInterface = {
        .get_profile_interface = GetProfileInterfaceMock1,
    };
    AdapterManager::GetInstance()->setBluetoothInterface(&btInterface);

    g_paramUpdateEventManager->DoBtParamUpdate();
    HILOGI("param_update_event_manager_test_012 end");
}

static int HwDynamicConfigMock()
{
    HILOGI("enter");
    return 0;
}

bthwif_interface_t g_bthwInterface1 = {
    .hwDynamicConfig = nullptr,
};

bthwif_interface_t g_bthwInterface2 = {
    .hwDynamicConfig = HwDynamicConfigMock,
};

static const void* GetProfileInterfaceMock2(const char* profileId)
{
    HILOGI("enter");
    return (void *)(&g_bthwInterface1);
}

static const void* GetProfileInterfaceMock3(const char* profileId)
{
    HILOGI("enter");
    return (void *)(&g_bthwInterface2);
}

/*
 * @tc.number:param_update_event_manager_test_013
 * @tc.name: DoBtParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_013, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_013 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    bt_interface_t btInterface = {
        .get_profile_interface = GetProfileInterfaceMock2,
    };
    AdapterManager::GetInstance()->setBluetoothInterface(&btInterface);

    g_paramUpdateEventManager->DoBtParamUpdate();
    HILOGI("param_update_event_manager_test_013 end");
}

/*
 * @tc.number:param_update_event_manager_test_014
 * @tc.name: DoBtParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_014, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_014 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    bt_interface_t btInterface = {
        .get_profile_interface = GetProfileInterfaceMock3,
    };
    AdapterManager::GetInstance()->setBluetoothInterface(&btInterface);

    g_paramUpdateEventManager->DoBtParamUpdate();
    HILOGI("param_update_event_manager_test_014 end");
}

/*
 * @tc.number:param_update_event_manager_test_015
 * @tc.name: CheckIdleStateToTryParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_015, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_015 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    AdapterManager::GetInstance()->AdapterStop();
    g_paramUpdateEventManager->CheckIdleStateToTryParamUpdate();
    HILOGI("param_update_event_manager_test_015 end");
}

/*
 * @tc.number:param_update_event_manager_test_016
 * @tc.name: CheckIdleStateToTryParamUpdate
 * @tc.desc:
 */
HWTEST_F(ParamUpdateEventManagerTest, param_update_event_manager_test_016, TestSize.Level1)
{
    HILOGI("param_update_event_manager_test_016 enter");
    EXPECT_TRUE(g_paramUpdateEventManager != nullptr);
    AdapterManager::GetInstance()->CreateAdapters();
    g_paramUpdateEventManager->CheckIdleStateToTryParamUpdate();
    HILOGI("param_update_event_manager_test_016 end");
}
}  // namespace bluetooth
}  // namespace OHOS