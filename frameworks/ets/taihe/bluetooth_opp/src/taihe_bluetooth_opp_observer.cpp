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

#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_opp_observer"
#endif

#include "ohos.bluetooth.opp.proj.hpp"
#include "ohos.bluetooth.opp.impl.hpp"
#include "bluetooth_log.h"
#include "taihe_bluetooth_opp_observer.h"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {

namespace {
::ohos::bluetooth::opp::OppTransferInformation ConvertTransferInformation(
    const BluetoothOppTransferInformation &info)
{
    return ::ohos::bluetooth::opp::OppTransferInformation{
        ::taihe::string(info.GetFilePath()),
        ::taihe::string(info.GetDeviceName()),
        ::taihe::string(info.GetDeviceAddress()),
        ::ohos::bluetooth::opp::DirectionType::from_value(info.GetDirection()),
        ::ohos::bluetooth::opp::TransferStatus::from_value(info.GetStatus()),
        ::ohos::bluetooth::opp::TransferResult::from_value(info.GetResult()),
        static_cast<int64_t>(info.GetCurrentBytes()),
        static_cast<int64_t>(info.GetTotalBytes()),
        info.GetCurrentCount(),
        info.GetTotalCount(),
    };
}
}  // namespace

TaiheBluetoothOppObserver::TaiheBluetoothOppObserver()
{}

void TaiheBluetoothOppObserver::OnReceiveIncomingFileChanged(
    const BluetoothOppTransferInformation &transferInformation)
{
    HILOGI("enter");
    auto taiheResult = ConvertTransferInformation(transferInformation);
    receiveIncomingFile_.PublishEvent(taiheResult);
}

void TaiheBluetoothOppObserver::OnTransferStateChanged(
    const BluetoothOppTransferInformation &transferInformation)
{
    HILOGI("enter");
    auto taiheResult = ConvertTransferInformation(transferInformation);
    transferStateChange_.PublishEvent(taiheResult);
}

}  // namespace Bluetooth
}  // namespace OHOS
