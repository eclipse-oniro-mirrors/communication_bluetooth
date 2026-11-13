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

#ifndef PBAP_PCE_DATASHARE_HELPER_H
#define PBAP_PCE_DATASHARE_HELPER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "datashare_helper.h"
#include "datashare_values_bucket.h"
#include "pbap_pce_vcard_parser.h"
#include "uri.h"

namespace OHOS {
namespace bluetooth {

/// 联系人入库子批大小（datashare 单次批量插入上限约束）
constexpr int32_t CONTACTS_SUB_BATCH_SIZE = 250;

/// Call log type, determined by PBAP path (MCH/ICH/OCH).
enum PbapPceCallType {
    PBAP_PCE_CALL_TYPE_MISSED = 0,   // MCH path
    PBAP_PCE_CALL_TYPE_INCOMING = 1, // ICH path
    PBAP_PCE_CALL_TYPE_OUTGOING = 2, // OCH path
};

class PbapPceDataShareHelper {
public:
    PbapPceDataShareHelper();
    ~PbapPceDataShareHelper();

    std::string StartContactSync();
    int32_t BatchInsertContacts(
        const std::vector<PbapPceVCardContact> &contacts, bool isFinish,
        const std::string &traceId, uint32_t batchId);
    int32_t BatchInsertCallLogs(const std::vector<PbapPceVCardContact> &contacts, PbapPceCallType callType);

    void Release();

private:
    std::shared_ptr<DataShare::DataShareHelper> CreateHelper(const std::string &uri);
    std::shared_ptr<DataShare::DataShareHelper> GetContactHelper();
    std::shared_ptr<DataShare::DataShareHelper> GetCallLogHelper();

    static std::string BuildContactBatchUri(const std::string &traceId, bool isFinish, uint32_t batchId);
    int32_t InsertContactBatches(std::shared_ptr<DataShare::DataShareHelper> helper,
        const std::vector<PbapPceVCardContact> &contacts, bool isFinish,
        const std::string &traceId, uint32_t batchId);
    static std::string GenerateTraceId();
    static std::string BuildPhoneNumbersString(const PbapPceVCardContact &contact);
    static std::vector<DataShare::DataShareValuesBucket> BuildContactValuesBuckets(
        const std::vector<PbapPceVCardContact> &contacts, size_t batchStart, size_t batchEnd);
    static int64_t ParseCallLogTimestamp(const PbapPceVCardContact &contact);
    static int64_t ParseTimestampString(const std::string &timestampStr);
    static int32_t GetCallDirection(PbapPceCallType callType);
    static int32_t GetAnswerState(PbapPceCallType callType);

    std::mutex mutex_;
    std::shared_ptr<DataShare::DataShareHelper> contactHelper_ = nullptr;
    std::shared_ptr<DataShare::DataShareHelper> callLogHelper_ = nullptr;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PBAP_PCE_DATASHARE_HELPER_H
