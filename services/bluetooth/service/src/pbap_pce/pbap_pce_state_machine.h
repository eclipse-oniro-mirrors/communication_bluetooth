/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PCE_STATE_MACHINE_H
#define PCE_STATE_MACHINE_H
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "../obex/obex_header.h"
#include "btcommon/timer_manager.h"
#include "pbap_pce_tlv.h"
#include "interface_profile_pbap_pce.h"
#include "log.h"
#include "btcommon/message.h"
#include "pbap_pce_def.h"
#include "pbap_pce_sdp.h"
#include "pbap_pce_header_msg.h"
#include "pbap_pce_obex_client.h"
#include "raw_address.h"
#include "btcommon/state_machine.h"
#include "bt_def.h"
#include "pbap_pce_app_params.h"

namespace OHOS {
namespace bluetooth {

class PbapPceServiceImpl;

class PbapPceStateMachine : public utility::StateMachine,
    public std::enable_shared_from_this<PbapPceStateMachine> {
public:
    /**
     * @brief constructor
     * @details constructor
     * @param dev remote device
     * @param pceService pce service
     * @param pbapPceSdp sdp service
     * @param observerMgrList pce observer list
     * @return
     */
    explicit PbapPceStateMachine(const RawAddress &dev, PbapPceServiceImpl &pceService, PbapPceSdp &pbapPceSdp,
        std::shared_ptr<IPbapPceObserver> observer);

    /**
     * @brief destructor
     * @details destructor
     */
    virtual ~PbapPceStateMachine();

    /**
     * @brief get remote device
     * @details get remote device
     * @return RawAddress&
     */
    const RawAddress &GetDevice() const;

    /**
     * @brief get pce service
     * @details get pce service
     * @return Reference to PbapPceService
     */
    PbapPceServiceImpl &GetPceService() const;

    /**
     * @brief get pce obex client
     * @details get pce obex client
     * @return Reference to PbapPceObexClient
     */
    std::shared_ptr<PbapPceObexClient> GetObexClientPtr() const;

    /**
     * @brief Set Version Number
     * @details Set Version Number
     * @param versionNumber version Number
     * @return
     */
    void SetVersionNumber(uint16_t versionNumber);

    /**
     * @brief get version number
     * @details get version number
     * @return uint16_t
     */
    uint16_t GetVersionNumber() const;

    /**
     * @brief Set supported repositories
     * @details Set supported repositories
     * @param supportedRes supported repositories
     * @return
     */
    void SetSupportedRes(uint8_t supportedRes);

    /**
     * @brief get supported repositories
     * @details get supported repositories
     * @return uint8_t
     */
    uint8_t GetSupportedRes() const;

    /**
     * @brief Set supported features
     * @details Set supported features
     * @param supportedRes supported features
     * @return
     */
    void SetSupportedFeature(uint32_t supportedFeature);

    /**
     * @brief get supported features
     * @details get supported features
     * @return uint32_t
     */
    uint32_t GetSupportedFeature() const;

    bool GetFeatureFlag() const;

    /**
     * @brief Set feature flag
     * @details Set feature flag
     * @param featureFlag
     * @return
     */
    void SetFeatureFlag(bool featureFlag);

    /**
     * @brief get sdp
     * @details get sdp
     * @return PbapPceSdp reference
     */
    PbapPceSdp &GetSdp() const;

    /**
     * @brief Create Pce Obex Client
     * @details Create Pce Obex Client
     * @param obexConfig
     * @return
     */
    void CreatePceObexClient(const PbapPceObexConfig &obexConfig);

    /**
     * @brief PullPhoneBook
     * @details PullPhoneBook
     */
    void PullPhoneBook(const PbapPceAppParams &param);

    /**
     * @brief IsBusy
     * @details Is Busy
     */
    bool IsBusy() const;

    /**
     * @brief SetBusy
     * @details SetBusy
     */
    bool SetBusy(bool isBusy);

    /**
     * @brief SetPowerStatusBusy
     * @details SetPowerStatusBusy
     */
    void SetPowerStatusBusy(bool isBusy);

    /**
     * @brief AbortDownloading
     * @details AbortDownloading
     */
    int AbortDownloading();

    /**
     * @brief SetConnectId
     * @details SetConnectId
     */
    void SetConnectId(uint32_t connectId);

    /**
     * @brief GetConnectId
     * @details GetConnectId
     * @return uint32_t
     */
    uint32_t GetConnectId() const;

    /**
     * @brief 强制关闭 obex client（shutdown socket），用于断连时唤醒阻塞的下载线程。
     * @details 调用 PbapPceObexClient::ForceClose()，绕过 OBEX DISCONNECT 协议，
     *          直接 shutdown+close socket，并发送 PCE_OBEX_DISCONNECTED 消息。
     */
    void ForceCloseObexClient();

    /**
     * @brief 释放 obex client 引用，供 DisconnectedState 清理。
     * @details 将 pceObexClient_ 置空，确保重连时 CreatePceObexClient 新建对象。
     *          ffrt 下载线程持有的 shared_ptr 仍保持 obexClient 存活，直到 lambda 退出。
     */
    void ResetObexClient();

public:
    void HandlePhoneBookSizeActionCompleted(const utility::Message &msg);
    void HandlePullPhoneBookActionCompleted(const utility::Message &msg);
    void HandleAbortDownloadingActionCompleted(const utility::Message &msg);
    void TryReleasePbapMsg(const utility::Message &msg) const;
    // 释放 Message arg2_ 携带的裸指针负载（按消息类型确定删除类型）。
    // 契约：每个携带 arg2_ 的消息有且仅有一个释放点——正常路径由消费方接管
    // （如 ProcessSdpFinish/ProcessObexConnected 的 unique_ptr），消息无法投递
    // 或落到非预期状态时由本函数兜底，避免泄漏与双重释放。
    static void ReleasePbapMsgPayload(const utility::Message &msg);
    void TransitTargetState();
    int GetTargetState() const;
    void SetTargetState(int targetState);

    /**
     * @brief 标记本次连接尝试失败（对端拒绝/超时/SDP失败等）
     * @details 置位后，进入 Disconnected 状态时会将同步状态置为
     *          PBAP_PCE_PHONEBOOK_STATE_DOWNLOAD_ERROR 并通知应用层。
     */
    void MarkConnectFailed();

    /**
     * @brief 连接尝试是否已标记为失败
     */
    bool IsConnectFailed() const;

    /**
     * @brief 清除连接失败标志
     */
    void ClearConnectFailed();

    // CONTACTS_SYN_STATUS_RECORD 通讯录同步打点（本模块内采集并上报）
    void SetConnectFailReason(int reason);
    int GetConnectFailReason() const;
    void RecordContactsConnecting();
    void MarkChrConnConnected();
    void RecordContactsConnectFailed(int reason);
    void MarkChrSyncStart();
    void MarkChrSyncFinish(bool failed, int failReason);
    void RecordContactsSyncFinish(bool failed, int failReason);
    void ReportContactsSynUeEvent(bool connectFailed, int connectFailReason);

private:
    RawAddress device_;                                           // remote device
    PbapPceServiceImpl *pceService_ {nullptr};                        // pce service
    std::shared_ptr<PbapPceObexClient> pceObexClient_ {nullptr};  // obex client
    // P0-3: pceObexClient_ 跨线程访问保护——PbapThread 写（创建/复位），
    // binder 线程读（IsDownloading IPC）。锁范围仅限本成员的拷贝/赋值，
    // 拷贝出的 shared_ptr 在锁外使用（调用方持引用保活）。
    mutable std::mutex obexClientMutex_ {};

    uint16_t versionNumber_ = 0;     // received from pse version number
    uint8_t supportedRes_ = 0;       // received from pse Supported Repositories
    uint32_t supportedFeature_ = 0;  // received from pse Supported feature
    bool featureFlag_ = false;       // true:can received feature; false:cannot received feature

    uint32_t connectId_ {0};          // session connectId from PceConnectedState state

    PbapPceSdp &pbapPceSdp_;                                // sdp
    std::shared_ptr<IPbapPceObserver> observer_;            // observer
    std::atomic_bool powerStatusBusy_ {false};              // powerStatusBusy_
    std::atomic_int targetState_ {0};                       // target State
    bool connectFailed_ {false};                            // connect attempt failed flag
    int connectFailReason_;                                 // PbapPceConnFailReason of last failed attempt
    // CONTACTS_SYN_STATUS_RECORD 打点数据（time 为 CLOCK_BOOTTIME ms，上报时经 ChrTS 转换）
    int64_t chrConnConnectingTime_ {0};
    int64_t chrConnConnectedTime_ {0};
    int64_t chrSyncStartTime_ {0};
    int64_t chrSyncEndTime_ {0};
    bool chrSyncStarted_ {false};
    bool chrSyncFailed_ {false};
    int chrSyncFailReason_;
    BT_DISALLOW_COPY_AND_ASSIGN(PbapPceStateMachine);
};

/// pbap pce state machine state names
inline const std::string PCE_DISCONNECTED_STATE = "PceDisconnectedState";
inline const std::string PCE_CONNECTING_STATE = "PceConnectingState";
inline const std::string PCE_CONNECTED_STATE = "PceConnectedState";
inline const std::string PCE_DISCONNECTING_STATE = "PceDisconnectingState";

/**
 * @brief base state of pbap pce states
 * @details common context of all pce states
 */
class PceBaseState : public utility::StateMachine::State {
public:
    /**
     * @brief constructor
     * @details constructor
     */
    explicit PceBaseState(
        const std::string &name,
        PbapPceStateMachine &sm,
        std::shared_ptr<IPbapPceObserver> observer);

    /**
     * @brief destructor
     * @details destructor
     */
    virtual ~PceBaseState() = default;

protected:
    PbapPceStateMachine &stateMachine_;
    std::shared_ptr<IPbapPceObserver> observer_;
};

/**
 * @brief disconnected statemachine
 * control disconnected state
 */
class PceDisconnectedState : public PceBaseState {
public:
    /**
     * @brief constructor
     * construct PceDisconnectedState instance
     */
    explicit PceDisconnectedState(
        const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer);

    /**
     * @brief destructor
     * @details destructor
     */
    ~PceDisconnectedState() override = default;

    /**
     * @brief entry
     * @details when become disconnected, call this
     * @return void
     */
    void Entry() override;

    /**
     * @brief exit
     * @details when leave disconnected, call this
     * @return void
     */
    void Exit() override;

    /**
     * @brief Dispatch
     * @details when Dispatch disconnected, call this
     * @param msg message
     * @return bool true:success false:failure
     */
    bool Dispatch(const utility::Message &msg) override;
};

/**
 * @brief connecting statemachine
 * control connecting state
 */
class PceConnectingState : public PceBaseState {
public:
    explicit PceConnectingState(
        const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer);

    /**
     * @brief destructor
     * @details destructor
     */
    ~PceConnectingState() override = default;

    /**
     * @brief entry
     * @details when become connecting, call this
     * @return void
     */
    void Entry() override;

    /**
     * @brief exit
     * @details when leave connecting, call this
     * @return void
     */
    void Exit() override;

    /**
     * @brief Dispatch
     * @details when Dispatch PceConnectingState, call this
     * @param msg message
     * @return bool true:success false:failure
     */
    bool Dispatch(const utility::Message &msg) override;

private:
    /**
     * @brief connect to obex
     * @details connect to obex
     * @param sdpMsg pass obex message
     * @return int @c  0 success
     *              @c -1 failure
     */
    int CreateObexClient(const PbapPceHeaderSdpMsg &sdpMsg);

    /**
     * @brief set Obex Client Config
     * @details set Obex Client Config
     * @param obexConfig Client Config
     * @return
     */
    void SetObexClientConfigDetail(PbapPceObexConfig &obexConfig) const;

    /**
     * @brief InitAuth
     * @details Initialize Auth data
     */
    void InitAuth();

    /**
     * @brief Process ObexConnected
     * @details Process ObexConnected
     * @param msg
     */
    void ProcessObexConnected(const utility::Message &msg);

    /**
     * @brief Process SdpFinish
     * @details Process SdpFinish
     * @param msg
     */
    void ProcessSdpFinish(const utility::Message &msg);

    /**
     * @brief Process ObexConnectFailed
     * @details Process ObexConnectFailed
     * @param msg
     */
    void ProcessObexConnectFailed(const utility::Message &msg);

    /**
     * @brief Process ObexTransportFailed
     * @details Process ObexTransportFailed
     * @param msg
     */
    void ProcessObexTransportFailed(const utility::Message &msg);

    /**
     * @brief Process ReqDisconnected
     * @details Process PCE_REQ_DISCONNECTED message
     * @param msg
     */
    void ProcessReqDisconnected(const utility::Message &msg);

    /**
     * @brief Process SdpFailed
     * @details Process PCE_SDP_FAILED message
     * @param msg
     */
    void ProcessSdpFailed(const utility::Message &msg);

    /**
     * @brief Process PasswordInput
     * @details Process PasswordInput
     * @param msg
     */
    void ProcessPasswordInput(const utility::Message &msg) const;

    void OnConnectTimeout();

    PbapPceObexConfig obexConfig_ {};
    std::vector<uint8_t> authDescription_ {};
    std::vector<uint8_t> authNonce_ {};
    uint8_t authUserCharset_ = 0;
    bool authFullAccess_ = false;
    bool authNeedUser_ = false;
    bool authChallenge_ = false;
    std::unique_ptr<ObexHeader> header_ {nullptr};
    std::shared_ptr<utility::Timer> connectTimer_ = nullptr;
};

/**
 * @brief connected statemachine
 * control connected state
 */
class PceConnectedState : public PceBaseState {
public:
    explicit PceConnectedState(
        const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer);

    /**
     * @brief destructor
     * @details destructor
     */
    ~PceConnectedState() override = default;

    /**
     * @brief entry
     * @details when become connected, call this
     * @return void
     */
    void Entry() override;

    /**
     * @brief exit
     * @details when leave connected, call this
     * @return void
     */
    void Exit() override;

    /**
     * @brief Dispatch
     * @details when Dispatch PceConnectedState, call this
     * @param msg message
     * @return bool true:success false:failure
     */
    bool Dispatch(const utility::Message &msg) override;

private:
    bool DispatchActionCompleted(const utility::Message &msg) const;
    void ReqPullPhoneBook(const utility::Message &msg) const;
    void ProcessReqDownload(const utility::Message &msg);
    void ProcessDownloadComplete(const utility::Message &msg);
    void ProcessDownloadFailed(const utility::Message &msg);
};

/**
 * @brief disconnecting statemachine
 * control disconnecting state
 */
class PceDisconnectingState : public PceBaseState {
public:
    explicit PceDisconnectingState(
        const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer);

    ~PceDisconnectingState() override = default;

    void Entry() override;

    void Exit() override;

    bool Dispatch(const utility::Message &msg) override;

private:
    void OnDisconnectTimeout();

    std::shared_ptr<utility::Timer> disconnectTimer_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // PCE_STATE_MACHINE_H
