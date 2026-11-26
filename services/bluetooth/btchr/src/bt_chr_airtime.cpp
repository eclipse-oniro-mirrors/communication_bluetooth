/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "bt_chr_airtime.h"

#include <sys/time.h>

#include <atomic>
#include <chrono>
#include <ctime>
#include <limits>
#include <map>
#include <mutex>
#include <utility>
#include <vector>

#include "bluetooth_log.h"
#include "bt_chr_base.h"
#include "bt_chr_util.h"
#include "securec.h"

namespace OHOS {
namespace bluetooth {

namespace {
std::map<std::string, uint16_t> g_bleConnIntervalMap;
std::map<std::string, std::string> g_gattPkgMap;
std::string g_scanPkgName;
std::map<int32_t, std::string> g_advPkgMap;
std::map<std::string, A2dpAirtimeRecord> g_a2dpMap;
std::map<std::string, std::pair<int32_t, int32_t>> g_a2dpLastCodec;
std::map<std::string, ScoAirtimeRecord> g_scoMap;
std::map<std::string, HidAirtimeRecord> g_hidMap;
std::map<int32_t, SocketAirtimeRecord> g_socketMap;
std::map<std::string, GattAirtimeRecord> g_gattMap;
// 扫描侧只有一条配置记录，不支持多实例并发
ScanAirtimeRecord g_scanRecord;
std::map<int32_t, AdvAirtimeRecord> g_advMap;
std::map<int32_t, DiscAirtimeRecord> g_discMap;
std::mutex g_bleIntervalMutex;
std::mutex g_airtimeMutex;

using SteadyClock = std::chrono::steady_clock;

// 时间换算
constexpr int64_t MS_PER_SEC = 1000;
constexpr int64_t NS_PER_MS = 1000000;

std::atomic_uint32_t g_airtimeSessionId {0};

int64_t NowSteadyMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        SteadyClock::now().time_since_epoch()).count();
}

uint32_t AllocSessionId()
{
    return ++g_airtimeSessionId;
}

int64_t NowBoottimeMs()
{
    struct timespec ts = {};
    if (clock_gettime(CLOCK_BOOTTIME, &ts) != 0) {
        return 0;
    }
    return static_cast<int64_t>(ts.tv_sec) * MS_PER_SEC + ts.tv_nsec / NS_PER_MS;
}

int32_t ClampToInt32(int64_t value)
{
    if (value > std::numeric_limits<int32_t>::max()) {
        return std::numeric_limits<int32_t>::max();
    }
    if (value < std::numeric_limits<int32_t>::min()) {
        return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(value);
}

std::string TsStr(int64_t bootMs)
{
    if (bootMs <= 0) {
        return "";
    }
    struct timeval tv = {};
    if (gettimeofday(&tv, nullptr) != 0) {
        return "";
    }
    int64_t wallMs = bootMs + static_cast<int64_t>(tv.tv_sec) * MS_PER_SEC +
        tv.tv_usec / MS_PER_SEC - NowBoottimeMs();
    if (wallMs < 0) {
        return "";
    }
    time_t sec = static_cast<time_t>(wallMs / MS_PER_SEC);
    struct tm localTime = {};
    if (localtime_r(&sec, &localTime) == nullptr) {
        return "";
    }
    char timeBuf[24];
    if (strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &localTime) == 0) {
        return "";
    }
    char result[32];
    int32_t ms = static_cast<int32_t>(wallMs % MS_PER_SEC);
    if (snprintf_s(result, sizeof(result), sizeof(result) - 1, "%s.%03d", timeBuf, ms) < 0) {
        return "";
    }
    return result;
}

void AppendTs(std::vector<BtChrEventParam> &params, const char *name, const std::string &ts)
{
    if (!ts.empty()) {
        params.emplace_back(name, ts);
    }
}

void AppendInt(std::vector<BtChrEventParam> &params, const char *name, int32_t value)
{
    params.emplace_back(name, value);
}

void AppendPkg(std::vector<BtChrEventParam> &params, const std::string &pkgName)
{
    if (!pkgName.empty()) {
        params.emplace_back("PACKAGE_NAME", pkgName);
    }
}

constexpr int32_t ACL_PKT_TYPE_UNKNOWN = 0;
constexpr int32_t RETRY_Q10_NO_RETRANS = 1024;
constexpr int32_t RETRY_X100_NO_RETRANS = 100;
constexpr int32_t PHY_UNKNOWN = 0;
constexpr int32_t SCHED_SLOTS_UNKNOWN = 0;
constexpr int32_t A2DP_BITRATE_MAX_BPS = 2000000;
constexpr int32_t CODEC_INDEX_UNKNOWN_UINT16 = 0xFFFF;
constexpr int32_t BITS_PER_BYTE = 8;
constexpr int64_t MIN_ELAPSED_MS = 1;
constexpr int32_t SCO_T_TSCO_DEFAULT_TS = 7500;
constexpr int32_t SCO_FAIL_SCENE_PEER_DISCONNECT = 7;
constexpr int32_t HID_FAIL_SCENE_CONN_FAIL = 1;
constexpr int32_t SOCKET_FAIL_SCENE_CONN_FAIL = 1;
constexpr int32_t GATT_FAIL_SCENE_CONN_OPEN = 1;
constexpr int32_t SCAN_FAIL_STAT_ENABLE_FAIL = 1;

int32_t GetBleConnInterval(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_bleIntervalMutex);
    auto it = g_bleConnIntervalMap.find(addr);
    return it == g_bleConnIntervalMap.end() ? 0 : static_cast<int32_t>(it->second);
}

void ClearBleConnInterval(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_bleIntervalMutex);
    g_bleConnIntervalMap.erase(addr);
}

// 调用者需持有 g_airtimeMutex
std::string GetGattPkgName(const std::string &addr)
{
    auto it = g_gattPkgMap.find(addr);
    return it == g_gattPkgMap.end() ? "" : it->second;
}

// 调用者需持有 g_airtimeMutex
std::string GetAdvPkgName(int32_t advertiserId)
{
    auto it = g_advPkgMap.find(advertiserId);
    return it == g_advPkgMap.end() ? "" : it->second;
}
}  // namespace

void BtChr2g4BleConnInterval(const std::string &addr, uint16_t interval)
{
    if (addr.empty() || interval == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_bleIntervalMutex);
    g_bleConnIntervalMap[addr] = interval;
}

// BLE 业务包名缓存
void BtChr2g4BleGattPkgNameSet(const std::string &addr, const std::string &pkgName)
{
    if (addr.empty() || pkgName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    g_gattPkgMap[addr] = pkgName;
}

void BtChr2g4BleScanPkgNameSet(const std::string &pkgName)
{
    if (pkgName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    g_scanPkgName = pkgName;
}

void BtChr2g4BleAdvPkgNameSet(int advertiserId, const std::string &pkgName)
{
    if (pkgName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    g_advPkgMap[advertiserId] = pkgName;
}

// A2DP
// 调用者需持有 g_airtimeMutex
static bool TakeA2dpAirtimeRecord(const std::string &addr, A2dpAirtimeRecord &rec)
{
    auto it = g_a2dpMap.find(addr);
    if (it == g_a2dpMap.end() || it->second.reported) {
        return false;
    }
    rec = it->second;
    rec.reported = true;
    g_a2dpMap.erase(it);
    return true;
}

// 锁外上报，避免持锁跨进程写事件
static void ReportA2dpAirtime(const std::string &addr, const A2dpAirtimeRecord &rec)
{
    HILOGI("[BtChr2g4] a2dp airtime report addr=%{public}s role=%{public}d failScene=%{public}d",
        GET_ENCRYPT_ADDR(addr), rec.role, rec.failScene);
    std::vector<BtChrEventParam> params;
    AppendInt(params, "SESSION_ID", static_cast<int32_t>(rec.sessionId));
    AppendTs(params, "START_TS", TsStr(rec.startTs));
    AppendTs(params, "ACTIVE_TS", TsStr(rec.activeTs));
    AppendTs(params, "STOP_TS", TsStr(rec.stopTs));
    AppendTs(params, "FAIL_TS", TsStr(rec.failTs));
    AppendInt(params, "CODEC_INDEX", rec.codecIndex);
    int32_t bitrateBps = rec.bitrateBps;
    if (bitrateBps < 0 || (bitrateBps > 0 && (rec.codecIndex < 0 || bitrateBps > A2DP_BITRATE_MAX_BPS))) {
        HILOGW("[BtChr2g4] a2dp airtime invalid bitrate %{public}d codec %{public}d, reset to 0",
            bitrateBps, rec.codecIndex);
        bitrateBps = 0;
    }
    AppendInt(params, "BITRATE_BPS", bitrateBps);
    AppendInt(params, "ACL_PKT_TYPE", ACL_PKT_TYPE_UNKNOWN);
    AppendInt(params, "RETRY_FACTOR_Q10", RETRY_Q10_NO_RETRANS);
    AppendInt(params, "STATUS", rec.status);
    AppendInt(params, "FAIL_SCENE", rec.failScene);
    AppendInt(params, "ROLE", rec.role);
    BtChrUeEventWrite(CHR_UE_BT_2G4_A2DP_AIRTIME, params);
}

void BtChr2g4A2dpStart(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    A2dpAirtimeRecord &rec = g_a2dpMap[addr];
    rec = A2dpAirtimeRecord();
    rec.sessionId = AllocSessionId();
    rec.startTs = NowBoottimeMs();
    auto it = g_a2dpLastCodec.find(addr);
    if (it != g_a2dpLastCodec.end()) {
        rec.codecIndex = it->second.first;
        rec.bitrateBps = it->second.second;
    }
}

void BtChr2g4A2dpActive(const std::string &addr, int codecIndex, int bitrateBps, int role)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    A2dpAirtimeRecord &rec = g_a2dpMap[addr];
    if (rec.sessionId == 0) {
        // 未收到 start，主动补一个会话
        rec.sessionId = AllocSessionId();
        rec.startTs = NowBoottimeMs();
        auto it = g_a2dpLastCodec.find(addr);
        if (it != g_a2dpLastCodec.end()) {
            rec.codecIndex = it->second.first;
            rec.bitrateBps = it->second.second;
        }
    }
    // 只在首次 active 时记录，避免刷新测量窗口
    if (rec.activeTs != 0) {
        return;
    }
    rec.activeTs = NowBoottimeMs();
    rec.role = role;
    if (codecIndex >= 0 && codecIndex != CODEC_INDEX_UNKNOWN_UINT16) {
        rec.codecIndex = codecIndex;
    }
    if (bitrateBps > 0) {
        rec.bitrateBps = bitrateBps;
    }
}

void BtChr2g4A2dpSinkCodec(const std::string &addr, int codecIndex, int bitrateBps)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    if (codecIndex >= 0 && codecIndex != CODEC_INDEX_UNKNOWN_UINT16) {
        g_a2dpLastCodec[addr] = {codecIndex, bitrateBps};
    }
    A2dpAirtimeRecord &rec = g_a2dpMap[addr];
    if (rec.sessionId == 0) {
        rec.sessionId = AllocSessionId();
        rec.startTs = NowBoottimeMs();
    }
    if (codecIndex >= 0 && codecIndex != CODEC_INDEX_UNKNOWN_UINT16) {
        rec.codecIndex = codecIndex;
    }
    if (bitrateBps > 0) {
        rec.bitrateBps = bitrateBps;
    }
}

void BtChr2g4A2dpStop(const std::string &addr)
{
    A2dpAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeA2dpAirtimeRecord(addr, rec)) {
            return;
        }
    }
    rec.stopTs = NowBoottimeMs();
    ReportA2dpAirtime(addr, rec);
}

void BtChr2g4A2dpFail(const std::string &addr, int status, int failScene)
{
    A2dpAirtimeRecord rec;
    bool hasRecord = false;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        auto it = g_a2dpMap.find(addr);
        if (it != g_a2dpMap.end()) {
            if (it->second.reported) {
                return;
            }
            rec = it->second;
            rec.reported = true;
            g_a2dpMap.erase(it);
            hasRecord = true;
        }
    }
    if (!hasRecord) {
        // 未收到 start 就失败，补一条失败会话
        rec.sessionId = AllocSessionId();
    }
    rec.failTs = NowBoottimeMs();
    rec.status = status;
    rec.failScene = failScene;
    ReportA2dpAirtime(addr, rec);
}

// SCO
// 调用者需持有 g_airtimeMutex
static bool TakeScoAirtimeRecord(const std::string &addr, ScoAirtimeRecord &rec)
{
    auto it = g_scoMap.find(addr);
    if (it == g_scoMap.end() || it->second.reported) {
        return false;
    }
    rec = it->second;
    rec.reported = true;
    g_scoMap.erase(it);
    return true;
}

// 锁外上报，避免持锁跨进程写事件
static void ReportScoAirtime(const std::string &addr, const ScoAirtimeRecord &rec)
{
    HILOGI("[BtChr2g4] sco airtime report addr=%{public}s failScene=%{public}d",
        GET_ENCRYPT_ADDR(addr), rec.failScene);
    std::vector<BtChrEventParam> params;
    AppendInt(params, "SESSION_ID", static_cast<int32_t>(rec.sessionId));
    AppendTs(params, "START_TS", TsStr(rec.startTs));
    AppendTs(params, "ACTIVE_TS", TsStr(rec.activeTs));
    AppendTs(params, "STOP_TS", TsStr(rec.stopTs));
    AppendTs(params, "FAIL_TS", TsStr(rec.failTs));
    AppendInt(params, "SCO_CODEC", rec.scoCodec);
    AppendInt(params, "PKT_TYPE", rec.pktType);
    AppendInt(params, "T_SCO", rec.tScoUs > 0 ? rec.tScoUs : SCO_T_TSCO_DEFAULT_TS);
    AppendInt(params, "RETRY_FACTOR_Q10", RETRY_Q10_NO_RETRANS);
    AppendInt(params, "HCI_STATUS", rec.hciStatus);
    AppendInt(params, "FAIL_SCENE", rec.failScene);
    BtChrUeEventWrite(CHR_UE_BT_2G4_SCO_AIRTIME, params);
}

void BtChr2g4ScoStart(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    ScoAirtimeRecord &rec = g_scoMap[addr];
    rec = ScoAirtimeRecord();
    rec.sessionId = AllocSessionId();
    rec.startTs = NowBoottimeMs();
}

void BtChr2g4ScoActive(const std::string &addr, int codec, int pktType, int tScoUs)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    ScoAirtimeRecord &rec = g_scoMap[addr];
    if (rec.sessionId == 0) {
        rec.sessionId = AllocSessionId();
        rec.startTs = NowBoottimeMs();
    }
    if (rec.activeTs == 0) {
        rec.activeTs = NowBoottimeMs();
    }
    if (codec > 0) {
        rec.scoCodec = codec;
    }
    if (pktType > 0) {
        rec.pktType = pktType;
    }
    if (tScoUs > 0) {
        rec.tScoUs = tScoUs;
    }
}

void BtChr2g4ScoStop(const std::string &addr, int hciStatus, int failScene)
{
    ScoAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeScoAirtimeRecord(addr, rec)) {
            return;
        }
    }
    rec.stopTs = NowBoottimeMs();
    rec.hciStatus = hciStatus;
    if (failScene > 0) {
        // 调用方明确失败归因时优先使用
        rec.failTs = rec.stopTs;
        rec.failScene = failScene;
    } else if (rec.activeTs == 0) {
        // 从未激活且调用方未给归因，按对端断开失败上报
        rec.failTs = rec.stopTs;
        rec.failScene = SCO_FAIL_SCENE_PEER_DISCONNECT;
    }
    ReportScoAirtime(addr, rec);
}

void BtChr2g4ScoFail(const std::string &addr, int hciStatus, int failScene)
{
    ScoAirtimeRecord rec;
    bool hasRecord = false;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        auto it = g_scoMap.find(addr);
        if (it != g_scoMap.end()) {
            if (it->second.reported) {
                return;
            }
            rec = it->second;
            rec.reported = true;
            g_scoMap.erase(it);
            hasRecord = true;
        }
    }
    if (!hasRecord) {
        // 未收到 start 就失败，补一条失败会话
        rec.sessionId = AllocSessionId();
    }
    rec.failTs = NowBoottimeMs();
    rec.hciStatus = hciStatus;
    rec.failScene = failScene;
    ReportScoAirtime(addr, rec);
}

// HID (BR + BLE HOGP)
namespace {
void AccumulateHidReport(HidAirtimeRecord &rec, int len)
{
    rec.reportCount++;
    rec.reportBytes += len;
    rec.notifCount++;
}

// REPORT_RATE_HZ 与 REPORT_SIZE_B 均为活跃期内的会话均值，NOTIF_COUNT 为会话总次数
int32_t HidReportRate(const HidAirtimeRecord &rec)
{
    if (rec.activeSteadyMs <= 0 || rec.reportCount == 0) {
        return 0;
    }
    int64_t elapsedMs = NowSteadyMs() - rec.activeSteadyMs;
    if (elapsedMs < MIN_ELAPSED_MS) {
        elapsedMs = MIN_ELAPSED_MS;
    }
    return ClampToInt32(static_cast<int64_t>(rec.reportCount) * MS_PER_SEC / elapsedMs);
}

int32_t HidReportSize(const HidAirtimeRecord &rec)
{
    return rec.reportCount > 0 ? ClampToInt32(rec.reportBytes / rec.reportCount) : 0;
}

// 调用者需持有 g_airtimeMutex
bool TakeHidAirtimeRecord(const std::string &addr, bool isLe, HidAirtimeRecord &rec)
{
    auto it = g_hidMap.find(addr);
    if (it == g_hidMap.end() || it->second.isLe != isLe || it->second.reported) {
        return false;
    }
    rec = it->second;
    rec.reported = true;
    g_hidMap.erase(it);
    return true;
}
}

// 锁外上报，避免持锁跨进程写事件
static void ReportHidAirtime(const std::string &event, const std::string &addr, const HidAirtimeRecord &rec)
{
    HILOGI("[BtChr2g4] hid airtime report event=%{public}s addr=%{public}s le=%{public}d failScene=%{public}d",
        event.c_str(), GET_ENCRYPT_ADDR(addr), static_cast<int>(rec.isLe), rec.failScene);
    std::vector<BtChrEventParam> params;
    AppendInt(params, "SESSION_ID", static_cast<int32_t>(rec.sessionId));
    AppendTs(params, "START_TS", TsStr(rec.startTs));
    AppendTs(params, "ACTIVE_TS", TsStr(rec.activeTs));
    AppendTs(params, "STOP_TS", TsStr(rec.stopTs));
    AppendTs(params, "FAIL_TS", TsStr(rec.failTs));
    if (rec.isLe) {
        int32_t interval = GetBleConnInterval(addr);
        if (interval <= 0) {
            interval = rec.connInterval;
        }
        AppendInt(params, "CONN_INTERVAL", interval);
        AppendInt(params, "REPORT_RATE_HZ", HidReportRate(rec));
        AppendInt(params, "REPORT_SIZE_B", HidReportSize(rec));
        AppendInt(params, "NOTIF_COUNT", rec.notifCount);
        AppendInt(params, "PHY", PHY_UNKNOWN);
    } else {
        AppendInt(params, "REPORT_RATE_HZ", HidReportRate(rec));
        AppendInt(params, "REPORT_SIZE_B", HidReportSize(rec));
        AppendInt(params, "ACL_PKT_TYPE", ACL_PKT_TYPE_UNKNOWN);
        AppendInt(params, "BR_SCHED_SLOTS", SCHED_SLOTS_UNKNOWN);
    }
    AppendInt(params, "STATUS", rec.status);
    AppendInt(params, "FAIL_SCENE", rec.failScene);
    BtChrUeEventWrite(event, params);
    if (rec.isLe) {
        ClearBleConnInterval(addr);
    }
}

void BtChr2g4BrHidStart(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    HidAirtimeRecord &rec = g_hidMap[addr];
    rec = HidAirtimeRecord();
    rec.sessionId = AllocSessionId();
    rec.startTs = NowBoottimeMs();
    rec.isLe = false;
}

void BtChr2g4BrHidActive(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    HidAirtimeRecord &rec = g_hidMap[addr];
    if (rec.sessionId == 0) {
        rec.sessionId = AllocSessionId();
        rec.startTs = NowBoottimeMs();
        rec.isLe = false;
    }
    if (rec.activeTs == 0) {
        rec.activeTs = NowBoottimeMs();
        rec.activeSteadyMs = NowSteadyMs();
    }
}

void BtChr2g4BrHidReportData(const std::string &addr, int len)
{
    if (len <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    auto it = g_hidMap.find(addr);
    if (it != g_hidMap.end() && !it->second.isLe) {
        AccumulateHidReport(it->second, len);
    }
}

void BtChr2g4BrHidStop(const std::string &addr, int status)
{
    HidAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeHidAirtimeRecord(addr, false, rec)) {
            return;
        }
    }
    rec.stopTs = NowBoottimeMs();
    rec.status = status;
    if (rec.activeTs == 0) {
        rec.failTs = rec.stopTs;
        rec.failScene = HID_FAIL_SCENE_CONN_FAIL;
    }
    ReportHidAirtime(CHR_UE_BT_2G4_BR_HID_AIRTIME, addr, rec);
}

void BtChr2g4BrHidFail(const std::string &addr, int status)
{
    HidAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeHidAirtimeRecord(addr, false, rec)) {
            return;
        }
    }
    rec.failTs = NowBoottimeMs();
    rec.status = status;
    rec.failScene = HID_FAIL_SCENE_CONN_FAIL;
    ReportHidAirtime(CHR_UE_BT_2G4_BR_HID_AIRTIME, addr, rec);
}

void BtChr2g4BleHogpStart(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    HidAirtimeRecord &rec = g_hidMap[addr];
    rec = HidAirtimeRecord();
    rec.sessionId = AllocSessionId();
    rec.startTs = NowBoottimeMs();
    rec.isLe = true;
}

void BtChr2g4BleHogpActive(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    HidAirtimeRecord &rec = g_hidMap[addr];
    if (rec.sessionId == 0) {
        rec.sessionId = AllocSessionId();
        rec.startTs = NowBoottimeMs();
        rec.isLe = true;
    }
    rec.isLe = true;
    if (rec.activeTs == 0) {
        rec.activeTs = NowBoottimeMs();
        rec.activeSteadyMs = NowSteadyMs();
    }
}

void BtChr2g4BleHogpReportData(const std::string &addr, int len)
{
    if (len <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    auto it = g_hidMap.find(addr);
    if (it != g_hidMap.end() && it->second.isLe) {
        AccumulateHidReport(it->second, len);
    }
}

void BtChr2g4BleHogpStop(const std::string &addr, int status)
{
    HidAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeHidAirtimeRecord(addr, true, rec)) {
            return;
        }
    }
    rec.stopTs = NowBoottimeMs();
    rec.status = status;
    if (rec.activeTs == 0) {
        rec.failTs = rec.stopTs;
        rec.failScene = HID_FAIL_SCENE_CONN_FAIL;
    }
    ReportHidAirtime(CHR_UE_BT_2G4_BLE_HOGP_AIRTIME, addr, rec);
}

void BtChr2g4BleHogpFail(const std::string &addr, int status)
{
    HidAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeHidAirtimeRecord(addr, true, rec)) {
            return;
        }
    }
    rec.failTs = NowBoottimeMs();
    rec.status = status;
    rec.failScene = HID_FAIL_SCENE_CONN_FAIL;
    ReportHidAirtime(CHR_UE_BT_2G4_BLE_HOGP_AIRTIME, addr, rec);
}

// BR socket
static void ReportSocketAirtime(const SocketAirtimeRecord &rec)
{
    int32_t payloadBps = 0;
    if (rec.activeSteadyMs > 0 && (rec.txBytes + rec.rxBytes) > 0) {
        int64_t elapsedMs = NowSteadyMs() - rec.activeSteadyMs;
        if (elapsedMs <= 0) {
            elapsedMs = MIN_ELAPSED_MS;
        }
        payloadBps = ClampToInt32((rec.rxBytes + rec.txBytes) * BITS_PER_BYTE * MS_PER_SEC / elapsedMs);
    }
    HILOGI("[BtChr2g4] socket airtime report addr=%{public}s ch=%{public}d svc=%{public}d failScene=%{public}d",
        GET_ENCRYPT_ADDR(rec.addr), rec.channel, rec.serviceType, rec.failScene);
    std::vector<BtChrEventParam> params;
    AppendInt(params, "SESSION_ID", static_cast<int32_t>(rec.sessionId));
    AppendTs(params, "START_TS", TsStr(rec.startTs));
    AppendTs(params, "ACTIVE_TS", TsStr(rec.activeTs));
    AppendTs(params, "STOP_TS", TsStr(rec.stopTs));
    AppendTs(params, "FAIL_TS", TsStr(rec.failTs));
    AppendInt(params, "CHANNEL", rec.channel);
    AppendInt(params, "PAYLOAD_BPS", payloadBps);
    AppendInt(params, "ACL_PKT_TYPE", ACL_PKT_TYPE_UNKNOWN);
    AppendInt(params, "BR_SCHED_SLOTS", SCHED_SLOTS_UNKNOWN);
    AppendInt(params, "RETRY_FACTOR_Q10", RETRY_Q10_NO_RETRANS);
    AppendInt(params, "STATUS", rec.status);
    AppendInt(params, "FAIL_SCENE", rec.failScene);
    AppendInt(params, "SERVICE_TYPE", rec.serviceType);
    BtChrUeEventWrite(CHR_UE_BT_2G4_BR_SOCKET_AIRTIME, params);
}

void BtChr2g4BrSocketStart(int32_t handle, const std::string &addr, int channel, int serviceType)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    SocketAirtimeRecord &rec = g_socketMap[handle];
    rec = SocketAirtimeRecord();
    rec.addr = addr;
    rec.sessionId = AllocSessionId();
    rec.startTs = NowBoottimeMs();
    rec.channel = channel;
    rec.serviceType = serviceType;
}

void BtChr2g4BrSocketActive(int32_t handle, const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    auto it = g_socketMap.find(handle);
    if (it == g_socketMap.end()) {
        it = g_socketMap.emplace(handle, SocketAirtimeRecord()).first;
        it->second.addr = addr;
        it->second.sessionId = AllocSessionId();
        it->second.startTs = NowBoottimeMs();
    }
    if (it->second.activeTs == 0) {
        it->second.activeTs = NowBoottimeMs();
        it->second.activeSteadyMs = NowSteadyMs();
    }
}

void BtChr2g4BrSocketData(int32_t handle, int len)
{
    if (len <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    auto it = g_socketMap.find(handle);
    if (it != g_socketMap.end()) {
        it->second.txBytes += len;
        return;
    }
}

void BtChr2g4BrSocketStop(int32_t handle, int status)
{
    SocketAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        auto it = g_socketMap.find(handle);
        if (it == g_socketMap.end()) {
            return;
        }
        rec = it->second;
        g_socketMap.erase(it);
    }
    rec.stopTs = NowBoottimeMs();
    rec.status = status;
    if (rec.activeTs == 0) {
        // 未激活即停止，按连接失败上报
        rec.failTs = rec.stopTs;
        rec.failScene = SOCKET_FAIL_SCENE_CONN_FAIL;
    }
    ReportSocketAirtime(rec);
}

void BtChr2g4BrSocketFail(const std::string &addr, int status, int failScene)
{
    SocketAirtimeRecord rec;
    bool hasRecord = false;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        // 失败入口按地址命中既有会话，避免同一次失败被 Stop/Fail 重复上报
        for (auto it = g_socketMap.begin(); it != g_socketMap.end(); ++it) {
            if (it->second.addr != addr) {
                continue;
            }
            rec = it->second;
            g_socketMap.erase(it);
            hasRecord = true;
            break;
        }
    }
    if (!hasRecord) {
        // 失败可能发生在会话建立前，直接构造一条失败记录
        rec.addr = addr;
        rec.sessionId = AllocSessionId();
    }
    rec.failTs = NowBoottimeMs();
    rec.status = status;
    rec.failScene = failScene;
    ReportSocketAirtime(rec);
}

// BLE GATT
// 调用者需持有 g_airtimeMutex
static bool TakeGattAirtimeRecord(const std::string &addr, GattAirtimeRecord &rec)
{
    auto it = g_gattMap.find(addr);
    if (it == g_gattMap.end() || it->second.reported) {
        return false;
    }
    rec = it->second;
    rec.reported = true;
    g_gattMap.erase(it);
    if (rec.pkgName.empty()) {
        rec.pkgName = GetGattPkgName(addr);
    }
    g_gattPkgMap.erase(addr);
    return true;
}

// 锁外上报，避免持锁跨进程写事件
static void ReportGattAirtime(const std::string &addr, const GattAirtimeRecord &rec)
{
    int32_t payloadBps = 0;
    if (rec.activeSteadyMs > 0 && (rec.txBytes + rec.rxBytes) > 0) {
        int64_t elapsedMs = NowSteadyMs() - rec.activeSteadyMs;
        if (elapsedMs <= 0) {
            elapsedMs = MIN_ELAPSED_MS;
        }
        payloadBps = ClampToInt32((rec.txBytes + rec.rxBytes) * BITS_PER_BYTE * MS_PER_SEC / elapsedMs);
    }
    HILOGI("[BtChr2g4] gatt airtime report addr=%{public}s payloadBps=%{public}d failScene=%{public}d",
        GET_ENCRYPT_ADDR(addr), payloadBps, rec.failScene);
    std::vector<BtChrEventParam> params;
    AppendInt(params, "SESSION_ID", static_cast<int32_t>(rec.sessionId));
    AppendTs(params, "START_TS", TsStr(rec.startTs));
    AppendTs(params, "ACTIVE_TS", TsStr(rec.activeTs));
    AppendTs(params, "STOP_TS", TsStr(rec.stopTs));
    AppendTs(params, "FAIL_TS", TsStr(rec.failTs));
    int32_t interval = GetBleConnInterval(addr);
    if (interval <= 0) {
        interval = rec.connInterval;
    }
    AppendInt(params, "CONN_INTERVAL", interval);
    AppendInt(params, "PHY", PHY_UNKNOWN);
    AppendInt(params, "TX_BYTES", ClampToInt32(rec.txBytes));
    AppendInt(params, "RX_BYTES", ClampToInt32(rec.rxBytes));
    AppendInt(params, "PAYLOAD_BPS", payloadBps);
    AppendInt(params, "RETRY_FACTOR_X100", RETRY_X100_NO_RETRANS);
    AppendInt(params, "STATUS", rec.status);
    AppendPkg(params, rec.pkgName);
    AppendInt(params, "FAIL_SCENE", rec.failScene);
    BtChrUeEventWrite(CHR_UE_BT_2G4_BLE_GATT_AIRTIME, params);
}

void BtChr2g4BleGattStart(const std::string &addr, const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    GattAirtimeRecord &rec = g_gattMap[addr];
    rec = GattAirtimeRecord();
    rec.sessionId = AllocSessionId();
    rec.startTs = NowBoottimeMs();
    if (!pkgName.empty()) {
        rec.pkgName = pkgName;
    }
}

void BtChr2g4BleGattActive(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    GattAirtimeRecord &rec = g_gattMap[addr];
    if (rec.sessionId == 0) {
        rec.sessionId = AllocSessionId();
        rec.startTs = NowBoottimeMs();
    }
    if (rec.activeTs == 0) {
        rec.activeTs = NowBoottimeMs();
        rec.activeSteadyMs = NowSteadyMs();
    }
}

void BtChr2g4BleGattTx(const std::string &addr, int len)
{
    if (len <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    auto it = g_gattMap.find(addr);
    if (it != g_gattMap.end()) {
        it->second.txBytes += len;
        return;
    }
}

void BtChr2g4BleGattRx(const std::string &addr, int len)
{
    if (len <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    auto it = g_gattMap.find(addr);
    if (it != g_gattMap.end()) {
        it->second.rxBytes += len;
        return;
    }
}

void BtChr2g4BleGattStop(const std::string &addr, int status)
{
    GattAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeGattAirtimeRecord(addr, rec)) {
            return;
        }
    }
    rec.stopTs = NowBoottimeMs();
    rec.status = status;
    if (rec.activeTs == 0) {
        // 未激活即停止，按连接建立失败上报
        rec.failTs = rec.stopTs;
        rec.failScene = GATT_FAIL_SCENE_CONN_OPEN;
    }
    ReportGattAirtime(addr, rec);
    ClearBleConnInterval(addr);
}

void BtChr2g4BleGattFail(const std::string &addr, int status, int failScene)
{
    GattAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        auto it = g_gattMap.find(addr);
        if (it != g_gattMap.end()) {
            if (it->second.reported) {
                return;
            }
            rec = it->second;
            rec.reported = true;
            g_gattMap.erase(it);
        } else {
            rec.sessionId = AllocSessionId();
        }
        if (rec.pkgName.empty()) {
            rec.pkgName = GetGattPkgName(addr);
        }
        g_gattPkgMap.erase(addr);
    }
    rec.failTs = NowBoottimeMs();
    rec.status = status;
    rec.failScene = failScene;
    ReportGattAirtime(addr, rec);
    ClearBleConnInterval(addr);
}

namespace {
// BLE scan
// 调用者需持有 g_airtimeMutex
static bool TakeScanAirtimeRecord(ScanAirtimeRecord &rec)
{
    if (g_scanRecord.reported || g_scanRecord.startTs == 0) {
        // 早退同样复位，避免上一轮的 STOP/ACTIVE/FAIL 时刻残留到下一轮
        g_scanRecord = ScanAirtimeRecord();
        g_scanPkgName.clear();
        return false;
    }
    rec = g_scanRecord;
    g_scanRecord = ScanAirtimeRecord();
    g_scanPkgName.clear();
    return true;
}

// 锁外上报，避免持锁跨进程写事件
static void ReportScanAirtime(const ScanAirtimeRecord &rec)
{
    HILOGI("[BtChr2g4] ble scan airtime report window=%{public}d interv=%{public}d failStat=%{public}d",
        rec.scanWindowSlot, rec.scanIntervalSlot, rec.failStat);
    std::vector<BtChrEventParam> params;
    AppendTs(params, "START_TS", TsStr(rec.startTs));
    AppendTs(params, "ACTIVE_TS", TsStr(rec.activeTs));
    AppendTs(params, "STOP_TS", TsStr(rec.stopTs));
    AppendTs(params, "FAIL_TS", TsStr(rec.failTs));
    AppendInt(params, "SCAN_WINDOW", rec.scanWindowSlot);
    AppendInt(params, "SCAN_INTERVAL", rec.scanIntervalSlot);
    AppendInt(params, "SCAN_TYPE", rec.scanType);
    AppendInt(params, "PHY", PHY_UNKNOWN);
    if (rec.failStat > 0) {
        AppendInt(params, "FAIL_STAT", rec.failStat);
    }
    AppendPkg(params, rec.pkgName);
    BtChrUeEventWrite(CHR_UE_BT_2G4_BLE_SCAN_AIRTIME, params);
}
}

void BtChr2g4BleScanParams(int scanWindowSlot, int scanIntervalSlot, int scanType, const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    if (g_scanRecord.startTs == 0) {
        g_scanRecord.startTs = NowBoottimeMs();
        g_scanRecord.reported = false;
    }
    g_scanRecord.scanWindowSlot = scanWindowSlot;
    g_scanRecord.scanIntervalSlot = scanIntervalSlot;
    g_scanRecord.scanType = scanType;
    if (!pkgName.empty()) {
        g_scanRecord.pkgName = pkgName;
    } else if (g_scanRecord.pkgName.empty()) {
        g_scanRecord.pkgName = g_scanPkgName;
    }
}

void BtChr2g4BleScanEnable(bool success, const std::string &pkgName)
{
    ScanAirtimeRecord rec;
    bool needReport = false;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (g_scanRecord.startTs == 0) {
            g_scanRecord.startTs = NowBoottimeMs();
            g_scanRecord.reported = false;
        }
        if (success) {
            // ACTIVE_TS 记录首次使能成功时间
            if (g_scanRecord.activeTs == 0) {
                g_scanRecord.activeTs = NowBoottimeMs();
            }
            if (g_scanRecord.pkgName.empty()) {
                g_scanRecord.pkgName = !pkgName.empty() ? pkgName : g_scanPkgName;
            }
        } else {
            g_scanRecord.failTs = NowBoottimeMs();
            g_scanRecord.failStat = SCAN_FAIL_STAT_ENABLE_FAIL;
            needReport = TakeScanAirtimeRecord(rec);
        }
    }
    if (needReport) {
        ReportScanAirtime(rec);
    }
}

void BtChr2g4BleScanDisable()
{
    ScanAirtimeRecord rec;
    bool needReport = false;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        g_scanRecord.stopTs = NowBoottimeMs();
        needReport = TakeScanAirtimeRecord(rec);
    }
    if (needReport) {
        ReportScanAirtime(rec);
    }
}

void BtChr2g4BleScanFlush()
{
    ScanAirtimeRecord rec;
    bool needReport = false;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        needReport = TakeScanAirtimeRecord(rec);
    }
    if (needReport) {
        ReportScanAirtime(rec);
    }
}

// BLE advertise
// 调用者需持有 g_airtimeMutex
static bool TakeAdvAirtimeRecord(int32_t advertiserId, AdvAirtimeRecord &rec)
{
    auto it = g_advMap.find(advertiserId);
    if (it == g_advMap.end() || it->second.reported) {
        return false;
    }
    rec = it->second;
    rec.reported = true;
    g_advMap.erase(it);
    g_advPkgMap.erase(advertiserId);
    return true;
}

// 锁外上报，避免持锁跨进程写事件
static void ReportAdvAirtime(int32_t advertiserId, const AdvAirtimeRecord &rec)
{
    HILOGI("[BtChr2g4] ble adv airtime report advId=%{public}d failScene=%{public}d",
        advertiserId, rec.failScene);
    std::vector<BtChrEventParam> params;
    AppendInt(params, "SESSION_ID", static_cast<int32_t>(rec.sessionId));
    AppendTs(params, "START_TS", TsStr(rec.startTs));
    AppendTs(params, "ACTIVE_TS", TsStr(rec.activeTs));
    AppendTs(params, "STOP_TS", TsStr(rec.stopTs));
    AppendTs(params, "FAIL_TS", TsStr(rec.failTs));
    AppendInt(params, "ADV_INTERVAL", rec.advIntervalSlot);
    AppendInt(params, "ADV_TYPE", rec.advType);
    AppendInt(params, "PAYLOAD_LEN_B", rec.payloadLenB);
    AppendPkg(params, rec.pkgName);
    AppendInt(params, "PHY", PHY_UNKNOWN);
    BtChrUeEventWrite(CHR_UE_BT_2G4_BLE_ADV_AIRTIME, params);
}

void BtChr2g4BleAdvStart(int advertiserId, int advIntervalSlot, int advType,
    int payloadLenB, const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    AdvAirtimeRecord &rec = g_advMap[advertiserId];
    rec = AdvAirtimeRecord();
    rec.sessionId = AllocSessionId();
    rec.startTs = NowBoottimeMs();
    rec.advIntervalSlot = advIntervalSlot;
    rec.advType = advType;
    rec.payloadLenB = payloadLenB;
    if (!pkgName.empty()) {
        rec.pkgName = pkgName;
    } else {
        rec.pkgName = GetAdvPkgName(advertiserId);
    }
}

void BtChr2g4BleAdvActive(int advertiserId)
{
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    AdvAirtimeRecord &rec = g_advMap[advertiserId];
    if (rec.sessionId == 0) {
        rec.sessionId = AllocSessionId();
        rec.startTs = NowBoottimeMs();
        rec.pkgName = GetAdvPkgName(advertiserId);
    }
    if (rec.activeTs == 0) {
        rec.activeTs = NowBoottimeMs();
    }
}

void BtChr2g4BleAdvStop(int advertiserId)
{
    AdvAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeAdvAirtimeRecord(advertiserId, rec)) {
            return;
        }
    }
    rec.stopTs = NowBoottimeMs();
    ReportAdvAirtime(advertiserId, rec);
}

void BtChr2g4BleAdvFail(int advertiserId, int failScene)
{
    AdvAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeAdvAirtimeRecord(advertiserId, rec)) {
            return;
        }
    }
    rec.failTs = NowBoottimeMs();
    rec.failScene = failScene;
    ReportAdvAirtime(advertiserId, rec);
}

namespace {
constexpr int32_t DISC_TYPE_INQUIRY = 1;
constexpr int32_t DISC_TYPE_PAGE = 2;
constexpr int32_t DISC_TYPE_INQUIRY_SCAN = 3;
constexpr int32_t DISC_TYPE_PAGE_SCAN = 4;

// 调用者需持有 g_airtimeMutex
bool TakeDiscAirtimeRecord(int procedureType, DiscAirtimeRecord &rec)
{
    auto it = g_discMap.find(procedureType);
    if (it == g_discMap.end() || it->second.reported || it->second.startTs == 0) {
        return false;
    }
    rec = it->second;
    rec.reported = true;
    g_discMap.erase(it);
    return true;
}

// 锁外上报，避免持锁跨进程写事件
void ReportDiscAirtime(int procedureType, const DiscAirtimeRecord &rec)
{
    HILOGI("[BtChr2g4] classic discovery airtime report type=%{public}d failReason=%{public}d",
        procedureType, rec.failReason);
    std::vector<BtChrEventParam> params;
    AppendInt(params, "PROCEDURE_TYPE", procedureType);
    AppendTs(params, "START_TS", TsStr(rec.startTs));
    AppendTs(params, "ACTIVE_TS", TsStr(rec.activeTs));
    AppendTs(params, "FAIL_TS", TsStr(rec.failTs));
    AppendTs(params, "STOP_TS", TsStr(rec.stopTs));
    AppendInt(params, "SCAN_WINDOW_SLOTS", rec.scanWindowSlots);
    AppendInt(params, "SCAN_INTERVAL_SLOTS", rec.scanIntervalSlots);
    if (rec.failReason > 0) {
        AppendInt(params, "FAIL_REASON", rec.failReason);
    }
    BtChrUeEventWrite(CHR_UE_BT_2G4_CLASSIC_DISCOVERY_AIRTIME, params);
}
}

void BtChr2g4DiscStart(int procedureType, int scanWindowSlots, int scanIntervalSlots)
{
    switch (procedureType) {
        case DISC_TYPE_INQUIRY:
        case DISC_TYPE_PAGE:
        case DISC_TYPE_INQUIRY_SCAN:
        case DISC_TYPE_PAGE_SCAN:
            break;
        default:
            return;
    }
    DiscAirtimeRecord pending;
    bool needReport = false;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        auto it = g_discMap.find(procedureType);
        if (it != g_discMap.end()) {
            DiscAirtimeRecord &old = it->second;
            if (old.stopTs != 0 || old.failTs != 0) {
                if (old.stopTs == 0) {
                    old.stopTs = NowBoottimeMs();
                }
            } else if (old.scanWindowSlots == scanWindowSlots &&
                old.scanIntervalSlots == scanIntervalSlots) {
                return;
            }
            if (!old.reported && old.startTs != 0) {
                pending = old;
                pending.reported = true;
                needReport = true;
            }
            g_discMap.erase(it);
        }

        DiscAirtimeRecord &rec = g_discMap[procedureType];
        rec = DiscAirtimeRecord();
        rec.startTs = NowBoottimeMs();
        rec.scanWindowSlots = scanWindowSlots;
        rec.scanIntervalSlots = scanIntervalSlots;
    }
    if (needReport) {
        ReportDiscAirtime(procedureType, pending);
    }
}

void BtChr2g4DiscActive(int procedureType)
{
    if (procedureType < DISC_TYPE_INQUIRY || procedureType > DISC_TYPE_PAGE_SCAN) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_airtimeMutex);
    DiscAirtimeRecord &rec = g_discMap[procedureType];
    if (rec.startTs == 0) {
        rec.startTs = NowBoottimeMs();
    }
    if (rec.activeTs == 0) {
        rec.activeTs = NowBoottimeMs();
    }
}

void BtChr2g4DiscStop(int procedureType, int failReason)
{
    DiscAirtimeRecord rec;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        if (!TakeDiscAirtimeRecord(procedureType, rec)) {
            return;
        }
    }
    if (failReason > 0) {
        rec.failTs = NowBoottimeMs();
        rec.failReason = failReason;
    } else {
        rec.stopTs = NowBoottimeMs();
    }
    ReportDiscAirtime(procedureType, rec);
}

void BtChr2g4DiscFlushAll()
{
    std::vector<std::pair<int32_t, DiscAirtimeRecord>> pending;
    {
        std::lock_guard<std::mutex> lock(g_airtimeMutex);
        pending.reserve(g_discMap.size());
        for (auto it = g_discMap.begin(); it != g_discMap.end(); ++it) {
            if (it->second.reported || it->second.startTs == 0) {
                continue;
            }
            DiscAirtimeRecord rec = it->second;
            rec.reported = true;
            pending.emplace_back(it->first, rec);
        }
        for (const auto &entry : pending) {
            g_discMap.erase(entry.first);
        }
    }
    for (const auto &entry : pending) {
        ReportDiscAirtime(entry.first, entry.second);
    }
}

}  // namespace bluetooth
}  // namespace OHOS
