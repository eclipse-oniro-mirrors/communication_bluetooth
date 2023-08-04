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
#include "napi_bluetooth_base_profile.h"

#include "bluetooth_log.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
napi_value NapiBaseProfile::DefineBaseProfileJSFunction(napi_env env, napi_value exports)
{
    HILOGI("start");
    BaseProfilePropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    HILOGI("end");
    return exports;
}

napi_value NapiBaseProfile::BaseProfilePropertyValueInit(napi_env env, napi_value exports)
{
    HILOGI("start");
    napi_value strategyObj = ConnectionStrategyInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("ConnectionStrategy", strategyObj),
    };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    HILOGI("end");
    return exports;
}

napi_value NapiBaseProfile::ConnectionStrategyInit(napi_env env)
{
    HILOGI("enter");
    napi_value strategyObj = nullptr;
    napi_create_object(env, &strategyObj);
    SetNamedPropertyByInteger(env, strategyObj, ConnectionStrategy::CONNECTION_UNKNOWN,
        "CONNECT_STRATEGY_UNSUPPORTED");
    SetNamedPropertyByInteger(env, strategyObj, ConnectionStrategy::CONNECTION_ALLOWED,
        "CONNECT_STRATEGY_ALLOWED");
    SetNamedPropertyByInteger(env, strategyObj, ConnectionStrategy::CONNECTION_FORBIDDEN,
        "CONNECT_STRATEGY_FORBIDDEN");
    return strategyObj;
}
}  // namespace Bluetooth
}  // namespace OHOS