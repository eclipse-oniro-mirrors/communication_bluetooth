/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_bas"
#endif

#include "bas_service.h"

#include "adapter_manager.h"
#include "bas_defines.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_interface.h"
#include "class_creator.h"
#include "common_util.h"
#include "connect_strategy_manager.h"
#include "bt_chr_ue_manager.h"
#include "bt_chr_dft_exception.h"
#include "permission_manager.h"
#include "hitrace_meter.h"
#include "control_intercept_plugin.h"
#include "interface_adapter_manager.h"
#include "ipc_skeleton.h"
#include "bluetooth_common_event_helper.h"
#include "bluetooth_connection_manager.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "remote_device_properties.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
namespace {
constexpr char const *DEFAULT_LIB_NAME = "libbtbas.z.so";
constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreateBasServiceImplInterface";
constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyBasServiceImplInterface";
constexpr uint32_t DEFAULT_UNLOAD_TIMER_MS = 600000;
};

class BasService::BasObserver : public IBasObserver {
public:
    explicit BasObserver() = default;
    ~BasObserver() override = default;

    void OnGetBatteryLevelEvent(const RawAddress &device, int32_t batteryLevel) override
    {
        auto service = BasService::GetService();
        if (service != nullptr) {
            service->NotifyReadBatteryLevelEvent(device, batteryLevel);
        }
    }

    void OnBatteryLevelChanged(const RawAddress &device, int32_t batteryLevel) override
    {
        auto service = BasService::GetService();
        if (service != nullptr) {
            service->NotifyBatteryLevelChanged(device, batteryLevel);
        }
    }
};

class BasService::BasGattClientObserver : public BluetoothGattInterface::GattClientObserver {
public:
    explicit BasGattClientObserver() = default;
    ~BasGattClientObserver() override = default;

    void ConnStateChangedCallback(int clientIf, const STACK::RawAddress &bda,
        int connId, bool connected, int reason) override
    {
        HILOGI("clientIf=%{public}d, bda=%{public}s, connId=%{public}d, connected=%{public}d, reason=%{public}d",
            clientIf, GetEncryptAddr(bda.ToString()).c_str(), connId, connected, reason);
        DoInBleThread([device = ServiceUtil::AddrFromStack(bda), clientIf, connected] () {
            auto service = BasService::GetService();
            if (service != nullptr) {
                int state = connected ? static_cast<int>(BTConnectState::CONNECTED) :
                    static_cast<int>(BTConnectState::DISCONNECTED);
                service->HandleGattStateChanged(device, clientIf, state);
            }
        });
    }
};

BasService::BasService() : utility::Context(PROFILE_NAME_BAS, "1.1.1"),
    loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS, DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME)
{
    gattObserverImpl_ = std::make_shared<BasGattClientObserver>();
    basObserverImpl_ = std::make_shared<BasObserver>();
}

BasService::~BasService()
{}

utility::Context *BasService::GetContext()
{
    return this;
}

BasService *BasService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    return static_cast<BasService *>(servManager->GetProfileService(PROFILE_NAME_BAS));
}

void BasService::LoadBasServiceInterfaceLib(void)
{
    if (!isLoaded_.load()) {
        loader_.OpenLib();
    }

    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(basServiceImpl, "Get bas service impl interface failed");

    isLoaded_.store(true);
    basServiceImpl->Init();
    basServiceImpl->RegisterObserver(basObserverImpl_);
}

void BasService::UnloadBasServiceInterfaceLib(void)
{
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(basServiceImpl, "Get bas service impl interface failed");
    basServiceImpl->DeregisterObserver(basObserverImpl_);
}

void BasService::Enable()
{
    DoInBleThread([this]() { this->StartUp(); });
}

void BasService::StartUp()
{
    GetContext()->OnEnable(PROFILE_NAME_BAS, true);
    BluetoothGattInterface::GetInstance()->AddGattClientObserver(gattObserverImpl_);
    HILOGI("[BAS_SERVICE]BasService started successfully.");
}
void BasService::Disable()
{
    auto basServiceImpl = loader_.GetLibInstance();
    if (basServiceImpl != nullptr) {
        basServiceImpl->DeInit();
    }
    DoInBleThread([this]() { this->ShutDown(); });
}

void BasService::ShutDown()
{
    BluetoothGattInterface::GetInstance()->RemoveGattClientObserver(gattObserverImpl_);
    UnloadBasServiceInterfaceLib();
    GetContext()->OnDisable(PROFILE_NAME_BAS, true);
    HILOGW("[BAS_SERVICE]BasService has already been shutdown before.");
}

void BasService::RegisterObserver(std::shared_ptr<IBasObserver> observer)
{
    basObservers_.Register(observer);
}

void BasService::DeregisterObserver(std::shared_ptr<IBasObserver> observer)
{
    basObservers_.Deregister(observer);
}

void BasService::HandleGattStateChanged(const RawAddress &device, int clientIf, int state)
{
    if (clientIf <= 0 || GetBasAppId(device) == clientIf) {
        return;
    }
    UpdateGattClientIfMap(device, clientIf, state);
    if (!BluetoothConnectionManager::GetInstance()->IsBleConnected(device.GetAddress())) {
        HILOGI("ble acl not connected, ignore");
        return;
    }
    if (state == static_cast<int>(BTConnectState::CONNECTED) &&
        GetDeviceState(device) != static_cast<int>(BTConnectState::CONNECTED)) {
        Connect(device);
        return;
    }
    if (state == static_cast<int>(BTConnectState::DISCONNECTED) && !IsAnyBleProfileConnected(device)) {
        Disconnect(device);
    }
}

void BasService::UpdateGattClientIfMap(const RawAddress &device, int clientIf, int state)
{
    if (clientIf <= 0 || GetBasAppId(device) == clientIf) {
        return;
    }
    bool isEmpty = false;
    otherConnectedClientIfMap_.ChangeValueByLambda(device.GetAddress(),
        [&isEmpty, clientIf, state](std::set<int> &value) {
            if (state == static_cast<int>(BTConnectState::CONNECTED)) {
                value.insert(clientIf);
            }
            if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
                value.erase(clientIf);
            }
            isEmpty = value.empty();
        });
    if (isEmpty) {
        otherConnectedClientIfMap_.Erase(device.GetAddress());
    }
}

bool BasService::IsAnyBleProfileConnected(const RawAddress &device)
{
    std::set<int> otherConnectedClientIfSet = {};
    if (otherConnectedClientIfMap_.Find(device.GetAddress(), otherConnectedClientIfSet) &&
        !otherConnectedClientIfSet.empty()) {
        return true;
    }
    return false;
}

int BasService::GetBasAppId(const RawAddress &device)
{
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, -1, "bas service impl not load");
    return basServiceImpl->GetBasAppId(device);
}

void BasService::NotifyReadBatteryLevelEvent(const RawAddress &device, int batteryLevel)
{
    basObservers_.ForEach([device, batteryLevel](std::shared_ptr<IBasObserver> observer) {
        if (observer != nullptr) {
            observer->OnGetBatteryLevelEvent(device, batteryLevel);
        }
    });
}

void BasService::NotifyBatteryLevelChanged(const RawAddress &device, int batteryLevel)
{
    basObservers_.ForEach([device, batteryLevel](std::shared_ptr<IBasObserver> observer) {
        if (observer != nullptr) {
            observer->OnBatteryLevelChanged(device, batteryLevel);
        }
    });
}

int BasService::Connect(const RawAddress &device)
{
    if (!IsBasSupportedByRemoteDevice(device) ||
        GetConnectStrategy(device) != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        HILOGE("bas not Supported, ignore!");
        return Bluetooth::BT_ERR_PROFILE_DISABLED;
    }
    if (!isLoaded_.load()) {
        LoadBasServiceInterfaceLib();
    }
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "bas service impl not load");
    return basServiceImpl->Connect(device);
}

bool BasService::IsBasSupportedByRemoteDevice(const RawAddress &device)
{
    std::vector<Uuid> uuids = RemoteDeviceProperties::GetInstance()->GetDeviceUuids(device);
    return std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_BAS)) != uuids.end();
}

int BasService::Disconnect(const RawAddress &device)
{
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "bas service impl not load");
    return basServiceImpl->Disconnect(device);
}

int32_t BasService::SetConnectStrategy(const RawAddress &device, int strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_BAS_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if ((GetDeviceState(device)) == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
            HILOGI("connect bas.");
        }
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if ((GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTED) &&
            (GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTING)) {
            Disconnect(device);
            HILOGI("disconnect bas.");
        }
    } else {
        HILOGE("Strategy set failed");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    return Bluetooth::BT_NO_ERROR;
}

int32_t BasService::GetConnectStrategy(const RawAddress &device)
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_BAS_CONNECTION_POLICY);
}

int32_t BasService::GetBatteryLevel(const RawAddress &device)
{
    CHECK_AND_RETURN_LOG_RET(BluetoothConnectionManager::GetInstance()->IsBleConnected(device.GetAddress()),
        Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED, "ble acl not connected");
    if (!IsBasSupportedByRemoteDevice(device)) {
        HILOGE("Device[%{public}s] not support bas", GET_ENCRYPT_ADDR(device));
        return Bluetooth::BT_ERR_PROFILE_DISABLED;
    }
    CHECK_AND_RETURN_LOG_RET(GetDeviceState(device) == static_cast<int>(BTConnectState::CONNECTED),
        Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED, "bas not connected");
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "bas service impl not load");
    return basServiceImpl->GetBatteryLevel(device);
}

int32_t BasService::GetDeviceState(const RawAddress &device)
{
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, static_cast<int>(BTConnectState::DISCONNECTED),
        "bas service impl not load");
    return basServiceImpl->GetDeviceState(device);
}

std::list<RawAddress> BasService::GetConnectDevices(void)
{
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, std::list<RawAddress>(),
        "bas service impl not load");
    return basServiceImpl->GetConnectDevices();
}

int BasService::GetConnectState(void)
{
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, PROFILE_STATE_DISCONNECTED,
        "bas service impl not load");
    return basServiceImpl->GetConnectState();
}

int BasService::GetMaxConnectNum()
{
    return 0;
}

std::vector<RawAddress> BasService::GetDevicesByStates(const std::vector<int> &states)
{
    std::vector<RawAddress> devices;
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, devices,
        "bas service impl not load");
    return basServiceImpl->GetDevicesByStates(states);
}

std::map<RawAddress, int32_t> BasService::GetConnectedDeviceBatteryInfos()
{
    auto basServiceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(basServiceImpl, {}, "bas service impl not load");
    return basServiceImpl->GetConnectedDeviceBatteryInfos();
}

REGISTER_CLASS_CREATOR(BasService);
}  // namespace bluetooth
}  // namespace OHOS
