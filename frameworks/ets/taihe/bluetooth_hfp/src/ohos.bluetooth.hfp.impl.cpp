/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_hfp_impl_ohbluetooth"
#endif

#include "ohos.bluetooth.hfp.proj.hpp"
#include "ohos.bluetooth.hfp.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_errorcode.h"
#include "bluetooth_hfp_ag.h"
#include "bluetooth_hfp_hf.h"
#include "bluetooth_log.h"
#include "taihe_bluetooth_hfp_ag_observer.h"
#include "taihe_bluetooth_hfp_hf_observer.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_ha_event_utils.h"

namespace OHOS {
namespace Bluetooth {
class HandsFreeAudioGatewayProfileImpl {
public:
    HandsFreeAudioGatewayProfileImpl()
    {
        observer_ = std::make_shared<TaiheHandsFreeAudioGatewayObserver>();
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
        if (!isRegistered_) {
            HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
            profile->RegisterObserver(observer_);
            isRegistered_ = true;
        }
    }

    void OffConnectionStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }

    void Connect(taihe::string_view deviceId)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void Disconnect(taihe::string_view deviceId)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        int32_t profileState = TaiheUtils::GetProfileConnectionState(state);
        TAIHE_BT_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState));

        return ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    taihe::array<taihe::string> GetConnectedDevices()
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetConnectedDevices(devices);
        HILOGI("errorCode:%{public}s, devices size:%{public}zu", GetTaiheErrMsg(errorCode).c_str(), devices.size());
        TAIHE_BT_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode, taihe::array<taihe::string>{});

        std::vector<std::string> deviceVector;
        for (auto &device: devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<::taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));

        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d, strategy: %{public}d", err, strategy);
        TAIHE_BT_ASSERT_RETURN_VERIFY(err == BT_NO_ERROR, err,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy));

        return ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    void SetConnectionStrategySync(taihe::string_view deviceId,
                                   ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGD("start");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        int32_t connStrategy = strategy.get_value();
        bool checkRet = CheckSetConnectStrategyParam(remoteAddr, connStrategy);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, connStrategy);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(err == BT_NO_ERROR, err);
    }
private:
    std::shared_ptr<TaiheHandsFreeAudioGatewayObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

::ohos::bluetooth::hfp::HandsFreeAudioGatewayProfile CreateHfpAgProfile()
{
    return taihe::make_holder<HandsFreeAudioGatewayProfileImpl, ohos::bluetooth::hfp::HandsFreeAudioGatewayProfile>();
}

class HandsFreeHfProfileImpl {
public:
    HandsFreeHfProfileImpl()
    {
        observer_ = std::make_shared<TaiheHandsFreeUnitObserver>();
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
        if (!isRegistered_) {
            HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
            profile->RegisterObserver(observer_);
            isRegistered_ = true;
        }
    }

    void OffConnectionStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }

    void Connect(taihe::string_view deviceId)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hfp.Connect", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void Disconnect(taihe::string_view deviceId)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hfp.Disconnect", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

        HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = profile->GetDeviceState(device);
        int32_t profileState = TaiheUtils::GetProfileConnectionState(state);
        return ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    taihe::array<taihe::string> GetConnectedDevices()
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
        std::vector<int> states = { static_cast<int>(BTConnectState::CONNECTED) };
        std::vector<BluetoothRemoteDevice> devices = profile->GetDevicesByStates(states);

        std::vector<std::string> deviceVector;
        for (auto &device : devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<::taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));

        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d, strategy: %{public}d", err, strategy);
        TAIHE_BT_ASSERT_RETURN_VERIFY(err == BT_NO_ERROR, err,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy));

        return ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    void SetConnectionStrategySync(taihe::string_view deviceId,
                                   ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGD("start");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        int32_t connStrategy = strategy.get_value();
        bool checkRet = CheckSetConnectStrategyParam(remoteAddr, connStrategy);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, connStrategy);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(err == BT_NO_ERROR, err);
    }

private:
    std::shared_ptr<TaiheHandsFreeUnitObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

::ohos::bluetooth::hfp::HandsFreeHfProfile CreateHfpHfProfile()
{
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hfp.CreateHfpHfProfile", {});
    return taihe::make_holder<HandsFreeHfProfileImpl, ohos::bluetooth::hfp::HandsFreeHfProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateHfpAgProfile(OHOS::Bluetooth::CreateHfpAgProfile);
TH_EXPORT_CPP_API_CreateHfpHfProfile(OHOS::Bluetooth::CreateHfpHfProfile);
// NOLINTEND
