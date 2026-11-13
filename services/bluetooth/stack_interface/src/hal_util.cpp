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

#ifndef LOG_TAG
#define LOG_TAG "bt_stack_interface"
#endif

#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD000102
#endif

#include "hal_util.h"

#include <dlfcn.h>
#include <mutex>

#include "bt_dm_interface.h"
#include "hilog/log.h"

#undef HILOGF
#undef HILOGE
#undef HILOGW
#undef HILOGI
#undef HILOGD
#define HILOGE(fmt, ...) HILOG_ERROR(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)
#define HILOGW(fmt, ...) HILOG_WARN(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)
#define HILOGI(fmt, ...) HILOG_INFO(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)
#define HILOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)
#define HILOGF(fmt, ...) HILOG_FATAL(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)

namespace {
std::once_flag g_loadOnceFlag;
const bt_interface_t *g_btInterface = nullptr;
int g_loadResult = 0;
void *g_btLibraryHandle = nullptr;

/* Stack library selection: both the closed-source and open stack expose the
 * same DM entry symbol and ship under the fixed name libbt_bluetooth.z.so. */
const char *GetStackLibraryName()
{
    return "libbt_bluetooth.z.so";
}

using GetInterfaceFn = bt_interface_t *(*)();

void LoadBluetoothLibrary()
{
    HILOGI("enter");
    const char *libName = GetStackLibraryName();
    void *handle = dlopen(libName, RTLD_NOW);
    if (handle == nullptr) {
        HILOGE("Failed to dlopen %{public}s: %{public}s", libName, dlerror());
        g_loadResult = -1;
        return;
    }
    GetInterfaceFn getter = reinterpret_cast<GetInterfaceFn>(dlsym(handle, BT_DM_GET_INTERFACE_SYMBOL));
    if (getter == nullptr) {
        HILOGE("Failed to dlsym %{public}s from %{public}s: %{public}s", BT_DM_GET_INTERFACE_SYMBOL, libName,
            dlerror());
        g_loadResult = -1;
        return;
    }
    g_btInterface = getter();
    if (g_btInterface == nullptr) {
        HILOGE("%{public}s returned a null bt_interface_t", libName);
        g_loadResult = -1;
        return;
    }
    /* resolve the link-time bridged stack symbols from the same library */
    g_btLibraryHandle = handle;
    stack_interface_bind(handle);
    HILOGI("Bluetooth stack loaded: %{public}s", libName);
    g_loadResult = 0;
}
}  // namespace

int hal_util_load_bt_library(const bt_interface_t **btInterface)
{
    HILOGI("hal_util_load_bt_library enter");
    if (btInterface == nullptr) {
        HILOGI("hal_util_load_bt_library btInterface is nullptr");
        return -1;
    }
    std::call_once(g_loadOnceFlag, LoadBluetoothLibrary);
    *btInterface = g_btInterface;
    return g_loadResult;
}