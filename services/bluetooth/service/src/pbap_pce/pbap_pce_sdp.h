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

#ifndef PBAP_PCE_SDP_H
#define PBAP_PCE_SDP_H

#include <cstdint>
#include <functional>
#include "btcommon/message.h"
#include "../sdp_adapter/sdp_adapter.h"
#include "pbap_pce_header_msg.h"

namespace OHOS {
namespace bluetooth {
constexpr uint16_t PBAP_PCE_VERSION_NUMBER = 0x0102;

class PbapPceSdp {
public:
    using SdpResponseCallback = std::function<void(const std::string &addr, const utility::Message &msg)>;

    explicit PbapPceSdp(SdpResponseCallback callback);
    virtual ~PbapPceSdp();

    int Register();
    int Deregister() const;
    int SdpSearch(const std::string &address);

private:
    struct SdpParams {
        PbapPceObexConfig obexConfig {};
        int32_t l2capPsm = 0;
        int32_t rfcommChannel = 0;
        uint16_t versionNumber = 0;
        uint8_t supportedRes = 0;
        uint32_t supportedFeature = 0;
        bool featureFlag = false;

        bool IsValid() const
        {
            return (versionNumber != 0) && (supportedRes != 0) && (supportedFeature != 0) &&
                   (l2capPsm != 0 || rfcommChannel != 0);
        }
    };

    void OnSdpFound(const PbapPceSdpFoundRecord &record);
    bool ValidateSdpRecord(const PbapPceSdpFoundRecord &record);
    PbapPceObexConfig BuildObexConfig(
        const PbapPceSdpFoundRecord &record, int32_t l2capPsm, int32_t rfcommChannel);
    SdpParams ExtractSdpParams(const PbapPceSdpFoundRecord &record);
    void PostSdpFinish(const std::string &addr, SdpParams params);
    void NotifySdpFailed(const std::string &addr, const std::string &reason);

    // 必须为有符号：CreatePbapPceSdpRecord 返回 int32_t，负值表示失败。
    // 无符号类型会使 < 0 失败检查恒为 false（注册失败检测失效）
    int32_t sdpHandle_ = 0;
    SdpResponseCallback callback_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_SDP_H
