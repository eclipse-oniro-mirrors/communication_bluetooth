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
#include "hw_bt_watch_interface.h"

namespace bluetooth {
namespace watch {

bt_status_t g_status = BT_STATUS_FAIL;
bool g_isPhoneMajorClass = false;
bt_status_t g_sendReportResult = BT_STATUS_FAIL;
void SetStatus(bt_status_t status)
{
    g_status = status;
}

bt_status_t InitMock(bthd_callbacks_t* callbacks)
{
    return g_status;
}

bt_status_t RegisterAppMock(bthd_app_param_t* app_param, bthd_qos_param_t* in_qos, bthd_qos_param_t* out_qos)
{
    return g_status;
}

bt_status_t UnregisterAppMock()
{
    return g_status;
}

bt_status_t ConnectMock(RawAddress* bdAddr)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t DisconnectMock()
{
    return BT_STATUS_SUCCESS;
}

bt_status_t initWatchMock(BtWatchCallbacks* callbacks)
{
    return g_status;
}

bt_status_t sendTbsfMock(const RawAddress* bdAddr)
{
    return g_status;
}
void sendTbsrMock(const RawAddress& bdAddr, uint16_t value)
{}

void sendScreenOn2McuMock(void)
{}

void sendHfpState2HisiMock(const RawAddress &bdAddr, bool isStartConnect)
{}

void sendBtIsOnMock(bool isOn)
{}

void updateSendReportResult(bt_status_t ret)
{
    g_sendReportResult = ret;
}

bt_status_t SendReportMock(bthd_report_type_t type, uint8_t id, uint16_t len, uint8_t* p_data)
{
    return g_sendReportResult;
}

void updatePhoneMajorClass(bool isMajorClass)
{
    g_isPhoneMajorClass = isMajorClass;
}

bool IsPhoneMajorClassMock(const RawAddress& bdAddr)
{
    return g_isPhoneMajorClass;
}

void updateDeviceMock(const RawAddress& bdAddr, bool isActiveDevice)
{}
void updateReconnectStateMock(const RawAddress& bdAddr, bool isAllowConnect)
{}

}  // namespace watch
}  // namespace bluetooth
#endif  // INTERFACE_MOCK_H