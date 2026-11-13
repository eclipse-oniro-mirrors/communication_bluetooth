/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
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
#define LOG_TAG "bt_service_watch_pbap"
#endif

#include "watch_pbap_client.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <endian.h>
#include "bt_def.h"
#include "bt_uuid.h"
#include "bt_sock.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_publish_info.h"
#include "common_util.h"
#include "interface_profile.h"
#include "interface_profile_manager.h"
#include "interface_profile_socket.h"
#include "log.h"
#include "obex_body.h"
#include "obex_def.h"
#include "obex_server_transport.h"
#include "watch_define.h"
#include "want.h"

namespace OHOS {
namespace bluetooth {

const std::string WATCH_PBAP_PATH_PB = "telecom/pb";

namespace {
// OBEX GET 响应 BODY 的内存收集器（对齐 PbapPceObexClient::MemoryBodyObject，
// 增加收集上限：本客户端仅发起 MAXLISTCOUNT=1 的单卡片查询，对端违约超发
// 分片时拒收并置溢出标志，防止 data_ 无界累积被对端触发 OOM）
class WatchPbapMemoryBody : public ObexBodyObject {
public:
    WatchPbapMemoryBody() = default;
    ~WatchPbapMemoryBody() override = default;

    void OpenFile(const int fileFd) override {}
    int Close() override
    {
        return 0;
    }
    size_t Read(uint8_t *buf, size_t bufLen) override
    {
        return 0;
    }
    size_t Write(const uint8_t *buf, size_t bufLen) override
    {
        if (buf == nullptr || bufLen == 0) {
            return 0;
        }
        // OBEX 层忽略 Write 返回值（WriteRespBody），靠返回值无法中止续传，
        // 只能停止收集；溢出由调用方检查 IsOverflow 按对端违约处理
        if (overflow_ || data_.size() + bufLen > WATCH_PBAP_MAX_LISTING_BODY_SIZE) {
            overflow_ = true;
            return 0;
        }
        data_.insert(data_.end(), buf, buf + bufLen);
        return bufLen;
    }
    bool IsReadFinished() override
    {
        return true;
    }

    const std::vector<uint8_t> &GetData() const
    {
        return data_;
    }

    bool IsOverflow() const
    {
        return overflow_;
    }

private:
    std::vector<uint8_t> data_;
    bool overflow_ = false;
};

// vCard listing XML 属性值中的 XML 实体与对应字符
// （PSE 侧 WriteVcardXmlData 转义的逆过程）
const std::pair<std::string, char> WATCH_PBAP_XML_ENTITIES[] = {
    {"&lt;", '<'},
    {"&gt;", '>'},
    {"&amp;", '&'},
    {"&quot;", '"'},
    {"&apos;", '\''},
};

// vCard listing XML 中属性值的 XML 实体反转义
std::string DecodeXmlEntities(const std::string &in)
{
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); i++) {
        bool decoded = false;
        for (const auto &entity : WATCH_PBAP_XML_ENTITIES) {
            if (in.compare(i, entity.first.size(), entity.first) == 0) {
                out.push_back(entity.second);
                i += entity.first.size() - 1;
                decoded = true;
                break;
            }
        }
        if (!decoded) {
            out.push_back(in[i]);
        }
    }
    return out;
}

// 从 "<card handle=\"N.vcf\" name=\"xxx\"/>" 片段中提取属性值
std::string GetXmlAttribute(const std::string &fragment, const std::string &key)
{
    std::string pattern = key + "=\"";
    size_t pos = fragment.find(pattern);
    if (pos == std::string::npos) {
        return "";
    }
    pos += pattern.size();
    size_t end = fragment.find('"', pos);
    if (end == std::string::npos || end <= pos) {
        return "";
    }
    return DecodeXmlEntities(fragment.substr(pos, end - pos));
}
}  // namespace

WatchPbapClient::WatchPbapClient(const std::string &address) : address_(address)
{
    HILOGI("WatchPbapClient create addr=%{public}s", GetEncryptAddr(address_).c_str());
}

WatchPbapClient::~WatchPbapClient()
{
    {
        std::lock_guard<std::mutex> lock(taskMutex_);
        running_ = false;
    }
    taskCv_.notify_all();
    if (workerThread_ != 0) {
        if (pthread_equal(workerThread_, pthread_self()) != 0) {
            // 析构发生在工作线程内（最后一个引用随 WorkerLoop 的 self 释放）：
            // pthread_join 自身返回 EDEADLK，无法回收。必须 pthread_detach 本线程，
            // 否则其终止后保持 joinable 且无人 join，按 POSIX 语义栈/TCB 永不回收；
            // 此处不存在并发 join 者（析构即最后引用，无其他线程可触达该 tid）
            pthread_detach(workerThread_);
            workerThread_ = 0;
        } else {
            pthread_join(workerThread_, nullptr);
            workerThread_ = 0;
        }
    }
    CloseTransport();
    HILOGI("WatchPbapClient destroy addr=%{public}s", GetEncryptAddr(address_).c_str());
}

bool WatchPbapClient::IsConnected() const
{
    return state_.load() == WatchPbapState::CONNECTED;
}

const std::string &WatchPbapClient::GetAddress() const
{
    return address_;
}

bool WatchPbapClient::StartWorkerLocked()
{
    if (running_ || workerThread_ != 0) {
        return true;
    }
    running_ = true;
    pthread_t tid = 0;
    int ret = pthread_create(&tid, nullptr, WatchPbapClient::WorkerLoopTrampoline, this);
    if (ret != 0) {
        running_ = false;
        HILOGE("WatchPbapClient start worker failed, addr=%{public}s, errno=%{public}d",
            GetEncryptAddr(address_).c_str(), ret);
        return false;
    }
    workerThread_ = tid;
    return true;
}

void *WatchPbapClient::WorkerLoopTrampoline(void *arg)
{
    static_cast<WatchPbapClient *>(arg)->WorkerLoop();
    return nullptr;
}

bool WatchPbapClient::PostTask(const PbapTask &task)
{
    std::lock_guard<std::mutex> lock(taskMutex_);
    if (!running_) {
        HILOGW("WatchPbapClient::PostTask worker not running, drop task, addr=%{public}s",
            GetEncryptAddr(address_).c_str());
        return false;
    }
    taskQueue_.push_back(task);
    taskCv_.notify_all();
    return true;
}

void WatchPbapClient::WorkerLoop()
{
    // 线程自持引用：任务链执行期间对象始终存活，外部提前释放 shared_ptr 亦安全
    auto self = shared_from_this();
    HILOGI("WatchPbapClient worker start, addr=%{public}s", GetEncryptAddr(address_).c_str());
    while (true) {
        PbapTask task = nullptr;
        {
            std::unique_lock<std::mutex> lock(taskMutex_);
            taskCv_.wait(lock, [this]() { return !taskQueue_.empty() || !running_; });
            if (!running_) {
                // 确立不变量：running_==false 后本线程不再启动任何新任务（含队列
                // 中已投递的残留任务，一并丢弃以及时释放任务自持引用）。
                // Disconnect 的同步兜底（PostTask 失败路径）依赖此不变量：否则
                // 调用线程 DoDisconnectTask 会与工作线程残留任务并发无锁读写
                // clientSession_/transport_/socketFd_，导致 socket fd 双重
                // close 与 ObexClientSession use-after-free
                taskQueue_.clear();
                break;
            }
            if (taskQueue_.empty()) {
                continue;
            }
            task = std::move(taskQueue_.front());
            taskQueue_.pop_front();
        }
        if (task != nullptr) {
            task();
        }
    }
    HILOGI("WatchPbapClient worker exit, addr=%{public}s", GetEncryptAddr(address_).c_str());
}

void WatchPbapClient::Connect()
{
    {
        std::lock_guard<std::mutex> lock(taskMutex_);
        if (!StartWorkerLocked()) {
            HILOGE("WatchPbapClient::Connect start worker failed, addr=%{public}s",
                GetEncryptAddr(address_).c_str());
            return;
        }
    }
    auto self = shared_from_this();
    PostTask([self]() { self->DoConnectTask(); });
}

void WatchPbapClient::Disconnect()
{
    auto self = shared_from_this();
    bool posted = PostTask([self]() {
        self->DoDisconnectTask();
        {
            std::lock_guard<std::mutex> lock(self->taskMutex_);
            self->running_ = false;
        }
        self->taskCv_.notify_all();
    });
    if (!posted) {
        // PostTask 失败仅代表 running_==false。置位者只可能是：从未启动过工作
        // 线程、已完成自身清理的 Disconnect 任务、或析构函数（析构与本次调用
        // 因 shared_from_this 引用互斥不会重叠）。WorkerLoop 保证 running_==
        // false 后不再启动任何残留任务，故此处同步清理不会与工作线程并发
        // 触碰传输层资源
        DoDisconnectTask();
    }
}

void WatchPbapClient::QueryContactByNumber(const std::string &number)
{
    // SEARCH_VALUE TLV 长度字段为单字节：超长号码在入口拒绝并告警，避免静默
    // 截断导致 TLV 与搜索内容不一致；空串同样无搜索意义，一并拒绝
    if (number.empty() || number.size() > WATCH_PBAP_MAX_SEARCH_VALUE_LEN) {
        HILOGW("WatchPbapClient::QueryContactByNumber invalid number, len=%{public}zu, addr=%{public}s",
            number.size(), GetEncryptAddr(address_).c_str());
        return;
    }
    auto self = shared_from_this();
    PostTask([self, number]() { self->DoQueryTask(number); });
}

void WatchPbapClient::DoConnectTask()
{
    if (state_.load() != WatchPbapState::IDLE) {
        HILOGW("WatchPbapClient::DoConnectTask state=%{public}d, skip", static_cast<int>(state_.load()));
        return;
    }
    state_.store(WatchPbapState::CONNECTING);
    HILOGI("WatchPbapClient start connecting, addr=%{public}s", GetEncryptAddr(address_).c_str());

    if (!DoSocketConnect()) {
        state_.store(WatchPbapState::IDLE);
        return;
    }
    if (!DoObexConnect()) {
        CloseTransport();
        state_.store(WatchPbapState::IDLE);
        return;
    }
    state_.store(WatchPbapState::CONNECTED);
    HILOGI("WatchPbapClient pbap connected, addr=%{public}s, connectionId=%{public}d",
        GetEncryptAddr(address_).c_str(), connectionId_);
}

bool WatchPbapClient::DoSocketConnect()
{
    // UUID 模式 socket 连接（对齐 IProfileSocket::Connect 语义："client queries
    // the SDP and finds the channel to be connected through UUID"）：SDP 查询
    // PSE 服务由 btsock 内部完成（btsock_rfc_connect →
    // BTA_JvStartDiscovery），查到 RFCOMM channel 后自动建链，并按先后顺序经
    // fd 下发 4 字节 channel 与 sock_connect_info_t。全程无需 SdpAdapter，
    // 也无需占用进程级 SDP found 回调槽位。RFCOMM 为 PSE 必选承载（PBAP 规范），
    // L2CAP GOEP 为 1.2 可选承载，此处固定走 RFCOMM。
    IProfileSocket *socketService = static_cast<IProfileSocket *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP));
    if (socketService == nullptr) {
        HILOGE("WatchPbapClient SocketService is nullptr");
        return false;
    }

    Uuid pbapUuid = Uuid::ConvertFromString(BLUETOOTH_UUID_PBAP_PSE);
    int fd = socketService->Connect(address_, pbapUuid, WATCH_PBAP_SOCKET_SEC_ENCRYPT_AUTHENTICATE,
        static_cast<int>(SocketType::TYPE_RFCOMM), 0);
    if (fd == WATCH_PBAP_INVALID_SOCKET_FD) {
        HILOGE("WatchPbapClient socket connect failed, addr=%{public}s", GetEncryptAddr(address_).c_str());
        return false;
    }
    socketFd_ = fd;

    // 接收超时兜底：先于任何 recv 设置。UUID 模式下 SDP 查询 + RFCOMM 建链的
    // 完成信号都经由该 fd 下发（SDP 失败时对端关闭 fd 使 recv 返回 0），
    // SO_RCVTIMEO 防止对端半包停滞导致工作线程永久挂起（对齐 PbapPceObexClient）
    struct timeval recvTimeout = {WATCH_PBAP_GET_TIMEOUT_MS / WATCH_PBAP_MS_PER_SECOND,
        (WATCH_PBAP_GET_TIMEOUT_MS % WATCH_PBAP_MS_PER_SECOND) * WATCH_PBAP_MS_PER_SECOND};
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout)) != 0) {
        HILOGW("WatchPbapClient setsockopt SO_RCVTIMEO failed, errno=%{public}d", errno);
    }

    // SDP 查询完成后先收到 4 字节 channel（send_app_scn）
    int32_t channel = 0;
    if (recv(fd, &channel, sizeof(channel), MSG_NOSIGNAL) != static_cast<ssize_t>(sizeof(channel))) {
        HILOGE("WatchPbapClient recv channel failed, errno=%{public}d", errno);
        CloseTransport();
        return false;
    }
    // RFCOMM 建链完成后收到 sock_connect_info_t（send_app_connect_signal）
    sock_connect_info_t connectInfo {};
    if (recv(fd, &connectInfo, sizeof(connectInfo), MSG_NOSIGNAL) !=
        static_cast<ssize_t>(sizeof(connectInfo))) {
        HILOGE("WatchPbapClient recv connect info failed, errno=%{public}d", errno);
        CloseTransport();
        return false;
    }
    HILOGI("WatchPbapClient socket connected, addr=%{public}s, channel=%{public}d, "
           "mtu tx=%{public}d rx=%{public}d",
        GetEncryptAddr(address_).c_str(), channel, connectInfo.max_tx_packet_size,
        connectInfo.max_rx_packet_size);

    auto socketDevice = std::make_shared<ObexSocketDevice>(fd, WATCH_PBAP_DEFAULT_MTU,
        WATCH_PBAP_DEFAULT_MTU, SocketType::TYPE_RFCOMM, address_);
    transport_ = std::make_shared<ObexServerTransport>(socketDevice);
    clientSession_ = std::make_shared<ObexClientSession>(transport_, shared_from_this());
    if (clientSession_ == nullptr) {
        HILOGE("WatchPbapClient create ObexClientSession failed");
        CloseTransport();
        return false;
    }
    // 按号码查询为单次小数据量 GET，无需 SRM 流式协商
    clientSession_->SetSrmEnabled(false);
    return true;
}

bool WatchPbapClient::DoObexConnect()
{
    ObexHeader connectHeader;
    BuildConnectHeader(connectHeader);

    isObexConnected_ = false;
    connectionId_ = WATCH_PBAP_INVALID_CONNECTION_ID;
    ObexHeader resp;
    // Connect 内同步完成 OBEX CONNECT 请求/响应，结果经 OnConnectResp 回填
    clientSession_->Connect(connectHeader, resp);
    if (!isObexConnected_) {
        HILOGE("WatchPbapClient obex connect failed, addr=%{public}s", GetEncryptAddr(address_).c_str());
        return false;
    }
    return true;
}

void WatchPbapClient::DoDisconnectTask()
{
    HILOGI("WatchPbapClient disconnect, addr=%{public}s, state=%{public}d",
        GetEncryptAddr(address_).c_str(), static_cast<int>(state_.load()));
    if (clientSession_ != nullptr && isObexConnected_) {
        // 发送 OBEX DISCONNECT，响应回调（OnDisConnectResp）仅复位状态标志，
        // 传输层清理统一收敛在下方任务边界完成
        ObexHeader header;
        header.SetConnectionId(static_cast<uint32_t>(connectionId_));
        ObexHeader resp;
        clientSession_->DisConnect(header, resp);
    }
    CloseTransport();
    isObexConnected_ = false;
    connectionId_ = WATCH_PBAP_INVALID_CONNECTION_ID;
    state_.store(WatchPbapState::IDLE);
}

void WatchPbapClient::DoQueryTask(const std::string &number)
{
    if (state_.load() != WatchPbapState::CONNECTED || clientSession_ == nullptr) {
        HILOGW("WatchPbapClient query skipped, pbap not ready, state=%{public}d, addr=%{public}s",
            static_cast<int>(state_.load()), GetEncryptAddr(address_).c_str());
        return;
    }

    ObexHeader req;
    BuildSearchByNumberRequest(req, number);
    auto writer = std::make_shared<WatchPbapMemoryBody>();
    ObexHeader resp;
    int ret = clientSession_->Get(req, resp, writer);
    if (writer->IsOverflow()) {
        // 对端无视 MAXLISTCOUNT=1 超限发送 BODY（协议违约）：响应不可信且
        // 会话状态可疑，按传输级失败处理——断开链路并复位，不解析溢出数据
        HILOGE("WatchPbapClient listing body overflow (limit=%{public}zu), teardown, addr=%{public}s",
            WATCH_PBAP_MAX_LISTING_BODY_SIZE, GetEncryptAddr(address_).c_str());
        isObexConnected_ = false;
        connectionId_ = WATCH_PBAP_INVALID_CONNECTION_ID;
        CloseTransport();
        state_.store(WatchPbapState::IDLE);
        return;
    }
    if (ret != RET_NO_ERROR) {
        uint8_t responseCode = resp.GetResponseCode();
        HILOGE("WatchPbapClient query by number failed, ret=%{public}d, responseCode=0x%{public}02x, "
               "addr=%{public}s", ret, responseCode, GetEncryptAddr(address_).c_str());
        if (responseCode != WATCH_PBAP_RESPONSE_CODE_NONE) {
            // 协议层失败（服务端返回非成功码），底层链路仍可用，保持连接
            return;
        }
        // 传输级失败：底层链路已不可用，按断开处理并复位状态
        isObexConnected_ = false;
        connectionId_ = WATCH_PBAP_INVALID_CONNECTION_ID;
        CloseTransport();
        state_.store(WatchPbapState::IDLE);
        return;
    }

    std::vector<uint8_t> body = writer->GetData();
    ExtractBodyFromResponse(resp, body);
    auto cards = ParseVcardListing(body);
    HandleQueryResult(number, cards);
}

void WatchPbapClient::HandleQueryResult(const std::string &number, const std::vector<VcardListingCard> &cards)
{
    if (cards.empty()) {
        HILOGI("WatchPbapClient contact not found by number, addr=%{public}s", GetEncryptAddr(address_).c_str());
        return;
    }
    // handle/name 属隐私信息不入日志；计数用于观测对端违约（MAXLISTCOUNT=1
    // 却返回多条）的情况
    HILOGI("WatchPbapClient contact matched, count=%{public}zu, addr=%{public}s",
        cards.size(), GetEncryptAddr(address_).c_str());
    // 按号码查询 MAXLISTCOUNT=1，正常仅命中一条；防御性取首条对外广播
    SendSecondPhoneCallNameEvent(cards.front().name, number);
}

void WatchPbapClient::SendSecondPhoneCallNameEvent(const std::string &name, const std::string &number)
{
    HILOGI("WatchPbapClient send second phone call name event");
    // 发布方式参照 WatchService::PublishEventWithIntParam（非有序广播，订阅者需 MANAGE_BLUETOOTH 权限）
    AAFwk::Want want;
    want.SetAction(EVENT_BLUETOOTH_SECOND_PHONE_CALL_NAME);
    want.SetParam(PARAM_KEY_CALL_NAME, name);
    want.SetParam(PARAM_KEY_CALL_NUMBER, number);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    publishInfo.SetSubscriberPermissions({MANAGE_BLUETOOTH});
    bool publishResult = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    HILOGI("WatchPbapClient publish second phone call name event, result=%{public}d", publishResult);
}

// 传输层清理唯一入口。线程不变量：仅在任务边界调用——工作线程的串行任务、
// running_==false 后的调用线程同步兜底（WorkerLoop 保证此后不再启动任何
// 任务，包括队列残留任务）、或 join 完成后的析构线程，三者互斥不并发；
// OBEX 回调（OnDisConnectResp 等）内禁止调用。因此本函数触碰的
// clientSession_/transport_/socketFd_/isObexConnected_ 无需加锁保护
void WatchPbapClient::CloseTransport()
{
    if (clientSession_ != nullptr) {
        clientSession_->CloseTransport();
        clientSession_ = nullptr;
    }
    transport_ = nullptr;
    if (socketFd_ != WATCH_PBAP_INVALID_SOCKET_FD) {
        shutdown(socketFd_, SHUT_RDWR);
        close(socketFd_);
        socketFd_ = WATCH_PBAP_INVALID_SOCKET_FD;
    }
    isObexConnected_ = false;
}

void WatchPbapClient::BuildConnectHeader(ObexHeader &header)
{
    header.SetHeader(TARGET, std::vector<uint8_t>(WATCH_PBAP_PSE_SERVICE_UUID,
        WATCH_PBAP_PSE_SERVICE_UUID + WATCH_PBAP_PSE_SERVICE_UUID_LEN));
    // 声明本端能力（下载+浏览）。UUID 模式连接拿不到对端 SDP 的 supported
    // features 属性，按主流 PCE 实现无条件携带；PBAP 规范要求忽略不认识的
    // 应用参数，对旧版本 PSE 无副作用
    std::vector<uint8_t> appParams;
    uint32_t features = htobe32(WATCH_PBAP_FEATURES_DOWNLOAD_BROWSING);
    AppendTlv(appParams, WATCH_PBAP_TAG_SUPPORTED_FEATURES,
        reinterpret_cast<const uint8_t *>(&features), sizeof(features));
    header.SetHeader(APPLICATION_PARAMETER, appParams);
    header.SetConnectionId(WATCH_PBAP_OBEX_CONNECTION_ID_NONE);
}

void WatchPbapClient::BuildSearchByNumberRequest(ObexHeader &req, const std::string &number)
{
    req.SetConnectionId(static_cast<uint32_t>(connectionId_));
    // TYPE：vCard listing（搜索结果为 XML 名片列表）
    const std::string listingType = "x-bt/vcard-listing";
    req.SetHeader(TYPE, std::vector<uint8_t>(listingType.begin(), listingType.end()));
    // NAME：联系人电话簿
    SetNameHeaderU16(req, WATCH_PBAP_PATH_PB);
    // 应用参数：按号码搜索（SEARCH_VALUE + SEARCH_ATTRIBUTE=number），最多返回 1 条
    std::vector<uint8_t> appParams;
    AppendTlv(appParams, WATCH_PBAP_TAG_SEARCH_VALUE,
        reinterpret_cast<const uint8_t *>(number.c_str()), static_cast<uint8_t>(number.size()));
    uint8_t searchAttr = WATCH_PBAP_SEARCH_ATTR_NUMBER;
    AppendTlv(appParams, WATCH_PBAP_TAG_SEARCH_PROPERTY, &searchAttr, sizeof(searchAttr));
    uint16_t maxListCount = htobe16(WATCH_PBAP_SEARCH_MAX_COUNT);
    AppendTlv(appParams, WATCH_PBAP_TAG_MAX_LIST_COUNT,
        reinterpret_cast<const uint8_t *>(&maxListCount), sizeof(maxListCount));
    req.SetHeader(APPLICATION_PARAMETER, appParams);
    req.SetRecvTimeoutMs(WATCH_PBAP_GET_TIMEOUT_MS);
}

void WatchPbapClient::ExtractBodyFromResponse(ObexHeader &resp, std::vector<uint8_t> &outBody)
{
    // 非 SRM 模式下最终响应的 BODY/END_OF_BODY 头需手动提取（Continue 中间
    // 分片已由 Get 写入 writer）
    if (!resp.GetHeader(BODY).empty()) {
        uint16_t bodyLen = 0;
        std::vector<uint8_t> bodyData = resp.GetBody(bodyLen);
        if (bodyLen > 0) {
            outBody.insert(outBody.end(), bodyData.begin(), bodyData.end());
        }
    }
    if (!resp.GetHeader(END_OF_BODY).empty()) {
        uint16_t endBodyLen = 0;
        std::vector<uint8_t> endBodyData = resp.GetEndOfBody(endBodyLen);
        if (endBodyLen > 0) {
            outBody.insert(outBody.end(), endBodyData.begin(), endBodyData.end());
        }
    }
}

std::vector<WatchPbapClient::VcardListingCard> WatchPbapClient::ParseVcardListing(
    const std::vector<uint8_t> &body)
{
    std::vector<VcardListingCard> cards;
    if (body.empty()) {
        return cards;
    }
    std::string xml(body.begin(), body.end());
    size_t pos = 0;
    while (true) {
        size_t cardStart = xml.find("<card", pos);
        if (cardStart == std::string::npos) {
            break;
        }
        size_t cardEnd = xml.find('>', cardStart);
        if (cardEnd == std::string::npos) {
            break;
        }
        std::string fragment = xml.substr(cardStart, cardEnd - cardStart);
        VcardListingCard card;
        card.handle = GetXmlAttribute(fragment, "handle");
        card.name = GetXmlAttribute(fragment, "name");
        if (!card.handle.empty() || !card.name.empty()) {
            cards.push_back(card);
        }
        pos = cardEnd + 1;
    }
    return cards;
}

std::u16string WatchPbapClient::Utf8ToUtf16(const std::string &text)
{
    // 轻量 UTF-8 → UTF-16 解码：逐码点解码（含 BMP 外字符的代理对），
    // 非法首字节/截断/过长编码以 U+FFFD 替换且不抛异常（wstring_convert
    // 对非法输入会抛 range_error，工作线程内不可传播）
    constexpr char16_t REPLACEMENT_CHAR = 0xFFFD;
    std::u16string out;
    out.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        uint32_t codePoint = 0;
        size_t seqLen = GetUtf8SeqLen(static_cast<uint8_t>(text[i]), codePoint);
        if (seqLen == 0) {
            out.push_back(REPLACEMENT_CHAR);
            ++i;
            continue;
        }
        if (i + seqLen > text.size()) {
            // 序列在末尾被截断
            out.push_back(REPLACEMENT_CHAR);
            break;
        }
        if (!ReadUtf8CodePoint(text, i, seqLen, codePoint) || !IsValidUtf8CodePoint(seqLen, codePoint)) {
            out.push_back(REPLACEMENT_CHAR);
            ++i;
            continue;
        }
        AppendUtf16CodePoint(out, codePoint);
        i += seqLen;
    }
    return out;
}

size_t WatchPbapClient::GetUtf8SeqLen(uint8_t lead, uint32_t &codePoint)
{
    if (lead < 0x80) {
        codePoint = lead;
        return 1;
    }
    if ((lead & 0xE0) == 0xC0) {
        codePoint = lead & 0x1F;
        return 2;
    }
    if ((lead & 0xF0) == 0xE0) {
        codePoint = lead & 0x0F;
        return 3;
    }
    if ((lead & 0xF8) == 0xF0) {
        codePoint = lead & 0x07;
        return 4;
    }
    return 0;
}

bool WatchPbapClient::ReadUtf8CodePoint(const std::string &text, size_t index, size_t seqLen, uint32_t &codePoint)
{
    for (size_t k = 1; k < seqLen; ++k) {
        uint8_t cont = static_cast<uint8_t>(text[index + k]);
        if ((cont & 0xC0) != 0x80) {
            return false;
        }
        codePoint = (codePoint << 6) | (cont & 0x3F);
    }
    return true;
}

bool WatchPbapClient::IsValidUtf8CodePoint(size_t seqLen, uint32_t codePoint)
{
    constexpr uint32_t MAX_CODE_POINT = 0x10FFFF;
    constexpr uint32_t SURROGATE_FIRST = 0xD800;
    constexpr uint32_t SURROGATE_LAST = 0xDFFF;
    // 过长编码、代理区码点、超 Unicode 范围均视为非法
    if (seqLen == 2 && codePoint < 0x80) {
        return false;
    }
    if (seqLen == 3 && codePoint < 0x800) {
        return false;
    }
    if (seqLen == 4 && codePoint < 0x10000) {
        return false;
    }
    if (codePoint > MAX_CODE_POINT) {
        return false;
    }
    return codePoint < SURROGATE_FIRST || codePoint > SURROGATE_LAST;
}

void WatchPbapClient::AppendUtf16CodePoint(std::u16string &out, uint32_t codePoint)
{
    if (codePoint <= 0xFFFF) {
        out.push_back(static_cast<char16_t>(codePoint));
        return;
    }
    codePoint -= 0x10000;
    out.push_back(static_cast<char16_t>(0xD800 + (codePoint >> 10)));
    out.push_back(static_cast<char16_t>(0xDC00 + (codePoint & 0x3FF)));
}

void WatchPbapClient::SetNameHeaderU16(ObexHeader &header, const std::string &name)
{
    // OBEX NAME 头：UTF-16BE 编码 + NUL（0x0000）终止符。name 为 UTF-8，
    // 多字节字符（如中文姓名）须先解码为码点再编码，禁止按字节平铺
    std::u16string u16Name = Utf8ToUtf16(name);
    std::vector<uint8_t> value;
    for (char16_t c : u16Name) {
        AppendUint16Be(value, static_cast<uint16_t>(c));
    }
    AppendUint16Be(value, WATCH_PBAP_U16_NULL_TERMINATOR);
    header.SetHeader(NAME, value);
}

void WatchPbapClient::AppendUint16Be(std::vector<uint8_t> &out, uint16_t val)
{
    uint16_t be = htobe16(val);
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&be);
    out.push_back(bytes[0]);
    out.push_back(bytes[1]);
}

void WatchPbapClient::AppendTlv(std::vector<uint8_t> &out, uint8_t tag, const uint8_t *val, uint8_t len)
{
    out.push_back(tag);
    out.push_back(len);
    out.insert(out.end(), val, val + len);
}

void WatchPbapClient::OnConnectResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGI("WatchPbapClient OnConnectResp, responseCode=0x%{public}02x", responseCode);
    if (responseCode != OBEX_HTTP_OK) {
        isObexConnected_ = false;
        return;
    }
    int32_t connectionId = reply.GetConnectionId();
    if (connectionId != WATCH_PBAP_OBEX_CONNECTION_ID_NONE) {
        connectionId_ = connectionId;
    }
    isObexConnected_ = true;
}

void WatchPbapClient::OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGD("WatchPbapClient OnPutDateAvailableResp, responseCode=0x%{public}02x", responseCode);
}

void WatchPbapClient::OnGetDateAvailableResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGD("WatchPbapClient OnGetDateAvailableResp, responseCode=0x%{public}02x", responseCode);
}

void WatchPbapClient::OnDisConnectResp(ObexHeader &reply, uint8_t responseCode)
{
    // 仅复位纯状态标志：传输层资源（clientSession_/transport_/socketFd_）的
    // 清理统一收敛在任务边界（DoDisconnectTask/DoQueryTask/析构），回调内不
    // 触碰，避免清理逻辑分散到多个调用上下文；发起方任务随后必然执行统一清理
    HILOGI("WatchPbapClient OnDisConnectResp, responseCode=0x%{public}02x", responseCode);
    isObexConnected_ = false;
    connectionId_ = WATCH_PBAP_INVALID_CONNECTION_ID;
    state_.store(WatchPbapState::IDLE);
}

void WatchPbapClient::OnAbortResp()
{
    HILOGW("WatchPbapClient OnAbortResp");
    isObexConnected_ = false;
}
}  // namespace bluetooth
}  // namespace OHOS
