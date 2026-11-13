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

#ifndef POWER_INTERFACE_H
#define POWER_INTERFACE_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace bluetooth {
/*
 * 与 power_manager 原生 PowerMgr::PowerErrors 对齐（跨 so 边界无法直接引用原生枚举）。
 */
namespace PowerErrors {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_FAILURE = 1;
}  // namespace PowerErrors

/*
 * 桥接接口：仅透传 power_manager 的亮屏状态查询与 RunningLock 操作。
 * 每套接口在 service 内维护一把锁：通用套（OH_Power*）供 AdapterManager（原生 RUNNINGLOCK_BACKGROUND），
 * 手表套（OH_WATCH_Power*）供 WatchService（原生 RUNNINGLOCK_BACKGROUND_TASK）。
 * vendor_interface 模块经 dlopen/dlsym 获取符号后直接调用，
 * 签名须与 service 仓 bt_vendor/include/power_interface.h 保持同步。
 */
#ifdef __cplusplus
extern "C" {
#endif
/* Generic interface: used by adapter manager. Lock object is kept for reuse. */
bool OH_PowerIsScreenOn();
int32_t OH_PowerHoldRunningLock(const char *name);
int32_t OH_PowerReleaseRunningLock();
bool OH_PowerIsRunningLockUsed();

/* Watch interface: fixed background task lock, release destroys the lock object. */
int32_t OH_WATCH_PowerHoldRunningLock(const char *name, int32_t timeoutMs);
int32_t OH_WATCH_PowerReleaseRunningLock();
#ifdef __cplusplus
}
#endif

/*
 * power 通用封装层：供 AdapterManager 与 IsScreenOn 调用方使用，锁对象由 service 保留复用。
 * 库不可用（未加载/符号缺失）时 IsScreenOn 返回 false，其余返回失败码。
 */
class PowerInterface {
public:
    static bool IsScreenOn();
    static int32_t HoldRunningLock(const std::string &name);
    static int32_t ReleaseRunningLock();
    static bool IsRunningLockUsed();
};

/*
 * 手表专有封装层：锁类型固定为后台任务锁，释放时 service 侧销毁锁对象。
 */
class WatchPowerInterface {
public:
    static int32_t HoldRunningLock(const std::string &name, int32_t timeoutMs);
    static int32_t ReleaseRunningLock();
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // POWER_INTERFACE_H