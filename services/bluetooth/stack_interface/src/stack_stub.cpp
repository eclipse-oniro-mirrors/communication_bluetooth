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

/*
 * Bridge for the stack symbols the open-source service layer references at
 * link time. The real definitions live in the closed-source stack library
 * (libbt_bluetooth.z.so); they are resolved with dlsym when the library is
 * loaded by hal_util (see stack_interface_bind) and forwarded from here.
 */

#ifndef LOG_TAG
#define LOG_TAG "bt_stack_interface"
#endif

#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD000102
#endif

#include <cstring>
#include <dlfcn.h>
#include <mutex>
#include <shared_mutex>

#include "a2dp_snk_audio_render.h"
#include "hardware/bt_gatt.h"
#include "hal_util.h"
#include "btif_sock.h"
#include "btif_storage.h"
#include "btif_dm.h"
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

using GetGattClientInterfaceFn = const btgatt_client_interface_t* (*)(void);
using GetGattServerInterfaceFn = const btgatt_server_interface_t* (*)(void);
using GetBleAdvertiserInstanceFn = BleAdvertiserInterface* (*)(void);
using GetBleScannerInstanceFn = BleScannerInterface* (*)(void);
using BtifGetAddressTypeFn = bool (*)(const RawAddress&, tBLE_ADDR_TYPE*);
using BtifStorageGetRemoteAddrTypeFn = bt_status_t (*)(const RawAddress*, uint8_t*);
using BtifSockCleanupFn = void (*)(void);
using A2dpAudioRenderSetInstanceFn = void (*)(std::shared_ptr<OHOS::bluetooth::IA2dpAudioRender>);

/*
 * 符号缓存表：写锁只在解析/重新绑定时持有，运行期一律读写锁的读锁取指针。
 * 取到的指针在释放读锁之后再调用，锁不跨越栈调用，避免栈内重入与写者等待。
 */
struct StackSymbolTable {
    GetBleAdvertiserInstanceFn getBleAdvertiserInstance = nullptr;
    GetBleScannerInstanceFn getBleScannerInstance = nullptr;
    BtifGetAddressTypeFn btifGetAddressType = nullptr;
    BtifStorageGetRemoteAddrTypeFn btifStorageGetRemoteAddrType = nullptr;
    BtifSockCleanupFn btifSockCleanup = nullptr;
    A2dpAudioRenderSetInstanceFn a2dpAudioRenderSetInstance = nullptr;
    GetGattClientInterfaceFn gattClientInterface = nullptr;
    GetGattServerInterfaceFn gattServerInterface = nullptr;
};

std::shared_mutex g_symbolsMutex;
StackSymbolTable g_symbols;
bool g_symbolsResolved = false;

void* Resolve(void* handle, const char* name)
{
    if (handle == nullptr) {
        return nullptr;
    }
    return dlsym(handle, name);
}

/* 调用方必须已持有写锁 */
template <typename Fn>
bool BindSymbol(void* handle, Fn& slot, const char* name)
{
    void* symbol = Resolve(handle, name);
    if (symbol == nullptr) {
        HILOGE("BindSymbol(%{public}s) failed", name);
    } else {
        HILOGI("BindSymbol(%{public}s) success", name);
    }
    slot = reinterpret_cast<Fn>(symbol);
    return slot != nullptr;
}

/* 锁外解析：结果写入调用方提供的局部表，不触碰任何共享状态 */
void ResolveAllSymbols(void* handle, StackSymbolTable& table)
{
    BindSymbol(handle, table.getBleAdvertiserInstance, "get_ble_advertiser_instance");
    BindSymbol(handle, table.getBleScannerInstance, "get_ble_scanner_instance");
    BindSymbol(handle, table.btifGetAddressType, "btif_get_address_type");
    BindSymbol(handle, table.btifStorageGetRemoteAddrType, "btif_storage_get_remote_addr_type");
    BindSymbol(handle, table.btifSockCleanup, "btif_sock_cleanup");
    BindSymbol(handle, table.a2dpAudioRenderSetInstance, "A2dpAudioRenderSetInstance");
    BindSymbol(handle, table.gattClientInterface, "get_gatt_client_interface");
    BindSymbol(handle, table.gattServerInterface, "get_gatt_server_interface");
}

/*
 * 取缓存：只读，不解析。符号只在 stack_interface_bind 中解析并发布，
 * 调用路径上不再出现任何 dlsym / 加载器锁动作。
 */
template <typename Fn>
Fn GetSymbol(Fn StackSymbolTable::*member)
{
    std::shared_lock<std::shared_mutex> lock(g_symbolsMutex);
    return g_symbolsResolved ? g_symbols.*member : nullptr;
}

}  // namespace

extern "C" {

const btgatt_client_interface_t* BtGetGattClientInterface(void)
{
    auto fn = GetSymbol(&StackSymbolTable::gattClientInterface);
    return fn != nullptr ? fn() : nullptr;
}

const btgatt_server_interface_t* BtGetGattServerInterface(void)
{
    auto fn = GetSymbol(&StackSymbolTable::gattServerInterface);
    return fn != nullptr ? fn() : nullptr;
}

BleAdvertiserInterface* BtGetBleAdvertiserInstance(void)
{
    auto fn = GetSymbol(&StackSymbolTable::getBleAdvertiserInstance);
    return fn != nullptr ? fn() : nullptr;
}

BleScannerInterface* BtGetBleScannerInstance(void)
{
    auto fn = GetSymbol(&StackSymbolTable::getBleScannerInstance);
    return fn != nullptr ? fn() : nullptr;
}

bool BtGetAddressType(const RawAddress& bda, tBLE_ADDR_TYPE* p_addr_type)
{
    auto fn = GetSymbol(&StackSymbolTable::btifGetAddressType);
    if (fn == nullptr) {
        if (p_addr_type != nullptr) {
            *p_addr_type = BLE_ADDR_ANONYMOUS;
        }
        return false;
    }
    return fn(bda, p_addr_type);
}

}  // extern "C"

bt_status_t BtifStorageGetRemoteAddrType(const RawAddress* remote_bd_addr,
                                              uint8_t* p_remote_addr_type)
{
    auto fn = GetSymbol(&StackSymbolTable::btifStorageGetRemoteAddrType);
    if (fn == nullptr) {
        if (p_remote_addr_type != nullptr) {
            *p_remote_addr_type = 0;
        }
        return BT_STATUS_FAIL;
    }
    return fn(remote_bd_addr, p_remote_addr_type);
}

void BtifSockCleanup(void)
{
    auto fn = GetSymbol(&StackSymbolTable::btifSockCleanup);
    if (fn != nullptr) {
        fn();
    }
}

/*
 * 由 hal_util 在 dlopen 成功后调用：锁外解析全部符号，再锁内一次性发布。
 * 禁止在栈库构造函数/初始化回调（加载器锁未释放）中调用。
 * 注：btgattClientInterface / btgattServerInterface 不在此处理，
 * 它们由 stack_adapter 的 bt_gatt.h 定义、AdapterGattInit() 填充。
 */
void stack_interface_bind(void* handle)
{
    HILOGI("stack_interface_bind enter");
    if (handle == nullptr) {
        return;
    }

    StackSymbolTable fresh;
    ResolveAllSymbols(handle, fresh);

    std::unique_lock<std::shared_mutex> lock(g_symbolsMutex);
    g_symbols = fresh;
    g_symbolsResolved = true;
}

void BtA2dpAudioRendersetInstance(std::shared_ptr<OHOS::bluetooth::IA2dpAudioRender> instance)
{
    auto fn = GetSymbol(&StackSymbolTable::a2dpAudioRenderSetInstance);
    if (fn == nullptr) {
        HILOGE("dlsym a2dpAudioRenderSetInstance failed");
        return;
    }
    return fn(instance);
}