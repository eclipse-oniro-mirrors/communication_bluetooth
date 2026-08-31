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

#ifndef TAIHE_EVENT_MODULE_H
#define TAIHE_EVENT_MODULE_H

#include <shared_mutex>
#include <vector>

#include "bluetooth_log.h"
#include "stdexcept"
#include "taihe/callback.hpp"
#include "taihe/optional.hpp"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {
// Upper bound of callbacks registered on a single EventModule, guards against
// unbounded growth when the subscriber leaks its registration.
constexpr size_t MAX_EVENT_CALLBACK_COUNT = 50;

template<typename T>
class EventModule {
public:
    void RegisterEvent(::taihe::callback_view<T> callback);
    void DeregisterEvent(::taihe::optional_view<::taihe::callback<T>> callback);
    template<typename Arg>
    void PublishEvent(const Arg& arg);

    std::vector<::taihe::optional<::taihe::callback<T>>> callbackVec_;
    std::shared_mutex lock_;
};

class TaiheUtils {
public:
    static int GetProfileConnectionState(int state);
};

template<typename T>
void EventModule<T>::RegisterEvent(::taihe::callback_view<T> callback)
{
    std::unique_lock<std::shared_mutex> guard(lock_);
    auto eventCb = ::taihe::optional<::taihe::callback<T>>{std::in_place_t{}, callback};
    if (std::find(callbackVec_.begin(), callbackVec_.end(), eventCb) != callbackVec_.end()) {
        return;
    }
    if (callbackVec_.size() >= MAX_EVENT_CALLBACK_COUNT) {
        HILOGE("register event callback exceeds the upper bound %{public}zu", MAX_EVENT_CALLBACK_COUNT);
        return;
    }
    callbackVec_.emplace_back(eventCb);
}

template<typename T>
void EventModule<T>::DeregisterEvent(::taihe::optional_view<::taihe::callback<T>> callback)
{
    std::unique_lock<std::shared_mutex> guard(lock_);
    if (callback.has_value()) {
        callbackVec_.erase(
            std::remove_if(callbackVec_.begin(), callbackVec_.end(),
                [callback](const ::taihe::optional<::taihe::callback<T>>& it) {
                    return it == callback;
                }),
            callbackVec_.end());
    } else {
        callbackVec_.clear();
    }
}

template<typename T>
template<typename Arg>
void EventModule<T>::PublishEvent(const Arg& arg)
{
    // Copy the callbacks out of the lock, the callback may re-enter
    // RegisterEvent/DeregisterEvent which requires the unique lock.
    std::vector<::taihe::optional<::taihe::callback<T>>> callbacks;
    {
        std::unique_lock<std::shared_mutex> guard(lock_);
        callbacks = callbackVec_;
    }
    for (auto& cb : callbacks) {
        if (cb.has_value()) {
            cb.value()(arg);
        }
    }
}
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_EVENT_MODULE_H
