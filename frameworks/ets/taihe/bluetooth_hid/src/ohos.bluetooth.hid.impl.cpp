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
#define LOG_TAG "bt_hid_impl_ohbluetooth"
#endif

#include "ohos.bluetooth.hid.proj.hpp"
#include "ohos.bluetooth.hid.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_errorcode.h"
#include "bluetooth_hid_host.h"
#include "bluetooth_hid_device.h"
#include "bluetooth_address_info.h"
#include "bluetooth_log.h"
#include "taihe_bluetooth_hid_host_observer.h"
#include "taihe_bluetooth_hid_device_observer.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_ha_event_utils.h"
#include "hid_device_utils.h"

namespace OHOS {
namespace Bluetooth {
class HidHostProfileImpl {
public:
    HidHostProfileImpl()
    {
        observer_ = std::make_shared<TaiheBluetoothHidHostObserver>();
        HidHost *profile = HidHost::GetProfile();
        profile->RegisterObserver(observer_);
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
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
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void Disconnect(taihe::string_view deviceId)
    {
        HILOGI("Disconnect called");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        int32_t profileState = TaiheUtils::GetProfileConnectionState(state);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState));

        return ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    taihe::array<taihe::string> GetConnectedDevices()
    {
        HILOGI("enter");
        HidHost *profile = HidHost::GetProfile();
        std::vector<int> states = { static_cast<int>(BTConnectState::CONNECTED) };
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetDevicesByStates(states, devices);
        HILOGI("errorCode:%{public}s, devices size:%{public}zu", GetTaiheErrMsg(errorCode).c_str(), devices.size());
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode, taihe::array<taihe::string>{});

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
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HidHost *profile = HidHost::GetProfile();
        int strategy = 0;
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d, strategy: %{public}d", err, strategy);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy));

        return ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    void SetConnectionStrategySync(taihe::string_view deviceId,
                                   ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGD("start");
        std::string remoteAddr = std::string(deviceId);
        int32_t connStrategy = strategy.get_value();
        bool checkRet = CheckSetConnectStrategyParam(remoteAddr, connStrategy);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HidHost *profile = HidHost::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, connStrategy);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }
private:
    std::shared_ptr<TaiheBluetoothHidHostObserver> observer_ = nullptr;
};

::ohos::bluetooth::hid::HidHostProfile CreateHidHostProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return taihe::make_holder<HidHostProfileImpl, ::ohos::bluetooth::hid::HidHostProfile>();
}

static bool IsValidHidDeviceSdp(const ::ohos::bluetooth::hid::HidDeviceSdp& sdp)
{
    if (!IsValidSDPSettingParam(std::string(sdp.name), MAX_NAME_SIZE)) {
        return false;
    }
    if (!IsValidSDPSettingParam(std::string(sdp.description), MAX_DESCRIPTION_SIZE)) {
        return false;
    }
    if (!IsValidSDPSettingParam(std::string(sdp.provider), MAX_PROVIDER_SIZE)) {
        return false;
    }
    if (!IsValidSubclass(sdp.subclass.get_value())) {
        return false;
    }
    if (sdp.descriptors.size() == 0 || sdp.descriptors.size() > MAX_DESCRIPTOR_SIZE) {
        return false;
    }
    return true;
}

static bool IsValidHidDeviceQos(const ::ohos::bluetooth::hid::HidDeviceQos& qos)
{
    if (qos.serviceType.has_value()) {
        if (!IsValidServiceType(qos.serviceType.value().get_value())) {
            return false;
        }
    }
    return true;
}

static BluetoothHidDeviceSdp ConvertHidDeviceSdp(const ::ohos::bluetooth::hid::HidDeviceSdp& sdp)
{
    return BluetoothHidDeviceSdp(bluetooth::SdpSetting(
        std::string(sdp.name), std::string(sdp.description), std::string(sdp.provider),
        sdp.subclass.get_value(),
        std::vector<uint8_t>(sdp.descriptors.data(), sdp.descriptors.data() + sdp.descriptors.size())));
}

static BluetoothHidDeviceQos ConvertHidDeviceQos(const ::ohos::bluetooth::hid::HidDeviceQos& qos)
{
    int32_t serviceType = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    if (qos.serviceType.has_value()) {
        serviceType = static_cast<int32_t>(qos.serviceType.value().get_value());
    }
    return BluetoothHidDeviceQos(bluetooth::QosSetting(serviceType, qos.tokenRate.value_or(0),
        qos.tokenBucketSize.value_or(0), qos.peakBandwidth.value_or(0),
        qos.latency.value_or(-1), qos.delayVariation.value_or(-1)));
}

std::shared_ptr<TaiheBluetoothHidDeviceObserver> g_hidDeviceObserver =
    std::make_shared<TaiheBluetoothHidDeviceObserver>();

class HidDeviceProfileImpl {
public:
    HidDeviceProfileImpl()
    {
        observer_ = g_hidDeviceObserver;
        HidDevice *profile = HidDevice::GetProfile();
        profile->RegisterObserver(g_hidDeviceObserver);
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->connectionStateChangeEventModule_.RegisterEvent(callback);
        }
    }

    void OffConnectionStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->connectionStateChangeEventModule_.DeregisterEvent(callback);
        }
    }

    void OnSetProtocol(::taihe::callback_view<void(
        ::ohos::bluetooth::hid::ProtocolData const& data)> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OnSetProtocol", {});
        if (observer_) {
            observer_->setProtocolEventModule_.RegisterEvent(callback);
        }
    }

    void OffSetProtocol(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::hid::ProtocolData const& data)>> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OffSetProtocol", {});
        if (observer_) {
            observer_->setProtocolEventModule_.DeregisterEvent(callback);
        }
    }

    void OnInterruptDataReceived(::taihe::callback_view<void(
        ::ohos::bluetooth::hid::InterruptData const& data)> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OnInterruptDataReceived", {});
        if (observer_) {
            observer_->interruptDataReceivedEventModule_.RegisterEvent(callback);
        }
    }

    void OffInterruptDataReceived(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::hid::InterruptData const& data)>> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OffInterruptDataReceived", {});
        if (observer_) {
            observer_->interruptDataReceivedEventModule_.DeregisterEvent(callback);
        }
    }

    void OnSetReport(::taihe::callback_view<void(
        ::ohos::bluetooth::hid::SetReportData const& data)> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OnSetReport", {});
        if (observer_) {
            observer_->setReportEventModule_.RegisterEvent(callback);
        }
    }

    void OffSetReport(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::hid::SetReportData const& data)>> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OffSetReport", {});
        if (observer_) {
            observer_->setReportEventModule_.DeregisterEvent(callback);
        }
    }

    void OnGetReport(::taihe::callback_view<void(
        ::ohos::bluetooth::hid::GetReportData const& data)> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OnGetReport", {});
        if (observer_) {
            observer_->getReportEventModule_.RegisterEvent(callback);
        }
    }

    void OffGetReport(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::hid::GetReportData const& data)>> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OffGetReport", {});
        if (observer_) {
            observer_->getReportEventModule_.DeregisterEvent(callback);
        }
    }

    void OnVirtualCableUnplug(::taihe::callback_view<void(::ohos::bluetooth::hid::UndefinedType const& data)> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OnVirtualCableUnplug", {});
        if (observer_) {
            observer_->virtualCableUnplugEventModule_.RegisterEvent(callback);
        }
    }

    void OffVirtualCableUnplug(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::hid::UndefinedType const& data)>> callback)
    {
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.OffVirtualCableUnplug", {});
        if (observer_) {
            observer_->virtualCableUnplugEventModule_.DeregisterEvent(callback);
        }
    }

    void Connect(::ohos::bluetooth::common::BluetoothAddress const& deviceId)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_INVALID_STATE, BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR,
                                               BT_ERR_HID_APP_NOT_REGISTER };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.connect", validErrCodes);
        std::string addr = std::string(deviceId.address);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidAddress(addr), BT_ERR_INVALID_PARAM);
        uint8_t addressType = static_cast<uint8_t>(deviceId.addressType);
        bool isRealAddr = (addressType == AddressType::REAL_ADDRESS);
        bool isVirtualAddr = (addressType == AddressType::VIRTUAL_ADDRESS);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(isRealAddr || isVirtualAddr, BT_ERR_INVALID_PARAM);
        if (deviceId.rawAddressType.has_value()) {
            uint8_t rawAddrType = static_cast<uint8_t>(deviceId.rawAddressType.value().get_value());
            bool isPublicAddr = (rawAddrType == RawAddressType::PUBLIC_ADDRESS);
            bool isRandomAddr = (rawAddrType == RawAddressType::RANDOM_ADDRESS);
            TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(isPublicAddr || isRandomAddr, BT_ERR_INVALID_PARAM);
        }
        AddressInfo addressInfo;
        addressInfo.SetAddress(std::string(deviceId.address));
        addressInfo.SetAddressType(static_cast<uint8_t>(deviceId.addressType));
        auto rawAddrType = deviceId.rawAddressType.value_or(
            ::ohos::bluetooth::common::BluetoothRawAddressType::key_t::PUBLIC);
        addressInfo.SetRawAddressType(static_cast<uint8_t>(rawAddrType));
        HidDevice *profile = HidDevice::GetProfile();
        int32_t errorCode = profile->Connect(addressInfo);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void Disconnect()
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                               BT_ERR_HID_APP_NOT_REGISTER };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.disconnect", validErrCodes);
        HidDevice *profile = HidDevice::GetProfile();
        int32_t errorCode = profile->Disconnect();
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void ReportError(::ohos::bluetooth::hid::ErrorReason error)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR, BT_ERR_HID_APP_NOT_REGISTER,
                                               BT_ERR_HID_DEVICE_NOT_CONNECTED };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.ReportError", validErrCodes);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(
            IsValidErrorReason(error.get_value()), BT_ERR_INVALID_PARAM);
        HidDevice *profile = HidDevice::GetProfile();
        int32_t errorCode = profile->ReportError(static_cast<ErrorReason>(error.get_value()));
        HILOGI("errorCode:%{public}d", errorCode);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void UnregisterHidDevice()
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.unregisterHidDevice", validErrCodes);
        if (observer_) {
            observer_->appStatusChangeEventModule_.DeregisterEvent(std::nullopt);
        }
        HidDevice *profile = HidDevice::GetProfile();
        int32_t errorCode = profile->UnregisterHidDevice();
        HILOGI("errorCode:%{public}d", errorCode);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void SendReport(int32_t id, ::taihe::array_view<uint8_t> reportData)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR, BT_ERR_HID_APP_NOT_REGISTER,
                                               BT_ERR_HID_DEVICE_NOT_CONNECTED };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.sendReport", validErrCodes);
        std::vector<uint8_t> data;
        if (reportData.size() > 0) {
            data.assign(reportData.data(), reportData.data() + reportData.size());
        }
        HidDevice *profile = HidDevice::GetProfile();
        int32_t errorCode = profile->SendReport(id, data);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void ReplyReport(::ohos::bluetooth::hid::ReportType type, int32_t id, ::taihe::array_view<uint8_t> reportData)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR, BT_ERR_HID_APP_NOT_REGISTER,
                                               BT_ERR_HID_DEVICE_NOT_CONNECTED };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.replyReport", validErrCodes);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidReportType(type.get_value()), BT_ERR_INVALID_PARAM);
        std::vector<uint8_t> data;
        if (reportData.size() > 0) {
            data.assign(reportData.data(), reportData.data() + reportData.size());
        }
        HidDevice *profile = HidDevice::GetProfile();
        int32_t errorCode = profile->ReplyReport(static_cast<ReportType>(type.get_value()), id, data);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
    }

    void RegisterHidDevice(::ohos::bluetooth::hid::HidDeviceSdp const& sdp,
        ::ohos::bluetooth::hid::HidDeviceQos const& inQos,
        ::ohos::bluetooth::hid::HidDeviceQos const& outQos,
        ::taihe::callback_view<void(bool data)> callback)
    {
        HILOGI("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR,
                                               BT_ERR_HID_APPLICATION_NOT_IN_FOREGROUND,
                                               BT_ERR_HID_APP_HAS_BEEN_REGISTERED };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.registerHidDevice", validErrCodes);

        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidHidDeviceSdp(sdp), BT_ERR_INVALID_PARAM);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidHidDeviceQos(inQos), BT_ERR_INVALID_PARAM);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidHidDeviceQos(outQos), BT_ERR_INVALID_PARAM);

        BluetoothHidDeviceSdp sdpSetting = ConvertHidDeviceSdp(sdp);
        BluetoothHidDeviceQos inQosSetting = ConvertHidDeviceQos(inQos);
        BluetoothHidDeviceQos outQosSetting = ConvertHidDeviceQos(outQos);

        HidDevice *profile = HidDevice::GetProfile();
        int32_t errorCode = profile->RegisterHidDevice(sdpSetting, inQosSetting, outQosSetting);
        HILOGI("errorCode:%{public}s", GetTaiheErrMsg(errorCode).c_str());
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(errorCode == BT_NO_ERROR, errorCode);
        if (observer_) {
            observer_->appStatusChangeEventModule_.RegisterEvent(callback);
        }
    }

    ::ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(::taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ::ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

        HidDevice *profile = HidDevice::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetConnectionState(device, state);
        int32_t profileState = TaiheUtils::GetProfileConnectionState(state);
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
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.getConnectedDevices", validErrCodes);
        HidDevice *profile = HidDevice::GetProfile();
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetConnectedDevices(devices);
        HILOGI("errorCode:%{public}s, devices size:%{public}zu",
            GetTaiheErrMsg(errorCode).c_str(), devices.size());
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode,
            taihe::array<taihe::string>{});

        std::vector<std::string> deviceVector;
        for (auto &device : devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }
        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<::taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    ::ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(::taihe::string_view deviceId)
    {
        HILOGD("start");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.getConnectionStrategy", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
            ::ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HidDevice *profile = HidDevice::GetProfile();
        int strategy = 0;
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d, strategy: %{public}d", err, strategy);
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(err == BT_NO_ERROR, err,
            ::ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy));
        return ::ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    void SetConnectionStrategySync(::taihe::string_view deviceId,
        ::ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGD("start");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "hid.setConnectionStrategy", validErrCodes);
        std::string remoteAddr = std::string(deviceId);
        int32_t connStrategy = strategy.get_value();
        bool checkRet = CheckSetConnectStrategyParam(remoteAddr, connStrategy);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HidDevice *profile = HidDevice::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, connStrategy);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(err == BT_NO_ERROR, err);
    }

private:
    std::shared_ptr<TaiheBluetoothHidDeviceObserver> observer_ = nullptr;
};

::ohos::bluetooth::hid::HidDeviceProfile CreateHidDeviceProfile()
{
    return taihe::make_holder<HidDeviceProfileImpl, ::ohos::bluetooth::hid::HidDeviceProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateHidHostProfile(OHOS::Bluetooth::CreateHidHostProfile);
TH_EXPORT_CPP_API_CreateHidDeviceProfile(OHOS::Bluetooth::CreateHidDeviceProfile);
// NOLINTEND
