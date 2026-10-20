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

#ifndef BLUETOOTH_PARCEL_DEPTH_H
#define BLUETOOTH_PARCEL_DEPTH_H

#include <cstdint>

namespace OHOS {
namespace Bluetooth {
// GATT 服务允许的合法嵌套层数远小于该值，32 层足以覆盖现网所有服务定义。
constexpr uint32_t BT_PARCEL_MAX_NESTING_DEPTH = 32;

// 说明：当前仓库内仅 BluetoothGattService 存在自嵌套（includeServices_），
// BluetoothGattCharacteristic 只解析描述符、BluetoothGattDescriptor 只解析 uuid，
// 二者不会递归到自身，其守卫为防御性预留，不依赖其触发来阻断递归。
// 深度计数仅在解析侧（ReadFromParcel）生效，写入侧不做限制。
namespace parcel_depth_detail {
// 每个 parcel 类型使用独立的线程局部计数器，避免不同类型互相影响；
// 计数器不对外暴露，使用方只能通过 ParcelDepthExceeded 与 ParcelDepthGuard 访问。
template <typename T>
uint32_t &DepthCounter()
{
    static thread_local uint32_t depth = 0;
    return depth;
}
}  // namespace parcel_depth_detail

template <typename T>
bool ParcelDepthExceeded()
{
    return parcel_depth_detail::DepthCounter<T>() >= BT_PARCEL_MAX_NESTING_DEPTH;
}

// RAII 守卫，保证解析提前返回时深度计数能够正确回退。
template <typename T>
class ParcelDepthGuard final {
public:
    ParcelDepthGuard()
    {
        ++parcel_depth_detail::DepthCounter<T>();
    }

    ~ParcelDepthGuard()
    {
        // 计数不应降到 0 以下，判零避免无符号回绕成极大值后该线程后续解析全部被判超限。
        uint32_t &depth = parcel_depth_detail::DepthCounter<T>();
        if (depth > 0) {
            --depth;
        }
    }

    ParcelDepthGuard(const ParcelDepthGuard &) = delete;
    ParcelDepthGuard &operator=(const ParcelDepthGuard &) = delete;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_DEPTH_H
