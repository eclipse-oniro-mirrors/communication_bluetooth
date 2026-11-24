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

#ifndef BT_CALLER_IDENTITY_H
#define BT_CALLER_IDENTITY_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {

/**
 * @brief 单次 IPC 请求内的调用方身份快照。
 *
 * 在 IPC 入口 Capture() 一次，沿同步调用链显式传递，避免重复读取 IPCSkeleton
 * 与 AccessTokenKit。pid/uid/tokenId 在 Capture 时固定；fullTokenId 与系统 HAP
 * 判定按需计算并缓存，同一请求内最多查询一次。
 *
 * 使用约束：
 * 1. 仅在当前 binder 线程的同步链路内使用；
 * 2. 异步任务必须显式拷贝已读取的字段，禁止在工作线程内重新 Capture()；
 * 3. 跨请求不得复用（binder 线程会被复用，残留会串号）。
 */
struct CallerIdentity {
    int32_t pid = 0;
    int32_t uid = 0;
    uint32_t tokenId = 0;

    static CallerIdentity Capture();
    uint64_t GetFullTokenId() const;
    bool IsSystemHap() const;

private:
    mutable uint64_t fullTokenId_ = 0;
    mutable int8_t fullTokenFlag_ = -1;
    mutable int8_t systemHapFlag_ = -1;
};

} // namespace bluetooth
} // namespace OHOS

#endif // BT_CALLER_IDENTITY_H
