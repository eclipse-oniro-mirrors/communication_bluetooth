/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef DATA_SHARE_HELPER_IMPL_TEST_H
#define DATA_SHARE_HELPER_IMPL_TEST_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>
#include <cstdint>
#include <optional>
#include "datashare_helper.h"

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace OHOS::DataShare;
class DataShareHelperImplTest : public DataShare::DataShareHelper {
public:
    DataShareHelperImplTest()
    {}

    ~DataShareHelperImplTest() override
    {}
    bool Release() override
    {
        return false;
    }

    std::vector<std::string> GetFileTypes(Uri &uri, const string &mimeTypeFilter) override
    {
        std::vector<std::string> res;
        return res;
    }

    int OpenFile(Uri &uri, const string &mode) override
    {
        return 0;
    }

    int OpenFileWithErrCode(Uri &uri, const string &mode, int32_t &errCode) override
    {
        return 0;
    }

    int OpenRawFile(Uri &uri, const string &mode) override
    {
        return 0;
    }

    int Insert(Uri &uri, const DataShare::DataShareValuesBucket &value) override
    {
        return 0;
    }

    int InsertExt(Uri &uri, const DataShare::DataShareValuesBucket &value, std::string &result) override
    {
        return 0;
    }

    int Update(Uri &uri, const DataShare::DataSharePredicates &predicates,
        const DataShare::DataShareValuesBucket &value) override
    {
        return 0;
    }

    int Delete(Uri &uri, const DataShare::DataSharePredicates &predicates) override
    {
        return 0;
    }

    std::shared_ptr<DataShare::DataShareResultSet> Query(Uri &uri, const DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns, DataShare::DatashareBusinessError *businessError) override
    {
        std::shared_ptr<DataShare::DataShareResultSet> res = std::make_shared<DataShare::DataShareResultSet>();
        return res;
    }

    std::string GetType(Uri &uri) override
    {
        std::string res;
        return res;
    }

    int BatchInsert(Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values) override
    {
        return 0;
    }

    int ExecuteBatch(const std::vector<DataShare::OperationStatement> &statements,
    DataShare::ExecResultSet &result) override
    {
        return 0;
    }

    int BatchUpdate(const DataShare::UpdateOperations &operations,
        std::vector<DataShare::BatchUpdateResult> &results) override
    {
        return 0;
    }

    int RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override
    {
        return 0;
    }

    int UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override
    {
        return 0;
    }

    void NotifyChange(const Uri &uri) override
    {}

    int RegisterObserverExtProvider(const Uri &uri, const std::shared_ptr<DataShareObserver> dataObserver,
        bool isDescendants) override
    {
        return 0;
    }

    int UnregisterObserverExtProvider(const Uri &uri,
        const std::shared_ptr<DataShareObserver> dataObserver) override
    {
        return 0;
    }

    void NotifyChangeExtProvider(const DataShareObserver::ChangeInfo &changeInfo) override
    {}

    Uri NormalizeUri(Uri &uri) override
    {
        return uri;
    }

    Uri DenormalizeUri(Uri &uri) override
    {
        return uri;
    }

    int AddQueryTemplate(const string &uri, int64_t subscriberId, DataShare::Template &tpl) override
    {
        return 0;
    }

    int DelQueryTemplate(const string &uri, int64_t subscriberId) override
    {
        return 0;
    }

    std::vector<DataShare::OperationResult> Publish(const DataShare::Data &data, const string &bundleName) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }

    DataShare::Data GetPublishedData(const string &bundleName, int &resultCode) override
    {
        DataShare::Data data;
        return data;
    }

    std::vector<DataShare::OperationResult> SubscribeRdbData(const std::vector<std::string> &uris,
    const DataShare::TemplateId &templateId,
        const std::function<void(const DataShare::RdbChangeNode &)> &callback) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }

    std::vector<DataShare::OperationResult> UnsubscribeRdbData(const std::vector<std::string> &uris,
        const DataShare::TemplateId &templateId) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }

    std::vector<DataShare::OperationResult> EnableRdbSubs(const std::vector<std::string> &uris,
        const DataShare::TemplateId &templateId) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }

    std::vector<DataShare::OperationResult> DisableRdbSubs(const std::vector<std::string> &uris,
        const DataShare::TemplateId &templateId) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }

    std::vector<DataShare::OperationResult> SubscribePublishedData(const std::vector<std::string> &uris,
    int64_t subscriberId,
        const std::function<void(const DataShare::PublishedDataChangeNode &)> &callback) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }

    std::vector<DataShare::OperationResult> UnsubscribePublishedData(const std::vector<std::string> &uris,
        int64_t subscriberId) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }

    std::vector<DataShare::OperationResult> EnablePubSubs(const std::vector<std::string> &uris,
    int64_t subscriberId) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }

    std::vector<DataShare::OperationResult> DisablePubSubs(const std::vector<std::string> &uris,
    int64_t subscriberId) override
    {
        std::vector<DataShare::OperationResult> res;
        return res;
    }
};
}  // namespace bluetooth
}  // namespace OHOS
#endif /* DATA_SHARE_HELPER_IMPL_TEST_H */