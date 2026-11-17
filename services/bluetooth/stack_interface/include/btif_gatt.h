/******************************************************************************
 *
 *  Copyright (c) 2021-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/*****************************************************************************
 *
 *  Name:          btif_gatt.h
 *
 *  Description:
 *
 *****************************************************************************/

#pragma once

#include "hardware/bt_gatt.h"

/* Interface tables bridged from the stack library via dlsym
 * (see stack_interface_bind); filled before the profiles start. */
extern btgatt_client_interface_t btgattClientInterface;
extern btgatt_server_interface_t btgattServerInterface;

extern "C"
const btgatt_client_interface_t* BtGetGattClientInterface(void);
extern "C"
const btgatt_server_interface_t* BtGetGattServerInterface(void);