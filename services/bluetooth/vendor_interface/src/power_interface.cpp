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
#define LOG_TAG "bt_power_interface"
#endif

#include "power_interface.h"
#include "vendor_loader.h"
#include "vendor_log.h"

namespace {
/*
 * 懒加载 vendor 库并解析单个导出符号（extern "C"，如 OH_PowerIsScreenOn）。
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
bool PowerInterface::IsScreenOn()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_PowerIsScreenOn)>(
        GetVendorSymbol("OH_PowerIsScreenOn"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_PowerIsScreenOn GetVendorSymbol failed");
        return false;
    }
    return vendorFunc();
}

int32_t PowerInterface::HoldRunningLock(const std::string &name)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_PowerHoldRunningLock)>(
        GetVendorSymbol("OH_PowerHoldRunningLock"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_PowerHoldRunningLock GetVendorSymbol failed");
        return PowerErrors::ERR_FAILURE;
    }
    return vendorFunc(name.c_str());
}

int32_t PowerInterface::ReleaseRunningLock()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_PowerReleaseRunningLock)>(
        GetVendorSymbol("OH_PowerReleaseRunningLock"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_PowerReleaseRunningLock GetVendorSymbol failed");
        return PowerErrors::ERR_FAILURE;
    }
    return vendorFunc();
}

bool PowerInterface::IsRunningLockUsed()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_PowerIsRunningLockUsed)>(
        GetVendorSymbol("OH_PowerIsRunningLockUsed"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_PowerIsRunningLockUsed GetVendorSymbol failed");
        return false;
    }
    return vendorFunc();
}

int32_t WatchPowerInterface::HoldRunningLock(const std::string &name, int32_t timeoutMs)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_WATCH_PowerHoldRunningLock)>(
        GetVendorSymbol("OH_WATCH_PowerHoldRunningLock"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_WATCH_PowerHoldRunningLock GetVendorSymbol failed");
        return PowerErrors::ERR_FAILURE;
    }
    return vendorFunc(name.c_str(), timeoutMs);
}

int32_t WatchPowerInterface::ReleaseRunningLock()
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_WATCH_PowerReleaseRunningLock)>(
        GetVendorSymbol("OH_WATCH_PowerReleaseRunningLock"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_WATCH_PowerReleaseRunningLock GetVendorSymbol failed");
        return PowerErrors::ERR_FAILURE;
    }
    return vendorFunc();
}
}  // namespace bluetooth
}  // namespace OHOS