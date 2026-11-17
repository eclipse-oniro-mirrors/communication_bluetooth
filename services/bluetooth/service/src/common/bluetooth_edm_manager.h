/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#ifndef BLUETOOTH_ALLOWEDLIST_MANAGER_H
#define BLUETOOTH_ALLOWEDLIST_MANAGER_H

#include <string>
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>
#include <set>
#include <map>
#include <shared_mutex>
#include <chrono>
#include "raw_address.h"
#include "bluetooth_common_event_subscriber.h"
#include "control_intercept_plugin.h"

typedef struct cJSON cJSON;

namespace OHOS {
namespace bluetooth {
/**
 * @brief  Real address convert func: convert an external address (real or virtual) to real address.
 * @return Returns true if the real address is obtained, otherwise returns false.
 */
using EdmRealAddrConvertFunc = std::function<bool(const RawAddress &addr, RawAddress &realAddr)>;

/**
 * @brief  Represents bluetooth allowedList manager.
 */
class BluetoothEdmManager {
public:
    /**
     * @brief Get bluetooth allowedList manager singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 6
     */
    static BluetoothEdmManager& GetInstance();
    void Init();
    bool IsAllowedConnect(const std::string &mac);
    bool IsAccountConfigAllowedConnect(const ControlInterceptMessage &msg, const std::string &profile);
    bool IsAccountConfigAllowedReceive(const ControlInterceptMessage &msg, const std::string &profile);
    void OnEdmListChanged(const std::string &action);
    void RegisterRealAddrConvertFunc(const EdmRealAddrConvertFunc &func);
    void ReportEdmAuditInfo(const std::string &connectedDeviceId, int appId, int userId,
        const std::string &filePath = "");
private:
    BluetoothEdmManager();
    ~BluetoothEdmManager();
    static std::mutex instanceMutexLock_;

    std::shared_ptr<BluetoothCommonEventSubscriber> deviceEventSubscriber_ = nullptr;
    std::shared_ptr<BluetoothCommonEventSubscriber> accountDisallowedEventSubscriber_ = nullptr;

    std::shared_mutex allowedListMutexLock_;
    // Key is the original address got from EDM, value is its real address.
    std::map<std::string, std::string> allowedListMap_;
    // Real addresses of allowedListMap_, used for O(log n) lookup.
    std::set<std::string> realAllowedAddrSet_;
    int GetAllowedList();

    std::shared_mutex disallowedListMutexLock_;
    // Key is the original address got from EDM, value is its real address.
    std::map<std::string, std::string> disallowedListMap_;
    // Real addresses of disallowedListMap_, used for O(log n) lookup.
    std::set<std::string> realDisallowedAddrSet_;
    int GetDisallowedList();

    std::shared_mutex accountDenylistMutexLock_;
    std::map<std::string, std::vector<std::string>> accountDenylist_;
    int GetAccountSendDenylist();
    int GetAccountRecvDenylist();

    std::shared_mutex accountRecvDenylistMutexLock_;
    std::map<std::string, std::vector<std::string>> accountRecvDenylist_;

    int GetAccountDenylistInternal(std::shared_mutex& mutexLock,
        std::map<std::string, std::vector<std::string>>& denylist, const char* jsonKey);
    int ParseAccountDenylist(cJSON *obj, std::map<std::string, std::vector<std::string>> &denylist);

    bool IsValidAddress(const std::string& address);
    bool ConvertToRealAddress(const std::string &mac, std::string &realMac);
    void UpdateListMap(std::map<std::string, std::string> &listMap, std::set<std::string> &realAddrSet,
        const std::vector<std::string> &addrList);
    static bool IsAddrInList(const std::map<std::string, std::string> &listMap,
        const std::set<std::string> &realAddrSet, const std::string &addr);
    bool IsAccountConfigAllowedInternal(const ControlInterceptMessage &msg, const std::string &profile,
        std::shared_mutex &mutexLock, std::map<std::string, std::vector<std::string>> &denylist);

    std::mutex convertFuncMutexLock_;
    EdmRealAddrConvertFunc realAddrConvertFunc_ = nullptr;
    std::atomic_bool isInit_ = false;
};
} // namespace bluetooth
} // namespace OHOS

#endif // BLUETOOTH_ALLOWEDLIST_MANAGER_H