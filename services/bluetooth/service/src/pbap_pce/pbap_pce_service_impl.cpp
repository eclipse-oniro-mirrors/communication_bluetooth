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
#define LOG_TAG "bt_service_pbap_service_impl"
#endif

#include "pbap_pce_service_impl.h"
#include "datetime_ex.h"
#include "adapter_config.h"
#include "class_creator.h"
#include "interface_adapter.h"
#include "log.h"
#include "pbap_pce_state_machine.h"
#include "profile_config.h"
#include "pbap_pce_header_msg.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
extern "C" PbapPceServiceImplInterface *CreatePbapPceServiceImplInterface(void)
{
    HILOGI("enter");
    return new PbapPceServiceImpl();
}

extern "C" void DestroyPbapPceServiceImplInterface(PbapPceServiceImplInterface *interface)
{
    HILOGI("enter");
    if (interface == nullptr) {
        HILOGE("pbap pce manager Interface is nullptr");
        return;
    }
    delete interface;
}

PbapPceServiceImpl::PbapPceServiceImpl()
{
    HILOGI("PbapPceServiceImpl Create");
    pbapPceSdp_ = std::make_unique<PbapPceSdp>(
        [this](const std::string &addr, const utility::Message &msg) {
            OnSdpResponse(addr, msg);
        });
}

PbapPceServiceImpl::~PbapPceServiceImpl()
{
    HILOGI("PbapPceServiceImpl Release");
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    machineMap_.clear();
}


void PbapPceServiceImpl::RegisterObserver(std::shared_ptr<IPbapPceObserver> &observer)
{
    // Protect pceConnectionObserver_ cross-thread write
    std::lock_guard<std::mutex> lk(observerMutex_);
    pceConnectionObserver_ = observer;
}

void PbapPceServiceImpl::DeregisterObserver(std::shared_ptr<IPbapPceObserver> &observer)
{
    // Protect pceConnectionObserver_ cross-thread write
    std::lock_guard<std::mutex> lk(observerMutex_);
    pceConnectionObserver_ = nullptr;
}

void PbapPceServiceImpl::PostConnectEvent(const PbapPceMessage &event)
{
    DoInPbapThread(std::bind(&PbapPceServiceImpl::ProcessConnectEvent, this, event));
}

void PbapPceServiceImpl::ProcessConnectEvent(const PbapPceMessage &event)
{
    switch (event.what_) {
        case PBAP_PCE_CONNECT_EVT:
            ConnectInternal(event.dev_);
            break;
        case PBAP_PCE_DISCONNECT_EVT:
            DisconnectInternal(event.dev_);
            break;
        default:
            break;
    }
}

int32_t PbapPceServiceImpl::Connect(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(device.GetAddress()).c_str());
    RawAddress rDevice(device.GetAddress());
    BTStrategyType strate = BTStrategyType(GetConnectionStrategy(rDevice));
    if (strate == BTStrategyType::CONNECTION_FORBIDDEN) {
        PBAP_PCE_LOG_ERROR("%{public}s end, connect forbidden.", __PRETTY_FUNCTION__);
        return RET_BAD_STATUS;
    }

    // P0-3: binder/IPC path must NOT hold machineMapMutex_ across PostConnectEvent.
    // Do a quick scoped pre-check under lock (read state/IsBusy/connected-count
    // into locals), release the lock, then judge + PostConnectEvent outside the
    // lock. Authoritative state-machine creation/transition is performed in
    // ConnectInternal on the FFRT queue.
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    bool isBusy = false;
    size_t connectedCount = 0;
    {
        std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
        state = GetDeviceStateNoLock(device);
        isBusy = IsBusyNoLock(device);
        connectedCount = GetConnectedDevicesNoLock().size();
    }

    int32_t checkRet = CheckConnectPrecondition(state, isBusy, connectedCount, device);
    if (checkRet != BT_NO_ERROR) {
        return checkRet;
    }

    PbapPceMessage event(PBAP_PCE_CONNECT_EVT);
    event.dev_ = device;
    PostConnectEvent(event);
    return BT_NO_ERROR;
}

int32_t PbapPceServiceImpl::CheckConnectPrecondition(
    int state, bool isBusy, size_t connectedCount, const RawAddress &device)
{
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        if (isBusy) {
            PBAP_PCE_LOG_INFO("%{public}s already connected and syncing, return BT_NO_ERROR (idempotent), "
                "addr=%{public}s", __PRETTY_FUNCTION__, GetEncryptAddr(device.GetAddress()).c_str());
            return BT_NO_ERROR;
        }
    } else if ((state == static_cast<int>(BTConnectState::CONNECTING)) ||
        (state == static_cast<int>(BTConnectState::DISCONNECTING))) {
        PBAP_PCE_LOG_ERROR("%{public}s end, bad status. state=%{public}d, addr=%{public}s", __PRETTY_FUNCTION__, state,
            GetEncryptAddr(device.GetAddress()).c_str());
        return RET_BAD_STATUS;
    }

    if (state != static_cast<int>(BTConnectState::CONNECTED)) {
        int maxNum = GetMaxConnectNum();
        if (maxNum <= 0 || connectedCount >= static_cast<size_t>(maxNum)) {
            PBAP_PCE_LOG_ERROR("%{public}s end, maximum devices is %{public}d.", __PRETTY_FUNCTION__, maxNum);
            return RET_NO_SUPPORT;
        }
    }

    if (isBusy) {
        PBAP_PCE_LOG_ERROR("%{public}s end, Is busy! addr=%{public}s", __PRETTY_FUNCTION__,
            GetEncryptAddr(device.GetAddress()).c_str());
        return RET_BAD_STATUS;
    }
    return BT_NO_ERROR;
}

int PbapPceServiceImpl::ConnectInternal(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(device.GetAddress()).c_str());
    if (serviceState_ != PBAP_PCE_STATE_STARTUP) {
        PBAP_PCE_LOG_ERROR("%{public}s end, pbap pce service isn't started.", __PRETTY_FUNCTION__);
        return RET_NO_SUPPORT;
    }

    std::string addr = device.GetAddress();
    // P0-3: authoritative state-machine creation/transition happens here on the
    // FFRT queue. The machineMapMutex_ is held only for the map mutation; the
    // manual re-sync dispatch (PCE_REQ_DOWNLOAD) is performed AFTER releasing the
    // lock because PceConnectedState::Dispatch may issue a blocking OBEX Get.
    std::shared_ptr<utility::StateMachine> stmToDispatch;
    int createRet = CreateOrReuseStateMachine(device, addr, stmToDispatch);
    if (createRet != BT_NO_ERROR) {
        return createRet;
    }

    // Dispatch the manual re-sync outside machineMapMutex_.
    if (stmToDispatch != nullptr) {
        bool ret = stmToDispatch->ProcessMessage(utility::Message(PCE_REQ_DOWNLOAD));
        PBAP_PCE_LOG_INFO("%{public}s PCE_REQ_DOWNLOAD dispatch ret=%{public}d, addr=%{public}s",
            __PRETTY_FUNCTION__, ret, GetEncryptAddr(addr).c_str());
    }
    return BT_NO_ERROR;
}

int PbapPceServiceImpl::CreateOrReuseStateMachine(const RawAddress &device, const std::string &addr,
    std::shared_ptr<utility::StateMachine> &stmToDispatch)
{
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    int maxNum = GetMaxConnectNum();
    if (maxNum <= 0 || GetConnectedDevicesNoLock().size() >= static_cast<size_t>(maxNum)) {
        PBAP_PCE_LOG_ERROR("%{public}s end, maximum devices is %{public}d.",  __PRETTY_FUNCTION__, maxNum);
        return RET_NO_SUPPORT;
    }

    if (machineMap_.find(addr) == machineMap_.end()) {
        PBAP_PCE_LOG_INFO("%{public}s creating NEW state machine, addr=%{public}s, mapSize=%{public}zu",
            __PRETTY_FUNCTION__, GetEncryptAddr(addr).c_str(), machineMap_.size());
        // M-1: copy shared_ptr under observerMutex_ before passing to state machine
        std::shared_ptr<IPbapPceObserver> observer;
        {
            std::lock_guard<std::mutex> lk(observerMutex_);
            observer = pceConnectionObserver_;
        }
        machineMap_[addr] = std::make_shared<PbapPceStateMachine>(device, *this, *pbapPceSdp_, observer);
        return BT_NO_ERROR;
    }

    int state = GetDeviceStateNoLock(device);
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        if (IsBusyNoLock(device)) {
            PBAP_PCE_LOG_INFO("%{public}s connected & busy in PBAP thread, skip manual sync (idempotent), "
                "addr=%{public}s", __PRETTY_FUNCTION__, GetEncryptAddr(addr).c_str());
            return BT_NO_ERROR;
        }
        PBAP_PCE_LOG_INFO("%{public}s manual re-sync triggered by Connect, deferred to post-unlock, addr=%{public}s",
            __PRETTY_FUNCTION__, GetEncryptAddr(addr).c_str());
        // Defer dispatch to after lock release (PCE_REQ_DOWNLOAD -> OBEX Get may block).
        stmToDispatch = machineMap_[addr];
    } else if ((state == static_cast<int>(BTConnectState::DISCONNECTING)) ||
        (state == static_cast<int>(BTConnectState::DISCONNECTED))) {
        machineMap_[addr]->InitState(PCE_CONNECTING_STATE);
    } else {
        PBAP_PCE_LOG_ERROR("%{public}s end, already connecting or for connected device.", __PRETTY_FUNCTION__);
        return RET_BAD_STATUS;
    }
    return BT_NO_ERROR;
}

bool PbapPceServiceImpl::IsBusy(const RawAddress &device)
{
    bool ret = false;
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    ret = IsBusyNoLock(device);
    return ret;
}

bool PbapPceServiceImpl::IsBusyNoLock(const RawAddress &device)
{
    bool ret = false;
    std::string strAddr = device.GetAddress();
    if (machineMap_.find(strAddr) != machineMap_.end()) {
        auto sm = static_cast<PbapPceStateMachine *>(machineMap_[strAddr].get());
        ret = sm->IsBusy();
    }
    return ret;
}

int32_t PbapPceServiceImpl::GetConnectionStrategy(const RawAddress &device)
{
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    if (!LoadConnectPolicy(device.GetAddress(), strategy)) {
        PBAP_PCE_LOG_ERROR("%{public}s end, load strategy value failed.", __PRETTY_FUNCTION__);
        return strategy;
    }
    return strategy;
}

bool PbapPceServiceImpl::LoadConnectPolicy(const std::string &addr, int &strategy)
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    if (config == nullptr) {
        HILOGE("addr: %{public}s ProfileConfig::GetInstance() is null, %{public}s load fail.",
            GetEncryptAddr(addr).c_str(), PROPERTY_PBAP_CLIENT_CONNECTION_POLICY);
        return false;
    }
    bool ok = config->GetValue(addr, SECTION_CONNECTION_POLICIES, PROPERTY_PBAP_CLIENT_CONNECTION_POLICY, strategy);
    if (!ok) {
        HILOGE("addr: %{public}s %{public}s load fail, section=%{public}s, strategy(default)=%{public}d.",
            GetEncryptAddr(addr).c_str(), PROPERTY_PBAP_CLIENT_CONNECTION_POLICY,
            SECTION_CONNECTION_POLICIES, strategy);
        return false;
    }
    PBAP_PCE_LOG_INFO("%{public}s strategy=%{public}d, addr=%{public}s", __PRETTY_FUNCTION__, strategy,
        GetEncryptAddr(addr).c_str());
    return true;
}

int32_t PbapPceServiceImpl::GetDeviceState(const RawAddress &device)
{
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    return GetDeviceStateNoLock(device);
}

int32_t PbapPceServiceImpl::GetDeviceStateNoLock(const RawAddress &device)
{
    BTConnectState result = BTConnectState::DISCONNECTED;
    auto target = machineMap_.find(device.GetAddress());
    if (target != machineMap_.end()) {
        std::string name = target->second->GetState()->Name();
        if (name == PCE_DISCONNECTED_STATE) {
            result = BTConnectState::DISCONNECTED;
        } else if (name == PCE_CONNECTING_STATE) {
            result = BTConnectState::CONNECTING;
        } else if (name == PCE_CONNECTED_STATE) {
            result = BTConnectState::CONNECTED;
        } else if (name == PCE_DISCONNECTING_STATE) {
            result = BTConnectState::DISCONNECTING;
        }
    }
    return static_cast<int>(result);
}

std::vector<RawAddress> PbapPceServiceImpl::GetConnectedDevices()
{
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    return GetConnectedDevicesNoLock();
}

std::vector<RawAddress> PbapPceServiceImpl::GetConnectedDevicesNoLock()
{
    std::vector<int> states {static_cast<int>(BTConnectState::CONNECTED)};
    return GetDevicesByStatesNoLock(states);
}

std::vector<RawAddress> PbapPceServiceImpl::GetDevicesByStates(const std::vector<int32_t> &states)
{
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    return GetDevicesByStatesNoLock(states);
}

std::vector<RawAddress> PbapPceServiceImpl::GetDevicesByStatesNoLock(const std::vector<int32_t> &states)
{
    std::vector<RawAddress> devices;
    for (auto &state : states) {
        for (auto &it : machineMap_) {
            // m-1: inline state check instead of calling GetDeviceState (avoids recursive lock overhead)
            std::string name = it.second->GetState()->Name();
            int currentState = static_cast<int>(BTConnectState::DISCONNECTED);
            if (name == PCE_DISCONNECTED_STATE) {
                currentState = static_cast<int>(BTConnectState::DISCONNECTED);
            } else if (name == PCE_CONNECTING_STATE) {
                currentState = static_cast<int>(BTConnectState::CONNECTING);
            } else if (name == PCE_CONNECTED_STATE) {
                currentState = static_cast<int>(BTConnectState::CONNECTED);
            } else if (name == PCE_DISCONNECTING_STATE) {
                currentState = static_cast<int>(BTConnectState::DISCONNECTING);
            }
            if (currentState == state) {
                auto sm = static_cast<PbapPceStateMachine *>(it.second.get());
                devices.push_back(sm->GetDevice());
            }
        }
    }
    return devices;
}

int32_t PbapPceServiceImpl::GetConnectState()
{
    int32_t result = 0;
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    for (auto &sm : machineMap_) {
        std::string name = sm.second->GetState()->Name();
        if (name == PCE_DISCONNECTED_STATE) {
            result |= PROFILE_STATE_DISCONNECTED;
        } else if (name == PCE_CONNECTING_STATE) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (name == PCE_CONNECTED_STATE) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (name == PCE_DISCONNECTING_STATE) {
            result |= PROFILE_STATE_DISCONNECTING;
        }
    }
    return result;
}

bool PbapPceServiceImpl::IsDownloading(const RawAddress &device)
{
    return IsBusy(device);
}

int PbapPceServiceImpl::GetMaxConnectNum()
{
    return pbapPceConfig_.pceMaxDevices_;
}

const PbapPceConfig &PbapPceServiceImpl::GetPceConfig() const
{
    return pbapPceConfig_;
}

int32_t PbapPceServiceImpl::Disconnect(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(device.GetAddress()).c_str());
    int state = GetDeviceState(device);
    if ((state == static_cast<int>(BTConnectState::CONNECTING)) ||
        (state == static_cast<int>(BTConnectState::DISCONNECTING)) ||
        (state == static_cast<int>(BTConnectState::DISCONNECTED))) {
        PBAP_PCE_LOG_ERROR("%{public}s end, bad status.", __PRETTY_FUNCTION__);
        return RET_BAD_STATUS;
    }

    PbapPceMessage event(PBAP_PCE_DISCONNECT_EVT);
    event.dev_ = device;
    PostConnectEvent(event);
    return BT_NO_ERROR;
}

int PbapPceServiceImpl::DisconnectInternal(const RawAddress &device)
{
    utility::Message msg(PCE_REQ_DISCONNECTED);
    ForwardMsgToStm(device, msg);
    return BT_NO_ERROR;
}

void PbapPceServiceImpl::ForwardMsgToStm(const RawAddress &device, const utility::Message &msg)
{
    // Copy the StateMachine shared_ptr under lock, then release the lock
    // BEFORE calling ProcessMessage. ProcessMessage may dispatch into OBEX/SDP
    // blocking I/O which can block; holding machineMapMutex_ during such a
    // dispatch would deadlock binder threads competing for the same lock.
    std::shared_ptr<utility::StateMachine> sm;
    {
        std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
        auto it = machineMap_.find(device.GetAddress());
        if (it == machineMap_.end()) {
            PBAP_PCE_LOG_ERROR("%{public}s device NOT in machineMap_, msg.what=%{public}d, addr=%{public}s, "
                "mapSize=%{public}zu", __PRETTY_FUNCTION__, msg.what_,
                GetEncryptAddr(device.GetAddress()).c_str(), machineMap_.size());
            // 消息无法投递时释放 arg2_ 携带的裸指针负载（如迟到 SDP 结果的
            // PbapPceHeaderSdpMsg），防止泄漏
            PbapPceStateMachine::ReleasePbapMsgPayload(msg);
            return;
        }
        sm = it->second;
    }
    sm->ProcessMessage(msg);
}

int32_t PbapPceServiceImpl::SetConnectionStrategy(const RawAddress &device, int strategy)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s, strategy=%{public}d", __PRETTY_FUNCTION__,
        GetEncryptAddr(device.GetAddress()).c_str(), strategy);
    switch (strategy) {
        case static_cast<int>(BTStrategyType::CONNECTION_ALLOWED):
        case static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN):
            if (!SaveConnectPolicy(device.GetAddress(), strategy)) {
                PBAP_PCE_LOG_ERROR("%{public}s end, save strategy value failed.", __PRETTY_FUNCTION__);
                return RET_NO_SUPPORT;
            }
            break;
        default:
            PBAP_PCE_LOG_ERROR("%{public}s end, save strategy invalid value.", __PRETTY_FUNCTION__);
            return RET_BAD_PARAM;
    }

    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if (IsConnected(device)) {
            Disconnect(device);
        } else if (GetDeviceState(device) == static_cast<int>(BTConnectState::CONNECTING)) {
            // P0-4: binder 线程不得直接进入状态机（与 PbapThread 并发操作状态栈），
            // 经 DoInPbapThread 转投串行队列（与 OnObexResponse 同模式）
            utility::Message msg(PCE_REQ_SET_TARGET_STATE, PCE_TARGET_STATE_DISCONNECTED);
            DoInPbapThread([this, device, msg]() {
                ForwardMsgToStm(device, msg);
            });
        }
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if (GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
        } else if (GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTING)) {
            utility::Message msg(PCE_REQ_SET_TARGET_STATE, PCE_TARGET_STATE_CONNECTED);
            DoInPbapThread([this, device, msg]() {
                ForwardMsgToStm(device, msg);
            });
        }
    }
    return BT_NO_ERROR;
}

bool PbapPceServiceImpl::SaveConnectPolicy(const std::string &addr, int strategy)
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    if (!config->SetValue(addr, SECTION_CONNECTION_POLICIES, PROPERTY_PBAP_CLIENT_CONNECTION_POLICY, strategy)) {
        HILOGE("addr: %{public}s %{public}s save fail.",
            GetEncryptAddr(addr).c_str(), PROPERTY_PBAP_CLIENT_CONNECTION_POLICY);
        return false;
    }
    return true;
}

bool PbapPceServiceImpl::IsConnected(const RawAddress &device)
{
    return GetDeviceState(device) == static_cast<int>(BTConnectState::CONNECTED);
}

bool PbapPceServiceImpl::SetBusy(const RawAddress &device, bool busy)
{
    bool ret = false;
    std::string strAddr = device.GetAddress();
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    if (machineMap_.find(strAddr) != machineMap_.end()) {
        auto sm = static_cast<PbapPceStateMachine *>(machineMap_[strAddr].get());
        ret = sm->SetBusy(busy);
    }
    return ret;
}

void PbapPceServiceImpl::OnObexResponse(const RawAddress &device, const utility::Message &msg)
{
    DoInPbapThread([this, device, msg]() {
        ProcessObexRespMessage(device, msg);
    });
}

void PbapPceServiceImpl::ProcessObexRespMessage(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
    SetPowerStatusBusy(device, false);
    switch (msg.what_) {
        case PCE_OBEX_CONNECTED:
            ProcessObexConnected(device, msg);
            break;
        case PCE_OBEX_DISCONNECTED:
        case PCE_OBEX_CONNECT_FAILED:
        case PCE_TRANSPORT_FAILED:
            ProcessObexDisconnected(device, msg);
            break;
        case PCE_DOWNLOAD_COMPLETE:
            ForwardMsgToStm(device, msg);
            break;
        case PCE_DOWNLOAD_FAILED:
            PBAP_PCE_LOG_INFO("%{public}s PCE_DOWNLOAD_FAILED, forwarding to sm", __PRETTY_FUNCTION__);
            ForwardMsgToStm(device, msg);
            break;
        default:
            break;
    }
}

void PbapPceServiceImpl::ProcessObexConnected(const RawAddress &device, const utility::Message &msg)
{
    ForwardMsgToStm(device, msg);
}

void PbapPceServiceImpl::ProcessObexDisconnected(const RawAddress &device, const utility::Message &msg)
{
    ForwardMsgToStm(device, msg);
}

void PbapPceServiceImpl::ProcessPhonebookMessage(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
    SetPowerStatusBusy(device, false);
    switch (msg.what_) {
        case PCE_PULLPHONEBOOKSIZE_COMPLETED:
        case PCE_PULLPHONEBOOK_COMPLETED:
        case PCE_ABORTDOWNLOADING_COMPLETED:
            ProcessPhoneBookActionCompleted(device, msg);
            break;
        default:
            break;
    }
}

void PbapPceServiceImpl::ProcessPhoneBookActionCompleted(const RawAddress &device, const utility::Message &msg)
{
    ForwardMsgToStm(device, msg);
    utility::Message msg2(PCE_REQ_TRANSIT_TARGET_STATE);
    ForwardMsgToStm(device, msg2);
}

void PbapPceServiceImpl::SetPowerStatusBusy(const RawAddress &device, bool busy)
{
    std::string strAddr = device.GetAddress();
    std::lock_guard<BtRecursiveMutex> lock(machineMapMutex_);
    if (machineMap_.find(strAddr) != machineMap_.end()) {
        auto sm = static_cast<PbapPceStateMachine *>(machineMap_[strAddr].get());
        sm->SetPowerStatusBusy(busy);
    }
}

void PbapPceServiceImpl::OnSdpResponse(const std::string &addr, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s msg.what=%{public}d, addr=%{public}s", __PRETTY_FUNCTION__,
        msg.what_, GetEncryptAddr(addr).c_str());
    RawAddress device(addr);
    DoInPbapThread([this, device, msg]() {
        ProcessSdpMessage(device, msg);
    });
}

void PbapPceServiceImpl::ProcessSdpMessage(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s msg.what=%{public}d, addr=%{public}s", __PRETTY_FUNCTION__,
        msg.what_, GetEncryptAddr(device.GetAddress()).c_str());
    switch (msg.what_) {
        case PCE_SDP_FAILED:
        case PCE_SDP_FINISH:
            ForwardMsgToStm(device, msg);
            break;
        default:
            PBAP_PCE_LOG_ERROR("%{public}s unknown msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
            break;
    }
}

void PbapPceServiceImpl::LoadPceConfig()
{
    IAdapterConfig *adpterConfig = AdapterConfig::GetInstance();
    if (!adpterConfig->GetValue(SECTION_PBAP_PCE_SERVICE, PROPERTY_SRM_ENABLE, pbapPceConfig_.srmEnable_)) {
        PBAP_PCE_LOG_ERROR("Load config %{public}s failure", PROPERTY_SRM_ENABLE.c_str());
    }
    // MTU 固定在代码中，不再读取 bt_config.xml。OBEX MaxPacketSize 字段为 2 字节，
    // 上限 0xFFFE；该值仅在 OBEX CONNECT 请求中宣告本端接收能力，实际包大小由对端
    // PSE 协商决定，不存在破坏协商的风险。
    constexpr int PCE_OBEX_RFCOMM_MTU = 0xFFFE;
    constexpr int PCE_OBEX_L2CAP_MTU = 0xFFFE;
    pbapPceConfig_.rfcommMtu_ = PCE_OBEX_RFCOMM_MTU;
    pbapPceConfig_.l2capMtu_ = PCE_OBEX_L2CAP_MTU;
    if (!adpterConfig->GetValue(SECTION_PBAP_PCE_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES,
        pbapPceConfig_.pceMaxDevices_)) {
        PBAP_PCE_LOG_ERROR("Load config %{public}s failure", PROPERTY_MAX_CONNECTED_DEVICES.c_str());
    }
    if (pbapPceConfig_.pceMaxDevices_ <= 0) {
        PBAP_PCE_LOG_WARN("Invalid pceMaxDevices_=%{public}d, fallback to PCE_MAXIMUM_DEVICES(%{public}d)",
            pbapPceConfig_.pceMaxDevices_, PCE_MAXIMUM_DEVICES);
        pbapPceConfig_.pceMaxDevices_ = PCE_MAXIMUM_DEVICES;
    }
    PBAP_PCE_LOG_INFO("srmEnable = %{public}d, rfcommMtu=0x%X, l2capMtu_=0x%X, maxDevices=%{public}d",
        pbapPceConfig_.srmEnable_,
        pbapPceConfig_.rfcommMtu_,
        pbapPceConfig_.l2capMtu_,
        pbapPceConfig_.pceMaxDevices_);
}

void PbapPceServiceImpl::Enable()
{
    serviceState_ = PBAP_PCE_STATE_STARTUPING;
    LoadPceConfig();
    int retVal = pbapPceSdp_->Register();
    if (retVal != Bluetooth::BT_NO_ERROR) {
        PBAP_PCE_LOG_ERROR("SDP Register failed, retVal=%{public}d, service stays shutdown", retVal);
        serviceState_ = PBAP_PCE_STATE_SHUTDOWN;
        return;
    }
    serviceState_ = PBAP_PCE_STATE_STARTUP;
}

void PbapPceServiceImpl::Disable()
{
    serviceState_ = PBAP_PCE_STATE_SHUTDOWNING;

    int retVal = 0;
    if (pbapPceSdp_) {
        retVal = pbapPceSdp_->Deregister();
        if (retVal != Bluetooth::BT_NO_ERROR) {
            PBAP_PCE_LOG_ERROR("pbapPceSdp_->Deregister() error");
        }
    }

    std::vector<int32_t> states {
        static_cast<int32_t>(BTConnectState::CONNECTING),
        static_cast<int32_t>(BTConnectState::CONNECTED),
        static_cast<int32_t>(BTConnectState::DISCONNECTING)
    };
    auto devices = GetDevicesByStates(states);
    for (auto &device : devices) {
        Disconnect(device);
    }

    serviceState_ = PBAP_PCE_STATE_SHUTDOWN;
}

}  // namespace bluetooth
}  // namespace OHOS
