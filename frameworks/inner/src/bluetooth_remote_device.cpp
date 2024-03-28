/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <string>

#include "bluetooth_raw_address.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_device.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_remote_device.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
BluetoothRemoteDevice::BluetoothRemoteDevice(const std::string &addr, const int transport)
{
    address_ = addr;
    transport_ = transport;
}

int BluetoothRemoteDevice::GetDeviceType() const
{
    HILOGI("enter");
    int type = 0;
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), type, "Invalid remote device.");

    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, type, "proxy is nullptr.");

    return hostProxy->GetDeviceType(transport_, address_);
}

bool BluetoothRemoteDevice::IsValidBluetoothRemoteDevice() const
{
    CHECK_AND_RETURN_LOG_RET(BluetoothHost::IsValidBluetoothAddr(address_), false,
        "invalid bluetooth addr, address_: %{public}s", GetEncryptAddr(address_).c_str());

    CHECK_AND_RETURN_LOG_RET(transport_ == BT_TRANSPORT_BREDR ||
        transport_ == BT_TRANSPORT_BLE || transport_ == BT_TRANSPORT_NONE,
        false, "invalid transport type.");
    return true;
}

int BluetoothRemoteDevice::GetTransportType() const
{
    return transport_;
}

int BluetoothRemoteDevice::GetPhonebookPermission() const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), INVALID_VALUE, "Invalid remote device.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, INVALID_VALUE, "proxy is nullptr.");

    return hostProxy->GetPhonebookPermission(address_);
}

bool BluetoothRemoteDevice::SetPhonebookPermission(int permission)
{
    HILOGI("enter, permission: %{public}d", permission);
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), false, "Invalid remote device.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, false, "proxy is nullptr.");
    return hostProxy->SetPhonebookPermission(address_, permission);
}

int BluetoothRemoteDevice::GetMessagePermission() const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), INVALID_VALUE, "Invalid remote device.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, INVALID_VALUE, "proxy is nullptr.");
    return hostProxy->GetMessagePermission(address_);
}

bool BluetoothRemoteDevice::SetMessagePermission(int permission)
{
    HILOGI("enter, permission: %{public}d", permission);
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), false, "Invalid remote device.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, false, "proxy is nullptr.");
    return hostProxy->SetMessagePermission(address_, permission);
}

int BluetoothRemoteDevice::GetPowerMode(void) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), INVALID_VALUE, "Invalid remote device.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, INVALID_VALUE, "proxy is nullptr.");
    return hostProxy->GetPowerMode(address_);
}

std::string BluetoothRemoteDevice::GetDeviceName() const
{
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), INVALID_NAME, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), INVALID_NAME, "bluetooth is off.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, INVALID_NAME, "proxy is nullptr.");
    std::string name = INVALID_NAME;
    hostProxy->GetDeviceName(transport_, address_, name);
    return name;
}

int BluetoothRemoteDevice::GetDeviceName(std::string &name) const
{
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->GetDeviceName(transport_, address_, name);
}

std::string BluetoothRemoteDevice::GetDeviceAlias() const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), INVALID_NAME, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, INVALID_NAME, "proxy is nullptr.");
    return hostProxy->GetDeviceAlias(address_);
}

int32_t BluetoothRemoteDevice::SetDeviceAlias(const std::string &aliasName)
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(
        IsValidBluetoothRemoteDevice() && aliasName != INVALID_NAME, BT_ERR_INVALID_PARAM, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->SetDeviceAlias(address_, aliasName);
}

int BluetoothRemoteDevice::GetDeviceBatteryLevel() const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), INVALID_VALUE, "Invalid remote device.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, INVALID_VALUE, "proxy is nullptr.");
    return hostProxy->GetDeviceBatteryLevel(address_);
}

int BluetoothRemoteDevice::GetPairState(int &pairState) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), INVALID_VALUE, "Invalid remote device.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, INVALID_VALUE, "proxy is nullptr.");
    return hostProxy->GetPairState(transport_, address_, pairState);
}

int BluetoothRemoteDevice::StartPair()
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->StartPair(transport_, address_);
}

int BluetoothRemoteDevice::StartCrediblePair()
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->StartCrediblePair(transport_, address_);
}

int BluetoothRemoteDevice::CancelPairing()
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    if (hostProxy->CancelPairing(transport_, address_)) {
        return BT_NO_ERROR;
    }
    return  BT_ERR_INTERNAL_ERROR;
}

bool BluetoothRemoteDevice::IsBondedFromLocal() const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), false, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, false, "proxy is nullptr.");
    return hostProxy->IsBondedFromLocal(transport_, address_);
}

bool BluetoothRemoteDevice::IsAclConnected() const
{
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), false, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, false, "proxy is nullptr.");
    return hostProxy->IsAclConnected(transport_, address_);
}

bool BluetoothRemoteDevice::IsAclEncrypted() const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), false, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, false, "proxy is nullptr.");
    return hostProxy->IsAclEncrypted(transport_, address_);
}

int BluetoothRemoteDevice::GetDeviceClass(int &cod) const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    int ret = hostProxy->GetDeviceClass(address_, cod);
    return ret;
}

int BluetoothRemoteDevice::GetDeviceProductId(std::string &prodcutId) const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");

    return hostProxy->GetDeviceProductId(address_, prodcutId);
}

int BluetoothRemoteDevice::GetDeviceUuids(std::vector<std::string> &uuids) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->GetDeviceUuids(address_, uuids);
}

int BluetoothRemoteDevice::SetDevicePin(const std::string &pin)
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->SetDevicePin(address_, pin);
}

int BluetoothRemoteDevice::SetDevicePairingConfirmation(bool accept)
{
    HILOGI("enter, accept: %{public}d", accept);
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->SetDevicePairingConfirmation(transport_, address_, accept);
}

bool BluetoothRemoteDevice::SetDevicePasskey(int passkey, bool accept)
{
    HILOGI("enter, accept: %{public}d", accept);
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), false, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, false, "proxy is nullptr.");
    return hostProxy->SetDevicePasskey(transport_, address_, passkey, accept);
}

bool BluetoothRemoteDevice::PairRequestReply(bool accept)
{
    HILOGI("enter, accept: %{public}d", accept);
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), false, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, false, "proxy is nullptr.");
    return hostProxy->PairRequestReply(transport_, address_, accept);
}

bool BluetoothRemoteDevice::ReadRemoteRssiValue()
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), false, "Invalid remote device");
    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, false, "proxy is nullptr.");
    return hostProxy->ReadRemoteRssiValue(address_);
}

int BluetoothRemoteDevice::GetDeviceProductType(int &cod, int &majorClass, int &majorMinorClass) const
{
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "Invalid remote device");

    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    int deviceCod = 0;
    int ret = GetDeviceClass(deviceCod);
    BluetoothDeviceClass deviceClass = BluetoothDeviceClass(deviceCod);
    cod = deviceClass.GetClassOfDevice();
    majorClass = deviceClass.GetMajorClass();
    majorMinorClass = deviceClass.GetMajorMinorClass();
    if (cod == 0) {
        HILOGW("cod = %{public}d", cod);
        cod = BluetoothDevice::MAJOR_UNCATEGORIZED;
        majorClass = BluetoothDevice::MAJOR_UNCATEGORIZED;
        majorMinorClass = BluetoothDevice::MAJOR_UNCATEGORIZED;
    }
    HILOGD("cod = %{public}#X, majorClass = %{public}#X, majorMinorClass = %{public}#X",
        cod, majorClass, majorMinorClass);

    return ret;
}

int32_t BluetoothRemoteDevice::SetDeviceCustomType(int32_t deviceType) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice() && deviceType >= DeviceType::DEVICE_TYPE_DEFAULT &&
        deviceType <= DeviceType::DEVICE_TYPE_SPEAKER, BT_ERR_INVALID_PARAM,
        "Invalid remote device or Invalid device Type");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->SetDeviceCustomType(address_, deviceType);
}

int32_t BluetoothRemoteDevice::GetDeviceCustomType(int32_t &deviceType) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "Invalid remote device");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> hostProxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr.");
    return hostProxy->GetDeviceCustomType(address_, deviceType);
}
}  // namespace Bluetooth
}  // namespace OHOS
