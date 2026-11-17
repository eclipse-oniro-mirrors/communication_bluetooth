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

#include "car_metrics_reporter_fuzzer.h"

#include "car_metrics_reporter.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "../../bt_utils/fuzztest_utils.h"

using namespace OHOS::bluetooth;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    CarMetricsReporter reporter;

    // 通话记录上报：fuzz 三个时点串与 SCO 切换拼接串
    std::string callIncoming = provider.ConsumeRandomLengthString(32);
    std::string callStart = provider.ConsumeRandomLengthString(32);
    std::string callEnd = provider.ConsumeRandomLengthString(32);
    std::string scoSwitch = provider.ConsumeRandomLengthString(64);
    reporter.ReportCallRecord(callIncoming, callStart, callEnd, scoSwitch);

    // PBAP 同步上报：fuzz 五个时点串（空串覆盖"缺失时点"语义）
    std::string connecting = provider.ConsumeRandomLengthString(32);
    std::string connected = provider.ConsumeRandomLengthString(32);
    std::string syncStart = provider.ConsumeRandomLengthString(32);
    std::string syncEnd = provider.ConsumeRandomLengthString(32);
    std::string disconnected = provider.ConsumeRandomLengthString(32);
    reporter.ReportPbapSync(connecting, connected, syncStart, syncEnd, disconnected);

    // Profile 链路上报：profileType(0=HFP 1=A2DP_SINK)、reconnect(1=车机策略回连)
    int profileType = provider.ConsumeIntegralInRange<int>(0, 1);
    int reconnect = provider.ConsumeIntegralInRange<int>(0, 1);
    std::string linkConnecting = provider.ConsumeRandomLengthString(32);
    std::string linkConnected = provider.ConsumeRandomLengthString(32);
    std::string linkDisconnected = provider.ConsumeRandomLengthString(32);
    reporter.ReportProfileLinkRecord(profileType, reconnect, linkConnecting, linkConnected, linkDisconnected);

    // 扫描延迟上报：fuzz 三个时点串
    std::string scanStart = provider.ConsumeRandomLengthString(32);
    std::string scanFound = provider.ConsumeRandomLengthString(32);
    std::string scanFinished = provider.ConsumeRandomLengthString(32);
    reporter.ReportScanDelay(scanStart, scanFound, scanFinished);
    return true;
}
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::FUZZ_DATA_LEN) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
