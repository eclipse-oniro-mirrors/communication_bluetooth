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

#ifndef PBAP_PCE_HEADER_MSG_H
#define PBAP_PCE_HEADER_MSG_H

#include <cstdint>
#include <cstring>
#include <memory>
#include "../obex/obex_header.h"
#include "btcommon/message.h"
#include "pbap_pce_def.h"
#include "pbap_pce_tlv.h"

namespace OHOS {
namespace bluetooth {
/// pbap pce state mathine message
enum PbapPceStmMessage {
    PCE_SDP_FINISH = 0x1001,             // sdp search success
    PCE_SDP_FAILED,                      // sdp search failed
    PCE_REQ_DISCONNECTED,                // disconnect request
    PCE_OBEX_CONNECTED,                  // obex connected
    PCE_OBEX_CONNECT_FAILED,             // obex connect failed
    PCE_TRANSPORT_FAILED,                // transport failed
    PCE_OBEX_DISCONNECTED,               // obex disconnected
    PCE_DISCONNECT_TIMEOUT,              // disconnect timeout (3s)
    PCE_CONNECT_TIMEOUT,                 // connect timeout (10s)
    PCE_REQ_SET_TARGET_STATE,            // request setting target state
    PCE_REQ_TRANSIT_TARGET_STATE,        // transit target state
    PCE_REQ_PULLPHONEBOOKSIZE,           // pull phonebooksize
    PCE_PULLPHONEBOOKSIZE_COMPLETED,     // pull phonebooksize completed
    PCE_REQ_PULLPHONEBOOK,               // pull phonebook
    PCE_PULLPHONEBOOK_COMPLETED,         // pull phonebook completed
    PCE_REQ_ABORTDOWNLOADING,            // abort downloading
    PCE_ABORTDOWNLOADING_COMPLETED,      // abort downloading completed
    PCE_DOWNLOAD_COMPLETE,               // auto-download of contacts and call logs completed
    PCE_DOWNLOAD_FAILED,                 // auto-download failed (batch error / zero progress)
    PCE_REQ_DOWNLOAD,                    // request to trigger contacts/call-logs download
};

struct PbapPceObexConfig {
    std::string addr {};
    uint16_t l2capLocalPsm = 0;
    uint16_t l2capPsm = 0;
    uint16_t rfCommChannel = 0;
    uint16_t mtu = 0;
    int32_t fileCount = 0;
    bool isL2capPSM = false;
    bool isSupportSrm = false;
    bool isSupportReliableSession = false;
    std::string serviceUUID {};
};

class PbapPceHeaderSdpMsg {
public:
    explicit PbapPceHeaderSdpMsg(PbapPceObexConfig &obexConfig, uint16_t versionNumber, uint8_t supportedRes,
        uint32_t supportedFeature, bool featureFlag)
        : obexConfig_(obexConfig),
          versionNumber_(versionNumber),
          supportedRes_(supportedRes),
          supportedFeature_(supportedFeature),
          featureFlag_(featureFlag)
    {}
    PbapPceHeaderSdpMsg(const PbapPceHeaderSdpMsg &other) = default;
    PbapPceHeaderSdpMsg &operator=(const PbapPceHeaderSdpMsg &other) = default;
    virtual ~PbapPceHeaderSdpMsg() = default;

    const PbapPceObexConfig &GetObexClientConfig() const
    {
        return obexConfig_;
    }

    uint16_t GetVersionNumber() const
    {
        return versionNumber_;
    }

    uint8_t GetSupportedRes() const
    {
        return supportedRes_;
    }

    uint32_t GetSupportedFeature() const
    {
        return supportedFeature_;
    }

    bool GetFeatureFlag() const
    {
        return featureFlag_;
    }

private:
    PbapPceObexConfig obexConfig_ {};
    uint16_t versionNumber_ = 0;
    uint8_t supportedRes_ = 0;
    uint32_t supportedFeature_ = 0;
    bool featureFlag_ = false;
};

class PbapPceObexMessage {
public:
    explicit PbapPceObexMessage(const std::string &remoteAddr, const ObexHeader &resp)
        : remoteAddr_(remoteAddr), resp_(resp)
    {}

    virtual ~PbapPceObexMessage() = default;

    const std::string &GetRemoteAddress() const
    {
        return remoteAddr_;
    }

    const ObexHeader &GetObexHeader() const
    {
        return resp_;
    }

private:
    std::string remoteAddr_;
    ObexHeader resp_;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_HEADER_MSG_H
