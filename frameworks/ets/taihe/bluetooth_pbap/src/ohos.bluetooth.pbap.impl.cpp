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
#define LOG_TAG "bt_pbap_impl_bluetooth"
#endif

#include "ohos.bluetooth.pbap.proj.hpp"
#include "ohos.bluetooth.pbap.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_errorcode.h"
#include "bluetooth_pbap_pse.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_pbap_observer.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_event_module.h"
#include "taihe_ha_event_utils.h"

namespace OHOS {
namespace Bluetooth {

class PbapServerProfileImpl {
public:
    PbapServerProfileImpl()
    {
        observer_ = std::make_shared<TaiheBluetoothPbapObserver>();
        PbapPse *profile = PbapPse::GetProfile();
        profile->RegisterObserver(observer_);
    }

    ::ohos::bluetooth::constant::AccessAuthorization GetPhoneBookAccessAuthorizationSync(
        ::taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "pbap.GetPhoneBookAccessAuthorization", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ::ohos::bluetooth::constant::AccessAuthorization::from_value(0));

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t accessAuthorization = 0;
        int32_t errorCode = profile->GetPhoneBookAccessAuthorization(remoteDevice, accessAuthorization);
        HILOGI("errorCode: %{public}d, accessAuthorization: %{public}d", errorCode, accessAuthorization);
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode,
            ::ohos::bluetooth::constant::AccessAuthorization::from_value(accessAuthorization));
        return ::ohos::bluetooth::constant::AccessAuthorization::from_value(accessAuthorization);
    }

    ::ohos::bluetooth::pbap::ShareType GetShareTypeSync(::taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "pbap.GetShareType", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ::ohos::bluetooth::pbap::ShareType::from_value(0));

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t shareType = 0;
        int32_t errorCode = profile->GetShareType(remoteDevice, shareType);
        HILOGI("errorCode: %{public}d, shareType: %{public}d", errorCode, shareType);
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode,
            ::ohos::bluetooth::pbap::ShareType::from_value(shareType));
        return ::ohos::bluetooth::pbap::ShareType::from_value(shareType);
    }

    void SetShareTypeSync(::taihe::string_view deviceId, ::ohos::bluetooth::pbap::ShareType type)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "pbap.SetShareType", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr) && IsShareTypeValid(type.get_value());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->SetShareType(remoteDevice, type.get_value());
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void SetPhoneBookAccessAuthorizationSync(::taihe::string_view deviceId,
        ::ohos::bluetooth::constant::AccessAuthorization authorization)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "pbap.SetPhoneBookAccessAuthorization", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr) && IsAccessAuthorizationValid(authorization.get_value());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->SetPhoneBookAccessAuthorization(remoteDevice, authorization.get_value());
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void Disconnect(::taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "pbap.Disconnect", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->Disconnect(remoteDevice);
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    ::ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(::taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ::ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

        PbapPse *profile = PbapPse::GetProfile();
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
        PbapPse *profile = PbapPse::GetProfile();
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
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ::ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t strategy = 0;
        int32_t errorCode = profile->GetConnectionStrategy(remoteDevice, strategy);
        HILOGI("errorCode: %{public}d, strategy: %{public}d", errorCode, strategy);
        TAIHE_BT_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode,
            ::ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy));
        return ::ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    void SetConnectionStrategySync(::taihe::string_view deviceId,
        ::ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        int32_t connStrategy = strategy.get_value();
        bool checkRet = CheckSetConnectStrategyParam(remoteAddr, connStrategy);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->SetConnectionStrategy(remoteDevice, connStrategy);
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->connectionStateChange_.RegisterEvent(callback);
        }
    }

    void OffConnectionStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->connectionStateChange_.DeregisterEvent(callback);
        }
    }

private:
    std::shared_ptr<TaiheBluetoothPbapObserver> observer_ = nullptr;
};

::ohos::bluetooth::pbap::PbapServerProfile CreatePbapServerProfile()
{
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "pbap.CreatePbapServerProfile", {});
    return taihe::make_holder<PbapServerProfileImpl, ::ohos::bluetooth::pbap::PbapServerProfile>();
}

}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreatePbapServerProfile(OHOS::Bluetooth::CreatePbapServerProfile);
// NOLINTEND
