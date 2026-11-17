/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef OBEX_CLIENT_SESSION_H
#define OBEX_CLIENT_SESSION_H

#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include "bt_recursive_mutex.h"
#include "obex_body.h"
#include "obex_server_transport.h"
#include "obex_request_handler.h"
#include "obex_response_handler.h"
#include "obex_header.h"

namespace OHOS {
namespace bluetooth {
class ObexClientSendObject {
public:
    ObexClientSendObject(std::shared_ptr<ObexHeader> bodyHeader,
        std::shared_ptr<ObexBodyObject> bodyReader, uint32_t mtu, uint64_t fileLen);
    virtual ~ObexClientSendObject() = default;
    std::vector<uint8_t> GetSendFileBodyData();
    std::shared_ptr<ObexHeader> GetBodyHeaderReq();
    bool ConvertSendFileToVector(const uint16_t &headerLength, std::vector<uint8_t> &outvector);
    bool IsDone() const;
    void SetBusy(bool isBusy);
    bool IsBusy() const;
    uint64_t fileLen_ = 0;

private:
    bool isDone_ = false;
    bool isBusy_ = false;
    uint16_t mtu_ = 0;
    uint64_t sendIndex_ = 0;
    uint64_t sendPacketLen_ = 0;
    std::shared_ptr<ObexHeader> bodyHeaderReq_ = nullptr;
    std::shared_ptr<ObexBodyObject> bodyReader_ = nullptr;
};
class ObexClientSession {
public:
    ObexClientSession(std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexResponseHandler> handler);
    ~ObexClientSession();

    void Connect(ObexHeader &header, ObexHeader &outHeader);
    void DisConnect(ObexHeader &header, ObexHeader &outHeader);
    void Put(ObexHeader &header, ObexHeader &outHeader, bool isFinal, std::vector<uint8_t> bodyData);
    void Abort(ObexHeader &header, ObexHeader &outHeader);
    void SendRequest(uint8_t code, const std::vector<uint8_t> &package, ObexHeader &outHeader);
    // GET 续传轮次默认上限：防御对端无限 Continue 导致收包死循环。默认值需覆盖 PBAP
    // 大电话簿下载（上游实现无上限）；properties 类小响应可显式传更小的 maxIterations。
    static constexpr uint32_t MAX_GET_CONTINUE_ITERATIONS = 1000;
    // OBEX GET 请求（内部自动协商 SRM）：profile 开启（SetSrmEnabled）且本地 transport 支持
    // SRM（非 RFCOMM）时自动在首个请求携带 SRM=1；对端在首个响应中回 SRM=1 时进入流式收包
    // （不再发送 GET 请求）；对端响应携带 SRPM=1（未就绪）时该轮回退为 GET_FINAL 轮询，避免
    // 双方互等死锁；对端不回 SRM 时整体回退为逐轮 GET_FINAL 轮询。接收超时由 req 传播到 resp
    // （req 已设置且 resp 为 0 时），否则 timeout=0 退化为裸阻塞 recv。响应体分片写入 writer
    // （writer 为空时仅收首个响应）；maxIterations 为续传轮次上限，超出后截断返回。
    // 返回 0 表示最终响应 OK；-1 表示失败或截断（截断时 resp.GetResponseCode()==CONTINUE）。
    // 同一会话同时仅允许一个在途 GET（重入直接失败返回），防止并发请求/响应交错导致流失步。
    int Get(ObexHeader &req, ObexHeader &resp, std::shared_ptr<ObexBodyObject> writer = nullptr,
        uint32_t maxIterations = MAX_GET_CONTINUE_ITERATIONS);
    // OBEX SETPATH 请求。flags 为协议标志位（bit0 backup / bit1 create），path 为单级目录名。
    int SetPath(uint8_t flags, const std::u16string &path, ObexHeader &resp);
    int ReadResponse(uint8_t code, uint8_t &responseCode, uint32_t &length, ObexHeader &outHeader);
    int HandleResponse(uint8_t code, uint8_t &responseCode, ObexHeader &outHeader);
    void SendEvent(ObexHeader &request, const std::string &eventData);
    void SetBusy(bool isBusy);
    bool IsBusy();
    void CloseTransport();
    std::shared_ptr<ObexClientSendObject> CreateSendObject(std::shared_ptr<ObexHeader> bodyHeader,
        std::shared_ptr<ObexBodyObject> bodyReader, uint32_t mtu, uint64_t fileLen);
    std::shared_ptr<ObexClientSendObject> GetSendObject();
    std::string GetRemoteAddress();
    uint32_t GetMaxTxPackageSize();
    uint32_t GetTransportMaxTxPackageSize();
    void FreeSendObject();
    uint8_t GetLastOpeId();
    void SetLastOpeId(uint8_t lastOpeId);
    uint8_t GetLastRespCd();
    void SetLastRespCd(uint8_t lastRespCd);
    bool isSupportSrm_ = false;
    /// SRM（Single Response Mode）GET 总开关。
    /// 由客户端 profile（如 PbapPceService）配置；按操作协商结果在 Get() 内从首个
    /// Continue 响应中检测。
    void SetSrmEnabled(bool enabled) { srmEnabled_ = enabled; }
    bool IsSrmEnabled() const { return srmEnabled_; }
private:
    void HandleConnectResult(uint32_t length, ObexHeader &outHeader);
    uint8_t SendEventRequest(uint8_t code, const std::vector<uint8_t> &eventData);
    /// 读取下一条流式 OBEX 响应（SRM 模式，不发送请求）。
    int ReadStreamingResponse(ObexHeader &resp);
    /// 处理一次 Continue 轮次：按 SRM 状态发送 continue-GET 或直接读取流式响应。
    /// @return false 表示流式读取失败。
    bool ProcessGetContinue(ObexHeader &req, ObexHeader &resp, bool srmActive);
    /// 响应携带 SINGLE_RESPONSE_MODE(0x97)=enabled 时返回 true。
    static bool HasSrmEnabled(const ObexHeader &resp);
    /// 响应携带 SINGLE_RESPONSE_MODE_PARAMETER(0x98)=pending 时返回 true。
    static bool HasSrmp(const ObexHeader &resp);
    /// 将响应中的 BODY/END_OF_BODY 写入 writer；见到 END_OF_BODY 时返回 true。
    static bool WriteRespBody(ObexHeader &resp, const std::shared_ptr<ObexBodyObject> &writer);
    /// 将 u16 路径按 OBEX NAME 头编码（UTF-16BE + NUL 终止符）写入请求头。
    static void SetHeaderNameU16(ObexHeader &header, const std::u16string &name);
private:
    std::shared_ptr<ObexServerTransport> transport_ = nullptr;
    // GET 路径 SRM 总开关：由 profile（如 PbapPceService/BIP）通过 SetSrmEnabled 配置，
    // Get() 内部再按 transport 能力（非 RFCOMM）双重门控；
    // 与 isSupportSrm_（PUT 路径，OPP 在用）相互独立。
    bool srmEnabled_ = false;
    bool isBusy_ = false;
    // GET 操作互斥：同一会话同时仅允许一个在途 GET（Get 内 try-lock，重入失败返回）。
    BtMutex operationMutex_;
    uint32_t maxTxPackageSize_ = 0;
    uint8_t lastOpeId_ = 0;
    uint8_t lastRespCd_ = 0;
    std::shared_ptr<ObexClientSendObject> sendObject_ = nullptr;
    std::weak_ptr<ObexResponseHandler> responseHandler_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_SERVER_SESSION_H
