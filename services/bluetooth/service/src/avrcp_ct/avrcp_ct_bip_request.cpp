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
#define LOG_TAG "avrcp_ct_bip_request"
#endif

#include "avrcp_ct_bip_request.h"
#include "log.h"
#include "obex_def.h"
#include <utility>

namespace OHOS {
namespace bluetooth {

namespace {
// 内存版 body 收集器：BIP GET 响应体（properties XML / 图片）整体缓存在内存，
// 收满后由调用方经 GetData() 一次性取走。按仓库惯例，ObexBodyObject 的具体实现
// 放在使用方 profile 文件内（对齐 OPP 文件版 body 在 OppObexClient 内的定位）。
class ObexMemoryBody : public ObexBodyObject {
public:
    void OpenFile(const int fileFd) override {}
    int Close() override { return 0; }
    size_t Read(uint8_t *buf, size_t bufLen) override
    {
        return 0;
    }
    size_t Write(const uint8_t *buf, size_t bufLen) override
    {
        if (buf == nullptr || bufLen == 0) {
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

private:
    std::vector<uint8_t> data_;
};
}  // namespace

static std::vector<uint8_t> Utf8ToUtf16BeWithNullTerm(const std::string &str)
{
    std::vector<uint8_t> ret;
    ret.reserve((str.size() + 1) * 2);
    for (unsigned char c : str) {
        ret.push_back(0x00);
        ret.push_back(c);
    }
    ret.push_back(0x00);
    ret.push_back(0x00);
    return ret;
}

static bool MatchesSignature(const std::vector<uint8_t> &data, size_t offset, const uint8_t *sig, size_t sigLen)
{
    if (data.size() < offset + sigLen) {
        return false;
    }
    for (size_t i = 0; i < sigLen; ++i) {
        if (data[offset + i] != sig[i]) {
            return false;
        }
    }
    return true;
}

// 校验图片负载真实有效：JPEG 校验 SOI/EOI，PNG 校验文件签名/IEND 尾部。
// 防止把 img-properties XML（234 字节，'<?xml' 开头）或截断数据当图片上抛。
static bool ValidateImageData(const std::vector<uint8_t> &data, uint8_t format)
{
    static const uint8_t JPEG_SOI[] = {0xFF, 0xD8};
    static const uint8_t JPEG_EOI[] = {0xFF, 0xD9};
    static const uint8_t PNG_SIG[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    static const uint8_t PNG_IEND[] = {0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};

    if (data.empty()) {
        HILOGE("Image data is empty");
        return false;
    }
    if (format == BIP_IMAGE_FORMAT_JPEG) {
        if (!MatchesSignature(data, 0, JPEG_SOI, sizeof(JPEG_SOI))) {
            HILOGE("Invalid JPEG: SOI marker missing, size=%{public}zu", data.size());
            return false;
        }
        // EOI 允许出现在尾部若干字节内（个别编码器在 EOI 后有填充字节）
        size_t tailStart = data.size() > 8 ? data.size() - 8 : 0;
        for (size_t i = tailStart; i + 1 < data.size(); ++i) {
            if (data[i] == JPEG_EOI[0] && data[i + 1] == JPEG_EOI[1]) {
                return true;
            }
        }
        HILOGE("Invalid JPEG: EOI marker missing, size=%{public}zu", data.size());
        return false;
    }
    if (format == BIP_IMAGE_FORMAT_PNG) {
        if (!MatchesSignature(data, 0, PNG_SIG, sizeof(PNG_SIG))) {
            HILOGE("Invalid PNG: signature mismatch, size=%{public}zu", data.size());
            return false;
        }
        if (!MatchesSignature(data, data.size() - sizeof(PNG_IEND), PNG_IEND, sizeof(PNG_IEND))) {
            HILOGE("Invalid PNG: IEND trailer missing, size=%{public}zu", data.size());
            return false;
        }
        return true;
    }
    return true;
}

RequestGetImageProperties::RequestGetImageProperties(const std::string &imageHandle)
{
    imageHandle_ = imageHandle;
}

int RequestGetImageProperties::GetType() const
{
    return TYPE_GET_IMAGE_PROPERTIES;
}

std::string RequestGetImageProperties::ToString() const
{
    return "RequestGetImageProperties { handle=" + imageHandle_ + " }";
}

// 收到 0x90 Continue 后的续传（空 GET、不带 TYPE/handle 头）与 SRM 协商
// 均收敛在 ObexClientSession::Get 内部处理：
// 重发完整头等于开启新 OBEX 操作，对端会为每个 GET 排队一个完整响应，
// 导致响应队列错位（见 2hci.xml 中 27 个重复 img-properties GET）。
bool RequestGetImageProperties::Execute(std::shared_ptr<ObexClientSession> clientSession)
{
    HILOGI("Processing GetImageProperties request: %{public}s", imageHandle_.c_str());

    // responseCode_ 成员默认 -1 且请求对象单次使用，null 失败路径无需再显式赋值。
    CHECK_AND_RETURN_LOG_RET(clientSession != nullptr, false, "clientSession is null");

    ObexHeader reqHeader;
    // OBEX 规范：会话建立后所有请求必须带 ConnectionId（且 GenerateHeaderValue 会把它放在 header 最前），
    // 否则对端 OBEX Server 无法关联回会话，会丢弃请求不回响应（表现为 ReadResponse 永远等不到包，
    // 最终 L2CAP 超时关闭）。对齐 PBAP PbapPceObexClient::BuildGetHeader 的处理。
    if (connectionId_ >= 0) {
        reqHeader.SetConnectionId(static_cast<uint32_t>(connectionId_));
    }
    std::string typeStr{BipRequest::HEAD_TYPE_GET_IMAGE_PROPERTIES};
    std::vector<uint8_t> typeVec(typeStr.begin(), typeStr.end());
    reqHeader.SetHeader(TYPE, typeVec);
    // IMG_HANDLE(0x30) 为 UNICODE_TEXT 类 header，必须 UTF-16BE + null 结尾。
    // 详见本文件顶部 Utf8ToUtf16BeWithNullTerm 注释及 avrcp_tg_bip_obex_server.cpp:61。
    reqHeader.SetHeader(BipRequest::HEADER_ID_IMG_HANDLE, Utf8ToUtf16BeWithNullTerm(imageHandle_));

    // OBEX GET（内部自动协商 SRM）：接收超时配置在请求头，Get 内部传播到响应头。
    auto bodyWriter = std::make_shared<ObexMemoryBody>();
    reqHeader.SetRecvTimeoutMs(BipRequest::GET_RESPONSE_TIMEOUT_MS);
    ObexHeader outHeader;
    int ret = clientSession->Get(reqHeader, outHeader, bodyWriter, BipRequest::MAX_CONTINUE_ITERATIONS);
    responseCode_ = outHeader.GetResponseCode();

    // 无论成败都先解析：失败路径回调方（NotifyCaller）仍会读取 properties 内容用于定位问题。
    properties_.ParseFromData(bodyWriter->GetData());

    CHECK_AND_RETURN_LOG_RET(ret == 0 || responseCode_ == OBEX_HTTP_CONTINUE, false,
        "GetImageProperties failed, responseCode=0x%{public}02X", responseCode_);
    if (ret != 0) {
        HILOGW("Max continuation iterations reached, truncating data");
    }

    // 响应中的 handle 必须与请求一致；不一致说明读到的是历史遗留响应（流已失步），
    // 返回 false 让上层销毁 OBEX 会话，避免脏数据继续污染后续请求。
    CHECK_AND_RETURN_LOG_RET(properties_.GetImageHandle() == imageHandle_, false,
        "GetImageProperties handle mismatch: request=%{public}s, response=%{public}s",
        imageHandle_.c_str(), properties_.GetImageHandle().c_str());

    HILOGI("GetImageProperties complete for handle: %{public}s, responseCode=0x%{public}02X, %{public}s",
           imageHandle_.c_str(), responseCode_, properties_.ToString().c_str());
    return true;
}

RequestGetImage::RequestGetImage(const std::string &imageHandle, const BipImageDescriptor &descriptor)
{
    imageHandle_ = imageHandle;
    descriptor_ = descriptor;
}

int RequestGetImage::GetType() const
{
    return TYPE_GET_IMAGE;
}

std::string RequestGetImage::ToString() const
{
    return "RequestGetImage { handle=" + imageHandle_ + ", format=" + std::to_string(descriptor_.format) + ", " +
           std::to_string(descriptor_.width) + "x" + std::to_string(descriptor_.height) + " }";
}

// 对齐 BipImageDescriptor.serialize()：SCHEME_NATIVE 时不发 IMG_DESCRIPTOR header（信号 native 格式），
// SCHEME_THUMBNAIL 时按 descriptor 字段动态生成 XML（避免硬编码 200*200）
static void AppendImageDescriptorHeader(ObexHeader &reqHeader, const BipImageDescriptor &descriptor)
{
    if (descriptor.format == BIP_IMAGE_FORMAT_NATIVE) {
        return;
    }
    std::string encodingStr;
    switch (descriptor.format) {
        case BIP_IMAGE_FORMAT_PNG:  encodingStr = "PNG";  break;
        case BIP_IMAGE_FORMAT_JPEG:
        default:                    encodingStr = "JPEG"; break;
    }
    std::string xml = "<image-descriptor version=\"1.0\">"
                      "<image encoding=\"" + encodingStr + "\""
                      " pixel=\"" + std::to_string(descriptor.width) + "*"
                      + std::to_string(descriptor.height) + "\"/>"
                      "</image-descriptor>";
    std::vector<uint8_t> descriptorBytes(xml.begin(), xml.end());
    reqHeader.SetHeader(BipRequest::HEADER_ID_IMG_DESCRIPTOR, descriptorBytes);
}

bool RequestGetImage::Execute(std::shared_ptr<ObexClientSession> clientSession)
{
    HILOGI("Processing GetImage request: %{public}s", imageHandle_.c_str());

    // responseCode_ 成员默认 -1 且请求对象单次使用，null 失败路径无需再显式赋值。
    CHECK_AND_RETURN_LOG_RET(clientSession != nullptr, false, "clientSession is null");

    ObexHeader reqHeader;
    // OBEX 规范：会话建立后所有请求必须带 ConnectionId，否则对端 OBEX Server 无法
    if (connectionId_ >= 0) {
        reqHeader.SetConnectionId(static_cast<uint32_t>(connectionId_));
    }
    std::string typeStr{BipRequest::HEAD_TYPE_GET_IMAGE};
    std::vector<uint8_t> typeVec(typeStr.begin(), typeStr.end());
    reqHeader.SetHeader(TYPE, typeVec);
    reqHeader.SetHeader(BipRequest::HEADER_ID_IMG_HANDLE, Utf8ToUtf16BeWithNullTerm(imageHandle_));
    // SCHEME_NATIVE 时不发 IMG_DESCRIPTOR header
    //（信号 native 格式），SCHEME_THUMBNAIL 时按 descriptor_ 字段动态生成 XML。
    AppendImageDescriptorHeader(reqHeader, descriptor_);

    // OBEX GET（内部自动协商 SRM）：大图下载场景，L2CAP 下对端回显 SRM=1 后分片连续下发，
    // 省去逐分片 GET_FINAL 往返；接收超时配置在请求头，Get 内部传播到响应头。
    // 图片本体远大于 properties XML，续传上限用 MAX_IMAGE_CONTINUE_ITERATIONS 避免合法图片被截断。
    auto bodyWriter = std::make_shared<ObexMemoryBody>();
    reqHeader.SetRecvTimeoutMs(BipRequest::GET_RESPONSE_TIMEOUT_MS);
    ObexHeader outHeader;
    int ret = clientSession->Get(reqHeader, outHeader, bodyWriter, BipRequest::MAX_IMAGE_CONTINUE_ITERATIONS);
    responseCode_ = outHeader.GetResponseCode();

    CHECK_AND_RETURN_LOG_RET(ret == 0 || responseCode_ == OBEX_HTTP_CONTINUE, false,
        "GetImage failed, responseCode=0x%{public}02X", responseCode_);
    if (ret != 0) {
        HILOGW("Max continuation iterations reached, truncating image data");
    }

    std::vector<uint8_t> imageData = bodyWriter->GetData();
    if (!ValidateImageData(imageData, descriptor_.format)) {
        HILOGE("GetImage payload validation failed, handle=%{public}s, size=%{public}zu",
            imageHandle_.c_str(), imageData.size());
        responseCode_ = -1;
        return false;
    }

    image_.data = std::move(imageData);
    HILOGI("Image download complete: responseCode=0x%{public}02X, size=%{public}zu",
        responseCode_, image_.data.size());
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS
