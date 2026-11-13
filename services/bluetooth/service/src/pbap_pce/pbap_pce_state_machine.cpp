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
#define LOG_TAG "bt_pbap_pce_state_machine"
#endif

#include "pbap_pce_state_machine.h"
#include <vector>
#include "bt_def.h"
#include "pbap_pce_app_params.h"
#include "pbap_pce_service_impl.h"
#include "power_manager.h"
#include "thread_util.h"

// 合并为单文件后，各状态日志格式串加状态名前缀，便于按状态过滤
#define PCE_DISCONNECTED_LOG_DEBUG(fmt, ...) PBAP_PCE_LOG_DEBUG("[Disconnected]" fmt, ##__VA_ARGS__)
#define PCE_DISCONNECTED_LOG_INFO(fmt, ...) PBAP_PCE_LOG_INFO("[Disconnected]" fmt, ##__VA_ARGS__)
#define PCE_DISCONNECTED_LOG_WARN(fmt, ...) PBAP_PCE_LOG_WARN("[Disconnected]" fmt, ##__VA_ARGS__)
#define PCE_DISCONNECTED_LOG_ERROR(fmt, ...) PBAP_PCE_LOG_ERROR("[Disconnected]" fmt, ##__VA_ARGS__)
#define PCE_CONNECTING_LOG_DEBUG(fmt, ...) PBAP_PCE_LOG_DEBUG("[Connecting]" fmt, ##__VA_ARGS__)
#define PCE_CONNECTING_LOG_INFO(fmt, ...) PBAP_PCE_LOG_INFO("[Connecting]" fmt, ##__VA_ARGS__)
#define PCE_CONNECTING_LOG_WARN(fmt, ...) PBAP_PCE_LOG_WARN("[Connecting]" fmt, ##__VA_ARGS__)
#define PCE_CONNECTING_LOG_ERROR(fmt, ...) PBAP_PCE_LOG_ERROR("[Connecting]" fmt, ##__VA_ARGS__)
#define PCE_CONNECTED_LOG_DEBUG(fmt, ...) PBAP_PCE_LOG_DEBUG("[Connected]" fmt, ##__VA_ARGS__)
#define PCE_CONNECTED_LOG_INFO(fmt, ...) PBAP_PCE_LOG_INFO("[Connected]" fmt, ##__VA_ARGS__)
#define PCE_CONNECTED_LOG_WARN(fmt, ...) PBAP_PCE_LOG_WARN("[Connected]" fmt, ##__VA_ARGS__)
#define PCE_CONNECTED_LOG_ERROR(fmt, ...) PBAP_PCE_LOG_ERROR("[Connected]" fmt, ##__VA_ARGS__)
#define PCE_DISCONNECTING_LOG_DEBUG(fmt, ...) PBAP_PCE_LOG_DEBUG("[Disconnecting]" fmt, ##__VA_ARGS__)
#define PCE_DISCONNECTING_LOG_INFO(fmt, ...) PBAP_PCE_LOG_INFO("[Disconnecting]" fmt, ##__VA_ARGS__)
#define PCE_DISCONNECTING_LOG_WARN(fmt, ...) PBAP_PCE_LOG_WARN("[Disconnecting]" fmt, ##__VA_ARGS__)
#define PCE_DISCONNECTING_LOG_ERROR(fmt, ...) PBAP_PCE_LOG_ERROR("[Disconnecting]" fmt, ##__VA_ARGS__)

namespace OHOS {
namespace bluetooth {
PbapPceStateMachine::PbapPceStateMachine(const RawAddress &dev, PbapPceServiceImpl &pceService, PbapPceSdp &pbapPceSdp,
    std::shared_ptr<IPbapPceObserver> observer)
    : device_(dev),
      pceService_(&pceService),
      pbapPceSdp_(pbapPceSdp),
      observer_(std::move(observer)),
      targetState_(PCE_TARGET_STATE_UNKNOWN)
{
    std::unique_ptr<State> disconnected =
        std::make_unique<PceDisconnectedState>(PCE_DISCONNECTED_STATE, *this, observer_);
    std::unique_ptr<State> connecting =
        std::make_unique<PceConnectingState>(PCE_CONNECTING_STATE, *this, observer_);
    std::unique_ptr<State> disconnecting =
        std::make_unique<PceDisconnectingState>(PCE_DISCONNECTING_STATE, *this, observer_);
    std::unique_ptr<State> connected = std::make_unique<PceConnectedState>(PCE_CONNECTED_STATE, *this, observer_);
    Move(disconnected);
    Move(connecting);
    Move(disconnecting);
    Move(connected);

    InitState(PCE_CONNECTING_STATE);
}

void PbapPceStateMachine::PullPhoneBook(const PbapPceAppParams &appParams)
{
    auto obexClient = GetObexClientPtr();
    if (obexClient == nullptr) {
        PBAP_PCE_LOG_ERROR("%{public}s end, pceObexClient_ is null", __PRETTY_FUNCTION__);
        return;
    }

    // 1. 创建 OBEX GET 请求头
    ObexHeader pbReqHeader;

    pbReqHeader.SetConnectionId(connectId_);
    // TYPE 头：ASCII 字符串字节直拷（与原公共 SetType 实现一致，不加 NUL 终止符）
    const std::string phonebookType = "x-bt/phonebook";
    pbReqHeader.SetHeader(TYPE, std::vector<uint8_t>(phonebookType.begin(), phonebookType.end()));
    pbReqHeader.SetName(appParams.GetName());
    // 绑定整个 GET 操作的接收超时（由 ObexClientSession::Get 传播到响应读取），
    // 防止对端停滞时 FFRT 任务被永久阻塞。
    pbReqHeader.SetRecvTimeoutMs(PBAP_PCE_GET_TIMEOUT_MS);

    appParams.AddToObexHeader(pbReqHeader);

    bool requestSize = (appParams.GetMaxListCount() == 0);

    SetPowerStatusBusy(true);

    obexClient->Get(pbReqHeader,
                    requestSize ? PCE_REQ_PULLPHONEBOOKSIZE : PCE_REQ_PULLPHONEBOOK);
}

int PbapPceStateMachine::GetTargetState() const
{
    return targetState_;
}

void PbapPceStateMachine::SetTargetState(int targetState)
{
    targetState_ = targetState;
}

void PbapPceStateMachine::TransitTargetState()
{
    PBAP_PCE_LOG_INFO("%{public}s start target=%{public}d", __PRETTY_FUNCTION__, GetTargetState());
    if (GetTargetState() == PCE_TARGET_STATE_CONNECTED) {
        GetPceService().Connect(GetDevice());
        SetTargetState(PCE_TARGET_STATE_UNKNOWN);
    } else if (GetTargetState() == PCE_TARGET_STATE_DISCONNECTED) {
        GetPceService().Disconnect(GetDevice());
        SetTargetState(PCE_TARGET_STATE_UNKNOWN);
    }
}

bool PbapPceStateMachine::IsBusy() const
{
    auto obexClient = GetObexClientPtr();
    if (obexClient == nullptr) {
        PBAP_PCE_LOG_ERROR("%{public}s, pceObexClient_ is null", __PRETTY_FUNCTION__);
        return false;
    }
    return obexClient->IsBusy();
}

bool PbapPceStateMachine::SetBusy(bool isBusy)
{
    auto obexClient = GetObexClientPtr();
    if (obexClient == nullptr) {
        PBAP_PCE_LOG_ERROR("%{public}s, pceObexClient_ is null", __PRETTY_FUNCTION__);
        return false;
    }
    obexClient->SetBusy(isBusy);
    return true;
}

void PbapPceStateMachine::SetPowerStatusBusy(bool isBusy)
{
    if (powerStatusBusy_ == isBusy) {
        return;
    }
    powerStatusBusy_ = isBusy;
    if (isBusy) {
        HILOGI("[%{public}s] PowerStatusUpdate -> BUSY", GetEncryptAddr(device_.GetAddress()).c_str());
        IPowerManager::GetInstance()->StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_PBAP_PCE, device_);
    } else {
        HILOGI("[%{public}s] PowerStatusUpdate -> IDLE", GetEncryptAddr(device_.GetAddress()).c_str());
        IPowerManager::GetInstance()->StatusUpdate(RequestStatus::IDLE, PROFILE_NAME_PBAP_PCE, device_);
    }
    return;
}

int PbapPceStateMachine::AbortDownloading()
{
    auto obexClient = GetObexClientPtr();
    if (obexClient == nullptr) {
        PBAP_PCE_LOG_ERROR("%{public}s, pceObexClient_ is null", __PRETTY_FUNCTION__);
        return RET_BAD_STATUS;
    }
    SetPowerStatusBusy(true);
    return obexClient->Abort();
}

void PbapPceStateMachine::SetConnectId(uint32_t connectId)
{
    connectId_ = connectId;
}

uint32_t PbapPceStateMachine::GetConnectId() const
{
    return connectId_;
}

void PbapPceStateMachine::ForceCloseObexClient()
{
    auto obexClient = GetObexClientPtr();
    if (obexClient == nullptr) {
        PBAP_PCE_LOG_WARN("%{public}s pceObexClient_ is null, skip", __PRETTY_FUNCTION__);
        return;
    }
    obexClient->ForceClose();
}

void PbapPceStateMachine::ResetObexClient()
{
    {
        std::lock_guard<std::mutex> lock(obexClientMutex_);
        pceObexClient_ = nullptr;
    }
}

void PbapPceStateMachine::HandlePhoneBookSizeActionCompleted(const utility::Message &msg)
{
    auto &device = GetDevice();
    if (observer_) {
        observer_->OnSyncStateChange(device, static_cast<int>(PBAP_PCE_PHONEBOOK_STATE_DOWNLOADING));
    }
}

void PbapPceStateMachine::HandlePullPhoneBookActionCompleted(const utility::Message &msg)
{
}

void PbapPceStateMachine::HandleAbortDownloadingActionCompleted(const utility::Message &msg)
{
    auto &device = GetDevice();
    if (observer_) {
        observer_->OnSyncStateChange(device, static_cast<int>(PBAP_PCE_PHONEBOOK_STATE_DOWNLOAD_ERROR));
    }
}

void PbapPceStateMachine::TryReleasePbapMsg(const utility::Message &msg) const
{
    if (msg.what_ == PCE_SDP_FINISH || msg.what_ == PCE_OBEX_CONNECTED) {
        ReleasePbapMsgPayload(msg);
        PBAP_PCE_LOG_ERROR("%{public}s msg.what_=[%{public}d] release memory", __PRETTY_FUNCTION__, msg.what_);
    }
}

void PbapPceStateMachine::ReleasePbapMsgPayload(const utility::Message &msg)
{
    switch (msg.what_) {
        case PCE_SDP_FINISH:
            delete static_cast<PbapPceHeaderSdpMsg *>(msg.arg2_);
            break;
        case PCE_OBEX_CONNECTED:
            delete static_cast<PbapPceObexMessage *>(msg.arg2_);
            break;
        default:
            break;
    }
}

void PbapPceStateMachine::MarkConnectFailed()
{
    PBAP_PCE_LOG_INFO("%{public}s connectFailed_ set to true", __PRETTY_FUNCTION__);
    connectFailed_ = true;
}

bool PbapPceStateMachine::IsConnectFailed() const
{
    return connectFailed_;
}

void PbapPceStateMachine::ClearConnectFailed()
{
    PBAP_PCE_LOG_INFO("%{public}s connectFailed_ cleared", __PRETTY_FUNCTION__);
    connectFailed_ = false;
}

const RawAddress &PbapPceStateMachine::GetDevice() const
{
    return this->device_;
}

PbapPceServiceImpl &PbapPceStateMachine::GetPceService() const
{
    return *pceService_;
}

std::shared_ptr<PbapPceObexClient> PbapPceStateMachine::GetObexClientPtr() const
{
    std::lock_guard<std::mutex> lock(obexClientMutex_);
    return pceObexClient_;
}

void PbapPceStateMachine::SetVersionNumber(uint16_t versionNumber)
{
    versionNumber_ = versionNumber;
}

uint16_t PbapPceStateMachine::GetVersionNumber() const
{
    return versionNumber_;
}

void PbapPceStateMachine::SetSupportedRes(uint8_t supportedRes)
{
    supportedRes_ = supportedRes;
}

uint8_t PbapPceStateMachine::GetSupportedRes() const
{
    return supportedRes_;
}

void PbapPceStateMachine::SetSupportedFeature(uint32_t supportedFeature)
{
    supportedFeature_ = supportedFeature;
}

uint32_t PbapPceStateMachine::GetSupportedFeature() const
{
    return supportedFeature_;
}

bool PbapPceStateMachine::GetFeatureFlag() const
{
    return featureFlag_;
}

void PbapPceStateMachine::SetFeatureFlag(bool featureFlag)
{
    featureFlag_ = featureFlag;
}

PbapPceSdp &PbapPceStateMachine::GetSdp() const
{
    return pbapPceSdp_;
}

PbapPceStateMachine::~PbapPceStateMachine()
{
}

void PbapPceStateMachine::CreatePceObexClient(const PbapPceObexConfig &obexConfig)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s, rfcomm=%{public}u, l2capPsm=%{public}u, "
        "isL2capPSM=%{public}d, mtu=%{public}u", __PRETTY_FUNCTION__,
        GetEncryptAddr(obexConfig.addr).c_str(), obexConfig.rfCommChannel, obexConfig.l2capPsm,
        obexConfig.isL2capPSM, obexConfig.mtu);
    if (pceService_ == nullptr) {
        PBAP_PCE_LOG_ERROR("pceService_ == nullptr");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(obexClientMutex_);
        this->pceObexClient_ = std::make_shared<PbapPceObexClient>(obexConfig, *pceService_);
        PBAP_PCE_LOG_INFO("%{public}s end, pceObexClient_=%{public}p", __PRETTY_FUNCTION__, pceObexClient_.get());
    }
}

// ---------------------------------------------------------------------------
// PceBaseState
// ---------------------------------------------------------------------------
PceBaseState::PceBaseState(
    const std::string &name,
    PbapPceStateMachine &sm,
    std::shared_ptr<IPbapPceObserver> observer)
    : State(name, sm), stm_(sm), observer_(std::move(observer))
{}

// ---------------------------------------------------------------------------
// PceDisconnectedState
// ---------------------------------------------------------------------------
PceDisconnectedState::PceDisconnectedState(
    const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer)
    : PceBaseState(name, sm, std::move(observer))
{}

void PceDisconnectedState::Entry()
{
    PCE_DISCONNECTED_LOG_INFO("%{public}s, observer->OnServiceConnectionStateChanged", __PRETTY_FUNCTION__);
    auto &device = stm_.GetDevice();
    IPowerManager::GetInstance()->StatusUpdate(RequestStatus::CONNECT_OFF, PROFILE_NAME_PBAP_PCE, device);
    if (observer_) {
        observer_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::DISCONNECTED), 0);
    }
    // 连接尝试失败（对端拒绝/超时/SDP失败等）时，在 DISCONNECTED 通知之后将同步状态
    // 置为 DOWNLOAD_ERROR 并通知应用层。必须在 OnConnectionStateChanged(DISCONNECTED)
    // 之后执行，否则会被连接状态回调覆盖。该状态保持到下次发起连接或连接成功后被覆盖。
    if (stm_.IsConnectFailed()) {
        stm_.ClearConnectFailed();
        if (observer_) {
            observer_->OnSyncStateChange(device, static_cast<int>(PBAP_PCE_PHONEBOOK_STATE_DOWNLOAD_ERROR));
        }
    }
    // 清理 obex client 引用，确保重连时 CreatePceObexClient 新建对象。
    // ffrt 下载线程持有的 shared_ptr 仍保持 obexClient 存活，直到其 lambda 退出。
    stm_.ResetObexClient();
    stm_.TransitTargetState();
}

void PceDisconnectedState::Exit()
{
}

bool PceDisconnectedState::Dispatch(const utility::Message &msg)
{
    switch (msg.what_) {
        case PCE_REQ_SET_TARGET_STATE:
            PCE_DISCONNECTED_LOG_DEBUG("%{public}s handling PCE_REQ_SET_TARGET_STATE, arg1=%{public}d",
                __PRETTY_FUNCTION__, msg.arg1_);
            stm_.SetTargetState(msg.arg1_);
            break;
        case PCE_REQ_TRANSIT_TARGET_STATE:
            PCE_DISCONNECTED_LOG_DEBUG("%{public}s handling PCE_REQ_TRANSIT_TARGET_STATE", __PRETTY_FUNCTION__);
            stm_.TransitTargetState();
            break;
        default:
            PCE_DISCONNECTED_LOG_WARN("%{public}s unhandled msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
            stm_.TryReleasePbapMsg(msg);
            break;
    }
    return true;
}

// ---------------------------------------------------------------------------
// PceConnectingState
// ---------------------------------------------------------------------------
PceConnectingState::PceConnectingState(
    const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer)
    : PceBaseState(name, sm, std::move(observer)),
      authDescription_(),
      authNonce_(),
      authUserCharset_(0),
      authFullAccess_(false),
      authNeedUser_(false),
      authChallenge_(false),
      header_(nullptr)
{}

void PceConnectingState::Entry()
{
    PCE_CONNECTING_LOG_INFO("observer_ -> CONNECTING");
    auto device = stm_.GetDevice();
    if (observer_) {
        observer_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::CONNECTING), 0);
    } else {
        PCE_CONNECTING_LOG_ERROR("%{public}s observer_ is null!", __PRETTY_FUNCTION__);
    }
    PCE_CONNECTING_LOG_INFO("%{public}s calling SdpSearch, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(stm_.GetDevice().GetAddress()).c_str());
    int retVal = stm_.GetSdp().SdpSearch(stm_.GetDevice().GetAddress());
    PCE_CONNECTING_LOG_INFO("%{public}s SdpSearch returned=%{public}d", __PRETTY_FUNCTION__, retVal);
    if (retVal != Bluetooth::BT_NO_ERROR) {
        PCE_CONNECTING_LOG_ERROR("%{public}s end, pce client Call SDP_ServiceSearchAttribute Error",
            __PRETTY_FUNCTION__);
        stm_.MarkConnectFailed();
        Transition(PCE_DISCONNECTING_STATE);
        // 已流转离开本状态：不得再创建/启动本状态的连接超时定时器（否则定时器
        // 无人 Stop，到期后跨线程二次流转状态机）
        return;
    }
    const int CONNECT_TIMEOUT_MS = 10000;
    // P0-2: 超时回调经 DoInPbapThread 转投 PCE_CONNECT_TIMEOUT 消息，由状态机在
    // PbapThread 串行处理，避免 timer 线程直接 Transition 与 PbapThread 并发操作
    // 状态栈。若消息到达时已离开 CONNECTING 状态，由新状态的 Dispatch 安全忽略
    //（PCE_CONNECT_TIMEOUT 分支仅存在于 ConnectingState）。
    auto weakSm = stm_.weak_from_this();
    connectTimer_ = std::make_shared<utility::Timer>([weakSm]() {
        DoInPbapThread([weakSm]() {
            auto sm = weakSm.lock();
            if (sm != nullptr) {
                sm->ProcessMessage(utility::Message(PCE_CONNECT_TIMEOUT));
            }
        });
    });
    connectTimer_->Start(CONNECT_TIMEOUT_MS);
}

void PceConnectingState::Exit()
{
    if (connectTimer_ != nullptr) {
        connectTimer_->Stop();
        connectTimer_ = nullptr;
    }
}

int PceConnectingState::CreateObexClient(const PbapPceHeaderSdpMsg &sdpMsg)
{
    // set version, repository, feature to sm
    stm_.SetVersionNumber(sdpMsg.GetVersionNumber());
    stm_.SetSupportedRes(sdpMsg.GetSupportedRes());
    stm_.SetSupportedFeature(sdpMsg.GetSupportedFeature());
    stm_.SetFeatureFlag(sdpMsg.GetFeatureFlag());

    obexConfig_ = sdpMsg.GetObexClientConfig();
    SetObexClientConfigDetail(obexConfig_);
    stm_.CreatePceObexClient(obexConfig_);
    return true;
}

void PceConnectingState::InitAuth()
{
    authDescription_.clear();
    authChallenge_ = true;
    authFullAccess_ = true;
    authNeedUser_ = false;
    authNonce_.clear();
}


void PceConnectingState::SetObexClientConfigDetail(PbapPceObexConfig &obexConfig) const
{
    obexConfig.isSupportReliableSession = false;
    // SRM 总开关来自 PbapPceService 配置（SrmEnable，默认 true）。SRM 在 L2CAP/RFCOMM
    // 上均可使用（OBEX SRM 与传输层无关）；实际激活在 ObexClientSession::Get() 中按 GET
    // 操作协商，不支持 SRM 的对端自动回退经典 ping-pong 模式。
    obexConfig.isSupportSrm = stm_.GetPceService().GetPceConfig().srmEnable_;
    if (obexConfig.isL2capPSM) {
        obexConfig.mtu = stm_.GetPceService().GetPceConfig().l2capMtu_;
    } else {
        obexConfig.mtu = stm_.GetPceService().GetPceConfig().rfcommMtu_;
    }
}

void PceConnectingState::ProcessObexConnected(const utility::Message &msg)
{
    PCE_CONNECTING_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    // 接管 arg2_ 裸指针所有权（生产方 obex_client HandleConnectSuccess release()），
    // 作用域结束自动释放；消息落到其他状态时由 TryReleasePbapMsg 兜底释放
    std::unique_ptr<PbapPceObexMessage> obexMsg(static_cast<PbapPceObexMessage *>(msg.arg2_));
    if (obexMsg != nullptr) {
        auto connectId = obexMsg->GetObexHeader().GetConnectionId();
        if (connectId != INVALID_CONNECTION_ID) {
            stm_.SetConnectId(connectId);
        }
    }
    stm_.ClearConnectFailed();
    Transition(PCE_CONNECTED_STATE);
    PCE_CONNECTING_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
}

void PceConnectingState::ProcessSdpFinish(const utility::Message &msg)
{
    PCE_CONNECTING_LOG_INFO("%{public}s start, msg.what_=[%{public}d], arg2=%{public}p", __PRETTY_FUNCTION__, msg.what_,
        msg.arg2_);
    std::unique_ptr<PbapPceHeaderSdpMsg> sdpMsg(static_cast<PbapPceHeaderSdpMsg *>(msg.arg2_));
    if (sdpMsg == nullptr) {
        PCE_CONNECTING_LOG_ERROR("sdpMsg is null, %{public}s end, msg.what_=[%{public}d], arg2=%{public}p",
            __PRETTY_FUNCTION__, msg.what_, msg.arg2_);
        return;
    }
    PCE_CONNECTING_LOG_INFO("%{public}s sdpMsg valid, version=%{public}u, supportedRes=%{public}u, "
        "supportedFeature=%{public}u, featureFlag=%{public}d", __PRETTY_FUNCTION__,
        sdpMsg->GetVersionNumber(), sdpMsg->GetSupportedRes(), sdpMsg->GetSupportedFeature(),
        sdpMsg->GetFeatureFlag());
    int createRet = CreateObexClient(*sdpMsg);
    PCE_CONNECTING_LOG_INFO("%{public}s CreateObexClient returned=%{public}d", __PRETTY_FUNCTION__, createRet);
    if (!createRet) {
        PCE_CONNECTING_LOG_ERROR("%{public}s CreateObexClient failed, transitioning to DISCONNECTING",
            __PRETTY_FUNCTION__);
        stm_.MarkConnectFailed();
        Transition(PCE_DISCONNECTING_STATE);
    } else {
        auto self = stm_.GetObexClientPtr();
        PCE_CONNECTING_LOG_INFO("%{public}s obexClient=%{public}p", __PRETTY_FUNCTION__, self.get());
        if (self != nullptr) {
            PCE_CONNECTING_LOG_INFO("%{public}s calling obexClient->Connect, featureFlag=%{public}d",
                __PRETTY_FUNCTION__, stm_.GetFeatureFlag());
            self->Connect(self, stm_.GetFeatureFlag());
            PCE_CONNECTING_LOG_INFO("%{public}s obexClient->Connect returned", __PRETTY_FUNCTION__);
        } else {
            PCE_CONNECTING_LOG_ERROR("%{public}s obexClient is null! cannot proceed with Connect", __PRETTY_FUNCTION__);
        }
    }
    PCE_CONNECTING_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
}

void PceConnectingState::ProcessObexConnectFailed(const utility::Message &msg)
{
    PCE_CONNECTING_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    stm_.MarkConnectFailed();
    Transition(PCE_DISCONNECTING_STATE);
    PCE_CONNECTING_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
}

void PceConnectingState::ProcessObexTransportFailed(const utility::Message &msg)
{
    PCE_CONNECTING_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    PCE_CONNECTING_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
}

void PceConnectingState::OnConnectTimeout()
{
    PCE_CONNECTING_LOG_WARN("%{public}s connect timeout, closing obexClient", __PRETTY_FUNCTION__);
    stm_.MarkConnectFailed();
    auto obexClient = stm_.GetObexClientPtr();
    if (obexClient != nullptr) {
        // OBEX client exists -> tear down via DISCONNECTING (sends OBEX DISCONNECT,
        // waits for response, then transitions to DISCONNECTED).
        obexClient->ObexConnectFailed();
    } else {
        // P1-1: obexClient_ is null means we are still in the SDP phase
        // (CreateObexClient has not run, e.g. SDP response never arrived or the
        // ForwardMsgToStm dispatch was delayed). There is no OBEX transport to
        // disconnect, so skip the DISCONNECTING state (which would only wait 3s on
        // an OBEX DISCONNECT response that can never come) and transition directly
        // to DISCONNECTED. Any late PCE_SDP_FINISH arriving afterwards is released
        // by PceDisconnectedState::Dispatch (TryReleasePbapMsg), so it is safe.
        // Previously this branch only did Transition(PCE_DISCONNECTING_STATE), see
        // hilog.040 17:02:41.888 "pceObexClient_ is null, skip" where the timeout
        // effectively became a no-op while the FFRT worker was blocked holding
        // machineMapMutex_.
        PCE_CONNECTING_LOG_WARN("%{public}s obexClient null (SDP phase), transition directly to DISCONNECTED",
            __PRETTY_FUNCTION__);
        Transition(PCE_DISCONNECTED_STATE);
    }
}

bool PceConnectingState::Dispatch(const utility::Message &msg)
{
    PCE_CONNECTING_LOG_INFO("%{public}s start, state=CONNECTING, msg.what_=[%{public}d]",
        __PRETTY_FUNCTION__, msg.what_);
    switch (msg.what_) {
        case PCE_SDP_FINISH:
            ProcessSdpFinish(msg);
            break;
        case PCE_REQ_DISCONNECTED:
            ProcessReqDisconnected(msg);
            break;
        case PCE_OBEX_CONNECTED:
            ProcessObexConnected(msg);
            break;
        case PCE_SDP_FAILED:
            ProcessSdpFailed(msg);
            break;
        case PCE_OBEX_CONNECT_FAILED:
            ProcessObexConnectFailed(msg);
            break;
        case PCE_REQ_SET_TARGET_STATE:
            PCE_CONNECTING_LOG_INFO("%{public}s handling PCE_REQ_SET_TARGET_STATE, arg1=%{public}d",
                __PRETTY_FUNCTION__, msg.arg1_);
            stm_.SetTargetState(msg.arg1_);
            break;
        case PCE_REQ_TRANSIT_TARGET_STATE:
            stm_.TransitTargetState();
            break;
        case PCE_OBEX_DISCONNECTED:
            PCE_CONNECTING_LOG_INFO("%{public}s handling PCE_OBEX_DISCONNECTED", __PRETTY_FUNCTION__);
            stm_.MarkConnectFailed();
            Transition(PCE_DISCONNECTING_STATE);
            Transition(PCE_DISCONNECTED_STATE);
            break;
        case PCE_TRANSPORT_FAILED:
            ProcessObexTransportFailed(msg);
            break;
        case PCE_CONNECT_TIMEOUT:
            OnConnectTimeout();
            break;
        default:
            PCE_CONNECTING_LOG_ERROR("%{public}s unhandled msg.what_=[%{public}d], returning false",
                __PRETTY_FUNCTION__, msg.what_);
            // 兜底释放携带 arg2_ 裸指针的消息负载（与其他状态的 default 分支一致）
            stm_.TryReleasePbapMsg(msg);
            return false;
    }

    PCE_CONNECTING_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    return true;
}
void PceConnectingState::ProcessReqDisconnected(const utility::Message &msg)
{
    auto obexClient = stm_.GetObexClientPtr();
    if (obexClient != nullptr) {
        obexClient->Disconnect(false);
    }
    Transition(PCE_DISCONNECTING_STATE);
}

void PceConnectingState::ProcessSdpFailed(const utility::Message &msg)
{
    stm_.MarkConnectFailed();
    Transition(PCE_DISCONNECTING_STATE);
    Transition(PCE_DISCONNECTED_STATE);
}

// ---------------------------------------------------------------------------
// PceConnectedState
// ---------------------------------------------------------------------------
PceConnectedState::PceConnectedState(
    const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer)
    : PceBaseState(name, sm, std::move(observer))
{}

void PceConnectedState::Entry()
{
    PCE_CONNECTED_LOG_INFO("%{public}s, observer->OnConnectionStateChanged", __PRETTY_FUNCTION__);
    auto &device = stm_.GetDevice();
    IPowerManager::GetInstance()->StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_PBAP_PCE, device);
    if (observer_) {
        observer_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::CONNECTED), 0);
    }
    // 将 PCE_REQ_DOWNLOAD 延迟到下一事件循环，避免在 Entry 调用栈内递归 Dispatch。
    // 若延迟任务执行前状态已变化（如断连），PCE_REQ_DOWNLOAD 会被新状态的 Dispatch
    // 安全忽略。weak_ptr 捕获：与动态库空闲卸载（delete ServiceImpl → 释放 SM）
    // 竞态时 lock 失败即丢弃任务，避免悬垂指针 UAF。
    auto weakSm = stm_.weak_from_this();
    DoInPbapThread([weakSm]() {
        auto sm = weakSm.lock();
        if (sm != nullptr) {
            sm->ProcessMessage(utility::Message(PCE_REQ_DOWNLOAD));
        }
    });
    stm_.TransitTargetState();
}

void PceConnectedState::Exit()
{
    PCE_CONNECTED_LOG_DEBUG("%{public}s Exit", __PRETTY_FUNCTION__);
}

void PceConnectedState::ReqPullPhoneBook(const utility::Message &msg) const
{
    // 🔴 接收方负责释放内存
    std::unique_ptr<PbapPceAppParams> appParams(
        static_cast<PbapPceAppParams*>(msg.arg2_));
    
    if (!appParams) {
        PCE_CONNECTED_LOG_ERROR("appParams is null");
        return;
    }
    
    // 直接使用 appParams
    stm_.PullPhoneBook(*appParams);
}

bool PceConnectedState::DispatchActionCompleted(const utility::Message &msg) const
{
    bool ret = true;

    switch (msg.what_) {
        case PCE_PULLPHONEBOOKSIZE_COMPLETED:
            stm_.HandlePhoneBookSizeActionCompleted(msg);
            break;
        case PCE_PULLPHONEBOOK_COMPLETED:
            stm_.HandlePullPhoneBookActionCompleted(msg);
            break;
        case PCE_ABORTDOWNLOADING_COMPLETED:
            stm_.HandleAbortDownloadingActionCompleted(msg);
            break;
        default:
            ret = false;
            stm_.TryReleasePbapMsg(msg);
            PCE_CONNECTED_LOG_ERROR("%{public}s msg.what_=%{public}d can't be handled.",
            __PRETTY_FUNCTION__, msg.what_);
            break;
    }
    return ret;
}

bool PceConnectedState::Dispatch(const utility::Message &msg)
{
    bool ret = true;

    switch (msg.what_) {
        case PCE_REQ_DISCONNECTED:
            Transition(PCE_DISCONNECTING_STATE);
            stm_.ForceCloseObexClient();
            break;
        case PCE_TRANSPORT_FAILED:
            PCE_CONNECTED_LOG_WARN("%{public}s PCE_TRANSPORT_FAILED, force close and transition", __PRETTY_FUNCTION__);
            Transition(PCE_DISCONNECTING_STATE);
            stm_.ForceCloseObexClient();
            break;
        case PCE_REQ_SET_TARGET_STATE:
            stm_.SetTargetState(msg.arg1_);
            break;
        case PCE_REQ_TRANSIT_TARGET_STATE:
            stm_.TransitTargetState();
            break;
        case PCE_OBEX_DISCONNECTED:
            Transition(PCE_DISCONNECTING_STATE);
            Transition(PCE_DISCONNECTED_STATE);
            break;
        case PCE_REQ_PULLPHONEBOOK:
            ReqPullPhoneBook(msg);
            break;
        case PCE_REQ_DOWNLOAD:
            ProcessReqDownload(msg);
            break;
        case PCE_REQ_ABORTDOWNLOADING:
            stm_.AbortDownloading();
            break;
        case PCE_DOWNLOAD_COMPLETE:
            ProcessDownloadComplete(msg);
            break;
        case PCE_DOWNLOAD_FAILED:
            PCE_CONNECTED_LOG_WARN("%{public}s PCE_DOWNLOAD_FAILED received", __PRETTY_FUNCTION__);
            if (observer_) {
                observer_->OnSyncStateChange(stm_.GetDevice(),
                    static_cast<int>(PBAP_PCE_PHONEBOOK_STATE_DOWNLOAD_ERROR));
            }
            stm_.TransitTargetState();
            break;
        default:
            ret = DispatchActionCompleted(msg);
            break;
    }
    return ret;
}

void PceConnectedState::ProcessReqDownload(const utility::Message &msg)
{
    // 统一的下载触发入口：Entry 自动触发与外部 Connect 手动触发均经此处理
    // 禁止在此处理中触发 Transition（Entry 调用栈内同步分发，递归 Transition 会破坏状态栈）
    auto &dev = stm_.GetDevice();
    if (stm_.IsBusy()) {
        PCE_CONNECTED_LOG_INFO("PCE_REQ_DOWNLOAD ignored, already busy, addr=%{public}s",
            GetEncryptAddr(dev.GetAddress()).c_str());
        return;
    }
    auto obexClient = stm_.GetObexClientPtr();
    if (obexClient == nullptr) {
        PCE_CONNECTED_LOG_ERROR("PCE_REQ_DOWNLOAD failed, obexClient null, addr=%{public}s",
            GetEncryptAddr(dev.GetAddress()).c_str());
        return;
    }
    PCE_CONNECTED_LOG_INFO("download triggered via PCE_REQ_DOWNLOAD, addr=%{public}s",
        GetEncryptAddr(dev.GetAddress()).c_str());
    DoInPbapSyncThread([obexClient]() {
        obexClient->DownloadContactsAndCallLogs();
    });
}

void PceConnectedState::ProcessDownloadComplete(const utility::Message &msg)
{
    PCE_CONNECTED_LOG_INFO("%{public}s PCE_DOWNLOAD_COMPLETE received", __PRETTY_FUNCTION__);
    if (observer_) {
        observer_->OnSyncStateChange(stm_.GetDevice(),
            static_cast<int>(PBAP_PCE_PHONEBOOK_STATE_DOWNLOADED));
    }
    stm_.TransitTargetState();
}

// ---------------------------------------------------------------------------
// PceDisconnectingState
// ---------------------------------------------------------------------------
PceDisconnectingState::PceDisconnectingState(
    const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer)
    : PceBaseState(name, sm, std::move(observer))
{}

void PceDisconnectingState::Entry()
{
    PCE_DISCONNECTING_LOG_INFO("%{public}s, Observer()->OnServiceConnectionStateChanged()->DISCONNECTING",
        __PRETTY_FUNCTION__);
    auto &device = stm_.GetDevice();
    if (observer_) {
        observer_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::DISCONNECTING), 0);
    }

    const int DISCONNECT_TIMEOUT_MS = 3000;
    // P0-2: 超时回调经 DoInPbapThread 转投 PCE_DISCONNECT_TIMEOUT 消息，由状态机
    // 在 PbapThread 串行处理，避免 timer 线程直接 Transition/ForceClose 与
    // PbapThread 并发操作状态栈。消息到达时若已离开本状态，由新状态安全忽略。
    auto weakSm = stm_.weak_from_this();
    disconnectTimer_ = std::make_shared<utility::Timer>([weakSm]() {
        DoInPbapThread([weakSm]() {
            auto sm = weakSm.lock();
            if (sm != nullptr) {
                sm->ProcessMessage(utility::Message(PCE_DISCONNECT_TIMEOUT));
            }
        });
    });
    disconnectTimer_->Start(DISCONNECT_TIMEOUT_MS);
}

void PceDisconnectingState::Exit()
{
    PCE_DISCONNECTING_LOG_DEBUG("%{public}s Exit", __PRETTY_FUNCTION__);
    if (disconnectTimer_ != nullptr) {
        disconnectTimer_->Stop();
        disconnectTimer_ = nullptr;
    }
}

void PceDisconnectingState::OnDisconnectTimeout()
{
    PCE_DISCONNECTING_LOG_WARN("Disconnect timeout, force closing socket and transition to DISCONNECTED");
    stm_.ForceCloseObexClient();
    Transition(PCE_DISCONNECTED_STATE);
}

bool PceDisconnectingState::Dispatch(const utility::Message &msg)
{
    uint8_t ret = true;
    switch (msg.what_) {
        case PCE_OBEX_DISCONNECTED:
            PCE_DISCONNECTING_LOG_DEBUG("%{public}s handling PCE_OBEX_DISCONNECTED", __PRETTY_FUNCTION__);
            Transition(PCE_DISCONNECTED_STATE);
            break;
        case PCE_DOWNLOAD_COMPLETE:
            PCE_DISCONNECTING_LOG_INFO("%{public}s PCE_DOWNLOAD_COMPLETE ignored in DISCONNECTING",
                __PRETTY_FUNCTION__);
            break;
        case PCE_DISCONNECT_TIMEOUT:
            PCE_DISCONNECTING_LOG_DEBUG("%{public}s handling PCE_DISCONNECT_TIMEOUT", __PRETTY_FUNCTION__);
            OnDisconnectTimeout();
            break;
        case PCE_REQ_SET_TARGET_STATE:
            PCE_DISCONNECTING_LOG_DEBUG("%{public}s handling PCE_REQ_SET_TARGET_STATE, arg1=%{public}d",
                __PRETTY_FUNCTION__, msg.arg1_);
            stm_.SetTargetState(msg.arg1_);
            break;
        case PCE_REQ_TRANSIT_TARGET_STATE:
            PCE_DISCONNECTING_LOG_DEBUG("%{public}s handling PCE_REQ_TRANSIT_TARGET_STATE", __PRETTY_FUNCTION__);
            stm_.TransitTargetState();
            break;
        default:
            stm_.TryReleasePbapMsg(msg);
            return false;
    }

    return ret;
}
}  // namespace bluetooth
}  // namespace OHOS
