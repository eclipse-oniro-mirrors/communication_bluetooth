/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "car_device_type_resolver.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_device"
#endif
#include "log.h"
#include "common_util.h"

#include "adapter_manager.h"
#include "bluetooth_def.h"
#include "bluetooth_hw_interface.h"
#include "classic_adapter.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {

// 公共判定逻辑，通过 GetRemoteDeviceInfo 获取协议栈上报的厂商 ID 与设备类型：
//   vendorId_     == VENDOR_ID_OF_HW_DEVICE(0x12D1)
//   deviceOfType_ == 期望类型(PHONE 0x01 / CAR 0x69) → 手机/车机
// 这两个字段由 echo 9,1 DEVICE_DESCRIP 上报后存入 BluetoothDevice，
// 配对完成时即写入 config 持久化，回连谓词调用时已就绪。
bool CarDeviceTypeResolver::IsHwDeviceOfType(const std::string &addr, uint8_t expectedType,
    const char *caller, const char *fieldLabel) const
{
    // 缓存加密地址，三条日志路径共用，避免重复计算。
    const std::string encryptedAddr = GetEncryptAddr(addr);
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGW("%{public}s: ClassicAdapter null, addr=%{public}s -> false", caller, encryptedAddr.c_str());
        return false;
    }

    Bluetooth::BluetoothRemoteDeviceInfo info;
    int32_t ret = classicAdapter->GetRemoteDeviceInfo(RawAddress(addr), info);
    if (ret != Bluetooth::BT_NO_ERROR) {
        HILOGI("%{public}s: GetRemoteDeviceInfo fail, addr=%{public}s, ret=%{public}d -> false",
               caller, encryptedAddr.c_str(), ret);
        return false;
    }

    bool isHwVendor = (info.vendorId_ == VENDOR_ID_OF_HW_DEVICE);
    bool isTarget = (info.deviceOfType_ == expectedType);
    HILOGI("%{public}s: addr=%{public}s, vendorId=0x%{public}04x, deviceType=0x%{public}02x, "
           "isHwVendor=%{public}d, %{public}s=%{public}d",
           caller, encryptedAddr.c_str(), info.vendorId_, info.deviceOfType_, isHwVendor,
           fieldLabel, isTarget);
    return isHwVendor && isTarget;
}

bool CarDeviceTypeResolver::IsHwPhone(const std::string &addr) const
{
    return IsHwDeviceOfType(addr, static_cast<uint8_t>(HW_ECHO_PRODUCT::PHONE), "IsHwPhone", "isPhone");
}

// 与 IsHwPhone 同样的判定方式，仅期望类型不同：
// HW_CAR_DEVICE_TYPE 宏(remote_device_properties.h:45)定义为 105 即 0x69，与本判定等价。
bool CarDeviceTypeResolver::IsHwCarDevice(const std::string &addr) const
{
    return IsHwDeviceOfType(addr, static_cast<uint8_t>(HW_ECHO_PRODUCT::CAR), "IsHwCarDevice", "isCar");
}

}  // namespace bluetooth
}  // namespace OHOS
