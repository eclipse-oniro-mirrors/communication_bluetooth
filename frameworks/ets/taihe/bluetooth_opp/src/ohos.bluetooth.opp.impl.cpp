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
#define LOG_TAG "bt_opp_impl_bluetooth"
#endif

#include "ohos.bluetooth.opp.proj.hpp"
#include "ohos.bluetooth.opp.impl.hpp"
#include "taihe/runtime.hpp"
#include "bluetooth_opp.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_opp_observer.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_event_module.h"
#include "taihe_ha_event_utils.h"

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

class OppServerProfileImpl {
public:
    OppServerProfileImpl()
    {
        observer_ = std::make_shared<TaiheBluetoothOppObserver>();
        Opp *profile = Opp::GetProfile();
        profile->RegisterObserver(observer_);
    }

    void SetIncomingFileConfirmationSync(bool accept, int32_t fileFd)
    {
        HILOGI("enter, fileFd: %{public}d", fileFd);
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR, BT_ERR_TRANSFER_BUSY,
                                               BT_ERR_FILE_NOT_ACCESSIBLE };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "opp.SetIncomingFileConfirmation", validErrCodes);
        Opp *profile = Opp::GetProfile();
        int32_t errorCode = profile->SetIncomingFileConfirmation(accept, fileFd);
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void SetLastReceivedFileUriSync(::taihe::string_view uri)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_PROHIBITED_BY_EDM, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "opp.SetLastReceivedFileUri", validErrCodes);
        std::string uriStr = std::string(uri);
        Opp *profile = Opp::GetProfile();
        int32_t errorCode = profile->SetLastReceivedFileUri(uriStr);
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    ::ohos::bluetooth::opp::OppTransferInformation GetCurrentTransferInformationSync()
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_PROHIBITED_BY_EDM, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR,
                                               BT_ERR_TRANSFER_INFORMATION_EMPTY };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "opp.GetCurrentTransferInformation", validErrCodes);
        Opp *profile = Opp::GetProfile();
        BluetoothOppTransferInformation information;
        int32_t errorCode = profile->GetCurrentTransferInformation(information);
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode,
            (::ohos::bluetooth::opp::OppTransferInformation{
                ::taihe::string(""), ::taihe::string(""), ::taihe::string(""),
                ::ohos::bluetooth::opp::DirectionType::from_value(0),
                ::ohos::bluetooth::opp::TransferStatus::from_value(0),
                ::ohos::bluetooth::opp::TransferResult::from_value(0),
                0, 0, 0, 0,
            }));
        return ConvertTransferInformation(information);
    }

    void CancelTransferSync()
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_PROHIBITED_BY_EDM, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR, BT_ERR_TRANSFER_BUSY };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "opp.CancelTransfer", validErrCodes);
        Opp *profile = Opp::GetProfile();
        bool result = false;
        int32_t errorCode = profile->CancelTransfer(result);
        HILOGI("errorCode: %{public}d, result: %{public}d", errorCode, result);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void OnTransferStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::opp::OppTransferInformation const& data)> callback)
    {
        if (observer_) {
            observer_->transferStateChange_.RegisterEvent(callback);
        }
    }

    void OffTransferStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::opp::OppTransferInformation const& data)>> callback)
    {
        if (observer_) {
            observer_->transferStateChange_.DeregisterEvent(callback);
        }
    }

    void OnReceiveIncomingFile(::taihe::callback_view<void(
        ::ohos::bluetooth::opp::OppTransferInformation const& data)> callback)
    {
        if (observer_) {
            observer_->receiveIncomingFile_.RegisterEvent(callback);
        }
    }

    void OffReceiveIncomingFile(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::opp::OppTransferInformation const& data)>> callback)
    {
        if (observer_) {
            observer_->receiveIncomingFile_.DeregisterEvent(callback);
        }
    }

    ::ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(::taihe::string_view deviceId)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ::ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

        Opp *profile = Opp::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        int32_t profileState = TaiheUtils::GetProfileConnectionState(state);
        HILOGI("errorCode: %{public}d, profileState: %{public}d", errorCode, profileState);
        TAIHE_BT_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode,
            ::ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState));
        return ::ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    ::taihe::array<::taihe::string> GetConnectedDevices()
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        Opp *profile = Opp::GetProfile();
        std::vector<int32_t> states = {static_cast<int32_t>(BTConnectState::CONNECTED)};
        std::vector<BluetoothRemoteDevice> devices;
        int32_t errorCode = profile->GetDevicesByStates(states, devices);
        HILOGI("errorCode: %{public}d, devices size: %{public}zu", errorCode, devices.size());
        TAIHE_BT_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode, taihe::array<::taihe::string>{});

        std::vector<std::string> deviceVector;
        for (auto &device : devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }
        return deviceVector.empty() ? taihe::array<::taihe::string>{}
            : taihe::array<::taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    ::ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(::taihe::string_view deviceId)
    {
        HILOGW("OPP does not support GetConnectionStrategy");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        TAIHE_BT_ASSERT_RETURN_VERIFY(false, BT_ERR_API_NOT_SUPPORT,
            ::ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));
    }

    void SetConnectionStrategySync(::taihe::string_view deviceId,
        ::ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGW("OPP does not support SetConnectionStrategy");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(false, BT_ERR_API_NOT_SUPPORT);
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& info)> callback)
    {
        HILOGW("OPP does not support OnConnectionStateChange");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(false, BT_ERR_API_NOT_SUPPORT);
    }

    void OffConnectionStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& info)>> callback)
    {
        HILOGW("OPP does not support OffConnectionStateChange");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(false, BT_ERR_API_NOT_SUPPORT);
    }

private:
    std::shared_ptr<TaiheBluetoothOppObserver> observer_ = nullptr;
};

::ohos::bluetooth::opp::OppServerProfile CreateOppServerProfile()
{
    return taihe::make_holder<OppServerProfileImpl, ::ohos::bluetooth::opp::OppServerProfile>();
}

}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateOppServerProfile(OHOS::Bluetooth::CreateOppServerProfile);
// NOLINTEND
