/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_ASYNC_WORK_H
#define TAIHE_ASYNC_WORK_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

#include "taihe_bluetooth_utils.h"
#include "taihe_ha_event_utils.h"
#include "taihe_native_object.h"

namespace OHOS {
namespace Bluetooth {
struct TaiheAsyncCallback;

enum TaiheAsyncType : int {
    GATT_CLIENT_READ_CHARACTER,
    GATT_CLIENT_READ_REMOTE_RSSI_VALUE,
    GATT_CLIENT_READ_DESCRIPTOR,
    GATT_CLIENT_WRITE_CHARACTER,
    GATT_CLIENT_WRITE_DESCRIPTOR,
    GATT_CLIENT_ENABLE_CHARACTER_CHANGED,
    GATT_SERVER_NOTIFY_CHARACTERISTIC,
    GET_ADVERTISING_HANDLE,
    BLE_STOP_ADVERTISING,
    BLE_START_SCAN,
    BLE_STOP_SCAN,
    BLE_ENABLE_ADVERTISING,
    BLE_DISABLE_ADVERTISING,
    BAS_GET_BATTERY_LEVEL,
    GATT_CLIENT_READ_PHY,
    GATT_CLIENT_SET_PHY,
    GATT_CLIENT_WRITE_CHARACTER_WITH_CONTEXT,
    GATT_CLIENT_MTU_CHANGED,
    GATT_CLIENT_UPDATE_CONNECTION_PRIORITY,
    GATT_SERVER_SET_PHY,
    GATT_SERVER_READ_PHY,
    GENERATE_LOCAL_OOB_DATA
};

struct TaiheAsyncWorkRet {
    explicit TaiheAsyncWorkRet(int errCode) : errCode(errCode) {}
    TaiheAsyncWorkRet(int errCode, std::shared_ptr<TaiheNativeObject> object)
        : errCode(errCode), object(std::move(object)) {}

    int errCode = -1;
    std::shared_ptr<TaiheNativeObject> object = nullptr;
};

class TaiheAsyncWork : public std::enable_shared_from_this<TaiheAsyncWork> {
public:
    TaiheAsyncWork(ani_env *env, std::function<TaiheAsyncWorkRet(void)> func,
        std::shared_ptr<TaiheAsyncCallback> asyncCallback,
        std::shared_ptr<TaiheHaEventUtils> haUtils = nullptr)
        : env_(env), func_(func), taiheAsyncCallback_(asyncCallback), haUtils_(haUtils) {}
    TaiheAsyncWork(ani_env *env, std::function<TaiheAsyncWorkRet(void)> func,
        std::shared_ptr<TaiheAsyncCallback> asyncCallback, ApiContext apiContext)
        : env_(env), func_(func), taiheAsyncCallback_(asyncCallback),
          haUtils_(apiContext.haUtils), validErrCodes_(apiContext.validErrCodes) {}
    ~TaiheAsyncWork() = default;

    void Run(void);
    void CallFunction(int errCode, std::shared_ptr<TaiheNativeObject> object);
    ani_object GetRet(void);
    std::shared_ptr<TaiheHaEventUtils> GetHaUtilsPtr(void) const;
    // Sets whether the asynchronous work waits for a business callback to settle.
    // No-response GATT writes have no server response, so they are settled
    // directly in Complete() by the protocol stack return value.
    void SetNeedCallback(bool needCallback) { needCallback_.store(needCallback); }
    bool IsCallbackWork(void) const;

    struct Info {
        void Execute(void);
        void Complete(void);

        int errCode = -1;
        bool needCallback = true;
        std::shared_ptr<TaiheNativeObject> object;
        std::shared_ptr<TaiheAsyncWork> taiheAsyncWork = nullptr;
    };

private:
    friend class TaiheAsyncWorkMap;

    void TimeoutCallback(void);

    ani_env *env_ = nullptr;
    std::atomic<uint32_t> timerId_ {0};  // Is used to reference a timer.
    std::function<TaiheAsyncWorkRet(void)> func_;
    std::shared_ptr<TaiheAsyncCallback> taiheAsyncCallback_ = nullptr;
    std::atomic_bool triggered_ = false; // Indicates whether the asynchronous callback is called.
    std::atomic_bool needCallback_ = true; // Indicates whether to wait for the business callback to settle.
    std::shared_ptr<TaiheHaEventUtils> haUtils_; // HA report tool, which is transferred from the original API interface
    std::vector<int32_t> validErrCodes_ {}; // Indicates valid error codes.
};

class TaiheAsyncWorkFactory {
public:
    static std::shared_ptr<TaiheAsyncWork> CreateAsyncWork(ani_env *env, ani_object info,
        std::function<TaiheAsyncWorkRet(void)> asyncWork, std::shared_ptr<TaiheHaEventUtils> haUtils = nullptr);
    static std::shared_ptr<TaiheAsyncWork> CreateAsyncWork(ani_env *env, ani_object info,
        std::function<TaiheAsyncWorkRet(void)> asyncWork, ApiContext apiContext);
};

class TaiheAsyncWorkMap {
public:
    bool TryPush(TaiheAsyncType type, std::shared_ptr<TaiheAsyncWork> asyncWork);
    void Erase(TaiheAsyncType type);
    std::shared_ptr<TaiheAsyncWork> Get(TaiheAsyncType type);

private:
    mutable std::mutex mutex_ {};
    std::map<int, std::shared_ptr<TaiheAsyncWork>> map_ {};
};

void AsyncWorkCallFunction(TaiheAsyncWorkMap &map, TaiheAsyncType type, std::shared_ptr<TaiheNativeObject> nativeObject,
    int status);

void TaiheAsyncWorkSettleIntResult(TaiheAsyncWorkMap &map, TaiheAsyncType type, int result);

// Settles an async work with a result object: callback form receives the object
// (same as napi), promise form receives undefined (nullptr, converted by
// TaiheAsyncWork::CallFunction), since arkts Promise<void> rejects a non-undefined
// resolve value.
void TaiheAsyncWorkSettleWithObject(TaiheAsyncWorkMap &map, TaiheAsyncType type,
    std::shared_ptr<TaiheNativeObject> object, int result);

}  // namespace Bluetooth
}  // namespace OHOS
#endif // TAIHE_ASYNC_WORK_H