/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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
#define LOG_TAG "bt_service_util_datashare"
#endif

#include "bluetooth_datashare_utils.h"

#include <cinttypes>
#include <vector>

#include "log.h"
#include "common_util.h"
#include "bt_def.h"
#include "hitrace_meter.h"
#include "adapter_properties.h"
#include "bluetooth_os_account.h"

#ifdef BLUETOOTH_OS_ACCOUNT
#include "os_account_manager.h"
#include "os_account_subprofile_client.h"
#endif

namespace OHOS {
namespace bluetooth {
namespace {
constexpr const char *SETTINGS_DATASHARE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATASHARE_EXTENSION_URI = "datashare://com.ohos.settingsdata.DataAbility";
constexpr const char *SETTINGS_DATA_COLUMN_KEYWORD = "KEYWORD";
constexpr const char *SETTINGS_DATA_COLUMN_VALUE = "VALUE";
constexpr const char *SETTINGS_DATASHARE_BASE_URI = "datashare:///com.ohos.settingsdata/entry/settingsdata/";
constexpr const char *SETTINGS_DATASHARE_USER_TABLE_PREFIX = "USER_SETTINGSDATA_";
constexpr const char *SETTINGS_DATASHARE_PROXY_KEY_SUFFIX = "?Proxy=true&key=";
constexpr uint64_t MAIN_DRIVING_DISPLAY_ID = 0;
} // namespace

BluetoothDataShareHelperUtils::BluetoothDataShareHelperUtils()
{
    auto [_, helper] = CreateDataShareHelper();
    dataShareHelper_ = helper;
}

std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> BluetoothDataShareHelperUtils::CreateDataShareHelper()
{
    HITRACE_METER(BT_TRACE_TAG);
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils GetSystemAbilityManager failed.");
        return {};
    }
#ifdef BT_MCU_PROXY_ENABLE
    sptr<IRemoteObject> dataShareRemote = saManager->CheckSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    if (dataShareRemote == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils datashare is not ready.");
        return {};
    }
#endif
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils GetSystemAbility Service Failed.");
        return {};
    }
    std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> helperPair =
        DataShare::DataShareHelper::Create(remoteObj, SETTINGS_DATASHARE_URI, SETTINGS_DATASHARE_EXTENSION_URI);
    if (helperPair.first != DataShare::E_OK) {
        HILOGE("DataShareHelper create failed, ret: %{public}d", helperPair.first);
    }
    return helperPair;
}

bool BluetoothDataShareHelperUtils::CheckDataShareIsReady(void)
{
    auto [ret, _] = CreateDataShareHelper();
    return ret != DataShare::E_DATA_SHARE_NOT_READY;
}

bool BluetoothDataShareHelperUtils::RegisterObserver(Uri &uri, const sptr<BluetoothObserver> &dataObserver)
{
    if (dataShareHelper_ == nullptr) {
        HILOGE("dataShareHelper_ is nullptr");
        return false;
    }
    dataShareHelper_->RegisterObserver(uri, dataObserver);
    return true;
}

bool BluetoothDataShareHelperUtils::UnregisterObserver(Uri &uri, const sptr<BluetoothObserver> &dataObserver)
{
    if (dataShareHelper_ == nullptr) {
        HILOGE("dataShareHelper_ is nullptr");
        return false;
    }
    dataShareHelper_->UnregisterObserver(uri, dataObserver);
    return true;
}

int32_t BluetoothDataShareHelperUtils::Query(Uri &uri, const std::string &key, std::string &value)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (dataShareHelper_ == nullptr) {
        HILOGE("dataShareHelper_ is nullptr");
        return RET_BAD_PARAM;
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    predicates.EqualTo(SETTINGS_DATA_COLUMN_KEYWORD, key);
    auto result = dataShareHelper_->Query(uri, predicates, columns);
    if (result == nullptr) {
        HILOGE("DataShareHelper query error, result is null");
        return RET_BAD_STATUS;
    }

    if (result->GoToFirstRow() != DataShare::E_OK) {
        HILOGE("DataShareHelper query failed,go to first row error");
        result->Close();
        return RET_BAD_STATUS;
    }

    int columnIndex;
    result->GetColumnIndex(SETTINGS_DATA_COLUMN_VALUE, columnIndex);
    result->GetString(columnIndex, value);
    result->Close();
    HILOGI("DataShareHelper query success,value[%{public}s]", GET_ENCRYPT_DEVICE_NAME(value));
    return RET_NO_ERROR;
}

int32_t BluetoothDataShareHelperUtils::Insert(Uri &uri, const std::string &key, const std::string &value)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (dataShareHelper_ == nullptr) {
        HILOGE("dataShareHelper_ is nullptr");
        return RET_BAD_PARAM;
    }
    DataShare::DataShareValuesBucket valuesBucket;
    DataShare::DataShareValueObject keyObj(key);
    DataShare::DataShareValueObject valueObj(value);
    valuesBucket.Put(SETTINGS_DATA_COLUMN_KEYWORD, keyObj);
    valuesBucket.Put(SETTINGS_DATA_COLUMN_VALUE, valueObj);
    int32_t result = dataShareHelper_->Insert(uri, valuesBucket);
    if (result == INVALID_TYPE) {
        HILOGE("DataShareHelper insert failed, insert result:%{public}d", result);
        return RET_BAD_STATUS;
    }
    dataShareHelper_->NotifyChange(uri);
    HILOGI("DataShareHelper insert success");
    return RET_NO_ERROR;
}

int32_t BluetoothDataShareHelperUtils::Update(Uri &uri, const std::string &key, const std::string &value)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (dataShareHelper_ == nullptr) {
        HILOGE("dataShareHelper_ is nullptr");
        return RET_BAD_PARAM;
    }
    std::string queryValue;
    int32_t ret = Query(uri, key, queryValue);
    if (ret == RET_BAD_STATUS) {
        return Insert(uri, key, value);
    }

    DataShare::DataShareValuesBucket valuesBucket;
    DataShare::DataShareValueObject valueObj(value);
    valuesBucket.Put(SETTINGS_DATA_COLUMN_VALUE, valueObj);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTINGS_DATA_COLUMN_KEYWORD, key);
    int32_t result = dataShareHelper_->Update(uri, predicates, valuesBucket);
    if (result == INVALID_TYPE) {
        return RET_BAD_STATUS;
    }
    dataShareHelper_->NotifyChange(uri);
    HILOGI("DataShareHelper update success");
    return RET_NO_ERROR;
}

bool BluetoothDataShareQuery(const char *uriStr, const char *keyStr, std::string &outValue)
{
    CHECK_AND_RETURN_LOG_RET(uriStr && keyStr, false, "uri or key is nullptr");
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();

    Uri uri(uriStr);
    std::string key = keyStr;
    std::string value = "";
    if (dataShareHelperUtils->Query(uri, key, value) != RET_NO_ERROR) {
        HILOGE("Query uri(%{public}s) key(%{public}s) fail", uriStr, keyStr);
        return false;
    }

    outValue = value;
    return true;
}

int32_t BluetoothGetMainDrivingUserId()
{
#ifdef BLUETOOTH_OS_ACCOUNT
    // 主驾屏幕的 displayId 固定为 0
    int32_t localId = INVALID_ID;
    ErrCode err = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(MAIN_DRIVING_DISPLAY_ID, localId);
    if (err != 0) {
        HILOGE("BluetoothGetMainDrivingUserId: GetForegroundOsAccountLocalId failed, err=%{public}d", err);
        return INVALID_ID;
    }
    HILOGI("BluetoothGetMainDrivingUserId: displayId=%{public}" PRIu64 ", userId=%{public}d",
        MAIN_DRIVING_DISPLAY_ID, localId);
    return localId;
#else
    return INVALID_ID;
#endif
}

std::string BuildUriFromUserId(const std::string &key, int32_t userId)
{
    // 用户空间 ID 无效时不回退到全局 SETTINGSDATA（涉及用户隐私），返回空串由上层按失败处理
    if (userId == INVALID_ID) {
        return "";
    }
    // 用户空间隔离数据 URI：USER_SETTINGSDATA_SECURE_<userId>?Proxy=true&key=<key>
    return std::string(SETTINGS_DATASHARE_BASE_URI) + SETTINGS_DATASHARE_USER_TABLE_PREFIX
        + std::to_string(userId) + SETTINGS_DATASHARE_PROXY_KEY_SUFFIX + key;
}

std::string BluetoothBuildSettingsUri(const std::string &key)
{
    return BuildUriFromUserId(key, BluetoothGetMainDrivingUserId());
}

bool BluetoothDataShareQueryForUser(const std::string &key, std::string &outValue)
{
    std::string uriStr = BluetoothBuildSettingsUri(key);
    if (uriStr.empty()) {
        HILOGE("BluetoothDataShareQueryForUser: invalid userId, skip query key(%{public}s)", key.c_str());
        return false;
    }
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    Uri uri(uriStr);
    std::string value;
    if (dataShareHelperUtils->Query(uri, key, value) != RET_NO_ERROR) {
        HILOGE("BluetoothDataShareQueryForUser: Query key(%{public}s) fail", key.c_str());
        return false;
    }
    outValue = value;
    return true;
}

bool BluetoothDataShareUpdateForUser(const std::string &key, const std::string &value)
{
    std::string uriStr = BluetoothBuildSettingsUri(key);
    if (uriStr.empty()) {
        HILOGE("BluetoothDataShareUpdateForUser: invalid userId, skip update key(%{public}s)", key.c_str());
        return false;
    }
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    Uri uri(uriStr);
    if (dataShareHelperUtils->Update(uri, key, value) != RET_NO_ERROR) {
        HILOGE("BluetoothDataShareUpdateForUser: Update key(%{public}s) fail", key.c_str());
        return false;
    }
    return true;
}

#ifdef BLUETOOTH_OS_ACCOUNT
ErrCode QueryMainDrivingSubProfileId(int32_t &subProfileId)
{
    int32_t userId = BluetoothGetMainDrivingUserId();
    if (userId <= 0) {
        // 主驾屏前台用户未就绪（开机初期/未登录）：无查询目标，按账号不存在处理
        return ERR_ACCOUNT_COMMON_ACCOUNT_NOT_EXIST_ERROR;
    }
    return AccountSA::OsAccountSubProfileClient::GetInstance().GetOsAccountForegroundSubProfileId(
        userId, subProfileId);
}
#endif
} // namespace bluetooth
} // namespace OHOS