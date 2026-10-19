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
#define LOG_TAG "bt_connection_impl_bluetooth"
#endif

#include "ohos.bluetooth.connection.proj.hpp"
#include "ohos.bluetooth.connection.impl.hpp"
#include "ohos.bluetooth.connection.OobData.ani.1.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"


#include <regex>
#include <sstream>
#include <iomanip>
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_oob_data.h"
#include "taihe_bluetooth_connection_callback.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_event_module.h"
#include "taihe_ha_event_utils.h"
#include "taihe_async_work.h"

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<TaiheBluetoothRemoteDeviceObserver> g_remoteDeviceObserver =
    TaiheBluetoothRemoteDeviceObserver::GetInstance();
std::shared_ptr<TaiheBluetoothConnectionObserver> g_connectionObserver =
    TaiheBluetoothConnectionObserver::GetInstance();

static void ConvertUuidsVectorToTaiheArray(std::vector<std::string> const& uuids,
    taihe::array<ohos::bluetooth::constant::ProfileUuids>& result)
{
    if (uuids.empty()) {
        return;
    }

    std::vector<ohos::bluetooth::constant::ProfileUuids> profileUuidsVec;
    for (auto& uuid : uuids) {
        ohos::bluetooth::constant::ProfileUuids uuidEnum =
            ohos::bluetooth::constant::ProfileUuids::from_value(uuid);
        if (uuidEnum == ohos::bluetooth::constant::ProfileUuids::key_t(-1)) {
            continue;
        }
        profileUuidsVec.push_back(uuidEnum);
    }

    if (profileUuidsVec.empty()) {
        return;
    }

    result = taihe::array<ohos::bluetooth::constant::ProfileUuids>(
        taihe::copy_data_t{}, profileUuidsVec.data(), profileUuidsVec.size());
}

constexpr int ADDRESS_BYTE_LEN = 6;
constexpr int ADDRESS_WITH_TYPE_BYTE_LEN = 7;
constexpr int HEX_CHAR_PER_BYTE = 2;
constexpr int HEX_BASE = 16;
constexpr int OOB_C_SIZE = 16;         // size of confirmationHash of OobData
constexpr int OOB_R_SIZE = 16;         // size of randomizerHash of OobData
constexpr int OOB_NAME_MAX_SIZE = 256; // size limit of deviceName of OobData

static std::string HexByteToString(uint8_t byte)
{
    std::stringstream hexStream;
    hexStream << std::hex << std::setw(HEX_CHAR_PER_BYTE) << std::setfill('0') << static_cast<int>(byte);
    return hexStream.str();
}

static void GetAddressWithTypeFromAddress(const std::string &address, uint8_t rawAddressType,
    std::vector<uint8_t> &addressWithType)
{
    std::string hexAddrStr = std::regex_replace(address, std::regex(":"), "");
    for (size_t i = 0; i + 1 < hexAddrStr.length(); i += HEX_CHAR_PER_BYTE) {
        std::string byteStr = hexAddrStr.substr(i, HEX_CHAR_PER_BYTE);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteStr, nullptr, HEX_BASE));
        addressWithType.push_back(byte);
    }
    addressWithType.push_back(rawAddressType);
}

static std::string BuildAddressStringFromBytes(const std::vector<uint8_t> &addressBytes)
{
    std::string addr = "";
    if (addressBytes.size() != ADDRESS_BYTE_LEN) {
        HILOGE("Invalid address length");
        return addr;
    }
    for (size_t i = 0; i < ADDRESS_BYTE_LEN; i++) {
        addr += HexByteToString(addressBytes[i]);
        if (i < ADDRESS_BYTE_LEN - 1) {
            addr += ":";
        }
    }
    std::transform(addr.begin(), addr.end(), addr.begin(), ::toupper);
    return addr;
}

static bool IsValidTaiheOobData(const ::ohos::bluetooth::connection::OobData &taiheOobData, int32_t transportType)
{
    std::string address = static_cast<std::string>(taiheOobData.deviceId.address);
    if (!IsValidAddress(address)) {
        HILOGE("Invalid oob address");
        return false;
    }
    if (!taiheOobData.deviceId.rawAddressType.has_value()) {
        HILOGE("rawAddressType must be given for OOB");
        return false;
    }
    bool isBrTransport = (transportType == BT_TRANSPORT_BREDR);
    int32_t rawAddressType = taiheOobData.deviceId.rawAddressType.value().get_value();
    bool isPublicAddr =
        rawAddressType == static_cast<int32_t>(::ohos::bluetooth::common::BluetoothRawAddressType::key_t::PUBLIC);
    if (isBrTransport && !isPublicAddr) {
        HILOGE("Invalid transport and rawAddressType");
        return false;
    }
    if (taiheOobData.confirmationHash.size() != OOB_C_SIZE) {
        HILOGE("size of confirmationHash should be 16");
        return false;
    }
    if (taiheOobData.randomizerHash.has_value() && taiheOobData.randomizerHash.value().size() != OOB_R_SIZE) {
        HILOGE("size of randomizerHash should be 16");
        return false;
    }
    if (taiheOobData.deviceName.has_value()) {
        std::string deviceName = static_cast<std::string>(taiheOobData.deviceName.value());
        if (deviceName.empty() || deviceName.length() > OOB_NAME_MAX_SIZE) {
            HILOGE("size of deviceName should be between 0 and 256");
            return false;
        }
    }
    return true;
}

static OobData ConvertTaiheOobDataToNative(
    const ::ohos::bluetooth::connection::OobData &taiheOobData, uint8_t oobType)
{
    OobData nativeOobData;
    std::string address = static_cast<std::string>(taiheOobData.deviceId.address);
    uint8_t rawAddressType = RawAddressType::RANDOM_ADDRESS;
    if (taiheOobData.deviceId.rawAddressType.has_value()) {
        rawAddressType = static_cast<uint8_t>(taiheOobData.deviceId.rawAddressType.value().get_value());
    }
    std::vector<uint8_t> addressWithType;
    GetAddressWithTypeFromAddress(address, rawAddressType, addressWithType);
    nativeOobData.SetAddressWithType(addressWithType);

    std::vector<uint8_t> confirmHash;
    for (auto b : taiheOobData.confirmationHash) {
        confirmHash.push_back(b);
    }
    nativeOobData.SetConfirmationHash(confirmHash);

    if (taiheOobData.randomizerHash.has_value()) {
        std::vector<uint8_t> randomHash;
        for (auto b : taiheOobData.randomizerHash.value()) {
            randomHash.push_back(b);
        }
        nativeOobData.SetRandomizerHash(randomHash);
    }

    if (taiheOobData.deviceName.has_value()) {
        nativeOobData.SetDeviceName(static_cast<std::string>(taiheOobData.deviceName.value()));
    }

    if (taiheOobData.deviceRole.has_value()) {
        nativeOobData.SetDeviceRole(static_cast<uint8_t>(taiheOobData.deviceRole.value().get_value()));
    }

    nativeOobData.SetOobDataType(oobType);
    return nativeOobData;
}

static ::ohos::bluetooth::connection::OobData ConvertNativeOobDataToTaihe(const OobData &nativeOobData)
{
    std::string address = "";
    uint8_t rawAddressType = RawAddressType::RANDOM_ADDRESS;
    if (nativeOobData.HasAddressWithType()) {
        std::vector<uint8_t> addressWithType = nativeOobData.GetAddressWithType();
        if (addressWithType.size() == ADDRESS_WITH_TYPE_BYTE_LEN) {
            std::vector<uint8_t> addressVec(addressWithType.begin(), addressWithType.begin() + ADDRESS_BYTE_LEN);
            address = BuildAddressStringFromBytes(addressVec);
            rawAddressType = addressWithType[ADDRESS_BYTE_LEN];
        }
    }

    auto deviceId = ::ohos::bluetooth::common::BluetoothAddress{
        .address = ::taihe::string(address),
        .addressType = ::ohos::bluetooth::common::BluetoothAddressType(
            ::ohos::bluetooth::common::BluetoothAddressType::key_t::REAL),
        .rawAddressType = ::taihe::optional<::ohos::bluetooth::common::BluetoothRawAddressType>(
            std::in_place, ::ohos::bluetooth::common::BluetoothRawAddressType::from_value(rawAddressType)),
    };

    std::vector<uint8_t> confirmHashVec =
        nativeOobData.HasConfirmHash() ? nativeOobData.GetConfirmationHash() : std::vector<uint8_t>{};

    ::taihe::optional<::taihe::array<uint8_t>> randomizerHashOpt;
    if (nativeOobData.HasRandomHash()) {
        std::vector<uint8_t> randomHash = nativeOobData.GetRandomizerHash();
        randomizerHashOpt.emplace(::taihe::copy_data, randomHash.data(), randomHash.size());
    }

    ::taihe::optional<::taihe::string> deviceNameOpt;
    if (nativeOobData.HasDeviceName()) {
        deviceNameOpt.emplace(nativeOobData.GetDeviceName());
    }

    ::taihe::optional<::ohos::bluetooth::connection::DeviceRole> deviceRoleOpt;
    deviceRoleOpt.emplace(::ohos::bluetooth::connection::DeviceRole::from_value(nativeOobData.GetDeviceRole()));

    return ::ohos::bluetooth::connection::OobData{
        .deviceId = deviceId,
        .confirmationHash = ::taihe::array<uint8_t>(::taihe::copy_data, confirmHashVec.data(), confirmHashVec.size()),
        .randomizerHash = randomizerHashOpt,
        .deviceName = deviceNameOpt,
        .deviceRole = deviceRoleOpt,
    };
}

class TaiheNativeOobData : public TaiheNativeObject {
public:
    explicit TaiheNativeOobData(const ::ohos::bluetooth::connection::OobData &oobData) : oobData_(oobData) {}
    ~TaiheNativeOobData() override = default;

    ani_ref ToTaiheValue(ani_env *env) const override
    {
        ani_object object = ::taihe::into_ani<::ohos::bluetooth::connection::OobData>(env, oobData_);
        return reinterpret_cast<ani_ref>(object);
    }

private:
    ::ohos::bluetooth::connection::OobData oobData_;
};

class TaiheOobCallback : public BluetoothOobCallback {
public:
    void OnGenerateLocalOobData(int32_t ret, const OobData &oobData) override
    {
        HILOGI("ret: %{public}d", ret);
        auto nativeObject = std::make_shared<TaiheNativeOobData>(ConvertNativeOobDataToTaihe(oobData));
        AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GENERATE_LOCAL_OOB_DATA, nativeObject, ret);
    }

    TaiheAsyncWorkMap asyncWorkMap_;
};

taihe::string GetRemoteProductId(taihe::string_view deviceId)
{
    HILOGD("start");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, deviceId);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    std::string productId = "";
    int32_t err = remoteDevice.GetDeviceProductId(productId);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, productId);
    HILOGI("GetRemoteProductId :%{public}s", productId.c_str());
    return productId;
}

taihe::string GetRemoteDeviceName(taihe::string_view deviceId, taihe::optional_view<bool> alias)
{
    HILOGD("start");
    std::string remoteAddr = std::string(deviceId);
    std::string name = INVALID_NAME;
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, deviceId);

    bool deviceAlias = true;
    if (alias.has_value()) {
        deviceAlias = alias.value();
    }

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceName(name, deviceAlias);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, name);

    return name;
}

taihe::array<taihe::string> GetPairedDevices()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::vector<BluetoothRemoteDevice> remoteDeviceLists;

    int32_t err = host->GetPairedDevices(BT_TRANSPORT_BREDR, remoteDeviceLists);
    std::vector<std::string> dstDevicesvec;

    for (auto vec : remoteDeviceLists) {
        dstDevicesvec.push_back(vec.GetDeviceAddr().c_str());
    }
    taihe::array<taihe::string> result(taihe::copy_data_t{}, dstDevicesvec.data(), dstDevicesvec.size());
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, result);
    HILOGI("end");
    return result;
}

void SetBluetoothScanMode(ohos::bluetooth::connection::ScanMode mode, int duration)
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->SetBtScanMode(mode, duration);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);

    int32_t bondableMode = 1;
    host->SetBondableMode(BT_TRANSPORT_BREDR, bondableMode);
}

ohos::bluetooth::connection::DeviceClass GetRemoteDeviceClass(taihe::string_view deviceId)
{
    HILOGD("start");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    ohos::bluetooth::connection::DeviceClass deviceClass = {
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED,
        ohos::bluetooth::constant::MajorMinorClass::key_t::COMPUTER_UNCATEGORIZED,
        0
    };
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, deviceClass);

    int tmpCod = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorClass = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorMinorClass = ohos::bluetooth::constant::MajorMinorClass(
        ohos::bluetooth::constant::MajorMinorClass::key_t::COMPUTER_UNCATEGORIZED).get_value();
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceProductType(tmpCod, tmpMajorClass, tmpMajorMinorClass);
    deviceClass = {
        ohos::bluetooth::constant::MajorClass::from_value(tmpMajorClass),
        ohos::bluetooth::constant::MajorMinorClass::from_value(tmpMajorMinorClass),
        tmpCod
    };
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, deviceClass);

    return deviceClass;
}

void OnDiscoveryResult(::taihe::callback_view<void(
    ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->discoveryResultLock);
    auto discoveryResultCb = ::taihe::optional<::taihe::callback<void(
        ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_connectionObserver->discoveryResultVec;
    // callback重复注册不处理
    if (std::find(callbackVec.begin(), callbackVec.end(), discoveryResultCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(discoveryResultCb);
}

void OffDiscoveryResult(::taihe::optional_view<::taihe::callback<void(
    ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->discoveryResultLock);
    auto& callbackVec = g_connectionObserver->discoveryResultVec;
    // callback有值，则删除该callback，否则删除vec中注册的所有callback
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<::taihe::callback<void(
                    ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void OnBatteryChange(::taihe::callback_view<void(::ohos::bluetooth::connection::BatteryInfo const& data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_remoteDeviceObserver->remoteBatteryChangeLock);
    auto batteryChangeCb = ::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::BatteryInfo const& data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_remoteDeviceObserver->remoteBatteryChangeVec;
    if (std::find(callbackVec.begin(), callbackVec.end(), batteryChangeCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(batteryChangeCb);
}

void OffBatteryChange(::taihe::optional_view<::taihe::callback<void(
    ::ohos::bluetooth::connection::BatteryInfo const& data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_remoteDeviceObserver->remoteBatteryChangeLock);
    auto& callbackVec = g_remoteDeviceObserver->remoteBatteryChangeVec;
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::ohos::bluetooth::connection::BatteryInfo const& data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void OnBluetoothDeviceFind(::taihe::callback_view<void(::taihe::array_view<::taihe::string> data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->bluetoothDeviceFindLock);
    auto bluetoothDeviceFindCb = ::taihe::optional<::taihe::callback<void(
        ::taihe::array_view<::taihe::string> data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_connectionObserver->bluetoothDeviceFindVec;
    if (std::find(callbackVec.begin(), callbackVec.end(), bluetoothDeviceFindCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(bluetoothDeviceFindCb);
}

void OffBluetoothDeviceFind(::taihe::optional_view<::taihe::callback<void(
    ::taihe::array_view<::taihe::string> data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->bluetoothDeviceFindLock);
    auto& callbackVec = g_connectionObserver->bluetoothDeviceFindVec;
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::taihe::array_view<::taihe::string> data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void OnBondStateChange(::taihe::callback_view<void(::ohos::bluetooth::connection::BondStateParam const& data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_remoteDeviceObserver->bondStateChangeLock);
    auto bondStateChangeCb = ::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::BondStateParam const& data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_remoteDeviceObserver->bondStateChangeVec;
    if (std::find(callbackVec.begin(), callbackVec.end(), bondStateChangeCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(bondStateChangeCb);
}

void OffBondStateChange(::taihe::optional_view<::taihe::callback<void(
    ::ohos::bluetooth::connection::BondStateParam const& data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_remoteDeviceObserver->bondStateChangeLock);
    auto& callbackVec = g_remoteDeviceObserver->bondStateChangeVec;
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::ohos::bluetooth::connection::BondStateParam const& data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void OnPinRequired(::taihe::callback_view<void(::ohos::bluetooth::connection::PinRequiredParam const& data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->pinRequiredLock);
    auto pinRequiredCb = ::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::PinRequiredParam const& data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_connectionObserver->pinRequiredVec;
    if (std::find(callbackVec.begin(), callbackVec.end(), pinRequiredCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(pinRequiredCb);
}

void OffPinRequired(::taihe::optional_view<::taihe::callback<
    void(::ohos::bluetooth::connection::PinRequiredParam const& data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->pinRequiredLock);
    auto& callbackVec = g_connectionObserver->pinRequiredVec;
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::ohos::bluetooth::connection::PinRequiredParam const& data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void ControlDeviceAction(ohos::bluetooth::connection::ControlDeviceActionParams controlDeviceActionParams)
{
    HILOGD("ControlDeviceAction enter");
    std::string deviceId = std::string(controlDeviceActionParams.deviceId);
    bool checkRet = CheckDeviceIdParam(deviceId);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    uint32_t controlType = (controlDeviceActionParams.type).get_value();
    uint32_t controlTypeVal = (controlDeviceActionParams.typeValue).get_value();
    uint32_t controlObject = (controlDeviceActionParams.controlObject).get_value();
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(deviceId);
    int32_t err = remoteDevice.ControlDeviceAction(controlType, controlTypeVal, controlObject);
    HILOGI("ControlDeviceAction err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

int64_t GetLastConnectionTime(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, 0);

    int64_t connectionTime = 0;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetLastConnectionTime(connectionTime);
    HILOGI("GetRemoteDeviceConnectionTime GetLastConnectionTime err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, connectionTime);

    return connectionTime;
}

ohos::bluetooth::connection::DeviceType GetRemoteDeviceType(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, ohos::bluetooth::connection::DeviceType::from_value(0));

    int32_t deviceType = 0;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceCustomType(deviceType);
    HILOGI("GetRemoteDeviceType err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, ohos::bluetooth::connection::DeviceType::from_value(deviceType));

    return ohos::bluetooth::connection::DeviceType::from_value(deviceType);
}

void UpdateCloudBluetoothDevice(ohos::bluetooth::connection::TrustedPairedDevices trustedPairedDevices)
{
    HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDevice enter");
    std::vector<TrustPairDeviceParam> trustPairs {};
    for (const auto &device : trustedPairedDevices.trustedPairedDevices) {
        TrustPairDeviceParam trustPair;
        trustPair.sn_ = std::string(device.sn);
        trustPair.deviceType_ = std::string(device.deviceType);
        trustPair.modelId_ = std::string(device.modelId);
        trustPair.manufactory_ = std::string(device.manufactory);
        trustPair.productId_ = std::string(device.productId);
        trustPair.hiLinkVersion_ = std::string(device.hiLinkVersion);
        trustPair.macAddress_ = std::string(device.macAddress);
        trustPair.serviceType_ = std::string(device.serviceType);
        trustPair.serviceId_ = std::string(device.serviceId);
        trustPair.deviceName_ = std::string(device.deviceName);
        trustPair.uuids_.clear();
        std::string uuidStr = device.uuids.c_str();
        auto first = uuidStr.begin();
        while (first != uuidStr.end()) {
            auto last = std::find(first, uuidStr.end(), ',');
            std::string uuid(first, last);
            if (!uuid.empty()) {
                trustPair.uuids_.push_back(uuid);
            }
            if (last == uuidStr.end()) {
                break;
            }
            first = last + 1;
        }
        trustPair.bluetoothClass_ = device.bluetoothClass;
        trustPair.token_.clear();
        for (const auto &token : device.token) {
            trustPair.token_.push_back(static_cast<uint8_t>(token));
        }
        trustPair.deviceNameTime_ = device.deviceNameTime;
        trustPair.secureAdvertisingInfo_.clear();
        for (const auto &info : device.secureAdvertisingInfo) {
            trustPair.secureAdvertisingInfo_.push_back(static_cast<uint8_t>(info));
        }
        trustPair.pairState_ = device.pairState;
        trustPairs.push_back(trustPair);
    }
    int32_t err = BluetoothHost::GetDefaultHost().UpdateCloudBluetoothDevice(trustPairs);
    HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDevice err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

ohos::bluetooth::connection::BatteryInfo GetRemoteDeviceBatteryInfo(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.GetRemoteDeviceBatteryInfo", validErrCodes);
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    ohos::bluetooth::connection::BatteryInfo batterys {"1", 1, 2,
        ohos::bluetooth::connection::DeviceChargeState(
            ohos::bluetooth::connection::DeviceChargeState::key_t::DEVICE_NORMAL_CHARGE_NOT_CHARGED),
        3,
        ohos::bluetooth::connection::DeviceChargeState(
            ohos::bluetooth::connection::DeviceChargeState::key_t::DEVICE_SUPER_CHARGE_NOT_CHARGED),
        4,
        ohos::bluetooth::connection::DeviceChargeState(
            ohos::bluetooth::connection::DeviceChargeState::key_t::DEVICE_SUPER_CHARGE_IN_CHARGING)
    };
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM, batterys);

    DeviceBatteryInfo batteryInfo;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetRemoteDeviceBatteryInfo(batteryInfo);
    HILOGI("err: %{public}d", err);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(err == BT_NO_ERROR, err, batterys);

    batterys.deviceId = batteryInfo.deviceId_;
    batterys.batteryLevel = batteryInfo.batteryLevel_;
    batterys.leftEarBatteryLevel = batteryInfo.leftEarBatteryLevel_;
    batterys.leftEarChargeState =
        ohos::bluetooth::connection::DeviceChargeState::from_value(static_cast<int>(batteryInfo.leftEarChargeState_));
    batterys.rightEarBatteryLevel = batteryInfo.rightEarBatteryLevel_;
    batterys.rightEarChargeState =
        ohos::bluetooth::connection::DeviceChargeState::from_value(static_cast<int>(batteryInfo.rightEarChargeState_));
    batterys.boxBatteryLevel = batteryInfo.boxBatteryLevel_;
    batterys.boxChargeState =
        ohos::bluetooth::connection::DeviceChargeState::from_value(static_cast<int>(batteryInfo.boxChargeState_));

    return batterys;
}

void SetRemoteDeviceName(taihe::string_view deviceId, taihe::string_view name)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    std::string deviceName = static_cast<std::string>(name);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.SetDeviceAlias(deviceName);
    HILOGI("SetDeviceName err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

void SetRemoteDeviceType(taihe::string_view deviceId, ohos::bluetooth::connection::DeviceType type)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    int32_t deviceType = type.get_value();
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.SetDeviceCustomType(deviceType);
    HILOGI("SetRemoteDeviceType err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

ohos::bluetooth::connection::ScanMode GetBluetoothScanMode()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t scanMode = 0;
    int32_t err = host->GetBtScanMode(scanMode);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, ohos::bluetooth::connection::ScanMode::from_value(scanMode));
    HILOGI("end");
    return ohos::bluetooth::connection::ScanMode::from_value(scanMode);
}

ohos::bluetooth::constant::ProfileConnectionState GetProfileConnectionState(
    taihe::optional<ohos::bluetooth::constant::ProfileId> profileId)
{
    HILOGD("enter");
    int state = static_cast<int>(BTConnectState::DISCONNECTED);

    if (profileId.has_value()) {
        uint32_t conProfileId = profileId.value().get_value();
        BluetoothHost *host = &BluetoothHost::GetDefaultHost();
        int32_t err = host->GetBtProfileConnState(conProfileId, state);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(state));

        state = TaiheUtils::GetProfileConnectionState(state);
    } else {
        BluetoothHost *host = &BluetoothHost::GetDefaultHost();
        int32_t err = host->GetBtConnectionState(state);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(state));
    }

    return ohos::bluetooth::constant::ProfileConnectionState::from_value(state);
}

taihe::string GetLocalName()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::string localName = INVALID_NAME;
    int32_t err = host->GetLocalName(localName);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, localName);
    HILOGI("end");
    return localName;
}

void DealPairStatus(const int &status, int &bondStatus)
{
    HILOGD("status is %{public}d", status);
    switch (status) {
        case PAIR_NONE:
            bondStatus = ohos::bluetooth::connection::BondState(
                ohos::bluetooth::connection::BondState::key_t::BOND_STATE_INVALID).get_value();
            break;
        case PAIR_PAIRING:
            bondStatus = ohos::bluetooth::connection::BondState(
                ohos::bluetooth::connection::BondState::key_t::BOND_STATE_BONDING).get_value();
            break;
        case PAIR_PAIRED:
            bondStatus = ohos::bluetooth::connection::BondState(
                ohos::bluetooth::connection::BondState::key_t::BOND_STATE_BONDED).get_value();
            break;
        default:
            break;
    }
}

ohos::bluetooth::connection::BondState GetPairState(taihe::string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, ohos::bluetooth::connection::BondState::from_value(0));

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int state = PAIR_NONE;
    int32_t err = remoteDevice.GetPairState(state);
    int pairState = ohos::bluetooth::connection::BondState(
        ohos::bluetooth::connection::BondState::key_t::BOND_STATE_INVALID).get_value();
    DealPairStatus(state, pairState);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, ohos::bluetooth::connection::BondState::from_value(pairState));
    HILOGI("getPairState :%{public}d", pairState);

    return ohos::bluetooth::connection::BondState::from_value(pairState);
}

void StartBluetoothDiscovery()
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->StartBtDiscovery();
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

bool IsBluetoothDiscovering()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    bool isDiscovering = false;
    int32_t err = host->IsBtDiscovering(isDiscovering);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, isDiscovering);
    HILOGI("isBluetoothDiscovering :%{public}d", isDiscovering);

    return isDiscovering;
}

void StopBluetoothDiscovery()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->CancelBtDiscovery();
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void SetDevicePairingConfirmation(taihe::string_view deviceId, bool accept)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    HILOGI("SetDevicePairingConfirmation::accept = %{public}d", accept);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t ret = BT_NO_ERROR;
    if (accept) {
        ret = remoteDevice.SetDevicePairingConfirmation(accept);
    } else {
        ret = remoteDevice.CancelPairing();
    }
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void ConnectAllowedProfilesSync(taihe::string_view deviceId)
{
    HILOGI("enter");
    std::string remoteAddr = static_cast<std::string>(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->ConnectAllowedProfiles(remoteAddr);
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

taihe::array<ohos::bluetooth::constant::ProfileUuids> GetRemoteProfileUuidsSync(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::string address = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(address);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, taihe::array<ohos::bluetooth::constant::ProfileUuids>{});

    std::vector<std::string> uuids{};
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(address);
    int32_t err = remoteDevice.GetDeviceUuids(uuids);
    HILOGI("err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, taihe::array<ohos::bluetooth::constant::ProfileUuids>{});

    taihe::array<ohos::bluetooth::constant::ProfileUuids> result = {};
    ConvertUuidsVectorToTaiheArray(uuids, result);
    return result;
}

void PairDeviceSync(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
    std::string remoteAddr = static_cast<std::string>(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.StartPair();
    HILOGI("err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(err == BT_NO_ERROR, err);
}

void DisconnectAllowedProfilesSync(taihe::string_view deviceId)
{
    HILOGI("enter");
    std::string remoteAddr = static_cast<std::string>(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->DisconnectAllowedProfiles(remoteAddr);
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

taihe::array<ohos::bluetooth::constant::ProfileUuids> GetLocalProfileUuidsSync()
{
    HILOGD("enter");
    std::vector<std::string> uuids{};
    int32_t err = BluetoothHost::GetDefaultHost().GetLocalProfileUuids(uuids);
    HILOGI("err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, taihe::array<ohos::bluetooth::constant::ProfileUuids>{});

    taihe::array<ohos::bluetooth::constant::ProfileUuids> result = {};
    ConvertUuidsVectorToTaiheArray(uuids, result);
    return result;
}

void SetDevicePinCodeSync(taihe::string_view deviceId, taihe::string_view code)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    std::string pinCode = std::string(code);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.SetDevicePin(pinCode);
    HILOGI("SetDevicePinCode err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

void CancelPairingDeviceSync(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.CancelPairing();
    HILOGI("err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

void CancelPairedDeviceSync(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t err = host->RemovePair(remoteDevice);
    HILOGI("err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

void PairCredibleDeviceSync(taihe::string_view deviceId, ohos::bluetooth::connection::BluetoothTransport transport)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    int transportType = transport.get_value();
    bool checkRet = CheckPairCredibleDeviceParam(remoteAddr, transportType);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transportType);
    int32_t err = remoteDevice.StartCrediblePair();
    HILOGI("err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

void SetLocalName(::taihe::string_view name)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.SetLocalName", validErrCodes);
    std::string localName = static_cast<std::string>(name);
    bool checkRet = CheckLocalNameParam(localName);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->SetLocalName(localName);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

void StartPairOutOfBandSync(::taihe::string_view deviceId,
    ::ohos::bluetooth::connection::BluetoothTransport transport,
    ::taihe::optional_view<::ohos::bluetooth::connection::OobData> p192Data,
    ::taihe::optional_view<::ohos::bluetooth::connection::OobData> p256Data)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_INVALID_STATE,
                                           BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.StartPairOutOfBand", validErrCodes);
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

    int32_t transportType = transport.get_value();
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transportType);

    int32_t err = BT_ERR_INVALID_PARAM;
    if (p256Data.has_value()) {
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidTaiheOobData(p256Data.value(), transportType),
            BT_ERR_INVALID_PARAM);
        OobData nativeOobData = ConvertTaiheOobDataToNative(p256Data.value(), OobDataType::P256);
        err = remoteDevice.StartPairOutOfBand(nativeOobData);
    } else if (p192Data.has_value()) {
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidTaiheOobData(p192Data.value(), transportType),
            BT_ERR_INVALID_PARAM);
        OobData nativeOobData = ConvertTaiheOobDataToNative(p192Data.value(), OobDataType::P192);
        err = remoteDevice.StartPairOutOfBand(nativeOobData);
    }
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(err == BT_NO_ERROR, err);
}

uintptr_t GenerateLocalOobDataPromise(::ohos::bluetooth::connection::BluetoothTransport transport)
{
    HILOGD("enter");
    ani_env *env = taihe::get_env();
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_INVALID_STATE,
                                           BT_ERR_TIMEOUT, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(env, "connection.GenerateLocalOobData", validErrCodes);
    int32_t transportType = transport.get_value();

    static std::shared_ptr<TaiheOobCallback> callback = std::make_shared<TaiheOobCallback>();
    auto func = [transportType]() {
        int32_t ret = BluetoothHost::GetDefaultHost().GenerateLocalOobData(transportType, callback);
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, apiContext.haUtils);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));
    bool success = callback->asyncWorkMap_.TryPush(TaiheAsyncType::GENERATE_LOCAL_OOB_DATA, asyncWork);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(success, BT_ERR_INTERNAL_ERROR,
        reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();

    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

::taihe::string GetVirtualAddressByHash(::ohos::bluetooth::connection::HashAlgorithmType algorithmType,
    ::taihe::string_view hashValue)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_INVALID_STATE,
                                           BT_ERR_PARAM_NOT_COMPLIANT, BT_ERR_UNPAIRED_DEVICE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.GetVirtualAddressByHash", validErrCodes);
    int32_t hashAlgorithmType = algorithmType.get_value();
    std::string hash = static_cast<std::string>(hashValue);
    std::string virtualAddress = "";
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t err = host->GetVirtualAddressByHash(hashAlgorithmType, hash, virtualAddress);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(err == BT_NO_ERROR, err, virtualAddress);
    return virtualAddress;
}

::taihe::string GetBleMacByBrMac(::taihe::string_view brMacValue)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                           BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT, BT_ERR_INVALID_STATE,
                                           BT_ERR_UNPAIRED_DEVICE, BT_ERR_INTERNAL_ERROR, BT_ERR_NO_BLE_ASSOCIATION };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.GetBleMacByBrMac", validErrCodes);
    std::string brMac = static_cast<std::string>(brMacValue);
    std::string bleMac = "";
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t err = host->GetBleMacByBrMac(brMac, bleMac);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(err == BT_NO_ERROR, err, bleMac);
    return bleMac;
}

void PairDeviceOutOfBandSync(::ohos::bluetooth::connection::BluetoothTransport transport,
    ::ohos::bluetooth::connection::OobDataNullValue const& p192Data,
    ::ohos::bluetooth::connection::OobDataNullValue const& p256Data)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_INVALID_STATE,
                                           BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.PairDeviceOutOfBand", validErrCodes);
    int32_t transportType = transport.get_value();

    int32_t err = BT_ERR_INVALID_PARAM;
    if (p256Data.holds_data()) {
        auto &taiheOobData = p256Data.get_data_ref();
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidTaiheOobData(taiheOobData, transportType),
            BT_ERR_INVALID_PARAM);
        std::string address = static_cast<std::string>(taiheOobData.deviceId.address);
        OobData nativeOobData = ConvertTaiheOobDataToNative(taiheOobData, OobDataType::P256);
        BluetoothRemoteDevice remoteDevice(address, transportType);
        err = remoteDevice.StartPairOutOfBand(nativeOobData);
    } else if (p192Data.holds_data()) {
        auto &taiheOobData = p192Data.get_data_ref();
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(IsValidTaiheOobData(taiheOobData, transportType),
            BT_ERR_INVALID_PARAM);
        std::string address = static_cast<std::string>(taiheOobData.deviceId.address);
        OobData nativeOobData = ConvertTaiheOobDataToNative(taiheOobData, OobDataType::P192);
        BluetoothRemoteDevice remoteDevice(address, transportType);
        err = remoteDevice.StartPairOutOfBand(nativeOobData);
    }
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(err == BT_NO_ERROR, err);
}

::taihe::string GetCarKeyDfxData()
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_INVALID_STATE,
                                           BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.GetCarKeyDfxData", validErrCodes);
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::string dfxData = "";
    int32_t err = host->GetCarKeyDfxData(dfxData);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(err == BT_NO_ERROR, err, dfxData);
    return dfxData;
}

::ohos::bluetooth::connection::BluetoothTransport GetRemoteDeviceTransport(::taihe::string_view deviceId)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_SERVICE_DISCONNECTED,
                                           BT_ERR_INVALID_STATE, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.GetRemoteDeviceTransport", validErrCodes);
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    int32_t transport = 0;
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM,
        ::ohos::bluetooth::connection::BluetoothTransport::from_value(transport));

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceTransport(transport);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(err == BT_NO_ERROR, err,
        ::ohos::bluetooth::connection::BluetoothTransport::from_value(transport));
    return ::ohos::bluetooth::connection::BluetoothTransport::from_value(transport);
}

void SetCarKeyDfxData(::taihe::string_view deviceId, ::ohos::bluetooth::connection::CarKeyActionType action)
{
    HILOGD("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_INVALID_STATE,
                                           BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.SetCarKeyDfxData", validErrCodes);
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);

    int32_t actionValue = action.get_value();
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->SetCarKeyCardData(remoteAddr, actionValue);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(ret == BT_NO_ERROR, ret);
}

void OnAclStateChange(
    ::taihe::callback_view<void(::ohos::bluetooth::connection::AclStateResult const& data)> callback)
{
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.OnAclStateChange", {});
    g_remoteDeviceObserver->aclStateChangeEventModule_.RegisterEvent(callback);
}

void OffAclStateChange(
    ::taihe::optional_view<::taihe::callback<
        void(::ohos::bluetooth::connection::AclStateResult const& data)>> callback)
{
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.OffAclStateChange", {});
    g_remoteDeviceObserver->aclStateChangeEventModule_.DeregisterEvent(callback);
}

void OnScanModeChange(::taihe::callback_view<void(::ohos::bluetooth::connection::ScanMode data)> callback)
{
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.OnScanModeChange", {});
    g_connectionObserver->scanModeChangeEventModule_.RegisterEvent(callback);
}

void OffScanModeChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::bluetooth::connection::ScanMode data)>> callback)
{
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "connection.OffScanModeChange", {});
    g_connectionObserver->scanModeChangeEventModule_.DeregisterEvent(callback);
}
}  // Bluetooth
}  // OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_OnDiscoveryResult(OHOS::Bluetooth::OnDiscoveryResult);
TH_EXPORT_CPP_API_OffDiscoveryResult(OHOS::Bluetooth::OffDiscoveryResult);
TH_EXPORT_CPP_API_OnBatteryChange(OHOS::Bluetooth::OnBatteryChange);
TH_EXPORT_CPP_API_OffBatteryChange(OHOS::Bluetooth::OffBatteryChange);
TH_EXPORT_CPP_API_OnBluetoothDeviceFind(OHOS::Bluetooth::OnBluetoothDeviceFind);
TH_EXPORT_CPP_API_OffBluetoothDeviceFind(OHOS::Bluetooth::OffBluetoothDeviceFind);
TH_EXPORT_CPP_API_OnBondStateChange(OHOS::Bluetooth::OnBondStateChange);
TH_EXPORT_CPP_API_OffBondStateChange(OHOS::Bluetooth::OffBondStateChange);
TH_EXPORT_CPP_API_OnPinRequired(OHOS::Bluetooth::OnPinRequired);
TH_EXPORT_CPP_API_OffPinRequired(OHOS::Bluetooth::OffPinRequired);
TH_EXPORT_CPP_API_GetRemoteProductId(OHOS::Bluetooth::GetRemoteProductId);
TH_EXPORT_CPP_API_GetRemoteDeviceName(OHOS::Bluetooth::GetRemoteDeviceName);
TH_EXPORT_CPP_API_GetPairedDevices(OHOS::Bluetooth::GetPairedDevices);
TH_EXPORT_CPP_API_SetBluetoothScanMode(OHOS::Bluetooth::SetBluetoothScanMode);
TH_EXPORT_CPP_API_GetRemoteDeviceClass(OHOS::Bluetooth::GetRemoteDeviceClass);
TH_EXPORT_CPP_API_ControlDeviceAction(OHOS::Bluetooth::ControlDeviceAction);
TH_EXPORT_CPP_API_GetLastConnectionTime(OHOS::Bluetooth::GetLastConnectionTime);
TH_EXPORT_CPP_API_GetRemoteDeviceType(OHOS::Bluetooth::GetRemoteDeviceType);
TH_EXPORT_CPP_API_UpdateCloudBluetoothDevice(OHOS::Bluetooth::UpdateCloudBluetoothDevice);
TH_EXPORT_CPP_API_GetRemoteDeviceBatteryInfo(OHOS::Bluetooth::GetRemoteDeviceBatteryInfo);
TH_EXPORT_CPP_API_SetRemoteDeviceName(OHOS::Bluetooth::SetRemoteDeviceName);
TH_EXPORT_CPP_API_SetRemoteDeviceType(OHOS::Bluetooth::SetRemoteDeviceType);
TH_EXPORT_CPP_API_GetBluetoothScanMode(OHOS::Bluetooth::GetBluetoothScanMode);
TH_EXPORT_CPP_API_GetProfileConnectionState(OHOS::Bluetooth::GetProfileConnectionState);
TH_EXPORT_CPP_API_GetLocalName(OHOS::Bluetooth::GetLocalName);
TH_EXPORT_CPP_API_GetPairState(OHOS::Bluetooth::GetPairState);
TH_EXPORT_CPP_API_StartBluetoothDiscovery(OHOS::Bluetooth::StartBluetoothDiscovery);
TH_EXPORT_CPP_API_IsBluetoothDiscovering(OHOS::Bluetooth::IsBluetoothDiscovering);
TH_EXPORT_CPP_API_StopBluetoothDiscovery(OHOS::Bluetooth::StopBluetoothDiscovery);
TH_EXPORT_CPP_API_SetDevicePairingConfirmation(OHOS::Bluetooth::SetDevicePairingConfirmation);
TH_EXPORT_CPP_API_ConnectAllowedProfilesSync(OHOS::Bluetooth::ConnectAllowedProfilesSync);
TH_EXPORT_CPP_API_GetRemoteProfileUuidsSync(OHOS::Bluetooth::GetRemoteProfileUuidsSync);
TH_EXPORT_CPP_API_PairDeviceSync(OHOS::Bluetooth::PairDeviceSync);
TH_EXPORT_CPP_API_DisconnectAllowedProfilesSync(OHOS::Bluetooth::DisconnectAllowedProfilesSync);
TH_EXPORT_CPP_API_GetLocalProfileUuidsSync(OHOS::Bluetooth::GetLocalProfileUuidsSync);
TH_EXPORT_CPP_API_SetDevicePinCodeSync(OHOS::Bluetooth::SetDevicePinCodeSync);
TH_EXPORT_CPP_API_CancelPairingDeviceSync(OHOS::Bluetooth::CancelPairingDeviceSync);
TH_EXPORT_CPP_API_CancelPairedDeviceSync(OHOS::Bluetooth::CancelPairedDeviceSync);
TH_EXPORT_CPP_API_PairCredibleDeviceSync(OHOS::Bluetooth::PairCredibleDeviceSync);
TH_EXPORT_CPP_API_SetLocalName(OHOS::Bluetooth::SetLocalName);
TH_EXPORT_CPP_API_StartPairOutOfBandSync(OHOS::Bluetooth::StartPairOutOfBandSync);
TH_EXPORT_CPP_API_GenerateLocalOobDataPromise(OHOS::Bluetooth::GenerateLocalOobDataPromise);
TH_EXPORT_CPP_API_GetVirtualAddressByHash(OHOS::Bluetooth::GetVirtualAddressByHash);
TH_EXPORT_CPP_API_GetBleMacByBrMac(OHOS::Bluetooth::GetBleMacByBrMac);
TH_EXPORT_CPP_API_PairDeviceOutOfBandSync(OHOS::Bluetooth::PairDeviceOutOfBandSync);
TH_EXPORT_CPP_API_GetCarKeyDfxData(OHOS::Bluetooth::GetCarKeyDfxData);
TH_EXPORT_CPP_API_GetRemoteDeviceTransport(OHOS::Bluetooth::GetRemoteDeviceTransport);
TH_EXPORT_CPP_API_SetCarKeyDfxData(OHOS::Bluetooth::SetCarKeyDfxData);
TH_EXPORT_CPP_API_OnAclStateChange(OHOS::Bluetooth::OnAclStateChange);
TH_EXPORT_CPP_API_OffAclStateChange(OHOS::Bluetooth::OffAclStateChange);
TH_EXPORT_CPP_API_OnScanModeChange(OHOS::Bluetooth::OnScanModeChange);
TH_EXPORT_CPP_API_OffScanModeChange(OHOS::Bluetooth::OffScanModeChange);
// NOLINTEND
