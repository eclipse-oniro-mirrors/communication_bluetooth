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

#ifndef NAPI_NATIVE_OBJECT_H
#define NAPI_NATIVE_OBJECT_H

#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
class NapiNativeObject {
public:
    virtual ~NapiNativeObject() = default;
    virtual napi_value ToNapiValue(napi_env env) const = 0;
};

class NapiNativeEmpty : public NapiNativeObject {
public:
    ~NapiNativeEmpty() override = default;

    napi_value ToNapiValue(napi_env env) const override
    {
        return NapiGetNull(env);
    }
};

class NapiNativeInt : public NapiNativeObject {
public:
    explicit NapiNativeInt(int value) : value_(value) {}
    ~NapiNativeInt() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    int value_;
};

class NapiNativeString : public NapiNativeObject {
public:
    explicit NapiNativeString(std::string value) : value_(std::move(value)) {}
    ~NapiNativeString() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::string value_;
};

class NapiNativeUuidsArray : public NapiNativeObject {
public:
    NapiNativeUuidsArray(const std::vector<std::string> uuids) : uuids_(uuids) {}
    ~NapiNativeUuidsArray() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::vector<std::string> uuids_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_NATIVE_OBJECT_H