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

#ifndef BLUETOOTH_DATASHARE_UTILS_H
#define BLUETOOTH_DATASHARE_UTILS_H

#include <memory>
#include <utility>

#include "errors.h"

#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"
#include "data_ability_observer_stub.h"
#include "uri.h"

namespace OHOS {
namespace bluetooth {

constexpr const char *SETTINGS_DATASHARE_URL_AIRPLANE_MODE =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=airplane_mode";
constexpr const char *SETTINGS_DATASHARE_KEY_AIRPLANE_MODE = "settings.telephony.airplanemode";

constexpr const char *SETTINGS_DATASHARE_URL_DEVICE_NAME =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_SECURE_";
constexpr const char *SETTINGS_DATASHARE_KEY_DEVICE_NAME = "settings.general.display_device_name";

constexpr const char *SETTINGS_DATASHARE_URI_COLLABORATION_SWITCH =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=";
constexpr const char *SETTINGS_DATASHARE_KEY_COLLABORATION_SWITCH =
    "settings.collaboration.multi_device_collaboration_service_switch";

constexpr const char *SETTINGS_DATASHARE_URI_DEFAULT_DEVICE_NAME =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=settings.general.device_name";
constexpr const char *SETTINGS_DATASHARE_KEY_DEFAULT_DEVICE_NAME = "settings.general.device_name";

class BluetoothObserver : public AAFwk::DataAbilityObserverStub {
public:
    explicit BluetoothObserver(const std::function<void(void)> &onChange)
        : onChange_(onChange) {}
    ~BluetoothObserver() override = default;
    void OnChange() override
    {
        if (onChange_) {
            onChange_();
        }
    }
private:
    std::function<void(void)> onChange_;
};

class BluetoothDataShareHelperUtils final {
public:
    BluetoothDataShareHelperUtils();
    ~BluetoothDataShareHelperUtils() = default;
    int32_t Query(Uri &uri, const std::string &key, std::string &value);
    int32_t Insert(Uri &uri, const std::string &key, const std::string &value);
    int32_t Update(Uri &uri, const std::string &key, const std::string &value);

    static bool CheckDataShareIsReady(void);
    bool RegisterObserver(Uri &uri, const sptr<BluetoothObserver> &dataObserver);
    bool UnregisterObserver(Uri &uri, const sptr<BluetoothObserver> &dataObserver);
private:
    static std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> CreateDataShareHelper();
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper_ = nullptr;
};

bool BluetoothDataShareQuery(const char *uriStr, const char *keyStr, std::string &outValue);

/** 
* 获取主驾屏幕对应的用户空间 ID。
* 主驾屏幕的 displayId 固定为 0，通过该 displayId 获取前台用户空间 ID；获取失败时返回 INVALID_ID。
**/
int32_t BluetoothGetMainDrivingUserId();

/**
* 构建指定用户空间的 DataShare URI。
* userId 有效时返回 "USER_SETTINGSDATA_SECURE_<userId>?Proxy=true&key=<key>"（用户空间隔离数据），
* userId 为 INVALID_ID 时返回空串（不回退到全局 SETTINGSDATA，避免跨用户数据隐私泄露），由上层按失败处理。
**/
std::string BuildUriFromUserId(const std::string &key, int32_t userId);

/* 
* 构建主驾用户空间的 DataShare URI（内部调用 BluetoothGetMainDrivingUserId 获取 userId）。
**/
std::string BluetoothBuildSettingsUri(const std::string &key);

/** 
* 查询主驾用户空间的 DataShare 设置值。
**/
bool BluetoothDataShareQueryForUser(const std::string &key, std::string &outValue);

/**
* 更新主驾用户空间的 DataShare 设置值。
* DataShare settings 表不支持删行，清空操作应传入空字符串。
**/
bool BluetoothDataShareUpdateForUser(const std::string &key, const std::string &value);

/**
* 查询主驾屏前台用户当前的账号子空间 ID（subProfileId）。
* 双参接口（主驾 userId + 出参 subProfileId）：单参接口对 native 进程（uid→0=ADMIN）
* 必然失败；主驾屏前台用户未就绪（开机初期/未登录）时按账号不存在返回错误码。
**/
ErrCode QueryMainDrivingSubProfileId(int32_t &subProfileId);

} // namespace bluetooth
} // namespace OHOS
#endif // BLUETOOTH_DATASHARE_UTILS_H