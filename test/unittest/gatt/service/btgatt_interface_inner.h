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

#ifndef BTGATT_INTERFACE_INNER_H
#define BTGATT_INTERFACE_INNER_H

#include <functional>
#include "btgatt_server_interface_mock.h"
#include "btgatt_client_interface_mock.h"

extern "C" {
extern void RegisterGattServerInterfaceMock(std::function<void(const btgatt_server_interface_t *interface)> func,
    OHOS::bluetooth::BtgattServerInterfaceMock *interfaceMock);
extern void UnRegisterGattServerInterfaceMock(void);

extern void RegisterGattClientInterfaceMock(std::function<void(const btgatt_client_interface_t *interface)> func,
    OHOS::bluetooth::BtgattClientInterfaceMock *interfaceMock);
extern void UnRegisterGattClientInterfaceMock(void);

}  // extern "C"

#endif  // BTGATT_INTERFACE_INNER_H