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

#ifndef BTIF_DM_OH_H
#define BTIF_DM_OH_H

#include "types/ble_address_with_type.h"

#ifdef __cplusplus
extern "C" {
#endif

bool BtGetAddressType(const RawAddress& bda, tBLE_ADDR_TYPE* p_addr_type);

#ifdef __cplusplus
}
#endif

#endif  // BTIF_DM_OH_H
