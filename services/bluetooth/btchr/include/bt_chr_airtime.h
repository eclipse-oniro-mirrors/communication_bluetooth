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

#ifndef BT_CHR_AIRTIME_H
#define BT_CHR_AIRTIME_H

#include <cstdint>
#include <map>
#include <string>
#include <utility>

namespace OHOS {
namespace bluetooth {

void BtChr2g4BleConnInterval(const std::string &addr, uint16_t interval);

void BtChr2g4A2dpStart(const std::string &addr);
void BtChr2g4A2dpActive(const std::string &addr, int codecIndex, int bitrateBps, int role);
void BtChr2g4A2dpSinkCodec(const std::string &addr, int codecIndex, int bitrateBps);
void BtChr2g4A2dpStop(const std::string &addr);
void BtChr2g4A2dpFail(const std::string &addr, int status, int failScene);

void BtChr2g4ScoStart(const std::string &addr);
void BtChr2g4ScoActive(const std::string &addr, int codec, int pktType, int tScoUs);
void BtChr2g4ScoStop(const std::string &addr, int hciStatus, int failScene);
void BtChr2g4ScoFail(const std::string &addr, int hciStatus, int failScene);

void BtChr2g4BrHidStart(const std::string &addr);
void BtChr2g4BrHidActive(const std::string &addr);
void BtChr2g4BrHidReportData(const std::string &addr, int len);
void BtChr2g4BrHidStop(const std::string &addr, int status);
void BtChr2g4BrHidFail(const std::string &addr, int status);

void BtChr2g4BrSocketStart(int32_t handle, const std::string &addr, int channel, int serviceType);
void BtChr2g4BrSocketActive(int32_t handle, const std::string &addr);
void BtChr2g4BrSocketData(int32_t handle, int len);
void BtChr2g4BrSocketStop(int32_t handle, int status);
void BtChr2g4BrSocketFail(const std::string &addr, int status, int failScene);

void BtChr2g4BleHogpStart(const std::string &addr);
void BtChr2g4BleHogpActive(const std::string &addr);
void BtChr2g4BleHogpReportData(const std::string &addr, int len);
void BtChr2g4BleHogpStop(const std::string &addr, int status);
void BtChr2g4BleHogpFail(const std::string &addr, int status);

void BtChr2g4BleGattStart(const std::string &addr, const std::string &pkgName);
void BtChr2g4BleGattActive(const std::string &addr);
void BtChr2g4BleGattTx(const std::string &addr, int len);
void BtChr2g4BleGattRx(const std::string &addr, int len);
void BtChr2g4BleGattStop(const std::string &addr, int status);
void BtChr2g4BleGattFail(const std::string &addr, int status, int failScene);

void BtChr2g4BleScanParams(int scanWindow, int scanInterval, int scanType, const std::string &pkgName);
void BtChr2g4BleScanEnable(bool success, const std::string &pkgName);
void BtChr2g4BleScanDisable();
void BtChr2g4BleScanFlush();

void BtChr2g4BleAdvStart(int advertiserId, int advInterval, int advType, int payloadLenB, const std::string &pkgName);
void BtChr2g4BleAdvActive(int advertiserId);
void BtChr2g4BleAdvStop(int advertiserId);
void BtChr2g4BleAdvFail(int advertiserId, int failScene);

void BtChr2g4BleGattPkgNameSet(const std::string &addr, const std::string &pkgName);
void BtChr2g4BleScanPkgNameSet(const std::string &pkgName);
void BtChr2g4BleAdvPkgNameSet(int advertiserId, const std::string &pkgName);

void BtChr2g4DiscStart(int procedureType, int scanWindowSlots, int scanIntervalSlots);
void BtChr2g4DiscActive(int procedureType);
void BtChr2g4DiscStop(int procedureType, int failReason);
void BtChr2g4DiscFlushAll();
struct A2dpAirtimeRecord {
    uint32_t sessionId {};
    int64_t startTs {0};
    int64_t activeTs {0};
    int64_t stopTs {0};
    int64_t failTs {0};
    int32_t codecIndex {-1};
    int32_t bitrateBps {0};
    int32_t status {0};
    int32_t failScene {0};
    int32_t role {0};
    bool reported {false};
};

struct ScoAirtimeRecord {
    uint32_t sessionId {};
    int64_t startTs {0};
    int64_t activeTs {0};
    int64_t stopTs {0};
    int64_t failTs {0};
    int32_t scoCodec {0};
    int32_t pktType {0};
    int32_t tScoUs {0};
    int32_t hciStatus {0};
    int32_t failScene {0};
    bool reported {false};
};

struct HidAirtimeRecord {
    uint32_t sessionId {};
    int64_t startTs {0};
    int64_t activeTs {0};
    int64_t stopTs {0};
    int64_t failTs {0};
    int64_t activeSteadyMs {0};
    int32_t reportCount {0};
    int64_t reportBytes {0};
    int32_t notifCount {0};
    int32_t connInterval {0};
    int32_t status {0};
    int32_t failScene {0};
    bool isLe {false};
    bool reported {false};
};

struct SocketAirtimeRecord {
    uint32_t sessionId {};
    std::string addr;
    int64_t startTs {0};
    int64_t activeTs {0};
    int64_t stopTs {0};
    int64_t failTs {0};
    int32_t channel {0};
    int32_t serviceType {0};
    int64_t activeSteadyMs {0};
    int64_t txBytes {0};
    int64_t rxBytes {0};
    int32_t status {0};
    int32_t failScene {0};
};

struct GattAirtimeRecord {
    uint32_t sessionId {};
    int64_t startTs {0};
    int64_t activeTs {0};
    int64_t stopTs {0};
    int64_t failTs {0};
    std::string pkgName;
    int64_t activeSteadyMs {0};
    int64_t txBytes {0};
    int64_t rxBytes {0};
    int32_t connInterval {0};
    int32_t status {0};
    int32_t failScene {0};
    bool reported {false};
};

struct ScanAirtimeRecord {
    int64_t startTs {0};
    int64_t activeTs {0};
    int64_t stopTs {0};
    int64_t failTs {0};
    std::string pkgName;
    int32_t scanWindowSlot {0};
    int32_t scanIntervalSlot {0};
    int32_t scanType {0};
    int32_t failStat {-1};
    bool reported {false};
};

struct AdvAirtimeRecord {
    uint32_t sessionId {};
    int64_t startTs {0};
    int64_t activeTs {0};
    int64_t stopTs {0};
    int64_t failTs {0};
    std::string pkgName;
    int32_t advIntervalSlot {0};
    int32_t advType {0};
    int32_t payloadLenB {0};
    int32_t failScene {0};
    bool reported {false};
};

struct DiscAirtimeRecord {
    int64_t startTs {0};
    int64_t activeTs {0};
    int64_t failTs {0};
    int64_t stopTs {0};
    int32_t scanWindowSlots {0};
    int32_t scanIntervalSlots {0};
    int32_t failReason {-1};
    bool reported {false};
};

}
}
#endif
