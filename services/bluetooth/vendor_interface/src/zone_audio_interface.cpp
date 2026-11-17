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

#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD000102
#endif
#ifndef LOG_TAG
#define LOG_TAG "bt_zone_audio_interface"
#endif

#include "zone_audio_interface.h"
#include "vendor_loader.h"
#include "vendor_log.h"

namespace {
/*
 * 懒加载 vendor 库并解析单个导出符号（extern "C"，如 OH_BtZoneAudioRegisterDeviceChangeCallback）。
 * 库不可用或符号缺失时返回 nullptr，由调用方回退默认值。
 */
void *GetVendorSymbol(const char *symbolName)
{
    void *handle = nullptr;
    if (vendor_interface_load_vendor_library(&handle) != 0) {
        HILOGE("vendor library is unavailable");
        return nullptr;
    }
    return vendor_interface_get_vendor_symbol(symbolName);
}
}  // namespace

namespace OHOS {
namespace bluetooth {
int32_t ZoneAudioInterface::RegisterDeviceChangeCallback(int32_t displayId,
    ZoneAudioDeviceChangeFunc callback, void *userData)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtZoneAudioRegisterDeviceChangeCallback)>(
        GetVendorSymbol("OH_BtZoneAudioRegisterDeviceChangeCallback"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_BtZoneAudioRegisterDeviceChangeCallback GetVendorSymbol failed");
        return ZONE_AUDIO_INTERFACE_FAILED;
    }
    int32_t ret = vendorFunc(displayId, callback, userData);
    HILOGI("OH_BtZoneAudioRegisterDeviceChangeCallback displayId=%{public}d, ret=%{public}d",
        displayId, ret);
    return ret;
}

int32_t ZoneAudioInterface::UnregisterDeviceChangeCallback(int32_t displayId)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtZoneAudioUnregisterDeviceChangeCallback)>(
        GetVendorSymbol("OH_BtZoneAudioUnregisterDeviceChangeCallback"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_BtZoneAudioUnregisterDeviceChangeCallback GetVendorSymbol failed");
        return ZONE_AUDIO_INTERFACE_FAILED;
    }
    int32_t ret = vendorFunc(displayId);
    HILOGI("OH_BtZoneAudioUnregisterDeviceChangeCallback displayId=%{public}d, ret=%{public}d",
        displayId, ret);
    return ret;
}
}  // namespace bluetooth
}  // namespace OHOS
