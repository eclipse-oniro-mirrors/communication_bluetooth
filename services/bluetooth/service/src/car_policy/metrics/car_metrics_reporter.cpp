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

#include "car_metrics_reporter.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_metrics"
#endif
#include "log.h"

#include "bt_chr_ue_manager.h"

namespace OHOS {
namespace bluetooth {

void CarMetricsReporter::ReportCallRecord(const std::string &callIncoming, const std::string &callStart,
                                          const std::string &callEnd, const std::string &scoSwitch)
{
    auto ue = BtChrUeManager::GetInstance();
    if (ue == nullptr) {
        HILOGW("BtChrUeManager null, skip ReportCallRecord");
        return;
    }
    ue->WriteCallServiceStatusRecordUe(callIncoming, callStart, callEnd, scoSwitch);
}

void CarMetricsReporter::ReportPbapSync(const std::string &connecting, const std::string &connected,
                                        const std::string &syncStart, const std::string &syncEnd,
                                        const std::string &disconnected)
{
    auto ue = BtChrUeManager::GetInstance();
    if (ue == nullptr) {
        HILOGW("BtChrUeManager null, skip ReportPbapSync");
        return;
    }
    ue->WriteContactsSynStatusRecordUe(connecting, connected, syncStart, syncEnd, disconnected);
}

void CarMetricsReporter::ReportProfileLinkRecord(int profileType, int reconnect,
                                                 const std::string &connecting, const std::string &connected,
                                                 const std::string &disconnected)
{
    auto ue = BtChrUeManager::GetInstance();
    if (ue == nullptr) {
        HILOGW("BtChrUeManager null, skip ReportProfileLinkRecord");
        return;
    }
    ue->WriteCallMusicLinkStatusRecordUe(profileType, reconnect, connecting, connected, disconnected);
}

void CarMetricsReporter::ReportScanDelay(const std::string &startTime, const std::string &foundTime,
                                         const std::string &finishedTime)
{
    auto ue = BtChrUeManager::GetInstance();
    if (ue == nullptr) {
        HILOGW("BtChrUeManager null, skip ReportScanDelay");
        return;
    }
    ue->WriteScanDelayUe(startTime, foundTime, finishedTime);
}

}  // namespace bluetooth
}  // namespace OHOS
