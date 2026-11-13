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
#define LOG_TAG "bt_vendor_loader"
#endif

#include "vendor_loader.h"

#include <dlfcn.h>
#include <mutex>

#include "vendor_log.h"

namespace {
std::once_flag g_loadOnceFlag;
void *g_vendorLibraryHandle = nullptr;
int g_loadResult = 0;

/* Vendor library ships under the fixed name libbt_vendor_service.z.so. */
const char *GetVendorLibraryName()
{
    return "libbt_vendor_service.z.so";
}

void LoadVendorLibrary()
{
    const char *libName = GetVendorLibraryName();
    void *handle = dlopen(libName, RTLD_NOW);
    if (handle == nullptr) {
        HILOGE("Failed to dlopen %{public}s: %{public}s", libName, dlerror());
        g_loadResult = -1;
        return;
    }
    g_vendorLibraryHandle = handle;
    HILOGI("Vendor library loaded: %{public}s", libName);
    g_loadResult = 0;
}
}  // namespace

int vendor_interface_load_vendor_library(void **handle)
{
    if (handle == nullptr) {
        return -1;
    }
    std::call_once(g_loadOnceFlag, LoadVendorLibrary);
    *handle = g_vendorLibraryHandle;
    return g_loadResult;
}

void *vendor_interface_get_vendor_symbol(const char *symbolName)
{
    if (g_vendorLibraryHandle == nullptr || symbolName == nullptr) {
        HILOGE("vendor library not loaded or symbol name is null");
        return nullptr;
    }
    void *symbol = dlsym(g_vendorLibraryHandle, symbolName);
    if (symbol == nullptr) {
        HILOGE("Failed to dlsym %{public}s: %{public}s", symbolName, dlerror());
    }
    return symbol;
}
