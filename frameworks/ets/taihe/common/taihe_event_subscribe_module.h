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

#ifndef TAIHE_EVENT_SUBSCRIBE_MODULE_H
#define TAIHE_EVENT_SUBSCRIBE_MODULE_H

#include <algorithm>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "bluetooth_log.h"
#include "taihe/callback.hpp"
#include "taihe/optional.hpp"
#include "taihe_event_module.h"

namespace OHOS {
namespace Bluetooth {

class EventModuleBase {
public:
    virtual ~EventModuleBase() = default;
};

template<typename T>
class EventModuleWrapper : public EventModuleBase {
public:
    EventModule<T> module;
};

class TaiheEventSubscribeModule {
public:
    TaiheEventSubscribeModule(const char *validEventName, const char *moduleName)
        : validEventNameVec_(std::vector<std::string>{validEventName}), moduleName_(moduleName)
    {}
    TaiheEventSubscribeModule(std::vector<std::string> validEventNameVec, const char *moduleName)
        : validEventNameVec_(validEventNameVec), moduleName_(moduleName)
    {}

    template<typename T>
    void RegisterEvent(std::string name, ::taihe::callback_view<T> callback)
    {
        std::unique_lock<std::shared_mutex> guard(lock_);
        if (!IsValidEventName(name)) {
            HILOGE("invalid event name, module: %{public}s, event: %{public}s",
                moduleName_.c_str(), name.c_str());
            return;
        }
        GetOrCreateModule<T>(name).RegisterEvent(callback);
    }

    template<typename T>
    void DeregisterEvent(std::string name, ::taihe::optional_view<::taihe::callback<T>> callback)
    {
        std::unique_lock<std::shared_mutex> guard(lock_);
        if (!IsValidEventName(name)) {
            HILOGE("invalid event name, module: %{public}s, event: %{public}s",
                moduleName_.c_str(), name.c_str());
            return;
        }
        auto it = eventModules_.find(name);
        if (it == eventModules_.end()) {
            return;
        }
        if (it->second.typeTag != GetTypeTag<T>()) {
            HILOGE("event module type mismatch, module: %{public}s, event: %{public}s",
                moduleName_.c_str(), name.c_str());
            return;
        }
        static_cast<EventModuleWrapper<T> *>(it->second.wrapper.get())->module.DeregisterEvent(callback);
    }

    void DeregisterAllCallback(std::string name)
    {
        std::unique_lock<std::shared_mutex> guard(lock_);
        if (!IsValidEventName(name)) {
            HILOGE("invalid event name, module: %{public}s, event: %{public}s",
                moduleName_.c_str(), name.c_str());
            return;
        }
        eventModules_.erase(name);
    }

    template<typename T>
    EventModule<T> &GetModule(std::string name)
    {
        std::unique_lock<std::shared_mutex> guard(lock_);
        if (!IsValidEventName(name)) {
            HILOGE("invalid event name, module: %{public}s, event: %{public}s",
                moduleName_.c_str(), name.c_str());
            return GetDefaultModule<T>();
        }
        // The reference stays valid only while no concurrent DeregisterAllCallback
        // erases the module from eventModules_; all current call sites guarantee
        // this by deregistering on the same thread that publishes the event.
        // A type-mismatched name also falls back to the inert default module.
        return GetOrCreateModule<T>(name);
    }

    bool IsValidEventName(const std::string &eventName) const
    {
        return std::find(validEventNameVec_.begin(), validEventNameVec_.end(), eventName) !=
            validEventNameVec_.end();
    }

private:
    // Address of a per-instantiation static variable is used as the type tag.
    // It identifies the callback type without RTTI (no std::type_index/typeid).
    template<typename T>
    static const void *GetTypeTag(void)
    {
        static char typeTag;
        return &typeTag;
    }

    struct EventModuleEntry {
        const void *typeTag = nullptr;
        std::shared_ptr<EventModuleBase> wrapper;
    };

    template<typename T>
    EventModule<T> &GetOrCreateModule(std::string name)
    {
        auto it = eventModules_.find(name);
        if (it != eventModules_.end()) {
            if (it->second.typeTag != GetTypeTag<T>()) {
                HILOGE("event module type mismatch, module: %{public}s, event: %{public}s",
                    moduleName_.c_str(), name.c_str());
                return GetDefaultModule<T>();
            }
            return static_cast<EventModuleWrapper<T> *>(it->second.wrapper.get())->module;
        }
        auto wrapper = std::make_shared<EventModuleWrapper<T>>();
        auto &ref = wrapper->module;
        eventModules_[name] = EventModuleEntry{GetTypeTag<T>(), std::move(wrapper)};
        return ref;
    }

    template<typename T>
    static EventModule<T> &GetDefaultModule(void)
    {
        static EventModule<T> defaultModule;
        return defaultModule;
    }

    std::map<std::string, EventModuleEntry> eventModules_;
    std::vector<std::string> validEventNameVec_;
    std::string moduleName_;
    std::shared_mutex lock_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_EVENT_SUBSCRIBE_MODULE_H
