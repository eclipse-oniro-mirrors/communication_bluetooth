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

#ifndef DATASHARE_PREDICATES_H
#define DATASHARE_PREDICATES_H

#include <string>
#include <variant>
#include <vector>

namespace OHOS {
namespace DataShare {
class SingleValue {
public:
    /**
    * @brief Use Type replace variant namespace.
    */
    using Type = std::variant<std::monostate, int, int64_t, double, std::string, bool>;
    Type value;

    SingleValue() = default;
    ~SingleValue() = default;
    SingleValue(Type val) noexcept : value(std::move(val)) {}
    SingleValue(int val) : value(val) {}
    SingleValue(int64_t val) : value(val) {}
    SingleValue(double val) : value(val) {}
    SingleValue(bool val) : value(val) {}
    SingleValue(std::string val) : value(std::move(val)) {}
};

class MutliValue {
};

class OperationItem {
};

class DataSharePredicates {
public:
    DataSharePredicates *EqualTo(const std::string &field, const SingleValue &value)
    {
        return this;
    }
    DataSharePredicates *NotEqualTo(const std::string &field, const SingleValue &value)
    {
        return this;
    }
    DataSharePredicates *GreaterThan(const std::string &field, const SingleValue &value)
    {
        return this;
    }
    DataSharePredicates *LessThan(const std::string &field, const SingleValue &value)
    {
        return this;
    }
    DataSharePredicates *GreaterThanOrEqualTo(const std::string &field, const SingleValue &value)
    {
        return this;
    }
    DataSharePredicates *LessThanOrEqualTo(const std::string &field, const SingleValue &value)
    {
        return this;
    }
    DataSharePredicates *In(const std::string &field, const MutliValue &values)
    {
        return this;
    }
    DataSharePredicates *NotIn(const std::string &field, const MutliValue &values)
    {
        return this;
    }
    DataSharePredicates *BeginWrap()
    {
        return this;
    }
    DataSharePredicates *EndWrap()
    {
        return this;
    }
    DataSharePredicates *Or()
    {
        return this;
    }
    DataSharePredicates *And()
    {
        return this;
    }
    DataSharePredicates *Contains(const std::string &field, const std::string &value)
    {
        return this;
    }
    DataSharePredicates *BeginsWith(const std::string &field, const std::string &value)
    {
        return this;
    }
    DataSharePredicates *EndsWith(const std::string &field, const std::string &value)
    {
        return this;
    }
    DataSharePredicates *IsNull(const std::string &field)
    {
        return this;
    }
    DataSharePredicates *IsNotNull(const std::string &field)
    {
        return this;
    }
    DataSharePredicates *Like(const std::string &field, const std::string &value)
    {
        return this;
    }
    DataSharePredicates *Unlike(const std::string &field, const std::string &value)
    {
        return this;
    }
    DataSharePredicates *Glob(const std::string &field, const std::string &value)
    {
        return this;
    }
    DataSharePredicates *Between(const std::string &field, const std::string &low, const std::string &high)
    {
        return this;
    }
    DataSharePredicates *NotBetween(const std::string &field, const std::string &low, const std::string &high)
    {
        return this;
    }
    DataSharePredicates *OrderByAsc(const std::string &field)
    {
        return this;
    }
    DataSharePredicates *OrderByDesc(const std::string &field)
    {
        return this;
    }
    DataSharePredicates *Distinct()
    {
        return this;
    }
    DataSharePredicates *Limit(const int number, const int offset)
    {
        return this;
    }
    DataSharePredicates *GroupBy(const std::vector<std::string> &fields)
    {
        return this;
    }
    DataSharePredicates *IndexedBy(const std::string &indexName)
    {
        return this;
    }
    DataSharePredicates *KeyPrefix(const std::string &prefix)
    {
        return this;
    }
    DataSharePredicates *InKeys(const std::vector<std::string> &keys)
    {
        return this;
    }
    DataSharePredicates *CrossJoin(const std::string &tableName)
    {
        return this;
    }
    DataSharePredicates *InnerJoin(const std::string &tableName)
    {
        return this;
    }
    DataSharePredicates *LeftOuterJoin(const std::string &tableName)
    {
        return this;
    }
    DataSharePredicates *Using(const std::vector<std::string> &fields)
    {
        return this;
    }
    DataSharePredicates *On(const std::vector<std::string> &fields)
    {
        return this;
    }
    const std::vector<OperationItem> &GetOperationList() const
    {
        return items_;
    }
    std::string GetWhereClause() const
    {
        return "";
    }
    int SetWhereClause(const std::string &whereClause)
    {
        return 0;
    }
    std::vector<std::string> GetWhereArgs() const
    {
        return {};
    }
    int SetWhereArgs(const std::vector<std::string> &whereArgs)
    {
        return 0;
    }
    std::string GetOrder() const
    {
        return "";
    }
    int SetOrder(const std::string &order)
    {
        return 0;
    }

private:
    std::vector<OperationItem> items_ {};
};

}
}
#endif  // DATASHARE_PREDICATES_H
