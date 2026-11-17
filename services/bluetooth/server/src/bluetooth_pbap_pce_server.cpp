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
#define LOG_TAG "bt_server_pbap_pce"
#endif

#include "bluetooth_pbap_pce_server.h"

#include "ipc_skeleton.h"

#include <atomic>
#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "interface_adapter_manager.h"
#include "i_bluetooth_pbap_pce.h"
#include "interface_profile_manager.h"
#include "interface_profile_pbap_pce.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"
#include "bt_def.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

struct BluetoothPbapPceServer::impl {
    impl();
    RemoteObserverList<IBluetoothPbapPceObserver> observers_;

    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_{nullptr};

    class PbapPceObserver;
    std::unique_ptr<PbapPceObserver> observerImp_{nullptr};

    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;

    // C-1: use std::atomic to protect cross-thread read/write.
    // Written by AdapterManager thread (OnSystemStateChange) and construction thread;
    // read by IPC worker threads in stub methods. SA-process profile service object
    // is not individually destroyed during bluetooth toggle (only ProfileServiceManager::Stop
    // deletes it at SA shutdown), so in-flight IPC dereference is safe.
    std::atomic<bluetooth::IProfilePbapPce*> pbapPceService_{nullptr};
};

class BluetoothPbapPceServer::impl::PbapPceObserver : public bluetooth::IPbapPceObserver {
public:
    explicit PbapPceObserver(BluetoothPbapPceServer::impl *pimpl) : pimpl_(pimpl) {};
    ~PbapPceObserver() override = default;
    void OnConnectionStateChanged(const bluetooth::RawAddress& device, int state, int cause) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr:%{public}s, state:%{public}d, cause:%{public}d", GET_ENCRYPT_ADDR(device), state, cause);
        if (pimpl_ == nullptr || observers_ == nullptr) {
            HILOGW("OnConnectionStateChanged: pimpl_ or observers_ not ready, skip");
            return;
        }
        // M-3: copy observer+tokenId list under lock, do permission check + IPC outside lock
        // to avoid AB-BA deadlock (ForEach holds RemoteObserverList::lock_ while doing synchronous IPC)
        std::vector<std::pair<sptr<IBluetoothPbapPceObserver>, uint64_t>> targets;
        observers_->ForEach([this, &targets](IBluetoothPbapPceObserver* observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            targets.emplace_back(observer, tokenId);
        });
        for (auto &entry : targets) {
            auto &observer = entry.first;
            uint64_t tokenId = entry.second;
            // 权限校验失败的 observer 仅跳过自身，不得中断其余 observer 的通知
            //（RegisterObserverInner 无权限要求，任意应用可注册；中断会被恶意应用
            // 利用为通知抑制）
            if (!PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId)) {
                HILOGI("[PERMISSION] check ACCESS_BLUETOOTH permission failed, skip one observer, "
                    "callingName(%{public}s)", PermissionManager::GetCallingName(tokenId).c_str());
                continue;
            }
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr, state, cause);
            }
        }
    }

    void OnSyncStateChange(const bluetooth::RawAddress &device, int syncState) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr:%{public}s, syncState:%{public}d", GET_ENCRYPT_ADDR(device), syncState);
        if (pimpl_ == nullptr || observers_ == nullptr) {
            HILOGW("OnSyncStateChange: pimpl_ or observers_ not ready, skip");
            return;
        }
        // M-3: same pattern as OnConnectionStateChanged
        std::vector<std::pair<sptr<IBluetoothPbapPceObserver>, uint64_t>> targets;
        observers_->ForEach([this, &targets](IBluetoothPbapPceObserver* observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            targets.emplace_back(observer, tokenId);
        });
        for (auto &entry : targets) {
            auto &observer = entry.first;
            uint64_t tokenId = entry.second;
            if (!PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId)) {
                HILOGI("[PERMISSION] check ACCESS_BLUETOOTH permission failed, skip one observer, "
                    "callingName(%{public}s)", PermissionManager::GetCallingName(tokenId).c_str());
                continue;
            }
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnSyncStateChange(randomAddr, syncState);
            }
        }
    }

    void SetObserver(RemoteObserverList<IBluetoothPbapPceObserver>* observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothPbapPceObserver>* observers_ = nullptr;
    BluetoothPbapPceServer::impl *pimpl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PbapPceObserver);
};

class BluetoothPbapPceServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothPbapPceServer::impl* impl) : impl_(impl) {};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        HILOGI("OnSystemStateChange state=%{public}d", static_cast<int>(state));
        switch (state) {
            case bluetooth::BTSystemState::ON: {
                bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
                CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");
                bluetooth::IProfile* profileService =
                    serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PCE);
                CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find PbapPceService");
                // observerImp_ 未就绪（构造窗口）时跳过注册，避免空指针解引用；
                // 正常顺序下 impl() 已先于 SystemStateObserver 注册完成创建
                if (impl_->observerImp_ == nullptr) {
                    HILOGW("OnSystemStateChange: observerImp_ not ready, skip RegisterObserver");
                    break;
                }
                impl_->pbapPceService_.store(
                    static_cast<bluetooth::IProfilePbapPce*>(profileService), std::memory_order_release);
                impl_->pbapPceService_.load(std::memory_order_acquire)->RegisterObserver(*impl_->observerImp_);
                break;
            }
            case bluetooth::BTSystemState::OFF:
                impl_->pbapPceService_.store(nullptr, std::memory_order_release);
                break;
            default:
                break;
            }
    }

private:
    BluetoothPbapPceServer::impl* impl_;
};

BluetoothPbapPceServer::impl::impl()
{
    // 构造顺序：先完成 appContainer_/observerImp_ 的创建与初始化，最后注册
    // SystemStateObserver——消除 BT ON 事件在 observerImp_ 就绪前到达时的
    // 空指针/未初始化窗口
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
    observerImp_ = std::make_unique<PbapPceObserver>(this);
    observerImp_->SetObserver(&observers_);
    systemStateObserver_ = std::make_unique<SystemStateObserver>(this);
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
}

BluetoothPbapPceServer::BluetoothPbapPceServer()
{
    HILOGI("BluetoothPbapPceServer created");
    // observerImp_ 已在 impl() 内创建并 SetObserver（先于 SystemStateObserver 注册）
    pimpl = std::make_unique<impl>();

    bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");

    bluetooth::IProfile* profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PCE);
    CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find PbapPceService");

    CHECK_AND_RETURN_LOG((pimpl->observerImp_ != nullptr), "observerImp_ is null");
    pimpl->pbapPceService_.store((bluetooth::IProfilePbapPce*)profileService, std::memory_order_release);
    pimpl->pbapPceService_.load(std::memory_order_acquire)->RegisterObserver(*pimpl->observerImp_);
}

BluetoothPbapPceServer::~BluetoothPbapPceServer()
{
    if (pimpl == nullptr) {
        return;
    }
    // 先断事件源：防止注销期间 BT ON 事件经 SystemStateObserver 重新注册 observer
    // （AdapterManager::systemObservers_ 为持锁 ForEach 容器，注销返回后无在途通知）
    if (pimpl->systemStateObserver_ != nullptr) {
        bluetooth::IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*pimpl->systemStateObserver_);
    }
    // 再注销数据面 observer（BaseObserverList 持锁 ForEach，注销返回后无在途回调，
    // 随后 ~impl 销毁 observerImp_ 安全，消除悬垂指针/UAF）
    auto* svc = pimpl->pbapPceService_.load(std::memory_order_acquire);
    if (svc != nullptr && pimpl->observerImp_ != nullptr) {
        svc->DeregisterObserver(*pimpl->observerImp_);
    }
}

int32_t BluetoothPbapPceServer::GetPhoneBookSyncState(const BluetoothRawAddress &device)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    // m-4: pimpl nullptr check moved before ConvertToRealAddress
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr), BT_ERR_INTERNAL_ERROR, "pimpl is null");
    auto* svc = pimpl->pbapPceService_.load(std::memory_order_acquire);
    CHECK_AND_RETURN_LOG_RET((svc != nullptr), BT_ERR_INTERNAL_ERROR, "pbapPceService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        HILOGE("GetPhoneBookSyncState: addr: %{public}s, ConvertToRealAddress failed",
            GET_ENCRYPT_ADDR(device));
        return BT_ERR_INTERNAL_ERROR;
    }
    return svc->GetPhoneBookSyncState(realAddr);
}

int32_t BluetoothPbapPceServer::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr), BT_ERR_INTERNAL_ERROR, "pimpl is null");
    auto* svc = pimpl->pbapPceService_.load(std::memory_order_acquire);
    CHECK_AND_RETURN_LOG_RET((svc != nullptr), BT_ERR_INTERNAL_ERROR, "pbapPceService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        HILOGE("GetDeviceState: addr: %{public}s, ConvertToRealAddress failed",
            GET_ENCRYPT_ADDR(device));
        return BT_ERR_INTERNAL_ERROR;
    }
    state = svc->GetDeviceState(realAddr);
    HILOGD("state : %{public}d", state);
    return BT_NO_ERROR;
}

int32_t BluetoothPbapPceServer::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress> &rawDevices)
{
    HILOGI("GetDevicesByStates: states count=%{public}zu", states.size());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr), BT_ERR_INTERNAL_ERROR, "pimpl is null");
    auto* svc = pimpl->pbapPceService_.load(std::memory_order_acquire);
    CHECK_AND_RETURN_LOG_RET((svc != nullptr), BT_ERR_INTERNAL_ERROR, "pbapPceService_ is null");

    auto devices = svc->GetDevicesByStates(states);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(devices, rawDevices);
    return BT_NO_ERROR;
}

int32_t BluetoothPbapPceServer::Connect(const BluetoothRawAddress& device)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr), BT_ERR_INTERNAL_ERROR, "pimpl is null");
    auto* svc = pimpl->pbapPceService_.load(std::memory_order_acquire);
    CHECK_AND_RETURN_LOG_RET((svc != nullptr), BT_ERR_INTERNAL_ERROR, "pbapPceService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        HILOGE("Connect: addr: %{public}s, ConvertToRealAddress failed",
            GET_ENCRYPT_ADDR(device));
        return BT_ERR_INTERNAL_ERROR;
    }
    return svc->Connect(realAddr);
}

int32_t BluetoothPbapPceServer::Disconnect(const BluetoothRawAddress& device)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr), BT_ERR_INTERNAL_ERROR, "pimpl is null");
    auto* svc = pimpl->pbapPceService_.load(std::memory_order_acquire);
    CHECK_AND_RETURN_LOG_RET((svc != nullptr), BT_ERR_INTERNAL_ERROR, "pbapPceService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        HILOGE("Disconnect: addr: %{public}s, ConvertToRealAddress failed",
            GET_ENCRYPT_ADDR(device));
        return BT_ERR_INTERNAL_ERROR;
    }
    return svc->Disconnect(realAddr);
}

int32_t BluetoothPbapPceServer::SetConnectionStrategy(const BluetoothRawAddress& device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), strategy);
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr), BT_ERR_INTERNAL_ERROR, "pimpl is null");
    auto* svc = pimpl->pbapPceService_.load(std::memory_order_acquire);
    CHECK_AND_RETURN_LOG_RET((svc != nullptr), BT_ERR_INTERNAL_ERROR, "pbapPceService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        HILOGE("SetConnectionStrategy: addr: %{public}s, ConvertToRealAddress failed",
            GET_ENCRYPT_ADDR(device));
        return BT_ERR_INTERNAL_ERROR;
    }
    return svc->SetConnectionStrategy(realAddr, strategy);
}

int32_t BluetoothPbapPceServer::GetConnectionStrategy(const BluetoothRawAddress& device, int32_t &strategy)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr), BT_ERR_INTERNAL_ERROR, "pimpl is null");
    auto* svc = pimpl->pbapPceService_.load(std::memory_order_acquire);
    CHECK_AND_RETURN_LOG_RET((svc != nullptr), BT_ERR_INTERNAL_ERROR, "pbapPceService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        HILOGE("GetConnectionStrategy: addr: %{public}s, ConvertToRealAddress failed",
            GET_ENCRYPT_ADDR(device));
        return BT_ERR_INTERNAL_ERROR;
    }
    strategy = svc->GetConnectionStrategy(realAddr);
    return BT_NO_ERROR;
}

void BluetoothPbapPceServer::RegisterObserver(const sptr<IBluetoothPbapPceObserver>& observer)
{
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ or appContainer_ too much, count=%{public}d",
            pimpl->observers_.Size());
        return;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
}

void BluetoothPbapPceServer::DeregisterObserver(const sptr<IBluetoothPbapPceObserver>& observer)
{
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");

    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
}

}  // namespace Bluetooth
}  // namespace OHOS