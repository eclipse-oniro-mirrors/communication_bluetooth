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
#define LOG_TAG "bt_pbap_pce_obex_client"
#endif

#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <endian.h>
#include "pbap_pce_obex_client.h"
#include "pbap_pce_service_impl.h"
#include "interface_profile_socket.h"
#include "bt_chr_dft_exception.h"
#include "profile_service_manager.h"
#include "raw_address.h"
#include "service_util.h"
#include "bt_uuid.h"
#include "bt_sock.h"
#include "log.h"
#include "thread_util.h"
#include "interface_profile.h"
#include "interface_profile_manager.h"
#include "pbap_pce_header_msg.h"
#include "obex_socket_device.h"
#include "obex_def.h"
#include "pbap_pce_app_params.h"
#include "pbap_pce_utils.h"
#include "pbap_pce_vcard_parser.h"


namespace OHOS {
namespace bluetooth {

const std::string PATH_PB = "telecom/pb";      // 联系人电话本
const std::string PATH_MCH = "telecom/mch";    // 未接来电
const std::string PATH_ICH = "telecom/ich";    // 已接来电
const std::string PATH_OCH = "telecom/och";    // 已拨电话
const std::string VCARD_FILE_SUFFIX = ".vcf";

size_t MemoryBodyObject::Write(const uint8_t *buf, size_t bufLen)
{
    if (buf == nullptr || bufLen == 0) {
        return 0;
    }
    data_.insert(data_.end(), buf, buf + bufLen);
    return bufLen;
}

PbapPceObexClient::PbapPceObexClient(const PbapPceObexConfig &config, PbapPceServiceImpl &pceService)
    : obexConfig_(config), pceService_(pceService)
{
    HILOGI("PbapPceObexClient Create addr=%{public}s",  GetEncryptAddr(obexConfig_.addr).c_str());
}

PbapPceObexClient::~PbapPceObexClient()
{
    CloseTransport();
    HILOGI("PbapPceObexClient destroy");
}

void PbapPceObexClient::CloseTransport()
{
    std::lock_guard<std::mutex> lock(socketFdLock_);
    if (clientSession_ != nullptr) {
        clientSession_->CloseTransport();
        clientSession_ = nullptr;
    }
    socketFd_ = -1;
    isObexConnected_ = false;
}

std::shared_ptr<ObexClientSession> PbapPceObexClient::GetClientSessionLocked()
{
    std::lock_guard<std::mutex> lock(socketFdLock_);
    return clientSession_;
}

int64_t PbapPceObexClient::ElapsedMs(const TimePoint &start, const TimePoint &end)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}


int PbapPceObexClient::CreateSocketConnection()
{
    HILOGI("[PbapPceObexClient] CreateSocketConnection, addr: %{public}s",
           GetEncryptAddr(obexConfig_.addr).c_str());

    IProfileSocket *socketService = static_cast<IProfileSocket *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP));
    CHECK_AND_RETURN_LOG_RET(socketService != nullptr, -1,
                              "SocketService is nullptr");

    Uuid pbapUuid = Uuid::ConvertFromString(BLUETOOTH_UUID_PBAP_PSE);

    int securityFlag = PCE_SOCKET_SEC_ENCRYPT_AUTHENTICATE;
    int socketType = static_cast<int>(SocketType::TYPE_RFCOMM);
    int psm = 0;

    if (obexConfig_.isL2capPSM && obexConfig_.l2capPsm > 0) {
        socketType = static_cast<int>(SocketType::TYPE_L2CAP);
        psm = obexConfig_.l2capPsm;
    } else {
        socketType = static_cast<int>(SocketType::TYPE_RFCOMM);
        psm = obexConfig_.rfCommChannel;
    }

    int socketFd = socketService->Connect(obexConfig_.addr, pbapUuid, securityFlag, socketType, psm);

    if (socketFd == -1) {
        HILOGE("[PbapPceObexClient] SocketService::Connect failed");
        return -1;
    }

    HILOGD("[PbapPceObexClient] SocketService::Connect success, fd: %{public}d", socketFd);
    return socketFd;
}

void PbapPceObexClient::CloseSocket()
{
    HILOGD("[PbapPceObexClient] CloseSocket, fd: %{public}d", socketFd_);
    isDisconnectRequested_ = true;
    std::shared_ptr<ObexClientSession> session;
    {
        std::lock_guard<std::mutex> lock(socketFdLock_);
        if (socketFd_ == -1) {
            HILOGW("[PbapPceObexClient] CloseSocket, socket already invalid");
            return;
        }
        session = clientSession_;
        socketFd_ = -1;
        isObexConnected_ = false;
    }
    if (session != nullptr) {
        // 仅关闭底层 device fd（device 内加锁置 -1），不置空 clientSession_，
        // 避免与同步线程正在执行的 Get() 产生对象释放竞态；
        // 在飞的 Write/Read 会立即快速失败并触发下载循环退出。
        session->CloseTransport();
    }
    utility::Message msg(PCE_OBEX_DISCONNECTED);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);

    HILOGI("[PbapPceObexClient] CloseSocket success");
}

void PbapPceObexClient::ForceClose()
{
    HILOGI("[PbapPceObexClient] ForceClose");
    CloseSocket();
}

std::shared_ptr<ObexSocketDevice> PbapPceObexClient::RecvSocketDevice(SocketType socketType)
{
    HILOGI("[PbapPceObexClient] RecvSocketDevice, socketType: %{public}d", socketType);
    int socketFd = -1;
    {
        std::lock_guard<std::mutex> lock(socketFdLock_);
        socketFd = socketFd_;
    }
    if (socketFd == -1) {
        HILOGE("[PbapPceObexClient] RecvSocketDevice: socket already closed");
        return nullptr;
    }
    int32_t channel = 0;
    auto ret = recv(socketFd, &channel, sizeof(channel), MSG_NOSIGNAL);
    if (ret != static_cast<ssize_t>(sizeof(channel))) {
        HILOGE("[PbapPceObexClient] recv channel error, ret=%{public}d", static_cast<int>(ret));
        return nullptr;
    }
    sock_connect_info_t cs {};
    ret = recv(socketFd, &cs, sizeof(sock_connect_info_t), MSG_NOSIGNAL);
    if (ret != static_cast<ssize_t>(sizeof(sock_connect_info_t))) {
        HILOGE("[PbapPceObexClient] recv connect_info error, ret=%{public}d", static_cast<int>(ret));
        return nullptr;
    }
    HILOGI("[PbapPceObexClient] channel=%{public}d, mtu tx=%{public}d rx=%{public}d",
        channel, cs.max_tx_packet_size, cs.max_rx_packet_size);

    // PCE 自建 socket 上设置接收超时兜底：公共 ObexServerTransport 的 Read 在
    // timeoutMs=0 时退化为裸阻塞 recv()，若对端半包后停滞会永久挂起读取线程。
    // socket 由 PCE 创建并独占使用，在此层设置不影响公共模块及其它 profile。
    struct timeval recvTimeout = {PBAP_PCE_GET_TIMEOUT_MS / 1000,
        (PBAP_PCE_GET_TIMEOUT_MS % 1000) * 1000};
    int sockOptRet = setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout));
    if (sockOptRet != 0) {
        HILOGW("[PbapPceObexClient] setsockopt SO_RCVTIMEO failed, errno=%{public}d", errno);
    }

    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
        socketFd, obexConfig_.mtu, obexConfig_.mtu, socketType, obexConfig_.addr);
    return socketDevice;
}

void PbapPceObexClient::ObexConnectFailed()
{
    isObexConnected_ = false;
    utility::Message msg(PCE_OBEX_CONNECT_FAILED);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
    CloseSocket();
    CloseTransport();
}

void PbapPceObexClient::BuildConnectHeader(ObexHeader& header, bool supported)
{
    HILOGD("[PbapPceObexClient] BuildConnectHeader, supported: %{public}d", supported);

    header.SetHeader(TARGET, std::vector<uint8_t>(PBAP_PCE_SERVICE_UUID,
        PBAP_PCE_SERVICE_UUID + PBAP_PCE_SERVICE_UUID_LEN));

    if (supported) {
        ObexTlvParameters appParams;
        uint32_t features = PBAP_PCE_FEATURES_DOWNLOAD | PBAP_PCE_FEATURES_BROWSING;
        uint32_t beFeatures = htobe32(features);
        TlvTriplet tlv(PbapPceAppParams::PBAP_SUPPORTED_FEATURES, beFeatures);
        appParams.AppendTlvtriplet(tlv);
        std::vector<uint8_t> appParamsBytes = appParams.ToBytes();
        if (!appParamsBytes.empty()) {
            header.SetHeader(APPLICATION_PARAMETER, appParamsBytes);
        }
    }

    header.SetConnectionId(0);

}

int PbapPceObexClient::Connect(std::shared_ptr<PbapPceObexClient> self, bool supported)
{
    HILOGI("[PbapPceObexClient] Connect enter, supported: %{public}d", supported);

    int socketFd = CreateSocketConnection();
    if (socketFd == -1) {
        HILOGE("[PbapPceObexClient] Socket connection failed");
        isObexConnected_ = false;
        utility::Message msg(PCE_OBEX_CONNECT_FAILED);
        pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
        return RET_BAD_STATUS;
    }
    socketFd_ = socketFd;

    SocketType transportType = obexConfig_.isL2capPSM ? SocketType::TYPE_L2CAP : SocketType::TYPE_RFCOMM;
    auto socketDevice = RecvSocketDevice(transportType);
    if (socketDevice == nullptr) {
        HILOGE("[PbapPceObexClient] RecvSocketDevice failed");
        ObexConnectFailed();
        return RET_BAD_STATUS;
    }

    std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(socketDevice);
    if (transport == nullptr) {
        HILOGE("[PbapPceObexClient] Create ObexServerTransport failed");
        ObexConnectFailed();
        return RET_BAD_STATUS;
    }

    clientSession_ = std::make_shared<ObexClientSession>(transport, self);

    if (clientSession_ == nullptr) {
        HILOGE("[PbapPceObexClient] Create ObexClientSession failed");
        ObexConnectFailed();
        return RET_BAD_STATUS;
    }

    // 接入 SRM 总开关（来自 PbapPceService 的 SrmEnable 配置）；按 GET 操作的协商
    // 在 ObexClientSession::Get() 内部完成。
    clientSession_->SetSrmEnabled(obexConfig_.isSupportSrm);

    ObexHeader connectHeader;
    BuildConnectHeader(connectHeader, supported);

    clientSession_->Connect(connectHeader, connectHeader_);

    return RET_NO_ERROR;
}

int PbapPceObexClient::Disconnect(bool withObexReq)
{
    HILOGI("PbapPceObexClient::Disconnect start withObexReq=%{public}d", withObexReq);
    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("Disconnect: clientSession_ is null");
        return RET_BAD_STATUS;
    }

    if (withObexReq && isObexConnected_) {
        ObexHeader header;
        header.SetConnectionId(connectionId_);
        ObexHeader resp;
        session->DisConnect(header, resp);
    } else {
        CloseTransport();
        isObexConnected_ = false;
        utility::Message msg(PCE_OBEX_DISCONNECTED);
        pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
    }
    return RET_NO_ERROR;
}

int PbapPceObexClient::Get(const ObexHeader &req, int reqMsgType, std::shared_ptr<ObexBodyObject> writer)
{
    HILOGI("[PbapPceObexClient] Get enter, reqMsgType: %{public}d", reqMsgType);

    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("[PbapPceObexClient] Get failed, clientSession_ is null");
        return RET_BAD_STATUS;
    }

    currentReqMsgType_.store(reqMsgType, std::memory_order_release);

    if (!isBatchDownloading_.load()) {
        pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), true);
    }

    ObexHeader resp;
    int ret = session->Get(const_cast<ObexHeader &>(req), resp, writer);

    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] Get failed, ret: %{public}d", ret);
        if (!isBatchDownloading_.load()) {
            pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), false);
        }
        return ret;
    }
    return RET_NO_ERROR;
}

int PbapPceObexClient::Abort()
{
    HILOGI("PbapPceObexClient::Abort start");
    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("Abort: clientSession_ is null");
        return RET_BAD_STATUS;
    }

    ObexHeader header;
    header.SetConnectionId(connectionId_);
    ObexHeader resp;
    session->Abort(header, resp);
    SetAbort(true);
    return RET_NO_ERROR;
}

const std::string &PbapPceObexClient::GetRemoteAddress() const
{
    return obexConfig_.addr;
}

void PbapPceObexClient::SetBusy(bool isBusy)
{
    isBusy_ = isBusy;
}

bool PbapPceObexClient::IsBusy() const
{
    return isBusy_ || isBatchDownloading_.load();
}

void PbapPceObexClient::SetAbort(bool isAbort)
{
    isAbort_ = isAbort;
}

void PbapPceObexClient::OnConnectResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGI("[PbapPceObexClient] OnConnectResp enter, responseCode: 0x%02x", responseCode);

    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("[PbapPceObexClient] OnConnectResp: clientSession_ is null");
        NotifyConnectFailed(responseCode);
        return;
    }

    switch (responseCode) {
        case OBEX_HTTP_OK:
            HandleConnectSuccess(reply);
            break;
        case OBEX_HTTP_UNAUTHORIZED:
        case OBEX_HTTP_FORBIDDEN:
        case OBEX_HTTP_UNAVAILABLE:
            HILOGE("[PbapPceObexClient] Service unavailable");
            NotifyConnectFailed(responseCode);
            break;
        default:
            HILOGE("[PbapPceObexClient] Connect failed, responseCode: 0x%02x", responseCode);
            NotifyConnectFailed(responseCode);
            break;
    }
}

void PbapPceObexClient::HandleConnectSuccess(ObexHeader &reply)
{
    HILOGI("[PbapPceObexClient] HandleConnectSuccess");
    int32_t connectionId = reply.GetConnectionId();
    if (connectionId != 0) {
        connectionId_ = connectionId;
        reply.SetConnectionId(connectionId);
        HILOGD("[PbapPceObexClient] Connection ID: %{public}d", connectionId);
    }

    isObexConnected_ = true;
    std::unique_ptr<PbapPceObexMessage> obexMsg =
        std::make_unique<PbapPceObexMessage>(obexConfig_.addr, connectHeader_);
    utility::Message msg(PCE_OBEX_CONNECTED, 0, static_cast<void*>(obexMsg.release()));
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}


void PbapPceObexClient::NotifyConnectFailed(int responseCode)
{
    HILOGI("[PbapPceObexClient] NotifyConnectFailed, responseCode: %{public}d", responseCode);
    isObexConnected_ = false;
    utility::Message msg(PCE_OBEX_CONNECT_FAILED, responseCode, nullptr);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}


void PbapPceObexClient::OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGD("OnPutDateAvailableResp responseCode=%{public}02x", responseCode);
}

void PbapPceObexClient::OnGetDateAvailableResp(ObexHeader &reply, uint8_t responseCode)
{
    if (isBatchDownloading_) {
        // 批量下载期间每批触发两次本回调，静默返回避免日志刷屏
        return;
    }
    HILOGI("OnGetDateAvailableResp responseCode=%{public}02x", responseCode);
    isObexConnected_ = true;
    switch (currentReqMsgType_.load(std::memory_order_acquire)) {
        case PCE_REQ_PULLPHONEBOOK:
            pceService_.ProcessPhonebookMessage(RawAddress(obexConfig_.addr),
                utility::Message(PCE_PULLPHONEBOOK_COMPLETED));
            break;
        case PCE_REQ_PULLPHONEBOOKSIZE:
            pceService_.ProcessPhonebookMessage(RawAddress(obexConfig_.addr),
                utility::Message(PCE_PULLPHONEBOOKSIZE_COMPLETED));
            break;
        default:
            break;
    }
}

void PbapPceObexClient::OnDisConnectResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGI("OnDisConnectResp responseCode=%{public}02x", responseCode);
    isBusy_ = false;
    isObexConnected_ = false;
    CloseTransport();
    utility::Message msg(PCE_OBEX_DISCONNECTED);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}

void PbapPceObexClient::OnAbortResp()
{
    HILOGD("OnAbortResp");
    isBusy_ = false;
}

void PbapPceObexClient::BuildGetHeader(ObexHeader &header, const std::string &path, const PbapPceAppParams &appParams)
{
    HILOGD("[PbapPceObexClient] BuildGetHeader, path: %{public}s", path.c_str());

    header.SetConnectionId(connectionId_);
    // TYPE 头：ASCII 字符串字节直拷（与原公共 SetType 实现一致，不加 NUL 终止符）
    const std::string phonebookType = "x-bt/phonebook";
    header.SetHeader(TYPE, std::vector<uint8_t>(phonebookType.begin(), phonebookType.end()));

    std::u16string u16Path(path.begin(), path.end());
    SetHeaderNameU16(header, u16Path);

    appParams.AddToObexHeader(header);
}

void PbapPceObexClient::BuildPullPhoneBookRequest(const PullPhoneBookReqParams &params, ObexHeader &reqHeader)
{
    std::string fullPath = params.path + VCARD_FILE_SUFFIX;

    PbapPceAppParams appParams;
    appParams.SetMaxListCount(params.count);
    if (params.withOffset) {
        appParams.SetListStartOffset(params.offset);
    }
    appParams.SetFormat(PBAP_PCE_VCARD_FORMAT_30);
    if (params.withPropertySelector) {
        // 仅请求 VERSION + FN + N + TEL 基础属性；不支持的 PSE 自动忽略该 TLV（返回完整 vCard）
        appParams.SetPropertySelector(PBAP_PCE_PROPERTY_SELECTOR_MINIMAL);
    }
    BuildGetHeader(reqHeader, fullPath, appParams);
    reqHeader.SetRecvTimeoutMs(PBAP_PCE_GET_TIMEOUT_MS);
}

int PbapPceObexClient::ExecuteGetWithBusyGuard(ObexHeader &reqHeader, int reqMsgType,
    std::shared_ptr<ObexBodyObject> writer, ObexHeader &resp)
{
    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("[PbapPceObexClient] ExecuteGetWithBusyGuard failed, session null, reqMsgType: %{public}d",
            reqMsgType);
        return RET_BAD_STATUS;
    }

    currentReqMsgType_.store(reqMsgType, std::memory_order_release);
    // 批量下载期间由入口统一持有 BUSY，per-GET 不再翻转，避免电源状态抖动竞态
    if (!isBatchDownloading_.load()) {
        pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), true);
    }

    int ret = session->Get(reqHeader, resp, writer);

    if (!isBatchDownloading_.load()) {
        pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), false);
    }

    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] ExecuteGetWithBusyGuard failed, reqMsgType: %{public}d, ret: %{public}d",
            reqMsgType, ret);
    }
    return ret;
}

uint16_t PbapPceObexClient::PullPhoneBookSize(const std::string &path)
{
    HILOGD("[PbapPceObexClient] PullPhoneBookSize, path: %{public}s", path.c_str());

    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("[PbapPceObexClient] PullPhoneBookSize failed, clientSession_ is null");
        return 0;
    }

    std::string fullPath = path + VCARD_FILE_SUFFIX;

    ObexHeader reqHeader;
    PbapPceAppParams appParams;
    appParams.SetMaxListCount(0);
    BuildGetHeader(reqHeader, fullPath, appParams);
    reqHeader.SetRecvTimeoutMs(PBAP_PCE_GET_TIMEOUT_MS);

    ObexHeader resp;
    int ret = ExecuteGetWithBusyGuard(reqHeader, PCE_REQ_PULLPHONEBOOKSIZE, nullptr, resp);
    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] PullPhoneBookSize failed, ret: %{public}d", ret);
        return 0;
    }

    PbapPceAppParams respParams = PbapPceAppParams::FromObexHeader(resp);
    uint16_t phonebookSize = respParams.HasPhonebookSize() ? respParams.GetPhonebookSize() : 0;

    HILOGI("[PbapPceObexClient] PullPhoneBookSize success, size: %{public}u", phonebookSize);
    return phonebookSize;
}

int PbapPceObexClient::PullCalllog(const std::string &path, uint16_t offset, uint16_t count,
    std::vector<uint8_t> &outBodyData)
{
    HILOGI("[PbapPceObexClient] PullCalllog, path: %{public}s, offset: %{public}u, count: %{public}u",
        path.c_str(), offset, count);

    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("[PbapPceObexClient] PullCalllog failed, clientSession_ is null");
        return RET_BAD_STATUS;
    }

    ObexHeader reqHeader;
    BuildPullPhoneBookRequest({path, offset, count, false, false}, reqHeader);
    auto writer = std::make_shared<MemoryBodyObject>();

    ObexHeader resp;
    int ret = ExecuteGetWithBusyGuard(reqHeader, PCE_REQ_PULLPHONEBOOK, writer, resp);
    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] PullCalllog failed, ret: %{public}d", ret);
        return ret;
    }

    // body 由 ObexClientSession::Get 全权写入 writer（含最终包 END_OF_BODY），
    // 此处不得再从 resp 提取，否则最终包体双写（通话记录 2 倍重复根因）。
    outBodyData = writer->GetData();

    HILOGI("[PbapPceObexClient] PullCalllog success, path=%{public}s, body size: %{public}zu",
        path.c_str(), outBodyData.size());

    return RET_NO_ERROR;
}

std::vector<PbapPceVCardContact> PbapPceObexClient::ParseCallLogsBody(const std::vector<uint8_t> &bodyData)
{
    HILOGI("[PbapPceObexClient] ParseCallLogsBody enter, bodySize=%{public}zu", bodyData.size());

    if (bodyData.empty()) {
        HILOGW("[PbapPceObexClient] ParseCallLogsBody: body data is empty");
        return {};
    }

    std::vector<PbapPceVCardContact> contacts =
        PbapPceVCardParser::Parse(bodyData, PbapPceParseScenario::CALL_LOGS);
    HILOGI("[PbapPceObexClient] vCard parsed (calllogs): contacts count=%{public}zu", contacts.size());

    return contacts;
}

int PbapPceObexClient::InsertCallLogsToDb(std::vector<PbapPceVCardContact> contacts, const std::string &path)
{
    HILOGI("[PbapPceObexClient] InsertCallLogsToDb: count=%{public}zu, path=%{public}s",
        contacts.size(), path.c_str());

    // 解析器 keep-all 重构后，旧 CALL_LOGS keep 策略（无号码且无姓名丢弃）移至
    // 入库侧执行，保持通话记录入库行为不变
    size_t before = contacts.size();
    contacts.erase(std::remove_if(contacts.begin(), contacts.end(),
        [](const PbapPceVCardContact &c) { return c.phones.empty() && c.displayName.empty(); }),
        contacts.end());
    if (before != contacts.size()) {
        HILOGI("[PbapPceObexClient] InsertCallLogsToDb: dropped %{public}zu empty calllog entries",
            before - contacts.size());
    }

    // 部分手机 ICH 响应混入 MISSED 条目（PSE 合规行为）：按路径统一标记会显示为
    // 已接来电且与前序 MCH 拉取重复入库，故 ICH 丢弃 MISSED 条目
    size_t missedDropped = 0;
    contacts = FilterCallLogsForInsert(contacts, path, missedDropped);
    if (missedDropped > 0) {
        HILOGI("[PbapPceObexClient] InsertCallLogsToDb: ICH mixed missed entries dropped: "
            "count=%{public}zu (delivered by MCH pull)", missedDropped);
    }

    if (contacts.empty()) {
        HILOGW("[PbapPceObexClient] InsertCallLogsToDb: contacts list is empty, skip, path=%{public}s",
            path.c_str());
        return RET_NO_ERROR;
    }

    // P2-25: helper 统一在 DownloadContactsAndCallLogs 入口单点创建（先行于所有
    // 批次投递），此处仅判空——消除 DB 线程 check-then-act 懒初始化竞态
    CHECK_AND_RETURN_LOG_RET(dataShareHelper_ != nullptr, RET_BAD_STATUS,
        "[PbapPceObexClient] InsertCallLogsToDb: dataShareHelper_ is null");

    PbapPceCallType callType = DetermineCallType(path);

    auto dbStart = SteadyClock::now();
    int32_t ret = dataShareHelper_->BatchInsertCallLogs(contacts, callType);
    auto dbEnd = SteadyClock::now();
    int64_t dbMs = ElapsedMs(dbStart, dbEnd);

    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] [TIMING] InsertCallLogsToDb failed, path=%{public}s, ret=%{public}d, "
            "dbLatency=%{public}lldms", path.c_str(), ret, static_cast<long long>(dbMs));
    } else {
        HILOGI("[PbapPceObexClient] [TIMING] InsertCallLogsToDb OK, path=%{public}s, count=%{public}zu, "
            "dbLatency=%{public}lldms", path.c_str(), contacts.size(), static_cast<long long>(dbMs));
    }
    return ret;
}

PbapPceCallType PbapPceObexClient::DetermineCallType(const std::string &path)
{
    PbapPceCallType callType = PBAP_PCE_CALL_TYPE_MISSED;
    if (path == PATH_MCH) {
        callType = PBAP_PCE_CALL_TYPE_MISSED;
    } else if (path == PATH_ICH) {
        callType = PBAP_PCE_CALL_TYPE_INCOMING;
    } else if (path == PATH_OCH) {
        callType = PBAP_PCE_CALL_TYPE_OUTGOING;
    } else {
        HILOGW("[PbapPceObexClient] DetermineCallType: unknown path=%{public}s, default to MISSED", path.c_str());
    }
    return callType;
}

int PbapPceObexClient::PullPhoneBookBatch(const std::string &path, uint16_t offset, uint16_t count, bool isFinish,
    std::vector<uint8_t> &outBodyData)
{
    HILOGI("[PbapPceObexClient] PullPhoneBookBatch, path: %{public}s, offset: %{public}u, count: %{public}u, "
        "isFinish: %{public}d",
        path.c_str(), offset, count, static_cast<int>(isFinish));

    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("[PbapPceObexClient] PullPhoneBookBatch failed, clientSession_ is null");
        return RET_BAD_STATUS;
    }

    ObexHeader reqHeader;
    BuildPullPhoneBookRequest({path, offset, count, true, true}, reqHeader);
    auto writer = std::make_shared<MemoryBodyObject>();

    ObexHeader resp;
    int ret = ExecuteGetWithBusyGuard(reqHeader, PCE_REQ_PULLPHONEBOOK, writer, resp);
    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] PullPhoneBookBatch failed, ret: %{public}d", ret);
        return ret;
    }

    // body 由 ObexClientSession::Get 全权写入 writer（含最终包 END_OF_BODY），
    // 此处不得再从 resp 提取，否则最终包体双写。
    outBodyData = writer->GetData();

    HILOGI("[PbapPceObexClient] PullPhoneBookBatch success, path=%{public}s, offset=%{public}u, body size: %{public}zu",
        path.c_str(), offset, outBodyData.size());

    return RET_NO_ERROR;
}

std::vector<PbapPceVCardContact> PbapPceObexClient::ParseContactsBody(const std::vector<uint8_t> &bodyData)
{
    HILOGI("[PbapPceObexClient] ParseContactsBody enter, bodySize=%{public}zu", bodyData.size());

    if (bodyData.empty()) {
        HILOGW("[PbapPceObexClient] ParseContactsBody: body data is empty");
        return {};
    }

    std::vector<PbapPceVCardContact> contacts = PbapPceVCardParser::Parse(bodyData);
    HILOGI("[PbapPceObexClient] vCard parsed (contacts): count=%{public}zu", contacts.size());

    return contacts;
}

int PbapPceObexClient::InsertContactsToDb(std::vector<PbapPceVCardContact> contacts, bool isFinish,
    const std::string &traceId, uint32_t batchId)
{
    HILOGI("[PbapPceObexClient] InsertContactsToDb: count=%{public}zu, isFinish=%{public}d, "
        "traceId=%{public}s, batchId=%{public}u",
        contacts.size(), static_cast<int>(isFinish), traceId.c_str(), batchId);

    if (contacts.empty() && !isFinish) {
        // 空入库集且无 finish：无事可做；空集+finish 时仍需下发 commit-only
        // 请求闭合服务端导入会话（由 BatchInsertContacts 处理）
        HILOGW("[PbapPceObexClient] InsertContactsToDb: empty insert set without finish, skip");
        return RET_NO_ERROR;
    }

    // P2-25: 同 InsertCallLogsToDb——单点创建见 DownloadContactsAndCallLogs
    CHECK_AND_RETURN_LOG_RET(dataShareHelper_ != nullptr, RET_BAD_STATUS,
        "[PbapPceObexClient] InsertContactsToDb: dataShareHelper_ is null");

    auto dbStart = SteadyClock::now();
    int32_t ret = dataShareHelper_->BatchInsertContacts(contacts, isFinish, traceId, batchId);
    auto dbEnd = SteadyClock::now();
    int64_t dbMs = ElapsedMs(dbStart, dbEnd);

    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] [TIMING] InsertContactsToDb failed, batchId=%{public}u, ret=%{public}d, "
            "dbLatency=%{public}lldms", batchId, ret, static_cast<long long>(dbMs));
    } else {
        HILOGI("[PbapPceObexClient] [TIMING] InsertContactsToDb OK, batchId=%{public}u, count=%{public}zu, "
            "dbLatency=%{public}lldms", batchId, contacts.size(), static_cast<long long>(dbMs));
    }
    return ret;
}

bool PbapPceObexClient::CalcFirstContactBatch(uint32_t totalSize, uint32_t &firstOffset)
{
    // PBAP v1.2.3 §3.1.5：pb 目录位置 0 为 0.vcf 本机名片（"shall always exist"），
    // PhonebookSize 计数含名片。本机名片不入车机库，由请求级跳过实现：从位置 1
    // 起拉。totalSize<=1（空簿或仅名片槽位）无真实联系人可拉。
    if (totalSize <= 1) {
        return false;
    }
    firstOffset = 1;
    return true;
}

std::vector<PbapPceVCardContact> PbapPceObexClient::FilterContactsForInsert(
    const std::vector<PbapPceVCardContact> &contacts, size_t &emptyDropped)
{
    std::vector<PbapPceVCardContact> insertSet;
    insertSet.reserve(contacts.size());
    for (size_t i = 0; i < contacts.size(); i++) {
        const PbapPceVCardContact &contact = contacts[i];
        if (contact.phones.empty() && contact.displayName.empty()) {
            // 空卡（无电话且无姓名）不入库，防服务端空 bucket
            emptyDropped++;
            continue;
        }
        insertSet.push_back(contact);
    }
    return insertSet;
}

// 文件内静态工具（与 pbap_pce_vcard_parser.cpp 的文件级 helper 风格一致，
// 无跨编译单元链接冲突）

/// ASCII 串整体转大写（防御部分 PSE 小写参数实现，逐字节转换无 locale 依赖）
static std::string ToUpper(const std::string &s)
{
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    return result;
}

/// 判定 X-IRMC-CALL-DATETIME 的参数串是否含 MISSED token。
/// 参数串形如 ";MISSED"（裸参数，vCard 2.1 常见）、";TYPE=MISSED"（3.0 形式），
/// 按 ';'/ '=' 分词，任一 token 等于 MISSED 即命中（输入须已转大写）。
static bool ContainsMissedParam(const std::string &params)
{
    static const std::string MISSED_PARAM = "MISSED";
    std::string token;
    for (char c : params) {
        if (c != ';' && c != '=') {
            token.push_back(c);
            continue;
        }
        if (token == MISSED_PARAM) {
            return true;
        }
        token.clear();
    }
    return token == MISSED_PARAM;
}

bool PbapPceObexClient::IsMissedCallEntry(const PbapPceVCardContact &contact)
{
    static const std::string CALL_DATETIME_PROP = "X-IRMC-CALL-DATETIME";
    for (const auto &xData : contact.unknownXData) {
        // 解析层 prop 名已转大写、裸参数键保留原样，此处统一转大写做防御匹配
        std::string upperKey = ToUpper(xData.first);
        size_t propPos = upperKey.find(CALL_DATETIME_PROP);
        if (propPos == std::string::npos) {
            continue;
        }
        // 属性名之后为参数串（如 ";MISSED"、";TYPE=MISSED"），分词判定见
        // ContainsMissedParam
        if (ContainsMissedParam(upperKey.substr(propPos + CALL_DATETIME_PROP.size()))) {
            return true;
        }
    }
    return false;
}

std::vector<PbapPceVCardContact> PbapPceObexClient::FilterCallLogsForInsert(
    const std::vector<PbapPceVCardContact> &contacts, const std::string &path, size_t &missedDropped)
{
    if (path != PATH_ICH) {
        // 仅 ICH 过滤：MCH 的 MISSED 条目是其正主数据，OCH 未观察到混入
        return contacts;
    }
    std::vector<PbapPceVCardContact> insertSet;
    insertSet.reserve(contacts.size());
    for (const auto &contact : contacts) {
        if (IsMissedCallEntry(contact)) {
            missedDropped++;
            continue;
        }
        insertSet.push_back(contact);
    }
    return insertSet;
}

void PbapPceObexClient::FlushHeldDbBatch(bool withFinish, const std::string &traceId)
{
    if (!heldDbBatch_.valid) {
        return;
    }
    HeldDbBatch batch = std::move(heldDbBatch_);
    heldDbBatch_ = HeldDbBatch {};
    HILOGI("[PbapPceObexClient] FlushHeldDbBatch: count=%{public}zu, withFinish=%{public}d, "
        "batchId=%{public}u, traceId=%{public}s",
        batch.insertSet.size(), static_cast<int>(withFinish), batch.batchId, traceId.c_str());
    // DB 入库流水线化：投递到低优先级线程，OBEX 下载与入库并行
    auto self = shared_from_this();
    uint32_t dbBatchId = batch.batchId;
    DispatchDbInsert([self, insertSet = std::move(batch.insertSet), withFinish, traceId, dbBatchId]() {
        self->InsertContactsToDb(std::move(insertSet), withFinish, traceId, dbBatchId);
    });
}

uint32_t PbapPceObexClient::AllocDbBatchId(size_t insertCount)
{
    uint32_t base = nextDbBatchId_;
    size_t subBatches = (insertCount + static_cast<size_t>(CONTACTS_SUB_BATCH_SIZE) - 1)
        / static_cast<size_t>(CONTACTS_SUB_BATCH_SIZE);
    // 空集占 1 个号：末批过滤为空时 commit-only 请求仍需 batchId，且后续批次不重叠
    size_t increment = (subBatches == 0) ? 1 : subBatches;
    nextDbBatchId_ += static_cast<uint32_t>(increment);
    return base;
}

void PbapPceObexClient::PrepareDownloadState()
{
    // isBatchDownloading_ 已由 DownloadContactsAndCallLogs 入口 CAS 置位，此处不再
    // 重复赋值（唯一复位点在 FinalizeDownload）
    isDisconnectRequested_ = false;
    inFlightDbTasks_ = 0;
    contactsTotalMs_ = 0;
    calllogsTotalMs_ = 0;
    downloadStartTime_ = SteadyClock::now();
    contactsStartTime_ = downloadStartTime_;
    // B2 扣留状态与 batchId 全局计数复位（新一轮同步从 0 重新编号）
    heldDbBatch_ = HeldDbBatch {};
    nextDbBatchId_ = 0;

    // 全程持有电源 BUSY；Pull*/Get 内部的 per-GET SetPowerStatusBusy 由
    // isBatchDownloading_ 跳过，避免状态翻转；唯一释放点在 FinalizeDownload。
    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), true);
}

void PbapPceObexClient::DownloadContactsAndCallLogs()
{
    HILOGI("[PbapPceObexClient] DownloadContactsAndCallLogs start, addr: %{public}s",
        GetEncryptAddr(obexConfig_.addr).c_str());

    if (isBatchDownloading_.exchange(true)) {
        // CAS 防重入：check-then-act 在并发触发下可能双双通过检查，导致两条下载链
        // 并发访问 heldDbBatch_/nextDbBatchId_（数据竞争）
        HILOGW("[PbapPceObexClient] DownloadContactsAndCallLogs already in progress, skip, addr: %{public}s",
            GetEncryptAddr(obexConfig_.addr).c_str());
        return;
    }

    auto session = GetClientSessionLocked();
    if (session == nullptr) {
        HILOGE("[PbapPceObexClient] DownloadContactsAndCallLogs failed, clientSession_ is null");
        isBatchDownloading_.store(false);
        // 会话已不存在，按断开归因，避免消费端把默认零值误判成 OBEX 报文错误
        NotifyDownloadFailed(PBAP_PCE_SYNC_FAIL_DISCONNECT);
        return;
    }

    PrepareDownloadState();

    uint16_t totalSizeRaw = PullPhoneBookSize(PATH_PB);
    HILOGI("[PbapPceObexClient] Phonebook total size: %{public}u", totalSizeRaw);

    // 对异常话机簿总数的防御性钳制
    uint32_t totalSize = totalSizeRaw;
    if (totalSize > PBAP_PCE_MAX_TOTAL_SIZE) {
        HILOGW("[PbapPceObexClient] Phonebook size %{public}u exceeds cap %{public}u, clamped",
            totalSize, PBAP_PCE_MAX_TOTAL_SIZE);
        totalSize = PBAP_PCE_MAX_TOTAL_SIZE;
    }

    if (dataShareHelper_ == nullptr) {
        dataShareHelper_ = std::make_shared<PbapPceDataShareHelper>();
    }
    std::string traceId = dataShareHelper_->StartContactSync();

    // 请求级名片跳过：位置 0 为 0.vcf 名片槽位，不请求（不解析不入库）；
    // 从位置 1 起拉，请求数量按 totalSize-1（首批 count 由下载循环按窗口计算）
    uint32_t firstOffset = 0;
    if (!CalcFirstContactBatch(totalSize, firstOffset)) {
        // 空话机簿（或仅名片槽位）也必须下发一次 commit-only isFinish：闭合
        // contactsdataability 的 StartContactSync 导入会话，并触发其同步清空
        // 该设备通话记录表——通话记录阶段的入库依赖清库动作先行完成，否则上一轮
        // 残留会与新一轮混存。经 DispatchDbInsert 入队以保证与后续 calllog
        // 批次的 FIFO 顺序。
        uint32_t batchId = AllocDbBatchId(0);
        auto self = shared_from_this();
        DispatchDbInsert([self, traceId, batchId]() {
            self->InsertContactsToDb({}, true, traceId, batchId);
        });
        FinishContactsPhase(false, "phonebook empty or owner-card only");
        DownloadNextCalllog(0, traceId);
        return;
    }

    DownloadNextContactBatch(firstOffset, totalSize, 0, traceId);
}

void PbapPceObexClient::FinishContactsPhase(bool failed, const char *reason)
{
    contactsTotalMs_ = ElapsedMs(contactsStartTime_, SteadyClock::now());
    HILOGI("[PbapPceObexClient] [TIMING] Contacts phase ended (%{public}s), "
        "contactsTotalLatency=%{public}lldms", reason, static_cast<long long>(contactsTotalMs_));
    if (failed) {
        FinalizeDownload(true);
        return;
    }
    calllogsStartTime_ = SteadyClock::now();
}

int PbapPceObexClient::PullAndParseContactBatch(uint32_t offset, uint16_t count, bool preliminaryFinish,
    std::vector<PbapPceVCardContact> &contacts)
{
    auto batchStart = SteadyClock::now();
    std::vector<uint8_t> bodyData;
    int batchRet = PullPhoneBookBatch(PATH_PB, offset, count, preliminaryFinish, bodyData);
    if (batchRet != RET_NO_ERROR) {
        auto batchEnd = SteadyClock::now();
        HILOGE("[PbapPceObexClient] Contacts batch failed at offset=%{public}u, ret=%{public}d, "
            "batchLatency=%{public}lldms, terminate sync",
            offset, batchRet, static_cast<long long>(ElapsedMs(batchStart, batchEnd)));
        return -1;
    }
    // vCard 解析留在下载线程（CPU 计算约 2ms/批，可忽略）
    contacts = ParseContactsBody(bodyData);
    auto batchEnd = SteadyClock::now();
    int64_t batchMs = ElapsedMs(batchStart, batchEnd);
    // GET 成功但 0 卡片视为对端没有更多数据（Size 上报与实际条数可能不一致），
    // 返回 0 由调用方提前结束联系人阶段，不能按失败终止同步
    if (contacts.empty()) {
        HILOGI("[PbapPceObexClient] Contacts batch at offset=%{public}u returned 0 cards, "
            "no more contacts", offset);
        return 0;
    }
    HILOGI("[PbapPceObexClient] [TIMING] Contacts batch done: offset=%{public}u, parsed=%{public}zu, "
        "batchLatency=%{public}lldms", offset, contacts.size(), static_cast<long long>(batchMs));
    return static_cast<int>(contacts.size());
}

void PbapPceObexClient::DownloadNextContactBatch(
    uint32_t offset, uint32_t totalSize, uint32_t batchIdx, const std::string &traceId)
{
    HILOGI("[PbapPceObexClient] DownloadNextContactBatch: offset=%{public}u, totalSize=%{public}u, "
        "batchIdx=%{public}u", offset, totalSize, batchIdx);

    if (isDisconnectRequested_.load()) {
        HILOGI("[PbapPceObexClient] disconnect requested, abort contacts download");
        // 断连路径：扣留批次数据照发但不带 finish；断连属异常终止，failed=true 经
        // FinalizeDownload 收尾（drain 在飞 DB 任务、复位 isBatchDownloading_、释放
        // 电源 BUSY、上报 PCE_DOWNLOAD_FAILED），修复批次间隙断连时收尾永不执行、
        // BUSY 泄漏、标志悬挂的问题
        AbortContactsDownload(traceId, "disconnect");
        return;
    }
    if (offset >= totalSize) {
        // 全部批次拉取完成：派发扣留的末批并携带 isFinish，闭合服务端导入会话
        CompleteContactsPhase(traceId, "completed");
        return;
    }

    uint16_t count = static_cast<uint16_t>(
        std::min(static_cast<uint32_t>(PBAP_PCE_DEFAULT_BATCH_SIZE), totalSize - offset));
    // 请求侧的预判 isFinish（仅供日志）；真正的 DB 提交标志按实际解析条数计算
    bool preliminaryFinish = (offset + count >= totalSize);

    std::vector<PbapPceVCardContact> contacts;
    int actualCount = PullAndParseContactBatch(offset, count, preliminaryFinish, contacts);
    if (actualCount < 0) {
        // 仅传输级失败才终止同步；FinishContactsPhase(true) 会跳过通话记录阶段。
        // 失败路径：扣留批次数据照发但不带 finish
        AbortContactsDownload(traceId, "batch failed");
        return;
    }
    if (actualCount == 0) {
        // 对端没有返回更多数据：提前结束联系人阶段，继续通话记录下载。
        // 扣留的上一批即最后一批，派发并携带 isFinish
        CompleteContactsPhase(traceId, "no more contacts");
        return;
    }

    // C-2: 按实际解析条数推进 offset。keep-all 解析下"解析数 == 传输条数"，
    // offset 与 PSE 位置空间（位置 0 为名片槽位，请求从位置 1 起拉，该槽位
    // 不下发）严格对齐；入库过滤（空卡丢弃）不参与 offset 推进，isFinish 判定
    // 不受过滤影响。
    uint32_t nextOffset = offset + actualCount;
    bool isFinish = (nextOffset >= totalSize);

    // 入库过滤：解析结果 -> 入库集合（空卡丢弃/无名有电话保留）
    size_t emptyDropped = 0;
    std::vector<PbapPceVCardContact> insertSet = FilterContactsForInsert(contacts, emptyDropped);

    // B2 末批扣留：先派发上一批（中间批次不带 finish），再扣留本批；finish 在
    // 下载确认终止（completed/no-more-contacts）时随末批 flush 下发，杜绝
    // "isFinish 永远为 false"的悬挂会话。派发顺序与批次顺序一致（低优先级线程 FIFO）。
    FlushHeldDbBatch(false, traceId);
    HoldDbBatch(insertSet);
    HILOGI("[PbapPceObexClient] batch held: offset=%{public}u, parsed=%{public}d, insert=%{public}zu, "
        "emptyDropped=%{public}zu, nextOffset=%{public}u, computedFinish=%{public}d, "
        "dbBatchId=%{public}u",
        offset, actualCount, heldDbBatch_.insertSet.size(), emptyDropped, nextOffset,
        static_cast<int>(isFinish), heldDbBatch_.batchId);

    // 下一批作为新的 FFRT 任务投递（保证单任务时长远小于 ffrt 60s 阈值）
    auto self = shared_from_this();
    uint32_t nextBatchIdx = batchIdx + 1;
    DoInPbapSyncThread([self, nextOffset, totalSize, nextBatchIdx, traceId]() {
        self->DownloadNextContactBatch(nextOffset, totalSize, nextBatchIdx, traceId);
    });
}

void PbapPceObexClient::AbortContactsDownload(const std::string &traceId, const char *reason)
{
    FlushHeldDbBatch(false, traceId);
    FinishContactsPhase(true, reason);
}

void PbapPceObexClient::CompleteContactsPhase(const std::string &traceId, const char *reason)
{
    FlushHeldDbBatch(true, traceId);
    FinishContactsPhase(false, reason);
    DownloadNextCalllog(0, traceId);
}

void PbapPceObexClient::HoldDbBatch(std::vector<PbapPceVCardContact> &insertSet)
{
    heldDbBatch_.insertSet = std::move(insertSet);
    heldDbBatch_.batchId = AllocDbBatchId(heldDbBatch_.insertSet.size());
    heldDbBatch_.valid = true;
}

int PbapPceObexClient::PullAndParseCalllog(const std::string &path,
    std::vector<PbapPceVCardContact> &contacts)
{
    auto calllogStart = SteadyClock::now();
    std::vector<uint8_t> bodyData;
    int ret = PullCalllog(path, 0, PBAP_PCE_CALLLOG_MAX_COUNT, bodyData);
    if (ret != RET_NO_ERROR) {
        auto calllogEnd = SteadyClock::now();
        HILOGE("[PbapPceObexClient] [TIMING] Calllog download failed, path=%{public}s, ret=%{public}d, "
            "calllogLatency=%{public}lldms, terminate sync",
            path.c_str(), ret, static_cast<long long>(ElapsedMs(calllogStart, calllogEnd)));
        return -1;
    }
    contacts = ParseCallLogsBody(bodyData);
    auto calllogEnd = SteadyClock::now();
    int64_t calllogMs = ElapsedMs(calllogStart, calllogEnd);
    HILOGI("[PbapPceObexClient] [TIMING] Calllog batch done, path=%{public}s, parsed=%{public}zu, "
        "calllogLatency=%{public}lldms",
        path.c_str(), contacts.size(), static_cast<long long>(calllogMs));
    return static_cast<int>(contacts.size());
}

void PbapPceObexClient::DownloadNextCalllog(int calllogIdx, const std::string &traceId)
{
    static const std::string CALLLOG_PATHS[] = {PATH_MCH, PATH_ICH, PATH_OCH};
    static constexpr int CALLLOG_COUNT = 3;
    (void)traceId;  // traceId 仅联系人入库使用；通话记录走简单插入

    HILOGI("[PbapPceObexClient] DownloadNextCalllog: calllogIdx=%{public}d/%{public}d",
        calllogIdx, CALLLOG_COUNT);

    if (calllogIdx >= CALLLOG_COUNT) {
        calllogsTotalMs_ = ElapsedMs(calllogsStartTime_, SteadyClock::now());
        HILOGI("[PbapPceObexClient] [TIMING] Calllogs phase completed, "
            "calllogsTotalLatency=%{public}lldms", static_cast<long long>(calllogsTotalMs_));
        FinalizeDownload();
        return;
    }
    if (isDisconnectRequested_.load()) {
        calllogsTotalMs_ = ElapsedMs(calllogsStartTime_, SteadyClock::now());
        HILOGI("[PbapPceObexClient] [TIMING] Calllogs phase ended (disconnect), "
            "calllogsTotalLatency=%{public}lldms", static_cast<long long>(calllogsTotalMs_));
        // 断连属异常终止：上报 PCE_DOWNLOAD_FAILED（原实现误报 COMPLETE，
        // 应用层无法感知下载不完整）
        FinalizeDownload(true);
        return;
    }

    const std::string &path = CALLLOG_PATHS[calllogIdx];
    std::vector<PbapPceVCardContact> contacts;
    int parsedCount = PullAndParseCalllog(path, contacts);
    if (parsedCount < 0) {
        calllogsTotalMs_ = ElapsedMs(calllogsStartTime_, SteadyClock::now());
        FinalizeDownload(true);
        return;
    }

    if (parsedCount > 0) {
        // DB 入库流水线化：投递到低优先级线程
        auto self = shared_from_this();
        DispatchDbInsert([self, contacts = std::move(contacts), path]() {
            self->InsertCallLogsToDb(std::move(contacts), path);
        });
    }

    int nextIdx = calllogIdx + 1;
    DoInPbapSyncThread([self = shared_from_this(), nextIdx, traceId]() {
        self->DownloadNextCalllog(nextIdx, traceId);
    });
}

void PbapPceObexClient::FinalizeDownload(bool failed)
{
    auto endTime = SteadyClock::now();
    int64_t totalMs = ElapsedMs(downloadStartTime_, endTime);
    bool disconnectedDuringSync = isDisconnectRequested_.load();

    HILOGI("[PbapPceObexClient] FinalizeDownload, addr: %{public}s, failed=%{public}d, "
        "inFlightDbTasks=%{public}d",
        GetEncryptAddr(obexConfig_.addr).c_str(), static_cast<int>(failed), inFlightDbTasks_.load());

    // 等待全部在飞 DB 入库任务 drain，保证完成（或失败）通知不会被观察在最后
    // 一批数据提交之前。
    std::unique_lock<std::mutex> lock(dbDrainMutex_);
    bool drained = dbDrainCv_.wait_for(lock, std::chrono::milliseconds(PBAP_PCE_DB_DRAIN_TIMEOUT_MS),
        [this]() { return inFlightDbTasks_.load() <= 0; });
    if (!drained) {
        HILOGW("[PbapPceObexClient] in-flight DB tasks (%{public}d) not drained within %{public}dms, "
            "continue finalize", inFlightDbTasks_.load(), PBAP_PCE_DB_DRAIN_TIMEOUT_MS);
    }

    int syncFailReason = PBAP_PCE_SYNC_FAIL_OBEX_ERR;
    if (failed) {
        if (disconnectedDuringSync) {
            syncFailReason = PBAP_PCE_SYNC_FAIL_DISCONNECT;
        } else if (!drained) {
            syncFailReason = PBAP_PCE_SYNC_FAIL_DB_TIMEOUT;
        }
    }
    isBatchDownloading_ = false;
    isDisconnectRequested_ = false;
    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), false);

    HILOGI("[PbapPceObexClient] [TIMING] Download all completed, addr: %{public}s, failed=%{public}d, "
        "totalLatency=%{public}lldms, contactsTotalLatency=%{public}lldms, calllogsTotalLatency=%{public}lldms",
        GetEncryptAddr(obexConfig_.addr).c_str(), static_cast<int>(failed),
        static_cast<long long>(totalMs),
        static_cast<long long>(contactsTotalMs_),
        static_cast<long long>(calllogsTotalMs_));

    if (failed) {
        NotifyDownloadFailed(syncFailReason);
    } else {
        NotifyDownloadComplete();
    }
}

void PbapPceObexClient::DispatchDbInsert(std::function<void()> insertFunc)
{
    // 背压：在飞入库任务达到上限时，阻塞调用（下载）线程等待队列空位后再入队，
    // 限制内存占用。禁止改为在下载线程同步执行——那会使本任务越过低优先级串行
    // 队列中已排队的任务，破坏 "联系人批次(含isFinish=true末批) -> mch -> ich -> och"
    // 的入库顺序契约：contactsdataability 收到 isFinish=true 会在同步处理路径内
    // 清空该设备通话记录表，抢先入库的通话记录批次会被误删（och 丢失问题）。
    {
        std::unique_lock<std::mutex> lock(dbDrainMutex_);
        bool slotAvailable = dbDrainCv_.wait_for(lock,
            std::chrono::milliseconds(PBAP_PCE_DB_DRAIN_TIMEOUT_MS),
            [this]() { return inFlightDbTasks_.load() < PBAP_PCE_MAX_INFLIGHT_DB_TASKS; });
        if (!slotAvailable) {
            // 超时兜底仍入队（绝不回退为同步执行）：病态场景只放宽内存上界，
            // 顺序契约不被破坏
            HILOGW("[PbapPceObexClient] DB insert backpressure: %{public}d in flight not drained "
                "within %{public}dms, enqueue anyway to preserve ordering",
                inFlightDbTasks_.load(), PBAP_PCE_DB_DRAIN_TIMEOUT_MS);
        }
    }
    inFlightDbTasks_.fetch_add(1);
    auto self = shared_from_this();
    DoInLowPriorityThread([self, insertFunc = std::move(insertFunc)]() {
        insertFunc();
        // 在 dbDrainMutex_ 内递减，使 FinalizeDownload 的谓词检查与本次 store+notify
        // 串行化；否则 notify 可能错过刚评估完谓词的等待者（10s 虚假停滞）。
        {
            std::lock_guard<std::mutex> lock(self->dbDrainMutex_);
            self->inFlightDbTasks_.fetch_sub(1);
        }
        self->dbDrainCv_.notify_all();
    });
}

void PbapPceObexClient::NotifyDownloadComplete()
{
    utility::Message msg(PCE_DOWNLOAD_COMPLETE);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}

void PbapPceObexClient::NotifyDownloadFailed(int failReason)
{
    utility::Message msg(PCE_DOWNLOAD_FAILED, failReason, nullptr);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}

}  // namespace bluetooth
}  // namespace OHOS
