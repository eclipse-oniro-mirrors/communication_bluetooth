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

#ifndef TAIHE_BLUETOOTH_ERROR_H_
#define TAIHE_BLUETOOTH_ERROR_H_

#include <cstdint>
#include <memory>
#include <string>
#include <optional>

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "taihe_ha_event_utils.h"

namespace OHOS {
namespace Bluetooth {

// 基础错误处理宏：只做错误映射与抛出，不进行 HA 上报，供不使用 HA 打点的代码使用。
#ifndef TAIHE_BT_ASSERT_RETURN
#define TAIHE_BT_ASSERT_RETURN(cond, errCode, retObj)                          \
do {                                                                           \
    if (!(cond)) {                                                             \
        HandleSyncErr((errCode));                                              \
        HILOGE("bluetoothManager taihe assert failed.");                       \
        return (retObj);                                                       \
    }                                                                          \
} while (0)
#endif

#ifndef TAIHE_BT_ASSERT_RETURN_VOID
#define TAIHE_BT_ASSERT_RETURN_VOID(cond, errCode)                             \
do {                                                                           \
    if (!(cond)) {                                                             \
        HandleSyncErr((errCode));                                              \
        HILOGE("bluetooth taihe assert failed.");                              \
        return;                                                                \
    }                                                                          \
} while (0)
#endif

// verify error code
#define TAIHE_BT_ASSERT_RETURN_VERIFY(cond, errCode, retObj)                 \
do {                                                                         \
    std::vector<int32_t> validErrCodes = apiContext.validErrCodes;           \
    if (!(cond)) {                                                           \
        HandleSyncErrAdapter((errCode), validErrCodes);                      \
        return (retObj);                                                     \
    }                                                                        \
} while (0)

#define TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(cond, errCode)                    \
do {                                                                         \
    std::vector<int32_t> validErrCodes = apiContext.validErrCodes;           \
    if (!(cond)) {                                                           \
        HandleSyncErrAdapter((errCode), validErrCodes);                      \
        return;                                                              \
    }                                                                        \
} while (0)

struct ApiContext {
    std::shared_ptr<TaiheHaEventUtils> haUtils = nullptr;
    std::vector<int32_t> validErrCodes {};
};

// 非 HA 上下文宏：不创建 haUtils，供不使用 HA 打点的模块使用，对应 NAPI_BT_CONTEXT_WITHOUT_HA。
#ifndef TAIHE_BT_CONTEXT_WITHOUT_HA
#define TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes)     \
ApiContext apiContext = ApiContext{                    \
    nullptr,                                           \
    validErrCodes                                      \
}
#endif

// HA 上下文宏：一键创建带 haUtils 的 ApiContext，对齐 NAPI_BT_CONTEXT。
#ifndef TAIHE_BT_CONTEXT_HA
#define TAIHE_BT_CONTEXT_HA(env, apiName, validErrCodes)   \
ApiContext apiContext = ApiContext{                        \
    std::make_shared<TaiheHaEventUtils>(env, apiName),     \
    validErrCodes                                          \
}
#endif

// 异步工作宏：基于 apiContext 创建异步对象（同时携带 haUtils 与 validErrCodes），
// 异步错误结算时经 ProcessErrCode 做 inner→business 映射，对齐 CREATE_ASYNC_WORK_WITH_CONTEXT。
#ifndef CREATE_ASYNC_WORK_WITH_CONTEXT
#define CREATE_ASYNC_WORK_WITH_CONTEXT(env, info, func)                            \
    TaiheAsyncWorkFactory::CreateAsyncWork(env, info, func, apiContext)
#endif

// HA 打点宏：失败时先经 ProcessErrCode 做 inner→business 映射（对齐 NAPI 上报处理后的 business errCode），
// 再由 HandleSyncErrWithHaReport 通过 apiContext.haUtils 记录错误码，haUtils 析构时统一上报 api_exec_end。
#define TAIHE_BT_HA_ASSERT_RETURN_VERIFY(cond, failCode, retObj)           \
do {                                                                       \
    if (!(cond)) {                                                         \
        HandleSyncErrWithHaReport((failCode), apiContext);                 \
        return (retObj);                                                   \
    }                                                                      \
} while (0)

#define TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(cond, failCode)              \
do {                                                                       \
    if (!(cond)) {                                                         \
        HandleSyncErrWithHaReport((failCode), apiContext);                 \
        return;                                                            \
    }                                                                      \
} while (0)

std::string GetTaiheErrMsg(const int32_t errCode);
void HandleSyncErr(int32_t errCode);
void HandleSyncErrWithValidCodes(int32_t errCode, const std::vector<int32_t> &validErrCodes);
void HandleSyncErrAdapter(int32_t errCode, std::vector<int32_t> &validErrCodes);
bool IsInnerErrorCode(int32_t errCode);

struct ErrInfo {
    int32_t errCode;
    std::string errMsg;
};
void ConvertInnerToBusinessErrCode(int32_t innerCode, ErrInfo &info);
ErrInfo ProcessErrCode(int32_t originalCode, const std::vector<int32_t> &validErrCodes);

// 上报 HA 错误码并处理同步错误：先经 ProcessErrCode 做 inner→business 映射，再经 haUtils 记录错误码。
inline void HandleSyncErrWithHaReport(int32_t failCode, const ApiContext &apiContext)
{
    std::vector<int32_t> validErrCodes = apiContext.validErrCodes;
    auto processResult = ProcessErrCode(failCode, validErrCodes);
    if (apiContext.haUtils != nullptr) {
        apiContext.haUtils->WriteErrCode(processResult.errCode);
    }
    HandleSyncErrAdapter(failCode, validErrCodes);
}

struct TaihePromiseAndCallback {
    bool success;
    BtErrCode errorCode;
    std::optional<uintptr_t> object;

    static TaihePromiseAndCallback Success(uintptr_t object = 0)
    {
        return {true, BtErrCode::BT_NO_ERROR, object};
    }

    static TaihePromiseAndCallback Failure(BtErrCode code)
    {
        return {false, code, std::nullopt};
    }
};
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_ERROR_H_
