/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "avrcp_ct_bip_client"
#endif

#include "avrcp_ct_bip_client.h"
#include "profile_service_manager.h"
#include "socket_service.h"
#include "bt_uuid.h"
#include "bt_sock.h"
#include "obex_def.h"
#include <sys/socket.h>
#include <unistd.h>

namespace OHOS {
namespace bluetooth {

static const uint8_t BIP_UUID_AVRCP_COVER_ART[] = {0x71, 0x63, 0xDD, 0x54, 0x4A, 0x7E, 0x11, 0xE2,
                                                   0xB4, 0x7C, 0x00, 0x50, 0xC2, 0x49, 0x00, 0x48};

AvrcpCtBipClient::AvrcpCtBipClient(const std::string &deviceAddr, uint16_t psm, std::shared_ptr<Callback> callback)
    : deviceAddr_(deviceAddr),
      psm_(psm),
      state_(STATE_DISCONNECTED),
      callback_(callback),
      socketFd_(-1)
{
    HILOGI("AvrcpCtBipClient created psm: %{public}d", psm);
}

AvrcpCtBipClient::~AvrcpCtBipClient()
{
    HILOGI("AvrcpCtBipClient destroying");
}

bool AvrcpCtBipClient::Connect()
{
    // 构造后由调用方触发 CONNECT，投递到 BIP 线程异步执行。
    // 此时对象已被 shared_ptr 管理（CoverArtManager::Connect 中 make_shared 后存入 clients_），
    // 可安全使用 shared_from_this() 保证 lambda 执行时对象生命周期。
    auto self = shared_from_this();
    PostToBipThread([self]() { self->ProcessConnect(); });
    return true;
}

void AvrcpCtBipClient::Disconnect()
{
    HILOGI("Posting DISCONNECT to BIP thread");
    auto self = shared_from_this();
    PostToBipThread([self]() { self->ProcessDisconnect(); });
}

void AvrcpCtBipClient::Shutdown()
{
    HILOGI("Shutdown: posting DISCONNECT to BIP thread");
    // shutdown()：投递 DISCONNECT 到 BIP 线程异步执行。
    // 用 shared_from_this 保证 lambda 执行时对象生命周期，
    // CoverArtManager::Disconnect 会在 Shutdown 后 erase clients_，
    // 回调 OnConnectionStateChanged(STATE_DISCONNECTED) 时 GetClient 返回 null，不会误重连。
    auto self = shared_from_this();
    PostToBipThread([self]() { self->ProcessDisconnect(); });
}

int AvrcpCtBipClient::GetState() const
{
    return state_;
}

bool AvrcpCtBipClient::IsConnected() const
{
    return state_ == STATE_CONNECTED && clientSession_ != nullptr;
}

uint16_t AvrcpCtBipClient::GetPsm() const
{
    return psm_;
}

bool AvrcpCtBipClient::GetImageProperties(const std::string &imageHandle)
{
    if (!IsConnected()) {
        HILOGE("Not connected");
        return false;
    }

    auto self = shared_from_this();
    auto request = std::make_shared<RequestGetImageProperties>(imageHandle);
    PostToBipThread([self, request]() { self->ProcessRequest(request); });
    return true;
}

bool AvrcpCtBipClient::GetImage(const std::string &imageHandle, const BipImageDescriptor &descriptor)
{
    if (!IsConnected()) {
        HILOGE("Not connected");
        return false;
    }

    auto self = shared_from_this();
    auto request = std::make_shared<RequestGetImage>(imageHandle, descriptor);
    PostToBipThread([self, request]() { self->ProcessRequest(request); });
    return true;
}

void AvrcpCtBipClient::PostToBipThread(const ThreadUtilFunc &func)
{
    DoInBipThread(func);
}

void AvrcpCtBipClient::ProcessConnect()
{
    HILOGI("Processing CONNECT in BIP thread");

    if (state_ == STATE_CONNECTED) {
        HILOGW("Already connected");
        return;
    }

    SetConnectionState(STATE_CONNECTING);

    if (!ConnectL2cap()) {
        HILOGE("Failed to connect L2CAP");
        SetConnectionState(STATE_DISCONNECTED);
        return;
    }

    if (!CreateObexSession()) {
        HILOGE("Failed to create OBEX session");
        DestroyObexSession();
        SetConnectionState(STATE_DISCONNECTED);
        return;
    }

    SetConnectionState(STATE_CONNECTED);
    HILOGI("BIP connection established");
}

void AvrcpCtBipClient::ProcessDisconnect()
{
    HILOGI("Processing DISCONNECT in BIP thread");

    if (state_ == STATE_DISCONNECTED) {
        return;
    }

    SetConnectionState(STATE_DISCONNECTING);
    DestroyObexSession();
    SetConnectionState(STATE_DISCONNECTED);
    HILOGI("BIP disconnected");
}

void AvrcpCtBipClient::ProcessRequest(std::shared_ptr<BipRequest> request)
{
    if (!request || !callback_) {
        return;
    }

    if (!IsConnected()) {
        HILOGE("Not connected, cannot process request");
        NotifyCaller(request);
        return;
    }
    request->SetConnectionId(connectHeader_.GetConnectionId());

    HILOGI("Executing request: %{public}s", request->ToString().c_str());
    // executeRequest：请求失败时销毁 OBEX 会话（不触发重连，避免持续失败循环）
    if (!request->Execute(clientSession_)) {
        HILOGE("Request execute failed, destroying OBEX session");
        NotifyCaller(request);
        DestroyObexSession();
        SetConnectionState(STATE_DISCONNECTED);
        return;
    }
    NotifyCaller(request);
}

void AvrcpCtBipClient::NotifyCaller(std::shared_ptr<BipRequest> request)
{
    if (!callback_ || !request) {
        return;
    }

    std::string imageHandle = request->GetImageHandle();
    int type = request->GetType();

    switch (type) {
        case BipRequest::TYPE_GET_IMAGE_PROPERTIES: {
            auto req = std::static_pointer_cast<RequestGetImageProperties>(request);
            callback_->OnGetImagePropertiesComplete(req->GetResponseCode(), imageHandle, req->GetProperties());
            break;
        }
        case BipRequest::TYPE_GET_IMAGE: {
            auto req = std::static_pointer_cast<RequestGetImage>(request);
            callback_->OnGetImageComplete(req->GetResponseCode(), imageHandle, req->GetImage());
            break;
        }
        default: {
            HILOGE("Unknown request type: %{public}d", type);
            break;
        }
    }
}

bool AvrcpCtBipClient::ConnectL2cap()
{
    HILOGI("Connecting L2CAP via SocketService, psm: %{public}d", psm_);

    auto *profile = IProfileManager::GetInstance()->GetProfileService("SocketService");
    auto *socketService = static_cast<SocketService *>(profile);
    if (!socketService) {
        HILOGE("Failed to get SocketService");
        return false;
    }

    Uuid uuid = Uuid();
    int securityFlags = 0;
    int socketType = 1;

    socketFd_ = socketService->Connect(deviceAddr_, uuid, securityFlags, socketType, psm_);
    if (socketFd_ < 0) {
        HILOGE("SocketService::Connect failed, fd=%{public}d", socketFd_);
        return false;
    }

    HILOGI("L2CAP connected via SocketService, fd=%{public}d", socketFd_);

    // SocketService::Connect 返回的 fd 上先传一段握手信息（4 字节 channel + sock_connect_info_t），
    // 必须先 recv 掉，否则 OBEX ReadResponse 会把 channel 字节误读为 responseCode。
    // 对齐 OppObexClient::RecvSocketDevice / MapMseMnsClient::RecvSocketDevice。
    socketDevice_ = RecvSocketDevice();
    if (!socketDevice_) {
        HILOGE("RecvSocketDevice failed, closing socket");
        close(socketFd_);
        socketFd_ = -1;
        return false;
    }

    return true;
}

std::shared_ptr<ObexSocketDevice> AvrcpCtBipClient::RecvSocketDevice()
{
    HILOGI("RecvSocketDevice start, fd=%{public}d", socketFd_);
 
    // 对齐 MNS/OPP：设置 5s 接收超时，避免握手数据未到时永久阻塞
    struct timeval time = {5, 0};
    setsockopt(socketFd_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&time), sizeof(time));
 
    int32_t channel = 0;
    auto ret = recv(socketFd_, &channel, sizeof(channel), MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[bip] recv channel error, ret=%{public}d", ret);
        return nullptr;
    }
    HILOGI("[bip] recv channel=%{public}d", channel);
 
    sock_connect_info_t cs;
    ret = recv(socketFd_, &cs, sizeof(sock_connect_info_t), MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[bip] recv sock_connect_info_t error, ret=%{public}d", ret);
        return nullptr;
    }
    HILOGI("[bip] cs.status=%{public}d, max_tx=%{public}d, max_rx=%{public}d",
        static_cast<int>(cs.status), cs.max_tx_packet_size, cs.max_rx_packet_size);
 
    if (!cs.status) {
        HILOGE("[bip] socket connect status false");
        return nullptr;
    }
 
    // L2CAP 使用对端协商的真实 MTU；RFCOMM 用 MAX_PACKET_SIZE。
    // 当前 BIP 客户端固定走 L2CAP（SocketService::Connect 传 type=1）。
    auto socketDevice = std::make_shared<ObexSocketDevice>(
        socketFd_, cs.max_tx_packet_size, cs.max_rx_packet_size, SocketType::TYPE_L2CAP, deviceAddr_);
    return socketDevice;
}

namespace {
// BipObexResponseHandler：BIP 客户端采用同步架构，OBEX Connect/Disconnect 的状态校验、
// connectionId 保存、状态机切换均由 CreateObexSession/ProcessConnect/DestroyObexSession 同步完成
// （clientSession_->Connect() 内部 ReadResponse 阻塞读完响应后才返回，OnConnectResp 在返回前触发，
// 此时 connectHeader_ 已被 HandleConnectResult 填充好 ConnectionId）。
class BipObexResponseHandler final : public ObexResponseHandler {
public:
    BipObexResponseHandler() = default;
    ~BipObexResponseHandler() = default;

    void OnConnectResp(ObexHeader &reply, uint8_t responseCode) override
    {
        HILOGI("OnConnectResp: responseCode=0x%{public}02X, connectionId=%{public}d",
            responseCode, reply.GetConnectionId());
    }
 
    void OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override
    {
        // BIP 客户端不发 PUT，理论不应回调到这里；保留实现以避免纯虚函数未实现。
        HILOGW("OnPutDateAvailableResp unexpected: responseCode=0x%{public}02X", responseCode);
    }
 
    void OnDisConnectResp(ObexHeader &reply, uint8_t responseCode) override
    {
        HILOGI("OnDisConnectResp: responseCode=0x%{public}02X", responseCode);
    }
 
    void OnAbortResp() override
    {
        HILOGW("OnAbortResp: OBEX request aborted");
    }
};
}  // namespace

bool AvrcpCtBipClient::CreateObexSession()
{
    HILOGI("Creating OBEX session using existing OBEX API");

    if (!socketDevice_) {
        HILOGE("socketDevice_ is null");
        return false;
    }

    transport_ = std::make_shared<ObexServerTransport>(socketDevice_);
    responseHandler_ = std::make_shared<BipObexResponseHandler>();
    clientSession_ = std::make_shared<ObexClientSession>(transport_, responseHandler_);
    clientSession_->SetSrmEnabled(true);
    ObexHeader header;
    header.SetHeader(TARGET, std::vector<uint8_t>(BIP_UUID_AVRCP_COVER_ART,
                                                  BIP_UUID_AVRCP_COVER_ART + sizeof(BIP_UUID_AVRCP_COVER_ART)));

    // 设置 OBEX 接收超时，避免 ObexHeader 默认 recvTimeoutMs_=0 导致 poll 立即返回。
    // 对齐 OppObexClient 使用 RECV_TIMEOUT_DEFAULT_MS(5s)。
    connectHeader_.SetRecvTimeoutMs(RECV_TIMEOUT_DEFAULT_MS);
    clientSession_->Connect(header, connectHeader_);

    uint8_t lastResp = clientSession_->GetLastRespCd();
    if (lastResp != 0xA0) {
        HILOGE("OBEX Connect failed, response code=%{public}d", lastResp);
        clientSession_ = nullptr;
        return false;
    }

    HILOGI("OBEX session created successfully");
    return true;
}

void AvrcpCtBipClient::DestroyObexSession()
{
    HILOGI("Destroying OBEX session");

    if (clientSession_) {
        ObexHeader header;
        header.SetConnectionId(connectHeader_.GetConnectionId());
        clientSession_->DisConnect(header, connectHeader_);
        clientSession_->CloseTransport();
        clientSession_ = nullptr;
    }

    transport_ = nullptr;
    responseHandler_ = nullptr;

    if (socketFd_ >= 0) {
        close(socketFd_);
        socketFd_ = -1;
    }
    socketDevice_ = nullptr;
}

void AvrcpCtBipClient::SetConnectionState(int state)
{
    int oldState = state_;
    state_ = state;
    if (callback_ && oldState != state) {
        callback_->OnConnectionStateChanged(oldState, state);
    }
}

}  // namespace bluetooth
}  // namespace OHOS
