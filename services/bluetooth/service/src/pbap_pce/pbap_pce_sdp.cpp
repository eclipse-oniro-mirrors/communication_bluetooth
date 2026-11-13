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

#include "pbap_pce_sdp.h"
#include <memory>
#include "btcommon/message.h"
#include "pbap_pce_header_msg.h"
#include "pbap_pce_app_params.h"
#include "bt_def.h"
#include "log.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_pbap_pce_sdp"
#endif

namespace OHOS {
namespace bluetooth {

PbapPceSdp::PbapPceSdp(SdpResponseCallback callback) : callback_(std::move(callback))
{}

PbapPceSdp::~PbapPceSdp()
{
    // 注销全局回调：防止析构后晚到的 SDP 查询结果回调悬垂 this（UAF）。
    // Set 与派发互斥（SdpAdapter 内锁），注销会等待在途回调执行完成
    SdpAdapter::GetInstance().SetPbapPceSdpFoundCallback(nullptr);
}

int PbapPceSdp::Register()
{
    PbapPceSdpCreateParam param = {
        .name = PCE_SERVICE_NAME.c_str(),
        .l2capPsm = PBAP_PCE_LOCAL_GOEP_L2CAP_PSM,
        .version = PBAP_PCE_VERSION_NUMBER,
    };
    sdpHandle_ = SdpAdapter::GetInstance().CreatePbapPceSdpRecord(param);
    if (sdpHandle_ < 0) {
        PBAP_PCE_LOG_ERROR("CreatePbapPceSdpRecord failed, handle=%{public}d", sdpHandle_);
        return sdpHandle_;
    }
    PBAP_PCE_LOG_INFO("%{public}s end, handle=%{public}d", __PRETTY_FUNCTION__, sdpHandle_);
    return 0;
}

int PbapPceSdp::Deregister() const
{
    // 无有效 handle（未注册或注册失败）时无可注销记录，直接返回
    if (sdpHandle_ <= 0) {
        PBAP_PCE_LOG_WARN("%{public}s no valid sdp handle, skip deregister", __PRETTY_FUNCTION__);
        return 0;
    }
    bool ret = SdpAdapter::GetInstance().RemoveSdpRecord(sdpHandle_);
    if (!ret) {
        PBAP_PCE_LOG_ERROR("RemoveSdpRecord failed");
        return -1;
    }
    return 0;
}

int PbapPceSdp::SdpSearch(const std::string &address)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(address).c_str());

    // 回调内校验结果地址与本次请求地址一致：SDP 回调为进程级单槽位，并发搜索时
    // 后注册的回调覆盖前者，地址不匹配的晚到结果直接丢弃，避免串扰
    SdpAdapter::GetInstance().SetPbapPceSdpFoundCallback(
        [this, address](const PbapPceSdpFoundRecord &record) {
            if (record.deviceAddr != address) {
                PBAP_PCE_LOG_WARN("sdp result addr mismatch, drop (stale or overwritten search)");
                return;
            }
            OnSdpFound(record);
        });

    bool ret = SdpAdapter::GetInstance().StartRemoteSdpSearch(address, BLUETOOTH_UUID_PBAP_PSE);
    if (!ret) {
        PBAP_PCE_LOG_ERROR("StartRemoteSdpSearch failed");
        return -1;
    }
    return 0;
}

void PbapPceSdp::OnSdpFound(const PbapPceSdpFoundRecord &record)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s, status=%{public}d", __PRETTY_FUNCTION__,
        GetEncryptAddr(record.deviceAddr).c_str(), record.status);
    if (record.status != 0) {
        NotifySdpFailed(record.deviceAddr, "SDP search failed, status=" + std::to_string(record.status));
        return;
    }

    if (!ValidateSdpRecord(record)) {
        NotifySdpFailed(record.deviceAddr, "invalid sdp record");
        return;
    }

    SdpParams params = ExtractSdpParams(record);
    PBAP_PCE_LOG_INFO("%{public}s rfcomm=%{public}d, l2capPsm=%{public}d, version=%{public}u, "
        "supportedRes=%{public}u, supportedFeature=%{public}u", __PRETTY_FUNCTION__,
        params.rfcommChannel, params.l2capPsm, params.versionNumber, params.supportedRes,
        params.supportedFeature);

    if (params.IsValid()) {
        PostSdpFinish(record.deviceAddr, params);
    } else {
        NotifySdpFailed(record.deviceAddr, "sdp failed, invalid params");
    }
}

PbapPceSdp::SdpParams PbapPceSdp::ExtractSdpParams(const PbapPceSdpFoundRecord &record)
{
    SdpParams params {};
    params.rfcommChannel = record.rfcommChannel;
    params.l2capPsm = record.l2capPsm;
    if (params.l2capPsm == -1) {
        PBAP_PCE_LOG_INFO("l2capPsm=-1 (sentinel: GOEP_L2CAP_PSM absent), normalize to 0, fallback to RFCOMM");
        params.l2capPsm = 0;
    }

    params.versionNumber = static_cast<uint16_t>(record.version);
    params.supportedRes = static_cast<uint8_t>(record.supportedRepositories);
    params.supportedFeature = static_cast<uint32_t>(record.features);
    params.featureFlag = (record.features != 0);
    if (!params.featureFlag) {
        params.supportedFeature = PbapPceAppParams::PBAP_SUPPORTED_FEATURES;
    }

    params.obexConfig = BuildObexConfig(record, params.l2capPsm, params.rfcommChannel);
    return params;
}

void PbapPceSdp::PostSdpFinish(const std::string &addr, SdpParams params)
{
    auto sdpMsg = std::make_unique<PbapPceHeaderSdpMsg>(
        params.obexConfig, params.versionNumber, params.supportedRes, params.supportedFeature, params.featureFlag);
    void *rawPtr = static_cast<void *>(sdpMsg.release());
    utility::Message msg(PCE_SDP_FINISH, 0, rawPtr);
    PBAP_PCE_LOG_INFO("%{public}s PCE_SDP_FINISH created, arg2=%{public}p, addr=%{public}s, invoking callback",
        __PRETTY_FUNCTION__, rawPtr, GetEncryptAddr(addr).c_str());
    if (callback_) {
        callback_(addr, msg);
    } else {
        // 回调缺失：msg 所有权无人接管，主动释放避免泄漏
        PBAP_PCE_LOG_ERROR("%{public}s callback_ is null! delete sdpMsg, arg2=%{public}p", __PRETTY_FUNCTION__,
            rawPtr);
        delete static_cast<PbapPceHeaderSdpMsg *>(rawPtr);
    }
}

void PbapPceSdp::NotifySdpFailed(const std::string &addr, const std::string &reason)
{
    PBAP_PCE_LOG_ERROR("%{public}s %{public}s", __PRETTY_FUNCTION__, reason.c_str());
    utility::Message msg(PCE_SDP_FAILED);
    if (callback_) {
        callback_(addr, msg);
    }
}

bool PbapPceSdp::ValidateSdpRecord(const PbapPceSdpFoundRecord &record)
{
    if (record.rfcommChannel < 0 || record.rfcommChannel > 0xFFFF) {
        PBAP_PCE_LOG_ERROR("Invalid rfcommChannel=%{public}d, out of uint16_t range", record.rfcommChannel);
        return false;
    }
    if (record.l2capPsm < -1 || record.l2capPsm > 0xFFFF) {
        PBAP_PCE_LOG_ERROR("Invalid l2capPsm=%{public}d, out of uint16_t range", record.l2capPsm);
        return false;
    }
    if (record.version < 0 || record.version > 0xFFFF) {
        PBAP_PCE_LOG_ERROR("Invalid version=%{public}d, out of uint16_t range", record.version);
        return false;
    }
    if (record.supportedRepositories < 0 || record.supportedRepositories > 0xFF) {
        PBAP_PCE_LOG_ERROR("Invalid supportedRepositories=%{public}d, out of uint8_t range",
            record.supportedRepositories);
        return false;
    }
    return true;
}

PbapPceObexConfig PbapPceSdp::BuildObexConfig(
    const PbapPceSdpFoundRecord &record, int32_t l2capPsm, int32_t rfcommChannel)
{
    PbapPceObexConfig obexConfig;
    obexConfig.addr = record.deviceAddr;

    bool isGoepL2capPSM = false;
#ifdef PBAP_PCE_ENABLE_L2CAP_PSM
    isGoepL2capPSM = record.hasL2capPsm;
    if (isGoepL2capPSM) {
        PBAP_PCE_LOG_INFO("GOEP_L2CAP_PSM attribute present, l2capPsm=%{public}d", int(l2capPsm));
    } else {
        PBAP_PCE_LOG_INFO("GOEP_L2CAP_PSM attribute absent, falling back to rfcomm=%{public}d", int(rfcommChannel));
    }
#endif

    if (isGoepL2capPSM) {
        obexConfig.l2capPsm = static_cast<uint16_t>(l2capPsm);
        obexConfig.l2capLocalPsm = PBAP_PCE_LOCAL_GOEP_L2CAP_PSM;
        obexConfig.isL2capPSM = true;
        obexConfig.isSupportSrm = true;
        PBAP_PCE_LOG_INFO("connect with psm:[%{public}d]", int(l2capPsm));
    } else {
        obexConfig.rfCommChannel = static_cast<uint16_t>(rfcommChannel);
        PBAP_PCE_LOG_INFO("connect with rfcommNo:[%u]", static_cast<unsigned>(rfcommChannel));
    }
    obexConfig.serviceUUID = BLUETOOTH_UUID_PBAP_PSE;
    return obexConfig;
}
}  // namespace bluetooth
}  // namespace OHOS
