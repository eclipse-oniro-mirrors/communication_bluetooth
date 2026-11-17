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

#ifndef CAR_METRICS_REPORTER_H
#define CAR_METRICS_REPORTER_H

#include <string>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace bluetooth {

/// 数据上报门面：封装通话记录、扫描延迟、PBAP 同步等指标上报，通过 BtChrUeManager 写入 UE 事件。
/// 薄包装 BtChrUeManager，提供语义化 API 供 CarPolicyManager 调用。
class CarMetricsReporter {
public:
    CarMetricsReporter() = default;
    ~CarMetricsReporter() = default;

    /// 通话记录上报（BT_CALL_SERVICE_STATUS_RECORD）。
    /// 时间为 GetCurrentTimeMs() 格式化串；scoSwitch 为 "[1:3, 0:8]" 形式拼接串。
    void ReportCallRecord(const std::string &callIncoming, const std::string &callStart,
                          const std::string &callEnd, const std::string &scoSwitch);

    /// PBAP 通讯录同步上报（CONTACTS_SYN_STATUS_RECORD）。五个时点为格式化串，缺失传空串。
    void ReportPbapSync(const std::string &connecting, const std::string &connected,
                        const std::string &syncStart, const std::string &syncEnd,
                        const std::string &disconnected);

    /// Profile 链路上报（BT_CALL_MUSIC_LINK_STATUS_RECORD）。
    /// @param profileType 0=HFP 1=A2DP_SINK  @param reconnect 1=车机策略回连
    void ReportProfileLinkRecord(int profileType, int reconnect, const std::string &connecting,
                                 const std::string &connected, const std::string &disconnected);

    /// 扫描延迟上报（BT_SCAN_DELAY）。三个时点为 GetCurrentTimeMs() 格式化串，缺失传空串。
    void ReportScanDelay(const std::string &startTime, const std::string &foundTime,
                         const std::string &finishedTime);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_METRICS_REPORTER_H
