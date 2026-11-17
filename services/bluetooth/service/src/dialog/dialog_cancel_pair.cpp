/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
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
#define LOG_TAG "bt_service_dialog_cancel_pair"
#endif

#include <algorithm>
#include <string>

#include "ability_connection.h"
#include "ability_connect_callback.h"
#include "bluetooth_dialog.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "extension_manager_client.h"
#include "int_wrapper.h"
#include "ipc_skeleton.h"
#include "json_utils.h"
#include "log.h"
#include "permission_manager.h"
#include "raw_address.h"
#include "refbase.h"
#include "remote_device_properties.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"
#include "dialog_cancel_pair.h"
#include "vendor_interface.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
bool DialogCancelPair::RequestCancelPairDialog(const RawAddress &device)
{
    const std::string abilityName = "BluetoothUnpairDialog";
    const std::string bundleName = VendorInterface::GetSettingsCallingName();
    std::string thirdlyBundleName = PermissionManager::GetCallingName();
    int32_t userId = PermissionManager::GetCallingUserId();
    std::string appLabel = PermissionManager::GetCallingAppLabel(thirdlyBundleName, userId);
    if (appLabel == "") {
        HILOGI("fail to get appLabel for %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
        return false;
    }
    std::string connectStr = DialogCancelPair::BuildStartCommand(device, appLabel);
    if (!BluetoothDialog::DialogConnectExtension(connectStr, bundleName, abilityName, userId)) {
        HILOGE("failed to connect cancel pair dialog for %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
        return false;
    }
    HILOGI("cancel pair dialog connected for %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    return true;
}

std::string DialogCancelPair::BuildStartCommand(const RawAddress &device, const std::string &appLabel)
{
    cJSON* root = cJSON_CreateObject();
    if (root == nullptr) {
        HILOGE("cJSON_CreateObject failed");
        return "";
    }
    std::string deviceName = RemoteDeviceProperties::GetInstance()->GetDeviceName(device);
    std::string uiType = "sysDialog/common";
    cJSON_AddStringToObject(root, "ability.want.params.uiExtensionType", uiType.c_str());
    cJSON_AddStringToObject(root, "deviceId", device.GetAddress().c_str());
    cJSON_AddStringToObject(root, "deviceName", deviceName.c_str());
    cJSON_AddStringToObject(root, "appName", appLabel.c_str());

    std::string result = JsonUtil::GetSimpleFormatString(root);
    if (result.empty()) {
        HILOGE("cJSON_Print error");
    }
    cJSON_Delete(root);
    return result;
}
} // namespace bluetooth
} // namespace OHOS