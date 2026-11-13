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

#ifndef LOG_TAG
#define LOG_TAG "bt_pbap_pce_datashare_helper"
#endif

#include "pbap_pce_datashare_helper.h"

#include <chrono>
#include <ctime>
#include <random>
#include <sstream>
#include <securec.h>
#include "iservice_registry.h"
#include "log.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace bluetooth {

/// DataShare URIs (aligned with contactdatashare.md and PSE pbap_pse_datashare_utils.cpp)
static constexpr const char *CONTACT_IMPORT_BATCH_URI_BASE =
    "datashare:///com.ohos.contactsdataability/contacts/import_batch";
static constexpr const char *CALLLOG_URI =
    "datashare:///com.ohos.calllogability/calls/calllog";

/// Calllog database field names (aligned with PSE pbap_pse_datashare_utils.cpp)
static constexpr const char *CALLLOG_DISPLAY_NAME = "display_name";
static constexpr const char *CALLLOG_PHONE_NUMBER = "phone_number";
static constexpr const char *CALLLOG_CREATE_TIME = "create_time";
static constexpr const char *CALLLOG_CALL_DIRECTION = "call_direction";
static constexpr const char *CALLLOG_ANSWER_STATE = "answer_state";

/// Call direction constants (aligned with PSE)
static const int32_t CALL_DIRECTION_INCOMING = 0;
static const int32_t CALL_DIRECTION_OUTGOING = 1;

/// Answer state constants (aligned with PSE)
static const int32_t CALL_ANSWER_MISSED = 0;
static const int32_t CALL_ANSWER_ACTIVED = 1;

/// X-IRMC-CALL-DATETIME property name prefix
static constexpr const char *TIMESTAMP_PROPERTY = "X-IRMC-CALL-DATETIME";

/// Timestamp format for X-IRMC-CALL-DATETIME: "yyyyMMdd'T'HHmmss"
static constexpr const char *TIMESTAMP_FORMAT = "%Y%m%dT%H%M%S";

PbapPceDataShareHelper::PbapPceDataShareHelper()
{
    HILOGI("[PbapPceDataShareHelper] created");
}

PbapPceDataShareHelper::~PbapPceDataShareHelper()
{
    Release();
    HILOGI("[PbapPceDataShareHelper] destroyed");
}

void PbapPceDataShareHelper::Release()
{
    // 锁内仅摘取 helper 引用并置空；跨模块的 helper->Release() 在锁外调用，
    // 避免持本端锁进入 DataShare（锁内跨模块调用有重入/死锁风险）
    std::shared_ptr<DataShare::DataShareHelper> contactHelper = nullptr;
    std::shared_ptr<DataShare::DataShareHelper> callLogHelper = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        contactHelper = std::move(contactHelper_);
        contactHelper_ = nullptr;
        callLogHelper = std::move(callLogHelper_);
        callLogHelper_ = nullptr;
    }
    if (contactHelper != nullptr) {
        contactHelper->Release();
    }
    if (callLogHelper != nullptr) {
        callLogHelper->Release();
    }
    HILOGI("[PbapPceDataShareHelper] Release: helpers released");
}

std::shared_ptr<DataShare::DataShareHelper> PbapPceDataShareHelper::CreateHelper(const std::string &uri)
{
    HILOGI("[PbapPceDataShareHelper] CreateHelper: (uri omitted)");

    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HILOGE("[PbapPceDataShareHelper] GetSystemAbilityManager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOGE("[PbapPceDataShareHelper] GetSystemAbility(BLUETOOTH_HOST) failed");
        return nullptr;
    }

    std::shared_ptr<DataShare::DataShareHelper> helper =
        DataShare::DataShareHelper::Creator(remoteObj, uri);
    if (helper == nullptr) {
        HILOGE("[PbapPceDataShareHelper] DataShareHelper::Creator failed (uri omitted)");
    }
    return helper;
}

std::shared_ptr<DataShare::DataShareHelper> PbapPceDataShareHelper::GetContactHelper()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (contactHelper_ == nullptr) {
        contactHelper_ = CreateHelper(CONTACT_IMPORT_BATCH_URI_BASE);
    }
    return contactHelper_;
}

std::shared_ptr<DataShare::DataShareHelper> PbapPceDataShareHelper::GetCallLogHelper()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callLogHelper_ == nullptr) {
        callLogHelper_ = CreateHelper(CALLLOG_URI);
    }
    return callLogHelper_;
}

int32_t PbapPceDataShareHelper::BatchInsertContacts(
    const std::vector<PbapPceVCardContact> &contacts, bool isFinish,
    const std::string &traceId, uint32_t batchId)
{
    HILOGI("[PbapPceDataShareHelper] BatchInsertContacts: count=%{public}zu, isFinish=%{public}d, "
        "traceId=%{public}s, batchId=%{public}u",
        contacts.size(), static_cast<int>(isFinish), traceId.c_str(), batchId);

    if (contacts.empty()) {
        if (!isFinish) {
            HILOGW("[PbapPceDataShareHelper] BatchInsertContacts: contacts list is empty, skip");
            return 0;
        }
        // 空入库集 + finish：仍需发送 commit-only 请求携带 isFinish，闭合服务端
        // 导入会话（末批全部被过滤时会走到此分支；不发则会话悬挂）
        auto helper = GetContactHelper();
        if (helper == nullptr) {
            HILOGE("[PbapPceDataShareHelper] BatchInsertContacts: contact helper is null");
            return -1;
        }
        std::string uriStr = BuildContactBatchUri(traceId, true, batchId);
        Uri uri(uriStr);
        HILOGI("[PbapPceDataShareHelper] commit-only batch (empty insert set), batchId=%{public}u, "
            "traceId=%{public}s", batchId, traceId.c_str());
        std::vector<DataShare::DataShareValuesBucket> emptyValues;
        int ret = helper->BatchInsert(uri, emptyValues);
        if (ret < 0) {
            HILOGE("[PbapPceDataShareHelper] commit-only batch failed, batchId=%{public}u, ret=%{public}d",
                batchId, ret);
            return ret;
        }
        return 0;
    }

    auto helper = GetContactHelper();
    if (helper == nullptr) {
        HILOGE("[PbapPceDataShareHelper] BatchInsertContacts: contact helper is null");
        return -1;
    }

    return InsertContactBatches(helper, contacts, isFinish, traceId, batchId);
}

int32_t PbapPceDataShareHelper::InsertContactBatches(std::shared_ptr<DataShare::DataShareHelper> helper,
    const std::vector<PbapPceVCardContact> &contacts, bool isFinish,
    const std::string &traceId, uint32_t batchId)
{
    // 按 CONTACTS_SUB_BATCH_SIZE 分组插入，每个子批使用独立 URI（batchId 为调用方
    // 分配的子批基号，跨批次全局递增；子批 id = 基号 + subBatchIdx）。
    size_t totalSubBatches = (contacts.size() + CONTACTS_SUB_BATCH_SIZE - 1) / CONTACTS_SUB_BATCH_SIZE;

    for (size_t batchStart = 0, subBatchIdx = 0; batchStart < contacts.size();
         batchStart += CONTACTS_SUB_BATCH_SIZE, subBatchIdx++) {
        size_t batchEnd = std::min(batchStart + static_cast<size_t>(CONTACTS_SUB_BATCH_SIZE), contacts.size());
        std::vector<DataShare::DataShareValuesBucket> values = BuildContactValuesBuckets(
            contacts, batchStart, batchEnd);

        // isFinish 仅在调用方标记最后一批时的最后一个子批为 true。
        bool subBatchFinish = isFinish && (subBatchIdx == totalSubBatches - 1);
        uint32_t curBatchId = batchId + static_cast<uint32_t>(subBatchIdx);
        std::string uriStr = BuildContactBatchUri(traceId, subBatchFinish, curBatchId);
        Uri uri(uriStr);

        HILOGI("[PbapPceDataShareHelper] inserting contacts batch: start=%{public}zu, count=%{public}zu, "
            "batchId=%{public}u, isFinish=%{public}d",
            batchStart, values.size(), curBatchId, static_cast<int>(subBatchFinish));

        int ret = helper->BatchInsert(uri, values);
        if (ret < 0) {
            HILOGE("[PbapPceDataShareHelper] BatchInsert contacts failed at batchStart=%{public}zu, "
                "batchId=%{public}u, ret=%{public}d",
                batchStart, curBatchId, ret);
            return ret;
        }
        HILOGI("[PbapPceDataShareHelper] contacts batch inserted OK: count=%{public}zu, "
            "batchId=%{public}u", values.size(), curBatchId);
    }

    return 0;
}

std::vector<DataShare::DataShareValuesBucket> PbapPceDataShareHelper::BuildContactValuesBuckets(
    const std::vector<PbapPceVCardContact> &contacts, size_t batchStart, size_t batchEnd)
{
    std::vector<DataShare::DataShareValuesBucket> values;
    for (size_t i = batchStart; i < batchEnd; i++) {
        const PbapPceVCardContact &contact = contacts[i];
        DataShare::DataShareValuesBucket bucket;
        // 无名联系人（仅有号码）不设置 name 字段：由库侧按空 display_name 处理
        // （生态惯例：空名联系人按号码显示），避免以号码伪造姓名导致判重失真
        if (!contact.displayName.empty()) {
            bucket.Put("name", contact.displayName);
        }
        std::string phoneNumbers = BuildPhoneNumbersString(contact);
        if (!phoneNumbers.empty()) {
            bucket.Put("phoneNumbers", phoneNumbers);
        }
        values.push_back(bucket);
    }
    return values;
}

int32_t PbapPceDataShareHelper::BatchInsertCallLogs(
    const std::vector<PbapPceVCardContact> &contacts, PbapPceCallType callType)
{
    HILOGI("[PbapPceDataShareHelper] BatchInsertCallLogs: count=%{public}zu, callType=%{public}d",
        contacts.size(), static_cast<int32_t>(callType));

    if (contacts.empty()) {
        HILOGW("[PbapPceDataShareHelper] BatchInsertCallLogs: contacts list is empty");
        return 0;
    }

    auto helper = GetCallLogHelper();
    if (helper == nullptr) {
        HILOGE("[PbapPceDataShareHelper] BatchInsertCallLogs: calllog helper is null");
        return -1;
    }

    int32_t callDirection = GetCallDirection(callType);
    int32_t answerState = GetAnswerState(callType);

    // Build values buckets for batch insert
    std::vector<DataShare::DataShareValuesBucket> values;
    for (const PbapPceVCardContact &contact : contacts) {
        DataShare::DataShareValuesBucket bucket;

        std::string phoneNumber = contact.phones.empty() ? "" : contact.phones[0];
        bucket.Put(CALLLOG_PHONE_NUMBER, phoneNumber);
        bucket.Put(CALLLOG_DISPLAY_NAME, contact.displayName);
        bucket.Put(CALLLOG_CALL_DIRECTION, callDirection);
        bucket.Put(CALLLOG_ANSWER_STATE, answerState);

        int64_t timestamp = ParseCallLogTimestamp(contact);
        if (timestamp > 0) {
            bucket.Put(CALLLOG_CREATE_TIME, timestamp);
        }

        values.push_back(bucket);
    }

    Uri uri(CALLLOG_URI);
    HILOGI("[PbapPceDataShareHelper] inserting calllog batch: count=%{public}zu", values.size());

    int ret = helper->BatchInsert(uri, values);
    if (ret < 0) {
        HILOGE("[PbapPceDataShareHelper] BatchInsert calllogs failed, ret=%{public}d", ret);
        return ret;
    }

    HILOGI("[PbapPceDataShareHelper] BatchInsertCallLogs done: inserted=%{public}zu", values.size());
    return 0;
}

std::string PbapPceDataShareHelper::StartContactSync()
{
    std::string traceId = GenerateTraceId();
    HILOGI("[PbapPceDataShareHelper] StartContactSync: new traceId=%{public}s", traceId.c_str());
    return traceId;
}

std::string PbapPceDataShareHelper::BuildContactBatchUri(
    const std::string &traceId, bool isFinish, uint32_t batchId)
{
    std::ostringstream oss;
    oss << CONTACT_IMPORT_BATCH_URI_BASE
        << "?traceId=" << traceId
        << "&isFinish=" << (isFinish ? "true" : "false")
        << "&batchId=" << batchId;
    return oss.str();
}

std::string PbapPceDataShareHelper::GenerateTraceId()
{
    // Generate a unique traceId using milliseconds timestamp + random number.
    auto now = std::chrono::system_clock::now();
    int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    std::random_device rd;
    uint32_t randVal = rd();
    std::ostringstream oss;
    oss << std::hex << ms << "-" << randVal;
    return oss.str();
}

std::string PbapPceDataShareHelper::BuildPhoneNumbersString(const PbapPceVCardContact &contact)
{
    std::string result;
    for (size_t i = 0; i < contact.phones.size(); i++) {
        if (i > 0) {
            result += ",";
        }
        result += contact.phones[i];
    }
    return result;
}

int64_t PbapPceDataShareHelper::ParseCallLogTimestamp(const PbapPceVCardContact &contact)
{
    // Search unknownXData for X-IRMC-CALL-DATETIME property.
    for (const auto &xData : contact.unknownXData) {
        // xData.first is like "X-IRMC-CALL-DATETIME;MISSED"
        // Check if it starts with the timestamp property name
        if (xData.first.find(TIMESTAMP_PROPERTY) != std::string::npos) {
            return ParseTimestampString(xData.second);
        }
    }
    return 0;
}

int64_t PbapPceDataShareHelper::ParseTimestampString(const std::string &timestampStr)
{
    // Parse vCard timestamp format "yyyyMMdd'T'HHmmss" to epoch seconds (10-digit).
    // Aligned with PSE ToRfc2455Format(): create_time stored as seconds (time_t).
    if (timestampStr.empty()) {
        return 0;
    }

    struct tm tmVal {};
    if (memset_s(&tmVal, sizeof(tmVal), 0, sizeof(tmVal)) != EOK) {
        HILOGE("[PbapPceDataShareHelper] ParseTimestampString memset_s failed");
        return 0;
    }

    // Use strptime to parse the format
    char *result = strptime(timestampStr.c_str(), TIMESTAMP_FORMAT, &tmVal);
    if (result == nullptr) {
        HILOGW("[PbapPceDataShareHelper] ParseTimestampString failed: %{public}s", timestampStr.c_str());
        return 0;
    }

    // Convert to epoch seconds (10-digit timestamp)
    time_t epochSecs = mktime(&tmVal);
    if (epochSecs < 0) {
        HILOGW("[PbapPceDataShareHelper] mktime failed for timestamp: %{public}s", timestampStr.c_str());
        return 0;
    }

    return static_cast<int64_t>(epochSecs); // epoch seconds, 10-digit timestamp for calllog create_time
}

int32_t PbapPceDataShareHelper::GetCallDirection(PbapPceCallType callType)
{
    // Aligned with PSE pbap_pse_datashare_utils.cpp:
    //   INCOMING_TYPE = 0, OUTGOING_TYPE = 1
    if (callType == PBAP_PCE_CALL_TYPE_OUTGOING) {
        return CALL_DIRECTION_OUTGOING;
    }
    return CALL_DIRECTION_INCOMING; // MISSED and INCOMING both use INCOMING direction
}

int32_t PbapPceDataShareHelper::GetAnswerState(PbapPceCallType callType)
{
    // Aligned with PSE pbap_pse_datashare_utils.cpp:
    //   CALL_ANSWER_MISSED = 0, CALL_ANSWER_ACTIVED = 1
    if (callType == PBAP_PCE_CALL_TYPE_MISSED) {
        return CALL_ANSWER_MISSED;
    }
    return CALL_ANSWER_ACTIVED; // INCOMING and OUTGOING both use ACTIVED
}

}  // namespace bluetooth
}  // namespace OHOS
