/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_service_obex_client_session"
#endif

#include "obex_client_session.h"
#include <sstream>
#include "obex_def.h"
#include "log.h"
#include "common_util.h"

namespace OHOS {
namespace bluetooth {
const uint32_t MAX_CLIENT_PACKET_SIZE = 0xFC00;
const ssize_t INVAILD_SOCKET = -1;
ObexClientSession::ObexClientSession(std::shared_ptr<ObexServerTransport> transport,
    std::shared_ptr<ObexResponseHandler> handler)
    : transport_(transport), responseHandler_(handler)
{
    HILOGI("ObexClientSession Create");
}

ObexClientSession::~ObexClientSession()
{
    HILOGI("ObexClientSession destroy");
}

void ObexClientSession::Connect(ObexHeader &header, ObexHeader &outHeader)
{
    int32_t totalLength = 4;
    std::vector<uint8_t> headerArray;
    header.GenerateHeaderValue(headerArray);
    totalLength += headerArray.size();

    std::vector<uint8_t> outData;
    outData.push_back(0x10);
    outData.push_back(0x00);
    outData.push_back(transport_->GetMaxRxPacketSize() >> BIT_LEN_8);
    outData.push_back(transport_->GetMaxRxPacketSize() & 0xFF);

    outData.insert(outData.end(), headerArray.begin(), headerArray.end());

    return SendRequest(OBEX_OPCODE_CONNECT, outData, outHeader);
}

void ObexClientSession::CloseTransport()
{
    if (transport_ == nullptr) {
        HILOGE("transport_ is nullptr.");
        return;
    }
    transport_->Close();
}

void ObexClientSession::DisConnect(ObexHeader &header, ObexHeader &outHeader)
{
    std::vector<uint8_t> headerArray;
    header.GenerateHeaderValue(headerArray);
    return SendRequest(OBEX_OPCODE_DISCONNECT, headerArray, outHeader);
}

uint8_t ObexClientSession::GetLastOpeId()
{
    return lastOpeId_;
}
void ObexClientSession::SetLastOpeId(uint8_t lastOpeId)
{
    lastOpeId_ = lastOpeId;
}
uint8_t ObexClientSession::GetLastRespCd()
{
    return lastRespCd_;
}
void ObexClientSession::SetLastRespCd(uint8_t lastRespCd)
{
    lastRespCd_ = lastRespCd;
}

void ObexClientSession::Abort(ObexHeader &header, ObexHeader &outHeader)
{
    std::vector<uint8_t> headerArray;
    header.GenerateHeaderValue(headerArray);

    return SendRequest(OBEX_OPCODE_ABORT, headerArray, outHeader);
}

void ObexClientSession::SendRequest(uint8_t code, const std::vector<uint8_t> &package, ObexHeader &outHeader)
{
    std::vector<uint8_t> sendData;
    sendData.push_back(code);
    sendData.push_back((package.size() + HEADER_PREFIX_LENGTH) >> BIT_LEN_8);
    sendData.push_back(package.size() + HEADER_PREFIX_LENGTH);
    sendData.insert(sendData.end(), package.begin(), package.end());
    std::shared_ptr<ObexResponseHandler> responseHandlerSptr = responseHandler_.lock();
    if (transport_->Write(sendData.data(), sendData.size()) == INVAILD_SOCKET && responseHandlerSptr != nullptr) {
        responseHandlerSptr->OnAbortResp();
        return;
    }
    SetLastOpeId(code);
    HILOGI("write code %{public}0x, isSupportSrm is %{public}d package size is %{public}d sendData size is %{public}d",
        code, isSupportSrm_, package.size(), sendData.size());
    if (code == OBEX_OPCODE_PUT && isSupportSrm_) {
        return;
    }
    uint8_t responseCode = 0;
    uint32_t length = 0;
    isSupportSrm_ = transport_->IsSrmSupported();
    int readResponseResult = ReadResponse(code, responseCode, length, outHeader);
    if (readResponseResult < 0 && responseHandlerSptr != nullptr) {
        HILOGE("ReadResponse error");
        responseHandlerSptr->OnAbortResp();
        return;
    }
    int handleResponseResult = HandleResponse(code, responseCode, outHeader);
    if (handleResponseResult < 0 && responseHandlerSptr != nullptr) {
        HILOGE("handleResponseResult error");
        responseHandlerSptr->OnAbortResp();
        return;
    }
}

int ObexClientSession::ReadResponse(uint8_t code, uint8_t &responseCode, uint32_t &length, ObexHeader &outHeader)
{
    int readResult = transport_->Read(&responseCode, sizeof(responseCode), outHeader.GetRecvTimeoutMs());
    SetLastRespCd(responseCode);
    uint8_t data = 0;
    readResult = transport_->Read(&data, sizeof(data), outHeader.GetRecvTimeoutMs());
    length = (data << BIT_LEN_8);
    readResult = transport_->Read(&data, sizeof(data), outHeader.GetRecvTimeoutMs());
    length = (length | data);
    HILOGI("read responseCode = %{public}0x length is %{public}d", responseCode, length);
    if (readResult == -1 || length > transport_->GetMaxRxPacketSize()) {
        HILOGE("readResult = %{public}d maxRxPacketSize is %{public}d", readResult, transport_->GetMaxRxPacketSize());
        return -1;
    }
    if (length < HEADER_PREFIX_LENGTH) {
        HILOGI("length = %{public}d", length);
        return -1;
    }
    if (code == OBEX_OPCODE_CONNECT) {
        HandleConnectResult(length, outHeader);
    } else if (length > HEADER_PREFIX_LENGTH) {
        HILOGI("read receivedData size %{public}d", length - HEADER_PREFIX_LENGTH);
        std::vector<uint8_t> receivedData(length - HEADER_PREFIX_LENGTH);
        // 将请求的接收超时传递给 body 读取。否则默认 timeoutMs=0 跳过 poll() 退化为
        // 裸阻塞 recv()，对端在包中间停滞时任务会被无限挂起。
        int32_t ret = transport_->Read(receivedData.data(), receivedData.size(), outHeader.GetRecvTimeoutMs());
        CHECK_AND_RETURN_LOG_RET(ret >= 0, -1, "ret is %{public}d", ret);
        while (ret != static_cast<int32_t>(length - HEADER_PREFIX_LENGTH) &&
            static_cast<int32_t>(receivedData.size()) >= ret) {
            int32_t currentRet = transport_->Read(receivedData.data() + ret, receivedData.size() - ret,
                outHeader.GetRecvTimeoutMs());
            if (currentRet <= 0) {
                HILOGE("currentRet is %{public}d", currentRet);
                break;
            }
            ret = ret + currentRet;
        }
        outHeader.ParseHeaderValue(receivedData);
        HILOGI("responseCode code = %{public}0x code = %{public}0x", responseCode, code);
    }
    return 0;
}

int ObexClientSession::HandleResponse(uint8_t code, uint8_t &responseCode, ObexHeader &outHeader)
{
    std::shared_ptr<ObexResponseHandler> responseHandlerSptr = responseHandler_.lock();
    if (responseHandlerSptr == nullptr) {
        HILOGE("responseHandler is nullptr");
        return -1;
    }
    if (code == OBEX_OPCODE_CONNECT) {
        responseHandlerSptr->OnConnectResp(outHeader, responseCode);
    } else if (code == OBEX_OPCODE_ABORT) {
        HILOGI("abort ignore reponse.");
        return -1;
    } else if (code == OBEX_OPCODE_PUT || code == OBEX_OPCODE_PUT_FINAL) {
        responseHandlerSptr->OnPutDateAvailableResp(outHeader, responseCode);
        outHeader.SetResponseCode(responseCode);
    } else if (code == OBEX_OPCODE_GET || code == OBEX_OPCODE_GET_FINAL) {
        responseHandlerSptr->OnGetDateAvailableResp(outHeader, responseCode);
        outHeader.SetResponseCode(responseCode);
    } else if (code == OBEX_OPCODE_DISCONNECT) {
        responseHandlerSptr->OnDisConnectResp(outHeader, responseCode);
    }
    return 0;
}

uint8_t ObexClientSession::SendEventRequest(uint8_t code, const std::vector<uint8_t> &eventData)
{
    std::vector<uint8_t> sendData{};
    sendData.push_back(code);
    sendData.push_back((eventData.size() + HEADER_PREFIX_LENGTH) >> BIT_LEN_8);
    sendData.push_back(eventData.size() + HEADER_PREFIX_LENGTH);
    sendData.insert(sendData.end(), eventData.begin(), eventData.end());
    transport_->Write(sendData.data(), sendData.size());

    uint8_t responseCode = 0;
    ssize_t readResult = transport_->Read(&responseCode, sizeof(responseCode), RECV_TIMEOUT_DEFAULT_MS);
    uint8_t data = 0;
    readResult = transport_->Read(&data, sizeof(data), RECV_TIMEOUT_DEFAULT_MS);
    uint32_t length = (data << BIT_LEN_8);
    readResult = transport_->Read(&data, sizeof(data), RECV_TIMEOUT_DEFAULT_MS);
    CHECK_AND_RETURN_LOG_RET(readResult >= 0, responseCode, "readResult is %{public}d", readResult);
    length = (length | data);
    if (length > transport_->GetMaxRxPacketSize()) {
        return responseCode;
    }
    if (length <= HEADER_PREFIX_LENGTH) {
        return responseCode;
    }

    std::vector<uint8_t> receivedData(length - HEADER_PREFIX_LENGTH);
    int32_t ret = transport_->Read(receivedData.data(), receivedData.size());
    CHECK_AND_RETURN_LOG_RET(ret >= 0, responseCode, "ret is %{public}d", ret);
    while (ret != static_cast<int32_t>(length - HEADER_PREFIX_LENGTH) &&
        static_cast<int32_t>(receivedData.size()) >= ret) {
        int32_t currentRet = transport_->Read(receivedData.data(), receivedData.size() - ret);
        if (currentRet <= 0) {
            HILOGE("currentRet is %{public}d", currentRet);
            break;
        }
        ret = ret + currentRet;
    }

    ObexHeader outHeader;
    outHeader.ParseHeaderValue(receivedData);

    return responseCode;
}

void ObexClientSession::SendEvent(ObexHeader &request, const std::string &eventData)
{
    std::vector<uint8_t> headerRaw{};
    request.GenerateHeaderValue(headerRaw);

    std::vector<uint8_t> sendPacket{};
    sendPacket.push_back(BODY);
    sendPacket.push_back((eventData.length() + HEADER_PREFIX_LENGTH) >> BIT_LEN_8);
    sendPacket.push_back(eventData.length() + HEADER_PREFIX_LENGTH);
    sendPacket.insert(sendPacket.end(), eventData.begin(), eventData.end());
    sendPacket.insert(sendPacket.begin(), headerRaw.begin(), headerRaw.end());

    uint8_t responseCode = SendEventRequest(OBEX_OPCODE_PUT, sendPacket);
    while (responseCode == ResponseCodes::OBEX_HTTP_CONTINUE) {
        int32_t connectionId = request.GetConnectionId();
        request.Clear();
        request.SetConnectionId(connectionId);
        std::vector<uint8_t> headerRaw{};
        request.GenerateHeaderValue(headerRaw);

        std::vector<uint8_t> sendPacket{};
        sendPacket.push_back(END_OF_BODY);
        sendPacket.push_back(HEADER_PREFIX_LENGTH >> BIT_LEN_8);
        sendPacket.push_back(HEADER_PREFIX_LENGTH);
        sendPacket.insert(sendPacket.begin(), headerRaw.begin(), headerRaw.end());

        responseCode = SendEventRequest(OBEX_OPCODE_PUT_FINAL, sendPacket);
    }
}

void ObexClientSession::Put(ObexHeader &request, ObexHeader &outHeader, bool isFinal, std::vector<uint8_t> bodyData)
{
    uint8_t code = isFinal ? OBEX_OPCODE_PUT_FINAL : OBEX_OPCODE_PUT;
    std::vector<uint8_t> headerRaw{};
    request.GenerateHeaderValue(headerRaw);
    std::vector<uint8_t> sendData{};
    sendData.insert(sendData.end(), headerRaw.begin(), headerRaw.end());

    std::vector<uint8_t> body{};
    body.push_back(isFinal ? END_OF_BODY : BODY);
    if (!bodyData.empty()) {
        body.push_back((bodyData.size() + HEADER_PREFIX_LENGTH) >> BIT_LEN_8);
        body.push_back(bodyData.size() + HEADER_PREFIX_LENGTH);
        body.insert(body.end(), bodyData.begin(), bodyData.end());
        sendData.insert(sendData.end(), body.begin(), body.end());
    } else {
        body.push_back(HEADER_PREFIX_LENGTH >> BIT_LEN_8);
        body.push_back(HEADER_PREFIX_LENGTH);
        sendData.insert(sendData.end(), body.begin(), body.end());
    }
    SendRequest(code, sendData, outHeader);
}

// ObexClientSendObject
ObexClientSendObject::ObexClientSendObject(
    std::shared_ptr<ObexHeader> bodyHeader, std::shared_ptr<ObexBodyObject> bodyReader, uint32_t mtu, uint64_t fileLen)
    : isDone_(false),
      bodyHeaderReq_(bodyHeader),
      bodyReader_(bodyReader)
{
    mtu_ = mtu;
    fileLen_ = fileLen;
    HILOGE("fileLen is %{public}d", fileLen_);
}

std::shared_ptr<ObexHeader> ObexClientSendObject::GetBodyHeaderReq()
{
    return bodyHeaderReq_;
}

std::vector<uint8_t> ObexClientSendObject::GetSendFileBodyData()
{
    std::vector<uint8_t> bodyData {};
    if (isDone_) {
        return bodyData;
    }
    if (bodyHeaderReq_ == nullptr) {
        HILOGE("ObexClientSendObjec need send header first");
        return bodyData;
    }
    bool convertResult = ConvertSendFileToVector(HEADER_PREFIX_LENGTH, bodyData);
    if (!convertResult) {
        HILOGE("ConvertSendFileToVector error");
        return bodyData;
    }
    if (bodyData.empty()) {
        return bodyData;
    }
    return bodyData;
}

bool ObexClientSendObject::ConvertSendFileToVector(const uint16_t &headerLength,
    std::vector<uint8_t> &outVector)
{
    if (sendPacketLen_ == 0) {
        sendPacketLen_ = mtu_ - headerLength;
    }
    if (fileLen_ == 0) {
        isDone_ = true;
        return true;
    }
    HILOGI("mtu_ is [%{public}d] headerLength is [%{public}d] sendPacketLen_  is [%{public}d]",
        mtu_, headerLength, sendPacketLen_);
    auto buf = std::make_unique<uint8_t[]>(sendPacketLen_);
    int cnt = bodyReader_->Read(buf.get(), sendPacketLen_);
    HILOGI("read cnt [%{public}d]", cnt);

    if (bodyReader_->IsReadFinished()) {
        isDone_ = true;
    }
    if (cnt <= 0) {
        return false;
    }
    if (cnt < static_cast<int64_t>(sendPacketLen_)) {
        isDone_ = true;
    }

    sendIndex_++;
    std::vector<uint8_t> bufVector(&buf.get()[0], &buf[cnt]);
    outVector = std::move(bufVector);
    return true;
}

bool ObexClientSendObject::IsDone() const
{
    return isDone_;
}

void ObexClientSendObject::SetBusy(bool isBusy)
{
    isBusy_ = isBusy;
}

bool ObexClientSendObject::IsBusy() const
{
    return isBusy_;
}

std::shared_ptr<ObexClientSendObject> ObexClientSession::CreateSendObject(
    std::shared_ptr<ObexHeader> bodyHeader, std::shared_ptr<ObexBodyObject> bodyReader, uint32_t mtu, uint64_t fileLen)
{
    sendObject_ = std::make_shared<ObexClientSendObject>(bodyHeader, bodyReader, mtu, fileLen);
    return sendObject_;
}

std::shared_ptr<ObexClientSendObject> ObexClientSession::GetSendObject()
{
    return sendObject_;
}

void ObexClientSession::HandleConnectResult(uint32_t length, ObexHeader &outHeader)
{
    uint8_t data = 0;
    uint8_t version = 0;
    uint8_t flags = 0;
    transport_->Read(&version, sizeof(version));
    transport_->Read(&flags, sizeof(flags));
    transport_->Read(&data, sizeof(data));
    maxTxPackageSize_ = data;
    transport_->Read(&data, sizeof(data));
    maxTxPackageSize_ += (maxTxPackageSize_ << BIT_LEN_8) + data;
    if (maxTxPackageSize_ > MAX_CLIENT_PACKET_SIZE) {
        maxTxPackageSize_ = MAX_CLIENT_PACKET_SIZE;
    }
    if (maxTxPackageSize_ > transport_->GetMaxRxPacketSize()) {
        maxTxPackageSize_ = transport_->GetMaxRxPacketSize();
    }
    HILOGI("maxTxPackageSize_ is %{public}d", maxTxPackageSize_);
    std::vector<uint8_t> receivedData;
    if (length > HEAD_CONNECT_LEN) {
        receivedData = std::move(std::vector<uint8_t>(length - HEAD_CONNECT_LEN));
        int32_t ret = transport_->Read(receivedData.data(), receivedData.size());
        CHECK_AND_RETURN_LOG(ret >= 0, "ret is %{public}d", ret);
        while (ret != static_cast<int32_t>(length - HEAD_CONNECT_LEN) &&
            static_cast<int32_t>(receivedData.size()) >= ret) {
            int32_t currentRet = transport_->Read(receivedData.data() + ret, receivedData.size() - ret);
            if (currentRet <= 0) {
                HILOGE("currentRet is %{public}d", currentRet);
                break;
            }
            ret = ret + currentRet;
        }
        outHeader.ParseHeaderValue(receivedData);
    }
}

std::string ObexClientSession::GetRemoteAddress()
{
    return transport_->GetRemoteAddress();
}

uint32_t ObexClientSession::GetMaxTxPackageSize()
{
    return maxTxPackageSize_;
}

uint32_t ObexClientSession::GetTransportMaxTxPackageSize()
{
    return transport_->GetMaxTxPacketSize();
}

bool ObexClientSession::IsBusy()
{
    return isBusy_;
}
void ObexClientSession::SetBusy(bool isBusy)
{
    isBusy_ = isBusy;
}
void ObexClientSession::FreeSendObject()
{
    sendObject_ = nullptr;
}

bool ObexClientSession::WriteRespBody(ObexHeader &resp, const std::shared_ptr<ObexBodyObject> &writer)
{
    uint16_t bodyLen = 0;
    std::vector<uint8_t> bodyData = resp.GetBody(bodyLen);
    if (bodyLen > 0) {
        writer->Write(bodyData.data(), bodyLen);
    }
    if (!resp.GetHeader(END_OF_BODY).empty()) {
        uint16_t endBodyLen = 0;
        std::vector<uint8_t> endBodyData = resp.GetEndOfBody(endBodyLen);
        if (endBodyLen > 0) {
            writer->Write(endBodyData.data(), endBodyLen);
        }
        return true;
    }
    return false;
}

int ObexClientSession::ReadStreamingResponse(ObexHeader &resp)
{
    uint8_t responseCode = 0;
    uint32_t length = 0;
    if (ReadResponse(OBEX_OPCODE_GET_FINAL, responseCode, length, resp) < 0) {
        return -1;
    }
    if (HandleResponse(OBEX_OPCODE_GET_FINAL, responseCode, resp) < 0) {
        return -1;
    }
    return 0;
}

bool ObexClientSession::HasSrmEnabled(const ObexHeader &resp)
{
    std::vector<uint8_t> value = resp.GetHeader(SINGLE_RESPONSE_MODE);
    return (!value.empty() && value[0] == 0x01);
}

bool ObexClientSession::HasSrmp(const ObexHeader &resp)
{
    std::vector<uint8_t> value = resp.GetHeader(SINGLE_RESPONSE_MODE_PARAMETER);
    return (!value.empty() && value[0] == 0x01);
}

bool ObexClientSession::ProcessGetContinue(ObexHeader &req, ObexHeader &resp, bool srmActive)
{
    // SRM 流式模式下服务端不等待请求即推送响应，仅在需要提示时携带 SRMP。
    bool needRequest = !srmActive || HasSrmp(resp);
    int32_t recvTimeoutMs = resp.GetRecvTimeoutMs();
    resp.Clear();
    // Clear() 只清 headerMap_ 不清 responseCode_：显式清零防止 SendRequest 失败（写失败/读超时）
    // 时残留上一轮的 CONTINUE，导致 Get 循环误判继续迭代——对端停滞场景下每轮空耗一个
    // 读超时，直至 maxIterations 上限（最坏可达数小时）。
    resp.SetResponseCode(0);
    if (needRequest) {
        ObexHeader continueReq;
        // ConnectionId 仅在会话确实持有（>=0）时携带；无连接时发送 0xFFFFFFFF 属协议违例。
        int32_t connectionId = req.GetConnectionId();
        if (connectionId != -1) {
            continueReq.SetConnectionId(static_cast<uint32_t>(connectionId));
        }
        std::vector<uint8_t> continueHeaderArray;
        continueReq.GenerateHeaderValue(continueHeaderArray);
        SendRequest(OBEX_OPCODE_GET_FINAL, continueHeaderArray, resp);
        return true;
    }
    // 保持接收超时跨 Clear()，直接读取下一条流式响应（同样的单次读超时边界）
    resp.SetRecvTimeoutMs(recvTimeoutMs);
    CHECK_AND_RETURN_LOG_RET(ReadStreamingResponse(resp) >= 0, false, "ReadStreamingResponse error");
    return true;
}

int ObexClientSession::Get(ObexHeader &req, ObexHeader &resp, std::shared_ptr<ObexBodyObject> writer,
    uint32_t maxIterations)
{
    CHECK_AND_RETURN_LOG_RET(transport_ != nullptr, -1, "transport is null");
    // 同一会话同时仅允许一个在途 GET：并发请求/响应交错会导致 OBEX 流失步（读到他方响应）。
    std::unique_lock<BtMutex> operationLock(operationMutex_, std::try_to_lock);
    CHECK_AND_RETURN_LOG_RET(operationLock.owns_lock(), -1, "another OBEX GET operation is already in progress");
    std::vector<uint8_t> headerArray;
    // 接收超时由调用方配置在请求头（如 BIP GET_RESPONSE_TIMEOUT_MS），但 ReadResponse()
    // 从响应头读取。传播该值使本次操作的所有读取均有超时边界；否则 timeout=0 退化为
    // 裸阻塞 recv()（对端停滞 = 永久挂起）。
    if (resp.GetRecvTimeoutMs() == 0 && req.GetRecvTimeoutMs() > 0) {
        resp.SetRecvTimeoutMs(req.GetRecvTimeoutMs());
    }
    // SRM 双重门控：profile 配置（SetSrmEnabled）+ transport 能力（非 RFCOMM）。
    // SRM 头仅随首个请求发送；后续 continue-GET 刻意不携带（对齐双框架语义）。
    // 不支持 SRM 的对端按规范忽略 0x97 头，整体回退经典 ping-pong 模式。
    bool srmCapable = srmEnabled_ && transport_->IsSrmSupported();
    if (srmCapable) {
        req.SetSrm(true);
    }
    req.GenerateHeaderValue(headerArray);
    SendRequest(OBEX_OPCODE_GET_FINAL, headerArray, resp);

    // 当本操作首个响应回带 SINGLE_RESPONSE_MODE=enabled 时 SRM 被接受；否则回退
    // 经典请求/响应 ping-pong 模式（与无 SRM 构建行为完全一致）。
    bool srmActive = srmCapable && HasSrmEnabled(resp);
    if (srmActive) {
        HILOGI("SRM enabled by peer, streaming mode active");
    }

    uint32_t iterations = 0;
    while (writer != nullptr) {
        // 防御：个别对端不在首个响应、而在续传中途才回 SRM=1，逐包重查避免整轮误走轮询。
        if (!srmActive && srmCapable && HasSrmEnabled(resp)) {
            srmActive = true;
            HILOGI("SRM enabled by peer during continuation");
        }
        // 先写 body 再判断终止：单包 OK+EndOfBody 响应（如小体积 properties）也必须落盘，
        // 否则最终包体会被整包丢弃。
        if (WriteRespBody(resp, writer)) {
            break;
        }
        if (resp.GetResponseCode() != ResponseCodes::OBEX_HTTP_CONTINUE) {
            break;
        }
        if (iterations >= maxIterations) {
            HILOGW("Max continuation iterations reached, truncating data");
            break;
        }
        iterations++;
        if (!ProcessGetContinue(req, resp, srmActive)) {
            return -1;
        }
        uint8_t code = resp.GetResponseCode();
        if (code != ResponseCodes::OBEX_HTTP_CONTINUE && code != ResponseCodes::OBEX_HTTP_OK) {
            HILOGE("Get: request failed, responseCode=0x%{public}02X, exit loop", code);
            break;
        }
    }

    return (resp.GetResponseCode() == ResponseCodes::OBEX_HTTP_OK) ? 0 : -1;
}

int ObexClientSession::SetPath(uint8_t flags, const std::u16string &path, ObexHeader &resp)
{
    ObexHeader reqHeader;
    if (!path.empty()) {
        SetHeaderNameU16(reqHeader, path);
    }

    std::vector<uint8_t> headerArray;
    reqHeader.GenerateHeaderValue(headerArray);

    uint16_t totalLen = static_cast<uint16_t>(HEAD_SETPATH_LEN + headerArray.size());

    std::vector<uint8_t> sendData;
    sendData.push_back(OBEX_OPCODE_SETPATH);
    sendData.push_back((totalLen >> 8) & 0xFF);
    sendData.push_back(totalLen & 0xFF);
    sendData.push_back(flags);
    sendData.push_back(0x00);
    sendData.insert(sendData.end(), headerArray.begin(), headerArray.end());

    std::shared_ptr<ObexResponseHandler> responseHandlerSptr = responseHandler_.lock();
    if (transport_ == nullptr) {
        return -1;
    }
    if (transport_->Write(sendData.data(), sendData.size()) == INVAILD_SOCKET && responseHandlerSptr != nullptr) {
        responseHandlerSptr->OnAbortResp();
        return -1;
    }

    SetLastOpeId(OBEX_OPCODE_SETPATH);
    uint8_t responseCode = 0;
    uint32_t length = 0;
    int readResult = ReadResponse(OBEX_OPCODE_SETPATH, responseCode, length, resp);
    if (readResult < 0 && responseHandlerSptr != nullptr) {
        responseHandlerSptr->OnAbortResp();
        return -1;
    }
    resp.SetResponseCode(responseCode);

    if (responseCode == ResponseCodes::OBEX_HTTP_OK) {
        return 0;
    }
    return -1;
}

void ObexClientSession::SetHeaderNameU16(ObexHeader &header, const std::u16string &name)
{
    // OBEX NAME 头按协议定义为 UTF-16BE 编码并以 NUL（0x0000）终止。
    constexpr uint8_t NUL_LOW_BYTE = 0x00;
    constexpr uint8_t NUL_HIGH_BYTE = 0x00;
    constexpr uint8_t LOW_BYTE_MASK = 0xFF;
    constexpr uint8_t HIGH_BYTE_SHIFT = 8;
    std::vector<uint8_t> value;
    for (char16_t c : name) {
        uint16_t v = static_cast<uint16_t>(c);
        value.push_back((v >> HIGH_BYTE_SHIFT) & LOW_BYTE_MASK);
        value.push_back(v & LOW_BYTE_MASK);
    }
    value.push_back(NUL_HIGH_BYTE);
    value.push_back(NUL_LOW_BYTE);
    header.SetHeader(NAME, value);
}
}  // namespace bluetooth
}  // namespace OHOS
