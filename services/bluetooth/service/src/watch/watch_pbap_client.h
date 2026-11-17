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

#ifndef WATCH_PBAP_CLIENT_H
#define WATCH_PBAP_CLIENT_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <string>
#include <vector>
#include "obex_client_session.h"
#include "obex_header.h"
#include "obex_response_handler.h"
#include "obex_socket_device.h"

namespace OHOS {
namespace bluetooth {

// ---- PBAP 应用参数 TLV tag（与本平台 pbap_pce_app_params.h / pbap_pse_appparam.h 取值对齐）----
constexpr uint8_t WATCH_PBAP_TAG_SEARCH_VALUE = 0x02;     // SEARCH_VALUE：ASCII 搜索串
constexpr uint8_t WATCH_PBAP_TAG_SEARCH_PROPERTY = 0x03;  // SEARCH_ATTRIBUTE：0=name 1=number
constexpr uint8_t WATCH_PBAP_TAG_MAX_LIST_COUNT = 0x04;   // MAXLISTCOUNT（BE16）
constexpr uint8_t WATCH_PBAP_TAG_SUPPORTED_FEATURES = 0x10;  // PBAP supported features（BE32）

// SEARCH_ATTRIBUTE 取值：按号码搜索（对齐 pbap_pse SEARCH_ATTR_NUMBER）
constexpr uint8_t WATCH_PBAP_SEARCH_ATTR_NUMBER = 0x01;
// 按号码查询仅取首条匹配联系人
constexpr uint16_t WATCH_PBAP_SEARCH_MAX_COUNT = 1;
// 单次 vcard-listing 响应 BODY 收集上限。MAXLISTCOUNT=1 仅为请求参数，恶意/
// 异常 PSE 可无视并持续发送 BODY 分片；OBEX 层丢弃 writer->Write 返回值
// （WriteRespBody），无法在会话层收口，须由收集器自限。合法响应为单卡片
// XML（远小于 1KB），64KB 已留足余量
constexpr size_t WATCH_PBAP_MAX_LISTING_BODY_SIZE = 64 * 1024;
// SEARCH_VALUE TLV 长度字段为单字节，搜索号码串长度上限（超长在入口拒绝，防静默截断）
constexpr size_t WATCH_PBAP_MAX_SEARCH_VALUE_LEN = 255;
// PCE 声明支持的能力：下载 + 浏览（与 PbapPceObexClient CONNECT 协商一致）
constexpr uint32_t WATCH_PBAP_FEATURES_DOWNLOAD_BROWSING = 0x03;

// OBEX GET 接收超时（对齐 PBAP_PCE_GET_TIMEOUT_MS）；同时作为 socket 级
// SO_RCVTIMEO，覆盖 UUID 模式连接期间（SDP + RFCOMM 建链）的等待上限
constexpr int WATCH_PBAP_GET_TIMEOUT_MS = 30000;
// socket 安全等级：加密 + 鉴权（对齐 PCE_SOCKET_SEC_ENCRYPT_AUTHENTICATE）
constexpr int WATCH_PBAP_SOCKET_SEC_ENCRYPT_AUTHENTICATE = 3;
// RFCOMM 默认 OBEX MTU（对齐 PbapPceConfig 默认值）
constexpr uint32_t WATCH_PBAP_DEFAULT_MTU = 1024;

// 毫秒/秒换算系数（SO_RCVTIMEO 的 tv_sec/tv_usec 换算使用）
constexpr int WATCH_PBAP_MS_PER_SECOND = 1000;
// 无效 socket fd 哨兵值
constexpr int32_t WATCH_PBAP_INVALID_SOCKET_FD = -1;
// 无效 OBEX ConnectionId 哨兵值
constexpr int32_t WATCH_PBAP_INVALID_CONNECTION_ID = -1;
// OBEX CONNECT 请求中 ConnectionId 头的固定取值（尚未建立连接上下文）
constexpr int32_t WATCH_PBAP_OBEX_CONNECTION_ID_NONE = 0;
// GET 失败且未收到任何有效响应码（传输级失败判据）
constexpr uint8_t WATCH_PBAP_RESPONSE_CODE_NONE = 0;
// OBEX NAME 头 UTF-16BE 编码的 NUL 终止符
constexpr uint16_t WATCH_PBAP_U16_NULL_TERMINATOR = 0x0000;

// PBAP PSE 服务 UUID（OBEX CONNECT TARGET，对齐 PBAP_PCE_SERVICE_UUID）
constexpr uint8_t WATCH_PBAP_PSE_SERVICE_UUID_LEN = 16;
constexpr uint8_t WATCH_PBAP_PSE_SERVICE_UUID[WATCH_PBAP_PSE_SERVICE_UUID_LEN] = {
    0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8, 0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66
};

/// 联系人电话簿路径（PBAP 规范 telecom/pb）
extern const std::string WATCH_PBAP_PATH_PB;

/// WatchPbapClient 连接状态
enum class WatchPbapState {
    IDLE = 0,
    CONNECTING,
    CONNECTED,
};

/**
 * @brief 手表侧独立 PBAP PCE 客户端（不复用 pbap_pce profile 状态机）。
 *
 * 连接采用 SPP socket 服务的 UUID 模式：向 SocketService::Connect 传入
 * PBAP PSE UUID，SDP 查询由 btsock 内部完成（BTA_JvStartDiscovery），
 * 成功后自动建立 RFCOMM 连接并经 fd 下发 channel + sock_connect_info_t，
 * 随后进行 OBEX CONNECT，连接成功后保持。全程不依赖 sdp_adapter。
 * 来电时按号码发起 x-bt/vcard-listing 搜索（SEARCH_VALUE + SEARCH_ATTRIBUTE=number），
 * 命中的联系人信息打印日志，并发送 usual.event.bluetooth.SECOND_PHONE_CALL_NAME
 * 广播（携带 name 与 number 两个参数）。
 *
 * 所有连接/查询/断开任务由内部单一工作线程串行执行（OBEX 会话不支持并发请求），
 * 外部接口均为异步投递，不会阻塞调用线程。
 */
class WatchPbapClient : public ObexResponseHandler,
                        public std::enable_shared_from_this<WatchPbapClient> {
public:
    explicit WatchPbapClient(const std::string &address);
    virtual ~WatchPbapClient();

    /**
     * @brief 异步发起连接：SDP -> socket -> OBEX CONNECT，成功后保持连接。
     */
    void Connect();

    /**
     * @brief 异步断开：发送 OBEX DISCONNECT 并关闭底层传输。
     */
    void Disconnect();

    /**
     * @brief 按号码查询联系人（vcard-listing search by number），结果打印日志并广播。
     * @param number 来电号码
     */
    void QueryContactByNumber(const std::string &number);

    bool IsConnected() const;
    const std::string &GetAddress() const;

    // ObexResponseHandler 回调（ObexClientSession 同步收发时在本客户端工作线程上触发）
    void OnConnectResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnGetDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnDisConnectResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnAbortResp() override;

private:
    struct VcardListingCard {
        std::string handle {};
        std::string name {};
    };

    using PbapTask = std::function<void()>;

    bool PostTask(const PbapTask &task);
    bool StartWorkerLocked();
    void WorkerLoop();
    static void *WorkerLoopTrampoline(void *arg);

    void DoConnectTask();
    void DoDisconnectTask();
    void DoQueryTask(const std::string &number);
    void HandleQueryResult(const std::string &number, const std::vector<VcardListingCard> &cards);

    /**
     * @brief 发送 usual.event.bluetooth.SECOND_PHONE_CALL_NAME 广播，携带
     *        name（联系人姓名）与 number（来电号码）两个参数。
     *        虚函数，供单测派生类替换捕获，不实际发布公共事件。
     */
    virtual void SendSecondPhoneCallNameEvent(const std::string &name, const std::string &number);

    bool DoSocketConnect();
    bool DoObexConnect();

    void CloseTransport();
    void BuildConnectHeader(ObexHeader &header);
    void BuildSearchByNumberRequest(ObexHeader &req, const std::string &number);
    void ExtractBodyFromResponse(ObexHeader &resp, std::vector<uint8_t> &outBody);
    static std::vector<VcardListingCard> ParseVcardListing(const std::vector<uint8_t> &body);
    static void SetNameHeaderU16(ObexHeader &header, const std::string &name);
    static std::u16string Utf8ToUtf16(const std::string &text);
    static size_t GetUtf8SeqLen(uint8_t lead, uint32_t &codePoint);
    static bool ReadUtf8CodePoint(const std::string &text, size_t index, size_t seqLen, uint32_t &codePoint);
    static bool IsValidUtf8CodePoint(size_t seqLen, uint32_t codePoint);
    static void AppendUtf16CodePoint(std::u16string &out, uint32_t codePoint);
    static void AppendTlv(std::vector<uint8_t> &out, uint8_t tag, const uint8_t *val, uint8_t len);
    static void AppendUint16Be(std::vector<uint8_t> &out, uint16_t val);

    std::string address_ {};

    std::atomic<WatchPbapState> state_ {WatchPbapState::IDLE};

    std::shared_ptr<ObexServerTransport> transport_ = nullptr;
    std::shared_ptr<ObexClientSession> clientSession_ = nullptr;
    int32_t connectionId_ = WATCH_PBAP_INVALID_CONNECTION_ID;
    bool isObexConnected_ = false;
    int32_t socketFd_ = WATCH_PBAP_INVALID_SOCKET_FD;

    // 模块以 -fno-exceptions 编译，std::thread 创建失败会抛异常，
    // 改用 pthread_create 以 errno 方式处理建线程失败
    pthread_t workerThread_ {0};
    std::deque<PbapTask> taskQueue_ {};
    std::mutex taskMutex_ {};
    std::condition_variable taskCv_ {};
    bool running_ = false;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // WATCH_PBAP_CLIENT_H
