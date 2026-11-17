/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef INTERFACE_MOCK_H
#define INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include "bt_hf.h"
#include "raw_address.h"
#include "bluetooth_log.h"

namespace bluetooth {
namespace headset {

bt_status_t connect(RawAddress* bd_addr)
{
    HILOGI("connect enter");
    return BT_STATUS_SUCCESS;
}

bt_status_t disconnect(RawAddress* bd_addr)
{
    HILOGI("disconnect enter");
    return BT_STATUS_SUCCESS;
}

bt_status_t virtual_unplug(RawAddress* bd_addr)
{
    HILOGI("virtual_unplug enter");
    return BT_STATUS_FAIL;
}

bt_status_t set_report(RawAddress* bd_addr, bthh_report_type_t reportType, char* report)
{
    HILOGI("set_report enter");
    return BT_STATUS_FAIL;
}

bt_status_t get_report(RawAddress* bd_addr, bthh_report_type_t reportType, uint8_t reportId, int bufferSize)
{
    HILOGI("get_report enter");
    return BT_STATUS_FAIL;
}

}  // namespace headset
}  // namespace bluetooth
#endif  // INTERFACE_MOCK_H