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

#ifndef PBAP_PCE_OBEX_CLIENT_H
#define PBAP_PCE_OBEX_CLIENT_H

#include <cstdint>
#include <cstring>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>
#include "../obex/obex_body.h"
#include "../obex/obex_client_session.h"
#include "../obex/obex_header.h"
#include "../obex/obex_socket_device.h"
#include "obex_response_handler.h"
#include "pbap_pce_app_params.h"
#include "pbap_pce_datashare_helper.h"
#include "pbap_pce_header_msg.h"
#include "interface_profile.h"
#include "interface_profile_manager.h"
#include "interface_profile_socket.h"
#include "common_util.h"

namespace OHOS {
namespace bluetooth {

class PbapPceServiceImpl;

/// download feature
constexpr uint32_t PBAP_PCE_FEATURES_DOWNLOAD = 1 << 0;
/// browsing feature
constexpr uint32_t PBAP_PCE_FEATURES_BROWSING = 1 << 1;
/// database identifier feature
constexpr uint32_t PBAP_PCE_FEATURES_DATABASE_IDENTIFIER = 1 << 2;
/// folder version counter feature
constexpr uint32_t PBAP_PCE_FEATURES_FOLDER_VERSION_COUNTERS = 1 << 3;
/// vcard selecting feature
constexpr uint32_t PBAP_PCE_FEATURES_VCARD_SELECTING = 1 << 4;
/// enhanced missed calls feature
constexpr uint32_t PBAP_PCE_FEATURES_ENHANCED_MISSED_CALLS = 1 << 5;
/// uci property feature
constexpr uint32_t PBAP_PCE_FEATURES_X_BT_UCI_VCARD_PROPERTY = 1 << 6;
/// uid property feature
constexpr uint32_t PBAP_PCE_FEATURES_X_BT_UID_VCARD_PROPERTY = 1 << 7;
/// contact referencing feature
constexpr uint32_t PBAP_PCE_FEATURES_CONTACT_REFERENCING = 1 << 8;
/// contact image feature
constexpr uint32_t PBAP_PCE_FEATURES_DEFAULT_CONTACT_IMAGE_FORMAT = 1 << 9;

/// uuid length
constexpr uint8_t PBAP_PCE_SERVICE_UUID_LEN = 16;

/// pbap uuid
constexpr uint8_t PBAP_PCE_SERVICE_UUID[PBAP_PCE_SERVICE_UUID_LEN] = {
    0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8, 0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66
};

// PBAP v1.2.3 Sec. 7.1.2
constexpr int PBAP_PCE_SUPPORTED_REPOS_LOCALPB = 1 << 0;
constexpr int PBAP_PCE_SUPPORTED_REPOS_SIMCARD = 1 << 1;
constexpr int PBAP_PCE_SUPPORTED_REPOS_FAV = 1 << 3;

//call log name
extern const std::string PATH_PB;
extern const std::string PATH_MCH;
extern const std::string PATH_ICH;
extern const std::string PATH_OCH;

// Download constants (aligned with design doc §5.2.1.3)
constexpr uint16_t PBAP_PCE_DEFAULT_BATCH_SIZE = 500;   // 每批最多500条联系人
constexpr uint16_t PBAP_PCE_CALLLOG_MAX_COUNT = 100;    // 通话记录每类型最多100条
constexpr uint8_t PBAP_PCE_VCARD_FORMAT_30 = 1;         // vCard 3.0格式
// DB 入库流水线背压：在飞任务数达到上限时阻塞下载线程等待队列空位（超时兜底
// 仍入队），限制内存占用。绝不回退为下载线程同步执行，否则会越过串行队列中已
// 排队的任务，破坏 "contacts各批次 -> contacts末批(isFinish=true) -> mch -> ich
// -> och" 的入库顺序契约（contactsdataability 收到 isFinish=true 会同步清空该
// 设备通话记录表，抢先入库的通话记录批次会被误删）。
constexpr int PBAP_PCE_MAX_INFLIGHT_DB_TASKS = 4;
// 对端上报异常话机簿总数的防御性钳制（PhonebookSize 为 uint16，叠加产品上限）
constexpr uint32_t PBAP_PCE_MAX_TOTAL_SIZE = 60000;
// FinalizeDownload 等待在飞 DB 入库任务 drain 的最大等待时间
constexpr int PBAP_PCE_DB_DRAIN_TIMEOUT_MS = 10000;
// PropertySelector 位掩码：bit0 VERSION、bit1 FN、bit2 N、bit7 TEL。
// 仅请求同步所需的基础属性（版本号 + 姓名 + 电话），不支持的 PSE 忽略该 TLV。
constexpr uint64_t PBAP_PCE_PROPERTY_SELECTOR_MINIMAL = 0x87;

// PBAP v1.2.3 §3.1.5：pb 目录位置 0 恒为本机名片（0.vcf，规格要求 "shall
// always exist"），且 PhonebookSize 计数含名片。本机名片不入车机库，由请求级
// 跳过实现：联系人从 ListStartOffset=1 起拉，MaxListCount 按 size-1 计算。

// vCard file suffix
extern const std::string VCARD_FILE_SUFFIX;

class MemoryBodyObject : public ObexBodyObject {
public:
    MemoryBodyObject() = default;
    ~MemoryBodyObject() override = default;

    void OpenFile(const int fileFd) override {}
    int Close() override { return 0; }
    size_t Read(uint8_t *buf, size_t bufLen) override { return 0; }
    size_t Write(const uint8_t *buf, size_t bufLen) override;
    bool IsReadFinished() override { return true; }

    const std::vector<uint8_t> &GetData() const { return data_; }

private:
    std::vector<uint8_t> data_;
};

class PbapPceObexClient : public ObexResponseHandler,
    public std::enable_shared_from_this<PbapPceObexClient> {
public:
    explicit PbapPceObexClient(const PbapPceObexConfig &config, PbapPceServiceImpl &pceService);
    virtual ~PbapPceObexClient();

    int Connect(std::shared_ptr<PbapPceObexClient> self, bool supported);
    int Disconnect(bool withObexReq = true);
    int Get(const ObexHeader &req, int reqMsgType, std::shared_ptr<ObexBodyObject> writer = nullptr);
    int Abort();

    uint16_t PullPhoneBookSize(const std::string &path);
    int PullCalllog(const std::string &path, uint16_t offset, uint16_t count,
        std::vector<uint8_t> &outBodyData);
    int PullPhoneBookBatch(const std::string &path, uint16_t offset, uint16_t count, bool isFinish,
        std::vector<uint8_t> &outBodyData);
    void DownloadContactsAndCallLogs();

    // --- 任务链下载（每个 FFRT 任务 < 60s，避免 ffrt 拥塞杀进程）---
    // 入口：查询话机簿总数、开启同步会话，拉起第一个联系人批次；后续每批作为独立
    // FFRT 任务投递到 bt_pbap_sync 线程。B2 末批扣留：每批解析后扣留，下一批结果
    // 到达后派发上一批；finish 标志仅在确认终止（completed/no-more-contacts）时
    // 随末批下发，失败/断连路径数据照发但不带 finish。
    void DownloadNextContactBatch(uint32_t offset, uint32_t totalSize, uint32_t batchIdx,
        const std::string &traceId);
    // 联系人批次全部完成后，逐类型（MCH/ICH/OCH）下载通话记录，每个类型一个任务。
    void DownloadNextCalllog(int calllogIdx, const std::string &traceId);
    // 整个下载（联系人+通话记录）结束时调用；复位标志并通知。failed=true 上报
    // PCE_DOWNLOAD_FAILED（observer 收到 DOWNLOAD_ERROR）而非 COMPLETE。
    void FinalizeDownload(bool failed = false);

    // --- 解析与入库拆分（流水线化）---
    // vCard body 解析为联系人列表（CPU 工作，留在下载线程执行）。
    std::vector<PbapPceVCardContact> ParseCallLogsBody(const std::vector<uint8_t> &bodyData);
    std::vector<PbapPceVCardContact> ParseContactsBody(const std::vector<uint8_t> &bodyData);

    // DB 入库（低优先级线程执行，与下载并行）。
    // 空集 + isFinish=true 时仍会下发 commit-only 请求闭合服务端导入会话。
    int InsertContactsToDb(std::vector<PbapPceVCardContact> contacts, bool isFinish,
        const std::string &traceId, uint32_t batchId);
    int InsertCallLogsToDb(std::vector<PbapPceVCardContact> contacts, const std::string &path);

    const std::string &GetRemoteAddress() const;

    void SetBusy(bool isBusy);
    bool IsBusy() const;
    void SetAbort(bool isAbort);

    void OnConnectResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnGetDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnDisConnectResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnAbortResp() override;

    void ObexConnectFailed();

    /// 强制关闭底层 socket，唤醒阻塞的下载线程。
    /// 用于断连流程，绕过 OBEX DISCONNECT 协议，直接 shutdown+close，
    /// 并发送 PCE_OBEX_DISCONNECTED 消息以推动状态机流转。
    void ForceClose();

private:
    // --- B2 末批扣留（hold-last-batch）---
    // 每批解析后扣留、下一批拉取结果到达后再派发上一批；completed/no-more 路径
    // flush 携带 isFinish=true，disconnect/batch-failed 路径 flush 不带 finish
    // （数据照发）。仅 PBAP_SYNC 任务链串行访问，无并发。
    struct HeldDbBatch {
        std::vector<PbapPceVCardContact> insertSet {};
        uint32_t batchId {0};
        bool valid {false};
    };

    struct PullPhoneBookReqParams {
        std::string path;
        uint16_t offset {0};
        uint16_t count {0};
        bool withOffset {false};
        bool withPropertySelector {false};
    };

    std::shared_ptr<ObexSocketDevice> RecvSocketDevice(SocketType socketType);
    void CloseTransport();
    int CreateSocketConnection();
    void HandleConnectSuccess(ObexHeader &reply);
    void NotifyConnectFailed();
    void CloseSocket();
    void BuildConnectHeader(ObexHeader& header, bool supported);
    void BuildGetHeader(ObexHeader &header, const std::string &path, const PbapPceAppParams &appParams);
    void BuildPullPhoneBookRequest(const PullPhoneBookReqParams &params, ObexHeader &reqHeader);
    int ExecuteGetWithBusyGuard(ObexHeader &reqHeader, int reqMsgType,
        std::shared_ptr<ObexBodyObject> writer, ObexHeader &resp);
    static PbapPceCallType DetermineCallType(const std::string &path);
    void NotifyDownloadComplete();
    void NotifyDownloadFailed();
    // 记录联系人阶段结束耗时；failed=true 以失败收尾，否则准备进入通话记录阶段
    void FinishContactsPhase(bool failed, const char *reason);
    // 联系人阶段异常终止公共路径（disconnect/batch-failed）：扣留批次数据照发但
    // 不带 finish，以 failed=true 收尾（跳过通话记录阶段）。
    void AbortContactsDownload(const std::string &traceId, const char *reason);
    // 联系人阶段正常结束公共路径（completed/no-more-contacts）：扣留的末批携带
    // isFinish 派发，闭合服务端导入会话后进入通话记录阶段。
    void CompleteContactsPhase(const std::string &traceId, const char *reason);
    // 复位下载状态标志与计时器，并全程持有电源 BUSY（唯一释放点在 FinalizeDownload）
    void PrepareDownloadState();
    // 拉取并解析一个联系人批次。返回解析条数（0 表示对端没有返回更多数据，属正常
    // 情况，调用方据此提前结束联系人阶段）；负数表示传输失败（内部已记日志，调用方
    // 应终止同步）。
    int PullAndParseContactBatch(uint32_t offset, uint16_t count, bool preliminaryFinish,
        std::vector<PbapPceVCardContact> &contacts);
    // 拉取并解析一个通话记录批次。返回解析条数（0 为正常空记录）；负数表示拉取失败
    // （内部已记日志，调用方应终止同步）。
    int PullAndParseCalllog(const std::string &path, std::vector<PbapPceVCardContact> &contacts);
    // DB 入库投递到低优先级串行线程，保证执行顺序 == 投递顺序。带背压：在飞任务
    // 达到上限时阻塞调用线程等待空位（超时兜底仍入队，绝不同步执行，见上方常量
    // 注释中的顺序契约）。
    void DispatchDbInsert(std::function<void()> insertFunc);

    // 入库过滤（仅 PATH_PB 联系人批次）：解析结果 -> 入库集合。
    // 1) 无电话且无姓名的空卡丢弃（防服务端空 bucket）；
    // 2) 无姓名有电话的联系人保留（入库时不设置 name 字段）。
    // 被过滤卡片仍计入解析数（offset 推进/isFinish 判定不受影响）。
    static std::vector<PbapPceVCardContact> FilterContactsForInsert(
        const std::vector<PbapPceVCardContact> &contacts, size_t &emptyDropped);

    // 计算联系人下载首个请求窗口（请求级名片跳过的入口决策）。
    // PBAP v1.2.3 §3.1.5：pb 目录位置 0 为 0.vcf 本机名片（"shall always exist"），
    // PhonebookSize 计数含名片。本机名片不入车机库：联系人从位置 1 起拉。
    // totalSize<=1（空簿或仅名片槽位）返回 false，调用方跳过联系人下载直接进入
    // 通话记录阶段；否则 firstOffset 置 1 返回 true。
    static bool CalcFirstContactBatch(uint32_t totalSize, uint32_t &firstOffset);

    // 通话记录入库过滤（仅 PATH_ICH 生效）：部分手机 ICH 响应中混入 MISSED 条目
    // （未接来电属"未接听的来电"，PSE 合规行为）。带 X-IRMC-CALL-DATETIME;MISSED
    // 参数的条目按 INCOMING+ACTIVED 入库会显示为已接来电，且与前序 MCH 拉取重复
    // 入库。 MCH 固定在 ICH 之前拉取（DownloadNextCalllog 路径序），过滤不丢记录。
    // 返回过滤后集合，missedDropped 记录丢弃条数（仅计数，不含隐私字段）。
    static std::vector<PbapPceVCardContact> FilterCallLogsForInsert(
        const std::vector<PbapPceVCardContact> &contacts, const std::string &path,
        size_t &missedDropped);

    // 判定单条通话记录是否带 MISSED 标记：扫描 unknownXData 中
    // X-IRMC-CALL-DATETIME 属性的参数（解析层 keep-all 保留完整属性名）。
    // 兼容 ";MISSED"（裸参数，解析器不转大写）与 ";TYPE=MISSED" 两种形式，
    // 大小写不敏感（防御部分 PSE 小写实现）。
    static bool IsMissedCallEntry(const PbapPceVCardContact &contact);

    // 派发扣留的上一批。withFinish=true 时入库请求携带 isFinish（completed/
    // no-more 路径）；false 仅数据照发（disconnect/batch-failed 路径或中间批次
    // 前移）。无扣留批次时为空操作。
    void FlushHeldDbBatch(bool withFinish, const std::string &traceId);

    // 扣留当前批次（B2 末批扣留）：移入入库集合并分配子批 batchId。
    void HoldDbBatch(std::vector<PbapPceVCardContact> &insertSet);

    // 从全局递增计数器为入库集合分配子批基号：基号 += ceil(n/CONTACTS_SUB_BATCH_SIZE)
    // （空集占 1 个号，供 commit-only 请求使用）。跨批次 batchId 严格递增无重复。
    uint32_t AllocDbBatchId(size_t insertCount);

    // 在锁保护下获取 clientSession_（下载线程的线程安全访问点）。
    std::shared_ptr<ObexClientSession> GetClientSessionLocked();

    // 下载耗时统计辅助。
    using SteadyClock = std::chrono::steady_clock;
    using TimePoint = SteadyClock::time_point;
    static int64_t ElapsedMs(const TimePoint &start, const TimePoint &end);

    std::atomic<int> currentReqMsgType_ {0};
    PbapPceObexConfig obexConfig_;
    PbapPceServiceImpl &pceService_;
    std::shared_ptr<ObexClientSession> clientSession_ = nullptr;
    ObexHeader connectHeader_;
    int32_t connectionId_ = -1;
    bool isObexConnected_ = false;
    std::atomic_bool isBusy_ {false};
    std::atomic_bool isAbort_ {false};
    int socketFd_ = -1;
    std::mutex socketFdLock_;
    std::shared_ptr<ObexBodyObject> currentWriter_ = nullptr;
    int lastReqMsgType_ = 0;
    std::atomic_bool isBatchDownloading_ {false};
    std::atomic_bool isDisconnectRequested_ {false};  // 断连请求标志，下载循环检测后主动退出
    std::shared_ptr<PbapPceDataShareHelper> dataShareHelper_ = nullptr;  // shared：供 DB 线程持有
    std::atomic<int> inFlightDbTasks_ {0};  // 流水线 DB 写入的背压计数
    // FinalizeDownload 需等待全部在飞 DB 入库任务 drain 完成
    std::mutex dbDrainMutex_ {};
    std::condition_variable dbDrainCv_ {};
    // B2 扣留批次与 batchId 全局计数（仅 PBAP_SYNC 任务链串行访问）
    HeldDbBatch heldDbBatch_ {};
    uint32_t nextDbBatchId_ {0};

    // 计时：下载各阶段耗时（在 DownloadContactsAndCallLogs 中复位）
    TimePoint downloadStartTime_;       // 整体下载起始
    TimePoint contactsStartTime_;       // 联系人阶段起始
    TimePoint calllogsStartTime_;       // 通话记录阶段起始
    int64_t contactsTotalMs_ = 0;       // 联系人阶段累计耗时
    int64_t calllogsTotalMs_ = 0;       // 通话记录阶段累计耗时
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_OBEX_CLIENT_H
