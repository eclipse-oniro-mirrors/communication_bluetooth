/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef DATA_SHARE_HELPER_H
#define DATA_SHARE_HELPER_H

#include <string>

#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "iservice_registry.h"
#include "uri.h"

namespace OHOS {
namespace DataShare {
constexpr int32_t E_OK = 0;

class DataShareObserver {
public:
    DataShareObserver() = default;
    virtual ~DataShareObserver() = default;
    class ChangeInfo {
    };

    virtual void OnChange(const ChangeInfo &changeInfo) = 0;
};

class DatashareBusinessError {};

class DataShareHelper {
public:
    DataShareHelper() = default;
    ~DataShareHelper() = default;

    static std::shared_ptr<DataShareHelper> Creator(
        const sptr<IRemoteObject> &token, const std::string &strUri, const std::string &extUri = "")
    {
        return nullptr;
    }
    std::shared_ptr<DataShareResultSet> Query(Uri &uri, const DataSharePredicates &predicates,
        std::vector<std::string> &columns, DatashareBusinessError *businessError = nullptr)
    {
        return nullptr;
    }

    bool Release() {
        return true;
    }
};

}
}
#endif  // DATA_SHARE_HELPER_H
